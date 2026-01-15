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

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//#define TCP_DEFAULT_WINDOW_SIZE         (4 * 1460)   // 5840 bytes
//#define TCP_TX_BUFFER_SIZE              (4 * 1460)   // 5840 bytes
//#define TCP_RX_BUFFER_SIZE              (4 * 1460)   // 5840 bytes
//#define UDP_RX_BUFFER_SIZE              2048
//#define RAW_RX_BUFFER_SIZE              1536

//-------------------------------------------------------------------------------------------------

Socket* SocketManager::AllocSocket(SocketType_e Type)
{
    for(uint16_t i = 0; i < SOCKET_MAX_COUNT; i++)
    {
        if(SocketManager::m_SocketInUse[i] == false)
        {
            SocketManager::m_SocketInUse[i] = true;
            Socket* pSock = &SocketManager::m_SocketTable[i];
            memset(pSock, 0, sizeof(Socket));                       // Wipe the entire socket object (protocol structs included)
            pSock->m_SocketID = i;                                  // Assign ID before Create()
            pSock->Create(Type);                                    // Fully initialize protocol-specific structures
            return pSock;
        }
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

void SocketManager::FreeSocket(Socket** ppSocket)
{
    if((ppSocket == nullptr) || (*ppSocket == nullptr))
    {
        return;
    }

    Socket* pSock = *ppSocket;
    uint16_t id = pSock->m_SocketID;

    pSock->Close();
    SocketManager::m_SocketInUse[id] = false;
    *ppSocket = nullptr;
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
    m_TimeoutMs    = DEFAULT_SOCKET_TIMEOUT;
    m_IsBlocking   = true;
    m_IsBound      = false;
    m_IsListening  = false;
    m_Backlog      = 0;
    m_Flags        = 0;

    // Reset local/remote endpoint info
    m_LocalInfo.Port  = 0;
    m_LocalInfo.IP    = NetIF_GetLocalIP();

    m_RemoteInfo.Port = 0;
    m_RemoteInfo.IP   = 0;

    // Clear protocol pointers
    m_Protocol.pUDP = nullptr;
    m_Protocol.pTCP = nullptr;

    switch(Type)
    {
        case SOCKET_TYPE_DGRAM:
        {
            m_Protocol.pUDP = &m_UDPStorage;

            UDP_Socket_t* pUdp = m_Protocol.pUDP;
            pUdp->LocalPort = 0;
            pUdp->LocalIP   = NetIF_GetLocalIP();
            pUdp->Flags     = 0;
            nOS_QueueCreate(&pUdp->RxQueue, pUdp->RxQueueBuffer, sizeof(UDP_Message_t), UDP_RX_QUEUE_DEPTH);
        }
        break;

        case SOCKET_TYPE_STREAM:
        {
            m_Protocol.pTCP    = &m_TCPStorage;
            TCP_Socket_t* pTcp = m_Protocol.pTCP;

            // Minimal TCP initialization
            pTcp->State        = TCP_STATE_CLOSED;
            pTcp->RxQueueCount = 0;
            pTcp->TxQueueCount = 0;
            pTcp->Flags        = 0;

            // (Full TCP state machine, seq numbers, windows, etc. will be added later)
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
    if(m_Type != SOCKET_TYPE_DGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUdp       = m_Protocol.pUDP;
    IP_Port_t     ActualPort = Port;

    if(Port == 0)
    {
        ActualPort = m_pNetUDP->AllocateEphemeralPort();
        
        if(ActualPort == 0)
        {
            return SYS_FAIL_PORT_IN_USE;   // No free ephemeral port
        }
    }

    if(m_pNetUDP->RegisterSocket(this, Port) == false)              // Ask NetUDP to register this port
    {
        return SYS_FAIL_PORT_IN_USE;
    }

    pUdp->LocalPort = Port;                                         // Store port locally
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
SystemState_e Socket::Listen(uint16_t Backlog)
{
    if(m_Type != SOCKET_TYPE_STREAM)            // Only TCP supports Listen()
    {
        return SYS_INVALID_STATE;
    }

    if(Backlog == 0)                            // Backlog must be non-zero
    {
        return SYS_INVALID_PARAM;
    }

    if(m_LocalInfo.Port == 0)                   // Socket must be bound to a local port before listening
    {
        return SYS_INVALID_STATE;
    }

    m_Tcp.m_Backlog = Backlog;                  // Configure TCP passive-open state
    m_IsListening = true;                       // Mark socket as listening
    return m_Tcp.EnterListenState();            // Enter LISTEN state
}

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

SystemState_e Socket::Accept(Socket** ppNewSocket)
{
    if(m_Type != SOCKET_TYPE_STREAM)
        return SYS_INVALID_STATE;

    if(ppNewSocket == nullptr)
        return SYS_INVALID_PARAM;

    // TCP passive open
    return m_Tcp.Accept(ppNewSocket);
}
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
SystemState_e Socket::Send(uint8_t* pData, size_t Length, size_t* pBytesSent)
{
    if(m_Type != SOCKET_TYPE_DGRAM)
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
    if(m_Type != SOCKET_TYPE_DGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUdp = m_Protocol.pUDP;
    return UDP_Send(pUdp, pData, Length, pDestInfo, pBytesSent);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Recv
//
//  Parameter(s):   uint8_t* pBuffer            Pointer to the user buffer where the received
//                                              payload will be copied.
//                  size_t   BufferSize         Size of the user buffer in bytes.
//                  size_t*  pBytesReceived     Output: number of payload bytes copied into
//                                              pBuffer.
//
//  Return:         SystemState_e   SYS_READY         – A packet was received and delivered.
//                                  SYS_TIMEOUT       – No packet available within the configured
//                                                      timeout.
//                                  SYS_INVALID_STATE – Called on a non‑UDP socket.
//
//  Description:    Receives the next UDP datagram from the socket’s RX queue. This function is
//                  a simplified variant of RecvFrom(), returning only the payload data without
//                  exposing the sender’s addressing information. The function extracts the UDP
//                  header, determines the payload length, copies the payload into the user
//                  buffer (clipped to BufferSize), and frees the underlying packet buffers.
//                  Ownership of the packet is returned to the memory pool after processing.
//
//-------------------------------------------------------------------------------------------------
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
        return SYS_TIMEOUT;
    }

    TCP_Header_t* pTCP = &pMsg->pPacket->TCP_Frame.Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->TCP_Frame.IP_Header;

    size_t headerLen = (pTCP->DataOffset >> 4) * 4;
    size_t ipLen     = ntohs(pIP->Length);
    size_t dataLen   = ipLen - sizeof(IP_IP_Header_t) - headerLen;

    if(dataLen > BufferSize)
    {
        dataLen = BufferSize;
    }

    uint8_t* pPayload = (uint8_t*)((uint8_t*)pTCP + headerLen);
    memcpy(pBuffer, pPayload, dataLen);

    // Free segment
    pMemoryPool->Free((void**)&pMsg->pPacket);
    pMemoryPool->Free((void**)&pMsg);

    *pBytesReceived = dataLen;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RecvFrom
//
//  Parameter(s):   uint8_t*        pBuffer         Pointer to the user buffer where the received
//                                                  payload will be copied.
//                  size_t          BufferSize      Size of the user buffer in bytes.
//                  SocketInfo_t*   pSrcInfo        Optional output: source IP address, UDP port
//                                                  and MAC address.
//                  size_t*         pBytesReceived  Output: number of payload bytes copied into
//                                                  pBuffer.
//
//  Return:         SystemState_e   SYS_READY         – A packet was received and delivered.
//                                  SYS_TIMEOUT       – No packet available within the configured
//                                                      timeout.
//                                  SYS_INVALID_STATE – Called on a non‑UDP socket.
//
//  Description:    Retrieves the next UDP datagram from the socket’s RX queue. The function
//                  extracts the UDP and IP headers, copies the payload into the user buffer,
//                  optionally returns the sender’s addressing information, and frees the
//                  underlying packet buffers. Ownership of the packet is transferred back to
//                  the memory pool after processing.
//
//-------------------------------------------------------------------------------------------------
SystemState_e Socket::RecvFrom(uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived)
{
    if(m_Type != SOCKET_TYPE_DGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUdp      = m_Protocol.pUDP;
    IP_PacketMsg_t* pMsg    = nullptr;
    nOS_TickCounter timeout = m_IsBlocking ? m_TimeoutMs : 0;           // Blocking or non-blocking timeout

    if(nOS_QueueRead(&pUdp->RxQueue, &pMsg, timeout) != NOS_OK)         // Read next message from UDP RX queue
    {
        return SYS_TIMEOUT;
    }

    // Extract headers
    UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Frame.Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->UDP_Frame.IP_Header;

    size_t udpLen = ntohs(pUDP->Length);
    size_t payloadLen = udpLen - sizeof(UDP_Header_t);

    if(payloadLen > BufferSize)
    {
        payloadLen = BufferSize;
    }

    uint8_t* pPayload = (uint8_t*)(pUDP + 1);                           // Payload pointer (UDP header is immediately followed by data)
    memcpy(pBuffer, pPayload, payloadLen);

    if(pSrcInfo != nullptr)                                             // Fill source info if requested
    {
        pSrcInfo->Address = pIP->SrcIP_Addr;
        pSrcInfo->Port    = ntohs(pUDP->SrcPort);
    }

    // Free packet buffers (zero-copy release)
    pMemoryPool->Free((void**)&pMsg->pPacket);
    pMemoryPool->Free((void**)&pMsg);

    *pBytesReceived = payloadLen;
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
void Socket::Close()
{
    if(m_Type == SOCKET_TYPE_DGRAM)
    {
        UDP_Socket_t* pUdp = m_Protocol.pUDP;

        if(pUdp->LocalPort != 0)                            // Unbind port if bound
        {
            m_pNetUDP->UnregisterSocket(pUdp->LocalPort);
            pUdp->LocalPort = 0;
        }

        FreeAllMessages(&pUdp->RxQueue);                    // Flush RX queue
    }

    else if(m_Type == SOCKET_TYPE_STREAM)
    {
        TCP_Socket_t* pTcp = m_Protocol.pTCP;

        // TCP cleanup (state machine, queues, etc.)
        FreeAllMessages(&pUdp->RxQueue);                    // Flush RX queue

        // Additional TCP teardown if needed
        // (state machine, retransmission buffers, etc.)
    }

    m_Type = SOCKET_TYPE_NONE;                              // Reset type so the allocator knows it's clean
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FreeAllMessages
//
//  Parameter(s):   nOS_Queue*  pQueue      Pointer to the message queue containing
//                                          IP_PacketMsg_t* entries.
//
//  Return:         None
//
//  Description:    Empties the specified message queue and releases all packet resources.
//                  Each entry in the queue is expected to be an IP_PacketMsg_t* containing
//                  a packet buffer allocated from the memory pool. The function repeatedly
//                  dequeues messages, frees the associated packet buffer, and then frees the
//                  message wrapper itself. This helper centralizes cleanup logic for both UDP
//                  and TCP sockets and ensures deterministic, leak‑free teardown.
//
//-------------------------------------------------------------------------------------------------
void Socket::FreeAllMessages(nOS_Queue* pQueue)
{
    IP_PacketMsg_t* pMsg = nullptr;

    while(!nOS_QueueIsEmpty(pQueue))                            // Drain the queue and free all pending messages
    {
        if(nOS_QueueRead(pQueue, &pMsg, 0) == NOS_OK)           // Read next pointer from the queue (non-blocking)  
    {
            if(pMsg != nullptr)
            {
                if(pMsg->pPacket != nullptr)                    // Free the packet buffer if allocated
                {
                    pMemoryPool->Free((void**)&pMsg->pPacket);
                }

                pMemoryPool->Free((void**)&pMsg);               // Free the message wrapper
            }
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

    if(m_Type == SOCKET_TYPE_DGRAM)
    {
        return (nOS_QueueIsEmpty(&m_Protocol.pUDP->RxQueue) == false) ? true : false;
    }

    if(m_Type == SOCKET_TYPE_STREAM)
    {
        return (nOS_QueueIsEmpty(&m_Protocol.pTCP->RxQueue) == false) ? true : false;
    }

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
