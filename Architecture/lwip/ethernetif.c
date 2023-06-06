#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "lwip/err.h"
#include "ethernetif.h"
#include "lwip/tcpip.h"

#include <string.h>

// MAC address configuration.
#define MAC_ADDR0	(((char *)0x1FFF7A10)[0])
#define MAC_ADDR1	(((char *)0x1FFF7A10)[2])
#define MAC_ADDR2	(((char *)0x1FFF7A10)[4])
#define MAC_ADDR3	(((char *)0x1FFF7A10)[6])
#define MAC_ADDR4	(((char *)0x1FFF7A10)[8])
#define MAC_ADDR5	(((char *)0x1FFF7A10)[10])

#define netifMTU                            1500
#define netifGUARD_BLOCK_TIME			    250
#define ETHERNET_FRAME_SIZE                 1514

// The time to block waiting for input.
#define BLOCK_TIME_WAITING_FOR_INPUT	(NOS_TICKS_WAIT_MAX)

// Define those to better describe your network interface.
#define IFNAME0 'e' 
#define IFNAME1 'n'

/**
 * Struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 */
struct ethernetif
{
    DRIVER_ETH_MAC*         mac;              // Registered MAC driver
    DRIVER_PHY_Interface*   phy;              // Registered PHY driver
    ARM_ETH_LINK_STATE      link;             // Ethernet Link State
    bool                    event_rx_frame;   // Callback RX event generated
    bool                    phy_ok;           // PHY initialized successfully
};

nOS_Sem RX_Sem;
nOS_Mutex TX_Mutex;
static struct ethernetif  Eth0;


#define TASK_ETHERNET_IF_STACK_SIZE              512
#define TASK_ETHERNET_IF_PRIO                    4

static nOS_Thread   TaskHandle;
static nOS_Stack    Stack[TASK_ETHERNET_IF_STACK_SIZE];


/* Forward declarations. */
static void Eth_Link_ITHandler(void);
static void ethernetif_input(void *param);
static void arp_timer(void *arg);
static void ethernetif_Callback(uint32_t event);
static err_t EthernetModeAndSpeed(void);

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It also do what was called low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
	ETH_MAC_Capabilities_t cap;
	uint16_t RegValue;

    // Do whatever else is needed to initialize interface.
	Eth0.phy    = &ARM_Driver_ETH_PHY_(ETH_PHY_NUM);
	Eth0.mac    = &ARM_Driver_ETH_MAC_(ETH_MAC_NUM);
	Eth0.link   = ARM_ETH_LINK_DOWN;
	Eth0.phy_ok = false;

    cap = Eth0.mac->GetCapabilities();
    Eth0.event_rx_frame = (cap.event_rx_frame) ? true : false;
    Eth0.mac->Initialize(ethernetif_Callback);							// Init IO, PUT ETH in RMII, Clear control structure

    Eth0.mac->PowerControl(ARM_POWER_FULL);								// Enable Clock, Reset ETH, Init MAC, Enable ETH IRQ
    Eth0.mac->SetMacAddress((ARM_ETH_MAC_ADDR *)netif->hwaddr);
    Eth0.mac->Control(ARM_ETH_MAC_CONTROL_TX, 0);
    Eth0.mac->Control(ARM_ETH_MAC_CONTROL_RX, 0);

	Eth0.mac->Control(ARM_ETH_MAC_CONFIGURE, ETH_MAC_CHECKSUM_OFFLOAD_RX |
			                                 ETH_MAC_CHECKSUM_OFFLOAD_TX |
											 ETH_MAC_DUPLEX_FULL         |
											 ETH_MAC_SPEED_100M          |
											 ETH_MAC_ADDRESS_BROADCAST);
    Eth0.mac->Control(ARM_ETH_DMA_CONFIGURE, ETH_DMAOMR_REG        |
											 ETH_DMAOMR_OSF);           // Second Frame Operate
    Eth0.mac->Control(ARM_ETH_DMA_CONFIGURE, ETH_DMABMR_REG        |
    		                                 ETH_DMABMR_AAB        |    // Address Aligned Beats
											 ETH_DMABMR_EDE        |    // Enhanced Descriptor format enable
											 ETH_DMABMR_FB         |    // Fixed Burst
											 ETH_DMABMR_RTPR_2_1   |    // Arbitration Round Robin RxTx 2 1
											 ETH_DMABMR_RDP_32Beat |    // Rx DMA Burst Length 32 Beat
											 ETH_DMABMR_PBL_32Beat |    // Tx DMA Burst Length 32 Beat
											 ETH_DMABMR_USP);           // Enable use of separate PBL for Rx and Tx

	// Initialize Physical Media Interface
	if(Eth0.phy->Initialize(Eth0.mac->PHY_Read, Eth0.mac->PHY_Write) == SYS_READY)
	{
		Eth0.phy->PowerControl(ETH_POWER_FULL);
		Eth0.phy->SetInterface (cap.media_interface);
		Eth0.phy->SetMode(ETH_PHY_AUTO_NEGOTIATE);
		EthernetModeAndSpeed();

		Eth0.phy_ok = true;

		Eth0.mac->PHY_Read(ETH_PHY_NUM, REG_IRQCS, &RegValue);
		RegValue |= IRQCS_LINK_UP_IE;
		Eth0.mac->PHY_Write(ETH_PHY_NUM, REG_IRQCS, RegValue);
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
                                 (void*)netif),
                                 &Stack[0],
                                 TASK_ETHERNET_IF_STACK_SIZE,
                                 TASK_ETHERNET_IF_PRIO);

        // Enable MAC and DMA transmission and reception
        Eth0.mac->Control(ARM_ETH_MAC_CONTROL_TX, 1);
        Eth0.mac->Control(ARM_ETH_MAC_CONTROL_RX, 1);
        Eth0.mac->Control(ARM_ETH_MAC_FLUSH, ARM_ETH_MAC_FLUSH_TX);
    }
    
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, nullptr);

	return ERR_OK;
}



