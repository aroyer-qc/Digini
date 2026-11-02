//-------------------------------------------------------------------------------------------------
//
//  File : lib_generic_spi_lcd_128X160 - ER_TFTM018_3 - ST7735.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define GRAFX_NUMBER_OF_ACTIVE_LAYER                2

#define GRAFX_DRIVER_USE_V_SYNC                     DEF_DISABLED

// Display size
#define GRAFX_DRIVER_SIZE_X                         128
#define GRAFX_DRIVER_SIZE_Y                         160

#define GRAFX_DRIVER_OFFSET_X                       2
#define GRAFX_DRIVER_OFFSET_Y                       1

#define GRAFX_RAM_SIZE_X                            132
#define GRAFX_RAM_SIZE_Y                            162


//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

enum ST7735_Command_e : uint8_t
{
    ST7735_NOP          = 0x00,
    ST7735_SWRESET      = 0x01,
    ST7735_RDDID        = 0x04,
    ST7735_RDDST        = 0x09,
    ST7735_SLPIN        = 0x10,
    ST7735_SLPOUT       = 0x11,
    ST7735_PTLON        = 0x12,
    ST7735_NORON        = 0x13,
    ST7735_INVOFF       = 0x20,
    ST7735_INVON        = 0x21,
    ST7735_DISPOFF      = 0x28,
    ST7735_DISPON       = 0x29,
    ST7735_CASET        = 0x2A,
    ST7735_RASET        = 0x2B,
    ST7735_RAM_WRITE    = 0x2C,
    ST7735_RAMRD        = 0x2E,
    ST7735_PTLAR        = 0x30,
    ST7735_MADCTL       = 0x36,
    ST7735_COLMOD       = 0x3A,
    ST7735_FRMCTR1      = 0xB1,
    ST7735_FRMCTR2      = 0xB2,
    ST7735_FRMCTR3      = 0xB3,
    ST7735_INVCTR       = 0xB4,
    ST7735_DISSET5      = 0xB6,
    ST7735_POWER_CTRL_1 = 0xC0,
    ST7735_POWER_CTRL_2 = 0xC1,
    ST7735_POWER_CTRL_3 = 0xC2,
    ST7735_POWER_CTRL_4 = 0xC3,
    ST7735_POWER_CTRL_5 = 0xC4,
    ST7735_VMCTR1       = 0xC5,
    ST7735_RDID_1       = 0xDA,
    ST7735_RDID_2       = 0xDB,
    ST7735_RDID_3       = 0xDC,
    ST7735_RDID_4       = 0xDD,
    ST7735_GMCTRP1      = 0xE0,
    ST7735_GMCTRN1      = 0xE1,
    ST7735_POWER_CTRL_6 = 0xFC
};

enum ST7735_Parameter_e : uint8_t
{
    ST7735_PIXEL_FORMAT_RGB565 = 0x05,
    ST7735_MADCTL_RGB = 0x00,
    ST7735_MADCTL_BGR = 0x08,
    ST7735_MADCTL_MX = 0x40,
    ST7735_MADCTL_MY = 0x80
};

// Colors
//#define BLACK                           0x0000
//#define WHITE                           0xFFFF
//#define RED                             0xF800
//#define BLUE                            0x001F
//#define GREEN                           0x07E0

#define ST7735_SUCCESS                  0
#define ST7735_ERROR                    1

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

struct ST7735_InitCMD_t
{
    uint8_t Register;
    uint8_t Data[16];
    size_t Size;
    uint32_t Delay;
};

class GrafxDriver : public GrafxGenDriver
{
    public:

        using GrafxGenDriver::DrawRectangle;

        void        Initialize      		(const void* pArg)                                                                                      override;
        void        DisplayOn       		(void)                 override         { SendCommand(ST7735_DISPON, nullptr, 0, 100); }
        void        DisplayOff      		(void)                 override         { SendCommand(ST7735_DISPOFF, nullptr, 0, 1); }

        void        ClearLayer              (Layer_e Layer) override;
        void        CopyWidgetToDevice      (ImageID_e Image, Cartesian_t Position) override;
        void        CopyWidgetToDevice      (BoxSize_t BoxSize, Cartesian_t Position) override;
        void        CopyBackgroundToConstruction(Cartesian_t Position) override;
        void        ImageCopy               (ImageID_e Image, uint16_t PosX, uint16_t PosY);
        void        BlendFromImage          (ImageID_e Image, Cartesian_t Position, BlendMode_e BlendMode) override;

        void        LayerConfig     		(DisplayLayer* pLayer)                                                                                  override;

        void        BlockCopy       		(void* pSrc, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height,
											 uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)                     override;
        void        BlockCopy       		(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)    override;
        void        DrawBox         		(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t Thickness)                     override;
        void        DrawHLine       		(uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)                                     override;
        void        DrawVLine       		(uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)                                     override;
        void        DrawRectangle   		(Box_t* pBox)                                                                                           override;
        //void    DrawRectangle        		(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height);
        void        DrawPixel      			(uint16_t PosX, uint16_t PosY)                                                                          override;
        void        DrawLine                (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction) override;
        void        PrintFont       		(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)                                                      override;

    private:

        void        SendCommand     (uint8_t Register, const uint8_t* pData, size_t Size, uint32_t Delay = 0);
        void        SendCommand     (uint8_t Register, uint8_t Data, uint32_t Delay = 0);
        void        WriteCommand    (uint8_t Register);
        void        WriteData       (uint16_t Data);
        void        WriteData       (uint8_t* pData, size_t Size);
        void        SetWindow       (uint8_t PosX1, uint8_t PosY1, uint8_t PosX2, uint8_t PosY2);
        void        SetWindow       (uint16_t PosX, uint16_t PosY, BoxSize_t* pBoxSize);
        void        SetWindow       (Box_t* pBox);
        void        WriteRLE16      (StaticImageRLE_16_t* pData, uint16_t* pDestination, size_t Size);
        void        WriteRLE32      (StaticImageRLE_32_t* pData, uint32_t* pDestination, size_t Size);
        void        PutColor        (uint16_t Color, size_t Count);
        void        PutPixel        (int32_t PosX, int32_t PosY, uint16_t Color, uint8_t Alpha = 255);
        void        DrawImage       (StaticImageInfo_t* pImage, Cartesian_t Position, BlendMode_e BlendMode);
        uint16_t    ConvertColor    (uint32_t Color);
        uint16_t    BlendColor      (uint16_t Source, uint16_t Background, uint8_t Alpha);

        SPI_Driver* m_pSPI;
        StaticImageInfo_t* m_pBackground;
        Cartesian_t m_ConstructPosition;
};

//-------------------------------------------------------------------------------------------------

#ifdef LIB_ST7735_GLOBAL
GrafxDriver         Grafx;
GrafxDriver*        myGrafx = &Grafx;
#else
extern GrafxDriver* myGrafx;
#endif

//-------------------------------------------------------------------------------------------------

#else // USE_SPI_DRIVER == DEF_ENABLED

#pragma message("DIGINI driver for SPI must be enable and configure to use this device driver")

#endif // USE_SPI_DRIVER == DEF_ENABLED
#endif // DIGINI_USE_GRAFX
