//-------------------------------------------------------------------------------------------------
//
//  File : crc.h
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
// include(s)
//-------------------------------------------------------------------------------------------------

#include "crc_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_CRC_AS_ENUM(ENUM_ID, POLY, INIT, XOR_OUT, REF_IN, REF_OUT, WIDTH)          ENUM_ID,
#define EXPAND_X_CRC_AS_CLASS_CONST(ENUM_ID, POLY, INIT, XOR_OUT, REF_IN, REF_OUT, WIDTH)   {POLY, INIT, XOR_OUT, REF_IN, REF_OUT, WIDTH},

#define CRC_METHOD_DEF(X_CRC)\
/*                                                 Algorithm	        Poly           Init	        XorOut      RefIn   RefOut	 Size */  \
    IF_USE( DIGINI_USE_CRC_8,               X_CRC( CRC_8,               0x07,          0x00,        0x00,       false,  false,   8  ) )\
    IF_USE( DIGINI_USE_CRC_8_CDMA2000,      X_CRC( CRC_8_CDMA2000,      0x9B,          0xFF,        0x00,       false,  false,   8  ) )\
    IF_USE( DIGINI_USE_CRC_8_DARC,          X_CRC( CRC_8_DARC,          0x39,          0x00,        0x00,       true,   true,    8  ) )\
    IF_USE( DIGINI_USE_CRC_8_DVB_S2,        X_CRC( CRC_8_DVB_S2,        0xD5,          0x00,        0x00,       false,  false,   8  ) )\
    IF_USE( DIGINI_USE_CRC_8_EBU,           X_CRC( CRC_8_EBU,           0x1D,          0xFF,        0x00,       true,   true,    8  ) )\
    IF_USE( DIGINI_USE_CRC_8_I_CODE,        X_CRC( CRC_8_I_CODE,        0x1D,          0xFD,        0x00,       false,  false,   8  ) )\
    IF_USE( DIGINI_USE_CRC_8_ITU,           X_CRC( CRC_8_ITU,           0x07,          0x00,        0x55,       false,  false,   8  ) )\
    IF_USE( DIGINI_USE_CRC_8_MAXIM,         X_CRC( CRC_8_MAXIM,         0x31,          0x00,        0x00,       true,   true,    8  ) )\
    IF_USE( DIGINI_USE_CRC_8_ROHC,          X_CRC( CRC_8_ROHC,          0x07,          0xFF,        0x00,       true,   true,    8  ) )\
    IF_USE( DIGINI_USE_CRC_8_WCDMA,         X_CRC( CRC_8_WCDMA,         0x9B,          0x00,        0x00,       true,   true,    8  ) )\
    IF_USE( DIGINI_USE_CRC_16_ARC,          X_CRC( CRC_16_ARC,          0x8005,        0x0000,      0x0000,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_AUG_CCITT,    X_CRC( CRC_16_AUG_CCITT,    0x1021,        0x1D0F,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_BUYPASS,      X_CRC( CRC_16_BUYPASS,      0x8005,        0x0000,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_CCITT_FALSE,  X_CRC( CRC_16_CCITT_FALSE,  0x1021,        0xFFFF,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_CDMA2000,     X_CRC( CRC_16_CDMA2000,     0xC867,        0xFFFF,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_DDS_110,      X_CRC( CRC_16_DDS_110,      0x8005,        0x800D,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_DECT_R,       X_CRC( CRC_16_DECT_R,       0x0589,        0x0000,      0x0001,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_DECT_X,       X_CRC( CRC_16_DECT_X,       0x0589,        0x0000,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_DNP,          X_CRC( CRC_16_DNP,          0x3D65,        0x0000,      0xFFFF,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_EN_13757,     X_CRC( CRC_16_EN_13757,     0x3D65,        0x0000,      0xFFFF,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_GENIBUS,      X_CRC( CRC_16_GENIBUS,      0x1021,        0xFFFF,      0xFFFF,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_KERMIT,       X_CRC( CRC_16_KERMIT,       0x1021,        0x0000,      0x0000,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_MAXIM,        X_CRC( CRC_16_MAXIM,        0x8005,        0x0000,      0xFFFF,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_MCRF4XX,      X_CRC( CRC_16_MCRF4XX,      0x1021,        0xFFFF,      0x0000,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_MODBUS,       X_CRC( CRC_16_MODBUS,       0x8005,        0xFFFF,      0x0000,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_RIELLO,       X_CRC( CRC_16_RIELLO,       0x1021,        0xB2AA,      0x0000,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_T10_DIF,      X_CRC( CRC_16_T10_DIF,      0x8BB7,        0x0000,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_TELEDISK,     X_CRC( CRC_16_TELEDISK,     0xA097,        0x0000,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_TMS37157,     X_CRC( CRC_16_TMS37157,     0x1021,        0x89EC,      0x0000,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_USB,          X_CRC( CRC_16_USB,          0x8005,        0xFFFF,      0xFFFF,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_X25,          X_CRC( CRC_16_X25,          0x1021,        0xFFFF,      0xFFFF,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_16_XMODEM,       X_CRC( CRC_16_XMODEM,       0x1021,        0x0000,      0x0000,     false,  false,   16 ) )\
    IF_USE( DIGINI_USE_CRC_16_A,            X_CRC( CRC_16_A,            0x1021,        0xC6C6,      0x0000,     true,   true,    16 ) )\
    IF_USE( DIGINI_USE_CRC_32,              X_CRC( CRC_32,              0x04C11DB7,    0xFFFFFFFF,  0xFFFFFFFF, true,   true,    32 ) )\
    IF_USE( DIGINI_USE_CRC_32_BZIP2,        X_CRC( CRC_32_BZIP2,        0x04C11DB7,    0xFFFFFFFF,  0xFFFFFFFF, false,  false,   32 ) )\
    IF_USE( DIGINI_USE_CRC_32_JAMCRC,       X_CRC( CRC_32_JAMCRC,       0x04C11DB7,    0xFFFFFFFF,  0x00000000, true,   true,    32 ) )\
    IF_USE( DIGINI_USE_CRC_32_MPEG_2,       X_CRC( CRC_32_MPEG_2,       0x04C11DB7,    0xFFFFFFFF,  0x00000000, false,  false,   32 ) )\
    IF_USE( DIGINI_USE_CRC_32_POSIX,        X_CRC( CRC_32_POSIX,        0x04C11DB7,    0x00000000,  0xFFFFFFFF, false,  false,   32 ) )\
    IF_USE( DIGINI_USE_CRC_32_SATA,         X_CRC( CRC_32_SATA,         0x04C11DB7,    0x52325032,  0x00000000, false,  false,   32 ) )\
    IF_USE( DIGINI_USE_CRC_32_XFER,         X_CRC( CRC_32_XFER,         0x000000AF,    0x00000000,  0x00000000, false,  false,   32 ) )\
    IF_USE( DIGINI_USE_CRC_32_C,            X_CRC( CRC_32_C,            0x1EDC6F41,    0xFFFFFFFF,  0xFFFFFFFF, true,   true,    32 ) )\
    IF_USE( DIGINI_USE_CRC_32_D,            X_CRC( CRC_32_D,            0xA833982B,    0xFFFFFFFF,  0xFFFFFFFF, true,   true,    32 ) )\
    IF_USE( DIGINI_USE_CRC_32_Q,            X_CRC( CRC_32_Q,            0x814141AB,    0x00000000,  0x00000000, false,  false,   32 ) )\

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

enum CRC_Type_e
{
    CRC_METHOD_DEF(EXPAND_X_CRC_AS_ENUM)
    NUMBER_OF_CRC_METHOD,
};

struct CRC_Info_t
{
    uint32_t    Polynomial;
    uint32_t    Init;
    uint32_t    XorOut;
    bool        RefIn;
    bool        RefOut;
    uint8_t     Width;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CRC_Calc
{
    public:

                    CRC_Calc        (CRC_Type_e Type);
                   ~CRC_Calc        ();                         // destructor does something only if hardware module exist

        void        Start           (void);
        uint32_t    Done            (void);
        void        Calculate       (const uint8_t Value);
        void        CalculateBuffer (const uint8_t *pBuffer, size_t Length);

    private:

        CRC_Type_e                  m_Type;
        uint32_t                    m_Polynomial;
        bool                        m_RefIn;
        uint8_t                     m_Width;
        uint32_t                    m_TopBit;
        uint32_t                    m_Remainder;
        //static mutex..  ?? for usage of the internal CRC module... will be use only if method is supported by the module.. if not soft method will be used
        static const CRC_Info_t     m_MethodList[NUMBER_OF_CRC_METHOD];
};

//-------------------------------------------------------------------------------------------------