/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	(void)netif;
    struct pbuf *q;

    if(nOS_MutexLock(&TX_Mutex, netifGUARD_BLOCK_TIME))
    {
        for(q = p; q != nullptr; q = q->next)
        {
            // Send the data from the pbuf to the interface, one pbuf at a time. The size of the data in each pbuf is kept in the ->len variable.
               u32_t flags = (q->next) ? ARM_ETH_MAC_TX_FRAME_FRAGMENT : 0;
               Eth0.mac->SendFrame(q->payload, q->len, flags);
        }

        nOS_MutexUnlock(&TX_Mutex);
    }
    else
    {
	   //DEBUG_Lwip("low_level_output: Sem Timeout\n\r");
    }

    return ERR_OK;
}

/**
 * This function is the ethernetif_input task, it is processed when a packet 
 * is ready to be read from the interface. Tthat should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
 
#define BLOCK_TIME_WAITING_FOR_INPUT            250 mSec
 
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
                Length = Eth0.mac->GetRxFrameSize();                        // Obtain the size of the packet and put it into the "len" variable.

                if(Length != 0)            // No packet available
                {
                    if(Length > ETHERNET_FRAME_SIZE)
                    {
                        Eth0.mac->ReadFrame(nullptr, 0);                    // Drop oversized packet
                    }
                    else
                    {
                        p = pbuf_alloc(PBUF_RAW, Length, PBUF_POOL);        // We allocate a pbuf chain of pbufs from the pool.
                        Eth0.mac->ReadFrame (p, Length);

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
            if(Eth0.phy->GetLinkState() == ARM_ETH_LINK_UP)
            {
                EthernetModeAndSpeed();
                netif_set_link_up( netif_find("en0") );
            }
            else
            {
                netif_set_link_down( netif_find("en0") );
            }
        }
    }
}

static void arp_timer(void* pArg)
{
	(VAR_UNUSED(arg);
	etharp_tmr();
    sys_timeout(ARP_TMR_INTERVAL, arp_timer, nullptr);
}

static err_t EthernetModeAndSpeed(void)
{
	uint16_t RegValue;
	uint32_t Mode = 0;
	uint32_t timeout = 0;

	// Wait for auto negotiation completed
	do
	{
		Eth0.mac->PHY_Read(ETH_PHY_NUM, REG_BMSR, &RegValue);
	    timeout++;
	}
	while(((RegValue & BMSR_ANEG_COMPLETE) == 0) && (timeout < PHY_READ_TO));

	// Return ERROR in case of timeout
	if(timeout == PHY_READ_TO)
	{
	    return ERR_TIMEOUT;
	}


	Eth0.mac->PHY_Read(ETH_PHY_NUM, REG_PHYCR1, &RegValue);

	// Configure the MAC with the Duplex Mode fixed by the auto-negotiation process
	if((RegValue & PHYCR1_OM_FD) == PHYCR1_OM_FD)
	{
		// Set Ethernet duplex mode to Full-duplex following the auto-negotiation
		Mode = ARM_ETH_PHY_SPEED_100M;
	}

	// Configure the MAC with the speed fixed by the auto-negotiation process
	if((RegValue & PHYCR1_OM_100B) == PHYCR1_OM_100B)
	{
        // Set Ethernet speed to 100M following the auto-negotiation
		Mode |= ARM_ETH_PHY_DUPLEX_FULL;
	}

	Eth0.phy->SetMode(Mode);

	return ERR_OK;
}

// ISR activated when we receive a new message on the ethernet
void ethernetif_Callback(uint32_t event)
{
    /* Send notification on RX event */
    if(event == ARM_ETH_MAC_EVENT_RX_FRAME)
    {
		// Give the semaphore to wakeup LwIP task
        nOS_SemGive(&RX_Sem);
    }
}


