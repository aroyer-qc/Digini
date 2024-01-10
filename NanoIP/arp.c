//-------------------------------------------------------------------------------------------------
//
//  File : arp.c
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
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------
#define ARP_GLOBAL
#include <ip.h>

//-------------------------------------------------------------------------------------------------
// Private macro(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private function(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

void ARP_UpdateEntry				(uint32_t IP_Addr, IP_EthernetAddress_t* pEthAdr);
void ARP_TimerCallBack				(OS_TMR* pTmr, void* pArg);

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_Init
// 
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Clear ARP table of any entry
// 					Setup OS timer for ARP table entry
//
//-------------------------------------------------------------------------------------------------
void ARP_Init(void)
{
	uint8_t Error;

	// Clear the ARP cache table
	for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
	{
		ARP_TableEntry[i].IP_Addr = 0;
	}

	// Initialize an OS timer for the ARP timer
	pARP_Timer = OSTmrCreate((int32_t)OS_TMR_CFG_TICKS_PER_SEC * 10,
							 (int32_t)OS_TMR_CFG_TICKS_PER_SEC * 10,	// Period is define in ip_cfg.h
							 OS_TMR_OPT_PERIODIC,					// it will repeat indefinitely
							 (OS_TMR_CALLBACK)ARP_TimerCallBack,	// this timer callback function
							 nullptr,	 								// no argument
							 nullptr,									// no names assign for debug
							 &Error);

	if(Error == ERR_NONE)	
	{
		OSTmrStart(pARP_Timer, &Error);
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_ProcessIP	
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
void ARP_ProcessIP(IP_PacketMsg_t* pRX)
{
	if((pRX->Packet.u.IP_Frame.Header.SrcIP_Addr & IP_SubnetMaskAddr) == (IP_HostAddr & IP_SubnetMaskAddr))
	{
		ARP_UpdateEntry(pRX->Packet.u.IP_Frame.Header.SrcIP_Addr, &pRX->Packet.u.ETH_Header.Src);
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_ProcessARP	
// 
//  Parameter(s):   IP_PacketMsg_t* 	pRX
//  Return:         void
//
//  Description:    
//	
//-------------------------------------------------------------------------------------------------
void ARP_ProcessARP(IP_PacketMsg_t* pRX)
{
	uint8_t 		Error;
	IP_ARP_Frame_t*	pRX_ARP;
	IP_PacketMsg_t* pTX = nullptr;
	IP_ARP_Frame_t*	pTX_ARP = nullptr;

	if(pRX->PacketSize < (sizeof(IP_ARP_Frame_t) - sizeof(IP_EthernetHeader_t)))
	{
		return;
	}

	pRX_ARP = &pRX->Packet.u.ARP_Frame;

	switch(pRX_ARP->wOpcode)
	{
		case ARP_REQUEST:
		{
            // ARP request. If it asked for our address, we send out a reply.
			if(pRX_ARP->DstIP_Addr == IP_HostAddr)
			{
				pTX = (IP_PacketMsg_t*)MemGetAndClear(pRX->wPacketSize + 2, &Error);			// Get memory for TX packet + Size 
				pTX->PacketSize = pRX->PacketSize;											    // Get the packet size from request packet (PING)
                pTX_ARP = &pTX->Packet.u.ARP_Frame;

				pTX_ARP->Opcode = ARP_REPLY;
                LIB_memcpy(pTX_ARP->Dst.Addr, pRX_ARP->Src.Addr, 6);
				LIB_memcpy(pTX_ARP->ETH_Header.Dst.Addr, pRX_ARP->Src.Addr, 6);
				LIB_memcpy(pTX_ARP->Src.Addr, IP_MAC.Addr, 6);
				LIB_memcpy(pTX_ARP->ETH_Header.Src.Addr, IP_MAC.Addr, 6);
	  
				pTX_ARP->DstIP_Addr = pRX_ARP->SrcIP_Addr;
				pTX_ARP->SrcIP_Addr = IP_HostAddr;

				pTX_ARP->HardwareType        = ARP_HARDWARE_TYPE_ETHERNET;
				pTX_ARP->Protocol            = IP_ETHERNET_TYPE_IP;
				pTX_ARP->HardwareAddrLenght  = 6;
				pTX_ARP->ProtocolLenght      = 4;
				pTX_ARP->ETH_Header.Type     = IP_ETHERNET_TYPE_ARP;
			}      
			break;
		}

		case ARP_REPLY:
		{
			// ARP reply. We insert or update the ARP table if it was for us.
			if((pRX_ARP->DstIP_Addr == IP_HostAddr))
			{
				ARP_UpdateEntry(pRX_ARP->SrcIP_Addr, &pRX_ARP->Src);
			}
		}
		break;
	}

	if(pTX != nullptr)
	{
		NIC_Send(pTX);
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_UpdateEntry	
// 
//  Parameter(s):   
//  Return:         
//
//  Description:    
//	
//-------------------------------------------------------------------------------------------------
void ARP_UpdateEntry(int32_t IP_dwAddr, IP_EthernetAddress_t* pEthernet)
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
		pTable = &ARP_TableEntry[i];
		// Only check those entries that are actually in use.
		if(pTable->IP_Addr != 0)
		{
            // Check if the source IP address of the incoming packet matches
			// the IP address in this ARP table entry.
			if(IP_dwAddr == pTable->IP_Addr)
			{
				// An old entry found, update this and return.
				LIB_memcpy(pTable->Ethernet.Addr, pEthernet->Addr, 6);
				pTable->Time = ARP_Time;
   		      #if (IP_DBG_ARP == DEF_ENABLED)
                DBG_Printf("ARP Cache - (%d.%d.%d.%d) Update an existing entry %d\n", pTable->IP_Addr.by.by0,
				                                                                      pTable->IP_Addr.by.by1,
 																					  pTable->IP_Addr.by.by2,
																					  pTable->IP_Addr.by.by3, i);
			  #endif
				return;
			}
		}
	}

	// If we get here, no existing ARP table entry was found, so we create one.

	// First, we try to find an unused entry in the ARP table.
	for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
	{
		pTable = &ARP_TableEntry[i];
		if(pTable->IP_Addr == 0)
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
		TimePage = 0;
		OldestEntry = 0;
		
        for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
		{
			pTable = &ARP_TableEntry[i];
			if((ARP_Time - pTable->Time) > TimePage)
			{
				TimePage = ARP_Time - pTable->Time;
				OldestEntry = i;
			}
		}
		i = OldestEntry; // for debug only

		pTable = &ARP_TableEntry[OldestEntry];
   	  #if (IP_DBG_ARP == DEF_ENABLED)
		DBG_Printf("ARP Cache - (%d.%d.%d.%d) Flush an old entry %d\n", pTable->IP_Addr.by.by0,
		                                                                pTable->IP_Addr.by.by1,
																		pTable->IP_Addr.by.by2,
																		pTable->IP_Addr.by.by3,
																		i);
      #endif
	}

	// Now, pTable pointer is on ARP table entry which we will fill with the new information.
	pTable->IP_Addr = IP_Addr;
  #if (IP_DBG_ARP == DEF_ENABLED)
	DBG_Printf("ARP Cache - (%d.%d.%d.%d) Added a new entry %d\n", pTable->IP_Addr.by.by0,
	                                                               pTable->IP_Addr.by.by1,
																   pTable->IP_Addr.by.by2,
																   pTable->IP_Addr.by.by3,
																   i);
  #endif
	LIB_memcpy(pTable->Ethernet.Addr, pEthernet->Addr, 6);
    pTable->Time = ARP_Time;
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
//                  of the TaskIP and it is replace  an ARP request packet for the IP destination
//                  address. The IP packet will be retransmitted later (After ARP process).
//
// 					If the destination IP address is not on the local network, the IP address of
//                  the default router is used instead. 
//  
//-------------------------------------------------------------------------------------------------
void ARP_ProcessOut(IP_PacketMsg_t* pTX)
{
	uint8_t              i;
	ARP_TableEntry_t*    pTable   	= nullptr;
	IP_ARP_Frame_t*      pARP;
	IP_EthernetPacket_t* pFrame;

	if(pTX != nullptr)                        		// If data are to be sent back, then send the data
	{                       

		pARP	= &pTX->Packet.u.ARP_Frame;
		pFrame  = &pTX->Packet;
		
        // Check if the destination address is on the local network.
		if((pFrame->u.IP_Frame.Header.DstIP_Addr & IP_SubnetMaskAddr) != (IP_HostAddr & IP_SubnetMaskAddr))
		{
			// Use the default router's IP address instead of the destination
			//IP_Addr = IP_DefaultGatewayAddr;
		}
		else
		{
			// Else, we use the destination IP address.
			//IP_Addr = pFrame->u.IP_Frame.Header.DstIP_Addr;
		}
		  
		for(i = 0; i < IP_ARP_TABLE_SIZE; i++)
		{
			pTable = &ARP_TableEntry[i];
			if(IP_Addr == pTable->IP_Addr)
			{
				break;
			}
		}
		
		if(i == IP_ARP_TABLE_SIZE)
		{
			// The destination address is not in our ARP table
			// Send a ARP request instead
		
			//LIB_memset(pARP->ETH_Header.Dst.Addr, 0xFF, 6);
			//LIB_memset(pARP->Dst.Addr, 0x00, 6);
			//LIB_memcpy(pARP->ETH_Header.Src.Addr, MAC.Addr, 6);
			//LIB_memcpy(pARP->Src.Addr, MAC.Addr, 6);
		
			//pARP->DstIP_Addr           = IP_Addr;
			//pARP->SrcIP_Addr           = IP_HostAddr;
			//pARP->wOpcode              = ARP_REQUEST;
			//pARP->wHardwareType        = ARP_HARDWARE_TYPE_ETHERNET;
			//pARP->wProtocol            = IP_ETHERNET_TYPE_IP;
			//pARP->ByHardwareAddrLenght = 6;
			//pARP->byProtocolLenght     = 4;
			//pARP->ETH_Header.wType     = IP_ETHERNET_TYPE_IP;
		
			//pTX->wPacketSize = (int16_t)sizeof(IP_ARP_Frame_t);
			//return;
		}

		//// Build an ethernet header.
		//LIB_memcpy(pFrame->u.ETH_Header.Dst.Addr, pTable->Ethernet.Addr, 6);
		//LIB_memcpy(pFrame->u.ETH_Header.Src.Addr, MAC.Addr, 6);
		//
		//pFrame->u.ETH_Header.wType = IP_ETHERNET_TYPE_IP;
	
		//pTX->wPacketSize += sizeof(IP_EthernetHeader_t);
	
		NIC_Send(pTX);
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_Resolve
// 
//  Parameter(s):   
// 					
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
void ARP_Resolve(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ARP_TimerCallBack
// 
//  Parameter(s):  	OS_TMR*		pTmr	N/U
// 					void* 		pArg    N/U
//  Return:         void
//
//  Description:    
// 
//-------------------------------------------------------------------------------------------------
void ARP_TimerCallBack(OS_TMR *pTmr, void* pArg)
{
	uint16_t          Time;
	ARP_TableEntry_t* pTable;

    VAR_UNUSED(pTmr):
	VAR_UNUSED(pArg);

	ARP_Time++;

	for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
	{
		pTable = &ARP_TableEntry[i];
		
        if(pTable->IP_Addr != 0)
		{
			Time = uint16_t(ARP_Time);
			
            if(ARP_Time < pTable->Time)
			{
				Time += uint16_t(IP_ARP_TIME_OUT);
			}

			if((Time - pTable->Time) >= IP_ARP_TIME_OUT)
			{
   		      #if (IP_DBG_ARP == DEF_ENABLED)
   		      	DBG_Printf("ARP Cache - (%d.%d.%d.%d) Remove entry number %d\n", uint8_t(pTable->IP_Addr >> 24),
                                                                 				 uint8_t(pTable->IP_Addr >> 16),
																				 uint8_t(pTable->IP_Addr >> 8),
																				 uint8_t(pTable->IP_Addr),
																				 i);
		      #endif
				pTable->IP_Addr = IP_ADDR(0,0,0,0);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
