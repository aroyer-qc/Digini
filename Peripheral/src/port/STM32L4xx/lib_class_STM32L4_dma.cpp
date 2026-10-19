//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32L4_dma.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#define DMA_DIRECTION_MASK             (DMA_CCR_DIR_Msk | DMA_CCR_MEM2MEM_Msk)

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

//#if (DIGINI_USE_DMA_MEM2MEM_FUNCTION == DEF_ENABLED)
//nOS_Mutex MEM2MEM_Mutex;
//#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   Source
//                  Destination
//                  Length
//  Return:         None
//
//  Description:    Setup transfert from source to destination. according to configuration
//
//  Note(s):        Add in direction support for M2M
//
//-------------------------------------------------------------------------------------------------

void DMA_Driver::Initialize(DMA_Info_t* pInfo)
{
    m_pDMA         = pInfo->pDMA;
    m_Flag         = pInfo->Flag;
    m_pDMA_Channel = pInfo->pDMA_Channel;
    m_IRQn_Channel = pInfo->IRQn_Channel;
    m_PreempPrio   = pInfo->PreempPrio;
    EnableClock();
    Disable();
    while((m_pDMA_Channel->CCR & DMA_CCR_EN) != 0) {};
    m_pDMA_Channel->CCR = pInfo->Config & ~DMA_CCR_EN;
    m_Direction         = pInfo->Config & DMA_DIRECTION_MASK;

    uintptr_t ChannelBase = uintptr_t(DMA1_Channel1);
  #if (DMA2_SUPPORT == DEF_ENABLED)
    if(m_pDMA == DMA2)
    {
        ChannelBase = uintptr_t(DMA2_Channel1);
    }
  #endif

    m_ChannelNumber = uint32_t((uintptr_t(m_pDMA_Channel) - ChannelBase) / (uintptr_t(DMA1_Channel2) - uintptr_t(DMA1_Channel1)));
    m_pDMA->IFCR = DMA_IFCR_CGIF1 << (m_ChannelNumber * 4U);

  #if defined(DMAMUX1)
    CalcDMAMUX_ChannelBaseAndMask();
    uint32_t Request = ((m_Direction & DMA_CCR_MEM2MEM) != 0) ? DMA_REQUEST_MEM2MEM : pInfo->MUX_Request;
    m_pDMAMUX_Channel->CCR = Request & DMAMUX_CxCR_DMAREQ_ID;
    m_pDMAMUX_ChannelStatus->CFR = m_DMAMUX_ChannelStatusMask;

    if((Request >= DMA_REQUEST_GENERATOR0) && (Request <= DMA_REQUEST_GENERATOR3))
    {
        CalcDMAMUX_RequestGenBaseAndMask(Request);
        m_pDMAMUX_RequestGen->RGCR = 0;
        m_pDMAMUX_RequestGenStatus->RGCFR = m_DMAMUX_RequestGenStatusMask;
    }
    else
    {
        m_pDMAMUX_RequestGen = nullptr;
        m_pDMAMUX_RequestGenStatus = nullptr;
        m_DMAMUX_RequestGenStatusMask = 0;
    }

  #else

    if((m_Direction & DMA_CCR_MEM2MEM) == 0)
    {
        DMA_Request_TypeDef* pSelection = DMA1_CSELR;

      #if (DMA2_SUPPORT == DEF_ENABLED)
        if(m_pDMA == DMA2)
        {
            pSelection = DMA2_CSELR;
        }
      #endif

        uint32_t Shift = m_ChannelNumber * 4U;
        MODIFY_REG(pSelection->CSELR, DMA_CSELR_C1S << Shift, (pInfo->MUX_Request & DMA_CSELR_C1S) << Shift);
    }
  #endif
}

#if defined(DMAMUX1)
void DMA_Driver::CalcDMAMUX_ChannelBaseAndMask(void)
{
        uint32_t ChannelNumber = m_ChannelNumber;
    #if (DMA2_SUPPORT == DEF_ENABLED)
        if(m_pDMA == DMA2)
        {
                ChannelNumber += 7U;
        }
    #endif
        m_pDMAMUX_Channel = reinterpret_cast<DMAMUX_Channel_TypeDef*>(
        uintptr_t(DMAMUX1_Channel0) + ChannelNumber * sizeof(DMAMUX_Channel_TypeDef));
        m_pDMAMUX_ChannelStatus = DMAMUX1_ChannelStatus;
        m_DMAMUX_ChannelStatusMask = 1U << ChannelNumber;
}

