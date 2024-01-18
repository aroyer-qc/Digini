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
//  Return:             uint32_t   dwIP
//
//  Description:    Send the DNS Query to DNS Server
//
//-------------------------------------------------------------------------------------------------
uint32_t DNS_Query(SOCKET SocketNumber, uint8_t* pDomainName, uint8_t* pError)
{
    size_t       Len;
    uint16_t     Port;
    DNS_Msg_t*   pTX;
    uint8_t*     pQuery;
    uint8_t*     pStr;
    IP_Address_t IP_Address = DNS_NO_IP;

    if(strlen(pDomainName) <= DNS_MAX_DOMAIN_NAME_LENGHT)                       // Check if domain name string is to long
    {
        pTX = pMemory->AllocAndClear(DHCP_Msg_t);

        if(pTX != nullptr)
        {
            pQuery = (uint8_t*)&pTX->Data[0];
            Port = uint16_t(RNG_GetRandomFromRange(32768, 65535));              // Get a random source port for the query from 32768 to 65535

            if(SOCK_Socket(SocketNumber, Sn_MR_UDP, Port, 0) != 0)
            {

                // Fill up standard info for DNS query Packet
                TIME_Randomize();
                DNS_ID                  =uint16_t(RNG_GetRandom());             // Get a random ID
                pTX->ID                 = DNS_ID;                               // Keep copy for verification in answers
                pTX->Flags_1.s.QR       = DNS_QR_QUERY;                         // This is a DNS query
                pTX->Flags_1.s.OPCODE   = DNS_OPCODE_STANDARD_QUERY;
                pTX->Flags_1.s.RD       = DNS_RD_RECURSION;
                pTX->QD_Count           = htons(1);                             // we always only ask one question

                // Extract each string segment from the Domain Name
                do
                {
                    pStr = LIB_strchr(pDomainName, '.');                        // find the first '.'

                    if(pStr != nullptr)                                         // Get size of the segment
                    {
                        Len = (size_t)(pStr - pDomainName);
                    }
                    else
                    {
                        Len = (size_t)LIB_strlen(pDomainName);
                    }
                    
                    *pQuery = Len;                                              // Put size of this segment in DNS message
                    pQuery++;
                    memcpy(pQuery, pDomainName, Len);                           // Copy segment in DNS message
                    pDomainName += (Len + 1);
                    pQuery += (uint32_t)Len;
                }
                while(pStr != nullptr);

                pQuery++;                                                       // End of this name
                *((uint16_t*)pQuery) = htons(0x0001);                           // QTYPE
                pQuery += 2;
                *((uint16_t*)pQuery) = htons(0x0001);                           // QCLASS
                pQuery += 2;

                Len = (uint16_t)(pQuery - &pTX->Data[0]);
                
                if(SOCK_SendTo(SocketNumber, (uint8_t*)pTX, (uint16_t)(sizeof(DNS_Msg_t) - (500 - Len)), IP_GetDNS_IP(), DNS_PORT) != 0)
                {
                    IP_Address = DNS_Response(SocketNumber);
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

    return IP_Address;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DNS_Response
//
//  Parameter(s):   void
//  Return:         uint32_t   IP           Resolve IP
//
//  Description:    This Function process the answer to the DNS Request
//
//  Note(s):        No special treatment here, get the first IP and get out
//
//-------------------------------------------------------------------------------------------------
uint32_t DNS_Response(SOCKET SocketNumber)
{
    DNS_Msg_t*   pRX             = nullptr;
    uint8_t*     pAnswer;
    IP_Address_t ServerAddr;
    uint16_t     ServerPort;
    uint8_t      Error;
    IP_Address_t IP_Address = DNS_NO_IP;
    size_t     Len;


    OSTimeDly(200); // why?
    
    do
    {
        if(SOCK_GetRX_RSR(SocketNumber) > 0)
        {
            pRX = MemGetAndClear(sizeof(DNS_Msg_t), &Error);
            
            if(pRX != nullptr)
            {
                pAnswer = (uint8_t*)&pRX->Data[0];
                SOCK_ReceivedFrom(SocketNumber, (uint8_t*)pRX, sizeof(DNS_Msg_t), &ServerAddr, &ServerPort);

//                if(DNS_wID == pRX->wID)
                {
                    pRX->QD_Count = ntohs(pRX->QD_Count);           // bring count to host endian
                    do                                              // Skip 'Question' block
                    {
                        Len = LIB_strlen(pAnswer);                  // skip domain name
                        pAnswer += (Len + 5);                       // skip 'end of this name' + QTYPE + QCLASS
                        pRX->QD_Count--;                            // decrement the query count
                    }
                    while(pRX->QD_Count != 0);

                    IP_Status.b.DNS_IP_Found = NO;

                    pRX->AN_Count = ntohs(pRX->AN_Count);           // bring count to host endian
                    do                                              // Now parse the 'Answer' block for the first IP (type A,  class IN)
                    {
                        // Skip Domain Name
                        do
                        {
                            if(*pAnswer != 0xC0)
                            {
                                Len      = *pAnswer;                // Get Size of the string segment
                                pAnswer += (Len + 1);               // Skip to next segment, if any
                            }
                        }
                        while((*pAnswer != 0x00) && (*pAnswer != 0xC0));

                        if((*pAnswer & 0xC0) == 0xC0)               // pointer is used, so skip 2
                        {
                            pAnswer += 2;
                        }

                        // Check if it is the right type
                        if(*((uint16_t*)pAnswer) == ntohs(DNS_TYPE_A))
                        {
                            pAnswer += 2;
                            if(*((uint16_t*)pAnswer) == ntohs(DNS_CLASS_IN))
                            {
                                IP_Status.b.DNS_IP_Found = YES;
                            }
                            pAnswer += 2;
                        }
                        else
                        {
                            pAnswer += 4;
                        }

                        pAnswer += 4;                                   // Skip the TTL field
                        Len      = ntohs(*((uint16_t*)pAnswer));        // get the lenght of the data field
                        pAnswer += 2;

                        if((Len == 4) && (IP_Status.b.DNS_IP_Found == true))
                        {
                            IP = ntohl(*((uint32_t*)pAnswer));
                        }
                        else
                        {
                            pAnswer += Len;                             // Skip RDATA field
                            pRX->AN_Count--;                            // decrement the answer count
                        }
                    }
                    while((pRX->AN_Count != 0) && (IP_Status.b.DNS_IP_Found == false));

                    // Dont use the 'Authority nameservers' or 'Additional records'
                }
            }
        }
    } while(pRX == nullptr);

    MemPut(&pRX);

    return IP;
}

//-------------------------------------------------------------------------------------------------
