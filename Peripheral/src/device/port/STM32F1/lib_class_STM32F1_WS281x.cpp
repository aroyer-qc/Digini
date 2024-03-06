//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_WS281x.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_WS281x_GLOBAL
#include "lib_class_STM32F1_WS281x.h"
#undef  LIB_WS281x_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define WS281x_RGB_SIZE                 3
#define WS281x_DMA_HALF_BUFFER_SIZE     WS281x_RGB_SIZE * sizeof(uint8_t)
#define WS281x_DMA_FULL_BUFFER_SIZE     2 * WS281x_DMA_HALF_BUFFER_SIZE

#define WS281x_LOGICAL_0                7
#define WS281x_LOGICAL_1                14

#define WS281x_DUMMY_LEDS               2

#define WS281x_RESET_CYCLE_COUNT        240         // Give 300 uSec reset time

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   uint16_t            NumberOfLED         Number of LED in the chain
//                  WS281x_ResetType_e  ResetType           Reset LED chip ID
//                  IO_ID_e             NeoDataPin          Pin ID of the IO
//
//  Return:         void
//
//  Description:    Initialize the WS281x interface
//
//-------------------------------------------------------------------------------------------------
void WS281x::Initialize(uint16_t NumberOfLED, WS281x_ResetType_e ResetType, IO_ID_e NeoDataPin)
{
    DMA_Channel_TypeDef* DMA_Channel;

    m_PWM_Driver
    
    
    // USE DMA to change Compare value in register of the timer
    // use 2 x 24 bits leds. So take advantage of the HT and TC of the DMA in a circular mode...
    // so we can use those to have time to prepare next set of 24 bit colors for next led

    IO_PinInit(NeoDataPin);
    
    m_ChainSize   = NumberOfLED + WS281x_DUMMY_LEDS;                                           // Chain LEDs Size.
    m_NumberOfLED = NumberOfLED;                                                               // Number of real LEDs.
    m_pLedChain   = (WS281x_Color_t*)pMemory->AllocAndClear(m_ChainSize + WS281x_DUMMY_LEDS);  // Reserved x bytes from the alloc mem library.
    m_pDMA_Buffer = pMemory->AllocAndClear(WS281x_DMA_FULL_BUFFER_SIZE);                       // Reserved 48 bytes DMA transfert to compare register.
    m_LedPointer  = 0;                                                                         // Start at Led 0
    m_NeedRefresh = true;

    // Configure timer
        // configure prescaler
        // Set compare channel value at zero... not started
        // configure auto-reload                800 Khz
        // Enable output of the compare channel
        // Enable timer and compare channel
        
            SET_BIT(RCC->AHBENR, m_pDMA_Info->RCC_AHBxPeriph);                  // Initialize DMA clock

            pDMA = m_pDMA_Info->DMA_ChannelRX;                                  // Write config that will never change
            pDMA->CCR = DMA_NORMAL_MODE                    |
                        DMA_MEMORY_TO_PERIPHERAL           |
                        DMA_PERIPHERAL_NO_INCREMENT        |
                        DMA_MEMORY_INCREMENT               |
                        DMA_PERIPHERAL_SIZE_16_BITS        |
                        DMA_MEMORY_SIZE_8_BITS             |
                        DMA_PRIORITY_LEVEL_HIGH;
            pDMA->CPAR = uint32_t(&m_pUart->DR);
            pDMA->CNDTR = UART_DRIVER_INTERNAL_RX_BUFFER_SIZE;
    

    
        // Configure Source 8 Bits memory, Destination 16 Bits compare register ...  checkif it works.
        // Configure burst at 1
        // Configure Circulat buffer size 48 bytes (WS281x_DMA_FULL_BUFFER_SIZE).
        // Configure DMA into HT and TC interrupt.
        // configure the trigger to be the compare channel..

    StartTransfer();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Start the DMA transfer of the serialize stream of data if required
//
//-------------------------------------------------------------------------------------------------
void WS281x::Process(void)
{
    if((m_NeedRefresh == true) && (m_ResetCount == 0))
    {
        m_LedPointer  = 0;
        m_NeedRefresh = false;
        m_ResetCount = WS281x_RESET_CYCLE_COUNT;        // This make sure no restart is done before
                                                        // it is done properly.
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetLed
//
//  Parameter(s):   uint16_t         Offset                  Led to change color      
//                  WS281x_Color_t   Color                   Color of the LED in RGB888
//
//  Return:         None
//
//  Description:    Set the Color of a LED
//
//-------------------------------------------------------------------------------------------------
void WS281x::SetLed(uint16_t Offset, WS281x_Color_t Color)
{
    if(Offset < m_LedChainSize)
    {
        if(m_pLedChain[Offset] != Color)
        {
            m_pLedChain[Offset] = Color;
            m_NeedRefresh       = true;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FillUp_24_Bits
//
//  Parameter(s):   uint8_t* pBuffer            24 Bytes buffer to fill-up
//
//
//  Return:         None
//
//  Description:    DMA interrupt, to fill up next 24 Bits value into DMA buffer for compare timer
//                  register.
//
//  Note(s):        Each bit, is a uint8_t value into a 24 bytes buffer, for DMA to process.
//
//-------------------------------------------------------------------------------------------------
void WS281x::FillUp_24_Bits(uint8_t* pBuffer)
{
    uint8_t* pColorData;
    uint8_t  Color;
    
    if(m_LedPointer > m_NumberOfLED)
    {
        m_ResetCount--;     // Reset State until it reach 0
    }
    else
    {
        pColorData = &m_pLedChain[m_LedPointer];
        
        for(int i = 0; i < WS281x_RGB_SIZE, i++)
        {        
            Color = *pColorData;

            for(int j = 0x80; j >= 0; j >>= 1)
            {
                *pBuffer = ((Color & j) == 0) ? WS281x_LOGICAL_0 : WS281x_LOGICAL_1;
                pBuffer++;
            }
            
            pColorData++;
        }

        m_LedPointer++;
    }
    else
}

//-------------------------------------------------------------------------------------------------

// We double buffer with half transfert IRQ. so prepare next 24 bit while sending first one


void WS281x::DMA_Stream_IRQ_Handler(TIM_ID_e TIM_ID)
{
    TIM_Info_t*         pInfo;
    DMA_Stream_TypeDef* pDMA;
    uint32_t            Flag;

    pInfo = &TIM_Info[TIM_ID];
    pInfo->pObject->m_DMA_Status = SYS_BUSY_B4_RELEASE;
    pDMA = pInfo->DMA_Stream;
    Flag = pInfo->TX_IT_Flag;
    DMA_ClearFlag(pDMA, Flag);
    
    
    
    // call the fill_up function for output.. or integrate directly here (or inline)
}

//-------------------------------------------------------------------------------------------------

