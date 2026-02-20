//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_tcp.cpp
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

#if (IP_USE_TCP_CLIENT == DEF_ENABLED) || (IP_USE_TCP_SERVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// TCP Control Flags (RFC 793)
#define TCP_FLAG_FIN            0x01    // Finish
#define TCP_FLAG_SYN            0x02    // Synchronize sequence numbers
#define TCP_FLAG_RST            0x04    // Reset connection
#define TCP_FLAG_PSH            0x08    // Push function
#define TCP_FLAG_ACK            0x10    // Acknowledgment field significant
#define TCP_FLAG_URG            0x20    // Urgent pointer field significant
#define TCP_FLAG_ECE            0x40    // ECN-Echo (RFC 3168)
#define TCP_FLAG_CWR            0x80    // Congestion Window Reduced (RFC 3168)

#define TCP_RETRANSMIT_TIMEOUT  1000
#define TCP_TIME_WAIT_TIMEOUT   30000

//-------------------------------------------------------------------------------------------------
//  Name:           TCP_Socket (constructor)
//
//  Parameter(s):   NetworkContext& Context
//                      Reference to the global network context used by the base Socket class.
//
//                  TCP_Manager& TCP
//                      Reference to the TCP manager that owns this socket. The socket stores a
//                      back-pointer so it can request segment transmission (SYN, ACK, FIN, data).
//
//  Return:         None
//
//  Description:    Initializes a TCP socket instance. This constructor sets all TCP state
//                  variables to their defaults, clears internal buffers, initializes sequence and
//                  acknowledgment numbers, and places the socket into the CLOSED state.
//
//                  The socket does not become active until Bind(), Connect(), or EnterListen()
//                  is called. The TCP_Manager reference is stored so the socket can invoke
//                  SendSegment() and participate in the TCP state machine.
//-------------------------------------------------------------------------------------------------
TCP_Socket::TCP_Socket(NetworkContext& Context, TCP_Manager& TCP) : Socket(Context)
{
    m_pTCP              = &TCP;                     // Store back-pointer to TCP manager
    m_State             = TCP_STATE_CLOSED;

    m_SeqNumber         = 0;
    m_AckNumber         = 0;

    m_RemoteWindow      = 0;
    m_LocalWindow       = TCP_DEFAULT_WINDOW_SIZE;

    m_LastSendTick      = 0;
    m_LastReceivedTick  = 0;
    m_RetransmitStart   = 0;

    m_RetransmitPending = false;
    m_LastFlags         = 0;
    m_LastPayloadLength = 0;

    m_TX_Length         = 0;
    m_RX_Length         = 0;

    memset(m_TX_Buffer, 0, sizeof(m_TX_Buffer));
    memset(m_RX_Buffer, 0, sizeof(m_RX_Buffer));
}

