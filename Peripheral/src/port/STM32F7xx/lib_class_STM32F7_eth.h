//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_eth.h
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <string.h>
#include "nOS.h"
#include "ethif_config.h"


#define DIGINI_USE_ETH_TIME_STAMP       DEF_DISABLED        // put in cfg file
#define DIGINI_USE_ETH_CHECKSUM_OFFLOAD DEF_DISABLED        // put in cfg file

/****** Ethernet MAC Control Codes *****/

#define ETH_MAC_CONFIGURE           (0x01UL)    // Configure MAC; arg = configuration
#define ETH_MAC_CONTROL_TX          (0x02UL)    // Transmitter; arg: 0=disabled (default), 1=enabled
#define ETH_MAC_CONTROL_RX          (0x03UL)    // Receiver; arg: 0=disabled (default), 1=enabled
#define ETH_MAC_FLUSH               (0x04UL)    // Flush buffer; arg = ETH_MAC_FLUSH_...
#define ETH_MAC_SLEEP               (0x05UL)    // Sleep mode; arg: 1=enter and wait for Magic packet, 0=exit
#define ETH_MAC_VLAN_FILTER         (0x06UL)    // VLAN Filter for received frames; arg15..0: VLAN Tag; arg16: optional ETH_MAC_VLAN_FILTER_ID_ONLY; 0=disabled (default)

/*----- Ethernet MAC Configuration -----*/
#define ETH_MAC_SPEED_Pos            0
#define ETH_MAC_SPEED_Msk           (3UL             << ETH_MAC_SPEED_Pos)
#define ETH_MAC_SPEED_10M           (ETH_SPEED_10M   << ETH_MAC_SPEED_Pos)  // 10 Mbps link speed
#define ETH_MAC_SPEED_100M          (ETH_SPEED_100M  << ETH_MAC_SPEED_Pos)  // 100 Mbps link speed
#define ETH_MAC_SPEED_1G            (ETH_SPEED_1G    << ETH_MAC_SPEED_Pos)  // 1 Gpbs link speed
#define ETH_MAC_DUPLEX_Pos           2
#define ETH_MAC_DUPLEX_Msk          (1UL                 << ETH_MAC_DUPLEX_Pos)
#define ETH_MAC_DUPLEX_HALF         (ETH_DUPLEX_HALF << ETH_MAC_DUPLEX_Pos) // Half duplex link
#define ETH_MAC_DUPLEX_FULL         (ETH_DUPLEX_FULL << ETH_MAC_DUPLEX_Pos) // Full duplex link
#define ETH_MAC_LOOPBACK            (1UL << 4)  // Loop-back test mode
#define ETH_MAC_CHECKSUM_OFFLOAD_RX (1UL << 5)  // Receiver Checksum offload
#define ETH_MAC_CHECKSUM_OFFLOAD_TX (1UL << 6)  // Transmitter Checksum offload
#define ETH_MAC_ADDRESS_BROADCAST   (1UL << 7)  // Accept frames with Broadcast address
#define ETH_MAC_ADDRESS_MULTICAST   (1UL << 8)  // Accept frames with any Multicast address
#define ETH_MAC_ADDRESS_ALL         (1UL << 9)  // Accept frames with any address (Promiscuous Mode)

/*----- Ethernet MAC Flush Flags -----*/
#define ETH_MAC_FLUSH_RX            (1UL << 0)  // Flush Receive buffer
#define ETH_MAC_FLUSH_TX            (1UL << 1)  // Flush Transmit buffer

/*----- Ethernet MAC VLAN Filter Flag -----*/
#define ETH_MAC_VLAN_FILTER_ID_ONLY (1UL << 16) // Compare only the VLAN Identifier (12-bit)


/****** Ethernet MAC Frame Transmit Flags *****/
#define ETH_MAC_TX_FRAME_FRAGMENT   (1UL << 0)  // Indicate frame fragment
#define ETH_MAC_TX_FRAME_EVENT      (1UL << 1)  // Generate event when frame is transmitted
#define ETH_MAC_TX_FRAME_TIMESTAMP  (1UL << 2)  // Capture frame time stamp


