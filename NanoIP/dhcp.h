//-------------------------------------------------------------------------------------------------
//
//  File :  dhcp.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2010-2024 Alain Royer.
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

#ifdef DHCP_GLOBAL
    #define DHCP_EXTERN
    #define DHCP_PRIVATE
#else
    #define DHCP_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DHCP_PACKET_SIZE                    	328
#define DHCP_OPTION_IN_PACKET_SIZE          	308

#define DHCP_STATE_INITIAL                  	0
#define DHCP_STATE_DISCOVER                 	1
#define DHCP_STATE_OFFER_RECEIVED           	2
#define DHCP_STATE_BOUND                    	3

#define DHCP_DISCOVER                       	1
#define DHCP_OFFER                          	2
#define DHCP_REQUEST                        	3
#define DHCP_DECLINE                        	4
#define DHCP_ACK                            	5
#define DHCP_NACK                           	6
#define DHCP_RELEASE                        	7

#define DHCP_OPTION_PADDING                 	0x00
#define DHCP_OPTION_END_OF_FIELD            	0xFF
#define DHCP_OPTION_SUBNET_MASK             	0x01
#define DHCP_OPTION_GATEWAY                 	0x03
#define DHCP_OPTION_DNS_SERVER              	0x06
#define DHCP_OPTION_HOST_NAME               	0x0C
#define DHCP_OPTION_CLIENT_IP               	0x32
#define DHCP_OPTION_LEASE_TIME              	0x33
#define DHCP_OPTION_MESSAGE_TYPE            	0x35
#define DHCP_OPTION_SERVER_IP               	0x36
#define DHCP_OPTION_PARAMETER_LIST          	0x37
#define DHCP_OPTION_CLIENT_IDENTIFIER       	0x3D

#define DHCP_MESSAGE_TYPE_DISCOVER          	0x01

#define DHCP_BOOT_REQUEST                   	0x01
#define DHCP_BOOT_REPLY                     	0x02

#define DHCP_SERVER_PORT                    	htons(67)
#define DHCP_CLIENT_PORT                    	htons(68)

#define DHCP_HARWARE_TYPE_ETHERNET_10       	0x01
#define DHCP_HARWARE_TYPE_ETHERNET_100      	0x02
#define DHCP_HARWARE_ADDRESS_LENGHT         	0x06

#define DHCP_MAGIC_COOKIE                   	0x63538263      // 0x63825363 in Big Endian
#define DHCP_FLAGS_BROADCAST                	0x8000


#define DHCP_PUT_OPTION_CLIENT_IDENTIFIER   	(uint8_t)0x01
#define DHCP_PUT_OPTION_HOST_NAME           	(uint8_t)0x02
#define DHCP_PUT_OPTION_PL_DISCOVER         	(uint8_t)0x04
#define DHCP_PUT_OPTION_PL_REQUEST          	(uint8_t)0x08
#define DHCP_PUT_OPTION_REQUESTED_CLIENT_IP 	(uint8_t)0x10
#define DHCP_PUT_OPTION_SERVER_IP           	(uint8_t)0x20

#define DHCP_OST_TIMEOUT                    	10              // 10 second before a retry at DHCP

// Action Type for struct MSG_t for MSG_TYPE_DHCP_MANAGEMENT
#define DHCP_MSG_ACTION_LEASE_RENEWAL       	1
#define DHCP_MSG_ACTION_REBIND              	2
#define DHCP_MSG_ACTION_TIME_OUT            	3

//-------------------------------------------------------------------------------------------------
// macro(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef struct
{
    uint8_t    Type;
    int32_t   dwGatewayIP;
    int32_t   dwSubnetMaskIP;
    int32_t   dwDNS_ServerIP;
    int32_t   dwClientIP;
    int32_t   dwServerIP;
    int32_t   dwLeaseTime;
} DHCP_Options_t;

typedef struct
{
    uint8_t    Op;
    uint8_t    Htype;
    uint8_t    Hlen;
    uint8_t    Hops;
    int32_t   dwX_ID;
    int16_t    wSecs;
    int16_t    wFlags;
    int32_t   dwClientIP_Addr;
    int32_t   dwYourIP_Addr;
    int32_t   dwServerIP_Addr;
    int32_t   dwRelayAgentIP_Addr;
    uint8_t    baClientHardware[16];
    uint8_t    baSname[64];
    uint8_t    baFile[128];
    uint32_t  MagicCookie;
    uint8_t    baOptions[DHCP_OPTION_IN_PACKET_SIZE];
} DHCP_Msg_t;

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef DHCP_GLOBAL
    const uint8_t DHCP_baOPL_Discover[8] = {55,        // Parameter list
                                         6,         // Size
                                         1,         // Subnet Mask
                                         3,         // Gateway
                                         6,         // DNS Server
                                         15,        // Domain Name
                                         58,        // DHCP T1 Value
                                         59};       // DHCP T2 Value

    const uint8_t DHCP_baOPL_Request[10] = {55,        // Parameter list
                                          8,        // Size
                                          1,        // Subnet Mask
                                          3,        // Gateway
                                          6,        // DNS Server
                                          15,       // Domain Name
                                          58,       // DHCP T1 Value
                                          59,       // DHCP T2 Value
                                          31,       // Perform Router Discovery
                                          33};      // Static Route
#else
    extern const uint8_t DHCP_baOPL_Discover[8];
    extern const uint8_t DHCP_baOPL_Request[10];
#endif

//-------------------------------------------------------------------------------------------------
// Global variable(s), constant(s) and Private(s)
//-------------------------------------------------------------------------------------------------

move private into c or cpp file
#ifdef DHCP_PRIVATE

static      uint32_t               DHCP_Xid;
static      DHCP_Options_t         DHCP_Options;
static      uint8_t                DHCP_byOST_Discover    = TIME_TIMER_nullptr;
static      uint8_t                DHCP_byOST_T1_Lease    = TIME_TIMER_nullptr;
static      uint8_t                DHCP_byOST_T2_Rebind   = TIME_TIMER_nullptr;
static      OS_EVENT*              DHCP_pQ;

#endif

DHCP_EXTERN uint8_t                DHCP_byState;

//-------------------------------------------------------------------------------------------------
// Private Function prototype(s)
//-------------------------------------------------------------------------------------------------

#ifdef DHCP_PRIVATE

bool     DHCP_Start          (void);
void     DHCP_ParseOffer     (DHCP_Msg_t* pRX);
void     DHCP_IsBound        (void);
void     DHCP_ParseOption    (DHCP_Msg_t* pRX);
uint16_t DHCP_PutOption      (uint8_t* baArray, uint8_t Option, uint8_t Message);
void     DHCP_PutHeader      (DHCP_Msg_t* pTX);
bool     DHCP_Discover       (void);
bool     DHCP_Request        (void);

#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    DHCP_Init           (void* pQ);
bool    DHCP_Process        (MSG_t* pMsg);

//-------------------------------------------------------------------------------------------------

