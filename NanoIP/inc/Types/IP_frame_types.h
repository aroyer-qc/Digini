//-------------------------------------------------------------------------------------------------
//
//  File : IP_frame_types.h
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
// Typedef(s)
//-------------------------------------------------------------------------------------------------

#pragma pack(push, 1)
// The Ethernet header
struct IP_EthernetHeader_t
{
	IP_MAC_Address_t 	DestinationMAC;                 //     6
	IP_MAC_Address_t 	SourceMAC;                      // +   6
	uint16_t		    Type;                           // +   2
};                           	                        // 14 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
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
#pragma pack(pop)

#pragma pack(push, 1)
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
#pragma pack(pop)

#pragma pack(push, 1)
struct ICMP_Header_t
{
	uint8_t         Type;                               //     1
    uint8_t         Code;                               // +   1
	uint16_t	    Checksum;                           // +   2
    uint16_t        ID;                                 // +   2
    uint16_t 	    Sequence;                           // +   2
};                                                      // =   8 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
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
#pragma pack(pop)

#pragma pack(push, 1)
struct OptionData_t
{
    uint8_t  Kind;     // always 2 for MSS
    uint8_t  Length;   // always 4
    uint16_t MSS;      // htons(TCP_MSS)
};
#pragma pack(pop)

#pragma pack(push, 1)
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
	OptionData_t    OptionData;                         // +   4
};                              	                    // =  24 with option data
#pragma pack(pop)

#pragma pack(push, 1)
struct UDP_Header_t
{
	IP_Port_t 	    SrcPort;                            //     2
	IP_Port_t 	    DstPort;                            // +   2
	uint16_t	    Length;                             // +   2
	uint16_t	    Checksum;                           // +   2
};                                                      // =   8 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
struct IP_PseudoHeader_t                                // note that the element are not in same order as define in the protocol,
{                                                       // but match the IP in the union
	uint8_t         Zero_s;                             //     1
	uint8_t         Protocol;                           // +   1
    uint16_t        Length;                             // +   2
	IP_Address_t  	SrcIP;                              // +   4
	IP_Address_t  	DstIP;                              // +   4
}; 	                                                    // =  12 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
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
#pragma pack(pop)

#pragma pack(push, 1)
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
#pragma pack(pop)

#pragma pack(push, 1)
struct DHCP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;                 // +   8
	DHCP_Header_t		    Header;  		            // + 240
};                                                      // = 282 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
struct DNS_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t 			IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;                 // +   8
	DNS_Header_t	    	Header;		                // +  12
};                                                      // =  54 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
struct ICMP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t 			IP_Header;                  // +  20
	ICMP_Header_t	    	Header;		                // +   8
};                                                      // =  42 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
struct MQTT_Frame_t
{
    IP_EthernetHeader_t     ETH_Header;                 //    14
    IP_Header_t             IP_Header;                  //    20
    TCP_Header_t            TCP_Header;                 //    20
    // MQTT payload (CONNECT / PUBLISH / SUBSCRIBE / etc.)
    // MQTT messages are variable-length, so we reserve a buffer.
    uint8_t                 MQTT_Data[512]; // Adjust size as needed
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;			        //    14
	IP_Header_t		    	Header;                     // +  20
};                                                      // =  34 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
struct TCP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;			        //    14
	IP_Header_t			    IP_Header;                  // +  20
	TCP_Header_t			Header; 	                // +  20
};                                                      // =  54 Bytes
#pragma pack(pop)

//struct TCP_PseudoFrame_t
//{
//	IP_EthernetHeader_t 	ETH_Header;                 //    14
//	uint8_t 				Dummy[8]; 			        // +   8
//	IP_PseudoHeader_t		Header;	       	 		    // +  12
//	TCP_Header_t			TCP_Header;		            // +  20
//};		                                                // =  54 Bytes

#pragma pack(push, 1)
struct UDP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;		            // +   8
};                                                      // =  42 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
struct PseudoHeader_t
{
	IP_Address_t    SrcIP;                              //     4
	IP_Address_t    DstIP;                              // +   4
	uint8_t         Zero;                               // +   1
	uint8_t         Protocol;                           // +   1
	uint16_t	    Length;                             // +   2
};		                                                // =  12 Bytes
#pragma pack(pop)


#pragma pack(push, 1)
struct SNTP_Frame_t
{
	IP_EthernetHeader_t 	ETH_Header;                 //    14
	IP_Header_t			    IP_Header;                  // +  20
	UDP_Header_t			UDP_Header;                 // +   8
    SNTP_Header_t           SNTP_Header;                // +  48
};                                                      // =  90 Bytes
#pragma pack(pop)

#pragma pack(push, 1)
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
//        TCP_PseudoFrame_t         TCP_PseudoFrame;    // use for TCP Checksum calculation
		UDP_Frame_t				    UDP_Frame;
	//UDP_PseudoFrame_t		    UDP_PseudoFrame;	// use for UDP Checksum calculation
	};
};
#pragma pack(pop)

struct IP_PacketMsg_t
{
	uint16_t    		    PacketSize;
	IP_EthernetPacket_t*	pPacket;
    uint8_t*                Payload;                // Pointer to UDP payload
    uint16_t                PayloadSize;            // length of payload
	uint8_t                 Protocol;               // IP protocol (TCP=6, UDP=17, ICMP=1, etc.)
};

struct TCP_TX_Segment_t
{
    uint8_t*        pPayload;
    bool            InUse;
    uint32_t        SeqStart;
    uint32_t        SeqEnd;
    uint8_t         Flags;
    uint16_t        Window;
    size_t          Length;
    TickCount_t     TimeStamp;
    uint8_t         RetryCount;
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
