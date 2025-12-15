//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_IP_Manager.h
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
//
//   Best Order of Ethernet Protocol Development
//   - Ethernet MAC/PHY drivers
//   - Implement low-level drivers for the Ethernet controller.
//   - Handle DMA, interrupts, and basic frame TX/RX.
//   - Frame parsing and validation
//   - CRC checks, length/type validation, MAC address filtering.
//   - ARP (Address Resolution Protocol)
//       - Map IP addresses to MAC addresses.
//       - Maintain ARP cache with timeout/refresh logic.
//       - IPv4 (or IPv6)
//       - Encapsulation/decapsulation of packets.
//       - Fragmentation/reassembly if required.
//       - Basic routing logic (single-interface for embedded).
//       - ICMP (Internet Control Message Protocol)
//       - Support ping and error reporting for diagnostics.
//       - UDP (User Datagram Protocol)
//       - Lightweight transport layer.
//       - Implement checksum, port handling, and socket abstraction.
//       - TCP (Transmission Control Protocol)
//       - Full state machine, retransmission, congestion control.
//       - More resource-intensive, add only if required.
//       - DHCP (Dynamic Host Configuration Protocol)
//       - Automatic IP assignment.
//       - DNS (Domain Name System)
//       - Resolve hostnames to IP addresses.
//       - Application-level protocols
//       - Examples: Modbus/TCP, MQTT, HTTP, or custom protocols.
//       - Build on UDP/TCP depending on requirements.
//       - Socket API abstraction
//       - Provide a clean interface for applications.
//       - Timers and retransmission logic
//       - Centralized management for ARP, TCP, DHCP, etc.
//       - Security and robustness
//       - Input validation, buffer overflow protection, DoS resilience.
//       - Performance tuning
//       - Zero-copy buffers, interrupt coalescing, checksum offloading.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// need to find where it is used
#define IP_STREAM_SOCKET                        0
#define IP_DNS_SOCKET                           2
#define IP_DHCP_SOCKET                          3
#define IP_SNTP_SOCKET                          3

#define TASK_IP_MANAGER_STACK_SIZE              512
#define TASK_IP_MANAGER_PRIO                    4

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#if (IP_USE_HOSTNAME == DEF_ENABLED)
  #define EXPAND_X_IF_AS_ENUM(ENUM_ID, HOST_NAME, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) ENUM_ID,
  #define EXPAND_X_IF_AS_STRUCT_DATA(ENUM_ID, HOST_NAME, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                            { HOST_NAME, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, { MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS } },
#else
  #define EXPAND_X_IF_AS_ENUM(ENUM_ID, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) ENUM_ID,
  #define EXPAND_X_IF_AS_STRUCT_DATA(ENUM_ID, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                            { PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, { MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS } },
#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum IF_ID_e
{
    IF_ETH_DEF(EXPAND_X_IF_AS_ENUM)
    IP_NUMBER_OF_INTERFACE,
};

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class IP_Manager
{
    public:

                            IP_Manager               () : m_IF_Driver(m_Context),
                                                          m_ARP(m_Context)
                                                        #if (IP_USE_DHCP == DEF_ENABLED)
                                                        , m_DHCP(m_Context)
                                                        #endif
                                                        #if (IP_USE_ICMP == DEF_ENABLED)
                                                        , m_ICMP(m_Context)
                                                        #endif
                                                        #if (IP_USE_UDP == DEF_ENABLED)
                                                        , m_UDP(m_Context)
                                                        #endif
                                                          {}


        void                Initialize                  (IF_ID_e IF_ID);
        void                Run                         (void);

        IP_PacketMsg_t*     ProcessIP                   (IP_PacketMsg_t* pRX);
        char*               ProcessURL                  (char* pBuffer, IP_Address_t* pIP_Address, IP_Port_t* pPort);

        IP_Address_t        GetDNS                      (void);
        IP_Address_t        GetHost                     (void);

        char*               IP_ToAscii                  (IP_Address_t IP_Address);
        IP_Address_t        AsciiToIP                   (char* pBuffer);

        void                PutHeader                   (IP_PacketMsg_t* pTX);
        int16_t             CalculateChecksum           (void* pBuffer, uint16_t Count);


    private:

        NetworkContext                  m_Context;


        ETH_IF_Driver                   m_IF_Driver;
        //IP_Flag_t                       m_Flag;                               // Configuration of IP Stack
        bool                            m_IP_Status;                            // TODO give better name
        bool                            m_DNS_IP_Found;

//        IP_Address_t                    m_StaticGatewayIP;                      // Gateway IP Address
//        IP_Address_t                    m_StaticSubnetMask;                     // Subnet Mask
//        IP_Address_t                    m_StaticIP;                             // Static IP Address
//        IP_Address_t                    m_StaticDNS_IP;                         // Static DNS Server IP Address

        uint8_t                         m_TX_SocketMemorySize;                  // TX Socket Memory Configuration for all x Sockets
        uint8_t                         m_RX_SocketMemorySize;                  // RX Socket Memory Configuration for all x Sockets

        NetARP                          m_ARP;                                 // Address Resolution Protocol

      #if (IP_USE_DHCP == DEF_ENABLED)
        NetDHCP                         m_DHCP;                                // Dynamic Host Control Protocol. Need UDP
      #endif

      #if (IP_USE_ICMP == DEF_ENABLED)
        NetICMP                         m_ICMP;                                // Internet Control Message Protocol
      #endif

      #if (IP_USE_NTP == DEF_ENABLED)
        NetNTP                          m_NTP;                                 // Network Time Protocol

        uint8_t                         m_NTP_Server_1[IP_MAX_URL_SIZE];       // move this to NTP
        uint8_t                         m_NTP_Server_2[IP_MAX_URL_SIZE];
      #endif

      #if (IP_USE_SNTP == DEF_ENABLED)
        NetSNTP                         m_SNTP;                                // Simple Network Transport Protocol
        bool                            m_FlagSNTP_Fail;
      #endif

      #if (IP_USE_SOAP == DEF_ENABLED)
        NetSOAP                         m_SOAP                                 // Simple Object Access Protocol

        uint8_t                         m_SOAP_Server_1[IP_MAX_URL_SIZE];       // Messaging protocol specification for exchanging structured information.
        uint8_t                         m_SOAP_Server_2[IP_MAX_URL_SIZE];
      #endif

      #if (IP_USE_SOCKET == DEF_ENABLED)
        NetSOCK                         m_SOCK                                 // Socket
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        NetTCP                          m_TCP;                                 // Transport Control Protocol
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        NetUDP                          m_UDP;                                 // User Datagram Protocol
      #endif

        IP_ETH_Config_t*                m_pEthernetIF;                          // Ethernet Configuration
        static const IP_Config_t        m_Config[IP_NUMBER_OF_INTERFACE];
        uint16_t                        m_SequenceID;

        nOS_Thread                      m_Handle;
        nOS_Stack                       m_Stack[TASK_IP_MANAGER_STACK_SIZE];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "ip_var.h"         // Project variable

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)





