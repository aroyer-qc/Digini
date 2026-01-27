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

#define IP_ETHERNET_TYPE_ARP 			        0x0806
#define IP_ETHERNET_TYPE_IP  			        0x0800
//#define IP_ETHERNET_TYPE_IP6 			        0x86DD

#define IP_VERSION4_IHL20				        0x45
#define IP_TIME_TO_LIVE					        128
#define IP_BROADCAST_ADDRESS                    0xFFFFFFFF

#define IP_PROTOCOL_ICMP				        0x01
#define IP_PROTOCOL_TCP					        0x06
#define IP_PROTOCOL_UDP					        0x11

#define DHCP_PACKET_SIZE                        576
#define DHCP_OPTION_IN_PACKET_SIZE              308
#define DHCP_HOSTNAME_MAX_LENGTH                63



//#if (IP_USE_DHCP == DEF_DISABLED)
//    #define GetActiveGatewayIP()                GetStaticGatewayIP()
//    #define GetActiveSubnetMask()               GetStaticSubnetMask()
//    #define GetActiveIP()                       GetStaticIP()
//    #define GetActiveDNS_IP()                   GetStaticDNS_IP()
//#endif

//-------------------------------------------------------------------------------------------------
// Macro(s)
//-------------------------------------------------------------------------------------------------

#define IP_ADDRESS                              U32MACRO    // usage: IP_ADDRESS(192,168,0,0);

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


//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef uint32_t    IP_Address_t;
typedef uint16_t    IP_Port_t;
//typedef void        (*ETH_CallBack_t) (uint32_t Event);                         // Pointer to ETH_CallBack function

#pragma pack(push, 1)

struct IP_MAC_Address_t
{
    uint8_t     Byte[IP_MAC_ADDRESS_SIZE];
};

// The Ethernet header
struct IP_EthernetHeader_t
{
	IP_MAC_Address_t 	DestinationMAC;                 //     6
	IP_MAC_Address_t 	SourceMAC;                      // +   6
	uint16_t		    Type;                           // +   2
};                           	    // 14 Bytes

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
	IP_Address_t    SrcIP_Addr;                         // +   4
    IP_Address_t    DstIP_Addr;                         // +   4
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

struct ICMP_Header_t
{
	uint8_t         Type;                               //     1
    uint8_t         Code;                               // +   1
	uint16_t	    Checksum;                           // +   2
    uint16_t        ID;                                 // +   2
    uint16_t 	    Sequence;                           // +   2
};                                                      // =   8 Bytes

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

// the UDP pseudo frame
struct TCP_PseudoFrame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	uint8_t 				Dummy[8]; 			        // +   8
	IP_PseudoHeader_t		Header;	       	 		    // +  12
	TCP_Header_t			TCP_Header;		            // +  20
};		                                                // =  54 Bytes

// the ICMP frame
struct ICMP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t 			IP_Header;                  // +  20
	ICMP_Header_t	    	Header;		                // +   8
};                                                      // =  42 Bytes

// the UDP frame
struct UDP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;		            // +   8
};                                                      // =  42 Bytes

// the UDP pseudo frame
struct UDP_PseudoFrame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	uint8_t 				Dummy[8]; 			        // +   8
	IP_PseudoHeader_t		Header;	       	 		    // +  12
	UDP_Header_t			UDP_Header;		            // +   8
};		                                                // =  42 Bytes

// the DHCP frame
struct DHCP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;                 // +   8
	DHCP_Header_t		    Header;  		            // + 240
};                                                      // = 282 Bytes

struct IP_EthernetPacket_t
{
	union
	{
		uint8_t						U8RawData[IP_ETHERNET_FRAME_SIZE];
		uint16_t					U16RawData[IP_ETHERNET_FRAME_SIZE / 2];
		IP_EthernetHeader_t			ETH_Header;
		IP_Frame_t				    IP_Frame;
		ARP_Frame_t                 ARP_Frame;
		TCP_Frame_t	                TCP_Frame;
		TCP_PseudoFrame_t           TCP_PseudoFrame;    // use for TCP Checksum calculation
		ICMP_Frame_t            	ICMP_Frame;
		UDP_Frame_t				    UDP_Frame;
		UDP_PseudoFrame_t		    UDP_PseudoFrame;	// use for UDP Checksum calculation
		DHCP_Frame_t                DHCP_Frame;
	};
};

struct IP_PacketMsg_t
{
	uint16_t    		    PacketSize;
	IP_EthernetPacket_t*	pPacket;
};

#pragma pack(pop)

struct IP_ETH_Config_t
{
    IP_MAC_Address_t            MAC_Address;
    class ETH_DriverInterface*  pETH_Driver;                                    // Driver for embedded MAC controller
    class PHY_DriverInterface*  pPHY_Driver;                                    // Driver for PHY
    uint8_t                     PHY_Address;
};


struct IP_Config_t                                                              // Host Name, IP_ Address, Protocol (ip_cfg.h)
{
  #if (IP_USE_HOSTNAME == DEF_ENABLED)
    const char*         pHostName;
  #endif
    nOS_Stack*          pStack;
    uint16_t            ProtocolFlag;
    IP_Address_t        DefaultStatic_IP;         // check in context
    IP_Address_t        DefaultGateway;
    IP_Address_t        DefaultSubnetMask;
    IP_Address_t        DefaultStaticDNS;
    IP_ETH_Config_t     IP_ETH_Config;
};

// EMAC Driver Control Information
struct ETH_Control_t
{
 //   ETH_CallBack_t          pCallBack;            // Signal Event callback
    uint8_t                 TX_HeadIndex;           // Used by SendTX_Packet
    uint8_t                 TX_TailIndex;           // Used by ISR_CallBack
    uint8_t                 RX_Index;               // Receive descriptor index
  #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
    uint8_t                 TX_TS_Index;            // Transmit Timestamps descriptor index
  #endif
    uint8_t*                FrameEnd;               // End of assembled frame fragments
};

struct IP_Message_t
{
    // TODO missing stuff
};

// Ethernet Link Info
struct ETH_LinkInfo_t
{
    ETH_LinkSpeed_e     Speed;                      // Link speed: 0 = 10 MBit, 1 = 100 MBit, 2 = 1 GBit
    ETH_Duplex_e        Duplex;                     // Duplex mode: 0 = Half, 1 = Full
};

struct  ETH_MacTime_t
{
    uint32_t naneSecond;                         // Nano seconds
    uint32_t Second;                             // Seconds
};

//-------------------------------------------------------------------------------------------------
// Inline function(s)
//-------------------------------------------------------------------------------------------------

// This is for ARM

inline uint16_t htons(uint16_t x)
{
    return static_cast<uint16_t>(__REV16(x));
}

inline uint16_t ntohs(uint16_t x)
{
    return static_cast<uint16_t>(__REV16(x));
}

inline uint32_t htonl(uint32_t x)
{
    return __REV(x);
}

inline uint32_t ntohl(uint32_t x)
{
    return __REV(x);
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


