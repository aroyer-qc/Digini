//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_arp.cpp
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
//------ Note(s) ----------------------------------------------------------------------------------
//
//  ARP - Address Resolution Protocol
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"


//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define NET_ARP_TMR_INTERVAL                        1000

//-------------------------------------------------------------------------------------------------

void ARP_TimerCallBack(nOS_Timer* pTimer, void* pArg);


volatile void* pPending = nullptr;

//-------------------------------------------------------------------------------------------------
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext*   pContext
//
//  Return:         SystemState_e     SYS_READY on success
//                                    SYS_FAIL  on timer creation/start failure
//
//  Description:    Initializes the ARP subsystem.
//
//                  - Stores the network context pointer.
//                  - Clears all ARP table entries (IP -> MAC mappings).
//                  - Resets the pending ARP resolution state (no pending packet/IP).
//                  - Creates and starts the periodic ARP timer used for aging entries
//                    and driving ARP retry logic.
//
//  Note(s):        This function must be called once during network stack initialization
//                  before any ARP processing occurs.
//-------------------------------------------------------------------------------------------------
SystemState_e ARP_Protocol::Initialize(NetworkContext* pContext)
{
    nOS_Error Error;

    m_pContext       = pContext;
    m_pPendingPacket = nullptr;
    m_PendingIP      = IP_ADDRESS(0,0,0,0);

	// Clear the ARP cache table
	for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
	{
		m_TableEntry[i].IP_Address = IP_ADDRESS(0,0,0,0);
	}

    // Initialize an OS timer for the ARP timer
    Error = nOS_TimerCreate(m_pTimer,
                            &ARP_TimerCallBack,                     // Timer callback function
                            this,                                   // This class object
                            NET_ARP_TMR_INTERVAL,
                            NOS_TIMER_FREE_RUNNING);                // It will repeat indefinitely

	if(Error == NOS_OK)
	{
		Error = nOS_TimerStart(m_pTimer);
	}

    return (Error == NOS_OK) ? SYS_READY : SYS_FAIL;
}

//-------------------------------------------------------------------------------------------------
//  Name:           ProcessIP
//
//  Parameter(s):   IP_PacketMsg_t*   pRX
//
//  Return:         void
//
//  Description:    Learns (IP -> MAC) mappings from inbound IPv4 packets.
//
//                  This function inspects the source IP and source MAC of received IP frames
//                  and updates the ARP table when the traffic originates from a valid host
//                  on the local subnet.
//
//                  Packets are ignored if:
//                      - The interface is not yet configured (IP or subnet mask is zero)
//                      - The destination IP or MAC is multicast
//                      - The source IP is invalid, zero, or equal to our own IP
//                      - The source host is outside the local subnet
//                      - The source MAC is broadcast, multicast, or our own MAC
//                      - The destination MAC is not ours
//
//                  If all filters pass, the sender’s (IP -> MAC) mapping is learned via
//                  UpdateEntry(). This may also resolve a pending ARP request.
//
//  Note(s):        Only learns from hosts on the local network. This function never sends
//                  ARP requests; it only updates the ARP cache based on observed IP traffic.
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::ProcessIP(IP_PacketMsg_t* pRX)
{
    IP_Address_t SubnetMask = m_pContext->GetActiveSubnetMask();
    IP_Address_t ActiveIP   = m_pContext->GetActiveIP();

    if((ActiveIP == IP_ADDRESS(0,0,0,0)) || (SubnetMask == IP_ADDRESS(0,0,0,0)))    // Interface not configured yet -> do not learn from IP traffic
    {
        return;
    }

    IP_Address_t      DestIP   = pRX->pPacket->IP_Frame.Header.DstIP_Address;
    IP_MAC_Address_t* pDstMAC  = &pRX->pPacket->ETH_Header.DestinationMAC;
    IP_Manager* pIP_Manager = m_pContext->GetIP_Manager();


    if(pIP_Manager->IsItMulticast(DestIP))                                          // Ignore multicast IP
    {
        return;
    }

    if(pIP_Manager->IsItMulticastMAC(pDstMAC))                                      //  Ignore multicast MAC destination
    {
        return;
    }

    IP_Address_t      SourceIP = pRX->pPacket->IP_Frame.Header.SrcIP_Address;
    IP_MAC_Address_t* pSrcMAC  = &pRX->pPacket->ETH_Header.SourceMAC;

    if((SourceIP == 0)                                      ||                      // Ignore invalid IPs
       (SourceIP == ActiveIP)                               ||                      // Ignore our own IP
       ((SourceIP & SubnetMask) != (ActiveIP & SubnetMask)) ||                      // Ignore packets outside our subnet
       (pIP_Manager->IsItBroadcastMAC(pSrcMAC))             ||                      // Ignore broadcast MAC
       (pIP_Manager->IsItMulticastMAC(pSrcMAC))             ||                      // Ignore multicast MAC
       (m_pContext->IsItMyMAC_Address(pSrcMAC))             ||                      // Ignore our own MAC
       (m_pContext->IsItMyMAC_Address(pDstMAC) == false))                           // Ignore destination if it is not own MAC
    {
        return;
    }

    // Optional: ignore spoofed packets (MAC/IP mismatch)
    //if(ARP_Table.HasEntry(SourceIP) && (ARP_Table.MatchesMAC(SourceIP, pSrcMAC) == false))
    //{
    //    return; // suspicious -> ignore
    //}

    UpdateEntry(SourceIP, pSrcMAC);                                                 // Passed all filters -> learn entry
}

