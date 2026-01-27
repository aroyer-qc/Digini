//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_arp.cpp
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
SystemState_e NetARP::Initialize(NetworkContext* pContext)
{
    nOS_Error Error;

    m_pContext = pContext;

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
void NetARP::ProcessIP(IP_PacketMsg_t* pRX)
{
    IP_Address_t SubnetMask = m_pContext->GetActiveSubnetMask();
    IP_Address_t ActiveIP   = m_pContext->GetActiveIP();

        // Interface not configured yet -> do not learn from IP traffic
    if((ActiveIP == 0) || (SubnetMask == 0))
    {
        return;
    }

    IP_Address_t SourceIP = pRX->pPacket->IP_Frame.Header.SrcIP_Addr;

    if((SourceIP & SubnetMask) == (ActiveIP & SubnetMask))
    {
        UpdateEntry(SourceIP, &pRX->pPacket->ETH_Header.SourceMAC);
    }
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
void NetARP::ProcessARP(IP_PacketMsg_t* pRX)
{
    // Sanity check: must have at least ARP frame (sans ETH header)
    if((pRX == nullptr) || (pRX->pPacket == nullptr) || (pRX->PacketSize < (sizeof(ARP_Frame_t) - sizeof(IP_EthernetHeader_t))))
    {
        IP_Manager::FreeMessage(pRX);
        return;
    }

    ARP_Frame_t* pRX_ARP = &pRX->pPacket->ARP_Frame;

    switch(ntohs(pRX_ARP->Opcode))
    {
        case ARP_REQUEST:
        {
            // Respond only if the ARP request targets our IP address
            if(pRX_ARP->DstIP_Address == m_pContext->GetActiveIP())
            {
                // Re-tag for zero-copy reuse
                pMemoryPool->ChangeDebugID(pRX,          MEM_DBG_IPPKT,   MEM_DBG_ARP);
                pMemoryPool->ChangeDebugID(pRX->pPacket, MEM_DBG_ETHDMARX2, MEM_DBG_ARPDT);

                // Zero-copy: reuse RX buffer as TX
                IP_PacketMsg_t*      pTX  = pRX;
                IP_EthernetHeader_t* pETH = &pTX->pPacket->ETH_Header;
                ARP_Frame_t*         pARP = &pTX->pPacket->ARP_Frame;

                // Ethernet header
                memcpy(pETH->DestinationMAC.Byte, pETH->SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);    // Destination = requester MAC
                m_pContext->GetMAC_Address(&pETH->SourceMAC);                                       // Source = our MAC

                pETH->Type = htons(IP_ETHERNET_TYPE_ARP);

                // ARP payload Fixed fields
                pARP->HardwareType       = htons(ARP_HARDWARE_TYPE_ETHERNET);
                pARP->Protocol           = htons(IP_ETHERNET_TYPE_IP);
                pARP->HardwareAddrLength = IP_MAC_ADDRESS_SIZE;
                pARP->ProtocolLength     = 4;
                pARP->Opcode             = htons(ARP_REPLY);

                memcpy(pARP->SourceMAC.Byte, pETH->SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);            // Sender = us
                memcpy(pARP->DestinationMAC.Byte, pETH->DestinationMAC.Byte, IP_MAC_ADDRESS_SIZE);    // Target = original requester
                pARP->SrcIP_Address = m_pContext->GetActiveIP();
                pARP->DstIP_Address = pRX_ARP->SrcIP_Address;
                m_pContext->SendPacket(pTX);                                                        // Send ARP reply via normal TX path (zero-copy)

                // IMPORTANT: do not free pRX, it is now TX
                return;
            }
        }
        break;
        case ARP_REPLY:
        {
            // We learn this only if we are the destination
            if (pRX_ARP->DstIP_Address == m_pContext->GetActiveIP())
            {
                UpdateEntry(pRX_ARP->SrcIP_Address, &pRX_ARP->SourceMAC);
            }
        }
        break;

        default:
            // Unknown Opcode -> so it is ignore
            break;
    }

    // In all case, ARP est owner of pRX -> we free it (not for ARP_REQUEST if it is for us)
    IP_Manager::FreeMessage(pRX);
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
void NetARP::UpdateEntry(IP_Address_t IP_Address, IP_MAC_Address_t* pMAC_Address)
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
                DBG_Printf("ARP Cache - (%d.%d.%d.%d) Update an existing entry %d\n", uint8_t(pTable->IP_Address >> 24),
				                                                                      uint8_t(pTable->IP_Address >> 16),
 																					  uint8_t(pTable->IP_Address >> 8),
																					  uint8_t(pTable->IP_Address),
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
			DBG_Printf("ARP Cache - Found a free entry %d\n", i);
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
		DBG_Printf("ARP Cache - (%d.%d.%d.%d) Flush an old entry %d\n", uint8_t(pTable->IP_Address >> 24),
		                                                                uint8_t(pTable->IP_Address >> 16),
																		uint8_t(pTable->IP_Address >> 8),
																		uint8_t(pTable->IP_Address),
																		i);
      #endif
	}

	// Now, pTable pointer is on ARP table entry which we will fill with the new information.
	pTable->IP_Address = IP_Address;
  #if (IP_DBG_ARP == DEF_ENABLED)
	DBG_Printf("ARP Cache - (%d.%d.%d.%d) Added a new entry %d\n", uint8_t(pTable->IP_Address >> 24),
	                                                               uint8_t(pTable->IP_Address >> 16),
																   uint8_t(pTable->IP_Address >> 8),
																   uint8_t(pTable->IP_Address),
																   i);
  #endif
	memcpy(pTable->MAC_Address.Byte, pMAC_Address->Byte, IP_MAC_ADDRESS_SIZE);
    pTable->TimeToLive = m_Time;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_ProcessOut
//
//  Parameter(s):   IP_PacketMsg_t* 	pTX
//  Return:         none
//
//  Description:    Check if the outbound IP packet need a ARP request first.
//
//                  This function is called before sending out an IP packet.
// 					The function check the destination IP address of the packet to see what MAC
//                  address to use as a destination MAC address.
//
// 					If the destination IP address is in the local network the function check the
//                  ARP cache if an entry for this IP address is found. If so, the packet is filled
//                  with the missing destination MAC and the packet is sent. If no ARP cache entry
//                  is found for the destination IP address, the packet is sent back in the Queue
//                  of the TaskIP and it is replace by an ARP request packet for the IP destination
//                  address. The IP packet will be retransmitted later (After ARP process).
//
// 					If the destination IP address is not on the local network, the IP address of
//                  the default router is used instead.
//
//-------------------------------------------------------------------------------------------------
void NetARP::ProcessOut(IP_PacketMsg_t* pTX)
{
	uint8_t              i;
	ARP_TableEntry_t*    pTable   	= nullptr;
//	IP_ARP_Frame_t*      pARP;
	IP_EthernetPacket_t* pFrame;

	if(pTX != nullptr)                        		// If data are to be sent back, then send the data
	{
        IP_Address_t SubnetMask = m_pContext->GetActiveSubnetMask();

//		pARP	= &pTX->pPacket->ARP_Frame;
		pFrame  = pTX->pPacket;

        // Check if the destination address is on the local network.
		if((pFrame->IP_Frame.Header.DstIP_Addr & SubnetMask) != (m_pContext->GetActiveIP() & SubnetMask))
		{
			// Use the default router's IP address instead of the destination
			//IP_Address = m_pContext->GetIP_     DefaultGatewayAddress;
		}
		else
		{
			// Else, we use the destination IP address.
			//IP_Address = pFrame->u.IP_Frame.Header.DstIP_Address;
		}

		for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
		{
			pTable = &m_TableEntry[i];

            if(m_IP_Address == pTable->IP_Address)
			{
				break;
			}
		}

		if(i == IP_ARP_TABLE_SIZE)
		{
			// The destination address is not in our ARP table
			// Send a ARP request instead

			//memset(pARP->ETH_Header.Dst.Address, 0xFF, IP_MAC_ADDRESS_SIZE);
			//memset(pARP->Dst.Address, 0x00, IP_MAC_ADDRESS_SIZE);
			//memcpy(pARP->ETH_Header.Src.Address, MAC.Address, IP_MAC_ADDRESS_SIZE);
			//memcpy(pARP->Src.Address, MAC.Address, IP_MAC_ADDRESS_SIZE);

			//pARP->DstIP_Address      = IP_Address;
			//pARP->SrcIP_Address      = IP_HostAddress;
			//pARP->Opcode             = ARP_REQUEST;
			//pARP->HardwareType       = ARP_HARDWARE_TYPE_ETHERNET;
			//pARP->Protocol           = htons(IP_ETHERNET_TYPE_IP);
			//pARP->HardwareAddrLength = IP_MAC_ADDRESS_SIZE;
			//pARP->ProtocolLengtht    = 4;
			//pARP->ETH_Header.Type    = htons(IP_ETHERNET_TYPE_IP);

			//pTX->PacketSize = sizeof(IP_ARP_Frame_t);
			//return;
		}

		//// Build an ethernet header.
		//memcpy(pFrame->pPacket->ETH_Header.Dst.Address, pTable->Ethernet.Address, IP_MAC_ADDRESS_SIZE);
		//memcpy(pFrame->u.ETH_Header.Src.Address, MAC.Address, IP_MAC_ADDRESS_SIZE);
		//
		//pFrame->u.ETH_Header.Type = htons(IP_ETHERNET_TYPE_IP);

		//pTX->PacketSize += sizeof(IP_EthernetHeader_t);

		//NIC_Send(pTX);
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ARP_Resolve
//
//  Parameter(s):   IP      IPv4 address to resolve.
//                  pMAC    Pointer to a MAC address structure to receive the result
//                          when the address is already known.
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
//                  - If no entry exists, a new PENDING entry is created (if space is
//                    available) and an ARP request is triggered via ProcessOut().
//                    The function then returns false.
//
//                  - If a PENDING entry already exists, no new request is sent and the
//                    function returns false.
//
//                  - If the ARP table is full, no entry is created and the function
//                    returns false.
//
//  Note(s):        This function does not transmit packets directly. It only manages ARP
//                  table state and triggers ARP requests when needed. The caller (e.g.,
//                  IP_Manager::SendPacket) is responsible for retrying transmission once
//                  the entry becomes VALID.
//
//-------------------------------------------------------------------------------------------------
bool NetARP::Resolve(IP_Address_t IP, IP_MAC_Address_t* pMAC)
{
    // Search ARP table
    for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
    {
        if((m_TableEntry[i].IP_Address == IP) && (m_TableEntry[i].State == ARP_STATE_VALID))
        {
            // Found -> return MAC
            memcpy(pMAC->Byte, m_TableEntry[i].MAC_Address.Byte, IP_MAC_ADDRESS_SIZE);
            return true;
        }
    }

    // Not found -> trigger ARP request
    m_IP_Address = IP;          // store target IP for ARP request
    ProcessOut(nullptr);        // send ARP request (your existing function)
    return false;               // unresolved → caller must retry later
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
void NetARP::TimerCallBack(void)
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
   		      	DBG_Printf("ARP Cache - (%d.%d.%d.%d) Remove entry number %d\n", uint8_t(pTable->IP_Address >> 24),
                                                                 				 uint8_t(pTable->IP_Address >> 16),
																				 uint8_t(pTable->IP_Address >> 8),
																				 uint8_t(pTable->IP_Address),
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
//                  void*       pArg            holding the object of the class NetARP
//
//  Return:         void
//
//  Description:    Callback for the nOS_Timer to handle ARP time out. Will call the proper object
//
//-------------------------------------------------------------------------------------------------

void ARP_TimerCallBack(nOS_Timer* pTimer, void* pArg)
{
    VAR_UNUSED(pTimer);
    NetARP* pARP = (NetARP*)pArg;

    pARP->TimerCallBack();
}

//-------------------------------------------------------------------------------------------------
