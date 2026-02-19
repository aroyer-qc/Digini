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
// class(s)
//-------------------------------------------------------------------------------------------------

class NetworkContext;
class IP_Manager;
class ARP_Manager;
class DHCP_Manager;
class DNS_Manager;
class ICMP_Manager;
class NTP_Manager;
class IP_Manager;
class RAW_Manager;
class SocketManager;
class Socket;
class SNTP_Manager;
class TCP_Socket;
class TCP_Manager;
class UDP_Manager;
class ETH_IF_Driver;

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

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

enum ARP_State_e
{
    ARP_STATE_EMPTY = 0,     // No entry
    ARP_STATE_PENDING,       // ARP request sent, waiting for reply
    ARP_STATE_VALID          // MAC resolved and usable
};

enum DNS_State_e
{
    DNS_STATE_IDLE = 0,
    DNS_STATE_WAIT_RESPONSE,
    DNS_STATE_RESPONSE_RECEIVED,
    DNS_STATE_TIMEOUT
};

// Socket Types
enum SocketType_e
{
    SOCKET_TYPE_NONE,
    SOCKET_TYPE_STREAM,
    SOCKET_TYPE_DATAGRAM,
    SOCKET_TYPE_RAW_IP,
    SOCKET_TYPE_UNKNOWN,
    SOCKET_TYPE_INVALID
};

// Socket States
enum SocketState_e
{
    SOCKET_STATE_CLOSED,
    SOCKET_STATE_LISTENING,
    SOCKET_STATE_CONNECTING,
    SOCKET_STATE_CONNECTED,
    SOCKET_STATE_CLOSING,
    SOCKET_STATE_ERROR
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

// Socket Options
enum SocketOption_e
{
    SOCKET_OPT_NON_BLOCKING,
    SOCKET_OPT_BROADCAST,
    SOCKET_OPT_REUSE_ADDRESS,
    SOCKET_OPT_TIMEOUT,
    SOCKET_OPT_KEEP_ALIVE,
};

enum SocketFlag_e
{
    SOCKET_FLAG_NONE        = 0x00000000,

    SOCKET_FLAG_BROADCAST   = 0x00000001,   // Allow sending to 255.255.255.255
    SOCKET_FLAG_REUSEADDR   = 0x00000002,   // Allow binding to an address already in use

