//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_fmc_lcd.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_FMC_LCD_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define FMC_WRITE_OPERATION_ENABLE                  0x00001000              // Always enable

#define FMC_BTR1_ADDRESS_SETUP_TIME_POS             0
#define FMC_BTR1_ADDRESS_HOLD_TIME_POS              4
#define FMC_BTR1_DATA_SETUP_TIME_POS                8

#define FMC_BWTR1_ADDRESS_SETUP_TIME_POS            0
#define FMC_BWTR1_ADDRESS_HOLD_TIME_POS             4
#define FMC_BWTR1_DATA_SETUP_TIME_POS               8

//-------------------------------------------------------------------------------------------------
//
//   Function name: FMC_LCD_Initialize
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   Performs the LCD device initialization sequence on the FMC.
//
//-------------------------------------------------------------------------------------------------
void FMC_LCD_Initialize(void)
{
    // ---- FMC Reset ----
    RCC->AHB3RSTR |=  RCC_AHB3RSTR_FMCRST;
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_FMCRST;
    RCC->AHB3ENR  |=  RCC_AHB3ENR_FMCEN;                    // Enable Clock

    FMC_Bank1->BTCR[CFG_FMC_LCD_BANK] = FMC_WRITE_OPERATION_ENABLE | CFG_FMC_LCD_MEM_BUS_WIDTH;

    // FSM LCD device timing parameters
    FMC_Bank1->BTCR[CFG_FMC_LCD_BANK + 1] = (CFG_FMC_LCD_TIMING_ADDRESS_SETUP_TIME << FMC_BTR1_ADDRESS_SETUP_TIME_POS) |
                                            (CFG_FMC_LCD_TIMING_ADDRESS_HOLD_TIME  << FMC_BTR1_ADDRESS_HOLD_TIME_POS)  |
                                            (CFG_FMC_LCD_TIMING_DATA_SETUP_TIME    << FMC_BTR1_DATA_SETUP_TIME_POS);

    FMC_Bank1E->BWTR[CFG_FMC_LCD_BANK] =
          (CFG_FMC_LCD_WRITE_TIMING_ADDRESS_SETUP_TIME << FMC_BWTR1_ADDRESS_SETUP_TIME_POS) |
          (CFG_FMC_LCD_WRITE_TIMING_ADDRESS_HOLD_TIME << FMC_BWTR1_ADDRESS_HOLD_TIME_POS) |
          (CFG_FMC_LCD_WRITE_TIMING_DATA_SETUP_TIME << FMC_BWTR1_DATA_SETUP_TIME_POS);


    SET_BIT(FMC_Bank1->BTCR[CFG_FMC_LCD_BANK], FMC_BCR1_MBKEN);
}

//-------------------------------------------------------------------------------------------------

#endif // USE_SDRAM_DRIVER