//-------------------------------------------------------------------------------------------------
//  Name:           ProcessARP
//
//  Parameter(s):   IP_PacketMsg_t*   pRX
//
//  Return:         void
//
//  Description:    Handles inbound ARP frames (requests and replies).
//
//                  - Validates the received ARP frame and extracts the ARP payload.
//
//                  - For ARP_REQUEST:
//                        If the request targets our IP address, a zero‑copy ARP reply is
//                        constructed directly in the received buffer and transmitted.
//
//                  - For ARP_REPLY:
//                        If the reply is directed to our IP address, the sender’s
//                        (IP -> MAC) mapping is learned via UpdateEntry(). Any pending
//                        ARP resolution is also completed by UpdateEntry().
//
//                  - All non‑matching or unsupported ARP opcodes are ignored.
//
//                  The received ARP packet is always freed unless it is reused for
//                  zero‑copy transmission when replying to an ARP request.
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::ProcessARP(IP_PacketMsg_t* pRX)
{
    // Sanity check: must have at least ARP frame (sans ETH header)
    if((pRX == nullptr) || (pRX->pPacket == nullptr) || (pRX->PacketSize < (sizeof(ARP_Frame_t) - sizeof(IP_EthernetHeader_t))))
    {
        IP_Manager::FreeMessage(pRX);
        return;
    }

    ARP_Frame_t* pRX_ARP = &pRX->pPacket->ARP_Frame;
    IP_Address_t ActiveIP = m_pContext->GetActiveIP();
    IP_Address_t SrcIP = pRX_ARP->SrcIP_Address;
    IP_Address_t DstIP = pRX_ARP->DstIP_Address;


    switch(ntohs(pRX_ARP->Opcode))
    {
        case ARP_REQUEST:
        {
            if(DstIP == ActiveIP)                                                                   // Respond only if the ARP request targets our IP address
            {
                pMemoryPool->ChangeDebugID(pRX,          MEM_DBG_IPPKT,     MEM_DBG_ARP);           // Re-tag for zero-copy reuse
                pMemoryPool->ChangeDebugID(pRX->pPacket, MEM_DBG_ETHDMARX2, MEM_DBG_ARPDT);

                IP_PacketMsg_t*      pTX  = pRX;                                                    // Zero-copy: reuse RX buffer as TX
                IP_EthernetHeader_t* pETH = &pTX->pPacket->ETH_Header;
                ARP_Frame_t*         pARP = &pTX->pPacket->ARP_Frame;

                // Ethernet header
                memcpy(pETH->DestinationMAC.Byte, pETH->SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);       // Destination = requester MAC
                m_pContext->GetMAC_Address(&pETH->SourceMAC);                                       // Source = our MAC
                pETH->Type = htons(IP_ETHERNET_TYPE_ARP);

                FillCommon(pARP, ARP_REPLY);                                                        // ARP payload Fixed fields

                memcpy(pARP->SourceMAC.Byte,     pETH->SourceMAC.Byte,     IP_MAC_ADDRESS_SIZE);    // Sender = us
                memcpy(pARP->DestinationMAC.Byte,pETH->DestinationMAC.Byte,IP_MAC_ADDRESS_SIZE);    // Target = original requester

                pARP->SrcIP_Address = ActiveIP;
                pARP->DstIP_Address = SrcIP;

                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Reply Request\n");
                m_pContext->SendPacket(pTX);                                                        // Send ARP reply via normal TX path (zero-copy)
                return;
            }
        }
        break;

        case ARP_REPLY:
        {
            if(DstIP == ActiveIP)                                                                   // We learn this only if we are the destination
            {
                UpdateEntry(SrcIP, &pRX_ARP->SourceMAC);
            }
        }
        break;

        default:
            // Unknown Opcode -> so it is ignore
            break;
    }

    IP_Manager::FreeMessage(pRX);                                                                   // In all case, ARP est owner of pRX -> we free it (not for ARP_REQUEST if it is for us)
}

