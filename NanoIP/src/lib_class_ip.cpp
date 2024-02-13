//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ip.c
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

#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:         	IP_Init
// 
//  Parameter(s):   none
//  Return:         void
//
//  Description:    Initialize the Stack IP
//	
//-------------------------------------------------------------------------------------------------
void IP_Init(void)
{
	uint16_t Port;

	// Setup the Ethernet MAC
	//IP_SetEthernetAddress(IP_ETH_Address1, IP_ETH_Address2, IP_ETH_Address3, IP_ETH_Address4, IP_ETH_Address5, IP_ETH_Address6);
	//E2_WriteRecord(&IP_MACAddress[0], E2_ETHERNET_MAC_Address, 0, 0);

   //while(1)
   //{
        DB_Central.Get(&IP_MACAddress[0], ETHERNET_MAC_Address, 0, 0);
   //   OSTimeDly(50);
   //}

	crcin = IP_MACAddress[5] ^ IP_MACAddress[4];	// Incremental Sequence ID put in each packet
	crcin = IP_MACAddress[4];                       // we start according to a value generated 
	IP_SequenceID = crcd;					        // CRC from MAC address
												
    //IP_Config.bUseDHCP     = YES;
	//DB_Central.Set(&IP_Config, E2_CONFIGURATION_IP);	// Save IP configuration
    DB_Central.Get(&IP_Config, E2_CONFIGURATION_IP);
	//E2_ReadRecord(&IP_Config, E2_CONFIGURATION_IP);		// Load IP configuration

	pARP->Initialize();
	pDHCP->Initialize();
	pSOCK->Initialize();
	pTCP->Initialize();

	// Initialize the device driver.
	NIC_Init();

	// Setup the IP address
	 //if(IP_Config.bUseDHCP  == NO)
	{
		//IP_SetHostAddress(0,0,0,0);
		//IP_SetHostAddress(192,168,1,120);
		//E2_WriteRecord(&IP_HostAddress, E2_STATIC_HOST_IP, 0, 0);
		DB_Central.Get(&IP_HostAddress, E2_STATIC_HOST_IP, 0, 0);
	}

	// Setup the IP subnet mask
	//IP_SetNetMaskAddress(0,0,0,0);
	//IP_SetNetMaskAddress(255,255,255,0);
	//E2_WriteRecord(&IP_SubnetMaskAddress, E2_SUBNET_MASK_IP, 0, 0);
	DB_Central.Get(&IP_SubnetMaskAddress, E2_SUBNET_MASK_IP, 0, 0);

	// Setup the IP default gateway
	//IP_SetDefaultGatewayAddress(0,0,0,0);
	//IP_SetDefaultGatewayAddress(192,168,1,1);
	//E2_WriteRecord(&IP_DefaultGatewayAddress, E2_DEFAULT_GATEWAY_IP, 0, 0);
	DB_Central.Get(&IP_DefaultGatewayAddress, E2_GATEWAY_IP, 0, 0);

	//E2_ReadRecord(&wPort, E2_HTTP_PORT, 0, 0);
	//SOCK_OpenPort(wPort);
	SOCK_OpenPort(TCP_HTTP_PORT, IP_PROTOCOL_TCP, nullptr);

	//E2_ReadRecord(&wPort, E2_SPECIAL_APP_PORT, 0, 0);
	//SOCK_OpenPort(wPort);
	SOCK_OpenPort(htons(12345), IP_PROTOCOL_TCP, nullptr);

    
  #if (IP_DBG_STACK_IP == DEF_ENABLED)
    DBG_Printf("Ethernet MAC      %02X %02X %02X %02X %02X %02X\n", IP_MACAddress[0], IP_MACAddress[1], IP_MACAddress[2], IP_MACAddress[3], IP_MACAddress[4], IP_MACAddress[5]);
	DBG_Printf("Host IP           %d.%d.%d.%d\n", uint8_t(IP_HostAddress >> 24), uint8_t(IP_HostAddress >> 16), uint8_t(IP_HostAddress >> 8), uint8_t(IP_HostAddress));
	DBG_Printf("SubNet Mask IP    %d.%d.%d.%d\n", uint8_t(IP_SubnetMaskAddress >> 24), uint8_t(IP_SubnetMaskAddress >> 16), uint8_t(IP_SubnetMaskAddress >> 8), uint8_t(IP_SubnetMaskAddress));
	DBG_Printf("Default router IP %d.%d.%d.%d\n\n", uint8_t(IP_DefaultGatewayAddress >> 24), uint8_t(IP_DefaultGatewayAddress >> 16), uint8_t(IP_DefaultGatewayAddress >> 8), uint8_t(IP_DefaultGatewayAddress));
  #endif
	
	OSTimeDly(400); // why


	// Initialize the HTTP server.
	//pHTTPD->Initialize();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process	
// 
//  Parameter(s):   IP_PacketMsg_t* pMsg 
//  Return:         void 
//
//  Description:    
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* NetIP:Process(IP_PacketMsg_t* pRX)
{
	IP_PacketMsg_t* pTX = nullptr;

	switch(pRX->Packet.u.IP_Frame.Header.Protocol)
	{
		case IP_PROTOCOL_ICMP:  {	pTX = pICMP->Process(pRX);  }   break;
        case IP_PROTOCOL_UDP:   {   pTX = pUDP->Process(pRX);   }	break;
		case IP_PROTOCOL_TCP:	{	pTX = pTCP->Process(pRX);	}	break;
        default                 {          /* trap debug */     }   break;
	}

	return pTX;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:          PutHeader	
// 
//  Parameter(s):   void* 	    pBuffer
// 				    uint16_t 	Count 
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
void NetIP::PutHeader(IP_PacketMsg_t* pTX)
{
	IP_IP_Header_t* 	pIP_TX;

	pIP_TX = &pTX->Packet.u.IP_Frame.Header;

	// Setup Ethernet header
	memcpy(pTX->Packet.u.ETH_Header.Src.Address, IP_MACAddress, 6);		// Put our MAC in it
	pTX->Packet.u.ETH_Header.Type = IP_ETHERNET_TYPE_IP;

	// Setup IP header
	pIP_TX->ID		    = htons(IP_SequenceID++);
	pIP_TX->VersionIHL 	= IP_VERSION4_IHL20;
    pIP_TX->TimeToLive 	= IP_TIME_TO_LIVE;

	pIP_TX->Checksum 		= 0;  // use lib checksum.. or make one
	pIP_TX->Checksum 		= IP_CalculateChecksum(pIP_TX, uint16_t(sizeof(IP_IP_Header_t)));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CalculateChecksum	
// 
//  Parameter(s):   void* 	    pBuffer
// 				    uint16_t 	Count 
//  Return:         void 
//
//  Description:    Calculate the checksum of the IP header 
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int16_t NetIP:CalculateChecksum(void* pBuffer, uint16_t Count)
{
	int16_t 	i;
	uint16_t*	Value;
	uint32_t 	Checksum = 0;

	i = Count >> 1;
    Value = (uint16_t*)pBuffer;

	// Calculate the sum of all words
	while(i--)
	{
		Checksum += (uint32_t)*Value++;
	}

	// Add in the sum of the remaining te, if present
	if(((uint16_t*)&Count)->b.b0)
	{
		Checksum += (uint32_t)*(uint8_t*)Value;
	}

	// Do an end-around carry (one's complement arithmatic)
	Checksum = (uint32_t)Checksum.Array[0] + (int32_t)Checksum.Array[1];

	// Do another end-around carry in case if the prior add 
	// caused a carry out
	Checksum.Array[0] += Checksum.Array[1];

	// Return the resulting checksum
	return ~Checksum.Array[0];
}

//-------------------------------------------------------------------------------------------------