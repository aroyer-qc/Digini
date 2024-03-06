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
// Note(s)
//-------------------------------------------------------------------------------------------------
//
// This library is specific to F1, it use too complex feature of timer to be generic or use timer
// Library. Although it might be easy to convert to another timer from other CPU family.
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
//                  Auto Reload = 25 Counts
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

struct WS281x_ResetType_e
{
    WS2812B_RESET,
    WS2811_RESET,
};

struct WS281x_Color_t
{
    uint8_t     Red;
    uint8_t     Green;
    uint8_t     Blue;
};

//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------

class WS281x
{
    public:

        void    Initialize          (TIM_ID_e TimID, uint16_t NumberOfLED, WS281x_ResetType_e ResetType, IO_ID_e NeoDataPin);
        void    SetLed              (uint16_t Offset, WS281x_Color_t Color);
        void    Process             (void);   
        void    FillUp_24_Bits      (uint8_t* pBuffer);

    private:

        DMA_Driver                  m_DMA_Driver;
        PWM_Driver                  m_PWM_Driver;
        
        uint16_t                    m_LedChainSize;
        uint16_t                    m_ResetTime;
        volatile uint16_t           m_LedPointer;
        WS281x_Color_t*             m_pLedChain;
        uint8_t*                    m_pDMA_Buffer;
        bool                        m_NeedRefresh;
        uint8_t                     m_ResetCount;
};

//-------------------------------------------------------------------------------------------------
