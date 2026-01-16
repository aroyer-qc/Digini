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
SystemState_e NetARP::Initialize(void)
{
    nOS_Error Error;

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
    IP_Address_t SubnetMask = m_Context.GetActiveSubnetMask();
    IP_Address_t ActiveIP   = m_Context.GetActiveIP();
    IP_Address_t SourceIP   = pRX->pPacket->IP_Frame.Header.SrcIP_Addr;

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
    /*
    if((pRX == nullptr) || (pRX->pPacket == nullptr) || (pRX->PacketSize < (sizeof(IP_ARP_Frame_t) - sizeof(IP_EthernetHeader_t))))
    {
        if(pRX != nullptr)
        {
            if(pRX->pPacket != nullptr)
            {
                pMemoryPool->Free((void**)&pRX->pPacket);
            }

            pMemoryPool->Free((void**)&pRX);
        }

        return;
    }
    */

    ARP_Frame_t* pRX_ARP = &pRX->pPacket->ARP_Frame;
    IP_PacketMsg_t* pTX  = nullptr;
    ARP_Frame_t* pTX_ARP = nullptr;

    switch(pRX_ARP->Opcode)
    {
        case ARP_REQUEST:
        {
            // On ne répond que si la requête est pour notre IP
            if(pRX_ARP->DstIP_Address == m_Context.GetActiveIP())
            {
                // Allouer le wrapper TX
                pTX = (IP_PacketMsg_t*)pMemoryPool->AllocAndClear(sizeof(IP_PacketMsg_t), MEM_DBG_ARP);

                if(pTX == nullptr)
                {
                    break;  // pas de réponse, mais on va quand même free pRX plus bas
                }

                // Allouer le packet Ethernet complet
                pTX->pPacket = (IP_EthernetPacket_t*)pMemoryPool->AllocAndClear(pRX->PacketSize, MEM_DBG_ARPDT);

                if(pTX->pPacket == nullptr)
                {
                    pMemoryPool->Free((void**)&pTX);
                    break;  // pas de réponse
                }

                pTX->PacketSize = pRX->PacketSize;
                pTX_ARP         = &pTX->pPacket->ARP_Frame;

                // Construire la réponse ARP
                pTX_ARP->Opcode = ARP_REPLY;

                // MAC de destination = MAC source de la requête
                memcpy(pTX_ARP->DestinationMAC.Byte, pRX_ARP->SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);
                memcpy(pTX_ARP->ETH_Header.DestinationMAC.Byte, pRX_ARP->SourceMAC.Byte, IP_MAC_ADDRESS_SIZE);

                // MAC source = notre MAC
                m_Context.GetMAC_Address(&pTX_ARP->SourceMAC);
                m_Context.GetMAC_Address(&pTX_ARP->ETH_Header.SourceMAC);

                // IPs
                pTX_ARP->DstIP_Address = pRX_ARP->SrcIP_Address;
                pTX_ARP->SrcIP_Address = m_Context.GetActiveIP();

                // Champs ARP
                pTX_ARP->HardwareType       = ARP_HARDWARE_TYPE_ETHERNET;
                pTX_ARP->Protocol           = htons(IP_ETHERNET_TYPE_IP);
                pTX_ARP->HardwareAddrLength = IP_MAC_ADDRESS_SIZE;
                pTX_ARP->ProtocolLength     = 4;

                // Header Ethernet
                pTX_ARP->ETH_Header.Type = htons(IP_ETHERNET_TYPE_ARP);

                // Envoyer via ton chemin TX standard
                m_Context.SendPacket(pTX);
                // TX IRQ libérera pTX->pPacket et pTX (selon ton implémentation)
            }
        }
        break;

        case ARP_REPLY:
        {
            // On apprend seulement si la réponse nous est destinée
            if (pRX_ARP->DstIP_Address == m_Context.GetActiveIP())
            {
                UpdateEntry(pRX_ARP->SrcIP_Address, &pRX_ARP->SourceMAC);
            }
        }
        break;

        default:
            // Opcode inconnu → ignoré
            break;
    }

    // Dans tous les cas, ARP est propriétaire de pRX → on doit le libérer
    pMemoryPool->Free((void**)&pRX->pPacket);
    pMemoryPool->Free((void**)&pRX);
}

