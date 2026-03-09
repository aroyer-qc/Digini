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
#define TCP_FLAG_FIN                0x01    // Finish
#define TCP_FLAG_SYN                0x02    // Synchronize sequence numbers
#define TCP_FLAG_RST                0x04    // Reset connection
#define TCP_FLAG_PSH                0x08    // Push function
#define TCP_FLAG_ACK                0x10    // Acknowledgment field significant
#define TCP_FLAG_URG                0x20    // Urgent pointer field significant
#define TCP_FLAG_ECE                0x40    // ECN-Echo (RFC 3168)
#define TCP_FLAG_CWR                0x80    // Congestion Window Reduced (RFC 3168)

#define TCP_RETRANSMIT_TIMEOUT      1000
#define TCP_TIME_WAIT_TIMEOUT       30000
#define TCP_GENERIC_CLOSE_TIMEOUT   2000
#define TCP_CONNECT_TIMEOUT         3000

//-------------------------------------------------------------------------------------------------
//  Name:           TCP_SocketSystem (constructor)
//
//  Parameter(s):   NetworkContext* pContextt
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
TCP_SocketSystem::TCP_SocketSystem(NetworkContext* pContext, TCP_Manager& TCP) : Socket(pContext)
{
    m_pTCP              = &TCP;                     // Store back-pointer to TCP manager
    m_State             = TCP_STATE_CLOSED;

    m_SeqNumber         = 0;
    m_AckNumber         = 0;

    m_RemoteWindow      = 0;
    m_LocalWindow       = TCP_DEFAULT_WINDOW_SIZE;

    TickCount_t Now     = GetTick();
    m_LastReceivedTick  = Now;
    m_LastSendTick      = Now;

    m_RetransmitTimeOut = TCP_RETRANSMIT_TIMEOUT;

    for(size_t i = 0; i < TCP_MAX_TX_SEGMENTS; i++)
    {
        m_TX_Window[i].pPayload  = nullptr;
        m_TX_Window[i].SeqStart  = 0;
        m_TX_Window[i].SeqEnd    = 0;
        m_TX_Window[i].Flags     = 0;
        m_TX_Window[i].Window    = 0;
        m_TX_Window[i].Length    = 0;
        m_TX_Window[i].TimeStamp = 0;
        m_TX_Window[i].InUse     = false;
    }
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
size_t TCP_SocketSystem::Send(const uint8_t* pData, size_t Length)
{
    // Must be connected
    if(m_State != TCP_STATE_ESTABLISHED)
    {
        return 0;
    }

    // Validate input
    if((pData == nullptr) || (Length == 0))
    {
        return 0;
    }

    size_t TotalSent = 0;

    while(TotalSent < Length)
    {
        size_t Remaining = Length - TotalSent;
        size_t ChunkSize = (Remaining > TCP_MSS) ? TCP_MSS : Remaining;

        IP_PacketMsg_t* pMsg = m_pTCP->SendSegment(this, pData + TotalSent, ChunkSize, TCP_FLAG_ACK | TCP_FLAG_PSH, false);

        if(pMsg == nullptr)
        {
            break;
        }

        TotalSent += ChunkSize;
    }

    return TotalSent;
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
size_t TCP_SocketSystem::Receive(uint8_t* pBuffer, size_t MaxLength)
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

    IP_PacketMsg_t* pMsg = nullptr;

    // Pop next received segment
    if(DequeueMessage(pMsg) != true)
    {
        return 0;   // No data available
    }

    if((pMsg == nullptr) || (pMsg->pPacket == nullptr))
    {
        return 0;
    }

    IP_EthernetPacket_t* pPacket = pMsg->pPacket;
    TCP_Header_t& tcp = pPacket->TCP_Frame.Header;

    uint8_t tcpHeaderLen = (tcp.Offset >> 4) * 4;
    uint8_t* pPayloadStart = ((uint8_t*)&pPacket->TCP_Frame.Header) + tcpHeaderLen;

    // Compute payload length
    size_t PayloadLen = pMsg->PacketSize - sizeof(IP_EthernetHeader_t) - sizeof(IP_Header_t) - tcpHeaderLen;

    if(PayloadLen > MaxLength)
    {
        PayloadLen = MaxLength;
    }

    // Copy payload to application buffer
    memcpy(pBuffer, pPayloadStart, PayloadLen);

    // Free the message buffer
    IP_Manager::FreeMessage(pMsg);

    return PayloadLen;
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
void TCP_SocketSystem::Close(void)
{
    if(m_State == TCP_STATE_CLOSED)
    {
        return;
    }

    for(size_t i = 0; i < TCP_MAX_TX_SEGMENTS; i++)
    {
        FlushTX_Slot(&m_TX_Window[i]);
    }

    uint8_t Flags = TCP_FLAG_FIN | TCP_FLAG_ACK;

    IP_PacketMsg_t* pMsg = m_pTCP->SendSegment(this, nullptr, 0, Flags, false);

    if(pMsg == nullptr)
    {
        return;
    }

    if(m_State == TCP_STATE_ESTABLISHED)
    {
        m_State = TCP_STATE_FIN_WAIT_1;
    }
    else if(m_State == TCP_STATE_CLOSE_WAIT)
    {
        m_State = TCP_STATE_LAST_ACK;
    }
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
void TCP_SocketSystem::ProcessIncomingFlags(TCP_Socket*     pSocket,
                                            IP_PacketMsg_t* pMsg,
                                            uint8_t         Flags,
                                            uint32_t        Seq,
                                            uint32_t        Ack)
{
    if((pSocket == nullptr) || (pMsg == nullptr) || (pMsg->pPacket == nullptr))
    {
        if(pMsg != nullptr)
        {
            IP_Manager::FreeMessage(pMsg);
        }
        return;
    }

    TCP_SocketSystem* pSystem = static_cast<TCP_SocketSystem*>(pSocket);

    pSystem->m_LastReceivedTick = GetTick();

    if((Flags & TCP_FLAG_ACK) != 0)
    {
        pSystem->m_AckNumber = Ack;

        for(size_t i = 0; i < TCP_MAX_TX_SEGMENTS; i++)
        {
            TCP_TX_Segment_t* pSlot = &pSystem->m_TX_Window[i];

            if(pSlot->InUse == true)
            {
                if(pSlot->SeqEnd <= Ack)
                {
                    FlushTX_Slot(pSlot);
                }
            }
        }
    }

    switch(pSystem->m_State)
    {
        case TCP_STATE_LISTEN:
        {
            if((Flags & TCP_FLAG_SYN) != 0)
            {
                pSystem->m_AckNumber = Seq + 1;
                pSystem->m_SeqNumber = (uint32_t)GetTick();

                pSystem->m_pTCP->SendSegment(pSocket, nullptr, 0, TCP_FLAG_SYN | TCP_FLAG_ACK, false);

                pSystem->m_State = TCP_STATE_SYN_RECEIVED;
            }
            break;
        }

        case TCP_STATE_SYN_SENT:
        {
            if(((Flags & TCP_FLAG_SYN) != 0) && ((Flags & TCP_FLAG_ACK) != 0))
            {
                pSystem->m_AckNumber = Seq + 1;

                pSystem->m_pTCP->SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK, false);

                pSystem->m_State = TCP_STATE_ESTABLISHED;
            }
            break;
        }

        case TCP_STATE_SYN_RECEIVED:
        {
            if((Flags & TCP_FLAG_ACK) != 0)
            {
                pSystem->m_State = TCP_STATE_ESTABLISHED;
            }
            break;
        }

        case TCP_STATE_ESTABLISHED:
        {
            if(pMsg->PayloadSize > 0)
            {
                pSystem->EnqueueMessage(pMsg);

                pSystem->m_AckNumber = Seq + pMsg->PayloadSize;

                pSystem->m_pTCP->SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK, false);

                if(pSystem->m_pEventHandler != nullptr)
                {
                    pSystem->m_pEventHandler->OnSocketEvent(pSocket, SOCKET_EVENT_RX_READY);
                }

                return;
            }

            if((Flags & TCP_FLAG_FIN) != 0)
            {
                pSystem->m_AckNumber = Seq + 1;

                pSystem->m_pTCP->SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK, false);

                pSystem->m_State = TCP_STATE_CLOSE_WAIT;
            }
            break;
        }

        case TCP_STATE_FIN_WAIT_1:
        {
            if((Flags & TCP_FLAG_ACK) != 0)
            {
                pSystem->m_State = TCP_STATE_FIN_WAIT_2;
            }

            if((Flags & TCP_FLAG_FIN) != 0)
            {
                pSystem->m_AckNumber = Seq + 1;

                pSystem->m_pTCP->SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK, false);

                pSystem->m_State = TCP_STATE_TIME_WAIT;
            }
            break;
        }

        case TCP_STATE_FIN_WAIT_2:
        {
            if((Flags & TCP_FLAG_FIN) != 0)
            {
                pSystem->m_AckNumber = Seq + 1;

                pSystem->m_pTCP->SendSegment(pSocket, nullptr, 0, TCP_FLAG_ACK, false);

                pSystem->m_State = TCP_STATE_TIME_WAIT;
            }
            break;
        }

        case TCP_STATE_CLOSE_WAIT:
        {
            break;
        }

        case TCP_STATE_LAST_ACK:
        {
            if((Flags & TCP_FLAG_ACK) != 0)
            {
                pSystem->m_State = TCP_STATE_CLOSED;
            }
            break;
        }

        case TCP_STATE_TIME_WAIT:
        {
            break;
        }

        default:
            break;
    }

    IP_Manager::FreeMessage(pMsg);
}