/****** Ethernet MAC Timer Control Codes *****/
#define ETH_MAC_TIMER_GET_TIME      (0x01UL)    // Get current time
#define ETH_MAC_TIMER_SET_TIME      (0x02UL)    // Set new time
#define ETH_MAC_TIMER_INC_TIME      (0x03UL)    // Increment current time
#define ETH_MAC_TIMER_DEC_TIME      (0x04UL)    // Decrement current time
#define ETH_MAC_TIMER_SET_ALARM     (0x05UL)    // Set alarm time
#define ETH_MAC_TIMER_ADJUST_CLOCK  (0x06UL)    // Adjust clock frequency; time->ns: correction factor * 2^31


/****** Ethernet MAC Event *****/
#define ETH_MAC_EVENT_RX_FRAME      (1UL << 0)  // Frame Received
#define ETH_MAC_EVENT_TX_FRAME      (1UL << 1)  // Frame Transmitted
#define ETH_MAC_EVENT_WAKEUP        (1UL << 2)  // Wake-up (on Magic Packet)
#define ETH_MAC_EVENT_TIMER_ALARM   (1UL << 3)  // Timer Alarm

// DMA configure not in Driver_ETH_MAC.h
#define ETH_DMA_CONFIGURE           (0x07UL)    // Configure DMA; arg = configuration

#define ETH_DMABMR_REG				(0x01UL << 28)
#define ETH_DMAOMR_REG				(0x02UL << 28)

// EMAC Driver state flags
#define EMAC_FLAG_INIT      (1 << 0)    // Driver initialized
#define EMAC_FLAG_POWER     (1 << 1)    // Driver power on
#define EMAC_FLAG_DMA_INIT  (1 << 2)    // DMA Initialized

// TDES0 - DMA Descriptor TX Packet Control/Status
#define DMA_TX_OWN      0x80000000U     // Own bit 1=DMA,0=CPU
#define DMA_TX_IC       0x40000000U     // Interrupt on completion
#define DMA_TX_LS       0x20000000U     // Last segment
#define DMA_TX_FS       0x10000000U     // First segment
#define DMA_TX_DC       0x08000000U     // Disable CRC
#define DMA_TX_DP       0x04000000U     // Disable pad
#define DMA_TX_TTSE     0x02000000U     // Transmit time stamp enable
#define DMA_TX_CIC      0x00C00000U     // Checksum insertion control
#define DMA_TX_CIC_IP   0x00400000U     // Checksum insertion for IP header only
#define DMA_TX_TER      0x00200000U     // Transmit end of ring
#define DMA_TX_TCH      0x00100000U     // Second address chained
#define DMA_TX_TTSS     0x00020000U     // Transmit time stamp status
#define DMA_TX_IHE      0x00010000U     // IP header error status
#define DMA_TX_ES       0x00008000U     // Error summary
#define DMA_TX_JT       0x00004000U     // Jabber timeout
#define DMA_TX_FF       0x00002000U     // Frame flushed
#define DMA_TX_IPE      0x00001000U     // IP payload error
#define DMA_TX_LC       0x00000800U     // Loss of carrier
#define DMA_TX_NC       0x00000400U     // No carrier
#define DMA_TX_LCOL     0x00000200U     // Late collision
#define DMA_TX_EC       0x00000100U     // Excessive collision
#define DMA_TX_VF       0x00000080U     // VLAN frame
#define DMA_TX_CC       0x00000078U     // Collision count
#define DMA_TX_ED       0x00000004U     // Excessive deferral
#define DMA_TX_UF       0x00000002U     // Underflow error
#define DMA_TX_DB       0x00000001U     // Deferred bit

// TDES1 - DMA Descriptor TX Packet Control
#define DMA_RX_TBS2     0x1FFF0000U     // Transmit buffer 2 size
#define DMA_RX_TBS1     0x00001FFFU     // Transmit buffer 1 size

