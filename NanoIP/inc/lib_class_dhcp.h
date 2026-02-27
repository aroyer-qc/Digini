//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_dhcp.h
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

#if (IP_USE_DHCP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DHCP_HARDWARE_TYPE_ETHERNET             1

#define DHCP_OPTION_PADDING                     0x00
#define DHCP_OPTION_END_OF_FIELD                0xFF
#define DHCP_OPTION_SUBNET_MASK                 0x01
#define DHCP_OPTION_GATEWAY                     0x03
#define DHCP_OPTION_DNS_SERVER                  0x06
#define DHCP_OPTION_HOST_NAME                   0x0C        // 12
#define DHCP_OPTION_CLIENT_IP                   0x32        // 50
#define DHCP_OPTION_LEASE_TIME                  0x33        // 51 not handle
#define DHCP_OPTION_MESSAGE_TYPE                0x35        // 53
#define DHCP_OPTION_SERVER_IP                   0x36        // 54
#define DHCP_OPTION_PARAMETER_LIST              0x37        // 55
#define DHCP_OPTION_VENDOR_CLASS                0x3C        // 60
#define DHCP_OPTION_CLIENT_IDENTIFIER           0x3D        // 61

#define DHCP_MESSAGE_TYPE_DISCOVER              0x01

#define DHCP_BOOT_REQUEST                       0x01
#define DHCP_BOOT_REPLY                         0x02

#define DHCP_SERVER_PORT                        67
#define DHCP_CLIENT_PORT                        68

#define DHCP_HARDWARE_ADDRESS_LENGTH            0x06

#define DHCP_MAGIC_COOKIE                       HTONL(0x63825363)
#define DHCP_FLAGS_BROADCAST                    HTONS(0x8000)

#define DHCP_PUT_OPTION_HOST_NAME               (uint8_t)0x01
#define DHCP_PUT_OPTION_REQUESTED_CLIENT_IP     (uint8_t)0x02
#define DHCP_PUT_OPTION_CLIENT_IDENTIFIER       (uint8_t)0x10
#define DHCP_PUT_OPTION_PL_DISCOVER             (uint8_t)0x04
#define DHCP_PUT_OPTION_PL_REQUEST              (uint8_t)0x08
#define DHCP_PUT_OPTION_SERVER_IP               (uint8_t)0x20
#define DHCP_PUT_OPTION_VENDOR_CLASS            (uint8_t)0x40

#define DHCP_DISCOVER_TIME_OUT                  uint32_t(4 * TIME_TICKS_PER_SECOND)
#define DHCP_T1_LEASE_TIME_OUT                  uint32_t(12 * TIME_TICKS_PER_HOUR)
#define DHCP_T2_REBIND_TIME_OUT                 uint32_t(21 * TIME_TICKS_PER_HOUR)
#define DHCP_TIMEOUT                            10              // 10 second before a retry at DHCP

#define DHCP_IS_ON                              true
#define DHCP_IS_OFF                             false

#define DHCP_HEADER_SIZE                        240

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum DHCP_State_e
{
    DHCP_STATE_INITIAL,
    DHCP_STATE_SELECTING,
    DHCP_STATE_REQUESTING,
    DHCP_STATE_BOUND,
    DHCP_STATE_RENEWING,
    DHCP_STATE_REBINDING,
};

enum DHCP_OptionType_e
{
    DHCP_OPTION_UNKNOWN  = 0,
    DHCP_OPTION_DISCOVER = 1,
    DHCP_OPTION_OFFER    = 2,
    DHCP_OPTION_REQUEST  = 3,
    DHCP_OPTION_DECLINE  = 4,        // not used   we don't decline an offer
    DHCP_OPTION_ACK      = 5,
    DHCP_OPTION_NACK     = 6,
    DHCP_OPTION_RELEASE  = 7,        // not used???
    DHCP_OPTION_INFORM   = 8        // not used???
};

// DHCP Message Actions
enum  DHCP_MsgAction_e
{
    DHCP_MSG_ACTION_TIME_OUT,
    DHCP_MSG_ACTION_LEASE_RENEWAL,
    DHCP_MSG_ACTION_REBIND
};

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

#pragma pack(push, 1)
struct DHCP_Msg_t
{
    uint8_t      Op;
    uint8_t      H_Type;
    uint8_t      H_Length;
    uint8_t      Hops;

    uint32_t     X_ID;
    uint16_t     Seconds;
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
#pragma pack(pop)

struct DHCP_Options_t
{
    IP_Address_t    ClientIP;
    IP_Address_t    ServerIP;
    IP_Address_t    SubnetMaskIP;
    IP_Address_t    GatewayIP;
    IP_Address_t    DNS_ServerIP;
    uint32_t        LeaseTime;
    uint8_t         Type;
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class DHCPv4_Manager
{
    public:

        void            Initialize      (NetworkContext& Context);
        bool            Process         (void);

        bool            GetMode         (void)                      { return m_Mode;       }
        IP_Address_t    GetIP           (void)                      { return m_IP;         }
        IP_Address_t    GetGatewayIP    (void)                      { return m_GatewayIP;  }
        IP_Address_t    GetSubnetMask   (void)                      { return m_SubnetMask; }
        IP_Address_t    GetServerIP     (void)                      { return m_ServerIP;   }
        IP_Address_t    GetDNS_IP       (void)                      { return m_DNS_IP;     }
        bool            IsItEnabled     (void)                      { return m_Enabled;    }

        void            SetEnabled      (bool State)                { m_Enabled = State;   }
        void            SetMode         (bool Mode)                 { m_Mode = Mode;       }

        void            Reset           (void);
        bool            Start           (void);

    private:

        void            ParseOffer      (DHCP_Msg_t* pRX);
        void            IsBound         (void);
        void            ParseOption     (DHCP_Msg_t* pRX);
        size_t          PutOption       (uint8_t* Array, uint8_t Option, uint8_t Message);
        void            PutHeader       (DHCP_Msg_t* pTX);
        bool            Discover        (void);
        bool            Request         (void);

        NetworkContext*         m_pContext;
        Socket*                 m_pSocket;                      // Socket UDP pour DHCP
        uint32_t                m_XID;
        DHCP_Options_t          m_Options;
       // One shot timer for DHCP transaction time out
        nOS_Timer               m_TimerDiscover;
        nOS_Timer               m_TimerT1_Lease;
        nOS_Timer               m_TimerT2_Rebind;
        bool                    m_Mode;
        DHCP_State_e            m_State;

        bool                    m_Enabled;
        IP_Address_t            m_GatewayIP;                    // Gateway IP Address from server
        IP_Address_t            m_SubnetMask;                   // Subnet Mask from server
        IP_Address_t            m_ServerIP;                     // Server IP
        IP_Address_t            m_IP;                           // IP Address from server
        IP_Address_t            m_DNS_IP;                       // DNS Server IP Address from server

        static const uint8_t    m_OPL_Discover[8];
        static const uint8_t    m_OPL_Request[10];
};

//-------------------------------------------------------------------------------------------------

#else // (IP_USE_DHCP == DEF_DISABLED)
    
    #define GetActiveGatewayIP()                GetStaticGatewayIP()
    #define GetActiveSubnetMask()               GetStaticSubnetMask()
    #define GetActiveIP()                       GetStaticIP()
    #define GetActiveDNS_IP()                   GetStaticDNS_IP()

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_DHCP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
