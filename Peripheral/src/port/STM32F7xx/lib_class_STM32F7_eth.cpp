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
//      The Driver requires:
//              - Setup of HCLK to 25MHz or higher
//              - Optional setup of SYSCLK to 50MHz or higher, when Ethernet PTP is used
//              - Setup of ETH in MII or RMII mode
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"
#include "lwip/netif.h"
#include "lwip/err.h"

//-------------------------------------------------------------------------------------------------

#if (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

// Timeouts
#define PHY_TIMEOUT         2               // PHY Register access timeout in ms
#define RESET_TIMEOUT       10

// Interrupt Handler Prototype
extern "C" void ETH_IRQHandler(void);


RX_Descriptor_t ETH_Driver::m_RX_Descriptor   [NUM_RX_Buffer]                     __attribute__((aligned(4)));   // Ethernet RX & TX DMA Descriptors
uint32_t        ETH_Driver::m_RX_Buffer       [NUM_RX_Buffer][ETH_BUF_SIZE >> 2]  __attribute__((aligned(4)));   // Ethernet Receive buffers
TX_Descriptor_t ETH_Driver::m_TX_Descriptor   [NUM_TX_Buffer]                     __attribute__((aligned(4)));
uint32_t        ETH_Driver::m_TX_Buffer       [NUM_TX_Buffer][ETH_BUF_SIZE >> 2]  __attribute__((aligned(4)));   // Ethernet Transmit buffers

ETH_MAC_Control_t ETH_Driver::m_MAC_Control;

// Driver Capabilities
ETH_MAC_Capability_t ETH_Driver::m_Capabilities =
{
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_rx_ip4
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_rx_ip6
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_rx_udp
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_rx_tcp
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_rx_icmp
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_tx_ip4
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_tx_ip6
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_tx_udp
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_tx_tcp
    (DIGINI_USE_ETH_CHECKSUM_OFFLOAD != 0) ? 1 : 0, // checksum_offload_tx_icmp
    DIGINI_ETH_INTERFACE_CFG,                       // media_interface
    0,                                              // mac_address
    1,                                              // event_rx_frame
    1,                                              // event_tx_frame
    1,                                              // event_wakeup
    (DIGINI_USE_ETH_TIME_STAMP != 0) ? 1 : 0        // precision_timer
};

