//-------------------------------------------------------------------------------------------------
//
//  File : lib_ethernet_typedef.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

// Ethernet MAC and DMA Control Codes
enum ETH_ControlCode_e
{
    ETH_MAC_CONFIGURE,
    ETH_MAC_CONTROL_TX,                                 // Transmitter        : arg: 0=disabled (default), 1=enabled
    ETH_MAC_CONTROL_RX,                                 // Receiver           : arg: 0=disabled (default), 1=enabled
    ETH_MAC_FLUSH,                                      // Flush buffer       : arg = ETH_MAC_FLUSH_...
    ETH_MAC_SLEEP,                                      // Sleep mode         : arg 1=enter and wait for Magic packet, 0=exit
    ETH_MAC_VLAN_FILTER,                                // VLAN Filter for received frames: arg15..0: VLAN Tag; arg16: optional ETH_MAC_VLAN_FILTER_ID_ONLY; 0=disabled (default)
};

// Ethernet MAC Timer Control Codes
enum ETH_ControlTimer_e
{
    ETH_MAC_TIMER_GET_TIME,                             // Get Current Time
    ETH_MAC_TIMER_SET_TIME,                             // Set New Time
    ETH_MAC_TIMER_INC_TIME,                             // Increment Current Time
    ETH_MAC_TIMER_DEC_TIME,                             // Decrement Current Time
    ETH_MAC_TIMER_SET_ALARM,                            // Set Alarm Time
    ETH_MAC_TIMER_ADJUST_CLOCK,                         // Adjust Clock Frequency; Time->NanoSecond: Correction Factor * 2^31
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

enum ETH_PHY_Mode_e
 {
    ETH_PHY_MODE_NOT_DEFINED           = 0x0000,

    ETH_PHY_MODE_SPEED_10M             = 0x0001,
    ETH_PHY_MODE_SPEED_100M            = 0x0002,
    ETH_PHY_MODE_SPEED_1G              = 0x0004,
    ETH_PHY_MODE_SPEED_RESERVED        = 0x0008,
    ETH_PHY_MODE_SPEED_MASK            = 0x000F,


    ETH_PHY_MODE_DUPLEX_HALF           = 0x0010,
    ETH_PHY_MODE_DUPLEX_FULL           = 0x0020,
    ETH_PHY_MODE_DUPLEX_MASK           = 0x0030,


    ETH_PHY_MODE_AUTO_NEGOTIATE        = 0x0040,
    ETH_PHY_MODE_LOOPBACK              = 0x0080,
    ETH_PHY_MODE_ISOLATE               = 0x0100,
 };

// Ethernet Link State
enum ETH_LinkState_e
{
    ETH_LINK_DOWN,                                  // Link is Down
    ETH_LINK_UP,                                    // Link is Up
    ETH_LINK_UNKNOWN,
};

// Ethernet Link Info
struct ETH_LinkInfo_t
{
    ETH_LinkSpeed_e     Speed;                      // Link speed: 0 = 10 MBit, 1 = 100 MBit, 2 = 1 GBit
    ETH_Duplex_e        Duplex;                     // Duplex mode: 0 = Half, 1 = Full
};

// Ethernet MAC Address
struct ETH_MAC_Address_t
{
  uint8_t Byte[6];                                  // MAC Address (6 bytes), MSB first
};

struct ETH_MAC_Capability_t
{
    uint32_t checksum_offload_rx_ip4  : 1;          // 1 = IPv4 header checksum verified on receive
    uint32_t checksum_offload_rx_ip6  : 1;          // 1 = IPv6 checksum verification supported on receive
    uint32_t checksum_offload_rx_udp  : 1;          // 1 = UDP payload checksum verified on receive
    uint32_t checksum_offload_rx_tcp  : 1;          // 1 = TCP payload checksum verified on receive
    uint32_t checksum_offload_rx_icmp : 1;          // 1 = ICMP payload checksum verified on receive
    uint32_t checksum_offload_tx_ip4  : 1;          // 1 = IPv4 header checksum generated on transmit
    uint32_t checksum_offload_tx_ip6  : 1;          // 1 = IPv6 checksum generation supported on transmit
    uint32_t checksum_offload_tx_udp  : 1;          // 1 = UDP payload checksum generated on transmit
    uint32_t checksum_offload_tx_tcp  : 1;          // 1 = TCP payload checksum generated on transmit
    uint32_t checksum_offload_tx_icmp : 1;          // 1 = ICMP payload checksum generated on transmit
    uint32_t media_interface          : 2;          //     Ethernet Media Interface type (ETH_INTERFACE_MII or ETH_INTERFACE_RMII or ETH_INTERFACE_SMII)
    uint32_t mac_address              : 1;          // 1 = driver provides initial valid MAC address
    uint32_t event_rx_frame           : 1;          // 1 = callback event \ref ETH_MAC_EVENT_RX_FRAME generated
    uint32_t event_tx_frame           : 1;          // 1 = callback event \ref ETH_MAC_EVENT_TX_FRAME generated
    uint32_t event_wakeup             : 1;          // 1 = wakeup event \ref ETH_MAC_EVENT_WAKEUP generated
    uint32_t precision_timer          : 1;          // 1 = Precision Timer supported
    uint32_t reserved                 : 15;         // Reserved (must be zero)
};

struct  ETH_MacTime_t
{
    uint32_t naneSecond;                         // Nano seconds
    uint32_t Second;                             // Seconds
};

typedef SystemState_e (*ETH_PHY_Read_t)  (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* pData);   // Read Ethernet PHY Register.
typedef SystemState_e (*ETH_PHY_Write_t) (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t   Data);   // Write Ethernet PHY Register.

//-------------------------------------------------------------------------------------------------




