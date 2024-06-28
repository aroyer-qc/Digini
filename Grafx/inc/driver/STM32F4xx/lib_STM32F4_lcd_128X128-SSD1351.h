//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_lcd_128x128-SSD1779.h
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "digini_cfg.h"
#ifdef DIGINI_USE_GRAFX
#include "./Digini/Grafx/inc/lib_grafx.h"
#include "lib_lcd_driver.h"// TODO old stuff
#include "./Peripheral/inc/Port/lib_io.h"
#include "./Peripheral/inc/Port/lib_class_io_bus.h"
#include "stm32f4xx.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define GRAFX_NUMBER_OF_ACTIVE_LAYER                2

#define GRAFX_DRIVER_USE_V_SYNC                     DEF_DISABLED
//#define GRAFX_USE_SOFT_PIXEL
//#define GRAFX_USE_SOFT_BOX
//#define GRAFX_USE_SOFT_VLINE
//#define GRAFX_USE_SOFT_HLINE
//#define GRAFX_USE_SOFT_DLINE
#define GRAFX_USE_SOFT_CIRCLE

//#define GRAFX_USE_SOFT_PRINT_FONT
#define GRAFX_USE_SOFT_ALPHA
//#define GRAFX_USE_SOFT_COPY
//#define GRAFX_USE_SOFT_FILL

// Display size
#define GRAFX_DRIVER_SIZE_X     128
#define GRAFX_DRIVER_SIZE_Y     128

//#define GRAFX_DRIVER_USE_OWN_COPY_LAYER_TO_LAYER

