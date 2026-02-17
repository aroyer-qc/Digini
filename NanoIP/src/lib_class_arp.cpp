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
// Prototype(s)
//-------------------------------------------------------------------------------------------------

void ARP_TimerCallBack(nOS_Timer* pTimer, void* pArg);

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
//                  - Resets the ARP pending-packet queue (no queued resolutions).
//                  - Initializes and starts the periodic ARP timer used for entry aging
//                    and retry scheduling.
//
//  Note(s):        This function must be called once during network stack initialization,
//                  before any ARP processing or IP transmission occurs.
//-------------------------------------------------------------------------------------------------
SystemState_e ARP_Manager::Initialize(NetworkContext* pContext)
{
    nOS_Error Error;

    m_pContext = pContext;
    m_PendingOldest = 0;

    memset((void*)m_PendingQueue, 0, sizeof(ARP_PendingEntry_t) * ARP_PENDING_QUEUE_SIZE);  // Clear the pending queue
    memset((void*)m_TableEntry,   0, sizeof(ARP_TableEntry_t)   * IP_ARP_TABLE_SIZE);       // Clear the ARP cache table

    // Initialize an OS timer for the ARP timer
    Error = nOS_TimerCreate(&m_Timer,
                            &ARP_TimerCallBack,                                             // Timer callback function
                            this,                                                           // This class object
                            NET_ARP_TMR_INTERVAL,
                            NOS_TIMER_FREE_RUNNING);                                        // It will repeat indefinitely

	if(Error == NOS_OK)
	{
		Error = nOS_TimerStart(&m_Timer);
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
void ARP_Manager::ProcessIP(IP_PacketMsg_t* pRX)
{
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
//-------------------------------------------------------------------------------------------------
void ARP_Manager::ProcessARP(IP_PacketMsg_t* pRX)
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

    switch(pRX_ARP->Opcode)
    {
        case ARP_REQUEST:
        {
            UpdateEntry(SrcIP, &pRX_ARP->SourceMAC);                                                // RFC-friendly: learn sender’s IP->MAC even if request is not for us

            if(DstIP == ActiveIP)                                                                   // Only reply if the request targets our IP
            {
                pMemoryPool->ChangeDebugID(pRX,          MEM_DBG_IPPKT,     MEM_DBG_ARP);
                pMemoryPool->ChangeDebugID(pRX->pPacket, MEM_DBG_ETHDMARX2, MEM_DBG_ARPDT);

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
            //
            // Only learn replies actually addressed to us:
            //   - IP destination matches our IP
            //   - Ethernet destination MAC is ours (anti‑spoofing)
            //
            if((DstIP == ActiveIP) && m_pContext->IsItMyMAC_Address(&pRX->pPacket->ETH_Header.DestinationMAC))
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
    uint8_t Index;
    uint8_t OldestIndex = 0;
    uint8_t OldestAge   = 0;
    ARP_TableEntry_t* pTable = nullptr;

    // Try to update an existing ARP table entry
    for(Index = 0; Index < IP_ARP_TABLE_SIZE; Index++)
    {
        pTable = &m_TableEntry[Index];

        if((pTable->IP_Address != 0) && (pTable->IP_Address == IP_Address))
        {
            memcpy(pTable->MAC_Address.Byte, pMacAddress->Byte, IP_MAC_ADDRESS_SIZE);
            pTable->TimeToLive = m_Time;

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

        if(pTable->IP_Address == 0)
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
        OldestIndex = 0;
        OldestAge   = 0;

        for(Index = 0; Index < IP_ARP_TABLE_SIZE; Index++)
        {
            pTable = &m_TableEntry[Index];
            uint8_t Age = m_Time - pTable->TimeToLive;

            if(Age > OldestAge)
            {
                OldestAge   = Age;
                OldestIndex = Index;
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
    pTable->IP_Address = IP_Address;
    pTable->TimeToLive = m_Time;
    pTable->State      = ARP_STATE_VALID;
    memcpy(pTable->MAC_Address.Byte, pMacAddress->Byte, IP_MAC_ADDRESS_SIZE);


  #if (IP_DBG_ARP == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Added new entry %d\n", IP_A(pTable->IP_Address),
                                                                                                     IP_B(pTable->IP_Address),
                                                                                                     IP_C(pTable->IP_Address),
                                                                                                     IP_D(pTable->IP_Address),
                                                                                                     Index);
  #endif

FlushPending:

    // Flush all pending packets waiting for this IP
    for(int Offset = 0; Offset < ARP_PENDING_QUEUE_SIZE; Offset++)
    {
        int PendingIndex = (m_PendingOldest + Offset) % ARP_PENDING_QUEUE_SIZE;
        ARP_PendingEntry_t* pEntry = &m_PendingQueue[PendingIndex];

        if((pEntry->State == ARP_STATE_PENDING) && (pEntry->IP == IP_Address))
        {
            memcpy(pEntry->pMsg->pPacket->ETH_Header.DestinationMAC.Byte, pMacAddress->Byte, IP_MAC_ADDRESS_SIZE);

            SystemState_e State = m_pContext->SendPacket(pEntry->pMsg);

            if(State == SYS_READY)
            {
              #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Sent pending packet for resolved IP\n");
              #endif
            }
            else
            {
                IP_Manager::FreeMessage(pEntry->pMsg);

              #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Sent pending - SendPacket failed: Drop the packet\n");
              #endif
            }

            pEntry->pMsg  = nullptr;
            pEntry->IP    = IP_ADDRESS(0,0,0,0);
            pEntry->State = ARP_STATE_EMPTY;
        }
    }

    // Advance m_PendingOldest to next non-empty entry
    for(int Offset = 0; Offset < ARP_PENDING_QUEUE_SIZE; Offset++)
    {
        int PendingIndex = (m_PendingOldest + Offset) % ARP_PENDING_QUEUE_SIZE;

        if(m_PendingQueue[PendingIndex].State == ARP_STATE_PENDING)
        {
            m_PendingOldest = PendingIndex;
            m_IP_Address    = m_PendingQueue[PendingIndex].IP;

          #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Next pending entry at %d, sending ARP\n", PendingIndex);
          #endif

            ProcessOut();
            return;
        }
    }

    // No pending entries remain
    m_PendingOldest = 0;

  #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: No more pending entries\n");
  #endif
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
//                  This function is invoked by ARP_Manager::Resolve() when an ARP lookup
//                  fails. It allocates a fresh packet buffer, constructs a broadcast ARP
//                  request, and sends it through the normal transmit path.
//
//                  Only outgoing ARP requests are generated here. Incoming ARP requests and
//                  replies are handled in ARP_Manager::ProcessARP().
//
//  Note(s):        - Zero-copy is NOT used here; ARP requests are always built in a new buffer.
//                  - The pending packet (if any) is resent upon receiving an ARP reply.
//-------------------------------------------------------------------------------------------------
void ARP_Manager::ProcessOut(void)
{
    // Allocate wrapper + ARP packet buffer using the new helper
    IP_PacketMsg_t* pMsg;// = nullptr;
    SystemState_e State  = IP_Manager::AllocPacket(&pMsg, sizeof(ARP_Frame_t), MEM_DBG_ARP, MEM_DBG_ARPDT);

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
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)                                                  // Check ARP table for a VALID entry
    {
        if((m_TableEntry[i].IP_Address == IP) && (m_TableEntry[i].State == ARP_STATE_VALID))
        {
            memcpy(pMAC->Byte, m_TableEntry[i].MAC_Address.Byte, IP_MAC_ADDRESS_SIZE);
            return true;                                                                        // Resolved immediately
        }
    }

    // Not resolved -> check if this IP is already pending
    for(int Offset = 0; Offset < ARP_PENDING_QUEUE_SIZE; Offset++)
    {
        int Index = (m_PendingOldest + Offset) % ARP_PENDING_QUEUE_SIZE;
        ARP_PendingEntry_t* pEntry = &m_PendingQueue[Index];

        if((pEntry->State == ARP_STATE_PENDING) && (pEntry->IP == IP))
        {
            // Already have a pending entry for this IP.
            // ARP will eventually send or timeout that packet.
            // This new one cannot be queued -> free it.
            IP_Manager::FreeMessage(pMsg);

          #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
             DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Duplicate pending for %d.%d.%d.%d, dropping packet\n", IP_A(IP),
                                                                                                                         IP_B(IP),
                                                                                                                         IP_C(IP),
                                                                                                                         IP_D(IP));
          #endif

            return false;                                                                       // Still unresolved
        }
    }

    // Not resolved and not already pending -> enqueue into pending queue. Scan circularly for an EMPTY slot
    for(int Offset = 0; Offset < ARP_PENDING_QUEUE_SIZE; Offset++)
    {
        int Index = (m_PendingOldest + Offset) % ARP_PENDING_QUEUE_SIZE;
        ARP_PendingEntry_t* pEntry = &m_PendingQueue[Index];

        if(pEntry->State == ARP_STATE_EMPTY)
        {
            pEntry->IP    = IP;                                                                 // Fill pending entry
            pEntry->pMsg  = pMsg;
            pEntry->State = ARP_STATE_PENDING;
            bool FirstPending = true;                                                           // If this is the ONLY pending entry, send ARP request now

            for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)
            {
                if(i == Index)
                {
                    continue;
                }

                if(m_PendingQueue[i].State == ARP_STATE_PENDING)
                {
                    FirstPending = false;
                    break;
                }
            }

            if(FirstPending == true)
            {
                m_PendingOldest = Index;                                                        // This becomes the oldest pending entry
                m_IP_Address    = IP;
                ProcessOut();                                                                   // Send ARP request
            }

            return false;                                                                       // Not resolved yet
        }
    }

  #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Pending queue FULL, dropping packet\n");
  #endif

    IP_Manager::FreeMessage(pMsg);                                                              // No EMPTY slot -> queue full -> drop packet
    return false;
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
//                  ARP_Manager::ProcessARP() when generating ARP replies (zero-copy) and
//                  ARP_Manager::ProcessOut() when constructing outgoing ARP requests.
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
    int PhysicalIndex = (m_PendingOldest + PendingOffset) % ARP_PENDING_QUEUE_SIZE;     // Convert logical offset to physical index
    ARP_PendingEntry_t* pEntry = &m_PendingQueue[PhysicalIndex];

    if(pEntry->pMsg != nullptr)                                                         // Free message if present
    {
        IP_Manager::FreeMessage(pEntry->pMsg);
        pEntry->pMsg = nullptr;
    }

    pEntry->IP    = IP_ADDRESS(0,0,0,0);                                                // Mark entry empty
    pEntry->State = ARP_STATE_EMPTY;

    for(int i = 0; i < ARP_PENDING_QUEUE_SIZE; i++)                                     // Advance m_PendingOldest until it points to a non-EMPTY entry
    {
        int Index = (m_PendingOldest + i) % ARP_PENDING_QUEUE_SIZE;

        if(m_PendingQueue[Index].State != ARP_STATE_EMPTY)
        {
            m_PendingOldest = Index;
            m_IP_Address = m_PendingQueue[Index].IP;
            ProcessOut();                                                               // Send ARP for new oldest
            return;
        }
    }

    m_PendingOldest = 0;                                                                // No pending entries remain
    m_IP_Address    = IP_ADDRESS(0,0,0,0);
}

//-------------------------------------------------------------------------------------------------
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
    int Index = (m_PendingOldest + Offset) % ARP_PENDING_QUEUE_SIZE;
    return &m_PendingQueue[Index];
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
void ARP_Manager::TimerCallBack(void)
{
    m_Time++;                                                           // Advance ARP time counter

    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)                          // Scan ARP table
    {
        ARP_TableEntry_t* pTable = &m_TableEntry[i];

        if(pTable->IP_Address != IP_ADDRESS(0,0,0,0))                   // Entry in use?
        {
            uint16_t Age = uint16_t(m_Time - pTable->TimeToLive);       // Unsigned wrap-safe age

            if(Age >= IP_ARP_TIME_OUT)                                  // Entry expired?
            {
              #if (IP_DBG_ARP == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Remove entry number %d\n", IP_A(pTable->IP_Address),
                                                                                                                     IP_B(pTable->IP_Address),
                                                                                                                     IP_C(pTable->IP_Address),
                                                                                                                     IP_D(pTable->IP_Address),
                                                                                                                     i);
              #endif

                memset(pTable, 0, sizeof(ARP_TableEntry_t));            // Clear entry
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//  Name:           ARP_TimerCallBack
//
//  Parameter(s):   nOS_Timer*  pTimer     Unused timer handle (provided by nOS)
//                  void*       pArg       Pointer to the ARP_Manager instance
//
//  Return:         void
//
//  Description:    Global nOS timer callback used to service ARP maintenance.
//
//                  - Calls ARP_Manager::TimerCallBack() to age ARP table entries.
//                  - Iterates through all pending ARP resolution requests.
//                  - For each pending entry, checks whether a VALID ARP table entry still exists.
//                  - If the ARP entry expired or was removed, the pending packet is dropped.
//
//                  This prevents stale packets from remaining queued indefinitely when ARP
//                  resolution fails or times out.
//-------------------------------------------------------------------------------------------------
void ARP_TimerCallBack(nOS_Timer* pTimer, void* pArg)
{
    VAR_UNUSED(pTimer);
    ARP_Manager* pARP = (ARP_Manager*)pArg;

    pARP->TimerCallBack();                                                          // Age ARP table entries
    ARP_TableEntry_t* pTable = pARP->GetTableEntryPointer(0);

    for(int Offset = 0; Offset < ARP_PENDING_QUEUE_SIZE; Offset++)                  // Scan entire pending queue in circular order
    {
        ARP_PendingEntry_t* pEntry = pARP->GetPendingEntryByOffset(Offset);

        if(pEntry->State == ARP_STATE_EMPTY)                                        // Skip empty slots
        {
            continue;
        }

        bool Resolved = false;                                                      // Check if ARP table has resolved this IP

        for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
        {
            if((pTable[i].IP_Address == pEntry->IP) && (pTable[i].State == ARP_STATE_VALID))
            {
                Resolved = true;
                break;
            }
        }

        if(Resolved == false)                                                       // If unresolved → timeout this entry
        {
            pARP->OnPendingTimeOut(Offset);
            break;                                                                  // Let next timer tick re-scan
        }
    }
}

//-------------------------------------------------------------------------------------------------