//-------------------------------------------------------------------------------------------------
//  Name:           UpdateEntry
//
//  Parameter(s):   IP_Address_t        IP_Address
//                  IP_MAC_Address_t*   pMAC_Address
//
//  Return:         void
//
//  Description:    Updates (or creates) an entry in the ARP table for the specified IPv4 address.
//
//                  - If an existing entry matches IP_Address, its MAC address and timestamp
//                    are refreshed.
//
//                  - If no entry exists, a free slot is used. If the table is full, the
//                    oldest entry is evicted and replaced.
//
//                  - After updating the table, this function also checks whether the updated
//                    IP address matches the ARP resolver’s pending request. If so, the
//                    pending packet is completed (Ethernet destination MAC is filled) and
//                    transmitted immediately. The pending state is then cleared.
//
//  Note(s):        This function is called from both ARP and IP processing paths. Any event
//                  that provides a valid (IP -> MAC) mapping—ARP reply, ARP request directed
//                  to us, or inbound IP traffic—can resolve a pending ARP request.
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::UpdateEntry(IP_Address_t IP_Address, IP_MAC_Address_t* pMAC_Address)
{
    uint8_t           i;
    uint8_t           OldestEntry;
    uint8_t           TimePage;
    ARP_TableEntry_t* pTable = nullptr;

    // Walk through the ARP mapping table and try to find an entry to update.
    // If none is found, the IP -> MAC address mapping is inserted in the ARP table.
    for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
    {
        pTable = &m_TableEntry[i];

        if(pTable->IP_Address != 0)
        {
            if(IP_Address == pTable->IP_Address)
            {
                memcpy(pTable->MAC_Address.Byte, pMAC_Address->Byte, IP_MAC_ADDRESS_SIZE);
                pTable->TimeToLive = m_Time;
              #if (IP_DBG_ARP == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Update an existing entry %d\n", IP_A(pTable->IP_Address),
                                                                                                                          IP_B(pTable->IP_Address),
                                                                                                                          IP_C(pTable->IP_Address),
                                                                                                                          IP_D(pTable->IP_Address),
                                                                                                                          i);
              #endif
                goto CheckPending;
            }
        }
    }

    // No existing entry found, try to find a free one.
    for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
    {
        pTable = &m_TableEntry[i];

        if(pTable->IP_Address == 0)
        {
          #if (IP_DBG_ARP == DEF_ENABLED)
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - Found a free entry %d\n", i);
          #endif
            break;
        }
    }

    // If no unused entry is found, evict the oldest.
    if(i == IP_ARP_TABLE_SIZE)
    {
        TimePage    = 0;
        OldestEntry = 0;

        for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
        {
            pTable = &m_TableEntry[i];

            if((m_Time - pTable->TimeToLive) > TimePage)
            {
                TimePage    = m_Time - pTable->TimeToLive;
                OldestEntry = i;
            }
        }

        i      = OldestEntry;                 // for debug only
        pTable = &m_TableEntry[OldestEntry];
      #if (IP_DBG_ARP == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Flush an old entry %d\n", IP_A(pTable->IP_Address),
                                                                                                            IP_B(pTable->IP_Address),
                                                                                                            IP_C(pTable->IP_Address),
                                                                                                            IP_D(pTable->IP_Address),
                                                                                                            i);
      #endif
    }

    pTable->IP_Address = IP_Address;
  #if (IP_DBG_ARP == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Added a new entry %d\n", IP_A(pTable->IP_Address),
                                                                                                       IP_B(pTable->IP_Address),
                                                                                                       IP_C(pTable->IP_Address),
                                                                                                       IP_D(pTable->IP_Address),
                                                                                                       i);
  #endif
    memcpy(pTable->MAC_Address.Byte, pMAC_Address->Byte, IP_MAC_ADDRESS_SIZE);
    pTable->TimeToLive = m_Time;

