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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"
#include <new>

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SOCKET_DEFAULT_TIME_OUT             1000

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext* pContext
//                      Pointer to the global network context used by all sockets.
//
//  Return:         void
//
//  Description:    Initializes the socket manager by storing the network context pointer and
//                  resetting the active socket count. This function must be called once during
//                  system startup before any sockets are created. No sockets are allocated or
//                  modified here; the manager simply prepares its internal tracking state.
//
//  Notes:          - The manager does not take ownership of the NetworkContext pointer.
//                  - All subsequent socket allocations will reference this context.
//                  - Safe to call only once during initialization.
//
//-------------------------------------------------------------------------------------------------
void SocketManager::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;
    m_ActiveCount = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AllocSocket
//
//  Parameter(s):   SocketType_e Type
//                      The protocol type (UDP, TCP, RAW) to initialize the socket as.
//
//  Return:         Socket*
//                      Pointer to a fully constructed and initialized socket, or nullptr if
//                      allocation fails or the maximum socket count has been reached.
//
//  Description:    Allocates a new socket object from the memory pool, constructs it in-place
//                  using placement-new, and initializes it via Socket::Create(). The newly
//                  created socket is added to the active socket list and becomes eligible to
//                  receive packets from the protocol dispatchers.
//
//                  This function performs no protocol-specific allocation itself; all protocol
//                  storage is allocated inside Socket::Create() according to the requested type.
//
//  Notes:          - The socket manager owns the lifetime of all sockets it allocates.
//                  - The caller must eventually release the socket via FreeSocket().
//                  - The memory pool is responsible for zeroing or initializing raw memory.
//
//-------------------------------------------------------------------------------------------------
Socket* SocketManager::AllocSocket(SocketType_e Type)
{
    if(m_ActiveCount >= SOCKET_MAX_COUNT)                                                       // Enforce maximum socket count
    {
        return nullptr;
    }

    void* pSocketMemory = pMemoryPool->Alloc(sizeof(Socket), MEM_DBG_SOCKALLOC);                // Allocate raw memory for the socket object

    if(pSocketMemory == nullptr)
    {
        return nullptr;
    }

    Socket* pSocket = new (pSocketMemory) Socket(*m_pContext, *m_pContext->GetIP_Manager());    // Construct the socket in-place (placement new)
    pSocket->Create(Type);                                                                      // Initialize protocol-specific structures
    m_ActiveSockets[m_ActiveCount++] = pSocket;                                                 // Register in active socket list
    return pSocket;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FreeSocket
//
//  Parameter(s):   Socket** ppSocket
//                      Pointer to a socket pointer. On return, *ppSocket is set to nullptr.
//
//  Return:         void
//
//  Description:    Releases all resources associated with a socket. The function performs a
//                  protocol-specific unregistration (UDP port, RAW protocol filter, TCP teardown),
//                  flushes the unified socket-level RX queue, frees protocol-specific storage,
//                  and finally frees the socket object itself.
//
//                  After this call, the socket pointer is invalid and set to nullptr.
//
//  Notes:          - Safe to call on inactive or partially initialized sockets.
//                  - The demultiplexer will no longer enqueue packets because m_Active is cleared.
//                  - All message freeing is delegated to IP_Manager::FreeMessage().
//
//-------------------------------------------------------------------------------------------------
void SocketManager::FreeSocket(Socket** ppSocket)
{
    if((ppSocket == nullptr) || (*ppSocket == nullptr))
    {
        return;
    }

    Socket* pSocket = *ppSocket;
    pSocket->m_Active = false;                                      // Prevent any new packets from being enqueued
    pSocket->FreeAllMessages(&pSocket->m_RX_Queue);                  // Flush all pending RX messages (unified queue)

    switch(pSocket->m_Type)                                         // Protocol-specific cleanup
    {
      #if (IP_USE_UDP == DEF_ENABLED)
        case SOCKET_TYPE_DATAGRAM:
        {
            UDP_Socket_t* pUDP = pSocket->m_Protocol.pUDP;

            if(pUDP && pUDP->LocalPort != 0)
            {
                UDP_UnregisterSocket(pUDP->LocalPort);
            }
        }
        break;
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        case SOCKET_TYPE_STREAM:
        {
            TCP_Close(pSocket);                                     // Let TCP manager handle FIN/RST, state machine tear-down, etc.
        }
        break;
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        case SOCKET_TYPE_RAW:
        {
            RAW_Socket_t* pRAW = pSocket->m_Protocol.pRAW;

            if(pRAW && pRAW->Protocol != 0)
            {
                RAW_UnregisterSocket(pRAW->Protocol);
            }
        }
        break;
      #endif

        default:
            break;
    }

    pSocket->FreeProtocolData();                                        // Free protocol-specific storage (dynamic allocation)
    pMemoryPool->Free((void**)&pSocket);                                // Free the socket object itself
    *ppSocket = nullptr;                                                // Invalidate caller's pointer
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UDP_UnregisterSocket
//
//  Parameter(s):   IP_Port_t Port
//                      The UDP port to unbind.
//
//  Return:         void
//
//  Description:    Removes the UDP socket bound to the specified port from the active binding
//                  table. After this call, incoming UDP datagrams addressed to this port will
//                  no longer be delivered to any socket. If no socket is bound to the port,
//                  the function performs no action.
//
//  Notes:          - Safe to call during socket tear-down.
//                  - The function does not free the socket; it only removes the binding.
//                  - The UDP dispatcher relies on FindUDP_SocketByPort(), so unbinding simply
//                    ensures that lookup returns nullptr.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_UDP == DEF_ENABLED)
void SocketManager::UDP_UnregisterSocket(IP_Port_t Port)
{
    for(uint8_t i = 0; i < m_ActiveCount; i++)
    {
        Socket* pSocket = m_ActiveSockets[i];

        if(pSocket->m_Type != SOCKET_TYPE_DATAGRAM)         // Only UDP sockets can be bound to ports
        {
            continue;
        }

        UDP_Socket_t* pUDP = pSocket->m_Protocol.pUDP;
        if(pUDP == nullptr)
        {
            continue;
        }

        if(pUDP->LocalPort == Port)                         // Match the bound port
        {
            pUDP->LocalPort = 0;                            // Unbind
            return;
        }
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindUDP_SocketByPort
//
//  Parameter(s):   IP_Port_t Port      The UDP destination port to search for.
//
//  Return:         Socket*             Pointer to the matching UDP socket, or nullptr if no socket
//                                      is bound to the specified port.
//
//  Description:    Searches the active socket list for a UDP socket whose dynamically allocated
//                  UDP_Socket_t structure has a LocalPort matching the specified value. This
//                  function is used by the UDP dispatcher to deliver incoming datagrams to the
//                  correct socket.
//
//                  Only sockets of type SOCKET_TYPE_DATAGRAM are considered. Sockets that are
//                  inactive, uninitialized, or whose protocol storage is missing are skipped.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_UDP == DEF_ENABLED)
Socket* SocketManager::FindUDP_SocketByPort(IP_Port_t Port)
{
    for(uint8_t i = 0; i < m_ActiveCount; i++)
    {
        Socket* pSocket = m_ActiveSockets[i];

        if(pSocket->m_Type != SOCKET_TYPE_DATAGRAM)             // Must be a UDP socket
        {
            continue;
        }

        UDP_Socket_t* pUDP = pSocket->m_Protocol.pUDP;

        if(pUDP == nullptr)                                     // Protocol storage must exist
        {
            continue;
        }

        if(pUDP->LocalPort == Port)                             // Match bound port
        {
            return pSocket;
        }
    }

    return nullptr;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindRAW_ByProtocol
//
//  Parameter(s):   uint8_t Protocol    The IP protocol number (e.g., ICMP = 1, IGMP = 2, UDP = 17,
//                                      etc.).
//
//  Return:         Socket*             Pointer to the matching RAW socket, or nullptr if no socket
//                                      is registered for the specified protocol.
//
//  Description:    Searches the active socket list for a RAW socket whose dynamically allocated
//                  RAW_Socket_t structure has a Protocol field matching the specified value.
//                  This function is used by the RAW dispatcher to deliver incoming packets
//                  based on their IP protocol number.
//
//                  Only sockets of type SOCKET_TYPE_RAW are considered. Sockets that are inactive,
//                  uninitialized, or whose protocol storage is missing are skipped.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_RAW == DEF_ENABLED)
Socket* SocketManager::FindRAW_ByProtocol(uint8_t Protocol)
{
    for(uint8_t i = 0; i < m_ActiveCount; i++)
    {
        Socket* pSocket = m_ActiveSockets[i];

        if(pSocket->m_Type != SOCKET_TYPE_RAW)                  // Must be a RAW socket
        {
            continue;
        }

        RAW_Socket_t* pRAW = pSocket->m_Protocol.pRAW;

        if(pRAW == nullptr)                                     // Protocol storage must exist
        {
            continue;
        }

        if(pRAW->Protocol == Protocol)                          // Match protocol number
        {
            return pSocket;
        }
    }

    return nullptr;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindTCP_Connection
//
//  Parameter(s):   uint32_t    LocalIP
//                      The local IPv4 address associated with the TCP connection.
//                  IP_Port_t   LocalPort
//                      The local TCP port.
//                  uint32_t    RemoteIP
//                      The remote IPv4 address of the peer.
//                  IP_Port_t   RemotePort
//                      The remote TCP port.
//
//  Return:         Socket*
//                      Pointer to the matching TCP socket, or nullptr if no active connection
//                      matches the specified 4‑tuple.
//
//  Description:    Searches the active socket list for a TCP socket whose dynamically allocated
//                  TCP_Socket_t structure matches the specified connection identifiers. This
//                  function is used by the TCP dispatcher to route incoming segments to the
//                  correct socket based on the full 4‑tuple (LocalIP, LocalPort, RemoteIP,
//                  RemotePort).
//
//                  Only sockets of type SOCKET_TYPE_STREAM are considered. Sockets that are
//                  inactive, uninitialized, or whose protocol storage is missing are skipped.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_TCP == DEF_ENABLED)
Socket* SocketManager::FindTCP_Connection(uint32_t LocalIP, IP_Port_t LocalPort, uint32_t RemoteIP, IP_Port_t RemotePort)
{
    for(uint8_t i = 0; i < m_ActiveCount; i++)
    {
        Socket* pSocket = m_ActiveSockets[i];

        // Must be a TCP socket
        if(pSocket->m_Type != SOCKET_TYPE_STREAM)
        {
            continue;
        }

        TCP_Socket_t* pTCP = pSocket->GetTCP();

        // Protocol storage must exist
        if(pTCP == nullptr)
        {
            continue;
        }

        // Match full 4‑tuple
        if((pTCP->LocalIP    == LocalIP)   &&
           (pTCP->LocalPort  == LocalPort) &&
           (pTCP->RemoteIP   == RemoteIP)  &&
           (pTCP->RemotePort == RemotePort))
        {
            return pSocket;
        }
    }
    return nullptr;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           Socket (Constructor)
//
//  Parameter(s):   NetworkContext& Context
//                      Reference to the global network context.
//                  IP_Manager&     Manager
//                      Reference to the IP manager responsible for packet allocation,
//                      routing, and message destruction.
//
//  Return:         None
//
//  Description:    Initializes a newly allocated socket object with default state. This
//                  constructor sets only socket-level fields; protocol-specific storage
//                  (UDP_Socket_t, TCP_Socket_t, RAW_Socket_t) is allocated later by
//                  Socket::Create() based on the requested socket type.
//
//                  The unified RX queue is also created in Socket::Create(), not here,
//                  because queue depth and buffer ownership depend on the protocol type.
//
//  Notes:          - The socket begins in a CLOSED state and is marked active so that
//                    the dispatcher may deliver packets after Create() completes.
//                  - All protocol pointers are initialized to nullptr.
//                  - No memory allocation occurs here.
//
//-------------------------------------------------------------------------------------------------
Socket::Socket(NetworkContext& Context, IP_Manager& Manager) : m_Context(Context), m_Manager(Manager)
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

  #if (IP_USE_RAW == DEF_ENABLED)
    m_Protocol.pRAW = nullptr;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Create
//
//  Parameter(s):   SocketType_e    Type    Socket type to initialize (UDP, TCP, RAW).
//
//  Return:         None
//
//  Description:    Initializes the socket according to the specified protocol type. Protocol
//                  storage is dynamically allocated and stored in the SocketProtocol_t union.
//                  The socket-level RX queue is initialized once and shared by all protocols.
//                  After initialization, the socket is ready for Bind(), Connect(), Send(),
//                  Recv().
//
//-------------------------------------------------------------------------------------------------
void Socket::Create(SocketType_e Type)
{
    m_Type  = Type;
    m_State = SOCKET_STATE_CLOSED;

    // Reset endpoint info
    m_LocalInfo.Address  = m_Context.GetActiveIP();
    m_LocalInfo.Port     = 0;
    m_RemoteInfo.Address = 0;
    m_RemoteInfo.Port    = 0;

    // Clear protocol pointer
    m_Protocol.pPtr = nullptr;

    // Initialize unified RX queue (shared by all protocols)
    nOS_QueueCreate(&m_RX_Queue, m_RX_QueueBuffer, sizeof(IP_PacketMsg_t*), SOCKET_RX_QUEUE_DEPTH);

    switch(Type)
    {
      #if (IP_USE_UDP == DEF_ENABLED)
        case SOCKET_TYPE_DATAGRAM:
        {
            // Allocate and zero UDP protocol storage
            m_Protocol.pUDP = (UDP_Socket_t*)pMemoryPool->AllocAndSet(sizeof(UDP_Socket_t), 0, MEM_DBG_SOCKET);

            if(m_Protocol.pUDP == nullptr)
            {
                m_State = SOCKET_STATE_ERROR;
                return;
            }

            UDP_Socket_t* pUDP = m_Protocol.pUDP;

            // This the only non-zero initialization
            pUDP->LocalIP = m_Context.GetActiveIP();
        }
        break;
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        case SOCKET_TYPE_STREAM:
        {
            // Allocate and zero TCP protocol storage
            m_Protocol.pTCP = (TCP_Socket_t*)pMemoryPool->->AllocAndSet(sizeof(TCP_Socket_t), 0, MEM_DBG_SOCKET);

            if(m_Protocol.pTCP == nullptr)
            {
                m_State = SOCKET_STATE_ERROR;
                return;
            }

            TCP_Socket_t* pTCP = m_Protocol.pTCP;

            // Only non-zero initialization
            pTCP->State      = TCP_STATE_CLOSED;
            pTCP->Mss        = TCP_DEFAULT_MSS;
            pTCP->WindowSize = TCP_DEFAULT_WINDOW_SIZE;
            pTCP->pSocket    = this;
        }
        break;
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        case SOCKET_TYPE_RAW:
        {
            // Allocate and zero RAW protocol storage
            m_Protocol.pRAW = (RAW_Socket_t*)m_Context.GetMemoryPool()->AllocAndSet(sizeof(RAW_Socket_t), 0, MEM_DBG_SOCKET);

            if(m_Protocol.pRAW == nullptr)
            {
                m_State = SOCKET_STATE_ERROR;
                return;
            }
        }
        break;
      #endif

        default:
        {
            m_State = SOCKET_STATE_ERROR;
            m_Type  = SOCKET_TYPE_INVALID;
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
//  Return:         SystemState_e       SYS_READY            - Port successfully bound.
//                                      SYS_FAIL_PORT_IN_USE - Requested port already in use.
//                                      SYS_INVALID_STATE    - Called on a non-UDP socket.
//
//  Description:    Associates the UDP socket with a local port. If Port is non-zero, the
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
        ActualPort = m_pContext->GetUDP().AllocateEphemeralPort();  // Allocate ephemeral port if needed

        if(ActualPort == 0)
        {
            return SYS_FAIL_PORT_IN_USE;                            // No free ephemeral port
        }
    }

    if(m_Manager.UDP_RegisterSocket(this, ActualPort) == false)     // Ask UDP_Manager to register this port
    {
        return SYS_FAIL_PORT_IN_USE;
    }

    pUDP->LocalPort = ActualPort;                                   // Store port locally
    m_IsBound = true;
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
//  Return:         SystemState_e   SYS_READY         - Socket successfully placed in listening
//                                                      state.
//                                  SYS_INVALID_STATE - Socket type does not support Listen() or
//                                                      socket is not bound to a local port.
//                                  SYS_INVALID_PARAM - Backlog value is zero.
//                                  SYS_FAIL          - TCP layer failed to enter passive-open
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
//  Return:         SystemState_e   SYS_READY         - Connection established or remote endpoint
//                                                       stored.
//                                  SYS_INVALID_STATE - Socket type does not support Connect().
//                                  SYS_INVALID_PARAM - Null pointer or invalid destination port.
//                                  SYS_FAIL          - TCP handshake failed (for stream sockets).
//
//  Description:    Establishes a connection to a remote endpoint. For UDP sockets, this call
//                  does not perform any network exchange; it simply stores the destination
//                  address and port so that subsequent Send() operations can use the socket’s
//                  default remote endpoint.
//
//                  For TCP sockets, this function initiates an active open. It configures the
//                  TCP control block, sends a SYN segment, and transitions the socket into the
//                  SYN-SENT state. The function returns SYS_READY once the TCP three-way
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
//  Return:         SystemState_e           SYS_READY         - Packet successfully queued.
//                                          SYS_INVALID_STATE - Socket not connected or not a UDP
//                                                              socket.
//                                          SYS_FAIL          - Lower layer rejected the packet.
//
//  Description:    Sends a UDP datagram using the socket’s preconfigured remote endpoint.
//                  This function requires the socket to be "connected" via Connect(), which
//                  stores the destination address and port in m_RemoteInfo. The function
//                  delegates the actual transmission to SendTo(), preserving the zero-copy
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
//  Return:         SystemState_e   SYS_READY   - Packet successfully queued for transmission.
//                                  SYS_FAIL    - Interface TX callback rejected the packet.
//                                  SYS_INVALID_STATE - Called on a non-UDP socket.
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
//                      SYS_READY         - A TCP segment was dequeued and delivered.
//                      SYS_TIMEOUT       - No segment available within the configured timeout.
//                      SYS_INVALID_STATE - Called on a non-TCP socket.
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
SystemState_e Socket::Recv(IP_PacketMsg_t** ppMsg)
{
    if(m_Type != SOCKET_TYPE_STREAM)
    {
        return SYS_INVALID_STATE;
    }

    TCP_Socket_t* pTCP = m_Protocol.pTCP;
    nOS_TickCounter Timeout = m_TimeoutMs;

    if(nOS_QueueRead(&pTCP->RxQueue, ppMsg, Timeout) != NOS_OK)
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
//                      SYS_READY         - A TCP segment was received and delivered.
//                      SYS_TIMEOUT       - No segment available within the configured timeout.
//                      SYS_INVALID_STATE - Called on a non-TCP socket.
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

    TCP_Socket_t* pTCP_Socket = m_Protocol.pTCP;
    IP_PacketMsg_t* pMsg = nullptr;

    if(nOS_QueueRead(&pTCP_Socket->RxQueue, &pMsg, m_TimeoutMs) != NOS_OK)
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
//  Parameter(s):   IP_PacketMsg_t** ppMsg
//                      Output: pointer to the received packet message. The caller obtains full
//                      ownership of the message, including headers, payload pointer, and payload
//                      size. The caller is responsible for freeing the message via
//                      IP_Manager::FreeMessage() when processing is complete.
//
//  Return:         SystemState_e
//                      SYS_READY         - A packet was dequeued and delivered to the caller.
//                      SYS_TIMEOUT       - No packet available within the configured timeout.
//                      SYS_INVALID_STATE - Called on a non-UDP socket.
//
//  Description:    Retrieves the next UDP datagram from the socket’s unified RX queue.
//                  Zero-copy: the caller receives the full IP_PacketMsg_t* and must free it.
//
//-------------------------------------------------------------------------------------------------
SystemState_e Socket::RecvFrom(IP_PacketMsg_t** ppMsg)
{
    if(m_Type != SOCKET_TYPE_DATAGRAM)
    {
        return SYS_INVALID_STATE;
    }

    nOS_TickCounter Timeout = m_IsBlocking ? m_TimeoutMs : 0;

    if(nOS_QueueRead(&m_RX_Queue, ppMsg, Timeout) != NOS_OK)     // Read next message from the socket-level RX queue
    {
        return SYS_TIME_OUT;
    }

    return SYS_READY;                                           // Caller now owns the message
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RecvFrom   (Buffered Variant)
//
//  Parameter(s):   uint8_t*        pBuffer        Pointer to the user buffer where the received
//                                                 UDP payload will be copied.
//                  size_t          BufferSize     Size of the user buffer in bytes.
//                  SocketInfo_t*   pSrcInfo       Optional output: source IP address and UDP port.
//                  size_t*         pBytesReceived Output: number of payload bytes copied.
//
//  Return:         SystemState_e
//                      SYS_READY         - A UDP datagram was received and delivered.
//                      SYS_TIMEOUT       - No datagram available within the configured timeout.
//                      SYS_INVALID_STATE - Called on a non-UDP socket.
//
//  Description:    Retrieves the next UDP datagram from the socket’s unified RX queue, extracts
//                  the IP and UDP headers, determines the payload length, and copies the payload
//                  into the caller-provided buffer (clipped to BufferSize). The caller must free
//                  the underlying packet message after processing.
//
//-------------------------------------------------------------------------------------------------
SystemState_e Socket::RecvFrom(uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived)
{
    if(m_Type != SOCKET_TYPE_DATAGRAM)
    {
        return SYS_INVALID_STATE;
    }

    IP_PacketMsg_t* pMsg = nullptr;
    nOS_TickCounter Timeout = m_IsBlocking ? m_TimeoutMs : 0;

    if(nOS_QueueRead(&m_RX_Queue, &pMsg, Timeout) != NOS_OK)             // Read next message from the unified socket RX queue
    {
        return SYS_TIME_OUT;
    }

    size_t PayloadLength = pMsg->PayloadSize;                           // Extract payload length (already validated by UDP layer)

    if(PayloadLength > BufferSize)
    {
        PayloadLength = BufferSize;
    }

    memcpy(pBuffer, pMsg->Payload, PayloadLength);                      // Copy payload into user buffer

    if(pSrcInfo != nullptr)                                             // Optional: return source IP + port
    {
        IP_Header_t*  pIP  = &pMsg->pPacket->UDP_Frame.IP_Header;
        UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Frame.UDP_Header;

        pSrcInfo->Address = pIP->SrcIP_Address;
        pSrcInfo->Port    = ntohs(pUDP->SrcPort);
    }

    IP_Manager::FreeMessage(pMsg);                                      // Caller now owns the message -> free it
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
//                  the function unregisters the bound port (if any). For TCP sockets, the TCP
//                  manager performs protocol-specific tear-down. All pending RX messages in the
//                  unified socket RX queue are freed. Protocol storage is dynamically freed.
//                  After cleanup, the socket is inert and ready for reuse.
//
//-------------------------------------------------------------------------------------------------
void Socket::Close(void)
{
    m_Active = false;

    FreeAllMessages(&m_RX_Queue);                                        // Flush all pending RX messages (shared queue for all protocols)

    switch(m_Type)
    {
      #if (IP_USE_UDP == DEF_ENABLED)
        case SOCKET_TYPE_DATAGRAM:
        {
            UDP_Socket_t* pUDP = m_Protocol.pUDP;

            if((pUDP != nullptr) && (pUDP->LocalPort != 0))             // Unregister bound port
            {
                m_Manager.UDP_UnregisterSocket(pUDP->LocalPort);
            }
        }
        break;
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        case SOCKET_TYPE_STREAM:
        {
            m_Manager.TCP_Close(this);                                  // Let TCP manager handle tear-down (FIN, RST, etc.)
        }
        break;
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        case SOCKET_TYPE_RAW:
        {
            RAW_Socket_t* pRAW = m_Protocol.pRAW;

            if((pRAW != nullptr) && (pRAW->Protocol != 0))              // Unregister protocol filter if set
            {
                m_Manager.RAW_UnregisterSocket(pRAW->Protocol);
            }
        }
        break;
      #endif

        default:
            break;
    }

    FreeProtocolData();                                                 // Free protocol-specific storage
    m_Type = SOCKET_TYPE_NONE;                                          // Reset type so allocator knows this socket is free
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
//                  Each dequeued message is passed to IP_Manager::FreeMessage(), which frees
//                  both the packet buffer and the message wrapper.
//
//-------------------------------------------------------------------------------------------------
void Socket::FreeAllMessages(nOS_Queue* pQueue)
{
    IP_PacketMsg_t* pMsg = nullptr;

    while(nOS_QueueRead(pQueue, &pMsg, 0) == NOS_OK)                    // Drain queue completely (non-blocking)
    {
        IP_Manager::FreeMessage(pMsg);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FreeProtocolData
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Releases any dynamically allocated protocol-specific storage associated with
//                  this socket (UDP_Socket_t, TCP_Socket_t, RAW_Socket_t). This function does
//                  not modify the socket type or state; it only frees protocol-level structures
//                  that were allocated by Socket::Create().
//
//  Notes:          - Safe to call multiple times; null checks prevent double-free.
//                  - The socket object itself is not freed here.
//                  - The caller is responsible for updating m_Type as needed.
//
//-------------------------------------------------------------------------------------------------
void Socket::FreeProtocolData(void)
{
  #if (IP_USE_UDP == DEF_ENABLED)
    if(m_Protocol.pUDP != nullptr)
    {
        pMemoryPool->Free((void**)&m_Protocol.pUDP);
        m_Protocol.pUDP = nullptr;
    }
  #endif

  #if (IP_USE_TCP == DEF_ENABLED)
    if(m_Protocol.pTCP != nullptr)
    {
        pMemoryPool->Free((void**)&m_Protocol.pTCP);
        m_Protocol.pTCP = nullptr;
    }
  #endif

  #if (IP_USE_RAW == DEF_ENABLED)
    if(m_Protocol.pRAW != nullptr)
    {
        pMemoryPool->Free((void**)&m_Protocol.pRAW);
        m_Protocol.pRAW = nullptr;
    }
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HasData
//
//  Parameter(s):   None
//
//  Return:         bool
//                      true  - One or more received messages are queued for this socket.
//                      false - No data is currently available.
//
//  Description:    Indicates whether the socket has pending received data. This function performs
//                  a non-blocking check of the socket’s unified RX queue, which is populated by
//                  the UDP, TCP, or RAW dispatcher depending on the socket type.
//
//-------------------------------------------------------------------------------------------------
bool Socket::HasData(void)
{
    if(m_State == SOCKET_STATE_CLOSED)
    {
        return false;
    }

    return (nOS_QueueIsEmpty(&m_RX_Queue) == false);
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

    *pInfo = m_LocalInfo;                       // Copy the stored local endpoint information
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
//                  endpoint is established during the three-way handshake and represents the
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

    *pInfo = m_RemoteInfo;                      // Copy the stored remote endpoint information
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetOption
//
//  Parameter(s):   SocketOption_e  Option      Socket option to configure.
//                  void*           pValue      Pointer to the option value.
//                  size_t          ValueSize   Size of the option value in bytes.
//
//  Return:         SystemState_e
//                      SYS_READY             – Option successfully applied.
//                      SYS_INVALID_PARAMETER – Invalid option or incorrect value size.
//
//  Description:    Configures socket-level options such as blocking mode or broadcast
//                  permissions. The function validates the option identifier and the size
//                  of the provided value before applying it. Only socket-level behavior is
//                  affected; protocol-specific options are handled elsewhere.
//
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
