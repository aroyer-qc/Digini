//-------------------------------------------------------------------------------------------------
//
//  File : ip.c
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

#include <stddef.h>

#define IP_GLOBAL
#include <ip.h>
#include <argo_ip.h>
#include <debug.h>
#include <E2_record.h>
#include <library.h>
#include <iom16c62p.h>

//-------------------------------------------------------------------------------------------------
// Private macro(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private function(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:         	IP_Init
// 
//  Parameter(s):   none
//  Return:         void
//
//  Description:    Initialize the Stack IP
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IP_Init(void)
{
	int16_t wPort;

	// Setup the Ethernet MAC
	//IP_SetEthernetAddress(IP_ETH_ADDR1, IP_ETH_ADDR2, IP_ETH_ADDR3, IP_ETH_ADDR4, IP_ETH_ADDR5, IP_ETH_ADDR6);
	//E2_WriteRecord(&IP_MAC.baAddr[0], E2_ETHERNET_MAC_ADDRESS, 0, 0);

   //while(1)
   //{
      E2_ReadRecord(&IP_MAC.baAddr[0], E2_ETHERNET_MAC_ADDRESS, 0, 0);
   //   OSTimeDly(50);
   //}

	crcin = IP_MAC.baAddr[5] ^ IP_MAC.baAddr[4];	// Incremental Sequence ID put in each packet
	crcin = IP_MAC.baAddr[4];                       // we start according to a value generated 
	IP_wSequenceID = crcd;					        // CRC from MAC address
												
    //IP_Config.bUseDHCP     = YES;
	//E2_WriteRecord(&IP_Config, E2_CONFIGURATION_IP, 0, 0);	// Load IP configuration
	E2_ReadRecord(&IP_Config, E2_CONFIGURATION_IP, 0, 0);		// Load IP configuration

	ARP_Init();
	DHCP_Init();
	SOCK_Init();
	TCP_Init();

	// Initialize the device driver.
	NIC_Init();

	// Setup the IP address
	 //if(IP_Config.bUseDHCP  == NO)
	{
		//IP_SetHostAddress(0,0,0,0);
		//IP_SetHostAddress(192,168,1,120);
		//E2_WriteRecord(&IP_HostAddr.dw, E2_STATIC_HOST_IP, 0, 0);
		E2_ReadRecord(&IP_HostAddr.dw, E2_STATIC_HOST_IP, 0, 0);
	}

	// Setup the IP subnet mask
	//IP_SetNetMaskAddress(0,0,0,0);
	//IP_SetNetMaskAddress(255,255,255,0);
	//E2_WriteRecord(&IP_SubnetMaskAddr.dw, E2_SUBNET_MASK_IP, 0, 0);
	E2_ReadRecord(&IP_SubnetMaskAddr.dw, E2_SUBNET_MASK_IP, 0, 0);

	// Setup the IP default gateway
	//IP_SetDefaultGatewayAddress(0,0,0,0);
	//IP_SetDefaultGatewayAddress(192,168,1,1);
	//E2_WriteRecord(&IP_DefaultGatewayAddr.dw, E2_DEFAULT_GATEWAY_IP, 0, 0);
	E2_ReadRecord(&IP_DefaultGatewayAddr.dw, E2_DEFAULT_GATEWAY_IP, 0, 0);

	//E2_ReadRecord(&wPort, E2_HTTP_PORT, 0, 0);
	//SOCK_OpenPort(wPort);
	SOCK_OpenPort(TCP_HTTP_PORT, IP_PROTOCOL_TCP, nullptr);

	//E2_ReadRecord(&wPort, E2_SPECIAL_APP_PORT, 0, 0);
	//SOCK_OpenPort(wPort);
	SOCK_OpenPort(htons(12345), IP_PROTOCOL_TCP, nullptr);

    
  #if (IP_DBG_STACK_IP == DEF_ENABLED)
    DBG_Printf("Ethernet MAC      %02X %02X %02X %02X %02X %02X\n", IP_MAC.baAddr[0], IP_MAC.baAddr[1], IP_MAC.baAddr[2], IP_MAC.baAddr[3], IP_MAC.baAddr[4], IP_MAC.baAddr[5]);
	DBG_Printf("Host IP           %d.%d.%d.%d\n", IP_HostAddr.by.by0, IP_HostAddr.by.by1, IP_HostAddr.by.by2, IP_HostAddr.by.by3);
	DBG_Printf("SubNet Mask IP    %d.%d.%d.%d\n", IP_SubnetMaskAddr.by.by0, IP_SubnetMaskAddr.by.by1, IP_SubnetMaskAddr.by.by2, IP_SubnetMaskAddr.by.by3);
	DBG_Printf("Default router IP %d.%d.%d.%d\n\n", IP_DefaultGatewayAddr.by.by0, IP_DefaultGatewayAddr.by.by1, IP_DefaultGatewayAddr.by.by2, IP_DefaultGatewayAddr.by.by3);
  #endif
	
	OSTimeDly(400);


	// Initialize the HTTP server.
	//HTTPD_Init();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_Process	
// 
//  Parameter(s):   IP_PacketMsg_t* pMsg 
//  Return:         void 
//
//  Description:    
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* IP_Process(IP_PacketMsg_t* pRX)
{
	IP_PacketMsg_t* pTX = nullptr;

	switch(pRX->Packet.u.IP_Frame.Header.byProtocol)
	{
		case IP_PROTOCOL_ICMP:
		{
			pTX = ICMP_Process(pRX);
			break;
		}
		case IP_PROTOCOL_UDP:
		{
			pTX = UDP_Process(pRX);
			break;
		}
		case IP_PROTOCOL_TCP:
		{
			pTX = TCP_Process(pRX);
			break;
		}
	}

	return(pTX);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_PutHeader	
// 
//  Parameter(s):   void* 	pBuffer
// 				    int16_t 	wCount 
//  Return:         void 
//
//  Description:    Put in header everything static
//	
//  Requirement:	All other data must be already in the header
// 
//  Notes:			UDP packet should be set before the IP, because UDP use same data space to
// 					calculate it's own checksum from pseudo header + UDP datagram
//
//-------------------------------------------------------------------------------------------------
void IP_PutHeader(IP_PacketMsg_t* pTX)
{
	IP_IP_Header_t* 	pIP_TX;

	pIP_TX = &pTX->Packet.u.IP_Frame.Header;

	// Setup Ethernet header
	LIB_memcpy(pTX->Packet.u.ETH_Header.Src.baAddr, IP_MAC.baAddr, 6);		// Put our MAC in it
	pTX->Packet.u.ETH_Header.wType = IP_ETHERNET_TYPE_IP;

	// Setup IP header
	pIP_TX->wID			    = htons(IP_wSequenceID++);
	pIP_TX->byVersionIHL 	= IP_VERSION4_IHL20;
    pIP_TX->byTimeToLive 	= IP_TIME_TO_LIVE;

	pIP_TX->wChecksum 		= 0;
	pIP_TX->wChecksum 		= IP_CalculateChecksum(pIP_TX, (int16_t)sizeof(IP_IP_Header_t));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IP_CalculateChecksum	
// 
//  Parameter(s):   void* 	pBuffer
// 				    int16_t 	wCount 
//  Return:         void 
//
//  Description:    Calculate the checksum of the IP header 
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int16_t IP_CalculateChecksum(void* pBuffer, int16_t wCount)
{
	int16_t 	i;
	int16_t*	Value;
	uint32_t 	Checksum = {0x00000000ul};

	i = wCount >> 1;
    Value = (int16_t*)pBuffer;

	// Calculate the sum of all words
	while(i--)
	{
		Checksum.dw += (int32_t)*Value++;
	}

	// Add in the sum of the remaining te, if present
	if(((Uint16_t*)&wCount)->b.b0)
	{
		Checksum.dw += (int32_t)*(uint8_t*)Value;
	}

	// Do an end-around carry (one's complement arithmatic)
	Checksum.dw = (int32_t)Checksum.wArray[0] + (int32_t)Checksum.wArray[1];

	// Do another end-around carry in case if the prior add 
	// caused a carry out
	Checksum.wArray[0] += Checksum.wArray[1];

	// Return the resulting checksum
	return(~Checksum.wArray[0]);
}

//-------------------------------------------------------------------------------------------------