CheckPending:

    // If this IP resolves a pending packet, send it now.
    if((m_pPendingPacket != nullptr) && (m_PendingIP == IP_Address))
    {
        memcpy(m_pPendingPacket->pPacket->ETH_Header.DestinationMAC.Byte, pMAC_Address->Byte, IP_MAC_ADDRESS_SIZE);
        m_pContext->SendPacket(m_pPendingPacket);
        m_pPendingPacket = nullptr;
        m_PendingIP      = 0;

      #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Send Pending Packet\n");
      #endif
    }
}

//-------------------------------------------------------------------------------------------------
//  Name:           ProcessOut
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Builds and transmits an ARP request when the ARP table does not contain a
//                  valid entry for the destination IP.
//
//                  This function is invoked by ARP_Protocol::Resolve() when an ARP lookup
//                  fails. It allocates a fresh packet buffer, constructs a broadcast ARP
//                  request, and sends it through the normal transmit path.
//
//                  Only outgoing ARP requests are generated here. Incoming ARP requests and
//                  replies are handled in ARP_Protocol::ProcessARP().
//
//  Note(s):        - Zero-copy is NOT used here; ARP requests are always built in a new buffer.
//                  - The pending packet (if any) is resent upon receiving an ARP reply.
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::ProcessOut(void)
{
    // Allocate wrapper + ARP packet buffer using the new helper
    IP_PacketMsg_t* pMsg;// = nullptr;
    SystemState_e State  = m_pContext->GetIP_Manager()->AllocPacket(&pMsg, sizeof(ARP_Frame_t), MEM_DBG_ARP, MEM_DBG_ARPDT);

    if(State != SYS_READY)
    {
  #if (IP_DBG_ARP == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Failed to allocate ARP request buffer\n");
  #endif
        return;     // No memory -> cannot send ARP request
    }

    IP_EthernetPacket_t* pFrame = pMsg->pPacket;
    ARP_Frame_t*         pARP   = &pFrame->ARP_Frame;
    memset(pFrame->ETH_Header.DestinationMAC.Byte, 0xFF, IP_MAC_ADDRESS_SIZE);      // Ethernet header (broadcast)
    m_pContext->GetMAC_Address(&pFrame->ETH_Header.SourceMAC);
    pFrame->ETH_Header.Type = htons(IP_ETHERNET_TYPE_ARP);
    FillCommon(pARP, ARP_REQUEST);                                                  // ARP header (common fields + opcode)

    // ARP payload
    memcpy(pARP->SourceMAC.Byte, pFrame->ETH_Header.SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);
    memset(pARP->DestinationMAC.Byte, 0x00, IP_MAC_ADDRESS_SIZE);

    pARP->SrcIP_Address = m_pContext->GetActiveIP();
    pARP->DstIP_Address = m_IP_Address;                                             // IP we are resolving

    pMsg->PacketSize = sizeof(ARP_Frame_t);

  #if (IP_DBG_ARP == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Request for %d.%d.%d.%d\n", IP_A(m_IP_Address),
                                                                                     IP_B(m_IP_Address),
                                                                                     IP_C(m_IP_Address),
                                                                                     IP_D(m_IP_Address));
  #endif

    m_pContext->SendPacket(pMsg);                                                   // Transmit ARP request
}

