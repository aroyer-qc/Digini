#pragma once


#include "lwip/netif.h"
#include "lwip/err.h"

//#define ETH_IRQ_PRIO				   configLIBRARY_KERNEL_INTERRUPT_PRIORITY


#ifdef __cplusplus
 extern "C" {
#endif
err_t    ethernetif_init(struct netif *netif);
err_t	 ethernetif_BSP_Config();

#ifdef __cplusplus
 }
#endif

#endif
