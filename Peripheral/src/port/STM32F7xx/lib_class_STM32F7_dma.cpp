//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_dma.cpp
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
//  Function:       Initialize
//
//  Parameter(s):   DMA_ID_e        DMA_ID
//  Return:         None
//
//  Description:    Initialize the DMA pointer 
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Initialize(DMA_ID_e DMA_ID)
{
    // link to physical device;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetStreamRX
//
//  Parameter(s):   Source
//                  Destination
//                  Length
//  Return:         None
//
//  Description:    Setup receive data from peripheral for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetStreamRX(void* pSource, void* pDestination, size_t Length)
{
    m_pDMA->M0AR = uint32_t(pDestination);
    m_pDMA->PAR  = uint32_t(pSource);
    m_pDMA->NDTR = uint32_t(Length);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetStreamTX
//
//  Parameter(s):   Source
//                  Destination
//                  Length
//  Return:         None
//
//  Description:    Setup transmit data to peripheral for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetStreamTX(void* pSource, void* pDestination, size_t Length)
{
    m_pDMA->M0AR = uint32_t(pSource);
    m_pDMA->PAR  = uint32_t(pDestination);
    m_pDMA->NDTR = uint32_t(Length);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearFlag
//
//  Parameter(s):   Flag        to clear
//  Return:         None
//
//  Description:    Clear flag for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::ClearFlag(uint32_t Flag)
{
    volatile uint32_t* pRegister;

    switch(intptr_t(m_pDMA))
    {
        case (uint32_t)DMA1_Stream0_BASE:
        case (uint32_t)DMA1_Stream1_BASE:
        case (uint32_t)DMA1_Stream2_BASE:
        case (uint32_t)DMA1_Stream3_BASE: pRegister = &DMA1->LIFCR; break;

        case (uint32_t)DMA1_Stream4_BASE:
        case (uint32_t)DMA1_Stream5_BASE:
        case (uint32_t)DMA1_Stream6_BASE:
        case (uint32_t)DMA1_Stream7_BASE: pRegister = &DMA1->HIFCR; break;

        case (uint32_t)DMA2_Stream0_BASE:
        case (uint32_t)DMA2_Stream1_BASE:
        case (uint32_t)DMA2_Stream2_BASE:
        case (uint32_t)DMA2_Stream3_BASE: pRegister = &DMA2->LIFCR; break;

        case (uint32_t)DMA2_Stream4_BASE:
        case (uint32_t)DMA2_Stream5_BASE:
        case (uint32_t)DMA2_Stream6_BASE:
        case (uint32_t)DMA2_Stream7_BASE: pRegister = &DMA2->HIFCR; break;
    }

    SET_BIT(*pRegister, Flag);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CheckFlag
//
//  Parameter(s):   Flag        To check
//  Return:         None
//
//  Description:    Check flag for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
uint32_t DMA_Driver::CheckFlag(uint32_t Flag)
{
    volatile uint32_t Register;
    uint32_t          Result     = 0;

    switch(intptr_t(m_pDMA))
    {
        case (uint32_t)DMA1_Stream0_BASE:
        case (uint32_t)DMA1_Stream1_BASE:
        case (uint32_t)DMA1_Stream2_BASE:
        case (uint32_t)DMA1_Stream3_BASE: Register = DMA1->LISR; break;

        case (uint32_t)DMA1_Stream4_BASE:
        case (uint32_t)DMA1_Stream5_BASE:
        case (uint32_t)DMA1_Stream6_BASE:
        case (uint32_t)DMA1_Stream7_BASE: Register = DMA1->HISR; break;

        case (uint32_t)DMA2_Stream0_BASE:
        case (uint32_t)DMA2_Stream1_BASE:
        case (uint32_t)DMA2_Stream2_BASE:
        case (uint32_t)DMA2_Stream3_BASE: Register = DMA2->LISR; break;

        case (uint32_t)DMA2_Stream4_BASE:
        case (uint32_t)DMA2_Stream5_BASE:
        case (uint32_t)DMA2_Stream6_BASE:
        case (uint32_t)DMA2_Stream7_BASE: Register = DMA2->HISR; break;
    }

    if((Register & Flag) != 0)
    {
        Result = 1;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Enable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Enable(void)
{
    SET_BIT(m_pDMA->CR, DMA_SxCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Disable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Disable(void)
{
    CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableInterrupt
//
//  Parameter(s):   Interrupt   Interrupt to enable
//  Return:         None
//
//  Description:    Enable a group of interrupt for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableInterrupt(uint32_t Interrupt)
{
    SET_BIT(m_pDMA->CR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisableInterrupt
//
//  Parameter(s):   Interrupt   Interrupt to enable
//  Return:         None
//
//  Description:    Disable a group of interrupt for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::DisableInterrupt(uint32_t Interrupt)
{
    CLEAR_BIT(m_pDMA->CR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableTransmitCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableTransmitCompleteInterrupt((void)
{
    SET_BIT(m_pDMA->CR, DMA_SxCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisableTransmitCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::DisableTransmitCompleteInterrupt(void)
{
    CLEAR_BIT(m_pDMA->CR, DMA_SxCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableTransmitHalfCompleteInterrupt(void)
{
    SET_BIT(m_pDMA->CR, DMA_SxCR_HTIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::DisableTransmitHalfCompleteInterrupt(void)
{
    CLEAR_BIT(m_pDMA->CR, DMA_SxCR_HTIE);
}

//-------------------------------------------------------------------------------------------------
