//-------------------------------------------------------------------------------------------------
//
//  File : lib_Grafx_GenDriver.h
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
// Class
//-------------------------------------------------------------------------------------------------

class GrafxGenDriver
{
        public:

            virtual     void    Initialize            (void* pArg);
	        virtual     void    DisplayOn             (void);
            virtual     void    DisplayOff            (void);
            virtual     void    LayerConfig           (CLayer* pLayer);
            virtual	    void    CopyLinear            (void* pSrc, Box_t* pBox, PixelFormat_e PixelFormat, BlendMode_e BlendMode);
            virtual     void    CopyLinear            (void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, PixelFormat_e PixelFormat, BlendMode_e BlendMode);
            virtual	    void    BlockCopy             (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
            virtual	    void    BlockCopy             (void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
            virtual	    void    DrawRectangle         (Box_t* pBox);
            virtual	    void    DrawRectangle         (uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height);
            virtual	    void    PrintFont             (FontDescriptor_t* pDescriptor, Cartesian_t* pPos);
            virtual	    void    DrawBox               (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness);
            virtual	    void    DrawPixel             (uint16_t PosX, uint16_t PosY);
            virtual	    void    DrawHLine             (uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t ThickNess);
            virtual	    void    DrawVLine             (uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t ThickNess);
            virtual	    void    DrawLine              (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction);
            virtual	    void    DrawCircle            (uint16_t PosX, uint16_t PosY, uint16_t Radius, PolygonMode_e Mode);
            virtual	    void    DrawCircle            (Circle_t* pCircle, PolygonMode_e PolygonMode);
            virtual	    void    Copy                  (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat_e, BlendMode_e BlendMode);
            virtual	    void    CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, Box_t* pBox);
            virtual	    void    CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height);
            virtual	    void    CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, uint16_t SrcX, uint16_t SrcY, uint16_t DstX, uint16_t DstY, uint16_t Width, uint16_t Height);
            virtual	    void    CopyLinear            (Skin_e Image, Cartesian_t Position, BlendMode_e BlendMode);
   #if (GRAFX_DRIVER_USE_V_SYNC == DEF_ENABLED)
            virtual	    void    WaitFor_V_Sync        (void);
   #endif
};

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