// RDES0 - DMA Descriptor RX Packet Status
#define DMA_RX_OWN      0x80000000U     // Own bit 1=DMA,0=CPU
#define DMA_RX_AFM      0x40000000U     // Destination address filter fail
#define DMA_RX_FL       0x3FFF0000U     // Frame length mask
#define DMA_RX_ES       0x00008000U     // Error summary
#define DMA_RX_DE       0x00004000U     // Descriptor error
#define DMA_RX_SAF      0x00002000U     // Source address filter fail
#define DMA_RX_LE       0x00001000U     // Length error
#define DMA_RX_OE       0x00000800U     // Overflow error
#define DMA_RX_VLAN     0x00000400U     // VLAN tag
#define DMA_RX_FS       0x00000200U     // First descriptor
#define DMA_RX_LS       0x00000100U     // Last descriptor
#define DMA_RX_IPHCE    0x00000080U     // IPv4 header checksum error
#define DMA_RX_LC       0x00000040U     // late collision
#define DMA_RX_FT       0x00000020U     // Frame type
#define DMA_RX_RWT      0x00000010U     // Receive watchdog timeout
#define DMA_RX_RE       0x00000008U     // Receive error
#define DMA_RX_DRE      0x00000004U     // Dribble bit error
#define DMA_RX_CE       0x00000002U     // CRC error
#define DMA_RX_RMAM     0x00000001U     // Rx MAC adr.match/payload cks.error

// RDES1 - DMA Descriptor RX Packet Control
#define DMA_RX_DIC      0x80000000U     // Disable interrupt on completion
#define DMA_RX_RBS2     0x1FFF0000U     // Receive buffer 2 size
#define DMA_RX_RER      0x00008000U     // Receive end of ring
#define DMA_RX_RCH      0x00004000U     // Second address chained
#define DMA_RX_RBS1     0x00001FFFU     // Receive buffer 1 size

enum ETH_PowerState_e
{
    ETH_POWER_OFF,
    ETH_POWER_LOW,
    ETH_POWER_FULL,
};

typedef void (*ETH_MAC_SignalEvent_t) (uint32_t event);  // Pointer to \ref ETH_MAC_SignalEvent : Signal Ethernet Event.

struct ETH_MacCapabilities_t
{
    uint32_t checksum_offload_rx_ip4  : 1;        // 1 = IPv4 header checksum verified on receive
    uint32_t checksum_offload_rx_ip6  : 1;        // 1 = IPv6 checksum verification supported on receive
    uint32_t checksum_offload_rx_udp  : 1;        // 1 = UDP payload checksum verified on receive
    uint32_t checksum_offload_rx_tcp  : 1;        // 1 = TCP payload checksum verified on receive
    uint32_t checksum_offload_rx_icmp : 1;        // 1 = ICMP payload checksum verified on receive
    uint32_t checksum_offload_tx_ip4  : 1;        // 1 = IPv4 header checksum generated on transmit
    uint32_t checksum_offload_tx_ip6  : 1;        // 1 = IPv6 checksum generation supported on transmit
    uint32_t checksum_offload_tx_udp  : 1;        // 1 = UDP payload checksum generated on transmit
    uint32_t checksum_offload_tx_tcp  : 1;        // 1 = TCP payload checksum generated on transmit
    uint32_t checksum_offload_tx_icmp : 1;        // 1 = ICMP payload checksum generated on transmit
    uint32_t media_interface          : 2;        //     Ethernet Media Interface type (ETH_INTERFACE_MII or ETH_INTERFACE_RMII)
    uint32_t mac_address              : 1;        // 1 = driver provides initial valid MAC address
    uint32_t event_rx_frame           : 1;        // 1 = callback event \ref ETH_MAC_EVENT_RX_FRAME generated
    uint32_t event_tx_frame           : 1;        // 1 = callback event \ref ETH_MAC_EVENT_TX_FRAME generated
    uint32_t event_wakeup             : 1;        // 1 = wakeup event \ref ETH_MAC_EVENT_WAKEUP generated
    uint32_t precision_timer          : 1;        // 1 = Precision Timer supported
    uint32_t reserved                 : 15;       // Reserved (must be zero)
};

