//-------------------------------------------------------------------------------------------------
//
//  File : lib_stm32xxx_dma2d_Grafx_GenDriver.cpp
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
#include "./Grafx/inc/driver/STM32/lib_STM32_Generic.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const int32_t GrafxGenDriver::m_PixelFormatTable[PIXEL_FORMAT_COUNT] =
{
  #if (GRAFX_COLOR_ARGB8888 == DEF_ENABLED)
    DMA2D_CONVERSION_ARGB8888,
  #endif
  #if (GRAFX_COLOR_RGB888 == DEF_ENABLED)
    DMA2D_CONVERSION_RGB888,
  #endif
  #if (GRAFX_COLOR_RGB565 == DEF_ENABLED)
    DMA2D_CONVERSION_RGB565,
  #endif
  #if (GRAFX_COLOR_ARGB1555 == DEF_ENABLED)
    DMA2D_CONVERSION_ARGB1555,
  #endif
  #if (GRAFX_COLOR_ARGB4444 == DEF_ENABLED)
    DMA2D_CONVERSION_ARGB4444,
  #endif
  #if (GRAFX_COLOR_L8 == DEF_ENABLED)
    DMA2D_CONVERSION_L8,
  #endif
  #if (GRAFX_COLOR_AL44 == DEF_ENABLED)
    DMA2D_CONVERSION_AL44,
  #endif
  #if (GRAFX_COLOR_AL88 == DEF_ENABLED)
    DMA2D_CONVERSION_AL88,
  #endif
  #if (GRAFX_COLOR_L4 == DEF_ENABLED)
    DMA2D_CONVERSION_L4,
  #endif
  #if (GRAFX_COLOR_A8 == DEF_ENABLED)
    DMA2D_CONVERSION_A8,
  #endif
  #if (GRAFX_COLOR_A4 == DEF_ENABLED)
    DMA2D_CONVERSION_A4,
  #endif
    -1
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   const void*         pArg        N/U
//  Return:         None
//
//  Description:   Nothing to initialize on DMA2D
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::Initialize(const void* pArg)
{
    VAR_UNUSED(pArg);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearLayer
//
//  Parameter(s):   Layer_e 		Layer
//  Return:         None
//
//  Description:    Generic Clear layer function
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::ClearLayer(Layer_e Layer)
{
    DisplayLayer* pLayer      = &LayerTable[Layer];
    uint32_t      AreaConfig  = (uint32_t(pLayer->GetSize().X) << 16) | uint32_t(pLayer->GetSize().Y);

    if(AreaConfig != 0) // Do not try to erase layer with no size
    {
        uint32_t PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
        uint32_t Address     = pLayer->GetAddress();

        // Configure DMA2D for Register-to-Memory (constant color fill)
        DMA2D->CR      = DMA2D_R2M;
        DMA2D->OCOLR   = pLayer->GetColor();                        // Constant color
        DMA2D->OMAR    = Address;                                   // Destination address
        DMA2D->OOR     = 0;                                         // No line offset
        DMA2D->OPFCCR  = PixelFormat;                               // Pixel format
        DMA2D->NLR     = AreaConfig;                                // Width + Height

        SET_BIT(DMA2D->CR, DMA2D_CR_START);                         // Start operation
        while((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);              // Wait for transfer complete
        DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                          // Clear flag
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BlockCopy
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  Cartesian_t*    pDstPos
//                  PixelFormat_e   SrcPixelFormat
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from square memory region to another square memory
//                  region according to drawing layer
//
//
//  THSI function has a big FLAW, it assume both source and destination has a size of GRAFX_DRIVER_SIZE_X
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    uint32_t      PixelFormatSrc;
    uint32_t      PixelFormatDst;
    uint32_t      Address;
    DisplayLayer* pLayer;
    uint8_t       PixelSize;
    uint32_t      SizeX = uint32_t(pBox->Size.Width);

    pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormatSrc = m_PixelFormatTable[SrcPixelFormat];
    PixelFormatDst = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize      = pLayer->GetPixelSize();
    Address        = pLayer->GetAddress() + (((pDstPos->Y * GRAFX_DRIVER_SIZE_X) + pDstPos->X) * (uint32_t)PixelSize);

    DMA2D->CR      = ((BlendMode == CLEAR_BLEND) ? DMA2D_M2M : DMA2D_M2M_BLEND) | DMA2D_CR_TCIE;                        // Memory to memory and TCIE blending BG + Source

    //Source
    DMA2D->FGMAR   = (uint32_t)(pSrc) + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);    // Source address
    DMA2D->FGOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - SizeX;                                                             // Source line offset none as we are linear
    DMA2D->FGPFCCR = PixelFormatSrc;                                                                                    // Defines the size of pixel

    // Source
    DMA2D->BGMAR   = Address;                                                                                           // Source address
    DMA2D->BGOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - SizeX;                                                             // Source line offset
    DMA2D->BGPFCCR = PixelFormatDst;                                                                                    // Defines the size of pixel

    //Destination
    DMA2D->OMAR    = Address;                                                                                           // Destination address
    DMA2D->OOR     = (uint32_t)GRAFX_DRIVER_SIZE_X - SizeX;                                                             // Destination line offset none as we are linear
    DMA2D->OPFCCR  = PixelFormatDst;                                                                                    // Defines the size of pixel

    DMA2D->NLR     = (SizeX  << 16) | uint32_t(pBox->Size.Height);                                                      // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                                                 // Start operation
    while((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                                                                      // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                                                                                  // Clear flag
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyLinear
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  PixelFormat_e   SrcPixelFormat
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from linear memory region to square memory area
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::CopyLinear(void* pSrc, Box_t* pBox, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    uint32_t      PixelFormatSrc;
    uint32_t      PixelFormatDst;
    uint32_t      Address;
    DisplayLayer* pLayer;
    uint8_t       PixelSize;
    uint32_t      SizeX;
    uint32_t      Width;

    pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormatSrc = m_PixelFormatTable[SrcPixelFormat];
    PixelFormatDst = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize      = pLayer->GetPixelSize();
    SizeX          = uint32_t(pLayer->GetSize().X);
    Width          = uint32_t(pBox->Size.Width);
    Address        = pLayer->GetAddress() + (((pBox->Pos.Y * SizeX) + pBox->Pos.X) * (uint32_t)PixelSize);

    if(PixelFormatSrc == PixelFormatDst)
    {
        DMA2D->CR = ((BlendMode == CLEAR_BLEND) ? DMA2D_M2M : DMA2D_M2M_BLEND);     // Memory to memory or M2M with blending BG + Source
    }
    else
    {
        DMA2D->CR = DMA2D_M2M_PFC;                                                  // Memory to memory with pixel conversion
    }

    // Source 1
    DMA2D->FGMAR       = (uint32_t)pSrc;                                            // Source address
    DMA2D->FGOR        = 0;                                                         // Source line offset none as we are linear
    DMA2D->FGPFCCR     = PixelFormatSrc;                                            // Defines the size of pixel

    // Source 2 (Source2 vs Destination = Read modify write)
    DMA2D->BGMAR       = Address;                                                   // Source address
    DMA2D->BGOR        = SizeX - Width;                                             // Source line offset
    DMA2D->BGPFCCR     = PixelFormatDst;                                            // Defines the size of pixel

    // Destination
    DMA2D->OMAR        = Address;                                                   // Destination address
    DMA2D->OOR         = SizeX - Width;                                             // Destination line offset
    DMA2D->OPFCCR      = PixelFormatDst;                                            // Defines the size of pixel

    DMA2D->NLR         = (Width << 16) | (uint32_t(pBox->Size.Height));             // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                             // Start operation
    while((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                                  // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                                              // Clear flag
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawRectangle
//
//  Parameter(s):   Box_t*   pBox
//  Return:         None
//
//  Description:    Fill a region in a specific color
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawRectangle(Box_t* pBox)
{
    uint32_t      PixelFormat;
    uint32_t      Address;
    uint32_t      Color;
    DisplayLayer* pLayer;
    uint8_t       PixelSize;
    uint32_t      SizeX;
    uint32_t      Width;

    pLayer        = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat   = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize     = pLayer->GetPixelSize();
    SizeX         = uint32_t(pLayer->GetSize().X);
    Address       = pLayer->GetAddress() + (((pBox->Pos.Y * SizeX) + pBox->Pos.X) * (uint32_t)PixelSize);
    Color         = pLayer->GetColor();
    Width         = uint32_t(pBox->Size.Width);

    DMA2D->CR     = DMA2D_R2M;                                      // Register to memory
    DMA2D->OCOLR  = Color;                                          // Color to be used
    DMA2D->OMAR   = Address;                                        // Destination address
    DMA2D->OOR    = SizeX - Width;                                  // Destination line offset
    DMA2D->OPFCCR = PixelFormat;                                    // Defines the pixel format
    DMA2D->NLR    = (Width << 16) | pBox->Size.Height;              // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                             // Start operation
    while((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                  // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                              // Clear flag
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawLine
//
//  Parameter(s):   uint16_t    PosX       Specifies the X position, can be a value from 0 to 240.
//                  uint16_t    PosY       Specifies the Y position, can be a value from 0 to 320.
//                  uint16_t    Length     Line length.
//                  uint16_t    Thickness
//                  DrawMode_e  Direction  line direction.
//                                         This Parameter can be one of the following values
//                                                 DRAW_HORIZONTAL
//                                                 DRAW_VERTICAL
//  Return:         None
//
//  Description:    Displays a line of a specific Thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawLine(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction)
{
    uint32_t      PixelFormat;
    uint8_t       PixelSize;
    uint32_t      Address;
    uint32_t      Color;
    DisplayLayer* pLayer;
    //uint32_t      Offset;
    uint32_t      Width;
    uint32_t      Height;
    uint32_t      SizeX;

    pLayer      = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    SizeX       = uint32_t(pLayer->GetSize().X);
    Address     = pLayer->GetAddress() + (((PosY * SizeX) + PosX) * (uint32_t)PixelSize);
    Color       = pLayer->GetColor();

    if(Direction == DRAW_HORIZONTAL)
    {
        Width  = uint32_t(Length);
        Height = uint32_t(Thickness);
    }
    else // DRAW_VERTICAL
    {
        Width  = uint32_t(Thickness);
        Height = uint32_t(Length);
    }

    DMA2D->CR      = DMA2D_R2M;                                                     // Register to memory
    DMA2D->OCOLR   = Color;                                                         // Color to be used
    DMA2D->OMAR    = Address;                                                       // Destination address
    DMA2D->OOR     = SizeX;//  - Offset;                                                // Destination line offset
    DMA2D->OPFCCR  = PixelFormat;                                                   // Defines the number of pixels to be transfered
    DMA2D->NLR     = (Width << 16) | Height;                                        // Size configuration of area

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                             // Start operation
    while((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                                  // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                                              // Clear flag
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintFont
//
//  Parameter(s):   FontDescriptor_t*   pDescriptor
//                  sCartesian*         pPos
//  Return:         none
//
//  Description:    This function will print a font to drawing layer with the drawing color
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::PrintFont(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)
{
    uint32_t      PixelFormat;
    uint8_t       PixelSize;
    uint32_t      Address;
    DisplayLayer* pLayer;
    uint32_t      AreaConfig;
    uint32_t      Offset;
    uint32_t      SizeX;

    pLayer      = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    SizeX       = uint32_t(pLayer->GetSize().X);
    Address     = pLayer->GetAddress() + (((pPos->Y * SizeX) + pPos->X) * (uint32_t)PixelSize);
    AreaConfig  = (uint32_t(pDescriptor->WidthPixel) << 16) | uint32_t(pDescriptor->HeightPixel);
    Offset      = pDescriptor->WidthPixel;

    DMA2D->CR      = DMA2D_M2M_BLEND;                                   // Memory to memory

    // Font layer in Alpha blending linear (A8)
    DMA2D->FGMAR   = (uint32_t)pDescriptor->pAddress;                   // Source address 1
    DMA2D->FGOR    = 0;                                                 // Font source line offset - none as we are linear
    DMA2D->FGCOLR  = pLayer->GetTextColor();
    DMA2D->FGPFCCR = DMA2D_CONVERSION_A8;                               // Defines the number of pixels to be transfered

    DMA2D->BGMAR   = Address;                                           // Source address 2
    DMA2D->BGOR    = SizeX - Offset;                                    // Font source line offset - none as we are linear
    DMA2D->BGPFCCR = PixelFormat;                                       // Defines the number of pixels to be transfered

    // Output Layer
    DMA2D->OMAR    = Address;
    DMA2D->OOR     = SizeX - Offset;                                    // Destination line offset
    DMA2D->OPFCCR  = PixelFormat;

    // Area
    DMA2D->NLR     = AreaConfig;                                        // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                 // Start operation
    while((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                      // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                                  // Clear flag
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LayerConfig
//
//  Parameter(s):   DisplayLayer* pLayer
//  Return:         None
//
//  Description:    Configuration for layer
//
//-------------------------------------------------------------------------------------------------
/*
void GrafxGenDriver::LayerConfig(Layer_e Layer)
{
    DisplayLayer* pLayer = &LayerTable[Layer];
    LayerConfig(pLayer);
}
*/

// what is the use of this fonction ????
void GrafxGenDriver::LayerConfig(DisplayLayer* pLayer)
{
    //uint32_t    PixelFormat;
    //uint32_t    PixelSize;
    //LayerType_e ActiveLayer;

    //ActiveLayer = pLayer->GetActive();
    //PixelFormat = pLayer->GetPixelFormat();
    //PixelSize   = GFX_PixelSize[PixelFormat];

    VAR_UNUSED(pLayer);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyLinear
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  PixelFormat_e   PixelFormat)
//                  BlendMode_e     BlendMode
//  Return:         None
//
//   Description:   Copy a rectangle region from linear memory region to square memory area
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
/*
void GrafxGenDriver::CopyLinear(void* pSrc, Box_t* pBox, PixelFormat_e PixelFormat, BlendMode_e BlendMode)
{
//    Not supported for now

    VAR_UNUSED(pSrc);
    VAR_UNUSED(pBox);
    VAR_UNUSED(PixelFormat);
    VAR_UNUSED(BlendMode);
}

void GrafxGenDriver::CopyLinear(ImageID_e Image, Cartesian_t Pos, BlendMode_e BlendMode)
{
//    Not supported for now

    VAR_UNUSED(Image);
    VAR_UNUSED(Pos);
    VAR_UNUSED(BlendMode);
}
*/


void GrafxGenDriver::DrawPixel(uint16_t PosX, uint16_t PosY)
{
    VAR_UNUSED(PosX);
    VAR_UNUSED(PosY);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyLinear
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  PixelFormat_e   PixelFormat)
//                  BlendMode_e     BlendMode
//  Return:         None
//
//   Description:   Copy a rectangle region from linear memory region to square memory area
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
/*void GrafxGenDriver::CopyLinear(void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, PixelFormat_e PixelFormat, BlendMode_e BlendMode)
{
//    Not supported for now

    VAR_UNUSED(pSrc);
    VAR_UNUSED(PosX);
    VAR_UNUSED(PosY);
    VAR_UNUSED(pSrc);
    VAR_UNUSED(Width);
    VAR_UNUSED(Height);
    VAR_UNUSED(PixelFormat);
    VAR_UNUSED(BlendMode);
}
*/
//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_GRAFX == DEF_ENABLED)
