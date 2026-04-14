//-------------------------------------------------------------------------------------------------
//
//  File : lib_lcd_128X160-ST7735.h
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
#define GRAFX_USE_SOFT_COPY_LINEAR
#define GRAFX_USE_SOFT_COPY_LAYER_TO_LAYER
//#define GRAFX_USE_SOFT_PIXEL
//#define GRAFX_USE_SOFT_BOX
                    //                  #define GRAFX_USE_SOFT_VLINE     should not exist anymore with the override!!!!!
                    //                  #define GRAFX_USE_SOFT_HLINE     should not exist anymore with the override!!!!!
                    //                  #define GRAFX_USE_SOFT_DLINE     should not exist anymore with the override!!!!!
                    //                  #define GRAFX_USE_SOFT_CIRCLE    should not exist anymore with the override!!!!!
//#define GRAFX_USE_SOFT_RECTANGLE

//#define GRAFX_USE_SOFT_PRINT_FONT
//#define GRAFX_USE_SOFT_ALPHA                      needon this project as LCD doesn't have alpha
//#define GRAFX_USE_SOFT_COPY
//#define GRAFX_USE_SOFT_FILL

// Display size
#define GRAFX_DRIVER_SIZE_X                         128
#define GRAFX_DRIVER_SIZE_Y                         160

#define GRAFX_RAM_SIZE_X                            132
#define GRAFX_RAM_SIZE_Y                            162

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class GrafxDriver : public GrafxGenDriver
{
    public:

        using GrafxGenDriver::DrawRectangle;            // Expose all DrawRectangle from base class

        void        Initialize      (const void* pArg)                                                                                      override;
        void        DisplayOn       (void)                                                                                                  override;
        void        DisplayOff      (void)                                                                                                  override;

        void        LayerConfig     (DisplayLayer* pLayer)                                                                                  override;

        void        BlockCopy       (void* pSrc, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height,
                                     uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)                     override;
        void        BlockCopy       (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)    override;
        void        DrawBox         (uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t Thickness)                     override;
        void        DrawHLine       (uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)                                     override;
        void        DrawVLine       (uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)                                     override;
        void        DrawRectangle   (Box_t* pBox)                                                                                           override;
        //void    DrawRectangle         (uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height);
        void        DrawPixel       (uint16_t PosX, uint16_t PosY)                                                                          override;
        void        PrintFont       (FontDescriptor_t* pDescriptor, Cartesian_t* pPos)                                                      override;

    private:

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
