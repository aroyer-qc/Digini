//-------------------------------------------------------------------------------------------------
//
//  File : lib_generic_spi_lcd_128X160 - ER_TFTM018_3 - ST7735.cpp
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
//
// Note(s)
//
//      This driver use LCD as foreground layer, and internal ram for construction layer.
//      RGB565 only because of RAM limitation on the STM32F4
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_ST7735_GLOBAL
#include "./lib_digini.h"
#undef  LIB_ST7735_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED) && (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

static const ST7735_InitCMD_t InitCMD[] =
{
    {ST7735_SWRESET,      {}, 0, 150},
    {ST7735_SLPOUT,       {}, 0, 500},
    {ST7735_FRMCTR1,      {0x01, 0x2C, 0x2D}, 3, 0},
    {ST7735_FRMCTR2,      {0x01, 0x2C, 0x2D}, 3, 0},
    {ST7735_FRMCTR3,      {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D}, 6, 0},
    {ST7735_INVCTR,       {0x07}, 1, 0},
    {ST7735_POWER_CTRL_1, {0xA2, 0x02, 0x84}, 3, 0},
    {ST7735_POWER_CTRL_2, {0xC5}, 1, 0},
    {ST7735_POWER_CTRL_3, {0x0A, 0x00}, 2, 0},
    {ST7735_POWER_CTRL_4, {0x8A, 0x2A}, 2, 0},
    {ST7735_POWER_CTRL_5, {0x8A, 0xEE}, 2, 0},
    {ST7735_VMCTR1,       {0x0E}, 1, 0},
    {ST7735_INVOFF,       {}, 0, 0},
    {ST7735_MADCTL,       {ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_BGR}, 1, 0},
    {ST7735_COLMOD,       {ST7735_PIXEL_FORMAT_RGB565}, 1, 0},
    {ST7735_CASET,        {0x00, GRAFX_DRIVER_OFFSET_X, 0x00, GRAFX_DRIVER_OFFSET_X + GRAFX_DRIVER_SIZE_X - 1}, 4, 0},
    {ST7735_RASET,        {0x00, GRAFX_DRIVER_OFFSET_Y, 0x00, GRAFX_DRIVER_OFFSET_Y + GRAFX_DRIVER_SIZE_Y - 1}, 4, 0},
    {ST7735_GMCTRP1,      {0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10}, 16, 0},
    {ST7735_GMCTRN1,      {0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10}, 16, 0},
    {ST7735_NORON,        {}, 0, 10}
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
//                  Resets the LCD controller and sends the ST7735R green-tab initialization
//                  sequence adapted from the Adafruit reference below.
//                  Once the controller is ready, the foreground display layer is cleared
//                  to a known state.
//
//  Note(s):        IO with those name must exist into bsp_io_def.h:
//                  IO_ST7735_CS, IO_ST7735_DC, IO_LCD_RESET, IO_ST7735_BACKLIGHT
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Initialize(const void* pArg)
{
    m_pSPI = (SPI_Driver*)pArg;
    m_pBackground = nullptr;
    m_ConstructPosition.X = 0;
    m_ConstructPosition.Y = 0;
    
    if(m_pSPI == nullptr)
    {
        return;
    }

    m_pSPI->Initialize();
    IO_SetPinLow(IO_ST7735_BACKLIGHT);
    IO_SetPinLow(IO_LCD_RESET);
    LIB_Delay_mSec(10);
    IO_SetPinHigh(IO_LCD_RESET);
    LIB_Delay_mSec(120);

    for(const ST7735_InitCMD_t& Command : InitCMD)
    {
        SendCommand(Command.Register, Command.Data, Command.Size, Command.Delay);
    }

    ClearLayer(FOREGROUND_DISPLAY_LAYER_0);
    DisplayOn();
    IO_SetPinHigh(IO_ST7735_BACKLIGHT);
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
    DisplayLayer* pLayer = &LayerTable[Layer];
    size_t Count = size_t(pLayer->GetSize().X) * pLayer->GetSize().Y;

    if(Layer == FOREGROUND_DISPLAY_LAYER_0)
    {
        m_pSPI->LockToDevice(IO_ST7735_CS);
        SetWindow(0, 0, GRAFX_DRIVER_SIZE_X - 1, GRAFX_DRIVER_SIZE_Y - 1);
        PutColor(0, Count);
        m_pSPI->UnlockFromDevice(IO_ST7735_CS);
    }
    else
    {
        uint16_t* pPixels = (uint16_t*)pLayer->GetAddress();
        if(pPixels != nullptr)
        {
            while(Count-- != 0)
            {
                *pPixels++ = 0;
            }
        }
    }
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
    DisplayLayer* pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    uint16_t* pPixels = (uint16_t*)pLayer->GetAddress();

     if((pPixels == nullptr) || (pLayer->GetPixelFormat() != PIXEL_FORMAT_RGB565) || (Position.X < 0) || (Position.Y < 0) || 
        (Position.X >= GRAFX_DRIVER_SIZE_X) || (Position.Y >= GRAFX_DRIVER_SIZE_Y) || (BoxSize.Width == 0) || (BoxSize.Height == 0))
    {
        return;
    }

    size_t Width = BoxSize.Width;
    size_t Height = BoxSize.Height;
    if(Width > size_t(pLayer->GetSize().X)) Width = pLayer->GetSize().X;
    if(Height > size_t(pLayer->GetSize().Y)) Height = pLayer->GetSize().Y;
    if(Width > size_t(GRAFX_DRIVER_SIZE_X - Position.X)) Width = GRAFX_DRIVER_SIZE_X - Position.X;
    if(Height > size_t(GRAFX_DRIVER_SIZE_Y - Position.Y)) Height = GRAFX_DRIVER_SIZE_Y - Position.Y;

    m_pSPI->LockToDevice(IO_ST7735_CS);
    SetWindow(Position.X, Position.Y, Position.X + Width - 1, Position.Y + Height - 1);
    uint8_t Data[GRAFX_DRIVER_SIZE_X * sizeof(uint16_t)];
    
    for(size_t Row = 0; Row < Height; Row++)
    {
        for(size_t Column = 0; Column < Width; Column++)
        {
            uint16_t Color = pPixels[Row * pLayer->GetSize().X + Column];
            Data[Column * 2] = uint8_t(Color >> 8);
            Data[Column * 2 + 1] = uint8_t(Color);
        }
        WriteData(Data, Width * sizeof(uint16_t));
    }

    m_pSPI->UnlockFromDevice(IO_ST7735_CS);
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
    SetWindow(StartX, StartY, EndX, EndY);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SetWindow
//
//   Parameter(s):  PosX1         X and ...
//                  PosY1         Y for top left corner of the window
//                  PosX2         X and ...
//                  PosY2         Y for bottom right corner of the window
//   Return value:  None
//
//   Description:   Set drawing window
//
//-------------------------------------------------------------------------------------------------

void GrafxDriver::SetWindow(uint8_t PosX1, uint8_t PosY1, uint8_t PosX2, uint8_t PosY2)
{
    uint8_t Data[4] = {0,0,0,0};
    Data[1] = PosX1 + GRAFX_DRIVER_OFFSET_X;                // X start
    Data[3] = PosX2 + GRAFX_DRIVER_OFFSET_X;                // X end
    WriteCommand(ST7735_CASET);
    WriteData(Data, sizeof(Data));
    Data[1] = PosY1 + GRAFX_DRIVER_OFFSET_Y;                // Y start
    Data[3] = PosY2 + GRAFX_DRIVER_OFFSET_Y;                // Y end
    WriteCommand(ST7735_RASET);
    WriteData(Data, sizeof(Data));
    WriteCommand(ST7735_RAM_WRITE);
    IO_SetPinHigh(IO_ST7735_DC);
}

void GrafxDriver::WriteCommand(uint8_t Register)
{
    IO_SetPinLow(IO_ST7735_DC);
    m_pSPI->Write(Register);
}

void GrafxDriver::WriteData(uint16_t Data)
{
    uint8_t Bytes[2] = {uint8_t(Data >> 8), uint8_t(Data)};
    WriteData(Bytes, sizeof(Bytes));
}

void GrafxDriver::WriteData(uint8_t* pData, size_t Size)
{
    IO_SetPinHigh(IO_ST7735_DC);
    m_pSPI->Write(pData, Size);
}

void GrafxDriver::SendCommand(uint8_t Register, const uint8_t* pData, size_t Size, uint32_t Delay)
{
    m_pSPI->LockToDevice(IO_ST7735_CS);
    WriteCommand(Register);

    if((pData != nullptr) && (Size != 0))
    {
        IO_SetPinHigh(IO_ST7735_DC);
        m_pSPI->Write(const_cast<uint8_t*>(pData), Size);
    }
    m_pSPI->UnlockFromDevice(IO_ST7735_CS);
    
if(Delay != 0)
    {
        LIB_Delay_mSec(Delay);
    }
}

void GrafxDriver::SendCommand(uint8_t Register, uint8_t Data, uint32_t Delay)
{
    SendCommand(Register, &Data, 1, Delay);
}

void GrafxDriver::LayerConfig(DisplayLayer* pLayer)
{
    VAR_UNUSED(pLayer);
}

void GrafxDriver::DrawPixel(uint16_t PosX, uint16_t PosY)
{
    Box_t Box;
    Box.Pos.X = PosX;
    Box.Pos.Y = PosY;
    Box.Size.Width = 1;
    Box.Size.Height = 1;
    DrawRectangle(&Box);
}

void GrafxDriver::DrawRectangle(Box_t* pBox)
{
    DisplayLayer* pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    
    if(pBox == nullptr)
    {
        return;
    }
    
    Box_t LocalBox = *pBox;

    if(DisplayLayer::GetDrawing() != FOREGROUND_DISPLAY_LAYER_0)
    {
        LocalBox.Pos.X -= m_ConstructPosition.X;
        LocalBox.Pos.Y -= m_ConstructPosition.Y;
    }

    pBox = &LocalBox;

    if((pBox->Pos.X < 0) || (pBox->Pos.Y < 0) || (pBox->Pos.X >= pLayer->GetSize().X) || (pBox->Pos.Y >= pLayer->GetSize().Y) ||
       (pBox->Size.Width == 0) || (pBox->Size.Height == 0))
    {
        return;
    }

    size_t Width = pBox->Size.Width;
    size_t Height = pBox->Size.Height;

    if(Width > size_t(pLayer->GetSize().X - pBox->Pos.X)) Width = pLayer->GetSize().X - pBox->Pos.X;
    if(Height > size_t(pLayer->GetSize().Y - pBox->Pos.Y)) Height = pLayer->GetSize().Y - pBox->Pos.Y;

    uint16_t Color = uint16_t(pLayer->GetColor());

    if(DisplayLayer::GetDrawing() == FOREGROUND_DISPLAY_LAYER_0)
    {
        m_pSPI->LockToDevice(IO_ST7735_CS);
        SetWindow(pBox->Pos.X, pBox->Pos.Y, pBox->Pos.X + Width - 1, pBox->Pos.Y + Height - 1);
        PutColor(Color, Width * Height);
        m_pSPI->UnlockFromDevice(IO_ST7735_CS);
    }
    else
    {
        uint16_t* pPixels = (uint16_t*)pLayer->GetAddress();
        if(pPixels == nullptr)
        {
            return;
        }

        for(size_t Row = 0; Row < Height; Row++)
        {
            for(size_t Column = 0; Column < Width; Column++)
            {
                pPixels[(pBox->Pos.Y + Row) * pLayer->GetSize().X + pBox->Pos.X + Column] = Color;
            }
        }
    }
}

void GrafxDriver::PutColor(uint16_t Color, size_t Count)
{
    constexpr size_t PixelsPerTransfer = 64;
    uint8_t Data[PixelsPerTransfer * sizeof(uint16_t)];
    for(size_t Pixel = 0; Pixel < PixelsPerTransfer; Pixel++)
    {
        Data[Pixel * 2] = uint8_t(Color >> 8);
        Data[Pixel * 2 + 1] = uint8_t(Color);
    }
    while(Count != 0)
    {
        size_t Pixels = (Count > PixelsPerTransfer) ? PixelsPerTransfer : Count;
        WriteData(Data, Pixels * sizeof(uint16_t));
        Count -= Pixels;
    }
}

void GrafxDriver::DrawLine(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction)
{
    Box_t Box;
    Box.Pos.X = PosX;
    Box.Pos.Y = PosY;
    Box.Size.Width = (Direction == DRAW_HORIZONTAL) ? Length : Thickness;
    Box.Size.Height = (Direction == DRAW_HORIZONTAL) ? Thickness : Length;
    DrawRectangle(&Box);
}

void GrafxDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t Thickness)
{
    if(Thickness > Width) Thickness = Width;
    if(Thickness > Height) Thickness = Height;
    if(Thickness == 0) return;
    DrawLine(PosX, PosY, Width, Thickness, DRAW_HORIZONTAL);
    DrawLine(PosX, PosY + Height - Thickness, Width, Thickness, DRAW_HORIZONTAL);
    DrawLine(PosX, PosY, Height, Thickness, DRAW_VERTICAL);
    DrawLine(PosX + Width - Thickness, PosY, Height, Thickness, DRAW_VERTICAL);
}

void GrafxDriver::DrawHLine(uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)
{
    GrafxGenDriver::DrawHLine(PosY, PosX1, PosX2, Thickness);
}

void GrafxDriver::DrawVLine(uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)
{
    GrafxGenDriver::DrawVLine(PosX, PosY1, PosY2, Thickness);
}

uint16_t GrafxDriver::ConvertColor(uint32_t Color)
{
    return uint16_t(((Color >> 8) & 0xF800) | ((Color >> 5) & 0x07E0) | ((Color >> 3) & 0x001F));
}

uint16_t GrafxDriver::BlendColor(uint16_t Source, uint16_t Background, uint8_t Alpha)
{
    uint32_t Inverse = 255 - Alpha;
    uint32_t Red = (((Source >> 11) * Alpha) + ((Background >> 11) * Inverse)) / 255;
    uint32_t Green = ((((Source >> 5) & 63) * Alpha) + (((Background >> 5) & 63) * Inverse)) / 255;
    uint32_t Blue = (((Source & 31) * Alpha) + ((Background & 31) * Inverse)) / 255;
    return uint16_t((Red << 11) | (Green << 5) | Blue);
}

void GrafxDriver::PutPixel(int32_t PosX, int32_t PosY, uint16_t Color, uint8_t Alpha)
{
    DisplayLayer* pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    if(DisplayLayer::GetDrawing() != FOREGROUND_DISPLAY_LAYER_0)
    {
        PosX -= m_ConstructPosition.X;
        PosY -= m_ConstructPosition.Y;
    }
    if((Alpha == 0) || (PosX < 0) || (PosY < 0) ||
       (PosX >= pLayer->GetSize().X) || (PosY >= pLayer->GetSize().Y)) return;

    if(DisplayLayer::GetDrawing() == FOREGROUND_DISPLAY_LAYER_0)
    {
        m_pSPI->LockToDevice(IO_ST7735_CS);
        SetWindow(PosX, PosY, PosX, PosY);
        WriteData(Color);
        m_pSPI->UnlockFromDevice(IO_ST7735_CS);
    }
    else
    {
        uint16_t* pPixels = (uint16_t*)pLayer->GetAddress();
        if(pPixels == nullptr) return;
        size_t Index = size_t(PosY) * pLayer->GetSize().X + PosX;
        pPixels[Index] = BlendColor(Color, pPixels[Index], Alpha);
    }
}

void GrafxDriver::BlockCopy(void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height,
                          uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    Box_t Box;
    Box.Pos.X = PosX;
    Box.Pos.Y = PosY;
    Box.Size.Width = Width;
    Box.Size.Height = Height;
    Cartesian_t Position;
    Position.X = DstX;
    Position.Y = DstY;
    BlockCopy(pSrc, &Box, &Position, SrcPixelFormat, BlendMode);
}

void GrafxDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    if((pSrc == nullptr) || (pBox == nullptr) || (pDstPos == nullptr) ||
       (pBox->Pos.X < 0) || (pBox->Pos.Y < 0) ||
       (pBox->Pos.X + pBox->Size.Width > GRAFX_DRIVER_SIZE_X) ||
       (pBox->Pos.Y + pBox->Size.Height > GRAFX_DRIVER_SIZE_Y)) return;

    for(size_t Row = 0; Row < pBox->Size.Height; Row++)
    {
        for(size_t Column = 0; Column < pBox->Size.Width; Column++)
        {
            size_t Index = (pBox->Pos.Y + Row) * GRAFX_DRIVER_SIZE_X + pBox->Pos.X + Column;
            uint16_t Color;
            uint8_t Alpha = 255;
            if(SrcPixelFormat == PIXEL_FORMAT_RGB565)
            {
                Color = ((uint16_t*)pSrc)[Index];
            }
          #if (GRAFX_COLOR_ARGB8888 == DEF_ENABLED)
            else if(SrcPixelFormat == PIXEL_FORMAT_ARGB8888)
            {
                uint32_t Pixel = ((uint32_t*)pSrc)[Index];
                Color = ConvertColor(Pixel);
                if(BlendMode == ALPHA_BLEND) Alpha = uint8_t(Pixel >> 24);
            }
          #endif
            else return;
            PutPixel(pDstPos->X + Column, pDstPos->Y + Row, Color, Alpha);
        }
    }
}

