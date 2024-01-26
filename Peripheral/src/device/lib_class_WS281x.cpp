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

#include "lib_class_WS281x.h"

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
//  Parameter(s):   void*       pArg
//                  uint16_t    NumberOfLED             Number of LED in the chain
//                   uint16_t   ResetTime               Reset time in uSec, as per datasheet
//
//  Return:         void
//
//  Description:    Initialize the WS281x interface
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
//                          WS2812B: > 50  uSec
//                          WS2811:  > 280 uSec
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
//-------------------------------------------------------------------------------------------------
void WS281x::Initialize(void* pArg, uint16_t NumberOfLED, uint16_t ResetTime)
{
//    m_pPinStruct = (WS281x_PinStruct_t*)pArg;

// maybe CPU should provide frequency in multiple of 20Mhz example F103.. other CPU may have better frequency input
// USE DMA to change Compare value in register of the timer
// use 2 x 24 bits leds. So take advantage of the HT and TC of the DMA in a circular mode...
// so we can use those to have time to prepare next set of 24 bit colors for next led

//    IO_PinInit(m_pPinStruct->IO_DOut);
    
    m_ChainSize   = NumberOfLED + WS281x_DUMMY_LEDS;                                                // Chain LEDs Size.
    m_NumberOfLED = NumberOfLED;                                                                    // Number of real LEDs.
    
// Maybe we should not use allocation, since this is for duration of running apps.    
    m_pLedChain    = (WS281x_Color_t*)pMemory->AllocAndClear(m_ChainSize + WS281x_DUMMY_LEDS);      // Reserved x bytes from the alloc mem library.
    m_pDMA_Buffer  = pMemory->AllocAndClear(WS281x_DMA_FULL_BUFFER_SIZE);                           // Reserved 48 bytes DMA transfert to Compare register.

    m_LedPointer  = 0;              // Start at Led 0
    m_NeedRefresh = true;

    // Configure timer
        // configure prescaler
        // Set compare channel value at zero... not started
        // configure auto-reload                800 Khz
        // Enable output of the compare channel
        // Enable timer and compare channel
        
    // Configure DMA
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
                                                        // is done properly.
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
            }
            
            pBuffer++;
            pColorData++;
        }

        m_LedPointer++;
    }
    else
}

//-------------------------------------------------------------------------------------------------