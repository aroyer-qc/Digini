//-------------------------------------------------------------------------------------------------
//
//  File : lib_color.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum __ColorChannel_e
{
    CHANNEL_BLUE = 0,
    CHANNEL_GREEN,
    CHANNEL_RED,
    CHANNEL_ALPHA,
} ColorChannel_e;

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetFormatColor
//
//   Parameter(s):  PixelFormat_e   PixelFormat
//                  eColorTable     Index
//   Return Value:  uint32_t        Color
//
//   Description:   Get the color from table and apply conversion according to format
//
//-------------------------------------------------------------------------------------------------
uint32_t GetFormatColor(PixelFormat_e PixelFormat, ColorTable_e Index)
{
    uint32_t Color;
    uint32_t NewColor;

    Color = GFX_ColorTable[Index];

    switch(PixelFormat)
    {
      #if (GRAFX_COLOR_ARGB4444 == DEF_ENABLED)
        case PIXEL_FORMAT_ARGB4444:
        {
            NewColor = ((COLOR_A(Color) & 0xF0) << 8) | ((COLOR_R(Color) & 0xF0) << 4) | ((COLOR_G(Color) & 0xF0)) | ((COLOR_B(Color) >> 4));
        }
        break;
      #endif

      #if (GRAFX_COLOR_RGB565 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB565:
        {
            NewColor = ((COLOR_R(Color) & 0xF8) << 8) | ((COLOR_G(Color) & 0xFC) << 3) | ((COLOR_B(Color) >> 3));
        }
        break;
      #endif

      #if (GRAFX_COLOR_ARGB1555 == DEF_ENABLED)
        case PIXEL_FORMAT_ARGB1555:
        {
            NewColor  = (COLOR_A(Color) >= 0x80) ? 0x8000 : 0x0000;
            NewColor != ((COLOR_R(Color) & 0xF8) << 7) | ((COLOR_G(Color) & 0xF8) << 2) | ((COLOR_B(Color) >> 3));
        }
        break;
      #endif

      #if (GRAFX_COLOR_RGB888 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB888:
        {
            NewColor = Color & 0x00FFFFFF;
        }
        break;
      #endif

        // TO DO
      #if (GRAFX_COLOR_L8 == DEF_ENABLED)
        case PIXEL_FORMAT_L8:        // Lookup table  16 color
      #endif

      #if (GRAFX_COLOR_AL44 == DEF_ENABLED)
        case PIXEL_FORMAT_AL44:      // Lookup table  16 alpha level,  16 color
      #endif

      #if (GRAFX_COLOR_AL88 == DEF_ENABLED)
        case PIXEL_FORMAT_AL88:      // Lookup table  256 alpha level, 256 color
      #endif

      #if (GRAFX_COLOR_ARGB8888 == DEF_ENABLED)
        case PIXEL_FORMAT_ARGB8888:
        {
            NewColor = Color;
        }
        break;
      #endif
      #if (GRAFX_COLOR_L4 == DEF_ENABLED)
        case PIXEL_FORMAT_L4:
      #endif

      #if (GRAFX_COLOR_A8 == DEF_ENABLED)
        case PIXEL_FORMAT_A8:
      #endif

      #if (GRAFX_COLOR_A4 == DEF_ENABLED)
        case PIXEL_FORMAT_A4:
      #endif

      #if (GRAFX_COLOR_RGB332 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB332:
      #endif

      #if (GRAFX_COLOR_RGB444 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB444:
      #endif

        default:
        {
            NewColor = Color;
        }
        break;
    }

    return NewColor;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetUpConvertColor
//
//   Parameter(s):  PixelFormat_e   PixelFormat
//                  uint32_t        dwColor
//   Return Value:  uint32_t        dwColor
//
//   Description:   Upconverter from any format to 32 Bit's ARGB
//
//-------------------------------------------------------------------------------------------------
uint32_t GetUpConvertColor(PixelFormat_e PixelFormat, uint32_t Color)
{
    switch(PixelFormat)
    {
      #if (GRAFX_COLOR_ARGB4444 == DEF_ENABLED)
        case PIXEL_FORMAT_ARGB4444:
        {
            Color = (((uint32_t)((Color >> 8)            | 0x0F)) << 24) |
                    (((uint32_t)(((Color & 0x0F00) >> 4) | 0x0F)) << 16) |
                    (((uint32_t)((Color & 0x00F0)        | 0x0F)) << 8)  |
                    ((uint32_t)(((Color & 0x000F) << 4) | 0x0F));
        }
        break;
      #endif

      #if (GRAFX_COLOR_RGB565 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB565:
        {
            Color = (((uint32_t)0xFF) << 24)                             |
                    (((uint32_t)(((Color & 0xF800) >> 8) | 0x07)) << 16) |
                    (((uint32_t)(((Color & 0x07E0) >> 3) | 0x03)) << 8)  |
                    ((uint32_t)(((Color & 0x001F) << 3) | 0x07));
        }
        break;
      #endif

      #if (GRAFX_COLOR_ARGB1555 == DEF_ENABLED)
        case PIXEL_FORMAT_ARGB1555:
        {
            Color = (((uint32_t)((Color & 0x8000) ? 0xFF : 0x00)) << 24) |
                    (((uint32_t)(((Color & 0x7C00) >> 7) | 0x07)) << 16) |
                    (((uint32_t)(((Color & 0x03E0) >> 2) | 0x07)) << 8)  |
                    ((uint32_t)(((Color & 0x001F) << 3) | 0x07));
        }
        break;
      #endif

      #if (GRAFX_COLOR_RGB888 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB888:
        {
            Color = Color | 0xFF000000;
        }
        break;
      #endif

        // TO DO
      #if (GRAFX_COLOR_L8 == DEF_ENABLED)
        case PIXEL_FORMAT_L8:        // Lookup table  16 color
      #endif
      #if (GRAFX_COLOR_AL44 == DEF_ENABLED)
        case PIXEL_FORMAT_AL44:      // Lookup table  16 alpha level,  16 color
      #endif
      #if (GRAFX_COLOR_AL88 == DEF_ENABLED)
        case PIXEL_FORMAT_AL88:      // Lookup table  256 alpha level, 256 color
      #endif
      #if (GRAFX_COLOR_ARGB8888 == DEF_ENABLED)
        case PIXEL_FORMAT_ARGB8888:                                  // Nothing to do
      #endif
      #if (GRAFX_COLOR_L4 == DEF_ENABLED)
        case PIXEL_FORMAT_L4:
      #endif
      #if (GRAFX_COLOR_A8 == DEF_ENABLED)
        case PIXEL_FORMAT_A8:
      #endif
      #if (GRAFX_COLOR_A4 == DEF_ENABLED)
        case PIXEL_FORMAT_A4:
      #endif
      #if (GRAFX_COLOR_RGB332 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB332:
      #endif
      #if (GRAFX_COLOR_RGB444 == DEF_ENABLED)
        case PIXEL_FORMAT_RGB444:
      #endif
        default:
        {
            break;
        }
    }

    return Color;
}

//-------------------------------------------------------------------------------------------------
#endif // DIGINI_USE_GRAFX


