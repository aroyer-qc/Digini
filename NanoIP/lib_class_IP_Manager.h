//-------------------------------------------------------------------------------------------------
//
//  File : lib_Class_IP_Manager.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2009-2024 Alain Royer.
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

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define IP_MAX_URL_SIZE                         128

#define IP_STREAM_SOCKET                        0
#define IP_DNS_SOCKET                           2
#define IP_DHCP_SOCKET                          3
#define IP_SNTP_SOCKET                          3

// Message type for struct MSG_t
#define IP_MSG_TYPE_IP_MANAGEMENT               0
#define IP_MSG_TYPE_DHCP_MANAGEMENT             1
#define IP_MSG_TYPE_SNTP_MANAGEMENT             2

//-------------------------------------------------------------------------------------------------
// Macro(s)
//-------------------------------------------------------------------------------------------------

#define IP_Address                                U32MACRO    // usage: IP_ADDRESS(192,168,0,0);
#define IP_IP_PRINTF(IP)                       

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef     IP_Address_t       uint32_t
typedef     IP_Port_t          uint16_t

typedef union
{
    uint8_t  ba[6];
} MAC_Address_t;

typedef union
{
    struct
    {
        int16_t   DNS_IP_Found     :1;
        int16_t   SNTP_Fail        :1;
        int16_t   bFree03          :1;
        int16_t   bFree04          :1;
        int16_t   bFree05          :1;
        int16_t   bFree06          :1;
        int16_t   bFree07          :1;

        int16_t   bFree08          :1;
        int16_t   bFree09          :1;
        int16_t   bFree10          :1;
        int16_t   bFree11          :1;
        int16_t   bFree12          :1;
        int16_t   bFree13          :1;
        int16_t   bFree14          :1;
        int16_t   Status           :1;                     // Do not use globally
    }b;
    int16_t w;
} IP_StatusFlag_t;

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class IP_Manager
{
    public:
    
        void                Initialize                  (void);
        void                Run                         (void);

        IP_Address_t        GetDNS                      (void);
        IP_Address_t        GetHost                     (void);
        uint8_t*            IP_ToAscii                  (IP_Address_t IP_Address);
        IP_Address_t        AsciiToIP                   (uint8_t* pBuffer);
        uint8_t*            ProcessURL                  (uint8_t* pBuffer, IP_Address_t* pIP_Address, IP_Port_t* pPort);

        
    private:    
 
    
        nOS_Queue           m_MsgQueue;
        uint8_t             m_GetQueueArray[1024];                  // need to define this it was lost somewhere
            
        IP_Flag_t           m_Flag;                                 // Configuration of IP Stack
        IP_StatusFlag_t     m_Status;                               // Dynamic Flag od IP Stack
        MAC_Address_t       m_MAC;                                  // MAC Address of the module
        
        IP_Address_t        m_StaticGatewayIP;                      // Gateway IP Address
        IP_Address_t        m_StaticSubnetMask;                     // Subnet Mask
        IP_Address_t        m_StaticIP;                             // Static IP Address
        IP_Address_t        m_StaticDNS_IP;                         // Static DNS Server IP Address
        
        uint8_t             m_TX_SocketMemorySize;                  // TX Socket Memory Configuration for all x Sockets
        uint8_t             m_RX_SocketMemorySize;                  // RX Socket Memory Configuration for all x Sockets

      #if (IP_USE_ARP == DEF_ENABLED)
        NetARP              m_ARP;                                  // Address Resolution Protocol
      #endif

      #if (IP_USE_DHCP == DEF_ENABLED)
        NetDHCP             m_DHCP;                                 // Dynamic Host Control Protocol. Need UDP

        // Move this to DHCP    use getter!!
        IP_Address_t        m_DHCP_GatewayIP;                       // Gateway IP Address from server
        IP_Address_t        m_DHCP_SubnetMask;                      // Subnet Mask from server
        IP_Address_t        m_DHCP_IP;                              // IP Address from server
        IP_Address_t        m_DHCP_DNS_IP;                          // DNS Server IP Address from server
        bool                m_IP_IsValid;
      #endif

      #if (IP_USE_ICMP == DEF_ENABLED)
        NetICMP             m_ICMP;                                 // Internet Control Message Protocol
      #endif

      #if (IP_USE_NTP == DEF_ENABLED)
        NetNTP              m_NTP;                                  // Network Time Protocol
    
        uint8_t             m_NTP_Server_1[IP_MAX_URL_SIZE];// move this to NTP
        uint8_t             m_NTP_Server_2[IP_MAX_URL_SIZE];
      #endif

      #if (IP_USE_SNTP == DEF_ENABLED)
        NetSNTP             m_SNTP;                                 // Simple Network Transport Protocol
      #endif

      #if (IP_USE_SOAP == DEF_ENABLED)
        NetSOAP             m_pSOAP                                 // Simple Object Access Protocol
      
        uint8_t             m_SOAP_Server_1[IP_MAX_URL_SIZE];       // Messaging protocol specification for exchanging structured information.
        uint8_t             m_SOAP_Server_2[IP_MAX_URL_SIZE];
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        NetTCP          m_TCP;                                      // Transport Control Protocol
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        NetUDP          m_UDP;                                      // User Datagram Protocol
      #endif

        
};

//-------------------------------------------------------------------------------------------------

