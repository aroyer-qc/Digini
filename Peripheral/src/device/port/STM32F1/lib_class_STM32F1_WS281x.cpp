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

#if (USE_TIM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define WS281x_TIMER_RANGE              25
#define WS281x_TIMER_RANGE              25
#define WS281x_RGB_SIZE                 3
#define WS281x_LED_BUFFER_SIZE          24                                  // One led need 24 Bits for the color.
#define WS281x_DMA_FULL_BUFFER_SIZE     2 * WS281x_LED_BUFFER_SIZE          // DMA need 2 leds.
#define WS281x_LOGICAL_0                7
#define WS281x_LOGICAL_1                14
#define WS281x_DUMMY_LEDS               2

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   WS281x_Config_t*       Configuration located in device_var.h
//
//  Return:         void
//
//  Description:    Initialize the WS281x interface
//
//-------------------------------------------------------------------------------------------------
void WS281x::Initialize(const WS281x_Config_t* pConfig)
{
    m_pPWM = new PWM_Driver(pConfig->PWM_ChannelID);
    m_pPWM->Initialize();

    m_SetCountReset   = uint8_t((uint16_t(pConfig->ResetType) / WS281x_TIMER_RANGE));
    m_ResetCount      = m_SetCountReset;
    m_NumberOfLED     = pConfig->NumberOfLED;                                                                                 // Number of real LEDs.
    m_LedChainSize    = m_NumberOfLED + WS281x_DUMMY_LEDS;                                                                    // Chain LEDs Size.
    m_pLedChain       = (WS281x_Color_t*)pMemoryPool->AllocAndClear((m_LedChainSize + WS281x_DUMMY_LEDS) * WS281x_RGB_SIZE);  // Reserved x bytes  from the alloc mem library.
    m_pDMA_Buffer     = (uint8_t*)pMemoryPool->AllocAndSet(WS281x_DMA_FULL_BUFFER_SIZE, WS281x_LOGICAL_0);                                    // Reserved 48 bytes DMA transfert to compare register.
    m_pDMA_HalfBuffer = m_pDMA_Buffer + WS281x_LED_BUFFER_SIZE;
    m_LedPointer    = 0;                                                                                                    // Start at Led 0
    m_pDMA          = pConfig->DMA_Channel;
    m_DMA_Flag      = pConfig->DMA_Flag;

  #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
    m_NeedRefresh   = true;
  #endif


  #if (DMA2_SUPPORT == DEF_ENABLED)
    SET_BIT(RCC->AHBENR, int(pConfig->TimID) <= int(TIM_DRIVER_ID_4) ? RCC_AHBENR_DMA1EN : \
                                                                       RCC_AHBENR_DMA2EN);                                  // Initialize DMA clock
  #else
    SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);                                                                                // Initialize DMA clock
  #endif

    // USE DMA to change compare value in register of the timer
    // use 2 x 24 bits leds. So take advantage of the HT and TC of the DMA in a circular mode...
    // So we can use those to have time to prepare next set of 24 bit colors for next led
    m_pDMA->CPAR  = uint32_t(m_pPWM->GetCompareRegisterPointer());
    m_pDMA->CMAR  = uint32_t(m_pDMA_Buffer);
    m_pDMA->CNDTR = WS281x_DMA_FULL_BUFFER_SIZE;
    m_pDMA->CCR   = DMA_CIRCULAR_MODE                |      // loop continously the 48 bytes compare value buffer
                    DMA_MEMORY_TO_PERIPHERAL         |
                    DMA_PERIPHERAL_NO_INCREMENT      |
                    DMA_MEMORY_INCREMENT             |
                    DMA_PERIPHERAL_SIZE_16_BITS      |      // ??? check if it works.
                    DMA_MEMORY_SIZE_8_BITS           |
                    DMA_PRIORITY_LEVEL_HIGH          |
                    DMA_HALF_TRANSFERT_IRQ           |
                    DMA_TRANSFER_COMPLETE_IRQ        |
                    DMA_START_TRANSFERT;

    ISR_Init(pConfig->IRQn_Channel, pConfig->PreempPrio);
    Start();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start the LED display process
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Start the DMA transfer of the serialize stream of data if required
//
//-------------------------------------------------------------------------------------------------
void WS281x::Start(void)
{
    m_pPWM->Start();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start the LED display process
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Start the DMA transfer of the serialize stream of data if required
//
//-------------------------------------------------------------------------------------------------
void WS281x::Stop(void)
{
    m_pPWM->Stop();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Refresh
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Running process to update LED chain if necessary
//
//-------------------------------------------------------------------------------------------------
#if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
void WS281x::Refresh(void)
{
    if((m_NeedRefresh == true) && (m_ResetCount == 0))
    {
        m_LedPointer  = 0;
        m_NeedRefresh = false;
        m_ResetCount  = m_SetCountReset;                 // This make sure no restart is done before it is done properly.
        Start();
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetLed
//
//  Parameter(s):   uint32_t         Offset                  Led to change color
//                  WS281x_Color_t   Color                   Color of the LED in RGB888
//
//  Return:         None
//
//  Description:    Set the Color of a LED
//
//-------------------------------------------------------------------------------------------------
void WS281x::SetLed(uint32_t Offset, WS281x_Color_t Color)
{
    if(Offset < m_LedChainSize)                                                 // Offset must inside length of chain
    {
        if(memcmp(&m_pLedChain[Offset], &Color, sizeof(WS281x_Color_t)) != 0)   // Only refresh if color is different
        {
            m_pLedChain[Offset] = Color;

          #if ((WS281x_SET_LED_TRIGGER_REFRESH == DEF_DISABLED) || (WS281x_CONTINUOUS_SCAN == DEF_DISABLED))
            m_NeedRefresh = true;
          #endif
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DMA_Channel_IRQ_Handler
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    DMA interrupt, to fill up next 24 Bits value into DMA buffer for compare timer
//                  register.
//
//  Note(s):        Each bit, is a uint8_t value into a 24 bytes buffer, for DMA to process.
//                  Double buffer with half transfert IRQ. so prepare next 24 bit while sending
//
//-------------------------------------------------------------------------------------------------

void WS281x::DMA_Channel_IRQ_Handler(bool IsItTransferComplete)
{
    register uint8_t* pBuffer;
    register uint8_t* pColorData;
    register uint8_t  Color;

    if(IsItTransferComplete == true) pBuffer = m_pDMA_Buffer;
    else                             pBuffer = m_pDMA_HalfBuffer;

    if(m_LedPointer > m_NumberOfLED)
    {
        m_ResetCount--;     // Reset state until it reach 0

      #if (WS281x_CONTINUOUS_SCAN == DEF_ENABLED)
        if(m_ResetCount == 0)                           // Restart the scan in continuous scan mode
        {
            m_ResetCount = m_SetCountReset;
            m_LedPointer = 0;
        }
      #endif
    }
    else
    {
        pColorData = (uint8_t*)&m_pLedChain[m_LedPointer];

        for(int i = 0; i < WS281x_RGB_SIZE; i++)
        {
            Color = *pColorData;

            for(int j = 0x80; j != 0; j >>= 1)
            {
                *pBuffer = ((Color & j) == 0) ? WS281x_LOGICAL_0 : WS281x_LOGICAL_1;
                pBuffer++;
            }

            pColorData++;
        }

        m_LedPointer++;
    }


}

//-------------------------------------------------------------------------------------------------

#endif // (USE_TIM_DRIVER == DEF_ENABLED)

