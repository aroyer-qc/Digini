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
void NetICMP::Initialize(NetworkContext& pContext)
{
    m_pContext = pContext;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:          ICMP_Process
//
//  Parameter(s):  IP_PacketMsg_t*      pRX             RX packet
//  Return:        None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void NetICMP::Process(IP_PacketMsg_t* pRX)
{
    uint16_t Count;


    //  Check if IP is Vaild            and  Validate size
    if((m_pContext->IsIP_Valid() == false) || (pRX->PacketSize < sizeof(IP_ICMP_Frame_t)))
    {
        pMemoryPool->Free((void**)&pRX->pPacket);
        pMemoryPool->Free((void**)&pRX);
        return;
    }

    IP_ICMP_Frame_t* pICMP = &pRX->pPacket->ICMP_Frame;

    switch (pICMP->Header.Type)
    {
        case ICMP_TYPE_PING_REQUEST:
        {
            // Allocate TX message wrapper
            IP_PacketMsg_t* pTX = (IP_PacketMsg_t*)pMemoryPool->AllocAndClear(pRX->PacketSize, MEM_DBG_ICMP);

            if(pTX == nullptr)
            {
                IP_Manager::FreeMessage(pRX);                                                                       // Cannot reply → just drop RX safely
                return;
            }

            // Allocate TX packet buffer
            pTX->pPacket = (IP_EthernetPacket_t*)pMemoryPool->AllocAndClear(pRX->PacketSize, MEM_DBG_ICMPDT);

            if(pTX->pPacket == nullptr)
            {
                // Free wrapper and RX, no reply possible
                pMemoryPool->Free((void**)&pTX);
                IP_Manager::FreeMessage(pRX);
                return;
            }

            // Build reply
            IP_CopyPacketMessage(pTX, pRX);                                                                         // Copy RX → TX
            pTX->pPacket->ICMP_Frame.Header.Type = ICMP_TYPE_PING_REPLY;                                            // Modify ICMP header
            Count  = htons(pTX->pPacket->ICMP_Frame.IP_Header.Length);
            Count -= sizeof(IP_IP_Header_t);
            pTX->pPacket->ICMP_Frame.Header.Checksum = IP_CalculateChecksum(&pTX->pPacket->ICMP_Frame.Header, Count);
            memcpy(pTX->pPacket->ETH_Header.Dst.Byte, pTX->pPacket->ETH_Header.Src.Byte, IP_MAC_ADDRESS_SIZE);      // Swap MACs
            // Fix IP header
            pTX->pPacket->ICMP_Frame.IP_Header.TimeToLive = IP_TIME_TO_LIVE;
            pTX->pPacket->ICMP_Frame.IP_Header.DstIP_Addr = pTX->pPacket->ICMP_Frame.IP_Header.SrcIP_Addr;
            pTX->pPacket->ICMP_Frame.IP_Header.SrcIP_Addr = IP_HostAddress;
            IP_PutHeader(pTX);
            m_pContext->SendPacket(pTX);                                                                              // Send reply internally
            // Free TX (SendPacket will free after TX IRQ)
        }
        break;

        default:
            break;
    }

    IP_Manager::FreeMessage(pRX);                                                                                   // Always free RX
}

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_ICMP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
