//-------------------------------------------------------------------------------------------------
//
//  File : dns.c
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

//------ Note(s) ----------------------------------------------------------------------------------
//
//       <Message Format>
//
//                                    1  1  1  1  1  1
//      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                      ID                       |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    QDCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    ANCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    NSCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                    ARCOUNT                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+           <QUESTION FORMAT >
//    |                                               |
//    /                     QNAME                     /
//    /                                               /
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QTYPE                     |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QCLASS                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define DNS_GLOBAL
#include <include.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:           DNS_Query
//
//  Parameter(s):       SOCKET  SocketNumber
//                      uint8_t*   pDomainName     Domain Name to get the IP
//                      uint8_t*   pError          Pointer to return an error code
//
//  Return:             int32_t   dwIP
//
//  Description:    Send the DNS Query to DNS Server
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int32_t DNS_Query(SOCKET SocketNumber, uint8_t* pDomainName, uint8_t* pError)
{
    int16_t            wLen;
    int16_t            wPort;
    DNS_Msg_t*      pTX;
    uint8_t*           pQuery;
    uint8_t*           pStr;
    uint8_t            Error     = ERR_NONE;
    int32_t           dwIP        = DNS_NO_IP;

    if(LIB_strlen(pDomainName) <= DNS_MAX_DOMAIN_NAME_LENGHT)               // Check if domain name string is to long
    {
        pTX = MemGetAndClear(sizeof(DHCP_Msg_t), &Error);

        if(pTX != nullptr)
        {
            pQuery = (uint8_t*)&pTX->baData[0];

            TIME_Randomize();
            wPort = (int16_t)TIME_Random(32768) + (int16_t)32768;                 // Get a random source port for the query from 32768 to 65535

            if(SOCK_Socket(SocketNumber, Sn_MR_UDP, wPort, 0) != 0)
            {

                // Fill up standard info for DNS query Packet
                TIME_Randomize();
                DNS_wID                 =(int16_t)TIME_Random(65536);              // Get a random ID
                pTX->wID                = DNS_wID;                              // Keep copy for verification in answers
                pTX->Flags_1.s.QR       = DNS_QR_QUERY;                         // This is a DNS query
                pTX->Flags_1.s.OPCODE   = DNS_OPCODE_STANDARD_QUERY;
                pTX->Flags_1.s.RD       = DNS_RD_RECURSION;
                pTX->wQD_Count          = htons(1);                             // we always only ask one question

                // Extract each string segment from the Domain Name
                do
                {
                    pStr = LIB_strchr(pDomainName, '.');                        // find the first '.'

                    if(pStr != nullptr)                                            // Get size of the segment
                    {
                        wLen = (int16_t)(pStr - pDomainName);
                    }
                    else
                    {
                        wLen = (int16_t)LIB_strlen(pDomainName);
                    }
                    *pQuery = wLen;                                             // Put size of this segment in DNS message
                    pQuery++;
                    LIB_memcpy(pQuery, pDomainName, wLen);                      // Copy segment in DNS message
                    pDomainName += (wLen + 1);
                    pQuery += (int32_t)wLen;
                }
                while(pStr != nullptr);

                pQuery++;                                                       // End of this name
                *((int16_t*)pQuery) = htons(0x0001);                               // QTYPE
                pQuery += 2;
                *((int16_t*)pQuery) = htons(0x0001);                               // QCLASS
                pQuery += 2;

                wLen = (int16_t)(pQuery - &pTX->baData[0]);
                if(SOCK_SendTo(SocketNumber, (uint8_t*)pTX, (int16_t)(sizeof(DNS_Msg_t) - (500 - wLen)), IP_GetDNS_IP(), DNS_PORT) != 0)
                {
                    dwIP = DNS_Response(SocketNumber);
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
    else
    {
        *pError = ERR_STRING_TO_LONG;
    }

    return(dwIP);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DNS_Response
//
//  Parameter(s):   void
//  Return:         int32_t   dwIP        Resolve IP
//
//  Description:    This Function process the answer to the DNS Request
//
//  Note(s):        No special treatment here, get the first IP and get out
//
//-------------------------------------------------------------------------------------------------
int32_t DNS_Response(SOCKET SocketNumber)
{
    DNS_Msg_t*      pRX             = nullptr;
    uint8_t*           pAnswer;
    uint32_t          ServerAddr;
    int16_t            wServerPort;
    uint8_t            Error;
    int32_t           dwIP            = DNS_NO_IP;
    int16_t            wLen;


    OSTimeDly(200);
    do
    {
        if(SOCK_GetRX_RSR(SocketNumber) > 0)
        {
            pRX = MemGetAndClear(sizeof(DNS_Msg_t), &Error);
            if(pRX != nullptr)
            {
                pAnswer = (uint8_t*)&pRX->baData[0];
                SOCK_ReceivedFrom(SocketNumber, (uint8_t*)pRX, sizeof(DNS_Msg_t), &ServerAddr.dw, &wServerPort);

//                if(DNS_wID == pRX->wID)
                {
                    pRX->wQD_Count = ntohs(pRX->wQD_Count);         // bring count to host endian
                    do                                              // Skip 'Question' block
                    {
                        wLen = LIB_strlen(pAnswer);                 // skip domain name
                        pAnswer += (wLen + 5);                      // skip 'end of this name' + QTYPE + QCLASS
                        pRX->wQD_Count--;                           // decrement the query count
                    }
                    while(pRX->wQD_Count != 0);

                    IP_Status.b.DNS_IP_Found = NO;

                    pRX->wAN_Count = ntohs(pRX->wAN_Count);         // bring count to host endian
                    do                                              // Now parse the 'Answer' block for the first IP (type A,  class IN)
                    {
                        // Skip Domain Name
                        do
                        {
                            if(*pAnswer != 0xC0)
                            {
                                wLen     = *pAnswer;                // Get Size of the string segment
                                pAnswer += (wLen + 1);              // Skip to next segment, if any
                            }
                        }
                        while((*pAnswer != 0x00) && (*pAnswer != 0xC0));

                        if((*pAnswer & 0xC0) == 0xC0)               // pointer is used, so skip 2
                        {
                            pAnswer += 2;
                        }

                        // Check if it is the right type
                        if(*((int16_t*)pAnswer) == ntohs(DNS_TYPE_A))
                        {
                            pAnswer += 2;
                            if(*((int16_t*)pAnswer) == ntohs(DNS_CLASS_IN))
                            {
                                IP_Status.b.DNS_IP_Found = YES;
                            }
                            pAnswer += 2;
                        }
                        else
                        {
                            pAnswer += 4;
                        }

                        pAnswer += 4;                               // Skip the TTL field
                        wLen     = ntohs(*((int16_t*)pAnswer));        // get the lenght of the data field
                        pAnswer += 2;

                        if((wLen == 4) && (IP_Status.b.DNS_IP_Found == YES))
                        {
                            dwIP = ntohl(*((int32_t*)pAnswer));
                        }
                        else
                        {
                            pAnswer += wLen;                            // Skip RDATA field
                            pRX->wAN_Count--;                           // decrement the answer count
                        }
                    }
                    while((pRX->wAN_Count != 0) && (IP_Status.b.DNS_IP_Found == NO));

                    // we dont use the 'Authority nameservers' or 'Additional records'
                }
            }
        }
    } while(pRX == nullptr);

    MemPut(&pRX);

    return(dwIP);
}

//-------------------------------------------------------------------------------------------------