#define SSD1779_NO_FILL         0
#define SSD1779_FILL            1

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class GrafxDriver : public GRAFX_Interface
{
    public:

        void            Initialize            (void* pArg);
        void            DisplayOn             (void);
        void            DisplayOff            (void);
        void            LayerConfig           (CLayer* pLayer);
        void            CopyLinear            (void* pSrc, Box_t* pBox, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
        void            BlockCopy             (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
        void            BlockCopy             (void* pSrc, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
        void            DrawRectangle         (Box_t* pBox);
        void            PrintFont             (FontDescriptor_t* pDescriptor, Cartesian_t* pPos);
        void            DrawBox               (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness);
        void            DrawPixel             (uint16_t PosX, uint16_t PosY);
        void            DrawHLine             (uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t ThickNess);
        void            DrawVLine             (uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t ThickNess);
        void            DrawLine              (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction);
        void            DrawCircle            (uint8_t X, uint8_t Y, uint8_t Radius, uint8_t Mode);
        void            Copy                  (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat_e, BlendMode_e BlendMode);

      #if (GRAFX_DRIVER_USE_V_SYNC == DEF_ENABLED)
        void            WaitFor_V_Sync        (void) {};
      #endif

        void            CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, Box_t* pBox) {};
        void            CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height) {};
        void            CopyLayerToLayer      (Layer_e SrcLayer, Layer_e DstLayer, uint16_t SrcX, uint16_t SrcY, uint16_t DstX, uint16_t DstY, uint16_t Width, uint16_t Height) {};

    private:

        void            ControllerInitialize  (void);     // LCD is equipped with a IL9341 controller
        void            WriteCommand          (uint8_t Register);
        uint8_t         ReadCommand           (uint8_t Register);
        void            WriteData             (uint8_t Data);
        void            WriteData             (uint16_t Data);
        uint8_t         ReadData_8            (void);
        uint16_t        ReadData_16           (void);
        void            Line                  (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t ThickNess, DrawMode_e Direction);
        void            DrawRectangle         (Box_t* pBox, uint8_t Mode = SSD1779_NO_FILL);
        void            Clear                 (void);

        IO_ID_e         m_RegSelect;
        IO_ID_e         m_Reset;
        IO_BusDriver    m_Bus;
        CLayer*         m_pLayer;
};

//-------------------------------------------------------------------------------------------------

#ifdef LIB_SSD1779_GLOBAL
class GrafxDriver             Grafx;
class GRAFX_Interface*        myGrafx = &Grafx;
#else
extern class GRAFX_Interface* myGrafx;
#endif

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX


#if 0
#ifndef __SSD1351_H__
#define __SSD1351_H__

#include "main.h"

typedef struct
{
  uint16_t width;         // LCD width in pixels (default orientation)
  uint16_t height;        // LCD height in pixels (default orientation)
} lcdProperties_t;

/*=========================================================================
    SPI modes
    -----------------------------------------------------------------------
    The OLED supports both 3-pin and 4-pin SPI modes
    3-PIN MODE      Saves one GPIO pin (D/C) by adding the D/C bit before
                    every transfer, meaning that 9 bits are sent every frame
                    instead of 8.  You have slightly slower performance
                    but use less pins. Requires 3 GPIO pins (SCK, SID, RST)
    4-PIN MODE      Uses a normal SPI interface with 8-bits per transfer,
                    plus a dedicated D/C pin to indicate if this is a
                    command or data byte. Requires 4 GPIO pins (SCK, SID,
                    RST, DC).
    To select one of the SPI modes, make sure the BS0/1 pins are correctly
    set on the OLED display and uncomment the appropriate define below.
    -----------------------------------------------------------------------*/
  //  #define SSD1351_BUS_SPI3
    #define SSD1351_BUS_SPI4
/*=========================================================================*/

// Macros for control line state
#define SET_SID	(PORTB |= (1<<PORTB3))		// B3
#define CLR_SID	(PORTB &= ~(1<<PORTB3))
#define SET_SCK	(PORTB |= (1<<PORTB5))		// B5
#define CLR_SCK	(PORTB &= ~(1<<PORTB5))
#define SET_CS	(PORTC |= (1<<PORTC4))		// C4
#define CLR_CS	(PORTC &= ~(1<<PORTC4))
#define SET_DC	(PORTC |= (1<<PORTC3))		// C3
#define CLR_DC	(PORTC &= ~(1<<PORTC3))
#define SET_RST	(PORTC |= (1<<PORTC2))		// C2
#define CLR_RST	(PORTC &= ~(1<<PORTC2))

// SSD1351 Commands
enum
{
  SSD1351_CMD_SETCOLUMNADDRESS          = 0x15,
  SSD1351_CMD_SETROWADDRESS             = 0x75,
  SSD1351_CMD_WRITERAM                  = 0x5C, // Write data to GRAM and increment until another command is sent
  SSD1351_CMD_READRAM                   = 0x5D, // Read data from GRAM and increment until another command is sent
  SSD1351_CMD_COLORDEPTH                = 0xA0, // Numerous functions include increment direction ... see DS 
                                                // A0[0] = Address Increment Mode (0 = horizontal, 1 = vertical)
                                                // A0[1] = Column Address Remap (0 = left to right, 1 = right to left)
                                                // A0[2] = Color Remap (0 = ABC, 1 = CBA) - HW RGB/BGR switch
                                                // A0[4] = COM Scan Direction (0 = top to bottom, 1 = bottom to top)
                                                // A0[5] = Odd/Even Paid Split
                                                // A0[7:6] = Display Color Mode (00 = 8-bit, 01 = 65K, 10/11 = 262K, 8/16-bit interface only)
  SSD1351_CMD_SETDISPLAYSTARTLINE       = 0xA1,
  SSD1351_CMD_SETDISPLAYOFFSET          = 0xA2, 
  SSD1351_CMD_SETDISPLAYMODE_ALLOFF     = 0xA4, // Force entire display area to grayscale GS0
  SSD1351_CMD_SETDISPLAYMODE_ALLON      = 0xA5, // Force entire display area to grayscale GS63
  SSD1351_CMD_SETDISPLAYMODE_RESET      = 0xA6, // Resets the display area relative to the above two commands
  SSD1351_CMD_SETDISPLAYMODE_INVERT     = 0xA7, // Inverts the display contents (GS0 -> GS63, GS63 -> GS0, etc.)
  SSD1351_CMD_FUNCTIONSELECTION         = 0xAB, // Enable/Disable the internal VDD regulator
  SSD1351_CMD_SLEEPMODE_DISPLAYOFF      = 0xAE, // Sleep mode on (display off)
  SSD1351_CMD_SLEEPMODE_DISPLAYON       = 0xAF, // Sleep mode off (display on)
  SSD1351_CMD_SETPHASELENGTH            = 0xB1, // Larger capacitance may require larger delay to discharge previous pixel state
  SSD1351_CMD_ENHANCEDDRIVINGSCHEME     = 0xB2, 
  SSD1351_CMD_SETFRONTCLOCKDIV          = 0xB3, // DCLK divide ration fro CLK (from 1 to 16)
  SSD1351_CMD_SETSEGMENTLOWVOLTAGE      = 0xB4,
  SSD1351_CMD_SETGPIO                   = 0xB5,
  SSD1351_CMD_SETSECONDPRECHARGEPERIOD  = 0xB6,
  SSD1351_CMD_GRAYSCALELOOKUP           = 0xB8,
  SSD1351_CMD_LINEARLUT                 = 0xB9,
  SSD1351_CMD_SETPRECHARGEVOLTAGE       = 0xBB,
  SSD1351_CMD_SETVCOMHVOLTAGE           = 0xBE,
  SSD1351_CMD_SETCONTRAST               = 0xC1,
  SSD1351_CMD_MASTERCONTRAST            = 0xC7,
  SSD1351_CMD_SETMUXRRATIO              = 0xCA,
  SSD1351_CMD_NOP3                      = 0xD1,
  SSD1351_CMD_NOP4                      = 0xE3,
  SSD1351_CMD_SETCOMMANDLOCK            = 0xFD,
  SSD1351_CMD_HORIZONTALSCROLL          = 0x96,
  SSD1351_CMD_STOPMOVING                = 0x9E,
  SSD1351_CMD_STARTMOVING               = 0x9F  
};

void lcdFillRGB(uint8_t r,uint8_t g,uint8_t b);
void lcdFillRGB2(uint8_t r,uint8_t g,uint8_t b);
void lcdFillRGBdither(uint8_t r,uint8_t g,uint8_t b,uint8_t r2,uint8_t g2, uint8_t b2,uint8_t val);
void lcdInit(void);

#endif
#endif