//-------------------------------------------------------------------------------------------------
//  Name:           Send
//
//  Parameter(s):   const uint8_t* pData
//                      Pointer to the application-provided payload buffer.
//
//                  size_t Length
//                      Number of bytes to send. Must not exceed the internal TX buffer size.
//
//  Return:         size_t
//                      Number of bytes accepted for transmission. Returns 0 if the socket is not
//                      in ESTABLISHED state, if retransmission is pending, or if Length exceeds
//                      the TX buffer capacity.
//
//  Description:    Sends application data over an established TCP connection. This function
//                  copies the payload into the socket's internal TX buffer, constructs a TCP
//                  segment via the TCP_Manager, and initiates retransmission tracking.
//
//                  - Only valid when the socket is in ESTABLISHED state.
//                  - Sequence numbers are advanced by SendSegment().
//                  - Retransmission tracking is enabled so that Process() can retry the segment
//                    if no ACK is received within the timeout interval.
//-------------------------------------------------------------------------------------------------
size_t TCP_Socket::Send(const uint8_t* pData, size_t Length)
{
    // Must be connected
    if(m_State != TCP_STATE_ESTABLISHED)
    {
        return 0;
    }

    // Cannot send while retransmission is pending
    if(m_RetransmitPending)
    {
        return 0;
    }

    // Validate length
    if((pData == nullptr) || (Length == 0) || (Length > sizeof(m_TX_Buffer)))
    {
        return 0;
    }

    // Copy payload into TX buffer
    memcpy(m_TX_Buffer, pData, Length);
    m_TX_Length = Length;

    // Send PSH+ACK data segment
    const uint8_t Flags = TCP_FLAG_ACK | TCP_FLAG_PSH;

    if(m_pTCP->SendSegment(this, m_TX_Buffer, Length, Flags) == false)
    {
        return 0;
    }

    // Enable retransmission tracking
    m_LastFlags         = Flags;
    m_LastPayloadLength = Length;
    m_RetransmitStart   = GetTick();
    m_RetransmitPending = true;

    return Length;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Receive
//
//  Parameter(s):   uint8_t* pBuffer
//                      Pointer to the application-provided buffer where received data will be
//                      copied.
//
//                  size_t MaxLength
//                      Maximum number of bytes the application is prepared to receive.
//
//  Return:         size_t
//                      Number of bytes copied into pBuffer. Returns 0 if no data is available,
//                      if the socket is not in ESTABLISHED state, or if pBuffer is invalid.
//
//  Description:    Retrieves data previously received from the remote peer. Incoming TCP payloads
//                  are stored in the socket's internal RX buffer by the TCP_Manager when segments
//                  arrive. This function copies the buffered data into the application buffer and
//                  clears the internal RX buffer.
//
//                  - Only valid when the socket is in ESTABLISHED state.
//                  - Does not generate ACKs; ACKs are sent when data is received.
//                  - The internal RX buffer is cleared after the read.
//-------------------------------------------------------------------------------------------------
size_t TCP_Socket::Receive(uint8_t* pBuffer, size_t MaxLength)
{
    // Must be connected
    if(m_State != TCP_STATE_ESTABLISHED)
    {
        return 0;
    }

    // Validate parameters
    if((pBuffer == nullptr) || (MaxLength == 0))
    {
        return 0;
    }

    // No data available
    if(m_RX_Length == 0)
    {
        return 0;
    }

    // Determine how many bytes to copy
    size_t ToCopy = (m_RX_Length <= MaxLength) ? m_RX_Length : MaxLength;

    // Copy data to application buffer
    memcpy(pBuffer, m_RX_Buffer, ToCopy);

    // Clear internal RX buffer
    m_RX_Length = 0;
    memset(m_RX_Buffer, 0, sizeof(m_RX_Buffer));

    return ToCopy;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Close
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Initiates an active close of the TCP connection. This function sends a FIN+ACK
//                  segment to the remote peer and transitions the socket into the appropriate
//                  closing state:
//
//                      - ESTABLISHED → FIN_WAIT_1   (active close)
//                      - CLOSE_WAIT  → LAST_ACK     (passive close)
//
//                  The remainder of the TCP close handshake (ACK of our FIN, remote FIN, final
//                  ACK, TIME_WAIT, etc.) is handled asynchronously by the TCP_Manager via
//                  Process() and ProcessIncomingFlags().
//
//                  This function does NOT free the socket or remove it from the server table;
//                  socket lifecycle management is handled by the TCP_Manager and application.
//-------------------------------------------------------------------------------------------------
void TCP_Socket::Close(void)
{
    // Cannot close if already closed
    if(m_State == TCP_STATE_CLOSED)
    {
        return;
    }

    // Cannot close if a retransmission is pending
    if(m_RetransmitPending)
    {
        return;
    }

    uint8_t Flags = TCP_FLAG_FIN | TCP_FLAG_ACK;

    //---------------------------------------------------------------------------------------------
    // Active close: ESTABLISHED → FIN_WAIT_1
    //---------------------------------------------------------------------------------------------
    if(m_State == TCP_STATE_ESTABLISHED)
    {
        if(m_pTCP->SendSegment(this, nullptr, 0, Flags) == false)
        {
            return;
        }

        m_State = TCP_STATE_FIN_WAIT_1;
    }

    //---------------------------------------------------------------------------------------------
    // Passive close: CLOSE_WAIT → LAST_ACK
    //---------------------------------------------------------------------------------------------
    else if(m_State == TCP_STATE_CLOSE_WAIT)
    {
        if(m_pTCP->SendSegment(this, nullptr, 0, Flags) == false)
        {
            return;
        }

        m_State = TCP_STATE_LAST_ACK;
    }

    //---------------------------------------------------------------------------------------------
    // Set retransmission tracking
    //---------------------------------------------------------------------------------------------
    m_LastFlags         = Flags;
    m_LastPayloadLength = 0;
    m_RetransmitStart   = GetTick();
    m_RetransmitPending = true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Connect
//
//  Parameter(s):   const IP_Address_t& ServerIP
//                      Destination IPv4 address of the remote server.
//
//                  uint16_t Port
//                      Destination TCP port on the remote server.
//
//  Return:         TCP_Socket*
//                      Pointer to a newly allocated TCP socket on success.
//                      nullptr if allocation, binding, or SYN transmission fails.
//
//  Description:    Initiates an outbound TCP connection to a remote server. This function:
//
//                      1. Allocates a TCP socket through the SocketManager
//                      2. Binds it to an ephemeral local port
//                      3. Initializes remote endpoint information
//                      4. Initializes TCP sequence numbers and timers
//                      5. Sends the initial SYN segment
//                      6. Transitions the socket to SYN-SENT state
//
//                  The caller receives ownership of the returned TCP_Socket pointer.
//                  The TCP_Manager tracks the active client socket internally.
//-------------------------------------------------------------------------------------------------
bool TCP_Manager::Initialize(NetworkContext& Context)
{
    m_pSocketManager = &Context.GetSocketManager();                 // Retrieve socket manager
    m_pClientSocket  = nullptr;                                     // Reset internal state (client or server will set these later)
    m_pContext       = &Context;

  #if (IP_USE_TCP_SERVER == DEF_ENABLED)
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)                      // Clear server listen sockets
    {
        m_pServerSockets[i] = nullptr;
    }
  #endif

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           Connect
//
//  Parameter(s):   const IP_Address_t& ServerIP
//                      Destination IPv4 address of the remote server.
//
//                  uint16_t Port
//                      Destination TCP port on the remote server.
//
//  Return:         TCP_Socket*
//                      Pointer to the TCP protocol data associated with the newly allocated
//                      Socket object. Returns nullptr if allocation, binding, or SYN
//                      transmission fails.
//
//  Description:    Initiates an outbound TCP connection to a remote server. This function:
//
//                      1. Allocates a base Socket of type TCP
//                      2. Retrieves the TCP_Socket protocol structure (m_Protocol.pTCP)
//                      3. Binds the socket to an ephemeral local port
//                      4. Initializes remote endpoint information
//                      5. Initializes TCP sequence numbers and timers
//                      6. Sends the initial SYN segment
//                      7. Transitions the socket to SYN-SENT state
//
//                  The caller receives a pointer to the TCP_Socket protocol data.
//                  The underlying Socket object is owned and freed by SocketManager.
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP_CLIENT == DEF_ENABLED)
TCP_Socket* TCP_Manager::Connect(const IP_Address_t& ServerIP, uint16_t Port)
{
    // Allocate a base socket of type STREAM (TCP)
    Socket* pSocket = m_pSocketManager->AllocSocket(SOCKET_TYPE_STREAM);

    if(pSocket == nullptr)
    {
        return nullptr;
    }

    // Retrieve the TCP protocol data
    TCP_Socket* pTCP = pSocket->GetTCP();

    if(pTCP == nullptr)
    {
        m_pSocketManager->FreeSocket(&pSocket);
        return nullptr;
    }

    // Bind to ephemeral port (0 = auto-assign)
    if(pSocket->Bind(0) != SYS_READY)
    {
        m_pSocketManager->FreeSocket(&pSocket);
        return nullptr;
    }

    // Set remote endpoint using your new setter
    SocketInfo_t Remote;
    Remote.Address = ServerIP;
    Remote.Port    = Port;
    pSocket->SetRemoteInfo(Remote);

    // Initialize TCP state
    pTCP->m_SeqNumber     = (uint32_t)GetTick();   // Simple ISN
    pTCP->m_LastSeqNumber = pTCP->m_SeqNumber;
    pTCP->m_AckNumber     = 0;

    pTCP->m_RemoteWindow = 0;
    pTCP->m_LocalWindow  = TCP_DEFAULT_WINDOW_SIZE;

    TickCount_t Now = GetTick();
    pTCP->m_LastSendTick     = Now;
    pTCP->m_LastReceivedTick = Now;
    pTCP->m_RetransmitStart  = 0;

    pTCP->m_RetransmitPending = false;
    pTCP->m_LastFlags         = 0;
    pTCP->m_LastPayloadLength = 0;


    // Send SYN
    if(!SendSegment(pTCP, nullptr, 0, TCP_FLAG_SYN))
    {
        m_pSocketManager->FreeSocket(&pSocket);
        return nullptr;
    }

    // SYN successfully sent
    pTCP->m_State             = TCP_STATE_SYN_SENT;
    pTCP->m_RetransmitStart   = Now;
    pTCP->m_RetransmitPending = true;

    // Store as active client socket
    m_pClientSocket = pTCP;

    return pTCP;
}
#endif

//-------------------------------------------------------------------------------------------------
//  Name:           EnterListen
//
//  Parameter(s):   Socket* pSocket
//                      Pointer to a TCP_Socket instance that will be placed into LISTEN state.
//                      This socket must already be bound to a local port via Bind().
//
//                  uint16_t Backlog
//                      Maximum number of simultaneous pending connections allowed for this
//                      listening socket. The backlog value is stored but enforcement is handled
//                      by the TCP_Manager.
//
//  Return:         SystemState_e
//                      SYS_READY       - Socket successfully placed into LISTEN state.
//                      SYS_ERROR       - Invalid socket, socket already in use, or no available
//                                        entry in the server listen table.
//
//  Description:    Registers a TCP_Socket as a listening endpoint for incoming TCP connections.
//                  This function is only available when TCP server mode is enabled.
//
//                  - Validates that the provided socket is a TCP_Socket.
//                  - Ensures the socket is bound to a local port.
//                  - Places the socket into TCP_STATE_LISTEN.
//                  - Stores the socket in the TCP_Manager's server socket table so that
//                    ProcessSegment() can route incoming SYN packets to it.
//                  - The backlog parameter determines how many pending connections may be
//                    queued before new SYN requests are rejected.
//
//                  The accept/handshake process is handled asynchronously by Process() and
//                  ProcessSegment(), which create new TCP_Socket instances for established
//                  connections.
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP_SERVER == DEF_ENABLED)
SystemState_e TCP_Manager::EnterListen(Socket* pSocket, uint16_t Backlog)
{
    if(pSocket == nullptr)
    {
        return SYS_ERROR;
    }

    // Ensure the socket is actually a TCP_Socket
    TCP_Socket* pTCPSocket = dynamic_cast<TCP_Socket*>(pSocket);
    if(pTCPSocket == nullptr)
    {
        return SYS_ERROR;     // Not a TCP socket
    }

    // Ensure the socket is bound to a local port
    if(pTCPSocket->GetLocalPort() == 0)
    {
        return SYS_ERROR;     // Must call Bind() before EnterListen()
    }

    // Ensure the socket is not already in use
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        if(m_pServerSockets[i] == pTCPSocket)
        {
            return SYS_ERROR; // Already registered
        }
    }

    // Find a free slot in the server listen table
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        if(m_pServerSockets[i] == nullptr)
        {
            // Register the socket
            m_pServerSockets[i] = pTCPSocket;

            // Place socket into LISTEN state
            pTCPSocket->m_State = TCP_STATE_LISTEN;

            // Store backlog if you want (optional)
            // pTCPSocket->m_Backlog = Backlog;   // Add this member if needed

            return SYS_READY;
        }
    }

    // No free slot available
    return SYS_ERROR;
}
#endif

