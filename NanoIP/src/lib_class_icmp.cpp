//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_icmp.c
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
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (IP_USE_ICMP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext& 	pContext		Reference on the context
//  Return:         None
//
//  Description:    Initialize the ICMP protocol handler
//
//-------------------------------------------------------------------------------------------------
void ICMP_Manager::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   IP_PacketMsg_t*   pRX
//                      Pointer to the received IPv4 packet wrapper containing the
//                      Ethernet, IP, and ICMP frames.
//
//  Return:         None
//
//  Description:    Handles incoming ICMP messages. This function currently implements
//                  processing for ICMP Echo Requests (Ping). When a valid Echo Request
//                  is received, the packet buffer is reused to construct an Echo Reply
//                  in zero‑copy fashion.
//
//                  Processing steps:
//                      1) Validate the IPv4 header and ensure the packet is large enough
//                         to contain a complete ICMP frame.
//                      2) Detect ICMP Echo Request messages.
//                      3) Reuse the RX buffer as TX (zero‑copy), update MAC addresses,
//                         rebuild the IPv4 header, and convert the ICMP type to Echo Reply.
//                      4) Recompute the ICMP checksum using LIB_Checksum16() over the
//                         ICMP header and payload.
//                      5) Transmit the packet. If transmission fails, the buffer is freed.
//
//                  Any packet that is not a valid Echo Request is discarded. This function
//                  ensures full RFC‑compliant ICMP Echo Reply generation while maintaining
//                  NanoIP’s zero‑copy design philosophy.
//
//-------------------------------------------------------------------------------------------------
void ICMP_Manager::Process(IP_PacketMsg_t* pRX)
{
    //  Check if IP is Valid              and Validate size
    if((m_pContext->IsIP_Valid() == true) && (pRX->PacketSize >= sizeof(ICMP_Frame_t)))
    {
        ICMP_Frame_t* pICMP = &pRX->pPacket->ICMP_Frame;

        switch(pICMP->Header.Type)
        {
            case ICMP_TYPE_PING_REQUEST:
            {
                IP_PacketMsg_t* pTX = pRX;                                                                                  // Reuse RX as TX
                IP_EthernetPacket_t* pPacket = pTX->pPacket;
                pMemoryPool->ChangeDebugID(pTX, MEM_DBG_IPPKT, MEM_DBG_ICMP);
                pMemoryPool->ChangeDebugID(pTX->pPacket, MEM_DBG_ETHDMARX2, MEM_DBG_ICMPDT);
                pPacket->ICMP_Frame.Header.Type = ICMP_TYPE_PING_REPLY;                                                     // Modify ICMP
                memcpy(pPacket->ETH_Header.DestinationMAC.Byte, pPacket->ETH_Header.SourceMAC.Byte, 6);                     // Set Destination MAC = original Source MAC
                uint16_t ICMP_Length = pTX->PacketSize - sizeof(IP_EthernetHeader_t) - sizeof(IP_Header_t);                 // Compute ICMP length (everything after the IP header)
                IP_Manager* pIP_Manager = m_pContext->GetIP_Manager();
                pPacket->ICMP_Frame.IP_Header.TimeToLive = IP_TIME_TO_LIVE;
                pPacket->ICMP_Frame.Header.Checksum = 0;
                pIP_Manager->PutHeader(pTX, pPacket->ICMP_Frame.IP_Header.SrcIP_Address, ICMP_Length, IP_PROTOCOL_ICMP);    // Reply source = original destination
                pPacket->ICMP_Frame.Header.Checksum = 0;
                pPacket->ICMP_Frame.Header.Checksum = LIB_HTONS_Checksum16((uint8_t*)&pPacket->ICMP_Frame.Header, ICMP_Length);
//pPacket->ICMP_Frame.Header.Checksum = IP_Manager::IP_CalculateChecksum(&pPacket->ICMP_Frame.Header, ICMP_Length);
                SystemState_e State = m_pContext->SendPacket(pTX);                                                          // // Zero-copy TX

                if(State != SYS_READY)
                {
                    IP_Manager::FreeMessage(pTX);

                  #if (IP_DBG_ARP == DEF_ENABLED)
                    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ICMP: Ping Reply - SendPacket Failed!, Drop the packet\n");
                  #endif
                }

                return;                                                                                                     // Important: do NOT fall through to FreeMessage(pRX)
            }

            default:
                break;
        }
    }

    IP_Manager::FreeMessage(pRX);                                                                                           // Always free RX
}

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_ICMP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