//-------------------------------------------------------------------------------------------------
//  Name:
//
//  Parameter(s):
//
//  Return:
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
bool TCP_ManagerSystem::Initialize(NetworkContext* pContext)
{
    m_pSocketManager = &pContext->GetSocketManager();               // Retrieve socket manager
    m_pClientSocket  = nullptr;                                     // Reset internal state (client or server will set these later)
    m_pContext       = pContext;

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
TCP_Socket* TCP_ManagerSystem::Connect(const IP_Address_t& ServerIP, uint16_t Port)
{
    if(m_pContext == nullptr)
    {
        return nullptr;
    }

    SocketManager& socketMgr = m_pContext->GetSocketManager();

    Socket* pSocketBase = socketMgr.AllocSocket(SOCKET_TYPE_STREAM);
    if(pSocketBase == nullptr)
    {
        return nullptr;
    }

    TCP_SocketSystem* pSystem = static_cast<TCP_SocketSystem*>(pSocketBase);
    pSystem->m_pTCP = this;

    TCP_Socket* pTCP = pSystem;

    pSystem->m_State = TCP_STATE_CLOSED;

    SocketInfo_t remote;
    remote.Address = ServerIP;
    remote.Port    = Port;
    pSocketBase->SetRemoteInfo(remote);

    SocketInfo_t local;
    local.Address = m_pContext->GetActiveIP();
    local.Port    = htons(m_pContext->GetIP_Manager()->AllocateEphemeralPort());
    pSocketBase->SetLocalInfo(local);

    uint32_t isn = (uint32_t)GetTick();
    pSystem->m_SeqNumber = isn;
    pSystem->m_AckNumber = 0;

    pSystem->m_RemoteWindow = 0;
    pSystem->m_LocalWindow  = TCP_DEFAULT_WINDOW_SIZE;

    TickCount_t now = GetTick();
    pSystem->m_LastSendTick     = now;
    pSystem->m_LastReceivedTick = now;

    for(size_t i = 0; i < TCP_MAX_TX_SEGMENTS; i++)
    {
        pSystem->m_TX_Window[i].pPayload  = nullptr;
        pSystem->m_TX_Window[i].SeqStart  = 0;
        pSystem->m_TX_Window[i].SeqEnd    = 0;
        pSystem->m_TX_Window[i].TimeStamp = 0;
        pSystem->m_TX_Window[i].InUse     = false;
    }

    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "TCP: SEND SYN ->  %d.%d.%d.%d:%u localPort=%u\n", IP_A(ServerIP), IP_B(ServerIP), IP_C(ServerIP), IP_D(ServerIP),Port, local.Port);

    IP_PacketMsg_t* pMsg = SendSegment(pTCP, nullptr, 0, TCP_FLAG_SYN, false);

    if(pMsg == nullptr)
    {
        socketMgr.FreeSocket(&pSocketBase);
        return nullptr;
    }

    pSystem->m_State = TCP_STATE_SYN_SENT;
    m_pClientSocket  = pTCP;

    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "TCP: STATE -> SYN_SENT (waiting SYN+ACK)\n");
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
SystemState_e TCP_ManagerSystem::EnterListen(Socket* pSocket, uint16_t Backlog)
{
    if(pSocket == nullptr)
    {
        return SYS_ERROR;
    }

    TCP_Socket* pTCPSocket = static_cast<TCP_Socket*>(pSocket);

    if(pTCPSocket->GetLocalPort() == 0)
    {
        return SYS_ERROR;
    }

    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        if(m_pServerSockets[i] == pTCPSocket)
        {
            return SYS_ERROR;
        }
    }

    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        if(m_pServerSockets[i] == nullptr)
        {
            m_pServerSockets[i] = pTCPSocket;

            pTCPSocket->m_State = TCP_STATE_LISTEN;

            return SYS_READY;
        }
    }

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
void TCP_ManagerSystem::Close(Socket* pSocket)
{
    if(pSocket == nullptr)
    {
        return;
    }

    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        TCP_Socket* pTCPSock = m_pServerSockets[i];

        if(pTCPSock == nullptr)
        {
            continue;
        }

        if(pTCPSock == static_cast<TCP_Socket*>(pSocket))
        {
            pTCPSock->Close();
            return;
        }
    }
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
void TCP_ManagerSystem::Process(void)
{
    UpdateTimers();

#if (IP_USE_TCP_CLIENT == DEF_ENABLED)
    if(m_pClientSocket != nullptr)
    {
        TCP_SocketSystem* pSystem = static_cast<TCP_SocketSystem*>(m_pClientSocket);

        if(pSystem->IsConnected())
        {
            pSystem->RetransmitIfNeeded();
        }
    }
#endif

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
            TCP_SocketSystem* pSystem = static_cast<TCP_SocketSystem*>(pTCP);

            if(pSystem->IsConnected())
            {
                pSystem->RetransmitIfNeeded();
            }
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
void TCP_ManagerSystem::ProcessSegment(IP_PacketMsg_t* pMsg)
{
    do
    {
        if((pMsg == nullptr) || (pMsg->pPacket == nullptr))
        {
            break;
        }

        IP_EthernetPacket_t* pPacket = pMsg->pPacket;

        if(pMsg->PacketSize < sizeof(TCP_Frame_t))
        {
            break;
        }

        TCP_Socket* pSocket = nullptr;

        if(ParseTCP_Header(pPacket, pSocket) != true)
        {
            break;
        }

        const TCP_Header_t& hdr = pPacket->TCP_Frame.Header;
        const IP_Header_t&  ip  = pPacket->TCP_Frame.IP_Header;

        uint8_t  Flags = hdr.Flags;
        uint32_t Seq   = ntohl(hdr.SequenceNumber);
        uint32_t Ack   = ntohl(hdr.AcknowledgeNumber);

        uint8_t  ipHeaderLen  = (ip.VersionIHL & 0x0F) * 4;
        uint8_t  tcpHeaderLen = (hdr.Offset >> 4) * 4;
        uint16_t totalLength  = ntohs(ip.Length);

        size_t PayloadLen = 0;

        if(totalLength >= (ipHeaderLen + tcpHeaderLen))
        {
            PayloadLen = static_cast<size_t>(totalLength - ipHeaderLen - tcpHeaderLen);
        }

        // ---------------------------------------------------------------------
        // Compute payload pointer (zero-copy)
        // ---------------------------------------------------------------------
        pMsg->Payload = ((uint8_t*)&pPacket->TCP_Frame.Header) + tcpHeaderLen;
        pMsg->PayloadSize = PayloadLen;

        // ---------------------------------------------------------------------
        // Forward to TCP state machine (TCP owns pMsg)
        // ---------------------------------------------------------------------
        TCP_SocketSystem* pSystem = static_cast<TCP_SocketSystem*>(pSocket);


        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "TCP: RX flags=0x%02X seq=%u ack=%u state=%d payload=%u\n", Flags, Seq, Ack, pSystem->m_State, PayloadLen);

        if((Flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) == (TCP_FLAG_SYN | TCP_FLAG_ACK))
        {
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "TCP: RX SYN+ACK detected\n");
        }

        // ⭐ LOG #3 — ACK simple
        if((Flags & TCP_FLAG_ACK) && !(Flags & TCP_FLAG_SYN))
        {
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "TCP: RX ACK\n");
        }

        // ⭐ LOG #4 — FIN
        if(Flags & TCP_FLAG_FIN)
        {
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "TCP: RX FIN\n");
        }

        // ⭐ LOG #5 — Appel au state machine
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "TCP: Processing flags in state=%d\n", pSystem->m_State);



        pSystem->ProcessIncomingFlags(pSocket, pMsg, Flags, Seq, Ack);
        return;
    }
    while(0);

    IP_Manager::FreeMessage(pMsg);
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
IP_PacketMsg_t* TCP_ManagerSystem::SendSegment(TCP_Socket* pSocket, const uint8_t* pPayload, size_t Length, uint8_t Flags, bool Retransmit)
{
    if(pSocket == nullptr)
    {
        return nullptr;
    }

    TCP_SocketSystem* pSystem     = static_cast<TCP_SocketSystem*>(pSocket);
    Socket*           pSocketBase = static_cast<Socket*>(pSystem);
    IP_Manager*       pIP_Manager = m_pContext->GetIP_Manager();

    if(pIP_Manager == nullptr)
    {
        return nullptr;
    }

    SocketInfo_t localInfo;
    SocketInfo_t remoteInfo;
    pSocketBase->GetLocalInfo(&localInfo);
    pSocketBase->GetRemoteInfo(&remoteInfo);

    IP_PacketMsg_t* pMsg = nullptr;

    SystemState_e State = IP_Manager::AllocPacket(&pMsg, sizeof(IP_EthernetPacket_t), MEM_DBG_TCP, MEM_DBG_TCPDT);

    if((State != SYS_READY) || (pMsg == nullptr))
    {
        return nullptr;
    }

    IP_EthernetPacket_t* pPacket = pMsg->pPacket;
    TCP_Header_t&        hdr     = pPacket->TCP_Frame.Header;

    IP_MAC_Address_t SourceMAC;
    m_pContext->GetMAC_Address(&SourceMAC);
    memcpy(&pPacket->ETH_Header.SourceMAC, &SourceMAC, 6);
    pPacket->ETH_Header.Type = IP_ETHERNET_TYPE_IPV4;

    hdr.SrcPort           = localInfo.Port;
    hdr.DstPort           = remoteInfo.Port;
    hdr.SequenceNumber    = htonl(pSystem->m_SeqNumber);
    hdr.AcknowledgeNumber = htonl(pSystem->m_AckNumber);
    hdr.Flags             = Flags;
    hdr.Window            = htons(pSystem->m_LocalWindow);
    hdr.UrgentPointer     = 0;
    hdr.Offset            = (sizeof(TCP_Header_t) / 4) << 4;
    hdr.OptionData.Kind   = 2;                 // MSS option
    hdr.OptionData.Length = 4;                 // always 4
    hdr.OptionData.MSS    = HTONS(TCP_MSS);    // 1460


    if((pPayload != nullptr) && (Length > 0))
    {
        uint8_t* pTCP_Payload = ((uint8_t*)&pPacket->TCP_Frame.Header) + sizeof(TCP_Header_t);
        memcpy(pTCP_Payload, pPayload, Length);
    }

    IP_Header_t& ip = pPacket->TCP_Frame.IP_Header;
    uint16_t tcpLen = static_cast<uint16_t>(sizeof(TCP_Header_t) + Length);

    ip.VersionIHL          = (4u << 4) | 5u;
    ip.TypeOfService       = 0;
    ip.Length              = htons(static_cast<uint16_t>(sizeof(IP_Header_t) + tcpLen));
    ip.ID                  = 0;
    ip.FlagsFragmentOffset = 0;
    ip.TimeToLive          = 64;
    ip.Protocol            = IP_PROTOCOL_TCP;
    ip.SrcIP_Address       = localInfo.Address;
    ip.DstIP_Address       = remoteInfo.Address;
    ip.Checksum = 0;
    ip.Checksum = LIB_HTONS_Checksum16((uint8_t*)&ip, sizeof(IP_Header_t));
//    ip.Checksum = IP_Manager::IP_CalculateChecksum(&ip, sizeof(IP_Header_t));
    hdr.Checksum = 0;
    hdr.Checksum = LIB_HTONS_Checksum16((uint8_t*)&hdr, tcpLen);
//    hdr.Checksum = IP_Manager::TCP_CalculateChecksum(&ip, &hdr, tcpLen);

    pMsg->Payload     = nullptr;
    pMsg->PayloadSize = 0;
    pMsg->PacketSize  = sizeof(IP_EthernetHeader_t) + sizeof(IP_Header_t) + tcpLen;
    State = pIP_Manager->SendPacket(pMsg);

    if((State != SYS_READY) && (State != SYS_ARP_RESOLVE_PENDING))
    {
        IP_Manager::FreeMessage(pMsg);
        return nullptr;
    }

    if(Retransmit == false)
    {
        uint32_t SeqStart = pSystem->m_SeqNumber;
        uint32_t SeqEnd   = SeqStart;

        if((Flags & TCP_FLAG_SYN) != 0) SeqEnd++;
        if((Flags & TCP_FLAG_FIN) != 0) SeqEnd++;

        SeqEnd += static_cast<uint32_t>(Length);

        pSystem->m_SeqNumber = SeqEnd;

        for(size_t i = 0; i < TCP_MAX_TX_SEGMENTS; i++)
        {
            TCP_TX_Segment_t* pSlot = &pSystem->m_TX_Window[i];

            if(pSlot->InUse == false)
            {
                uint8_t* pCopy = nullptr;

                if(Length > 0)
                {
                    pCopy = (uint8_t*)pMemoryPool->Alloc(Length, MEM_DBG_TCPCPY);

                    if(pCopy == nullptr)
                    {
                        break;
                    }

                    memcpy(pCopy, pPayload, Length);
                }

                pSlot->pPayload  = pCopy;
                pSlot->SeqStart  = SeqStart;
                pSlot->SeqEnd    = SeqEnd;
                pSlot->Flags     = Flags;
                pSlot->Window    = pSystem->m_LocalWindow;
                pSlot->Length    = Length;
                pSlot->TimeStamp = GetTick();
                pSlot->InUse     = true;
                break;
            }
        }
    }

    pSystem->m_LastSendTick = GetTick();

    return pMsg;
}