void GrafxDriver::PrintFont(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)
{
    if((pDescriptor == nullptr) || (pPos == nullptr) || (pDescriptor->pAddress == nullptr)) return;
    uint16_t Color = uint16_t(DisplayLayer::GetTextColor());
    for(size_t Row = 0; Row < pDescriptor->HeightPixel; Row++)
    {
        for(size_t Column = 0; Column < pDescriptor->WidthPixel; Column++)
        {
            size_t Index = Row * pDescriptor->WidthPixel + Column;
            if(Index >= pDescriptor->TotalSize) return;
            PutPixel(int32_t(pPos->X) + Column, int32_t(pPos->Y) + Row, Color, pDescriptor->pAddress[Index]);
        }
    }
}

void GrafxDriver::DrawImage(StaticImageInfo_t* pImage, Cartesian_t Position, BlendMode_e BlendMode)
{
    if((pImage == nullptr) || (pImage->Info.pPointer == nullptr) || (pImage->Info.Size.Width == 0)) return;
    size_t PixelCount = size_t(pImage->Info.Size.Width) * pImage->Info.Size.Height;
    size_t Output = 0;
    for(size_t Entry = 0; (Entry < pImage->RawSize) && (Output < PixelCount); Entry++)
    {
        size_t Repeat = 1;
        uint16_t Color;
        uint8_t Alpha = 255;
        if(pImage->Compression == COMPX_RLE_16)
        {
            StaticImageRLE_16_t* pData = (StaticImageRLE_16_t*)pImage->Info.pPointer;
            Repeat = size_t(pData[Entry].Repeat) + 1;
            Color = pData[Entry].Pixel;
        }
        else if(pImage->Compression == COMPX_RLE_32)
        {
            StaticImageRLE_32_t* pData = (StaticImageRLE_32_t*)pImage->Info.pPointer;
            Repeat = size_t(pData[Entry].Repeat) + 1;
            Color = ConvertColor(pData[Entry].Pixel);
            if(BlendMode == ALPHA_BLEND) Alpha = uint8_t(pData[Entry].Pixel >> 24);
        }
        else if(pImage->Compression == COMPX_COMPRESSION_NONE)
        {
            if(pImage->Info.PixelFormat == PIXEL_FORMAT_RGB565)
            {
                Color = ((uint16_t*)pImage->Info.pPointer)[Entry];
            }
          #if (GRAFX_COLOR_ARGB8888 == DEF_ENABLED)
            else if(pImage->Info.PixelFormat == PIXEL_FORMAT_ARGB8888)
            {
                uint32_t Pixel = ((uint32_t*)pImage->Info.pPointer)[Entry];
                Color = ConvertColor(Pixel);
                if(BlendMode == ALPHA_BLEND) Alpha = uint8_t(Pixel >> 24);
            }
          #endif
            else return;
        }
        else return;

        while((Repeat-- != 0) && (Output < PixelCount))
        {
            PutPixel(int32_t(Position.X) + int32_t(Output % pImage->Info.Size.Width),
                     int32_t(Position.Y) + int32_t(Output / pImage->Info.Size.Width), Color, Alpha);
            Output++;
        }
    }
}

