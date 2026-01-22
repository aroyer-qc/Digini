//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_eth.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#define LIB_ETH_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  LIB_ETH_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_ETH_DRIVER == DEF_ENABLED) && (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//----- Timeouts -----
#define DRIVER_PHY_RESET_DELAY  4095
#define DRIVER_PHY_TIMEOUT      2000            // PHY Register access timeout in mSec ( ST in their HAL it is 65.5 Sec)
#define DRIVER_RESET_TIMEOUT    10

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

//----- Ethernet Fixed PTPTSSR register -----
#define ETH_PTPT_SSR_TSTTR      ((uint32_t)0x00000002)  // Time stamp target time reached
#define ETH_PTPT_SSR_TSSO       ((uint32_t)0x00000001)  // Time stamp seconds overflow

#define ETH_MACAxHR_AE          0x80000000

#define ETH_DMASR_ALL_FLAGS     (ETH_DMASR_TS   | ETH_DMASR_TPSS | ETH_DMASR_TBUS | ETH_DMASR_TJTS | \
                                 ETH_DMASR_ROS  | ETH_DMASR_TUS  | ETH_DMASR_RS   | ETH_DMASR_RBUS | \
                                 ETH_DMASR_RPSS | ETH_DMASR_RWTS | ETH_DMASR_ETS  | ETH_DMASR_FBES | \
                                 ETH_DMASR_ERS  | ETH_DMASR_AIS  | ETH_DMASR_NIS)
//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

// Interrupt Handler Prototype
extern "C" void ETH_IRQHandler(void);

//-------------------------------------------------------------------------------------------------
// Variables(s)
//-------------------------------------------------------------------------------------------------

RX_Descriptor_t   ETH_Driver::m_RX_Descriptor   [NUM_RX_Buffer] __attribute__((aligned(4)));   // Ethernet RX & TX DMA Descriptors
TX_Descriptor_t   ETH_Driver::m_TX_Descriptor   [NUM_TX_Buffer] __attribute__((aligned(4)));
ETH_Control_t     ETH_Driver::m_Control;

