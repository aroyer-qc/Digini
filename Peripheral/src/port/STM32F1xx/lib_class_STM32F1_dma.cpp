//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_dma.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define DMA_DRIVER_GLOBAL
#include "./Digini/lib_digini.h"
#undef  DMA_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DMA_DIRECTION_MASK             (DMA_CCR_DIR_Msk | DMA_CCR_MEM2MEM_Msk)

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
    m_pDMA              = pInfo->pDMA;
    m_pDMA_Channel      = pInfo->pDMA_Channel;
    EnableClock();
    m_pDMA_Channel->CCR = pInfo->Config;
    m_Direction         = pInfo->Config & DMA_DIRECTION_MASK;
    //m_CallBackType = DMA_CALLBACK_NONE;
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
    if((intptr_t(m_pDMA) & DMA1_BASE) == DMA1_BASE)
    {
        SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
    }
  #if (DMA2_SUPPORT == DEF_ENABLED)
    else
    {
        SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA2EN);
    }
  #endif
}

/*   i don't know if it is needed, so far it's not
//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableCallbackType
//
//  Parameter(s):   CallBackType    Type of the ISR callback
//  Return:         None
//
//  Description:    Enable the type of interrupt for the callback.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableCallbackType(int CallBackType)
{
    if((CallBackType & DMA_CALLBACK_HALF_TRANSFER) != 0)
    {
        m_CallBackType |= CallBackType;
    }

    if((CallBackType & DMA_CALLBACK_COMPLETED_TRANSFER) != 0)
    {
        m_CallBackType |= CallBackType;
    }
}
*/

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
