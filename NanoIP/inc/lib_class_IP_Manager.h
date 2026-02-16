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

#define TASK_IP_MANAGER_STACK_SIZE              512
#define TASK_IP_MANAGER_PRIO                    4

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#if (IP_USE_HOSTNAME == DEF_ENABLED)
  #define EXPAND_X_IF_AS_ENUM(ENUM_ID, HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) ENUM_ID,
  #define EXPAND_X_IF_AS_STRUCT_DATA(ENUM_ID, HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                            { HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, { MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS } },

  #define EXPAND_X_IF_AS_STACK_DECLARATION(ENUM_ID, HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                                     nOS_Stack STK_VAR[TASK_IP_MANAGER_STACK_SIZE]  NOS_STACK_LOCATION;

#else
  #define EXPAND_X_IF_AS_ENUM(ENUM_ID, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) ENUM_ID,
  #define EXPAND_X_IF_AS_STRUCT_DATA(ENUM_ID, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                            { STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, { MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS } },

  #define EXPAND_X_IF_AS_STACK_DECLARATION(ENUM_ID, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                          nOS_Stack STK_VAR[TASK_IP_MANAGER_STACK_SIZE]  NOS_STACK_LOCATION;
#endif

//-------------------------------------------------------------------------------------------------
// Enum(s)
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

        void                Initialize                  (IF_ID_e IF_ID);
        void                Run                         (void);

        void                ProcessIP                   (IP_PacketMsg_t* pRX);
        char*               ProcessURL                  (char* pBuffer, IP_Address_t* pIP_Address, IP_Port_t* pPort);

        IP_Address_t        GetHost                     (void);
        NetworkContext*     GetContext                  (void)                              { return &m_Context; }
        SocketManager*      GetSocketManager            (void)                              { return &m_SocketManager; }
        void                PutHeader                   (IP_PacketMsg_t* pTX, IP_Address_t dstIP, uint16_t payloadLength, uint8_t protocol);    // UDP=17, TCP=6

        SystemState_e       AllocPacket                 (IP_PacketMsg_t** ppMsg, size_t PacketSize, MEM_DebugListOfID_e DebugWrapperID, MEM_DebugListOfID_e DebugPacketID);
        SystemState_e       SendPacket                  (IP_PacketMsg_t* pMsg);

        static bool         IsItBroadcastMAC            (const IP_MAC_Address_t* Mac)       { const uint8_t* b = Mac->Byte; return (b[0] & b[1] & b[2] & b[3] & b[4] & b[5]) == 0xFF; }
        static bool         IsItMulticastMAC            (const IP_MAC_Address_t* mac)       { return (mac->Byte[0] & 0x01) != 0; }
        static bool         IsItMulticast               (IP_Address_t IP)                   { uint8_t First = IP_D(IP); return ((First >= IP_MULTICAST_MIN) && (First <= IP_MULTICAST_MAX)); }

        ARP_TableEntry_t*   GetTableEntryPointer        (int Entry)                         { return &m_ARP.GetTableBasePointer()[Entry]; }

      #if (IP_USE_DNS == DEF_ENABLED)
        IP_Address_t        GetDNS                      (void);
        bool                RequestDNS                  (const char* pHostName, DNS_Callback_t Callback, void* pContext);
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        bool                RAW_RegisterSocket          (Socket* pSock, uint8_t Protocol)   { return m_RAW.RegisterSocket(pSock, Protocol); }
        void                RAW_UnregisterSocket        (uint8_t Protocol)                  { m_RAW.UnregisterSocket(Protocol); }
        SystemState_e       RAW_Send                    (RAW_Socket_t* pSock,
                                                         uint8_t* pData,
                                                         size_t Length,
                                                         SocketInfo_t* pDestInfo,
                                                         size_t* pBytesSent)                { return m_RAW.Send(pSock, pData, Length, pDestInfo, pBytesSent); }
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        SystemState_e       TCP_EnterListen             (Socket* pSock, uint16_t Backlog)   { return m_TCP.EnterListen(pSock, Backlog); }
        void                TCP_Close                   (Socket* pSock)                     { m_TCP.Close(pSock); }
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        IP_Port_t           UDP_AllocateEphemeralPort   (void)                              { return m_UDP.AllocateEphemeralPort(); }
        bool                UDP_RegisterSocket          (Socket* pSock, IP_Port_t Port)     { return m_UDP.RegisterSocket(pSock, Port); }
        void                UDP_UnregisterSocket        (IP_Port_t Port)                    { m_UDP.UnregisterSocket(Port); }
        SystemState_e       UDP_Send                    (UDP_Socket_t* pSock,
                                                         uint8_t* pData,
                                                         size_t Length,
                                                         SocketInfo_t* pDestInfo,
                                                         size_t* pBytesSent)                { return m_UDP.Send(pSock, pData, Length, pDestInfo, pBytesSent); }
      #endif


        static uint32_t     ChecksumAccumulate          (const uint8_t* Data, uint16_t Count);
        static uint16_t     ChecksumFinalize            (uint32_t Checksum);
        static uint16_t     IP_CalculateChecksum        (const void* pBuffer, uint16_t Count);
      #if (IP_USE_TCP == DEF_ENABLED)
        static uint16_t     TCP_CalculateChecksum       (IP_Header_t* pIP, TCP_Header_t* pTCP, uint16_t TCP_Length);
      #endif
      #if (IP_USE_UDP == DEF_ENABLED)
        static uint16_t     UDP_CalculateChecksum       (IP_Header_t* pIP, UDP_Header_t* pUDP, uint16_t UDP_Length);
      #endif
        static void         FreeMessage                 (IP_PacketMsg_t* pMsg);
        static void         IP_ToAscii                  (char* pBuffer, IP_Address_t IP_Address);
        static IP_Address_t AsciiToIP                   (const char* pBuffer);

    private:

        NetworkContext                  m_Context;
        ETH_IF_Driver                   m_IF_Driver;
        bool                            m_DNS_IP_Found;

        uint8_t                         m_TX_SocketMemorySize;                  // TX Socket Memory Configuration for all x Sockets
        uint8_t                         m_RX_SocketMemorySize;                  // RX Socket Memory Configuration for all x Sockets

        ARP_Protocol                    m_ARP;                                  // Address Resolution Protocol

      #if (IP_USE_DHCP == DEF_ENABLED)
        DHCPv4_Client                   m_DHCP;                                 // Dynamic Host Control Protocol. Need UDP
      #endif

      #if (IP_USE_DNS == DEF_ENABLED)
        DNS_Client                      m_DNS;                                  // Domain name system Protocol. Need UDP
      #endif

      #if (IP_USE_ICMP == DEF_ENABLED)
        ICMP_Protocol                   m_ICMP;                                 // Internet Control Message Protocol
      #endif

      #if (IP_USE_NTP == DEF_ENABLED)
        NTP_Client                      m_NTP;                                  // Network Time Protocol
        uint8_t                         m_NTP_Server_1[IP_MAX_URL_SIZE];        // move this to NTP
        uint8_t                         m_NTP_Server_2[IP_MAX_URL_SIZE];
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        IP_RawProtocol                  m_RAW;                                  // RAW
      #endif

      #if (IP_USE_TCP_SERVER == DEF_ENABLED)
        TCP_Server                      m_TCP_Server;                           // Transport Control Protocol Server sise
      #endif

      #if (IP_USE_TCP_CLIENT == DEF_ENABLED)
        TCP_Client                      m_TCP_Client;                           // Transport Control Protocol Cleint Side
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        UDP_Protocol                    m_UDP;                                  // User Datagram Protocol
      #endif

        IP_ETH_Config_t*                m_pEthernetIF;                          // Ethernet Configuration
        static const IP_Config_t        m_Config[IP_NUMBER_OF_INTERFACE];
        uint16_t                        m_SequenceID;
        nOS_Thread                      m_Handle;
        SocketManager                   m_SocketManager;
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)





