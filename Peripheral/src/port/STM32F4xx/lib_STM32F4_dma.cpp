//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_dma.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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
// Public Function
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_ClearFlag
//
//  Parameter(s):   pDMA        DMA stream to modify
//                  Flag        to clear
//  Return:         None
//
//  Description:    Clear flag for specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_ClearFlag(DMA_Stream_TypeDef* pDMA, uint32_t Flag)
{
    volatile uint32_t* pRegister;

    switch(intptr_t(pDMA))
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

    SET_BIT(*pRegister, Flag);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_CheckFlag
//
//  Parameter(s):   pDMA        DMA stream to modify
//                  Flag        To check
//  Return:         None
//
//  Description:    Check flag for specific DMA stream.
//-------------------------------------------------------------------------------------------------
uint32_t DMA_CheckFlag(DMA_Stream_TypeDef* pDMA, uint32_t Flag)
{
    volatile uint32_t Register;
    uint32_t          Result     = 0;

    switch(intptr_t(pDMA))
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
//  Parameter(s):   pDMA        DMA stream to enable
//  Return:         None
//
//  Description:    Enable a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_Enable(DMA_Stream_TypeDef* pDMA)
{
    SET_BIT(pDMA->CR, DMA_SxCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Disable
//
//  Parameter(s):   pDMA        DMA stream to disable
//  Return:         None
//
//  Description:    Disable a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_Disable(DMA_Stream_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableInterrupt
//
//  Parameter(s):   pDMA        DMA stream
//                  Interrupt   Interrupt tn enable
//  Return:         None
//
//  Description:    Enable a group of interrupt for specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_EnableInterrupt(DMA_Stream_TypeDef* pDMA, uint32_t Interrupt)
{
    SET_BIT(pDMA->CR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableInterrupt
//
//  Parameter(s):   pDMA        DMA stream
//                  Interrupt   Interrupt tn enable
//  Return:         None
//
//  Description:    Disable a group of interrupt for specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_DisableInterrupt(DMA_Stream_TypeDef* pDMA, uint32_t Interrupt)
{
    CLEAR_BIT(pDMA->CR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableTransmitCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA stream to enable
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_EnableTransmitCompleteInterrupt(DMA_Stream_TypeDef* pDMA)
{
    SET_BIT(pDMA->CR, DMA_SxCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableTransmitCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA stream to enable
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_DisableTransmitCompleteInterrupt(DMA_Stream_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CR, DMA_SxCR_TCIE);
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
void DMA_EnableTransmitHalfCompleteInterrupt(DMA_Stream_TypeDef* pDMA)
{
    SET_BIT(pDMA->CR, DMA_SxCR_HTIE);
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
void DMA_DisableTransmitHalfCompleteInterrupt(DMA_Stream_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CR, DMA_SxCR_HTIE);
}

//---------------------------------------------------------------------------------------------------------------------------------
