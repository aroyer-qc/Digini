//=================================================================================================
//
//  File : ip_cfg.h
//
//  By   : Alain Royer
//
//  On   : December 5, 2011
//
//-------------------------------------------------------------------------------------------------

#ifndef __IP_CFG_H__
#define __IP_CFG_H__

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//--- PROTOCOL --------------------------------------------

#define 	IP_APP_USE_TCP 							DEF_ENABLED
#define 	IP_APP_USE_UDP 							DEF_ENABLED
#define 	IP_APP_USE_ICMP							DEF_ENABLED

//--- NIC driver list -------------------------------------

#define 	NIC_W5100								0
#define 	NIC_CS8900A								1

#define 	NIC_IN_USE								NIC_CS8900A

//---------------------------------------------------------

#define     IP_HOST_NAME                            "NetRadioAmp3"              // Host Name

#define     IP_MAC1                                 0x00                        // MAC
#define     IP_MAC2                                 0x20
#define     IP_MAC3                                 0xAF
#define     IP_MAC4                                 0x9E
#define     IP_MAC5                                 0x8D
#define     IP_MAC6                                 0x4E

#define     IP_STATIC_ADDR1                         192                         // if use static HOST
#define     IP_STATIC_ADDR2                         168
#define     IP_STATIC_ADDR3                         1
#define     IP_STATIC_ADDR4                         120

#define     IP_DEFAULT_GATEWAY1                     192                         // if use static DEFAULT GATEWAY
#define     IP_DEFAULT_GATEWAY2                     168
#define     IP_DEFAULT_GATEWAY3                     1
#define     IP_DEFAULT_GATEWAY4                     1

#define     IP_SUBNET_MASK1                         255                         // if use static SUBNET MASK
#define     IP_SUBNET_MASK2                         255
#define     IP_SUBNET_MASK3                         255
#define     IP_SUBNET_MASK4                         0

#define     IP_STATIC_DNS_ADDR1                     192                         // if use static HOST
#define     IP_STATIC_DNS_ADDR2                     168
#define     IP_STATIC_DNS_ADDR3                     1
#define     IP_STATIC_DNS_ADDR4                     1

#define     IP_DEFAULT_NTP_SERVER_1                 "0.ca.pool.ntp.org"
#define     IP_DEFAULT_NTP_SERVER_2                 "1.ca.pool.ntp.org"

//--- Socket allocation -----------------------------------
// in this stack socket can be allocated statically.
// It is useful for NIC with hardwared IP Stack like
// Wiznet W5100 with limited number of socket and 
// configurable socket memory size

#define 	IP_USE_STATIC_SOCKET					DEF_DISABLED

#if (IP_USE_STATIC_SOCKET == DEF_ENABLED)

// Socket use on for DHCP business
#define 	IP_SOCKET_DHCP                     		3



// ...

#endif //__IP_CFG_H__
//EOF
