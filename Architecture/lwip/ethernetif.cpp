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

#include "lib_digini.h"
#include "ethernetif.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/TimeOuts.h"
#include "netif/etharp.h"
#include "lwip/tcpip.h"
#include <string.h>


// All the stuff to move elsewhere

// MAC address configuration.
#define MAC_ADDR0	                            (((char *)0x1FFF7A10)[0])
#define MAC_ADDR1	                            (((char *)0x1FFF7A10)[2])
#define MAC_ADDR2	                            (((char *)0x1FFF7A10)[4])
#define MAC_ADDR3	                            (((char *)0x1FFF7A10)[6])
#define MAC_ADDR4	                            (((char *)0x1FFF7A10)[8])
#define MAC_ADDR5	                            (((char *)0x1FFF7A10)[10])

#define netifMTU                                1500
#define netifGUARD_BLOCK_TIME			        250
#define ETHERNET_FRAME_SIZE                     1514
#define BLOCK_TIME_WAITING_FOR_INPUT            250 // mSec


#define TASK_ETHERNET_IF_STACK_SIZE             512
#define TASK_ETHERNET_IF_PRIO                   4

ETH_Driver          EthMac;
PHY_LAN8742A_Driver Phy(0);

// Define those to better describe your network interface.
#define IFNAME0 'e'
#define IFNAME1 'n'
//-----------------------------------------------------------------------------------------


/**
 * Struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 */
 // won't work like this!!
struct ethernetif
{
    ETH_Driver*             Mac;              // Registered MAC driver
//    DRIVER_PHY_Interface*   Phy;              // Registered PHY driver
    PHY_LAN8742A_Driver*    Phy;              // Registered PHY driver
    ETH_LinkState_e         Link;             // Ethernet Link State
    bool                    EventRX_Frame;    // Callback RX event generated
    bool                    PHY_Ok;           // PHY initialized successfully
};


