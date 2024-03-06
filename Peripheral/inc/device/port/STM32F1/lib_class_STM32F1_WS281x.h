//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_WS281x.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_TIM_DRIVER == DEF_ENABLED)

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
//                  There two more entry in the m_pLedChain array. Last value is set to zero.
//                  when those LED entry are reach, DMA continue as nothing happened, but value is
//                  set to zero, so this emulate a reset, until StartTransfer is call again.
//
//
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum WS281x_ResetType_e
{
    WS2812B_RESET       = 1000,  // TODO is it right??
    WS2811_RESET        = 5600,
};

struct WS281x_Color_t
{
    uint8_t     Red;
    uint8_t     Green;
    uint8_t     Blue;
};

struct WS281x_Config_t
{
    // Led Chain info
    uint16_t                NumberOfLED;
    WS281x_ResetType_e      ResetType;
    IO_ID_e                 NeoDataPin;
    // Timer info
    TIM_ID_e                TimID;
    TIM_Compare_e           CompareChannel;
    // DMA info
    DMA_Channel_TypeDef*    DMA_Channel;
    uint32_t                DMA_Flag;
    IRQn_Type               IRQn;
};

//-------------------------------------------------------------------------------------------------
// class(s)
//-------------------------------------------------------------------------------------------------

class WS281x
{
    public:

        void    Initialize          (const WS281x_Config_t* pConfig);
        void    SetLed              (uint16_t Offset, WS281x_Color_t Color);
        void    Process             (void);
        void    Start               (void);
        void    Stop                (void);
        void    FillUp_24_Bits      (uint8_t* pBuffer);

        void    DMA_Channel_IRQ_Handler(TIM_ID_e TIM_ID);

    private:

 //       PWM_Driver                  m_PWM_Driver;

        TIM_Driver*                 m_pTimer;
        uint16_t                    m_LedChainSize;
        uint16_t                    m_NumberOfLED;
        volatile uint16_t           m_LedPointer;
        WS281x_Color_t*             m_pLedChain;
        uint8_t*                    m_pDMA_Buffer;
        bool                        m_NeedRefresh;
        uint8_t                     m_SetCountReset;
        uint8_t                     m_ResetCount;
        TIM_Compare_e               m_Channel;
        DMA_Channel_TypeDef*        m_pDMA;
        uint32_t                    m_DMA_Flag;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_WS281x__
#include "device_var.h"
#undef  __CLASS_WS281x__

//-------------------------------------------------------------------------------------------------

#else // (USE_TIM_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for TIM must be enable and configure to use this device driver")

#endif // (USE_TIM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
