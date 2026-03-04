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

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define ARP_TMR_INTERVAL                        1000
#define ARP_PENDING_NONE                        -1          // No oldest pending entry

//-------------------------------------------------------------------------------------------------
// Prototype(s)
//-------------------------------------------------------------------------------------------------

void ARP_TimerCallBack(nOS_Timer* pTimer, void* pArg);

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext&	  Reference on the context
//
//  Return:         SystemState_e     SYS_READY on success
//                                    SYS_FAIL  on timer creation/start failure
//
//  Description:    Initializes the ARP subsystem.
//
//                  - Stores the network context pointer.
//                  - Clears all ARP table entries (IP -> MAC mappings).
//                  - Resets the ARP pending-packet queue (no queued resolutions).
//                  - Initializes and starts the periodic ARP timer used for entry aging
//                    and retry scheduling.
//
//  Note(s):        This function must be called once during network stack initialization,
//                  before any ARP processing or IP transmission occurs.
//-------------------------------------------------------------------------------------------------
SystemState_e ARP_Manager::Initialize(NetworkContext& Context)
{
    nOS_Error Error;

    m_pContext      = &Context;
    m_PendingOldest = ARP_PENDING_NONE;
    m_IP_Address    = IP_ADDRESS(0,0,0,0);

    memset((void*)m_PendingQueue, 0, sizeof(ARP_PendingEntry_t) * ARP_PENDING_QUEUE_SIZE);  // Clear the pending queue
    memset((void*)m_TableEntry,   0, sizeof(ARP_TableEntry_t)   * IP_ARP_TABLE_SIZE);       // Clear the ARP cache table

    // Initialize an OS timer for the ARP timer
    Error = nOS_TimerCreate(&m_Timer,
                            &ARP_TimerCallBack,                                             // Timer callback function
                            this,                                                           // This class object
                            ARP_TMR_INTERVAL,
                            NOS_TIMER_FREE_RUNNING);                                        // It will repeat indefinitely

	if(Error == NOS_OK)
	{
		Error = nOS_TimerStart(&m_Timer);
	}

    return (Error == NOS_OK) ? SYS_READY : SYS_FAIL;
}