//-------------------------------------------------------------------------------------------------
//
//   Function name:     InitializeDMA
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Initialize DMA, RX DMA descriptors and TX DMA descriptors.
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::InitializeDMA(void)
{
    uint32_t Next;

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_DMA_INIT) == 0)
    {
        m_MAC_Control.Flags |= ETH_MAC_FLAG_DMA_INIT;

        #if ((DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED) || (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED))
        ETH->DMABMR |= ETH_DMABMR_EDE;                                                      // Enhanced descriptor enable
        #endif

        // Initialize DMA Descriptors
        for(uint32_t i = 0; i < NUM_TX_Buffer; i++)
        {
            m_TX_Descriptor[i].Stat = DMA_TX_TCH | DMA_TX_LS | DMA_TX_FS;
            m_TX_Descriptor[i].Addr = (uint8_t *)&m_TX_Buffer[i];
            Next = i + 1;

            if(Next == NUM_TX_Buffer)
            {
                Next = 0;
            }

            m_TX_Descriptor[i].Next = &m_TX_Descriptor[Next];
        }

        for(uint32_t i = 0; i < NUM_RX_Buffer; i++)
        {
            m_RX_Descriptor[i].Stat = DMA_RX_OWN;
            m_RX_Descriptor[i].Ctrl = DMA_RX_RCH | ETH_BUF_SIZE;
            m_RX_Descriptor[i].Addr = (uint8_t *)&m_RX_Buffer[i];
            Next = i + 1;

            if(Next == NUM_RX_Buffer)
            {
                Next = 0;
            }

            m_RX_Descriptor[i].Next = &m_RX_Descriptor[Next];
        }

        ETH->DMATDLAR = (uint32_t)&m_TX_Descriptor[0];
        ETH->DMARDLAR = (uint32_t)&m_RX_Descriptor[0];
        m_MAC_Control.TX_Index = 0;
        m_MAC_Control.RX_Index = 0;

        // Enable Rx interrupts
        ETH->DMAIER = ETH_DMAIER_NISE | ETH_DMAIER_RIE /*| ETH_DMAIER_TIE*/;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     GetCapabilities
//
//   Parameter(s):      None
//   Return value:      ETH_MAC_Capability_t
//
//   Description:       Get driver capabilities.
//
//-------------------------------------------------------------------------------------------------
ETH_MAC_Capability_t ETH_Driver::GetCapabilities(void)
{
    return m_Capabilities;
}


//-------------------------------------------------------------------------------------------------
//
//   Function name:     Initialize
//
//   Parameter(s):      CallbackEvent   Pointer to ETH_MAC_SignalEvent_t for processing.
//   Return value:      SystemState_e   State of function.
//
//   Description:       Initialize Ethernet MAC Device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::Initialize(ETH_MAC_SignalEvent_t CallbackEvent)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->PMC  |= SYSCFG_PMC_MII_RMII_SEL;

    IO_PinInit(IO_MX_ETH_MDC);
    IO_PinInit(IO_MX_ETH_MDIO);
    IO_PinInit(IO_MX_ETH_TXD0);
    IO_PinInit(IO_MX_ETH_TXD1);
    IO_PinInit(IO_MX_ETH_RXD0);
    IO_PinInit(IO_MX_ETH_RXD1);
    IO_PinInit(IO_MX_ETH_TX_EN);
    IO_PinInit(IO_MX_ETH_RX_ER);
    IO_PinInit(IO_MX_ETH_CRS_DV);
    IO_PinInit(IO_MX_ETH_REF_CLK);

    // Clear Control Structure
    memset ((void *)&m_MAC_Control, 0, sizeof(ETH_MAC_Control_t));

    m_MAC_Control.CallbackEvent = CallbackEvent;
    m_MAC_Control.Flags         = ETH_MAC_FLAG_INIT;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     Uninitialize
//
//   Parameter(s):      None
//   Return value:      SystemState_e   State of function.
//
//   Description:       De-initialize Ethernet MAC Device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::Uninitialize(void)
{
    IO_PinInitInput(IO_MX_ETH_MDC);
    IO_PinInitInput(IO_MX_ETH_MDIO);
    IO_PinInitInput(IO_MX_ETH_TXD0);
    IO_PinInitInput(IO_MX_ETH_TXD1);
    IO_PinInitInput(IO_MX_ETH_RXD0);
    IO_PinInitInput(IO_MX_ETH_RXD1);
    IO_PinInitInput(IO_MX_ETH_TX_EN);
    IO_PinInitInput(IO_MX_ETH_RX_ER);
    IO_PinInitInput(IO_MX_ETH_CRS_DV);
    IO_PinInitInput(IO_MX_ETH_REF_CLK);

    m_MAC_Control.Flags &= ~uint32_t(ETH_MAC_FLAG_INIT);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     PowerControl
//
//   Parameter(s):      ETH_PowerState_e    State    can be: ETH_POWER_OFF
//                                                           ETH_POWER_LOW
//                                                           ETH_POWER_FULL
//   Return value:      SystemState_e       State of function.
//
//   Description:       Control Ethernet MAC Device Power.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::PowerControl(ETH_PowerState_e State)
{
    switch(State)
    {
        case ETH_POWER_OFF:
        {
            EnableClock();
            ResetMac();
            NVIC_DisableIRQ(ETH_IRQn);
            ETH->DMAIER  = 0;
            ETH->PTPTSCR = 0;
            DisableClock();
            m_MAC_Control.Flags &= ~uint32_t(ETH_MAC_FLAG_POWER | ETH_MAC_FLAG_DMA_INIT);
        }
        break;

        case ETH_POWER_LOW:
        {
            return SYS_UNSUPPORTED_FEATURE;
        }

        case ETH_POWER_FULL:
        {
            TickCount_t TickStart;

            if((m_MAC_Control.Flags & ETH_MAC_FLAG_INIT) == 0)
            {
                DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Driver not initialized\n");
                return SYS_ERROR;                                           // Driver not initialized
            }

            if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) != 0)
            {
                DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Driver already powered\n");
                break;                                                      // Driver already powered
            }

            EnableClock();
            ResetMac();

            ETH->MACMIIAR = ETH_MACIIAR_CR_DIVIDER;                         // MDC clock range selection
            ETH->MACCR    = ETH_MACCR_RD | 0x00008000;                      // Initialize MAC configuration

            // Initialize Filter registers
            ETH->MACFFR    = ETH_MACFFR_PCF_BlockAll; //ETH_MACFFR_BFD;
            ETH->MACFCR    = ETH_MACFCR_ZQPD;
            ETH->MACVLANTR = 0;

            // Initialize Address registers
            ETH->MACA0HR = 0x80000000; ETH->MACA0LR = 0;
            //ETH->MACA1HR = 0; ETH->MACA1LR = 0;   // checkif it impair the behavior
            //ETH->MACA2HR = 0; ETH->MACA2LR = 0;
            //ETH->MACA3HR = 0; ETH->MACA3LR = 0;

            // Mask time stamp interrupts
            ETH->MACIMR = 0; // |= ETH_MACIMR_TSTIM | ETH_MACIMR_PMTIM; // might be the default

          #if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
            // Set clock accuracy to 20ns (50MHz) or 50ns (20MHz)
            if(SYS_CPU_CORE_CLOCK_FREQUENCY >= 51000000)
            {
                ETH->PTPSSIR = 20;
                ETH->PTPTSAR = (50000000ull << 32) / SYS_CPU_CORE_CLOCK_FREQUENCY;
            }
            else
            {
                ETH->PTPSSIR = 50;
                ETH->PTPTSAR = (20000000ull << 32) / SYS_CPU_CORE_CLOCK_FREQUENCY;
            }

            ETH->PTPTSCR = ETH_PTPTSSR_TSSIPV4FE | ETH_PTPTSSR_TSSIPV6FE | ETH_PTPTSSR_TSSSR |
                                                                           ETH_PTPTSCR_TSARU |
                                                                           ETH_PTPTSCR_TSFCU |
                                                                           ETH_PTPTSCR_TSE;
            m_MAC_Control.TX_TS_Index = 0;
          #endif

            // Disable MMC interrupts
            ETH->MMCTIMR = 0;
            ETH->MMCRIMR = 0;


            ETH->DMABMR = ETH_DMABMR_SR;                                    // Reset Ethernet MAC peripheral

            // Wait for software reset
            TickStart = GetTick();
            do
            {
                if((ETH->DMABMR & ETH_DMABMR_SR) == 0)
                {
                    break;
                }

                nOS_Yield();
            }
            while(TickHasTimeOut(TickStart, RESET_TIMEOUT) == false);

            // ---- Enable ETH interrupt ----
            NVIC_ClearPendingIRQ(ETH_IRQn);
            ISR_Init(ETH_IRQn, 0, ETH_IRQ_PRIO);

            m_MAC_Control.FrameEnd = nullptr;
            m_MAC_Control.Flags   |= ETH_MAC_FLAG_POWER;
        }
        break;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     GetMacAddress
//
//   Parameter(s):      pMAC_Address    Pointer to address.
//   Return value:      SystemState_e   State of function.
//
//   Description:       Get Ethernet MAC Address.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::GetMacAddress(ETH_MAC_Address_t* pMAC_Address)
{
    uint32_t RegisterValue;

    if(pMAC_Address == nullptr)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetMacAddress - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
    }

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetMacAddress - Driver not powered\n");
        return SYS_ERROR;
    }

    RegisterValue = ETH->MACA0LR;
    pMAC_Address->Byte[0] = uint8_t(RegisterValue);
    pMAC_Address->Byte[1] = uint8_t(RegisterValue >> 8);
    pMAC_Address->Byte[2] = uint8_t(RegisterValue >> 16);
    pMAC_Address->Byte[3] = uint8_t(RegisterValue >> 24);
    RegisterValue = ETH->MACA0HR;
    pMAC_Address->Byte[4] = uint8_t(RegisterValue);
    pMAC_Address->Byte[5] = uint8_t(RegisterValue >> 8);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     SetMacAddress
//
//   Parameter(s):      pMAC_Address    Pointer to address.
//   Return value:      SystemState_e   State of function.
//
//   Description:       Set Ethernet MAC Address.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::SetMacAddress(const ETH_MAC_Address_t* pMAC_Address)
{
    if(pMAC_Address == nullptr)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SetMacAddress - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
    }

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SetMacAddress - Driver not powered\n");
        return SYS_ERROR;
    }

    // Set Ethernet MAC Address registers
    ETH->MACA0LR = (uint32_t(pMAC_Address->Byte[0])       |
                    uint32_t(pMAC_Address->Byte[1] << 8)  |
                    uint32_t(pMAC_Address->Byte[2] << 16) |
                    uint32_t(pMAC_Address->Byte[3] << 24));

    ETH->MACA0HR = (uint32_t(pMAC_Address->Byte[4])       |
                    uint32_t(pMAC_Address->Byte[5] << 8));

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     SetAddressFilter
//
//   Parameter(s):      pMAC_Address    Pointer to address.
//                      NbAddress       Number of addresses to configure.
//   Return value:      SystemState_e   State of function.
//
//   Description:       Configure Address Filter.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::SetAddressFilter(const ETH_MAC_Address_t* pMAC_Address, uint32_t NbAddress)
{
    uint32_t CRC_Value;
    CRC_Calc myCrc(CRC_32);

    if((pMAC_Address == nullptr) && (NbAddress != 0))
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SetAddressFilter - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
    }

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SetAddressFilter - Driver not powered\n");
        return SYS_ERROR;
    }

    // Use unicast address filtering for first 3 MAC addresses
    ETH->MACFFR &= ~(ETH_MACFFR_HPF | ETH_MACFFR_HM);
    ETH->MACHTHR = 0; ETH->MACHTLR = 0;

    if(NbAddress == 0)
    {
        ETH->MACA1HR = 0; ETH->MACA1LR = 0;
        ETH->MACA2HR = 0; ETH->MACA2LR = 0;
        ETH->MACA3HR = 0; ETH->MACA3LR = 0;
        return SYS_READY;
    }

    ETH->MACA1HR = uint32_t(pMAC_Address->Byte[4])        | (uint32_t(pMAC_Address->Byte[5]) << 8) | ETH_MACA1HR_AE;
    ETH->MACA1LR = uint32_t(pMAC_Address->Byte[0])        | (uint32_t(pMAC_Address->Byte[1]) << 8) |
                  (uint32_t(pMAC_Address->Byte[2]) << 16) | (uint32_t(pMAC_Address->Byte[3]) << 24);
    NbAddress--;

    if(NbAddress == 0)
    {
        ETH->MACA2HR = 0; ETH->MACA2LR = 0;
        ETH->MACA3HR = 0; ETH->MACA3LR = 0;
        return SYS_READY;
    }

    pMAC_Address++;
    ETH->MACA2HR = uint32_t(pMAC_Address->Byte[4])        | (uint32_t(pMAC_Address->Byte[5]) << 8) | ETH_MACA2HR_AE;
    ETH->MACA2LR = uint32_t(pMAC_Address->Byte[0])        | (uint32_t(pMAC_Address->Byte[1]) << 8) |
                  (uint32_t(pMAC_Address->Byte[2]) << 16) | (uint32_t(pMAC_Address->Byte[3]) << 24);
    NbAddress--;

    if(NbAddress == 0)
    {
        ETH->MACA3HR = 0; ETH->MACA3LR = 0;
        return SYS_READY;
    }

    pMAC_Address++;
    ETH->MACA3HR = uint32_t(pMAC_Address->Byte[4])        | (uint32_t(pMAC_Address->Byte[5]) << 8) | ETH_MACA3HR_AE;
    ETH->MACA3LR = uint32_t(pMAC_Address->Byte[0])        | (uint32_t(pMAC_Address->Byte[1]) << 8) |
                  (uint32_t(pMAC_Address->Byte[2]) << 16) | (uint32_t(pMAC_Address->Byte[3]) << 24);
    NbAddress--;

    if(NbAddress != 0)
    {
        pMAC_Address++;

        // Calculate 64-bit hash table for remaining MAC addresses
        for(; NbAddress; pMAC_Address++, NbAddress--)
        {
            CRC_Value = myCrc.CalculateFullBuffer(&pMAC_Address->Byte[0], 6) >> 26;

            if(CRC_Value & 0x20)
            {
                ETH->MACHTHR |= (1 << (CRC_Value & 0x1F));
            }
            else
            {
                ETH->MACHTLR |= (1 << CRC_Value);
            }
        }

        // Enable both, unicast and hash address filtering
        ETH->MACFFR |= ETH_MACFFR_HPF | ETH_MACFFR_HM;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     SendFrame
//
//   Parameter(s):      Frame           Pointer to frame buffer with data to send.
//                      Length          Frame buffer length in bytes.
//                      Flags           Frame transmit Flags.
//
//   Return value:      SystemState_e   State of function.
//
//   Description:       Send Ethernet frame.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::SendFrame(const uint8_t* pFrame, size_t Length, uint32_t Flags)
{
    uint8_t* pDst = m_MAC_Control.FrameEnd;
    uint32_t Control;

    if((pFrame == nullptr) || (Length == 0))
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SendFrame - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
    }

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SendFrame - Driver not powered\n");
        return SYS_ERROR;
    }

    if(pDst == nullptr)
    {
        // Start of a new transmit frame
        if(m_TX_Descriptor[m_MAC_Control.TX_Index].Stat & DMA_TX_OWN)
        {
            // Transmitter is busy, wait
            DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SendFrame - TX Busy\n");
            return SYS_BUSY;
        }

        pDst = m_TX_Descriptor[m_MAC_Control.TX_Index].Addr;
        m_TX_Descriptor[m_MAC_Control.TX_Index].Size = Length;
    }
    else
    {
        // Sending data fragments in progress
        m_TX_Descriptor[m_MAC_Control.TX_Index].Size += Length;
    }

    LIB_FastMemcpy(pFrame, pDst, Length);

    if(Flags & ETH_MAC_TX_FRAME_FRAGMENT)
    {
        // More data to come, remember current write position
        m_MAC_Control.FrameEnd = pDst;
        return SYS_READY;
    }

    // Frame is now ready, send it to DMA
    Control = m_TX_Descriptor[m_MAC_Control.TX_Index].Stat & ~uint32_t(DMA_TX_CIC);

  #if(DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED)
    if(m_MAC_Control.TX_ChecksumsOffload)
    {
        /*  The following is a workaround for MAC Control silicon problem:
                "Incorrect layer 3 (L3) checksum is inserted in the transmitted IPV6 fragmented packets
                 without TCP, UDP or ICMP payloads
."
            Description:
                The application provides the per-frame control to instruct the MAC to insert the L3
                checksums for TCP, UDP and ICMP packets. When an automatic checksum insertion is
                enabled and the input packet is an IPv6 packet without the TCP, UDP or ICMP payload, then
                the MAC may incorrectly insert a checksum into the packet. For IPv6 packets without a TCP,
                UDP or ICMP payload, the MAC core considers the next header (NH) field as the extension
                header and continues to parse the extension header. Sometimes, the payload data in such
                packets matches the NH field for TCP, UDP or ICMP and, as a result, the MAC core inserts
                a checksum.
        */
        uint16_t Prot = UNALIGNED_UINT16_READ(&m_TX_Descriptor[m_MAC_Control.TX_Index].Addr[12]);
        uint16_t Frag = UNALIGNED_UINT16_READ(&m_TX_Descriptor[m_MAC_Control.TX_Index].Addr[20]);

        if((Prot == 0x0008) && (Fag & 0xFF3F))
        {
            Control |= DMA_TX_CIC_IP;               // Insert only IP header checksum in fragmented frame
        }
        else
        {
            Control |= DMA_TX_CIC;                  // Insert IP header and payload checksums (TCP,UDP,ICMP)
        }
    }
  #endif

    Control &= ~uint32_t(DMA_TX_IC | DMA_TX_TTSE);

    if(Flags & ETH_MAC_TX_FRAME_EVENT)
    {
        Control |= DMA_TX_IC;
    }

  #if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
    if(Flags & ETH_MAC_TX_FRAME_TIMESTAMP)
    {
        Control |= DMA_TX_TTSE;
    }

    m_MAC_Control.TX_TS_Index = m_MAC_Control.TX_Index;
  #endif

    m_TX_Descriptor[m_MAC_Control.TX_Index].Stat = Control | DMA_TX_OWN;
    m_MAC_Control.TX_Index++;

    if(m_MAC_Control.TX_Index == NUM_TX_Buffer)
    {
        m_MAC_Control.TX_Index = 0;
    }

    m_MAC_Control.FrameEnd = nullptr;

    // Start frame transmission
    ETH->DMASR   = ETH_DMASR_TBUS;
    ETH->DMATPDR = 0;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     ReadFrame
