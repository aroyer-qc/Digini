//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32xxx_lcd_480x272 - RK043FN48H.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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

#define LIB_RK043FN48H_GLOBAL
#include "./lib_digini.h"
#undef  LIB_RK043FN48H_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   pArg
//  Return:         None
//
//  Description:    LCD configuration specific for the LCD and processor used by this driver
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Initialize(void* pArg)
{
    GrafxGenDriver::Initialize(pArg);
    DisplayOn();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayOn
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enables the Display
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DisplayOn(void)
{
    LTDC->GCR |= LTDC_GCR_LTDCEN;
    IO_SetPinHigh(IO_LCD_TFT_DISPLAY);
    IO_SetPinHigh(IO_LCD_TFT_BL_CTRL);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayOff
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disables the Display
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DisplayOff(void)
{
    LTDC->GCR &= ~(LTDC_GCR_LTDCEN);
    IO_SetPinLow(IO_LCD_TFT_DISPLAY);
    IO_SetPinLow(IO_LCD_TFT_BL_CTRL);
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