//-------------------------------------------------------------------------------------------------
//  Name:           ARP_Resolve
//
//  Parameter(s):   IP      IPv4 address to resolve.
//                  pMAC    Pointer to a MAC address structure that receives the resolved
//                          address when it is already known.
//
//  Return:         bool    true  = MAC address is available and copied to *pMAC
//                                  (ARP table entry is VALID)
//                          false = MAC address is not available
//                                  (entry is missing or pending)
//
//  Description:    Attempts to resolve an IPv4 address to a MAC address using the ARP table.
//
//                  - If a VALID entry exists for the given IP, the MAC address is copied
//                    to *pMAC and the function returns true.
//
//                  - If no entry exists, an ARP request is triggered via ProcessOut() and
//                    the outgoing packet is stored as a pending packet. The function then
//                    returns false.
//
//                  - If a pending resolution already exists for the same IP, no additional
//                    ARP request is sent and the function returns false.
//
//                  - If a pending resolution exists for a *different* IP, the new request
//                    replaces the previous pending entry (caller’s packet becomes the new
//                    pending packet).
//
//  Note(s):        This function does not transmit IP packets directly. It only manages ARP
//                  resolution state and triggers ARP requests when needed. The caller must
//                  retry transmission once the ARP entry transitions to VALID (handled by
//                  UpdateEntry()).
//-------------------------------------------------------------------------------------------------
bool ARP_Protocol::Resolve(IP_Address_t IP, IP_MAC_Address_t* pMAC, IP_PacketMsg_t* pMsg)
{
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)                                                  // Search ARP table for a valid entry
    {
        if((m_TableEntry[i].IP_Address == IP) && (m_TableEntry[i].State == ARP_STATE_VALID))
        {
            memcpy(pMAC->Byte, m_TableEntry[i].MAC_Address.Byte, IP_MAC_ADDRESS_SIZE);
            return true;                                                                        // Found -> resolved
        }
    }

    // Not found -> send ARP request (if not already pending)
    if(m_pPendingPacket == nullptr)
    {
        // New ARP resolution request
        m_IP_Address = IP;                                                                      // Target IP for ARP request
        ProcessOut();                                                                           // Send ARP request

      #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET,
                             "ARP: Stack Msg to send later\n");
      #endif

        m_pPendingPacket = pMsg;                                                                // Store pending packet
pPending = m_pPendingPacket;
        m_PendingIP      = IP;                                                                  // Track which IP we wait for
    }
    else
    {
        // Already waiting for this IP -> drop new packet
        // (Future improvement: queue multiple pending packets)
      #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Already pending, drop new packet\n");
      #endif
    }

    return false;                                                                               // Not resolved yet
}

//-------------------------------------------------------------------------------------------------
//  Name:           FillCommon
//
//  Parameter(s):   pARP    Pointer to an ARP frame structure to initialize.
//                  Type    ARP opcode to assign (ARP_REQUEST or ARP_REPLY).
//
//  Return:         void
//
//  Description:    Initializes the common, fixed fields of an ARP frame.
//
//                  This helper fills the ARP header fields that are identical for both
//                  ARP requests and ARP replies:
//
//                      - HardwareType       (Ethernet)
//                      - Protocol           (IPv4)
//                      - HardwareAddrLength (MAC address size)
//                      - ProtocolLength     (IPv4 address size)
//                      - Opcode             (caller-specified)
//
//                  Fields that depend on direction (source/destination MAC and IP addresses)
//                  are intentionally *not* set here and must be filled by the caller.
//
//  Note(s):        This function does not modify Ethernet header fields. It is used by both
//                  ARP_Protocol::ProcessARP() when generating ARP replies (zero-copy) and
//                  ARP_Protocol::ProcessOut() when constructing outgoing ARP requests.
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::FillCommon(ARP_Frame_t* pARP, uint16_t Type)
{
    pARP->HardwareType       = htons(ARP_HARDWARE_TYPE_ETHERNET);
    pARP->Protocol           = htons(IP_ETHERNET_TYPE_IPV4);
    pARP->HardwareAddrLength = IP_MAC_ADDRESS_SIZE;
    pARP->ProtocolLength     = 4;
    pARP->Opcode             = htons(Type);
}

