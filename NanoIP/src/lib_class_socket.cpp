//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_socket.c
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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


// it was a work in progress.. to be continued


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

#include <new>


//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//#define TCP_DEFAULT_WINDOW_SIZE         (4 * 1460)   // 5840 bytes
//#define TCP_TX_BUFFER_SIZE              (4 * 1460)   // 5840 bytes
//#define TCP_RX_BUFFER_SIZE              (4 * 1460)   // 5840 bytes
//#define UDP_RX_BUFFER_SIZE              2048
//#define RAW_RX_BUFFER_SIZE              1536

#define SOCKET_DEFAULT_TIME_OUT             1000

//-------------------------------------------------------------------------------------------------

void SocketManager::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;
    m_ActiveCount = 0;
}

//-------------------------------------------------------------------------------------------------

Socket* SocketManager::AllocSocket(SocketType_e Type)
{
    if(m_ActiveCount >= SOCKET_MAX_COUNT)              // Enforce maximum number of sockets
    {
        return nullptr;
    }

    void* pSocketMemory = pMemoryPool->Alloc(sizeof(Socket), MEM_DBG_SOCKALLOC);

    if(pSocketMemory == nullptr)
    {
        return nullptr;
    }

    Socket* pSocket = new (pSocketMemory)Socket(*m_pContext, *m_pContext->GetIP_Manager());
    pSocket->Create(Type);

    m_ActiveSockets[m_ActiveCount++] = pSocket;
    return pSocket;
}

//-------------------------------------------------------------------------------------------------

void SocketManager::FreeSocket(Socket** ppSocket)
{
    if((ppSocket == nullptr) || (*ppSocket == nullptr))
    {
        return;
    }

    Socket* pSocket = *ppSocket;
    pSocket->m_Active = false;                              // Freeze the socket so no new packets enter its RX queue

    if(pSocket->m_Type == SOCKET_TYPE_DATAGRAM)             // Drain RX queue safely (no race because demux now drops)
    {
        UDP_Socket_t* pUDP = pSocket->m_Protocol.pUDP;
        IP_PacketMsg_t* pMsg = nullptr;

        IP_Port_t port = pSocket->GetLocalPort();
        pSocket->m_Manager.UDP_UnregisterSocket(port);

        while(nOS_QueueRead(&pUDP->RX_Queue, &pMsg, 0) == NOS_OK)
        {
            IP_Manager::FreeMessage(pMsg);
        }

        if(pSocket->m_Type == SOCKET_TYPE_DATAGRAM)         // Free protocol-specific structures
        {
            pMemoryPool->Free((void**)&pSocket->m_Protocol.pUDP);
        }
    }

    pMemoryPool->Free((void**)&pSocket);                    // Free the socket object itself
    *ppSocket = nullptr;
}

