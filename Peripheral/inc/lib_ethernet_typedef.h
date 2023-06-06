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
    ETH_MAC_CONFIGURE,                                  // Configure MAC; arg = configuration
    ETH_MAC_CONTROL_TX,                                 // Transmitter; arg: 0=disabled (default), 1=enabled
    ETH_MAC_CONTROL_RX,                                 // Receiver; arg: 0=disabled (default), 1=enabled
    ETH_MAC_FLUSH,                                      // Flush buffer; arg = ETH_MAC_FLUSH_...
    ETH_MAC_SLEEP,                                      // Sleep mode; arg: 1=enter and wait for Magic packet, 0=exit
    ETH_MAC_VLAN_FILTER,                                // VLAN Filter for received frames; arg15..0: VLAN Tag; arg16: optional ETH_MAC_VLAN_FILTER_ID_ONLY; 0=disabled (default)
    ETH_DMA_CONFIGURE,                                  // Configure DMA; arg = configuration
};

// Ethernet MAC Timer Control Codes
enum ETH_ControlTimer_e
{
    ETH_MAC_TIMER_GET_TIME,                             // Get current time
    ETH_MAC_TIMER_SET_TIME,                             // Set new time
    ETH_MAC_TIMER_INC_TIME,                             // Increment current time
    ETH_MAC_TIMER_DEC_TIME,                             // Decrement current time
    ETH_MAC_TIMER_SET_ALARM,                            // Set alarm time
    ETH_MAC_TIMER_ADJUST_CLOCK,                         // Adjust clock frequency; time->ns: correction factor * 2^31
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
    ETH_STATE_UNKNOWN                      =     0,     // Driver uninitialized
    ETH_INITIALIZED                        =     1,     // Driver initialized
    ETH_POWERED_ON                         =     2,     // Driver power is on
    ETH_INITIALIZED_AND_POWERED_ON         =     3,     // Driver is initialized and power is on
};

// Ethernet Media Interface type
enum ETH_MediaInterface_e
{
    ETH_INTERFACE_MII,                  // Media Independent Interface (MII)
    ETH_INTERFACE_RMII,                 // Reduced Media Independent Interface (RMII)
    ETH_INTERFACE_SMII,                 // Serial Media Independent Interface (SMII)
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
    ETH_PHY_MODE_SPEED_10M             = 0x0001,
    ETH_PHY_MODE_SPEED_100M            = 0x0002,
    ETH_PHY_MODE_SPEED_1G              = 0x0004,
    ETH_PHY_MODE_SPEED_RESERVED        = 0x0008,
    ETH_PHY_MODE_SPEED_MASK            = 0x000F,


    ETH_PHY_MODE_DUPLEX_HALF           = 0x0010,
    ETH_PHY_MODE_DUPLEX_FULL           = 0x0020,
    ETH_PHY_MODE_DUPLEX_MASK           = 0x0030,


    ETH_PHY_MODE_AUTO_NEGOTIATE         = 0x0040,
    ETH_PHY_MODE_LOOPBACK               = 0x0080,
    ETH_PHY_MODE_ISOLATE                = 0x0100,
 };

// Ethernet Link State
enum ETH_LinkState_e
{
    ETH_LINK_DOWN,                      // Link is down
    ETH_LINK_UP                         // Link is up
};

// Ethernet Link Info
struct ETH_LinkInfo_t
{
    ETH_LinkSpeed_e     Speed;          // Link speed: 0 = 10 MBit, 1 = 100 MBit, 2 = 1 GBit
    ETH_Duplex_e        Duplex;         // Duplex mode: 0 = Half, 1 = Full
};

// Ethernet MAC Address
struct ETH_MAC_Address_t
{
  uint8_t Byte[6];                         ///< MAC Address (6 bytes), MSB first
};

typedef SystemState_e (*ETH_PHY_Read_t)  (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* pData);   // Read Ethernet PHY Register.
typedef SystemState_e (*ETH_PHY_Write_t) (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t   Data);   // Write Ethernet PHY Register.

//-------------------------------------------------------------------------------------------------