//-------------------------------------------------------------------------------------------------
//
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
//
//-------------------------------------------------------------------------------------------------
void ARP_Manager::ProcessIP(IP_PacketMsg_t* pRX)
{
  #if (ARP_SECURE_MODE != DEF_ENABLED)												// Secure mode: do not learn IP→MAC from generic IP traffic
    IP_Address_t SubnetMask = m_pContext->GetActiveSubnetMask();
    IP_Address_t ActiveIP   = m_pContext->GetActiveIP();

    if((ActiveIP == IP_ADDRESS(0,0,0,0)) || (SubnetMask == IP_ADDRESS(0,0,0,0)))    // Interface not configured yet -> do not learn from IP traffic
    {
        return;
    }

    IP_Address_t      DestIP       = pRX->pPacket->IP_Frame.Header.DstIP_Address;
    IP_MAC_Address_t* pDstMAC     = &pRX->pPacket->ETH_Header.DestinationMAC;
    IP_Manager*       pIP_Manager = m_pContext->GetIP_Manager();

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

    if((SourceIP == IP_ADDRESS(0,0,0,0))                    ||                      // Ignore invalid IPs
       (SourceIP == ActiveIP)                               ||                      // Ignore our own IP
       ((SourceIP & SubnetMask) != (ActiveIP & SubnetMask)) ||                      // Ignore packets outside our subnet
       (pIP_Manager->IsItBroadcastMAC(pSrcMAC))             ||                      // Ignore broadcast MAC
       (pIP_Manager->IsItMulticastMAC(pSrcMAC))             ||                      // Ignore multicast MAC
       (m_pContext->IsItMyMAC_Address(pSrcMAC))             ||                      // Ignore our own MAC
       (m_pContext->IsItMyMAC_Address(pDstMAC) == false))                           // Ignore destination if it is not own MAC
    {
        return;
    }

    UpdateEntry(SourceIP, pSrcMAC);                                                 // Passed all filters -> learn entry
  #else
    VAR_UNUSED(pRX);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
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
//                        If the request targets our IP address, a zero-copy ARP reply is
//                        constructed directly in the received buffer and transmitted.
//
//                  - For ARP_REPLY:
//                        If the reply is directed to our IP address, the sender’s
//                        (IP -> MAC) mapping is learned via UpdateEntry(). Any pending
//                        ARP resolution is also completed by UpdateEntry().
//
//                  - All non-matching or unsupported ARP opcodes are ignored.
//
//                  The received ARP packet is always freed unless it is reused for
//                  zero-copy transmission when replying to an ARP request.
//
//-------------------------------------------------------------------------------------------------
void ARP_Manager::ProcessARP(IP_PacketMsg_t* pRX)
{
    // Sanity check: must have at least ARP frame (sans ETH header)
    if((pRX == nullptr) || (pRX->pPacket == nullptr) || (pRX->PacketSize < (sizeof(ARP_Frame_t) - sizeof(IP_EthernetHeader_t))))
    {
        IP_Manager::FreeMessage(pRX);
        return;
    }

    pMemoryPool->ChangeDebugID(pRX, MEM_DBG_IPPKT, MEM_DBG_ARPRX);
    pMemoryPool->ChangeDebugID(pRX->pPacket, MEM_DBG_ETHDMARX2, MEM_DBG_ARPDTRX);

    ARP_Frame_t* pRX_ARP = &pRX->pPacket->ARP_Frame;
    IP_Address_t ActiveIP = m_pContext->GetActiveIP();
    IP_Address_t SrcIP = pRX_ARP->SrcIP_Address;
    IP_Address_t DstIP = pRX_ARP->DstIP_Address;

  #if (ARP_SECURE_MODE == DEF_ENABLED)
	// Ignore ARP that does not involve our IP at all
	if((SrcIP != ActiveIP) && (DstIP != ActiveIP))
	{
		IP_Manager::FreeMessage(pRX);
		return;
	}
  #endif

    switch(pRX_ARP->Opcode)
    {
        case ARP_REQUEST:
        {
          #if (ARP_SECURE_MODE != DEF_ENABLED)													    // In non secure mode, learn if the request targets our IP
            UpdateEntry(SrcIP, &pRX_ARP->SourceMAC);                                                // RFC-friendly: learn sender’s IP->MAC even if request is not for us
		  #endif

            if(DstIP == ActiveIP)                                                                   // Only reply if the request targets our IP
            {
			  #if (ARP_SECURE_MODE == DEF_ENABLED)										    		// In secure mode, only learn if the request targets our IP
				UpdateEntry(SrcIP, &pRX_ARP->SourceMAC);                                            // RFC-friendly: learn sender’s IP->MAC even if request is not for us
			  #endif

                pMemoryPool->ChangeDebugID(pRX,          MEM_DBG_ARPRX,   MEM_DBG_ARPTX);
                pMemoryPool->ChangeDebugID(pRX->pPacket, MEM_DBG_ARPDTRX, MEM_DBG_ARPDTTX);

                IP_PacketMsg_t*      pTX  = pRX;                                                    // Zero-copy reuse
                IP_EthernetHeader_t* pETH = &pTX->pPacket->ETH_Header;
                ARP_Frame_t*         pARP = &pTX->pPacket->ARP_Frame;

                // Ethernet header
                memcpy(pETH->DestinationMAC.Byte, pETH->SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);       // Reply to requester
                m_pContext->GetMAC_Address(&pETH->SourceMAC);                                       // Our MAC
                pETH->Type = IP_ETHERNET_TYPE_ARP;

                // ARP payload
                FillCommon(pARP, ARP_REPLY);
                memcpy(pARP->SourceMAC.Byte, pETH->SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);            // Sender = us
                memcpy(pARP->DestinationMAC.Byte, pETH->DestinationMAC.Byte, IP_MAC_ADDRESS_SIZE);  // Target = requester
                pARP->SrcIP_Address = ActiveIP;
                pARP->DstIP_Address = SrcIP;

              #if (IP_DBG_ARP == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Reply to request\n");
              #endif

                SystemState_e State = m_pContext->SendPacket(pTX);                                  // Zero-copy TX

                if(State != SYS_READY)
                {
                    IP_Manager::FreeMessage(pTX);

                  #if (IP_DBG_ARP == DEF_ENABLED)
                    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Reply to Request - SendPacket Failed!, Drop the packet\n");
                  #endif
                }

                return;
            }
        }
        break;

        case ARP_REPLY:
        {
			// Only learn replies:
			//  - addressed to our IP
			//  - to our MAC
			//  - (ARP_SECURE_MODE only) for an IP we actually requested
			if((DstIP == ActiveIP) && (m_pContext->IsItMyMAC_Address(&pRX->pPacket->ETH_Header.DestinationMAC) == true))
            {
              #if (ARP_SECURE_MODE == DEF_ENABLED)
                if(IsPendingARP_Request(SrcIP) == true)
		      #endif
    			{
                    UpdateEntry(SrcIP, &pRX_ARP->SourceMAC);
			    }
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
//
//  Name:           UpdateEntry
//
//  Parameter(s):   IP_Address_t IP_Address         The IP address whose ARP mapping has just been
//                                                  resolved.
//
//                  IP_MAC_Address_t* pMacAddress   The resolved MAC address associated with
//                                                  IP_Address.
//
//  Return:         void
//
//  Description:    Updates the ARP table with the resolved IP → MAC mapping. If an existing entry
//                  matches IP_Address, its MAC and timestamp are refreshed. Otherwise, a free ARP
//                  table slot is allocated; if none are free, the oldest entry is evicted.
//
//                  After updating the ARP table, the function scans the ARP pending queue and
//                  flushes all packets waiting for this IP. Each matching pending entry has its
//                  Ethernet destination MAC patched, the packet is transmitted, and the pending
//                  slot is marked EMPTY.
//
//                  The pending queue is implemented as a circular array with no head/tail/count.
//                  Each entry carries its own State (EMPTY or PENDING). Scanning always begins at
//                  m_PendingOldest and wraps around ARP_PENDING_QUEUE_SIZE.
//
//                  Once all matching pending packets are flushed, m_PendingOldest is advanced to
//                  the next non-EMPTY pending entry. If no pending entries remain, m_PendingOldest
//                  is reset to 0.
//
//  Notes:          - ARP_STATE_VALID applies only to ARP table entries, never to pending entries.
//                  - Pending queue holds packets for unresolved IPs only; resolved packets are
//                    immediately transmitted and removed.
//                  - Zero-copy ownership rules apply: ARP owns pMsg only while pending.
//                  - This function does not compact or reorder the queue; it relies solely on
//                    per-entry State and circular scanning.
//
//-------------------------------------------------------------------------------------------------
void ARP_Manager::UpdateEntry(IP_Address_t IP_Address, IP_MAC_Address_t* pMacAddress)
{
    nOS_StatusReg     sr;
    uint8_t           Index;
    uint8_t           OldestIndex = 0;
    uint16_t          OldestTTL   = 0xFFFF;
    ARP_TableEntry_t* pTable = nullptr;

    // Ignore spoofed packets (MAC/IP mismatch). But only before TTL as expired
    if(ItHasEntry(IP_Address) && (IsItMatchingMAC(IP_Address, pMacAddress) == false))
    {
        return;                                                                     // Suspicious -> then ignore it
    }

    // Try to update an existing ARP table entry
    for(Index = 0; Index < IP_ARP_TABLE_SIZE; Index++)
    {
        pTable = &m_TableEntry[Index];

        if((pTable->IP_Address != 0) && (pTable->IP_Address == IP_Address))
        {
            memcpy(pTable->MAC_Address.Byte, pMacAddress->Byte, IP_MAC_ADDRESS_SIZE);
            pTable->TimeToLive = IP_ARP_TIME_OUT;

          #if (IP_DBG_ARP == DEF_ENABLED)
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Update existing entry %d\n", IP_A(pTable->IP_Address),
                                                                                                                   IP_B(pTable->IP_Address),
                                                                                                                   IP_C(pTable->IP_Address),
                                                                                                                   IP_D(pTable->IP_Address),
                                                                                                                   Index);
          #endif

            goto FlushPending;
        }
    }

    // No existing entry → find a free one
    for(Index = 0; Index < IP_ARP_TABLE_SIZE; Index++)
    {
        pTable = &m_TableEntry[Index];

        if(pTable->IP_Address == IP_ADDRESS(0,0,0,0))
        {
          #if (IP_DBG_ARP == DEF_ENABLED)
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - Found free entry %d\n", Index);
          #endif
            break;
        }
    }

    // No free entry -> delete the oldest
    if(Index == IP_ARP_TABLE_SIZE)
    {
        for(Index = 0; Index < IP_ARP_TABLE_SIZE; Index++)
        {
            pTable = &m_TableEntry[Index];

            if(pTable->IP_Address != IP_ADDRESS(0,0,0,0))
            {
                if(pTable->TimeToLive <= OldestTTL)             // Smaller is the TTL, the entry is the oldest
                {
                    OldestTTL   = pTable->TimeToLive;
                    OldestIndex = Index;
                }
            }
        }

        pTable = &m_TableEntry[OldestIndex];
        Index  = OldestIndex;

      #if (IP_DBG_ARP == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Delete old entry %d\n", IP_A(pTable->IP_Address),
                                                                                                          IP_B(pTable->IP_Address),
                                                                                                          IP_C(pTable->IP_Address),
                                                                                                          IP_D(pTable->IP_Address),
                                                                                                          Index);
      #endif
    }

    // Insert new ARP table entry
    nOS_EnterCritical(sr);
    pTable->IP_Address = IP_Address;
    pTable->TimeToLive = IP_ARP_TIME_OUT;
    pTable->State      = ARP_STATE_VALID;
    memcpy(pTable->MAC_Address.Byte, pMacAddress->Byte, IP_MAC_ADDRESS_SIZE);
    nOS_LeaveCritical(sr);

  #if (IP_DBG_ARP == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Added new entry %d\n", IP_A(pTable->IP_Address),
                                                                                                     IP_B(pTable->IP_Address),
                                                                                                     IP_C(pTable->IP_Address),
                                                                                                     IP_D(pTable->IP_Address),
                                                                                                     Index);
  #endif

FlushPending:

    // 1) Flush all pending packets for this IP (full scan)
    for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)
    {
        ARP_PendingEntry_t* pEntry = &m_PendingQueue[i];

        if((pEntry->State == ARP_STATE_PENDING) && (pEntry->IP == IP_Address))
        {
            memcpy(pEntry->pMsg->pPacket->ETH_Header.DestinationMAC.Byte, pMacAddress->Byte, IP_MAC_ADDRESS_SIZE);
            SystemState_e State = m_pContext->SendPacket(pEntry->pMsg);

            if(State != SYS_READY)
            {
                IP_Manager::FreeMessage(pEntry->pMsg);

              #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Sent pending - SendPacket failed: Drop the packet\n");
              #endif
            }
          #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
            else
            {
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Sent pending packet for resolved IP\n");
            }
          #endif

            nOS_EnterCritical(sr);
            pEntry->pMsg  = nullptr;
            pEntry->IP    = IP_ADDRESS(0,0,0,0);
            pEntry->State = ARP_STATE_EMPTY;
            nOS_LeaveCritical(sr);
        }
    }

    // 2) Recompute m_PendingOldest and maybe send ARP for new oldest
    nOS_EnterCritical(sr);
    m_PendingOldest = ARP_PENDING_NONE;
    m_IP_Address    = IP_ADDRESS(0,0,0,0);

    for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)
    {
        if(m_PendingQueue[i].State == ARP_STATE_PENDING)
        {
            m_PendingOldest = i;
            m_IP_Address    = m_PendingQueue[i].IP;

          #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Next pending entry at %d, sending ARP\n", i);
          #endif

            nOS_LeaveCritical(sr);
            ProcessOut();
            return;
        }
    }
    nOS_LeaveCritical(sr);

  #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: No more pending entries\n");
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ProcessOut
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Builds and transmits an ARP request when the ARP table does not contain a
//                  valid entry for the destination IP.
//
//                  This function is invoked by ARP_Manager::Resolve() when an ARP lookup
//                  fails. It allocates a fresh packet buffer, constructs a broadcast ARP
//                  request, and sends it through the normal transmit path.
//
//                  Only outgoing ARP requests are generated here. Incoming ARP requests and
//                  replies are handled in ARP_Manager::ProcessARP().
//
//  Note(s):        - Zero-copy is NOT used here; ARP requests are always built in a new buffer.
//                  - The pending packet (if any) is resent upon receiving an ARP reply.
//
//-------------------------------------------------------------------------------------------------
void ARP_Manager::ProcessOut(void)
{
    // Allocate wrapper + ARP packet buffer using the new helper
    IP_PacketMsg_t* pMsg;
    SystemState_e State = IP_Manager::AllocPacket(&pMsg, sizeof(ARP_Frame_t), MEM_DBG_ARPPO, MEM_DBG_ARPDTPO);

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
    pFrame->ETH_Header.Type = IP_ETHERNET_TYPE_ARP;
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

    State = m_pContext->SendPacket(pMsg);                                           // Transmit ARP request

    if(State != SYS_READY)
    {
      #if (IP_DBG_ARP == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Request - SendPacket failed for %d.%d.%d.%d\n", IP_A(m_IP_Address),
                                                                                                             IP_B(m_IP_Address),
                                                                                                             IP_C(m_IP_Address),
                                                                                                             IP_D(m_IP_Address));
      #endif
        IP_Manager::FreeMessage(pMsg);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ARP_Resolve
//
//  Parameter(s):   IP      IPv4 address to resolve.
//                  pMAC    Pointer to a MAC address structure that receives the resolved
//                          address when it is already known.
//
//  Return:         bool    true  = MAC address is available and copied to *pMAC
//                                  (ARP table entry is VALID)
//                          false = MAC address is not available
//                                  (entry is missing or resolution is pending)
//
//  Description:    Attempts to resolve the destination IP address to a MAC address. If the ARP
//                  table already contains a VALID entry for the IP, the MAC is copied to pMAC and
//                  the function returns true.
//
//                  If the IP is not yet resolved, the packet is inserted into the ARP pending
//                  queue. The pending queue is implemented as a circular array with no head/tail
//                  counters; each entry carries its own State (EMPTY or PENDING). The function
//                  scans for the first EMPTY slot starting from m_PendingOldest.
//
//                  If this is the first pending entry in the queue, an ARP request is transmitted
//                  immediately and m_PendingOldest is updated to point to the new entry.
//
//                  If the queue contains no EMPTY slots, the packet is dropped (ARP does not take
//                  ownership of it).
//
//  Notes:          - Pending queue holds packets for a single unresolved IP at a time.
//                  - ARP_STATE_VALID is used only in the ARP table, never in the pending queue.
//                  - This function does not compact or reorder the queue; it relies solely on
//                    per-entry State and circular scanning.
//                  - Zero-copy ownership rules apply: ARP owns pMsg only while it is pending.
//
//-------------------------------------------------------------------------------------------------
bool ARP_Manager::Resolve(IP_Address_t IP, IP_MAC_Address_t* pMAC, IP_PacketMsg_t* pMsg)
{
    nOS_StatusReg sr;

    // Check ARP table
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
    {
        if((m_TableEntry[i].IP_Address == IP) && (m_TableEntry[i].State == ARP_STATE_VALID))
        {
            memcpy(pMAC->Byte, m_TableEntry[i].MAC_Address.Byte, IP_MAC_ADDRESS_SIZE);
            return true;
        }
    }

    //  Check if already pending
    for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)
    {
        ARP_PendingEntry_t* pEntry = &m_PendingQueue[i];

        if((pEntry->State == ARP_STATE_PENDING) && (pEntry->IP == IP))
        {
            IP_Manager::FreeMessage(pMsg);

          #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET,
                                 "ARP: Duplicate pending for %d.%d.%d.%d, dropping packet\n",
                                 IP_A(IP), IP_B(IP), IP_C(IP), IP_D(IP));
          #endif
            return false;
        }
    }

    // 3) Find EMPTY slot
    int FreeIndex = -1;

    for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)
    {
        if(m_PendingQueue[i].State == ARP_STATE_EMPTY)
        {
            FreeIndex = i;
            break;
        }
    }

    if(FreeIndex < 0)
    {
      #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Pending queue FULL, dropping packet\n");
      #endif

        IP_Manager::FreeMessage(pMsg);
        return false;
    }

    // 4) Fill new pending entry
    ARP_PendingEntry_t* pEntry = &m_PendingQueue[FreeIndex];
    nOS_EnterCritical(sr);
    pEntry->IP    = IP;
    pEntry->pMsg  = pMsg;
    pEntry->State = ARP_STATE_PENDING;
    nOS_LeaveCritical(sr);

    // 5) If queue was empty, this becomes the oldest and we send ARP now
    if(m_PendingOldest == ARP_PENDING_NONE)
    {
        m_PendingOldest = FreeIndex;
        m_IP_Address    = IP;
        ProcessOut();
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsPendingARP_Request
//
//  Parameter(s):   IP      IP address to check for a pending ARP request.
//
//  Return:         bool    true  if an ARP request is already pending for this IP.
//                          false otherwise.
//
//  Description:    Scans the ARP pending queue to determine whether an ARP request is
//                  already outstanding for the specified IP address.
//
//                  This is used in secure mode to avoid issuing multiple ARP requests
//                  for the same IP while a previous request is still pending.
//
//  Note(s):        Only compiled when ARP secure mode is enabled.
//
//-------------------------------------------------------------------------------------------------
#if (ARP_SECURE_MODE == DEF_ENABLED)
bool ARP_Manager::IsPendingARP_Request(IP_Address_t IP)
{
    for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)
    {
        ARP_PendingEntry_t* pEntry = &m_PendingQueue[i];

        if((pEntry->State == ARP_STATE_PENDING) && (pEntry->IP == IP))
        {
            return true;
        }
    }
    return false;
}
#endif

