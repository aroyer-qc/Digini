//-------------------------------------------------------------------------------------------------
//
//  File : lib_ethernet_typedef.h
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
// Enum(s)
//-------------------------------------------------------------------------------------------------

// Ethernet MAC Timer Control Codes
enum ETH_ControlTimer_e
{
    ETH_TIMER_GET_TIME,                                 // Get Current Time
    ETH_TIMER_SET_TIME,                                 // Set New Time
    ETH_TIMER_INC_TIME,                                 // Increment Current Time
    ETH_TIMER_DEC_TIME,                                 // Decrement Current Time
    ETH_TIMER_SET_ALARM,                                // Set Alarm Time
    ETH_TIMER_ADJUST_CLOCK,                             // Adjust Clock Frequency; Time->NanoSecond: Correction Factor * 2^31
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





























// TODO find where it was used
// Message type for struct IP_Message_t
enum IP_MsgType_e
{
    IP_MSG_TYPE_IP_MANAGEMENT       = 0,
    IP_MSG_TYPE_DHCP_MANAGEMENT     = 1,
    IP_MSG_TYPE_SNTP_MANAGEMENT     = 2,
};

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
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct IP_ETH_Config_t
{
    IP_MAC_Address_t    MAC_Address;
    ETH_LinkDriver*     pLinkDriver;                // Genereic Driver (W5500, ENC28J60, STM32 via adaptor)
};

struct IP_Config_t                                  // Host Name, IP_ Address, Protocol (ip_cfg.h)
{
    const char*         pHostName;
    nOS_Stack*          pStack;
    uint16_t            ProtocolFlag;
    IP_Address_t        DefaultStatic_IP;           // check in context
    IP_Address_t        DefaultGateway;
    IP_Address_t        DefaultSubnetMask;
    IP_Address_t        DefaultStaticDNS;
    IP_ETH_Config_t     IP_ETH_Config;
};

// EMAC Driver Control Information
struct ETH_Control_t
{
    uint8_t                 TX_HeadIndex;           // Used by SendTX_Packet
    uint8_t                 TX_TailIndex;           // Used by ISR_CallBack
    uint8_t                 RX_Index;               // Receive descriptor index
  #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
    uint8_t                 TX_TS_Index;            // Transmit Timestamps descriptor index
  #endif
    uint8_t*                FrameEnd;               // End of assembled frame fragments
};

// Ethernet Link Info
struct ETH_LinkInfo_t
{
    ETH_LinkSpeed_e     Speed;                      // Link speed: 0 = 10 MBit, 1 = 100 MBit, 2 = 1 GBit
    ETH_Duplex_e        Duplex;                     // Duplex mode: 0 = Half, 1 = Full
};

struct  ETH_MacTime_t
{
    uint32_t naneSecond;                            // Nano seconds
    uint32_t Second;                                // Seconds
};


//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct UDP_Message_t
{
    uint32_t    RemoteIP;       // sender IP
    IP_Port_t   RemotePort;     // sender port
    uint32_t    LocalIP;        // destination IP (interface)
    IP_Port_t   LocalPort;      // destination port
    uint8_t*    pData;          // pointer to payload (zero-copy)
    uint16_t    Length;         // payload length
};

typedef struct
{
    uint8_t*    pData;         // pointer to received bytes (zero-copy)
    uint16_t    Length;        // number of bytes in this segment
} TCP_Message_t;

typedef struct
{
    uint32_t    RemoteIP;      // sender IP
    uint32_t    LocalIP;       // destination IP (interface)
    uint8_t     Protocol;      // IP protocol number (ICMP, IGMP, custom)
    uint8_t*    pData;         // pointer to payload (zero-copy)
    uint16_t    Length;        // payload length
} RAW_Message_t;

// Socket Address Structure
struct SocketInfo_t
{
    IP_Address_t    Address;
    IP_Port_t       Port;
};


struct UDP_Socket_t
{
    IP_Port_t       LocalPort;     // Bound port (0 = unbound)
  //IP_Address_t    LocalIP;       // Optional: only if multi-IP system
    uint16_t        Flags;         // Bitmask: broadcast allowed, reuse-port, etc. (optional, but future-proof)
};

struct RAW_Socket_t
{
    uint8_t         Protocol;           // IP protocol number (ICMP, etc.)
    IP_Address_t    LocalIP;            // Optional filter
    uint8_t         Flags;
};

union SocketProtocol_t
{
    TCP_Socket*     pTCP;   // Full C++ TCP protocol object.
                            // TCP is stateful and connection‑oriented, requiring:
                            //   - sequence/ack numbers
                            //   - sliding window management
                            //   - retransmission timers
                            //   - handshake/teardown state machine
                            //   - TX/RX buffering
                            //   - integration with TCP_Manager
                            // Because of this complexity, TCP uses a dedicated class
                            // rather than a lightweight struct.

    UDP_Socket_t*   pUDP;   // Lightweight POD (Plain Old Data) struct.
                            // UDP is stateless and connectionless:
                            //   - no sequence numbers
                            //   - no retransmission
                            //   - no timers
                            //   - no handshake
                            // Only stores bound port and simple flags, so a struct
                            // is sufficient and avoids unnecessary overhead.

    RAW_Socket_t*   pRAW;   // Lightweight POD struct.
                            // RAW sockets expose raw IP packets directly:
                            //   - no transport‑layer state
                            //   - no connection tracking
                            //   - no retransmission or timers
                            // Only protocol filters and flags are needed, so a
                            // simple struct is appropriate.

    void*           pPtr;   // Generic fallback pointer for future protocol types.
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


