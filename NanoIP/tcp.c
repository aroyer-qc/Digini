//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_IP.cpp
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
// 	 	Client                                  Server
//                            Flag's
//        	
//         	>---------------- SYN ----------------->	Request connection
//         	<-------------- SYN,ACK ---------------<
//         	>---------------- ACK ----------------->    note: request possible in ack for connection
//         
//        	>------------ ACK + data -------------->	Request (if more than one segment)
//          >---------- PSH,ACK + data ------------>    Request (last data segment of request)
//        	<---------------- ACK -----------------<
// 
//          <------------ ACK + data --------------<	Answers (if more than one segment)
//          <---------- PSH,ACK + data ------------<    Request (last data segment of Answers)
// 			>---------------- ACK ----------------->
//
// 			<---------------- FIN -----------------<
//          >------------- ACK (FIN) -------------->
//
//          Notes ... Packet are not necessarily in sequence
//
//
//
//
//*************************************************************************************************


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define TCP_GLOBAL
#include <ip.h>

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private macro(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

IP_PacketMsg_t* 	TCP_Ack			(SocketInfo_t* pSocket, uint8_t Flag, int16_t wSize);
void				TCP_Push		(SocketInfo_t* pSocket, IP_PacketMsg_t* pRX);
void				TCP_PutHeader	(SocketInfo_t* pSocket, IP_PacketMsg_t* pTX, int16_t wPacketSize);
IP_PacketMsg_t* 	TCP_Send		(SocketInfo_t* pSocket, uint8_t* pBuffer, int16_t wSize);

//-------------------------------------------------------------------------------------------------
// Private function(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:			TCP_Init
// 
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Initialize data structure for TCP and tiner for SOCKET
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void TCP_Init(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TCP_Process	
// 
//  Parameter(s):   IP_PacketMsg_t* pRX 
//  Return:         IP_PacketMsg_t* 
//
//  Description:    Process any incoming packet. First we check if the port is open, If it is,
// 					Basic negotiation is done here. request is then passed to the application
// 					using the listen() function with the right port pass as argument.
//	
//  Note(s):
//
//  client	>------- SYN ------->  Server
//  	 	<-- SYN (ACK SYN) --<
//  	 	>---- ACK (SYN) ---->
//       	
//  	 	>----- REQUEST ----->
//  	 	>------- FIN ------->
//       	<- ACK (Req + FIN) -<
//       	
//  	 	<----- ANSWER ------<
//  	 	<------- FIN -------<
//  	 	>---- ACK (FIN) ---->
//
//
//		Normal TCP Flag combination
// 
//			Except for the initial SYN packet, every packet in a connection must have the ACK bit set. 
//			
//          	handshake which establishes a TCP connection.
// 					SYN
//					SYN-ACK
//					ACK
//
//              Graceful teardown of an existing connection. 
//					FIN ACK
// 					ACK
//
//              Used to immediately terminate an existing connection. 
//					RST ACK 
//
// 				Packets during the "conversation" portion of the connection (after the three-way
//              handshake but before the teardown or termination) contain just an ACK  default. 
//				Optionally, they may also contain PSH and/or URG.
// 
// 
//      Abnormal TCP Flag combination
//				
// 				SYN FIN 				Packets with SYN FIN are malicious. in all forms 
//				SYN FIN PSH
//				SYN FIN RST
//              SYN FIN RST PSH
//              etc...
//     
// 				FIN 					Packets should never contain just a FIN flag
//              nullptr 		      		It is illegal to have a packet with no flags set. 
//
//      Some other strange condition are trap  the code
// 				SYN RST                 SYN must be alone for connection process to start
// 				FIN RST					RST is done in priority, so no packet is sent.
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* TCP_Process(IP_PacketMsg_t* pRX)
{
	IP_TCP_Header_t*		pTCP_RX;
	SocketInfo_t*			pSocket;
    PortInfo_t* 			pPort;
	uint8_t					Error;
	uint8_t					byFlag    = TCP_FLAG_nullptr;

	pTCP_RX = &pRX->Packet.u.TCP_Frame.Header;
    
	if((pPort = SOCK_ValidPort(pTCP_RX->wDstPort, IP_PROTOCOL_TCP)) != nullptr)	// Check first if the port is in our allowed port list
	{
		if((pTCP_RX->byFlags == TCP_FLAG_SYN) ||                                // Is it a SYN packet
		   ((pTCP_RX->byFlags & TCP_FLAG_ACK) == TCP_FLAG_ACK))          		// Packet other than SYN should alway have ACK bit set
		{
			pTCP_RX->byFlags &= ~(TCP_FLAG_ACK | TCP_FLAG_URG);            		// Remove ACK bit & URG bit ( don't care )

			if(pTCP_RX->byFlags == TCP_FLAG_SYN)								// Initiate a connection and create the socket
			{
				pSocket = SOCK_OpenSocket(pPort, pRX, &Error);
				byFlag = TCP_FLAG_SYN;
			}
        	else                                                                // Or get the already open socket
			{
				pSocket = SOCK_LookupSocket(pRX, &Error);
			}

            if(pSocket != nullptr)
			{
				pSocket->byTimer = SOCK_TIME_OUT;								// We received a packet than reset timeout

				switch(pTCP_RX->byFlags)
				{
					case TCP_FLAG_SYN:											// Close a connection
						pSocket->Send.wNext++;
						break;

					case TCP_FLAG_RST:                                     		// Force close of a connection
						// TO DO verify the sequence number for real Reset
						Error = SOCK_CloseSocket(pSocket);
						return(nullptr);
						break;

					case TCP_FLAG_PSH:                               			// Send Data to app
						// TO DO verify the sequence number
						TCP_Push(pSocket, pRX);
						break;

					case TCP_FLAG_FIN:											// Close a connection
						pSocket->Send.wNext++;
					//	pSocket->Receive.wNext   =  ntohl(pRX->Packet.u.TCP_Frame.Header.AcknowledgeNumber.dw - pSocket->AckNumber.dw); //??
						break;


					default:													// Here will end up any malicious bit combination
						return(nullptr);
						break;
				}

                // We reach this point we need to ACK!
				//	switch(pSocket->byConnectionState)
				//	{
				//		case TCP_SOCKET_SYN_RECEIVE:                            // Client has ack connection
				//			pSocket->byConnectionState 	= TCP_SOCKET_LISTENING;
				//			break;
                //
				//		case TCP_SOCKET_LISTENING:  break;
				//		case TCP_SOCKET_CLOSE_WAIT:	break;
				//	}
                //
{
	IP_PacketMsg_t* pTX;
	if(pTCP_RX->byFlags != TCP_FLAG_PSH)
	
					return(TCP_Ack(pSocket, Flag, 0));
	else
	{
		pTX = TCP_Ack(pSocket, Flag, 0);
		CS8900_Send(pTX);
		MemPut(&pTX);
		return(TCP_Send(pSocket, nullptr, 0));
	}
}


			}
		}
	}

	return(nullptr);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TCP_PutHeader	
// 
//  Parameter(s):   Socket_t* 			pSocket
//                  IP_PacketMsg_t* 	pTX
// 					int16_t                wPacketSize
//  Return:         void 
//
//  Description:    Put in header everything static
//	
//  Requirement:	All other data must be already in the header
// 
//-------------------------------------------------------------------------------------------------
void TCP_PutHeader(SocketInfo_t* pSocket, IP_PacketMsg_t* pTX, int16_t wPacketSize)
{
	IP_TCP_Header_t*	pTCP_TX;
	IP_PseudoHeader_t* 	pPseudo_TX;

	pTX->Packet.u.IP_Frame.Header.DstIP_Addr.dw = pSocket->ClientIP.dw;
	pTX->Packet.u.IP_Frame.Header.SrcIP_Addr.dw = IP_HostAddr.dw;

    pTCP_TX 						= &pTX->Packet.u.TCP_Frame.Header;
	pTCP_TX->wSrcPort				= pSocket->pPortInfo->wNumber;
	pTCP_TX->wDstPort				= pSocket->wClientPort;
	pTCP_TX->SequenceNumber.dw 		= htonl(pSocket->SequenceNumber.dw + (int32_t)pSocket->Receive.wNext);
	pTCP_TX->AcknowledgeNumber.dw	= htonl(pSocket->AckNumber.dw + (int32_t)pSocket->Send.wNext);
	pTCP_TX->byFlags 			   |= TCP_FLAG_ACK;
	pTCP_TX->byOffset				= 0x60;								// to do process this criss
	pTCP_TX->wWindow				= htons(TCP_WINDOW_SIZE);
	pTCP_TX->OptionData.by.by0		= 2;
	pTCP_TX->OptionData.by.by1		= 4;
	pTCP_TX->OptionData.by.by2		= 4;
	pTCP_TX->OptionData.by.by3		= 0xB0;

	// Setup pseudo header for checksum calculation
	pPseudo_TX                      = &pTX->Packet.u.TCP_PseudoFrame.Header;
	pPseudo_TX->byProtocol 			= IP_PROTOCOL_TCP;
	pPseudo_TX->wLenght 			= htons(wPacketSize);

	pTCP_TX->wChecksum 				= 0;
	pTCP_TX->wChecksum 				= IP_CalculateChecksum(pPseudo_TX, wPacketSize + (int16_t)sizeof(IP_PseudoHeader_t));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TCP_Send
// 
//  Parameter(s):   void
//  Return:         IP_PacketMsg_t* pTX
//
//  Description:    
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* TCP_Send(SocketInfo_t* pSocket, uint8_t* pBuffer, int16_t wSize)
{
	IP_PacketMsg_t* 	pTX			= nullptr;
	IP_TCP_Header_t*	pTCP_TX;
	uint8_t 				Error;

	uint8_t Temp[100];
	uint8_t Buffer[1000];

	LIB_sprintf(Temp, "HTTP/1.1 404 Not Found\r\n");
	LIB_strcat(Buffer, Temp);
	LIB_sprintf(Temp, "Content-type: text/html\r\n");
	LIB_strcat(Buffer, Temp);
	LIB_sprintf(Temp, "Content-length: 114\r\n");
	LIB_strcat(Buffer, Temp);
	LIB_sprintf(Temp, "\r\n");
	LIB_strcat(Buffer, Temp);
	LIB_sprintf(Temp, "<html><head><title>Not Found</title></head><body>\r\n");
	LIB_strcat(Buffer, Temp);
	LIB_sprintf(Temp, "Sorry, the object you requested was not found.\r\n");
	LIB_strcat(Buffer, Temp);
	LIB_sprintf(Temp, "</body><html>\r\n");
	LIB_strcat(Buffer, Temp);


	pTX = IP_GetPacketMemory((int16_t)(wSize + sizeof(IP_EthernetHeader_t) + TCP_ACK_IP_PACKET_SIZE), &Error);
	if(pTX != nullptr)
	{
		wSize = LIB_strlen(Buffer);
		LIB_memcpy(((uint8_t*)&pTX->Packet.u.TCP_Frame.Header.OptionData + 4), Buffer, wSize);
        
		pTX->wPacketSize = (int16_t)(wSize + sizeof(IP_EthernetHeader_t) + TCP_ACK_IP_PACKET_SIZE);
		pTCP_TX          = &pTX->Packet.u.TCP_Frame.Header;
		pTCP_TX->byFlags = TCP_FLAG_PSH;
		TCP_PutHeader(pSocket, pTX, TCP_ACK_PACKET_SIZE + wSize);

		// Setup MAC & IP header
		LIB_memcpy(&pTX->Packet.u.ETH_Header.Dst.baAddr, &pSocket->MAC[0], 6);
		pTX->Packet.u.IP_Frame.Header.wLenght	 = htons(wSize + TCP_ACK_IP_PACKET_SIZE);
		pTX->Packet.u.IP_Frame.Header.byProtocol = IP_PROTOCOL_TCP;
		IP_PutHeader(pTX);
	}

	return(pTX);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TCP_Ack
// 
//  Parameter(s):   SocketInfo_t* 		pSocket
// 					uint8_t				byFlag
// 					int16_t 				wSize
//  Return:         IP_PacketMsg_t* 	pTX
//
//  Description:    
//	
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* TCP_Ack(SocketInfo_t* pSocket, uint8_t Flag, int16_t wSize)
{
	IP_PacketMsg_t* 	pTX			= nullptr;
	IP_TCP_Header_t*	pTCP_TX;
	uint8_t 				Error;

	pTX = IP_GetPacketMemory((int16_t)(wSize + sizeof(IP_EthernetHeader_t) + TCP_ACK_IP_PACKET_SIZE), &Error);
	if(pTX != nullptr)
	{
		pTX->wPacketSize = (int16_t)(wSize + sizeof(IP_EthernetHeader_t) + TCP_ACK_IP_PACKET_SIZE);
		pTCP_TX          = &pTX->Packet.u.TCP_Frame.Header;
		pTCP_TX->byFlags = Flag;
		TCP_PutHeader(pSocket, pTX, TCP_ACK_PACKET_SIZE);

		// Setup MAC & IP header
		LIB_memcpy(&pTX->Packet.u.ETH_Header.Dst.baAddr, &pSocket->MAC[0], 6);
		pTX->Packet.u.IP_Frame.Header.wLenght	 = htons(wSize + TCP_ACK_IP_PACKET_SIZE);
		pTX->Packet.u.IP_Frame.Header.byProtocol = IP_PROTOCOL_TCP;
		IP_PutHeader(pTX);
	}

	return(pTX);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TCP_Push
// 
//  Parameter(s):   SocketInfo_t* 		pSocket
// 					IP_PacketMsg_t*		pRX
// 					
//  Return:         void
//
//  Description:    
//	
//  Note(s):
//	      
//-------------------------------------------------------------------------------------------------
void TCP_Push(SocketInfo_t* pSocket, IP_PacketMsg_t* pRX)
{
	uint8_t 				byDataOffset;
	int16_t 				wSize;
	uint8_t 				Error;

	byDataOffset     = (uint8_t)((pRX->Packet.u.TCP_Frame.Header.byOffset & 0xF0) >> 2) + (uint8_t)sizeof(IP_IP_Frame_t);
	wSize            = (int16_t)(pRX->wPacketSize - DataOffset);




	// Pass pointer of data to service function to get data
	if(pSocket->pPortInfo->pFunction != nullptr)  // temporary
	{
		pSocket->pPortInfo->pFunction(pSocket, &pRX->Packet.u.baRawData[byDataOffset], wSize); 
	}

	pSocket->byConnectionState  = TCP_SOCKET_LISTENING;
	pSocket->Send.wNext        += wSize;
	pSocket->Receive.wNext      = (int16_t)(ntohl(pRX->Packet.u.TCP_Frame.Header.AcknowledgeNumber.dw) - pSocket->SequenceNumber.dw);
}

//-------------------------------------------------------------------------------------------------




















/*
								DBG_UartPrintf("Socket Src:%d.%d.%d.%d:%d on port %d listening state\n", pSocket->ClientIP.by.by0,
																										 pSocket->ClientIP.by.by1,
																										 pSocket->ClientIP.by.by2,
																										 pSocket->ClientIP.by.by3,
																										 ntohs(pSocket->wClientPort),
																										 ntohs(pPort->wNumber));

		DBG_UartPrintf("Socket Src:%d.%d.%d.%d:%d on port %d Negotiating state\n", pSocket->ClientIP.by.by0,
																				   pSocket->ClientIP.by.by1,
																				   pSocket->ClientIP.by.by2,
																				   pSocket->ClientIP.by.by3,
																				   ntohs(pSocket->wClientPort),
																				   ntohs(pPort->wNumber));

//					DBG_UartPrintf("Socket Src:%d.%d.%d.%d:%d on port %d Receiving request state\n", pSocket->ClientIP.by.by0,
//																											 pSocket->ClientIP.by.by1,
//																											 pSocket->ClientIP.by.by2,
//																											 pSocket->ClientIP.by.by3,
//																											 ntohs(pSocket->wClientPort),
//																											 ntohs(pPort->wNumber));
*/
