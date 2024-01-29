//-------------------------------------------------------------------------------------------------
//
//  File : ethernetif.cpp
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
#include "lib_digini.h"
#undef  ETHERNET_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

void FreePacket(MemoryNode* pPacket);
{
    pPacket->Free(pPacket);
    pMemoryPool->Free(*pPacket);
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
SystemState_e ETH_IF_Driver::Initialize(struct EthernetIF_t* pNetIf)
{
    static nOS_Error Error;

    m_pNetIf = pNetIf;

	m_Link = ETH_LINK_DOWN;
  
  
  #if (IP_INTERFACE_SUPPORT_PHY == DEF_ENABLED) || (IP_INTERFACE_SUPPORT_MAC == DEF_ENABLED)
    //PHY CONFIG should be out of here...
    ETH_Mac.Initialize(ethernetif_Callback);							// Init IO, PUT ETH in RMII, Clear control structure
 
    // if(m_IF_Type == ETH_PHY_IF) 
        // if(m_IF_Type == ETH_MAC_IF)
    // {
	// Initialize Physical Media Interface
	if(ETH_Phy.Initialize(&ETH_Mac) == SYS_READY)
	{
		ETH_Phy.PowerControl(ETH_POWER_FULL);
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

	// ASSERT("pNetIf != nullptr", (pNetIf != nullptr));

	//pNetIf->output     = EtharpOutput;        not here!!!
	//pNetIf->linkoutput = LowLevelOutput;

    // Set netif MAC hardware address length
    pNetIf->hwaddr_len = ETH_HWADDR_LEN;     //  not here!!!

    
    // YES HERE
    ETH_Mac.SetMacAddress((ETH_MAC_Address_t*)pNetIf->hwaddr);

    // Set netif maximum transfer unit
    // NOT HERE
    pNetIf->mtu = netifMTU;

  #if IP_USE_ARP
    pNetIf->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  #else
    pNetIf->flags = NETIF_FLAG_BROADCAST;
  #endif // IP_USE_ARP

    // Create binary semaphore used for informing ethernetif of frame reception
    Error = nOS_SemCreate(&m_RX_Sem, 0, 20);
    Error = nOS_MutexCreate(&m_TX_Mutex, NOS_MUTEX_NORMAL, 1);
    VAR_UNUSED(Error);

     nOS_ThreadCreate(&TaskHandle,
                      Input,
                      (void*)pNetIf,
                      &m_Stack[0],
                      TASK_ETHERNET_IF_STACK_SIZE,
                      TASK_ETHERNET_IF_PRIO);


      #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
        myStacktistic.Register(&Stack[0], TASK_ETHERNET_IF_STACK_SIZE, "Ethernet Input");
      #endif

    ETH_Mac.Start();        // Enable MAC and DMA transmission and reception

	sys_timeout(ARP_TMR_INTERVAL, ArpTimer, nullptr);

	return ERR_OK;
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
        pPacket->Begin();       // Reset Node pointer to the begining
        Length   = pPacket->GetTotalSize();
        NodeSize = pPacket->GetNodeSize();
        
        do
        {
            if(Lenght < NodeSize)
            {
                NodeSize = Length;
            }
 
            Lenght   -= NodeSize;
            pNodeData = static_cast<uint8_t*>(pPacket->GetNext());
            
            // Send the data from the pPacket to the interface, one pNodeData at a time.
            uint32_t flags = (pNodeData != nullptr) ? ETH_MAC_TX_FRAME_FRAGMENT : 0;
            ETH_Mac.SendFrame(pNodeData, NodeSize, flags);     //  standard call from an interface class ...  call this function  SendFrame

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


/*
how to use the Nodelist

Create a chainlist pointer
Create an object for that nodelist with the size of one element.
use add to insert more element
Free the nodelist object when done

struct testNode
{
    uint8_t Poutine;
    uint32_t Poutine2;
    uint16_t Poutine3;
    uint8_t Poutine4;
};

NodeList myTestChainList(sizeof(testNode));

void* pDummy;

myTestChainList.AddNode(1, &pDummy);
myTestChainList.AddNode(2, &pDummy);
myTestChainList.AddNode(3, &pDummy);
myTestChainList.AddNode(4, &pDummy);
myTestChainList.AddNode(5, &pDummy);
myTestChainList.AddNode(6, &pDummy);
myTestChainList.AddNode(7, &pDummy);
myTestChainList.AddNode(8, &pDummy);
myTestChainList.AddNode(9, &pDummy);

static uint16_t number = myTestChainList.GetNumberOfNode();

myTestChainList.GetNodeDataPointer(5, &pDummy);
myTestChainList.GetNodeDataPointer(1, &pDummy);
myTestChainList.GetNodeDataPointer(4, &pDummy);
myTestChainList.GetNodeDataPointer(2, &pDummy);
myTestChainList.GetNodeDataPointer(3, &pDummy);
myTestChainList.GetNodeDataPointer(6, &pDummy);
myTestChainList.GetNodeDataPointer(7, &pDummy);
myTestChainList.GetNodeDataPointer(9, &pDummy);
myTestChainList.GetNodeDataPointer(8, &pDummy);

myTestChainList.RemoveNode(5);
myTestChainList.RemoveNode(7);
myTestChainList.RemoveNode(6);
myTestChainList.RemoveNode(3);
myTestChainList.RemoveNode(4);
myTestChainList.RemoveNode(8);
myTestChainList.RemoveNode(9);
myTestChainList.RemoveNode(1);
myTestChainList.RemoveNode(2);


*/

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
    size_t Length;

    Length = ETH_Mac.GetRX_FrameSize();                                             // Obtain the size of the packet and put it into the "len" variable.

    if(Length != 0)
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        m_DBG_RX_Count++;
      #endif

        if(Length <= ETHERNET_FRAME_SIZE)
        {
            MemoryNode* pPacket = pMemoryPool->AllocAndClear(sizeof(MemoryNode*));

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

        ETH_Mac.ReadFrame(pPacket, Length);                                         // Read or drop the packet
    }

    return pPacket;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Input
//
//  Parameter(s):   pParam       The network interface structure for this ethernetif
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
void ETH_IF_Driver::Input(void* pParam)
{
   	MemoryNode*   pPacket;
    EthernetIF_t* pNetIf = (EthernetIF_t*)pParam;  // this will be member since the IP_Manager is the interface obtaining the netif equvalent of lwip
    err_t         Error;
    bool          Exit;

    PollTheNetworkInterface();        // Initial polling of the link

	while(1)
	{
        if(nOS_SemTake(&ETH_RX_Sem, BLOCK_TIME_WAITING_FOR_INPUT) == NOS_OK)
		{
		    // if for some reason we receive a message and the link is down then poll the PHY for the link
		    if(ETH_Link == ETH_LINK_DOWN)
            {
                PollTheNetworkInterface();
            }

            Exit = false;

			do
			{
                if((pPacket = LowLevelInput()) != nullptr)
                {
                    if((Error = pNetIf->input(pPacket)) != ERR_OK)
                    {
                        VAR_UNUSED(Error);
                        FreePacket(pPacket);
                        
                      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
                        m_DBG_RX_Drop++;
                      #endif
                      Exit = true;
                    }
                    else
                    {
                        nOS_SemTake(&m_RX_Sem, 0); // why this one
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
static void ETH_IF_Driver::ArpTimer(void* pArg)
{
	VAR_UNUSED(pArg);
	etharp_tmr();
    sys_timeout(ARP_TMR_INTERVAL, ArpTimer, nullptr);           // Restart a new timer
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

    if(netif_find(IF_NAME))       // TODO temporary to debug reception of packet
    {
        LinkNow = ETH_Phy.GetLinkState();

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
//  Function:       ethernetif_Callback
//
//  Parameter(s):   Event           Event type received
//  Return:         None
//
//  Description:    ISR activated when we receive a new message on the ethernet
//
//-------------------------------------------------------------------------------------------------
void ETH_IF_Driver::_Callback(uint32_t Event)
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
