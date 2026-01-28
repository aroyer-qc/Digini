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
    m_SocketManager.Initialize(&m_Context);               // Initialize socket manager

    m_Context.SetDHCP_Enable(false);

    // Initialize Variables
    m_Context.SetIP_Valid(false);
    //m_DNS_IP_Found = false;  not used so far
    m_Context.InitializeMsgQ();                                             // this need to handle error
    m_Context.SetMAC_Address(&m_Config[IF_ID].IP_ETH_Config.MAC_Address);
    m_Context.SetHostName(m_Config[IF_ID].pHostName);
    m_Context.SetMTU(IP_NET_IF_MTU);                                        // Set netif maximum transfer unit

    m_Context.SetStaticIP(m_Config[IF_ID].DefaultStatic_IP);
    m_Context.SetStaticGatewayIP(m_Config[IF_ID].DefaultGateway);
    m_Context.SetStaticSubnetMask(m_Config[IF_ID].DefaultSubnetMask);
    m_Context.SetStaticDNS_IP(m_Config[IF_ID].DefaultStaticDNS);

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

  #if (IP_USE_ICMP == DEF_ENABLED)
    m_ICMP.Initialize(&m_Context);
  #endif

  #if (IP_USE_TCP == DEF_ENABLED)
    m_TCP.Initialize(&m_Context);
  #endif

  #if (IP_USE_NTP == DEF_ENABLED)
    m_NTP.Initialize(&m_Context);
  #endif

  #if (IP_USE_SNTP == DEF_ENABLED)
    m_pSNTP.Initialize(&m_Context);
  #endif

  #if (IP_USE_SOAP == DEF_ENABLED)
    m_SOAP.Initialize(&m_Context);
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
    IP_Address_t    IP;
    IP_Error_e      Error;
  #endif

    for(;;)
    {
      #if (IP_USE_DHCP == DEF_ENABLED)
        if(m_Context.GetLinkChange() == true)                       // Always react to link changes, regardless of DHCP enable state
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
                    m_DHCP.Reset();   // Ensure no stale DHCP state
                }
            }
            else // Link down
            {
                m_DHCP.Reset();
            }
        }

        // Run DHCP state machine only when enabled AND link is up
        if((m_Context.IsDHCP_Enable() == true) && (m_Context.GetLinkState() == ETH_LINK_UP))
        {
            (void)m_DHCP.Process();
        }
      #endif

        if(nOS_QueueRead(m_Context.GetMsgQ(), (void**)&pMsg, NOS_WAIT_INFINITE) == NOS_OK)
        {

if(pMsg->pPacket->U8RawData[23] == 0x01)
{
    __asm("nop");
}





            // Basic Ethernet header size check  peut-etre pas necessaire avec le default
            if(pMsg->PacketSize < sizeof(IP_EthernetHeader_t))
            {
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: Frame too small for Ethernet header (%u bytes)\n",  pMsg->PacketSize);
                FreeMessage(pMsg);
                continue;
            }

            switch(ntohs(pMsg->pPacket->ETH_Header.Type))
            {
                case IP_ETHERNET_TYPE_IP:
                {
                    // Check minimum size for IPv4 header
                    if(pMsg->PacketSize < (sizeof(IP_EthernetHeader_t) + sizeof(IP_Header_t)))
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: IPv4 frame too small (%u bytes)\n", pMsg->PacketSize);
                        FreeMessage(pMsg);
                        break;
                    }

                    // Validate IHL (Internet Header Length)
                    uint8_t ihl = pMsg->pPacket->IP_Frame.Header.VersionIHL & 0x0F;
                    if(ihl < 5)
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: Invalid IHL (%u)\n", ihl);
                        FreeMessage(pMsg);
                        break;
                    }

                   uint16_t ipHeaderSize = ihl * 4;

                    // Validate total IP length
                    uint16_t totalLength = ntohs(pMsg->pPacket->IP_Frame.Header.Length);

                    if(totalLength < ipHeaderSize)
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: totalLength < ipHeaderSize (%u < %u)\n", totalLength, ipHeaderSize);
                        FreeMessage(pMsg);
                        break;
                    }

                    // Ensure the received frame contains the full IP packet
                    if(pMsg->PacketSize < (sizeof(IP_EthernetHeader_t) + totalLength))
                    {
                        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DROP: Truncated IPv4 frame (%u < %u)\n", pMsg->PacketSize, sizeof(IP_EthernetHeader_t) + totalLength);
                        FreeMessage(pMsg);
                        break;
                    }

                    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "IP_ETHERNET_TYPE_IP: Protocol:0x%02X\n", pMsg->pPacket->IP_Frame.Header.Protocol);

                    m_ARP.ProcessIP(pMsg);                  // May update ARP cache, does NOT own pMsg
                    ProcessIP(pMsg);                        // Transfers ownership to protocol/socket
                }
                break;

                case IP_ETHERNET_TYPE_ARP:
                {
                    m_ARP.ProcessARP(pMsg);                 // ARP owns and frees pMsg
                }
                break;

                default:
                {
                    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH_Default: Protocol:0x%02X\n", pMsg->pPacket->IP_Frame.Header.Protocol);
                    // Unknown Ethernet type -> free
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
             m_UDP.Process(pMsg);
        }
        break;
      #endif

        default:
        {
            FreeMessage(pMsg);
        }
        break;
    }
}

