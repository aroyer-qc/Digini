//-------------------------------------------------------------------------------------------------
//
//  File : sntp.c
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
//  SNTP - Simple Network Time Protocol
//
//       <Message Format>
//
//                                 1  1  1  1  1  1  1  1  1  1  2  2  2  2  2  2  2  2  2  2  3  3
//   0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  LI |   VN   |  MODE  |         STRATUM       |         POLL          |      PRECISION        |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                           ROOT DELAY                                          |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                        ROOT DISPERSION                                        |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                REFERENCE TIMESTAMP SECOND (32)                                |
// |                               REFERENCE TIMESTAMP FRACTION (32)                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                ORIGINATE TIMESTAMP SECOND (32)                                |
// |                               ORIGINATE TIMESTAMP FRACTION (32)                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 RECEIVE TIMESTAMP SECOND (32)                                 |
// |                                RECEIVE TIMESTAMP FRACTION (32)                                |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 TRANSMIT TIMESTAMP SECOND (32)                                |
// |                                TRANSMIT TIMESTAMP FRACTION (32)                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 KEY IDENTIFIER (Optional) (32)                                |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 MESSAGE DIGEST (Optional) (128)                               |
// |                                                                                               |
// |                                                                                               |
// |                                                                                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:          Initialize
//
//  Parameter(s):   OS_EVENT*   pQ
//  Return:         void
//
//  Description:    Initialize a timer for resync with server
//
//-------------------------------------------------------------------------------------------------
void NetSNTP::Initialize(void* pQ)
{
    nOS_Error Error;

    m_pQ           = (OS_EVENT*)pQ;
    SNTP_byOST_Resync = TIME_TIMER_nullptr;
    
    Error = nOS_TimerCreate(&m_Resync,
                            nullptr,
                            nullptr,
                            (int32_t)1,                               // first timeout at 1 second
                            NOS_TIMER_ONE_SHOT);

// need to implement a timer Class or use OS one
/*
    TIMER_Start(&m_OST_Resync,
                
                m_pQ,
                IP_MSG_TYPE_SNTP_MANAGEMENT,
                SNTP_MSG_ACTION_TIME_OUT);
*/
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           Request
//
//  Parameter(s):       Socket_t  SocketNumber
//                      uint8_t*   pDomainName1    Domain Name of the NTP Server 1
//                      uint8_t*   pDomainName2    Domain Name of the NTP_Server 2
//                      uint8_t*   pError          Pointer to return an error code
//  Return:             uint32_t   IP              IP of the NTP server
//
//  Description:    Send the SNTP request
//
//-------------------------------------------------------------------------------------------------
int32_t NetSNTP::Request(Socket_t SocketNumber, uint8_t* pDomainName1, uint8_t* pDomainName2, uint8_t* pError)
{
    IP_Port_t    Port;
    SNTP_Msg_t*  pTX;
    uint8_t      Error     = ERR_NONE;
    IP_Address_t IP        = DNS_NO_IP;


    IP = pDNS->Query(SocketNumber, pDomainName1, &Error);                         // Try NTP Server 1
    
    if(IP == DNS_NO_IP)
    {
        IP = pDNS->Query(SocketNumber, pDomainName2, &Error);                     // if no IP Try NTP Server 2
    }

    if(IP != DNS_NO_IP)                                                         // Continue if we have an IP
    {
        pTX = (SNTP_Msg_t*)pMemory->AllocAndClear(sizeof(DHCP_Msg_t));

        if(pTX != nullptr)
        {
            Port = uint16_t(RNG_GetRandomFromRange(32768, 65535));               // Get a random source port for the query from 32768 to 65535

            if(SOCK_Socket(SocketNumber, Sn_MR_UDP, Port, 0) != 0)
            {
                // Fill up standard info for SNTP Packet
                pTX->Flags_1.s.MODE     = SNTP_MODE_CLIENT;
                pTX->Flags_1.s.VN       = SNTP_VERSION_4;
                pTX->TxmTimeStampSecond = htonl(SNTP_TIME_START);
                m_Seconds               = TIME_GetSecondTicks();

                if(SOCK_SendTo(SocketNumber, (uint8_t*)pTX, sizeof(SNTP_Msg_t) - SNTP_OPTIONS_IN_PACKET_SIZE, IP, SNTP_PORT) != 0)
                {
                    Reply(SocketNumber);
                }
                else
                {
                    *pError = ERR_CANNOT_SEND_TO_SOCKET;
                }
                
                pMemory->Free((void**)&pTX);
                SOCK_Close(SocketNumber);
            }
            else
            {
                *pError = ERR_NO_SOCKET;
            }
        }
        else
        {
            *pError = ERR_MEMORY_ALLOCATION;
        }
    }
    return IP;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Reply
//
//  Parameter(s):   Socket_t     SocketNumber
//  Return:         void
//
//  Description:    This Function process the answer to the DNS Request
//
//  Note(s):        No special treatment here, get the first IP and get out
//
//-------------------------------------------------------------------------------------------------
void SNTP_Reply(Socket_t SocketNumber)
{
    SNTP_Msg_t*     pRX             = nullptr;
    IP_Address_t    ServerAddr;
    IP_Port_t       ServerPort;
    size_t          Length;
    TickCount_t     Second;
    nOS_Error       Error;

    do
    {
        if(SOCK_GetRX_RSR(SocketNumber) > 0)
        {
            pRX = pMemory->AllocAndClear(sizeof(SNTP_Msg_t));
            
            if(pRX != nullptr)
            {
                SOCK_ReceivedFrom(SocketNumber, (uint8_t*)pRX, sizeof(DNS_Msg_t), &ServerAddr, &ServerPort);

                pRX->OriTimeStampSecond  = ntohl(pRX->OriTimeStampSecond);
                pRX->RcvTimeStampSecond  = ntohl(pRX->RcvTimeStampSecond);
                pRX->TxmTimeStampSecond  = ntohl(pRX->TxmTimeStampSecond);
                m_Seconds                = GeTick() - m_Seconds;
                pRX->TxmTimeStampSecond += (m_Seconds / 2);
                TIME_LocalTime(pRX->RcvTimeStampSecond - SNTP_UNIX_START);

                // Initialize timer resync a 1:00:00 every morning night
                Second  = TIME_DateTime.SecondOfTheDay;     // Take the timestamps seconds for this moment
                Second  = TIME_SECONDS_PER_DAY - Second;    // Remove this elapse time from number of second in a day
                Second += TIME_SECONDS_PER_HOUR;            // Add to it one hour

                
                Error = nOS_TimerCreate(&m_TimerResync,
                            EscapeCallback,
                            this,
                            SNTP_MSG_ACTION_TIME_OUT,
                            NOS_TIMER_ONE_SHOT);  //??

                /*
                TIMER_Start(&m_TimerResync,
                            Second,
                            m_pQ,
                            IP_MSG_TYPE_SNTP_MANAGEMENT,
                            SNTP_MSG_ACTION_TIME_OUT);
                            */
            }
        }
    }
    while(pRX == nullptr);

    pMemory->Free((void**)&pRX);
}

//-------------------------------------------------------------------------------------------------