//
//   Parameter(s):      ptrBuf          Pointer to on pbuf LWIP data.
//                      Length          Frame buffer length in bytes.
//   Return value:      SystemState_e   State of function.
//
//   Description:       Read data of received Ethernet frame.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::ReadFrame(struct pbuf* ptrBuf, size_t Length)
{
    SystemState_e State  = SYS_READY;
    uint8_t const* pSrc  = m_RX_Descriptor[m_MAC_Control.RX_Index].Addr;
    size_t Granularity   = size_t(ptrBuf->len);
    uint8_t* pFrame      = static_cast<uint8_t*>(ptrBuf->payload);

    if((ptrBuf == nullptr) && (Length != 0))
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: ReadFrame - Invalid Parameter\n");
        State = SYS_INVALID_PARAMETER;
    }
    else if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: ReadFrame - Driver not powered\n");
        State = SYS_ERROR;
    }
    else
    {
        while(Length > 0)
        {
            Length -= int32_t(ptrBuf->len);
            LIB_FastMemcpy(pSrc, pFrame, Granularity);
            ptrBuf = ptrBuf->next;

            if(ptrBuf != nullptr)
            {
                Granularity = size_t(ptrBuf->len);
                pFrame      = static_cast<uint8_t*>(ptrBuf->payload);
            }
            else
            {
                Length = 0;
            }
        }
    }

    // Return this block back to ETH-DMA
    m_RX_Descriptor[m_MAC_Control.RX_Index].Stat = DMA_RX_OWN;
    m_MAC_Control.RX_Index++;

    if(m_MAC_Control.RX_Index == NUM_RX_Buffer)
    {
        m_MAC_Control.RX_Index = 0;
    }

    if(ETH->DMASR & ETH_DMASR_RBUS)
    {
        // Receive buffer unavailable, resume DMA
        ETH->DMASR   = ETH_DMASR_RBUS;
        ETH->DMARPDR = 0;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     GetRX_FrameSize
//
//   Parameter(s):      None
//   Return value:      uint32_t    Number of bytes in received frame.
//
//   Description:       Get size of received Ethernet frame.
//
//-------------------------------------------------------------------------------------------------
uint32_t ETH_Driver::GetRX_FrameSize(void)
{
    uint32_t Stat = m_RX_Descriptor[m_MAC_Control.RX_Index].Stat;

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameSize - Invalid Parameter\n");
        return  0;
    }

    if(Stat & DMA_RX_OWN)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameSize - Driver not powered\n");
        return 0;                       // Owned by DMA
    }

    if(((Stat & DMA_RX_ES) != 0) || ((Stat & DMA_RX_FS) == 0) || ((Stat & DMA_RX_LS) == 0))
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameSize - This block is invalid\n");
        return (0xFFFFFFFF);            // Error, this block is invalid
    }

    return ((Stat & DMA_RX_FL) >> 16) - 4;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     GetRX_FrameTime