void GrafxDriver::ImageCopy(ImageID_e Image, uint16_t PosX, uint16_t PosY)
{
    Cartesian_t Position;
    Position.X = PosX;
    Position.Y = PosY;
    BlendFromImage(Image, Position, NO_BLEND);
}

void GrafxDriver::BlendFromImage(ImageID_e Image, Cartesian_t Position, BlendMode_e BlendMode)
{
    if(Image == INVALID_IMAGE) return;
    StaticImageInfo_t* pImage = nullptr;
    DB_Central.Get(&pImage, GFX_IMAGE_INFO, uint16_t(Image));
    if(pImage == nullptr) return;
    if((DisplayLayer::GetDrawing() == FOREGROUND_DISPLAY_LAYER_0) && (Position.X == 0) && (Position.Y == 0) &&
       (pImage->Info.Size.Width == GRAFX_DRIVER_SIZE_X) && (pImage->Info.Size.Height == GRAFX_DRIVER_SIZE_Y))
    {
        m_pBackground = pImage;
    }
    DrawImage(pImage, Position, BlendMode);
}

void GrafxDriver::CopyBackgroundToConstruction(Cartesian_t Position)
{
    if(DisplayLayer::GetDrawing() == FOREGROUND_DISPLAY_LAYER_0) return;
    m_ConstructPosition = Position;
    ClearLayer(DisplayLayer::GetDrawing());
    if(m_pBackground != nullptr)
    {
        Cartesian_t Origin;
        Origin.X = 0;
        Origin.Y = 0;
        DrawImage(m_pBackground, Origin, NO_BLEND);
    }
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





















#if 0

//-------------------------------------------------------------------------------------------------
//
//  Function name:  Write color pixels
//
//  Parameter(s):   uint16_t Color
//  Return value:   uint16_t Count
//
//  Description:    write a number of pixel with a pecific color
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::PutColor(uint16_t Color, uint16_t Count)
{
    m_pSPI->LockToDevice(IO_ST7735_CS);
    WriteCommand(ST7735_RAM_WRITE);                         // Access to RAM
    IO_SetPinHigh(IO_ST7735_DC);                            // Set memory to data
    m_pSPI->OverrideMemoryIncrement();
    m_pSPI->Write((uint8_t*)&Color, Count);                 // Write color

#if 0
    while(Count--)                                          // Counter
    {
        m_pSPI->Write(Color);                               // Write color
    }
#endif

    m_pSPI->UnlockFromDevice(IO_ST7735_CS);
}

//-------------------------------------------------------------------------------------------------
//
//  Function name:  WriteCommand
//
//  Parameter(s):   uint8_t         Command
//  Return value:   None
//
//  Description:    Send a command to LCD
//
//  Note(s):        Any sub data will be sent using LCD_WriteByte
//                  This function does not handle the CS
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteCommand(uint8_t Command)
{
    IO_SetPinLow(IO_ST7735_DC);
    m_pSPI->Write(Command);
}

//-------------------------------------------------------------------------------------------------
//
//  Function name:   ReadCommand
//
//  Parameter(s):   uint8_t         Command
//  Return value:   uint8_t         Data
//
//  Description:    Send a read command to LCD
//
//-------------------------------------------------------------------------------------------------
uint8_t GrafxDriver::ReadCommand(uint8_t Command)
{
    uint8_t Data[2];

    IO_SetPinLow(IO_ST7735_DC);         // Command (active low)
    m_pSPI->Transfer(&Command, 1, &Data[0], 2, IO_ST7735_CS);
    return Data[1];
}

//-------------------------------------------------------------------------------------------------
//
//  Function name:  ReadData
//
//  Parameter(s):   None
//  Return value:   uint8_t         Data
//
//  Description:    Read one byte from LCD
//
//-------------------------------------------------------------------------------------------------
uint8_t GrafxDriver::ReadData(void)
{
    uint8_t Data;

    IO_SetPinLow(IO_ST7735_DC);         // Data (active high)
    m_pSPI->Transfer(nullptr, 0, &Data, 1, IO_ST7735_CS);
    return Data;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint8_t         Data
//   Return value:  None
//
//   Description:   Write 8 bits data to LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint8_t Data)
{
    //IO_SetPinHigh(IO_ST7735_DC);				// Data (active high)
    m_pSPI->Write(&Data, 1);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint16_t         Data
//   Return value:  None
//
//   Description:   Write 16 bits data to LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint16_t Data)
{
    //IO_SetPinHigh(IO_ST7735_DC);			            // Data (active high)
    m_pSPI->Write((uint8_t*)&Data, 2);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint32_t         Data
//   Return value:  None
//
//   Description:   Write 32 bits data to LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint32_t Data)
{
    //IO_SetPinHigh(IO_ST7735_DC);			            // Data (active high)
    m_pSPI->Write((uint8_t*)&Data, 4);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint8_t*        pData
//                  uint16_t        Size
//   Return value:  None
//
//   Description:   Write a stream of data LCD
//
//-------------------------------------------------------------------------------------------------

void GrafxDriver::WriteData(uint8_t* pData, uint32_t Size)
{
    //IO_SetPinHigh(IO_ST7735_DC);        			    // Data (active high)
    m_pSPI->Write(pData, Size);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SendCommand
//
//   Parameter(s):  uint8_t         Register        Register command
//                  uint8_t         Data            Data to send
//   Return value:  None
//
//   Description:   Send a command and write a uint8_t data LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SendCommand(uint8_t Register, uint8_t Data, uint32_t Delay)
{
    SendCommand(Register, &Data, 1, Delay);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SendCommand
//
//   Parameter(s):  uint8_t         Register        Register command
//                  uint8_t*        pData           Array to send
//                  uint32_t        Size            Size of the array
//   Return value:  None
//
//   Description:   Send a command and write a stream of data LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SendCommand(uint8_t Register, uint8_t* pData, uint32_t Size, uint32_t Delay)
{
    m_pSPI->LockToDevice(IO_ST7735_CS);
    WriteCommand(Register);             // Send register

    if(Size > 0)
    {
        WriteData(pData, Size);
    }

    m_pSPI->UnlockFromDevice(IO_ST7735_CS);

    if(Delay > 0)
    {
        LIB_Delay_mSec(Delay);
    }
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
void GrafxDriver::LayerConfig(DisplayLayer* pLayer)
{
    VAR_UNUSED(pLayer);         // This is a single layer LCD controller
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
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    VAR_UNUSED(BlendMode);
    VAR_UNUSED(SrcPixelFormat);         // ST7735 driver does not use SrcPixelFormat.. it should be 65K color

//    WriteCommand(SSD1779_COPY);
    WriteData(uint8_t(pBox->Pos.X));
    WriteData(uint8_t(pBox->Pos.Y));
    WriteData(uint8_t(pBox->Pos.X + pBox->Size.Width));
    WriteData(uint8_t(pBox->Pos.Y + pBox->Size.Height));
    WriteData(uint8_t(pDstPos->X));
    WriteData(uint8_t(pDstPos->Y));
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawRectangle
//
//   Parameter(s):  Box_t*    pBox
//    Return Value: none
//
//   Description:   Draw a rectangle
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawRectangle(Box_t* pBox)
{
    DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
	uint16_t Color  = (uint16_t(pLayer->GetColor()));
    SetWindow(pBox->Pos.X, pBox->Pos.Y, pBox->Pos.X + (pBox->Size.Width - 1), pBox->Pos.Y + (pBox->Size.Height - 1));
    PutColor(Color, pBox->Size.Width * pBox->Size.Height);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawBox
//
//  Parameter(s):   uint16_t    PosX
//                  uint16_t    PosY
//                  uint16_t    Width
//                  uint16_t    Height
//                  uint16_t    Thickness
//  Return:         None
//
//  Description:    Draw a box in a specific Thickness
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t Thickness)
{
    DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
	uint16_t Color  = (uint16_t(pLayer->GetColor()));

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint16_t X2 = PosX + Width;
        uint16_t Y2 = PosY + Height;

        DrawVLine(PosX,           PosY, Y2, Thickness);
        DrawVLine(X2 - Thickness, PosY, Y2, Thickness);
        DrawHLine(PosY,           PosX, X2, Thickness);
        DrawHLine(Y2 - Thickness, PosX, X2, Thickness);
    }
    else
    {
        // Left Side
        SetWindow(PosX, PosY, PosX + (Thickness - 1), PosY + Height);
        PutColor(Color, Width * Height);

        // Right Side
        SetWindow(PosX + (Width - (Thickness - 1)), PosY, PosX + Width, PosY + Height);
        PutColor(Color, Width * Height);

        // Top
        SetWindow(PosX + Thickness, PosY, PosX + (Width - Thickness), PosY + (Thickness - 1));
        PutColor(Color, Width * Height);

        // Bottom
        SetWindow(PosX + Thickness, PosY + (Height - (Thickness - 1)), PosX + (Width - Thickness), PosY + Height);
        PutColor(Color, Width * Height);
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
    DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    uint16_t Color  = (uint16_t(pLayer->GetColor()));

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint16_t* pAddress = (uint16_t*)pLayer->GetAddress() + (((PosY * GRAFX_DRIVER_SIZE_X) + PosX) * 2);
        *pAddress = Color;
    }
    else
    {
        SetWindow(PosX, PosY, PosX, PosY);
        PutColor(Color, 1);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawHLine
//
//  Parameter(s):   uint16_t    Y
//                  uint16_t    X1
//                  uint16_t    X2
//                  uint16_t    Thickness
//  Return:         None
//
//  Description:    Displays a horizontal line of a specific Thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawHLine(uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)
{
    uint16_t Temp;
	uint16_t Length;

    // X1 need to be the lowest
    if(PosX1 > PosX2)
    {
        Temp  = PosX1;
        PosX1 = PosX2;
        PosX2 = Temp;
    }

    Length = (PosX2 - PosX1) + 1;

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        this->DrawLine(PosX1, PosY, Length, Thickness, DRAW_HORIZONTAL);
    }
    else
    {
        DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
        uint16_t Color  = (uint16_t(pLayer->GetColor()));
        SetWindow(PosX1, PosY, PosX2, PosY + (Thickness - 1));
        PutColor(Color, Length * Thickness);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawVLine
//
//  Parameter(s):   uint16_t    wPosX
//                  uint16_t    wPosY1
//                  uint16_t    wPosY2
//                  uint16_t    wThickness
//  Return:         None
//
//  Description:    Displays a vertical line of a specific Thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawVLine(uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)
{
    uint16_t Temp;
	uint16_t Length;

    // Y1 need to be the lowest
    if(PosY1 > PosY2)
    {
        Temp  = PosY1;
        PosY1 = PosY2;
		PosY2 = Temp;
    }

    Length = PosY2 - PosY1;

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        this->DrawLine(PosX, PosY1, Length, Thickness, DRAW_VERTICAL);
    }
    else
    {
        DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
        uint16_t Color  = (uint16_t(pLayer->GetColor()));
        SetWindow(PosX, PosY1, PosX + (Thickness - 1), PosY2);
        PutColor(Color, Length * Thickness);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintFont
//
//  Parameter(s):   FONT_sDescriptor*   pDescriptor
//                  Cartesian_t*        pPos
//  Return:         none
//
//  Description:    This function will print a font to drawing layer with the drawing color
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::PrintFont(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)
{
    uint8_t            PixelSize;
    uint32_t           Address;
    DisplayLayer*            pLayer;
    uint32_t           PixelFormat;
    struct32_t         AreaConfig;

    pLayer             = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat        = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize          = pLayer->GetPixelSize();
    Address            = pLayer->GetAddress() + (((pPos->Y * GRAFX_DRIVER_SIZE_X) + pPos->X) * (uint32_t)PixelSize);
    AreaConfig.u_16.u1 = pDescriptor->Size.Width;
    AreaConfig.u_16.u0 = pDescriptor->Size.Height;

    // TODO provide a method without DMA2D
    // use memory to memory normal DMA
    VAR_UNUSED(Address);
    VAR_UNUSED(PixelFormat);
    VAR_UNUSED(AreaConfig);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Clear
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear and home the LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Clear(void)
{
	SetWindow(0, 0, GRAFX_DRIVER_SIZE_X, GRAFX_DRIVER_SIZE_Y);      // Use the whole LCD
	PutColor(RED, GRAFX_RAM_SIZE_X * GRAFX_RAM_SIZE_Y); 	        // Draw individual pixels
}

//-------------------------------------------------------------------------------------------------

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_ST77xx::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m % 4; // can't be higher than 3

  switch (rotation) {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 1:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  case 2:
    madctl = ST77XX_MADCTL_RGB;
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 3:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}


/**
 * @desc    Set partial area / window
 *
 * @param   uint8_t x - start row
 * @param   uint8_t x - end row
 *
 * @return  uint8_t
 */
uint8_t ST7735_SetPartialArea(uint8_t sRow, uint8_t eRow)
{
  // check if coordinates is out of range
  if ((sRow > SIZE_Y) ||
      (eRow > SIZE_Y)) {
    // out of range
    return 0;
  }
  // column address set
  ST7735_CommandSend(PTLAR);
  // start start Row
  ST7735_Data8BitsSend(0x00);
  // start start Row
  ST7735_Data8BitsSend(sRow);
  // row end Row
  ST7735_Data8BitsSend(0x00);
  // end end Row
  ST7735_Data8BitsSend(eRow);
  // column address set
  ST7735_CommandSend(PTLON);
  // success
  return 1;
}

/**
 * @desc    Set text position x, y
 *
 * @param   uint8_t x - position
 * @param   uint8_t y - position
 *
 * @return  void
 */
char ST7735_SetPosition(uint8_t x, uint8_t y)
{
  // check if coordinates is out of range
  if ((x > MAX_X) && (y > MAX_Y)) {
    // error
    return ST7735_ERROR;

  } else if ((x > MAX_X) && (y <= MAX_Y)) {
    // set position y
    cacheMemIndexRow = y;
    // set position x
    cacheMemIndexCol = 2;
  } else {
    // set position y
    cacheMemIndexRow = y;
    // set position x
    cacheMemIndexCol = x;
  }
  // success
  return ST7735_SUCCESS;
}

/**
 * @desc    Check text position x, y
 *
 * @param   unsigned char x - position
 * @param   unsigned char y - position
 * @param   unsigned char
 *
 * @return  char
 */
char ST7735_CheckPosition(unsigned char x, unsigned char y, unsigned char max_y, ESizes size)
{
  // check if coordinates is out of range
  if ((x > MAX_X) && (y > max_y)) {
    // out of range
    return ST7735_ERROR;

  }
  // if next line
  if ((x > MAX_X) && (y <= max_y)) {
    // set position y
    cacheMemIndexRow = y;
    // set position x
    cacheMemIndexCol = 2;
  }

  // success
  return ST7735_SUCCESS;
}

#endif


















#if 0 //7735B

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SWRESET);                               // Software reset
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(50);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SLPOUT);                                // Out Of Sleep
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_COLMOD);                                // Set color mode
    WriteData(uint8_t(0x05));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_FRMCTR1);           // Frame rate control
    WriteData(uint8_t(0x00));               //     fastest refresh
    WriteData(uint8_t(0x06));               //     6 lines front porch
    WriteData(uint8_t(0x03));               //     3 lines back porch
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

      // D7  D6  D5  D4  D3  D2  D1  D0
      // MY  MX  MV  ML RGB  MH   -   -
      // ------------------------------
      // ------------------------------
      // MV  MX  MY -> {MV (row / column exchange) MX (column address order), MY (row address order)}
      // ------------------------------
      //  0   0   0 -> begin left-up corner, end right-down corner left-right (normal view)
      //  0   0   1 -> begin left-down corner, end right-up corner left-right (Y-mirror)
      //  0   1   0 -> begin right-up corner, end left-down corner right-left (X-mirror)
      //  0   1   1 -> begin right-down corner, end left-up corner right-left (X-mirror, Y-mirror)
      //  1   0   0 -> begin left-up corner, end right-down corner up-down (X-Y exchange)
      //  1   0   1 -> begin left-down corner, end right-up corner down-up (X-Y exchange, Y-mirror)
      //  1   1   0 -> begin right-up corner, end left-down corner up-down (X-Y exchange, X-mirror)
      //  1   1   1 -> begin right-down corner, end left-up corner
      //               down-up (X-Y exchange, X-mirror, Y-mirror)
      // ------------------------------
      //  ML: vertical refresh order
      //      0 -> refresh top to bottom
      //      1 -> refresh bottom to top
      // ------------------------------
      // RGB: filter panel
      //      0 -> RGB
      //      1 -> BGR
      // ------------------------------
      //  MH: horizontal refresh order
      //      0 -> refresh left to right
      //      1 -> refresh right to left
      // 0xA0 = 1010 0000
IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_MADCTL);                                // Set memory data access control
    WriteData(uint8_t(0x08));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low


IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_DISSET5);
    WriteData(uint8_t(0x15));
    WriteData(uint8_t(0x02));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_INVCTR);
  WriteData(uint8_t(0x00));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_1);
  WriteData(uint8_t(0x02));
  WriteData(uint8_t(0x70));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_2);
  WriteData(uint8_t(0x05));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_3);
  WriteData(uint8_t(0x01));
  WriteData(uint8_t(0x02));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_VMCTR1);
  WriteData(uint8_t(0x10));
  //WriteData(uint8_t(0x3C));
  //WriteData(uint8_t(0x38));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_6);
  WriteData(uint8_t(0x11));
  WriteData(uint8_t(0x15));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRP1);
  WriteData(uint8_t(0x09)); WriteData(uint8_t(0x16)); WriteData(uint8_t(0x09)); WriteData(uint8_t(0x20));
  WriteData(uint8_t(0x21)); WriteData(uint8_t(0x1B)); WriteData(uint8_t(0x13)); WriteData(uint8_t(0x19));
  WriteData(uint8_t(0x17)); WriteData(uint8_t(0x15)); WriteData(uint8_t(0x1E)); WriteData(uint8_t(0x2B));
  WriteData(uint8_t(0x04)); WriteData(uint8_t(0x05)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x0E));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRN1);
  WriteData(uint8_t(0x0B)); WriteData(uint8_t(0x14)); WriteData(uint8_t(0x08)); WriteData(uint8_t(0x1E));
  WriteData(uint8_t(0x22)); WriteData(uint8_t(0x1D)); WriteData(uint8_t(0x18)); WriteData(uint8_t(0x1E));
  WriteData(uint8_t(0x1B)); WriteData(uint8_t(0x1A)); WriteData(uint8_t(0x24)); WriteData(uint8_t(0x2B));
  WriteData(uint8_t(0x06)); WriteData(uint8_t(0x06)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x0F));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_CASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0x81));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_RASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0x81));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_NORON);
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_DISPON);                                // Main screen turn on
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

