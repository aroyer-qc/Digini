//-------------------------------------------------------------------------------------------------
//
//  File : crc.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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
// REF: https://reveng.sourceforge.io/crc-catalogue/17plus.htm
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

#define CRC_METHOD_DEF(X_CRC)   /* Check is base on string "123456789" */ \
/*                                                Algorithm	          Poly                     Init	                    XorOut                   RefIn  RefOut Size      Check                   */ \
    IF_USE( DIGINI_USE_CRC_8,              X_CRC( CRC_8,              0x07,                    0x00,                    0x00,                    false, false, 8  ) ) /* 0xF4                    */ \
    IF_USE( DIGINI_USE_CRC_8_AUTOSAR,      X_CRC( CRC_8_AUTOSAR,      0x2F,                    0xFF,                    0xFF,                    false, false, 8  ) ) /* 0xDF                    */ \
    IF_USE( DIGINI_USE_CRC_8_BLUETOOTH,    X_CRC( CRC_8_BLUETOOTH,    0xA7,                    0x00,                    0x00,                    true,  true,  8  ) ) /* 0x26                    */ \
    IF_USE( DIGINI_USE_CRC_8_CDMA2000,     X_CRC( CRC_8_CDMA2000,     0x9B,                    0xFF,                    0x00,                    false, false, 8  ) ) /* 0xDA                    */ \
    IF_USE( DIGINI_USE_CRC_8_DARC,         X_CRC( CRC_8_DARC,         0x39,                    0x00,                    0x00,                    true,  true,  8  ) ) /* 0x15                    */ \
    IF_USE( DIGINI_USE_CRC_8_DVB_S2,       X_CRC( CRC_8_DVB_S2,       0xD5,                    0x00,                    0x00,                    false, false, 8  ) ) /* 0xBC                    */ \
    IF_USE( DIGINI_USE_CRC_8_EBU,          X_CRC( CRC_8_EBU,          0x1D,                    0xFF,                    0x00,                    true,  true,  8  ) ) /* 0x97                    */ \
    IF_USE( DIGINI_USE_CRC_8_GSM_A,        X_CRC( CRC_8_GSM_A,        0x1D,                    0x00,                    0x00,                    false, false, 8  ) ) /* 0x37                    */ \
    IF_USE( DIGINI_USE_CRC_8_GSM_B,        X_CRC( CRC_8_GSM_B,        0x49,                    0x00,                    0xFF,                    false, false, 8  ) ) /* 0x94                    */ \
    IF_USE( DIGINI_USE_CRC_8_HITAG,        X_CRC( CRC_8_HITAG,        0x1D,                    0xFF,                    0x00,                    false, false, 8  ) ) /* 0xB4                    */ \
    IF_USE( DIGINI_USE_CRC_8_I_CODE,       X_CRC( CRC_8_I_CODE,       0x1D,                    0xFD,                    0x00,                    false, false, 8  ) ) /* 0x7E                    */ \
    IF_USE( DIGINI_USE_CRC_8_I_432_1,      X_CRC( CRC_8_I_432_1,      0x07,                    0x00,                    0x55,                    false, false, 8  ) ) /* 0xA1                    */ \
    IF_USE( DIGINI_USE_CRC_8_ITU,          X_CRC( CRC_8_ITU,          0x07,                    0x00,                    0x55,                    false, false, 8  ) ) /* 0xA1                    */ \
    IF_USE( DIGINI_USE_CRC_8_LTE,          X_CRC( CRC_8_LTE,          0x9B,                    0x00,                    0x00,                    false, false, 8  ) ) /* 0xEA                    */ \
    IF_USE( DIGINI_USE_CRC_8_MAXIM,        X_CRC( CRC_8_MAXIM,        0x31,                    0x00,                    0x00,                    true,  true,  8  ) ) /* 0xA1                    */ \
    IF_USE( DIGINI_USE_CRC_8_MIFARE_MAD,   X_CRC( CRC_8_MIFARE_MAD,   0x1D,                    0xC7,                    0x00,                    false, false, 8  ) ) /* 0x99                    */ \
    IF_USE( DIGINI_USE_CRC_8_NRSC_5,       X_CRC( CRC_8_NRSC_5,       0x31,                    0xFF,                    0x00,                    false, false, 8  ) ) /* 0xF7                    */ \
    IF_USE( DIGINI_USE_CRC_8_OPENSAFETY,   X_CRC( CRC_8_OPENSAFETY,   0x2F,                    0x00,                    0x00,                    false, false, 8  ) ) /* 0x3E                    */ \
    IF_USE( DIGINI_USE_CRC_8_ROHC,         X_CRC( CRC_8_ROHC,         0x07,                    0xFF,                    0x00,                    true,  true,  8  ) ) /* 0xD0                    */ \
    IF_USE( DIGINI_USE_CRC_8_SAE_J1850,    X_CRC( CRC_8_SAE_J1850,    0x1D,                    0xFF,                    0xFF,                    false, false, 8  ) ) /* 0x4B                    */ \
    IF_USE( DIGINI_USE_CRC_8_SMBUS,        X_CRC( CRC_8_SMBUS,        0x07,                    0x00,                    0x00,                    false, false, 8  ) ) /* 0xF4                    */ \
    IF_USE( DIGINI_USE_CRC_8_TECH_3250,    X_CRC( CRC_8_TECH_3250,    0x1D,                    0xFF,                    0x00,                    true,  true,  8  ) ) /* 0x97                    */ \
    IF_USE( DIGINI_USE_CRC_8_WCDMA,        X_CRC( CRC_8_WCDMA,        0x9B,                    0x00,                    0x00,                    true,  true,  8  ) ) /* 0x25                    */ \
    IF_USE( DIGINI_USE_CRC_10_ATM,         X_CRC( CRC_10_ATM,         0x233,                   0x000,                   0x000,                   false, false, 10 ) ) /* 0x199                   */ \
    IF_USE( DIGINI_USE_CRC_10_CDMA2000,    X_CRC( CRC_10_CDMA2000,    0x3D9,                   0x3FF,                   0x000,                   false, false, 10 ) ) /* 0x233                   */ \
    IF_USE( DIGINI_USE_CRC_10_GSM,         X_CRC( CRC_10_GSM,         0x175,                   0x000,                   0x3FF,                   false, false, 10 ) ) /* 0x12A                   */ \
    IF_USE( DIGINI_USE_CRC_11_FLEXRAY,     X_CRC( CRC_11_FLEXRAY,     0x385,                   0x01A,                   0x000,                   false, false, 11 ) ) /* 0x5A3                   */ \
    IF_USE( DIGINI_USE_CRC_11_UMTS,        X_CRC( CRC_11_UMTS,        0x307,                   0x000,                   0x000,                   false, false, 11 ) ) /* 0x061                   */ \
    IF_USE( DIGINI_USE_CRC_12_CDMA2000,    X_CRC( CRC_12_CDMA2000,    0xF13,                   0xFFF,                   0x000,                   false, false, 12 ) ) /* 0xD4D                   */ \
    IF_USE( DIGINI_USE_CRC_12_DECT,        X_CRC( CRC_12_DECT,        0x80F,                   0x000,                   0x000,                   false, false, 12 ) ) /* 0xF5B                   */ \
    IF_USE( DIGINI_USE_CRC_12_GSM,         X_CRC( CRC_12_GSM,         0xD31,                   0x000,                   0xFFF,                   false, false, 12 ) ) /* 0xB34                   */ \
    IF_USE( DIGINI_USE_CRC_12_UMTS,        X_CRC( CRC_12_UMTS,        0x80F,                   0x000,                   0x000,                   false, true,  12 ) ) /* 0xDAF                   */ \
    IF_USE( DIGINI_USE_CRC_13_BBC,         X_CRC( CRC_13_BBC,         0x1CF5,                  0x0000,                  0x0000,                  false, false, 13 ) ) /* 0x04FA                  */ \
    IF_USE( DIGINI_USE_CRC_14_DARC,        X_CRC( CRC_14_DARC,        0x0805,                  0x0000,                  0x0000,                  true,  true,  14 ) ) /* 0x082D                  */ \
    IF_USE( DIGINI_USE_CRC_14_GSM,         X_CRC( CRC_14_GSM,         0x202D,                  0x0000,                  0x3FFF,                  false, false, 14 ) ) /* 0x30AE                  */ \
    IF_USE( DIGINI_USE_CRC_15_CAN,         X_CRC( CRC_15_CAN,         0x4599,                  0x0000,                  0x0000,                  false, false, 15 ) ) /* 0x059E                  */ \
    IF_USE( DIGINI_USE_CRC_15_MPT1327,     X_CRC( CRC_15_MPT1327,     0x6815,                  0x0000,                  0x0001,                  false, false, 15 ) ) /* 0x2566                  */ \
    IF_USE( DIGINI_USE_CRC_16_ARC,         X_CRC( CRC_16_ARC,         0x8005,                  0x0000,                  0x0000,                  true,  true,  16 ) ) /* 0xBB3D                  */ \
    IF_USE( DIGINI_USE_CRC_16_AUG_CCITT,   X_CRC( CRC_16_AUG_CCITT,   0x1021,                  0x1D0F,                  0x0000,                  false, false, 16 ) ) /* 0xE5CC                  */ \
    IF_USE( DIGINI_USE_CRC_16_BUYPASS,     X_CRC( CRC_16_BUYPASS,     0x8005,                  0x0000,                  0x0000,                  false, false, 16 ) ) /* 0xFEE8                  */ \
    IF_USE( DIGINI_USE_CRC_16_CCITT_FALSE, X_CRC( CRC_16_CCITT_FALSE, 0x1021,                  0xFFFF,                  0x0000,                  false, false, 16 ) ) /* 0x29B1                  */ \
    IF_USE( DIGINI_USE_CRC_16_CDMA2000,    X_CRC( CRC_16_CDMA2000,    0xC867,                  0xFFFF,                  0x0000,                  false, false, 16 ) ) /* 0x4C06                  */ \
    IF_USE( DIGINI_USE_CRC_16_CMS,         X_CRC( CRC_16_CMS,         0x8005,                  0xFFFF,                  0x0000,                  false, false, 16 ) ) /* 0xxxxx                  */ \
    IF_USE( DIGINI_USE_CRC_16_DDS_110,     X_CRC( CRC_16_DDS_110,     0x8005,                  0x800D,                  0x0000,                  false, false, 16 ) ) /* 0x9ECF                  */ \
    IF_USE( DIGINI_USE_CRC_16_DECT_R,      X_CRC( CRC_16_DECT_R,      0x0589,                  0x0000,                  0x0001,                  false, false, 16 ) ) /* 0x007E                  */ \
    IF_USE( DIGINI_USE_CRC_16_DECT_X,      X_CRC( CRC_16_DECT_X,      0x0589,                  0x0000,                  0x0000,                  false, false, 16 ) ) /* 0x007F                  */ \
    IF_USE( DIGINI_USE_CRC_16_DNP,         X_CRC( CRC_16_DNP,         0x3D65,                  0x0000,                  0xFFFF,                  true,  true,  16 ) ) /* 0xEA82                  */ \
    IF_USE( DIGINI_USE_CRC_16_EN_13757,    X_CRC( CRC_16_EN_13757,    0x3D65,                  0x0000,                  0xFFFF,                  false, false, 16 ) ) /* 0xC2B3                  */ \
    IF_USE( DIGINI_USE_CRC_16_GENIBUS,     X_CRC( CRC_16_GENIBUS,     0x1021,                  0xFFFF,                  0xFFFF,                  false, false, 16 ) ) /* 0xD64E                  */ \
    IF_USE( DIGINI_USE_CRC_16_KERMIT,      X_CRC( CRC_16_KERMIT,      0x1021,                  0x0000,                  0x0000,                  true,  true,  16 ) ) /* 0x2189                  */ \
    IF_USE( DIGINI_USE_CRC_16_MAXIM,       X_CRC( CRC_16_MAXIM,       0x8005,                  0x0000,                  0xFFFF,                  true,  true,  16 ) ) /* 0x44C2                  */ \
    IF_USE( DIGINI_USE_CRC_16_MCRF4XX,     X_CRC( CRC_16_MCRF4XX,     0x1021,                  0xFFFF,                  0x0000,                  true,  true,  16 ) ) /* 0x6F91                  */ \
    IF_USE( DIGINI_USE_CRC_16_MODBUS,      X_CRC( CRC_16_MODBUS,      0x8005,                  0xFFFF,                  0x0000,                  true,  true,  16 ) ) /* 0x4B37                  */ \
    IF_USE( DIGINI_USE_CRC_16_RIELLO,      X_CRC( CRC_16_RIELLO,      0x1021,                  0xB2AA,                  0x0000,                  true,  true,  16 ) ) /* 0x63D0                  */ \
    IF_USE( DIGINI_USE_CRC_16_T10_DIF,     X_CRC( CRC_16_T10_DIF,     0x8BB7,                  0x0000,                  0x0000,                  false, false, 16 ) ) /* 0xD0DB                  */ \
    IF_USE( DIGINI_USE_CRC_16_TELEDISK,    X_CRC( CRC_16_TELEDISK,    0xA097,                  0x0000,                  0x0000,                  false, false, 16 ) ) /* 0x0FB3                  */ \
    IF_USE( DIGINI_USE_CRC_16_TMS37157,    X_CRC( CRC_16_TMS37157,    0x1021,                  0x89EC,                  0x0000,                  true,  true,  16 ) ) /* 0x26B1                  */ \
    IF_USE( DIGINI_USE_CRC_16_USB,         X_CRC( CRC_16_USB,         0x8005,                  0xFFFF,                  0xFFFF,                  true,  true,  16 ) ) /* 0xB4C8                  */ \
    IF_USE( DIGINI_USE_CRC_16_X25,         X_CRC( CRC_16_X25,         0x1021,                  0xFFFF,                  0xFFFF,                  true,  true,  16 ) ) /* 0x906E                  */ \
    IF_USE( DIGINI_USE_CRC_16_XMODEM,      X_CRC( CRC_16_XMODEM,      0x1021,                  0x0000,                  0x0000,                  false, false, 16 ) ) /* 0x31C3                  */ \
    IF_USE( DIGINI_USE_CRC_16_A,           X_CRC( CRC_16_A,           0x1021,                  0xC6C6,                  0x0000,                  true,  true,  16 ) ) /* 0xBF05                  */ \
    IF_USE( DIGINI_USE_CRC_17_CAN_FD,      X_CRC( CRC_17_CAN_FD,      0x1685B,                 0x00000,                 0x00000,                 false, false, 17 ) ) /* 0x04F03                 */ \
    IF_USE( DIGINI_USE_CRC_21_CAN_FD,      X_CRC( CRC_21_CAN_FD,      0x102899,                0x000000,                0x000000,                false, false, 21 ) ) /* 0x0ED841                */ \
    IF_USE( DIGINI_USE_CRC_24_BLE,         X_CRC( CRC_24_BLE,         0x00065B,                0x555555,                0x000000,                true,  true,  24 ) ) /* 0xC25A56                */ \
    IF_USE( DIGINI_USE_CRC_24_FLEXRAY_A,   X_CRC( CRC_24_FLEXRAY_A,   0x5D6DCB,                0xFEDCBA,                0x000000,                false, false, 24 ) ) /* 0x7979BD                */ \
    IF_USE( DIGINI_USE_CRC_24_FLEXRAY_B,   X_CRC( CRC_24_FLEXRAY_B,   0x5D6DCB,                0xABCDEF,                0x000000,                false, false, 24 ) ) /* 0x1F23B8                */ \
    IF_USE( DIGINI_USE_CRC_24_INTERLAKEN,  X_CRC( CRC_24_INTERLAKEN,  0x328B63,                0xFFFFFF,                0xFFFFFF,                false, false, 24 ) ) /* 0xB4F3E6                */ \
    IF_USE( DIGINI_USE_CRC_24_LTE_A,       X_CRC( CRC_24_LTE_A,       0x864CFB,                0x000000,                0x000000,                false, false, 24 ) ) /* 0xCDE703                */ \
    IF_USE( DIGINI_USE_CRC_24_LTE_B,       X_CRC( CRC_24_LTE_B,       0x800063,                0x000000,                0x000000,                false, false, 24 ) ) /* 0x23EF52                */ \
    IF_USE( DIGINI_USE_CRC_24_OPENPGP,     X_CRC( CRC_24_OPENPGP,     0x864CFB,                0xB704CE,                0x000000,                false, false, 24 ) ) /* 0x21CF02                */ \
    IF_USE( DIGINI_USE_CRC_24_OS_9,        X_CRC( CRC_24_OS_9,        0x800063,                0xFFFFFF,                0xFFFFFF,                false, false, 24 ) ) /* 0x200FA5                */ \
    IF_USE( DIGINI_USE_CRC_30_CDMA,        X_CRC( CRC_30_CDMA,        0x2030B9C7,              0x3FFFFFFF,              0x3FFFFFFF,              false, false, 30 ) ) /* 0x04C34ABF              */ \
    IF_USE( DIGINI_USE_CRC_31_PHILIPS,     X_CRC( CRC_31_PHILIPS,     0x00411DB7,              0x7FFFFFFF,              0x7FFFFFFF,              false, false, 31 ) ) /* 0x0CE9E46C              */ \
    IF_USE( DIGINI_USE_CRC_32,             X_CRC( CRC_32,             0x04C11DB7,              0xFFFFFFFF,              0xFFFFFFFF,              true,  true,  32 ) ) /* 0xCBF43926              */ \
    IF_USE( DIGINI_USE_CRC_32_AUTOSAR,     X_CRC( CRC_32_AUTOSAR,     0xF4ACFB13,              0xFFFFFFFF,              0xFFFFFFFF,              true,  true,  32 ) ) /* 0X1697D06A              */ \
    IF_USE( DIGINI_USE_CRC_32_BZIP2,       X_CRC( CRC_32_BZIP2,       0x04C11DB7,              0xFFFFFFFF,              0xFFFFFFFF,              false, false, 32 ) ) /* 0xFC891918              */ \
    IF_USE( DIGINI_USE_CRC_32_C,           X_CRC( CRC_32_C,           0x1EDC6F41,              0xFFFFFFFF,              0xFFFFFFFF,              true,  true,  32 ) ) /* 0xE3069283              */ /* Also name ISCSI    */\
    IF_USE( DIGINI_USE_CRC_32_CD_ROM_EDC,  X_CRC( CRC_32_CD_ROM_EDC,  0x8001801B,              0x00000000,              0x00000000,              true,  true,  32 ) ) /* 0X6EC2EDC4              */ \
    IF_USE( DIGINI_USE_CRC_32_D,           X_CRC( CRC_32_D,           0xA833982B,              0xFFFFFFFF,              0xFFFFFFFF,              true,  true,  32 ) ) /* 0x87315576              */ /* Also name BASE91_D */\
    IF_USE( DIGINI_USE_CRC_32_ISO_HDLC,    X_CRC( CRC_32_ISO_HDLC,    0x04C11DB7,              0xFFFFFFFF,              0xFFFFFFFF,              true,  true,  32 ) ) /* 0xCBF43926              */ \
    IF_USE( DIGINI_USE_CRC_32_JAMCRC,      X_CRC( CRC_32_JAMCRC,      0x04C11DB7,              0xFFFFFFFF,              0x00000000,              true,  true,  32 ) ) /* 0x340BC6D9              */ \
    IF_USE( DIGINI_USE_CRC_32_MEF,         X_CRC( CRC_32_MEF,         0x741B8CD7,              0xFFFFFFFF,              0x00000000,              true,  true,  32 ) ) /* 0xD2C22F51              */ \
    IF_USE( DIGINI_USE_CRC_32_MPEG_2,      X_CRC( CRC_32_MPEG_2,      0x04C11DB7,              0xFFFFFFFF,              0x00000000,              false, false, 32 ) ) /* 0x0376E6E7              */ \
    IF_USE( DIGINI_USE_CRC_32_POSIX,       X_CRC( CRC_32_POSIX,       0x04C11DB7,              0x00000000,              0xFFFFFFFF,              false, false, 32 ) ) /* 0x765E7680              */ /* Also name CKSUM    */\
    IF_USE( DIGINI_USE_CRC_32_Q,           X_CRC( CRC_32_Q,           0x814141AB,              0x00000000,              0x00000000,              false, false, 32 ) ) /* 0x3010BF7F              */ \
    IF_USE( DIGINI_USE_CRC_32_SATA,        X_CRC( CRC_32_SATA,        0x04C11DB7,              0x52325032,              0x00000000,              false, false, 32 ) ) /* 0xCF72AFE8              */ \
    IF_USE( DIGINI_USE_CRC_32_XFER,        X_CRC( CRC_32_XFER,        0x000000AF,              0x00000000,              0x00000000,              false, false, 32 ) ) /* 0xBD0BE338              */ \
    