//-------------------------------------------------------------------------------------------------
Socket* SocketManager::FindUDP_SocketByPort(IP_Port_t Port)
{
  #if (IP_USE_UDP == DEF_ENABLED)
    for(uint8_t i = 0; i < m_ActiveCount; i++)
    {
        Socket* pSocket = m_ActiveSockets[i];

        if(pSocket->m_Type != SOCKET_TYPE_DATAGRAM)
        {
            continue;
        }

        UDP_Socket_t* pUDP = pSocket->m_Protocol.pUDP;

        if(pUDP == nullptr)
        {
            continue;
        }

        if(pUDP->LocalPort == Port)
        {
            return pSocket;
        }
    }
  #endif

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

Socket* SocketManager::FindRAW_ByProtocol(uint8_t Protocol)
{
  #if (IP_USE_RAW == DEF_ENABLED)
    for(uint8_t i = 0; i < m_ActiveCount; i++)
    {
        Socket* pSocket = m_ActiveSockets[i];

        if(pSocket->m_Type != SOCKET_TYPE_RAW)
        {
            continue;
        }

        if(pSocket->GetRawProtocol() == Protocol)
        {
            return pSocket;
        }
    }
  #endif

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

Socket* SocketManager::FindTCP_Connection(uint32_t LocalIP, IP_Port_t LocalPort, uint32_t RemoteIP, IP_Port_t RemotePort)
{
  #if (IP_USE_TCP == DEF_ENABLED)
    for(uint8_t i = 0; i < m_ActiveCount; i++)
    {
        Socket* pSocket = m_ActiveSockets[i];

        if(pSocket->m_Type != SOCKET_TYPE_STREAM)
        {
            continue;
        }

        TCP_Socket_t* pTCP = pSocket->GetTCP();

        if(pTCP == nullptr)
        {
            continue;
        }

        if((pTCP->LocalIP   == LocalIP)   &&
           (pTCP->LocalPort == LocalPort) &&
           (pTCP->RemoteIP  == RemoteIP)  &&
           (pTCP->RemotePort== RemotePort))
        {
            return pSocket;
        }
    }
  #endif

    return nullptr;
}
//-------------------------------------------------------------------------------------------------

Socket::Socket(NetworkContext& Context, IP_Manager& Manager) : m_Context(Context),  m_Manager(Manager)
{
    m_Type        = SOCKET_TYPE_INVALID;
    m_State       = SOCKET_STATE_CLOSED;
    m_IsBlocking  = true;
    m_Active      = true;
    m_IsBound     = false;
    m_IsListening = false;
    m_Backlog     = 0;
    m_Flags       = 0;
    m_TimeoutMs   = SOCKET_DEFAULT_TIME_OUT;

    m_LocalInfo.Address  = 0;
    m_LocalInfo.Port     = 0;

    m_RemoteInfo.Address = 0;
    m_RemoteInfo.Port    = 0;

  #if (IP_USE_UDP == DEF_ENABLED)
    m_Protocol.pUDP = nullptr;
  #endif
  #if (IP_USE_TCP == DEF_ENABLED)
    m_Protocol.pTCP = nullptr;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Create
//
//  Parameter(s):   SocketType_e    Type    Socket type to initialize (UDP, TCP, etc.).
//
//  Return:         None
//
//  Description:    Initializes the socket according to the specified protocol type. The socket
//                  memory is assumed to have been cleared prior to this call (via AllocSocket).
//                  For UDP sockets, the function sets default local addressing, initializes the
//                  receive queue, and prepares protocol‑specific state. For TCP sockets, the
//                  function initializes the TCP control block and any required state machines.
//                  After initialization, the socket is in a valid, ready‑to‑use state.
//
//-------------------------------------------------------------------------------------------------
void Socket::Create(SocketType_e Type)
{
    m_Type         = Type;
    m_State        = SOCKET_STATE_CLOSED;

    // Reset local/remote endpoint info
    m_LocalInfo.Address  = m_Context.GetActiveIP();
    m_LocalInfo.Port     = 0;
    m_RemoteInfo.Address = 0;
    m_RemoteInfo.Port    = 0;

    // Clear protocol pointers

  #if (IP_USE_UDP == DEF_ENABLED)
    m_Protocol.pUDP = nullptr;
    memset(&m_UDP_Storage, 0, sizeof(m_UDP_Storage));
  #endif

  #if (IP_USE_TCP == DEF_ENABLED)
    m_Protocol.pTCP = nullptr;
    memset(&m_TCP_Storage, 0, sizeof(m_TCP_Storage));
  #endif

  #if (IP_USE_RAW == DEF_ENABLED)
    m_Protocol.pRAW = nullptr;
    memset(&m_RAW_Storage, 0, sizeof(m_RAW_Storage));
  #endif


    switch(Type)
    {
      #if (IP_USE_UDP == DEF_ENABLED)
        case SOCKET_TYPE_DATAGRAM:
        {
            m_Protocol.pUDP = &m_UDP_Storage;

            UDP_Socket_t* pUDP = m_Protocol.pUDP;
            pUDP->LocalPort = 0;
            pUDP->LocalIP   = m_Context.GetActiveIP();
            pUDP->Flags     = 0;
            nOS_QueueCreate(&pUDP->RX_Queue, pUDP->RX_QueueBuffer, sizeof(UDP_Message_t), UDP_RX_QUEUE_DEPTH);
        }
        break;
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        case SOCKET_TYPE_STREAM:
        {
            m_Protocol.pTCP    = &m_TCP_Storage;
            TCP_Socket_t* pTCP = m_Protocol.pTCP;

            // Minimal TCP initialization
            pTCP->State        = TCP_STATE_CLOSED;
            pTCP->RxQueueCount = 0;
            pTCP->TxQueueCount = 0;
            pTCP->Flags        = 0;

            // (Full TCP state machine, seq numbers, windows, etc. will be added later)
        }
        break;
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        case SOCKET_TYPE_RAW:
        {
            m_Protocol.pRAW = &m_RAW_Storage;
            m_RAW_Storage.Protocol = 0;     // user must set
            m_RAW_Storage.LocalIP  = 0;     // accept any
            m_RAW_Storage.RemoteIP = 0;     // accept any
        }
        break;
      #endif

        default:
        {
            m_State = SOCKET_STATE_ERROR;
            m_Type  = SOCKET_TYPE_INVALID;   // Add this enum value
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Bind
//
//  Parameter(s):   IP_Port_t   Port    Local UDP port to bind. If zero, an ephemeral port will be
//                                      automatically selected from the configured ephemeral range.
//
//  Return:         SystemState_e       SYS_READY            – Port successfully bound.
//                                      SYS_FAIL_PORT_IN_USE – Requested port already in use.
//                                      SYS_INVALID_STATE    – Called on a non‑UDP socket.
//
//  Description:    Associates the UDP socket with a local port. If Port is non‑zero, the
//                  function attempts to reserve that port via the UDP binding registry. If
//                  Port is zero, an ephemeral port is allocated and registered. Once bound,
//                  incoming datagrams addressed to this port will be delivered to the socket’s
//                  RX queue by the UDP layer.
//
//-------------------------------------------------------------------------------------------------
SystemState_e Socket::Bind(IP_Port_t Port)
{
    if(m_Type != SOCKET_TYPE_DATAGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUDP       = m_Protocol.pUDP;
    IP_Port_t     ActualPort = Port;

    if(Port == 0)
    {
        ActualPort = m_Manager.UDP_AllocateEphemeralPort();

        if(ActualPort == 0)
        {
            return SYS_FAIL_PORT_IN_USE;                        // No free ephemeral port
        }
    }

    if(m_Manager.UDP_RegisterSocket(this, ActualPort) == false) // Ask UDP_Protocol to register this port
    {
        return SYS_FAIL_PORT_IN_USE;
    }

    pUDP->LocalPort = ActualPort;                               // Store port locally
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Listen
//
//  Parameter(s):   uint16_t    Backlog     Maximum number of pending TCP connection requests that
//                                          may be queued before Accept() is called. Ignored for
//                                          UDP sockets.
//
//  Return:         SystemState_e   SYS_READY         – Socket successfully placed in listening
//                                                      state.
//                                  SYS_INVALID_STATE – Socket type does not support Listen() or
//                                                      socket is not bound to a local port.
//                                  SYS_INVALID_PARAM – Backlog value is zero.
//                                  SYS_FAIL          – TCP layer failed to enter passive-open
//                                                      state.
//
//  Description:    Places the socket into a passive listening state. This function is only
//                  valid for TCP stream sockets. The socket must already be bound to a local
//                  port via Bind(). Once in listening mode, incoming SYN segments addressed
//                  to the bound port are queued up to the specified backlog limit. Each
//                  pending connection may later be retrieved using Accept(), which creates a
//                  new socket representing the established TCP session.
//
//                  UDP sockets do not support Listen() and will return SYS_INVALID_STATE.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP == DEF_ENABLED)
SystemState_e Socket::Listen(uint16_t Backlog)
{
    if(m_Type != SOCKET_TYPE_STREAM)                    // Only TCP supports Listen()
    {
        return SYS_INVALID_STATE;
    }

    if(Backlog == 0)                                    // Backlog must be non-zero
    {
        return SYS_INVALID_PARAMETER;
    }

    if(m_LocalInfo.Port == 0)                           // Socket must be bound to a local port before listening
    {
        return SYS_INVALID_STATE;
    }

    m_IsListening = true;                               // Mark socket as listening
    return m_Manager.TCP_EnterListen(this, Backlog);    // Enter LISTEN state
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           Connect
//
//  Parameter(s):   SocketInfo_t*   pDestInfo     Remote endpoint information
//                                                (IP address, port, and optional MAC address).
//
//  Return:         SystemState_e   SYS_READY         – Connection established or remote endpoint
//                                                       stored.
//                                  SYS_INVALID_STATE – Socket type does not support Connect().
//                                  SYS_INVALID_PARAM – Null pointer or invalid destination port.
//                                  SYS_FAIL          – TCP handshake failed (for stream sockets).
//
//  Description:    Establishes a connection to a remote endpoint. For UDP sockets, this call
//                  does not perform any network exchange; it simply stores the destination
//                  address and port so that subsequent Send() operations can use the socket’s
//                  default remote endpoint.
//
//                  For TCP sockets, this function initiates an active open. It configures the
//                  TCP control block, sends a SYN segment, and transitions the socket into the
//                  SYN-SENT state. The function returns SYS_READY once the TCP three‑way
//                  handshake completes and the connection reaches the ESTABLISHED state. If the
//                  handshake fails or times out, the function returns SYS_FAIL.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP == DEF_ENABLED)
SystemState_e Socket::Connect(const SocketInfo_t* pDestInfo)
{
    if(m_Type == SOCKET_TYPE_DGRAM)
    {
        // UDP connect (store remote endpoint)
        if((pDestInfo == nullptr) || (pDestInfo->Port == 0))
        {
            return SYS_INVALID_PARAM;
        }

        m_RemoteInfo = *pDestInfo;
        return SYS_READY;
    }

    if(m_Type == SOCKET_TYPE_STREAM)
    {
        // TCP connect (active open)
        return m_Tcp.Connect(pDestInfo);
    }

    return SYS_INVALID_STATE;
}
#endif

//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP == DEF_ENABLED)
SystemState_e Socket::Accept(Socket** ppNewSocket)
{
    if(m_Type != SOCKET_TYPE_STREAM)
    {
        return SYS_INVALID_STATE;
    }

    if(ppNewSocket == nullptr)
    {
        return SYS_INVALID_PARAM;
    }

    // TCP passive open
    return m_Tcp.Accept(ppNewSocket);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameter(s):   uint8_t*    pData       Pointer to the payload buffer to transmit.
//                  size_t      Length      Number of payload bytes to send.
//                  size_t*     pBytesSent  Output: number of payload bytes successfully queued for
//
//  Return:         SystemState_e           SYS_READY         – Packet successfully queued.
//                                          SYS_INVALID_STATE – Socket not connected or not a UDP
//                                                              socket.
//                                          SYS_FAIL          – Lower layer rejected the packet.
//
//  Description:    Sends a UDP datagram using the socket’s preconfigured remote endpoint.
//                  This function requires the socket to be “connected” via Connect(), which
//                  stores the destination address and port in m_RemoteInfo. The function
//                  delegates the actual transmission to SendTo(), preserving the zero‑copy
//                  architecture.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP == DEF_ENABLED)
SystemState_e Socket::Send(uint8_t* pData, size_t Length, size_t* pBytesSent)
{
    if(m_Type != SOCKET_TYPE_DATAGRAM)
    {
        return SYS_INVALID_STATE;
    }

    // Must have a connected remote endpoint
    if(m_RemoteInfo.Port == 0)
    {
        return SYS_INVALID_STATE;
    }

    return SendTo(pData, Length, &m_RemoteInfo, pBytesSent);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendTo
//
//  Parameter(s):   uint8_t*        pData       Pointer to the payload buffer to transmit.
//                  size_t          Length      Number of payload bytes to send.
//                  SocketInfo_t*   pDestInfo   Destination addressing information
//                                              (IP address, UDP port, MAC address).
//                  size_t*         pBytesSent  Output: number of payload bytes successfully queued
//                                              for transmission.
//
//  Return:         SystemState_e   SYS_READY   – Packet successfully queued for transmission.
//                                  SYS_FAIL    – Interface TX callback rejected the packet.
//                                  SYS_INVALID_STATE – Called on a non‑UDP socket.
//
//  Description:    Sends a UDP datagram to the specified destination. The function validates
//                  the socket type, retrieves the associated UDP socket context, and delegates
//                  packet construction and transmission to the UDP layer. On success,
//                  ownership of the packet is transferred to the network interface TX engine,
//                  which is responsible for freeing the packet after transmission completes.
//
//-------------------------------------------------------------------------------------------------
SystemState_e Socket::SendTo(uint8_t* pData, size_t Length, SocketInfo_t* pDestInfo, size_t* pBytesSent)
{
    if(m_Type != SOCKET_TYPE_DATAGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUDP = m_Protocol.pUDP;
    return m_Manager.UDP_Send(pUDP, pData, Length, pDestInfo, pBytesSent);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Recv  (Zero-Copy Variant)
//
//  Parameter(s):   IP_PacketMsg_t** ppMessage
//                      Output: pointer to the received TCP segment message. The caller obtains
//                      full ownership of the message, including IP/TCP headers, payload pointer,
//                      and payload size. The caller must free the message via
//                      IP_Manager::FreeMessage() once processing is complete.
//
//  Return:         SystemState_e
//                      SYS_READY         – A TCP segment was dequeued and delivered.
//                      SYS_TIMEOUT       – No segment available within the configured timeout.
//                      SYS_INVALID_STATE – Called on a non-TCP socket.
//
//  Description:    Retrieves the next TCP segment from the socket’s RX queue and returns the
//                  complete IP_PacketMsg_t structure without copying any payload data. The caller
//                  accesses the TCP payload directly from the underlying packet buffer, enabling
//                  true zero-copy processing.
//
//  Note(s):        This method is intended for high-performance or protocol-level consumers that
//                  require direct access to the raw segment data. The buffered Recv() variant
//                  remains available for callers that prefer a traditional memcpy-based API.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP == DEF_ENABLED)
SystemState_e Socket::Recv(IP_PacketMsg_t** ppMessage)
{
    if(m_Type != SOCKET_TYPE_STREAM)
    {
        return SYS_INVALID_STATE;
    }

    TCP_Socket_t* pTcp = m_Protocol.pTCP;
    nOS_TickCounter Timeout = m_TimeoutMs;

    if(nOS_QueueRead(&pTcp->RxQueue, ppMessage, Timeout) != NOS_OK)
    {
        return SYS_TIME_OUT;
    }

    return SYS_READY;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           Recv
//
//  Parameter(s):   uint8_t* pBuffer            Pointer to the user buffer where the received
//                                              TCP payload will be copied.
//                  size_t   BufferSize         Size of the user buffer in bytes.
//                  size_t*  pBytesReceived     Output: number of payload bytes copied into
//                                              pBuffer.
//
//  Return:         SystemState_e
//                      SYS_READY         – A TCP segment was received and delivered.
//                      SYS_TIMEOUT       – No segment available within the configured timeout.
//                      SYS_INVALID_STATE – Called on a non‑TCP socket.
//
//  Description:    Retrieves the next TCP segment from the socket’s RX queue and copies its
//                  payload into the user-provided buffer. The function parses the IP and TCP
//                  headers to determine the payload offset and length, clips the copy to
//                  BufferSize, and frees the underlying packet message once processing is
//                  complete.
//
//  Note(s)         This is the buffered, POSIX-style receive method. A separate zero-copy
//                  variant is available for callers that require direct access to the packet
//                  memory without performing a memcpy.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP == DEF_ENABLED)
SystemState_e Socket::Recv(uint8_t* pBuffer, size_t BufferSize, size_t* pBytesReceived)
{
    if(m_Type != SOCKET_TYPE_STREAM)
    {
        return SYS_INVALID_STATE;
    }

    TCP_Socket_t* pTcp = m_Protocol.pTCP;
    IP_PacketMsg_t* pMsg = nullptr;

    if(nOS_QueueRead(&pTcp->RxQueue, &pMsg, m_TimeoutMs) != NOS_OK)
    {
        return SYS_TIME_OUT;
    }

    TCP_Header_t* pTCP = &pMsg->pPacket->TCP_Frame.Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->TCP_Frame.IP_Header;

    size_t headerLen = (pTCP->Offset >> 4) * 4;
    size_t ipLen     = ntohs(pIP->Length);
    size_t dataLen   = ipLen - sizeof(IP_Header_t) - headerLen;

    if(dataLen > BufferSize)
    {
        dataLen = BufferSize;
    }

    uint8_t* pPayload = (uint8_t*)((uint8_t*)pTCP + headerLen);
    memcpy(pBuffer, pPayload, dataLen);

    IP_Manager::FreeMessage(pMsg);                                      // Free segment

    *pBytesReceived = dataLen;
    return SYS_READY;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           RecvFrom
//
//  Parameter(s):   IP_PacketMsg_t** ppMessage
//                      Output: pointer to the received packet message. The caller obtains full
//                      ownership of the message, including headers, payload pointer, and payload
//                      size. The caller is responsible for freeing the message via
//                      IP_Manager::FreeMessage() when processing is complete.
//
//  Return:         SystemState_e
//                      SYS_READY         – A packet was dequeued and delivered to the caller.
//                      SYS_TIMEOUT       – No packet available within the configured timeout.
//                      SYS_INVALID_STATE – Called on a non-UDP socket.
//
//  Description:    Retrieves the next UDP datagram from the socket’s RX queue. Unlike the
//                  traditional buffered model, this function does not copy payload data into a
//                  user buffer. Instead, it returns the full IP_PacketMsg_t structure, which
//                  contains direct pointers to the UDP payload and its size.
//
//  Note(s)         This design follows the zero-copy principle: the UDP payload is never copied.
//                  The caller parses the packet directly from the underlying network buffer and
//                  must explicitly free the message once finished.
//
//-------------------------------------------------------------------------------------------------
SystemState_e Socket::RecvFrom(IP_PacketMsg_t** ppMessage)
{
    if(m_Type != SOCKET_TYPE_DATAGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUDP_Socket = m_Protocol.pUDP;
    nOS_TickCounter Timeout   = m_IsBlocking ? m_TimeoutMs : 0;

    // Read next message from UDP RX queue (returns pointer to message)
    if(nOS_QueueRead(&pUDP_Socket->RX_Queue, ppMessage, Timeout) != NOS_OK)
    {
        return SYS_TIME_OUT;
    }

    // Caller now owns the message and must free it
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RecvFrom   (Buffered Variant)
//
//  Parameter(s):   uint8_t*        pBuffer         Pointer to the user buffer where the received
//                                                  UDP payload will be copied.
//                  size_t          BufferSize      Size of the user buffer in bytes.
//                  SocketInfo_t*   pSrcInfo        Optional output: source IP address, UDP port,
//                                                  and MAC address.
//                  size_t*         pBytesReceived  Output: number of payload bytes copied into
//                                                  pBuffer.
//
//  Return:         SystemState_e
//                      SYS_READY         – A UDP datagram was received and delivered.
//                      SYS_TIMEOUT       – No datagram available within the configured timeout.
//                      SYS_INVALID_STATE – Called on a non‑UDP socket.
//
//  Description:    Retrieves the next UDP datagram from the socket’s RX queue, extracts the IP
//                  and UDP headers, determines the payload length, and copies the payload into
//                  the caller‑provided buffer (clipped to BufferSize). The function optionally
//                  returns the sender’s addressing information and frees the underlying packet
//                  buffers once processing is complete.
//
//  Notes(s):       This is the traditional buffered receive method. A separate zero‑copy
//                  RecvFrom() variant is available for callers that require direct access to the
//                  packet memory without performing a memcpy.
//
//-------------------------------------------------------------------------------------------------
SystemState_e Socket::RecvFrom(uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived)
{
    if(m_Type != SOCKET_TYPE_DATAGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t*   pUDP_Socket = m_Protocol.pUDP;
    IP_PacketMsg_t* pMsg        = nullptr;
    nOS_TickCounter Timeout     = m_IsBlocking ? m_TimeoutMs : 0;           // Blocking or non-blocking timeout

    if(nOS_QueueRead(&pUDP_Socket->RX_Queue, &pMsg, Timeout) != NOS_OK)         // Read next message from UDP RX queue
    {
        return SYS_TIME_OUT;
    }

    // Extract headers
    UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Frame.UDP_Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->UDP_Frame.IP_Header;

    size_t UDP_Length    = ntohs(pUDP->Length);
    size_t PayloadLength = UDP_Length - sizeof(UDP_Header_t);

    if(PayloadLength > BufferSize)
    {
        PayloadLength = BufferSize;
    }

    uint8_t* pPayload = (uint8_t*)(pUDP + 1);                           // Payload pointer  + 1 -> + sizeof(UDP header) (UDP header is immediately followed by data)
    memcpy(pBuffer, pPayload, PayloadLength);

    if(pSrcInfo != nullptr)                                             // Fill source info if requested
    {
        pSrcInfo->Address = pIP->SrcIP_Address;
        pSrcInfo->Port    = ntohs(pUDP->SrcPort);
    }

    IP_Manager::FreeMessage(pMsg);                                      // Free packet buffers (zero-copy release)
    *pBytesReceived = PayloadLength;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Close
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Closes the socket and releases all associated resources. For UDP sockets,
//                  the function unregisters the bound port (if any) and flushes the RX queue,
//                  freeing all pending packet buffers. For TCP sockets, the function performs
//                  protocol-specific teardown and clears any queued segments. After cleanup,
//                  the socket is left in an inert state and may be returned to the socket
//                  manager for reuse.
//
//-------------------------------------------------------------------------------------------------
void Socket::Close(void)
{
    m_Active = false;

    switch(m_Type)
    {
      #if (IP_USE_UDP == DEF_ENABLED)
        case SOCKET_TYPE_DATAGRAM:
        {
            UDP_Socket_t* pUDP_Socket = m_Protocol.pUDP;

            if(pUDP_Socket->LocalPort != 0)                             // Unbind port if bound
            {
                m_Manager.UDP_UnregisterSocket(pUDP_Socket->LocalPort);
                pUDP_Socket->LocalPort = 0;
            }

            FreeAllMessages(&pUDP_Socket->RX_Queue);                    // Flush RX queue
        }
        break;
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        case SOCKET_TYPE_STREAM:
        {
            TCP_Socket_t* pTCP_Socket = m_Protocol.pTCP;
            m_Manager.TCP_Close(this);                                  // Let TCP manager handle teardown

            // TCP cleanup (state machine, queues, etc.)
            FreeAllMessages(&pTCP_Socket->RX_Queue);                    // Flush RX queue

            // Additional TCP teardown if needed
            // (state machine, retransmission buffers, etc.)
        }
        break;
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        case SOCKET_TYPE_RAW:
        {
            RAW_Socket_t* pRAW = m_Protocol.pRAW;

            if(pRAW->Protocol != 0)
            {
                m_Manager.RAW_UnregisterSocket(pRAW->Protocol);
                pRAW->Protocol = 0;
            }

            FreeAllMessages(&pRAW->RX_Queue);
        }
        break;
      #endif

        default: break;
    }

    m_Type = SOCKET_TYPE_NONE;                              // Reset type so the allocator knows it's clean
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FreeAllMessages
//
//  Parameter(s):   nOS_Queue* pQueue        Queue containing IP_PacketMsg_t pointers.
//
//  Return:         void
//
//  Description:    Empties the specified message queue and releases every message it contains.
//                  Each dequeued message is passed to IP_Manager::FreeMessage(), the static
//                  destruction routine responsible for freeing both the packet buffer and the
//                  message wrapper. This ensures that all message cleanup follows the same
//                  zero‑copy‑safe logic, regardless of which subsystem generated the message.
//
//  Note(s):        - Uses non‑blocking queue reads to drain the queue completely.
//                  - Safe to call when the queue is already empty.
//                  - Intended for socket shutdown, error recovery, and cleanup paths.
//                  - Delegates all actual freeing logic to the centralized static FreeMessage().
//
//-------------------------------------------------------------------------------------------------
void Socket::FreeAllMessages(nOS_Queue* pQueue)
{
    IP_PacketMsg_t* pMsg = nullptr;

    while(nOS_QueueIsEmpty(pQueue) == false)                    // Drain the queue and free all pending messages
    {
        if(nOS_QueueRead(pQueue, &pMsg, 0) == NOS_OK)           // Read next pointer from the queue (non-blocking)
        {
            IP_Manager::FreeMessage(pMsg);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HasData
//
//  Parameter(s):   None
//
//  Return:         bool    true    – One or more received messages are queued for this socket.
//                          false   – No data is currently available.
//
//  Description:    Indicates whether the socket has pending received data. This function
//                  performs a non‑blocking check of the socket’s internal RX queue, which is
//                  populated asynchronously by the UDP or TCP dispatcher when incoming
//                  packets are delivered to the socket.
//
//                  For UDP sockets, each queued message corresponds to a complete datagram.
//                  For TCP sockets, queued segments represent available stream data. This
//                  function does not remove or inspect the data; it only reports whether any
//                  data is waiting to be read via Recv() or RecvFrom().
//
//-------------------------------------------------------------------------------------------------
bool Socket::HasData(void)
{
    if(m_State == SOCKET_STATE_CLOSED)
    {
        return false;
    }

  #if (IP_USE_UDP == DEF_ENABLED)
    if(m_Type == SOCKET_TYPE_DATAGRAM)
    {
        return (nOS_QueueIsEmpty(&m_Protocol.pUDP->RX_Queue) == false) ? true : false;
    }
  #endif

  #if (IP_USE_TCP == DEF_ENABLED)
    if(m_Type == SOCKET_TYPE_STREAM)
    {
        return (nOS_QueueIsEmpty(&m_Protocol.pTCP->RX_Queue) == false) ? true : false;
    }
  #endif

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetLocalInfo
//
//  Parameter(s):   SocketInfo_t*   pInfo   Output structure that receives the socket’s local
//                                          endpoint information, including IP address and port.
//                                          The caller must provide a valid pointer.
//
//  Return:         None
//
//  Description:    Retrieves the socket’s local endpoint information. For UDP and TCP sockets,
//                  this includes the local IP address and the port to which the socket is
//                  bound. If the socket has not been bound, the returned port value will be
//                  zero. This function does not modify socket state and performs no validation
//                  beyond checking the output pointer.
//
//-------------------------------------------------------------------------------------------------
void Socket::GetLocalInfo(SocketInfo_t* pInfo)
{
    if(pInfo == nullptr)
    {
        return;
    }

    // Copy the stored local endpoint information
    *pInfo = m_LocalInfo;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetRemoteInfo
//
//  Parameter(s):   SocketInfo_t*   pInfo   Output structure that receives the socket’s remote
//                                          endpoint information, including IP address and port.
//                                          The caller must provide a valid pointer.
//
//  Return:         None
//
//  Description:    Retrieves the socket’s remote endpoint information. For UDP sockets, this
//                  value is set when the application calls Connect() or when a datagram is
//                  received via RecvFrom(), depending on usage. For TCP sockets, the remote
//                  endpoint is established during the three‑way handshake and represents the
//                  connected peer.
//
//                  If the socket is not connected, the returned port value will be zero. This
//                  function does not modify socket state and performs no validation beyond
//                  checking the output pointer.
//
//-------------------------------------------------------------------------------------------------
void Socket::GetRemoteInfo(SocketInfo_t* pInfo)
{
    if(pInfo == nullptr)
    {
        return;
    }

    // Copy the stored remote endpoint information
    *pInfo = m_RemoteInfo;
}

//-------------------------------------------------------------------------------------------------

SystemState_e Socket::SetOption(SocketOption_e Option, void* pValue, size_t ValueSize)
{
    if(pValue == nullptr)
    {
        return SYS_INVALID_PARAMETER;
    }

    switch(Option)
    {
        case SOCKET_OPT_NON_BLOCKING:
        {
            if(ValueSize == sizeof(bool))
            {
                bool NonBlocking = *(bool*)pValue;
                m_IsBlocking = !NonBlocking;
                return SYS_READY;
            }

            return SYS_INVALID_PARAMETER;
        }

        case SOCKET_OPT_BROADCAST:
        {
            if(ValueSize == sizeof(bool))
            {
                bool Enable = *(bool*)pValue;

                if(Enable)
                {
                    m_Flags |= SOCKET_FLAG_BROADCAST;
                }
                else
                {
                    m_Flags &= ~SOCKET_FLAG_BROADCAST;
                }

                return SYS_READY;
            }

            return SYS_INVALID_PARAMETER;
        }
        // Add other options here as needed

        default:
            return SYS_INVALID_PARAMETER;
    }
}

//-------------------------------------------------------------------------------------------------
