//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_dns.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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
//    |                      xID                      |
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
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+           <QUESTION FORMAT>
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
#define DNS_RESPONSE_TIME_OUT           1000
#define DNS_LABEL_POINTER_FLAG          0xC0
#define DNS_LABEL_END                   0x00
#define DNS_HEADER_SIZE                 12

// TYPE

#define DNS_TYPE_A                      HTONS(1)			// The ARPA Internet
#define DNS_CLASS_IN                	HTONS(1)            // The Internet
#define DNS_RECEIVE_DATA_LENGTH         4                   // IPv4 address length

// Flag QR
#define DNS_FLAG_QR_QUERY               0x0000              // Query
#define DNS_FLAG_QR_RESPONSE            0x8000              // Response

// Flag OPCODE
#define DNS_FLAG_OPCODE_STANDARD        0x0000              // Standard query (0)
#define DNS_OPCODE_STATUS           	2                   // Server status request

//Flag RD
#define DNS_FLAG_RD_RECURSION_DESIRED   HTONS(0x0100)       // Recursion Desired

#define DNS_QDCOUNT_1                   HTONS(1)

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext*     pContext        Pointer on the context
//  Return:         None
//
//  Description:    Initialize the DNS Client. Allocates a single UDP socket used for all DNS
//                  transactions and clears the pending-request table.
//
//-------------------------------------------------------------------------------------------------
void DNS_Client::Initialize(NetworkContext* pContext)
{
    m_pContext    = pContext;

    // Start XID counter at a random value (never 0)
    m_XID_Counter = (uint16_t)RNG_GetRandom();

    // Clear pending request table
    for(int i = 0; i < DNS_MAX_PENDING_COUNT; i++)
    {
        m_Pending[i].Pending   = false;
        m_Pending[i].XID       = 0;
        m_Pending[i].pCallback = nullptr;
        m_Pending[i].pContext  = nullptr;
        m_Pending[i].TimeStamp = 0;
    }

    // Allocate the DNS UDP socket once
    SocketManager* pSocketManager = m_pContext->GetIP_Manager()->GetSocketManager();

    if(pSocketManager != nullptr)
    {
        m_pSocket = pSocketManager->AllocSocket(SOCKET_TYPE_DATAGRAM);

        if(m_pSocket != nullptr)
        {
            bool NonBlocking = true;
            m_pSocket->SetOption(SOCKET_OPT_NON_BLOCKING, &NonBlocking, sizeof(bool));

            // Bind to ephemeral port
            if(m_pSocket->Bind(0) != SYS_READY)
            {
                pSocketManager->FreeSocket(&m_pSocket);
                m_pSocket = nullptr;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   None
//
//  Return:         bool    - true  : At least one DNS transaction completed (success or timeout)
//                          - false : No DNS transaction completed in this cycle
//
//  Description:    Advances the DNS client. This function performs:
//
//                  1- Timeout checks for all pending DNS requests using the system-wide
//                     GetTick() / TickHasTimeOut() mechanism.
//                  2- A single non-blocking receive attempt using the socket’s zero-copy API.
//                  3- If a DNS response is received, it matches the XID to the correct pending
//                     request, parses the response, and invokes the associated callback.
//
//  Note(s):        Must be called periodically by the network task.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::Process(void)
{
    bool Completed = false;

    // Timeout checks for all pending DNS requests
    for(int i = 0; i < DNS_MAX_PENDING_COUNT; i++)
    {
        if(m_Pending[i].Pending == true)
        {
            if(TickHasTimeOut(m_Pending[i].TimeStamp, DNS_RESPONSE_TIME_OUT))
            {
              #if (IP_DBG_DNS == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: TIMEOUT XID=0x%04X\n",  m_Pending[i].XID);
              #endif

                // Timeout -> notify caller
                if(m_Pending[i].pCallback != nullptr)
                {
                    m_Pending[i].pCallback(m_Pending[i].pContext, false, IP_ADDRESS(0,0,0,0));
                }

                m_Pending[i].Pending = false;
                Completed = true;
            }
        }
    }

    // Attempt a single non-blocking receive
    IP_PacketMsg_t* pMsg = nullptr;
    SystemState_e State = m_pSocket->RecvFrom(&pMsg);

    if(State != SYS_READY)
    {
        return Completed;      // No packet received this cycle
    }

    // Parse DNS response
    uint8_t* pPayload = pMsg->Payload;
    size_t   Length   = pMsg->PayloadSize;

    if(Length >= DNS_HEADER_SIZE)
    {
        DNS_Header_t* pDNS = (DNS_Header_t*)pPayload;
        uint16_t xID = ntohs(pDNS->ID);

      #if (IP_DBG_DNS == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: RX response xid=0x%04X\n", xID);
      #endif

        int slot = FindSlotByXID(xID);

        if(slot >= 0)
        {
            IP_Address_t ResolvedIP;
            bool Success = ParseResponse(pDNS, Length, ResolvedIP);

          #if (IP_DBG_DNS == DEF_ENABLED)
            if(Success == true)
            {
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: RESOLVED -> %d.%d.%d.%d\n", IP_A(ResolvedIP),
                                                                                                 IP_B(ResolvedIP),
                                                                                                 IP_C(ResolvedIP),
                                                                                                 IP_D(ResolvedIP));
            }
            else
            {
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: INVALID respons\n");
            }
          #endif

            if(m_Pending[slot].pCallback != nullptr)
            {
                m_Pending[slot].pCallback(m_Pending[slot].pContext, Success, ResolvedIP);
            }

            m_Pending[slot].Pending = false;
            Completed = true;
        }
      #if (IP_DBG_DNS == DEF_ENABLED)
        else
        {
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "DNS: RX xid=0x%04X but no matching pending slot\n", xID);
        }
      #endif
    }

    // Caller frees the packet
    IP_Manager::FreeMessage(pMsg);

    return Completed;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendQuery
//
//  Parameter(s):   const char*       pDomainName     Domain name to encode into the DNS query.
//                  DNS_Callback_t    pCallback       User callback for this specific request.
//                  void*             pContext        User context passed back to callback.
//
//  Return:         bool
//                      true  = DNS query was successfully sent or queued (ARP pending)
//                      false = Could not allocate slot, build packet, or send
//
//  Description:    Initiates a DNS query. Allocates a pending-request slot, generates a unique
//                  XID, builds the DNS query message, timestamps the request, and sends the
//                  packet using the preallocated UDP socket.
//
//                  This function does NOT wait for a reply. Completion is handled in Process().
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::SendQuery(const char* pDomainName, DNS_Callback_t pCallback, void* pContext)
{
    if((pDomainName == nullptr) || (m_pSocket == nullptr))
    {
        return false;
    }

    // Find a free pending slot
    int slot = FindFreeSlot();

    if(slot < 0)
    {
        return false;                                       // Too many outstanding DNS requests
    }

    // Allocate TX buffer
    DNS_Header_t* pTX = (DNS_Header_t*)pMemoryPool->AllocAndClear(sizeof(DNS_Header_t), MEM_DBG_DNSTX);

    if(pTX == nullptr)
    {
        return false;
    }

    m_XID_Counter++;                                        // Generate XID and store pending request info
    m_Pending[slot].XID       = htons(m_XID_Counter);
    m_Pending[slot].pCallback = pCallback;
    m_Pending[slot].pContext  = pContext;
    m_Pending[slot].TimeStamp = GetTick();
    m_Pending[slot].Pending   = true;

    size_t Length = BuildDNS_Query(pTX, pDomainName);       // Build DNS query packet

    // Send packet to DNS server
    SocketInfo_t Destination;
    Destination.Address = m_pContext->GetActiveDNS_IP();
    Destination.Port    = DNS_PORT;

    size_t        BytesSent = 0;
    SystemState_e State     = m_pSocket->SendTo((uint8_t*)pTX, Length, &Destination, &BytesSent);

    bool Status = false;

    if((State == SYS_READY) && (BytesSent == Length))
    {
        Status = true;                                      // Sent immediately
    }
    else if(State == SYS_ARP_RESOLVE_PENDING)
    {
        Status = true;                                      // Queued while ARP resolves
    }

    pMemoryPool->Free((void**)&pTX);                        // Cleanup

    if(Status == false)
    {
        m_Pending[slot].Pending = false;                    // Sending failed -> cancel pending slot
    }

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseResponse
//
//  Parameter(s):   DNS_Header_t*   pMessage        Pointer to the received DNS message buffer
//                  size_t          PacketLength    Total number of bytes received
//                  IP_Address_t&   OutIP           Resolved IPv4 address (output)
//
//  Return:         bool            - true  : A valid IPv4 'A' record was found and OutIP is set
//                                  - false : No valid A record found
//
//  Description:    Parses a DNS response message. The caller is responsible for validating the
//                  transaction ID (XID) and selecting the correct pending-request slot.
//
//                  This function skips the Question section, then iterates through all Answer
//                  records. When the first valid IPv4 'A' record is found, the resolved address
//                  is written into OutIP and the function returns true.
//
//-------------------------------------------------------------------------------------------------
bool DNS_Client::ParseResponse(DNS_Header_t* pMessage, size_t PacketLength, IP_Address_t& OutIP)
{
    DNS_Header_t* pHeader = pMessage;

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
        if((*pRead & DNS_LABEL_POINTER_FLAG) == DNS_LABEL_POINTER_FLAG)     // Skip NAME (pointer or full label)
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

        uint16_t Type = *(uint16_t*)pRead;
        pRead += sizeof(uint16_t);
        uint16_t Class = *(uint16_t*)pRead;
        pRead += sizeof(uint16_t);
        pRead += sizeof(uint32_t);                                          // Skip TTL
        uint16_t DataLength = ntohs(*(uint16_t*)pRead);
        pRead += sizeof(uint16_t);

        // Found IPv4 A record
        if((Type == DNS_TYPE_A) && (Class == DNS_CLASS_IN) && (DataLength == DNS_RECEIVE_DATA_LENGTH))
        {
            memcpy(&OutIP, pRead, sizeof(uint32_t));
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
//  Description:    Constructs a DNS query message in the provided buffer. The function generates
//                  a new transaction ID, encodes the DNS header, formats the domain name into
//                  DNS label format, and appends QTYPE and QCLASS. No memory allocation occurs.
//
//-------------------------------------------------------------------------------------------------
size_t DNS_Client::BuildDNS_Query(DNS_Header_t* pMessage, const char* pDomainName)
{
    pMessage->ID      = m_XID_Counter;
    pMessage->Flags   = DNS_FLAG_RD_RECURSION_DESIRED;      // Recursion desired
    pMessage->QDCount = DNS_QDCOUNT_1;
    pMessage->ANCount = 0;
    pMessage->NSCount = 0;
    pMessage->ARCount = 0;

    uint8_t*    pWrite   = &pMessage->Payload[0];
    const char* pSegment = pDomainName;

    while(*pSegment)                                            // Encode domain name into DNS label format
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

    *pWrite++ = DNS_LABEL_END;                                  // End of name

    *((uint16_t*)pWrite) = DNS_TYPE_A;                          // QTYPE (A)
    pWrite += sizeof(uint16_t);
    *((uint16_t*)pWrite) = DNS_CLASS_IN;                        // QCLASS (IN)
    pWrite += sizeof(uint16_t);

    return (size_t)(DNS_HEADER_SIZE + (pWrite - &pMessage->Payload[0]));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindFreeSlot
//
//  Parameter(s):   None
//
//  Return:         int                 - Index of the first available pending-request slot
//                                      - -1 if no slot is available
//
//  Description:    Scans the DNS pending-request table and returns the index of the first entry
//                  that is not currently in use. This function supports multiple outstanding DNS
//                  transactions by allowing each request to occupy a dedicated slot.
//
//-------------------------------------------------------------------------------------------------
int DNS_Client::FindFreeSlot(void)
{
    for(int i = 0; i < DNS_MAX_PENDING_COUNT; i++)
    {
        if(m_Pending[i].Pending == false)
        {
            return i;
        }
    }

    return -1;      // No free slot
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindSlotByXID
//
//  Parameter(s):   uint16_t        XID             Transaction ID extracted from DNS response
//
//  Return:         int             - Index of the matching pending-request slot
//                                  - -1 if no matching slot is found
//
//  Description:    Searches the DNS pending-request table for an entry whose stored transaction
//                  ID matches the provided XID. This function is used by Process() to associate
//                  an incoming DNS response with the correct outstanding query.
//
//-------------------------------------------------------------------------------------------------

int DNS_Client::FindSlotByXID(uint16_t XID)
{
    for(int i = 0; i < DNS_MAX_PENDING_COUNT; i++)
    {
        if(m_Pending[i].Pending == true)
        {
            if(m_Pending[i].XID == XID)
            {
                return i;
            }
        }
    }

    return -1;
}

//-------------------------------------------------------------------------------------------------

#endif //(IP_USE_DNS == DEF_ENABLED)
