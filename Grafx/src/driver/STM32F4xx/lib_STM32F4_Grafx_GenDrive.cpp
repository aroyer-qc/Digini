//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_Grafx_GenDriver.cpp
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_USE_GRAFX

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    LCD configuration specific for the LCD and processor used by this driver
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::Initialize(void* pArg)
{
  #ifdef DMA2D

    LTDC_InitTypeDef LTDC_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE);                                                   // Enable the DMA2D Clock for transfert

    // ---- Enable Pixel Clock ----
    // Configure PLLSAI prescalers for LCD
    // PLLSAI_VCO Input                                             = 1 Mhz
    // PLLSAI_VCO Output    = PLLSAI_VCO Input * PLLSAI_N  = 1*192  = 192 Mhz
    // PLLLCDCLK            = PLLSAI_VCO Output/PLLSAI_R   = 192/3  = 64 Mhz
    // LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR   = 64/8   = 8.0 Mhz
    RCC_PLLSAIConfig(192, 7, 3);
    RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);

    RCC_PLLSAICmd(ENABLE);                                                                                  // Enable PLLSAI Clock
    while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET){};                                                // Wait for PLLSAI activation

    // ---- LTDC Initialization ----
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE);                                                    // Enable the LTDC Clock

    DRV_Config();                                                                                           // Configure the LCD Control pins

    LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;                                                   // Initialize the horizontal synchronization polarity as active low
    LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;                                                   // Initialize the vertical synchronization polarity as active low
    LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;                                                   // Initialize the data enable polarity as active low
    LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;                                                  // Initialize the pixel clock polarity as input pixel clock

    // Timing configuration
    LTDC_InitStruct.LTDC_HorizontalSync     = GRAFX_HSYNC;                                                  // Configure horizontal synchronization width
    LTDC_InitStruct.LTDC_VerticalSync       = GRAFX_VSYNC;                                                  // Configure vertical synchronization height
    LTDC_InitStruct.LTDC_AccumulatedHBP     = GRAFX_HSYNC + GRAFX_HBP;                                      // Configure accumulated horizontal back porch
    LTDC_InitStruct.LTDC_AccumulatedVBP     = GRAFX_VSYNC + GRAFX_VBP;                                      // Configure accumulated vertical back porch
    LTDC_InitStruct.LTDC_AccumulatedActiveW = GRAFX_HSYNC + GRAFX_HBP + GRAFX_DRIVER_SIZE_X;                // Configure accumulated active width
    LTDC_InitStruct.LTDC_AccumulatedActiveH = GRAFX_VSYNC + GRAFX_VBP + GRAFX_DRIVER_SIZE_Y;                // Configure accumulated active height
    LTDC_InitStruct.LTDC_TotalWidth         = GRAFX_HSYNC + GRAFX_HBP + GRAFX_DRIVER_SIZE_X + GRAFX_HFP;    // Configure total width
    LTDC_InitStruct.LTDC_TotalHeigh         = GRAFX_VSYNC + GRAFX_VBP + GRAFX_DRIVER_SIZE_Y + GRAFX_VFP;    // Configure total height

    // Configure R,G,B component values for LCD background color
    LTDC_InitStruct.LTDC_BackgroundRedValue   = 0;
    LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
    LTDC_InitStruct.LTDC_BackgroundBlueValue  = 0;

    LTDC_Init(&LTDC_InitStruct);                                                                            // Initialize LTDC

    //LTDC_DitherCmd(ENABLE);

    bDriverInitialize = true;

    // Reload configuration
    LTDC_ReloadConfig(LTDC_IMReload);

    // Enable The LCD
    LTDC_Cmd(ENABLE);
  #else
    VAR_UNUSED(pArg);
    __asm("nop");
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Copy
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  Cartesian_t*    pDstPos
//                  PixelFormat_e   SrcPixelFormat_e
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from square memory region to another square memory
//                  region
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::Copy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat_e, BlendMode_e BlendMode)
{
	if(CLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint32_t           PixelFormatSrc;
        uint32_t           PixelFormatDst;
        uint32_t           Address;
        struct32_t         AreaConfig;
        CLayer*            pLayer;
        uint8_t            PixelSize;

        pLayer             = &LayerTable[CLayer::GetDrawing()];
        PixelFormatSrc     = PixelTable[SrcPixelFormat_e];
        PixelFormatDst     = PixelTable[pLayer->GetPixelFormat()];
        PixelSize          = pLayer->GetPixelSize();
        Address            = pLayer->GetAddress() + (((pDstPos->Y * GRAFX_DRIVER_SIZE_X) + pDstPos->X) * (uint32_t)PixelSize);

        AreaConfig.u_16.u1 = pBox->Size.Width;
        AreaConfig.u_16.u0 = pBox->Size.Height;

      #ifdef DMA2D

        DMA2D->CR          = ((BlendMode == CLEAR_BLEND) ? DMA2D_M2M : DMA2D_M2M_BLEND) | DMA2D_CR_TCIE;                        // Memory to memory and TCIE blending BG + Source

        //Source
        DMA2D->FGMAR       = (uint32_t)(pSrc) + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);    // Source address
        DMA2D->FGOR        = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                                        // Source line offset none as we are linear
        DMA2D->FGPFCCR     = PixelFormatSrc;                                                                                    // Defines the size of pixel

        // Source
        DMA2D->BGMAR       = Address;                                                                                           // Source address
        DMA2D->BGOR        = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                                        // Source line offset
        DMA2D->BGPFCCR     = PixelFormatDst;                                                                                    // Defines the size of pixel

        //Destination
        DMA2D->OMAR        = Address;                                                                                           // Destination address
        DMA2D->OOR         = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                                        // Destination line offset none as we are linear
        DMA2D->OPFCCR      = PixelFormatDst;                                                                                    // Defines the size of pixel

        DMA2D->NLR         = AreaConfig.u_32;                                                                                   // Size configuration of area to be transfered

        SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                                                     // Start operation
        while(DMA2D->CR & DMA2D_CR_START);                                                                                      // Wait until transfer is done

      #else

        uint8_t* pLocalSrc = (uint8_t*)pSrc;
        uint32_t* pDst = (uint32_t*)Address;
        pLocalSrc += (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
        DMA_Memcpy(pLocalSrc, pDst, size_t(AreaConfig.u_32));

      #endif
    }
    else
    {

    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyLinear
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  Cartesian_t*    pDstPos
//                  PixelFormat_e   SrcPixelFormat_e
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from linear memory region to a square memory
//                  region
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::CopyLinear(void* pSrc, Box_t* pBox, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    uint32_t           PixelFormatSrc;
    uint32_t           PixelFormatDst;
    uint32_t           Address;
    struct32_t         AreaConfig;
    CLayer*            pLayer;
    uint8_t            PixelSize;

    pLayer             = &LayerTable[CLayer::GetDrawing()];
    PixelFormatSrc     = PixelTable[SrcPixelFormat];
    PixelFormatDst     = PixelTable[pLayer->GetPixelFormat()];
    PixelSize          = pLayer->GetPixelSize();
    Address            = pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);

    AreaConfig.u_16.u1 = pBox->Size.Width;
    AreaConfig.u_16.u0 = pBox->Size.Height;

  #ifdef DMA2D

    DMA2D->CR          = (BlendMode == CLEAR_BLEND) ? DMA2D_M2M : DMA2D_M2M_BLEND;         // Memory to memory and TCIE. Blending BG + Source
    DMA2D->CR         |= DMA2D_CR_TCIE;

    // Source
    DMA2D->FGMAR       = (uint32_t)pSrc;                                                   // Source address
    DMA2D->FGOR        = 0;                                                                // Source line offset none as we are linear
    DMA2D->FGPFCCR     = PixelFormatSrc;                                                   // Defines the size of pixel

    // Source
    DMA2D->BGMAR       = Address;                                                          // Source address
    DMA2D->BGOR        = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;       // Source line offset
    DMA2D->BGPFCCR     = PixelFormatDst;                                                   // Defines the size of pixel

    // Destination
    DMA2D->OMAR        = Address;                                                          // Destination address
    DMA2D->OOR         = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;       // Destination line offset
    DMA2D->OPFCCR      = PixelFormatDst;                                                   // Defines the size of pixel

    DMA2D->NLR         = AreaConfig.u_32;                                                  // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                                                 // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                                     // Wait until transfer is done

  #else

    uint8_t* pLocalSrc = (uint8_t*)pSrc;
    uint32_t* pDst = (uint32_t*)Address;
    pLocalSrc += (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
    DMA_Memcpy(pLocalSrc, pDst, size_t(AreaConfig.u_32));

  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawRectangle
//
//   Parameter(s):  uint16_t    X
//                  uint16_t    Y
//                  uint16_t    Width
//                  uint16_t    Height
//    Return Value: none
//
//   Description:   Draw a rectangle
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawRectangle(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height)
{
    Box_t Box;

    Box.Pos.X       = PosX;
    Box.Pos.Y       = PosY;
    Box.Size.Width  = Width;
    Box.Size.Height = Height;

    #ifdef GRAFX_USE_SOFT_FILL
        // TO DO write code if fill is done in software
    #else
        DrawRectangle(&Box);
    #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawRectangle
//
//  Parameter(s):   sBox*   pBox
//  Return:         None
//
//  Description:    Fill a region in a specific color
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawRectangle(Box_t* pBox)
{
    uint32_t           PixelFormat;
    uint32_t           Address;
    uint32_t           Color;
    s32_t              AreaConfig;
    CLayer*            pLayer;
    uint8_t            PixelSize;

    pLayer             = &LayerTable[GetDrawingLayer()];
    PixelFormat        = PixelTable[pLayer->GetPixelFormat()];
    PixelSize          = pLayer->GetPixelSize();
    Address            = pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
    Color              = pLayer->GetColor();
    AreaConfig.u_16.u1 = pBox->Size.Width;
    AreaConfig.u_16.u0 = pBox->Size.Height;

  #if defined(GRAFX_USE_SOFT_RECTANGLE)

    // if rectangle need to use software version

    #ifdef GRAFX_USE_SOFT_FILL
        // TO DO write code if fill is done in software
    #else
        // TO DO write code if done in software
    #endif

  #elif defined(DMA2D)

    DMA2D->CR          = DMA2D_R2M | DMA2D_CR_TCIE;                                     // Register to memory and TCIE
    DMA2D->OCOLR       = Color;                                                         // Color to be used
    DMA2D->OMAR        = Address;                                                       // Destination address
    DMA2D->OOR         = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;    // Destination line offset
    DMA2D->OPFCCR      = PixelFormat;                                                   // Defines the number of pixels to be transfered
    DMA2D->NLR         = AreaConfig.u_32;                                               // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                 // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                                  // Wait until transfer is done

  #else

  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawBox
//
//  Parameter(s):   uint16_t    PosX
//                  uint16_t    PosY
//                  uint16_t    Length
//                  uint16_t    Height
//                  uint16_t    Thickness
//  Return:         None
//
//  Description:    Draw a box in a specific thickness
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness)
{
    uint16_t X2 = PosX + Length;
    uint16_t Y2 = PosY + Height;

    DrawVLine(PosX,           PosY, Y2, Thickness);
    DrawVLine(X2 - Thickness, PosY, Y2, Thickness);
    DrawHLine(PosY,           PosX, X2, Thickness);
    DrawHLine(Y2 - Thickness, PosX, X2, Thickness);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawPixel
//
//  Parameter(s):   uint16_t    PosX
//                  uint16_t    PosY
//  Return:         None
//
//  Description:    Put a pixel on selected layer
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawPixel(uint16_t PosX, uint16_t PosY)
{
    uint32_t       PixelFormat;
    uint32_t       Address;
    uint32_t       Color;
    CLayer*        pLayer;
    uint8_t        PixelSize;

    pLayer         = &LayerTable[GetDrawingLayer()];
    PixelFormat    = PixelTable[pLayer->GetPixelFormat()];
    PixelSize      = pLayer->GetPixelSize();
    Address        = pLayer->GetAddress() + (((PosY * GRAFX_DRIVER_SIZE_X) + PosX) * (uint32_t)PixelSize);
    Color          = pLayer->GetColor();

  #ifdef DMA2D

    DMA2D->CR      = DMA2D_R2M | DMA2D_CR_TCIE;                 // Register to memory and TCIE
    DMA2D->OCOLR   = Color;                                     // Color to be used
    DMA2D->OMAR    = Address;                                   // Destination address
    DMA2D->OPFCCR  = PixelFormat;                               // Defines the number of pixels to be transfered
    DMA2D->NLR     = TRANSFERT_ONE_PIXEL;                       // Size configuration of area to be transfered
    SET_BIT(DMA2D->CR, DMA2D_CR_START);                         // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                          // Wait until transfer is done

  #else

  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawHLine
//
//  Parameter(s):   uint16_t    Y
//                  uint16_t    X1
//                  uint16_t    X2
//                  uint16_t    ThickNess
//  Return:         None
//
//  Description:    Displays a horizontal line of a specific thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawHLine(uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)
{
    uint16_t Length;

    // X1 need to be the lowest (STM32 need this)
    if(PosX1 > PosX2)
    {
        Length = PosX1 - PosX2;
        PosX1  = PosX2;
    }
    else
    {
        Length = PosX2 - PosX1;
    }

    DrawLine(PosX1, PosY, Length, Thickness, DRAW_HORIZONTAL);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawVLine
//
//  Parameter(s):   uint16_t    wPosX
//                  uint16_t    wPosY1
//                  uint16_t    wPosY2
//                  uint16_t    wThickNess
//  Return:         None
//
//  Description:    Displays a vertical line of a specific thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawVLine(uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)
{
    uint16_t Length;

    // Y1 need to be the lowest (STM32 need this)
    if(PosY1 > PosY2)
    {
        Length = PosY1 - PosY2;
        PosY1  = PosY2;
    }
    else
    {
        Length = PosY2 - PosY1;
    }

    DrawLine(PosX, PosY1, Length, Thickness, DRAW_VERTICAL);
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
//  Description:    Displays a line of a specific thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawLine(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, eDrawMode Direction)
{
    uint32_t PixelFormat;
    uint8_t  PixelSize;
    uint32_t Address;
    uint32_t Color;
    CLayer*  pLayer;
    s32_t    AreaConfig;

    pLayer      = &LayerTable[GetDrawingLayer()];
    PixelFormat = PixelTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    Address     = pLayer->GetAddress() + (((PosY * GRAFX_DRIVER_SIZE_X) + PosX) * (uint32_t)PixelSize);
    Color       = pLayer->GetColor();

    if(Direction == DRAW_HORIZONTAL)
    {
        AreaConfig.u_16.u1 = Length;
        AreaConfig.u_16.u0 = Thickness;
    }
    else
    {
        AreaConfig.u_16.u1 = Thickness;
        AreaConfig.u_16.u0 = Length;
    }

  #ifdef DMA2D

    DMA2D->CR      = DMA2D_R2M | DMA2D_CR_TCIE;                                     // Register to memory and TCIE
    DMA2D->OCOLR   = Color;                                                         // Color to be used
    DMA2D->OMAR    = Address;                                                       // Destination address
    DMA2D->OOR     = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)AreaConfig.u_16.u1;  // Destination line offset
    DMA2D->OPFCCR  = PixelFormat;                                                   // Defines the number of pixels to be transfered
    DMA2D->NLR     = AreaConfig.u_32;                                               // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                             // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                              // Wait until transfer is done

  #else

  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintFont
//
//  Parameter(s):   FONT_sDescriptor*   pDescriptor
//                  sCartesian*         pPos
//  Return:         none
//
//  Description:    This function will print a font to drawing layer with the drawing color
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::PrintFont(sFontDescriptor* pDescriptor, sCartesian* pPos)
{
    uint32_t           PixelFormat;
    uint8_t            PixelSize;
    uint32_t           Address;
    CLayer*            pLayer;
    ePixelFormat       PixFmt;
    s32_t              AreaConfig;

    pLayer             = &LayerTable[GetDrawingLayer()];
    PixFmt             = pLayer->GetPixelFormat();
    PixelFormat        = PixelTable[PixFmt];
    PixelSize          = pLayer->GetPixelSize();
    Address            = pLayer->GetAddress() + (((pPos->Y * GRAFX_DRIVER_SIZE_X) + pPos->X) * (uint32_t)PixelSize);
    AreaConfig.u_16.u1 = pDescriptor->Size.Width;
    AreaConfig.u_16.u0 = pDescriptor->Size.Height;

  #ifdef DMA2D

    DMA2D->CR          = DMA2D_M2M_BLEND | DMA2D_CR_TCIE;                               // Memory to memory and TCIE

    // Font layer in Alpha blending linear (A8)
    DMA2D->FGMAR       = (uint32_t)pDescriptor->pAddress;                               // Source address 1
    DMA2D->FGOR        = 0;                                                             // Font source line offset - none as we are linear
    DMA2D->FGCOLR      = pLayer->GetTextColor();
    DMA2D->FGPFCCR     = LTDC_Pixelformat_A8;                                           // Defines the number of pixels to be transfered

    DMA2D->BGMAR       = Address;                                                       // Source address 2
    DMA2D->BGOR        = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)AreaConfig.u_16.u1;  // Font source line offset - none as we are linear
    DMA2D->BGPFCCR     = PixelFormat;                                                   // Defines the number of pixels to be transfered

    // Output Layer
    DMA2D->OMAR        = Address;
    DMA2D->OOR         = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)AreaConfig.u_16.u1;  // Destination line offset
    DMA2D->OPFCCR      = PixelFormat;

    // Area
    DMA2D->NLR         = AreaConfig.u_32;                                               // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                 // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                                  // Wait until transfer is done

  #else

  #endif

}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DRV_LayerConfig
//
//  Parameter(s):   CLayer* pLayer
//  Return:         None
//
//  Description:    Configuration for layer
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::LayerConfig(CLayer* pLayer)
{
  #ifdef DMA2D
    LTDC_Layer_InitTypeDef      LTDC_Layer_InitStruct;
    uint8_t                     PixelSize;
    LTDC_Layer_TypeDef*         pLTDC_SelectedLayer;
    eLayerType                  ActiveLayer;

    ActiveLayer = pLayer->GetLayer();

    if(ActiveLayer < GRAFX_NUMBER_OF_ACTIVE_LAYER)
    {
        pLTDC_SelectedLayer = (LTDC_Layer_TypeDef*)LTDC_baLayer[ActiveLayer];
        PixelSize           = pLayer->GetPixelSize();

        //      Windowing configuration
        //          All the active display area is used to display
        //          Horizontal start = horizontal synchronization + Horizontal back porch + 1
        //          Horizontal stop  = Horizontal start + window width
        //          Vertical start   = vertical synchronization + vertical back porch + 1
        //          Vertical stop    = Vertical start + window height
        LTDC_Layer_InitStruct.LTDC_HorizontalStart   = GRAFX_HSYNC + GRAFX_HBP + 1;
        LTDC_Layer_InitStruct.LTDC_HorizontalStop    = GRAFX_HSYNC + GRAFX_HBP + GRAFX_DRIVER_SIZE_X;
        LTDC_Layer_InitStruct.LTDC_VerticalStart     = GRAFX_VSYNC + GRAFX_VBP + 1;
        LTDC_Layer_InitStruct.LTDC_VerticalStop      = GRAFX_VSYNC + GRAFX_VBP + GRAFX_DRIVER_SIZE_Y;
        LTDC_Layer_InitStruct.LTDC_PixelFormat       = PixelTable[pLayer->GetPixelFormat()];          // Pixel Format configuration
        LTDC_Layer_InitStruct.LTDC_ConstantAlpha     = (uint32_t)pLayer->GetAlpha();                        // Alpha constant (255 totally opaque)
        LTDC_Layer_InitStruct.LTDC_BlendingFactor_1  = LTDC_BlendingFactor1_PAxCA;                          // Configure blending factors
        LTDC_Layer_InitStruct.LTDC_BlendingFactor_2  = LTDC_BlendingFactor2_PAxCA;
        LTDC_Layer_InitStruct.LTDC_DefaultColorBlue  = 0;                                                   // Default Color configuration (configure A,R,G,B component values)
        LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
        LTDC_Layer_InitStruct.LTDC_DefaultColorRed   = 0;
        LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
        LTDC_Layer_InitStruct.LTDC_CFBStartAdress    = pLayer->GetAddress();                                // Input Address configuration

        // the length of one line of pixels in bytes + 3 then :
        // Line Lenth = Active high width x number of bytes per pixel + 3
        // Active high width         = LCD_SIZE_Y
        LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((GRAFX_DRIVER_SIZE_X * (uint32_t)PixelSize) + 3);

        // the pitch is the increment from the start of one line of pixels to the
        // start of the next line in bytes, then :
        // Pitch = Active high width x number of bytes per pixel
        LTDC_Layer_InitStruct.LTDC_CFBPitch      = (GRAFX_DRIVER_SIZE_X * (uint32_t)PixelSize);
        LTDC_Layer_InitStruct.LTDC_CFBLineNumber =  GRAFX_DRIVER_SIZE_Y;                                              // Configure the number of lines

        LTDC_LayerInit(pLTDC_SelectedLayer, &LTDC_Layer_InitStruct);

        if(bDriverInitialize == true)
        {
            LTDC_LayerCmd(pLTDC_SelectedLayer, ENABLE);
        }

        LTDC_ReloadConfig(LTDC_VBReload);
    }
  #else
    VAR_UNUSED(pLayer);
  #endif
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
void GrafxGenDriver::DisplayOn(void)
{

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
void GrafxGenDriver::DisplayOff(void)
{

}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DRV_WaitFor_V_Sync
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Wait for the vertical synchro
//
//  Note(s):        this will prevent glitching on screen while changing display
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::WaitFor_V_Sync()
{
  #ifdef DMA2D
    while(LTDC_GetCDStatus(LTDC_CDSR_VSYNCS) != SET);           // Wait for Vertical sync to occur
  #endif
}

//-------------------------------------------------------------------------------------------------





//-------------------------------------------------------------------------------------------------
//
//   Function Name: CopyLinear
//
//   Parameter(s):  Skin_e               Image
//                  Cartesian_t          Position
//                  BlendMode_e          BlendMode
//   Return Value:  none
//
//   Description:   Copy a rectangle region from linear memory region to square memory area
//
//-------------------------------------------------------------------------------------------------
#ifdef GRAFX_USE_SOFT_COPY_LINEAR
void GrafxGenDriver::CopyLinear(Skin_e Image, Cartesian_t Position, BlendMode_e BlendMode)
{
    ImageInfo_t ImageInfo;
    Box_t       Box;

    if(Image != INVALID_IMAGE)
    {
        DB_Central.Get(&ImageInfo, GFX_IMAGE_INFO, uint16_t(Image), 0);
        Box.Pos.X       = Position.X;
        Box.Size.Width  = ImageInfo.Size.Width;
        Box.Pos.Y       = Position.Y;
        Box.Size.Height = ImageInfo.Size.Height;
        myGrafx->CopyLinear(ImageInfo.pPointer, &Box, ImageInfo.PixelFormat, BlendMode);
    }
}
#endif

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
#ifdef GRAFX_USE_SOFT_COPY_LINEAR
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
#endif

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
#ifdef GRAFX_USE_SOFT_COPY_LINEAR
void GrafxGenDriver::CopyLinear(void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, PixelFormat_e PixelFormat, BlendMode_e BlendMode)
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
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function Name: CopyLayerToLayer
//
//   Parameter(s):  Layer_e              SrcLayer
//                  Layer_e              DstLayer
//                  Box_t*               pBox
//   Return Value:  none
//
//   Description:   Copy a rectangle region from a layer to another layer
//
//-------------------------------------------------------------------------------------------------
#ifdef GRAFX_USE_SOFT_COPY_LAYER_TO_LAYER
void GrafxGenDriver::CopyLayerToLayer(Layer_e SrcLayer, Layer_e DstLayer, Box_t* pBox)
{
    CLayer* pLayer;

    CLayer::PushDrawing();
    CLayer::SetDrawing(DstLayer);
    pLayer = &LayerTable[SrcLayer];
    BlockCopy((void*)pLayer->GetAddress(), pBox, &pBox->Pos, pLayer->GetPixelFormat(), CLEAR_BLEND);

    CLayer::PopDrawing();
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function Name: CopyLayerToLayer
//
//   Parameter(s):  Layer_e             SrcLayer
//                  Layer_e             DstLayer
//                  uint16_t            X
//                  uint16_t            Y
//                  uint16_t            Width
//                  uint16_t            Height
//   Return Value:  none
//
//   Description:   Copy a rectangle region from a layer to another layer
//
//-------------------------------------------------------------------------------------------------
#ifdef GRAFX_USE_SOFT_COPY_LAYER_TO_LAYER
void GrafxGenDriver::CopyLayerToLayer(Layer_e SrcLayer, Layer_e DstLayer, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height)
{
    Box_t        Box;

    Box.Pos.X       = X;
    Box.Size.Width  = Width;
    Box.Pos.Y       = Y;
    Box.Size.Height = Height;

    CopyLayerToLayer(SrcLayer, DstLayer, &Box);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function Name: CopyBlockLayerToLayer
//
//   Parameter(s):  Layer_e             SrcLayer
//                  Layer_e             DstLayer
//                  uint16_t            X
//                  uint16_t            Y
//                  uint16_t            Width
//                  uint16_t            Height
//   Return Value:  none
//
//   Description:   Copy a rectangle region from a layer to another layer
//
//-------------------------------------------------------------------------------------------------
#ifdef GRAFX_USE_SOFT_COPY_LAYER_TO_LAYER
void GrafxGenDriver::CopyLayerToLayer(Layer_e SrcLayer, Layer_e DstLayer, uint16_t SrcX, uint16_t SrcY, uint16_t DstX, uint16_t DstY, uint16_t Width, uint16_t Height)
{
    CLayer*      pLayer;
    Box_t        Box;
    Cartesian_t  Pos;

    Box.Pos.X       = SrcX;
    Box.Size.Width  = Width;
    Box.Pos.Y       = SrcY;
    Box.Size.Height = Height;

    Pos.X = DstX;
    Pos.Y = DstY;

    CLayer::PushDrawing();
    CLayer::SetDrawing(DstLayer);
    pLayer = &LayerTable[SrcLayer];
    BlockCopy((void*)pLayer->GetAddress(), &Box, &Pos, pLayer->GetPixelFormat(), CLEAR_BLEND);
    CLayer::PopDrawing();
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawDLine
//
//   Parameter(s):  float    X1
//                  float    Y1
//                  float    X2
//                  float    Y2
//   Return Value:  none
//
//   Description:   Draw a diagonal line of desire thickness using the Xiaolin Wu Algorithm
//                  This algorithm allow line with anti-aliasing ( improvement on Bresenham)
//
//-------------------------------------------------------------------------------------------------
#if 0
void GrafxGenDriver::DrawDLine(float X1, float Y1, float X2, float Y2)
{
    int16_t DeltaX;
    int16_t DeltaY;
    float   Gradient;
    float   xEnd;
    float   yEnd;
    float   xGap;
    int16_t xPxl1;
    int16_t yPxl1;
    int16_t xPxl2;
    int16_t yPxl2;
    float   Intery;

    DeltaX = X2 - X1;
    DeltaY = Y2 - Y1;

    if(abs(DeltaX) > abs(DeltaY))
    {
        // Handle horizontal line
        if(X2 < X1)
        {
            float TP;
            TP = X1; X1 = X2; X2 = TP;
            TP = Y1; Y1 = Y2; Y2 = TP;
        }

        Gradient = DeltaY / DeltaX;

        // Handle first end of line point
        xEnd = static_cast<float>(round(X1));
        yEnd = Y1 + Gradient * (xEnd - X1);
        xGap = rfpart(X1 + 0.5);

        xPxl1 = static_cast<int>(xEnd);
        yPxl1 = ipart(yEnd);

        // Add the first endpoint
        plot(xPxl1, yPxl1, rfpart(yEnd) * xGap);
        plot(xPxl1, yPxl1 + 1, fpart(yEnd) * xGap);

        Intery = yEnd + Gradient;

        // Handle second end of line point
        xEnd = static_cast<float>(round(X2));
        yEnd = Y2 + Gradient * (xEnd - X2);
        xGap = fpart(X2 + 0.5);

        xPxl2 = static_cast<int>(xEnd);
        yPxl2 = ipart(yEnd);

        plot(xPxl2, yPxl2, rfpart(yEnd) * xGap);
        plot(xPxl2, yPxl2 + 1, fpart(yEnd) * xGap);

        // Main loop
        for(int16_t x = xPxl1 + 1; x < xPxl2; x++)
        {
            plot(x, ipart(Intery), rfpart(Intery));
            plot(x, ipart(Intery) + 1, fpart(Intery));
            Intery += Gradient;
        }
    }
    else
    {
         // TODO Handle horizontal lines by swapping X and Y
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawCircle
//
//   Parameter(s):  Circle_t*        pCircle
//                  PolygonMode_e    PolygonMode
//   Return Value:  none
//
//   Description:   Draw a circle on screen base on Bresenham algo
//
//   notes:         Option are:         FILL_POLY  -> for a completely fill circle
//                                      SHAPE_POLY -> for the contour shape only
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawCircle(Circle_t* pCircle, PolygonMode_e PolygonMode)
{
    DrawCircle(pCircle->Pos.X, pCircle->Pos.Y, pCircle->R, PolygonMode);
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawCircle
//
//   Parameter(s):  uint16_t           X
//                  uint16_t           Y
//                  uint16_t           Radius
//                  GRAFX_PolygonMode_e   PolygonMode
//   Return Value:  none
//
//   Description:   Draw a circle on screen base on Bresenham algo
//
//   notes:         Option are:         POLY_FILL  -> for a completely fill circle
//                                      POLY_SHAPE -> for the contour shape only
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawCircle(uint16_t PosX, uint16_t PosY, uint16_t Radius, PolygonMode_e PolygonMode)
{
    int16_t     X;
    int16_t     Y;
    int16_t     Decision;
    uint16_t    q1;
    uint16_t    q2;
    uint16_t    q3;
    uint16_t    q4;
    uint16_t    q5;
    uint16_t    q6;
    uint16_t    q7;
    uint16_t    q8;
    uint8_t     Skip;

    X = 0;
    Y = Radius;
    Decision = 3 - ((int16_t)Radius << 1);

    while(X <= Y)
    {
        q1 = PosX - (uint16_t)X;
        q2 = PosY - (uint16_t)Y;
        q3 = PosX - (uint16_t)Y;
        q4 = PosY - (uint16_t)X;
        q5 = PosX + (uint16_t)X;
        q6 = PosY + (uint16_t)Y;
        q7 = PosX + (uint16_t)Y;
        q8 = PosY + (uint16_t)X;

        if(PolygonMode == POLY_FILL)
        {
            q1 = (X > (int16_t)PosX) ? 0 : q1;
            q2 = (Y > (int16_t)PosY) ? 0 : q2;
            q3 = (Y > (int16_t)PosX) ? 0 : q3;
            q4 = (X > (int16_t)PosY) ? 0 : q4;

            DrawHLine(q2, q1, q5, 1);
            DrawHLine(q6, q1, q5, 1);
            DrawHLine(q8, q3, q7, 1);
            DrawHLine(q4, q3, q7, 1);
        }
        else
        {
            Cartesian_t Size;

            Size.X = 240; Size.Y = 320;
           // Size = CLayer::GetMaxSize();
// TODO (Alain#1#): update this when it is time


            Skip = 0;
            if(X  >  (int16_t)PosX)   Skip |= 0x01;
            if(Y  >  (int16_t)PosY)   Skip |= 0x02;
            if(Y  >  (int16_t)PosX)   Skip |= 0x04;
            if(X  >  (int16_t)PosY)   Skip |= 0x08;
            if(q5 >= (int16_t)Size.X) Skip |= 0x10;
            if(q6 >= (int16_t)Size.Y) Skip |= 0x20;
            if(q7 >= (int16_t)Size.X) Skip |= 0x40;
            if(q8 >= (int16_t)Size.Y) Skip |= 0x80;

            if((Skip & 0x03) == 0) DrawPixel(q1, q2);
            if((Skip & 0x12) == 0) DrawPixel(q5, q2);
            if((Skip & 0x21) == 0) DrawPixel(q1, q6);
            if((Skip & 0x30) == 0) DrawPixel(q5, q6);
            if((Skip & 0x0C) == 0) DrawPixel(q3, q4);
            if((Skip & 0x48) == 0) DrawPixel(q7, q4);
            if((Skip & 0x84) == 0) DrawPixel(q3, q8);
            if((Skip & 0xC0) == 0) DrawPixel(q7, q8);
        }

        if(Decision < 0)
        {
            Decision += ((int16_t)X << 2) + 6;
        }
        else
        {
            Decision += ((((int16_t)X - (int16_t)Y) << 2) + 10);
            Y -= 1;
        }
        X++;
    }
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
