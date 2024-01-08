//-------------------------------------------------------------------------------------------------
//
//  File : ethernetif.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

#define ETHERNET_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  ETHERNET_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)
#if (DIGINI_USE_LWIP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

extern "C" {

//-------------------------------------------------------------------------------------------------
// Private variable(s)
//-------------------------------------------------------------------------------------------------

static nOS_Sem                     ETH_RX_Sem;
static nOS_Mutex                   ETH_TX_Mutex;

static nOS_Thread   TaskHandle;
static nOS_Stack    Stack[TASK_ETHERNET_IF_STACK_SIZE];

// Forward declarations.
static inline struct pbuf*  low_level_input         (void);
static err_t                low_level_output        (struct netif* pNetIf, struct pbuf* pPacket);
static void                 ArpTimer                (void* pArg);
static void                 ethernetif_Callback     (uint32_t Event);
static void                 ethernetif_PollThePHY   (void);
#if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
static void                 ethernetif_LinkCallBack (void* pArg);
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       ethernetif_init
//
//  Parameter(s):   netif*  The lwip network interface structure for this ethernetif
//  Return:         err_t   ERR_OK  If the loopif is initialized
//                          ERR_MEM If private data couldn't be allocated
//                          Any other err_t on error
//
//  Description:    This function initialize the MAC,PHY and IO
//
//  Note(s):        This function should be passed as a parameter to netif_add().
//                  Should be called at the beginning of the program to set up the network
//                  interface. It also do what was called low_level_init() to do the actual setup
//                  of the hardware.
//
//-------------------------------------------------------------------------------------------------
err_t ethernetif_init(struct netif* pNetIf)
{
    static nOS_Error Error;

	ETH_Link = ETH_LINK_DOWN;
    ETH_Mac.Initialize(ethernetif_Callback);							// Init IO, PUT ETH in RMII, Clear control structure

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

	LWIP_ASSERT("pNetIf != nullptr", (pNetIf != nullptr));

  #if LWIP_NETIF_HOSTNAME
	pNetIf->hostname = "lwip";                                       // Initialize interface host name
  #endif
	pNetIf->output = etharp_output;
	pNetIf->linkoutput = low_level_output;

  #if LWIP_IPV6
    pNetIf->output_ip6 = ethip6_output;
  #endif // LWIP_IPV6

    // Set netif MAC hardware address length
    pNetIf->hwaddr_len = ETH_HWADDR_LEN;

    // Set netif MAC hardware address
    pNetIf->hwaddr[0] =  MAC_ADDR0;
    pNetIf->hwaddr[1] =  MAC_ADDR1;
    pNetIf->hwaddr[2] =  MAC_ADDR2;
    pNetIf->hwaddr[3] =  MAC_ADDR3;
    pNetIf->hwaddr[4] =  MAC_ADDR4;
    pNetIf->hwaddr[5] =  MAC_ADDR5;
    ETH_Mac.SetMacAddress((ETH_MAC_Address_t*)pNetIf->hwaddr);

    // Set netif maximum transfer unit
    pNetIf->mtu = netifMTU;

  #if LWIP_ARP
    pNetIf->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  #else
    pNetIf->flags = NETIF_FLAG_BROADCAST;
  #endif // LWIP_ARP

    // Create binary semaphore used for informing ethernetif of frame reception
    Error = nOS_SemCreate(&ETH_RX_Sem, 0, 20);
    Error = nOS_MutexCreate(&ETH_TX_Mutex, NOS_MUTEX_NORMAL, 1);
    VAR_UNUSED(Error);

     nOS_ThreadCreate(&TaskHandle,
                      ethernetif_input,
                      (void*)pNetIf,
                      &Stack[0],
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
//  Function:       low_level_output
//
//  Parameter(s):   netif   The lwip network interface structure for this ethernetif
//                  p       The MAC packet to send (IP packet including MAC addresses and type)
//  Return:         err_t   ERR_OK if the packet could be sent an err_t value if the packet
//                          couldn't be sent
//
//  Description:    This function should do the actual transmission of the packet. The packet is
//                  contained in the pbuf that is passed to the function. This pbuf might be
//                  chained.
//
//  Note(s):        Returning ERR_MEM here if a DMA queue of your MAC is full can lead to strange
//                  results. You might consider waiting for space in the DMA queue to become
//                  available since the stack doesn't retry to send a packet dropped because of
//                  memory failure (except for the TCP timers).
//
//-------------------------------------------------------------------------------------------------
static err_t low_level_output(struct netif* pNetIf, struct pbuf* pPacket)
{
	VAR_UNUSED(pNetIf);

    if(nOS_MutexLock(&ETH_TX_Mutex, netifGUARD_BLOCK_TIME) == NOS_OK)
    {
        for(; pPacket != nullptr; pPacket = pPacket->next)
        {
            // Send the data from the pbuf to the interface, one pbuf at a time. The size of the data in each pbuf is kept in the ->len variable.
            uint32_t flags = (pPacket->next) ? ETH_MAC_TX_FRAME_FRAGMENT : 0;
            ETH_Mac.SendFrame((uint8_t*)pPacket->payload, pPacket->len, flags);

          #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
            DBG_TX_Count++;
          #endif
        }

        nOS_MutexUnlock(&ETH_TX_Mutex);
    }
    else
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: low_level_output: Sem TimeOut\n");
    }

    return ERR_OK;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       low_level_input
//
//  Parameter(s):   None
//  Return:         Pointer on buffer 'struct pBuf' or 'nullptr' on memory error
//
//  Description:    This function return a buffer filled with the received packet.
//                  (including MAC header)
//
//-------------------------------------------------------------------------------------------------
static inline struct pbuf* low_level_input(void)
{
   	struct pbuf* pPacket = nullptr;
    size_t       Length;

    Length = ETH_Mac.GetRX_FrameSize();                                             // Obtain the size of the packet and put it into the "len" variable.

    if(Length != 0)
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        DBG_RX_Count++;
      #endif

        if(Length <= ETHERNET_FRAME_SIZE)
        {
            if((pPacket = pbuf_alloc(PBUF_RAW, Length, PBUF_POOL)) == nullptr)      // We allocate a pbuf chain of pbufs from the pool.
            {
                Length = 0;                                                         // we cannot allocated memory so this will force the packet to be dropped
              #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
                DBG_RX_Drop++;
              #endif
            }
        }
        else
        {
            Length = 0;
          #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
            DBG_RX_Drop++;
          #endif
        }

        ETH_Mac.ReadFrame(pPacket, Length);                                         // Read or drop the packet
    }

    return pPacket;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ethernetif_input
//
//  Parameter(s):   netif   The lwip network interface structure for this ethernetif
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
void ethernetif_input(void* pParam)
{
   	struct pbuf * pPacket;
    struct netif* pNetIf = (struct netif *)pParam;
    err_t         Error;

    ethernetif_PollThePHY();        // Initial polling of the link

	while(1)
	{
        if((nOS_SemTake(&ETH_RX_Sem, BLOCK_TIME_WAITING_FOR_INPUT) == NOS_OK) && (pNetIf != nullptr))
		{
		    // if for some reason we receive a message and the link is down then poll the PHY for the link
		    if(ETH_Link == ETH_LINK_DOWN)
            {
                ethernetif_PollThePHY();
            }

			do
			{
	            LOCK_TCPIP_CORE();

                if((pPacket = low_level_input()) != nullptr)
                {
                    if((Error = pNetIf->input(pPacket, pNetIf)) != ERR_OK)
                    {
                        VAR_UNUSED(Error);
                        pbuf_free(pPacket);
                        pPacket = nullptr;
                      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
                        DBG_RX_Drop++;
                      #endif
                    }
                    else
                    {
                        nOS_SemTake(&ETH_RX_Sem, 0);
                    }
                }

                UNLOCK_TCPIP_CORE();
                nOS_Yield();
			}
			while(pPacket != nullptr);
		}
		else
        {
            // If no message are received, pool the PHY for link periodically
            ethernetif_PollThePHY();
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
static void ArpTimer(void* pArg)
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
void ethernetif_PollThePHY(void)
{
    ETH_LinkState_e ETH_LinkNow;

    if(netif_find(IF_NAME))       // TODO temporary to debug reception of packet
    {
        ETH_LinkNow = ETH_Phy.GetLinkState();

        if(ETH_LinkNow != ETH_Link)
        {
            if(ETH_LinkNow == ETH_LINK_UP)
            {
                netif_set_link_up(netif_find(IF_NAME));
                DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Link UP\n");
            }
            else
            {
                netif_set_link_down(netif_find(IF_NAME));
                DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Link DOWN\n");
            }

            ETH_Link = ETH_LinkNow;
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
void ethernetif_Callback(uint32_t Event)
{
    if(Event == ETH_MAC_EVENT_RX_FRAME)         // Send notification on RX event
    {
        nOS_SemGive(&ETH_RX_Sem);               // Give the semaphore to wakeup LwIP task
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
void ethernetif_LinkCallBack(void* pArg)
{
    VAR_UNUSED(pArg);
    ETH_Link = ETH_Phy.GetLinkState();
}
#endif

//-------------------------------------------------------------------------------------------------

} // extern "C"

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_LWIP == DEF_ENABLED)
#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
