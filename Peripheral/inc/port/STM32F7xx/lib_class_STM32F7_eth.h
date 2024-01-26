//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_eth.h
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
//
//  Note:  The m_RX_Descriptor and m_TX_Descriptor must be declared in a device non cacheable
//         memory region.
//         In this code they are declared at the end of the SRAM1 memory. This memory region
//         must be configured by MPU as a device memory.
//
//         In this code the ETH buffers are located in the SRAM2 with MPU configured as normal
//         not cacheable memory.
//
//         SRAM2 size is 16K
//
//         Please refer to MPU_Config() in main.c file.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define NUM_TX_Buffer               4
#define NUM_RX_Buffer               6
#define ETH_BUF_SIZE                1534
#define ETH_IRQ_PRIO                4

//----- Ethernet MAC Frame Transmit Flags -----
#define ETH_MAC_TX_FRAME_FRAGMENT   (1UL << 0)      // Indicate frame fragment
#define ETH_MAC_TX_FRAME_EVENT      (1UL << 1)      // Generate event when frame is transmitted
#define ETH_MAC_TX_FRAME_TIMESTAMP  (1UL << 2)      // Capture frame time stamp

//----- Ethernet MAC Event -----
#define ETH_MAC_EVENT_NONE          (0UL << 0)      // No event
#define ETH_MAC_EVENT_RX_FRAME      (1UL << 0)      // Frame received
#define ETH_MAC_EVENT_TX_FRAME      (1UL << 1)      // Frame transmitted
#define ETH_MAC_EVENT_WAKEUP        (1UL << 2)      // Wake-up (on Magic Packet)
#define ETH_MAC_EVENT_TIMER_ALARM   (1UL << 3)      // Timer alarm

#define ETH_OWNED_BY_DMA            0x00000000
#define ETH_INVALID_BLOCK           0xFFFFFFFF

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef void (*ETH_MAC_SignalEvent_t) (uint32_t Event);  // Pointer to ETH_MAC_SignalEvent fucntion

// EMAC Driver Control Information
struct ETH_MAC_Control_t
{
    ETH_MAC_SignalEvent_t   CallbackEvent;          // Event callback
    uint8_t                 TX_Index;               // Transmit descriptor index
    uint8_t                 RX_Index;               // Receive descriptor index
  #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
    uint8_t                 TX_TS_Index;            // Transmit Timestamp descriptor index
  #endif
    uint8_t*                FrameEnd;               // End of assembled frame fragments
};

// DMA RX Descriptor
struct RX_Descriptor_t
{
    uint32_t volatile       Stat;
    uint32_t                Ctrl;
    uint8_t const*          Addr;
    struct RX_Descriptor_t* Next;
#if ((ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED) || (ETH_USE_TIME_STAMP == DEF_ENABLED))
    uint32_t                ExtStat;
    uint32_t                Reserved[1];
    uint32_t                TimeLo;
    uint32_t                TimeHi;
#endif
};

// DMA TX Descriptor
struct TX_Descriptor_t
{
    uint32_t volatile       Stat;
    uint32_t                Size;
    uint8_t*                Addr;
    struct TX_Descriptor_t* Next;
#if ((ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED) || (ETH_USE_TIME_STAMP == DEF_ENABLED))
    uint32_t                Reserved[2];
    uint32_t                TimeLo;
    uint32_t                TimeHi;
#endif
};

//-------------------------------------------------------------------------------------------------
// Validate Memory footprint(s)
//-------------------------------------------------------------------------------------------------

#define ETH_MAX_DESC_MEMORY         1024
#define ETH_MAX_BUF_MEMORY          15 * 1024

#if ((ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED) || (ETH_USE_TIME_STAMP == DEF_ENABLED))
#define SIZEOF_RX_Desc              32
#define SIZEOF_TX_Desc              32
#else
#define SIZEOF_RX_Desc              16
#define SIZEOF_TX_Desc              16
#endif

#if ((NUM_RX_Buffer * SIZEOF_RX_Desc) + (NUM_TX_Buffer * SIZEOF_TX_Desc)) > ETH_MAX_DESC_MEMORY
    #error "SRAM2 Descriptor overflow"
#endif

#if (((NUM_RX_Buffer + NUM_TX_Buffer) * ETH_BUF_SIZE)) > ETH_MAX_BUF_MEMORY
    #error "SRAM2 Buffer overflow"
#endif

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class ETH_Driver
{
    public:

        SystemState_e           Initialize              (ETH_MAC_SignalEvent_t CallbackEvent);                           // Initialize Ethernet MAC Device.

        void                    Start                   (void);                                                          // Start ETH module
        SystemState_e           GetMacAddress           (      ETH_MAC_Address_t* pMAC_Address);                         // Get Ethernet MAC Address.
        SystemState_e           SetMacAddress           (const ETH_MAC_Address_t* pMAC_Address);                         // Set Ethernet MAC Address.
        SystemState_e           SetAddressFilter        (const ETH_MAC_Address_t* pMAC_Address, uint32_t NbAddress);     // Configure Address Filter.
        SystemState_e           SendFrame               (const uint8_t* frame, size_t Length, uint32_t flags);           // Send Ethernet frame.
        SystemState_e           ReadFrame               (MemoryNode* pPacket, size_t Length);                            // Read data of received Ethernet frame.
        uint32_t                GetRX_FrameSize         (void);                                                          // Get size of received Ethernet frame.
      #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
        SystemState_e           GetRX_FrameTime         (ETH_MacTime_t* Time);                                           // Get time of received Ethernet frame.
        SystemState_e           GetTX_FrameTime         (ETH_MacTime_t* Time);                                           // Get time of transmitted Ethernet frame.
        SystemState_e           ControlTimer            (ETH_ControlTimer_e Control, ETH_MacTime_t* Time);               // Control Precision Timer.
      #endif

        SystemState_e           PHY_Read                (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* pData); // Read Ethernet PHY Register through Management Interface.
        SystemState_e           PHY_Write               (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t   Data); // Write Ethernet PHY Register through Management Interface.

        static void             ISR_CallBack             (uint32_t Event);

    private:

        void                    InitializeDMA_Buffer    (void);
        void                    Control                 (void);
        SystemState_e           PHY_Busy                (void);

        static     ETH_MAC_Control_t           m_MAC_Control;
        static     RX_Descriptor_t             m_RX_Descriptor   [NUM_RX_Buffer]                     __attribute__((section(".RX_DescriptorSection"), aligned(4)));   // Ethernet RX & TX DMA Descriptors
        static     TX_Descriptor_t             m_TX_Descriptor   [NUM_TX_Buffer]                     __attribute__((section(".TX_DescriptorSection"), aligned(4)));
        static     uint32_t                    m_RX_Buffer       [NUM_RX_Buffer][ETH_BUF_SIZE >> 2]  __attribute__((section(".RX_ArraySection"),      aligned(4)));   // Ethernet Receive buffers
        static     uint32_t                    m_TX_Buffer       [NUM_TX_Buffer][ETH_BUF_SIZE >> 2]  __attribute__((section(".TX_ArraySection"),      aligned(4)));   // Ethernet Transmit buffers
};

//-------------------------------------------------------------------------------------------------

#endif // (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
