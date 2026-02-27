//-------------------------------------------------------------------------------------------------
//
//  File : lib_ethernet_enum.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define IP_MAX_URL_SIZE                         128
#define IP_MAC_ADDRESS_SIZE                     6
#define IP_MAC_V6_ADDRESS_SIZE                  8

#define IP_FLAG_USE_ARP                         0x0001
#define IP_FLAG_USE_DHCP                        0x0002
#define IP_FLAG_USE_ICMP                        0x0004
#define IP_FLAG_USE_NTP                         0x0008
#define IP_FLAG_USE_SNTP                        0x0010
#define IP_FLAG_USE_SOAP                        0x0020
#define IP_FLAG_USE_TCP                         0x0040
#define IP_FLAG_USE_UDP                         0x0080

#define IP_ETHERNET_FRAME_SIZE                  1518

#define IP_ETHERNET_TYPE_ARP 			        HTONS(0x0806)
#define IP_ETHERNET_TYPE_IPV4 			        HTONS(0x0800)
#define IP_ETHERNET_TYPE_IPV6 			        HTONS(0x86DD)

#define IP_VERSION4_IHL20				        0x45
#define IP_TIME_TO_LIVE					        128
#define IP_BROADCAST_ADDRESS                    0xFFFFFFFF

#define IP_MULTICAST_MIN                        224   // 224.x.x.x
#define IP_MULTICAST_MAX                        239   // 239.x.x.x

#define IP_PROTOCOL_ICMP				        0x01
#define IP_PROTOCOL_TCP					        0x06
#define IP_PROTOCOL_UDP					        0x11

#define DHCP_PACKET_SIZE                        576
#define DHCP_OPTION_IN_PACKET_SIZE              308
#define DHCP_HOSTNAME_MAX_LENGTH                63

#define DNS_MAX_PACKET_SIZE                     512

#if (IP_USE_DHCP == DEF_DISABLED)
    #define GetActiveGatewayIP()                GetStaticGatewayIP()
    #define GetActiveSubnetMask()               GetStaticSubnetMask()
    #define GetActiveIP()                       GetStaticIP()
    #define GetActiveDNS_IP()                   GetStaticDNS_IP()
#endif

//-------------------------------------------------------------------------------------------------
// Macro(s)
//-------------------------------------------------------------------------------------------------

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

    #define IP_ADDRESS(A,B,C,D)                 (uint32_t(A) + (uint32_t(B) << 8) + (uint32_t(C) << 16) + (uint32_t(D) << 24))
    #define IP_A(IP)                            uint8_t(IP)
    #define IP_B(IP)                            uint8_t(IP >> 8)
    #define IP_C(IP)                            uint8_t(IP >> 16)
    #define IP_D(IP)                            uint8_t(IP >> 24)

    // Use on static value to save code space
    #define HTONS(V)                            uint16_t(uint16_t(V) << 8 | uint16_t(V) >> 8)
    #define HTONL(V)                            (uint32_t((((V) & 0x000000FF) << 24) | (((V) & 0x0000FF00) << 8 ) | (((V) & 0x00FF0000) >> 8 ) | (((V) & 0xFF000000) >> 24)))

#else

    #define IP_ADDRESS(A,B,C,D)                 (uint32_t(D) + (uint32_t(C) << 8) + (uint32_t(B) << 16) + (uint32_t(A) << 24))
    #define IP_A(IP)                            uint8_t(IP >> 24)
    #define IP_B(IP)                            uint8_t(IP >> 16)
    #define IP_C(IP)                            uint8_t(IP >> 8)
    #define IP_D(IP)                            uint8_t(IP)

    #define HTONS(V)                            uint16_t(V)
    #define HTONL(V)                            uint32_t(V)

#endif

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum IP_EthernetIF_e
{
    ETH_INTERFACE_ENUM
};

// Ethernet MAC or PHY Power State
enum ETH_PowerState_e
{
    ETH_POWER_OFF,
    ETH_POWER_LOW,
    ETH_POWER_FULL,
};

// Driver State
enum ETH_State_e
{
    ETH_STATE_UNKNOWN                      =     0,     // Driver Uninitialized
    ETH_INITIALIZED                        =     1,     // Driver Initialized
    ETH_POWERED_ON                         =     2,     // Driver Power is on
    ETH_INITIALIZED_AND_POWERED_ON         =     3,     // Driver is Initialized and Power is on
};

// Ethernet Media Interface type
enum ETH_MediaInterface_e
{
    ETH_INTERFACE_MII,                                  // Media Independent Interface (MII)
    ETH_INTERFACE_RMII,                                 // Reduced Media Independent Interface (RMII)
    ETH_INTERFACE_SMII,                                 // Serial Media Independent Interface (SMII)
};

// Ethernet Link Speed
enum ETH_LinkSpeed_e
{
    ETH_PHY_SPEED_NONE,
    ETH_PHY_SPEED_10M,
    ETH_PHY_SPEED_100M,
    ETH_PHY_SPEED_1G,
};

// Ethernet Link Duplex
enum ETH_Duplex_e
{
    ETH_PHY_HALF_DUPLEX,
    ETH_PHY_FULL_DUPLEX,
};

// Ethernet Link State
enum ETH_LinkState_e
{
    ETH_LINK_DOWN,                                  // Link is Down
    ETH_LINK_UP,                                    // Link is Up
    ETH_LINK_UNKNOWN,
};

enum TCP_State_e
{
    TCP_STATE_CLOSED = 0,
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECEIVED,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_LAST_ACK,
    TCP_STATE_TIME_WAIT,
    TCP_STATE_ERROR
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


