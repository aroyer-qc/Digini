//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_IP.cpp
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

#define TASK_IP_GLOBAL
#include <ip.h>

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define IP_ASCII_IP_ADDRESS_SIZE            16

//-------------------------------------------------------------------------------------------------
//
//  Name:           TaskIP
//
//  Parameter(s):   void* pArg          Not used
//
//  Return:         none
//
//  Description:    This function will poll the IP stack and dispatch message to service
//
//  Note(s):        Will initialize the stack
//                  this task is also the task start point
//
//-------------------------------------------------------------------------------------------------
void IP_Manager::Run(void)
{
    uint32_t            Addr;
    uint8_t             Error;
    MSG_t*              pMsg        = nullptr;
    uint32_t            dwIP;

    for(;;)
    {
        if(DHCP_Process(nullptr) == true)                           // If enable, an IP must be valid to continue.
        {                                                           // If not enable it continue anyway
            OSTimeDly(1);

			pRX = CS8900_Poll();									// Network driver read an entire IP packet into the RX Buffer

			if(pRX != nullptr)										// Check if a packet is present
			{
				switch(pRX->Packet.u.ETH_Header.wType)				// Process depending on what kind of packet we have received.
				{
					case IP_ETHERNET_TYPE_IP:
					{
						ARP_ProcessIP(pRX);
						pTX = IP_Process(pRX);
						ARP_ProcessOut(pTX);               			// If data are to be sent back, then send the data
						break;
					}

					case IP_ETHERNET_TYPE_ARP:
					{
						ARP_ProcessARP(pRX);
						break;
					}
				}

				MemPut(&pRX);
			}

			if((pMsg = OSQAccept(Queue.Names.pTaskIP, &Error)) != nullptr)
            {
                switch(pMsg->byType)
                {
					case IP_MSG_TYPE_DHCP_MANAGEMENT:
					{
						IP_Status.b.Status = DHCP_Process(pMsg);
						if(IP_Status.b.Status == false)
						{
							for(int i = 0; i < IP_STACK_NUMBER_OF_SOCKET; i++)
							{
								SOCK_Close(i);
							}
						}
						break;
					}

                    case IP_MSG_TYPE_SNTP_MANAGEMENT:
                    {
                        dwIP = SNTP_Request(IP_SNTP_SOCKET, IP_DEFAULT_NTP_SERVER_1, IP_DEFAULT_NTP_SERVER_2, &Error);
                        IP_Status.b.SNTP_Fail = (bool)((dwIP == 0x00000000) ? false : true);
                        break;
                    }
					
					// put other management here
                }

                MemPut(&pMsg);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_GetDNS_IP()
//
//  Parameter(s):   void
//  Return:         uint32_t   dwIP
//
//  Description:    Return DNS server IP address according to configuration
//
//-------------------------------------------------------------------------------------------------
uint32_t IP_GetDNS_IP(void)
{
    if(DHCP_GetMode() == true)
    {
        return(IP_DHCP_DNS_IP);
    }
    return(IP_StaticDNS_IP);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_GetHost_IP
//
//  Parameter(s):   void
//  Return:         uint32_t   dwIP
//
//  Description:    Return host IP address according to configuration
//
//-------------------------------------------------------------------------------------------------
uint32_t IP_GetHost_IP(void)
{
    if(DHCP_GetMode() == true)
    {
        return(IP_DHCP_IP);
    }
    
    return(IP_StaticIP);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_Init
//
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Initialize IP Task and stack
//
//-------------------------------------------------------------------------------------------------
void IP_Init(void)
{
    IP_Status.b.IP_IsValid      = false;
    IP_Status.b.DNS_IP_Found    = false;
    
    AppTaskStart();

    // Initialize the TCP/IP stack.
    IP_Init();
	ARP_Init(Queue.Names.pTaskIP);				// is there a call back for ARP ???
	DHCP_Init(Queue.Names.pTaskIP);
    SNTP_Init(Queue.Names.pTaskIP);
    NIC_Init();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_ToAscii
//
//  Parameter(s):   uint32_t
//  Return:         uint8_t*
//
//  Description:    Put IP in a string following standard format EX. 192.168.1.100
//
//  Note(s):        Don't forget to MemPut the pointer after use
//
//-------------------------------------------------------------------------------------------------
uint8_t* IP_ToAscii(uint32_t IP)
{
    uint8_t*   pBuffer;

    pBuffer = pMemoryPool->AllocAndClear(IP_ASCII_IP_ADDRESS_SIZE); 

    if(pBuffer != nullptr)
    {
        LIB_sprintf(pBuffer, "%d.%d.%d.%d", ((uint32_t*)(uint32_t)&IP)->by.by0,
                                            ((uint32_t*)(uint32_t)&IP)->by.by1,
                                            ((uint32_t*)(uint32_t)&IP)->by.by2,
                                            ((uint32_t*)(uint32_t)&IP)->by.by3);
    }
    return pBuffer;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_AsciiToIP
//
//  Parameter(s):   uint8_t*        pBuffer
//  Return:         IP_Address_t
//
//  Description:    Extract the IP form a string
//
//  Note(s):        lenght is check and also number of dot, to confirm it is an IP
//
//-------------------------------------------------------------------------------------------------
IP_Address_t IP_AsciiToIP(uint8_t* pBuffer)
{
    IP_Address_t IP_Address;
    uint8_t      Count;
    uint8_t      DotCount;

    IP_Address         = IP_ADDR(0,0,0,0);
    IP_Status.b.Status = true;
    Count            = 0;

    while((*(pBuffer + Count) != nullptr) && (IP_Status.b.Status == true))           // Scan to see if it contain only number and dot
    {
        if(((*(pBuffer + Count) < '0') || (*(pBuffer + Count) > '9')) &&
           ((*(pBuffer + Count) != '.')))
        {
            IP_Status.b.Status = false;
        }
        Count++;
    }

    if(IP_Status.b.Status == true)                                                  // Yes it contain only number and dot
    {
        if((Count >= 7) && (Count <= 15))                                       // Check lenght
        {
            Count    = 4;
            DotCount = 0;

            do
            {
                Count--;

                IP_Status.b.Status = false;
                do
                {
                    if((*pBuffer >= '0') && (*pBuffer <= '9'))
                    {
                        if(IP_Status.b.Status == false) IP_Status.b.Status = true; // Trap first occurence
                        else                            IP.Array[Count] *= 10; // Other Must be multiply 10

                        IP.Array[Count] += (*pBuffer - '0');
                        pBuffer++;
                    }
                    else
                    {
                        IP_Status.b.Status = false;
                    }
                }
                while(IP_Status.b.Status == true);

                if(*pBuffer == '.')
                {
                    pBuffer++;                                                      // skip the dot
                    DotCount++;
                }
            }
            while(*pBuffer != nullptr);

            if((byCount != 0) && ( DotCount != 3))                                // Check if format was valid
            {
               IP_Address = IP_ADDR(0,0,0,0);
            }
        }
    }

    return IP;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_ProcessURL
//
//  Parameter(s):   uint8_t*      pBuffer
//                  IP_Address_t* pIP
//                  uint16_t*     pPort
//  Return:         uint8_t *     pURI
//
//  Description:    This function will process an URL
//                      -   Extract Port number if any ( return 80 otherwise )
//                      -   Get the IP from DNS
//                      -   Return pointer o the URI
//
//  Note(s):        It is assume that "http://" is always lowercase
//
//-------------------------------------------------------------------------------------------------
uint8_t* IP_ProcessURL(uint8_t* pBuffer, IP_Address_t* pIP, uint16_t* pPort)
{
    uint8_t*   pDomainName;
    uint8_t*   pSearch1        = nullptr;
    uint8_t*   pSearch2        = nullptr;
    uint8_t    Error;


    *pPort = 80;                                        // Set to default port if none are found

    // Get Domain Name or IP ......................................................................

    pSearch1 = LIB_strstr(pBuffer, "http://");          // Remove the unused "http://"

    if(pSearch1 == pBuffer)
    {
        LIB_strcpy(pBuffer, pBuffer + 7);               // Make the correction in the buffer
    }

    pDomainName = pBuffer;                              // Found the beginning of the domain name or IP

    // Get port number if any .....................................................................

    pSearch1     = LIB_strchr(pBuffer, ':');            // Search for a port number  looking at the semicolon :

    if(pSearch1 != nullptr)                                // not nullptr then extract port number
    {
        *pSearch1 = nullptr;                               // Put nullptr at the : position for nullptr terminated string
        pSearch1++;

        IP_Status.b.Status = false;
        do
        {
            if((*pSearch1 >= '0') && (*pSearch1 <= '9'))
            {
                if(IP_Status.b.Status == false)
                {
                    *pPort  = 0;                        // Trap first occurence
                    IP_Status.b.Status = true;
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
                IP_Status.b.Status = false;
            }
        }
        while(IP_Status.b.Status == true);
    }
    else
    {
        pSearch1 = pBuffer;                             // Replace search pointer if no port found
    }

    // Get URI pointer if any .....................................................................

    pSearch2 = pSearch1;

    pSearch1 = LIB_strchr(pSearch1, '/');               // Search for separator beginning of URI
    if(pSearch1 != nullptr)
    {
        *pSearch1 = nullptr;                               // Put nullptr at the / position for nullptr terminated 'Domain Name' string
        pSearch1++;                                     // Place pointer where URI start minus first '/'

        pSearch2 = pSearch1;
    }

    // Get the space at the end ..................................................................

    pSearch2 = LIB_strchr(pSearch2, ' ');
    if(pSearch2 != nullptr)
    {
        *pSearch2 = nullptr;                               // Put nullptr at the 'SPACE' for a nullptr terminated string
    }

    // Scan to found if it is an IP or "Domain Name" .............................................

    *pIP = IP_AsciiToIP(pDomainName);

    if(*pIP == IP_ADDR(0,0,0,0))
    {
        *pIP = DNS_Query(IP_DNS_SOCKET, pDomainName, &Error);
    }

    return pSearch1;
}

//-------------------------------------------------------------------------------------------------