extern "C" {


nOS_Sem RX_Sem;
nOS_Mutex TX_Mutex;
static struct ethernetif  ETH0;

static nOS_Thread   TaskHandle;
static nOS_Stack    Stack[TASK_ETHERNET_IF_STACK_SIZE];


// Forward declarations.
static err_t low_level_output(struct netif *netif, struct pbuf *p);
static void ethernetif_input(void *param);
static void arp_timer(void *arg);
static void ethernetif_Callback(uint32_t event);
//static err_t EthernetModeAndSpeed(void);

//-------------------------------------------------------------------------------------------------
//
//  Function:       ethernetif_init
//
//  Parameter(s):   netif   The lwip network interface structure for this ethernetif
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
err_t ethernetif_init(struct netif *netif)
{
	ETH_MAC_Capability_t cap;
	uint16_t RegValue;
    nOS_Error Error;

    // Do whatever else is needed to initialize interface.
	ETH0.Mac    = &EthMac;
	ETH0.Phy    = &Phy;
	ETH0.Link   = ETH_LINK_DOWN;
	ETH0.PHY_Ok = false;

    cap = ETH0.Mac->GetCapabilities();
    ETH0.EventRX_Frame = (cap.event_rx_frame) ? true : false;
    ETH0.Mac->Initialize(ethernetif_Callback);							// Init IO, PUT ETH in RMII, Clear control structure

    ETH0.Mac->PowerControl(ETH_POWER_FULL);								// Enable Clock, Reset ETH, Init MAC, Enable ETH IRQ
    ETH0.Mac->SetMacAddress((ETH_MAC_Address_t*)netif->hwaddr);
    ETH0.Mac->Control(ETH_MAC_CONTROL_TX, 0);
    ETH0.Mac->Control(ETH_MAC_CONTROL_RX, 0);

    // this should more generic.. to too close to the mac driver of the STM32
	ETH0.Mac->Control(ETH_MAC_CONFIGURE, ETH_MAC_CHECKSUM_OFFLOAD_RX |
			                             ETH_MAC_CHECKSUM_OFFLOAD_TX |
                                         ETH_MAC_DUPLEX_FULL         |
										 ETH_MAC_SPEED_100M          |
										 ETH_MAC_ADDRESS_BROADCAST);
    ETH0.Mac->Control(ETH_DMA_CONFIGURE, ETH_DMAOMR_REG        |
										 ETH_DMAOMR_OSF);           // Second Frame Operate
    ETH0.Mac->Control(ETH_DMA_CONFIGURE, ETH_DMABMR_REG        |
    		                             ETH_DMABMR_AAB        |    // Address Aligned Beats
										 ETH_DMABMR_EDE        |    // Enhanced Descriptor format enable
										 ETH_DMABMR_FB         |    // Fixed Burst
										 ETH_DMABMR_RTPR_2_1   |    // Arbitration Round Robin RxTx 2 1
										 ETH_DMABMR_RDP_32Beat |    // Rx DMA Burst Length 32 Beat
										 ETH_DMABMR_PBL_32Beat |    // Tx DMA Burst Length 32 Beat
										 ETH_DMABMR_USP);           // Enable use of separate PBL for Rx and Tx

	// Initialize Physical Media Interface
	if(ETH0.Phy->Initialize(ETH0.Mac) == SYS_READY)
	{
		ETH0.Phy->PowerControl(ETH_POWER_FULL);
		ETH0.Phy->SetInterface(cap.media_interface);
		ETH0.Phy->SetMode(ETH_PHY_MODE_AUTO_NEGOTIATE);
		//EthernetModeAndSpeed();

		ETH0.PHY_Ok = true;

      #if ETH_USE_LINK_IRQ // to rename with proper preprocessor
        ETH0.Phy->SetLinkUpInterrupt();
	  #endif
	}

	LWIP_ASSERT("netif != nullptr", (netif != nullptr));

  #if LWIP_NETIF_HOSTNAME
	// Initialize interface hostname
	netif->hostname = "lwip";
  #endif

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;

	netif->output     = etharp_output;
	netif->linkoutput = low_level_output;

	// Initialize the hardware -> low_level_init(netif)
    {
        // set netif MAC hardware address length
        netif->hwaddr_len = ETHARP_HWADDR_LEN;

        // set netif MAC hardware address
        netif->hwaddr[0] =  MAC_ADDR0;
        netif->hwaddr[1] =  MAC_ADDR1;
        netif->hwaddr[2] =  MAC_ADDR2;
        netif->hwaddr[3] =  MAC_ADDR3;
        netif->hwaddr[4] =  MAC_ADDR4;
        netif->hwaddr[5] =  MAC_ADDR5;

        // set netif maximum transfer unit
        netif->mtu = netifMTU;

        // Accept broadcast address and ARP traffic
        netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

        // create binary semaphore used for informing ethernetif of frame reception
        /*nOS_Error*/ nOS_SemCreate (&RX_Sem, 0, 20);
        /*nOS_Error*/ nOS_MutexCreate (&TX_Mutex, NOS_MUTEX_NORMAL, NOS_MUTEX_PRIO_INHERIT);

        Error = nOS_ThreadCreate(&TaskHandle,
                                 ethernetif_input,
                                 (void*)netif,
                                 &Stack[0],
                                 TASK_ETHERNET_IF_STACK_SIZE,
                                 TASK_ETHERNET_IF_PRIO);

        // Enable MAC and DMA transmission and reception
        ETH0.Mac->Control(ETH_MAC_CONTROL_TX, 1);
        ETH0.Mac->Control(ETH_MAC_CONTROL_RX, 1);
        ETH0.Mac->Control(ETH_MAC_FLUSH, ETH_MAC_FLUSH_TX);
    }

	sys_timeout(ARP_TMR_INTERVAL, arp_timer, nullptr);

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
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	(void)netif;
    struct pbuf *q;

    if(nOS_MutexLock(&TX_Mutex, netifGUARD_BLOCK_TIME))
    {
        for(q = p; q != nullptr; q = q->next)
        {
            // Send the data from the pbuf to the interface, one pbuf at a time. The size of the data in each pbuf is kept in the ->len variable.
               u32_t flags = (q->next) ? ETH_MAC_TX_FRAME_FRAGMENT : 0;
               ETH0.Mac->SendFrame(q->payload, q->len, flags);
        }

        nOS_MutexUnlock(&TX_Mutex);
    }
    else
    {
	   //DEBUG_Lwip("low_level_output: Sem TimeOut\n\r");
    }

    return ERR_OK;
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
static void ethernetif_input(void *param)
{
   	struct pbuf* p;
    struct netif* s_pxNetIf = (struct netif *)param;
    size_t Length;

	while(1)
	{
        if((nOS_SemTake(&RX_Sem, BLOCK_TIME_WAITING_FOR_INPUT) == NOS_OK) && (s_pxNetIf != nullptr))
		{
			do
			{
                Length = ETH0.Mac->GetRX_FrameSize();                        // Obtain the size of the packet and put it into the "len" variable.

                if(Length != 0)            // No packet available
                {
                    if(Length > ETHERNET_FRAME_SIZE)
                    {
                        ETH0.Mac->ReadFrame(nullptr, 0);                    // Drop oversized packet
                    }
                    else
                    {
                        p = pbuf_alloc(PBUF_RAW, Length, PBUF_POOL);        // We allocate a pbuf chain of pbufs from the pool.
                        ETH0.Mac->ReadFrame (p, Length);

                        if(s_pxNetIf->input(p, s_pxNetIf) != ERR_OK)
                        {
                            pbuf_free(p);
                            p = nullptr;
                        }
                        else
                        {
                            nOS_SemTake(&RX_Sem, 0);
                        }
                    }
                }

			}
			while(p != nullptr);
		}

        if(netif_find("en0"))
        {
            if(ETH0.Phy->GetLinkState() == ETH_LINK_UP)
            {
                //EthernetModeAndSpeed();
                netif_set_link_up( netif_find("en0") );
            }
            else
            {
                netif_set_link_down( netif_find("en0") );
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
static void arp_timer(void* pArg)
{
	VAR_UNUSED(pArg);
	etharp_tmr();
    sys_timeout(ARP_TMR_INTERVAL, arp_timer, nullptr);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
/*
static err_t EthernetModeAndSpeed(void)
{
	uint16_t       RegValue;
	ETH_PHY_Mode_e Mode    = ETH_PHY_MODE_NOT_DEFINED;
	uint32_t       TimeOut = 0;

	// Wait for auto negotiation completed
	do
	{
		ETH0.Mac->PHY_Read(ETH0.Phy->GetPHY_Address(), REG_BMSR, &RegValue);
	    TimeOut++;
	}
	while(((RegValue & BMSR_ANEG_COMPLETE) == 0) && (TimeOut < PHY_READ_TO));

	// Return ERROR in case of TimeOut
	if(TimeOut == PHY_READ_TO)
	{
	    return ERR_TimeOut;
	}


	ETH0.Mac->PHY_Read(ETH0.Phy->GetPHY_Address(), REG_PHYCR1, &RegValue);

	// Configure the MAC with the Duplex Mode fixed by the auto-negotiation process
	if((RegValue & PHYCR1_OM_FD) == PHYCR1_OM_FD)
	{
		// Set Ethernet duplex mode to Full-duplex following the auto-negotiation
		Mode = ETH_PHY_MODE_SPEED_100M;
	}

	// Configure the MAC with the speed fixed by the auto-negotiation process
	if((RegValue & PHYCR1_OM_100B) == PHYCR1_OM_100B)
	{
        // Set Ethernet speed to 100M following the auto-negotiation
		Mode = ETH_PHY_Mode_e(uint32_t(Mode) | uint32_t(ETH_PHY_FULL_DUPLEX));
	}

	ETH0.Phy->SetMode(Mode);

	return ERR_OK;
}
*/
//-------------------------------------------------------------------------------------------------
//
//  Function:
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
// ISR activated when we receive a new message on the ethernet
void ethernetif_Callback(uint32_t event)
{
    /* Send notification on RX event */
    if(event == ETH_MAC_EVENT_RX_FRAME)
    {
		// Give the semaphore to wakeup LwIP task
        nOS_SemGive(&RX_Sem);
    }
}

//-------------------------------------------------------------------------------------------------

}   // extern "C"
