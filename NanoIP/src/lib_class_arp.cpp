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

//-------------------------------------------------------------------------------------------------
//
//  Name:         	Initialize
//
//  Parameter(s):   SystemState_e
//  Return:         void
//
//  Description:    Clear ARP table of any entry
// 					Setup OS timer for ARP table entry
//
//-------------------------------------------------------------------------------------------------
SystemState_e ARP_Protocol::Initialize(NetworkContext* pContext)
{
    nOS_Error Error;

    m_pContext       = pContext;
    m_pPendingPacket = nullptr;

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
//
//  Name:         	ProcessIP
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//  Note(s):        Only insert or update an entry if the source IP address of the incoming IP
// 					packet comes from a host on the local network.
//
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::ProcessIP(IP_PacketMsg_t* pRX)
{
    IP_Address_t SubnetMask = m_pContext->GetActiveSubnetMask();
    IP_Address_t ActiveIP   = m_pContext->GetActiveIP();

    if((ActiveIP == IP_ADDRESS(0,0,0,0)) || (SubnetMask == IP_ADDRESS(0,0,0,0)))// Interface not configured yet -> do not learn from IP traffic
    {
        return;
    }

    IP_Address_t      DestIP   = pRX->pPacket->IP_Frame.Header.DstIP_Address;
    IP_MAC_Address_t* pDstMAC  = &pRX->pPacket->ETH_Header.DestinationMAC;
    IP_Manager* pIP_Manager = m_pContext->GetIP_Manager();


    if(pIP_Manager->IsItMulticast(DestIP))                                      // Ignore multicast IP
    {
        return;
    }

    if(pIP_Manager->IsItMulticastMAC(pDstMAC))                                  //  Ignore multicast MAC destination
    {
        return;
    }

    IP_Address_t      SourceIP = pRX->pPacket->IP_Frame.Header.SrcIP_Address;
    IP_MAC_Address_t* pSrcMAC  = &pRX->pPacket->ETH_Header.SourceMAC;

    if((SourceIP == 0)                                      ||                  // Ignore invalid IPs
       (SourceIP == ActiveIP)                               ||                  // Ignore our own IP
       ((SourceIP & SubnetMask) != (ActiveIP & SubnetMask)) ||                  // Ignore packets outside our subnet
       (pIP_Manager->IsItBroadcastMAC(pSrcMAC))             ||                  // Ignore broadcast MAC
       (pIP_Manager->IsItMulticastMAC(pSrcMAC))             ||                  // Ignore multicast MAC
       (m_pContext->IsItMyMAC_Address(pSrcMAC))             ||                  // Ignore our own MAC
       (m_pContext->IsItMyMAC_Address(pDstMAC) == false))                       // Ignore destination if it is not own MAC
    {
        return;
    }

    // Optional: ignore spoofed packets (MAC/IP mismatch)
    //if(ARP_Table.HasEntry(SourceIP) && (ARP_Table.MatchesMAC(SourceIP, pSrcMAC) == false))
    //{
    //    return; // suspicious -> ignore
    //}

    UpdateEntry(SourceIP, pSrcMAC);                                             // Passed all filters -> learn entry
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ProcessARP
//
//  Parameter(s):   IP_PacketMsg_t* 	pRX
//  Return:         void
//
//  Description:
//
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

                if(m_pPendingPacket != nullptr)                                                      // If a packet was waiting for this ARP resolution, resend it now
                {
                    memcpy(m_pPendingPacket->pPacket->ETH_Header.DestinationMAC.Byte, pRX_ARP->SourceMAC.Byte,  IP_MAC_ADDRESS_SIZE);                                                    // Update Ethernet destination MAC now that ARP is resolved
                    m_pContext->SendPacket(m_pPendingPacket);                                        // Send the packet
                    m_pPendingPacket = nullptr;                                                      // Clear pending pointer
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
//  Name:         	UpdateEntry
//
//  Parameter(s):   IP_Address_t            IP_Address
//                  IP_MAC_Address_t*       pEthernet
//  Return:         void
//
//  Description:    Update Entry in ARP table
//
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::UpdateEntry(IP_Address_t IP_Address, IP_MAC_Address_t* pMAC_Address)
{
	uint8_t           i;
	uint8_t           OldestEntry;
	uint8_t           TimePage;
	ARP_TableEntry_t* pTable	= nullptr;

	// Walk through the ARP mapping table and try to find an entry to
    // update. If none is found, the IP -> MAC address mapping is
    // inserted in the ARP table.
	for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
	{
		pTable = &m_TableEntry[i];
		// Only check those entries that are actually in use.
		if(pTable->IP_Address != 0)
		{
            // Check if the source IP address of the incoming packet matches
			// the IP address in this ARP table entry.
			if(IP_Address == pTable->IP_Address)
			{
				// An old entry found, update this and return.
				memcpy(pTable->MAC_Address.Byte, pMAC_Address->Byte, IP_MAC_ADDRESS_SIZE);
				pTable->TimeToLive = m_Time;
   		      #if (IP_DBG_ARP == DEF_ENABLED)
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Update an existing entry %d\n", IP_A(pTable->IP_Address),
                                                                                                                          IP_B(pTable->IP_Address),
                                                                                                                          IP_C(pTable->IP_Address),
                                                                                                                          IP_D(pTable->IP_Address),
                                                                                                                          i);
			  #endif
				return;
			}
		}
	}

	// If we get here, no existing ARP table entry was found, so we create one.

	// First, we try to find an unused entry in the ARP table.
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

	// If no unused entry is found, we try to find the oldest entry and throw it away.
	if(i == IP_ARP_TABLE_SIZE)
	{
		TimePage    = 0;
		OldestEntry = 0;

        for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
		{
			pTable = &m_TableEntry[i];

            if((m_Time - pTable->TimeToLive) > TimePage)
			{
				TimePage = m_Time - pTable->TimeToLive;
				OldestEntry = i;
			}
		}

		i = OldestEntry; // for debug only

		pTable = &m_TableEntry[OldestEntry];
   	  #if (IP_DBG_ARP == DEF_ENABLED)
		DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Flush an old entry %d\n", IP_A(pTable->IP_Address),
                                                                                                            IP_B(pTable->IP_Address),
                                                                                                            IP_C(pTable->IP_Address),
                                                                                                            IP_D(pTable->IP_Address),
                                                                                                            i);
      #endif
	}

	// Now, pTable pointer is on ARP table entry which we will fill with the new information.
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

  #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
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
//                                  (entry is missing, PENDING, or table is full)
//
//  Description:    Attempts to resolve an IPv4 address to a MAC address using the ARP table.
//
//                  - If a VALID entry exists for the given IP, the MAC address is copied
//                    to *pMAC and the function returns true.
//
//                  - If no entry exists, a new PENDING entry is created (if space allows)
//                    and an ARP request is triggered via ProcessOut(). The function then
//                    returns false.
//
//                  - If a PENDING entry already exists, no additional ARP request is sent
//                    and the function returns false.
//
//                  - If the ARP table is full, no entry is created and the function
//                    returns false.
//
//  Note(s):        This function does not transmit IP packets directly. It only manages ARP
//                  table state and triggers ARP requests when needed. The caller (e.g.,
//                  IP_Manager::SendPacket) must retry transmission once the ARP entry
//                  transitions to VALID.
//-------------------------------------------------------------------------------------------------
bool ARP_Protocol::Resolve(IP_Address_t IP, IP_MAC_Address_t* pMAC, IP_PacketMsg_t* pMsg)
{
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)													// Search ARP table
    {
        if((m_TableEntry[i].IP_Address == IP) && (m_TableEntry[i].State == ARP_STATE_VALID))
        {
            memcpy(pMAC->Byte, m_TableEntry[i].MAC_Address.Byte, IP_MAC_ADDRESS_SIZE);			// Found -> return MAC
            return true;
        }
    }

    // Not found -> trigger ARP request
    m_IP_Address = IP;          																// Store target IP for ARP request
    ProcessOut();               																// Send ARP request

    // TEMPORARY SAFETY: drop if already pending
    if (m_pPendingPacket != nullptr)
    {
        // At this time We drop the new packet to avoid overwriting the old one.
        // TODO: Replace with a proper pending queue.
        return false;
    }

  #if (IP_DBG_ARP_RETRY_MSG == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP: Stack Msg to send later\n");
  #endif
    m_pPendingPacket = pMsg;
    return false;               																// Unresolved -> caller must retry later
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
//                      - Opcode             (caller‑specified)
//
//                  Fields that depend on direction (source/destination MAC and IP addresses)
//                  are intentionally *not* set here and must be filled by the caller.
//
//  Note(s):        This function does not modify Ethernet header fields. It is used by both
//                  ARP_Protocol::ProcessARP() when generating ARP replies (zero‑copy) and
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
//
//  Name:         	TimerCallBack
//
//  Parameter(s):  	None
//
//  Return:         void
//
//  Description:    Callback for the nOS_Timer to handle ARP time out.
//
//-------------------------------------------------------------------------------------------------
void ARP_Protocol::TimerCallBack(void)
{
	uint16_t          Time;
	ARP_TableEntry_t* pTable;

	m_Time++;

	for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)                      // Scan Table for the entry
	{
		pTable = &m_TableEntry[i];

        if(pTable->IP_Address != 0)
		{
			Time = uint16_t(m_Time);

            if(m_Time < pTable->TimeToLive)
			{
				Time += uint16_t(IP_ARP_TIME_OUT);
			}

			if((Time - pTable->TimeToLive) >= IP_ARP_TIME_OUT)            // Remove entry from table
			{
   		      #if (IP_DBG_ARP == DEF_ENABLED)
   		      	DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ARP Cache - (%d.%d.%d.%d) Remove entry number %d\n", IP_A(pTable->IP_Address),
                                                                                                                     IP_B(pTable->IP_Address),
                                                                                                                     IP_C(pTable->IP_Address),
                                                                                                                     IP_D(pTable->IP_Address),
                                                                                                                     i);
		      #endif
				pTable->IP_Address = IP_ADDRESS(0,0,0,0);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_TimerCallBack
//
//  Parameter(s):  	nOS_Timer*  pTimer
//                  void*       pArg            holding the object of the class ARP_Protocol
//
//  Return:         void
//
//  Description:    Callback for the nOS_Timer to handle ARP time out. Will call the proper object
//
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
