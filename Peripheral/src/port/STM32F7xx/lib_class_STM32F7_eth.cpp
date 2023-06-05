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
/*
The Driver m_EMAC_Control requires:
  - Setup of HCLK to 25MHz or higher
  - Optional setup of SYSCLK to 50MHz or higher, when Ethernet PTP is used
  - Setup of ETH in MII or RMII mode

Clock Configuration tab
-----------------------
  1. Configure HCLK Clock: HCLK (MHz) = <b>25 or higher</b>
*/

#include "lib_class_STM32F7_eth.h"
//#include "Arch/armv7e-m/mpu.h"
#include "ethernetif.h"

// Timeouts
#define PHY_TIMEOUT         2               // PHY Register access timeout in ms

// ETH Memory Buffer configuration
// define in "ethif_config.h"

// Interrupt Handler Prototype
void ETH_IRQHandler(void);

// DMA RX Descriptor
struct
{
    uint32_t volatile       Stat;
    uint32_t                Ctrl;
    uint8_t const*          Addr;
    struct RX_Descriptor*   Next;
#if ((DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED) || (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED))
    uint32_t                ExtStat;
    uint32_t                Reserved[1];
    uint32_t                TimeLo;
    uint32_t                TimeHi;
#endif
} RX_Descriptor_t;

// DMA TX Descriptor
struct
{
    uint32_t volatile       Stat;
    uint32_t                Size;
    uint8_t*                Addr;
    struct TX_Descriptor*   Next;
#if ((DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED) || (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED))
    uint32_t                Reserved[2];
    uint32_t                TimeLo;
    uint32_t                TimeHi;
#endif
} TX_Descriptor_t;

