//-------------------------------------------------------------------------------------------------
//
//  File : lib_lcd_128X160-ST7735.h
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
//#define GRAFX_USE_SOFT_VLINE
//#define GRAFX_USE_SOFT_HLINE
#define GRAFX_USE_SOFT_DLINE
#define GRAFX_USE_SOFT_CIRCLE
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

class GrafxDriver : public GRAFX_Interface
{
    public:

        // This include all required prototype for a driver.
        #include "./Grafx/inc/driver/lib_grafx_driver_common.h"

    private:

        void            SendCommand             (uint8_t Register, uint8_t* pData, uint32_t Size, uint32_t Delay = 0);
        void            SendCommand             (uint8_t Register, uint8_t Data, uint32_t Delay = 0);
        void            WriteCommand            (uint8_t Register);
        uint8_t         ReadCommand             (uint8_t Register);
        void            WriteData               (uint8_t Data);
        void            WriteData               (uint16_t Data);
        void            WriteData               (uint32_t Data);
        void            WriteData               (uint8_t* pData, uint32_t Size);
        uint8_t         ReadData                (void);
        void            SetWindow               (uint8_t PosX1, uint8_t PosY1, uint8_t PosX2, uint8_t PosY2);
        void            PutColor                (uint16_t Color, uint16_t Count);
        void            Line                    (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t ThickNess, DrawMode_e Direction);
       // void            DrawRectangle         (Box_t* pBox, uint8_t Mode /*= SSD1779_NO_FILL*/);
        void            Clear                   (void);

        SPI_Driver*     m_pSPI;
        CLayer*         m_pLayer;
};

//-------------------------------------------------------------------------------------------------

#ifdef LIB_ST7735_GLOBAL
class GrafxDriver             Grafx;
class GRAFX_Interface*        myGrafx = &Grafx;
#else
extern class GRAFX_Interface* myGrafx;
#endif

//-------------------------------------------------------------------------------------------------

#else // USE_SPI_DRIVER == DEF_ENABLED

#pragma message("DIGINI driver for SPI must be enable and configure to use this device driver")

#endif // USE_SPI_DRIVER == DEF_ENABLED
#endif // DIGINI_USE_GRAFX
