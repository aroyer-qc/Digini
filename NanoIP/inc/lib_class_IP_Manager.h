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
// etc...

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

#define IP_ADDRESS                              U32MACRO    // usage: IP_ADDRESS(192,168,0,0);
#define IP_IP_PRINTF(IP)                       

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef IP_Address_t       uint32_t
typedef IP_Port_t          uint16_t

enum IP_EthernetIF_e 
{ 
    ETH_INTERFACE_ENUM
};

struct IP_MAC_Address_t
{
    uint8_t  Address[IP_MAC_ADDRESS_SIZE];
    
  #if (IP_USE_IP_V6 == DEF_ENABLED)             // Future :)
    uint16_t  AddressV6[IP_MAC_V6_ADDRESS_SIZE];
  #endif 
    
};

struct IP_ETH_Config_t
{
  #if (IP_USE_HOSTNAME == DEF_ENABLED)
    char*               HostName;
  #endif

    uint16_t            ProtocolFlag;

    IP_Address_t        DefaultStatic_IP;
    IP_Address_t        DefaultGateway;
    IP_Address_t        DefaultSubnetMask;
    IP_Address_t        DefaultStaticDNS;
    
  #if IP_INTERFACE_SUPPORT_PHY == DEF_ENABLED
    PHY_Driver*         pPHY_Driver;                                    // Driver for PHY
    ETH_Driver*         pETH_Driver;                                    // Driver for embedded MAC controller
  #endif

  #if IP_INTERFACE_SUPPORT_HEC == DEF_ENABLED
     HEC_Driver         pHEC_Driver;                                    // Driver for HEC (Hardwired Embedded Controller)
  #endif

  #if IP_INTERFACE_SUPPORT_MAC == DEF_ENABLED
    MAC_Driver*         pMAC_Driver;                                    // Driver for ethernal MAC and PHY chip
  #endif
};

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class IP_Manager
{
    public:
    
        void                Initialize                  (IP_EthernetIF_e IP_EthernetIF, IP_MAC_Address_t* pMAC_Address);
        void                Run                         (void);

        IP_Address_t        GetDNS                      (void);
        IP_Address_t        GetHost                     (void);
        
        uint8_t*            IP_ToAscii                  (IP_Address_t IP_Address);
        IP_Address_t        AsciiToIP                   (uint8_t* pBuffer);
        uint8_t*            ProcessURL                  (uint8_t* pBuffer, IP_Address_t* pIP_Address, IP_Port_t* pPort);

        
    private:    
 
    
        //IP_Flag_t                       m_Flag;                                 // Configuration of IP Stack
        IP_StatusFlag_t                 m_Status;                               // Dynamic Flag od IP Stack
 
        IP_MAC_Address_t                m_MAC_Address;
        IP_Address_t                    m_StaticGatewayIP;                      // Gateway IP Address
        IP_Address_t                    m_StaticSubnetMask;                     // Subnet Mask
        IP_Address_t                    m_StaticIP;                             // Static IP Address
        IP_Address_t                    m_StaticDNS_IP;                         // Static DNS Server IP Address
        
        uint8_t                         m_TX_SocketMemorySize;                  // TX Socket Memory Configuration for all x Sockets
        uint8_t                         m_RX_SocketMemorySize;                  // RX Socket Memory Configuration for all x Sockets

        uint16_t                        m_MTU;

      #if (IP_USE_ARP == DEF_ENABLED)
        NetARP*                         m_pARP;                                 // Address Resolution Protocol
      #endif

      #if (IP_USE_DHCP == DEF_ENABLED)
        NetDHCP*                        m_pDHCP;                                // Dynamic Host Control Protocol. Need UDP
      #endif

      #if (IP_USE_ICMP == DEF_ENABLED)
        NetICMP*                        m_pICMP;                                // Internet Control Message Protocol
      #endif

      #if (IP_USE_NTP == DEF_ENABLED)
        NetNTP*                         m_pNTP;                                 // Network Time Protocol
    
        uint8_t                         m_NTP_Server_1[IP_MAX_URL_SIZE];// move this to NTP
        uint8_t                         m_NTP_Server_2[IP_MAX_URL_SIZE];
      #endif

      #if (IP_USE_SNTP == DEF_ENABLED)
        NetSNTP*                        m_pSNTP;                                // Simple Network Transport Protocol
      #endif

      #if (IP_USE_SOAP == DEF_ENABLED)
        NetSOAP*                        m_pSOAP                                 // Simple Object Access Protocol
      
        uint8_t                         m_SOAP_Server_1[IP_MAX_URL_SIZE];       // Messaging protocol specification for exchanging structured information.
        uint8_t                         m_SOAP_Server_2[IP_MAX_URL_SIZE];
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        NetTCP*                         m_pTCP;                                 // Transport Control Protocol
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        NetUDP*                         m_pUDP;                                 // User Datagram Protocol
      #endif

        IP_ETH_Config_t*                m_pEthernetIF;                          // Ethernet Configuration
        static const IP_ETH_Config_t    m_EthernetIF[IP_NUMBER_OF_INTERFACE];
};

//-------------------------------------------------------------------------------------------------

