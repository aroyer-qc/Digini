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

#if (USE_SPI_WS281X_DRIVER == DEF_ENABLED)
#if (USE_TIM_DRIVER == DEF_ENABLED)
#if (USE_PWM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define WS281x_RGB_SIZE                     3
#define WS281x_LED_BUFFER_SIZE              24                                  // One led need 24 Bits for the color.

#if (WS281x_USE_PRECALCULATED_PWM_BUFFER == DEF_ENABLED)
  #define WS281x_DMA_FULL_BUFFER_SIZE           WS281x_LED_BUFFER_SIZE
#else
  #define WS281x_DMA_FULL_BUFFER_SIZE           2 * WS281x_LED_BUFFER_SIZE          // DMA need 2 leds.  ?? why
#endif

#define WS2812x_CALC_EXTRA_FOR_RESET(R)         (((R * 10) / 125) * 10)

// not sure about this
#if defined(STM32F4) || defined(STM32F7) // speculative for F7 until test are done
#define WS2812x_CALC_BIT_SHIFT                  8
#endif
#ifdef STM32F1
#define WS2812x_CALC_BIT_SHIFT                  4
#endif

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
    m_Method      = pConfig->Method;
    m_NumberOfLED = pConfig->NumberOfLED;                                                                // Number of real LEDs.
    m_DMA.Initialize((DMA_Info_t*)&pConfig->DMA_Info);
    m_pPWM_Driver = pConfig->pPWM_Driver;
    m_pTIM_Driver = pConfig->pTIM_Driver;
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
    size_t BufferSize;                                                                              // Buffer size in bytes
    size_t Size =  sizeof(WS_uint_t);

    m_pTIM_Driver->Initialize();
    m_pPWM_Driver->Initialize();

//-------------------------------------------------------------------------------------------------

  #if(WS281x_USE_PRECALCULATED_PWM_BUFFER == DEF_ENABLED)
    size_t ResetTime = WS281x::m_Methods[m_Method].ResetTime;

    m_IsItinFirstHalfOfBuffer = true;
    BufferSize    = (WS281x_DMA_FULL_BUFFER_SIZE * m_NumberOfLED) + ResetTime;                      // We need to add the reset time equivalent for the reset.
    m_pLedChain   = (WS281x_Color_t*)pMemoryPool->AllocAndClear(m_NumberOfLED * WS281x_RGB_SIZE);   // Reserved x bytes for the LED chain from the alloc mem library.

   #ifdef STM32F1
    m_pDMA_Buffer = (WS_uint_t*)pMemoryPool->Alloc(BufferSize);                                     // Reserved array of WS_uint_t for DMA transfer.
    memset(&m_pDMA_Buffer[0], WS281x::m_Methods[m_Method].T0H,  BufferSize - ResetTime);            // Set all value to T0H
    memset(&m_pDMA_Buffer[BufferSize - ResetTime],  0,  ResetTime);                                 // We need to add the reset time equivalent for the reset
   #endif

   #ifdef STM32F4
    m_pDMA_Buffer = (WS_uint_t*)pMemoryPool->Alloc(BufferSize * Size);                              // Reserved array of WS_uint_t for DMA transfer.

    for(size_t i = 0; i < (BufferSize - ResetTime); i++)
    {
        m_pDMA_Buffer[i] = WS_uint_t(WS281x::m_Methods[m_Method].T0H);                              // Set all bit to the T0H value
    }

    memset(&m_pDMA_Buffer[BufferSize - ResetTime], 0, ResetTime * Size);                            // We clear the reset part of the transfer
   #endif
  #endif

//-------------------------------------------------------------------------------------------------

  #if (WS281x_USE_48_BITS_DMA_TRANSFER == DEF_ENABLED)     // todo rethink not working

    volatile uint16_t                   m_LedPointer;
    volatile uint32_t                   m_SetCountReset;
//    WS_uint_t                           m_pDMA_HalfBuffer;

    for(int i = 0; i < WS2812x_HALF_BYTE_TH_SIZE; i++)
    {
        for(int j = 0; j < WS2812x_NUMBER_OF_BITS_IN_HALF_BYTE; j++)
        {
            m_HalfBytesTH_Array[i].Level[j] = ((i & (1 << j)) == 0) ? WS281x::m_Methods[m_Method].T0H : WS281x::m_Methods[m_Method].T1H;
        }
    }

    BufferSize        = WS281x_DMA_FULL_BUFFER_SIZE;
    m_LedPointer      = 0;                                                                                                      // Start at Led 0
    m_SetCountReset   = 32000;//uint8_t((uint16_t(m_ResetType) / WS281x_TIMER_RANGE));
    m_ResetCount      = m_SetCountReset;
    m_pLedChain       = (WS281x_Color_t*)pMemoryPool->AllocAndClear(m_NumberOfLED * WS281x_RGB_SIZE);                           // Reserved x bytes  from the alloc mem library.
    m_pDMA_Buffer     = (WS_uint_t*)pMemoryPool->AllocAndSet(BufferSize * sizeof(WS_uint_t), WS281x::m_Methods[m_Method].T0H);    // Reserved 48 bytes DMA transfer to compare register multiply by the number of LED.
    m_pDMA_HalfBuffer = m_pDMA_Buffer + (BufferSize / 2);

   #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
    m_NeedRefresh = true;
   #endif

  #endif

