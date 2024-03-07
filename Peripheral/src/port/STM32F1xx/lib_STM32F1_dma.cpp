//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F1_dma.cpp
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
#include "lib_digini.h"
#undef  DMA_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_ClearFlag
//
//  Parameter(s):   pDMA        DMA channel to modify flag
//                  Flag        to clear
//  Return:         None
//
//  Description:    Clear flag for specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_ClearFlag(DMA_Channel_TypeDef* pDMA, uint32_t Flag)
{
    volatile uint32_t* pRegister = nullptr;

    if((intptr_t(pDMA) & DMA1_BASE) == DMA1_BASE)
    {
        pRegister = reinterpret_cast<volatile uint32_t*>(&DMA1->IFCR);
    }

  #if (DMA2_SUPPORT == DEF_ENABLED)
    else if((intptr_t(pDMA) & DMA2_BASE) == DMA2_BASE)
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
//  Function:       DMA_CheckFlag
//
//  Parameter(s):   pDMA        DMA channel to check flag
//                  Flag        To check
//  Return:         None
//
//  Description:    Check flag for specific DMA channel.
//-------------------------------------------------------------------------------------------------
uint32_t DMA_CheckFlag(DMA_Channel_TypeDef* pDMA, uint32_t Flag)
{
    uint32_t Register = 0;
    uint32_t Result   = 0;

    if((intptr_t(pDMA) & DMA1_BASE) == DMA1_BASE)
    {
        Register = DMA1->ISR;
    }

  #if (DMA2_SUPPORT == DEF_ENABLED)
    else if((pDMA & DMA2_BASE) == DMA2_BASE)
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
//  Function:       DMA_Enable
//
//  Parameter(s):   pDMA        DMA channel to enable
//  Return:         None
//
//  Description:    Enable a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_Enable(DMA_Channel_TypeDef* pDMA)
{
    SET_BIT(pDMA->CCR, DMA_CCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Disable
//
//  Parameter(s):   pDMA        DMA channel to disable
//  Return:         None
//
//  Description:    Disable a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_Disable(DMA_Channel_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CCR, DMA_CCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableInterrupt
//
//  Parameter(s):   pDMA        DMA channel
//                  Interrupt   Interrupt tn enable
//  Return:         None
//
//  Description:    Enable a group of interrupt for specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_EnableInterrupt(DMA_Channel_TypeDef* pDMA, uint32_t Interrupt)
{
    SET_BIT(pDMA->CCR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableInterrupt
//
//  Parameter(s):   pDMA        DMA channel
//                  Interrupt   Interrupt tn enable
//  Return:         None
//
//  Description:    Disable a group of interrupt for specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_DisableInterrupt(DMA_Channel_TypeDef* pDMA, uint32_t Interrupt)
{
    CLEAR_BIT(pDMA->CCR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableTransmitCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA channel to enable
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_EnableTransmitCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    SET_BIT(pDMA->CCR, DMA_CCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableTransmitCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA channel to enable
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_DisableTransmitCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CCR, DMA_CCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA stream to enable
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_EnableTransmitHalfCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    SET_BIT(pDMA->CCR, DMA_CCR_HTIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA stream to enable
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_DisableTransmitHalfCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CCR, DMA_CCR_HTIE);
}

//---------------------------------------------------------------------------------------------------------------------------------
