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

#define IP_ASCII_IP_ADDRESS_SIZE            16

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const IP_Config_t IP_Manager::m_Config[IP_NUMBER_OF_INTERFACE] =
{
    IF_ETH_DEF(EXPAND_X_IF_AS_STRUCT_DATA)
};

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
    nOS_Error            Error;

       // Initialize Variables
    //m_IP_IsValid             = false;
    m_DNS_IP_Found = false;
    m_IP_Status    = false;

    m_IF_Driver.Initialize(&m_Config[IF_ID].IP_ETH_Config);


    // Initialize the MAC Address
   // m_pEthernetIF->pETH_Driver.SetMacAddress(pMAC_Address);
    //m_pEthernetIF->pETH_Driver.Initialize(pMAC_Address);


    // Set netif maximum transfer unit
   // m_MTU = IP_NET_IF_MTU;


//    pNIC->Initialize();


    //mIP->Initialize();

    // All protocol support are created dynamically if interface is set to use it, and if configuration is enable for that protocol


  #if (IP_USE_UDP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_UDP) != 0)
   #endif
    {
        m_pUDP = new NetUDP();
        m_pUDP->Initialize();
    }
  #endif

  #if (IP_USE_DHCP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_DHCP) != 0)
   #endif
    {
        m_pDHCP = new NetDHCP();
        m_pDHCP->Initialize();
    }
  #endif


  #if (IP_USE_ARP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_ARP) != 0)
   #endif
    {
        m_pARP = new NetARP();
        m_pARP->Initialize();
    }
  #endif

  #if (IP_USE_ICMP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_ICMP) != 0)
   #endif
    {
        m_pICMP = new NetICMP();
        m_pICMP->Initialize();
    }
  #endif

  #if (IP_USE_TCP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_TCP) != 0)
   #endif
    {
        m_pTCP = new NetTCP();
        m_pTCP->Initialize();
    }
  #endif

  #if (IP_USE_NTP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_NTP) != 0)
   #endif
    {
        m_pNTP = new NetNTP();
        m_pNTP->Initialize();
    }
  #endif

  #if (IP_USE_SNTP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
   #endif
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_SNTP) != 0)
    {
        m_pSNTP = new NetSNTP();
        m_pSNTP->Initialize();
    }
  #endif

  #if (IP_USE_SOAP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_SOAP) != 0)
   #endif
    {
        m_pSOAP = new NetSOAP();
        m_pSOAP->Initialize();
    }
  #endif
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
    IP_Address_t   Address;
    uint8_t        Error;
  //  MSG_t*         pMsg        = nullptr;
    IP_Address_t   IP;

    for(;;)
    {
        nOS_Yield();
#if 0
        if(pDHCP->Process(nullptr) == true)                         // If enable, an IP must be valid to continue.
        {                                                           // If not enable it continue anyway
			pRX = CS8900_Poll();									// Network driver read an entire IP packet into the RX Buffer

			if(pRX != nullptr)										// Check if a packet is present
			{
				switch(pRX->Packet.u.ETH_Header.Type)				// Process depending on what kind of packet we have received.
				{
					case IP_ETHERNET_TYPE_IP:
					{
                      #if (IP_USE_ARP == DEF_ENABLED)
                       #if (IP_NUMBER_OF_INTERFACE > 1)
                        if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_ARP) != 0)
                       #endif
                        {
                           m_pARP->ProcessIP(pRX);
                        }
                      #endif

                        pTX = IP_Process(pRX);

                      #if (IP_USE_ARP == DEF_ENABLED)
                       #if (IP_NUMBER_OF_INTERFACE > 1)
                        if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_ARP) != 0)
                       #endif
                        {
                            m_pARP->ProcessOut(pTX);               		// If data are to be sent back, then send the data
                        }
                      #endif
					}
                    break;

                  #if (IP_USE_ARP == DEF_ENABLED)
					case IP_ETHERNET_TYPE_ARP:
					{
                      #if (IP_NUMBER_OF_INTERFACE > 1)
                        if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_ARP) != 0)
                      #endif
						{
                            m_pARP->ProcessARP(pRX);
                        }
					}
                    break;
                  #endif
				}

				pMemory->Free((void**)&pRX);
			}

			/* if((pMsg = OSQAccept(Queue.Names.pTaskIP, &Error)) != nullptr)// nOS Q */
            if(nOS_QueueRead(&m_MsgQueue, pMsg, NOS_WAIT_INFINITE) == NOS_OK);
            {
                switch(pMsg->Type)
                {
                  #if (IP_USE_DHCP == DEF_ENABLED)
					case IP_MSG_TYPE_DHCP_MANAGEMENT:
					{
                      #if (IP_NUMBER_OF_INTERFACE > 1)
                        if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_DHCP) != 0)
                      #endif
                        {
                            m_IP_Status = m_pDHCP->Process(pMsg);

                            if(m_IP_Status == false)
                            {
                                for(int i = 0; i < IP_STACK_NUMBER_OF_SOCKET; i++)
                                {
                                    SOCK_Close(i);
                                }
                            }
                        }
					}
                    break;
                  #endif

                  #if (IP_USE_SNTP == DEF_ENABLED)
                    case IP_MSG_TYPE_SNTP_MANAGEMENT:
                    {
                      #if (IP_NUMBER_OF_INTERFACE > 1)
                        if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_SNTP) != 0)
                      #endif
                        {
                            IP = pSNTP->Request(IP_SNTP_SOCKET, IP_DEFAULT_NTP_SERVER_1, IP_DEFAULT_NTP_SERVER_2, &Error);
                            m_FlagSNTP_Fail = (IP == IP_ADDRESS(0,0,0,0)) ? false : true;
                        }
                    }
                    break;
                  #endif

					// put other management here
                }

                pMemory->Free((void**)&pMsg);
            }
        }
