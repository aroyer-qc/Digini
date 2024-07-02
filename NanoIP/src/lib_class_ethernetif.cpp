//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ethernetif.cpp
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

#define ETHERNET_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  ETHERNET_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#define ARP_TMR_INTERVAL                1000
#define netifGUARD_BLOCK_TIME           250               // todo rename
#define BLOCK_TIME_WAITING_FOR_INPUT    0xffff


//-------------------------------------------------------------------------------------------------

void FreePacket(MemoryNode* pPacket)
{
    pPacket->Free();
    pMemoryPool->Free((void**)&pPacket);
}


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
//  Parameter(s):   EthernetIF_t*  The NanoIP network interface structure
//  Return:         SystemState_e
//
//  Description:    This function initialize the MAC,PHY and IO
//
//  Note(s):        Should be called at the beginning of the program to set up the network
//                  interface.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_IF_Driver::Initialize(IP_ETH_Config_t* pETH_Config)
{
    static nOS_Error Error;

    m_pETH_Config = pETH_Config;

	m_Link = ETH_LINK_DOWN;

    //BSP_EthernetIF_Initialize();


// TODO move this into the lib_ethernetif  it will take care of the init phase
//    myETH_Driver.Initialize(nullptr);
    //myETH_Driver.SetMacAddress(&MAC);
    //myETH_Driver.Start();
    //myPHY_Driver.Initialize(&myETH_Driver, 0);








	//pNetIf->output     = EtharpOutput;        not here!!!
	//pNetIf->linkoutput = LowLevelOutput;

    // Create binary semaphore used for informing ethernetif of frame reception
    Error = nOS_SemCreate(&m_RX_Sem, 0, 20);
    Error = nOS_MutexCreate(&m_TX_Mutex, NOS_MUTEX_NORMAL, 1);
    VAR_UNUSED(Error);

     nOS_ThreadCreate(&m_TaskHandle,
                      ClassEthernetIf_Wrapper,
                      this,
                      &m_Stack[0],
                      TASK_ETHERNET_IF_STACK_SIZE,
                      TASK_ETHERNET_IF_PRIO);

      #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
        myStacktistic.Register(&m_Stack[0], TASK_ETHERNET_IF_STACK_SIZE, "Ethernet Input");
      #endif

    m_pETH_Config->pETH_Driver->Start();        // Enable MAC and DMA transmission and reception

// 	sys_timeout(ARP_TMR_INTERVAL, ArpTimer, nullptr);  impelement my version of this

	return SYS_READY;//ERR_OK;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       lowlevelOutput
//
//  Parameter(s):   MemoryNode*         pPacket   IP packet to send (Including MAC addresses and type)
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
SystemState_e ETH_IF_Driver::LowLevelOutput(MemoryNode* pPacket)
{
    uint8_t* pNodeData;
    size_t   NodeSize;
    size_t   Length;

    if(nOS_MutexLock(&m_TX_Mutex, netifGUARD_BLOCK_TIME) == NOS_OK)
    {
        pPacket->Begin();       // Reset Node pointer to the beginning
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
            m_pETH_Config->pETH_Driver->SendFrame(pNodeData, NodeSize, flags);     //  standard call from an interface class ...  call this function  SendFrame
        }
        while((pPacket->GetNext() != nullptr) && (Length > 0));

        FreePacket(pPacket);


      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        m_DBG_TX_Count++;
      #endif

// need to flush the MemoryNode

        nOS_MutexUnlock(&m_TX_Mutex);
    }
    else
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: low_level_output: Sem TimeOut\n");
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       LowLevelInput
//
//  Parameter(s):   None
//  Return:         Pointer on MemoryNode or 'nullptr' on memory error
//
//  Description:    This function return a node list buffer filled with the received packet.
//                  (including MAC header)
//
//  Note(s)         This is inline, break into 2 part to simplify reading
//
//-------------------------------------------------------------------------------------------------
inline MemoryNode* ETH_IF_Driver::LowLevelInput(void)
{
    MemoryNode* pPacket = nullptr;
    size_t      Length;

    Length = m_pETH_Config->pETH_Driver->GetRX_FrameSize();                                             // Obtain the size of the packet and put it into the "len" variable.

    if(Length != 0)
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        m_DBG_RX_Count++;
      #endif

        if(Length <= IP_ETHERNET_FRAME_SIZE)
        {
            pPacket = (MemoryNode*)pMemoryPool->AllocAndClear(sizeof(MemoryNode));

            if(pPacket->Alloc(Length) == SYS_READY)                                 // We allocate a MemoryNode of node from the pool.
            {
                Length = 0;                                                         // We cannot allocated memory so this will force the packet to be dropped
                FreePacket(pPacket);
              #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
                m_DBG_RX_Drop++;
              #endif
            }
        }
        else
        {
            Length = 0;
          #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
            m_DBG_RX_Drop++;
          #endif
        }

         m_pETH_Config->pETH_Driver->ReadFrame(pPacket, Length);                                         // Read or drop the packet
    }

    return pPacket;

}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Run
//
//  Parameter(s):   pParam       The network interface structure for this ETH_IF_Driver
//  Return:         None
//
//  Description:    This function is the ethernetif_input task
//
//  Note(s):        It is processed when a packet is ready to be read from the interface. That
//                  should handle the actual reception of bytes from the network interface. Then
//                  the type of the received packet is determined and the appropriate input
//                  function is called.
//
//-------------------------------------------------------------------------------------------------
void ETH_IF_Driver::Run(void)
{
   	MemoryNode*   pPacket;
//    EthernetIF_t* pNetIf = (EthernetIF_t*)pParam;  // this will be member since the IP_Manager is the interface obtaining the netif equvalent of lwip
    SystemState_e State;
    bool          Exit;

    PollTheNetworkInterface();        // Initial polling of the link

	while(1)
	{
        if(nOS_SemTake(&m_RX_Sem, BLOCK_TIME_WAITING_FOR_INPUT) == NOS_OK)
		{
		    // if for some reason we receive a message and the link is down then poll the PHY for the link
		    if(m_Link == ETH_LINK_DOWN)
            {
                PollTheNetworkInterface();
            }

            Exit = false;

			do
			{
                if((pPacket = LowLevelInput()) != nullptr)
                {
                    //??if((State = m_pETH_Config->pETH_Driver->pNetIf->input(pPacket)) != SYS_READY)
                    {
                        VAR_UNUSED(State);
                        FreePacket(pPacket);

                      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
                        m_DBG_RX_Drop++;
                      #endif
                      Exit = true;
                    }
              //      else
                    {
            //            nOS_SemTake(&m_RX_Sem, 0); // why this one
                    }
                }

                if(pPacket->GetNext() == nullptr)
                {
                    Exit = true;
                }

                nOS_Yield();
			}
			while(Exit == false);
		}
		else
        {
            // If no message are received, pool the PHY for link periodically
            PollTheNetworkInterface();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ArpTimer
//
//  Parameter(s):   pArg                none required
//  Return:         None
//
//  Description:    Call lwIP function etharp_tmr each time the configure timeout has ended
//
//-------------------------------------------------------------------------------------------------
void ETH_IF_Driver::ArpTimer(void* pArg)
{
	VAR_UNUSED(pArg);
//	etharp_tmr();
   // sys_timeout(ARP_TMR_INTERVAL, ArpTimer, nullptr);           // Restart a new timer
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ethernetif_PollThePHY
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

        if(LinkNow != m_Link)
        {
            if(LinkNow == ETH_LINK_UP)
            {
                //netif_set_link_up(netif_find(IF_NAME));
                DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Link UP\n");
            }
            else
            {
                //netif_set_link_down(netif_find(IF_NAME));
                DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Link DOWN\n");
            }

            m_Link = LinkNow;
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
    if(Event == ETH_MAC_EVENT_RX_FRAME)         // Send notification on RX event
    {
        nOS_SemGive(&m_RX_Sem);               // Give the semaphore to wakeup LwIP task
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ethernetif_LinkCallBack
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
    m_Link = ETH_Phy.GetLinkState();
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)


















// need to put this in BSP or BSP supported section



void BSP_EthernetIF_Initialize(void)
{
  #if (IP_INTERFACE_SUPPORT_PHY == DEF_ENABLED) || (IP_INTERFACE_SUPPORT_MAC == DEF_ENABLED)
    //PHY CONFIG should be out of here...
    ETH_Mac.Initialize(ethernetif_Callback);							// Init IO, PUT ETH in RMII, Clear control structure

    // if(m_IF_Type == ETH_PHY_IF)
        // if(m_IF_Type == ETH_MAC_IF)
    // {
	// Initialize Physical Media Interface
	if(ETH_Phy.Initialize(&ETH_Mac) == SYS_READY)
	{
		ETH_Phy.PowerControl(ETH_POWER_FULL);                   // configuration into the driver itself????  with config ??
		ETH_Phy.SetInterface(ETH_USED_INTERFACE);
		ETH_Phy.SetMode(ETH_PHY_MODE_AUTO_NEGOTIATE);

      #if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
        IO_PinInit(IO_ETH_PHY_LINK_IO);
        IO_InitIRQ(ETH_PHY_LINK_IO_ISR, ethernetif_LinkCallcack);
        ETH_Phy.SetLinkUpInterrupt();
        IO_EnableIRQ(ETH_PHY_LINK_IO_ISR);
	  #endif
	}
  // }
  #endif

  #if (IP_INTERFACE_SUPPORT_HEC == DEF_ENABLED)
    // if(m_IF_Type == ETH_HEC_IF)
    // {
            // This is for chip containing the everything  in them Example Microchip WS5100
    // }
  #endif

    // if(m_IF_Type == ETH_MAC_IF)
    // {
            // This is for chip containing the MAC controler in them Example Microchip LAN8650/1
    // }
}
