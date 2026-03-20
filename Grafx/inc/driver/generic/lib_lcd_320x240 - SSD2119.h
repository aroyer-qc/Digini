//-------------------------------------------------------------------------------------------------
//
//  File : lib_lcd_320x240_SSD2119.h
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
//      - The UMSH-8252MD-T does expose the RGB interface, so LTDC is valid.
//      - Configure the SSD2119 into RGB (moving picture) mode via SPI.
//      - LTDC should be configured for RGB565 or RGB666 depending on how many DB lines are wire.
//
//      - Pixel Clock is 6.5MHz
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
#define GRAFX_DRIVER_SIZE_X                         320
#define GRAFX_DRIVER_SIZE_Y                         240

#define GRAFX_HSYNC                                 2               // Horizontal synchronization
#define GRAFX_HBP                                   2               // Horizontal back porch
#define GRAFX_HFP                                   2               // Horizontal front porch
#define GRAFX_VSYNC                                 1               // Vertical synchronization
#define GRAFX_VBP                                   2               // Vertical back porch
#define GRAFX_VFP                                   2               // Vertical front porch

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

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class GrafxDriver : public GrafxGenDriver
{
    public:

        void        Initialize      (void* pArg)                                                                                            override;
        void        DisplayOn       (void)                                                                                                  override;
        void        DisplayOff      (void)                                                                                                  override;
};

//-------------------------------------------------------------------------------------------------

#ifdef LIB_SSD2119_GLOBAL
GrafxDriver         Grafx;
GrafxDriver*        myGrafx = &Grafx;
#else
extern GrafxDriver* myGrafx;
#endif

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
