//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_IP_Manager.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2009-2024 Alain Royer.
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

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define IP_ASCII_ADDRESS_SIZE               16
#define IP_EPHEMERAL_PORT_MIN               49152
#define IP_EPHEMERAL_PORT_MAX               65535

//-------------------------------------------------------------------------------------------------
//
//  Name:           TaskIP_Manager_Wrapper
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    main() for the IP_Manager
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" void TaskIP_Manager_Wrapper(void* pvParameters)
{
    (static_cast<IP_Manager*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext& pContext            Reference on the context
//  Return:         void
//
//  Description:    Initialize IP Task
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::Initialize(NetworkContext* pContext)
{
    nOS_Error Error;

    m_pContext          = pContext;
    m_SequenceID        = RNG_GetRandom();

    const char* Host = m_pContext->GetHostName();
    size_t HostLen = strlen(Host);
    strncat(m_ThreadName, Host, (HostLen < TASK_IP_MANAGER_TREAD_NAME_EXTRACT_SIZE) ? HostLen : TASK_IP_MANAGER_TREAD_NAME_EXTRACT_SIZE);   // Copy up to max characters allowed safely

    Error = nOS_ThreadCreate(&m_Handle,
                             TaskIP_Manager_Wrapper,
                             this,
                             &m_Stack[0],
                             TASK_IP_MANAGER_STACK_SIZE,
                             TASK_IP_MANAGER_PRIO,
                             m_ThreadName);

    VAR_UNUSED(Error);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Run
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    This function will poll the IP stack and dispatch message to service
//
//  Note(s):        Will initialize the stack
//                  this task is also the task start point
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::Run(void)
{
    IP_PacketMsg_t* pMsg;
    DHCPv4_Manager& DHCP = m_pContext->GetDHCP();

  #if (IP_USE_SNTP == DEF_ENABLED)
    //IP_Address_t    IP;
    //IP_Error_e      Error;
  #endif

    for(;;)
    {
      #if (IP_USE_DHCP == DEF_ENABLED)
        if(m_pContext->GetLinkChange() == true)                            // Always react to link changes, regardless of DHCP enable state
        {
            m_pContext->SetLinkChange(false);

            if(m_pContext->GetLinkState() == ETH_LINK_UP)
            {
                if(m_pContext->IsDHCP_Enable())
                {
                    DHCP.Start();
                }
                else
                {
                    DHCP.Reset();                                   // Ensure no stale DHCP state
                }
            }
            else // Link down
            {
                DHCP.Reset();
            }
        }

        if((DHCP.IsItEnabled() == true) && (m_pContext->GetLinkState() == ETH_LINK_UP))    // Run DHCP state machine only when enabled AND link is up
        {
            (void)DHCP.Process();
        }
      #endif

    #if (IP_USE_DNS == DEF_ENABLED)
        m_pContext->GetDNS().Process();
    #endif

    #if (IP_USE_TCP_CLIENT == DEF_ENABLED) || (IP_USE_TCP_SERVER == DEF_ENABLED)
        m_pContext->GetTCP()->Process();
    #endif

        if(nOS_QueueRead(m_pContext->GetMsgQ(), (void**)&pMsg, NOS_WAIT_INFINITE) == NOS_OK)
        {
            if(pMsg->PacketSize < sizeof(IP_EthernetHeader_t))                                  // Basic Ethernet header size check  peut-etre pas necessaire avec le default
            {
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: Frame too small for Ethernet header (%u bytes)\n",  pMsg->PacketSize);
                FreeMessage(pMsg);
                continue;
            }

            switch(pMsg->pPacket->ETH_Header.Type)
            {
                case IP_ETHERNET_TYPE_IPV4:
                {
                    if(pMsg->PacketSize < (sizeof(IP_EthernetHeader_t) + sizeof(IP_Header_t)))  // Check minimum size for IPv4 header
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: IPv4 frame too small (%u bytes)\n", pMsg->PacketSize);
                        FreeMessage(pMsg);
                        break;
                    }

                    uint8_t ihl = pMsg->pPacket->IP_Frame.Header.VersionIHL & 0x0F;             // Validate IHL (Internet Header Length)

                    if(ihl < 5)
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: Invalid IHL (%u)\n", ihl);
                        FreeMessage(pMsg);
                        break;
                    }

                    uint16_t ipHeaderSize = ihl * 4;
                    uint16_t totalLength = ntohs(pMsg->pPacket->IP_Frame.Header.Length);        // Validate total IP length

                    if(totalLength < ipHeaderSize)
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: totalLength < ipHeaderSize (%u < %u)\n", totalLength, ipHeaderSize);
                        FreeMessage(pMsg);
                        break;
                    }

                    if(pMsg->PacketSize < (sizeof(IP_EthernetHeader_t) + totalLength))              // Ensure the received frame contains the full IP packet
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: Truncated IPv4 frame (%u < %u)\n", pMsg->PacketSize, sizeof(IP_EthernetHeader_t) + totalLength);
                        FreeMessage(pMsg);
                        break;
                    }

                    //DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH type: IPV4\n");
                    pMsg->Protocol = pMsg->pPacket->IP_Frame.Header.Protocol;
                    m_pContext->GetARP().ProcessIP(pMsg);                                           // May update ARP cache, does NOT own pMsg
                    ProcessIP(pMsg);                                                                // Transfers ownership to protocol/socket
                }
                break;

                case IP_ETHERNET_TYPE_ARP:
                {
                  #if (IP_DBG_ARP == DEF_ENABLED)
                    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH type: ARP\n");
                  #endif

                    m_pContext->GetARP().ProcessARP(pMsg);                                          // ARP owns and frees pMsg
                }
                break;

                default:
                {
                    //DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH_Default: Type:0x%04X\n", pMsg->pPacket->ETH_Header.Type);
                    FreeMessage(pMsg);
                }
                break;
            }
        }

        nOS_Sleep(1);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ProcessIP
//
//  Parameter(s):   IP_PacketMsg_t* pMsg
//  Return:         void
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::ProcessIP(IP_PacketMsg_t* pMsg)
{
    switch(pMsg->Protocol)
    {
      #if (IP_USE_ICMP == DEF_ENABLED)
        case IP_PROTOCOL_ICMP:
        {
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "Ethernet IP-ICMP\n");
            m_pContext->GetICMP().Process(pMsg);    // ICMP owns + frees
        }
        break;
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        case IP_PROTOCOL_RAW:
        {
            m_RAW.Process(pMsg);
        }
        break;
      #endif

     #if (IP_USE_TCP_CLIENT == DEF_ENABLED) || (IP_USE_TCP_SERVER == DEF_ENABLED)
        case IP_PROTOCOL_TCP:
        {
            m_pContext->GetTCP()->ProcessSegment(pMsg);
        }
        break;
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        case IP_PROTOCOL_UDP:
        {
            //DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "Ethernet IP-UDP\n");
            m_pContext->GetUDP().Process(pMsg);
        }
        break;
      #endif

        default:
        {
            //DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "Ethernet IP N/U: Type:0x%02X\n", pMsg->Protocol);
            FreeMessage(pMsg);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendPacket
//
//  Parameter(s):   IP_PacketMsg_t* pMsg    Pointer to a fully constructed IP packet message. The
//                                          packet must contain valid Ethernet, IP, and
//                                          transport-layer headers. Ownership of the message
//                                          remains with the caller unless the interface driver
//                                          accepts it.
//
//  Return:         SystemState_e
//                      SYS_READY               – Packet was successfully handed off to the
//                                                network interface for transmission.
//                      SYS_ARP_RESOLVE_PENDING – Destination MAC address is not yet known.
//                                                The caller must retry once ARP resolution
//                                                completes.
//                      Other driver-specific error codes may be returned by the interface
//                      context.
//
//  Description:    Prepares an outgoing IP packet for transmission by resolving the destination
//                  MAC address and forwarding the packet to the active network interface.
//
//                  If the destination IP address is the broadcast address (255.255.255.255),
//                  the Ethernet destination MAC is set to FF:FF:FF:FF:FF:FF.
//
//                  For unicast destinations, the function attempts to resolve the MAC address
//                  through the ARP module. If ARP resolution is not yet available, the function
//                  returns SYS_ARP_RESOLVE_PENDING and the caller is responsible for retrying.
//
//                  Once the destination MAC is known, the packet is handed off to the interface
//                  context’s SendPacket() callback for actual transmission on the wire.
//
//-------------------------------------------------------------------------------------------------
SystemState_e IP_Manager::SendPacket(IP_PacketMsg_t* pMsg)
{
    IP_EthernetHeader_t* pETH = &pMsg->pPacket->ETH_Header;
    IP_Header_t*         pIP  = &pMsg->pPacket->IP_Frame.Header;
    IP_Address_t         dstIP = pIP->DstIP_Address;

    if(dstIP == IP_ADDRESS(255,255,255,255))                                            // Broadcast: 255.255.255.255 -> FF:FF:FF:FF:FF:FF
    {
        memset(pETH->DestinationMAC.Byte, 0xFF, IP_MAC_ADDRESS_SIZE);
    }
    else
    {
         if(m_pContext->GetARP().Resolve(dstIP, &pETH->DestinationMAC, pMsg) == false)  // Unicast -> resolve via ARP and store packet for later transmission
        {
            return SYS_ARP_RESOLVE_PENDING;                                             // ARP not ready -> caller decides what to do
        }
    }

    return m_pContext->SendPacket(pMsg);                                                // Hand off to interface context (driver callback)
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
IP_Port_t IP_Manager::AllocateEphemeralPort(void)
{
    IP_Port_t Candidate = RNG_GetRandomFromRange(IP_EPHEMERAL_PORT_MIN, IP_EPHEMERAL_PORT_MAX);

    // implement in future a check for already allocated port
    return Candidate;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_ToAscii
//
//  Parameter(s):   char*               Pointer to return formatted string
//                  IP_Address_t        IP_Address
//  Return:         void
//
//  Description:    Put IP in a string following standard format EX. 192.168.1.100
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::IP_ToAscii(char* pBuffer, IP_Address_t IP_Address)
{
    snprintf(pBuffer, IP_ASCII_ADDRESS_SIZE, "%d.%d.%d.%d        ", IP_A(IP_Address),
                                                                    IP_B(IP_Address),
                                                                    IP_C(IP_Address),
                                                                    IP_D(IP_Address));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AsciiToIP
//
//  Parameter(s):   uint8_t*        pBuffer
//  Return:         IP_Address_t
//
//  Description:    Extract the IP form a string
//
//  Note(s):        Length is check and also number of dot, to confirm it is an IP
//
//-------------------------------------------------------------------------------------------------
IP_Address_t IP_Manager::AsciiToIP(const char* pBuffer)
{
    if(pBuffer == nullptr)
    {
        return IP_ADDRESS(0,0,0,0);
    }

    uint8_t octets[4] = {0};
    uint8_t index     = 0;
    uint16_t value    = 0;

    while(*pBuffer != '\0')
    {
        if(*pBuffer >= '0' && *pBuffer <= '9')
        {
            value = value * 10 + (*pBuffer - '0');

            if(value > 255)
            {
                return IP_ADDRESS(0,0,0,0);
            }
        }
        else if(*pBuffer == '.')
        {
            if(index >= 4)
            {
                return IP_ADDRESS(0,0,0,0);
            }

            octets[index++] = (uint8_t)value;
            value = 0;
        }
        else
        {
            return IP_ADDRESS(0,0,0,0);
        }

        pBuffer++;
    }

    if(index != 3)
    {
        return IP_ADDRESS(0,0,0,0);
    }

    octets[3] = (uint8_t)value;

    return IP_ADDRESS(octets[0],octets[1],octets[2],octets[3]);         // TODO validate
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ProcessURL
//
//  Parameter(s):   char*         pBuffer
//                  IP_Address_t* pIP
//                  IP_Port_t     pPort
//  Return:         char*         pURI
//
//  Description:    This function will process an URL
//                      -   Extract Port number if any ( return 80 otherwise )
//                      -   Get the IP from DNS
//                      -   Return pointer o the URI
//
//  Note(s):        It is assume that "http://" is always lowercase
//
//-------------------------------------------------------------------------------------------------
char* IP_Manager::ProcessURL(char* pBuffer, IP_Address_t* pIP, IP_Port_t* pPort)
{
    char*   pDomainName;
    char*   pSearch1        = nullptr;
    char*   pSearch2        = nullptr;
    bool    IP_Status;

    *pPort = 80;                                        // Set to default port if none are found

    // Get Domain Name or IP ......................................................................
    pSearch1 = strstr(pBuffer, "http://");              // Remove the unused "http://"

    if(pSearch1 == pBuffer)
    {
        strcpy(pBuffer, pBuffer + 7);                   // Make the correction in the buffer
    }

    pDomainName = pBuffer;                              // Found the beginning of the domain name or IP

    // Get port number if any .....................................................................
    pSearch1 = strchr(pBuffer, ':');                    // Search for a port number  looking at the semicolon :

    if(pSearch1 != nullptr)                             // not nullptr then extract port number
    {
        *pSearch1 = '\0';                               // Put nullptr at the : position for nullptr terminated string
        pSearch1++;
        IP_Status = false;

        do
        {
            if((*pSearch1 >= '0') && (*pSearch1 <= '9'))
            {
                if(IP_Status == false)
                {
                    *pPort  = 0;                        // Trap first occurence
                    IP_Status = true;
                }
                else
                {
                    *pPort *= 10;                       // Other Must be multiply  10
                }

                *pPort += (*pSearch1 - '0');
                pSearch1++;
            }
            else
            {
                IP_Status = false;
            }
        }
        while(IP_Status == true);
    }
    else
    {
        pSearch1 = pBuffer;                             // Replace search pointer if no port found
    }

    // Get URI pointer if any .....................................................................
    pSearch2 = pSearch1;
    pSearch1 = strchr(pSearch1, '/');                   // Search for separator beginning of URI

    if(pSearch1 != nullptr)
    {
        *pSearch1 = '\0';                               // Put nullptr at the / position for nullptr terminated 'Domain Name' string
        pSearch1++;                                     // Place pointer where URI start minus first '/'

        pSearch2 = pSearch1;
    }

    // Get the space at the end ..................................................................
    pSearch2 = strchr(pSearch2, ' ');

    if(pSearch2 != nullptr)
    {
        *pSearch2 = '\0';                               // Put nullptr at the 'SPACE' for a nullptr terminated string
    }

    // Scan to found if it is an IP or "Domain Name" .............................................

    *pIP = AsciiToIP(pDomainName);

    if(*pIP == IP_ADDRESS(0,0,0,0))
    {
 //       *pIP = pDNS->Query(IP_DNS_SOCKET, pDomainName, &Error);
    }

    return pSearch1;
}

//-------------------------------------------------------------------------------------------------
//  Name:           AllocPacket
//
//  Parameter(s):   ppMsg           Output pointer that receives the allocated IP_PacketMsg_t
//                                  wrapper.
//                  PacketSize      Size of the protocol-specific packet buffer to allocate.
//                  DebugWrapperID  Debug ID used for the wrapper allocation.
//                  DebugPacketID   Debug ID used for the packet buffer allocation.
//
//  Return:         SystemState_e
//                      SYS_READY                   Allocation successful.
//                      SYS_INVALID_PARAMETER       ppMsg is null.
//                      SYS_FAIL_MEMORY_ALLOCATION  One of the allocations failed.
//
//  Description:    Allocates a transmit packet consisting of:
//
//                      1) An IP_PacketMsg_t wrapper
//                      2) A protocol-specific packet buffer (Ethernet + protocol frame)
//
//                  Both buffers are cleared and tagged with the provided debug IDs. If the packet
//                  buffer allocation fails, the wrapper is automatically freed.
//
//                  This helper centralizes the common allocation pattern used by ARP, UDP, DHCP,
//                  DNS, ICMP, SNTP, and TCP transmit paths.
//
//  Note(s):        - Caller must check the returned SystemState_e before using *ppMsg.
//                  - On success, *ppMsg is guaranteed to be valid and ready for header construction.
//-------------------------------------------------------------------------------------------------
SystemState_e IP_Manager::AllocPacket(IP_PacketMsg_t** ppMsg, size_t PacketSize, MEM_DebugListOfID_e DebugWrapperID, MEM_DebugListOfID_e DebugPacketID)
{
    if(ppMsg == nullptr)                                                                                        // Validate output pointer
    {
        return SYS_INVALID_PARAMETER;
    }

    *ppMsg = nullptr;
    IP_PacketMsg_t* pMsg = (IP_PacketMsg_t*)pMemoryPool->AllocAndClear(sizeof(IP_PacketMsg_t), DebugWrapperID); // Allocate wrapper

    if(pMsg == nullptr)
    {
        return SYS_FAIL_MEMORY_ALLOCATION;
    }

    pMsg->pPacket = (IP_EthernetPacket_t*)pMemoryPool->AllocAndClear(PacketSize, DebugPacketID);                // Allocate packet buffer

    if(pMsg->pPacket == nullptr)
    {
        pMemoryPool->Free((void**)&pMsg);
        return SYS_FAIL_MEMORY_ALLOCATION;
    }

    *ppMsg = pMsg;                                                                                              // Success
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PutHeader
//
//  Parameter(s):   IP_PacketMsg_t*     pMsg            Pointer to the packet wrapper containing
//                                                      the Ethernet/IP/UDP/TCP frame. The IP
//                                                      header will be written directly into this
//                                                      buffer.
//                  IP_Address_t        dstIP           Destination IPv4 address to place in the IP
//                                                      header.
//                  uint16_t            payloadLength   Length of the transport-layer payload
//                                                      (UDP/TCP header + data).
//                  uint8_t             protocol        Transport protocol identifier
//                                                      (e.g., IP_PROTOCOL_UDP, IP_PROTOCOL_TCP).
//
//  Return:         None
//
//  Description:    Constructs the IPv4 header for an outgoing packet. The function writes the
//                  Ethernet header (source MAC, EtherType), fills all IPv4 header fields
//                  (version, IHL, total length, identification, flags, TTL, protocol, source
//                  and destination addresses), and computes the IPv4 header checksum.
//
//                  The function uses the interface configuration stored in NetworkContext
//                  (MAC address, MTU, IP settings) and the internal sequence counter for the
//                  IP identification field. No memory allocation occurs; the header is written
//                  directly into the caller-provided packet buffer.
//
//                  After this function completes, the packet is fully assembled at the
//                  Ethernet/IP level and ready for transmission via NetworkContext::SendPacket().
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::PutHeader(IP_PacketMsg_t* pTX, IP_Address_t DstIP, uint16_t PayloadLength, uint8_t Protocol)
{
    IP_Header_t*         pIP  = &pTX->pPacket->IP_Frame.Header;
    IP_EthernetHeader_t* pETH = &pTX->pPacket->ETH_Header;

    // Ethernet header
    IP_MAC_Address_t MacAddress;
    m_pContext->GetMAC_Address(&MacAddress);
    memcpy(&pETH->SourceMAC.Byte[0], &MacAddress.Byte[0], IP_MAC_ADDRESS_SIZE);
    pETH->Type = IP_ETHERNET_TYPE_IPV4;

    // IPv4 header
    pIP->VersionIHL          = IP_VERSION4_IHL20;
    pIP->TypeOfService       = 0;
    pIP->Length              = htons(sizeof(IP_Header_t) + PayloadLength);
    pIP->FlagsFragmentOffset = htons(0);
    pIP->TimeToLive          = IP_TIME_TO_LIVE;
    pIP->Protocol            = Protocol;
    pIP->SrcIP_Address       = m_pContext->GetActiveIP();
    pIP->DstIP_Address       = DstIP;

    if(Protocol != IP_PROTOCOL_ICMP)
    {
        pIP->ID = htons(m_SequenceID++);
    }

    pIP->Checksum = 0;
    pIP->Checksum = LIB_HTONS_Checksum16((uint8_t*)pIP, sizeof(IP_Header_t));
}

//-------------------------------------------------------------------------------------------------

uint16_t IP_Manager::CalculateChecksum(IP_Header_t* pIP, uint8_t Protocol, void* pProtocolHeader, uint16_t Length)
{
    uint32_t Sum = 0;

    // PASS 1 : PSEUDO-HEADER
    PseudoHeader_t PseudoHeader;
    PseudoHeader.SrcIP = pIP->SrcIP_Address;
    PseudoHeader.DstIP = pIP->DstIP_Address;
    PseudoHeader.Zero      = 0;
    PseudoHeader.Protocol  = Protocol;
    PseudoHeader.Length    = htons(Length);
    Sum = uint16_t(~LIB_Checksum16((uint8_t*)&PseudoHeader, sizeof(PseudoHeader_t)));

    // PASS 2 : HEADER + PAYLOAD
    Sum += uint16_t(~LIB_Checksum16((uint8_t*)pProtocolHeader, Length));

    // Fold 32 -> 16 bits
    while(Sum >> 16)
    {
        Sum = (Sum & 0xFFFF) + (Sum >> 16);
    }

    return htons((uint16_t)(~Sum));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FreeMessage
//
//  Parameter(s):   IP_PacketMsg_t* pMsg     Pointer to a message wrapper containing a packet
//                                           buffer and associated metadata.
//
//  Return:         void
//
//  Description:    Releases all resources associated with a packet message. This static utility
//                  function frees the packet buffer (if allocated) and then frees the message
//                  wrapper itself. Because the freeing logic does not depend on any IP_Manager
//                  instance state, it is provided as a static function so that all protocol
//                  layers can invoke it directly.
//
//  Note(s):        - Safe to call with a null pointer.
//                  - Centralizes message destruction for the entire network stack.
//                  - Ensures consistent zero-copy buffer ownership release.
//                  - Intended for use by all components that dequeue or discard messages.
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::FreeMessage(IP_PacketMsg_t* pMsg)
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

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