#endif


#if 0 //7735R
static uint8_t Test[4];

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SWRESET);                           // Software reset
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(50);


//IO_SetPinLow(IO_ST7735_CS);
//WriteCommand(ST7735_RDID_1);
//Test[0] = ReadData();
//IO_SetPinHigh(IO_ST7735_CS);
/*
IO_SetPinLow(IO_ST7735_CS);
WriteCommand(ST7735_RDID_2);
Test[1] = ReadData();
IO_SetPinHigh(IO_ST7735_CS);

IO_SetPinLow(IO_ST7735_CS);
WriteCommand(ST7735_RDID_3);
Test[2] = ReadData();
IO_SetPinHigh(IO_ST7735_CS);

IO_SetPinLow(IO_ST7735_CS);
WriteCommand(ST7735_RDID_4);
Test[3] = ReadData();
IO_SetPinHigh(IO_ST7735_CS);
*/


//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SLPOUT);                                // Out Of Sleep
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_COLMOD);                                // Set color mode
    WriteData(uint8_t(0x05));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_FRMCTR1);           // Frame rate control - normal Rate = fosc/(1x2+40) * (LINE+2C+2D)
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    WriteData(uint8_t(0x01));
    WriteData(uint8_t(0x2C));
    WriteData(uint8_t(0x2D));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_FRMCTR2);           // Frame rate control - idle mode = fosc/(1x2+40) * (LINE+2C+2D)
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    WriteData(uint8_t(0x01));
    WriteData(uint8_t(0x2C));
    WriteData(uint8_t(0x2D));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
   WriteCommand(ST7735_FRMCTR3);           // Frame rate control - partial mode = fosc/(1x2+40) * (LINE+2C+2D)
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    WriteData(uint8_t(0x01));
    WriteData(uint8_t(0x2C));
    WriteData(uint8_t(0x2D));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_INVCTR);
  WriteData(uint8_t(0x03));
