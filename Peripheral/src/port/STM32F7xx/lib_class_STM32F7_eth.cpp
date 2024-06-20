//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_eth.cpp
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

#include "./Digini/lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED) && (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//----- Timeouts -----
#define PHY_TIMEOUT             2               // PHY Register access timeout in ms
#define RESET_TIMEOUT           10

//----- TDES0 - DMA Descriptor TX Packet Control/Status -----
#define DMA_TX_OWN              0x80000000      // Own bit 1=DMA,0=CPU
#define DMA_TX_IC               0x40000000      // Interrupt on completion
#define DMA_TX_LS               0x20000000      // Last segment
#define DMA_TX_FS               0x10000000      // First segment
#define DMA_TX_DC               0x08000000      // Disable CRC
#define DMA_TX_DP               0x04000000      // Disable pad
#define DMA_TX_TTSE             0x02000000      // Transmit time stamp enable
#define DMA_TX_CIC              0x00C00000      // Checksum insertion control
#define DMA_TX_CIC_IP           0x00400000      // Checksum insertion for IP header only
#define DMA_TX_TER              0x00200000      // Transmit end of ring
#define DMA_TX_TCH              0x00100000      // Second address chained
#define DMA_TX_TTSS             0x00020000      // Transmit time stamp status
#define DMA_TX_IHE              0x00010000      // IP header error status
#define DMA_TX_ES               0x00008000      // Error summary
#define DMA_TX_JT               0x00004000      // Jabber timeout
#define DMA_TX_FF               0x00002000      // Frame flushed
#define DMA_TX_IPE              0x00001000      // IP payload error
#define DMA_TX_LC               0x00000800      // Loss of carrier
#define DMA_TX_NC               0x00000400      // No carrier
#define DMA_TX_LCOL             0x00000200      // Late collision
#define DMA_TX_EC               0x00000100      // Excessive collision
#define DMA_TX_VF               0x00000080      // VLAN frame
#define DMA_TX_CC               0x00000078      // Collision count
#define DMA_TX_ED               0x00000004      // Excessive deferral
#define DMA_TX_UF               0x00000002      // Underflow error
#define DMA_TX_DB               0x00000001      // Deferred bit

//----- TDES1 - DMA Descriptor TX Packet Control -----
#define DMA_RX_TBS2             0x1FFF0000      // Transmit buffer 2 size
#define DMA_RX_TBS1             0x00001FFF      // Transmit buffer 1 size

//----- RDES0 - DMA Descriptor RX Packet Status -----
#define DMA_RX_OWN              0x80000000      // Own bit 1=DMA,0=CPU
#define DMA_RX_AFM              0x40000000      // Destination address filter fail
#define DMA_RX_FL               0x3FFF0000      // Frame length mask
  #define DMA_RX_FL_OFFSET      16
#define DMA_RX_ES               0x00008000      // Error summary
#define DMA_RX_DE               0x00004000      // Descriptor error
#define DMA_RX_SAF              0x00002000      // Source address filter fail
#define DMA_RX_LE               0x00001000      // Length error
#define DMA_RX_OE               0x00000800      // Overflow error
#define DMA_RX_VLAN             0x00000400      // VLAN tag
#define DMA_RX_FS               0x00000200      // First descriptor
#define DMA_RX_LS               0x00000100      // Last descriptor
#define DMA_RX_IPHCE            0x00000080      // IPv4 header checksum error
#define DMA_RX_LC               0x00000040      // late collision
#define DMA_RX_FT               0x00000020      // Frame type
#define DMA_RX_RWT              0x00000010      // Receive watchdog timeout
#define DMA_RX_RE               0x00000008      // Receive error
#define DMA_RX_DRE              0x00000004      // Dribble bit error
#define DMA_RX_CE               0x00000002      // CRC error
#define DMA_RX_RMAM             0x00000001      // Rx MAC adr.match/payload cks.error

//----- RDES1 - DMA Descriptor RX Packet Control -----
#define DMA_RX_DIC              0x80000000      // Disable interrupt on completion
#define DMA_RX_RBS2             0x1FFF0000      // Receive buffer 2 size
#define DMA_RX_RER              0x00008000      // Receive end of ring
#define DMA_RX_RCH              0x00004000      // Second address chained
#define DMA_RX_RBS1             0x00001FFF      // Receive buffer 1 size

