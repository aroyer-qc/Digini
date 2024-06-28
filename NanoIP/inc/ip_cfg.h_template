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

//-------------------------------------------------------------------------------------------------
// Driver

// Include file for the ETH PHY Driver
#define PHY_DRIVER_INCLUDE                      "lib_class_PHY_LAN8742A.h"

// Name of the driver class
#define PHY_DRIVER_INTERFACE                    PHY_LAN8742A_Driver

//-------------------------------------------------------------------------------------------------
// Driver configuration  (may vary according to driver)

// for PHY_LAN8742A_Driver

#define ETH_USED_INTERFACE                      ETH_INTERFACE_RMII
#define ETH_USE_TIME_STAMP                      DEF_DISABLED
#define ETH_USE_CHECKSUM_OFFLOAD                DEF_DISABLED
#define ETH_DEBUG_PACKET_COUNT                  DEF_ENABLED
#define ETH_USE_PHY_LINK_IRQ                    DEF_DISABLED
#define ETH_PHY_LINK_IO                         IO_NOT_DEFINED  // Put herethe IO ID if supported

//-------------------------------------------------------------------------------------------------
// Protocol supported by the application

#define IP_USE_ARP 							    DEF_ENABLED
#define IP_USE_DHCP							    DEF_ENABLED     // Need UDP
#define IP_USE_ICMP							    DEF_ENABLED
#define IP_USE_NTP							    DEF_ENABLED
#define IP_USE_SNTP							    DEF_ENABLED
#define IP_USE_SOAP 						    DEF_ENABLED
#define IP_USE_TCP 							    DEF_ENABLED
#define IP_USE_UDP 							    DEF_ENABLED

//---------------------------------------------------------
// External server URL

#define IP_DEFAULT_NTP_SERVER_1                 "0.ca.pool.ntp.org"
#define IP_DEFAULT_NTP_SERVER_2                 "1.ca.pool.ntp.org"

//-------------------------------------------------------------------------------------------------
// Interface configuration

#define IP_NUMBER_OF_INTERFACE                  1

#define IP_INTERFACE_SUPPORT_PHY                DEF_ENABLED                 // Code generated for support of PHY controller (MAC is Build-in the CPU) 
#define IP_INTERFACE_SUPPORT_HEC                DEF_DISABLED                // Code generated for support of HEC (Harwired Ethernet Controller)  EX. Wiznet W5100S
#define IP_INTERFACE_SUPPORT_MAC                DEF_DISABLED                // Code generated for support of MAC and PHY chip SPI and other

// If IP Interface use host name
#define IP_USE_HOSTNAME                         DEF_ENABLED

// For each interface create the data structure for it. Note: include only element of structure that need to be included according to the configuration above

#define ETH_INTERFACE_ENUM  { ETH_IF_GRBL, ETH_IF_TEST };

// Interface 1 
#define ETH_IF_GRBL_CONFIG  {                                                                                                                                                     \
                                "GRBL",                                                                                      /* HostName                                       */ \
                                (IP_FLAG_USE_ARP | IP_FLAG_USE_DHCP | IP_FLAG_USE_ICMP | IP_FLAG_USE_TCP | IP_FLAG_USE_UDP), /* Create support for these protocol in interface */ \
                                IP_ADDRESS(192,168,0,254),                                                                   /* Default Static Address                         */ \
                                IP_ADDRESS(192,168,0,1),                                                                     /* Default Gateway                                */ \
                                IP_ADDRESS(255,255,255,0),                                                                   /* Default Subnet Mask                            */ \
                                IP_ADDRESS(192,168,0,1),                                                                     /* Default Static DNS                             */ \
                            },

// Interface 2 
#define ETH_IF_TEST_CONFIG  {                                                               \
                                "TEST",                         /* HostName              */ \
                                (IP_FLAG_USE_TCP | IP_FLAG_USE_UDP),                        \
                                IP_ADDRESS(192,168,0,253),      /* Default Static Address*/ \
                                IP_ADDRESS(192,168,0,1),        /* Default Gateway       */ \
                                IP_ADDRESS(255,255,255,0),      /* Default Subnet Mask   */ \
                                IP_ADDRESS(192,168,0,1),        /* Default Static DNS    */ \
                            },

// Interface List
#define ETH_IF_CONFIG_LIST  ETH_IF_GRBL_CONFIG ## ETH_IF_TEST_CONFIG







/*

// MAC address configuration using GUID of the CPU.
#define MAC_ADDR0	                            (((char *)0x1FF0F420)[0])
#define MAC_ADDR1	                            (((char *)0x1FF0F420)[2])
#define MAC_ADDR2	                            (((char *)0x1FF0F420)[4])
#define MAC_ADDR3	                            (((char *)0x1FF0F420)[6])
#define MAC_ADDR4	                            (((char *)0x1FF0F420)[8])
#define MAC_ADDR5	                            (((char *)0x1FF0F420)[10])

#define netifGUARD_BLOCK_TIME			        250
#define ETHERNET_FRAME_SIZE                     1514
#define BLOCK_TIME_WAITING_FOR_INPUT            250             // mSec

#define TASK_ETHERNET_IF_STACK_SIZE             512
#define TASK_ETHERNET_IF_PRIO                   4

// Define those to better describe your network interface.
#define IFNAME0 'd'
#define IFNAME1 'g'
#define IF_NAME "dg0"

//-------------------------------------------------------------------------------------------------











//--- Socket allocation -----------------------------------
// in this stack socket can be allocated statically.
// It is useful for NIC with hardwired IP Stack like
// Wiznet W5100S with limited number of socket and 
// configurable socket memory size

#define IP_USE_STATIC_SOCKET					DEF_DISABLED

#if (IP_USE_STATIC_SOCKET == DEF_ENABLED)

// Socket use on for DHCP business
#define IP_SOCKET_DHCP                     		3

//-------------------------------------------------------------------------------------------------

#define ETH_DEBUG_PACKET_COUNT

//-------------------------------------------------------------------------------------------------

#define IP_NET_IF_MTU                           1500                // not sure it is used in my stack



need to add to database_cfg.h :

    X_EEPROM_DBASE( ETHERNET_MAC_Address,        myE2_Setting, 1, 1, sizeof(MAC_t)) \
    X_EEPROM_DBASE( CONFIGURATION_IP,            myE2_Setting, 1, 1, sizeof(struct of configuration size)) \
    X_EEPROM_DBASE( ETHERNET_STATIC_HOST_IP,     myE2_Setting, 1, 1, sizeof(IP_Address_t));
    X_EEPROM_DBASE( ETHERNET_SUBNET_MASK_IP,     myE2_Setting, 1, 1, sizeof(IP_Address_t));
    X_EEPROM_DBASE( ETHERNET_GATEWAY_IP,         myE2_Setting, 1, 1, sizeof(IP_Address_t));






*/