//  WriteData(uint8_t(0x07));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_1);
  WriteData(uint8_t(0x28));
  WriteData(uint8_t(0x08));
  WriteData(uint8_t(0x04));
//  WriteData(uint8_t(0xA2));
//  WriteData(uint8_t(0x02));
//  WriteData(uint8_t(0x84));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_2);
  WriteData(uint8_t(0xC0));
//  WriteData(uint8_t(0xC5));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_3);
//  WriteData(uint16_t(0x0D00));
  WriteData(uint16_t(0x0A00));
//  WriteData(uint8_t(0x0A));
//  WriteData(uint8_t(0x00));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_4);
//  WriteData(uint16_t(0x2A8D));
  WriteData(uint16_t(0x2A8A));
//  WriteData(uint8_t(0x8A));
//  WriteData(uint8_t(0x2A));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_5);
  //WriteData(uint16_t(0xEE8D));
  WriteData(uint16_t(0xEE8A));
//  WriteData(uint8_t(0x8A));
//  WriteData(uint8_t(0xEE));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_VMCTR1);
  WriteData(uint8_t(0x0E));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_INVOFF);
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

      // D7  D6  D5  D4  D3  D2  D1  D0
      // MY  MX  MV  ML RGB  MH   -   -
      // ------------------------------
      // ------------------------------
      // MV  MX  MY -> {MV (row / column exchange) MX (column address order), MY (row address order)}
      // ------------------------------
      //  0   0   0 -> begin left-up corner, end right-down corner left-right (normal view)
      //  0   0   1 -> begin left-down corner, end right-up corner left-right (Y-mirror)
      //  0   1   0 -> begin right-up corner, end left-down corner right-left (X-mirror)
      //  0   1   1 -> begin right-down corner, end left-up corner right-left (X-mirror, Y-mirror)
      //  1   0   0 -> begin left-up corner, end right-down corner up-down (X-Y exchange)
      //  1   0   1 -> begin left-down corner, end right-up corner down-up (X-Y exchange, Y-mirror)
      //  1   1   0 -> begin right-up corner, end left-down corner up-down (X-Y exchange, X-mirror)
      //  1   1   1 -> begin right-down corner, end left-up corner
      //               down-up (X-Y exchange, X-mirror, Y-mirror)
      // ------------------------------
      //  ML: vertical refresh order
      //      0 -> refresh top to bottom
      //      1 -> refresh bottom to top
      // ------------------------------
      // RGB: filter panel
      //      0 -> RGB
      //      1 -> BGR
      // ------------------------------
      //  MH: horizontal refresh order
      //      0 -> refresh left to right
      //      1 -> refresh right to left
      // 0xA0 = 1010 0000
