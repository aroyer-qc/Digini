//-------------------------------------------------------------------------------------------------
//
//  File : udp.cpp
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

//------ Note(s) ----------------------------------------------------------------------------------
//          
//  UDP - User Datagram Protocol
//          
// 	PSEUDO HEADER to calculate Checksum
//
//	_____________________________________________________________
// 	|       |   0 - 7   |    8 - 15    |   16 - 23   |  24- 31   |
//  |_______|__________________________|_________________________| 
// 	|   0   |    			  Source Address                     |
//  |_______|____________________________________________________|      Pseudo Header
// 	|  32   |    			Destination Address                  |
//  |_______|____________________________________________________| 
// 	|  64   | Zero's   	| Protocol 11h |      UDP Lenght         |
//  |_______|___________|______________|_________________________| __________________________
// 	|  96   |        Source Port       |    Destination Port     |
//  |_______|__________________________|_________________________|       Real Header
// 	|  128  |          Lenght          |       Checksum          |
//  |_______|__________________________|_________________________|___________________________ 
// 	|       |                                                    |
// 	|  160  |                       Data                         |           Data
//  |_______|____________________________________________________| 
// 
// 
//  The UDP Lenght field is the lenght of the Pseudo UDP header and Real Header + data
// 
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:         	Initialize
// 
//  Parameter(s):   none
//  Return:         void
//
//  Description:    
//
//-------------------------------------------------------------------------------------------------
void NetUDP::Initialize(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UDP_Process	
// 
//  Parameter(s):   IP_PacketMsg_t* pMsg 
//  Return:         void 
//
//  Description:    
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* NetUDP::Process(IP_PacketMsg_t* pRX)
{
	IP_PacketMsg_t* 		pTX = nullptr;
	PortInfo_t* 			pPort;

	if(pRX->Packet.u.UDP_Frame.Header.SrcPort == UDP_PORT_BOOT_P_SERVER)
	{
		pDHCP->Process(pRX); 																				    // No TX already process in DHCP
	}
	else
	{
		if((pPort = SOCK_ValidPort(pRX->Packet.u.UDP_Frame.Header.DstPort, IP_PROTOCOL_UDP)) != nullptr)	// Check first if the port is in our allowed port list
		{
			pPort->pFunction(nullptr, nullptr, 0);		// TO DO define UDP payload in frame so we can pass the address of the data easily or maybe copy data and post message
		}
	}
	return pTX;

// reference
	// use this if other case need a valid IP
	//if(IP_Status.b.IP_IsValid == true)

}

//-------------------------------------------------------------------------------------------------

