//-------------------------------------------------------------------------------------------------
//
//  File : lib_Grafx_GenSTM32.h
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
  #define DMA2D_R2M                         DMA2D_CR_MODE           // DMA2D register to memory transfer mode

  #define TRANSFERT_ONE_PIXEL               0x00010001
#endif

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
