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
//  ICMP - Internet Control Message Protocol
//
//  -Provides "ping" support only
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:         	ICMP_Initialize
// 
//  Parameter(s):   None
//  Return:         void
//
//  Description:
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void ICMP_Initialize(void)
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
	IP_PacketMsg_t*  		pTX  		= nullptr;
	IP_ICMP_Frame_t* 		pICMP;
	IP_EthernetHeader_t* 	pETH;
	uint16_t                Count;

	if(pIP->GetIP_isItValid() == true)
	{
		if(pRX->PacketSize < sizeof(IP_ICMP_Frame_t))
		{
			return nullptr;
		}
	
		switch(pRX->Packet.u.ICMP_Frame.Header.byType)
		{
			case ICMP_TYPE_PING_REQUEST:
            {
				pTX   = (IP_PacketMsg_t*)pMemory->AllocAndClear(pRX->PacketSize);					    // Get memory for TX packet
				pICMP = &pTX->Packet.u.ICMP_Frame;
				pETH  = &pTX->Packet.u.ETH_Header;
				IP_CopyPacketMessage(pTX, pRX);											                // Copy the entire IP payload From RX to TX buffer
				Count  = htons(pICMP->IP_Header.Length);
				Count -= (int16_t)sizeof(IP_IP_Header_t);
				pICMP->Header.Type     = ICMP_TYPE_PING_REPLY;
				pICMP->Header.Checksum = 0;                                 // TODO fix this
				pICMP->Header.Checksum= IP_CalculateChecksum(&pICMP->Header, Count);
	
				memcpy(pETH->Dst.Address, pETH->Src.Address, 6);						                // Put Mac header
				pICMP->IP_Header.TimeToLive    = IP_TIME_TO_LIVE;
				pICMP->IP_Header.DstIP_Address = pICMP->IP_Header.SrcIP_Address;
				pICMP->IP_Header.SrcIP_Address = IP_HostAddress;
				IP_PutHeader(pTX);
            }
            break;
            
            default: break; // No support for other ICMP command
		}
	}
	return pTX;
}

//-------------------------------------------------------------------------------------------------
