//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_tcp_client.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
// Email: aroyer.qc@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------------------------------

//------ Note(s) ----------------------------------------------------------------------------------
//
//  Description:    Minimal deterministic TCP client for NanoIP stack.
//                  - Active open only (client mode)
//                  - Non-blocking, state-machine driven
//                  - Zero dynamic allocation
//                  - Integrates with Socket + NetworkContext
//                  - Provides a byte-stream interface for MQTT/HTTP/etc.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (IP_USE_TCP_CLIENT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext* pContext
//                      Pointer to the network context used to allocate sockets and
//                      access lower-level IP/TCP services.
//
//  Return:         bool
//                  - true  : TCP client initialized and ready to connect
//                  - false : Initialization failed (invalid context or socket allocation error)
//
//  Description:
//      Initializes the TCP client instance with the provided network context. This function
//      allocates a TCP socket from the socket manager, configures it for non-blocking
//      operation, resets all internal state variables, and prepares the client for an
//      active connection attempt via Connect().
//
//-------------------------------------------------------------------------------------------------
bool TCP_Client::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;

    IP_Manager* pIP = m_pContext->GetIP_Manager();
    if(pIP == nullptr)
    {
        m_State = TCP_CLIENT_STATE_ERROR;
        return false;
    }

    SocketManager* pSockMgr = pIP->GetSocketManager();
    if(pSockMgr == nullptr)
    {
        m_State = TCP_CLIENT_STATE_ERROR;
        return false;
    }

    // Allocate a TCP socket
    m_pSocket = pSockMgr->AllocSocket(SOCKET_TYPE_STREAM);
    if(m_pSocket == nullptr)
    {
        m_State = TCP_CLIENT_STATE_ERROR;
        return false;
    }

    // Enable non-blocking mode
    bool NonBlocking = true;
    m_pSocket->SetOption(SOCKET_OPT_NON_BLOCKING, &NonBlocking, sizeof(bool));

    // Bind to an ephemeral local port (0 = auto-assign)
    SystemState_e State = m_pSocket->Bind(0);
    if(State != SYS_READY)
    {
        pSockMgr->FreeSocket(&m_pSocket);
        m_pSocket = nullptr;
        m_State   = TCP_CLIENT_STATE_ERROR;
        return false;
    }

    // Reset TCP state machine
    m_State = TCP_CLIENT_STATE_CLOSED;

    // Clear addressing
    m_ServerIP   = 0;
    m_ServerPort = 0;

    // Clear sequence/ack numbers
    m_SeqNumber = 0;
    m_AckNumber = 0;

    // Window sizes (will be updated after SYN/SYN+ACK)
    m_RemoteWindow = 0;
    m_LocalWindow  = TCP_DEFAULT_WINDOW_SIZE;   // define as needed

    // Timers
    m_LastSendTick      = 0;
    m_LastReceivedTick  = 0;
    m_RetransmitStart   = 0;
    m_ConnectionStart   = 0;

    // Retransmission tracking
    m_RetransmitPending = false;
    m_LastFlags         = 0;
    m_LastPayloadLength = 0;

    m_pLastSegment      = nullptr;
    m_LastSegmentLength = 0;

    // Temporary buffers (to be replaced with memory-pool allocations)
    m_TxLength = 0;
    m_RxLength = 0;

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Connect
//
//  Parameter(s):   const IP_Address_t* pServerIP
//                      Destination server IP address
//
//                  uint16_t Port
//                      Destination TCP port
//
//  Return:         bool
//                  - true  : SYN sent, client is now in SYN_SENT state
//                  - false : Invalid state, socket error, or failed to send SYN
//
//  Description:
//      Initiates an active TCP connection to the specified server. This function stores the
//      remote address, generates an initial sequence number, constructs a SYN segment, and
//      transitions the client into the SYN_SENT state. The handshake is completed asynchronously
//      through Process() and HandleIncoming().
//
//-------------------------------------------------------------------------------------------------
bool TCP_Client::Connect(const IP_Address_t* pServerIP, uint16_t Port)
{
    if((pServerIP == nullptr) || (m_pSocket == nullptr))
    {
        m_State = TCP_CLIENT_STATE_ERROR;
        return false;
    }

    // Only allowed from CLOSED state
    if(m_State != TCP_CLIENT_STATE_CLOSED)
    {
        return false;
    }

    // Store server info
    m_ServerIP   = *pServerIP;
    m_ServerPort = Port;

    // Generate Initial Sequence Number (ISN)
    // A simple monotonic tick-based ISN is fine for embedded systems
    m_SeqNumber = (uint32_t)GetTick();

    // No data expected yet
    m_AckNumber = 0;

    // Reset window tracking
    m_RemoteWindow = 0;
    m_LocalWindow  = TCP_DEFAULT_WINDOW_SIZE;

    // Reset timers
    m_ConnectionStart  = GetTick();
    m_LastSendTick     = 0;
    m_LastReceivedTick = 0;

    // Attempt to send SYN
    if(SendSyn() == false)
    {
        m_State = TCP_CLIENT_STATE_ERROR;
        return false;
    }

    // SYN successfully sent
    m_State = TCP_CLIENT_STATE_SYN_SENT;
    m_RetransmitStart = GetTick();
    m_RetransmitPending = true;

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Send
//
//  Parameter(s):   const uint8_t* pData
//                      Pointer to the application payload.
//
//                  size_t Length
//                      Number of bytes to send.
//
//  Return:         size_t
//                  Number of bytes accepted for transmission.
//                  Returns 0 if not connected or busy.
//
//  Description:
//      Queues application data for transmission. This function does not block and does not
//      guarantee immediate delivery. The actual TCP segment is constructed and transmitted
//      through SendSegment(), and retransmissions are handled by Process().
//
//      If the client is not in ESTABLISHED state, or if a retransmission is pending, the
//      function returns 0 to indicate that the caller should retry later.
//
//-------------------------------------------------------------------------------------------------
size_t TCP_Client::Send(const uint8_t* pData, size_t Length)
{
    if((pData == nullptr) || (Length == 0))
    {
        return 0;
    }

    // Must be connected
    if(m_State != TCP_CLIENT_STATE_ESTABLISHED)
    {
        return 0;
    }

    // Do not send new data while retransmission is pending
    if(m_RetransmitPending)
    {
        return 0;
    }

    // Limit to TX buffer size (temporary until memory-pool version)
    if(Length > sizeof(m_TxBuffer))
    {
        Length = sizeof(m_TxBuffer);
    }

    // Copy into TX buffer
    memcpy(m_TxBuffer, pData, Length);
    m_TxLength = Length;

    // Send immediately (non-blocking)
    if(!SendSegment(m_TxBuffer, m_TxLength, true))   // PSH flag = true
    {
        // Failed to send
        m_TxLength = 0;
        return 0;
    }

    // Data accepted for transmission
    return Length;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Receive
//
//  Parameter(s):   uint8_t* pBuffer
//                      Destination buffer provided by the caller.
//
//                  size_t MaxLength
//                      Maximum number of bytes the caller can accept.
//
//  Return:         size_t
//                  Number of bytes copied into pBuffer.
//                  Returns 0 if no data is available.
//
//  Description:
//      Copies any previously received TCP payload (stored in m_RxBuffer by
//      ParseTCP_Header()) into the caller's buffer. This function does not
//      block and does not attempt to read from the socket directly; all
//      incoming data is processed asynchronously through Process() and
//      HandleIncoming().
//
//      After copying, the internal RX length is cleared, making the buffer
//      ready for the next incoming segment.
//
//-------------------------------------------------------------------------------------------------
size_t TCP_Client::Receive(uint8_t* pBuffer, size_t MaxLength)
{
    if((pBuffer == nullptr) || (MaxLength == 0))
    {
        return 0;
    }

    // No data available
    if(m_RxLength == 0)
    {
        return 0;
    }

    // Determine how many bytes we can return
    size_t ToCopy = (m_RxLength <= MaxLength) ? m_RxLength : MaxLength;

    memcpy(pBuffer, m_RxBuffer, ToCopy);

    // Clear internal RX buffer state
    m_RxLength = 0;

    return ToCopy;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Close
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:
//      Initiates an active close of the TCP connection. If the client is in the ESTABLISHED
//      state, a FIN segment is transmitted and the state machine transitions to FIN_WAIT_1.
//      The remainder of the close handshake (FIN_WAIT_2 → TIME_WAIT → CLOSED) is handled
//      asynchronously through Process() and HandleIncoming().
//
//-------------------------------------------------------------------------------------------------
void TCP_Client::Close(void)
{
    // Only valid from ESTABLISHED
    if(m_State != TCP_CLIENT_STATE_ESTABLISHED)
    {
        return;
    }

    // Do not send FIN if retransmission is pending
    if(m_RetransmitPending)
    {
        return;
    }

    // Send FIN (this moves state to FIN_WAIT_1 internally)
    if(!SendFin())
    {
        m_State = TCP_CLIENT_STATE_ERROR;
        return;
    }

    // FIN_WAIT_1 is set inside SendFin()
}

//-------------------------------------------------------------------------------------------------
//  Name:           Process
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:
//      Drives the TCP client state machine. This function must be called periodically from the
//      main loop. It performs the following tasks:
//
//          - Polls the socket for incoming TCP segments
//          - Dispatches packets to HandleIncoming()
//          - Manages retransmission timers
//          - Handles connection timeout (SYN_SENT)
//          - Handles FIN_WAIT and TIME_WAIT timers
//
//      This function is fully non-blocking and performs no dynamic waiting.
//-------------------------------------------------------------------------------------------------
void TCP_Client::Process(void)
{
    if(m_pSocket == nullptr)
    {
        return;
    }

    //----------------------------------------------------------------------
    // 1. Poll socket for incoming packets
    //----------------------------------------------------------------------
    IP_EthernetPacket_t* pPacket = nullptr;
    size_t Bytes = 0;

    SystemState_e State = m_pSocket->RecvPacket(&pPacket, &Bytes);

    if(State == SYS_READY && pPacket != nullptr)
    {
        HandleIncoming(pPacket);

        // Packet ownership returns to socket manager
        m_pSocket->ReleasePacket(&pPacket);
    }

    //----------------------------------------------------------------------
    // 2. Handle retransmissions
    //----------------------------------------------------------------------
    RetransmitIfNeeded();

    //----------------------------------------------------------------------
    // 3. Handle connection timeout (SYN_SENT)
    //----------------------------------------------------------------------
    if(m_State == TCP_CLIENT_STATE_SYN_SENT)
    {
        TickCount_t Now = GetTick();
        if((Now - m_ConnectionStart) > TCP_CONNECT_TIMEOUT_MS)
        {
            m_State = TCP_CLIENT_STATE_ERROR;
            return;
        }
    }

    //----------------------------------------------------------------------
    // 4. Handle FIN_WAIT_1 → FIN_WAIT_2 transition
    //    (ACK of our FIN is processed in HandleIncoming)
    //----------------------------------------------------------------------
    // Nothing to do here — state changes happen in HandleIncoming()

    //----------------------------------------------------------------------
    // 5. Handle TIME_WAIT timeout
    //----------------------------------------------------------------------
    if(m_State == TCP_CLIENT_STATE_TIME_WAIT)
    {
        TickCount_t Now = GetTick();
        if((Now - m_LastReceivedTick) > TCP_TIME_WAIT_MS)
        {
            m_State = TCP_CLIENT_STATE_CLOSED;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//  Name:           SendSyn
//
//  Parameter(s):   None
//
//  Return:         bool
//                  - true  : SYN segment sent successfully
//                  - false : Failed to allocate buffer or send segment
//
//  Description:
//      Constructs and transmits a TCP SYN segment to initiate the connection handshake.
//      This function builds a minimal TCP header with the SYN flag set, computes the
//      checksum, and sends the segment through the underlying socket. Retransmission
//      metadata is updated so that Process() can retry if no SYN+ACK is received.
//-------------------------------------------------------------------------------------------------
bool TCP_Client::SendSyn(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    // Allocate a buffer for TCP header (no payload)
    uint8_t* pBuffer = (uint8_t*)pMemoryPool->Alloc(sizeof(TCP_Header_t), MEM_DBG_TCP_TX);
    if(pBuffer == nullptr)
    {
        return false;
    }

    TCP_Header_t* pTCP = (TCP_Header_t*)pBuffer;

    // Clear header
    memset(pTCP, 0, sizeof(TCP_Header_t));

    // Fill TCP header fields
    pTCP->SourcePort      = HTONS(m_pSocket->GetLocalPort());
    pTCP->DestinationPort = HTONS(m_ServerPort);
    pTCP->SeqNumber       = HTONL(m_SeqNumber);
    pTCP->AckNumber       = 0;                     // No ACK in SYN
    pTCP->DataOffset      = (sizeof(TCP_Header_t) / 4) << 4;
    pTCP->Flags           = TCP_FLAG_SYN;
    pTCP->WindowSize      = HTONS(m_LocalWindow);
    pTCP->UrgentPointer   = 0;

    // Compute checksum using your IP/TCP layer
    pTCP->Checksum = 0;
    pTCP->Checksum = m_pContext->GetIP_Manager()->ComputeTCPChecksum(
                        pTCP,
                        sizeof(TCP_Header_t),
                        m_pContext->GetIP_Manager()->GetLocalIP(),
                        m_ServerIP);

    // Send SYN segment
    SystemState_e State = m_pSocket->SendTo(
                              pBuffer,
                              sizeof(TCP_Header_t),
                              &m_ServerIP,
                              m_ServerPort);

    // Free buffer
    pMemoryPool->Free((void**)&pBuffer);

    if(State != SYS_READY)
    {
        return false;
    }

    // Store retransmission info
    m_RetransmitPending  = true;
    m_LastFlags          = TCP_FLAG_SYN;
    m_LastPayloadLength  = 0;
    m_LastSegmentLength  = sizeof(TCP_Header_t);
    m_LastSendTick       = GetTick();

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           SendAck
//
//  Parameter(s):   uint32_t AckNumber
//                      Acknowledgment number to include in the TCP header.
//
//  Return:         bool
//                  - true  : ACK segment sent successfully
//                  - false : Failed to allocate buffer or send segment
//
//  Description:
//      Constructs and transmits a pure TCP ACK segment. This function builds a minimal TCP
//      header with the ACK flag set, acknowledges the specified sequence number, computes the
//      checksum, and sends the segment through the underlying socket. Pure ACKs do not carry
//      payload and are not retransmitted. Retransmission metadata is updated for consistency.
//-------------------------------------------------------------------------------------------------
bool TCP_Client::SendAck(uint32_t AckNumber)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    // Allocate buffer for TCP header (no payload)
    uint8_t* pBuffer = (uint8_t*)pMemoryPool->Alloc(sizeof(TCP_Header_t), MEM_DBG_TCP_TX);
    if(pBuffer == nullptr)
    {
        return false;
    }

    TCP_Header_t* pTCP = (TCP_Header_t*)pBuffer;

    // Clear header
    memset(pTCP, 0, sizeof(TCP_Header_t));

    //----------------------------------------------------------------------
    // Fill TCP header
    //----------------------------------------------------------------------
    pTCP->SourcePort      = HTONS(m_pSocket->GetLocalPort());
    pTCP->DestinationPort = HTONS(m_ServerPort);

    // Our current sequence number (unchanged for pure ACK)
    pTCP->SeqNumber       = HTONL(m_SeqNumber);

    // Acknowledging the server's sequence
    pTCP->AckNumber       = HTONL(AckNumber);

    // Header length (no options)
    pTCP->DataOffset      = (sizeof(TCP_Header_t) / 4) << 4;

    // ACK flag only
    pTCP->Flags           = TCP_FLAG_ACK;

    // Our advertised window
    pTCP->WindowSize      = HTONS(m_LocalWindow);

    pTCP->UrgentPointer   = 0;

    //----------------------------------------------------------------------
    // Compute checksum
    //----------------------------------------------------------------------
    pTCP->Checksum = 0;
    pTCP->Checksum = m_pContext->GetIP_Manager()->ComputeTCPChecksum(
                        pTCP,
                        sizeof(TCP_Header_t),
                        m_pContext->GetIP_Manager()->GetLocalIP(),
                        m_ServerIP);

    //----------------------------------------------------------------------
    // Send segment
    //----------------------------------------------------------------------
    SystemState_e State = m_pSocket->SendTo(
                              pBuffer,
                              sizeof(TCP_Header_t),
                              &m_ServerIP,
                              m_ServerPort);

    // Free buffer
    pMemoryPool->Free((void**)&pBuffer);

    if(State != SYS_READY)
    {
        return false;
    }

    //----------------------------------------------------------------------
    // Update retransmission tracking
    //----------------------------------------------------------------------
    m_RetransmitPending  = false;            // Pure ACKs are not retransmitted
    m_LastFlags          = TCP_FLAG_ACK;
    m_LastPayloadLength  = 0;
    m_LastSegmentLength  = sizeof(TCP_Header_t);
    m_LastSendTick       = GetTick();

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           SendFin
//
//  Parameter(s):   None
//
//  Return:         bool
//                  - true  : FIN segment sent successfully
//                  - false : Failed to allocate buffer or send segment
//
//  Description:
//      Constructs and transmits a TCP FIN segment to initiate an active close of the connection.
//      This function builds a minimal TCP header with the FIN and ACK flags set, computes the
//      checksum, and sends the segment through the underlying socket. The sequence number is
//      advanced by one to account for the FIN. Retransmission metadata is updated so that
//      Process() can retry if the FIN acknowledgment is not received.
//-------------------------------------------------------------------------------------------------
bool TCP_Client::SendFin(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    //----------------------------------------------------------------------
    // Allocate buffer: TCP header only (FIN never carries payload)
    //----------------------------------------------------------------------
    size_t TotalLength = sizeof(TCP_Header_t);

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->Alloc(TotalLength, MEM_DBG_TCP_TX);
    if(pBuffer == nullptr)
    {
        return false;
    }

    TCP_Header_t* pTCP = (TCP_Header_t*)pBuffer;

    // Clear header
    memset(pTCP, 0, sizeof(TCP_Header_t));

    //----------------------------------------------------------------------
    // Fill TCP header
    //----------------------------------------------------------------------
    pTCP->SourcePort      = HTONS(m_pSocket->GetLocalPort());
    pTCP->DestinationPort = HTONS(m_ServerPort);

    // FIN consumes one sequence number
    pTCP->SeqNumber       = HTONL(m_SeqNumber);

    // Acknowledge the next expected byte from server
    pTCP->AckNumber       = HTONL(m_AckNumber);

    // Header length (no options)
    pTCP->DataOffset      = (sizeof(TCP_Header_t) / 4) << 4;

    // FIN + ACK
    uint8_t Flags = TCP_FLAG_FIN | TCP_FLAG_ACK;
    pTCP->Flags = Flags;

    // Advertise our window
    pTCP->WindowSize = HTONS(m_LocalWindow);

    //----------------------------------------------------------------------
    // Compute checksum
    //----------------------------------------------------------------------
    pTCP->Checksum = 0;
    pTCP->Checksum = m_pContext->GetIP_Manager()->ComputeTCPChecksum(
                        pTCP,
                        TotalLength,
                        m_pContext->GetIP_Manager()->GetLocalIP(),
                        m_ServerIP);

    //----------------------------------------------------------------------
    // Send segment
    //----------------------------------------------------------------------
    SystemState_e State = m_pSocket->SendTo(
                              pBuffer,
                              TotalLength,
                              &m_ServerIP,
                              m_ServerPort);

    // Free buffer
    pMemoryPool->Free((void**)&pBuffer);

    if(State != SYS_READY)
    {
        return false;
    }

    //----------------------------------------------------------------------
    // FIN consumes one sequence number
    //----------------------------------------------------------------------
    m_SeqNumber += 1;

    //----------------------------------------------------------------------
    // Store retransmission info
    //----------------------------------------------------------------------
    m_RetransmitPending  = true;
    m_LastFlags          = Flags;
    m_LastPayloadLength  = 0;
    m_LastSegmentLength  = TotalLength;
    m_LastSendTick       = GetTick();

    //----------------------------------------------------------------------
    // Move to FIN_WAIT_1
    //----------------------------------------------------------------------
    m_State = TCP_CLIENT_STATE_FIN_WAIT_1;

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           HandleIncoming
//
//  Parameter(s):   IP_EthernetPacket_t* pPacket
//                      Pointer to the received Ethernet/IP/TCP packet.
//
//  Return:         void
//
//  Description:
//      Processes an incoming TCP segment and updates the client state machine accordingly.
//      This function validates the TCP header, extracts flags, updates acknowledgment and
//      sequence tracking, and dispatches payload data to the receive buffer. It handles all
//      state transitions for the TCP handshake, data exchange, and connection teardown,
//      including:
//
//          - SYN+ACK processing during connection establishment
//          - ACK processing for sent segments (SYN, data, FIN)
//          - Payload extraction and buffering
//          - FIN reception and close‑sequence transitions
//          - RST reception and error handling
//
//      This function performs no retransmissions; those are handled by Process() through
//      RetransmitIfNeeded().
//-------------------------------------------------------------------------------------------------
bool TCP_Client::HandleIncoming(IP_EthernetPacket_t* pPacket)
{
    if(pPacket == nullptr)
    {
        return false;
    }

    // Parse TCP header (fills internal fields)
    if(!ParseTCP_Header(pPacket))
    {
        return false;
    }

    // Extract header pointer
    TCP_Header_t* pTCP = pPacket->TCP;

    uint8_t Flags = pTCP->Flags;

    //----------------------------------------------------------------------
    // 1. SYN_SENT -> expecting SYN+ACK
    //----------------------------------------------------------------------
    if(m_State == TCP_CLIENT_STATE_SYN_SENT)
    {
        // Must be SYN+ACK
        if((Flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) != (TCP_FLAG_SYN | TCP_FLAG_ACK))
        {
            return false;
        }

        // Validate ACK number
        uint32_t Ack = NTOHL(pTCP->AckNumber);
        if(Ack != (m_SeqNumber + 1))
        {
            // Wrong ACK → ignore
            return false;
        }

        // Extract server sequence number
        uint32_t ServerSeq = NTOHL(pTCP->SeqNumber);
        m_AckNumber = ServerSeq + 1;

        // Extract server window
        m_RemoteWindow = NTOHS(pTCP->WindowSize);

        // Send final ACK of handshake
        if(!SendAck(m_AckNumber))
        {
            m_State = TCP_CLIENT_STATE_ERROR;
            return false;
        }

        // Handshake complete
        m_State = TCP_CLIENT_STATE_ESTABLISHED;
        m_RetransmitPending = false;

        return true;
    }

    //----------------------------------------------------------------------
    // 2. ESTABLISHED → data or ACK handling (later)
    //----------------------------------------------------------------------
    if(m_State == TCP_CLIENT_STATE_ESTABLISHED)
    {
        // TODO: handle data, ACKs, PSH, FIN, etc.
        return true;
    }

    //----------------------------------------------------------------------
    // 3. FIN_WAIT states (later)
    //----------------------------------------------------------------------
    return false;
}

//-------------------------------------------------------------------------------------------------
//  Name:           ParseTCP_Header
//
//  Parameter(s):   IP_EthernetPacket_t* pPacket
//                      Pointer to a fully received Ethernet/IP/TCP packet.
//
//  Return:         bool
//                  - true  : TCP header parsed successfully
//                  - false : Malformed header or unsupported segment
//
//  Description:
//      Extracts and validates the TCP header from the incoming packet. This function verifies
//      that the TCP header length is valid, extracts sequence and acknowledgment numbers,
//      updates the remote window size, and exposes the payload pointer and length for higher-
//      level processing. No state transitions occur here; this function only decodes fields.
//
//-------------------------------------------------------------------------------------------------
bool TCP_Client::ParseTCP_Header(IP_EthernetPacket_t* pPacket)
{
    if(pPacket == nullptr)
    {
        return false;
    }

    TCP_Header_t* pTCP = pPacket->TCP;
    if(pTCP == nullptr)
    {
        return false;
    }

    //----------------------------------------------------------------------
    // Validate header length
    //----------------------------------------------------------------------
    uint8_t DataOffset = (pTCP->DataOffset >> 4) & 0x0F;   // in 32-bit words
    uint16_t HeaderLength = (uint16_t)DataOffset * 4;

    if(HeaderLength < sizeof(TCP_Header_t))
    {
        // Invalid or too small
        return false;
    }

    if(HeaderLength > pPacket->IP_TotalLength)
    {
        // Header claims more bytes than the IP packet contains
        return false;
    }

    //----------------------------------------------------------------------
    // Extract fields
    //----------------------------------------------------------------------
    uint16_t SrcPort = NTOHS(pTCP->SourcePort);
    uint16_t DstPort = NTOHS(pTCP->DestinationPort);

    uint32_t Seq     = NTOHL(pTCP->SeqNumber);
    uint32_t Ack     = NTOHL(pTCP->AckNumber);

    uint8_t  Flags   = pTCP->Flags;
    uint16_t Window  = NTOHS(pTCP->WindowSize);

    //----------------------------------------------------------------------
    // Update internal tracking
    //----------------------------------------------------------------------
    m_RemoteWindow = Window;

    // Store last received tick for timeout logic
    m_LastReceivedTick = GetTick();

    //----------------------------------------------------------------------
    // Expose payload pointer and length
    //----------------------------------------------------------------------
    uint16_t PayloadOffset = HeaderLength;
    uint16_t PayloadLength = pPacket->IP_TotalLength - HeaderLength;

    m_RxLength = PayloadLength;

    if(PayloadLength > 0)
    {
        // Copy payload into RX buffer (temporary until memory-pool version)
        if(PayloadLength <= sizeof(m_RxBuffer))
        {
            memcpy(m_RxBuffer, ((uint8_t*)pTCP) + HeaderLength, PayloadLength);
        }
        else
        {
            // Payload too large for buffer
            return false;
        }
    }

    //----------------------------------------------------------------------
    // Store extracted header fields for HandleIncoming()
    //----------------------------------------------------------------------
    m_LastFlags = Flags;

    // These are NOT applied to m_SeqNumber/m_AckNumber here.
    // HandleIncoming() decides what to do with them.
    pPacket->TCP_Seq = Seq;
    pPacket->TCP_Ack = Ack;

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           RetransmitIfNeeded
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:
//      Handles retransmission of the last unacknowledged TCP segment. This function is called
//      periodically from Process(). If the retransmission timeout expires and no ACK has been
//      received, the last segment (SYN, data, or FIN) is retransmitted.
//
//      If the retry limit is exceeded, the connection transitions to ERROR.
//-------------------------------------------------------------------------------------------------
void TCP_Client::RetransmitIfNeeded(void)
{
    if(!m_RetransmitPending)
    {
        return;
    }

    TickCount_t Now = GetTick();

    // Has the retransmission timeout expired?
    if((Now - m_LastSendTick) < TCP_RETRANSMIT_TIMEOUT_MS)
    {
        return;
    }

    //----------------------------------------------------------------------
    // Too many retries?
    //----------------------------------------------------------------------
    static const uint8_t MaxRetries = 5;

    if(m_pSocket->GetRetryCount() >= MaxRetries)
    {
        m_State = TCP_CLIENT_STATE_ERROR;
        m_RetransmitPending = false;
        return;
    }

    m_pSocket->IncrementRetryCount();

    //----------------------------------------------------------------------
    // Retransmit based on last flags
    //----------------------------------------------------------------------
    if(m_LastFlags & TCP_FLAG_SYN)
    {
        // Retransmit SYN
        SendSyn();
    }
    else if(m_LastFlags & TCP_FLAG_FIN)
    {
        // Retransmit FIN
        SendFin();
    }
    else
    {
        // Retransmit data segment
        // (Payload is still in m_TxBuffer)
        SendSegment(m_TxBuffer, m_LastPayloadLength, (m_LastFlags & TCP_FLAG_PSH) != 0);
    }

    // Update timestamp
    m_LastSendTick = Now;
}

//-------------------------------------------------------------------------------------------------
//  Name:           UpdateTimers
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:
//      Updates internal timing information used by the TCP state machine. This function does not
//      perform retransmissions or state transitions directly; it only updates timestamps and
//      checks for timeout expiration. Higher-level logic in Process() reacts to these values.
//
//-------------------------------------------------------------------------------------------------
void TCP_Client::UpdateTimers(void)
{
    TickCount_t Now = GetTick();

    //----------------------------------------------------------------------
    // SYN_SENT timeout (connection attempt)
    //----------------------------------------------------------------------
    if(m_State == TCP_CLIENT_STATE_SYN_SENT)
    {
        if((Now - m_ConnectionStart) > TCP_CONNECT_TIMEOUT_MS)
        {
            m_State = TCP_CLIENT_STATE_ERROR;
            return;
        }
    }

    //----------------------------------------------------------------------
    // FIN_WAIT_1 and FIN_WAIT_2 do not require timer actions here.
    // State transitions occur in HandleIncoming().
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    // TIME_WAIT expiration
    //----------------------------------------------------------------------
    if(m_State == TCP_CLIENT_STATE_TIME_WAIT)
    {
        if((Now - m_LastReceivedTick) > TCP_TIME_WAIT_MS)
        {
            m_State = TCP_CLIENT_STATE_CLOSED;
            return;
        }
    }

    //----------------------------------------------------------------------
    // Retransmission timer is handled in RetransmitIfNeeded()
    // This function only updates timestamps.
    //----------------------------------------------------------------------
}

//---------------------------------------------------------------------------------------------

#endif // (IP_USE_TCP_CLIENT == DEF_ENABLED)
