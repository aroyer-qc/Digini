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

//----- INTERFACE TYPE SUPPORT -----

#define IP_INTERFACE_SUPPORT_PHY                DEF_ENABLED                 // Code generated for support of PHY controller (MAC is Build-in the CPU) 
#define IP_INTERFACE_SUPPORT_HEC                DEF_DISABLED                // Code generated for support of HEC (Harwired Ethernet Controller)  EX. Wiznet W5100S
#define IP_INTERFACE_SUPPORT_MAC                DEF_DISABLED                // Code generated for support of MAC and PHY chip SPI and other

#define IP_USE_HOSTNAME                         DEF_ENABLED


//----- PROTOCOL -----
#define 	IP_USE_ARP 							DEF_ENABLED
#define 	IP_USE_DHCP							DEF_ENABLED
#define 	IP_USE_ICMP							DEF_ENABLED
#define  	IP_USE_NTP							DEF_ENABLED
#define 	IP_USE_SNTP							DEF_ENABLED
#define  	IP_USE_SOAP 						DEF_ENABLED
#define 	IP_USE_TCP 							DEF_ENABLED
#define 	IP_USE_UDP 							DEF_ENABLED

// If CPU has GUID serial number like some STM32, and want to use this as MAC
#define IP_USE_PART_OF_CPU_GUID_AS_MAC          DEF_ENABLED                 // Function into CPU MAC driver to provide MAC from GUID.
#define IP_CUSTOM_CALLBACK_FOR_MAC_ATTRIBUTION  DEF_DISABLED                // If more than one IF present, it might be useful for user to attribute the MAC Address for each IF. 

//---------------------------------------------------------

#if (IP_USE_PART_OF_CPU_GUID_AS_MAC == DEF_DISABLED)
  #define   IP_MAC1                                 0x00                    // Custom MAC (single IF)
  #define   IP_MAC2                                 0x20
  #define   IP_MAC3                                 0xAF
  #define   IP_MAC4                                 0x9E
  #define   IP_MAC5                                 0x8D
  #define   IP_MAC6                                 0x4E
#endif

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

#define     ETH_DEBUG_PACKET_COUNT

//-------------------------------------------------------------------------------------------------


//XMACRO to define interface, so we can have more than one IP_Manager class (multiple IF)
#define X_ETH_IF_DEF(X_ETH_IF)   \
/*            ENUM_ID,        HostName,  MAC,                                                  Default Static Address,    Default Gateway,         Default Subnet Mask,       DefaultStatic DNS */\  
    X_ETH_IF (ETH_IF_GRBL,    "GRBL",    IP_MAC1, IP_MAC2, IP_MAC3, IP_MAC4, IP_MAC5, IP_MAC6, IP_ADDRESS(192,168,0,100), IP_ADDRESS(192,168,0,1), IP_ADDRESS(255,255,255,0), IP_ADDRESS(192,168,0,1) ) \
                                          















/*

need to add to database_cfg.h :

    X_EEPROM_DBASE( ETHERNET_MAC_Address,        myE2_Setting, 1, 1, sizeof(MAC_t)) \
    X_EEPROM_DBASE( CONFIGURATION_IP,            myE2_Setting, 1, 1, sizeof(struct of configuration size)) \
    X_EEPROM_DBASE( ETHERNET_STATIC_HOST_IP,     myE2_Setting, 1, 1, sizeof(IP_Address_t));
    X_EEPROM_DBASE( ETHERNET_SUBNET_MASK_IP,     myE2_Setting, 1, 1, sizeof(IP_Address_t));
    X_EEPROM_DBASE( ETHERNET_GATEWAY_IP,         myE2_Setting, 1, 1, sizeof(IP_Address_t));






