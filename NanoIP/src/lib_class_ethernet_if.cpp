//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ethernet_if.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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
//
//  This is for NanoIP stack
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

//#define ETHERNET_DRIVER_GLOBAL
#include "./lib_digini.h"
//#undef  ETHERNET_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#define NET_ARP_TMR_INTERVAL                        1000
#define NET_GUARD_BLOCK_TIME                        250         // todo rename
#define NET_BLOCK_TIME_WAITING_FOR_INPUT            10          // 0xFFFF
#define NET_BLOCK_TIME_WAITING_FOR_Q                2
#define NET_RX_COUNT_MAX_SEMAPHORE                  2

//-------------------------------------------------------------------------------------------------
//
//   Static Variables
//
//-------------------------------------------------------------------------------------------------

nOS_Thread ETH_IF_Driver::m_Handle;
nOS_Stack  ETH_IF_Driver::m_Stack[TASK_ETHERNET_IF_STACK_SIZE];

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClassEthernetIf_Wrapper
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    main() for the ClassEthernetIf_Wrapper
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" void ClassEthernetIf_Wrapper(void* pvParameters)
{
    (static_cast<ETH_IF_Driver*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   IP_ETH_Config_t*  pETH_Config    The NanoIP network interface structure
//  Return:         SystemState_e
//
//  Description:    This function initialize the MAC,PHY and IO
//
//  Note(s):        Should be called at the beginning of the program to set up the network
//                  interface.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_IF_Driver::Initialize(const IP_ETH_Config_t* pETH_Config)
{
    nOS_Error            Error;
    SystemState_e        State;
    ETH_DriverInterface* pETH_Driver;
    IP_MAC_Address_t     MAC_Address;

    m_pETH_Config = pETH_Config;
	m_Context.SetLinkState(ETH_LINK_DOWN);

    Error = nOS_SemCreate(&m_RX_Sem, 0, NET_RX_COUNT_MAX_SEMAPHORE);
    Error = nOS_MutexCreate(&m_TX_Mutex, NOS_MUTEX_NORMAL, 1);
    VAR_UNUSED(Error);

  #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
    myStacktistic.Register(&m_Stack[0], TASK_ETHERNET_IF_STACK_SIZE, "Ethernet Input");
  #endif

    nOS_ThreadCreate(&m_Handle,
                     ClassEthernetIf_Wrapper,
                     this,
                     &m_Stack[0],
                     TASK_ETHERNET_IF_STACK_SIZE,
                     TASK_ETHERNET_IF_PRIO);

    pETH_Driver = m_pETH_Config->pETH_Driver;
    pETH_Driver->Initialize(this, m_pETH_Config->PHY_Address);      // TODO put in here the callback

    m_Context.GetMAC_Address(&MAC_Address);
    pETH_Driver->SetMacAddress(&MAC_Address);

    if((State = m_pETH_Config->pPHY_Driver->Initialize(pETH_Driver, m_pETH_Config->PHY_Address)) == SYS_READY)      // Interface ID is used as address
    {
        //pETH_Driver->InitializeInterface();
        pETH_Driver->Start();                                                               // Enable MAC and DMA transmission and reception
    }

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       LowLevelOutput
//
//  Parameter(s):
//  Return:         SystemState_e       SYS_READY if the packet could be sent
//                                             other value if the packet couldn't be sent
//
//  Description:    This function should do the actual transmission of the packet. The packet is
//                  contained in the pPacket that is passed to the function. This pPacket might be
//                  chained.
//
//  Note(s):        Returning ???ERR_MEM ( need error to define this) ??? here if a DMA queue of your MAC is full can lead to strange
//                  results. You might consider waiting for space in the DMA queue to become
//                  available since the stack doesn't retry to send a packet dropped because of
//                  memory failure (except for the TCP timers).
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_IF_Driver::LowLevelOutput(IP_PacketMsg_t** ppPacketMsg)
{


/*
    uint8_t* pNodeData;
    size_t   NodeSize;
    size_t   Length;

    if(nOS_MutexLock(&m_TX_Mutex, NET_GUARD_BLOCK_TIME) == NOS_OK)
    {
        pPacket->Begin();                               // Reset Node pointer to the beginning
        Length   = pPacket->GetTotalSize();
        NodeSize = pPacket->GetNodeSize();

        do
        {
            if(Length < NodeSize)
            {
                NodeSize = Length;
            }

            Length   -= NodeSize;
            pNodeData = static_cast<uint8_t*>(pPacket->GetNext());

            // Send the data from the pPacket to the interface, one pNodeData at a time.
            uint32_t flags = (pNodeData != nullptr) ? ETH_MAC_TX_FRAME_FRAGMENT : 0;
            m_pETH_Config->pETH_Driver->SendTX_Packet(pNodeData, NodeSize, flags);     //  standard call from an interface class ...  call this function  SendFrame
        }
        while((pPacket->GetNext() != nullptr) && (Length > 0));

      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        m_DBG_TX_Count++;
      #endif

        MemoryNode::FreeNode(&pPacket);
        nOS_MutexUnlock(&m_TX_Mutex);
    }
    else
    {
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: low_level_output: Sem TimeOut\n");
    }
*/
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Run
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    This function is the input task
//
//  Note(s):        It is processed when a packet is ready to be read from the interface. That
//                  should handle the actual reception of bytes from the network interface. Then
//                  the type of the received packet is determined and the appropriate input
//                  function is called.
//
//-------------------------------------------------------------------------------------------------
void ETH_IF_Driver::Run(void)
{
    IP_PacketMsg_t* pPacketMsg = nullptr;
    SystemState_e   State;
    int             Count;

    PollTheNetworkInterface();

    while(1)
    {
        if(nOS_SemTake(&m_RX_Sem, NET_BLOCK_TIME_WAITING_FOR_INPUT) == NOS_OK)
        {
            if(m_Context.GetLinkState() == ETH_LINK_DOWN)
            {
                PollTheNetworkInterface();
            }

            Count = 0;

            while(Count < 4)
            {
                State = m_pETH_Config->pETH_Driver->GetRX_Packet(&pPacketMsg);

                if((State != SYS_READY) || (pPacketMsg == nullptr))
                {
                    break;
                }

                if(nOS_QueueWrite(m_Context.GetMsgQ(), (void*)&pPacketMsg, NET_BLOCK_TIME_WAITING_FOR_Q) != NOS_OK)
                {
                    pMemoryPool->Free((void**)&pPacketMsg);
                  #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
                    DBG_RX_Drop++;
                  #endif
                }

                IO_SetPinLow(IO_ETH_EXT_LED);
                nOS_Yield();
                Count++;
            }
        }
        else
        {
            PollTheNetworkInterface();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       PollTheNetworkInterface
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Polling the PHY for link status, inform lwIP of it.
//
//-------------------------------------------------------------------------------------------------
void ETH_IF_Driver::PollTheNetworkInterface(void)
{
    ETH_LinkState_e LinkNow;

 //   if(netif_find(IF_NAME))       // TODO temporary to debug reception of packet
    {
        LinkNow = m_pETH_Config->pPHY_Driver->GetLinkState();

        if(LinkNow != m_Context.GetLinkState())
        {
            if(LinkNow == ETH_LINK_UP)
            {
                //netif_set_link_up(netif_find(IF_NAME));
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: Link UP\n");
            }
            else
            {
                //netif_set_link_down(netif_find(IF_NAME));
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: Link DOWN\n");
            }

            m_Context.SetLinkState(LinkNow);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CallBack
//
//  Parameter(s):   Event           Event type received
//  Return:         None
//
//  Description:    ISR activated when we receive a new message on the ethernet
//
//-------------------------------------------------------------------------------------------------
void ETH_IF_Driver::CallBack(uint32_t Event)
{
    if(Event == ETH_MAC_EVENT_RX_FRAME)       // Send notification on RX event
    {
        nOS_SemGive(&m_RX_Sem);               // Give the semaphore to wakeup IP_Manager task
    }

    // Handle only RX
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       LinkCallBack
//
//  Parameter(s):   pArg        N/U
//  Return:         None
//
//  Description:    This ISR is called when the link status from PHY detect change.
//
//  Note(s)         The function IO_CallBack(ETH_PHY_LINK_IO_ISR); should be include in ISR
//                  servicing the pin in the irq.cpp of the application
//-------------------------------------------------------------------------------------------------
#if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
void ETH_IF_Driver::LinkCallBack(void* pArg)
{
    VAR_UNUSED(pArg);
    m_Context->SetLinkState(ETH_Phy.GetLinkState());
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
