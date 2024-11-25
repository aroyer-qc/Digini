//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_uart_WS281x.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

//#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_UART_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Note(s)
//-------------------------------------------------------------------------------------------------
//
// It use too complex feature of timer to be generic or use timer
//
// It need a timer with DMA and compare register.
//
//  Note(s):        The Frequency for the smart LED are 800 kHz.
//
//                  Logical 0        ____________                      __
//                             ...__|            \____________________|  ...
//                         SK6812   <-- 300 nS --><-----  950 nS ----->
//                         WS2811   <-- 250 nS --><----- 1000 nS ----->
//                         WS2812   <-- 350 nS --><-----  900 nS ----->
//                         WS2812B  <-- 400 nS --><-----  850 nS ----->
//
//                  Logical 1        ___________________               __
//                             ...__|                   \_____________|  ...
//                         SK6812   <----- 600 nS -----><--- 650 nS -->
//                         WS2811   <----- 600 nS -----><--- 650 nS -->
//                         WS2812   <----- 700 nS -----><--- 550 nS -->
//                         WS2812B  <----- 800 nS -----><--- 450 nS -->
//
//                                  <---------- 1.25uSec ------------>
//                  _______________________________________________________________________________
//
//                  Reset         __                                   __
//                             ...  \_________________________________|  ...
//                                  <--------- ResetTime (1) -------->
//
//                  (1) Minimum reset pulse length depends on WS281x device. Check datasheet for
//                      your particular unit.
//
//                  _______________________________________________________________________________
//
//                  The Best granularity here will be 50 nSec, for a frequency of 20 MHz.
//
//
//                  SK6812MINI, SK6812, SK6812W and SK6812WWA
//                      Range for 1.25 uSec
//                      T0H:  300 nSec  =  6 Counts
//                      T1H:  600 nSec  = 12 Counts
//                      Auto Reload     = 25 Counts
//                      Reset           = > 80 uSec
//
//                  WS2811
//                      Range for 1.25 uSec
//                      T0H:  250 nSec  =  5 Counts
//                      T1H:  600 nSec  = 12 Counts
//                      Auto Reload     = 25 Counts
//                      Reset           = > 50 uSec
//
//                  WS2812
//
//                      Range for 1.25 uSec
//                      T0H:  350 nSec  = 7  Counts
//                      T1H:  700 nSec  = 14 Counts
//                      Auto Reload     = 25 Counts
//                      Reset           = > 50 uSec
//
//                  WS2812B and WS2813
//
//                      Range for 1.25 uSec
//                      T0H:  400 nSec  =  8 Counts
//                      T1H:  800 nSec  = 16 Counts
//                      Auto Reload     = 25 Counts
//                      Reset           = > 50 uSec
//                  _______________________________________________________________________________
//
//                  There more entry in the m_pLedChain array and there value are set to zero.
//                  when those LED entry are reach, DMA continue as nothing happened, but value is
//                  set to zero, so this emulate a reset.
//
//                  Maybe offer 2 mode, continuous scanning (animated led stream) and single scan
//                  for refresh on change only.
//
//  Example of IRQ call for DMA to put into irq.cpp of the append
//
//     NOS_ISR(DMA1_Channel5_IRQHandler)
//     {
//         WS281x_LedStream.DMA_Channel_IRQ_Handler((DMA_CheckFlag(DMA1_Channel5, DMA_IFCR_CTCIF5))
//                                                  ? true: false);
//     }
//
//
//   Note(s)
//          - If the CPU is too slow, then use the setting WS281x_USE_PRECALCULATED_PWM_BUFFER
//
//          - With STM32F1, It Better to use WS281x_USE_PRECALCULATED_PWM_BUFFER if not to much
//            time will be spent into DMA IRQ
//          - With STM32F4, It cannot use WS281x_USE_PRECALCULATED_PWM_BUFFER because the DMA as a
//            a bug. It cannot transfert a 8 bits to 16 Bits registers compare register.
//            It shadow the low part of the value for the compare register into high 8 bits value.
//            The result will be to double the buffer size for the compare value.
//          - With STM32F7   Not tested YET.
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#ifndef WS281x_USE_PRECALCULATED_PWM_BUFFER
  #define WS281x_USE_PRECALCULATED_PWM_BUFFER   DEF_DISABLED
#endif

// This enable the continuous scan of the LED stream. No refresh needed.
#ifndef WS281x_CONTINUOUS_SCAN
  #define WS281x_CONTINUOUS_SCAN                DEF_DISABLED
#endif

// If a LED change is done, it will trigger a refresh automatically is it is DEF_ENABLED.
// If has no effect if WS281x_CONTINUOUS_SCAN is DEF_ENABLED.
#ifndef WS281x_SET_LED_TRIGGER_REFRESH
  #define WS281x_SET_LED_TRIGGER_REFRESH        DEF_DISABLED
#endif



#ifndef WS281x_USE_SK6812
  #define WS281x_USE_SK6812                     DEF_DISABLED
#endif

#ifndef WS281x_USE_WS2811
  #define WS281x_USE_WS2811                     DEF_DISABLED
#endif

#ifndef WS281x_USE_WS2812
  #define WS281x_USE_WS2812                     DEF_DISABLED
#endif

#ifndef WS281x_USE_WS2812B
  #define WS281x_USE_WS2812B                    DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum WS281x_Methods_e
{
  #if (WS281x_USE_SK6812 == DEF_ENABLED)
    MODE_SK6812,
  #endif
  #if (WS281x_USE_WS2811 == DEF_ENABLED)
    MODE_WS2811,
  #endif
  #if (WS281x_USE_WS2812 == DEF_ENABLED)
    MODE_WS2812,
  #endif
  #if (WS281x_USE_WS2812B == DEF_ENABLED)
    MODE_WS2812B,
  #endif

  NUMBER_OF_METHODS,
};

struct WS2812x_MethodData_t
{
    uint8_t     T0H;
    uint8_t     T1H;
    uint16_t    ResetTime;
};

struct WS281x_Color_t
{
    uint8_t     Green;
    uint8_t     Red;
    uint8_t     Blue;
};

struct WS281x_Config_t
{
    WS281x_Methods_e        Method;
    UART_Driver*            pUART_Driver;
    uint16_t                NumberOfLED;        // Led Chain info
    DMA_Info_t              DMA_Info;           // DMA info
};

//-------------------------------------------------------------------------------------------------
// class(s)
//-------------------------------------------------------------------------------------------------

class WS281x
{
    public:

                WS281x                  (const WS281x_Config_t* pConfig);

        void    Initialize              ();
        void    SetLed                  (uint32_t Offset, WS281x_Color_t Color);
        void    Start                   (void);
        void    Stop                    (void);

      #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
        void    Refresh                 (void);
      #endif

        void    DMA_Channel_IRQ_Handler (bool IsItTransferComplete);

    private:

        WS281x_Methods_e                    m_Method;
        UART_Driver*                        m_pUART_Driver;
        DMA_Driver                          m_DMA;

        uint32_t                            m_NumberOfLED;
        WS281x_Color_t*                     m_pLedChain;
        volatile uint32_t                   m_ResetCount;
      #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
        bool                                m_NeedRefresh;
      #endif
        volatile bool                       m_IsItinFirstHalfOfBuffer;
        uint8_t*                            m_pDMA_Buffer;

        static const WS2812x_MethodData_t   m_Methods[NUMBER_OF_METHODS];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_WS281x__
#include "device_var.h"
#undef  __CLASS_WS281x__

//-------------------------------------------------------------------------------------------------
#else // (USE_UART_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for UART must be enable and configure to use this device driver")

#endif // (USE_UART_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
