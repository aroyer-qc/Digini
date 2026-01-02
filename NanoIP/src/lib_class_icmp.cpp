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
void NetICMP::Initialize(void)
{
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

    if (!m_Context.IsIP_Valid())
    {
        pMemoryPool->Free((void**)&pRX->pPacket);
        pMemoryPool->Free((void**)&pRX);
        return;
    }

    // Validate size
    if (pRX->PacketSize < sizeof(IP_ICMP_Frame_t))
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

            if (pTX == nullptr)
            {
                // Cannot reply → just drop RX safely
                pMemoryPool->Free((void**)&pRX->pPacket);
                pMemoryPool->Free((void**)&pRX);
                return;
            }

            // Allocate TX packet buffer
            pTX->pPacket = (IP_EthernetPacket_t*)pMemoryPool->AllocAndClear(pRX->PacketSize, MEM_DBG_ICMPDT);

            if(pTX->pPacket == nullptr)
            {
                // Free wrapper and RX, no reply possible
                pMemoryPool->Free((void**)&pTX);
                pMemoryPool->Free((void**)&pRX->pPacket);
                pMemoryPool->Free((void**)&pRX);
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
            m_Context.SendPacket(pTX);                                                                              // Send reply internally
            // Free TX (SendPacket will free after TX IRQ)
        }
        break;

        default:
            break;
    }

    // Always free RX
    pMemoryPool->Free((void**)&pRX->pPacket);
    pMemoryPool->Free((void**)&pRX);
}

#if 0
void NetICMP::Process(IP_PacketMsg_t* pRX)
{
	IP_PacketMsg_t*  		pTX  		= nullptr;
	IP_ICMP_Frame_t* 		pICMP;
	IP_EthernetHeader_t* 	pETH;
	uint16_t                Count;

	if(m_Context.IsIP_Valid() == true)
	{
		if(pRX->PacketSize < sizeof(IP_ICMP_Frame_t))
		{
			return nullptr;
		}

		switch(pRX->pPacket->ICMP_Frame.Header.Type)
		{
			case ICMP_TYPE_PING_REQUEST:
            {
				// need to handle the error

				pTX          = (IP_PacketMsg_t*)pMemoryPool->AllocAndClear(pRX->PacketSize, MEM_DBG_ICMP);		    // Get memory for TX IP_PacketMsg_t


				pTX->pPacket = (IP_EthernetPacket_t*)pMemoryPool->AllocAndClear(pRX->PacketSize, MEM_DBG_ICMPDT);		// Get memory for TX pPacket



				pICMP = &pTX->pPacket->ICMP_Frame;
				pETH  = &pTX->pPacket->ETH_Header;
				IP_CopyPacketMessage(pTX, pRX);											                // Copy the entire IP payload From RX to TX buffer
				Count  = htons(pICMP->IP_Header.Length);
				Count -= (int16_t)sizeof(IP_IP_Header_t);
				pICMP->Header.Type     = ICMP_TYPE_PING_REPLY;
				pICMP->Header.Checksum = IP_CalculateChecksum(&pICMP->Header, Count);

				memcpy(pETH->Dst.Byte, pETH->Src.Byte, IP_MAC_ADDRESS_SIZE);	                        // Put Mac header
				pICMP->IP_Header.TimeToLive = IP_TIME_TO_LIVE;
				pICMP->IP_Header.DstIP_Addr = pICMP->IP_Header.SrcIP_Addr;
				pICMP->IP_Header.SrcIP_Addr = IP_HostAddress;
				IP_PutHeader(pTX);
            }
            break;

            default: break; // No support for other ICMP command
		}
	}
	return pTX;
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_ICMP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