//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_MADCTL);                                // Set memory data access control
    //WriteData(uint8_t(0xC0));
    WriteData(uint8_t(0xC8));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_COLMOD);
    WriteData(uint8_t(0x05));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_CASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0x81));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_RASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x01)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0xA0));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRP1);
  //WriteData(uint32_t(0x0A072204));
  //WriteData(uint32_t(0x2A25302E));
  //WriteData(uint32_t(0x3A2E2628));
  //WriteData(uint32_t(0x13030100));
  WriteData(uint32_t(0x12071C02));
  WriteData(uint32_t(0x2D293237));
  WriteData(uint32_t(0x392B2529));
  WriteData(uint32_t(0x10030100));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRN1);
  //WriteData(uint32_t(0x0D061604));
  //WriteData(uint32_t(0x2723262D));
  //WriteData(uint32_t(0x3B2D2527));
  //WriteData(uint32_t(0x13040100));
  WriteData(uint32_t(0x06071D03));
  WriteData(uint32_t(0x2D292C2E));
  WriteData(uint32_t(0x3F372E2E));
  WriteData(uint32_t(0x10020000));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_NORON);
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_DISPON);                                // Main screen turn on
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

#endif




#if 0 // adafruit

#include "Adafruit_ST7735.h"
#include "Adafruit_ST77xx.h"

