//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_udp.cpp
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
// 	|  64   | Zero's   	| Protocol 11h |      UDP Length         |
//  |_______|___________|______________|_________________________| __________________________
// 	|  96   |        Source Port       |    Destination Port     |
//  |_______|__________________________|_________________________|       Real Header
// 	|  128  |          Length          |       Checksum          |
//  |_______|__________________________|_________________________|___________________________
// 	|       |                                                    |
// 	|  160  |                       Data                         |           Data
//  |_______|____________________________________________________|
//
//
//  The UDP Length field is the lenght of the Pseudo UDP header and Real Header + data
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (IP_USE_UDP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

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
//void NetUDP::Initialize(void)
//{
//}

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
void NetUDP::Process(IP_PacketMsg_t* pMsg)
{
    UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Header;
    uint16_t destPort = ntohs(pUDP->DestPort);

    Socket* pSock = FindUDPSocketByPort(destPort);   // Your lookup function

    if (pSock == nullptr)
    {
        // No socket bound to this port → drop
        pMemoryPool->Free((void**)&pMsg->pPacket);
        pMemoryPool->Free((void**)&pMsg);
        return;
    }

    UDP_Socket_t* pUdpSock = pSock->m_Proto.udp;
    // Enqueue packet for this socket
    if (nOS_QueueWrite(&pUdpSock->RxQueue, &pMsg, 0) != NOS_OK)
    {
        // Queue full → drop
        pMemoryPool->Free((void**)&pMsg->pPacket);
        pMemoryPool->Free((void**)&pMsg);
        return;
    }

    // Ownership now belongs to the socket. Do NOT free here.
}

SystemState_e NetUDP::Send(UDP_Socket_t* pUdp, uint8_t* pData, size_t Length, const SocketInfo_t* pDestInfo, size_t* pBytesSent)
{
    *pBytesSent = 0;

    // Allocate message wrapper
    IP_PacketMsg_t* pMsg = (IP_PacketMsg_t*)pMemoryPool->AllocAndClear(sizeof(IP_PacketMsg_t), MEM_DBG_UDP);
    if(pMsg == nullptr)
    {
        return SYS_NO_MEMORY;
    }

    // Allocate Ethernet/IP/UDP packet buffer
    size_t packetSize = sizeof(IP_EthernetPacket_t) + Length; // adjust to your actual layout
    pMsg->pPacket = (IP_EthernetPacket_t*)pMemoryPool->AllocAndClear(packetSize, MEM_DBG_UDPDT);

    if(pMsg->pPacket == nullptr)
    {
        pMemoryPool->Free((void**)&pMsg);
        return SYS_NO_MEMORY;
    }

    pMsg->PacketSize = packetSize;

    // Build headers: ETH + IP + UDP
    UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Frame.Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->UDP_Frame.IP_Header;
    IP_EthernetHeader_t* pETH = &pMsg->pPacket->ETH_Header;

    // Fill UDP header
    pUDP->SrcPort = htons(pUdp->LocalPort);
    pUDP->DstPort = htons(pDestInfo->Port);
    pUDP->Length  = htons(sizeof(UDP_Header_t) + Length);

    // Copy payload after UDP header
    uint8_t* pPayload = (uint8_t*)(pUDP + 1);
    memcpy(pPayload, pData, Length);

    // Fill IP header (src/dst, proto=UDP, length, etc.)
    IP_FillHeader_UDP(pIP, pUdp->LocalIP, pDestInfo->Address, sizeof(UDP_Header_t) + Length);

    // Fill ETH header (MAC src/dst, type=IP)
    ETH_FillHeader_IP(pETH, pDestInfo->MacAddress);

    // Calculate checksums
    pUDP->Checksum = UDP_CalculateChecksum(pIP, pUDP);
    IP_PutHeaderRaw(pMsg);   // If you have such a helper

    // Send via context (will queue to TX and free after TX complete)
    m_Context.SendPacket(pMsg);

    *pBytesSent = Length;
    return SYS_READY;
}
/*
IP_PacketMsg_t* NetUDP::Process(IP_PacketMsg_t* pRX)
{
	IP_PacketMsg_t* 		pTX = nullptr;
	PortInfo_t* 			pPort;

    if((pPort = SOCK_ValidPort(pRX->pPacket->UDP_Frame.Header.DstPort, IP_PROTOCOL_UDP)) != nullptr)	// Check first if the port is in our allowed port list
    {
        pPort->pFunction(nullptr, nullptr, 0);		// TO DO define UDP payload in frame so we can pass the address of the data easily or maybe copy data and post message
    }

	return pTX;

// reference
	// use this if other case need a valid IP
	//if(IP_Status.b.IP_IsValid == true)

}
*/
//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_UDP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
