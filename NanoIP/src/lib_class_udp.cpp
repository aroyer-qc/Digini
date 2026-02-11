//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_udp.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2010-2024 Alain Royer.
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
//  UDP - User Datagram Protocol
//
// 	PSEUDO HEADER to calculate Checksum
//
//	_____________________________________________________________
// 	|       |   0 - 7   |    8 - 15    |   16 - 23   |  24- 31   |
//  |_______|__________________________|_________________________|
// 	|   0   |    			  Source Address                     |
//  |_______|____________________________________________________|      Pseudo Header
// 	|  32   |    			Destination Address                  |
//  |_______|____________________________________________________|
// 	|  64   | Zero's   	| Protocol 11h |      UDP Length         |
//  |_______|___________|______________|_________________________| __________________________
// 	|  96   |        Source Port       |    Destination Port     |
//  |_______|__________________________|_________________________|       Real Header
// 	|  128  |          Length          |       Checksum          |
//  |_______|__________________________|_________________________|___________________________
// 	|       |                                                    |
// 	|  160  |                       Data                         |           Data
//  |_______|____________________________________________________|
//
//
//  The UDP Length field is the length of the Pseudo UDP header and Real Header + data
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (IP_USE_UDP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Initializes the UDP subsystem for the associated network context. The
//                  function clears the UDP binding table, resets the count of active bindings,
//                  and reinitializes the ephemeral port allocator to the start of the
//                  configured ephemeral range. No memory allocation occurs. After this call,
//                  the UDP layer is ready to register sockets, allocate ports, and process
//                  incoming datagrams.
//
//-------------------------------------------------------------------------------------------------
void UDP_Protocol::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;
    memset(m_BoundSockets, 0, sizeof(m_BoundSockets));      // Clear the binding table
    m_BoundCount = 0;                                       // Reset the number of active bindings
    m_NextEphemeralPort = UDP_EPHEMERAL_PORT_MIN;           // Reset ephemeral port allocator
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   IP_PacketMsg_t*  pMsg   Pointer to the received packet wrapper containing the
//                                          full Ethernet/IP/UDP frame. Ownership of this message
//                                          remains with the caller until the function either
//                                          delivers it to a socket or discards it.
//
//  Return:         None
//
//  Description:    Handles an incoming UDP datagram. The function parses the IP and UDP
//                  headers, validates the packet length, and extracts the destination port.
//                  Using the UDP binding table, it locates the socket bound to that port. If a
//                  matching socket is found, the packet is enqueued into the socket’s receive
//                  queue for later retrieval via Recv() or RecvFrom(). If no socket is bound
//                  to the destination port, the packet is discarded and its buffers are freed.
//
//                  This function performs no payload copying; it operates entirely on the
//                  zero-copy packet structure. Memory ownership is transferred to the target
//                  socket upon successful queueing. If delivery fails (queue full or no socket
//                  registered), the function frees all associated packet resources.
//
//-------------------------------------------------------------------------------------------------
void UDP_Protocol::Process(IP_PacketMsg_t* pMsg)
{
    IP_Header_t*  pIP  = &pMsg->pPacket->UDP_Frame.IP_Header;
    UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Frame.UDP_Header;

    // Validate IP and UDP lengths BEFORE using them
    size_t IpTotalLength = ntohs(pIP->Length);
    size_t IpHeaderSize  = (pIP->VersionIHL & 0x0F) * 4;
    size_t UDP_Length = ntohs(pUDP->Length);



DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "IP Len=%u, Hdr=%u, UDP Len=%u\n", (uint32_t)IpTotalLength, (uint32_t)IpHeaderSize, (uint32_t)UDP_Length);



    // IP total length must cover IP header + UDP header
    if(IpTotalLength < (IpHeaderSize + sizeof(UDP_Header_t)))
    {
        IP_Manager::FreeMessage(pMsg);                              // Malformed IP packet -> drop
        return;
    }

    //size_t UDP_Length = ntohs(pUDP->Length);

    // UDP length must include header and fit inside IP payload
    if((UDP_Length < sizeof(UDP_Header_t)) || (UDP_Length > (IpTotalLength - IpHeaderSize)))
    {
        IP_Manager::FreeMessage(pMsg);                              // Malformed UDP packet -> drop
        return;
    }

    // Compute payload pointer and size (now guaranteed safe)
    size_t PayloadLength = UDP_Length - sizeof(UDP_Header_t);

    pMsg->Payload     = (uint8_t*)(pUDP + 1);
    pMsg->PayloadSize = PayloadLength;

    // Socket lookup
    IP_Port_t DstPort = ntohs(pUDP->DstPort);
    Socket*   pSock   = FindSocketByPort(DstPort);                  // Your lookup function

    if((pSock == nullptr) || (pSock->GetActive() == false))
    {
        IP_Manager::FreeMessage(pMsg);                              // No socket bound -> drop
        return;
    }

    UDP_Socket_t* pUDP_Sock = pSock->GetUDP();

    // Enqueue packet for this socket
    if(nOS_QueueWrite(&pUDP_Sock->RX_Queue, &pMsg, 0) != NOS_OK)
    {
        IP_Manager::FreeMessage(pMsg);                              // Queue full -> drop
        return;
    }

    // Debug guard: pointer must be in valid SRAM range
    if(((uint32_t)pMsg < 0x20000000) || ((uint32_t)pMsg > 0x20020000))
    {
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "CORRUPTED BEFORE ENQUEUE: %08X\n", (uint32_t)pMsg);
        while(1);
    }

    // Ownership now belongs to the socket. Do NOT free here.
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameter(s):   UDP_Socket_t*           pUdp        Pointer to the UDP socket instance owning
//                                                      the local port and RX/TX state.
//                  uint8_t*                pData       Pointer to the payload buffer to transmit.
//                  size_t                  Length      Number of payload bytes to send.
//                  const SocketInfo_t*     pDestInfo   Destination addressing information
//                                                      (IP address, UDP port, MAC address)
//                  size_t*                 pBytesSent  Output: number of payload bytes
//                                                      successfully queued for transmission.
//
//  Return:         SystemState_e           SYS_READY                   on success.
//                                          SYS_FAIL_MEMORY_ALLOCATION  if packet buffers cannot be
//                                                                      allocated.
//                                          SYS_FAIL                    if the interface TX
//                                                                      callback rejects the
//                                                                      packet.
//
//
//  Description:    Builds and transmits a UDP datagram using the specified socket context.
//                  The function allocates a packet wrapper and Ethernet/IP/UDP frame buffer,
//                  constructs the UDP header and copies the payload, then delegates IP header
//                  construction to the IP layer (IP_Manager). The fully assembled packet is
//                  handed to the active network interface via NetworkContext::SendPacket().
//
//                  On successful queueing, ownership of the packet is transferred to the
//                  interface TX engine, which is responsible for freeing the packet after
//                  transmission completes. No memory is freed by this function on success.
//                  On failure, all allocated resources are released before returning.
//
//-------------------------------------------------------------------------------------------------
SystemState_e UDP_Protocol::Send(UDP_Socket_t* pUdp, uint8_t* pData, size_t Length, const SocketInfo_t* pDestInfo, size_t* pBytesSent)
{
    *pBytesSent = 0;

    size_t PacketSize = sizeof(UDP_Frame_t) + Length;                   // Compute total packet size (UDP header + payload)

    // Allocate wrapper + packet buffer using the new helper
    IP_PacketMsg_t* pMsg = nullptr;
    SystemState_e State  = m_pContext->GetIP_Manager()->AllocPacket(&pMsg, PacketSize, MEM_DBG_UDP, MEM_DBG_UDPDT);

    if(State != SYS_READY)
    {
        return State;
    }

    pMsg->PacketSize = PacketSize;

    // Build UDP header
    UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Frame.UDP_Header;
    pUDP->SrcPort = htons(pUdp->LocalPort);
    pUDP->DstPort = htons(pDestInfo->Port);
    uint16_t UDP_Length = sizeof(UDP_Header_t) + Length;
    pUDP->Length  = htons(UDP_Length);

    // Copy payload
    uint8_t* pPayload = (uint8_t*)(pUDP + 1);
    memcpy(pPayload, pData, Length);
    IP_Manager* pIP_Manager = m_pContext->GetIP_Manager();
    pIP_Manager->PutHeader(pMsg, pDestInfo->Address, UDP_Length, IP_PROTOCOL_UDP);          // Build IP header via IP_Manager
    pUDP->Checksum = pIP_Manager->UDP_CalculateChecksum(&pMsg->pPacket->IP_Frame.Header, pUDP, UDP_Length);
    State = pIP_Manager->SendPacket(pMsg);

    if(State == SYS_READY)
    {
        *pBytesSent = Length;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindSocketByPort
//
//  Parameter(s):   IP_Port_t   Port    Destination UDP port extracted from the incoming
//                                      datagram.
//
//  Return:         UDP_Socket_t*       Pointer to the UDP socket bound to the specified port.
//                                      Returns nullptr if no socket is registered for this port.
//
//  Description:    Looks up the UDP binding table to locate the socket associated with the
//                  specified destination port. This function is used by the UDP receive
//                  dispatcher to deliver incoming datagrams to the correct socket instance.
//                  If no socket is bound to the port, the function returns nullptr, and the
//                  caller is responsible for discarding the packet or generating an ICMP
//                  Port Unreachable message (if enabled).
//
//-------------------------------------------------------------------------------------------------
Socket* UDP_Protocol::FindSocketByPort(IP_Port_t port)
{
    for(size_t i = 0; i < m_BoundCount; i++)
    {
        if(m_BoundSockets[i].Port == port)
        {
            return m_BoundSockets[i].pSocket;
        }
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterSocket
//
//  Parameter(s):   IP_Port_t       Port    Local UDP port to associate with the socket.
//                  UDP_Socket_t*   pUdp    Pointer to the UDP socket instance to register.
//
//  Return:         SystemState_e   SYS_READY            – Port successfully registered.
//                                  SYS_FAIL_PORT_IN_USE – Port already bound by another socket.
//
//  Description:    Registers a UDP socket in the port binding table. Once registered, incoming
//                  datagrams addressed to the specified port will be delivered to the socket’s
//                  receive queue. The function enforces exclusive ownership of each port and
//                  prevents multiple sockets from binding the same port.
//
//-------------------------------------------------------------------------------------------------
bool UDP_Protocol::RegisterSocket(Socket* pSock, IP_Port_t Port)
{
    for(size_t i = 0; i < m_BoundCount; i++)                // Check if already bound
    {
        if(m_BoundSockets[i].Port == Port)
        {
            return false;                                   // Port already in use
        }
    }

    if(m_BoundCount >= UDP_MAX_BINDS)                       // Check capacity
    {
        return false;                                       // No room left
    }

    // Add new entry
    m_BoundSockets[m_BoundCount].Port    = Port;
    m_BoundSockets[m_BoundCount].pSocket = pSock;
    m_BoundCount++;
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UnregisterSocket
//
//  Parameter(s):   IP_Port_t   Port        Local UDP port to release.
//
//  Return:         None
//
//  Description:    Removes the specified port from the UDP binding table. After unregistration,
//                  incoming datagrams addressed to this port will no longer be delivered to
//                  the socket. This function is typically invoked during socket closure or when
//                  rebinding to a new port.
//
//-------------------------------------------------------------------------------------------------
void UDP_Protocol::UnregisterSocket(IP_Port_t Port)
{
    for(size_t i = 0; i < m_BoundCount; i++)
    {
        if(m_BoundSockets[i].Port == Port)
        {
            m_BoundSockets[i] = m_BoundSockets[m_BoundCount - 1];

            // Optional: clear last entry for debugging clarity
            m_BoundSockets[m_BoundCount - 1].Port    = 0;
            m_BoundSockets[m_BoundCount - 1].pSocket = nullptr;

            m_BoundCount--;
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AllocateEphemeralPort
//
//  Parameter(s):   None
//
//  Return:         IP_Port_t   A free ephemeral UDP port in the configured ephemeral range.
//                              Returns 0 if no ports are available.
//
//  Description:    Searches the UDP binding table for an unused port within the ephemeral
//                  port range. The function iterates through the configured range and returns
//                  the first unbound port. If all ephemeral ports are currently in use, the
//                  function returns 0 to indicate failure. This helper is used by the socket
//                  layer when a UDP socket is bound with port = 0, allowing automatic,
//                  conflict-free port assignment.
//
//-------------------------------------------------------------------------------------------------
IP_Port_t UDP_Protocol::AllocateEphemeralPort(void)
{
    IP_Port_t start = m_NextEphemeralPort;

    while(1)
    {
        // Wrap around if needed
        if(m_NextEphemeralPort > UDP_EPHEMERAL_PORT_MAX)
        {
            m_NextEphemeralPort = UDP_EPHEMERAL_PORT_MIN;
        }

        IP_Port_t Candidate = m_NextEphemeralPort;
        m_NextEphemeralPort++;

        bool inUse = false;

        for(size_t i = 0; i < m_BoundCount; i++)            // Check if already bound
        {
            if(m_BoundSockets[i].Port == Candidate)
            {
                inUse = true;
                break;
            }
        }

        if(inUse == false)
        {
            return Candidate;
        }

        if(m_NextEphemeralPort == start)                    // Full cycle → no free port
        {
            return 0;                                       // 0 = failure
        }
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_UDP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
