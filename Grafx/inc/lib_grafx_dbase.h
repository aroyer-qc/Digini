//-------------------------------------------------------------------------------------------------
//
//  File : lib_grafx_dbase.h
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

#if (DIGINI_USE_GRAFX == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------

#ifndef     DBASE_MAX_SKIN_IMAGE_QTY
    #define DBASE_MAX_SKIN_IMAGE_QTY            256                                 // this is max the number of image in database
#endif
#ifndef     DBASE_MAX_SKIN_FONT_QTY
    #define DBASE_MAX_SKIN_FONT_QTY             32                                  // this is max the number of font in database
#endif

#if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
  #define GFX_SKIN_RAM_DBASE_DEF(X_GFX_RAM_DBASE) \
    X_GFX_RAM_DBASE(  GFX_FREE_RAM_POINTER,    1,                         1,                           sizeof(void*)               )   \
    X_GFX_RAM_DBASE(  GFX_FREE_RELOAD_POINTER, 1,                         1,                           sizeof(void*)               )   \

#endif

#if (GRAFX_USE_QUAD_SPI_FOR_DATABASE == DEF_ENABLED)
  #define GFX_QSPI_DBASE_DEF(X_GFX_QSPI_DBASE) \
    X_GFX_QSPI_DBASE( GFX_FONT_INFO,          DBASE_MAX_SKIN_FONT_QTY,    1,                           sizeof(FontInfo_t)          )   \
    X_GFX_QSPI_DBASE( GFX_FONT_DESC_INFO,     DBASE_MAX_SKIN_FONT_QTY,    FONT_CHARACTER_PER_FONT_MAX, sizeof(FontDescriptor_t)    )   \
    X_GFX_QSPI_DBASE( GFX_IMAGE_INFO,         DBASE_MAX_SKIN_IMAGE_QTY,   1,                           sizeof(ImageInfo_t)         )   \

#endif

#if (GRAFX_USE_RAM_DATABASE == DEF_ENABLED)
  #define GFX_RAM_DBASE_DEF(X_GFX_RAM_DBASE) \
    X_GFX_RAM_DBASE( GFX_FONT_INFO,           DBASE_MAX_SKIN_FONT_QTY,    1,                           sizeof(FontInfo_t)          )   \
    X_GFX_RAM_DBASE( GFX_FONT_DESC_INFO,      DBASE_MAX_SKIN_FONT_QTY,    FONT_CHARACTER_PER_FONT_MAX, sizeof(FontDescriptor_t)    )   \
    X_GFX_RAM_DBASE( GFX_IMAGE_INFO,          DBASE_MAX_SKIN_IMAGE_QTY,   1,                           sizeof(ImageInfo_t)         )   \

#endif

#if (GRAFX_USE_ROM_DATABASE == DEF_ENABLED)
  #define GFX_ROM_DBASE_DEF(X_GFX_ROM_DBASE) \
    X_GFX_ROM_DBASE( GFX_FONT_INFO,          &FONT_InfoTable,       NUMBER_OF_FONT,            1,  sizeof(FontInfo_t)          )   \
    X_GFX_ROM_DBASE( GFX_IMAGE_INFO,         &StaticImageInfo,      NUMBER_OF_IMAGE,           1,  sizeof(StaticImageInfo_t*)  )   \

#endif

//    X_GFX_HARD_DBASE( GFX_FONT_DESC_INFO,      DBASE_MAX_FONT_QTY,        FONT_CHARACTER_PER_FONT_MAX, sizeof(FontDescriptor_t)    )

//-------------------------------------------------------------------------------------------------

 #endif // (DIGINI_USE_GRAFX == DEF_ENABLED)
