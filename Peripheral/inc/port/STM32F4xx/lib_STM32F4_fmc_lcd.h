//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_fmc_lcd.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_FMC_LCD_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define FMC_BANK1_NE1_ADDDRESS                  (0x60000000)
#define FMC_BANK1_NE2_ADDDRESS                  (0x64000000)
#define FMC_BANK1_NE3_ADDDRESS                  (0x68000000)
#define FMC_BANK1_NE4_ADDDRESS                  (0x6C000000)

// FMC LCD Bank
#define FMC_LCD_BANK1                           (0x00000000)
#define FMC_LCD_BANK2                           (0x00000002)
#define FMC_LCD_BANK3                           (0x00000004)
#define FMC_LCD_BANK4                           (0x00000006)

// FMC LCD Memory Bus Width
#define FMC_LCD_MEM_BUS_WIDTH_8                 (0x00000000)
#define FMC_LCD_MEM_BUS_WIDTH_16                (0x00000010)
#define FMC_LCD_MEM_BUS_WIDTH_32                (0x00000020)

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    FMC_LCD_Initialize              (void);

//-------------------------------------------------------------------------------------------------

#endif // USE_FMC_LCD_DRIVER