//-------------------------------------------------------------------------------------------------

    // USE DMA to change compare value in register of the timer
    //     - If WS281x_USE_PRECALCULATED_PWM_BUFFER is DEF_DISABLED
    //          use 2 x 24 bits leds. So take advantage of the HT and TC of the DMA in a circular mode...
    //          So we can use those to have time to prepare next set of 24 bit colors for next led
    //          loop continously the 48 bytes compare value buffer
    //      - If WS281x_USE_PRECALCULATED_PWM_BUFFER is DEF_ENABLED
    //          We loop to the entire buffer of value for the LEDs
    //          HT and TC are only used to signify the code were the DMA is, so the code can safely
    //          change the LED value.
    m_DMA.SetSource(m_pDMA_Buffer);
    m_DMA.SetDestination(m_pPWM_Driver->GetCompareRegisterPointer());
    m_DMA.SetLength(BufferSize);
    m_DMA.EnableTransmitCompleteInterrupt();
    m_DMA.EnableTransmitHalfCompleteInterrupt();
    m_DMA.EnableIRQ();
}


// MOST of the crap is working on the F4... scrap into the reset..


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
    m_pPWM_Driver->Start();
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
    m_pPWM_Driver->Stop();
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

          #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
           #if (WS281x_SET_LED_TRIGGER_REFRESH == DEF_ENABLED)
            m_NeedRefresh = true;
           #endif
          #else
           #if (WS281x_USE_PRECALCULATED_PWM_BUFFER == DEF_ENABLED)
            AtStartValue = m_IsItinFirstHalfOfBuffer;

            while(AtStartValue == m_IsItinFirstHalfOfBuffer)
            {
                nOS_Sleep(1);
            };

            register uint8_t* pColorData;
            register uint8_t  Color;

            pColorData = (uint8_t*)&m_pLedChain[Offset];
            Offset *= 24;     // 24 bytes per LED

            for(int i = 0; i < WS281x_RGB_SIZE; i++)
            {
                Color = *pColorData;

                for(int j = 0x80; j != 0; j >>= 1)
                {
                    m_pDMA_Buffer[Offset++] = ((Color & j) == 0) ? WS_uint_t(WS281x::m_Methods[m_Method].T0H) : WS_uint_t(WS281x::m_Methods[m_Method].T1H);
                }

                pColorData++;
            }
           #endif
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

///------------------------------------------------------------------------------------------------

  #if (WS281x_USE_48_BITS_DMA_TRANSFER == DEF_ENABLED)
    register WS_uint_t* pBufferLow;
    register WS_uint_t* pBufferHigh;
    register uint8_t*   pColorData;
    register uint8_t    ColorLow;
    register uint8_t    ColorHigh;

    if(IsItTransferComplete == true)
    {
        pBufferLow  = m_pDMA_HalfBuffer;
        pBufferHigh = m_pDMA_HalfBuffer + 4;
    }
    else
    {
        pBufferLow  = m_pDMA_Buffer;
        pBufferHigh = m_pDMA_Buffer + 4;
    }

    if(m_LedPointer >= m_NumberOfLED)
    {
      #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
        Stop();
      #endif

        if(m_LedPointer == m_NumberOfLED)
        {
            m_LedPointer++;
///            memset(pBuffer, 0xFF, WS281x_LED_BUFFER_SIZE);      // Reset for this stream.
        }

      #if (WS281x_CONTINUOUS_SCAN == DEF_ENABLED)
        m_ResetCount--;                                         // Reset state until it reach 0

        if(m_ResetCount == 0)                                   // Restart the scan in continuous scan mode
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
            ColorLow  = (*pColorData & 0x0F);
            ColorHigh = (*pColorData >> 4);
            pColorData++;

            for(int j = 0; j < 4; j ++)
            {
                *pBufferHigh = m_HalfBytesTH_Array[ColorHigh].Level[j];
                *pBufferLow  = m_HalfBytesTH_Array[ColorLow ].Level[j];
                pBufferLow++;
                pBufferHigh++;
            }
        }

        m_LedPointer++;
    }
   #endif

///------------------------------------------------------------------------------------------------

  #if (WS281x_USE_PRECALCULATED_PWM_BUFFER == DEF_ENABLED)

    if(IsItTransferComplete == true)        // TODO use to prevent glitch while changing Led color!
    {
        m_IsItinFirstHalfOfBuffer = false;
    }
    else
    {
        m_IsItinFirstHalfOfBuffer = true;
    }

  #endif

///------------------------------------------------------------------------------------------------
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_WS281X_DRIVER == DEF_ENABLED)
#endif // (USE_PWM_DRIVER == DEF_ENABLED)
#endif // (USE_TIM_DRIVER == DEF_ENABLED)

