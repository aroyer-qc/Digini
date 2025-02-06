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
                                                (((__REQUEST__) >= DMA_REQUEST_UART4_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_UART5_TX ))  || \
                                                (((__REQUEST__) >= DMA_REQUEST_USART6_RX) &&  ((__REQUEST__) <= DMA_REQUEST_USART6_TX))  || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART7_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_UART8_TX ))  || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART9_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_USART10_TX )))
#else
#define IS_DMA_UART_USART_REQUEST(__REQUEST__) ((((__REQUEST__) >= DMA_REQUEST_USART1_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_USART3_TX)) || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART4_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_UART5_TX ))  || \
                                                (((__REQUEST__) >= DMA_REQUEST_USART6_RX) &&  ((__REQUEST__) <= DMA_REQUEST_USART6_TX))  || \
                                                (((__REQUEST__) >= DMA_REQUEST_UART7_RX)  &&  ((__REQUEST__) <= DMA_REQUEST_UART8_TX )))

#endif

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
    DMA_Base_Registers *regs_dma;
    BDMA_Base_Registers *regs_bdma;

    m_Handle.pPtr = pInfo->pHandle;

    // DMA1 or DMA2 instance
    if(uintptr_t(m_Handlep.Ptr) < BDMA_BASE)
    {
        m_DMA_Type = DMA_TYPE;
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

        regs_dma = (DMA_Base_Registers *)DMA_CalcBaseAndBitshift(hdma);                                     // Initialize StreamBaseAddress and StreamIndex parameters to be used to calculate DMA steam Base Address needed by HAL_DMA_IRQHandler() and HAL_DMA_PollForTransfer()
        regs_dma->IFCR = 0x3F << (hdma->StreamIndex & 0x1F);                                                // Clear all interrupt flags
    }
    else // BDMA instance(s)
    {
        m_DMA_Type = BDMA_TYPE;
        CLEAR_BIT(m_Handle.pBDMA->CCR, BDMA_CCR_EN);
        MODIFY_REG(m_Handle.pBDMA->CCR, BDMA_CCR_INIT_MASK, pInfo->Config);

        // Prepare the DMA Channel configuration
        hdma->StreamIndex = (((uint32_t)((uint32_t*)hdma->Instance) - (uint32_t)BDMA_Channel0) / ((uint32_t)BDMA_Channel1 - (uint32_t)BDMA_Channel0)) << 2;           // calculation of the channel index
        regs_bdma = (BDMA_Base_Registers *)DMA_CalcBaseAndBitshift(hdma);                                   // Initialize StreamBaseAddress and StreamIndex parameters to be used to calculate DMA steam Base Address needed by HAL_DMA_IRQHandler() and HAL_DMA_PollForTransfer()
        regs_bdma->IFCR = ((BDMA_IFCR_CGIF0) << (hdma->StreamIndex & 0x1F));                                // Clear all interrupt flags
    }

    if(IS_DMA_DMAMUX_ALL_INSTANCE(hdma->Instance) != 0)
    {
        DMA_CalcDMAMUXChannelBaseAndMask(hdma);                                                             // Initialize parameters for DMAMUX channel : DMAmuxChannel, DMAmuxChannelStatus and DMAmuxChannelStatusMask

        if(hdma->Init.Direction == DMA_MEMORY_TO_MEMORY)
        {
            hdma->Init.Request = DMA_REQUEST_MEM2MEM;                                                       // if memory to memory force the request to 0
        }

        hdma->DMAmuxChannel->CCR = (hdma->Init.Request & DMAMUX_CxCR_DMAREQ_ID);                            // Set peripheral request  to DMAMUX channel
        hdma->DMAmuxChannelStatus->CFR = hdma->DMAmuxChannelStatusMask;                                     // Clear the DMAMUX synchro overrun flag

        // Initialize parameters for DMAMUX request generator : if the DMA request is DMA_REQUEST_GENERATOR0 to DMA_REQUEST_GENERATOR7
        if((hdma->Init.Request >= DMA_REQUEST_GENERATOR0) && (hdma->Init.Request <= DMA_REQUEST_GENERATOR7))
        {
            DMA_CalcDMAMUXRequestGenBaseAndMask(hdma);                                                      // Initialize parameters for DMAMUX request generator : DMAmuxRequestGen, DMAmuxRequestGenStatus and DMAmuxRequestGenStatusMask
            hdma->DMAmuxRequestGen->RGCR = 0;                                                               // Reset the DMAMUX request generator register
            hdma->DMAmuxRequestGenStatus->RGCFR = hdma->DMAmuxRequestGenStatusMask;                         // Clear the DMAMUX request generator overrun flag
        }
        else
        {
            hdma->DMAmuxRequestGen = 0;
            hdma->DMAmuxRequestGenStatus = 0;
            hdma->DMAmuxRequestGenStatusMask = 0;
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



#if 0


HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma)
{
    uint32_t registerValue;
    uint32_t tickstart = HAL_GetTick();
    DMA_Base_Registers *regs_dma;
    //BDMA_Base_Registers *regs_bdma;

    CLEAR_BIT(m_pDMA->CR, DMA_SxCR_EN);

    while(((m_pDMA->CR & DMA_SxCR_EN) != 0)                                                         // Check if the DMA Stream is effectively disabled
    {
        if((HAL_GetTick() - tickstart ) > HAL_TIMEOUT_DMA_ABORT)                                    // Check for the Timeout
        {
            return HAL_ERROR;
        }
    }


    registerValue = m_pDMA->CR;                                                                     // Get the CR register value

    MODIFY_REG(registerValue, ((uint32_t)~(DMA_SxCR_MBURST | DMA_SxCR_PBURST |                      // Clear CHSEL, MBURST, PBURST, PL, MSIZE, PSIZE, MINC, PINC, CIRC, DIR, CT and DBM bits
                                           DMA_SxCR_PL     | DMA_SxCR_MSIZE  | DMA_SxCR_PSIZE |
                                           DMA_SxCR_MINC   | DMA_SxCR_PINC   | DMA_SxCR_CIRC  |
                                           DMA_SxCR_DIR    | DMA_SxCR_CT     | DMA_SxCR_DBM),
                              (hdma->Init.Direction           |                                     // Prepare the DMA Stream configuration
                               hdma->Init.PeriphInc           | hdma->Init.MemInc           |
                               hdma->Init.PeriphDataAlignment | hdma->Init.MemDataAlignment |
                               hdma->Init.Mode                | hdma->Init.Priority));

    if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE)                                                  // the Memory burst and peripheral burst are not used when the FIFO is disabled
    {
        SET_BIT(registerValue, (hdma->Init.MemBurst | hdma->Init.PeriphBurst);                      // Get memory burst and peripheral burst
    }


    // Work around for Errata 2.22: UART/USART- DMA transfer lock: DMA stream could lock when transferring data to/from USART/UART
  #if (STM32H7_DEV_ID == 0x450UL)
    if((DBGMCU->IDCODE & 0xFFFF0000U) >= 0x20000000U)
    {
  #endif // STM32H7_DEV_ID == 0x450UL
        if(IS_DMA_UART_USART_REQUEST(hdma->Init.Request) != 0)
        {
            registerValue |= DMA_SxCR_TRBUFF;
        }
  #if (STM32H7_DEV_ID == 0x450UL)
    }
  #endif // STM32H7_DEV_ID == 0x450UL

    m_pDMA->CR = registerValue;                                                                     // Write to DMA Stream CR register


    registerValue = m_pDMA->FCR;                                                                    // Get the FCR register value
    registerValue &= (uint32_t)~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);                                  // Clear Direct mode and FIFO threshold bits
    registerValue |= hdma->Init.FIFOMode;                                                           // Prepare the DMA Stream FIFO configuration

    // the FIFO threshold is not used when the FIFO mode is disabled
    if(hdma->Init.FIFOMode == DMA_FIFOMODE_ENABLE)
    {
        registerValue |= hdma->Init.FIFOThreshold;                                                  // Get the FIFO threshold

        // Check compatibility between FIFO threshold level and size of the memory burst for INCR4, INCR8, INCR16
        if(hdma->Init.MemBurst != DMA_MBURST_SINGLE)
        {
            if(DMA_CheckFifoParam(hdma) != HAL_OK)
                {
                    return HAL_ERROR;
                }
            }
        }

        m_pDMA->FCR = registerValue;                                                                // Write to DMA Stream FCR
        // Initialize StreamBaseAddress and StreamIndex parameters to be used to calculate DMA steam Base Address needed by HAL_DMA_IRQHandler() and HAL_DMA_PollForTransfer()
        regs_dma = (DMA_Base_Registers *)DMA_CalcBaseAndBitshift(hdma);
        regs_dma->IFCR = 0x3FUL << (hdma->StreamIndex & 0x1FU);                                     // Clear all interrupt flags
    }

    if(IS_DMA_DMAMUX_ALL_INSTANCE(hdma->Instance) != 0U) /* No DMAMUX available for BDMA1 */
    {
        // Initialize parameters for DMAMUX channel : DMAmuxChannel, DMAmuxChannelStatus and DMAmuxChannelStatusMask
        DMA_CalcDMAMUXChannelBaseAndMask(hdma);

        if(hdma->Init.Direction == DMA_MEMORY_TO_MEMORY)
        {
            hdma->Init.Request = DMA_REQUEST_MEM2MEM;                                               /* if memory to memory force the request to 0*/
        }

        hdma->DMAmuxChannel->CCR = (hdma->Init.Request & DMAMUX_CxCR_DMAREQ_ID);                /* Set peripheral request  to DMAMUX channel */
        hdma->DMAmuxChannelStatus->CFR = hdma->DMAmuxChannelStatusMask;                         /* Clear the DMAMUX synchro overrun flag */

        // Initialize parameters for DMAMUX request generator :if the DMA request is DMA_REQUEST_GENERATOR0 to DMA_REQUEST_GENERATOR7
        if((hdma->Init.Request >= DMA_REQUEST_GENERATOR0) && (hdma->Init.Request <= DMA_REQUEST_GENERATOR7))
        {
            /* Initialize parameters for DMAMUX request generator : DMAmuxRequestGen, DMAmuxRequestGenStatus and DMAmuxRequestGenStatusMask */
            DMA_CalcDMAMUXRequestGenBaseAndMask(hdma);
            hdma->DMAmuxRequestGen->RGCR = 0U;                                                  /* Reset the DMAMUX request generator register */
            hdma->DMAmuxRequestGenStatus->RGCFR = hdma->DMAmuxRequestGenStatusMask;             /* Clear the DMAMUX request generator overrun flag */
        }
        else
        {
            hdma->DMAmuxRequestGen = 0U;
            hdma->DMAmuxRequestGenStatus = 0U;
            hdma->DMAmuxRequestGenStatusMask = 0U;
        }
    }
}

#endif
