//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_dns.cpp
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
//
//    DNS - Domain Name System
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
//    |                     QNAME                     |
//    |                                               |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QTYPE                     |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    |                     QCLASS                    |
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (IP_USE_DNS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DNS_PORT                        53
#define DNS_MAX_PACKET_SIZE             512
#define DNS_RESPONSE_TIME_OUT           1000
#define DNS_LABEL_POINTER_FLAG          0xC0
#define DNS_LABEL_END                   0x00

// TYPE
#define DNS_TYPE_A                      1   				// The ARPA Internet
#define DNS_CLASS_IN                	1                   // The Internet
#define DNS_RECEIVE_DATA_LENGHT         4                   // IPv4 address length

// Flag QR
#define DNS_FLAG_QR_QUERY               0x0000              // Query
#define DNS_FLAG_QR_RESPONSE            0x8000              // Response

// Flag OPCODE
#define DNS_FLAG_OPCODE_STANDARD        0x0000              // Standard query (0)
#define DNS_OPCODE_STATUS           	2                   // Server status request

//Flag RD
#define DNS_FLAG_RD_RECURSION_DESIRED   0x0100              // Recursion Desired

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext*		pContext		Pointer on the context
//  Return:         None
//
//  Description:    Initialize the DNS Client
//
//-------------------------------------------------------------------------------------------------
void DNS_Client::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;
    m_pSocket  = nullptr;
    m_LastID   = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Resolve
//
//  Parameter(s):   const char*     pDomainName     Domain name to resolve
//                  IP_Address_t*   pOutIP          Pointer to store the resolved IP address
//
//  Return:         bool                            true if resolution succeeded
//
//  Description:    Resolve a domain name using DNS. This function sends a DNS query to the
//                  configured DNS server, waits for the response, parses the first valid A
//                  record, and returns the resolved IPv4 address.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::Resolve(const char* pDomainName, IP_Address_t* pOutIP)
{
    if((pDomainName == nullptr) || (pOutIP == nullptr))
    {
        return false;
    }

    SocketManager* pSocketManager = m_pContext->GetIP_Manager()->GetSocketManager();                    // Get SocketManager once, locally

    if(pSocketManager == nullptr)
    {
        return false;
    }

    m_pSocket = pSocketManager->AllocSocket(SOCKET_TYPE_DATAGRAM);                                      // Allocate UDP socket

    if(m_pSocket == nullptr)
    {
        return false;
    }

    bool NonBlocking = true;
    m_pSocket->SetOption(SOCKET_OPT_NON_BLOCKING, &NonBlocking, sizeof(bool));

    if(m_pSocket->Bind(0) != SYS_READY)                                                                 // Bind to ephemeral port
    {
        pSocketManager->FreeSocket(&m_pSocket);
        return false;
    }

    if(SendQuery(pDomainName) == false)
    {
        pSocketManager->FreeSocket(&m_pSocket);
        return false;
    }

    bool Result = ReceiveResponse(pOutIP);
    pSocketManager->FreeSocket(&m_pSocket);
    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendQuery
//
//  Parameter(s):   const char*        pDomainName     Domain name to encode into the DNS query
//
//  Return:         bool                                true if the query was sent successfully
//
//  Description:    Build and transmit a DNS query message to the active DNS server using UDP.
//                  The function does not wait for a response; it only handles packet creation
//                  and transmission.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::SendQuery(const char* pDomainName)
{
    uint8_t Packet[DNS_MAX_PACKET_SIZE];
    size_t  Length = BuildDNS_Query(Packet, pDomainName);

    SocketInfo_t Destination;
    Destination.Address = m_pContext->GetActiveDNS_IP();
    Destination.Port    = DNS_PORT;
    size_t BytesSent    = 0;
    SystemState_e State = m_pSocket->SendTo(Packet, Length, &Destination, &BytesSent);
    return ((State == SYS_READY) && (BytesSent == Length));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReceiveResponse
//
//  Parameter(s):   IP_Address_t*      pOutIP          Pointer to store the resolved IPv4 address
//
//  Return:         bool                                true if a valid DNS response was received
//
//  Description:    Poll the UDP socket for a DNS response. If a valid response is received,
//                  the function forwards the packet to the DNS parser. A timeout mechanism is
//                  used to avoid blocking indefinitely.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::ReceiveResponse(IP_Address_t* pOutIP)
{
    uint8_t      Buffer[DNS_MAX_PACKET_SIZE];
    size_t       BytesReceived = 0;
    SocketInfo_t Source;

    // Poll for up to ~1 second (1000 × 1ms)
    for(int i = 0; i < DNS_RESPONSE_TIME_OUT; i++)
    {
        SystemState_e State = m_pSocket->RecvFrom(Buffer, sizeof(Buffer), &Source, &BytesReceived);

        if((State == SYS_READY) && (BytesReceived >= sizeof(DNS_Header_t)))
        {
            return ParseResponse(Buffer, BytesReceived, pOutIP);
        }

        nOS_Sleep(1);
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseResponse
//
//  Parameter(s):   uint8_t*           pPacket         Pointer to the received DNS packet
//                  size_t             Length          Length of the received packet
//                  IP_Address_t*      pOutIP          Pointer to store the resolved IPv4 address
//
//  Return:         bool                                true if a valid A record was found
//
//  Description:    Parse a DNS response packet. The function validates the header, skips the
//                  question section, iterates through the answer records, and extracts the first
//                  IPv4 address (A record) if present.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::ParseResponse(uint8_t* pPacket, size_t PacketLength, IP_Address_t* pOutIP)
{
    DNS_Header_t* pHeader = (DNS_Header_t*)pPacket;

    if(pHeader->ID != m_LastID)                                             // Validate transaction ID
    {
        return false;                                                       // Not our response
    }

    uint16_t QuestionCount = ntohs(pHeader->QDCount);
    uint16_t AnswerCount   = ntohs(pHeader->ANCount);
    uint8_t* pRead         = pPacket + sizeof(DNS_Header_t);

    while(QuestionCount--)                                                  // Skip Question Section
    {
        while(*pRead != DNS_LABEL_END)                                      // Skip domain name (label sequence)
        {
            uint8_t LabelLength = *pRead;
            pRead += (LabelLength + 1);
        }

        pRead += (1 + sizeof(uint16_t) + sizeof(uint16_t));                 // Skip null + QTYPE + QCLASS
    }

    while(AnswerCount--)                                                    // Parse Answer Section
    {
        if((*pRead & DNS_LABEL_POINTER_FLAG) == DNS_LABEL_POINTER_FLAG)     // Skip Name (either pointer or full label)
        {
            pRead += 2;                                                     // Pointer is always 2 bytes
        }
        else
        {
            while(*pRead != DNS_LABEL_END)
            {
                uint8_t LabelLength = *pRead;
                pRead += (LabelLength + 1);
            }

            pRead++;                                                        // Skip terminating zero
        }

        uint16_t Type = ntohs(*(uint16_t*)pRead);                           // Read Type
        pRead += sizeof(uint16_t);                                          // Type (2 bytes)
        uint16_t Class = ntohs(*(uint16_t*)pRead);                          // Read Class
        pRead += (sizeof(uint16_t) + sizeof(uint32_t));                     // Class (2 bytes) + Skip TTL (4 bytes)
        uint16_t DataLength = ntohs(*(uint16_t*)pRead);                     // Read DataLength
        pRead += sizeof(uint16_t);                                          // DataLenght (2 bytes)

        if((Type == DNS_TYPE_A) && (Class == DNS_CLASS_IN) && (DataLength == DNS_RECEIVE_DATA_LENGHT))
        {                                                                   // Check for IPv4 A record
            uint32_t RawIP;
            memcpy(&RawIP, pRead, sizeof(uint32_t));
            *pOutIP = ntohl(RawIP);
            return true;
        }

        pRead += DataLength;                                                // Skip RDATA for non-A records
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildDNS_Query
//
//  Parameter(s):   uint8_t*           pOut            Pointer to output buffer for DNS message
//                  const char*        pDomainName     Domain name to encode
//
//  Return:         size_t                             Total size of the encoded DNS query
//
//  Description:    Construct a DNS query message in the provided buffer. The function encodes
//                  the DNS header, formats the domain name into DNS label format, and appends
//                  the QTYPE and QCLASS fields. This function performs no memory allocation.
//
//-------------------------------------------------------------------------------------------------
size_t DNS_Client::BuildDNS_Query(uint8_t* pOut, const char* pDomainName)
{
    DNS_Header_t* pHeader = (DNS_Header_t*)pOut;

    m_LastID         = (uint16_t)RNG_GetRandom();
    pHeader->ID      = htons(m_LastID);
    pHeader->Flags   = htons(DNS_FLAG_RD_RECURSION_DESIRED);   // Recursion desired
    pHeader->QDCount = htons(1);
    pHeader->ANCount = 0;
    pHeader->NSCount = 0;
    pHeader->ARCount = 0;

    uint8_t*    pWrite    = pOut + sizeof(DNS_Header_t);
    const char* pSegment  = pDomainName;

    while(*pSegment)
    {
        const char* pDot = strchr(pSegment, '.');
        size_t SegmentLength = (pDot != nullptr) ? (size_t)(pDot - pSegment) : strlen(pSegment);
        *pWrite++ = (uint8_t)SegmentLength;
        memcpy(pWrite, pSegment, SegmentLength);
        pWrite += SegmentLength;

        if(pDot == nullptr)
        {
            break;
        }

        pSegment = pDot + 1;
    }

    *pWrite++ = DNS_LABEL_END;                 // End of name

    *((uint16_t*)pWrite) = htons(DNS_TYPE_A);
    pWrite += sizeof(uint16_t);

    *((uint16_t*)pWrite) = htons(DNS_CLASS_IN);
    pWrite += sizeof(uint16_t);

    return (size_t)(pWrite - pOut);
}

//-------------------------------------------------------------------------------------------------

#endif //(IP_USE_DNS == DEF_ENABLED)