//----- Ethernet Reset register value -----
#define ETH_MACCR_RESET_VALUE   0x00008000

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

// Interrupt Handler Prototype
extern "C" void ETH_IRQHandler(void);

//-------------------------------------------------------------------------------------------------
// Variables(s)
//-------------------------------------------------------------------------------------------------

RX_Descriptor_t ETH_Driver::m_RX_Descriptor   [NUM_RX_Buffer]                                   __attribute__((aligned(4)));   // Ethernet RX & TX DMA Descriptors
TX_Descriptor_t ETH_Driver::m_TX_Descriptor   [NUM_TX_Buffer]                                   __attribute__((aligned(4)));
uint32_t        ETH_Driver::m_RX_Buffer       [NUM_RX_Buffer][ETH_BUF_SIZE / sizeof(uint32_t)]  __attribute__((aligned(4)));   // Ethernet Receive buffers
uint32_t        ETH_Driver::m_TX_Buffer       [NUM_TX_Buffer][ETH_BUF_SIZE / sizeof(uint32_t)]  __attribute__((aligned(4)));   // Ethernet Transmit buffers

ETH_MAC_Control_t ETH_Driver::m_MAC_Control;

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
    TickCount_t TickStart;

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->PMC  |= SYSCFG_PMC_MII_RMII_SEL;

    // Enable Clock
  #if (ETH_USE_TIME_STAMP)
    RCC->AHB1ENR |= (RCC_AHB1ENR_ETHMACEN | RCC_AHB1ENR_ETHMACTXEN | RCC_AHB1ENR_ETHMACRXEN | RCC_AHB1ENR_ETHMACPTPEN);
  #else
    RCC->AHB1ENR |= (RCC_AHB1ENR_ETHMACEN | RCC_AHB1ENR_ETHMACTXEN | RCC_AHB1ENR_ETHMACRXEN);
  #endif

    // Reset MAC
    RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
    __asm("nop  \n\t nop  \n\t   \n\t nop   \n\t nop");
    RCC->AHB1RSTR &= ~uint32_t(RCC_AHB1RSTR_ETHMACRST);
    __asm("nop  \n\t nop  \n\t   \n\t nop   \n\t nop");

    // Init all IO
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
    memset((void *)&m_MAC_Control, 0, sizeof(ETH_MAC_Control_t));

    m_MAC_Control.CallbackEvent = CallbackEvent;

    // Reset Ethernet MAC peripheral
    ETH->DMABMR = ETH_DMABMR_SR;

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

    ETH->MACMIIAR = ETH_MACIIAR_CR_DIVIDER;                     // MDC clock range selection

    ETH->MACCR = (ETH_MACCR_RESET_VALUE |                       // Reset value, Bit 15 must be kept at value 1
                  ETH_MACCR_FES         |                       // Fast Ethernet speed / Speed 100M
                  ETH_MACCR_DM          |                       // Full duplex
                  ETH_MACCR_RD );                               // Retry TX disabled


    // Initialize Filter registers
    ETH->MACFFR    = ETH_MACFFR_PCF_BlockAll;                   // MAC filters all control frames from reaching the application
    ETH->MACFCR    = ETH_MACFCR_ZQPD;                           // Zero-quanta pause disabled
    ETH->MACVLANTR = 0;                                         // Reset Value

    // Initialize Address registers
    ETH->MACA0HR = 0x80000000; ETH->MACA0LR = 0;
    //ETH->MACA1HR = 0; ETH->MACA1LR = 0;   // checkif it impair the behavior
    //ETH->MACA2HR = 0; ETH->MACA2LR = 0;
    //ETH->MACA3HR = 0; ETH->MACA3LR = 0;

    // Mask time stamp interrupts
    ETH->MACIMR = 0;                                            // Reset value

  #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
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

    ETH->PTPTSCR = (ETH_PTPTSSR_TSSIPV4FE |
                    ETH_PTPTSSR_TSSIPV6FE |
                    ETH_PTPTSSR_TSSSR     |
                    ETH_PTPTSCR_TSARU     |
                    ETH_PTPTSCR_TSFCU     |
                    ETH_PTPTSCR_TSE);

    m_MAC_Control.TX_TS_Index = 0;
  #endif

    // Disable MMC interrupts
    ETH->MMCTIMR = 0;
    ETH->MMCRIMR = 0;

  #if (ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED)
    ETH->DMAOMR = (ETH_DMAOMR_RSF |
                   ETH_DMAOMR_TSF |
                   ETH_DMAOMR_OSF);                             // Second Frame Operate
  #else
    ETH->DMAOMR = (ETH_DMAOMR_OSF);                             // Second Frame Operate
  #endif

    ETH->DMABMR = (ETH_DMABMR_AAB        |                      // Address Aligned Beats
                 #if ((ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED) || (ETH_USE_TIME_STAMP == DEF_ENABLED))
                   ETH_DMABMR_EDE        |                      // Enhanced Descriptor format enable
                 #endif
                   ETH_DMABMR_FB         |                      // Fixed Burst
                   ETH_DMABMR_RTPR_2_1   |                      // Arbitration Round Robin RxTx 2 1
                   ETH_DMABMR_RDP_32Beat |                      // RX DMA Burst Length 32 Beats
                   ETH_DMABMR_PBL_32Beat |                      // TX DMA Burst Length 32 Beats
                   ETH_DMABMR_USP);                             // Enable use of separate PBL for Rx and Tx

    InitializeDMA_Buffer();                                     // Initialize buffer and descriptors

    // ---- Enable ETH interrupt ----
    NVIC_ClearPendingIRQ(ETH_IRQn);
    ISR_Init(ETH_IRQn, ETH_IRQ_PRIO);

    m_MAC_Control.FrameEnd = nullptr;
    return SYS_READY;
}

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
void ETH_Driver::InitializeDMA_Buffer(void)
{
    uint32_t Next;

    // Initialize DMA Descriptors
    for(uint32_t i = 0; i < NUM_TX_Buffer; i++)
    {
        m_TX_Descriptor[i].Stat = DMA_TX_TCH | DMA_TX_LS | DMA_TX_FS;
        m_TX_Descriptor[i].Addr = (uint8_t *)&m_TX_Buffer[i];
        Next = i + 1;
        Next = (Next == NUM_TX_Buffer) ? 0 : Next;
        m_TX_Descriptor[i].Next = &m_TX_Descriptor[Next];
    }

    for(uint32_t i = 0; i < NUM_RX_Buffer; i++)
    {
        m_RX_Descriptor[i].Stat = DMA_RX_OWN;
        m_RX_Descriptor[i].Ctrl = DMA_RX_RCH | ETH_BUF_SIZE;
        m_RX_Descriptor[i].Addr = (uint8_t *)&m_RX_Buffer[i];
        Next = i + 1;
        Next = (Next == NUM_RX_Buffer) ? 0 : Next;
        m_RX_Descriptor[i].Next = &m_RX_Descriptor[Next];
    }

    ETH->DMATDLAR = (uint32_t)&m_TX_Descriptor[0];
    ETH->DMARDLAR = (uint32_t)&m_RX_Descriptor[0];
    m_MAC_Control.TX_Index = 0;
    m_MAC_Control.RX_Index = 0;

    // Enable RX interrupts
    ETH->DMAIER = ETH_DMAIER_NISE | ETH_DMAIER_RIE;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     Start
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Start the ETH mac
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::Start(void)
{
    ETH->MACCR  |= (ETH_MACCR_TE  | ETH_MACCR_RE);      // Enable MAC transmitter/Receiver
    ETH->DMAOMR |= (ETH_DMAOMR_ST |                     // Start Transmission
                    ETH_DMAOMR_SR |                     // Start Receive
                    ETH_DMAOMR_FTF);                    // Flush TX Buffer
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
SystemState_e ETH_Driver::GetMacAddress(IP_MAC_Address_t* pMAC_Address)
{
    uint32_t RegisterValue;

    if(pMAC_Address == nullptr)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetMacAddress - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
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
SystemState_e ETH_Driver::SetMacAddress(const IP_MAC_Address_t* pMAC_Address)
{
    if(pMAC_Address == nullptr)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SetMacAddress - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
    }

    // Set Ethernet MAC Address registers
    ETH->MACA0LR = (uint32_t(pMAC_Address->Byte[0])       |
                    uint32_t(pMAC_Address->Byte[1]) << 8  |
                    uint32_t(pMAC_Address->Byte[2]) << 16 |
                    uint32_t(pMAC_Address->Byte[3]) << 24);

    ETH->MACA0HR = (0x1UL << 31)                          |
                   (uint32_t(pMAC_Address->Byte[4])       |
                    uint32_t(pMAC_Address->Byte[5]) << 8);

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
SystemState_e ETH_Driver::SetAddressFilter(const IP_MAC_Address_t* pMAC_Address, uint32_t NbAddress)
{
    uint32_t CRC_Value;
    CRC_Calc myCrc(CRC_32);

    if((pMAC_Address == nullptr) && (NbAddress != 0))
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: SetAddressFilter - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
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

  #if(ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED)
    /*  The following is a workaround for MAC Control silicon problem:
            "Incorrect layer 3 (L3) checksum is inserted in the transmitted IPV6 fragmented packets
             without TCP, UDP or ICMP payloads."
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
  #endif

    Control &= ~uint32_t(DMA_TX_IC | DMA_TX_TTSE);

    if(Flags & ETH_MAC_TX_FRAME_EVENT)
    {
        Control |= DMA_TX_IC;
    }

  #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
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
//   Parameter(s):      MemoryNode*     pPacket         Pointer to on MemoryNode NanoIP data.
//                      size_t          Length          Frame buffer length in bytes.
//   Return value:      SystemState_e                   State of function.
//
//   Description:       Read data of received Ethernet frame.
//
//   Note(s):           It is assume that:
//                          1 - Length is not 0, so 'length' lower or equal to ETHERNET_FRAME_SIZE.
//                          2 - No MemoryNode was allocated if packet is invalid.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::ReadFrame(MemoryNode* pPacket, size_t Length)
{
    SystemState_e State = SYS_READY;
    uint8_t const* pSrc = m_RX_Descriptor[m_MAC_Control.RX_Index].Addr;
    size_t NodeSize;
    uint8_t* pNodeData;

    if(pPacket == nullptr)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: ReadFrame - Invalid Parameter\n");
        State = SYS_INVALID_PARAMETER;
    }
    else
    {
        NodeSize = pPacket->GetNodeSize();
        pPacket->Begin();

        do
        {
            if(Lenght < NodeSize)
            {
                NodeSize = Length;
            }

            Lenght -= NodeSize;

            pNodeData = static_cast<uint8_t*>(pPacket->GetNext());
            memcpy(pNodeData, pSrc, NodeSize);                         // LIB_FastMemcpy(pSrc, pFrame, NodeSize);
        }
        while(Length > 0)
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

    if((Stat & DMA_RX_OWN) != 0)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameSize - Owned by DMA\n");
        return ETH_OWNED_BY_DMA;                       // Owned by DMA
    }

    if(((Stat & DMA_RX_ES) != 0) ||
       ((Stat & DMA_RX_FS) == 0) ||
       ((Stat & DMA_RX_LS) == 0))
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameSize - This block is invalid\n");
        return ETH_INVALID_BLOCK;                       // Error, this block is invalid
    }

    return ((Stat & DMA_RX_FL) >> DMA_RX_FL_OFFSET) - 4;
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
#if (ETH_USE_TIME_STAMP == DEF_ENABLED)
SystemState_e ETH_Driver::GetRX_FrameTime(ETH_MAC_Time_t* pTime)
{
    RX_Descriptor* RX_Desc = &RX_Descriptor[m_MAC_Control.RX_Index];

    if(RX_Desc->Stat & DMA_RX_OWN)
    {
        DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameTime - Owned by DMA\n");
        return SYS_BUSY;                        // Owned by DMA
    }

    pTime->ns  = RX_Desc->TimeLo;
    pTime->sec = RX_Desc->TimeHi;

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
#if (ETH_USE_TIME_STAMP == DEF_ENABLED)
SystemState_e ETH_Driver::GetTX_FrameTime(ETH_MAC_Time_t* pTime)
{
    TX_Descriptor *TX_Desc = &TX_Descriptor[m_MAC_Control.TX_TS_Index];

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

    pTime->ns  = TX_Desc->TimeLo;
    pTime->sec = TX_Desc->TimeHi;

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
#if (ETH_USE_TIME_STAMP == DEF_ENABLED)
SystemState_e ETH_Driver::ControlTimer(ETH_ControlTimer_e Control, ETH_MAC_Time_t* pTime)
{
    switch(uint32_t(Control))
    {
        case uint32_t(ETH_MAC_TIMER_GET_TIME):
        {
            // Get current time
            pTime->Second     = ETH->PTPTSHR;
            pTime->NanoSecond = ETH->PTPTSLR;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_SET_TIME):
        {
            // Set new time
            ETH->PTPTSHUR = pTime->Second;
            ETH->PTPTSLUR = pTime->NanoSecond;
            // Initialize TS time
            ETH->PTPTSCR |= ETH_PTPTSCR_TSSTI;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_INC_TIME):
        {
            // Increment current time
            ETH->PTPTSHUR = pTime->Second;
            ETH->PTPTSLUR = pTime->NanoSecond;

            // Time stamp system time update
            ETH->PTPTSCR |=  ETH_PTPTSCR_TSSTU;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_DEC_TIME):
        {
            // Decrement current time
            ETH->PTPTSHUR = pTime->Second;
            ETH->PTPTSLUR = pTime->NanoSecond | 0x80000000;

            // Time stamp system time update
            ETH->PTPTSCR |=  ETH_PTPTSCR_TSSTU;
        }
        break;

        case uint32_t(ETH_MAC_TIMER_SET_ALARM):
        {
            // Set alarm time
            ETH->PTPTTHR  = pTime->Second;
            ETH->PTPTTLR  = pTime->NanoSecond;

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
            ETH->PTPTSAR = (uint32_t)(((uint64_t)pTime->NanoSecond * ETH->PTPTSAR) >> 31);
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
//   Function name:     PHY_Read
//
//   Parameter(s):      PHY_Address         5-bit  Device address
//                      RegisterAddress     5-bit  Register address.
//                      pData               16-bit Data pointer for read.
//   Return value:      SystemState_e       State of function.
//
//   Description:       Read Ethernet PHY Register through Management Interface.
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::PHY_Read(uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* pData)
{
    uint32_t RegisterValue;

    RegisterValue = ETH->MACMIIAR & ETH_MACMIIAR_CR;

    while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0) {/* TODO Error management*/};
    ETH->MACMIIAR = RegisterValue | ETH_MACMIIAR_MB | (uint32_t(PHY_Address) << 11) | (uint32_t(RegisterAddress) <<  6);
    while((ETH->MACMIIAR & ETH_MACMIIAR_MB) != 0) {/* TODO Error management*/};
    *pData = ETH->MACMIIDR;

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
    TickCount_t TickStart = GetTick();

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
        uint32_t Register;
        uint32_t Event = ETH_MAC_EVENT_NONE;

        Register = ETH->DMASR;
        ETH->DMASR = Register & (ETH_DMASR_NIS | ETH_DMASR_RS | ETH_DMASR_TS);

        if(Register & ETH_DMASR_TS)
        {
            // Frame sent
            Event |= ETH_MAC_EVENT_TX_FRAME;
        }

        if(Register & ETH_DMASR_RS)
        {
            // Frame received
            Event |= ETH_MAC_EVENT_RX_FRAME;
        }

        Register = ETH->MACSR;

      #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
        if(Register & ETH_MACSR_TSTS)
        {
            // Timestamp interrupt
            if(ETH->PTPTSSR & 2 /*ETH_PTPTSSR_TSTTR*/)
            {
                // Time stamp target time reached
                Event |= ETH_MAC_EVENT_TIMER_ALARM;
            }
        }
      #endif

        if(Register & ETH_MACSR_PMTS)
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

#endif // (USE_ETH_DRIVER == DEF_ENABLED) && (DIGINI_USE_ETHERNET == DEF_ENABLED)

