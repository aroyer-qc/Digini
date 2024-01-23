//=================================================================================================
//-------------------------------------------------------------------------------------------------
//
//  File : ip_cfg.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2011-2024 Alain Royer.
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//----- PROTOCOL -----

#define 	IP_APP_USE_DHCP							DEF_ENABLED
#define 	IP_APP_USE_TCP 							DEF_ENABLED
#define 	IP_APP_USE_UDP 							DEF_ENABLED
#define 	IP_APP_USE_ICMP							DEF_ENABLED

//----- NIC driver list -----

//#define 	NIC_W5100								0
//#define 	NIC_CS8900A								1
//#define 	NIC_IN_USE								NIC_CS8900A         // this will be in driver

// If CPU has GUID serial number like some STM32, and want to use this as MAC
#define IP_USE_PART_OF_CPU_GUID_AS_MAC              DEF_ENABLED

//---------------------------------------------------------

#define     IP_HOST_NAME                            "myHost"                // Host Name

#if (IP_USE_PART_OF_CPU_GUID_AS_MAC == DEF_DISABLED)
  #define   IP_MAC1                                 0x00                        // MAC
  #define   IP_MAC2                                 0x20
  #define   IP_MAC3                                 0xAF
  #define   IP_MAC4                                 0x9E
  #define   IP_MAC5                                 0x8D
  #define   IP_MAC6                                 0x4E
#endif

#define     IP_DEFAULT_STATIC_Address1                 192                         // if use static HOST
#define     IP_DEFAULT_STATIC_Address2                 168
#define     IP_DEFAULT_STATIC_Address3                 1
#define     IP_DEFAULT_STATIC_Address4                 254

#define     IP_DEFAULT_GATEWAY1                     192                         // if use static DEFAULT GATEWAY
#define     IP_DEFAULT_GATEWAY2                     168
#define     IP_DEFAULT_GATEWAY3                     1
#define     IP_DEFAULT_GATEWAY4                     1

#define     IP_SUBNET_MASK1                         255                         // if use static SUBNET MASK
#define     IP_SUBNET_MASK2                         255
#define     IP_SUBNET_MASK3                         255
#define     IP_SUBNET_MASK4                         0

#define     IP_DEFAULT_STATIC_DNS_Address1             192                         // if use static HOST
#define     IP_DEFAULT_STATIC_DNS_Address2             168
#define     IP_DEFAULT_STATIC_DNS_Address3             1
#define     IP_DEFAULT_STATIC_DNS_Address4             1

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

//-------------------------------------------------------------------------------------------------

/*

need to add to database_cfg.h :

    X_EEPROM_DBASE( ETHERNET_MAC_Address,        myE2_Setting, 1, 1, sizeof(MAC_t)) \
    X_EEPROM_DBASE( CONFIGURATION_IP,            myE2_Setting, 1, 1, sizeof(struct of configuration size)) \
    X_EEPROM_DBASE( ETHERNET_STATIC_HOST_IP,     myE2_Setting, 1, 1, sizeof(IP_Address_t));
    X_EEPROM_DBASE( ETHERNET_SUBNET_MASK_IP,     myE2_Setting, 1, 1, sizeof(IP_Address_t));
    X_EEPROM_DBASE( ETHERNET_GATEWAY_IP,         myE2_Setting, 1, 1, sizeof(IP_Address_t));