struct  ETH_MacTime_t
{
    uint32_t ns;                          // Nano seconds
    uint32_t sec;                         // Seconds
};

// EMAC Driver Control Information
struct ETH_MacControl_t
{
    ETH_MAC_SignalEvent_t   CallbackEvent;          // Event callback
    uint8_t                 Flags;                  // Control and state flags
    uint8_t                 TX_Index;               // Transmit descriptor index
    uint8_t                 RX_Index;               // Receive descriptor index
  #if (DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED)
    bool                    TX_ChecksumsOffload;    // Checksum offload enabled/disabled
  #endif
  #if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
    uint8_t                 TX_TS_Index;            // Transmit Timestamp descriptor index
  #endif
    uint8_t*                Frame_End;              // End of assembled frame fragments
};

class ETH_Driver
{
    public:
    
        ETH_MacCapabilities_t   GetCapabilities         (void);                                                         // Get driver capabilities.
        int32_t                 Initialize              (ETH_MAC_SignalEvent_t cb_event);                               // Initialize Ethernet MAC Device.
        
        // TODO will disappear if not used.. or not
        int32_t                 Uninitialize            (void);                                                         // De-initialize Ethernet MAC Device.
        int32_t                 PowerControl            (ETH_PowerState_e state);                                       // Control Ethernet MAC Device Power.
        int32_t                 GetMacAddress           (      ETH_MacAddress_t* ptr_addr);                             // Get Ethernet MAC Address.
        int32_t                 SetMacAddress           (const ETH_MacAddress_t* ptr_addr);                             // Set Ethernet MAC Address.
        int32_t                 SetAddressFilter        (const ETH_MacAddress_t* ptr_addr, uint32_t num_addr);          // Configure Address Filter.
        int32_t                 SendFrame               (const uint8_t* frame, uint32_t len, uint32_t flags);           // Send Ethernet frame.
        SystemState_e           ReadFrame               (struct pbuf* p, uint32_t len);                                 // Read data of received Ethernet frame.
        uint32_t                GetRX_FrameSize         (void);                                                         // Get size of received Ethernet frame.
      #if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
        int32_t                 GetRX_FrameTime         (ETH_MacTime_t* Time);                                          // Get time of received Ethernet frame.
        int32_t                 GetTX_FrameTime         (ETH_MacTime_t* Time);                                          // Get time of transmitted Ethernet frame.
        int32_t                 ControlTimer            (uint32_t control, ETH_MacTime_t* Time);                        // Control Precision Timer.
      #endif  
        int32_t                 Control                 (uint32_t control, uint32_t arg);                               // Control Ethernet Interface.
        int32_t                 PHY_Read                (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* Data); // Read Ethernet PHY Register through Management Interface.
        int32_t                 PHY_Write               (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t  Data); // Write Ethernet PHY Register through Management Interface.
        
    private:

        void                    InitializeDMA           (void);
        ETH_MAC_Capability_t    GetCapabilities         (void);
        void                    ResetMac                (void);
        void                    EnableClock             (void);
        void                    DisableClock            (void);
        SystemState_e           PHY_Busy                (void);

        const   ETH_MacCapabilities_t       m_Capability;
                ETH_EmacControl             m_EMAC_Control;
/*static ?? */     RX_Descriptor_t             RX_Descriptor   [NUM_RX_Buffer]                     __attribute__((section(".dmaINIT"), aligned(4)));   // Ethernet RX & TX DMA Descriptors
/*static ?? */     uint32_t                    RX_Buffer       [NUM_RX_Buffer][ETH_BUF_SIZE >> 2]  __attribute__((section(".dmaINIT"), aligned(4)));   // Ethernet Receive buffers
/*static ?? */     TX_Descriptor_t             TX_Descriptor   [NUM_TX_Buffer]                     __attribute__((section(".dmaINIT"), aligned(4)));
/*static ?? */     uint32_t                    TX_Buffer       [NUM_TX_Buffer][ETH_BUF_SIZE >> 2]  __attribute__((section(".dmaINIT"), aligned(4)));   // Ethernet Transmit buffers


};