//-------------------------------------------------------------------------------------------------
//  Name:           Close
//
//  Parameter(s):   Socket* pSocket
//                      Pointer to the TCP_Socket instance to close. This must be a server-side
//                      socket previously registered in the TCP_Manager's listen/connection table.
//
//  Return:         void
//
//  Description:    Initiates an active close of a server-side TCP connection. This function is
//                  only available when TCP server mode is enabled.
//
//                  - Locates the specified TCP_Socket within the server socket table.
//                  - Invokes the socket's own Close() method to begin the FIN handshake.
//                  - Does NOT free the socket immediately; the remainder of the TCP close
//                    sequence (FIN → ACK → FIN → ACK → TIME_WAIT → CLOSED) is handled
//                    asynchronously by Process() and ProcessSegment().
//
//                  If the socket is not found in the server table, the function silently returns.
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP_SERVER == DEF_ENABLED)
void TCP_Manager::Close(Socket* pSocket)
{
    if(pSocket == nullptr)
    {
        return;
    }

    // Find the matching TCP_Socket in the server socket table
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        TCP_Socket* pTCPSock = m_pServerSockets[i];

        if(pTCPSock == nullptr)
        {
            continue;
        }

        // Match by pointer identity
        if(pTCPSock == (TCP_Socket*)pSocket)
        {
            // Initiate active close on that socket
            pTCPSock->Close();

            // Do NOT free the socket here.
            // The FIN/ACK/TIME_WAIT sequence is handled asynchronously.
            return;
        }
    }

    // If we reach here: pSocket was not found in the server table
}
#endif