// Driver Capabilities
const ETH_MacCapabilities_t ETH_Driver::m_Capabilities =
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
    uint32_t next;

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_DMA_INIT) == 0)
    {
        m_EMAC_Control.Flags |= m_EMAC_Control_FLAG_DMA_INIT;

        #if ((DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED) || (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED))
        ETH->DMABMR |= ETH_DMABMR_EDE;                                                      // Enhanced descriptor enable
        #endif

        // Initialize DMA Descriptors
        for(uint32_t i = 0; i < NUM_TX_Buffer; i++)
        {
            TX_Descriptor[i].Stat = DMA_TX_TCH | DMA_TX_LS | DMA_TX_FS;
            TX_Descriptor[i].Addr = (uint8_t *)&TX_Buffer[i];
            next = i + 1;
        
            if(next == NUM_TX_Buffer)
            {
                next = 0;
            }
            
            TX_Descriptor[i].Next = &TX_Descriptor[next];
        }
      
        for(uint32_t i = 0; i < NUM_RX_Buffer; i++)
        {
            RX_Descriptor[i].Stat = DMA_RX_OWN;
            RX_Descriptor[i].Ctrl = DMA_RX_RCH | ETH_BUF_SIZE;
            RX_Descriptor[i].Addr = (uint8_t *)&RX_Buffer[i];
            next = i + 1;
            
            if(next == NUM_RX_Buffer)
            {
                next = 0;
            }
            
            RX_Descriptor[i].Next = &RX_Descriptor[next];
        }

        ETH->DMATDLAR = (uint32_t)&TX_Descriptor[0];
        ETH->DMARDLAR = (uint32_t)&RX_Descriptor[0];
        m_EMAC_Control.TX_Index = 0;
        m_EMAC_Control.RX_Index = 0;

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
    return m_Capability;
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
    SYSCFG->PMC |=  SYSCFG_PMC_MII_RMII_SEL;

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
    
    // Clear cControl structure
    memset ((void *)&m_EMAC_Control, 0, sizeof (m_EMAC_Control_CTRL));

    m_EMAC_Control.CallbackEvent = CallbackEvent;
    m_EMAC_Control.Flags         = m_EMAC_Control_FLAG_INIT;

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

    m_EMAC_Control.Flags &= ~m_EMAC_Control_FLAG_INIT;

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
            PRIVILEGE_EXEC(NVIC_DisableIRQ(ETH_IRQn));
            ETH->DMAIER  = 0;
            ETH->PTPTSCR = 0;
            DisableClock();
            m_EMAC_Control.Flags &= ~(m_EMAC_Control_FLAG_POWER | m_EMAC_Control_FLAG_DMA_INIT);
        }
        break;

        case ETH_POWER_LOW:
        {
            return SYS_UNSUPPORTED_FEATURE;
        }

        case ETH_POWER_FULL:
        {
            if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_INIT)  == 0)
            {
                return SYS_ERROR;                                           // Driver not initialized
            }

            if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) != 0)
            {
                break;                                                      // Driver already powered
            }

            EnableClock();
            ResetMac();
            ETH->DMABMR = ETH_DMABMR_SR;                                    // Reset Ethernet MAC peripheral

            // Wait for software reset
            uint32_t nbTest = 0;
            
            while((ETH->DMABMR & ETH_DMABMR_SR) != 0)
            {
                if(nbTest++ > 1000)
                nOS_Yield();
            }

            ETH->MACMIIAR = ETH_MACIIAR_CR_DIVIDER;                         // MDC clock range selection
            ETH->MACCR   = ETH_MACCR_RD | 0x00008000;                       // Initialize MAC configuration

            // Initialize Filter registers
            ETH->MACFFR    = ETH_MACFFR_PCF_BlockAll; //ETH_MACFFR_BFD;
            ETH->MACFCR    = ETH_MACFCR_ZQPD;
            ETH->MACVLANTR = 0;

            // Initialize Address registers
            ETH->MACA0HR = 0x8; ETH->MACA0LR = 0;
            //ETH->MACA1HR = 0; ETH->MACA1LR = 0;   // checkif it impair the behavior
            //ETH->MACA2HR = 0; ETH->MACA2LR = 0;
            //ETH->MACA3HR = 0; ETH->MACA3LR = 0;

            // Mask timestamp interrupts
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
            m_EMAC_Control.TX_TS_Index  = 0;
          #endif

            // Disable MMC interrupts
            ETH->MMCTIMR = 0; // ETH_MMCTIMR_TGFM  | ETH_MMCTIMR_TGFMSCM | ETH_MMCTIMR_TGFSCM;  // As per old driver
            ETH->MMCRIMR = 0; //ETH_MMCRIMR_RGUFM | ETH_MMCRIMR_RFAEM   | ETH_MMCRIMR_RFCEM;    // As per old driver

            NVIC_ClearPendingIRQ(ETH_IRQn);
            ISR_Init(ETH_IRQn, ETH_IRQ_PRIO);
            HAL_NVIC_SetPriority(ETH_IRQn, ETH_IRQ_PRIO, 0);
            HAL_NVIC_EnableIRQ(ETH_IRQn);

            m_EMAC_Control.FrameEnd = nullptr;
            m_EMAC_Control.Flags   |= m_EMAC_Control_FLAG_POWER;
        }
        break;

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
SystemState_e ETH_Driver::GetMacAddress(ETH_MacAddress_t* pMAC_Address)
{
    uint32_t RegisterValue;

    if(pMAC_Address == nullptr)
    {
        return SYS_INVALID_PARAMETER;
    }

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }

    RegisterValue = ETH->MACA0HR;
    pMAC_Address->b[5] = (uint8_t)(RegisterValue >>  8);
    pMAC_Address->b[4] = (uint8_t)(RegisterValue);
    RegisterValue = ETH->MACA0LR;
    pMAC_Address->b[3] = (uint8_t)(RegisterValue >> 24);
    pMAC_Address->b[2] = (uint8_t)(RegisterValue >> 16);
    pMAC_Address->b[1] = (uint8_t)(RegisterValue >>  8);
    pMAC_Address->b[0] = (uint8_t)(RegisterValue);

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
SystemState_e ETH_Driver::SetMacAddress(const ETH_MacAddress_t* pMAC_Address)
{
    if(pMAC_Address == nullptr)
    {
        return SYS_INVALID_PARAMETER;
    }

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }

    // Set Ethernet MAC Address registers
    ETH->MACA0HR = ((uint32_t)pMAC_Address->b[5] <<  8) |
                    (uint32_t)pMAC_Address->b[4];
    ETH->MACA0LR = ((uint32_t)pMAC_Address->b[3] << 24) |
                   ((uint32_t)pMAC_Address->b[2] << 16) |
                   ((uint32_t)pMAC_Address->b[1] <<  8) |
                    (uint32_t)pMAC_Address->b[0];

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
SystemState_e ETH_Driver::SetAddressFilter(const ETH_MacAddress_t *pMAC_Address, uint32_t NbAddress)
{
    uint32_t CRC_Value;
    CRC_Calc myCrc(CRC_32);

    if((pMAC_Address == nullptr) && (NbAddress != 0))
    {
        return SYS_INVALID_PARAMETER;
    }

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
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

    ETH->MACA1HR = ((uint32_t)pMAC_Address->b[5] <<  8) |  (uint32_t)pMAC_Address->b[4] | ETH_MACA1HR_AE;
    ETH->MACA1LR = ((uint32_t)pMAC_Address->b[3] << 24) | ((uint32_t)pMAC_Address->b[2] << 16) |
                   ((uint32_t)pMAC_Address->b[1] <<  8) |  (uint32_t)pMAC_Address->b[0];
    NbAddress--;
    
    if(NbAddress == 0)
    {
        ETH->MACA2HR = 0; ETH->MACA2LR = 0;
        ETH->MACA3HR = 0; ETH->MACA3LR = 0;
        return SYS_READY;
    }
    
    pMAC_Address++;
    ETH->MACA2HR = ((uint32_t)pMAC_Address->b[5] <<  8) |  (uint32_t)pMAC_Address->b[4] | ETH_MACA2HR_AE;
    ETH->MACA2LR = ((uint32_t)pMAC_Address->b[3] << 24) | ((uint32_t)pMAC_Address->b[2] << 16) |
                   ((uint32_t)pMAC_Address->b[1] <<  8) |  (uint32_t)pMAC_Address->b[0];
    NbAddress--;
    
    if(NbAddress == 0)
    {
        ETH->MACA3HR = 0; ETH->MACA3LR = 0;
        return SYS_READY;
    }
  
    pMAC_Address++;
    ETH->MACA3HR = ((uint32_t)pMAC_Address->b[5] <<  8) |  (uint32_t)pMAC_Address->b[4] | ETH_MACA3HR_AE;
    ETH->MACA3LR = ((uint32_t)pMAC_Address->b[3] << 24) | ((uint32_t)pMAC_Address->b[2] << 16) |
                   ((uint32_t)pMAC_Address->b[1] <<  8) |  (uint32_t)pMAC_Address->b[0];
    NbAddress--;
  
    if(NbAddress == 0)
    {
        return SYS_READY;
    }
    
    pMAC_Address++;

    // Calculate 64-bit Hash table for remaining MAC addresses
    for(; NbAddress; pMAC_Address++, NbAddress--)
    {
        CRC_Value = myCrc.CalculateFullBuffer(&pMAC_Address->b[0], 6) >> 26;
        
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
SystemState_e ETH_Driver::SendFrame(const uint8_t* pFrame, uint32_t Length, uint32_t Flags)
{
    uint8_t* pDst = m_EMAC_Control.FrameEnd;
    uint32_t Control;

    if((pFrame == nullptr) || (Length == 0))
    {
        return SYS_INVALID_PARAMETER;
    }

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0U)
    {
        return SYS_ERROR;
    }

    if(Destination == nullptr)
    {
        // Start of a new transmit frame
        if(TX_Descriptor[m_EMAC_Control.TX_Index].Stat & DMA_TX_OWN)
        {
            // Transmitter is busy, wait
            return SYS_BUSY;
        }
    
        Destination = TX_Descriptor[m_EMAC_Control.TX_Index].Addr;
        TX_Descriptor[m_EMAC_Control.TX_Index].Size = Length;
    }
    else
    {
        // Sending data fragments in progress
        TX_Descriptor[m_EMAC_Control.TX_Index].Size += Length;
    }
    
    LIB_FastMemcpy(pFrame, pDst, Length);

    if(Flags & ETH_MAC_TX_FRAME_FRAGMENT)
    {
        // More data to come, remember current write position
        m_EMAC_Control.FrameEnd = Destination;
        return SYS_READY;
    }

    // Frame is now ready, send it to DMA
    Control = TX_Descriptor[m_EMAC_Control.TX_Index].Stat & ~DMA_TX_CIC;
    
  #if(DIGINI_USE_ETH_CHECKSUM_OFFLOAD == DEF_ENABLED)
    if(m_EMAC_Control.TX_ChecksumsOffload)
    {
        /*  The following is a workaround for m_EMAC_Control silicon problem:
                "Incorrect layer 3 (L3) checksum is inserted in the sent IPV4 fragmented packets."
            Description:
                When automatic checksum insertion is enabled and the packet is IPV4 frame fragment,
                then the MAC may incorrectly insert checksum into the packet. This corrupts the
                payload data and generates checksum errors at the receiver.
        */
        uint16_t prot = UNALIGNED_UINT16_READ(&TX_Descriptor[m_EMAC_Control.TX_Index].Addr[12]);
        uint16_t frag = UNALIGNED_UINT16_READ(&TX_Descriptor[m_EMAC_Control.TX_Index].Addr[20]);
        
        if((prot == 0x0008) && (frag & 0xFF3F))
        {
            // Insert only IP header checksum in fragmented frame
            Control |= DMA_TX_CIC_IP;
        }
        else
        {
            // Insert IP header and payload checksums (TCP,UDP,ICMP)
            Control |= DMA_TX_CIC;
        }
    }
  #endif
    
    Control &= ~(DMA_TX_IC | DMA_TX_TTSE);
  
    if(Flags & ETH_MAC_TX_FRAME_EVENT)
    {
        Control |= DMA_TX_IC;
    }

  #if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
    if(Flags & ETH_MAC_TX_FRAME_TIMESTAMP)
    {
        Control |= DMA_TX_TTSE;
    }
    
    m_EMAC_Control.TX_TS_Index = m_EMAC_Control.TX_Index;
  #endif
  
    TX_Descriptor[m_EMAC_Control.TX_Index].Stat = Control | DMA_TX_OWN;
    m_EMAC_Control.TX_Index++;

    if(m_EMAC_Control.TX_Index == NUM_TX_Buffer)
    {
        m_EMAC_Control.TX_Index = 0;
    }
  
    m_EMAC_Control.FrameEnd = nullptr;

    // Start frame transmission
    ETH->DMASR   = ETH_DMASR_TBUS ;//ETH_DMASR_TPSS;
    ETH->DMATPDR = 0;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:     ReadFrame 
//
//   Parameter(s):      pBuf            Pointer to on pbuf LWIP data.
//                      Length          Frame buffer length in bytes.
//   Return value:      SystemState_e   State of function.
//
//   Description:       Read data of received Ethernet frame.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::ReadFrame(struct pbuf* pBuf, uint32_t Length)
{
    SystemState_e State  = SYS_READY;
    uint8_t const* pSrc  = RX_Descriptor[m_EMAC_Control.RX_Index].Addr;
    uint32_t Granularity = (uint32_t)pBuf->len;
    uint8_t* pFrame      = (uint8_t*)pBuf->payload;


    if((pBuf == nullptr) && (Length != 0))
    {
        State = SYS_INVALID_PARAMETER;
    }
    else if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        State = SYS_ERROR;
    }
    else
    {
        do
        {
            LIB_FastMemcpy(pSrc, pFrame, Granularity);
            Length -= (int32_t)pBuf->len;

            if(Length > 0)
            {
                pBuf = pBuf->next;

                if(pBuf != nullptr)
                {
                    Granularity = (int32_t)pBuf->len;
                    pFrame      = (uint8_t*)pBuf->payload;
                }
                else
                {
                    Length = 0;
                }
            }
        }
        while(Length > 0);
    }

    // Return this block back to ETH-DMA
    RX_Descriptor[m_EMAC_Control.RX_Index].Stat = DMA_RX_OWN;
    m_EMAC_Control.RX_Index++;

    if(m_EMAC_Control.RX_Index == NUM_RX_Buffer)
    {
        m_EMAC_Control.RX_Index = 0;
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
    uint32_t Stat = RX_Descriptor[m_EMAC_Control.RX_Index].Stat;

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return  0;
    }

    if(Stat & DMA_RX_OWN)
    {
        return 0;                       // Owned by DMA
    }
  
    if(((Stat & DMA_RX_ES) != 0) || ((Stat & DMA_RX_FS) == 0) || ((Stat & DMA_RX_LS) == 0))
    {
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
    RX_Descriptor* RX_Desc = &RX_Descriptor[m_EMAC_Control.RX_Index];

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }

    if(RX_Desc->Stat & DMA_RX_OWN)
    {
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
    TX_Descriptor *TX_Desc = &TX_Descriptor[m_EMAC_Control.TX_TS_Index];

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }

    if(TX_Desc->Stat & DMA_RX_OWN)
    {
        // Owned by DMA
        return SYS_BUSY;
    }
  
    if((TX_Desc->Stat & DMA_TX_TTSS) == 0)
    {
        // No transmit time stamp available
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
SystemState_e ETH_Driver::ControlTimer(uint32_t Control, ETH_MAC_Time_t* Time)
{
    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
    
    if((Control != ETH_MAC_TIMER_GET_TIME)     && 
       (Control != ETH_MAC_TIMER_SET_TIME)     && 
       (Control != ETH_MAC_TIMER_INC_TIME)     && 
       (Control != ETH_MAC_TIMER_DEC_TIME)     && 
       (Control != ETH_MAC_TIMER_SET_ALARM)    && 
       (Control != ETH_MAC_TIMER_ADJUST_CLOCK))
    {
        return SYS_INVALID_PARAMETER;
    }

    switch(Control)
    {
        case ETH_MAC_TIMER_GET_TIME:
        {
            // Get current time
            Time->sec = ETH->PTPTSHR;
            Time->ns  = ETH->PTPTSLR;
        }
        break;

        case ETH_MAC_TIMER_SET_TIME:
        {
            // Set new time
            ETH->PTPTSHUR = Time->sec;
            ETH->PTPTSLUR = Time->ns;
            // Initialize TS time
            ETH->PTPTSCR |= ETH_PTPTSCR_TSSTI;
        }
        break;

        case ETH_MAC_TIMER_INC_TIME:
        {
            // Increment current time
            ETH->PTPTSHUR = Time->sec;
            ETH->PTPTSLUR = Time->ns;

            // Time stamp system time update
            ETH->PTPTSCR |=  ETH_PTPTSCR_TSSTU;
        }
        break;

        case ETH_MAC_TIMER_DEC_TIME:
        {
            // Decrement current time
            ETH->PTPTSHUR = Time->sec;
            ETH->PTPTSLUR = Time->ns | 0x80000000U;

            // Time stamp system time update
            ETH->PTPTSCR |=  ETH_PTPTSCR_TSSTU;
        }
        break;

        case ETH_MAC_TIMER_SET_ALARM:
        {
            // Set alarm time
            ETH->PTPTTHR  = Time->sec;
            ETH->PTPTTLR  = Time->ns;

            // Enable timestamp interrupt in PTP Control
            ETH->PTPTSCR |= ETH_PTPTSCR_TSITE;

            if(time->sec || time->ns)
            {
                // Enable timestamp trigger interrupt
                ETH->MACIMR &= ~ETH_MACIMR_TSTIM;
            }
            else
            {
                // Disable timestamp trigger interrupt
                ETH->MACIMR |= ETH_MACIMR_TSTIM;
            }
        }
        break;

        case ETH_MAC_TIMER_ADJUST_CLOCK:
        {
            // Adjust current time, fine correction
            // Correction factor is Q31 (0x80000000 = 1.000000000)
            ETH->PTPTSAR = (uint32_t)(((uint64_t)Time->ns * ETH->PTPTSAR) >> 31);
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
SystemState_e ETH_Driver::Control(uint32_t Control, uint32_t Arg)
{
    uint32_t maccr;
    uint32_t dmaomr;
    uint32_t dmabmr;
    uint32_t macffr;

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
  
    if((Control != ETH_MAC_CONFIGURE)   &&
       (Control != ETH_DMA_CONFIGURE)   &&
       (Control != ETH_MAC_CONTROL_TX)  &&
       (Control != ETH_MAC_CONTROL_RX)  && 
       (Control != ETH_MAC_FLUSH)       && 
       (Control != ETH_MAC_SLEEP)       && 
       (Control != ETH_MAC_VLAN_FILTER))
    {
        return SYS_INVALID_PARAMETER;
    }

    switch(Control)
    {
        case ETH_MAC_CONFIGURE:
        {
            maccr = ETH->MACCR & ~(ETH_MACCR_FES | ETH_MACCR_DM | ETH_MACCR_LM | ETH_MACCR_IPCO);

            // Configure 100MBit/10MBit mode
            switch(arg & ETH_MAC_SPEED_Msk)
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
                    return SYS_UNSUPPORTED_FEATURE;
                }
            }

            // Confige Half/Full duplex mode
            switch(Arg & ETH_MAC_DUPLEX_Msk)
            {
                case ETH_MAC_DUPLEX_FULL:
                {
                    maccr |= ETH_MACCR_DM;
                }
                break;
        
                case ETH_MAC_DUPLEX_HALF:
                break;
        
                default:
                    return SYS_ERROR;
            }

            // Configure loopback mode
            if(Arg & ETH_MAC_LOOPBACK)
            {
                maccr |= ETH_MACCR_LM;
            }

            dmaomr = ETH->DMAOMR & ~(ETH_DMAOMR_RSF| ETH_DMAOMR_TSF);

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
                m_EMAC_Control.TX_ChecksumsOffload = true;
            }
            else
            {
                m_EMAC_Control.TX_ChecksumsOffload = false;
            }
          #else
            if((Arg & ETH_MAC_CHECKSUM_OFFLOAD_RX) ||  (Arg & ETH_MAC_CHECKSUM_OFFLOAD_TX))
            {
                // Checksum offload is disabled in the driver
                return SYS_ERROR;
            }
          #endif
      
            ETH->DMAOMR = dmaomr;
            ETH->MACCR  = maccr;

            macffr = ETH->MACFFR & ~(ETH_MACFFR_PM | ETH_MACFFR_PAM | ETH_MACFFR_BFD);
        
            // Enable broadcast frame receive
            if((Arg & ETH_MacAddress_tESS_BROADCAST) == 0)
            {
                macffr |= ETH_MACFFR_BFD;
            }

            // Enable all multicast frame receive
            if(Arg & ETH_MacAddress_tESS_MULTICAST)
            {
                macffr |= ETH_MACFFR_PAM;
            }

            // Enable promiscuous mode (no filtering)
            if(Arg & ETH_MacAddress_tESS_ALL)
            {
                macffr |= ETH_MACFFR_PM;
            }
        
            ETH->MACFFR = macffr;
        }
        break;

        case ETH_DMA_CONFIGURE:
        {
            if(Arg & ETH_DMABMR_REG)
            {
                dmabmr = ETH->DMABMR &~(ETH_DMABMR_AAB  | ETH_DMABMR_FPM | ETH_DMABMR_USP |
                                        ETH_DMABMR_RDP  | ETH_DMABMR_FB  | ETH_DMABMR_FPM |
                                        ETH_DMABMR_RTPR | ETH_DMABMR_PBL | ETH_DMABMR_EDE |
                                        ETH_DMABMR_DSL  | ETH_DMABMR_DA);

                dmabmr |= (arg & ~ETH_DMABMR_REG);
                ETH->DMABMR = dmabmr;
            }
        
            if(Arg & ETH_DMAOMR_REG)
            {
                dmaomr = ETH->DMAOMR &~(ETH_DMAOMR_DTCEFD | ETH_DMAOMR_DFRF | ETH_DMAOMR_FTF |
                                        ETH_DMAOMR_TTC    | ETH_DMAOMR_ST   | ETH_DMAOMR_FEF |
                                        ETH_DMAOMR_FUGF   | ETH_DMAOMR_RTC  | ETH_DMAOMR_OSF |
                                        ETH_DMAOMR_SR);
                dmaomr |= (arg & ~ETH_DMAOMR_REG);
                ETH->DMAOMR = dmaomr;
            }
        }
        break;

        case ETH_MAC_CONTROL_TX:
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

        case ETH_MAC_CONTROL_RX:
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

        case ETH_MAC_FLUSH:
        {
            if(Arg & ETH_MAC_FLUSH_TX)
            {
                ETH->DMAOMR |= ETH_DMAOMR_FTF;          // Flush TX Buffer
            }
        }
        break;

        case ETH_MAC_VLAN_FILTER:
        {
            ETH->MACVLANTR = arg;                       // Configure VLAN filter
        }
        break;
    }
  
    return SYS_READY;
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
SystemState_e ETH_Driver::PHY_Read(uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t *Data)
{
    uint32_t RegisterValue;

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }

    RegisterValue = ETH->MACMIIAR & ETH_MACMIIAR_CR;

    ETH->MACMIIAR = RegisterValue | ETH_MACMIIAR_MB | ((uint32_t)PHY_Address << 11) | ((uint32_t)RegisterAddress <<  6);

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
SystemState_e ETH_Driver::PHY_Write(PHY_Address, uint8_t RegisterAddress, uint16_t Data)
{
    uint32_t RegisterValue;

    if((m_EMAC_Control.Flags & m_EMAC_Control_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }

    ETH->MACMIIDR = Data;
    RegisterValue = ETH->MACMIIAR & ETH_MACMIIAR_CR;
    ETH->MACMIIAR = RegisterValue | ETH_MACMIIAR_MB | ETH_MACMIIAR_MW | ((uint32_t)PHY_Address << 11) | ((uint32_t)RegisterAddress <<  6);

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
    while(TickHasTimeOut(TickStart, I2C_TIME_OUT) == false);

    if((ETH->MACMIIAR & ETH_MACMIIAR_MB) == 0)
    {
        return SYS_READY;
    }

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
    RCC->AHB1RSTR &= ~RCC_AHB1RSTR_ETHMACRST;
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
//   Function name:     ETH_IRQHandler
//
//   Parameter(s):      None
//   Return value:      None
//
//   Description:       Ethernet IRQ Handler.
//
//-------------------------------------------------------------------------------------------------
void ETH_IRQHandler(void)
{
    uint32_t dmasr, macsr, event = 0;

    dmasr = ETH->DMASR;
    ETH->DMASR = dmasr & (ETH_DMASR_NIS | ETH_DMASR_RS | ETH_DMASR_TS);

    if(dmasr & ETH_DMASR_TS)
    {
        // Frame sent
        event |= ETH_MAC_EVENT_TX_FRAME;
    }
    
    if(dmasr & ETH_DMASR_RS)
    {
        // Frame received
        event |= ETH_MAC_EVENT_RX_FRAME;
    }
    macsr = ETH->MACSR;

  #if (DIGINI_USE_ETH_TIME_STAMP == DEF_ENABLED)
    if(macsr & ETH_MACSR_TSTS)
    {
        // Timestamp interrupt
        if(ETH->PTPTSSR & 2 /*ETH_PTPTSSR_TSTTR*/)
        {
            // Time stamp target time reached
            event |= ETH_MAC_EVENT_TIMER_ALARM;
        }
    }
  #endif

    if(macsr & ETH_MACSR_PMTS)
    {
        ETH->MACPMTCSR;
        event |= ETH_MAC_EVENT_WAKEUP;
    }

    // Callback event notification
    if(event && m_EMAC_Control.CallbackEvent)
    {
        m_EMAC_Control.CallbackEvent(event);
    }
}
