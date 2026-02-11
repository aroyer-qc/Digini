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
#define DNS_RESPONSE_TIME_OUT           10000
#define DNS_LABEL_POINTER_FLAG          0xC0
#define DNS_LABEL_END                   0x00
#define DNS_HEADER_SIZE                 12

// TYPE
#define DNS_TYPE_A                      1   				// The ARPA Internet
#define DNS_CLASS_IN                	1                   // The Internet
#define DNS_RECEIVE_DATA_LENGTH         4                   // IPv4 address length

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
    m_pContext         = pContext;
    m_pCallbackContext = nullptr;
    m_pCallback        = nullptr;
    m_pSocket          = nullptr;
    m_LastID           = 0;
    m_State            = DNS_STATE_IDLE;
    nOS_TimerCreate(&m_TimerQuery, nullptr, nullptr, DNS_RESPONSE_TIME_OUT, NOS_TIMER_ONE_SHOT);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   None
//
//  Return:         bool
//                      - true  : DNS transaction has completed (success or timeout)
//                      - false : DNS transaction is still in progress
//
//  Description:    Advances the DNS client state machine. This function performs a single,
//                  non-blocking receive attempt using the socket’s zero-copy RecvFrom() API and
//                  checks whether the query timeout has expired. It must be called periodically
//                  by the network task.
//
//                  When a valid DNS response is received, the client parses the payload directly
//                  from the returned packet buffer (no memcpy). The resolved IPv4 address is
//                  stored in m_ResolvedIP and the user callback (if provided) is invoked.
//
//                  If the timeout expires before a response is received, the callback is invoked
//                  with a failure status. In both success and timeout cases, the function returns
//                  true to signal completion. The caller is responsible for freeing the received
//                  packet message after processing.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::Process(void)
{
    if(m_State != DNS_STATE_WAIT_RESPONSE)
    {
        return true;
    }

    if(nOS_TimerIsRunning(&m_TimerQuery) == false)
    {
        m_State = DNS_STATE_TIMEOUT;

        if(m_pCallback != nullptr)
        {
            m_pCallback(m_pCallbackContext, false, IP_ADDRESS(0,0,0,0));
        }

        return true;
    }

    IP_PacketMsg_t* pMsg = nullptr;                                 // Zero-copy receive
    SystemState_e State = m_pSocket->RecvFrom(&pMsg);

    if(State != SYS_READY)
    {
        return false;                                               // No packet in this loop
    }

    // Access payload directly
    uint8_t* pPayload = pMsg->Payload;
    size_t   Length   = pMsg->PayloadSize;

    bool Done = false;

    if(Length >= DNS_HEADER_SIZE)
    {
        if(ParseResponse((DNS_Header_t*)pPayload, Length))
        {
            m_State = DNS_STATE_RESPONSE_RECEIVED;
            nOS_TimerStop(&m_TimerQuery, true);

            if(m_pCallback != nullptr)
            {
                m_pCallback(m_pCallbackContext, true, m_ResolvedIP);
            }

            Done = true;
        }
    }

    // Caller frees the packet
    IP_Manager::FreeMessage(pMsg);
    return Done;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Resolve
//
//  Parameter(s):   const char*       pDomainName     Domain name to resolve
//                  DNS_Callback_t    pCallback       Callback invoked when resolution completes
//
//  Return:         bool                                true if the DNS query was successfully
//                                                      initiated (not resolved yet)
//
//  Description:    Initiates an asynchronous DNS resolution. This function allocates a UDP
//                  socket, builds and sends a DNS query to the configured DNS server, stores the
//                  user callback, clears any previous result, and starts the internal timeout
//                  timer.
//
//                  This function does NOT wait for the response. The DNS transaction continues
//                  inside Process(), which will invoke the callback upon success or timeout.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::Resolve(const char* pDomainName)
{
    if(pDomainName == nullptr)
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

  #if (IP_DBG_DNS == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: Ready to send Request\n");
  #endif

    if(SendQuery(pDomainName) == false)
    {
        pSocketManager->FreeSocket(&m_pSocket);
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendQuery
//
//  Parameter(s):   const char*     pDomainName           Domain name to encode into the DNS query.
//
//  Return:         bool    true  = DNS query was successfully handed off to the UDP layer
//                              (either transmitted immediately or queued pending ARP)
//                          false = DNS query could not be sent or queued
//
//  Description:    Builds a DNS query message into a temporary TX buffer and attempts to send it
//                  to the active DNS server using the UDP socket. This function does not wait for
//                  a reply; it only initiates transmission.
//
//                  If the destination MAC address is not yet known, the UDP layer may queue the
//                  packet while ARP resolution is in progress. In that case, the function still
//                  returns true so the DNS state machine can begin waiting for the response.
//
//                  On success (immediate send or queued), the DNS client enters WAIT_RESPONSE
//                  state and starts the query timeout timer.
//-------------------------------------------------------------------------------------------------
bool DNS_Client::SendQuery(const char* pDomainName)
{
    DNS_Header_t* pTX = (DNS_Header_t*)pMemoryPool->AllocAndClear(sizeof(DNS_Header_t), MEM_DBG_DNSTX);

    if(pTX == nullptr)
    {
      #if (IP_DBG_DNS == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: Failed to allocate TX buffer\n");
      #endif
        return false;
    }

    size_t Length = BuildDNS_Query(pTX, pDomainName);

    SocketInfo_t Destination;
    Destination.Address = m_pContext->GetActiveDNS_IP();
    Destination.Port    = DNS_PORT;

    size_t        BytesSent = 0;
    SystemState_e State     = m_pSocket->SendTo((uint8_t*)pTX, Length, &Destination, &BytesSent);

    bool Status = false;

    if((State == SYS_READY) && (BytesSent == Length))
    {
        Status = true;   // sent immediately
    }
    else if(State == SYS_ARP_RESOLVE_PENDING)
    {
        Status = true;   // queued pending ARP resolution
    }

    if(Status == true)
    {
        m_State = DNS_STATE_WAIT_RESPONSE;
        m_ResolvedIP = 0;                                                                                   // Clear previous result
        nOS_TimerStart(&m_TimerQuery);
      #if (IP_DBG_DNS == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: Query dispatched (Len=%u)\n", (unsigned)Length);
      #endif
    }
  #if (IP_DBG_DNS == DEF_ENABLED)
    else
    {
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: 'SendTo' failed (State=%d, Sent=%u)\n", State, (unsigned)BytesSent);
    }
  #endif

    pMemoryPool->Free((void**)&pTX);
    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseResponse
//
//  Parameter(s):   DNS_Header_t*   pMsg            Pointer to the received DNS message buffer
//                  size_t          PacketLength    Total number of bytes received
//
//  Return:         bool                              true if a valid 'IPv4 A' record was found
//
//  Description:    Parses a DNS response message. This function validates the transaction ID,
//                  skips the question section, and iterates through the answer records to locate
//                  the first valid 'IPv4 A' record. When found, the resolved address is written
//                  directly into m_ResolvedIP. The function returns true only when a 'valid A'
//                  record is extracted.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::ParseResponse(DNS_Header_t* pMessage, size_t PacketLength)
{
    DNS_Header_t* pHeader = pMessage;

    if(pHeader->ID != m_LastID)                                             // Validate transaction ID
    {
        return false;                                                       // Not our response
    }

    uint16_t QuestionCount = ntohs(pHeader->QDCount);
    uint16_t AnswerCount   = ntohs(pHeader->ANCount);

    uint8_t* pRead = (uint8_t*)pMessage;
    pRead += DNS_HEADER_SIZE;                                               // Skip fixed header

    while(QuestionCount--)                                                  // Skip Question Section
    {
        while(*pRead != DNS_LABEL_END)                                      // Skip domain name (label sequence)
        {
            uint8_t LabelLength = *pRead;
            pRead += (LabelLength + 1);
        }

        pRead += (1 + sizeof(uint16_t) + sizeof(uint16_t));                 // Skip terminating zero + QTYPE + QCLASS
    }

    while(AnswerCount--)                                                    // Parse Answer Section
    {
        if((*pRead & DNS_LABEL_POINTER_FLAG) == DNS_LABEL_POINTER_FLAG)     // Name (pointer or full label)
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

        uint16_t Type = ntohs(*(uint16_t*)pRead);
        pRead += sizeof(uint16_t);
        uint16_t Class = ntohs(*(uint16_t*)pRead);
        pRead += sizeof(uint16_t);
        pRead += sizeof(uint32_t);                                          // Skip TTL
        uint16_t DataLength = ntohs(*(uint16_t*)pRead);
        pRead += sizeof(uint16_t);

        if((Type == DNS_TYPE_A) && (Class == DNS_CLASS_IN) && (DataLength == DNS_RECEIVE_DATA_LENGTH))
        {
            uint32_t RawIP;
            memcpy(&RawIP, pRead, sizeof(uint32_t));
            m_ResolvedIP = RawIP;
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
//  Parameter(s):   DNS_Header_t*      pMessage        Pointer to output buffer for DNS message
//                  const char*        pDomainName     Domain name to encode
//
//  Return:         size_t                             Total size of the encoded DNS query
//
//  Description:    Construct a DNS query message in the provided buffer. The function encodes
//                  the DNS header, formats the domain name into DNS label format, and appends
//                  the QTYPE and QCLASS fields. This function performs no memory allocation.
//
//-------------------------------------------------------------------------------------------------
size_t DNS_Client::BuildDNS_Query(DNS_Header_t* pMessage, const char* pDomainName)
{
    m_LastID         = (uint16_t)RNG_GetRandom();
    pMessage->ID      = htons(m_LastID);
    pMessage->Flags   = htons(DNS_FLAG_RD_RECURSION_DESIRED);   // Recursion desired
    pMessage->QDCount = htons(1);
    pMessage->ANCount = 0;
    pMessage->NSCount = 0;
    pMessage->ARCount = 0;

    uint8_t*    pWrite    = &pMessage->Payload[0];
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

    return (size_t)(DNS_HEADER_SIZE + (pWrite - &pMessage->Payload[0]));
}

//-------------------------------------------------------------------------------------------------

#endif //(IP_USE_DNS == DEF_ENABLED)