    // Future flags:
    SOCKET_FLAG_KEEPALIVE   = 0x00000004,   // TCP keepalive
    SOCKET_FLAG_LINGER      = 0x00000008,   // TCP linger
    SOCKET_FLAG_RESERVED1   = 0x00000010,
    SOCKET_FLAG_RESERVED2   = 0x00000020,

};

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef uint32_t    IP_Address_t;
typedef uint16_t    IP_Port_t;

typedef void (*DNS_Callback_t)(void* pContext, bool Success, IP_Address_t ResolveIP);

#pragma pack(push, 1)

struct IP_MAC_Address_t
{
    uint8_t     Byte[IP_MAC_ADDRESS_SIZE];
};

struct ARP_TableEntry_t
{
    IP_Address_t        IP_Address;
    IP_MAC_Address_t    MAC_Address;
    ARP_State_e         State;                      // Entry state
    uint8_t             TimeToLive;                 // Optional aging counter
};

// The Ethernet header
struct IP_EthernetHeader_t
{
	IP_MAC_Address_t 	DestinationMAC;                 //     6
	IP_MAC_Address_t 	SourceMAC;                      // +   6
	uint16_t		    Type;                           // +   2
};                           	                        // 14 Bytes

struct DHCP_Header_t
{
	uint8_t         Op;                                 //     1
	uint8_t	        Htype;                              // +   1
	uint8_t	        Hlen;                               // +   1
	uint8_t	        Hops;                               // +   1
	uint32_t        X_ID;                               // +   4
	uint16_t 	    Secs;                               // +   2
	uint16_t 	    Flags;                              // +   2
	IP_Address_t    ClientIP_Addr;                      // +   4
	IP_Address_t    YourIP_Addr;                        // +   4
	IP_Address_t    ServerIP_Addr;                      // +   4
	IP_Address_t    RelayAgentIP_Addr;                  // +   4
	uint8_t         ClientHardware[16];                 // +  16
	uint8_t	        Sname[64];                          // +  64
	uint8_t	        File[128];                          // + 128
	uint32_t        MagicCookie;                        // +   4
	uint8_t         Options[308];                       // Do not use this struct with sizeof()
};                                                      // = 240 bytes + Options

struct DNS_Header_t
{
    uint16_t        ID;                                 //     2
    uint16_t        Flags;                              // +   2
    uint16_t        QDCount;                            // +   2
    uint16_t        ANCount;                            // +   2
    uint16_t        NSCount;                            // +   2
    uint16_t        ARCount;                            // +   2
     uint8_t        Payload[DNS_MAX_PACKET_SIZE];       // +   512
};                                                      // = 524 bytes

struct ICMP_Header_t
{
	uint8_t         Type;                               //     1
    uint8_t         Code;                               // +   1
	uint16_t	    Checksum;                           // +   2
    uint16_t        ID;                                 // +   2
    uint16_t 	    Sequence;                           // +   2
};                                                      // =   8 Bytes

struct IP_Header_t
{
	uint8_t 	    VersionIHL;                         //     1    IHL  = Internet Header Length
    uint8_t 	    TypeOfService;                      // +   1
    uint16_t	    Length;                             // +   2
    uint16_t 	    ID;                                 // +   2
    uint16_t 	    FlagsFragmentOffset;                // +   2
    uint8_t 	    TimeToLive;                         // +   1
    uint8_t 	    Protocol;                           // +   1
	uint16_t        Checksum;                           // +   2
	IP_Address_t    SrcIP_Address;                      // +   4
    IP_Address_t    DstIP_Address;                      // +   4
};                              		                // =  20 Bytes

struct TCP_Header_t
{
	IP_Port_t 	    SrcPort;                            //     2
    IP_Port_t 	    DstPort;                            // +   2
	uint32_t 	    SequenceNumber;                     // +   4
    uint32_t 	    AcknowledgeNumber;                  // +   4
    uint8_t 	    Offset;                             // +   1
    uint8_t 	    Flags;                              // +   1
    uint16_t 	    Window;                             // +   2
	uint16_t 	    Checksum;                           // +   2
	uint16_t 	    UrgentPointer;                      // +   2
	uint32_t        OptionData;                         // +   4
};                              	                    // =  20 Bytes before option data

struct UDP_Header_t
{
	IP_Port_t 	    SrcPort;                            //     2
	IP_Port_t 	    DstPort;                            // +   2
	uint16_t	    Length;                             // +   2
	uint16_t	    Checksum;                           // +   2
};                                                      // =   8 Bytes


struct IP_PseudoHeader_t                                // note that the element are not in same order as define in the protocol,
{                                                       // but match the IP in the union
	uint8_t         Zero_s;                             //     1
	uint8_t         Protocol;                           // +   1
    uint16_t        Length;                             // +   2
	IP_Address_t  	SrcIP;                              // +   4
	IP_Address_t  	DstIP;                              // +   4
}; 	                                                    // =  12 Bytes

struct SNTP_Header_t
{
    uint8_t   LI_VN_Mode;                               //     1    Leap Indicator (2 bits), Version (3 bits), Mode (3 bits)
    uint8_t   Stratum;                                  // +   1    0 for SNTP client request
    uint8_t   Poll;                                     // +   1    Not used by client
    int8_t    Precision;                                // +   1    Not used by client

    uint32_t  RootDelay;                                // +   4    Always 0 in SNTP request
    uint32_t  RootDispersion;                           // +   4    Always 0 in SNTP request
    uint32_t  ReferenceID;                              // +   4    Always 0 in SNTP request

    uint64_t  ReferenceTimestamp;                       // +   8    Not used by client
    uint64_t  OriginateTimestamp;                       // +   8    T1 (client send time)
    uint64_t  ReceiveTimestamp;                         // +   8    T2 (server receive time)
    uint64_t  TransmitTimestamp;                        // +   8    T3 (server transmit time)
}; 	                                                    // =  48 Bytes

// the ARP frame
struct ARP_Frame_t
{
	IP_EthernetHeader_t     ETH_Header;                 //    14
	uint16_t			    HardwareType;               // +   2
	uint16_t			    Protocol;                   // +   2
	uint8_t				    HardwareAddrLength;         // +   1
	uint8_t				    ProtocolLength;             // +   1
	uint16_t 			    Opcode;                     // +   2
	IP_MAC_Address_t 	    SourceMAC;     		        // +   6
	IP_Address_t			SrcIP_Address;     	        // +   4
	IP_MAC_Address_t 	    DestinationMAC;		        // +   6
	IP_Address_t			DstIP_Address;     	        // +   4
};                                                      // =  42 Bytes

// the DHCP frame
struct DHCP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;                 // +   8
	DHCP_Header_t		    Header;  		            // + 240
};                                                      // = 282 Bytes

