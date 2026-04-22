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

#ifdef DIGINI_USE_GRAFX

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define GRAFX_NUMBER_OF_ACTIVE_LAYER                2

#define GRAFX_DRIVER_USE_V_SYNC                     DEF_DISABLED

// Display size
#define GRAFX_DRIVER_SIZE_X                         128
#define GRAFX_DRIVER_SIZE_Y                         160

#define GRAFX_RAM_SIZE_X                            132
#define GRAFX_RAM_SIZE_Y                            162


//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DELAY                           0x80

#define ST7735_NOP                      0x00                    // No operation
#define ST7735_SWRESET                  0x01                    // Software Reset
#define ST7735_RDDID                    0x04                    // Read Display ID
#define ST7735_RDDST                    0x09                    // Read Display Status

#define ST7735_SLPIN                    0x10                    // Sleep In & Booster OFF
#define ST7735_SLPOUT                   0x11                    // Sleep Out & Booster ON
#define ST7735_PTLON                    0x12                    // Partial Mode ON
#define ST7735_NORON                    0x13                    // Partial Mode OFF (Normal)
#define ST7735_INVOFF                   0x20                    // Display Inversion OFF
#define ST7735_INVON                    0x21                    // Display Inversion ON
#define ST7735_DISPOFF                  0x28                    // Display OFF
#define ST7735_DISPON                   0x29                    // Display ON
#define ST7735_CASET                    0x2A                    // Column Address Set
#define ST7735_RAMRD                    0x2E                    // Memory read
#define ST7735_RASET                    0x2B                    // Row Address Set
#define ST7735_RAM_WRITE                0x2C                    // Memory Write

#define ST7735_PTLAR                    0x30                    // Partial Start/End Address Set
#define ST7735_MADCTL                   0x36                    // Memory Data Access Control
#define ST7735_COLMOD                   0x3A                    // Interface Pixel Format

#define ST7735_FRMCTR1                  0xB1                    //
#define ST7735_FRMCTR2                  0xB2                    //
#define ST7735_FRMCTR3                  0xB3                    //
#define ST7735_INVCTR                   0xB4                    //
#define ST7735_DISSET5                  0xB6                    //

#define ST7735_RDID_1                   0xDA                    // Read ID1
#define ST7735_RDID_2                   0xDB                    // Read ID2
#define ST7735_RDID_3                   0xDC                    // Read ID3
#define ST7735_RDID_4                   0xDD                    // Read ID4

#define ST7735_GMCTRP1                  0xE0                    //
#define ST7735_GMCTRN1                  0xE1                    //

#define ST7735_POWER_CTRL_1             0xC0                    //
#define ST7735_POWER_CTRL_2             0xC1                    //
#define ST7735_POWER_CTRL_3             0xC2                    //
#define ST7735_POWER_CTRL_4             0xC3                    //
#define ST7735_POWER_CTRL_5             0xC4                    //
#define ST7735_POWER_CTRL_6             0xFC                    //
#define ST7735_VMCTR1                   0xC5                    //

// Colors
#define BLACK                           0x0000
#define WHITE                           0xFFFF
#define RED                             0xF800
#define BLUE                            0x00F8
#define GREEN                           0x0707

#define ST7735_SUCCESS                  0
#define ST7735_ERROR                    1

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class GrafxDriver : public GrafxGenDriver
{
    public:

        //using GrafxGenDriver::DrawRectangle;            // Expose all DrawRectangle from base class

        void        Initialize      		(const void* pArg)                                                                                      override;
        void        DisplayOn       		(void)                 override         { SendCommand(ST7735_DISPON, nullptr, 0, 1); }
        void        DisplayOff      		(void)                 override         { SendCommand(ST7735_DISPOFF, nullptr, 0, 1); }


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
        void        PrintFont       		(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)                                                      override;

    private:

        void        SetWriteRAM_Ready       (void)                                  { IO_SetPinHigh(IO_ST7735_DC); } // Data (active high)



        void        SendCommand     (uint8_t Register, uint8_t* pData, uint32_t Size, uint32_t Delay = 0);
        void        SendCommand     (uint8_t Register, uint8_t Data, uint32_t Delay = 0);
        void        WriteCommand    (uint8_t Register);
        uint8_t     ReadCommand     (uint8_t Register);
        void        WriteData       (uint8_t Data);
        void        WriteData       (uint16_t Data);
        void        WriteData       (uint32_t Data);
        void        WriteData       (uint8_t* pData, uint32_t Size);
        uint8_t     ReadData        (void);
        void        SetWindow       (uint8_t PosX1, uint8_t PosY1, uint8_t PosX2, uint8_t PosY2);
        void        PutColor        (uint16_t Color, uint16_t Count);
        void        Line            (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction);
        void        Clear           (void);

        SPI_Driver* m_pSPI;
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