//-------------------------------------------------------------------------------------------------
//
//   Function name:     ETH_SetBitRegister
//
//   Parameter(s):      volatile uint32_t*  pRegister
//                      uint32_t            Value
//   Return value:      None
//
//   Description:       Support function, for Errata2.16.6 STM32F427/437xx and STM32F429/439xx
//                      Write to a register might not be fully taken into account if a previous
//                      write to the same register is performed within a time period of four
//                      TX_CLK/RX_CLK clock cycles.
//
// maybe enable offload checksum and RSF and TSF
//-------------------------------------------------------------------------------------------------
void ETH_SetBitRegister(volatile uint32_t* pRegister, uint32_t Value)
{
    *pRegister |= Value;
    Value = *pRegister;
    nOS_Sleep(1);
    *pRegister = Value;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     Initialize
//
//   Parameter(s):      void*           pContext            Pointer on context for callback
//                      uint32_t        PHY_Address         PHY Address
//   Return value:      SystemState_e                       State of function.
//
//   Description:       Initialize Ethernet MAC Device.
//
// maybe enable offload checksum and RSF and TSF
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::Initialize(void* pContext, uint8_t PHY_Address)
{
    TickCount_t TickStart;

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;                       // Enable SYSCFG clock

  #if (ETH_INTERFACE_RMII == 0)
	SYSCFG->PMC |=  SYSCFG_PMC_MII_RMII_SEL;
  #else
	SYSCFG->PMC &= ~SYSCFG_PMC_MII_RMII_SEL;
  #endif

    memset((void *)&m_Control, 0, sizeof(ETH_Control_t));       // Clear Control Structure
    m_pContext = pContext;                                      // Save context (pointer on ethernetif class)

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

    ETH->DMABMR = ETH_DMABMR_SR;                                    // Reset Ethernet MAC peripheral

    TickStart = GetTick();                                          // Wait for software reset
    do
    {
        if((ETH->DMABMR & ETH_DMABMR_SR) == 0)
        {
            break;
        }

        nOS_Sleep(1);
    }
    while(TickHasTimeOut(TickStart, DRIVER_RESET_TIMEOUT) == false);

    ETH->MACMIIAR = ETH_MACIIAR_CR_DIVIDER;                         // MDC clock range selection

	if(ETH_Driver::PHY_Write(PHY_Address, REG_BCR, BCR_RESET) == SYS_READY)
	{
        nOS_Sleep(DRIVER_PHY_RESET_DELAY);

        ETH->MACCR = (ETH_MACCR_RESET_VALUE |                       // Reset value, Bit 15 must be kept at value 1
                      ETH_MACCR_FES         |                       // Fast Ethernet speed / Speed 100M
                      ETH_MACCR_DM          |                       // Full duplex
                      ETH_MACCR_RD );                               // Retry TX disabled

        // Initialize Filter registers
        ETH->MACFFR = (ETH_MACFFR_PCF_BlockAll |                    // MAC filters all control frames from reaching the application
                       ETH_MACFFR_DAIF);                            // Destination Address Inverse Filtering

        SET_BIT(ETH->MACA1HR, ETH_MACA1HR_AE);                      // Enable MACA1 for the default register multicast value 0xFF FF FF FF FF FF

        ETH->MACFCR = ETH_MACFCR_ZQPD;                              // Zero-quanta pause disabled

        SET_BIT(ETH->MACVLANTR, ETH_MACVLANTR_VLANTC);              // Drop VLAN frames

      #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
        // Set clock accuracy to 20ns (50MHz) or 50ns (20MHz)
       #if (SYS_CPU_CORE_CLOCK_FREQUENCY >= 51000000)
        ETH->PTPSSIR = 20;
        ETH->PTPTSAR = (50000000ull << 32) / SYS_CPU_CORE_CLOCK_FREQUENCY;
       #else
        ETH->PTPSSIR = 50;
        ETH->PTPTSAR = (20000000ull << 32) / SYS_CPU_CORE_CLOCK_FREQUENCY;
       #endif

        ETH->PTPTSCR = (ETH_PTPTSSR_TSSIPV4FE |
                        ETH_PTPTSSR_TSSIPV6FE |
                        ETH_PTPTSSR_TSSSR     |
                        ETH_PTPTSCR_TSARU     |
                        ETH_PTPTSCR_TSFCU     |
                        ETH_PTPTSCR_TSE);
      #endif

      #if (ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED)
        ETH_SetBitRegister(&ETH->DMAOMR, ETH_DMAOMR_RSF | ETH_DMAOMR_TSF | ETH_DMAOMR_OSF);                             // Second Frame Operate
      #else
        ETH_SetBitRegister(&ETH->DMAOMR, ETH_DMAOMR_OSF);                               // Second Frame Operate
      #endif

        ETH_SetBitRegister(&ETH->DMABMR,
                           ETH_DMABMR_AAB        |                      // Address Aligned Beats
                     #if ((ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED) || (ETH_USE_TIME_STAMP == DEF_ENABLED))
                           ETH_DMABMR_EDE        |                      // Enhanced Descriptor format enable
                     #endif
                           ETH_DMABMR_FB         |                      // Fixed Burst
                           ETH_DMABMR_RTPR_2_1   |                      // Arbitration Round Robin RxTx 2 1
                           ETH_DMABMR_RDP_32Beat |                      // RX DMA Burst Length 32 Beats
                           ETH_DMABMR_PBL_32Beat |                      // TX DMA Burst Length 32 Beats
                           ETH_DMABMR_USP);                             // Enable use of separate PBL for Rx and Tx

        InitializeDMA_Buffer();                             		    // Initialize buffer and descriptors
		return SYS_READY;
	}

    return SYS_FAIL;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     InitializeInterface
//
//   Parameter(s):      None
//   Return value:      SystemState_e                       State of function.
//
//   Description:       Initialize Ethernet Interface..
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::InitializeInterface(void)
{
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     InitializeDMA_Buffer
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       RX DMA descriptors and TX DMA descriptors.
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::InitializeDMA_Buffer(void)
{
    uint32_t Next;

    // Initialize TX DMA Descriptors
    for(uint32_t i = 0; i < NUM_TX_Buffer; i++)
    {
        m_TX_Descriptor[i].Status         = DMA_TX_TCH | DMA_TX_LS | DMA_TX_FS;
        m_TX_Descriptor[i].BufferAddress  = uint32_t(nullptr);//  no allocation here.. (uint32_t)pMemoryPool->Alloc(ETH_BUF_SIZE, MEM_DBG_ETHDMATX);
        Next = i + 1;
        Next = (Next == NUM_TX_Buffer) ? 0 : Next;
        m_TX_Descriptor[i].NextDescriptor = &m_TX_Descriptor[Next];
    }

    // Initialize RX DMA Descriptors
    for(uint32_t i = 0; i < NUM_RX_Buffer; i++)
    {
        m_RX_Descriptor[i].Status            = DMA_RX_OWN;
        m_RX_Descriptor[i].ControlBufferSize = /*DMA_RX_DIC |*/ DMA_RX_RCH | ETH_BUF_SIZE;
        m_RX_Descriptor[i].BufferAddress     = (uint32_t)pMemoryPool->Alloc(ETH_BUF_SIZE, MEM_DBG_ETHDMARX1);
        Next = i + 1;
        Next = (Next == NUM_RX_Buffer) ? 0 : Next;
        m_RX_Descriptor[i].NextDescriptor    = &m_RX_Descriptor[Next];
    }

    ETH->DMATDLAR      = (uint32_t)&m_TX_Descriptor[0];
    ETH->DMARDLAR      = (uint32_t)&m_RX_Descriptor[0];
    m_Control.TX_HeadIndex = 0;
    m_Control.TX_TailIndex = 0;
    m_Control.RX_Index     = 0;
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
    ETH_SetBitRegister(&ETH->MACCR, 0);
    ETH_SetBitRegister(&ETH->DMAOMR, ETH_DMAOMR_ST | ETH_DMAOMR_SR | ETH_DMAOMR_FTF);       // Start DMA TX/RX
    ETH_SetBitRegister(&ETH->MACCR, ETH_MACCR_TE | ETH_MACCR_RE);                           // Enable MAC transmitter/receiver
    ISR_ClearPendingIRQ(ETH_IRQn);
    ISR_Init(ETH_IRQn, ETH_IRQ_PRIO);                                                       // Enable NVIC interrupt (CPU side ready)
    ETH->DMAIER = (ETH_DMAIER_NISE | ETH_DMAIER_RIE  | ETH_DMAIER_TIE | ETH_DMAIER_FBEIE |  // Enable DMA interrupts (peripheral side ready)
                                     ETH_DMAIER_AISE | ETH_DMAIER_RBUIE);
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
    if(pMAC_Address == nullptr)
    {
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: GetMacAddress - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
    }

    *((uint16_t*)&pMAC_Address->Byte[4]) = uint16_t(ETH->MACA0HR);
    *((uint32_t*)&pMAC_Address->Byte[0]) = uint32_t(ETH->MACA0LR);

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
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: SetMacAddress - Invalid Parameter\n");
        return SYS_INVALID_PARAMETER;
    }

    // Set Ethernet MAC Address registers
    ETH->MACA0HR = ETH_MACAxHR_AE | uint16_t(*(uint16_t*)&pMAC_Address->Byte[4]);
    ETH->MACA0LR = *(uint32_t*)&pMAC_Address->Byte[0];

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
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: SetAddressFilter - Invalid Parameter\n");
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

    // Set Ethernet MAC Address registers
    ETH->MACA1HR = ETH_MACAxHR_AE | uint16_t(*(uint16_t*)&pMAC_Address->Byte[4]);
    ETH->MACA1LR = *(uint32_t*)&pMAC_Address->Byte[0];
    NbAddress--;

    if(NbAddress == 0)
    {
        ETH->MACA2HR = 0; ETH->MACA2LR = 0;
        ETH->MACA3HR = 0; ETH->MACA3LR = 0;
        return SYS_READY;
    }

    pMAC_Address++;
    ETH->MACA2HR = ETH_MACAxHR_AE | uint16_t(*(uint16_t*)&pMAC_Address->Byte[4]);
    ETH->MACA2LR = *(uint32_t*)&pMAC_Address->Byte[0];
    NbAddress--;

    if(NbAddress == 0)
    {
        ETH->MACA3HR = 0; ETH->MACA3LR = 0;
        return SYS_READY;
    }

    pMAC_Address++;
    ETH->MACA3HR = ETH_MACAxHR_AE | uint16_t(*(uint16_t*)&pMAC_Address->Byte[4]);
    ETH->MACA3LR = *(uint32_t*)&pMAC_Address->Byte[0];
    NbAddress--;

    if(NbAddress != 0)
    {
        pMAC_Address++;

        // Calculate 64-bit hash table for remaining MAC addresses
        for(; NbAddress; pMAC_Address++, NbAddress--)
        {
            CRC_Value = myCrc.CalculateBuffer(&pMAC_Address->Byte[0], 6) >> 26;

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
//
//   Return value:      SystemState_e   State of function.
//
//   Description:       Send Ethernet frame.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::SendTX_Packet(IP_PacketMsg_t** ppPacketMsg)
{
    // Validate input parameters
    if((ppPacketMsg == nullptr) || (*ppPacketMsg == nullptr) || ((*ppPacketMsg)->pPacket == nullptr) || ((*ppPacketMsg)->PacketSize == 0))
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        DBG_TX_Drop++;
      #endif
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: SendTX_Packet - Invalid PacketMsg\n");
        return SYS_INVALID_PARAMETER;
    }

    IP_PacketMsg_t* pMsg    = *ppPacketMsg;
    uint8_t*        pBuffer = reinterpret_cast<uint8_t*>(pMsg->pPacket);
    size_t          Length  = pMsg->PacketSize;

    // Zero-copy: use the buffer directly, no memcpy involved
    // Only one buffer per frame; fragmentation flags are ignored in zero-copy mode

    if(m_TX_Descriptor[m_Control.TX_HeadIndex].Status & DMA_TX_OWN)                                             // Check if the current TX descriptor is free
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        DBG_TX_Drop++;
      #endif
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: SendTX_Packet - TX Busy\n");
        return SYS_BUSY;
    }

    // Attach the packet buffer directly to the DMA descriptor
    m_TX_Descriptor[m_Control.TX_HeadIndex].BufferAddress      = reinterpret_cast<uint32_t>(pBuffer);
    m_TX_Descriptor[m_Control.TX_HeadIndex].ControlBufferSize  = Length;
    m_TX_Descriptor[m_Control.TX_HeadIndex].pMessage           = pMsg;                                          // Store the message pointer for later freeing
    uint32_t Control = (m_TX_Descriptor[m_Control.TX_HeadIndex].Status & ~uint32_t(DMA_TX_CIC)) | DMA_TX_IC;    // Prepare descriptor control flags

#if (ETH_USE_CHECKSUM_OFFLOAD == DEF_ENABLED)
    //  The following is a workaround for MAC Control silicon problem:
    //      "Incorrect layer 3 (L3) checksum is inserted in the transmitted IPV6 fragmented packets
    //       without TCP, UDP or ICMP payloads."
    //  Description:
    //      The application provides the per-frame control to instruct the MAC to insert the L3
    //      checksums for TCP, UDP and ICMP packets. When an automatic checksum insertion is
    //      enabled and the input packet is an IPv6 packet without the TCP, UDP or ICMP payload, then
    //      the MAC may incorrectly insert a checksum into the packet. For IPv6 packets without a TCP,
    //      UDP or ICMP payload, the MAC core considers the next header (NH) field as the extension
    //      header and continues to parse the extension header. Sometimes, the payload data in such
    //      packets matches the NH field for TCP, UDP or ICMP and, as a result, the MAC core inserts
    //      a checksum.

    // Read protocol and fragmentation fields for checksum offload workaround
    uint16_t Prot = UNALIGNED_UINT16_READ(&pBuffer[12]);
    uint16_t Frag = UNALIGNED_UINT16_READ(&pBuffer[20]);

    if((Prot == 0x0008) && (Frag & 0xFF3F))                                                 // Apply silicon workaround for IPv6 fragmented packets without L4 payload
    {
        Control |= DMA_TX_CIC_IP;                                                           // Insert only IP header checksum
    }
    else
    {
        Control |= DMA_TX_CIC;                                                              // Insert full checksum (IP + L4)
    }
#endif

#if (ETH_USE_TIME_STAMP == DEF_ENABLED)
    Control |= DMA_TX_TTSE;
    m_Control.TX_TS_Index = m_Control.TX_HeadIndex;
#endif

    // NOTE: In zero-copy mode, the TX IRQ must free the packet buffer.
    // The driver must store pMsg in a user field of the descriptor.
    // (This field must be added to your descriptor structure.)
    m_TX_Descriptor[m_Control.TX_HeadIndex].Status = Control | DMA_TX_OWN;                      // Give ownership of the descriptor to the DMA
    m_Control.TX_HeadIndex++;                                                                   // Advance TX descriptor index

    if (m_Control.TX_HeadIndex == NUM_TX_Buffer)
    {
        m_Control.TX_HeadIndex = 0;
    }

    ETH->DMASR   = ETH_DMASR_TBUS;                                                          // Trigger transmission
    ETH->DMATPDR = 0;

  #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
    DBG_TX_Count++;
  #endif

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     GetRX_Packet
//
//   Parameter(s):      IP_PacketMsg_t**                ppPacketMsg
//   Return value:      SystemState_e                   State of function.
//
//   Description:       Get the RX packet data
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::GetRX_Packet(IP_PacketMsg_t** ppPacketMsg)
{
    size_t Length;

    *ppPacketMsg = nullptr;
    Length       = GetRX_FrameSize();

    if(Length == 0)
    {
        return SYS_NO_DATA;
    }

    if(Length > ETH_BUF_SIZE)               // est-ce vraiment possible ????
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        DBG_RX_Drop++;
      #endif
        m_RX_Descriptor[m_Control.RX_Index].Status = DMA_RX_OWN;                                            // Give back the buffer to the DMA without returning it
        m_Control.RX_Index++;

        if(m_Control.RX_Index == NUM_RX_Buffer)
        {
            m_Control.RX_Index = 0;
        }

        return SYS_OVERFLOW;
    }

    *ppPacketMsg = (IP_PacketMsg_t*)pMemoryPool->Alloc(sizeof(IP_PacketMsg_t), MEM_DBG_IPPKT);              // Allocated a IP_PacketMsg_t from the pool

    if(*ppPacketMsg == nullptr)
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        DBG_RX_Drop++;
      #endif
        m_RX_Descriptor[m_Control.RX_Index].Status = DMA_RX_OWN;                                            // Keep the actual buffer from the pool for the DMA descriptor (No choice)
        return SYS_POOL_NOT_ALLOCATED_ERROR;
    }

    (*ppPacketMsg)->PacketSize = Length;
    (*ppPacketMsg)->pPacket    = (IP_EthernetPacket_t*)m_RX_Descriptor[m_Control.RX_Index].BufferAddress;   // The DMA packet DMA is already from the pool (zero copy)

    void* pNewBuffer = pMemoryPool->Alloc(ETH_BUF_SIZE, MEM_DBG_ETHDMARX2);                                 // Allocate a new buffer for the DMA descriptor

    if(pNewBuffer == nullptr)
    {
      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        DBG_RX_Drop++;
      #endif

        pMemoryPool->Free((void**)ppPacketMsg);
        *ppPacketMsg = nullptr;
        m_RX_Descriptor[m_Control.RX_Index].Status = DMA_RX_OWN;                                            // Keep the actual buffer for DMA descriptor
        return SYS_POOL_NOT_ALLOCATED_ERROR;
    }

    m_RX_Descriptor[m_Control.RX_Index].BufferAddress = (uint32_t)pNewBuffer;
    m_RX_Descriptor[m_Control.RX_Index].Status        = DMA_RX_OWN;

    m_Control.RX_Index++;
    if(m_Control.RX_Index == NUM_RX_Buffer)
    {
        m_Control.RX_Index = 0;
    }

    if(ETH->DMASR & ETH_DMASR_RBUS)
    {
        ETH->DMASR   = ETH_DMASR_RBUS;
        ETH->DMARPDR = 0;
    }

  #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
    DBG_RX_Count++;
  #endif

    return SYS_READY;
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
    uint32_t Status = m_RX_Descriptor[m_Control.RX_Index].Status;

    // till owned by DMA: no frame available
    if (Status & DMA_RX_OWN)
    {
        return ETH_OWNED_BY_DMA;
    }

    // Basic validity checks: FS (first segment), LS (last segment), ES (error summary)
    if(((Status & DMA_RX_ES) != 0)  ||        // Error summary
       ((Status & DMA_RX_FS) == 0)  ||        // Not first segment
       ((Status & DMA_RX_LS) == 0))           // Not last segment
    {
        return ETH_INVALID_BLOCK;
    }

    // Extract frame length from descriptor
    uint32_t Length = (Status & DMA_RX_FL) >> DMA_RX_FL_OFFSET;

    // Sanity checks on length
    //    Ethernet minimum: 14 bytes header + payload
    //    Maximum: 1518 (Ethernet II) or 1522 (with VLAN)
    if((Length < 14) || (Length > 1522))
    {
        return ETH_INVALID_BLOCK;
    }

    // Remove CRC (4 bytes) if present
    //    STM32F4 always includes CRC in FL
    Length -= 4;  // i will need to put that into a #ifdef if we do crc by the module

    // Final safety check: must not exceed MTU
    if(Length > 1500)
    {
        return ETH_INVALID_BLOCK;
    }

    return Length;
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
    RX_Descriptor* RX_Desc = &RX_Descriptor[m_Control.RX_Index];

    if(RX_Desc->Status & DMA_RX_OWN)
    {
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: GetRX_FrameTime - Owned by DMA\n");
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
    TX_Descriptor *TX_Desc = &TX_Descriptor[m_Control.TX_TS_Index];

    if(TX_Desc->Status & DMA_RX_OWN)
    {
        // Owned by DMA
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: GetTX_FrameTime - Owned by DMA\n");
        return SYS_BUSY;
    }

    if((TX_Desc->Status & DMA_TX_TTSS) == 0)
    {
        // No transmit time stamp available
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: GetTX_FrameTime - No TX time Stamp Available\n");
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
                // Enable time stamp trigger interrupt
                ETH->MACIMR &= ~uint32_t(ETH_MACIMR_TSTIM)je;
            }
            else
            {
                // Disable time stamp trigger interrupt
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
    SystemState_e State;

    ETH->MACMIIAR = ETH_MACIIAR_CR_DIVIDER        |
					ETH_MACMIIAR_MB               |
                    (uint32_t(PHY_Address) << 11) |
                    (uint32_t(RegisterAddress) << 6);

    if((State = PHY_Busy()) != SYS_READY)
    {
        return State;
    }

    *pData = uint16_t(ETH->MACMIIDR);           // Only bit 0 to 15 are used in this register
    return SYS_READY;
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
    ETH->MACMIIDR = uint32_t(Data);
    ETH->MACMIIAR = ETH_MACIIAR_CR_DIVIDER        |
                    ETH_MACMIIAR_MB               |
                    ETH_MACMIIAR_MW               |
                    (uint32_t(PHY_Address) << 11) |
                    (uint32_t(RegisterAddress) << 6);

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
            return SYS_READY;
        }

        nOS_Yield();
    }
    while(TickHasTimeOut(TickStart, DRIVER_PHY_TIMEOUT) == false);

    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "ETH: PHY_Busy - It's busy\n");
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
    if(m_pContext == nullptr)
    {
        return;
    }

    ETH_IF_Driver::CallbackWrapper(m_pContext, Event);

    if(Event & ETH_MAC_EVENT_TX_FRAME)
    {
        uint8_t Index = m_Control.TX_TailIndex;

        // Process all descriptors between tail and head
        while((Index != m_Control.TX_HeadIndex) && (m_TX_Descriptor[Index].Status & DMA_TX_OWN) == 0)
        {
            IP_PacketMsg_t* pMsg =
                (IP_PacketMsg_t*)m_TX_Descriptor[Index].pMessage;

            if(pMsg != nullptr)
            {
                IP_Manager::FreeMessage(pMsg);
                m_TX_Descriptor[Index].pMessage = nullptr;
            }

            // Advance tail
            Index++;

            if(Index == NUM_TX_Buffer)
            {
                Index = 0;
            }
        }

        m_Control.TX_TailIndex = Index;
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

uint32_t RX_Count = 0;
uint32_t IRQ_Count = 0;

extern "C"
{
    NOS_ISR(ETH_IRQHandler)
    {
        uint32_t Register;
        uint32_t Event = ETH_MAC_EVENT_NONE;

        IO_SetPinHigh(IO_ETH_EXT_LED);

IRQ_Count++;
        Register = ETH->DMASR;
        ETH->DMASR = ETH_DMASR_ALL_FLAGS;

        if(Register & ETH_DMASR_TS)
        {
            Event |= ETH_MAC_EVENT_TX_FRAME;                        // Frame sent
        }

        if(Register & ETH_DMASR_RS)
        {
            Event |= ETH_MAC_EVENT_RX_FRAME;                        // Frame received
RX_Count++;
        }

        Register = ETH->MACSR;

      #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
        if(Register & ETH_MACSR_TSTS)
        {
            if(ETH->PTPTSSR & ETH_PTPT_SSR_TSTTR)                   // Time stamp interrupt
            {
                Event |= ETH_MAC_EVENT_TIMER_ALARM;                 // Time stamp target time reached
            }
        }
      #endif

        if(Register & ETH_MACSR_PMTS)
        {
            ETH->MACPMTCSR;
            Event |= ETH_MAC_EVENT_WAKEUP;
        }

        if(Event != ETH_MAC_EVENT_NONE)                             // Callback event notification
        {
            myETH_Driver.ISR_CallBack(Event);
        }
    }
}


//-------------------------------------------------------------------------------------------------

#endif // (USE_ETH_DRIVER == DEF_ENABLED) && (DIGINI_USE_ETHERNET == DEF_ENABLED)