//-------------------------------------------------------------------------------------------------
//  Name:           Process
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Periodic TCP processing function. This function must be called regularly from
//                  the main loop. It performs global TCP maintenance tasks, including:
//
//                  - Retransmission handling for client and server sockets.
//                  - TIME_WAIT and other timeout-based state transitions.
//                  - Updating per-socket timers (last send/receive, retransmission start, etc.).
//
//                  This function does NOT process incoming TCP segments; that is handled by
//                  ProcessSegment(). Instead, Process() drives the TCP state machine forward
//                  based on elapsed time and retransmission requirements.
//-------------------------------------------------------------------------------------------------
void TCP_Manager::Process(void)
{
    UpdateTimers();

    //---------------------------------------------------------------------------------------------
    // Process client socket (if TCP client is enabled)
    //---------------------------------------------------------------------------------------------
#if (IP_USE_TCP_CLIENT == DEF_ENABLED)
    if(m_pClientSocket != nullptr)
    {
        TCP_Socket* pTCP = m_pClientSocket->GetTCP();
        if(pTCP != nullptr)
        {
            RetransmitIfNeeded(pTCP);
        }
    }
#endif

    //---------------------------------------------------------------------------------------------
    // Process all server sockets (if TCP server is enabled)
    //---------------------------------------------------------------------------------------------
#if (IP_USE_TCP_SERVER == DEF_ENABLED)
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        Socket* pSock = m_pServerSockets[i];
        if(pSock == nullptr)
        {
            continue;
        }

        TCP_Socket* pTCP = pSock->GetTCP();
        if(pTCP != nullptr)
        {
            RetransmitIfNeeded(pTCP);
        }
    }
#endif
}

//-------------------------------------------------------------------------------------------------
//  Name:           ProcessSegment
//
//  Parameter(s):   IP_EthernetPacket_t* pPacket
//                      Pointer to a received Ethernet/IP/TCP packet. The packet has already been
//                      validated by the IP layer (checksum, length, protocol = TCP).
//
//  Return:         void
//
//  Description:    Handles an incoming TCP segment. This function performs the following steps:
//
//                  1. Parses the TCP header and extracts:
//                         - Source port
//                         - Destination port
//                         - Sequence number
//                         - Acknowledgment number
//                         - Flags (SYN, ACK, FIN, RST, PSH)
//                         - Payload length
//
//                  2. Identifies the corresponding TCP_Socket:
//                         - Client socket (if TCP client mode enabled)
//                         - Server listening socket (if TCP server mode enabled)
//                         - Server established connection sockets
//
//                  3. Dispatches the segment to the appropriate socket and invokes the TCP
//                     state machine logic via ProcessIncomingFlags().
//
//                  4. Handles connection establishment (SYN/SYN+ACK/ACK), data reception,
//                     connection teardown (FIN/ACK), and error/reset conditions.
//
//                  This function does NOT perform retransmission or timeout handling; those are
//                  handled by Process().
//-------------------------------------------------------------------------------------------------
void TCP_Manager::ProcessSegment(IP_EthernetPacket_t* pPacket)
{
    if(pPacket == nullptr)
    {
        return;
    }

    TCP_Socket* pSocket = nullptr;

    uint8_t  Flags      = 0;
    uint32_t Seq        = 0;
    uint32_t Ack        = 0;
    size_t   PayloadLen = 0;

    //---------------------------------------------------------------------------------------------
    // Parse TCP header and identify the socket
    //---------------------------------------------------------------------------------------------
    if(ParseTCP_Header(pPacket, pSocket) == false)
    {
        return;     // Invalid header or no matching socket
    }

    //---------------------------------------------------------------------------------------------
    // Extract TCP header fields
    //---------------------------------------------------------------------------------------------
    const TCP_Header_t& hdr = pPacket->TCP_Frame.Header;
    const IP_Header_t&  ip  = pPacket->TCP_Frame.IP_Header;

    Flags = hdr.Flags;
    Seq   = hdr.SequenceNumber;
    Ack   = hdr.AcknowledgeNumber;

    //---------------------------------------------------------------------------------------------
    // Compute payload length
    //---------------------------------------------------------------------------------------------
    uint8_t ipHeaderLen  = (ip.VersionIHL & 0x0F) * 4;       // IHL in bytes
    uint8_t tcpHeaderLen = (hdr.Offset >> 4) * 4;            // Data offset in bytes

    PayloadLen = ip.Length - ipHeaderLen - tcpHeaderLen;

    //---------------------------------------------------------------------------------------------
    // Dispatch to the TCP state machine
    //---------------------------------------------------------------------------------------------
    ProcessIncomingFlags(pSocket, pPacket, Flags, Seq, Ack, PayloadLen);
}

