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

#define TASK_IP_MANAGER_STACK_SIZE                      512// 256
#define TASK_IP_MANAGER_PRIO                            4

#define TASK_IP_MANAGER_TREAD_NAME_PREAMBULE            "Task IP Mgr: "
#define TASK_IP_MANAGER_TREAD_NAME_PREAMBULE_SIZE       sizeof(TASK_IP_MANAGER_TREAD_NAME_PREAMBULE)
#define TASK_IP_MANAGER_TREAD_NAME_SIZE                 32
#define TASK_IP_MANAGER_TREAD_NAME_EXTRACT_SIZE         (TASK_IP_MANAGER_TREAD_NAME_SIZE - TASK_IP_MANAGER_TREAD_NAME_PREAMBULE_SIZE) + 1

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class IP_Manager
{
    public:

        void                    Initialize                  (NetworkContext* pContextt);
        void                    Run                         (void);

        void                    ProcessIP                   (IP_PacketMsg_t* pRX);
        char*                   ProcessURL                  (char* pBuffer, IP_Address_t* pIP_Address, IP_Port_t* pPort);
        void                    PutHeader                   (IP_PacketMsg_t* pTX, IP_Address_t dstIP, uint16_t payloadLength, uint8_t protocol);    // UDP=17, TCP=6
        static bool             IsItBroadcastMAC            (const IP_MAC_Address_t* Mac)       { const uint8_t* b = Mac->Byte; return (b[0] & b[1] & b[2] & b[3] & b[4] & b[5]) == 0xFF; }
        static bool             IsItMulticastMAC            (const IP_MAC_Address_t* mac)       { return (mac->Byte[0] & 0x01) != 0; }
        static bool             IsItMulticast               (IP_Address_t IP)                   { uint8_t First = IP_D(IP); return ((First >= IP_MULTICAST_MIN) && (First <= IP_MULTICAST_MAX)); }
        IP_Port_t               AllocateEphemeralPort       (void);


        SystemState_e           SendPacket                  (IP_PacketMsg_t* pMsg);

        static SystemState_e    AllocPacket                 (IP_PacketMsg_t** ppMsg, size_t PacketSize, MEM_DebugListOfID_e DebugWrapperID, MEM_DebugListOfID_e DebugPacketID);

        static uint16_t         CalculateChecksum           (IP_Header_t* pIP, uint8_t Protocol, void* pProtocolHeader, uint16_t Length);


//        static uint16_t         IP_CalculateChecksum        (const void* pBuffer, uint16_t Count);
      #if (IP_USE_TCP_CLIENT == DEF_ENABLED) || (IP_USE_TCP_SERVER == DEF_ENABLED)
//        static uint16_t         TCP_CalculateChecksum       (IP_Header_t* pIP, TCP_Header_t* pTCP, uint16_t TCP_Length);
      #endif
      #if (IP_USE_UDP == DEF_ENABLED)
//        static uint16_t         UDP_CalculateChecksum       (IP_Header_t* pIP, UDP_Header_t* pUDP, uint16_t UDP_Length);
      #endif
        static void             FreeMessage                 (IP_PacketMsg_t* pMsg);
        static void             IP_ToAscii                  (char* pBuffer, IP_Address_t IP_Address);
        static IP_Address_t     AsciiToIP                   (const char* pBuffer);

    private:

        NetworkContext*                 m_pContext;
        uint16_t                        m_SequenceID;
        nOS_Thread                      m_Handle;
        nOS_Stack                       m_Stack                [TASK_IP_MANAGER_STACK_SIZE];
        char                            m_ThreadName           [TASK_IP_MANAGER_TREAD_NAME_SIZE]     = TASK_IP_MANAGER_TREAD_NAME_PREAMBULE;
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)





