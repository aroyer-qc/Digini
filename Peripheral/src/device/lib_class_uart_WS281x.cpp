//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_pwm_WS281x.cpp
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

#define LIB_WS281x_GLOBAL
#include "./lib_digini.h"
#undef  LIB_WS281x_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_UART_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define WS281x_RGB_SIZE                 3
#define WS281x_LED_BUFFER_SIZE          24                                  // One led need 24 Bits for the color.

#if (WS281x_USE_PRECALCULATED_PWM_BUFFER == DEF_ENABLED)
  #define WS281x_DMA_FULL_BUFFER_SIZE       WS281x_LED_BUFFER_SIZE
#else
  #define WS281x_DMA_FULL_BUFFER_SIZE       2 * WS281x_LED_BUFFER_SIZE          // DMA need 2 leds.  ?? why
#endif

#define WS2812x_CALC_EXTRA_FOR_RESET(R)     (((R * 10) / 125) * 10)

//-------------------------------------------------------------------------------------------------
//  private variable(s)
//-------------------------------------------------------------------------------------------------

const WS2812x_MethodData_t WS281x::m_Methods[NUMBER_OF_METHODS] =
{
  #if (WS281x_USE_SK6812 == DEF_ENABLED)
    {6, 18, WS2812x_CALC_EXTRA_FOR_RESET(80)},
  #endif
  #if (WS281x_USE_WS2811 == DEF_ENABLED)
    {5, 12, WS2812x_CALC_EXTRA_FOR_RESET(50)},
  #endif
  #if (WS281x_USE_WS2812 == DEF_ENABLED)
    {7, 14, WS2812x_CALC_EXTRA_FOR_RESET(50)},
  #endif
  #if (WS281x_USE_WS2812B == DEF_ENABLED)
    {8, 16, WS2812x_CALC_EXTRA_FOR_RESET(50)},
  #endif
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Constructor
//
//  Parameter(s):   const WS281x_Config_t* pConfig
//
//  Description:    Initialize configuration
//
//-------------------------------------------------------------------------------------------------
WS281x::WS281x(const WS281x_Config_t* pConfig)
{
    m_Method       = pConfig->Method;
    m_NumberOfLED  = pConfig->NumberOfLED;                                                                // Number of real LEDs.
    m_DMA.Initialize((DMA_Info_t*)&pConfig->DMA_Info);
    m_pUART_Driver = pConfig->pUART_Driver;
}

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
void WS281x::Initialize()
{
    size_t BufferSize;

    m_pUART_Driver->SetCustomBaudRate(8400000);

    BufferSize        = (WS281x_DMA_FULL_BUFFER_SIZE * m_NumberOfLED) + WS281x::m_Methods[m_Method].ResetTime;  // We need to add the reset time equivalent for the reset
    m_IsItinFirstHalfOfBuffer = true;

    m_pLedChain       = (WS281x_Color_t*)pMemoryPool->AllocAndClear(m_NumberOfLED * WS281x_RGB_SIZE);           // Reserved x bytes  from the alloc mem library.
    m_pDMA_Buffer     = (uint8_t*)pMemoryPool->AllocAndSet(BufferSize, WS281x::m_Methods[m_Method].T0H);        // Reserved 48 bytes DMA transfer to compare register multiply by the number of LED.

    m_IsItinFirstHalfOfBuffer = true;
    memset(&m_pDMA_Buffer[BufferSize - WS281x::m_Methods[m_Method].ResetTime], 0, WS281x::m_Methods[m_Method].ResetTime); // We need to add the reset time equivalent for the reset

  #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
    m_NeedRefresh = true;
  #endif

    // USE DMA to change compare value in register of the timer
    //     - If WS281x_USE_PRECALCULATED_PWM_BUFFER is DEF_DISABLED
    //          use 2 x 24 bits leds. So take advantage of the HT and TC of the DMA in a circular mode...
    //          So we can use those to have time to prepare next set of 24 bit colors for next led
    //          loop continously the 48 bytes compare value buffer
    //      - If WS281x_USE_PRECALCULATED_PWM_BUFFER is DEF_ENABLED
    //          We loop to the entire buffer of value for the LEDs
    //          HT and TC are only used to signify the code were the DMA is, so the code can safely
    //          change the LED value.
   // m_pUART_Driver->DMA_ConfigTX
   // m_pUART_Driver->DMA_EnableTX();
    m_pUART_Driver->SendData(m_pDMA_Buffer, &BufferSize);
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
    m_DMA.Enable();
    m_pUART_Driver->Enable();
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
    m_DMA.Disable();
    m_pUART_Driver->Disable();
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
  #if (WS281x_USE_PRECALCULATED_PWM_BUFFER == DEF_DISABLED)
    if((m_NeedRefresh == true) && (m_ResetCount == 0))
    {
        m_LedPointer  = 0;
        m_NeedRefresh = false;
        m_ResetCount  = m_SetCountReset;                 // This make sure no restart is done before it is done properly.
        Start();
    }
  #endif
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
    bool AtStartValue;

Color.Blue /= 4;
Color.Green /= 4;
Color.Red /= 4;

    if(Offset < m_NumberOfLED)                                                 // Offset must inside length of chain
    {
        if(memcmp(&m_pLedChain[Offset], &Color, sizeof(WS281x_Color_t)) != 0)   // Only refresh if color is different
        {
            m_pLedChain[Offset] = Color;
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
//  Description:    IF mode WS281x_USE_PRECALCULATED_PWM_BUFFER is DEF_DISABLED
//                      - DMA interrupt, to fill up next 24 Bits value into DMA buffer for compare
//                        timer register.
//                  If DEF_ENABLED
//                      - DMA only set/reset the flag that signify which part of the buffer is been
//                        processed
//
//  Note(s):        Each bit, is a uint8_t value into a 24 bytes buffer, for DMA to process.
//                  Double buffer with half transfert IRQ. so prepare next 24 bit while sending
//
//-------------------------------------------------------------------------------------------------

void WS281x::DMA_Channel_IRQ_Handler(bool IsItTransferComplete)
{
    if(IsItTransferComplete == true)
    {
//GPIOA->BSRR = 0x04;
        m_IsItinFirstHalfOfBuffer = false;
    }
    else
    {
//GPIOA->BSRR = 0x08;
        m_IsItinFirstHalfOfBuffer = true;
    }

//GPIOA->BSRR = 0xC0000;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_UART_DRIVER == DEF_ENABLED)