//-------------------------------------------------------------------------------------------------
//  Name:           SendSegment
//
//  Parameter(s):   TCP_Socket* pSocket
//                      Pointer to the TCP protocol data associated with the base Socket.
//
//                  const uint8_t* pPayload
//                      Optional payload buffer to include in the TCP segment.
//
//                  size_t Length
//                      Number of payload bytes to send.
//
//                  uint8_t Flags
//                      TCP control flags (SYN, ACK, FIN, etc.)
//
//  Return:         bool
//                      true  = Segment successfully built and passed to IP layer
//                      false = Allocation failure or IP-layer transmission failure
//
//  Description:    Builds and transmits a TCP segment using the IP layer. This function:
//
//                      - Allocates an outgoing packet from the IP layer
//                      - Fills the TCP header fields
//                      - Copies payload (if any)
//                      - Computes the TCP checksum
//                      - Sends the packet through the IP layer
//                      - Updates sequence numbers and retransmission timers
//
//                  The caller is responsible for managing retransmission state.
//-------------------------------------------------------------------------------------------------
bool TCP_Manager::SendSegment(TCP_Socket* pSocket, const uint8_t* pPayload, size_t Length, uint8_t Flags, bool Retransmit)
{
    if(pSocket == nullptr)
    {
        return false;
    }

    IP_Manager* pIP_Manager = m_pContext->GetIP_Manager();
    if(pIP_Manager == nullptr)
    {
        return false;
    }

    // Retrieve local/remote info
    SocketInfo_t localInfo;
    SocketInfo_t remoteInfo;

    pSocket->GetLocalInfo(&localInfo);
    pSocket->GetRemoteInfo(&remoteInfo);

    IP_PacketMsg_t* pMsg = nullptr;

    SystemState_e state = IP_Manager::AllocPacket(&pMsg, sizeof(IP_EthernetPacket_t), MEM_DBG_TCP, MEM_DBG_TCPDT);

    if(state != SYS_READY || pMsg == nullptr)
    {
        return false;
    }

    IP_EthernetPacket_t* pPacket = pMsg->pPacket;

    // Reference the TCP header inside the frame
    TCP_Header_t& hdr = pPacket->TCP_Frame.Header;

    // Build TCP header
    hdr.SrcPort           = localInfo.Port;
    hdr.DstPort           = remoteInfo.Port;

    hdr.SequenceNumber    = pSocket->m_SeqNumber;
    hdr.AcknowledgeNumber = pSocket->m_AckNumber;

    hdr.Flags             = Flags;
    hdr.Window            = pSocket->m_LocalWindow;
    hdr.UrgentPointer     = 0;

    // Compute TCP header length (no options)
    hdr.Offset = (sizeof(TCP_Header_t) / 4) << 4;

    // Copy payload
    if(pPayload && Length > 0)
    {
        uint8_t* pTCP_Payload = ((uint8_t*)&pPacket->TCP_Frame.Header) + sizeof(TCP_Header_t);
        memcpy(pTCP_Payload, pPayload, Length);
    }

    // Compute checksum
    IP_Header_t&  ip  = pPacket->TCP_Frame.IP_Header;
    TCP_Header_t& tcp = pPacket->TCP_Frame.Header;
    uint16_t tcpLen = sizeof(TCP_Header_t) + Length;
    tcp.Checksum = IP_Manager::TCP_CalculateChecksum(&ip, &tcp, tcpLen);

    // Prepare wrapper fields
    pMsg->Payload      = nullptr;                 // TCP payload is inside the frame
    pMsg->PayloadSize  = 0;
    pMsg->PacketSize   = sizeof(IP_EthernetPacket_t);

    // Send through IP layer
    SystemState_e state2 = pIP_Manager->SendPacket(pMsg);

    if(state2 != SYS_READY && state2 != SYS_ARP_RESOLVE_PENDING)
    {
        IP_Manager::FreeMessage(pMsg);
        return false;
    }

    if(Retransmit == false)
    {
        pSocket->m_LastSeqNumber = pSocket->m_SeqNumber;

        if((Flags & TCP_FLAG_SYN) || (Flags & TCP_FLAG_FIN))
        {
            pSocket->m_SeqNumber++;
        }

        pSocket->m_SeqNumber += Length;
    }

    // Update sequence number
    if(Flags & TCP_FLAG_SYN) pSocket->m_SeqNumber++;
    if(Flags & TCP_FLAG_FIN) pSocket->m_SeqNumber++;

    pSocket->m_SeqNumber += Length;

    // Update timestamp
    pSocket->m_LastSendTick = GetTick();

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Name:           ParseTCP_Header
//
//  Parameter(s):   IP_EthernetPacket_t* pPacket
//                      Pointer to a received Ethernet/IP/TCP packet. The packet has already been
//                      validated by the IP layer (checksum, length, protocol = TCP).
//
//                  TCP_Socket*& pSockOut
//                      Output pointer. On success, this is set to the TCP_Socket instance that
//                      matches the packet's destination port. On failure, it is set to nullptr.
//
//  Return:         bool
//                      true  - TCP header parsed successfully and a matching socket was found.
//                      false - Invalid header, unsupported port, or no matching socket.
//
//  Description:    Parses the TCP header of an incoming packet and identifies the corresponding
//                  TCP_Socket. This function extracts:
//
//                      - Source port
//                      - Destination port
//                      - Sequence number
//                      - Acknowledgment number
//                      - Flags
//                      - Payload length
//
//                  Socket matching rules:
//                      - If TCP client mode is enabled, match against the client socket.
//                      - If TCP server mode is enabled, match against:
//                            * Listening sockets (LISTEN state)
//                            * Established server-side sockets
//
//                  This function does NOT perform any state transitions or ACK logic; that is
//                  handled by ProcessIncomingFlags().
//-------------------------------------------------------------------------------------------------
bool TCP_Manager::ParseTCP_Header(IP_EthernetPacket_t* pPacket, TCP_Socket*& pSockOut)
{
    pSockOut = nullptr;

    if(pPacket == nullptr)
    {
        return false;
    }

    // Extract TCP header reference
    TCP_Header_t& tcp = pPacket->TCP_Frame.Header;

    uint16_t DstPort  = tcp.DstPort;

    //uint32_t Seq      = tcp.SequenceNumber;
    //uint32_t Ack      = tcp.AcknowledgeNumber;
    //uint8_t  Flags    = tcp.Flags;

    // Compute payload length from IP header
    //IP_Header_t& ip = pPacket->TCP_Frame.IP_Header;

    //uint8_t ipHeaderLen  = (ip.VersionIHL & 0x0F) * 4;
    //uint8_t tcpHeaderLen = (tcp.Offset >> 4) * 4;

//    size_t Payload = ip.Length - ipHeaderLen - tcpHeaderLen;

    //---------------------------------------------------------------------------------------------
    // 1. Try to match client socket
    //---------------------------------------------------------------------------------------------
#if (IP_USE_TCP_CLIENT == DEF_ENABLED)
    if(m_pClientSocket != nullptr)
    {
        if(m_pClientSocket->GetLocalPort() == DstPort)
        {
            pSockOut = static_cast<TCP_Socket*>(m_pClientSocket);
            return true;
        }
    }
#endif

    //---------------------------------------------------------------------------------------------
    // 2. Try to match server sockets
    //---------------------------------------------------------------------------------------------
#if (IP_USE_TCP_SERVER == DEF_ENABLED)
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        TCP_Socket* pSocket = m_pServerSockets[i];

        if(pSocket == nullptr)
        {
            continue;
        }

        if(pSocket->GetLocalPort() == DstPort)
        {
            pSockOut = pSocket;
            return true;
        }
    }
#endif

    return false;
}

