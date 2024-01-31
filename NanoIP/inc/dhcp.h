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

#define DHCP_PACKET_SIZE                        328
#define DHCP_OPTION_IN_PACKET_SIZE              308

#define DHCP_OPTION_PADDING                     0x00
#define DHCP_OPTION_END_OF_FIELD                0xFF
#define DHCP_OPTION_SUBNET_MASK                 0x01
#define DHCP_OPTION_GATEWAY                     0x03
#define DHCP_OPTION_DNS_SERVER                  0x06
#define DHCP_OPTION_HOST_NAME                   0x0C
#define DHCP_OPTION_CLIENT_IP                   0x32
#define DHCP_OPTION_LEASE_TIME                  0x33
#define DHCP_OPTION_MESSAGE_TYPE                0x35
#define DHCP_OPTION_SERVER_IP                   0x36
#define DHCP_OPTION_PARAMETER_LIST              0x37
#define DHCP_OPTION_CLIENT_IDENTIFIER           0x3D

#define DHCP_MESSAGE_TYPE_DISCOVER              0x01

#define DHCP_BOOT_REQUEST                       0x01
#define DHCP_BOOT_REPLY                         0x02

#define DHCP_SERVER_PORT                        htons(67)
#define DHCP_CLIENT_PORT                        htons(68)

#define DHCP_HARWARE_TYPE_ETHERNET_10           0x01
#define DHCP_HARDWARE_TYPE_ETHERNET_100         0x02
#define DHCP_HARDWARE_ADDRESS_LENGHT            0x06

#define DHCP_MAGIC_COOKIE                       0x63538263      // 0x63825363 in Big Endian
#define DHCP_FLAGS_BROADCAST                    0x8000

#define DHCP_PUT_OPTION_CLIENT_IDENTIFIER       (uint8_t)0x01
#define DHCP_PUT_OPTION_HOST_NAME               (uint8_t)0x02
#define DHCP_PUT_OPTION_PL_DISCOVER             (uint8_t)0x04
#define DHCP_PUT_OPTION_PL_REQUEST              (uint8_t)0x08
#define DHCP_PUT_OPTION_REQUESTED_CLIENT_IP     (uint8_t)0x10
#define DHCP_PUT_OPTION_SERVER_IP               (uint8_t)0x20

#define DHCP_TIMEOUT                            10              // 10 second before a retry at DHCP

// Action Type for struct MSG_t for MSG_TYPE_DHCP_MANAGEMENT
#define DHCP_MSG_ACTION_LEASE_RENEWAL           1
#define DHCP_MSG_ACTION_REBIND                  2
#define DHCP_MSG_ACTION_TIME_OUT                3

#define DHCP_IS_ON                              true
#define DHCP_IS_OFF                             false


//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

enum DHCP_State_e
{
    DHCP_STATE_INITIAL          = 0,
    DHCP_STATE_DISCOVER         = 1,
    DHCP_STATE_OFFER_RECEIVED   = 2,
    DHCP_STATE_BOUND            = 3,
};

enum DHCP_OptionType_e
{
    DHCP_OPTION_DISCOVER = 1,
    DHCP_OPTION_OFFER    = 2,
    DHCP_OPTION_REQUEST  = 3,
    DHCP_OPTION_DECLINE  = 4,        // not used   we don't declie an offer 
    DHCP_OPTION_ACK      = 5,
    DHCP_OPTION_NACK     = 6,
    DHCP_OPTION_RELEASE  = 7,        // not used???
};


struct DHCP_Options_t
{
    DHCP_OptionType_e   Type;
    IP_Address_t        GatewayIP;
    IP_Address_t        SubnetMaskIP;
    IP_Address_t        DNS_ServerIP;
    IP_Address_t        ClientIP;
    IP_Address_t        ServerIP;
    /* TickCount ? */uint32_t            LeaseTime;
};

struct DHCP_Msg_t
{
    uint8_t      Op;
    uint8_t      H_Type;
    uint8_t      H_Length;
    uint8_t      Hops;
    uint32_t     X_ID;
    uint16_t     Secs;
    uint16_t     Flags;
    IP_Address_t ClientIP_Address;
    IP_Address_t YourIP_Address;
    IP_Address_t ServerIP_Address;
    IP_Address_t RelayAgentIP_Address;
    uint8_t      ClientHardware[16];
    uint8_t      Sname[64];
    uint8_t      File[128];
    uint32_t     MagicCookie;
    uint8_t      Options[DHCP_OPTION_IN_PACKET_SIZE];
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

class NetDHCP
{
    public:
    
        void            Initialize      (void* pQ);
        bool            Process         (DHCP_Msg_t* pMsg);

        void            SetMode         (bool Mode)                 { m_Mode = Mode; }         
        bool            GetMode         (void)                      { return m_Mode; }
        
    private:    


        bool            Start           (void);
        void            ParseOffer      (DHCP_Msg_t* pRX);
        void            IsBound         (void);
        void            ParseOption     (DHCP_Msg_t* pRX);
        size_t          PutOption       (uint8_t* Array, uint8_t Option, uint8_t Message);
        void            PutHeader       (DHCP_Msg_t* pTX);
        bool            Discover        (void);
        bool            Request         (void);

        uint32_t                m_Xid;
        DHCP_Options_t          m_Options;
        
        // One shot timer for DHCP transaction time out
        nOS_Timer               m_TimerDiscover;
        nOS_Timer               m_TimerT1_Lease;
        nOS_Timer               m_TimerT2_Rebind;
        
        nOS_Queue               m_pQ; ??
        bool                    m_Mode;                      // External configuration can tell this class the DHCP is OFF or ON
        DHCP_State_e            m_State;


        static const uint8_t    m_OPL_Discover[8];
        static const uint8_t    m_OPL_Request[10];
        
        IP_Address_t            m_DHCP_GatewayIP;                       // Gateway IP Address from server
        IP_Address_t            m_DHCP_SubnetMask;                      // Subnet Mask from server
        IP_Address_t            m_DHCP_IP;                              // IP Address from server
        IP_Address_t            m_DHCP_DNS_IP;                          // DNS Server IP Address from server
        bool                    m_IP_IsValid;
};

//-------------------------------------------------------------------------------------------------

