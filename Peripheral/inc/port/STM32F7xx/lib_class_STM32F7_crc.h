//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_crc.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_HW_CRC_AS_ENUM(ENUM_ID, POLY, INIT, XOR_OUT, REF_IN, REF_OUT, WIDTH)          ENUM_ID,
#define EXPAND_X_HW_CRC_AS_CLASS_CONST(ENUM_ID, POLY, INIT, XOR_OUT, REF_IN, REF_OUT, WIDTH)   {POLY, INIT, XOR_OUT, (REF_IN | REF_OUT | WIDTH)},

#define CRC_32_HW_METHOD_DEF(X_CRC)   /* Check is base on string "123456789" */ \
/*                                                   Algorithm	            Poly,       Init        XorOut      Reflect Data In,         Reflect Data Out,            Poly Size,                 Check                   */ \
    IF_USE( DIGINI_USE_HW_CRC_8,              X_CRC( CRC_HW_8,              0x07,       0x00,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xF4                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_AUTOSAR,      X_CRC( CRC_HW_8_AUTOSAR,      0x2F,       0xFF,       0xFF,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xDF                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_BLUETOOTH,    X_CRC( CRC_HW_8_BLUETOOTH,    0xA7,       0x00,       0x00,       CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_8_BITS)) /* 0x26                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_CDMA2000,     X_CRC( CRC_HW_8_CDMA2000,     0x9B,       0xFF,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xDA                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_DARC,         X_CRC( CRC_HW_8_DARC,         0x39,       0x00,       0x00,       CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_8_BITS)) /* 0x15                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_DVB_S2,       X_CRC( CRC_HW_8_DVB_S2,       0xD5,       0x00,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xBC                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_EBU,          X_CRC( CRC_HW_8_EBU,          0x1D,       0xFF,       0x00,       CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_8_BITS)) /* 0x97                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_GSM_A,        X_CRC( CRC_HW_8_GSM_A,        0x1D,       0x00,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0x37                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_GSM_B,        X_CRC( CRC_HW_8_GSM_B,        0x49,       0x00,       0xFF,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0x94                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_HITAG,        X_CRC( CRC_HW_8_HITAG,        0x1D,       0xFF,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xB4                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_I_CODE,       X_CRC( CRC_HW_8_I_CODE,       0x1D,       0xFD,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0x7E                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_I_432_1,      X_CRC( CRC_HW_8_I_432_1,      0x07,       0x00,       0x55,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xA1                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_ITU,          X_CRC( CRC_HW_8_ITU,          0x07,       0x00,       0x55,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xA1                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_LTE,          X_CRC( CRC_HW_8_LTE,          0x9B,       0x00,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xEA                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_MAXIM,        X_CRC( CRC_HW_8_MAXIM,        0x31,       0x00,       0x00,       CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_8_BITS)) /* 0xA1                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_MIFARE_MAD,   X_CRC( CRC_HW_8_MIFARE_MAD,   0x1D,       0xC7,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0x99                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_NRSC_5,       X_CRC( CRC_HW_8_NRSC_5,       0x31,       0xFF,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xF7                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_OPENSAFETY,   X_CRC( CRC_HW_8_OPENSAFETY,   0x2F,       0x00,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0x3E                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_ROHC,         X_CRC( CRC_HW_8_ROHC,         0x07,       0xFF,       0x00,       CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_8_BITS)) /* 0xD0                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_SAE_J1850,    X_CRC( CRC_HW_8_SAE_J1850,    0x1D,       0xFF,       0xFF,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0x4B                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_SMBUS,        X_CRC( CRC_HW_8_SMBUS,        0x07,       0x00,       0x00,       CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_8_BITS)) /* 0xF4                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_TECH_3250,    X_CRC( CRC_HW_8_TECH_3250,    0x1D,       0xFF,       0x00,       CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_8_BITS)) /* 0x97                     */ \
    IF_USE( DIGINI_USE_HW_CRC_8_WCDMA,        X_CRC( CRC_HW_8_WCDMA,        0x9B,       0x00,       0x00,       CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_8_BITS)) /* 0x25                     */ \
\
    IF_USE( DIGINI_USE_HW_CRC_16_ARC,         X_CRC( CRC_HW_16_ARC,         0x8005,     0x0000,     0x0000,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0xBB3D                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_AUG_CCITT,   X_CRC( CRC_HW_16_AUG_CCITT,   0x1021,     0x1D0F,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0xE5CC                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_BUYPASS,     X_CRC( CRC_HW_16_BUYPASS,     0x8005,     0x0000,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0xFEE8                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_CCITT_FALSE, X_CRC( CRC_HW_16_CCITT_FALSE, 0x1021,     0xFFFF,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0x29B1                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_CDMA2000,    X_CRC( CRC_HW_16_CDMA2000,    0xC867,     0xFFFF,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0x4C06                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_CMS,         X_CRC( CRC_HW_16_CMS,         0x8005,     0xFFFF,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0xAEE7                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_DDS_110,     X_CRC( CRC_HW_16_DDS_110,     0x8005,     0x800D,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0x9ECF                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_DECT_R,      X_CRC( CRC_HW_16_DECT_R,      0x0589,     0x0000,     0x0001,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0x007E                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_DECT_X,      X_CRC( CRC_HW_16_DECT_X,      0x0589,     0x0000,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0x007F                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_DNP,         X_CRC( CRC_HW_16_DNP,         0x3D65,     0x0000,     0xFFFF,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0xEA82                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_EN_13757,    X_CRC( CRC_HW_16_EN_13757,    0x3D65,     0x0000,     0xFFFF,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0xC2B3                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_GENIBUS,     X_CRC( CRC_HW_16_GENIBUS,     0x1021,     0xFFFF,     0xFFFF,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0xD64E                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_KERMIT,      X_CRC( CRC_HW_16_KERMIT,      0x1021,     0x0000,     0x0000,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0x2189                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_MAXIM,       X_CRC( CRC_HW_16_MAXIM,       0x8005,     0x0000,     0xFFFF,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0x44C2                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_MCRF4XX,     X_CRC( CRC_HW_16_MCRF4XX,     0x1021,     0xFFFF,     0x0000,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0x6F91                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_MODBUS,      X_CRC( CRC_HW_16_MODBUS,      0x8005,     0xFFFF,     0x0000,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0x4B37                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_RIELLO,      X_CRC( CRC_HW_16_RIELLO,      0x1021,     0xB2AA,     0x0000,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0x63D0                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_T10_DIF,     X_CRC( CRC_HW_16_T10_DIF,     0x8BB7,     0x0000,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0xD0DB                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_TELEDISK,    X_CRC( CRC_HW_16_TELEDISK,    0xA097,     0x0000,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0x0FB3                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_TMS37157,    X_CRC( CRC_HW_16_TMS37157,    0x1021,     0x89EC,     0x0000,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0x26B1                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_USB,         X_CRC( CRC_HW_16_USB,         0x8005,     0xFFFF,     0xFFFF,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0xB4C8                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_X25,         X_CRC( CRC_HW_16_X25,         0x1021,     0xFFFF,     0xFFFF,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0x906E                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_XMODEM,      X_CRC( CRC_HW_16_XMODEM,      0x1021,     0x0000,     0x0000,     CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_16_BITS)) /* 0x31C3                  */ \
    IF_USE( DIGINI_USE_HW_CRC_16_A,           X_CRC( CRC_HW_16_A,           0x1021,     0xC6C6,     0x0000,     CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_16_BITS)) /* 0xBF05                  */ \
\
    IF_USE( DIGINI_USE_HW_CRC32_BZIP2,        X_CRC( CRC_HW_32_BZIP2,       0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_32_BITS)) /* 0xFC891918              */ /* Also name AAl5, DECT-B, B-CRC-32 */\
    IF_USE( DIGINI_USE_HW_CRC32_MPEG_2,       X_CRC( CRC_HW_32_MPEG_2,      0x04C11DB7, 0xFFFFFFFF, 0x00000000, CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_32_BITS)) /* 0x0376E6E7              */ /* used by CRC module of STM32F1/F2/F4/L1 */\
    IF_USE( DIGINI_USE_HW_CRC32_POSIX,        X_CRC( CRC_HW_32_POSIX,       0x04C11DB7, 0x00000000, 0xFFFFFFFF, CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_32_BITS)) /* 0x765E7680              */ /* Also name CKSUM */\
    IF_USE( DIGINI_USE_HW_CRC32_SATA,         X_CRC( CRC_HW_32_SATA,        0x04C11DB7, 0x52325032, 0x00000000, CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_32_BITS)) /* 0xCF72AFE8              */ \
    IF_USE( DIGINI_USE_HW_CRC32,              X_CRC( CRC_HW_32,             0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0xCBF43926              */ \
    IF_USE( DIGINI_USE_HW_CRC32_AUTOSAR,      X_CRC( CRC_HW_32_AUTOSAR,     0xF4ACFB13, 0xFFFFFFFF, 0xFFFFFFFF, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0X1697D06A              */ \
    IF_USE( DIGINI_USE_HW_CRC32_B,            X_CRC( CRC_HW_32_B,           0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0x9AE0DAAF              */ \
    IF_USE( DIGINI_USE_HW_CRC32_C,            X_CRC( CRC_HW_32_C,           0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0xE3069283              */ /* Also name ISCSI, BASE91-C, CASTAGNOLI, INTERLAKEN */\
    IF_USE( DIGINI_USE_HW_CRC32_CD_ROM_EDC,   X_CRC( CRC_HW_32_CD_ROM_EDC,  0x8001801B, 0x00000000, 0x00000000, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0X6EC2EDC4              */ \
    IF_USE( DIGINI_USE_HW_CRC32_D,            X_CRC( CRC_HW_32_D,           0xA833982B, 0xFFFFFFFF, 0xFFFFFFFF, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0x87315576              */ /* Also name BASE91_D */\
    IF_USE( DIGINI_USE_HW_CRC32_ISO_HDLC,     X_CRC( CRC_HW_32_ISO_HDLC,    0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0xCBF43926              */ /* Also name ADCCP, ISO 3309, V.42, CRC-32-IEEEE, ETHERNET, XZ, PKZIP */\
    IF_USE( DIGINI_USE_HW_CRC32_JAMCRC,       X_CRC( CRC_HW_32_JAMCRC,      0x04C11DB7, 0xFFFFFFFF, 0x00000000, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0x340BC6D9              */ \
    IF_USE( DIGINI_USE_HW_CRC32_MEF,          X_CRC( CRC_HW_32_MEF,         0x741B8CD7, 0xFFFFFFFF, 0x00000000, CRC_REV_INPUT_DATA_BYTE, CRC_REV_OUTPUT_DATA_ENABLE,  CRC_POLY_SIZE_32_BITS)) /* 0xD2C22F51              */ \
    IF_USE( DIGINI_USE_HW_CRC32_Q,            X_CRC( CRC_HW_32_Q,           0x814141AB, 0x00000000, 0x00000000, CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_32_BITS)) /* 0x3010BF7F              */ /* Also name AIXM */\
    IF_USE( DIGINI_USE_HW_CRC32_XFER,         X_CRC( CRC_HW_32_XFER,        0x000000AF, 0x00000000, 0x00000000, CRC_REV_INPUT_DATA_NONE, CRC_REV_OUTPUT_DATA_DISABLE, CRC_POLY_SIZE_32_BITS)) /* 0xBD0BE338              */ \


#define CRC_POLY_SIZE_7_BITS            0x00000018
#define CRC_POLY_SIZE_8_BITS            0x00000010
#define CRC_POLY_SIZE_16_BITS           0x00000008
#define CRC_POLY_SIZE_32_BITS           0x00000000
                                              
#define CRC_REV_INPUT_DATA_NONE         0x00000000
#define CRC_REV_INPUT_DATA_BYTE         0x00000020
#define CRC_REV_INPUT_DATA_HALFWORD     0x00000040
#define CRC_REV_INPUT_DATA_WORD         0x00000060

#define CRC_REV_OUTPUT_DATA_DISABLE     0x00000000
#define CRC_REV_OUTPUT_DATA_ENABLE      0x00000080

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum CRC_HW_Type_e
{
    CRC_32_HW_METHOD_DEF(EXPAND_X_HW_CRC_AS_ENUM)
    NUMBER_OF_HW_CRC_METHOD,
};

struct CRC_HW_Info_t
{
    uint32_t    Polynomial;
    uint32_t    Init;
    uint32_t    XorOut;
    uint32_t    Mode;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CRC_Driver
{
    public:

        void        Initialize          (CRC_HW_Type_e Type);
        void        Start               (void);
        uint32_t    GetValue            (void);
        void        AddByte             (uint8_t Value);
        void        AddBuffer           (const uint8_t* pBuffer, size_t Length);
        uint32_t    CalculateBuffer     (const uint8_t* pBuffer, size_t Length);

    private:

        CRC_HW_Type_e                   m_Type;
        static const CRC_HW_Info_t      m_MethodList       [NUMBER_OF_HW_CRC_METHOD];
        static nOS_Mutex                m_Mutex;
        static bool                     m_MutexIsInitialize;
};

//-------------------------------------------------------------------------------------------------