#if 0
void NetARP::ProcessARP(IP_PacketMsg_t* pRX)
{
	//uint8_t 		Error;
	ARP_Frame_t*	pRX_ARP;
	IP_PacketMsg_t* pTX     = nullptr;
	ARP_Frame_t*	pTX_ARP = nullptr;

	if(pRX->PacketSize < (sizeof(IP_ARP_Frame_t) - sizeof(IP_EthernetHeader_t)))
	{
		return;
	}

	pRX_ARP = &pRX->pPacket->ARP_Frame;

	switch(pRX_ARP->Opcode)
	{
		case ARP_REQUEST:
		{
            // ARP request. If it asked for our address, we send out a reply.
			if(pRX_ARP->DstIP_Addr == m_Context.GetActiveIP())
			{
                pTX = (IP_PacketMsg_t*)pMemoryPool->AllocAndClear(pRX->PacketSize + 2);     // Get memory for TX packet + Size
				pTX->PacketSize = pRX->PacketSize;											// Get the packet size from request packet (PING)
                pTX_ARP = &pTX->pPacket->ARP_Frame;

				pTX_ARP->Opcode = ARP_REPLY;
                memcpy(pTX_ARP->Dst.Byte, pRX_ARP->Src.Byte, IP_MAC_ADDRESS_SIZE);
				memcpy(pTX_ARP->ETH_Header.Dst.Byte, pRX_ARP->Src.Byte, IP_MAC_ADDRESS_SIZE);
                m_Context.GetMAC_Address(&pTX_ARP->Src);
                m_Context.GetMAC_Address(&pTX_ARP->ETH_Header.Src);
				pTX_ARP->DstIP_Addr = pRX_ARP->SrcIP_Addr;
				pTX_ARP->SrcIP_Addr = m_Context.GetActiveIP();

				pTX_ARP->HardwareType          = ARP_HARDWARE_TYPE_ETHERNET;
				pTX_ARP->Protocol              = htons(IP_ETHERNET_TYPE_IP);
				pTX_ARP->HardwareAddrLength    = IP_MAC_ADDRESS_SIZE;
				pTX_ARP->ProtocolLength        = 4;
				pTX_ARP->ETH_Header.Type       = htons(IP_ETHERNET_TYPE_ARP);
			}
		}
        break;

		case ARP_REPLY:
		{
			// ARP reply. We insert or update the ARP table if it was for us.
			if((pRX_ARP->DstIP_Addr == m_Context.GetActiveIP()))
			{
				UpdateEntry(pRX_ARP->SrcIP_Addr, &pRX_ARP->Src);
			}
		}
		break;
	}

	if(pTX != nullptr)
	{
		//NIC_Send(pTX);
	}
}
#endif
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
void NetARP::UpdateEntry(IP_Address_t IP_Address, IP_MAC_Address_t* pEthernet)
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
				memcpy(pTable->Ethernet.Byte, pEthernet->Byte, IP_MAC_ADDRESS_SIZE);
				pTable->Time = m_Time;
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

            if((m_Time - pTable->Time) > TimePage)
			{
				TimePage = m_Time - pTable->Time;
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
	memcpy(pTable->Ethernet.Byte, pEthernet->Byte, IP_MAC_ADDRESS_SIZE);
    pTable->Time = m_Time;
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
        IP_Address_t SubnetMask = m_Context.GetActiveSubnetMask();

//		pARP	= &pTX->pPacket->ARP_Frame;
		pFrame  = pTX->pPacket;

        // Check if the destination address is on the local network.
		if((pFrame->IP_Frame.Header.DstIP_Addr & SubnetMask) != (m_Context.GetActiveIP() & SubnetMask))
		{
			// Use the default router's IP address instead of the destination
			//IP_Address = m_Context.GetIP_     DefaultGatewayAddress;
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
//  Name:         	ARP_Resolve
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:   	Transmits an ARP request to resolve an IP address.
//  				This function transmits and ARP request to determine the hardware address of a
// 					given IP address.
//
//  Note(s):        This function is only required when the stack is a client, and therefore is
// 					only enabled when STACK_CLIENT_MODE is enabled.
//
//                  To retrieve the ARP query result, call the ARPIsResolved() function.
//
//-------------------------------------------------------------------------------------------------
void NetARP::Resolve(void)
{
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

            if(m_Time < pTable->Time)
			{
				Time += uint16_t(IP_ARP_TIME_OUT);
			}

			if((Time - pTable->Time) >= IP_ARP_TIME_OUT)            // Remove entry from table
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
