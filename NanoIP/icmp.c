//-------------------------------------------------------------------------------------------------
//
//  File : icmp.c
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
// Internet Control Message Protocol (ICMP)  
//  -Provides "ping" support only
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define ICMP_GLOBAL
#include <ip.h>

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private macro(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private function(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ICMP_Init
// 
//  Parameter(s):   None
//  Return:         void
//
//  Description:
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void ICMP_Init(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:          ICMP_Process
// 
//  Parameter(s):  IP_PacketMsg_t* 		RX packet 
//  Return:        IP_PacketMsg_t*      TX packet
//
//  Description:    
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* ICMP_Process(IP_PacketMsg_t* pRX)
{
	uint8_t 		  	 		Error;
	IP_PacketMsg_t*  		pTX  		= nullptr;
	IP_ICMP_Frame_t* 		pICMP;
	IP_EthernetHeader_t* 	pETH;
	int16_t					wCount;

	if(IP_Status.b.IP_IsValid == YES)
	{
		if(pRX->wPacketSize < sizeof(IP_ICMP_Frame_t))
		{
			return(nullptr);
		}
	
		switch(pRX->Packet.u.ICMP_Frame.Header.byType)
		{
			case ICMP_TYPE_PING_REQUEST:
	
				pTX = IP_GetPacketMemory(pRX->wPacketSize, &Error);					// Get memory for TX packet
				pICMP = &pTX->Packet.u.ICMP_Frame;
				pETH  = &pTX->Packet.u.ETH_Header;
				IP_CopyPacketMessage(pTX, pRX);											// copy the entire IP payload From RX to TX buffer
				wCount  = htons(pICMP->IP_Header.wLenght);
				wCount -= (int16_t)sizeof(IP_IP_Header_t);
				pICMP->Header.byType           = ICMP_TYPE_PING_REPLY;
				pICMP->Header.wChecksum        = 0;
				pICMP->Header.wChecksum		   = IP_CalculateChecksum(&pICMP->Header, wCount);
	
				LIB_memcpy(pETH->Dst.baAddr, pETH->Src.baAddr, 6);						// Put Mac header
				pICMP->IP_Header.byTimeToLive  = IP_TIME_TO_LIVE;
				pICMP->IP_Header.DstIP_Addr.dw = pICMP->IP_Header.SrcIP_Addr.dw;
				pICMP->IP_Header.SrcIP_Addr.dw = IP_HostAddr.dw;
				IP_PutHeader(pTX);
				break;
		}
	}
	return(pTX);
}

//-------------------------------------------------------------------------------------------------