//-------------------------------------------------------------------------------------------------

IP_PacketMsg_t* TCP_ManagerSystem::RebuildTCP_SegmentInPlace(
    IP_PacketMsg_t*      pMsg,
    const SocketInfo_t&  localInfo,
    const SocketInfo_t&  remoteInfo,
    uint32_t             Seq,
    uint32_t             Ack,
    uint8_t              Flags,
    uint16_t             Window,
    size_t               Length)
{
    if(pMsg == nullptr)
    {
        return nullptr;
    }

    return pMsg;
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
void TCP_SocketSystem::RetransmitIfNeeded(void)
{
    IP_Manager* pIP = GetContext()->GetIP_Manager();

    if(pIP == nullptr)
    {
        return;
    }

    TickCount_t Now = GetTick();

    SocketInfo_t localInfo;
    SocketInfo_t remoteInfo;
    GetLocalInfo(&localInfo);
    GetRemoteInfo(&remoteInfo);

    for(size_t i = 0; i < TCP_MAX_TX_SEGMENTS; i++)
    {
        TCP_TX_Segment_t* pSlot = &m_TX_Window[i];

        if(pSlot->InUse == true)
        {
            if((Now - pSlot->TimeStamp) >= m_RetransmitTimeOut)
            {
                pSlot->RetryCount++;

                if(pSlot->RetryCount >= TCP_MAX_RETRY)
                {
                    FlushTX_Slot(pSlot);
                    continue;
                }

                IP_PacketMsg_t* pMsg = nullptr;

                SystemState_e State =
                    IP_Manager::AllocPacket(&pMsg,
                                            sizeof(IP_EthernetPacket_t),
                                            MEM_DBG_TCP,
                                            MEM_DBG_TCPDT);

                if((State != SYS_READY) || (pMsg == nullptr))
                {
                    continue;
                }

                IP_EthernetPacket_t* pPacket = pMsg->pPacket;
                TCP_Header_t& hdr = pPacket->TCP_Frame.Header;

                IP_MAC_Address_t SourceMAC;
                GetContext()->GetMAC_Address(&SourceMAC);
                memcpy(&pPacket->ETH_Header.SourceMAC, &SourceMAC, 6);
                pPacket->ETH_Header.Type = IP_ETHERNET_TYPE_IPV4;


                hdr.SrcPort           = localInfo.Port;
                hdr.DstPort           = remoteInfo.Port;
                hdr.SequenceNumber    = htonl(pSlot->SeqStart);
                hdr.AcknowledgeNumber = htonl(m_AckNumber);
                hdr.Flags             = pSlot->Flags;
                hdr.Window            = htons(pSlot->Window);
                hdr.UrgentPointer     = 0;
                hdr.Offset            = (sizeof(TCP_Header_t) / 4) << 4;
                hdr.OptionData.Kind   = 2;                 // MSS option
                hdr.OptionData.Length = 4;                 // always 4
                hdr.OptionData.MSS    = HTONS(TCP_MSS);    // 1460

                if(pSlot->Length > 0)
                {
                    uint8_t* pTCP_Payload = ((uint8_t*)&pPacket->TCP_Frame.Header) + sizeof(TCP_Header_t);
                    memcpy(pTCP_Payload, pSlot->pPayload, pSlot->Length);
                }

                IP_Header_t& ip = pPacket->TCP_Frame.IP_Header;
                uint16_t tcpLen = static_cast<uint16_t>(sizeof(TCP_Header_t) + pSlot->Length);

                ip.VersionIHL          = (4u << 4) | 5u;
                ip.TypeOfService       = 0;
                ip.Length              = htons(static_cast<uint16_t>(sizeof(IP_Header_t) + tcpLen));
                ip.ID                  = 0;
                ip.FlagsFragmentOffset = 0;
                ip.TimeToLive          = 64;
                ip.Protocol            = IP_PROTOCOL_TCP;
                ip.SrcIP_Address       = localInfo.Address;
                ip.DstIP_Address       = remoteInfo.Address;

                //hdr.Checksum = IP_Manager::TCP_CalculateChecksum(&ip, &hdr, tcpLen);
                hdr.Checksum = LIB_HTONS_Checksum16((uint8_t*)&hdr, tcpLen);

                pMsg->Payload     = nullptr;
                pMsg->PayloadSize = 0;
                pMsg->PacketSize  = sizeof(IP_EthernetHeader_t) + sizeof(IP_Header_t) + tcpLen;   // ← CHANGÉ
                pIP->SendPacket(pMsg);

                pSlot->TimeStamp = Now;
            }
        }
    }
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
bool TCP_ManagerSystem::ParseTCP_Header(IP_EthernetPacket_t* pPacket, TCP_Socket*& pSockOut)
{
    pSockOut = nullptr;

    if(pPacket == nullptr)
    {
        return false;
    }

    TCP_Header_t& tcp = pPacket->TCP_Frame.Header;

    uint16_t DstPort = ntohs(tcp.DstPort);

#if (IP_USE_TCP_CLIENT == DEF_ENABLED)
    if(m_pClientSocket != nullptr)
    {
        Socket* pSocketBase = static_cast<Socket*>(static_cast<TCP_SocketSystem*>(m_pClientSocket));

        if(pSocketBase->GetLocalPort() == DstPort)
        {
            pSockOut = static_cast<TCP_Socket*>(m_pClientSocket);
            return true;
        }
    }
#endif

#if (IP_USE_TCP_SERVER == DEF_ENABLED)
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        TCP_Socket* pSocket = m_pServerSockets[i];

        if(pSocket == nullptr)
        {
            continue;
        }

        Socket* pSocketBase = static_cast<Socket*>(static_cast<TCP_SocketSystem*>(pSocket));

        if(pSocketBase->GetLocalPort() == DstPort)
        {
            pSockOut = pSocket;
            return true;
        }
    }
#endif

    return false;
}

//-------------------------------------------------------------------------------------------------

void TCP_SocketSystem::FlushTX_Slot(TCP_TX_Segment_t* pSlot)
{
    if(pSlot->pPayload != nullptr)
    {
        pMemoryPool->Free((void**)&pSlot->pPayload);
    }

    pSlot->pPayload  = nullptr;
    pSlot->SeqStart  = 0;
    pSlot->SeqEnd    = 0;
    pSlot->Flags     = 0;
    pSlot->Window    = 0;
    pSlot->Length    = 0;
    pSlot->TimeStamp = 0;
    pSlot->InUse     = false;
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
void TCP_ManagerSystem::UpdateTimers(void)
{
    TickCount_t Now = GetTick();

#if (IP_USE_TCP_CLIENT == DEF_ENABLED)
    if(m_pClientSocket != nullptr)
    {
        TCP_Socket*       pSocket = m_pClientSocket;
        TCP_SocketSystem* pSystem = static_cast<TCP_SocketSystem*>(pSocket);

        switch(pSystem->m_State)
        {
            case TCP_STATE_TIME_WAIT:
                if((Now - pSystem->m_LastReceivedTick) >= TCP_TIME_WAIT_TIMEOUT)
                {
                    CloseAndFreeSocket(pSystem);
                    m_pClientSocket = nullptr;
                }
                break;

            case TCP_STATE_CLOSED:
            case TCP_STATE_ERROR:
                CloseAndFreeSocket(pSystem);
                m_pClientSocket = nullptr;
                break;

            case TCP_STATE_LAST_ACK:
            case TCP_STATE_FIN_WAIT_2:
            case TCP_STATE_CLOSE_WAIT:
            case TCP_STATE_CLOSING:
                if((Now - pSystem->m_LastReceivedTick) >= TCP_GENERIC_CLOSE_TIMEOUT)
                {
                    CloseAndFreeSocket(pSystem);
                    m_pClientSocket = nullptr;
                }
                break;

            case TCP_STATE_SYN_SENT:
                if((Now - pSystem->m_LastReceivedTick) >= TCP_CONNECT_TIMEOUT)
                {
                    pSystem->m_State = TCP_STATE_CLOSED;
                    CloseAndFreeSocket(pSystem);
                    m_pClientSocket = nullptr;
                }
                break;

            default:
                break;
        }
    }
#endif

#if (IP_USE_TCP_SERVER == DEF_ENABLED)
    for(int i = 0; i < IP_TCP_MAX_LISTEN; i++)
    {
        TCP_Socket* pSocket = m_pServerSockets[i];
        if(pSocket == nullptr)
        {
            continue;
        }

        TCP_SocketSystem* pSystem = static_cast<TCP_SocketSystem*>(pSocket);

        switch(pSystem->m_State)
        {
            case TCP_STATE_TIME_WAIT:
                if((Now - pSystem->m_LastReceivedTick) >= TCP_TIME_WAIT_TIMEOUT)
                {
                    CloseAndFreeSocket(pSystem);
                    m_pServerSockets[i] = nullptr;
                }
                break;

            case TCP_STATE_CLOSED:
            case TCP_STATE_ERROR:
                CloseAndFreeSocket(pSystem);
                m_pServerSockets[i] = nullptr;
                break;

            case TCP_STATE_LAST_ACK:
            case TCP_STATE_FIN_WAIT_2:
            case TCP_STATE_CLOSE_WAIT:
            case TCP_STATE_CLOSING:
                if((Now - pSystem->m_LastReceivedTick) >= TCP_GENERIC_CLOSE_TIMEOUT)
                {
                    CloseAndFreeSocket(pSystem);
                    m_pServerSockets[i] = nullptr;
                }
                break;

            case TCP_STATE_SYN_SENT:
                if((Now - pSystem->m_LastReceivedTick) >= TCP_CONNECT_TIMEOUT)
                {
                    pSystem->m_State = TCP_STATE_CLOSED
                    CloseAndFreeSocket(pSystem);
                    m_pServerSockets[i] = nullptr;
                }
                break;

            default:
                break;
        }
    }
#endif
}

//-------------------------------------------------------------------------------------------------
void TCP_ManagerSystem::CloseAndFreeSocket(TCP_SocketSystem* pSystem)
{
    if(pSystem == nullptr)
    {
        return;
    }

    nOS_StatusReg   sr;
    IP_PacketMsg_t* pMsg = nullptr;

    while(pSystem->DequeueMessage(pMsg) == true)
    {
        if(pMsg != nullptr)
        {
            IP_Manager::FreeMessage(pMsg);
        }
    }

    for(size_t i = 0; i < TCP_MAX_TX_SEGMENTS; i++)
    {
        TCP_TX_Segment_t* pSlot = &pSystem->m_TX_Window[i];

        if(pSlot->InUse == true)
        {
            pSystem->FlushTX_Slot(pSlot);
        }
    }

    nOS_EnterCritical(sr);
    pSystem->m_State = TCP_STATE_CLOSED;

    if(pSystem->m_pEventHandler != nullptr)
    {
        pSystem->m_pEventHandler->OnSocketEvent(pSystem, SOCKET_EVENT_CLOSED);
    }

    pSystem->~TCP_SocketSystem();
    pMemoryPool->Free((void**)&pSystem);
    nOS_LeaveCritical(sr);
}

#endif //(IP_USE_TCP_CLIENT == DEF_ENABLED) || (IP_USE_TCP_SERVER == DEF_ENABLED)

