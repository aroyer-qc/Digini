//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ethernet_if.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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
SystemState_e ETH_IF_Driver::Initialize(const IP_ETH_Config_t* pETH_Config, NetworkContext* pContext)
{
    nOS_Error            Error;
    SystemState_e        State;
    ETH_DriverInterface* pETH_Driver;
    IP_MAC_Address_t     MAC_Address;

    m_pContext    = pContext;
    m_pETH_Config = pETH_Config;
	m_pContext->SetLinkState(ETH_LINK_DOWN);
	m_pContext->SetLinkSpeed(ETH_PHY_SPEED_NONE);

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

    m_pContext->GetMAC_Address(&MAC_Address);
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
//  Parameter(s):   ppPacketMsg     Pointer to a fully prepared IP_PacketMsg_t* wrapper.
//                                   The wrapper contains a contiguous Ethernet frame buffer
//                                   ready for transmission.
//
//  Return:         SystemState_e   SYS_READY if the packet was accepted by the hardware driver.
//                                   Any other value indicates the packet could not be queued.
//
//  Description:    This function performs the final step of packet transmission. At this stage,
//                  the Ethernet frame is already fully constructed:
//
//                      - Destination MAC address resolved (ARP or broadcast)
//                      - Source MAC address filled
//                      - Ethernet Type set
//                      - IP header complete and checksummed
//                      - Transport header and payload in place
//
//                  No additional processing, ARP lookup, or header modification occurs here.
//                  The function simply forwards the packet wrapper to the hardware-specific
//                  Ethernet driver, which is responsible for queuing the frame for DMA
//                  transmission and freeing the wrapper after TX completion (typically in IRQ).
//
//  Note(s):        If the hardware TX queue is full, the driver may return an error state.
//                  The IP stack does not automatically retry dropped packets (except TCP
//                  retransmissions driven by timers), so the driver may optionally block or
//                  wait for queue space depending on system requirements.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_IF_Driver::LowLevelOutput(IP_PacketMsg_t** ppPacketMsg)
{
    return m_pETH_Config->pETH_Driver->SendTX_Packet(ppPacketMsg);
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
            if(m_pContext->GetLinkState() == ETH_LINK_DOWN)
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

                if(nOS_QueueWrite(m_pContext->GetMsgQ(), (void*)&pPacketMsg, NET_BLOCK_TIME_WAITING_FOR_Q) != NOS_OK)
                {
                    IP_Manager::FreeMessage(pPacketMsg);
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
    PHY_DriverInterface* pDriverIF = m_pETH_Config->pPHY_Driver;
    ETH_LinkState_e LinkNow;

    LinkNow = pDriverIF->GetLinkState();

    if(LinkNow != m_pContext->GetLinkState())
    {
        m_pContext->SetLinkChange(true);

        if(LinkNow == ETH_LINK_UP)
        {
            m_pContext->SetLinkSpeed(pDriverIF->GetLinkInfo().Speed);
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: Link UP\n");
        }
        else
        {
            m_pContext->SetLinkSpeed(ETH_PHY_SPEED_NONE);
            DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: Link DOWN\n");
        }

        m_pContext->SetLinkState(LinkNow);
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
    if((Event & ETH_MAC_EVENT_RX_FRAME) != 0) // Send notification on RX event
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