SystemState_e IP_Manager::SendPacket(IP_PacketMsg_t* pMsg)
{
    IP_EthernetHeader_t* pETH = &pMsg->pPacket->ETH_Header;
    IP_Header_t*         pIP  = &pMsg->pPacket->IP_Frame.Header;
    IP_Address_t         dstIP = pIP->DstIP_Addr;

    // Broadcast: 255.255.255.255 -> FF:FF:FF:FF:FF:FF
    if(dstIP == IP_ADDRESS(255,255,255,255))
    {
        memset(pETH->DestinationMAC.Byte, 0xFF, IP_MAC_ADDRESS_SIZE);
    }
    else
    {
        // Unicast -> resolve via ARP
        if(m_ARP.Resolve(dstIP, &pETH->DestinationMAC) == false)
        {
            // ARP not ready -> caller decides what to do
            return SYS_ARP_RESOLVE_PENDING;
        }
    }

    // Hand off to interface context (driver callback)
    return m_Context.SendPacket(pMsg);
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
//  Name:           IP_ToAscii
//
//  Parameter(s):   char*               Pointer to return formatted string
//                  IP_Address_t        IP_Address
//  Return:         void
//
//  Description:    Put IP in a string following standard format EX. 192.168.1.100
//
//  Note(s):        Don't forget to pMemory->Free() the pointer after use
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::IP_ToAscii(char* pBuffer, IP_Address_t IP_Address)
{
    snprintf(pBuffer, IP_ASCII_ADDRESS_SIZE, "%d.%d.%d.%d", uint8_t(IP_Address),
                                                            uint8_t(IP_Address >> 8),
                                                            uint8_t(IP_Address >> 16),
                                                            uint8_t(IP_Address >> 24));
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
IP_Address_t IP_Manager::AsciiToIP(char* pBuffer)
{
    IP_Address_t IP_Address;
    uint32_t     Count;
    uint8_t      DotCount;
    bool         IP_Status;

    IP_Address = IP_ADDRESS(0,0,0,0);
    IP_Status  = true;
    Count      = 0;

    if(pBuffer != nullptr)
    {
        while(IP_Status == true)           // Scan to see if it contain only number and dot
        {
            if(((*(pBuffer + Count) < '0') || (*(pBuffer + Count) > '9')) &&
               ((*(pBuffer + Count) != '.')))
            {
                IP_Status = false;
            }
            Count++;
        }

        if(IP_Status == true)                                                // Yes it contain only number and dot
        {
            if((Count >= 7) && (Count <= 15))                                           // Check length
            {
                Count    = 4;
                DotCount = 0;

                do
                {
                    Count--;
                    IP_Status = false;

                    do
                    {
                        if((*pBuffer >= '0') && (*pBuffer <= '9'))
                        {
                            if(IP_Status == false) IP_Status = true;                      // Trap first occurrence
                          // ??  else                     IP.Array[Count] *= 10;            // Other Must be multiply 10

                          //  IP.Array[Count] += (*pBuffer - '0');
                            pBuffer++;
                        }
                        else
                        {
                            IP_Status = false;
                        }
                    }
                    while(IP_Status == true);

                    if(*pBuffer == '.')
                    {
                        pBuffer++;                                                      // skip the dot
                        DotCount++;
                    }
                }
                while(*pBuffer != '\0');

                if((Count != 0) && ( DotCount != 3))                                // Check if format was valid
                {
                   IP_Address = IP_ADDRESS(0,0,0,0);
                }
            }
        }
    }

    return IP_Address;
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
  // uint8_t Error;

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
//                  uint16_t            payloadLength   Length of the transport‑layer payload
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
//                  directly into the caller‑provided packet buffer.
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
    pETH->Type = htons(IP_ETHERNET_TYPE_IP);

    // IPv4 header
    pIP->VersionIHL          = IP_VERSION4_IHL20;
    pIP->TypeOfService       = 0;
    pIP->Length              = htons(sizeof(IP_Header_t) + PayloadLength);
    pIP->ID                  = htons(m_SequenceID++);
    pIP->FlagsFragmentOffset = htons(0);
    pIP->TimeToLive          = IP_TIME_TO_LIVE;
    pIP->Protocol            = Protocol;
    pIP->SrcIP_Addr          = m_Context.GetActiveIP();
    pIP->DstIP_Addr          = DstIP;

    pIP->Checksum = 0;
    pIP->Checksum = IP_CalculateChecksum(pIP, sizeof(IP_Header_t));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_CalculateChecksum
//
//  Parameter(s):   void*       pBuffer     Pointer to the start of the header to checksum
//                  uint16_t    Count       Number of bytes to include in the checksum
//
//  Return:         int16_t                 One's-complement checksum (network byte order)
//
//  Description:    Calculate the standard Internet checksum (RFC 1071) over the supplied buffer.
//                  The algorithm processes the data as 16-bit words, performs one's-complement
//                  addition with end-around carry, and returns the one's-complement of the final
//                  accumulated sum.
//
//  Note(s):        - If Count is odd, the final remaining byte is included as a padded 16-bit word.
//                  - This function assumes the buffer is aligned or accessible as 16-bit values.
//                  - Used for IPv4 header checksum and can be reused for UDP/TCP pseudo-header
//                    checksum calculations.
//                  - The caller is responsible for ensuring that the checksum field in the header
//                    is zero before invoking this function.
//
//-------------------------------------------------------------------------------------------------

int16_t IP_Manager::IP_CalculateChecksum(void* pBuffer, uint16_t Count)
{
	int16_t 	i;
	uint16_t*	Value;
	struct32_t 	Checksum;

    Checksum.u_32 = 0;
	i = Count >> 1;
    Value = (uint16_t*)pBuffer;

	while(i--)                                                                      // Calculate the sum of all words
	{
		Checksum.u_32 += (uint32_t)*Value++;
	}

	if(((struct16_t*)&Count)->u_8.u0)                                               // Add in the sum of the remaining byte, if present
	{
		Checksum.u_32 += (uint32_t)*(uint8_t*)Value;
	}

	Checksum.u_32 = (uint32_t)Checksum.u8_Array[0] + (int32_t)Checksum.u8_Array[1]; // Do an end-around carry (one's complement arithmetic)
	Checksum.u8_Array[0] += Checksum.u8_Array[1];                                   // Do another end-around carry in case if the prior add caused a carry out
	return ~Checksum.u8_Array[0];                                                  	// Return the resulting checksum
}


uint16_t IP_Manager::UDP_CalculateChecksum(IP_Header_t* pIP, UDP_Header_t* pUDP, uint16_t UDP_Length)
{
    uint32_t Sum = 0;

    // Pseudo-header
    Sum += (pIP->SrcIP_Addr >> 16) & 0xFFFF;
    Sum += (pIP->SrcIP_Addr      ) & 0xFFFF;
    Sum += (pIP->DstIP_Addr >> 16) & 0xFFFF;
    Sum += (pIP->DstIP_Addr      ) & 0xFFFF;
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
//                  - Ensures consistent zero‑copy buffer ownership release.
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




#if 0


void RAW_Input(IP_PacketMsg_t* pMsg)
{
    IP_Header_t* pIP = &pMsg->pPacket->IP_Header;
    uint8_t proto    = pIP->Protocol;

    // Find a RAW socket bound to this protocol
    Socket* pSock = Socket_FindRAWByProtocol(proto);
    if (pSock == nullptr)
    {
        // No RAW consumer → just free
        pMemoryPool->Free((void**)&pMsg->pPacket);
        pMemoryPool->Free((void**)&pMsg);
        return;
    }

    RAW_Socket_t* pRaw = pSock->m_Proto.raw;

    if (nOS_QueueWrite(&pRaw->RxQueue, &pMsg, 0) != NOS_OK)
    {
        // Queue full → drop
        pMemoryPool->Free((void**)&pMsg->pPacket);
        pMemoryPool->Free((void**)&pMsg);
        return;
    }

    // Ownership moves to RAW socket
}

void TCP_Input(IP_PacketMsg_t* pMsg)
{
    TCP_Header_t* pTCP = &pMsg->pPacket->TCP_Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->IP_Header;

    uint16_t destPort = ntohs(pTCP->DestPort);
    IP_Address_t localIP  = pIP->DestIP;
    IP_Address_t remoteIP = pIP->SrcIP;
    uint16_t     srcPort  = ntohs(pTCP->SrcPort);

    TCP_Socket_t* pConn = TCP_FindConnection(localIP, destPort, remoteIP, srcPort);

    if (pConn == nullptr)
    {
        // Maybe a listening socket (SYN to a listening port)?
        TCP_Socket_t* pListener = TCP_FindListener(localIP, destPort);
        if (pListener != nullptr)
        {
            TCP_HandleListenSegment(pListener, pMsg);
            return;
        }

        // Otherwise, send RST
        TCP_SendRST(pMsg);
        TCP_FreePacket(pMsg);
        return;
    }

    TCP_ProcessSegment(pConn, pMsg);
}

TCP_Socket_t* TCP_FindConnection(IP_Address_t localIP, uint16_t localPort,
                                 IP_Address_t remoteIP, uint16_t remotePort);

TCP_Socket_t* TCP_FindListener(IP_Address_t localIP, uint16_t localPort);

void TCP_ProcessSegment(TCP_Socket_t* pConn, IP_PacketMsg_t* pMsg)
{
    // Parse flags, seq, ack, window, etc.
    // Check if seq in window, ACK valid, etc.
    // Update SndUna based on ACK
    // Queue payload into pConn->RxQueue if in-order
    // Generate ACK if needed
    // Handle FIN: transition to CLOSE_WAIT / LAST_ACK / TIME_WAIT
    // Handle retransmission timers, etc.

    TCP_FreePacket(pMsg);
}

void TCP_TimerTick(uint32_t elapsedMs)
{
    for (each TCP_Socket_t in use)
    {
        // Decrement timers
        if (pConn->RtoTimer > elapsedMs) pConn->RtoTimer -= elapsedMs; else RTO_Expired(pConn);
        if (pConn->KeepAliveTimer > elapsedMs) { ... }
        if (pConn->TimeWaitTimer > elapsedMs) { ... }
    }
}

typedef struct
{
    Socket* pSocket;
    uint32_t Events;    // READABLE, WRITABLE, EXCEPTION
    uint32_t Returned;  // Set by Wait()
} SocketWaitItem_t;

SystemState_e Socket_Wait(SocketWaitItem_t* pItems, size_t Count, uint32_t TimeoutMs);
#endif