//
//   Parameter(s):      Time                Pointer to time structure.
//   Return value:      SystemState_e       State of function.
//
//   Description:       Get time of received Ethernet frame.
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
SystemState_e ETH_Driver::GetRX_FrameTime(ETH_MAC_Time_t* Time)
{
    RX_Descriptor* RX_Desc = &RX_Descriptor[m_MAC_Control.RX_Index];

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameTime - Driver not powered\n");
        return SYS_ERROR;
    }

    if(RX_Desc->Stat & DMA_RX_OWN)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameTime - Owned by DMA\n");
        return SYS_BUSY;                        // Owned by DMA
    }

    Time->ns  = RX_Desc->TimeLo;
    Time->sec = RX_Desc->TimeHi;

    return SYS_READY;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name:     GetTX_FrameTime
//
//   Parameter(s):      Time                Pointer to time structure.
//   Return value:      SystemState_e       State of function.
//
//   Description:       Get time of transmitted Ethernet frame.
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
SystemState_e ETH_Driver::GetTX_FrameTime(ETH_MAC_Time_t* Time)
{
    TX_Descriptor *TX_Desc = &TX_Descriptor[m_MAC_Control.TX_TS_Index];

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetTX_FrameTime - Driver not powered\n");
        return SYS_ERROR;
    }

    if(TX_Desc->Stat & DMA_RX_OWN)
    {
        // Owned by DMA
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetTX_FrameTime - Owned by DMA\n");
        return SYS_BUSY;
    }

    if((TX_Desc->Stat & DMA_TX_TTSS) == 0)
    {
        // No transmit time stamp available
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetTX_FrameTime - No TX time Stamp Available\n");
        return SYS_ERROR;
    }

    Time->ns  = TX_Desc->TimeLo;
    Time->sec = TX_Desc->TimeHi;

    return SYS_READY;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name:     ControlTimer