//-------------------------------------------------------------------------------------------------
//
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
//                  ARP_Manager::ProcessARP() when generating ARP replies (zero-copy) and
//                  ARP_Manager::ProcessOut() when constructing outgoing ARP requests.
//
//-------------------------------------------------------------------------------------------------
void ARP_Manager::FillCommon(ARP_Frame_t* pARP, uint16_t Type)
{
    pARP->HardwareType       = ARP_HARDWARE_TYPE_ETHERNET;
    pARP->Protocol           = IP_ETHERNET_TYPE_IPV4;
    pARP->HardwareAddrLength = IP_MAC_ADDRESS_SIZE;
    pARP->ProtocolLength     = 4;
    pARP->Opcode             = Type;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           OnPendingTimeOut
//
//  Parameter(s):   int PendingOffset   Logical offset from the oldest pending entry (0 = oldest).
//                                      This is NOT a physical array index; it is a circular offset
//                                      relative to m_PendingOldest.
//
//  Return:         void
//
//  Description:    Handles timeout of a pending ARP resolution request. The function clears the
//                  pending entry at the specified logical offset, marks it EMPTY, and advances
//                  m_PendingOldest until it points to the next non-EMPTY entry.
//
//                  The pending queue is a circular array. Each entry carries its own State
//                  (EMPTY or PENDING). Scanning always begins at m_PendingOldest and wraps around
//                  ARP_PENDING_QUEUE_SIZE.
//
//                  If all entries become EMPTY, m_PendingOldest is reset to 0. Otherwise, an ARP
//                  request is sent for the new oldest pending entry.
//
//-------------------------------------------------------------------------------------------------
void ARP_Manager::OnPendingTimeOut(int PendingOffset)
{
    nOS_StatusReg sr;

    if(m_PendingOldest == ARP_PENDING_NONE)
    {
        return;
    }

    int PhysicalIndex = (m_PendingOldest + PendingOffset) % ARP_PENDING_QUEUE_SIZE;
    ARP_PendingEntry_t* pEntry = &m_PendingQueue[PhysicalIndex];

    nOS_EnterCritical(sr);

    if(pEntry->pMsg != nullptr)
    {
        IP_Manager::FreeMessage(pEntry->pMsg);
        pEntry->pMsg = nullptr;
    }

    pEntry->IP    = IP_ADDRESS(0,0,0,0);
    pEntry->State = ARP_STATE_EMPTY;
    nOS_LeaveCritical(sr);

    // Recompute oldest
    m_PendingOldest = ARP_PENDING_NONE;
    m_IP_Address    = IP_ADDRESS(0,0,0,0);

    for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)
    {
        if(m_PendingQueue[i].State == ARP_STATE_PENDING)
        {
            m_PendingOldest = i;
            m_IP_Address    = m_PendingQueue[i].IP;
            ProcessOut();
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ItHasEntry
//
//  Parameter(s):   IP      IP address to search for in the ARP table.
//
//  Return:         bool    true  if an entry exists for this IP.
//                          false otherwise.
//
//  Description:    Scans the ARP table for a matching IP address. This helper is used by
//                  multiple ARP_Manager components (ProcessIP, ProcessARP, UpdateEntry,
//                  Resolve, etc.) to determine whether an ARP entry already exists.
//
//  Note(s):        Does not validate MAC address. Only checks for IP presence.
//
//-------------------------------------------------------------------------------------------------
bool ARP_Manager::ItHasEntry(IP_Address_t IP)
{
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
    {
        if(m_TableEntry[i].IP_Address == IP)
            return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsItMatchingMAC
//
//  Parameter(s):   IP      IP address whose MAC should be validated.
//                  pMAC    Pointer to the MAC address to compare.
//
//  Return:         bool    true  if the ARP table contains this IP AND the MAC matches.
//                          false otherwise.
//
//  Description:    Searches the ARP table for the specified IP address and compares the stored
//                  MAC address with the provided one. Used for spoofing detection and to
//                  validate ARP updates before modifying an existing entry.
//
//  Note(s):        Returns false if the IP is not found OR if the MAC does not match.
//
//-------------------------------------------------------------------------------------------------
bool ARP_Manager::IsItMatchingMAC(IP_Address_t IP, IP_MAC_Address_t* pMAC)
{
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
    {
        if(m_TableEntry[i].IP_Address == IP)
        {
            return memcmp(m_TableEntry[i].MAC_Address.Byte, pMAC->Byte, IP_MAC_ADDRESS_SIZE) == 0;
        }
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetPendingEntryByOffset
//
//  Parameter(s):   int Offset
//                      Circular offset from m_PendingOldest (0..ARP_PENDING_QUEUE_SIZE-1)
//
//  Return:         ARP_PendingEntry_t*
//                      Pointer to the pending entry at that circular position.
//
//  Description:    Converts a circular offset into a physical index in the pending queue. This
//                  helper allows external callbacks (such as ARP_TimerCallBack) to iterate the
//                  pending queue without exposing internal member variables.
//
//-------------------------------------------------------------------------------------------------
inline ARP_PendingEntry_t* ARP_Manager::GetPendingEntryByOffset(int Offset)
{
    if(m_PendingOldest == ARP_PENDING_NONE)
    {
        return nullptr;
    }

    int Index = (m_PendingOldest + Offset) % ARP_PENDING_QUEUE_SIZE;
    return &m_PendingQueue[Index];
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TimerCallBack
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Periodic ARP maintenance handler invoked by the nOS timer.
//
//                  ARP cache aging:
//                  - Iterates through all ARP table entries.
//                  - Computes the age of each entry using unsigned modular arithmetic.
//                  - Any entry whose age exceeds IP_ARP_TIME_OUT is removed from the table,
//                    preventing stale (IP -> MAC) mappings from accumulating.
//
//                  Pending queue maintenance:
//                  - If there are pending ARP resolutions (m_PendingOldest != ARP_PENDING_NONE),
//                    iterates through the pending queue in circular order.
//                  - For each non-empty pending entry, checks whether a VALID ARP table entry
//                    exists for its IP.
//                  - If no valid ARP entry exists for that IP (resolution failed/timed out),
//                    the pending packet is dropped via OnPendingTimeOut(), and the scan stops
//                    for this tick.
//
//                  This ensures both the ARP cache and the pending queue are periodically
//                  cleaned up without resending ARP requests here. New ARP requests are
//                  triggered by Resolve() and UpdateEntry().
//
//-------------------------------------------------------------------------------------------------
void ARP_Manager::TimerCallBack(void)
{
    nOS_StatusReg sr;

    // ARP table aging
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
    {
        ARP_TableEntry_t* pTable = &m_TableEntry[i];

        if(pTable->IP_Address != IP_ADDRESS(0,0,0,0))                               // Entry in use?
        {
            if(pTable->TimeToLive > 0)
            {
                pTable->TimeToLive--;
            }

            if(pTable->TimeToLive == 0)                                             // Entry expired?
            {
              #if (IP_DBG_ARP == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET,
                                     "ARP Cache - (%d.%d.%d.%d) Remove entry number %d\n",
                                     IP_A(pTable->IP_Address), IP_B(pTable->IP_Address),
                                     IP_C(pTable->IP_Address), IP_D(pTable->IP_Address), i);
              #endif
                nOS_EnterCritical(sr);
                memset(pTable, 0, sizeof(ARP_TableEntry_t));                        // Clear entry
                nOS_LeaveCritical(sr);
            }
        }
    }

    // No pending entries -> nothing to do on the queue side
    if(m_PendingOldest == ARP_PENDING_NONE)
    {
        return;
    }

    // Pending queue maintenance (single timeout per tick)
    for(int Offset = 0; Offset < ARP_PENDING_QUEUE_SIZE; Offset++)
    {
        ARP_PendingEntry_t* pEntry = GetPendingEntryByOffset(Offset);

        if(pEntry == nullptr)                                                       // Defensive: no valid oldest
        {
            break;
        }

        if(pEntry->State == ARP_STATE_EMPTY)                                        // Skip empty slots
        {
            continue;
        }

        bool Resolved = false;

        for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
        {
            if((m_TableEntry[i].IP_Address == pEntry->IP) && (m_TableEntry[i].State == ARP_STATE_VALID))
            {
                Resolved = true;
                break;
            }
        }

        if(Resolved == false)                                                       // If unresolved → timeout this entry
        {
            OnPendingTimeOut(Offset);
            break;                                                                  // Let next timer tick re-scan
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ARP_TimerCallBack
//
//  Parameter(s):   nOS_Timer*  pTimer     Unused timer handle (provided by nOS)
//                  void*       pArg       Pointer to the ARP_Manager instance
//
//  Return:         void
//
//  Description:    nOS timer trampoline for ARP maintenance.
//
//                  - Casts pArg to ARP_Manager*.
//                  - Delegates all periodic ARP work to ARP_Manager::TimerCallBack().
//
//-------------------------------------------------------------------------------------------------
void ARP_TimerCallBack(nOS_Timer* pTimer, void* pArg)
{
    VAR_UNUSED(pTimer);
    ARP_Manager* pARP = (ARP_Manager*)pArg;

    pARP->TimerCallBack();
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