#if 0 // to be implemented if needed (need different code)
    IF_USE( DIGINI_USE_CRC_3_GSM,          X_CRC( CRC_3_GSM,          0x3,                     0x0,                     0x7,                     false, false, 3  ) ) /* 0x4                     */ \
    IF_USE( DIGINI_USE_CRC_3_ROHC,         X_CRC( CRC_3_ROHC,         0x3,                     0x7,                     0x0,                     true,  true,  3  ) ) /* 0x6                     */ \
    IF_USE( DIGINI_USE_CRC_4_G_704,        X_CRC( CRC_4_G_704,        0x3,                     0x0,                     0x0,                     true,  true,  4  ) ) /* 0x7                     */ \
    IF_USE( DIGINI_USE_CRC_4_INTERLAKEN,   X_CRC( CRC_4_INTERLAKEN,   0x3,                     0xF,                     0xF,                     false, false, 4  ) ) /* 0xB                     */ \
    IF_USE( DIGINI_USE_CRC_5_EPC_C1G2,     X_CRC( CRC_5_EPC_C1G2,     0x09,                    0x09,                    0x00,                    false, false, 5  ) ) /* 0x00                    */ \
    IF_USE( DIGINI_USE_CRC_5_G_704,        X_CRC( CRC_5_G_704,        0x15,                    0x00,                    0x00,                    true,  true,  5  ) ) /* 0x07                    */ \
    IF_USE( DIGINI_USE_CRC_5_USB,          X_CRC( CRC_5_USB,          0x05,                    0x1F,                    0x1F,                    true,  true,  5  ) ) /* 0x19                    */ \
    IF_USE( DIGINI_USE_CRC_6_CDMA2000_A,   X_CRC( CRC_6_CDMA2000_A,   0x27,                    0x3F,                    0x00,                    false, false, 6  ) ) /* 0x0D                    */ \
    IF_USE( DIGINI_USE_CRC_6_CDMA2000_B,   X_CRC( CRC_6_CDMA2000_B,   0x07,                    0x3F,                    0x00,                    false, false, 6  ) ) /* 0x3B                    */ \
    IF_USE( DIGINI_USE_CRC_6_DARC,         X_CRC( CRC_6_DARC,         0x19,                    0x00,                    0x00,                    true,  true,  6  ) ) /* 0x26                    */ \
    IF_USE( DIGINI_USE_CRC_6_G_704,        X_CRC( CRC_6_G_704,        0x03,                    0x00,                    0x00,                    true,  true,  6  ) ) /* 0x06                    */ \
    IF_USE( DIGINI_USE_CRC_6_GSM,          X_CRC( CRC_6_GSM,          0x2F,                    0x00,                    0x3F,                    false, false, 6  ) ) /* 0x13                    */ \
    IF_USE( DIGINI_USE_CRC_7_MMC,          X_CRC( CRC_7_MMC,          0x09,                    0x00,                    0x00,                    false, false, 7  ) ) /* 0x75                    */ \
    IF_USE( DIGINI_USE_CRC_7_ROHC,         X_CRC( CRC_7_ROHC,         0x4F,                    0x7F,                    0x00,                    true,  true,  7  ) ) /* 0x53                    */ \
    IF_USE( DIGINI_USE_CRC_7_UMTS,         X_CRC( CRC_7_UMTS,         0x45,                    0x00,                    0x00,                    false, false, 7  ) ) /* 0x61                    */ \
    IF_USE( DIGINI_USE_CRC_40_GSM,         X_CRC( CRC_40_GSM,         0x0004820009,            0x0000000000,            0xFFFFFFFFFF,            false, false, 40 ) ) /* 0xD4164FC646            */ \
    IF_USE( DIGINI_USE_CRC_64_ECMA_182,    X_CRC( CRC_64_ECMA_182,    0x42F0E1EBA9EA3693,      0x0000000000000000,      0x0000000000000000,      false, false, 64 ) ) /* 0x6C40DF5F0B497347      */ \
    IF_USE( DIGINI_USE_CRC_64_GO_ISO,      X_CRC( CRC_64_GO_ISO,      0x000000000000001B,      0xFFFFFFFFFFFFFFFF,      0xFFFFFFFFFFFFFFFF,      true,  true,  64 ) ) /* 0xB90956C775A41001      */ \
    IF_USE( DIGINI_USE_CRC_64_MS,          X_CRC( CRC_64_MS,          0x259c84CBA6426349,      0xFFFFFFFFFFFFFFFF,      0x0000000000000000,      true,  true,  64 ) ) /* 0x75D4B74F024ECEEA      */ \
    IF_USE( DIGINI_USE_CRC_64_WE,          X_CRC( CRC_64_WE,          0x42f0E1EBA9EA3693,      0xFFFFFFFFFFFFFFFF,      0xFFFFFFFFFFFFFFFF,      false, false, 64 ) ) /* 0x62EC59E3F1A4F00A      */ \
    IF_USE( DIGINI_USE_CRC_64_XZ,          X_CRC( CRC_64_XZ,          0x42T0E1EBA9EA3693,      0xFFFFFFFFFFFFFFFF,      0xFFFFFFFFFFFFFFFF,      true,  true,  64 ) ) /* 0x995DC9BBDF1939FA      */ \
    IF_USE( DIGINI_USE_CRC_82_DARC,        X_CRC( CRC_82_DARC,        0x0308C0111011401440411, 0x000000000000000000000, 0x000000000000000000000, true,  true,  82 ) ) /* 0x09EA83F625023801FD612 */ \



#endif

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

                    CRC_Calc            (CRC_Type_e Type);
                   ~CRC_Calc            ();                         // destructor does something only if hardware module exist

        void        Start               (void);
        uint32_t    Done                (void);
        void        Calculate           (uint8_t Value);
        void        CalculateBuffer     (const uint8_t *pBuffer, size_t Length);
        uint32_t    CalculateFullBuffer (const uint8_t *pBuffer, size_t Length);

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