//
//   Parameter(s):      Control             Operation
//                      Time                Pointer to time structure.
//   Return value:      SystemState_e       State of function.
//
//   Description:       Control Precision Timer.
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
SystemState_e ETH_Driver::ControlTimer(ETH_ControlTimer_e Control, ETH_MAC_Time_t* Time)
{
    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: ControlTimer - Driver not powered\n");
        return SYS_ERROR;
    }

    switch(uint32_t(Control))
    {
        case uint32_t(ETH_MAC_TIMER_GET_TIME):
        {
            // Get current time
            Time->Second     = ETH->PTPTSHR;
            Time->NanoSecond = ETH->PTPTSLR;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_SET_TIME):
        {
            // Set new time
            ETH->PTPTSHUR = Time->Second;
            ETH->PTPTSLUR = Time->NanoSecond;
            // Initialize TS time
            ETH->PTPTSCR |= ETH_PTPTSCR_TSSTI;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_INC_TIME):
        {
            // Increment current time
            ETH->PTPTSHUR = Time->Second;
            ETH->PTPTSLUR = Time->NanoSecond;

            // Time stamp system time update
            ETH->PTPTSCR |=  ETH_PTPTSCR_TSSTU;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_DEC_TIME):
        {
            // Decrement current time
            ETH->PTPTSHUR = Time->Second;
            ETH->PTPTSLUR = Time->NanoSecond | 0x80000000;

            // Time stamp system time update
            ETH->PTPTSCR |=  ETH_PTPTSCR_TSSTU;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_SET_ALARM):
        {
            // Set alarm time
            ETH->PTPTTHR  = Time->Second;
            ETH->PTPTTLR  = Time->NanoSecond;

            // Enable timestamp interrupt in PTP Control
            ETH->PTPTSCR |= ETH_PTPTSCR_TSITE;

            if((Time->Second != 0) || (Time->NanoSecond != 0)
            {
                // Enable timestamp trigger interrupt
                ETH->MACIMR &= ~uint32_t(ETH_MACIMR_TSTIM)je;
            }
            else
            {
                // Disable timestamp trigger interrupt
                ETH->MACIMR |= ETH_MACIMR_TSTIM;
            }
        }
        break;

        case uint32_t(ETH_MAC_TIMER_ADJUST_CLOCK):
        {
            // Adjust current time, fine correction
            // Correction factor is Q31 (0x80000000 = 1.000000000)
            ETH->PTPTSAR = (uint32_t)(((uint64_t)Time->NanoSecond * ETH->PTPTSAR) >> 31);
            // Fine TS clock correction
            ETH->PTPTSCR |= ETH_PTPTSCR_TSARU;
        }
        break;
    }

    return SYS_READY;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name:     Control
//
//   Parameter(s):      Control             Operation
//                      Arg                 Argument of operation (optional)
//                      Data                16-bit Data to write.
//   Return value:      SystemState_e       State of function.
//
//   Description:       Control Ethernet Interface.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::Control(ETH_ControlCode_e Control, uint32_t Arg)
{
    uint32_t maccr;
    uint32_t dmaomr;
    uint32_t macffr;

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Control - Driver not powered\n");
        return SYS_ERROR;
    }

    switch(uint32_t(Control))
    {
        case uint32_t(ETH_MAC_CONFIGURE):
        {
            maccr = ETH->MACCR & ~uint32_t(ETH_MACCR_FES | ETH_MACCR_DM | ETH_MACCR_LM | ETH_MACCR_IPCO);

            // Configure 100MBit/10MBit mode
            switch(Arg & ETH_MAC_SPEED_MASK)
            {
                case ETH_MAC_SPEED_10M:
                {
                    // RMII Half Duplex Collision detection does not work
                    maccr |= ETH_MACCR_DM;
                }
                break;

                case ETH_MAC_SPEED_100M:
                {
                    maccr |= ETH_MACCR_FES;
                }
                break;

                default:
                {
                    DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Control - Unsupported speed\n");
                    return SYS_UNSUPPORTED_FEATURE;
                }
            }

            // Configure Half/Full duplex mode
            switch(Arg & ETH_MAC_DUPLEX_MASK)
            {
                case ETH_MAC_DUPLEX_FULL:
                {
                    maccr |= ETH_MACCR_DM;
                }
                break;

                case ETH_MAC_DUPLEX_HALF:
                break;

                default:
                    DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Control - Unsupported duplex config\n");
                    return SYS_ERROR;
            }

            // Configure loopback mode
            if(Arg & ETH_MAC_LOOPBACK)
            {
                maccr |= ETH_MACCR_LM;
            }

            dmaomr = ETH->DMAOMR & ~uint32_t(ETH_DMAOMR_RSF| ETH_DMAOMR_TSF);

          #if (DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED)
            // Enable rx checksum verification
            if(Arg & ETH_MAC_CHECKSUM_OFFLOAD_RX)
            {
                maccr  |= ETH_MACCR_IPCO;
                dmaomr |= ETH_DMAOMR_RSF;
            }

            // Enable tx checksum generation
            if(Arg & ETH_MAC_CHECKSUM_OFFLOAD_TX)
            {
                dmaomr |= ETH_DMAOMR_TSF;
                m_MAC_Control.TX_ChecksumsOffload = true;
            }
            else
            {
                m_MAC_Control.TX_ChecksumsOffload = false;
            }
          #else
            if((Arg & ETH_MAC_CHECKSUM_OFFLOAD_RX) ||  (Arg & ETH_MAC_CHECKSUM_OFFLOAD_TX))
            {
                // Checksum offload is disabled in the driver
                DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: Control - Checksum offload is disabled\n");
                return SYS_ERROR;
            }
          #endif

            ETH->DMAOMR = dmaomr;
            ETH->MACCR  = maccr;

            macffr = ETH->MACFFR & ~uint32_t(ETH_MACFFR_PM | ETH_MACFFR_PAM | ETH_MACFFR_BFD);

            // Enable broadcast frame receive
            if((Arg & ETH_MAC_ADDRESS_BROADCAST) == 0)
            {
                macffr |= ETH_MACFFR_BFD;
            }

            // Enable all multi-cast frame receive
            if(Arg & ETH_MAC_ADDRESS_MULTICAST)
            {
                macffr |= ETH_MACFFR_PAM;
            }

            // Enable promiscuous mode (no filtering)
            if(Arg & ETH_MAC_ADDRESS_ALL)
            {
                macffr |= ETH_MACFFR_PM;
            }

            ETH->MACFFR = macffr;
        }
        break;

        case uint32_t(ETH_MAC_CONTROL_TX):
        {
            // Enable/disable MAC transmitter
            maccr  = ETH->MACCR  & ~ETH_MACCR_TE;
            dmaomr = ETH->DMAOMR & ~ETH_DMAOMR_ST;

            if(Arg != 0)
            {
                InitializeDMA();
                maccr  |= ETH_MACCR_TE;
                dmaomr |= ETH_DMAOMR_ST;
            }

            ETH->MACCR  = maccr;
            ETH->DMAOMR = dmaomr;
        }
        break;

        case uint32_t(ETH_MAC_CONTROL_RX):
        {
            // Enable/disable MAC receiver
            maccr  = ETH->MACCR  & ~ETH_MACCR_RE;
            dmaomr = ETH->DMAOMR & ~ETH_DMAOMR_SR;

            if(Arg != 0)
            {
                InitializeDMA();
                maccr  |= ETH_MACCR_RE;
                dmaomr |= ETH_DMAOMR_SR;
            }

            ETH->MACCR  = maccr;
            ETH->DMAOMR = dmaomr;
        }
        break;

        case uint32_t(ETH_MAC_FLUSH):
        {
            if(Arg & ETH_MAC_FLUSH_TX)
            {
                ETH->DMAOMR |= ETH_DMAOMR_FTF;          // Flush TX Buffer
            }
        }
        break;

        case uint32_t(ETH_MAC_VLAN_FILTER):
        {
            ETH->MACVLANTR = Arg;                       // Configure VLAN filter
        }
        break;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     DMA_Configure
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Configure DMA non generic config
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::DMA_Configure(void)
{
    uint32_t dmabmr;
    uint32_t dmaomr;


    #define  ETH_DMAOMR_CFG         ETH_DMAOMR_OSF             // Second Frame Operate  // do better init


    #define  ETH_DMABMR_CFG         ETH_DMABMR_AAB        |    /* Address Aligned Beats                    */ \
									ETH_DMABMR_EDE        |    /* Enhanced Descriptor format enable        */ \
									ETH_DMABMR_FB         |    /* Fixed Burst                              */ \
									ETH_DMABMR_RTPR_2_1   |    /* Arbitration Round Robin RxTx 2 1         */ \
									ETH_DMABMR_RDP_32Beat |    /* Rx DMA Burst Length 32 Beat              */ \
									ETH_DMABMR_PBL_32Beat |    /* Tx DMA Burst Length 32 Beat              */ \
									ETH_DMABMR_USP             /* Enable use of separate PBL for Rx and Tx */


    dmabmr = ETH->DMABMR & ~uint32_t(ETH_DMABMR_AAB  | ETH_DMABMR_FPM | ETH_DMABMR_USP |
                                     ETH_DMABMR_RDP  | ETH_DMABMR_FB  | ETH_DMABMR_FPM |
                                     ETH_DMABMR_RTPR | ETH_DMABMR_PBL | ETH_DMABMR_EDE |
                                     ETH_DMABMR_DSL  | ETH_DMABMR_DA);

    dmabmr |= ETH_DMABMR_CFG;
    ETH->DMABMR = dmabmr;

    dmaomr = ETH->DMAOMR & ~uint32_t(ETH_DMAOMR_DTCEFD | ETH_DMAOMR_DFRF | ETH_DMAOMR_FTF |
                                     ETH_DMAOMR_TTC    | ETH_DMAOMR_ST   | ETH_DMAOMR_FEF |
                                     ETH_DMAOMR_FUGF   | ETH_DMAOMR_RTC  | ETH_DMAOMR_OSF |
                                     ETH_DMAOMR_SR);
    dmaomr |= ETH_DMAOMR_CFG;
    ETH->DMAOMR = dmaomr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     PHY_Read
//
//   Parameter(s):      PHY_Address         5-bit  Device address
//                      RegisterAddress     5-bit  Register address.
//                      Data                16-bit Data pointer for read.
//   Return value:      SystemState_e       State of function.
//
//   Description:       Read Ethernet PHY Register through Management Interface.
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::PHY_Read(uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* Data)
{
    uint32_t RegisterValue;

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: PHY_Read - Driver not powered\n");
        return SYS_ERROR;
    }

    RegisterValue = ETH->MACMIIAR & ETH_MACMIIAR_CR;

    while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0) {/* TODO Error management*/};
    ETH->MACMIIAR = RegisterValue | ETH_MACMIIAR_MB | (uint32_t(PHY_Address) << 11) | (uint32_t(RegisterAddress) <<  6);
    while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0) {/* TODO Error management*/};
    *Data = ETH->MACMIIDR;

    return PHY_Busy();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     PHY_Write
//
//   Parameter(s):      PHY_Address         5-bit  Device address
//                      RegisterAddress     5-bit  Register address.
//                      Data                16-bit Data to write.
//   Return value:      SystemState_e       State of function.
//
//   Description:       Write Ethernet PHY Register through PHY Driver.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::PHY_Write(uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t Data)
{
    uint32_t RegisterValue;

    if((m_MAC_Control.Flags & ETH_MAC_FLAG_POWER) == 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: PHY_Write - Driver not powered\n");
        return SYS_ERROR;
    }

    while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0) {/* TODO Error management*/};
    ETH->MACMIIDR = Data;
    RegisterValue = ETH->MACMIIAR & ETH_MACMIIAR_CR;
    ETH->MACMIIAR = RegisterValue | ETH_MACMIIAR_MB | ETH_MACMIIAR_MW | (uint32_t(PHY_Address) << 11) | (uint32_t(RegisterAddress) <<  6);
    while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0) {/* TDO Error management*/};

    return PHY_Busy();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     PHY_Busy
