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
#include "./Grafx/inc/driver/STM32/lib_STM32_generic.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

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
    uint32_t      PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    uint32_t      Address     = pLayer->GetAddress();
    uint32_t      AreaConfig  = (pLayer->GetSize().X << 16) | pLayer->GetSize().Y;

    if(AreaConfig != 0) // Do not try to erase layer with no size
    {
        // Configure DMA2D for Register-to-Memory (constant color fill)
        DMA2D->CR      = DMA2D_R2M | DMA2D_CR_TCIE;
        DMA2D->OCOLR   = pLayer->GetColor();                        // Constant color
        DMA2D->OMAR    = Address;                                   // Destination address
        DMA2D->OOR     = 0;                                         // No line offset
        DMA2D->OPFCCR  = PixelFormat;                               // Pixel format
        DMA2D->NLR     = AreaConfig;                                // Width + Height

        SET_BIT(DMA2D->CR, DMA2D_CR_START);                         // Start operation
        while (DMA2D->CR & DMA2D_CR_START);                         // Wait for completion
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
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    uint32_t           PixelFormatSrc;
    uint32_t           PixelFormatDst;
    uint32_t           Address;
    uint32_t           AreaConfig;
    DisplayLayer*            pLayer;
    uint8_t            PixelSize;

    pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormatSrc = m_PixelFormatTable[SrcPixelFormat];
    PixelFormatDst = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize      = pLayer->GetPixelSize();
    Address        = pLayer->GetAddress() + (((pDstPos->Y * GRAFX_DRIVER_SIZE_X) + pDstPos->X) * (uint32_t)PixelSize);
    AreaConfig     = (uint32_t(pBox->Size.Width)  << 16) | (uint32_t(pBox->Size.Height));

    DMA2D->CR      = ((BlendMode == CLEAR_BLEND) ? DMA2D_M2M : DMA2D_M2M_BLEND) | DMA2D_CR_TCIE;                        // Memory to memory and TCIE blending BG + Source

    //Source
    DMA2D->FGMAR   = (uint32_t)(pSrc) + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);    // Source address
    DMA2D->FGOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                                        // Source line offset none as we are linear
    DMA2D->FGPFCCR = PixelFormatSrc;                                                                                    // Defines the size of pixel

    // Source
    DMA2D->BGMAR   = Address;                                                                                           // Source address
    DMA2D->BGOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                                        // Source line offset
    DMA2D->BGPFCCR = PixelFormatDst;                                                                                    // Defines the size of pixel

    //Destination
    DMA2D->OMAR    = Address;                                                                                           // Destination address
    DMA2D->OOR     = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                                        // Destination line offset none as we are linear
    DMA2D->OPFCCR  = PixelFormatDst;                                                                                    // Defines the size of pixel

    DMA2D->NLR     = AreaConfig;                                                                                        // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                                                 // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                                                                  // Wait until transfer is done
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
    uint32_t PixelFormatSrc;
    uint32_t PixelFormatDst;
    uint32_t Address;
    uint32_t AreaConfig;
    DisplayLayer*  pLayer;
    uint8_t  PixelSize;

    pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormatSrc = m_PixelFormatTable[SrcPixelFormat];
    PixelFormatDst = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize      = pLayer->GetPixelSize();
    Address        = pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
    AreaConfig     = (uint32_t(pBox->Size.Width)  << 16) | (uint32_t(pBox->Size.Height));

    if(PixelFormatSrc == PixelFormatDst)
    {
        DMA2D->CR = ((BlendMode == CLEAR_BLEND) ? DMA2D_M2M : DMA2D_M2M_BLEND) | DMA2D_CR_TCIE; // Memory to memory or M2M with blending BG + Source
    }
    else
    {
        DMA2D->CR = DMA2D_M2M_PFC | DMA2D_CR_TCIE;                                              // Memory to memory with pixel conversion
    }

    // Source 1
    DMA2D->FGMAR       = (uint32_t)pSrc;                                                        // Source address
    DMA2D->FGOR        = 0;                                                                     // Source line offset none as we are linear
    DMA2D->FGPFCCR     = PixelFormatSrc;                                                        // Defines the size of pixel

    // Source 2 (Source2 vs Destination = Read modify write)
    DMA2D->BGMAR       = Address;                                                               // Source address
    DMA2D->BGOR        = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;            // Source line offset
    DMA2D->BGPFCCR     = PixelFormatDst;                                                        // Defines the size of pixel

    // Destination
    DMA2D->OMAR        = Address;                                                               // Destination address
    DMA2D->OOR         = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;            // Destination line offset
    DMA2D->OPFCCR      = PixelFormatDst;                                                        // Defines the size of pixel

    DMA2D->NLR         = AreaConfig;                                                            // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                         // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                                          // Wait until transfer is done
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
    uint32_t        PixelFormat;
    uint32_t        Address;
    uint32_t        Color;
    uint32_t        AreaConfig;
    DisplayLayer*   pLayer;
    uint8_t         PixelSize;

    pLayer      = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    Address     = pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
    Color       = pLayer->GetColor();
    AreaConfig  = (uint32_t(pBox->Size.Width)  << 16) | (uint32_t(pBox->Size.Height));

  #if defined(GRAFX_USE_SOFT_RECTANGLE)

    // if rectangle need to use software version

    #ifdef GRAFX_USE_SOFT_FILL
        // TO DO write code if fill is done in software
    #else
        // TO DO write code if done in software
    #endif

    DMA2D->CR     = DMA2D_R2M | DMA2D_CR_TCIE;                                     // Register to memory and TCIE
    DMA2D->OCOLR  = Color;                                                         // Color to be used
    DMA2D->OMAR   = Address;                                                       // Destination address
    DMA2D->OOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;    // Destination line offset
    DMA2D->OPFCCR = PixelFormat;                                                   // Defines the number of pixels to be transfered
    DMA2D->NLR    = AreaConfig;                                                    // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                            // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                             // Wait until transfer is done
  #endif
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
    uint32_t        PixelFormat;
    uint8_t         PixelSize;
    uint32_t        Address;
    uint32_t        Color;
    DisplayLayer*   pLayer;
    uint32_t        AreaConfig;
    uint32_t        Offset;

    pLayer      = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    Address     = pLayer->GetAddress() + (((PosY * GRAFX_DRIVER_SIZE_X) + PosX) * (uint32_t)PixelSize);
    Color       = pLayer->GetColor();

    DMA2D->CR      = DMA2D_R2M | DMA2D_CR_TCIE;                                     // Register to memory and TCIE
    DMA2D->OCOLR   = Color;                                                         // Color to be used
    DMA2D->OMAR    = Address;                                                       // Destination address
    DMA2D->OOR     = (uint32_t)GRAFX_DRIVER_SIZE_X - Offset;                        // Destination line offset
    DMA2D->OPFCCR  = PixelFormat;                                                   // Defines the number of pixels to be transfered
    DMA2D->NLR     = AreaConfig;                                                    // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                             // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                              // Wait until transfer is done
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
    uint32_t PixelFormat;
    uint8_t  PixelSize;
    uint32_t Address;
    DisplayLayer*  pLayer;
    uint32_t AreaConfig;
    uint32_t Offset;

    pLayer      = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    Address     = pLayer->GetAddress() + (((pPos->Y * GRAFX_DRIVER_SIZE_X) + pPos->X) * (uint32_t)PixelSize);
    AreaConfig  = (uint32_t(pDescriptor->WidthPixel) << 16) | uint32_t(pDescriptor->HeightPixel);
    Offset      = pDescriptor->WidthPixel;

    DMA2D->CR      = DMA2D_M2M_BLEND | DMA2D_CR_TCIE;                   // Memory to memory and TCIE

    // Font layer in Alpha blending linear (A8)
    DMA2D->FGMAR   = (uint32_t)pDescriptor->pAddress;                   // Source address 1
    DMA2D->FGOR    = 0;                                                 // Font source line offset - none as we are linear
    DMA2D->FGCOLR  = pLayer->GetTextColor();
    DMA2D->FGPFCCR = PIXEL_FORMAT_A8;                                   // Defines the number of pixels to be transfered

    DMA2D->BGMAR   = Address;                                           // Source address 2
    DMA2D->BGOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - Offset;            // Font source line offset - none as we are linear
    DMA2D->BGPFCCR = PixelFormat;                                       // Defines the number of pixels to be transfered

    // Output Layer
    DMA2D->OMAR    = Address;
    DMA2D->OOR     = (uint32_t)GRAFX_DRIVER_SIZE_X - Offset;            // Destination line offset
    DMA2D->OPFCCR  = PixelFormat;

    // Area
    DMA2D->NLR     = AreaConfig;                                        // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                 // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                  // Wait until transfer is done
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
void GrafxGenDriver::LayerConfig(DisplayLayer* pLayer)
{
    uint32_t            PixelFormat;
    uint32_t            PixelSize;
    LayerType_e         ActiveLayer;

    ActiveLayer = pLayer->GetActive();
    PixelFormat = pLayer->GetPixelFormat();
    PixelSize   = GFX_PixelSize[PixelFormat];
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
*/

void GrafxGenDriver::CopyLinear(ImageID_e Image, Cartesian_t Pos, BlendMode_e BlendMode)
{
//    Not supported for now

    VAR_UNUSED(Image);
    VAR_UNUSED(Pos);
    VAR_UNUSED(BlendMode);
}


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