//-------------------------------------------------------------------------------------------------
//  Name:           ProcessIncomingFlags
//
//  Parameter(s):   TCP_Socket* pSocket
//                      The socket associated with the incoming TCP segment.
//
//                  uint8_t Flags
//                      TCP control flags (SYN, ACK, FIN, PSH, RST).
//
//                  uint32_t Seq
//                      Sequence number of the incoming segment.
//
//                  uint32_t Ack
//                      Acknowledgment number of the incoming segment.
//
//                  size_t PayloadLen
//                      Number of payload bytes in the segment.
//
//  Return:         void
//
//  Description:    Implements the TCP state machine for an incoming segment. This function handles
//                  all TCP control flows, including:
//
//                      - Connection establishment (SYN → SYN_RECEIVED → ESTABLISHED)
//                      - Client-side handshake completion (SYN_SENT → ESTABLISHED)
//                      - Data reception and ACK processing
//                      - Active close (FIN_WAIT_1 → FIN_WAIT_2 → TIME_WAIT → CLOSED)
//                      - Passive close (CLOSE_WAIT → LAST_ACK → CLOSED)
//
//                  This function does NOT perform retransmission or timeout handling; those are
//                  handled by Process(). It also does not send segments directly; instead it calls
//                  SendSegment() as needed.
//-------------------------------------------------------------------------------------------------
void TCP_Manager::ProcessIncomingFlags(TCP_Socket* pSocket,
                                       IP_EthernetPacket_t* pPacket,
                                       uint8_t Flags,
                                       uint32_t Seq,
                                       uint32_t Ack,
                                       size_t PayloadLen)
{
    if(pSocket == nullptr)
    {
        return;
    }

    // Update last receive timestamp
    pSocket->m_LastReceivedTick = GetTick();

    //---------------------------------------------------------------------------------------------
    // Basic ACK processing (applies in most states)
    //---------------------------------------------------------------------------------------------
    if(Flags & TCP_FLAG_ACK)
    {
        // If ACK acknowledges our last transmission, clear retransmission pending
        if(pSocket->m_RetransmitPending)
        {
            uint32_t ExpectedAck = pSocket->m_SeqNumber;
            if(Ack == ExpectedAck)
            {
                pSocket->m_RetransmitPending = false;
            }
        }

        // Update acknowledgment number
        pSocket->m_AckNumber = Ack;
    }

    //---------------------------------------------------------------------------------------------
    // TCP STATE MACHINE
    //---------------------------------------------------------------------------------------------
    switch(pSocket->m_State)
    {
        //-----------------------------------------------------------------------------------------
        // LISTEN: Incoming SYN from a client
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_LISTEN:
        {
            if(Flags & TCP_FLAG_SYN)
            {
                // Initialize sequence/ack numbers
                pSocket->m_AckNumber = Seq + 1;
                pSocket->m_SeqNumber = (uint32_t)GetTick();

                // Send SYN+ACK
                SendSegment(pSocket, nullptr, 0, TCP_FLAG_SYN | TCP_FLAG_ACK);

                pSocket->m_State = TCP_STATE_SYN_RECEIVED;
                pSocket->m_RetransmitPending = true;
                pSocket->m_RetransmitStart = GetTick();
            }
            break;
        }

        //-----------------------------------------------------------------------------------------
        // SYN_SENT: Client waiting for SYN+ACK
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_SYN_SENT:
        {
            if((Flags & TCP_FLAG_SYN) && (Flags & TCP_FLAG_ACK))
            {
                // Complete handshake
                pSocket->m_AckNumber = Seq + 1;

                // Send final ACK
                SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK);

                pSocket->m_State = TCP_STATE_ESTABLISHED;
                pSocket->m_RetransmitPending = false;
            }
            break;
        }

        //-----------------------------------------------------------------------------------------
        // SYN_RECEIVED: Server waiting for final ACK
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_SYN_RECEIVED:
        {
            if(Flags & TCP_FLAG_ACK)
            {
                pSocket->m_State = TCP_STATE_ESTABLISHED;
                pSocket->m_RetransmitPending = false;
            }
            break;
        }

        //-----------------------------------------------------------------------------------------
        // ESTABLISHED: Normal data transfer
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_ESTABLISHED:
        {
            // Handle payload
            if(PayloadLen > 0)
            {
                TCP_Header_t& tcp = pPacket->TCP_Frame.Header;

                uint8_t tcpHeaderLen = (tcp.Offset >> 4) * 4;
                uint8_t* pPayloadStart =
                    ((uint8_t*)&pPacket->TCP_Frame.Header) + tcpHeaderLen;

                if(PayloadLen <= sizeof(pSocket->m_RX_Buffer))
                {
                    memcpy(pSocket->m_RX_Buffer, pPayloadStart, PayloadLen);
                    pSocket->m_RX_Length = PayloadLen;

                    pSocket->m_AckNumber = Seq + PayloadLen;

                    SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK);
                }
            }

            // FIN received → passive close
            if(Flags & TCP_FLAG_FIN)
            {
                pSocket->m_AckNumber = Seq + 1;

                // ACK the FIN
                SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK);

                pSocket->m_State = TCP_STATE_CLOSE_WAIT;
            }
            break;
        }

        //-----------------------------------------------------------------------------------------
        // FIN_WAIT_1: We sent FIN, waiting for ACK
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_FIN_WAIT_1:
        {
            if(Flags & TCP_FLAG_ACK)
            {
                pSocket->m_State = TCP_STATE_FIN_WAIT_2;
            }

            if(Flags & TCP_FLAG_FIN)
            {
                pSocket->m_AckNumber = Seq + 1;
                SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK);
                pSocket->m_State = TCP_STATE_TIME_WAIT;
            }
            break;
        }

        //-----------------------------------------------------------------------------------------
        // FIN_WAIT_2: Waiting for FIN from peer
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_FIN_WAIT_2:
        {
            if(Flags & TCP_FLAG_FIN)
            {
                pSocket->m_AckNumber = Seq + 1;
                SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK);
                pSocket->m_State = TCP_STATE_TIME_WAIT;
            }
            break;
        }

        //-----------------------------------------------------------------------------------------
        // CLOSE_WAIT: We received FIN, waiting for application to call Close()
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_CLOSE_WAIT:
        {
            // Nothing to do here; Close() will send FIN
            break;
        }

        //-----------------------------------------------------------------------------------------
        // LAST_ACK: We sent FIN after receiving FIN
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_LAST_ACK:
        {
            if(Flags & TCP_FLAG_ACK)
            {
                pSocket->m_State = TCP_STATE_CLOSED;
            }
            break;
        }

        //-----------------------------------------------------------------------------------------
        // TIME_WAIT: Wait for timeout before closing
        //-----------------------------------------------------------------------------------------
        case TCP_STATE_TIME_WAIT:
        {
            // Timer handled in UpdateTimers()
            break;
        }

        default:
            break;
    }
}