//-------------------------------------------------------------------------------------------------
//  Name:           TimerCallBack
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Periodic ARP maintenance handler invoked by the nOS timer.
//
//                  - Increments the internal ARP time counter (m_Time).
//                  - Iterates through all ARP table entries.
//                  - Computes the age of each entry using unsigned modular arithmetic.
//                  - Any entry whose age exceeds IP_ARP_TIME_OUT is removed from the table.
//
//                  This ensures that stale (IP -> MAC) mappings are automatically aged out
//                  and prevents the ARP cache from accumulating outdated entries.
//
//  Note(s):        This callback does not resend pending ARP requests or packets. Pending
//                  resolutions are completed by UpdateEntry() when a valid mapping is learned.
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::TimerCallBack(void)
{
    m_Time++;                                                         // Advance ARP time counter

    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)                        // Scan ARP table
    {
        ARP_TableEntry_t* pTable = &m_TableEntry[i];

        if(pTable->IP_Address != IP_ADDRESS(0,0,0,0))                 // Entry in use?
        {
            uint16_t age = uint16_t(m_Time - pTable->TimeToLive);     // Unsigned wrap-safe age

            if(age >= IP_ARP_TIME_OUT)                                // Entry expired?
            {
              #if (IP_DBG_ARP == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Remove entry number %d\n", IP_A(pTable->IP_Address),
                                                                                                                     IP_B(pTable->IP_Address),
                                                                                                                     IP_C(pTable->IP_Address),
                                                                                                                     IP_D(pTable->IP_Address),
                                                                                                                     i);
              #endif

                pTable->IP_Address = IP_ADDRESS(0,0,0,0);             // Clear entry
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//  Name:           ARP_TimerCallBack
//
//  Parameter(s):   nOS_Timer*  pTimer     Unused timer handle (provided by nOS)
//                  void*       pArg       Pointer to the ARP_Protocol instance
//
//  Return:         void
//
//  Description:    Global nOS timer callback used to service ARP maintenance.
//
//                  - Casts pArg back to the ARP_Protocol object and invokes its TimerCallBack()
//                    method to perform ARP entry aging.
//
//                  - After aging, checks whether a pending ARP resolution still has a corresponding
//                    VALID entry in the ARP table. If the entry has expired or no longer exists,
//                    the pending packet is dropped and the pending state is cleared.
//
//                  This ensures that stale pending packets do not remain queued indefinitely when
//                  ARP resolution fails or times out.
//
//  Note(s):        This function is the bridge between the OS timer system and the ARP_Protocol
//                  class. All ARP logic remains encapsulated inside ARP_Protocol; this callback
//                  only forwards the timer event and performs minimal cleanup.
//-------------------------------------------------------------------------------------------------
void ARP_TimerCallBack(nOS_Timer* pTimer, void* pArg)
{
    VAR_UNUSED(pTimer);
    ARP_Protocol* pARP = (ARP_Protocol*)pArg;

    pARP->TimerCallBack();
    IP_PacketMsg_t* pPendingPacket = pARP->GetPendingPacketPointer();

    if(pPendingPacket != nullptr)                                               // If ARP entry expired, drop pending packet
    {
        bool Found = false;                                                     // If the pending IP no longer exists in the table, drop it
        ARP_TableEntry_t* pTableEntry = pARP->GetTableEntryPointer(0);

        for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
        {
            if((pTableEntry[i].IP_Address == pARP->GetIP_Address()) && (pTableEntry[i].State == ARP_STATE_VALID))
            {
                Found = true;
                break;
            }
        }

        if(Found == false)
        {
            IP_Manager::FreeMessage(pPendingPacket);                            // Drop the pending packet
            pARP->SetPendingPacketPointer(nullptr);
        }
    }
}

//-------------------------------------------------------------------------------------------------
