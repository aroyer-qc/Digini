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

#define SNTP_GLOBAL
#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:           SNTP_Init
//
//  Parameter(s):   OS_EVENT* pQ
//  Return:         void
//
//  Description:    Initialize a timer for resync with server
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SNTP_Init(void* pQ)
{
    SNTP_pQ           = (OS_EVENT*)pQ;
    SNTP_byOST_Resync = TIME_TIMER_nullptr;

    TIMER_Start(&SNTP_byOST_Resync,
                (int32_t)1,                               // first timeout at 1 second
                SNTP_pQ,
                IP_MSG_TYPE_SNTP_MANAGEMENT,
                SNTP_MSG_ACTION_TIME_OUT);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SNTP_Request
//
//  Parameter(s):       SOCKET  SocketNumber
//                      uint8_t*   pDomainName1    Domain Name of the NTP Server 1
//                      uint8_t*   pDomainName2    Domain Name of the NTP_Server 2
//                      uint8_t*   pError          Pointer to return an error code
//  Return:             int32_t   dwIP            IP of the NTP server
//
//  Description:    Send the SNTP request
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int32_t SNTP_Request(SOCKET SocketNumber, uint8_t* pDomainName1, uint8_t* pDomainName2, uint8_t* pError)
{
    int16_t            wPort;
    SNTP_Msg_t*     pTX;
    uint8_t            Error     = ERR_NONE;
    int32_t           dwIP        = DNS_NO_IP;


    dwIP = DNS_Query(SocketNumber, pDomainName1, &Error);                     // Try NTP Server 1
    if(dwIP == DNS_NO_IP)
    {
        dwIP = DNS_Query(SocketNumber, pDomainName2, &Error);                 // if no IP Try NTP Server 2
    }

    if(dwIP != DNS_NO_IP)                                                       // Continue if we have an IP
    {
        pTX = MemGetAndClear(sizeof(DHCP_Msg_t), &Error);

        if(pTX != nullptr)
        {
            TIME_Randomize();
            wPort = (int16_t)TIME_Random(32768) + (int16_t)32768;                 // Get a random source port for the query from 32768 to 65535

            if(SOCK_Socket(SocketNumber, Sn_MR_UDP, wPort, 0) != 0)
            {
                // Fill up standard info for SNTP Packet
                pTX->Flags_1.s.MODE        = SNTP_MODE_CLIENT;
                pTX->Flags_1.s.VN          = SNTP_VERSION_4;
                pTX->dwTxmTimeStampSecond  = htonl(SNTP_TIME_START);

                SNTP_dwSeconds             = TIME_GetSecondTicks();
                if(SOCK_SendTo(SocketNumber, (uint8_t*)pTX, (int16_t)(sizeof(SNTP_Msg_t) - SNTP_OPTIONS_IN_PACKET_SIZE), dwIP, SNTP_PORT) != 0)
                {
                    SNTP_Reply(SocketNumber);
                }
                else
                {
                    *pError = ERR_CANNOT_SEND_TO_SOCKET;
                }
                MemPut(&pTX);
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
    return(dwIP);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SNTP_Reply
//
//  Parameter(s):   void
//  Return:         int32_t   dwIP        Resolve IP
//
//  Description:    This Function process the answer to the DNS Request
//
//  Note(s):        No special treatment here, get the first IP and get out
//
//-------------------------------------------------------------------------------------------------
void SNTP_Reply(SOCKET SocketNumber)
{
    SNTP_Msg_t*     pRX             = nullptr;
    uint32_t          ServerAddr;
    int16_t            wServerPort;
    uint8_t            Error;
    int16_t            wLen;
    int32_t           dwSecond;

    OSTimeDly(300);
    do
    {
        if(SOCK_GetRX_RSR(SocketNumber) > 0)
        {
            pRX = MemGetAndClear(sizeof(SNTP_Msg_t), &Error);
            if(pRX != nullptr)
            {
                SOCK_ReceivedFrom(SocketNumber, (uint8_t*)pRX, sizeof(DNS_Msg_t), &ServerAddr.dw, &wServerPort);

                pRX->dwOriTimeStampSecond  = ntohl(pRX->dwOriTimeStampSecond);
                pRX->dwRcvTimeStampSecond  = ntohl(pRX->dwRcvTimeStampSecond);
                pRX->dwTxmTimeStampSecond  = ntohl(pRX->dwTxmTimeStampSecond);
                SNTP_dwSeconds             = TIME_GetSecondTicks() - SNTP_dwSeconds;
                pRX->dwTxmTimeStampSecond += (SNTP_dwSeconds / 2);
                TIME_LocalTime(pRX->dwRcvTimeStampSecond - SNTP_UNIX_START);

                // Initialize timer resync a 1:00:00 every morning night
                dwSecond  = TIME_DateTime.dwSecondOfTheDay;     // Take the timestamps seconds for this moment
                dwSecond  = SNTP_SECONDS_PER_DAY - dwSecond;    // Remove this elapse time from number of second in a day
                dwSecond += SNTP_SECONDS_PER_HOUR;              // Add to it one hour

                TIMER_Start(&SNTP_byOST_Resync,
                            dwSecond,
                            SNTP_pQ,
                            IP_MSG_TYPE_SNTP_MANAGEMENT,
                            SNTP_MSG_ACTION_TIME_OUT);
            }
        }
    }
    while(pRX == nullptr);

    MemPut(&pRX);
}

//-------------------------------------------------------------------------------------------------