//-------------------------------------------------------------------------------------------------
//  Name:           RetransmitIfNeeded
//
//  Parameter(s):   TCP_Socket* pSocket
//                      Pointer to the TCP socket whose retransmission timer is being evaluated.
//
//  Return:         void
//
//  Description:    Handles TCP retransmission for a single socket. If a segment previously sent
//                  by this socket (SYN, data, FIN, etc.) has not been acknowledged within the
//                  retransmission timeout interval, this function retransmits the segment.
//
//                  Retransmission rules:
//                      - Only segments marked as "pending" are retransmitted.
//                      - Sequence numbers are NOT advanced during retransmission.
//                      - The retransmission timer is restarted after each retransmission.
//                      - The caller (Connect(), Send(), Close()) is responsible for recording
//                        the last transmitted flags and payload length.
//
//                  This function does NOT modify the TCP state machine directly; state transitions
//                  occur in ProcessIncomingFlags() when ACKs are received.
//-------------------------------------------------------------------------------------------------
void TCP_Manager::RetransmitIfNeeded(TCP_Socket* pSocket)
{
    if(pSocket == nullptr)
    {
        return;
    }

    // Nothing to do if no retransmission is pending
    if(pSocket->m_RetransmitPending == false)
    {
        return;
    }

    TickCount_t Now = GetTick();

    // Check if retransmission timeout has expired
    if((Now - pSocket->m_RetransmitStart) < TCP_RETRANSMIT_TIMEOUT)
    {
        return;     // Not yet time to retransmit
    }

    //---------------------------------------------------------------------------------------------
    // Retransmit the last segment
    //---------------------------------------------------------------------------------------------
    const uint8_t Flags = pSocket->m_LastFlags;
    const size_t  Len   = pSocket->m_LastPayloadLength;

    const uint8_t* pPayload = nullptr;

    if(Len > 0)
    {
        // Payload stored in socket's TX buffer
        pPayload = pSocket->m_TX_Buffer;
    }

    pSocket->m_SeqNumber = pSocket->m_LastSeqNumber;

    // Re-send the segment with the SAME sequence number
    // (SendSegment() will NOT advance sequence numbers for retransmissions)
    if(SendSegment(pSocket, pPayload, Len, Flags, true) == false)
    {
        // Transmission failure — keep retransmission pending
        // Caller may decide to close the socket after repeated failures
        return;
    }

    // Restart retransmission timer
    pSocket->m_RetransmitStart = Now;

    // Keep m_RetransmitPending = true until ACK arrives
}

