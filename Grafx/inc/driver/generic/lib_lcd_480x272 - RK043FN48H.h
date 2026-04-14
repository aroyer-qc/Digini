//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32xxx_lcd_480x272 - RK043FN48H.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define GRAFX_NUMBER_OF_ACTIVE_LAYER                2

// Display size
#define GRAFX_DRIVER_SIZE_X                         480
#define GRAFX_DRIVER_SIZE_Y                         272

#define GRAFX_HSYNC                                 41              // Horizontal synchronization
#define GRAFX_HBP                                   13              // Horizontal back porch
#define GRAFX_HFP                                   32              // Horizontal front porch
#define GRAFX_VSYNC                                 10              // Vertical synchronization
#define GRAFX_VBP                                   2               // Vertical back porch
#define GRAFX_VFP                                   2               // Vertical front porch

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class GrafxDriver : public GrafxGenDriver
{
    public:

        void        Initialize      (void* pArg)  override;
        void        DisplayOn       (void)        override;
        void        DisplayOff      (void)        override;
};

//-------------------------------------------------------------------------------------------------

#ifdef LIB_RK043FN48H_GLOBAL
GrafxDriver         Grafx;
GrafxDriver*        myGrafx = &Grafx;
#else
extern GrafxDriver* myGrafx;
#endif

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
