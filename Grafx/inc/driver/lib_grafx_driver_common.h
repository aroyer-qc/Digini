//-------------------------------------------------------------------------------------------------
//
//  File : lib_grafx_driver_common.h
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

//-------------------------------------------------------------------------------------------------
// This is inserted into the class GrafxDriver
//-------------------------------------------------------------------------------------------------

	void                Initialize            (void* pArg);
	void                DisplayOn             (void);
	void                DisplayOff            (void);
	void                LayerConfig           (CLayer* pLayer);
	void                CopyLinear            (void* pSrc, Box_t* pBox, PixelFormat_e PixelFormat, BlendMode_e BlendMode);
    void                CopyLinear            (void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, PixelFormat_e PixelFormat, BlendMode_e BlendMode);
	void                BlockCopy             (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
	void                BlockCopy             (void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
	void                DrawRectangle         (Box_t* pBox);
	void                DrawRectangle         (uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height);
	void                PrintFont             (FontDescriptor_t* pDescriptor, Cartesian_t* pPos);
	void                DrawBox               (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness);
	void                DrawPixel             (uint16_t PosX, uint16_t PosY);
	void                DrawHLine             (uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t ThickNess);
	void                DrawVLine             (uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t ThickNess);
	void                DrawLine              (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction);
	void                DrawCircle            (uint16_t PosX, uint16_t PosY, uint16_t Radius, PolygonMode_e Mode);
    void                DrawCircle            (Circle_t* pCircle, PolygonMode_e PolygonMode);
	void                Copy                  (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat_e, BlendMode_e BlendMode);
	void                CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, Box_t* pBox);
	void                CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height);
	void                CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, uint16_t SrcX, uint16_t SrcY, uint16_t DstX, uint16_t DstY, uint16_t Width, uint16_t Height);
    void                CopyLinear            (Skin_e Image, Cartesian_t Position, BlendMode_e BlendMode);
   #if (GRAFX_DRIVER_USE_V_SYNC == DEF_ENABLED)
	void                WaitFor_V_Sync        (void);
   #endif