#endif
    }
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
  #if (IP_USE_DHCP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_DHCP) != 0)
   #endif
    {
       if(pDHCP->GetMode() == DHCP_IS_ON)
        {
            return IP_DHCP_DNS_IP;
        }
    }
  #endif

    // TODO there might be a case where HEC have built-in DHCP.. need to handle that case

    return IP_ADDRESS(0,0,0,0);//IP_StaticDNS_IP;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetHost
//
//  Parameter(s):   void
//  Return:         IP_Address_t   dwIP
//
//  Description:    Return host IP address according to configuration
//
//-------------------------------------------------------------------------------------------------
IP_Address_t IP_Manager::GetHost(void)
{
  #if (IP_USE_DHCP == DEF_ENABLED)
   #if (IP_NUMBER_OF_INTERFACE > 1)
    if(m_pEthernetIF->ProtocolFlag & IP_FLAG_USE_DHCP) != 0)
   #endif
    {
        if(pDHCP->GetMode() == DHCP_IS_ON)
        {
            return IP_DHCP_IP;
        }
    }
  #endif

    // TODO there might be a case where HEC have built-in DHCP.. need to handle that case
    // maybe ass a define for external call to get IP Address..


    return IP_ADDRESS(0,0,0,0);//IP_StaticIP;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_ToAscii
//
//  Parameter(s):   IP_Address_t        IP_Address
//  Return:         char*
//
//  Description:    Put IP in a string following standard format EX. 192.168.1.100
//
//  Note(s):        Don't forget to pMemory->Free() the pointer after use
//
//-------------------------------------------------------------------------------------------------
char* IP_Manager::IP_ToAscii(IP_Address_t IP_Address)
{
    char* pBuffer;

    pBuffer = (char*)pMemoryPool->AllocAndClear(IP_ASCII_IP_ADDRESS_SIZE, MEM_DBG_CLASS_IP_MANAGER_1);

    if(pBuffer != nullptr)
    {
        snprintf(pBuffer, IP_ASCII_IP_ADDRESS_SIZE, "%d.%d.%d.%d", uint8_t(IP_Address >> 24),
                                                                   uint8_t(IP_Address >> 16),
                                                                   uint8_t(IP_Address >> 8),
                                                                   uint8_t(IP_Address));
    }
    return pBuffer;
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
//  Note(s):        lenght is check and also number of dot, to confirm it is an IP
//
//-------------------------------------------------------------------------------------------------
IP_Address_t IP_Manager::AsciiToIP(char* pBuffer)
{
    IP_Address_t IP_Address;
    uint32_t     Count;
    uint8_t      DotCount;

    IP_Address  = IP_ADDRESS(0,0,0,0);
    m_IP_Status = true;
    Count       = 0;

    if(pBuffer != nullptr)
    {
        while(m_IP_Status == true)           // Scan to see if it contain only number and dot
        {
            if(((*(pBuffer + Count) < '0') || (*(pBuffer + Count) > '9')) &&
               ((*(pBuffer + Count) != '.')))
            {
                m_IP_Status = false;
            }
            Count++;
        }

        if(m_IP_Status == true)                                                // Yes it contain only number and dot
        {
            if((Count >= 7) && (Count <= 15))                                           // Check length
            {
                Count    = 4;
                DotCount = 0;

                do
                {
                    Count--;
                    m_IP_Status = false;

                    do
                    {
                        if((*pBuffer >= '0') && (*pBuffer <= '9'))
                        {
                            if(m_IP_Status == false) m_IP_Status = true;                // Trap first occurence
                          // ??  else                     IP.Array[Count] *= 10;             // Other Must be multiply 10

                          //  IP.Array[Count] += (*pBuffer - '0');
                            pBuffer++;
                        }
                        else
                        {
                            m_IP_Status = false;
                        }
                    }
                    while(m_IP_Status == true);

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
    uint8_t Error;

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

        m_IP_Status = false;
        do
        {
            if((*pSearch1 >= '0') && (*pSearch1 <= '9'))
            {
                if(m_IP_Status == false)
                {
                    *pPort  = 0;                        // Trap first occurence
                    m_IP_Status = true;
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
                m_IP_Status = false;
            }
        }
        while(m_IP_Status == true);
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

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
