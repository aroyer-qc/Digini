//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_pwm_WS281x.h
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

//#include "./Digini/lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_TIM_DRIVER == DEF_ENABLED) && (USE_PWM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Note(s)
//-------------------------------------------------------------------------------------------------
//
// This library is specific to F1, it use too complex feature of timer to be generic or use timer
// Library. Although it might be easy to convert to another timer from other CPU family.
//
// It need a timer with DMA and compare register.
//
//  Note(s):        The Frequency for the smart LED are 800 kHz.
//
//                  Logical 0        ___________                       __
//                             ...__|           \_____________________|  ...
//                                  <- 350 nS -><------ 900 nS ------>
//
//                  Logical 1        ___________________               __
//                             ...__|                   \_____________|  ...
//                                  <----- 700 nS -----><-- 550 nS -->
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
//                          WS2812B: > 50  uSec -> use 1000 Counts
//                          WS2811:  > 280 uSec -> use 5600 Counts
//
//                  _______________________________________________________________________________
//
//                  The Best granularity here will be 50 nSec, for a frequency of 20 MHz.
//
//                  350 nSec    = 7  Counts
//                  900 nSec    = 18 Counts
//                  700 nSec    = 14 Counts
//                  550 nSec    = 11 Counts
//                  Auto Reload = 25 Counts         Range for 1.25 uSec
//                  _______________________________________________________________________________
//
//                  There two more entry in the m_pLedChain array and there value are set to zero.
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
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// This enable the continuous scan of the LED stream. No refresh needed.
#ifndef WS281x_CONTINUOUS_SCAN
  #define WS281x_CONTINUOUS_SCAN            DEF_DISABLED
#endif

// If a LED change is done, it will trigger a refresh automatically is it is DEF_ENABLED.
// If has no effect if WS281x_CONTINUOUS_SCAN is DEF_ENABLED.
#ifndef WS281x_SET_LED_TRIGGER_REFRESH
  #define WS281x_SET_LED_TRIGGER_REFRESH    DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum WS281x_ResetType_e
{
    WS2812B_RESET       = 32000,  // TODO is it right??  just check timing when it will work
    WS2811_RESET        = 5600,
};

struct WS281x_Color_t
{
    uint8_t     Green;
    uint8_t     Red;
    uint8_t     Blue;
};

struct WS281x_Config_t
{
    PWM_Driver*             pPWM_Driver;
    uint16_t                NumberOfLED;        // Led Chain info
    WS281x_ResetType_e      ResetType;
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

        PWM_Driver*                 m_pPWM_Driver;
        DMA_Driver                  m_DMA;

        uint32_t                    m_NumberOfLED;
        volatile uint16_t           m_LedPointer;
        WS281x_Color_t*             m_pLedChain;
        uint8_t*                    m_pDMA_Buffer;
        uint8_t*                    m_pDMA_HalfBuffer;      // to reduce DMA interrupt time
      #if (WS281x_CONTINUOUS_SCAN == DEF_DISABLED)
        bool                        m_NeedRefresh;
      #endif
        volatile uint32_t           m_SetCountReset;
        volatile uint32_t           m_ResetCount;

};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_WS281x__
#include "device_var.h"
#undef  __CLASS_WS281x__

//-------------------------------------------------------------------------------------------------
#else // (USE_TIM_DRIVER == DEF_ENABLED) && (USE_PWM_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for TIM and PWM must be enable and configure to use this device driver")

#endif // (USE_TIM_DRIVER == DEF_ENABLED) && (USE_PWM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