//-------------------------------------------------------------------------------------------------
//  Name:           UpdateTimers
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Updates TCP time-based state transitions for all active sockets. This function
//                  is called periodically from Process() and handles:
//
//                      - TIME_WAIT expiration
//                      - Optional future timers (keepalive, delayed ACK, linger)
//
//                  TIME_WAIT handling:
//                      When a socket enters TIME_WAIT, it must remain in that state for the
//                      required timeout interval to ensure that late or duplicate segments do not
//                      interfere with a future connection. Once the timeout expires, the socket is
//                      transitioned to CLOSED.
//
//                  This function does NOT perform retransmission handling; that is handled by
//                  RetransmitIfNeeded(). It also does not free sockets; socket lifecycle
//                  management is handled elsewhere.
//-------------------------------------------------------------------------------------------------
void TCP_Manager::UpdateTimers(void)
{
    TickCount_t Now = GetTick();

    //---------------------------------------------------------------------------------------------
    // Process client socket (if enabled)
    //---------------------------------------------------------------------------------------------
#if (IP_USE_TCP_CLIENT == DEF_ENABLED)
    if(m_pClientSocket != nullptr)
    {
        TCP_Socket* pSocket = static_cast<TCP_Socket*>(m_pClientSocket);

        // TIME_WAIT expiration
        if(pSocket->m_State == TCP_STATE_TIME_WAIT)
        {
            if((Now - pSocket->m_LastReceivedTick) >= TCP_TIME_WAIT_TIMEOUT)
            {
                pSocket->m_State = TCP_STATE_CLOSED;
                pSocket->m_RX_Length = 0;
                pSocket->m_TX_Length = 0;
            }
        }
    }
#endif

    //---------------------------------------------------------------------------------------------
    // Process server sockets (if enabled)
    //---------------------------------------------------------------------------------------------
#if (IP_USE_TCP_SERVER == DEF_ENABLED)
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        TCP_Socket* pSocket = m_pServerSockets[i];

        if(pSocket == nullptr)
        {
            continue;
        }

        // TIME_WAIT expiration
        if(pSocket->m_State == TCP_STATE_TIME_WAIT)
        {
            if((Now - pSocket->m_LastReceivedTick) >= TCP_TIME_WAIT_TIMEOUT)
            {
                pSocket->m_State = TCP_STATE_CLOSED;
                pSocket->m_RX_Length = 0;
                pSocket->m_TX_Length = 0;

                // NOTE:
                // We do NOT free the socket here.
                // The application or a future socket manager will handle cleanup.
            }
        }
    }
#endif
}

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_TCP_CLIENT == DEF_ENABLED)
