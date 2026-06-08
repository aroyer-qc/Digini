//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32xxx_fmc_lcd_320x240 - SSD2119.cpp
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

#include "digini_cfg.h"
#ifdef DIGINI_USE_GRAFX
#define LIB_SSD2119_GLOBAL
#include "./lib_digini.h"
#undef  LIB_SSD2119_GLOBAL
#include "./Grafx/inc/driver/STM32/lib_STM32_Generic.h"

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

#if (GRAFX_USE_FULL_FRAME_CONSTRUCTION_LAYER == DEF_ENABLED)
    #error GRAFX_USE_FULL_FRAME_CONSTRUCTION_LAYER must be set to DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

//notes: In R01h, bits REV, BGR, RL, CM will override the corresponding hardware pins settings. Setting R28h as 0x0006 is required before setting R25h and R29h registers.

const SSD2119_InitCMD_t GrafxDriver::m_InitCMD[GRAFX_NUMBER_OF_INIT_CMD] =
{
    {SSD2119_DISPLAY_CONTROL_REGISTER,              SSD2119_DISPLAY_OFF_VALUE},          // Display OFF
    {SSD2119_OSCILLATOR_START_REGISTER,             SSD2119_OSCILLATOR_ENABLE},          // Enable oscillator
    {SSD2119_POWER_CONTROL_1_REGISTER,              SSD2119_POWER_CONTROL_1_VALUE},      // Power step 1
    {SSD2119_POWER_CONTROL_2_REGISTER,              SSD2119_POWER_CONTROL_2_VALUE},      // Power step 2
    {SSD2119_POWER_CONTROL_3_REGISTER,              SSD2119_POWER_CONTROL_3_VALUE},      // Power step 3
    {SSD2119_POWER_CONTROL_4_REGISTER,              SSD2119_POWER_CONTROL_4_VALUE},      // Power step 4
    {SSD2119_POWER_CONTROL_5_REGISTER,              SSD2119_POWER_CONTROL_5_VALUE},      // Power step 5
    {SSD2119_VCOM_OTP_1_REGISTER,                   SSD2119_VCOM_OTP_1_VALUE},           // VCOM amplitude
    {SSD2119_VCOM_OTP_2_REGISTER,                   SSD2119_VCOM_OTP_2_VALUE},           // VCOM amplitude
    {SSD2119_SLEEP_MODE_REGISTER,                   SSD2119_DISPLAY_EXIT_SLEEP_MODE},    // Exit sleep
    {SSD2119_ENTRY_MODE_REGISTER,                   SSD2119_ENTRY_MODE_VALUE},           // RGB565, normal scan
    {SSD2119_OUTPUT_CONTROL_REGISTER,               SSD2119_OUTPUT_CONTROL_VALUE},       // Panel type + scan direction
    {SSD2119_GATE_SCAN_START_REGISTER,              0x0000},                             // Start at gate 0
    {SSD2119_FRAME_FREQUENCY_REGISTER,              SSD2119_FRAME_FREQUENCY_VALUE},      // Frame timing
    {SSD2119_FRAME_FREQUENCY_CONTROL_2_REGISTER,    SSD2119_FRAME_FREQUENCY_CONTROL_2_VALUE},
    {SSD2119_FRAME_CYCLE_CONTROL_REGISTER,          SSD2119_FRAME_CYCLE_CONTROL_VALUE},
    {SSD2119_VCOM_FINE_ADJUSTMENT_REGISTER,         0x0009},
    {SSD2119_GAMMA_CONTROL_1_REGISTER,              0x0000},
    {SSD2119_GAMMA_CONTROL_2_REGISTER,              0x0106},
    {SSD2119_GAMMA_CONTROL_3_REGISTER,              0x0100},
    {SSD2119_GAMMA_CONTROL_4_REGISTER,              0x0303},
    {SSD2119_GAMMA_CONTROL_5_REGISTER,              0x0003},
    {SSD2119_GAMMA_CONTROL_6_REGISTER,              0x0004},
    {SSD2119_GAMMA_CONTROL_7_REGISTER,              0x0203},
    {SSD2119_GAMMA_CONTROL_8_REGISTER,              0x0303},
    {SSD2119_GAMMA_CONTROL_9_REGISTER,              0x0100},
    {SSD2119_GAMMA_CONTROL_10_REGISTER,             0x0504},
    {SSD2119_DISPLAY_CONTROL_REGISTER,              SSD2119_DISPLAY_ON_VALUE}             // Display ON
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void*      pArg       Optional initialization argument passed to the driver.
//
//  Return:         None
//
//  Description:    Performs the complete initialization sequence of the graphics driver.
//                  Resets the LCD controller, reads the device identification code, and
//                  sends the full initialization command table to configure the SSD2119.
//                  Once the controller is ready, the foreground display layer is cleared
//                  to a known state.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Initialize(const void* pArg)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN;

    // I may need to provide a pointer to the background image for building element to display on the screen (merge)
    m_pBackground = (StaticImageInfo_t*)pArg;

    GrafxGenDriver::Initialize(nullptr);
    IO_SetPinHigh(IO_LCD_RESET);
    LIB_Delay_mSec(5);

    // Send the complete list of initialization command to LCD
    for(int i = 0; i < GRAFX_NUMBER_OF_INIT_CMD; i++)
    {
        WriteCommand(m_InitCMD[i].Register, m_InitCMD[i].Parameter);

        if(m_InitCMD[i].Register == SSD2119_SLEEP_MODE_REGISTER)
        {
            LIB_Delay_mSec(30);
        }
    }

    ClearLayer(FOREGROUND_DISPLAY_LAYER_0);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearLayer
//
//  Parameter(s):   Layer_e    Layer      Specifies the display layer to clear.
//
//  Return:         None
//
//  Description:    Clears the selected display layer by writing a constant color value to the
//                  entire GRAM region associated with this driver. The GRAM write pointer is
//                  positioned at the origin before sequential pixel data is written.
//
//  Note(s):        Only layers managed directly by this driver are cleared. Any additional
//                  layers are handled by the base graphics class.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::ClearLayer(Layer_e Layer)
{
   	if(Layer == FOREGROUND_DISPLAY_LAYER_0)
    {
        ResetWindow();

        for(uint32_t i = 0; i < GRAFX_DRIVER_SIZE; i++)
        {
            WriteData(0x0000);
        }
    }
    else
    {
        GrafxGenDriver::ClearLayer(Layer);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawBox
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness)
{
    Box_t Box;

    Box.Pos.X = PosX;
    Box.Pos.Y = PosY;
    Box.Size.Width  = Length;
    Box.Size.Height = Thickness;
    DrawRectangle(&Box);                        // Top
    Box.Pos.Y = PosY + Height - Thickness;
    DrawRectangle(&Box);                        // Bottom
    Box.Pos.Y = PosY + Thickness;
    Box.Size.Height = Height - (2 * Thickness);
    Box.Size.Width  = Thickness;
    DrawRectangle(&Box);                        // Left
    Box.Pos.X = PosX + Length - Thickness;
    DrawRectangle(&Box);                        // Right
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
void GrafxDriver::DrawRectangle(Box_t* pBox)
{
    if(DisplayLayer::GetDrawing() == GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint32_t      Color;
        uint32_t      Size;
        DisplayLayer* pLayer;

        pLayer = &LayerTable[DisplayLayer::GetDrawing()];
        Color  = pLayer->GetColor();
        SetWindow(pBox);
        Size = pBox->Size.Width * pBox->Size.Height;

        for(uint32_t i = 0; i < Size; i++)
        {
             WriteData(Color);
        }
    }
    else
    {
        GrafxGenDriver::DrawRectangle(pBox);
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
//  Description:    Copy a rectangle region from 2D memory region to another 2D memory region
//
//  Note(s):        Source is linear, also we work only on a RG565 destination.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    VAR_UNUSED(BlendMode);                      // NU On this LCD

    if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint32_t Width       = uint32_t(pBox->Size.Width);
        uint32_t Height      = uint32_t(pBox->Size.Height);

        DisplayLayer* pLayer         = &LayerTable[DisplayLayer::GetDrawing()];
        uint32_t ConstructAlphaLayer = pLayer->GetAddress();
        uint32_t Offset              = uint32_t(pLayer->GetSize().X) - Width;
        uint32_t OffsetSource        = ((pDstPos->Y * pLayer->GetSize().X) + pDstPos->X) * sizeof(uint32_t);

        DMA2D->CR = DMA2D_M2M_BLEND;                                                        // Memory to memory and TCIE blending BG + Source

        //Source of the image to blend
        DMA2D->FGMAR   = uint32_t(pSrc);
        DMA2D->FGOR    = 0;                                                                 // Source line offset so none as we are linear
        DMA2D->FGPFCCR = SrcPixelFormat;                                                    // Defines the size of pixel.

       // Source in construction layer of the previous blended image or just background
        DMA2D->BGMAR   = ConstructAlphaLayer + OffsetSource;                                // Source address
        DMA2D->BGOR    = Offset;                                                            // Source line offset
        DMA2D->BGPFCCR = DMA2D_CONVERSION_ARGB8888;                                         // Defines the size of pixel.

        //Destination write back to construction layer
        DMA2D->OMAR    = ConstructAlphaLayer + OffsetSource;                                // Destination address
        DMA2D->OOR     = Offset;                                                            // Destination line offset
        DMA2D->OPFCCR  = DMA2D_CONVERSION_ARGB8888;                                         // Defines the size of pixel.

        DMA2D->NLR     = (Width << 16) | Height;                                            // Size configuration of area to be transfered
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawPixel
//
//  Parameter(s):   uint16_t    PosX
//                  uint16_t    PosY
//  Return:         None
//
//  Description:    Put a pixel on LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawPixel(uint16_t PosX, uint16_t PosY)
{
    DisplayLayer* pLayer;
    uint32_t      Color;

    pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    Color  = pLayer->GetColor();
    SetRAM_Pointer(PosX, PosY);
    WriteData(Color);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawLine
//
//  Parameter(s):   uint16_t   PosX       Specifies the X position, can be a value from 0 to 240.
//                  uint16_t   PosY       Specifies the Y position, can be a value from 0 to 320.
//                  uint16_t   Length     Line length.
//                  uint16_t   Thickness
//                  DrawMode_e Direction  line direction.
//                                        This Parameter can be one of the following values
//                                                 DRAW_HORIZONTAL
//                                                 DRAW_VERTICAL
//  Return:         None
//
//  Description:    Displays a line of a specific Thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawLine(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction)
{

    //m_pLayer = &LayerTable[DisplayLayer::GetDrawing()];

    // if memory calculate offset check if we can call the default function
    // else

    //get the color!!
    SetRAM_Pointer(PosX, PosY);
    // loop for the thickness
    // configure the offset of auto increment...
    // loop for the number of pixel..
   // Write(Color);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BlendFromImage
//
//  Parameter(s):   ImageID     Image identifier used to retrieve image metadata and raw data.
//                  Position    Top-left coordinate where the image will be blended.
//                  BlendMode   Blending mode (unused on this LCD; alpha handled via DMA2D).
//
//  Return:         None
//
//  Description:    Performs a linear memory-to-memory copy of an ARGB8888 image into the active
//                  construction layer. If the image is RLE32-compressed, it is first expanded
//                  into a temporary ARGB8888 buffer. The STM32 DMA2D engine is then configured
//                  for M2M blending, combining:
//
//                      - Foreground : ARGB8888 source image
//                      - Background : ARGB8888 construction layer only into this driver
//
//                  The blended result is written into the construction layer buffer. This path
//                  is only executed when drawing on the foreground construction layer; otherwise,
//                  the function falls back to the standard CopyLinear() implementation.
//
//                  Temporary buffers allocated for RLE32 decoding are released after use.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::BlendFromImage(ImageID_e ImageID, Cartesian_t Position, BlendMode_e BlendMode)
{
    VAR_UNUSED(BlendMode);                      // NU On this LCD

    if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        StaticImageInfo_t* pImage;
        uint32_t*          pImageSourceAlpha = nullptr;

        DB_Central.Get(&pImage, GFX_IMAGE_INFO, uint16_t(ImageID));
        uint32_t Width     = uint32_t(pImage->Info.Size.Width);
        uint32_t Height    = uint32_t(pImage->Info.Size.Height);
        uint32_t ImageSize = Width * Height;

        // Copy source alpha to blend actual construction layer
        DisplayLayer* pLayer              = &LayerTable[DisplayLayer::GetDrawing()];
        uint32_t      ConstructAlphaLayer = pLayer->GetAddress();

        if(pImage->Compression == COMPX_RLE_32)
        {
            pImageSourceAlpha = (uint32_t*)pMemoryPool->Alloc(ImageSize * sizeof(uint32_t), MEM_DBG_GRAFX_CL1);
            WriteRLE32((StaticImageRLE_32_t*)pImage->Info.pPointer, pImageSourceAlpha, pImage->RawSize);
        }
        else
        {
            pImageSourceAlpha = (uint32_t*)pImage->Info.pPointer;
        }

        // DMA2D the 2 buffers
        uint32_t Offset = uint32_t(pLayer->GetSize().X) - Width;
        uint32_t AddressOffset = 0;

        if((Position.X > m_ConstructPosition.X) || (Position.Y > m_ConstructPosition.Y))
        {
            Position.X  -= m_ConstructPosition.X;
            Position.Y  -= m_ConstructPosition.Y;
            AddressOffset = ((Position.Y * pLayer->GetSize().X) + Position.X) * sizeof(uint32_t);
        }

        DMA2D->CR = DMA2D_M2M_BLEND;                                                        // Memory to memory and TCIE blending BG + Source

        //Source of the image to blend
        DMA2D->FGMAR   = uint32_t(pImageSourceAlpha);
        DMA2D->FGOR    = 0;                                                                 // Source line offset so none as we are linear
        DMA2D->FGPFCCR = DMA2D_CONVERSION_ARGB8888;                                         // Defines the size of pixel.

        // Source in construction layer of the previous blended image or just background
        DMA2D->BGMAR   = ConstructAlphaLayer + AddressOffset;                               // Source address
        DMA2D->BGOR    = Offset;                                                            // Source line offset
        DMA2D->BGPFCCR = DMA2D_CONVERSION_ARGB8888;                                         // Defines the size of pixel.

        //Destination write back to construction layer
        DMA2D->OMAR    = ConstructAlphaLayer + AddressOffset;                               // Destination address
        DMA2D->OOR     = Offset;                                                            // Destination line offset
        DMA2D->OPFCCR  = DMA2D_CONVERSION_ARGB8888;                                         // Defines the size of pixel.

        DMA2D->NLR     = (Width << 16) | Height;                                            // Size configuration of area to be transfered

        SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                 // Start operation
        while ((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                                     // Wait for transfer complete
        DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                                                  // Clear flag

        //-------------------------------------------------------------------------
        // Free Resource

        if(pImage->Compression == COMPX_RLE_32)
        {
            pMemoryPool->Free((void**)&pImageSourceAlpha);
        }
    }
    else
    {
        GrafxGenDriver::BlendFromImage(ImageID, Position, BlendMode);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintFont
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::PrintFont(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)
{
    if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint32_t    ConstructAlphaLayer;
        Cartesian_t Pos;

        DisplayLayer* pLayer = &LayerTable[DisplayLayer::GetDrawing()];
        Pos.X = pPos->X - m_ConstructPosition.X;
        Pos.Y = pPos->Y - m_ConstructPosition.Y;
        //uint32_t Width               = uint32_t(pDescriptor->WidthPixel);

        ConstructAlphaLayer  = ((Pos.Y * uint32_t(pLayer->GetSize().X)) + Pos.X);           // Calculate Offset of print
        ConstructAlphaLayer *= sizeof(uint32_t);                                            // Adjust for ARGB size
        ConstructAlphaLayer += pLayer->GetAddress();                                        // Add Address of the construction layer
        // DMA2D the 2 buffers

        _PrintFont(pDescriptor, ConstructAlphaLayer, uint32_t(pLayer->GetSize().X), pPos, pLayer->GetTextColor());
    }
    else
    {
        PrintFont(pDescriptor, pPos);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ImageCopy
//
//  Parameter(s):   ImageID     Image identifier used to retrieve image metadata and raw data.
//                  PosX        X coordinate where the image will be drawn.
//                  PosY        Y coordinate where the image will be drawn.
//
//  Return:         None
//
//  Description:    Draws an image directly to the LCD at the specified position. The function
//                  configures the LCD GRAM window to match the image dimensions, then transfers
//                  the pixel data according to the image compression format:
//
//                  - COMPX_COMPRESSION_NONE : Raw RGB565 data copied pixel-by-pixel.
//                  - COMPX_RLE_16           : RLE16-compressed data expanded and streamed to LCD.
//                  - COMPX_RLE_32           : Not supported on this LCD (no ARGB8888).
//
//                  The caller must ensure that the image fits within the display boundaries.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::ImageCopy(ImageID_e ImageID, uint16_t PosX, uint16_t PosY)
{
    StaticImageInfo_t* pImage;

    DB_Central.Get(&pImage, GFX_IMAGE_INFO, uint16_t(ImageID));
    uint32_t  Size  = pImage->RawSize;
    SetWindow(PosX, PosY, &pImage->Info.Size);

    switch(pImage->Compression)
    {
        case COMPX_COMPRESSION_NONE:
        {
            uint16_t* pData = (uint16_t*)pImage->Info.pPointer;

            for(uint32_t i = 0; i < Size; i++)
            {
                WriteData(*pData);
                pData++;
            }
        }
        break;

        case COMPX_RLE_16:  // Copy to LCD only
        {
            WriteRLE16((StaticImageRLE_16_t*)pImage->Info.pPointer, nullptr, Size);
        }
        break;

        // COMPX_RLE_32 // No 32 Bits on this LCD

        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyBackgroundToConstruction
//
//  Parameter(s):   ImageID_e ImageID           this is the source image for position ex
//
//  Return:         None
//
//  Description:    Put a pixel on LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::CopyBackgroundToConstruction(Cartesian_t Position)
{
    DisplayLayer*      pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    uint32_t           ConstructAlphaLayer = pLayer->GetAddress();
    uint32_t           BackgroundAddress = uint32_t(m_pBackground->Info.pPointer);

    m_ConstructPosition = Position;
    uint32_t Width      = pLayer->GetSize().X;
    uint32_t Height     = pLayer->GetSize().Y;

    DMA2D->CR      = DMA2D_M2M_PFC;                                                                                             // Memory-to-Memory with Pixel Format Conversion

    // Copy a portion of the background into a construction layer
    DMA2D->FGMAR   = BackgroundAddress + ((Position.Y * m_pBackground->Info.Size.Width) + Position.X) * sizeof(uint16_t);       // Source address
    DMA2D->FGOR    = (uint32_t)m_pBackground->Info.Size.Width - Width;                                                          // Source line offset so none as we are linear
    DMA2D->FGPFCCR = DMA2D_CONVERSION_RGB565;                                                                                   // Defines the size of pixel. 0 for format PIXEL_FORMAT_ARGB8888

    // Convert the portion of the background to ARGB8888
    DMA2D->OMAR    = ConstructAlphaLayer;                                                                                       // Destination address
    DMA2D->OOR     = 0;                                                                                                         // Destination line offset none as we are linear
    DMA2D->OPFCCR  = DMA2D_CONVERSION_ARGB8888;                                                                                 // Defines the size of pixel. 0 for format PIXEL_FORMAT_ARGB8888

    DMA2D->NLR     = (Width << 16) | Height;                                                                                    // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                                                                         // Start operation
    while ((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                                                                             // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                                                                                          // Clear flag
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyWidgetToDevice
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::CopyWidgetToDevice(ImageID_e Image, Cartesian_t Position)
{
    StaticImageInfo_t* pImage;

    DB_Central.Get(&pImage, GFX_IMAGE_INFO, uint16_t(Image));
    CopyWidgetToDevice(pImage->Info.Size, Position);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyWidgetToDevice
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::CopyWidgetToDevice(BoxSize_t BoxSize, Cartesian_t Position)
{
    DisplayLayer* pLayer   = &LayerTable[DisplayLayer::GetDrawing()];
    uint32_t      Address = pLayer->GetAddress();
    uint16_t*     pDataPtr;
    uint32_t      SizeX = BoxSize.Width;
    uint32_t      SizeY = BoxSize.Height;
    uint32_t      ImageSize = SizeX * SizeY;

    uint16_t* pImageDestination = (uint16_t*)pMemoryPool->Alloc(ImageSize * sizeof(uint16_t), MEM_DBG_GRAFX_CL1);

    DMA2D->CR      = DMA2D_M2M_PFC;                                 // Memory-to-Memory with Pixel Format Conversion
    DMA2D->FGMAR   = Address;                                       // Source address
    DMA2D->FGOR    = (uint32_t)pLayer->GetSize().X - SizeX;         // Source line offset so none as we are linear
    DMA2D->FGPFCCR = DMA2D_CONVERSION_ARGB8888;                     // Defines the size of pixel. 0 for format PIXEL_FORMAT_ARGB8888
    DMA2D->OMAR    = uint32_t(pImageDestination);                   // Destination address
    DMA2D->OOR     = 0;                                             // Destination line offset none as we are linear
    DMA2D->OPFCCR  = DMA2D_CONVERSION_RGB565;                       // Defines the size of pixel. 0 for format PIXEL_FORMAT_ARGB8888
    DMA2D->NLR     = (SizeX << 16) | SizeY;                         // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                             // Start operation
    while ((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                 // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                              // Clear flag

    SetWindow(Position.X, Position.Y, &BoxSize);
    pDataPtr = pImageDestination;

    for(uint32_t i = 0; i < ImageSize; i++)
    {
        WriteData(*pDataPtr);
        pDataPtr++;
    }

    pMemoryPool->Free((void**)&pImageDestination);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetRAM_Pointer
//
//  Parameter(s):   uint16_t   PosX       Specifies the horizontal GRAM address.
//                  uint16_t   PosY       Specifies the vertical   GRAM address.
//
//  Return:         None
//
//  Description:    Positions the internal GRAM write pointer of the LCD controller.
//                  The X address register is written first, followed by the RAM data
//                  register selection. The Y address register is then updated to
//                  complete the pointer setup before pixel
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SetRAM_Pointer(uint16_t PosX, uint16_t PosY)
{
	WriteCommand(SSD2119_Y_RAM_ADDRESS_REGISTER, PosY);
    WriteCommand(SSD2119_X_RAM_ADDRESS_REGISTER, PosX);
	SetWriteRAM_Ready();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadCommand
//
//  Parameter(s):   Register    8-bit register index to read from the LCD controller.
//
//  Return:         uint16_t    16-bit value returned by the LCD controller.
//
//  Description:    Sends the specified register index to the SSD2119, waits briefly for the bus
//                  to settle, and then reads the corresponding 16-bit data from the LCD RAM
//                  interface. This function performs a direct register read and assumes that the
//                  controller is already configured for indexed register access.
//
//-------------------------------------------------------------------------------------------------
uint16_t GrafxDriver::ReadCommand(uint8_t Register)
{
    LCD_REG = Register;
    LIB_Delay_uSec(3);
    return LCD_RAM;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetWindow
//
//  Parameter(s):
//
//  Return:
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SetWindow(uint16_t PosX, uint16_t PosY, BoxSize_t* pBoxSize)
{
    Box_t Box;
    Box.Pos.X       = PosX;
    Box.Pos.Y       = PosY;
    Box.Size.Width  = pBoxSize->Width;
    Box.Size.Height = pBoxSize->Height;
    SetWindow(&Box);
}

//-------------------------------------------------------------------------------------------------
//  Name:           SetWindow
//
//  Parameter(s):   pBox    Pointer to a Box_t structure defining the drawing region.
//                          - pBox->Pos.X / pBox->Pos.Y   : Top-left coordinate of the window
//                          - pBox->Size.Width / Height   : Dimensions of the window
//
//  Return:         None
//
//  Description:    Configure the SSD2119 GRAM access window to the rectangular region defined
//                  by pBox. This sets the horizontal and vertical RAM boundaries so that all
//                  subsequent pixel writes are clipped to this area. The GRAM cursor is then
//                  positioned at the top-left corner of the window and the controller is placed
//                  in write-ready state.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SetWindow(Box_t* pBox)
{
    uint16_t StartX = pBox->Pos.X;
    uint16_t EndX   = StartX + pBox->Size.Width  - 1;
    uint16_t StartY = pBox->Pos.Y;
    uint16_t EndY   = StartY + pBox->Size.Height - 1;
    WriteCommand(SSD2119_HORIZONTAL_RAM_START_REGISTER, StartX);        // Horizontal window (X) Start
    WriteCommand(SSD2119_HORIZONTAL_RAM_END_REGISTER,   EndX);          // Horizontal window (X) End
    uint16_t Vertical = (EndY << 8) | StartY;
    WriteCommand(SSD2119_VERTICAL_RAM_POSITION_REGISTER, Vertical);     // Vertical window (Y) packed into one register
    SetRAM_Pointer(StartX, StartY);                                     // Set GRAM cursor to top-left of window
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ResetWindow
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Restore the SSD2119 GRAM access window to the full display area. This resets
//                  both horizontal and vertical RAM boundaries to their maximum extents and
//                  positions the internal GRAM pointer at coordinate (0,0). Any subsequent pixel
//                  write will therefore start at the top-left corner of the display.
//
//-------------------------------------------------------------------------------------------------

void GrafxDriver::ResetWindow(void)
{
    WriteCommand(SSD2119_VERTICAL_RAM_POSITION_REGISTER, SSD2119_VERTICAL_WINDOWS_FULL_SIZE);
    WriteCommand(SSD2119_HORIZONTAL_RAM_START_REGISTER,  SSD2119_HORIZONTAL_WINDOWS_START_FULL_SIZE);
    WriteCommand(SSD2119_HORIZONTAL_RAM_END_REGISTER,    SSD2119_HORIZONTAL_WINDOWS_END_FULL_SIZE);
    SetRAM_Pointer(0, 0);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteRLE16
//
//  Parameter(s):   pData           Pointer to the RLE16 source data.
//                  pDestination    Pointer to the destination RGB565 buffer.
//                  Size            Number of RLE entries to decode.
//
//  Return:         None
//
//  Description:    Decode 16-bit RLE (RGB565) image data and write the expanded pixels either to
//                  the LCD interface (when pDestination is nullptr) or into a destination buffer.
//                  Each RLE entry contains a pixel value and a repeat count (Repeat + 1). The
//                  function expands all entries sequentially until the specified number of RLE
//                  items has been processed.
//
//  Note(s):        - When pDestination is nullptr, pixels are streamed directly to the LCD using
//                    WriteData(), which must accept RGB565 format.
//                  - The caller must ensure that the destination buffer is large enough to hold
//                    all expanded pixels.
//                  - No bounds checking is performed on the expanded output; RLE data must be
//                    valid and consistent with the expected image size.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteRLE16(StaticImageRLE_16_t* pData, uint16_t* pDestination, size_t Size)
{
    while(Size != 0)
    {
        uint32_t Repeat = pData->Repeat + 1;
        uint16_t Pixel  = pData->Pixel;
        pData++;

        if(pDestination == nullptr)
        {
            while(Repeat--)
            {
                WriteData(Pixel);
            }
        }
        else
        {
            while(Repeat--)
            {
                *pDestination = Pixel;
                pDestination++;
            }
        }

        Size--;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteRLE32
//
//  Parameter(s):   pData           Pointer to the RLE32 source data.
//                  pDestination    Pointer to the destination ARGB8888 buffer.
//                  Size            Number of RLE entries to decode.
//
//  Return:         None
//
//  Description:    Decode 32-bit RLE (ARGB8888) image data and write the expanded pixels into
//                  the destination buffer. Each RLE entry contains a pixel value and a repeat
//                  count (Repeat + 1). The function expands all entries sequentially until the
//                  specified number of RLE items has been processed.
//
//  Note(s):        - This LCD does not support ARGB8888, therefore no write to LCD memory.
//                  - The caller must ensure that the destination buffer is large enough to
//                    receive all expanded pixels.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteRLE32(StaticImageRLE_32_t* pData, uint32_t* pDestination, size_t Size)
{
    if(pDestination != nullptr)
    {
        while(Size != 0)
        {
            uint32_t Repeat = pData->Repeat + 1;
            uint32_t Pixel  = pData->Pixel;
            pData++;

            while(Repeat--)
            {
                *pDestination = Pixel;
                pDestination++;
            }

            Size--;
        }
    }
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
