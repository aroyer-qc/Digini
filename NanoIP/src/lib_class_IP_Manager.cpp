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

#define LIB_IP_MANAGER_GLOBAL
#include "./lib_digini.h"
#define LIB_IP_MANAGER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define IP_ASCII_ADDRESS_SIZE               16

//-------------------------------------------------------------------------------------------------
// Stack(s)
//-------------------------------------------------------------------------------------------------

IF_ETH_DEF(EXPAND_X_IF_AS_STACK_DECLARATION)

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const IP_Config_t IP_Manager::m_Config[IP_NUMBER_OF_INTERFACE] =
{
    IF_ETH_DEF(EXPAND_X_IF_AS_STRUCT_DATA)
};

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
//  Parameter(s):   IF_ID_e      IF_ID      ID of the IF interface configuration data
//  Return:         void
//
//  Description:    Initialize IP Task and stack
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::Initialize(IF_ID_e IF_ID)
{
    nOS_Error Error;

    m_Context.SetIP_Manager(this);
    m_SocketManager.Initialize(&m_Context);                                             // Initialize socket manager

  #if (IP_USE_DHCP == DEF_ENABLED)
    m_Context.SetDHCP_Enable(true);
  #endif

    // Initialize Variables
    //m_DNS_IP_Found = false;  not used so far
    m_Context.InitializeMsgQ();                                                         // this need to handle error
    m_Context.SetMAC_Address(&m_Config[IF_ID].IP_ETH_Config.MAC_Address);
    m_Context.SetHostName(m_Config[IF_ID].pHostName);
    m_Context.SetMTU(IP_NET_IF_MTU);                                                    // Set netif maximum transfer unit
    m_Context.SetStaticIP(m_Config[IF_ID].DefaultStatic_IP);
    m_Context.SetStaticGatewayIP(m_Config[IF_ID].DefaultGateway);
    m_Context.SetStaticSubnetMask(m_Config[IF_ID].DefaultSubnetMask);
    m_Context.SetStaticDNS_IP(m_Config[IF_ID].DefaultStaticDNS);
    m_Context.SetIP_Valid((m_Config[IF_ID].DefaultStatic_IP == IP_ADDRESS(255,255,255,255)) ? false : true);
    m_IF_Driver.Initialize(&m_Config[IF_ID].IP_ETH_Config, &m_Context);
    m_Context.RegisterSendCallback(&m_IF_Driver.LowLevelOutputWrapper, &m_IF_Driver);

    // All protocol support are created dynamically if interface is set to use it, and if configuration is enable for that protocol

  #if (IP_USE_UDP == DEF_ENABLED)
    m_UDP.Initialize(&m_Context);
  #endif

  #if (IP_USE_DHCP == DEF_ENABLED)
    m_DHCP.Initialize(&m_Context);
  #endif

    m_ARP.Initialize(&m_Context);

  #if (IP_USE_DNS == DEF_ENABLED)
    m_DNS.Initialize(&m_Context);
  #endif

  #if (IP_USE_ICMP == DEF_ENABLED)
    m_ICMP.Initialize(&m_Context);
  #endif

  #if (IP_USE_TCP == DEF_ENABLED)
    m_TCP.Initialize(&m_Context);
  #endif

   #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
    myStacktistic.Register(m_Config[IF_ID].pStack, TASK_IP_MANAGER_STACK_SIZE, m_Config[IF_ID].pHostName);
  #endif

    Error = nOS_ThreadCreate(&m_Handle,
                             TaskIP_Manager_Wrapper,
                             this,
                             m_Config[IF_ID].pStack,
                             TASK_IP_MANAGER_STACK_SIZE,
                             TASK_IP_MANAGER_PRIO);

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

  #if (IP_USE_SNTP == DEF_ENABLED)
    //IP_Address_t    IP;
    //IP_Error_e      Error;
  #endif

    for(;;)
    {
      #if (IP_USE_DHCP == DEF_ENABLED)
        if(m_Context.GetLinkChange() == true)                                                   // Always react to link changes, regardless of DHCP enable state
        {
            m_Context.SetLinkChange(false);

            if(m_Context.GetLinkState() == ETH_LINK_UP)
            {
                if(m_Context.IsDHCP_Enable())
                {
                    m_DHCP.Start();
                }
                else
                {
                    m_DHCP.Reset();                                                             // Ensure no stale DHCP state
                }
            }
            else // Link down
            {
                m_DHCP.Reset();
            }
        }

        if((m_Context.IsDHCP_Enable() == true) && (m_Context.GetLinkState() == ETH_LINK_UP))    // Run DHCP state machine only when enabled AND link is up
        {
            (void)m_DHCP.Process();
        }
      #endif

    #if (IP_USE_DNS == DEF_ENABLED)
        m_DNS.Process();
    #endif

        if(nOS_QueueRead(m_Context.GetMsgQ(), (void**)&pMsg, NOS_WAIT_INFINITE) == NOS_OK)
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

                    if(pMsg->PacketSize < (sizeof(IP_EthernetHeader_t) + totalLength))          // Ensure the received frame contains the full IP packet
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: Truncated IPv4 frame (%u < %u)\n", pMsg->PacketSize, sizeof(IP_EthernetHeader_t) + totalLength);
                        FreeMessage(pMsg);
                        break;
                    }

                    //DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH type: IPV4\n");
                    m_ARP.ProcessIP(pMsg);                                                      // May update ARP cache, does NOT own pMsg
                    ProcessIP(pMsg);                                                            // Transfers ownership to protocol/socket
                }
                break;

                case IP_ETHERNET_TYPE_ARP:
                {
                    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH type: ARP\n");
                    m_ARP.ProcessARP(pMsg);                                                     // ARP owns and frees pMsg
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
    switch(pMsg->pPacket->IP_Frame.Header.Protocol)
    {
      #if (IP_USE_ICMP == DEF_ENABLED)
        case IP_PROTOCOL_ICMP:
        {
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "Ethernet IP-ICMP\n");
            m_ICMP.Process(pMsg);    // ICMP owns + frees
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

      #if (IP_USE_TCP == DEF_ENABLED)
        case IP_PROTOCOL_TCP:
        {
            m_TCP.Process(pMsg);
        }
        break;
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        case IP_PROTOCOL_UDP:
        {
            //DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "Ethernet IP-UDP\n");
            m_UDP.Process(pMsg);
        }
        break;
      #endif

        default:
        {
            //DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "Ethernet IP N/U: Type:0x%02X\n", pMsg->pPacket->IP_Frame.Header.Protocol);
            FreeMessage(pMsg);
        }
        break;
    }
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

    if(dstIP == IP_ADDRESS(255,255,255,255))                            // Broadcast: 255.255.255.255 -> FF:FF:FF:FF:FF:FF
    {
        memset(pETH->DestinationMAC.Byte, 0xFF, IP_MAC_ADDRESS_SIZE);
    }
    else
    {
        if(m_ARP.Resolve(dstIP, &pETH->DestinationMAC, pMsg) == false)  // Unicast -> resolve via ARP and store packet for later transmission
        {
            return SYS_ARP_RESOLVE_PENDING;                             // ARP not ready -> caller decides what to do
        }
    }

    return m_Context.SendPacket(pMsg);                                  // Hand off to interface context (driver callback)
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetHost
//
//  Parameter(s):   void
//  Return:         IP_Address_t   Host IP
//
//  Description:    Return host IP address according to configuration
//
//-------------------------------------------------------------------------------------------------
IP_Address_t IP_Manager::GetHost(void)
{
    return m_Context.GetActiveIP();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetDNS
//
//  Parameter(s):   void
//  Return:         IP_Address_t   IP
//
//  Description:    Return DNS server IP address according to configuration
//
//-------------------------------------------------------------------------------------------------
IP_Address_t IP_Manager::GetDNS(void)
{
    return m_Context.GetActiveDNS_IP();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RequestDNS
//
//  Parameter(s):   const char*     pHostName       Null-terminated domain name to resolve.
//                  DNS_Callback_t  pCallback       Application-provided function invoked when the
//                                                  DNS resolution completes. The callback is
//                                                  executed asynchronously from within the DNS
//                                                  client's processing context.
//
//  Return:         bool
//                      - true  : DNS request accepted by the DNS client.
//                      - false : DNS client could not queue the request (e.g., no free slot).
//
//  Description:    Submits an asynchronous DNS resolution request. This function does not perform
//                  any network activity directly; it simply forwards the request to the DNS client,
//                  which manages its own socket, pending-request table, timeouts, and callbacks.
//
//                  Multiple DNS requests may be active concurrently. Each request is tracked
//                  independently inside the DNS client and resolved when the corresponding DNS
//                  response is received.
//
//-------------------------------------------------------------------------------------------------
#if (IP_USE_DNS == DEF_ENABLED)
bool IP_Manager::RequestDNS(const char* pHostName, DNS_Callback_t pCallback)
{
    return m_DNS.SendQuery(pHostName, pCallback, this);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           OnDNS_Completed
//
//  Parameter(s):   bool            Success     Indicates whether the DNS resolution succeeded.
//
//                  IP_Address_t    ResolvedIP  The resolved IPv4 address when Success is true.
//                                              Undefined when Success is false.
//
//  Return:         void
//
//  Description:    Internal completion handler for DNS queries initiated through the
//                  IP_Manager DNS request bridge. This function is invoked by the static
//                  DNS callback wrapper once the DNS client finishes processing a query.
//
//                  The function forwards the result to the application-provided callback
//                  (if registered) and releases the DNS request lock, allowing new DNS
//                  requests to be issued.
//
//-------------------------------------------------------------------------------------------------
/*
void IP_Manager::OnDNS_Completed(bool Success, IP_Address_t ResolvedIP)
{
    m_DNS_Request.Busy = false;                         // Release the mutex first

    if(m_DNS_Request.pCallback)                         // Forward to application
    {
        m_DNS_Request.pCallback(this, Success, ResolvedIP);
    }
}
*/
//-------------------------------------------------------------------------------------------------
//
//  Name:           DNS_StaticCallback
//
//  Parameter(s):   IP_Manager*     pContext    Pointer to the IP_Manager instance that initiated
//                                              the DNS request. Used to route the completion event
//                                              back to the correct object.
//                  bool            Success     Indicates whether the DNS resolution completed
//                                              successfully.
//                  IP_Address_t    ResolvedIP  The resolved IPv4 address when Success is true.
//                                              Undefined when Success is false.
//
//  Return:         void
//
//  Description:    Static wrapper function used by the DNS client to report completion of a DNS
//                  query. Because the DNS client operates with a generic callback signature,
//                  this function provides the necessary bridge to instance-level handling.
//
//                  The function casts the context pointer back to an IP_Manager object and
//                  forwards the result to the instance method OnDNS_Completed(), which performs
//                  final processing and releases the DNS request lock.
//
//-------------------------------------------------------------------------------------------------
/*void IP_Manager::DNS_StaticCallback(IP_Manager* pIP_Manager, bool Success, IP_Address_t ResolvedIP)
{
    if(pIP_Manager != nullptr)
    {
        pIP_Manager->OnDNS_Completed(Success, ResolvedIP);
    }
}
*/
#endif

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
    snprintf(pBuffer, IP_ASCII_ADDRESS_SIZE, "%d.%d.%d.%d", IP_A(IP_Address),
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
void IP_Manager::PutHeader(IP_PacketMsg_t* pTX, IP_Address_t DstIP, uint16_t PayloadLength, uint8_t Protocol)    // UDP=17, TCP=6
{
    IP_Header_t*         pIP  = &pTX->pPacket->IP_Frame.Header;
    IP_EthernetHeader_t* pETH = &pTX->pPacket->ETH_Header;

    // Ethernet header
    IP_MAC_Address_t MacAddress;
    m_Context.GetMAC_Address(&MacAddress);
    memcpy(&pETH->SourceMAC.Byte[0], &MacAddress.Byte[0], IP_MAC_ADDRESS_SIZE);
    pETH->Type = IP_ETHERNET_TYPE_IPV4;

    // IPv4 header
    pIP->VersionIHL          = IP_VERSION4_IHL20;
    pIP->TypeOfService       = 0;
    pIP->Length              = htons(sizeof(IP_Header_t) + PayloadLength);
    pIP->FlagsFragmentOffset = htons(0);
    pIP->TimeToLive          = IP_TIME_TO_LIVE;
    pIP->Protocol            = Protocol;
    pIP->SrcIP_Address       = m_Context.GetActiveIP();
    pIP->DstIP_Address       = DstIP;

    if(Protocol != IP_PROTOCOL_ICMP)
    {
        pIP->ID = htons(m_SequenceID++);
    }

    pIP->Checksum = 0;
    pIP->Checksum = htons(IP_CalculateChecksum(pIP, sizeof(IP_Header_t)));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_CalculateChecksum
//
//  Parameters:     void*       pBuffer     Pointer to the start of the data block to checksum
//                  uint16_t    Count       Number of bytes to include in the checksum
//
//  Return:         uint16_t                One's-complement checksum (network byte order)
//
//  Description:    Computes the standard Internet checksum as defined in RFC 1071. The algorithm
//                  processes the buffer as a sequence of 16-bit big-endian words, performs
//                  one's-complement addition with end-around carry, and returns the one's-
//                  complement of the final accumulated sum.
//
//  Notes:          - The caller must ensure that the checksum field within the header or message
//                    is set to zero before invoking this function.
//                  - If Count is odd, the final remaining byte is padded as the high byte of a
//                    16-bit word and included in the sum.
//                  - This function is suitable for IPv4 header checksums, ICMP checksums, and
//                    pseudo-header checksums used by UDP and TCP.
//                  - The buffer does not need to be 16-bit aligned; the function handles byte
//                    access safely and deterministically.
//
//-------------------------------------------------------------------------------------------------
uint16_t IP_Manager::IP_CalculateChecksum(const void* pBuffer, uint16_t Count)
{
    const uint8_t* Data = (const uint8_t*)pBuffer;
    uint32_t Sum = 0;

    while(Count > 1)
    {
        Sum   += (uint16_t)((Data[0] << 8) | Data[1]);
        Data  += 2;
        Count -= 2;

        if(Sum & 0x10000)
        {
            Sum = (Sum & 0xFFFF) + 1;
        }
    }

    if(Count == 1)
    {
        Sum += (uint16_t)(Data[0] << 8);

        if(Sum & 0x10000)
        {
            Sum = (Sum & 0xFFFF) + 1;
        }
    }

    return (uint16_t)~Sum;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UDP_CalculateChecksum
//
//  Parameters:     IP_Header_t*   pIP         Pointer to the IPv4 header containing source/dest IP
//                  UDP_Header_t*  pUDP        Pointer to the UDP header (checksum field = 0)
//                  uint16_t       UDP_Length  Length of UDP header + payload (in bytes)
//
//  Return:         uint16_t                    One's-complement UDP checksum (network byte order)
//
//  Description:    Computes the UDP checksum as defined in RFC 768 and RFC 1071. The checksum is
//                  calculated over the UDP header, UDP payload, and the IPv4 pseudo-header.
//                  The pseudo-header includes:
//                      - Source IP address
//                      - Destination IP address
//                      - Protocol number (UDP = 17)
//                      - UDP length
//
//                  The algorithm performs one's-complement addition of all 16-bit words, including
//                  end-around carry, and returns the one's-complement of the final accumulated sum.
//
//  Notes:          - The caller must ensure that the UDP checksum field is set to zero before
//                    invoking this function.
//                  - If UDP_Length is odd, the final remaining byte is padded as the high byte of
//                    a 16-bit word and included in the sum.
//                  - The function assumes that the IPv4 header is already in network byte order.
//                  - Suitable for both outgoing UDP packets and validating incoming packets.
//                  - The buffer does not need to be 16-bit aligned; the function handles byte
//                    access safely and deterministically.
//
//-------------------------------------------------------------------------------------------------
uint16_t IP_Manager::UDP_CalculateChecksum(IP_Header_t* pIP, UDP_Header_t* pUDP, uint16_t UDP_Length)
{
    uint32_t Sum = 0;

    // Pseudo-header
    Sum += (pIP->SrcIP_Address >> 16) & 0xFFFF;
    Sum += (pIP->SrcIP_Address      ) & 0xFFFF;
    Sum += (pIP->DstIP_Address >> 16) & 0xFFFF;
    Sum += (pIP->DstIP_Address      ) & 0xFFFF;
    Sum += htons(IP_PROTOCOL_UDP);
    Sum += htons(UDP_Length);

    // UDP header + payload
    uint16_t* pPtr = (uint16_t*)pUDP;

    for(uint16_t i = 0; i < (UDP_Length / 2); i++)
    {
        Sum += *pPtr++;
    }

    if(UDP_Length & 1)               // Odd byte?
    {
        Sum += *((uint8_t*)pPtr);
    }

    // Fold 32-bit sum to 16 bits
    while(Sum >> 16)
    {
        Sum = (Sum & 0xFFFF) + (Sum >> 16);
    }

    return ~((uint16_t)Sum);
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
