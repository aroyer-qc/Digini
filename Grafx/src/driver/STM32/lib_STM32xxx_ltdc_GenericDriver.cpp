//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32xxx_ltdc_Grafx_GenDriver.cpp
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
#include "./Grafx/inc/driver/STM32/lib_STM32_generic.h"

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
//  Parameter(s):   pArg
//  Return:         None
//
//  Description:    LCD configuration specific for the LCD and processor used by this driver
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::Initialize(const void* pArg)
{
    VAR_UNUSED(pArg);

  #ifdef LTDC
   #ifdef STM32H7xx
    RCC->APB3ENR |= RCC_APB3ENR_LTDCEN;
    RCC->AHB3ENR |= RCC_AHB3ENR_DMA2DEN;
   #else
    RCC->APB2ENR |= RCC_APB2ENR_LTDCEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN;
   #endif

    // Configures the HS, VS, DE and PC polarity
    LTDC->GCR = 0;

    // Sets Synchronization size
    MODIFY_REG(LTDC->SSCR,
               (LTDC_SSCR_VSH | LTDC_SSCR_HSW),
               (GRAFX_VSYNC - 1) | ((GRAFX_HSYNC - 1) << LTDC_SSCR_HSW_Pos));

    // Sets Accumulated Back porch
    MODIFY_REG(LTDC->BPCR,
               (LTDC_BPCR_AVBP | LTDC_BPCR_AHBP),
               (GRAFX_VSYNC  + GRAFX_VBP - 1) | ((GRAFX_HSYNC  + GRAFX_HBP - 1) << LTDC_BPCR_AHBP_Pos));

    // Sets Accumulated Active Width
    MODIFY_REG(LTDC->AWCR,
               (LTDC_AWCR_AAH | LTDC_AWCR_AAW),
               ((GRAFX_DRIVER_SIZE_Y + GRAFX_VSYNC + GRAFX_VBP - 1) |
                ((GRAFX_DRIVER_SIZE_X + GRAFX_HSYNC + GRAFX_HBP - 1) << LTDC_AWCR_AAW_Pos)));

    // Sets Total Width for vertical and horizontal
    MODIFY_REG(LTDC->TWCR, (LTDC_TWCR_TOTALH | LTDC_TWCR_TOTALW),
                           ((GRAFX_DRIVER_SIZE_Y + GRAFX_VSYNC + GRAFX_VBP + GRAFX_VFP - 1) |
                           ((GRAFX_DRIVER_SIZE_X + GRAFX_HSYNC + GRAFX_HBP + GRAFX_HFP - 1) << LTDC_TWCR_TOTALW_Pos)));

    CLEAR_BIT(LTDC->BCCR, (LTDC_BCCR_BCBLUE | LTDC_BCCR_BCGREEN | LTDC_BCCR_BCRED));    // Sets the background color value to zero for all
    SET_BIT(LTDC->IER, LTDC_IER_TERRIE | LTDC_IER_FUIE);                                // Enable the transfer Error interrupt and FIFO underrun
    SET_BIT(LTDC->GCR, LTDC_GCR_LTDCEN);                                                // Enable LTDC by setting LTDCEN bit

    LayerConfig(BACKGROUND_DISPLAY_LAYER_0);
    LayerConfig(FOREGROUND_DISPLAY_LAYER_0);

  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearLayer
//
//  Parameter(s):   Layer_e 		Layer
//  Return:         None
//
//  Description:    LCD configuration specific for the LCD and processor used by this driver
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::ClearLayer(Layer_e Layer)
{
    DisplayLayer*  pLayer  = &LayerTable[Layer];
    uint32_t Address = pLayer->GetAddress();

  #ifdef DMA2D

    uint32_t  PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    uint32_t  AreaConfig  = (pLayer->GetSize().X << 16) | pLayer->GetSize().Y;

    // Configure DMA2D for Register-to-Memory (constant color fill)
    DMA2D->CR      = DMA2D_R2M | DMA2D_CR_TCIE;
    DMA2D->OCOLR   = pLayer->GetColor();                        // Constant color
    DMA2D->OMAR    = Address;                                   // Destination address
    DMA2D->OOR     = 0;                                         // No line offset
    DMA2D->OPFCCR  = PixelFormat;                               // Pixel format
    DMA2D->NLR     = AreaConfig;                                // Width + Height

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                         // Start operation
    while (DMA2D->CR & DMA2D_CR_START);                         // Wait for completion

  #else // some F4xx doesn't have DMA2D

    uint32_t PixelSize = pLayer->GetPixelSize();
    uint32_t Color     = pLayer->GetColor();
    uint32_t Size      = (pLayer->GetSize().X << 16) | pLayer->GetSize().Y * PixelSize;

    // Clear using DMA memory-to-memory (increment only the destination)
    DMA_Memcpy(&Color, (void*)Address, Size, DMA_SxCR_MINC);

  #endif
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
//                  region
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
	//if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)   // why??
    {
        uint32_t           PixelFormatSrc;
        uint32_t           PixelFormatDst;
        uint32_t           Address;
        uint32_t           AreaConfig;
        DisplayLayer*      pLayer;
        uint8_t            PixelSize;

        pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
        PixelFormatSrc = m_PixelFormatTable[SrcPixelFormat];
        PixelFormatDst = m_PixelFormatTable[pLayer->GetPixelFormat()];
        PixelSize      = pLayer->GetPixelSize();
        Address        = pLayer->GetAddress() + (((pDstPos->Y * GRAFX_DRIVER_SIZE_X) + pDstPos->X) * (uint32_t)PixelSize);
        AreaConfig     = (uint32_t(pBox->Size.Width)  << 16) | (uint32_t(pBox->Size.Height));

      #ifdef DMA2D

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

      #else

        uint8_t* pLocalSrc = (uint8_t*)pSrc;
        uint32_t* pDst = (uint32_t*)Address;
        pLocalSrc += (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
        DMA_Memcpy(pLocalSrc, pDst, size_t(AreaConfig));

      #endif
    }
 //   else
    {

    }
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
    uint32_t        PixelFormatSrc;
    uint32_t        PixelFormatDst;
    uint32_t        Address;
    uint32_t        AreaConfig;
    DisplayLayer*   pLayer;
    uint8_t         PixelSize;

    pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormatSrc = m_PixelFormatTable[SrcPixelFormat];
    PixelFormatDst = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize      = pLayer->GetPixelSize();
    Address        = pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
    AreaConfig     = (uint32_t(pBox->Size.Width) << 16) | (uint32_t(pBox->Size.Height));

  #ifdef DMA2D
    if(PixelFormatSrc == PixelFormatDst)
    {
        DMA2D->CR = ((BlendMode == CLEAR_BLEND) ? DMA2D_M2M : DMA2D_M2M_BLEND) | DMA2D_CR_TCIE; // Memory to memory or M2M with blending BG + Source
    }
    else
    {
        DMA2D->CR = DMA2D_M2M_PFC | DMA2D_CR_TCIE;                                              // Memory to memory with pixel conversion
    }

    // Source 1
    DMA2D->FGMAR   = (uint32_t)pSrc;                                                            // Source address
    DMA2D->FGOR    = 0;                                                                         // Source line offset none as we are linear
    DMA2D->FGPFCCR = PixelFormatSrc;                                                            // Defines the size of pixel

    // Source 2 (Source2 vs Destination = Read modify write)
    DMA2D->BGMAR   = Address;                                                                   // Source address
    DMA2D->BGOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                // Source line offset
    DMA2D->BGPFCCR = PixelFormatDst;                                                            // Defines the size of pixel

    // Destination
    DMA2D->OMAR    = Address;                                                                   // Destination address
    DMA2D->OOR     = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;                // Destination line offset
    DMA2D->OPFCCR  = PixelFormatDst;                                                            // Defines the size of pixel

    DMA2D->NLR     = AreaConfig;                                                                // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                         // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                                          // Wait until transfer is done

  #else

    uint8_t* pLocalSrc = (uint8_t*)pSrc;
    uint32_t* pDst = (uint32_t*)Address;
    pLocalSrc += (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
    DMA_Memcpy(pLocalSrc, pDst, size_t(AreaConfig));

  #endif
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
    uint32_t PixelFormat;
    uint32_t Address;
    uint32_t Color;
    uint32_t AreaConfig;
    DisplayLayer*  pLayer;
    uint8_t  PixelSize;

    pLayer      = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    Address     = pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_DRIVER_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
    Color       = pLayer->GetColor();
    AreaConfig  = (uint32_t(pBox->Size.Width) << 16) | (uint32_t(pBox->Size.Height));

  #if defined(DMA2D)

    DMA2D->CR     = DMA2D_R2M | DMA2D_CR_TCIE;                                     // Register to memory and TCIE
    DMA2D->OCOLR  = Color;                                                         // Color to be used
    DMA2D->OMAR   = Address;                                                       // Destination address
    DMA2D->OOR    = (uint32_t)GRAFX_DRIVER_SIZE_X - (uint32_t)pBox->Size.Width;    // Destination line offset
    DMA2D->OPFCCR = PixelFormat;                                                   // Defines the number of pixels to be transfered
    DMA2D->NLR    = AreaConfig;                                                    // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                            // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                                             // Wait until transfer is done

  #else
    // TODO if needed
  #endif
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
    DisplayLayer*        pLayer;
    uint8_t        PixelSize;

    pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat    = m_PixelFormatTable[pLayer->GetPixelFormat()];
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
    uint32_t PixelFormat;
    uint8_t  PixelSize;
    uint32_t Address;
    uint32_t Color;
    DisplayLayer*  pLayer;
    uint32_t AreaConfig;
    uint32_t Offset;

    pLayer      = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize   = pLayer->GetPixelSize();
    Address     = pLayer->GetAddress() + (((PosY * GRAFX_DRIVER_SIZE_X) + PosX) * (uint32_t)PixelSize);
    Color       = pLayer->GetColor();

    if(Direction == DRAW_HORIZONTAL)
    {
        AreaConfig = (uint32_t(Length) << 16) | (uint32_t(Thickness));
        Offset     = Length;
    }
    else
    {
        AreaConfig = (uint32_t(Thickness) << 16) | (uint32_t(Length));
        Offset     = Thickness;
    }

  #ifdef DMA2D

    DMA2D->CR      = DMA2D_R2M | DMA2D_CR_TCIE;                 // Register to memory and TCIE
    DMA2D->OCOLR   = Color;                                     // Color to be used
    DMA2D->OMAR    = Address;                                   // Destination address
    DMA2D->OOR     = (uint32_t)GRAFX_DRIVER_SIZE_X - Offset;    // Destination line offset
    DMA2D->OPFCCR  = PixelFormat;                               // Defines the number of pixels to be transfered
    DMA2D->NLR     = AreaConfig;                                // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                         // Start operation
    while(DMA2D->CR & DMA2D_CR_START);                          // Wait until transfer is done

  #else

  #endif
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
void GrafxGenDriver::LayerConfig(DisplayLayer* pLayer)
{
  #ifdef DMA2D
    uint32_t            PixelFormat;
    uint32_t            PixelSize;
    LayerType_e         ActiveLayer;
    LTDC_Layer_TypeDef* pActiveLayer;

    ActiveLayer = pLayer->GetActive();
    PixelFormat = pLayer->GetPixelFormat();
    PixelSize   = GFX_PixelSize[PixelFormat];

    if(ActiveLayer < GRAFX_NUMBER_OF_ACTIVE_LAYER)
    {
        pActiveLayer = (ActiveLayer == 0) ? LTDC_Layer1 : LTDC_Layer2;
        pActiveLayer->WHPCR = ((((LTDC->BPCR & LTDC_BPCR_AHBP) >> LTDC_BPCR_AHBP_Pos) + 1) |    // Configures the horizontal start and stop position
                               (((GRAFX_DRIVER_SIZE_X - 1) + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> LTDC_BPCR_AHBP_Pos)) << LTDC_LxWHPCR_WHSPPOS_Pos));
        pActiveLayer->WVPCR = (((LTDC->BPCR & LTDC_BPCR_AVBP) + 1) |                            // Configures the vertical start and stop position
                               (((GRAFX_DRIVER_SIZE_Y - 1) +  (LTDC->BPCR & LTDC_BPCR_AVBP)) << LTDC_LxWVPCR_WVSPPOS_Pos));
        pActiveLayer->PFCR  = m_PixelFormatTable[PixelFormat];                                  // Specifies the pixel format
        pActiveLayer->DCCR  = 0;                                                                // Configures the default color values ( all zero)
        pActiveLayer->CACR  = (uint32_t)pLayer->GetAlpha();                                     // Specifies the constant alpha value
        pActiveLayer->BFCR  = (LTDC_BLENDING_FACTOR1_PAxCA | LTDC_BLENDING_FACTOR2_PAxCA);      // Specifies the blending factors
        pActiveLayer->CFBAR = pLayer->GetAddress();                                             // Configures the color frame buffer start address
        pActiveLayer->CFBLR = (((GRAFX_DRIVER_SIZE_X * PixelSize) << LTDC_LxCFBLR_CFBP_Pos) |   // Configures the color frame buffer pitch in byte
                               (((GRAFX_DRIVER_SIZE_X - 1) * PixelSize)  + 3));
        pActiveLayer->CFBLNR = GRAFX_DRIVER_SIZE_Y;                                             // Configures the frame buffer line number
        SET_BIT(pActiveLayer->CR, LTDC_LxCR_LEN);                                               // Enable LTDC_Layer by setting LEN bit
        LTDC->SRCR = LTDC_SRCR_IMR;                                                             // Reload
    }
  #else
    VAR_UNUSED(pLayer);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WaitFor_V_Sync
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Wait for the vertical synchro
//
//  Note(s):        this will prevent glitching on screen while changing display
//
//-------------------------------------------------------------------------------------------------
#if (GRAFX_DRIVER_USE_V_SYNC == DEF_ENABLED)
void GrafxGenDriver::WaitFor_V_Sync(void)
{
  #ifdef DMA2D
    while(LTDC_GetCDStatus(LTDC_CDSR_VSYNCS) != SET);           // Wait for Vertical sync to occur
  #endif
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_GRAFX == DEF_ENABLED)
