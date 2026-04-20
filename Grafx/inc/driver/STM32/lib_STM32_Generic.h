//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32_generic.h
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

#ifdef DIGINI_USE_GRAFX

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#ifdef DMA2D

  #define LTDC_BLENDING_FACTOR1_PAxCA       0x00000600              // Blending factor: Cte Alpha x Pixel Alpha
  #define LTDC_BLENDING_FACTOR2_PAxCA       0x00000007              // Blending factor: Cte Alpha x Pixel Alpha

  #define DMA2D_M2M                         0                       // DMA2D memory to memory transfer mode
  #define DMA2D_M2M_PFC                     ((uint32_t)0x00010000)  // DMA2D memory to memory with pixel format conversion transfer mode
  #define DMA2D_M2M_BLEND                   ((uint32_t)0x00020000)  // DMA2D memory to memory with blending transfer mode
  #define DMA2D_R2M                         ((uint32_t)0x00030000)  // DMA2D register to memory transfer mode

  #define DMA2D_ISR_ALL_FLAG                (DMA2D_ISR_TCIF | DMA2D_ISR_TEIF | DMA2D_ISR_CEIF)
  #define DMA2D_IFCR_ALL_FLAG               (DMA2D_IFCR_CTCIF | DMA2D_IFCR_CTEIF | DMA2D_IFCR_CCEIF)

  #define TRANSFERT_ONE_PIXEL               0x00010001

  #if (GRAFX_COLOR_RGB332 == DEF_ENABLED)
    #error GRAFX_COLOR_RGB332 is not supported
  #endif
  #if (GRAFX_COLOR_RGB444 == DEF_ENABLED)
    #error GRAFX_COLOR_RGB444 is not supported
  #endif

enum GFX_DMA2D_Conversion_e
{
    DMA2D_CONVERSION_ARGB8888           = 0x0,
    DMA2D_CONVERSION_RGB888             = 0x1,
    DMA2D_CONVERSION_RGB565             = 0x2,
    DMA2D_CONVERSION_ARGB1555           = 0x3,
    DMA2D_CONVERSION_ARGB4444           = 0x4,
    DMA2D_CONVERSION_L8                 = 0x5,
    DMA2D_CONVERSION_AL44               = 0x6,
    DMA2D_CONVERSION_AL88               = 0x7,
    DMA2D_CONVERSION_L4                 = 0x8,
    DMA2D_CONVERSION_A8                 = 0x9,
    DMA2D_CONVERSION_A4                 = 0xA,
};

#endif // DMA2D

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