// CONSTRUCTORS ************************************************************

/*!
    @brief  Instantiate Adafruit ST7735 driver with software SPI
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  mosi  SPI MOSI pin #
    @param  sclk  SPI Clock pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
Adafruit_ST7735::Adafruit_ST7735(int16_t cs, int16_t dc, int16_t mosi,
                                 int16_t sclk, int16_t rst)
    : Adafruit_ST77xx(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, cs, dc, mosi,
                      sclk, rst) {}

/*!
    @brief  Instantiate Adafruit ST7735 driver with default hardware SPI
    @param  cs   Chip select pin #
    @param  dc   Data/Command pin #
    @param  rst  Reset pin # (optional, pass -1 if unused)
*/
Adafruit_ST7735::Adafruit_ST7735(int16_t cs, int16_t dc, int16_t rst)
    : Adafruit_ST77xx(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, cs, dc, rst) {}

#if !defined(ESP8266)
/*!
    @brief  Instantiate Adafruit ST7735 driver with selectable hardware SPI
    @param  spiClass  Pointer to an SPI device to use (e.g. &SPI1)
    @param  cs        Chip select pin #
    @param  dc        Data/Command pin #
    @param  rst       Reset pin # (optional, pass -1 if unused)
*/
Adafruit_ST7735::Adafruit_ST7735(SPIClass *spiClass, int16_t cs, int16_t dc,
                                 int16_t rst)
    : Adafruit_ST77xx(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, spiClass, cs,
                      dc, rst) {}
#endif // end !ESP8266

// SCREEN INITIALIZATION ***************************************************

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.