void DMA_Driver::CalcDMAMUX_RequestGenBaseAndMask(uint32_t Request)
{
        uint32_t GeneratorNumber = Request - DMA_REQUEST_GENERATOR0;
        m_pDMAMUX_RequestGen = reinterpret_cast<DMAMUX_RequestGen_TypeDef*>(
        uintptr_t(DMAMUX1_RequestGenerator0) + GeneratorNumber * sizeof(DMAMUX_RequestGen_TypeDef));
        m_pDMAMUX_RequestGenStatus = DMAMUX1_RequestGenStatus;
        m_DMAMUX_RequestGenStatusMask = 1U << GeneratorNumber;
}
#endif

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
        m_pDMA_Channel->CMAR = uint32_t(pSource);
        m_pDMA_Channel->CPAR = uint32_t(pDestination);
    }
    else
    {
        m_pDMA_Channel->CMAR = uint32_t(pDestination);
        m_pDMA_Channel->CPAR = uint32_t(pSource);
    }
    m_pDMA_Channel->CNDTR = uint32_t(Length);
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
        m_pDMA_Channel->CMAR = uint32_t(pSource);
    }
    else
    {
        m_pDMA_Channel->CPAR = uint32_t(pSource);
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
        m_pDMA_Channel->CPAR = uint32_t(Destination);
    }
    else
    {
        m_pDMA_Channel->CMAR = uint32_t(Destination);
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

    if(intptr_t(m_pDMA) == DMA1_BASE)
    {
        pRegister = reinterpret_cast<volatile uint32_t*>(&DMA1->IFCR);
    }

  #if (DMA2_SUPPORT == DEF_ENABLED)
    else if(intptr_t(m_pDMA) == DMA2_BASE)
    {
        pRegister = reinterpret_cast<volatile uint32_t*>(&DMA2->IFCR);
    }
  #endif

    if(pRegister != nullptr)
    {
        SET_BIT(*pRegister, Flag);
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
    uint32_t Register = 0;
    uint32_t Result   = 0;

    if(intptr_t(m_pDMA) == DMA1_BASE)
    {
        Register = DMA1->ISR;
    }

  #if (DMA2_SUPPORT == DEF_ENABLED)
    else if(intptr_t(m_pDMA) == DMA2_BASE)
    {
        Register = DMA2->ISR;
    }
  #endif

    if((Register & Flag) != 0)
    {
        Result = 1;
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
        if(m_pDMA == DMA1)
    {
                SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);
    }
  #if (DMA2_SUPPORT == DEF_ENABLED)
        else if(m_pDMA == DMA2)
    {
                SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);
    }
  #endif
    #if defined(DMAMUX1)
        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN);
    #endif
        (void)RCC->AHB1ENR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableIRQ
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the IRQ DMA for the Channel and Stream
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableIRQ()
{
    ISR_Init(m_IRQn_Channel, m_PreempPrio);
}

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_DMA_MEM2MEM_FUNCTION == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           DMA_MEM2MEM_Initialize
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Initialize the DMA Memory to memory transfer
//
//-------------------------------------------------------------------------------------------------
void DMA_MEM2MEM_Initialize(void)
{
    nOS_MutexCreate(&MEM2MEM_Mutex, NOS_MUTEX_NORMAL, 1);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DMA_Memcpy
//
//  Parameter(s):   void*       pSource
//                  void*       pDestination
//                  size_t      Size
//
//  Return:         None
//
//  Description:    Initialize the DMA Memory to memory transfer
//
//-------------------------------------------------------------------------------------------------
void DMA_Memcpy(void* pSource, void* pDestination, size_t Size, uint32_t Parameter)
{
    if(nOS_MutexLock(&MEM2MEM_Mutex, DMA_MUTEX_GUARD_TIME) != NOS_OK)
    {
        return;
    }

    RCC->AHB1ENR |= DMA_MEM2MEM_ENABLE;
    DMA_MEM2MEM_STREAM->CR = 0;
    while(DMA_MEM2MEM_STREAM->CR & DMA_SxCR_EN){};
    DMA_MEM2MEM_MODULE->DMA_IFCR = DMA_FLAG;

    // Configure DMA
    DMA_MEM2MEM_STREAM->PAR  = (uint32_t)pSource;
    DMA_MEM2MEM_STREAM->M0AR = (uint32_t)pDestination;
    DMA_MEM2MEM_STREAM->NDTR = Size;
    DMA_MEM2MEM_STREAM->CR   = (DMA_MEM2MEM_CHANNEL << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_DIR_1 | Parameter;
    DMA_MEM2MEM_STREAM->CR  |= DMA_SxCR_EN;

    // Wait for completion
    while((DMA_MEM2MEM_MODULE->DMA_ISR & DMA_TCIF) == 0) nOS_Yield();
    DMA_MEM2MEM_MODULE->DMA_IFCR = DMA_FLAG;
    DMA_MEM2MEM_STREAM->CR &= ~DMA_SxCR_EN;
    while(DMA_MEM2MEM_STREAM->CR & DMA_SxCR_EN);

    nOS_MutexUnlock(&MEM2MEM_Mutex);
}

#endif // (DIGINI_USE_DMA_MEM2MEM_FUNCTION == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
