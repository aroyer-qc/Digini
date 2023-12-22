//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_process_lwip.cpp
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
#define LWIP_APP_GLOBAL
#include "lib_digini.h"
#undef  LWIP_APP_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)
#if (DIGINI_USE_LWIP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

extern "C"
{
    // Variable(s)
    extern sys_mbox_t   tcpip_mbox;

    // Function(s)
    extern void         tcpip_thread_handle_msg(struct tcpip_msg* pMsg);

    extern void         tcpip_timeouts_mbox_fetch(sys_mbox_t *mbox, void **msg);
}

//-------------------------------------------------------------------------------------------------

nOS_Error LWIP_Application::Initialize(void)
{
    nOS_Error Error = NOS_OK;

    lwipPlatformDiag("Initializing ClassNetwork\n");

    // Initialize lwip
    lwip_init();

    if(sys_mbox_new(&tcpip_mbox, TCPIP_MBOX_SIZE) != ERR_OK)
    {
        LWIP_ASSERT("failed to create tcpip_thread mbox", 0);
    }

  #if LWIP_TCPIP_CORE_LOCKING
    if (sys_mutex_new(&lock_tcpip_core) != ERR_OK)
    {
        LWIP_ASSERT("failed to create lock_tcpip_core", 0);
    }
  #endif // LWIP_TCPIP_CORE_LOCKING

  // Initialize webserver demo
  //http_server_socket_init();

  #if LWIP_DHCP
    ip_addr_set_zero_ip4(&m_IP_Address);
    ip_addr_set_zero_ip4(&m_SubnetMask);
    ip_addr_set_zero_ip4(&m_GatewayIP);
  #else
    // Use IP from ethernet_cfg.h
    IP_ADDR4(&m_IP_Address, ETH_IP_ADDR0,          ETH_IP_ADDR1,          ETH_IP_ADDR2,          ETH_IP_ADDR3);
    IP_ADDR4(&m_SubnetMask, ETH_SUBNET_MASK_ADDR0, ETH_SUBNET_MASK_ADDR1, ETH_SUBNET_MASK_ADDR2, ETH_SUBNET_MASK_ADDR3);
    IP_ADDR4(&m_GatewayIP,  ETH_GATEWAY_IP_ADDR0,  ETH_GATEWAY_IP_ADDR1,  ETH_GATEWAY_IP_ADDR2,  ETH_GATEWAY_IP_ADDR3);
  #endif

    // Registers the default network interface
	m_NetIf.name[0] = IFNAME0;
	m_NetIf.name[1] = IFNAME1;

    netif_set_default(&m_NetIf);

    // Add the network interface
    netif_add(&m_NetIf, &m_IP_Address, &m_SubnetMask, &m_GatewayIP, nullptr, &ethernetif_init, &tcpip_input);  //?? for tcp_input  in example it is netif_input

    if(netif_is_link_up(&m_NetIf))  netif_set_up(&m_NetIf);          // When the netif is fully configured this function must be called
    else                            netif_set_down(&m_NetIf);        // When the netif link is down this function must be called


    //ethernet_link_status_updated(&m_NetIf);

  #if LWIP_NETIF_LINK_CALLBACK
    netif_set_link_callback(&m_NetIf, ethernet_set_link_updated);
  #endif

  #if LWIP_DHCP
    dhcp_start(&m_NetIf);
  #endif


    return Error;
}

//-------------------------------------------------------------------------------------------------

void LWIP_Application::Process(void)
{
    struct tcpip_msg*   pMsg = nullptr;
    uint32_t            Result;

    // Wait for a message with timers disabled
    tcpip_timeouts_mbox_fetch(&tcpip_mbox, (void **)&pMsg);

    //if(Result != SYS_MBOX_EMPTY)
    {
         tcpip_thread_handle_msg(pMsg);
    }

    sys_check_timeouts();
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_LWIP == DEF_ENABLED)
#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
