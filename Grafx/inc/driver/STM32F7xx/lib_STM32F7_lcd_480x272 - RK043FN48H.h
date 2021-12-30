//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F7_lcd_480x272 - RK043FN48H.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

#ifdef DIGINI_USE_GRAFX

//-------------------------------------------------------------------------------------------------
// define(s)
//-------------------------------------------------------------------------------------------------

#define GRAFX_NUMBER_OF_ACTIVE_LAYER                2

#define GRAFX_USE_SOFT_COPY_LINEAR
#define GRAFX_USE_SOFT_COPY_LAYER_TO_LAYER
//#define GRAFX_USE_SOFT_PIXEL                      // We use this driver DMA for this function
//#define GRAFX_USE_SOFT_BOX                        // We use this driver function
//#define GRAFX_USE_SOFT_VLINE
//#define GRAFX_USE_SOFT_HLINE
#define GRAFX_USE_SOFT_DLINE
#define GRAFX_USE_SOFT_CIRCLE
//#define GRAFX_USE_SOFT_RECTANGLE

//#define GRAFX_USE_SOFT_PRINT_FONT                 // we use this driver to print FONT using DMA2D
//#define GRAFX_USE_SOFT_ALPHA                      // We use this uP + LCD controller has alpha acceleration
//#define GRAFX_USE_SOFT_COPY                       // We use this driver DMA for this function
//#define GRAFX_USE_SOFT_FILL                       // We use this driver DMA for this function

// Display size
#define GRAFX_SIZE_X                                480
#define GRAFX_SIZE_Y                                272

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class GrafxDriver : public GRAFX_Interface
{
    public:

        // This include all required prototype for a driver.
        #include "lib_grafx_driver_common.h"

    private:

        void            Line                  (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t ThickNess, DrawMode_e Direction);
        void            IO_Initialize         (void);     // Initialize the IO on the processor
        void            LayerInitialize       (void);
        void            LCD_Initialize        (void);
};

//-------------------------------------------------------------------------------------------------

#ifdef LIB_RK043FN48H_GLOBAL
class GrafxDriver       Grafx;
class GRAFX_Interface*  myGrafx = &Grafx;
#else
extern GRAFX_Interface* myGrafx;
#endif

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