//
//   Parameter(s):      None
//   Return value:      SystemState_e       State of function.
//
//   Description:       Wait here until PHY thru ETH is ready
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::PHY_Busy(void)
{
    TickCount_t TickStart;

	TickStart = GetTick();

    do
    {
        if((ETH->MACMIIAR & ETH_MACMIIAR_MB) == 0)
        {
            break;
        }

        nOS_Yield();
    }
    while(TickHasTimeOut(TickStart, PHY_TIMEOUT) == false);

    if((ETH->MACMIIAR & ETH_MACMIIAR_MB) == 0)
    {
        return SYS_READY;
    }

    DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: PHY_Busy - It's busy\n");
    return SYS_TIME_OUT;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     ResetMac
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Reset Ethernet MAC peripheral.
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::ResetMac(void)
{
    RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
    __asm("nop  \n\t nop  \n\t   \n\t nop   \n\t nop");
    RCC->AHB1RSTR &= ~uint32_t(RCC_AHB1RSTR_ETHMACRST);
    __asm("nop  \n\t nop  \n\t   \n\t nop   \n\t nop");
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     EnableClock
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Enable Ethernet clocks.
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::EnableClock(void)
{
    RCC->AHB1ENR |= (RCC_AHB1ENR_ETHMACEN | RCC_AHB1ENR_ETHMACTXEN | RCC_AHB1ENR_ETHMACRXEN);
  #if (DIGINI_USE_ETH_TIME_STAMP)
    RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACPTPEN;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     DisableClock
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Disable Ethernet clocks.
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::DisableClock(void)
{
    RCC->AHB1ENR &= ~uint32_t(RCC_AHB1ENR_ETHMACEN | RCC_AHB1ENR_ETHMACTXEN | RCC_AHB1ENR_ETHMACRXEN);
  #if (DIGINI_USE_ETH_TIME_STAMP)
    RCC->AHB1ENR &= ~uint32_t(RCC_AHB1ENR_ETHMACPTPEN);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     ISR_CallBack
//
//   Parameter(s):      Event           ETh type of event
//   Return value:      None
//
//   Description:       Ethernet ISR Callback.
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::ISR_CallBack(uint32_t Event)
{
    if(m_MAC_Control.CallbackEvent != nullptr)
    {
        m_MAC_Control.CallbackEvent(Event);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     ETH_IRQHandler
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Ethernet IRQ Handler.
//
//-------------------------------------------------------------------------------------------------
extern "C"
{
    NOS_ISR(ETH_IRQHandler)
    {
        uint32_t dmasr, macsr;
        uint32_t Event = ETH_MAC_EVENT_NONE;

        dmasr = ETH->DMASR;
        ETH->DMASR = dmasr & (ETH_DMASR_NIS | ETH_DMASR_RS | ETH_DMASR_TS);

        if(dmasr & ETH_DMASR_TS)
        {
            // Frame sent
            Event |= ETH_MAC_EVENT_TX_FRAME;
        }

        if(dmasr & ETH_DMASR_RS)
        {
            // Frame received
            Event |= ETH_MAC_EVENT_RX_FRAME;
        }

        macsr = ETH->MACSR;

      #if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
        if(macsr & ETH_MACSR_TSTS)
        {
            // Timestamp interrupt
            if(ETH->PTPTSSR & 2 /*ETH_PTPTSSR_TSTTR*/)
            {
                // Time stamp target time reached
                Event |= ETH_MAC_EVENT_TIMER_ALARM;
            }
        }
      #endif

        if(macsr & ETH_MACSR_PMTS)
        {
            ETH->MACPMTCSR;
            Event |= ETH_MAC_EVENT_WAKEUP;
        }


        // Callback event notification
        if(Event != ETH_MAC_EVENT_NONE)
        {
            ETH_Driver::ISR_CallBack(Event);
        }
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