// clang-format off
static const uint8_t PROGMEM
  Bcmd[] = {                        // Init commands for 7735B screens
    18,                             // 18 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      50,                           //     50 ms delay
    ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, no args, w/delay
      255,                          //     255 = max (500 ms) delay
    ST77XX_COLMOD,  1+ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      0x05,                         //     16-bit color
      10,                           //     10 ms delay
    ST7735_FRMCTR1, 3+ST_CMD_DELAY, //  4: Frame rate control, 3 args + delay:
      0x00,                         //     fastest refresh
      0x06,                         //     6 lines front porch
      0x03,                         //     3 lines back porch
      10,                           //     10 ms delay
    ST77XX_MADCTL,  1,              //  5: Mem access ctl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST7735_DISSET5, 2,              //  6: Display settings #5, 2 args:
      0x15,                         //     1 clk cycle nonoverlap, 2 cycle gate
                                    //     rise, 3 cycle osc equalize
      0x02,                         //     Fix on VTL
    ST7735_INVCTR,  1,              //  7: Display inversion control, 1 arg:
      0x0,                          //     Line inversion
    ST7735_PWCTR1,  2+ST_CMD_DELAY, //  8: Power control, 2 args + delay:
      0x02,                         //     GVDD = 4.7V
      0x70,                         //     1.0uA
      10,                           //     10 ms delay
    ST7735_PWCTR2,  1,              //  9: Power control, 1 arg, no delay:
      0x05,                         //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3,  2,              // 10: Power control, 2 args, no delay:
      0x01,                         //     Opamp current small
      0x02,                         //     Boost frequency
    ST7735_VMCTR1,  2+ST_CMD_DELAY, // 11: Power control, 2 args + delay:
      0x3C,                         //     VCOMH = 4V
      0x38,                         //     VCOML = -1.1V
      10,                           //     10 ms delay
    ST7735_PWCTR6,  2,              // 12: Power control, 2 args, no delay:
      0x11, 0x15,
    ST7735_GMCTRP1,16,              // 13: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x09, 0x16, 0x09, 0x20,       //     (Not entirely necessary, but provides
      0x21, 0x1B, 0x13, 0x19,       //      accurate colors)
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1,16+ST_CMD_DELAY, // 14: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x0B, 0x14, 0x08, 0x1E,       //     (Not entirely necessary, but provides
      0x22, 0x1D, 0x18, 0x1E,       //      accurate colors)
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,                           //     10 ms delay
    ST77XX_CASET,   4,              // 15: Column addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 2
      0x00, 0x81,                   //     XEND = 129
    ST77XX_RASET,   4,              // 16: Row addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 1
      0x00, 0x81,                   //     XEND = 160
    ST77XX_NORON,     ST_CMD_DELAY, // 17: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON,    ST_CMD_DELAY, // 18: Main screen turn on, no args, delay
      255 },                        //     255 = max (500 ms) delay

  Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
    15,                             // 15 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
      150,                          //     150 ms delay
    ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
      255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
      0x0E,
    ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
    ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
      0x05 },                       //     16-bit color

  Rcmd2green[] = {                  // 7735R init, part 2 (green tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 0
      0x00, 0x7F+0x02,              //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,                   //     XSTART = 0
      0x00, 0x9F+0x01 },            //     XEND = 159

  Rcmd2red[] = {                    // 7735R init, part 2 (red tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd2green144[] = {               // 7735R init, part 2 (green 1.44 tab)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F },                 //     XEND = 127

  Rcmd2green160x80[] = {            // 7735R init, part 2 (mini 160x80)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x4F,                   //     XEND = 79
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd2green160x80plugin[] = {      // 7735R init, part 2 (mini 160x80 with plugin FPC)
    3,                              //  3 commands in list:
    ST77XX_INVON,  0,              //   1: Display is inverted
    ST77XX_CASET,   4,              //  2: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x4F,                   //     XEND = 79
    ST77XX_RASET,   4,              //  3: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd3[] = {                       // 7735R init, part 3 (red or green tab)
    4,                              //  4 commands in list:
    ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
      0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
      0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
      100 };                        //     100 ms delay

// clang-format on

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7735B displays
*/
/**************************************************************************/
void Adafruit_ST7735::initB(void) {
  commonInit(Bcmd);
  setRotation(0);
}

/**************************************************************************/
/*!
    @brief  Initialization code common to all ST7735R displays
    @param  options  Tab color from adafruit purchase
*/
/**************************************************************************/
void Adafruit_ST7735::initR(uint8_t options) {
  commonInit(Rcmd1);
  if (options == INITR_GREENTAB) {
    displayInit(Rcmd2green);
    _colstart = 2;
    _rowstart = 1;
  } else if ((options == INITR_144GREENTAB) || (options == INITR_HALLOWING)) {
    _height = ST7735_TFTHEIGHT_128;
    _width = ST7735_TFTWIDTH_128;
    displayInit(Rcmd2green144);
    _colstart = 2;
    _rowstart = 3; // For default rotation 0
  } else if (options == INITR_MINI160x80) {
    _height = ST7735_TFTWIDTH_80;
    _width = ST7735_TFTHEIGHT_160;
    displayInit(Rcmd2green160x80);
    _colstart = 24;
    _rowstart = 0;
  } else if (options == INITR_MINI160x80_PLUGIN) {
    _height = ST7735_TFTWIDTH_80;
    _width = ST7735_TFTHEIGHT_160;
    displayInit(Rcmd2green160x80plugin);
    _colstart = 26;
    _rowstart = 1;
    invertOnCommand = ST77XX_INVOFF;
    invertOffCommand = ST77XX_INVON;
  } else {
    // colstart, rowstart left at default '0' values
    displayInit(Rcmd2red);
  }
  displayInit(Rcmd3);

  // Black tab, change MADCTL color filter
  if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80)) {
    uint8_t data = 0xC0;
    sendCommand(ST77XX_MADCTL, &data, 1);
  }

  if (options == INITR_HALLOWING) {
    // Hallowing is simply a 1.44" green tab upside-down:
    tabcolor = INITR_144GREENTAB;
    setRotation(2);
  } else {
    tabcolor = options;
    setRotation(0);
  }
}

// OTHER FUNCTIONS *********************************************************

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_ST7735::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  // For ST7735 with GREEN TAB (including HalloWing)...
  if ((tabcolor == INITR_144GREENTAB) || (tabcolor == INITR_HALLOWING)) {
    // ..._rowstart is 3 for rotations 0&1, 1 for rotations 2&3
    _rowstart = (rotation < 2) ? 3 : 1;
  }

  switch (rotation) {
  case 0:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    } else {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 1:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    } else {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  case 2:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_RGB;
    } else {
      madctl = ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    } else {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 3:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }

    if (tabcolor == INITR_144GREENTAB) {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 ||
               tabcolor == INITR_MINI160x80_PLUGIN) {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    } else {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}

#endif