//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H7_dma.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define DMA_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  DMA_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DMA_DIRECTION_MASK          (uint32_t(DMA_SxCR_DIR_0 | DMA_SxCR_DIR_1))

#define DMA_SxCR_INIT_MASK          (uint32_t(DMA_SxCR_MBURST | DMA_SxCR_PBURST | DMA_SxCR_PL    |\
                                              DMA_SxCR_MSIZE  | DMA_SxCR_PSIZE  | DMA_SxCR_CIRC  |\
                                              DMA_SxCR_MINC   | DMA_SxCR_PINC   | DMA_SxCR_DIR   |\
                                              DMA_SxCR_CT     | DMA_SxCR_DBM))

#define BDMA_CCR_INIT_MASK          (uint32_t(BDMA_CCR_PL    | BDMA_CCR_MSIZE   | BDMA_CCR_PSIZE |\
                                              BDMA_CCR_MINC  | BDMA_CCR_PINC    | BDMA_CCR_CIRC  |\
                                              BDMA_CCR_DIR   | BDMA_CCR_MEM2MEM | BDMA_CCR_DBM   |\
                                              BDMA_CCR_CT))

#if defined(UART9)
#define IS_DMA_UART_USART_REQUEST(__REQUEST__) ((((__REQUEST__) >= DMA_REQUEST_USART1_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_USART3_TX)) || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART4_RX)   &&  ((__REQUEST__) <= DMA_REQUEST_UART5_TX )) || \
                                                (((__REQUEST__) >= DMA_REQUEST_USART6_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_USART6_TX)) || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART7_RX)   &&  ((__REQUEST__) <= DMA_REQUEST_UART8_TX )) || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART9_RX)   &&  ((__REQUEST__) <= DMA_REQUEST_USART10_TX )))
#else
#define IS_DMA_UART_USART_REQUEST(__REQUEST__) ((((__REQUEST__) >= DMA_REQUEST_USART1_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_USART3_TX)) || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART4_RX)   &&  ((__REQUEST__) <= DMA_REQUEST_UART5_TX )) || \
                                                (((__REQUEST__) >= DMA_REQUEST_USART6_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_USART6_TX)) || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART7_RX)   &&  ((__REQUEST__) <= DMA_REQUEST_UART8_TX )))

#endif

#define DMA_SOURCE_TO_DESTINATION_MASK          DMA_SxCR_DIR_Msk



typedef struct
{
  __IO uint32_t ISR;   /*!< DMA interrupt status register */
  __IO uint32_t Reserved0;
  __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
} DMA_BaseRegisters;