// the DNS frame
struct DNS_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t 			IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;                 // +   8
	DNS_Header_t	    	Header;		                // +  12
};                                                      // =  54 Bytes

// the ICMP frame
struct ICMP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t 			IP_Header;                  // +  20
	ICMP_Header_t	    	Header;		                // +   8
};                                                      // =  42 Bytes

struct MQTT_Frame_t
{
    IP_EthernetHeader_t     ETH_Header;                 //    14
    IP_Header_t             IP_Header;                  //    20
    TCP_Header_t            TCP_Header;                 //    20
    // MQTT payload (CONNECT / PUBLISH / SUBSCRIBE / etc.)
    // MQTT messages are variable-length, so we reserve a buffer.
    uint8_t                 MQTT_Data[512]; // Adjust size as needed
};

// the IP frame
struct IP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;			        //    14
	IP_Header_t		    	Header;                     // +  20
};                                                      // =  34 Bytes

// the TCP frame
struct TCP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;			        //    14
	IP_Header_t			    IP_Header;                  // +  20
	TCP_Header_t			Header; 	                // +  20
};                                                      // =  54 Bytes

// the TCP pseudo frame
/*struct TCP_PseudoFrame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	uint8_t 				Dummy[8]; 			        // +   8
	IP_PseudoHeader_t		Header;	       	 		    // +  12
	TCP_Header_t			TCP_Header;		            // +  20
};		                                                // =  54 Bytes
*/

// the UDP frame
struct UDP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;		            // +   8
};                                                      // =  42 Bytes

// the UDP pseudo frame
/*struct UDP_PseudoFrame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	uint8_t 				Dummy[8]; 			        // +   8
	IP_PseudoHeader_t		Header;	       	 		    // +  12
	UDP_Header_t			UDP_Header;		            // +   8
};		                                                // =  42 Bytes
*/

// the DHCP frame
struct SNTP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;                 // +   8
    SNTP_Header_t           SNTP_Header;                // +  48
};                                                      // =  90 Bytes

struct IP_EthernetPacket_t
{
	union
	{
		uint8_t						U8RawData[IP_ETHERNET_FRAME_SIZE];
		uint16_t					U16RawData[IP_ETHERNET_FRAME_SIZE / 2];
		IP_EthernetHeader_t			ETH_Header;
		ARP_Frame_t                 ARP_Frame;
		DHCP_Frame_t                DHCP_Frame;
        DNS_Frame_t                 DNS_Frame;
		ICMP_Frame_t            	ICMP_Frame;
		IP_Frame_t				    IP_Frame;
		MQTT_Frame_t                MQTT_Frame;
        SNTP_Frame_t                SNTP_Frame;
		TCP_Frame_t	                TCP_Frame;
	//TCP_PseudoFrame_t         TCP_PseudoFrame;    // use for TCP Checksum calculation
		UDP_Frame_t				    UDP_Frame;
	//UDP_PseudoFrame_t		    UDP_PseudoFrame;	// use for UDP Checksum calculation
	};
};

struct IP_PacketMsg_t
{
	uint16_t    		    PacketSize;
	IP_EthernetPacket_t*	pPacket;
    uint8_t*                Payload;                // Pointer to UDP payload
    uint16_t                PayloadSize;            // length of payload
};

#pragma pack(pop)

#if (IP_USE_DNS == DEF_ENABLED)
struct DNS_PendingRequest_t
{
    uint16_t        XID;
    DNS_Callback_t  pCallback;
    void*           pContext;
    TickCount_t     TimeStamp;
    bool            Pending;                        // App requested a DNS lookup
};

#endif

struct IP_ETH_Config_t
{
    IP_MAC_Address_t            MAC_Address;
    class ETH_DriverInterface*  pETH_Driver;        // Driver for embedded MAC controller
    class PHY_DriverInterface*  pPHY_Driver;        // Driver for PHY
    uint8_t                     PHY_Address;
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
// Inline function(s)
//-------------------------------------------------------------------------------------------------

inline uint16_t htons(uint16_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return static_cast<uint16_t>(__REV16(x));
  #endif

    // ADD other CPU fast method to reverse 16 Bits order
}

inline uint16_t ntohs(uint16_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return static_cast<uint16_t>(__REV16(x));
  #endif

    // ADD other CPU fast method to reverse 16 Bits order
}

inline uint32_t htonl(uint32_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return __REV(x);
  #endif

    // ADD other CPU fast method to reverse 32 Bits order
}

inline uint32_t ntohl(uint32_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return __REV(x);
  #endif

    // ADD other CPU fast method to reverse 32 Bits order
}


//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