typedef struct
{
  __IO uint32_t ISR;   /*!< BDMA interrupt status register */
  __IO uint32_t IFCR;  /*!< BDMA interrupt flag clear register */
} BDMA_BaseRegisters;

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   DMA_Info_t* pInfo
//  Return:         None
//
//  Description:    Setup transfert from source to destination. according to configuration
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Initialize(DMA_Info_t* pInfo)
{
    uint32_t registerValue;
    DMA_BaseRegisters *regs_dma;
    BDMA_BaseRegisters *regs_bdma;

    m_Handle.pPtr = pInfo->pHandle;

    // DMA1 or DMA2 instance
    if(uintptr_t(m_Handle.pPtr) < BDMA_BASE)
    {
        m_DMA_Type     = DMA_TYPE;
        m_StreamNumber = ((uint32_t(m_Handle.pPtr) & 0xFF) - 16) / 24;

        if((uint32_t(m_Handle.pPtr) <= ((uint32_t)DMA2_Stream7) ) && (uint32_t(m_Handle.pPtr) >= ((uint32_t)DMA2_Stream0)))
        {
          m_StreamNumber += 8;
        }

        m_StreamIndex  = ((m_StreamNumber & 0x01) ? 0x06 : 0x00) | ((m_StreamNumber & 0x02) ? 0x10 : 0x00);   // Bitshift of flags within status registers

      #if (DMA_CHECK_FIFO_PARAMS == DEF_ENABLED)
        if(CheckFifoParam(pInfo) != SYS_READY)
        {
            while(1)
            {
                __asm("nop");
            }
        }
      #endif

        CLEAR_BIT(m_Handle.pDMA->CR, DMA_SxCR_EN);                                             // Disable the DMA
        while((m_Handle.pDMA->CR & DMA_SxCR_EN) != 0) {};                                      // Check if the DMA Stream is effectively disabled
        MODIFY_REG(m_Handle.pDMA->CR, DMA_SxCR_INIT_MASK, pInfo->Config);                      // Write the DMA Stream configuration

        // Work around for Errata 2.22: UART/USART- DMA transfer lock: DMA stream could be lock when transferring data to/from USART/UART
      #if (STM32H7_DEV_ID == 0x450UL)
        if((DBGMCU->IDCODE & 0xFFFF0000) >= 0x20000000)
        {
      #endif // STM32H7_DEV_ID == 0x450UL
            if(IS_DMA_UART_USART_REQUEST(pInfo->MUX_Request) != 0)
            {
                SET_BIT(m_Handle.pDMA->CR, DMA_SxCR_TRBUFF);
            }
      #if (STM32H7_DEV_ID == 0x450UL)
        }
      #endif // STM32H7_DEV_ID == 0x450UL

//--------------------------------------------------------------------------

/*

        MODIFY_REG(m_pHandle.pDMA->FCR, (uint32_t)~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH), pInfo->FIFO_Config));  // Prepare the DMA Stream FIFO configuration

        if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE)                                                      // the FIFO threshold is not used when the FIFO mode is disabled
        {
            if(hdma->Init.MemBurst != DMA_MEMORY_BURST_SINGLE)                                              // Check compatibility between FIFO threshold level and size of the memory burst for INCR4, INCR8, INCR16



        MODIFY_REG(m_pHandle.pDMA->FCR, (uint32_t)~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH, hdma->Init.FIFOMode)); // Prepare the DMA Stream FIFO configuration

        if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE)                                                      // the FIFO threshold is not used when the FIFO mode is disabled
        {
            if(hdma->Init.MemBurst != DMA_MBURST_SINGLE)                                                    // Check compatibility between FIFO threshold level and size of the memory burst for INCR4, INCR8, INCR16
            {
                if(DMA_CheckFifoParam(hdma) != HAL_OK)
                {
                    return HAL_ERROR;
                }
            }

            SET_BIT(m_pHandle.pDMA->FCR, hdma->Init.FIFOThreshold);
        }
*/
//--------------------------------------------------------------------------

        CalcBaseAndBitShift();
        regs_dma = (DMA_BaseRegisters *)m_StreamBaseAddress;                                        // Initialize StreamBaseAddress and StreamIndex parameters to be used to calculate DMA steam Base Address needed by HAL_DMA_IRQHandler() and HAL_DMA_PollForTransfer()
        regs_dma->IFCR = 0x3F << (m_StreamIndex & 0x1F);                                                // Clear all interrupt flags
    }
    else // BDMA instance(s)
    {
        m_DMA_Type   = BDMA_TYPE;
        m_StreamNumber = ((uint32_t(m_Handle.pPtr) & 0xFF) - 8) / 20;

        CLEAR_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_EN);
        MODIFY_REG(m_Handle.pBDMA->CCR, BDMA_CCR_INIT_MASK, pInfo->Config);

        // Prepare the DMA Channel configuration
        m_StreamIndex = ((uint32_t(m_Handle.pPtr) - (uint32_t)BDMA_Channel0) / ((uint32_t)BDMA_Channel1 - (uint32_t)BDMA_Channel0)) << 2;           // calculation of the channel index
        CalcBaseAndBitShift();
        regs_bdma = (BDMA_BaseRegisters *)m_StreamBaseAddress;                                   // Initialize StreamBaseAddress and StreamIndex parameters to be used to calculate DMA steam Base Address needed by HAL_DMA_IRQHandler() and HAL_DMA_PollForTransfer()
        regs_bdma->IFCR = ((BDMA_IFCR_CGIF0) << (m_StreamIndex & 0x1F));                                // Clear all interrupt flags
    }

    if(IS_DMA_DMAMUX_ALL_INSTANCE(m_Handle.pPtr) != 0)
    {
        CalcDMAMUX_ChannelBaseAndMask();                                                             // Initialize parameters for DMAMUX channel : DMAmuxChannel, DMAmuxChannelStatus and DMAmuxChannelStatusMask

// THIS is VERY Strange has BDMA and DMA has different config!!!
        if((pInfo->Config & DMA_SOURCE_TO_DESTINATION_MASK) == DMA_MEMORY_TO_MEMORY)
        {
  //TODO          //hdma->Init.Request = DMA_REQUEST_MEM2MEM;                                               // if memory to memory force the request to 0 ( TODO understand this as the comment is wrong!!
        }

        m_pDMAMUX_Channel->CCR = (hdma->Init.Request & DMAMUX_CxCR_DMAREQ_ID);                              // Set peripheral request  to DMAMUX channel
        m_pDMAMUX_ChannelStatus->CFR = m_DMAMUX_ChannelStatusMask;                                          // Clear the DMAMUX synchro overrun flag

        // Initialize parameters for DMAMUX request generator : if the DMA request is DMA_REQUEST_GENERATOR0 to DMA_REQUEST_GENERATOR7
        if((pInfo->MUX_Request >= DMA_REQUEST_GENERATOR0) && (pInfo->MUX_Request <= DMA_REQUEST_GENERATOR7))
        {
            CalcDMAMUX_RequestGenBaseAndMask(pInfo->MUX_Request);                                           // Initialize parameters for DMAMUX request generator : DMAmuxRequestGen, DMAmuxRequestGenStatus and DMAmuxRequestGenStatusMask
            m_pDMAMUX_RequestGen->RGCR = 0;                                                                 // Reset the DMAMUX request generator register
            m_pDMAMUX_RequestGenStatus->RGCFR = hdma->DMAmuxRequestGenStatusMask;                           // Clear the DMAMUX request generator overrun flag
        }
        else
        {
            m_pDMAMUX_RequestGen          = nullptr;
            m_pDMAMUX_RequestGenStatus    = nullptr;
            m_DMAMUX_RequestGenStatusMask = 0;
        }
    }

    /*
    uint32_t StreamNumber;

    m_pDMA         = pInfo->pDMA;
    m_Flag         = pInfo->Flag;
    m_IRQn_Channel = pInfo->IRQn_Channel;
    EnableClock();
    m_pDMA->CR     = pInfo->Config;
    m_Direction    = pInfo->Config & DMA_DIRECTION_MASK;

    // DMA1/DMA2 Streams are connected to DMAMUX1 channels
    StreamNumber = ((uint32_t(m_pDMA) & 0xFF) - 16) / 24;

    if((uintptr_t(m_pDMA) <= uintptr_t(DMA2_Stream7_BASE)) && (uintptr_t(m_pDMA) >= uintptr_t(DMA2_Stream0_BASE)))
    {
      StreamNumber += 8;
    }

// do i need to do this... will it be use again later
    m_DMAMUX_Channel            = (DMAMUX_Channel_TypeDef *)((uint32_t)(((uint32_t)DMAMUX1_Channel0) + (StreamNumber * 4)));
    m_DMAMUX_ChannelStatus      = DMAMUX1_ChannelStatus;     // not neccessary all DMA1 and 2 are on this
    m_DMAMUX_ChannelStatusMask  = uint32_t(1) << (StreamNumber & 0x1F);

    m_DMAMUX_Channel->CCR       = MUX_Request;                               // Set peripheral request  to DMAMUX channel
    m_DMAMUX_ChannelStatus->CFR = m_DMAMUX_ChannelStatusMask;           // Clear the DMAMUX synchro overrun flag

    */
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::Enable(void)
{
     if(m_DMA_Type == DMA_TYPE) SET_BIT(m_Handle.pDMA->CR,   DMA_SxCR_EN);
     else                       SET_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_EN);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::Disable(void)
{
    if(m_DMA_Type == DMA_TYPE) CLEAR_BIT(m_Handle.pDMA->CR,   DMA_SxCR_EN);
    else                       CLEAR_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetTransfer
//
//  Parameter(s):   Source
//                  Destination
//                  Length
//  Return:         None
//
//  Description:    Setup transfer from source to destination. according to configuration
//
//  Note(s):        Add in direction support for M2M
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetTransfer(void* pSource, void* pDestination, size_t Length)
{
    if(m_Direction == DMA_MEMORY_TO_PERIPHERAL)
    {
        if(m_DMA_Type == DMA_TYPE)
        {
            m_Handle.pDMA->M0AR = uint32_t(pSource);
            m_Handle.pDMA->PAR  = uint32_t(pDestination);
        }
        else
        {
            m_Handle.pBDMA->CM0AR = uint32_t(pSource);
            m_Handle.pBDMA->CPAR  = uint32_t(pDestination);
        }
    }
    else
    {
        if(m_DMA_Type == DMA_TYPE)
        {
            m_Handle.pDMA->M0AR = uint32_t(pDestination);
            m_Handle.pDMA->PAR  = uint32_t(pSource);
        }
        else
        {
            m_Handle.pBDMA->CM0AR = uint32_t(pDestination);
            m_Handle.pBDMA->CPAR  = uint32_t(pSource);
        }
    }

    if(m_DMA_Type == DMA_TYPE) m_Handle.pDMA->NDTR   = uint32_t(Length);
    else                       m_Handle.pBDMA->CNDTR = uint32_t(Length);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetSource
//
//  Parameter(s):   Source
//  Return:         None
//
//  Description:    Setup source for transfer from source to destination. according to
//                  configuration
//
//  Note(s):        Add in direction support for M2M
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetSource(void* pSource)
{
    if(m_Direction == DMA_MEMORY_TO_PERIPHERAL)
    {
        if(m_DMA_Type == DMA_TYPE) m_Handle.pDMA->M0AR   = uint32_t(pSource);
        else                       m_Handle.pBDMA->CM0AR = uint32_t(pSource);
    }
    else
    {
        if(m_DMA_Type == DMA_TYPE) m_Handle.pDMA->PAR   = uint32_t(pSource);
        else                       m_Handle.pBDMA->CPAR = uint32_t(pSource);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetDestination
//
//  Parameter(s):   Destination
//  Return:         None
//
//  Description:    Setup destination for transfer from source to destination. according to
//                  configuration
//
//  Note(s):        Add in direction support for M2M
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetDestination(void* Destination)
{
    if(m_Direction == DMA_MEMORY_TO_PERIPHERAL)
    {
        if(m_DMA_Type == DMA_TYPE) m_Handle.pDMA->PAR   = uint32_t(Destination);
        else                       m_Handle.pBDMA->CPAR = uint32_t(Destination);
    }
    else
    {
        if(m_DMA_Type == DMA_TYPE) m_Handle.pDMA->M0AR   = uint32_t(Destination);
        else                       m_Handle.pBDMA->CM0AR = uint32_t(Destination);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearFlag
//
//  Parameter(s):   Flag        Flag to clear
//  Return:         None
//
//  Description:    Clear flag for specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_Driver::ClearFlag(uint32_t Flag)
{
    volatile uint32_t* pRegister = nullptr;

    if(m_DMA_Type == DMA_TYPE)
    {
        switch(uintptr_t(m_Handle.pDMA))
        {
            case DMA1_Stream0_BASE:
            case DMA1_Stream1_BASE:
            case DMA1_Stream2_BASE:
            case DMA1_Stream3_BASE: pRegister = &DMA1->LIFCR; break;

            case DMA1_Stream4_BASE:
            case DMA1_Stream5_BASE:
            case DMA1_Stream6_BASE:
            case DMA1_Stream7_BASE: pRegister = &DMA1->HIFCR; break;

            case DMA2_Stream0_BASE:
            case DMA2_Stream1_BASE:
            case DMA2_Stream2_BASE:
            case DMA2_Stream3_BASE: pRegister = &DMA2->LIFCR; break;

            case DMA2_Stream4_BASE:
            case DMA2_Stream5_BASE:
            case DMA2_Stream6_BASE:
            case DMA2_Stream7_BASE: pRegister = &DMA2->HIFCR; break;
        }
    }
    else // BDMA_TYPE
    {
        pRegister = (uint32_t*)BDMA1->IFCR;
    }

    if(pRegister != nullptr)
    {
        SET_BIT(*pRegister, Flag);
    }
}

//-------------------------------------------------------------------------------------------------

size_t DMA_Driver::GetLength(void)
{
    if(m_DMA_Type == DMA_TYPE) return size_t(m_Handle.pDMA->NDTR);
                               return size_t(m_Handle.pBDMA->CNDTR);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::SetLength(size_t Length)
{
    if(m_DMA_Type == DMA_TYPE) m_Handle.pDMA->NDTR   = uint32_t(Length);
    else                       m_Handle.pBDMA->CNDTR = uint32_t(Length);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::SetMemoryIncrement(void)
{
    if(m_DMA_Type == DMA_TYPE) SET_BIT(m_Handle.pDMA->CR,   DMA_SxCR_MINC);
    else                       SET_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_MINC);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::SetNoMemoryIncrement(void)
{
    if(m_DMA_Type == DMA_TYPE) CLEAR_BIT(m_Handle.pDMA->CR,   DMA_SxCR_MINC);
    else                       CLEAR_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_MINC);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::SetFifoControl(uint32_t Control)
{
    if(m_DMA_Type == DMA_TYPE)
    {
        m_Handle.pDMA->FCR = Control;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CheckFlag
//
//  Parameter(s):   Flag        Flag to check
//  Return:         bool        If true then flag is set.
//
//  Description:    Check flag for specific DMA stream.
//-------------------------------------------------------------------------------------------------
bool DMA_Driver::CheckFlag(uint32_t Flag)
{
    volatile uint32_t Register = 0;
    bool              Result   = false;

    if(m_DMA_Type == DMA_TYPE)
    {
        // we may already have it with m_StreamBaseAddress

        switch(uintptr_t(m_Handle.pDMA))
        {
            case DMA1_Stream0_BASE:
            case DMA1_Stream1_BASE:
            case DMA1_Stream2_BASE:
            case DMA1_Stream3_BASE: Register = DMA1->LISR; break;

            case DMA1_Stream4_BASE:
            case DMA1_Stream5_BASE:
            case DMA1_Stream6_BASE:
            case DMA1_Stream7_BASE: Register = DMA1->HISR; break;

            case DMA2_Stream0_BASE:
            case DMA2_Stream1_BASE:
            case DMA2_Stream2_BASE:
            case DMA2_Stream3_BASE: Register = DMA2->LISR; break;

            case DMA2_Stream4_BASE:
            case DMA2_Stream5_BASE:
            case DMA2_Stream6_BASE:
            case DMA2_Stream7_BASE: Register = DMA2->HISR; break;
        }
        
        
    }
    else // BDMA_TYPE
    {
        Register = (uint32_t*)BDMA1->ISR;
    }

    if((Register & Flag) != 0)
    {
        Result = true;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableClock
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the associated DMA module clock
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableClock(void)
{

    if(uintptr_t(m_Handle.pBDMA) < BDMA_BASE)
    {
        if(uintptr_t(m_Handle.pDMA) < DMA2_Stream0_BASE)
        {
            SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);
        }
        else
        {
            SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);
        }
    }
    else
    {
        SET_BIT(RCC->AHB4ENR, RCC_AHB4ENR_BDMA2EN);  //?????
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableIRQ
//
//  Parameter(s):   uint8_t    PremptionPriority
//  Return:         None
//
//  Description:    Enable the IRQ DMA for the Channel and Stream
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableIRQ(uint8_t PremptionPriority)
{
    ISR_Init(m_IRQn_Channel, PremptionPriority);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::EnableInterrupt(uint32_t Interrupt)
{
    if(m_DMA_Type == DMA_TYPE) SET_BIT(m_Handle.pDMA->CR,   Interrupt);
    else                       SET_BIT(m_Handle.pBDMA->CCR, Interrupt);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::DisableInterrupt(uint32_t Interrupt)
{
    if(m_DMA_Type == DMA_TYPE) CLEAR_BIT(m_Handle.pDMA->CR,   Interrupt);
    else                       CLEAR_BIT(m_Handle.pBDMA->CCR, Interrupt);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::EnableTransmitCompleteInterrupt(void)
{
    if(m_DMA_Type == DMA_TYPE) SET_BIT(m_Handle.pDMA->CR,   DMA_SxCR_TCIE);
    else                       SET_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_TCIE);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::DisableTransmitCompleteInterrupt(void)
{
    if(m_DMA_Type == DMA_TYPE) CLEAR_BIT(m_Handle.pDMA->CR,   DMA_SxCR_TCIE);
    else                       CLEAR_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_TCIE);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::EnableTransmitHalfCompleteInterrupt(void)
{
    if(m_DMA_Type == DMA_TYPE) SET_BIT(m_Handle.pDMA->CR,   DMA_SxCR_HTIE);
    else                       SET_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_HTIE);
}

//-------------------------------------------------------------------------------------------------

void DMA_Driver::DisableTransmitHalfCompleteInterrupt(void)
{
    if(m_DMA_Type == DMA_TYPE) CLEAR_BIT(m_Handle.pDMA->CR,   DMA_SxCR_HTIE);
    else                       CLEAR_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_HTIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CheckFifoParam
//
//  Parameter(s):   DMA_Info_t* pInfo   Pointer to a configuration structure.
//  Return:         SystemState_e
//
//  Description:    Check compatibility between FIFO threshold level and size of the memory burst
//
//  Note(s):        Use this function only at development.
//
//-------------------------------------------------------------------------------------------------
#if (DMA_CHECK_FIFO_PARAMS == DEF_ENABLED)
SystemState_e DMA_Driver::CheckFifoParam(DMA_Info_t* pInfo)
{
    SystemState_e State;

    if((pInfo->Config & DMA_SxCR_MSIZE_Msk) == DMA_MEMORY_SIZE_8_BITS)             // Memory Data size equal to Byte
    {
        switch(pInfo->FIFO_Config & DMA_SxFCR_FTH_Msk)
        {
            case DMA_FIFO_THRESHOLD_1_QUARTER_FULL:
            case DMA_FIFO_THRESHOLD_3_QUARTERS_FULL:
            {
                if((pInfo->Config & DMA_SxCR_MBURST_Msk) == DMA_MEMORY_BURST_INC8)
                {
                    State = SYS_INVALID_PARAMETER;
                }
            }
            break;

            case DMA_FIFO_THRESHOLD_HALF_FULL:
            {
                if((pInfo->Config & DMA_SxCR_MBURST_Msk) == DMA_MEMORY_BURST_INC16)
                {
                    State = SYS_INVALID_PARAMETER;
                }
            }
            break;

            // case DMA_FIFO_THRESHOLD_FULL:
            default:
            break;
        }
    }
    else if((pInfo->Config & DMA_SxCR_MSIZE_Msk) == DMA_MEMORY_SIZE_16_BITS)                   // Memory Data size equal to Half-Word
    {
        switch(pInfo->FIFO_Config & DMA_SxFCR_FTH_Msk)
        {
            case DMA_FIFO_THRESHOLD_1_QUARTER_FULL:
            case DMA_FIFO_THRESHOLD_3_QUARTERS_FULL:
            {
                State = SYS_INVALID_PARAMETER;
            }
            break;

            case DMA_FIFO_THRESHOLD_HALF_FULL:
            {
                if((pInfo->Config & DMA_SxCR_MBURST_Msk) == DMA_MEMORY_BURST_INC8)
                {
                    State = SYS_INVALID_PARAMETER;
                }
            }
            break;

            case DMA_FIFO_THRESHOLD_FULL:
            {
                if((pInfo->Config & DMA_SxCR_MBURST_Msk) == DMA_MEMORY_BURST_INC16)
                {
                    State = SYS_INVALID_PARAMETER;
                }
            }
            break;

            default:
            break;
        }
    }
    else                                                                                        // Memory Data size equal to Word
    {
        switch(pInfo->FIFO_Config & DMA_SxFCR_FTH_Msk)
        {
            case DMA_FIFO_THRESHOLD_1_QUARTER_FULL:
            case DMA_FIFO_THRESHOLD_HALF_FULL:
            case DMA_FIFO_THRESHOLD_3_QUARTERS_FULL:
            {
                State = SYS_INVALID_PARAMETER;
            }
            break;

            case DMA_FIFO_THRESHOLD_FULL:
            {
                if((pInfo->Config & DMA_SxCR_MBURST_Msk) == DMA_MEMORY_BURST_INC8)
                {
                    State = SYS_INVALID_PARAMETER;
                }
            }
            break;

            default:
            break;
        }
    }

    return State;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CalcBaseAndBitShift
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Returns the DMA Stream base address depending on stream number
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::CalcBaseAndBitShift(void)
{
    if(m_DMA_Type == DMA_TYPE) // DMA1 or DMA2 instance
    {
        m_StreamBaseAddress = (uint32_t(m_Handle.pPtr) & uint32_t(~0x3FF));

        if(m_StreamIndex > 3)
        {
            m_StreamBaseAddress += 4;                         // Return pointer to LISR and LIFCR
        }
// we may have already have this point in m_Handle.pDMA .... so why we calculate this???  or we may use it in CheckFlag and clear flag
    }
    else // BDMA instance
    {
        m_StreamBaseAddress = (uint32_t(m_Handle.pPtr) & uint32_t(~0xFF));                              // return pointer to ISR and IFCR
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CalcDMAMUX_ChannelBaseAndMask
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Updates the DMAMUX channel and status mask depending on stream number
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::CalcDMAMUX_ChannelBaseAndMask(void)
{
    uint32_t DMAMUX_ChannelAddress;

    if(m_DMA_Type == DMA_TYPE) // DMA1 or DMA2 instance
    {
        DMAMUX_ChannelAddress   = DMAMUX1_Channel0;
        m_pDMAMUX_ChannelStatus = DMAMUX1_ChannelStatus;
    }
    else // BDMA instance
    {
        // BDMA Channels are connected to DMAMUX2 channels
        DMAMUX_ChannelAddress   = DMAMUX2_Channel0;
        m_pDMAMUX_ChannelStatus = DMAMUX2_ChannelStatus;
    }

    m_pDMAMUX_Channel          = (DMAMUX_Channel_TypeDef *)((uint32_t)(((uint32_t)DMAMUX_ChannelAddress) + (m_StreamIndex * 4)));
    m_DMAMUX_ChannelStatusMask = uint32_t(1) << (m_StreamIndex & 0x1F);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CalcDMAMUX_RequestGenBaseAndMask
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Updates the DMAMUX request generator params
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::CalcDMAMUX_RequestGenBaseAndMask(uint32_t Request)
{
      uint32_t DMAMUX_RequestGeneratorAddress;

  //uint32_t request =  hdma->Init.Request & DMAMUX_CxCR_DMAREQ_ID;

    //if((request >= DMA_REQUEST_GENERATOR0) && (request <= DMA_REQUEST_GENERATOR7))
    {
        if(m_DMA_Type == DMA_TYPE) // DMA1 or DMA2 instance
        {
            // DMA1 and DMA2 Streams use DMAMUX1 request generator blocks
            DMAMUX_RequestGeneratorAddress = uint32_t(DMAMUX1_RequestGenerator0);
            m_pDMAMUX_RequestGenStatus     = DMAMUX1_RequestGenStatus;
        }
        else
        {
            // BDMA Channels are connected to DMAMUX2 request generator blocks
            DMAMUX_RequestGeneratorAddress = uint32_t(DMAMUX2_RequestGenerator0);
            m_pDMAMUX_RequestGenStatus     = DMAMUX2_RequestGenStatus;
        }

        m_pDMAMUX_RequestGen          = (DMAMUX_RequestGen_TypeDef *)((uint32_t)((DMAMUX_RequestGeneratorAddress) + ((Request - 1) * 4)));
        m_DMAMUX_RequestGenStatusMask = uint32_t(1) << (Request - 1);
    }
}
