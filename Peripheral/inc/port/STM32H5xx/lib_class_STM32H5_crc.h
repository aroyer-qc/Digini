//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_crc.h
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "crc_cfg.h"

//-------------------------------------------------------------------------------------------------
// Error Checking
//-------------------------------------------------------------------------------------------------

#ifndef CRC_USE_CRC8
#error "CRC_USE_CRC8   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_AUTOSAR
#error "CRC_USE_CRC8_AUTOSAR   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_BLUETOOTH
#error "CRC_USE_CRC8_BLUETOOTH   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_CDMA2000
#error "CRC_USE_CRC8_CDMA2000   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_DARC
#error "CRC_USE_CRC8_DARC   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_DVB_S2
#error "CRC_USE_CRC8_DVB_S2   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_EBU
#error "CRC_USE_CRC8_EBU   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_GSM_A
#error "CRC_USE_CRC8_GSM_A   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_GSM_B
#error "CRC_USE_CRC8_GSM_B   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_HITAG
#error "CRC_USE_CRC8_HITAG   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_I_CODE
#error "CRC_USE_CRC8_I_CODE   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_I_432_1
#error "CRC_USE_CRC8_I_432_1   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_ITU
#error "CRC_USE_CRC8_ITU   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_LTE
#error "CRC_USE_CRC8_LTE   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_MAXIM
#error "CRC_USE_CRC8_MAXIM   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_MIFARE_MAD
#error "CRC_USE_CRC8_MIFARE_MAD   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_NRSC_5
#error "CRC_USE_CRC8_NRSC_5   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_OPENSAFETY
#error "CRC_USE_CRC8_OPENSAFETY   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_ROHC
#error "CRC_USE_CRC8_ROHC   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_SAE_J1850
#error "CRC_USE_CRC8_SAE_J1850   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_SMBUS
#error "CRC_USE_CRC8_SMBUS   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_TECH_3250
#error "CRC_USE_CRC8_TECH_3250   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC8_WCDMA
#error "CRC_USE_CRC8_WCDMA   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_ARC
#error "CRC_USE_CRC16_ARC   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_AUG_CCITT
#error "CRC_USE_CRC16_AUG_CCITT   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_BUYPASS
#error "CRC_USE_CRC16_BUYPASS   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_CCITT_FALSE
#error "CRC_USE_CRC16_CCITT_FALSE   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_CDMA2000
#error "CRC_USE_CRC16_CDMA2000   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_CMS
#error "CRC_USE_CRC16_CMS   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_DDS_110
#error "CRC_USE_CRC16_DDS_110   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_DECT_R
#error "CRC_USE_CRC16_DECT_R   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_DECT_X
#error "CRC_USE_CRC16_DECT_X   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_DNP
#error "CRC_USE_CRC16_DNP   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_EN_13757
#error "CRC_USE_CRC16_EN_13757   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_GENIBUS
#error "CRC_USE_CRC16_GENIBUS   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_KERMIT
#error "CRC_USE_CRC16_KERMIT   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_MAXIM
#error "CRC_USE_CRC16_MAXIM   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_MCRF4XX
#error "CRC_USE_CRC16_MCRF4XX   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_MODBUS
#error "CRC_USE_CRC16_MODBUS   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_RIELLO
#error "CRC_USE_CRC16_RIELLO   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_T10_DIF
#error "CRC_USE_CRC16_T10_DIF   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_TELEDISK
#error "CRC_USE_CRC16_TELEDISK   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_TMS37157
#error "CRC_USE_CRC16_TMS37157   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_USB
#error "CRC_USE_CRC16_USB   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_X25
#error "CRC_USE_CRC16_X25   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_XMODEM
#error "CRC_USE_CRC16_XMODEM   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC16_A
#error "CRC_USE_CRC16_A   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_BZIP2
#error "CRC_USE_CRC32_BZIP2   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_MPEG_2
#error "CRC_USE_CRC32_MPEG_2   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_POSIX
#error "CRC_USE_CRC32_POSIX   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_SATA
#error "CRC_USE_CRC32_SATA   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32
#error "CRC_USE_CRC32   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_AUTOSAR
#error "CRC_USE_CRC32_AUTOSAR   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_B
#error "CRC_USE_CRC32_B   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_C
#error "CRC_USE_CRC32_C   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_CD_ROM_EDC
#error "CRC_USE_CRC32_CD_ROM_EDC   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_D
#error "CRC_USE_CRC32_D   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_ISO_HDLC
#error "CRC_USE_CRC32_ISO_HDLC   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_JAMCRC
#error "CRC_USE_CRC32_JAMCRC   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_MEF
#error "CRC_USE_CRC32_MEF   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_Q
#error "CRC_USE_CRC32_Q   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif
#ifndef CRC_USE_CRC32_XFER
#error "CRC_USE_CRC32_XFER   Must be define as DEF_ENABLED or DEF_DISABLED"
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_CRC_AS_ENUM(ENUM_ID, POLY, INIT, XOR_OUT, REF_IN, REF_OUT, WIDTH)          ENUM_ID,
#define EXPAND_X_CRC_AS_CLASS_CONST(ENUM_ID, POLY, INIT, XOR_OUT, REF_IN, REF_OUT, WIDTH)   {POLY, INIT, XOR_OUT, (REF_IN | REF_OUT | WIDTH)},

#define CRC_METHOD_DEF(X_CRC)   /* Check is base on string "123456789ABC" for size 9,10,11,12 */ \
/*                                            Algorithm	         Poly,       Init        XorOut      Reflect Data In,              Reflect Data Out,                 Poly Size,              Check size 9        Chek Size 10        Check Size 11       Check Size 12                   */ \
    IF_USE( CRC_USE_CRC8,              X_CRC( CRC8,              0x07,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xF4                                                                                        */ \
    IF_USE( CRC_USE_CRC8_AUTOSAR,      X_CRC( CRC8_AUTOSAR,      0x2F,       0xFF,       0xFF,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xDF                                                                                        */ \
    IF_USE( CRC_USE_CRC8_BLUETOOTH,    X_CRC( CRC8_BLUETOOTH,    0xA7,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_8B)) /*  0x26                                                                                        */ \
    IF_USE( CRC_USE_CRC8_CDMA2000,     X_CRC( CRC8_CDMA2000,     0x9B,       0xFF,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xDA                                                                                        */ \
    IF_USE( CRC_USE_CRC8_DARC,         X_CRC( CRC8_DARC,         0x39,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_8B)) /*  0x15                                                                                        */ \
    IF_USE( CRC_USE_CRC8_DVB_S2,       X_CRC( CRC8_DVB_S2,       0xD5,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xBC                                                                                        */ \
    IF_USE( CRC_USE_CRC8_EBU,          X_CRC( CRC8_EBU,          0x1D,       0xFF,       0x00,       CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_8B)) /*  0x97                                                                                        */ \
    IF_USE( CRC_USE_CRC8_GSM_A,        X_CRC( CRC8_GSM_A,        0x1D,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0x37                                                                                        */ \
    IF_USE( CRC_USE_CRC8_GSM_B,        X_CRC( CRC8_GSM_B,        0x49,       0x00,       0xFF,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0x94                                                                                        */ \
    IF_USE( CRC_USE_CRC8_HITAG,        X_CRC( CRC8_HITAG,        0x1D,       0xFF,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xB4                                                                                        */ \
    IF_USE( CRC_USE_CRC8_I_CODE,       X_CRC( CRC8_I_CODE,       0x1D,       0xFD,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0x7E                                                                                        */ \
    IF_USE( CRC_USE_CRC8_I_432_1,      X_CRC( CRC8_I_432_1,      0x07,       0x00,       0x55,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xA1                                                                                        */ \
    IF_USE( CRC_USE_CRC8_ITU,          X_CRC( CRC8_ITU,          0x07,       0x00,       0x55,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xA1                                                                                        */ \
    IF_USE( CRC_USE_CRC8_LTE,          X_CRC( CRC8_LTE,          0x9B,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xEA                                                                                        */ \
    IF_USE( CRC_USE_CRC8_MAXIM,        X_CRC( CRC8_MAXIM,        0x31,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_8B)) /*  0xA1                                                                                        */ \
    IF_USE( CRC_USE_CRC8_MIFARE_MAD,   X_CRC( CRC8_MIFARE_MAD,   0x1D,       0xC7,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0x99                                                                                        */ \
    IF_USE( CRC_USE_CRC8_NRSC_5,       X_CRC( CRC8_NRSC_5,       0x31,       0xFF,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xF7                                                                                        */ \
    IF_USE( CRC_USE_CRC8_OPENSAFETY,   X_CRC( CRC8_OPENSAFETY,   0x2F,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0x3E                                                                                        */ \
    IF_USE( CRC_USE_CRC8_ROHC,         X_CRC( CRC8_ROHC,         0x07,       0xFF,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_8B)) /*  0xD0                                                                                        */ \
    IF_USE( CRC_USE_CRC8_SAE_J1850,    X_CRC( CRC8_SAE_J1850,    0x1D,       0xFF,       0xFF,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0x4B                                                                                        */ \
    IF_USE( CRC_USE_CRC8_SMBUS,        X_CRC( CRC8_SMBUS,        0x07,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_8B)) /*  0xF4                                                                                        */ \
    IF_USE( CRC_USE_CRC8_TECH_3250,    X_CRC( CRC8_TECH_3250,    0x1D,       0xFF,       0x00,       CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_8B)) /*  0x97                                                                                        */ \
    IF_USE( CRC_USE_CRC8_WCDMA,        X_CRC( CRC8_WCDMA,        0x9B,       0x00,       0x00,       CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_8B)) /*  0x25                                                                                        */ \
/*                                            Algorithm	         Poly,       Init        XorOut      Reflect Data In,              Reflect Data Out,                 Poly Size,              Check size 9        Chek Size 10        Check Size 11       Check Size 12                   */ \
    IF_USE( CRC_USE_CRC16_ARC,         X_CRC( CRC16_ARC,         0x8005,     0x0000,     0x0000,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0xBB3D                                                                                      */ \
    IF_USE( CRC_USE_CRC16_AUG_CCITT,   X_CRC( CRC16_AUG_CCITT,   0x1021,     0x1D0F,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0xE5CC                                                                                      */ \
    IF_USE( CRC_USE_CRC16_BUYPASS,     X_CRC( CRC16_BUYPASS,     0x8005,     0x0000,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0xFEE8                                                                                      */ \
    IF_USE( CRC_USE_CRC16_CCITT_FALSE, X_CRC( CRC16_CCITT_FALSE, 0x1021,     0xFFFF,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0x29B1                                                                                      */ \
    IF_USE( CRC_USE_CRC16_CDMA2000,    X_CRC( CRC16_CDMA2000,    0xC867,     0xFFFF,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0x4C06                                                                                      */ \
    IF_USE( CRC_USE_CRC16_CMS,         X_CRC( CRC16_CMS,         0x8005,     0xFFFF,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0xAEE7                                                                                      */ \
    IF_USE( CRC_USE_CRC16_DDS_110,     X_CRC( CRC16_DDS_110,     0x8005,     0x800D,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0x9ECF                                                                                      */ \
    IF_USE( CRC_USE_CRC16_DECT_R,      X_CRC( CRC16_DECT_R,      0x0589,     0x0000,     0x0001,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0x007E                                                                                      */ \
    IF_USE( CRC_USE_CRC16_DECT_X,      X_CRC( CRC16_DECT_X,      0x0589,     0x0000,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0x007F                                                                                      */ \
    IF_USE( CRC_USE_CRC16_DNP,         X_CRC( CRC16_DNP,         0x3D65,     0x0000,     0xFFFF,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0xEA82                                                                                      */ \
    IF_USE( CRC_USE_CRC16_EN_13757,    X_CRC( CRC16_EN_13757,    0x3D65,     0x0000,     0xFFFF,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0xC2B3                                                                                      */ \
    IF_USE( CRC_USE_CRC16_GENIBUS,     X_CRC( CRC16_GENIBUS,     0x1021,     0xFFFF,     0xFFFF,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0xD64E                                                                                      */ \
    IF_USE( CRC_USE_CRC16_KERMIT,      X_CRC( CRC16_KERMIT,      0x1021,     0x0000,     0x0000,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0x2189                                                                                      */ \
    IF_USE( CRC_USE_CRC16_MAXIM,       X_CRC( CRC16_MAXIM,       0x8005,     0x0000,     0xFFFF,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0x44C2                                                                                      */ \
    IF_USE( CRC_USE_CRC16_MCRF4XX,     X_CRC( CRC16_MCRF4XX,     0x1021,     0xFFFF,     0x0000,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0x6F91                                                                                      */ \
    IF_USE( CRC_USE_CRC16_MODBUS,      X_CRC( CRC16_MODBUS,      0x8005,     0xFFFF,     0x0000,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0x4B37                                                                                      */ \
    IF_USE( CRC_USE_CRC16_RIELLO,      X_CRC( CRC16_RIELLO,      0x1021,     0xB2AA,     0x0000,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0x63D0                                                                                      */ \
    IF_USE( CRC_USE_CRC16_T10_DIF,     X_CRC( CRC16_T10_DIF,     0x8BB7,     0x0000,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0xD0DB                                                                                      */ \
    IF_USE( CRC_USE_CRC16_TELEDISK,    X_CRC( CRC16_TELEDISK,    0xA097,     0x0000,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0x0FB3                                                                                      */ \
    IF_USE( CRC_USE_CRC16_TMS37157,    X_CRC( CRC16_TMS37157,    0x1021,     0x89EC,     0x0000,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0x26B1                                                                                      */ \
    IF_USE( CRC_USE_CRC16_USB,         X_CRC( CRC16_USB,         0x8005,     0xFFFF,     0xFFFF,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0xB4C8                                                                                      */ \
    IF_USE( CRC_USE_CRC16_X25,         X_CRC( CRC16_X25,         0x1021,     0xFFFF,     0xFFFF,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0x906E                                                                                      */ \
    IF_USE( CRC_USE_CRC16_XMODEM,      X_CRC( CRC16_XMODEM,      0x1021,     0x0000,     0x0000,     CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_16B)) /* 0x31C3                                                                                      */ \
    IF_USE( CRC_USE_CRC16_A,           X_CRC( CRC16_A,           0x1021,     0xC6C6,     0x0000,     CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_16B)) /* 0xBF05                                                                                      */ \
/*                                            Algorithm	         Poly,       Init        XorOut      Reflect Data In,              Reflect Data Out,                 Poly Size,              Check size 9        Chek Size 10        Check Size 11       Check Size 12                   */ \
    IF_USE( CRC_USE_CRC32_BZIP2,       X_CRC( CRC32_BZIP2,       0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_32B)) /* 0xFC891918                                                                                  */ /* Also name AAl5, DECT-B, B-CRC-32 */\
    IF_USE( CRC_USE_CRC32_MPEG_2,      X_CRC( CRC32_MPEG_2,      0x04C11DB7, 0xFFFFFFFF, 0x00000000, CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_32B)) /* 0x0376E6E7          0x4BE2AC19          0xC065E90F          0x01A6C937                      */ /* used by CRC module of STM32F1/F2/F4/L1 */\
    IF_USE( CRC_USE_CRC32_POSIX,       X_CRC( CRC32_POSIX,       0x04C11DB7, 0x00000000, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_32B)) /* 0x765E7680                                                                                  */ /* Also name CKSUM */\
    IF_USE( CRC_USE_CRC32_SATA,        X_CRC( CRC32_SATA,        0x04C11DB7, 0x52325032, 0x00000000, CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_32B)) /* 0xCF72AFE8                                                                                  */ \
    IF_USE( CRC_USE_CRC32,             X_CRC( CRC32,             0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0xCBF43926                                                  0x2B17826E                      */ \
    IF_USE( CRC_USE_CRC32_AUTOSAR,     X_CRC( CRC32_AUTOSAR,     0xF4ACFB13, 0xFFFFFFFF, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0X1697D06A          0x7241197D          0x9B1DDB8E          0xE3E57E03                      */ \
    IF_USE( CRC_USE_CRC32_B,           X_CRC( CRC32_B,           0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0x9AE0DAAF                                                                                  */ \
    IF_USE( CRC_USE_CRC32_C,           X_CRC( CRC32_C,           0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0xE3069283                                                                                  */ /* Also name ISCSI, BASE91-C, CASTAGNOLI, INTERLAKEN */\
    IF_USE( CRC_USE_CRC32_CD_ROM_EDC,  X_CRC( CRC32_CD_ROM_EDC,  0x8001801B, 0x00000000, 0x00000000, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0X6EC2EDC4          0xDABF47EC          0xCD3B1146          0x021D3E11                      */ \
    IF_USE( CRC_USE_CRC32_D,           X_CRC( CRC32_D,           0xA833982B, 0xFFFFFFFF, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0x87315576                                                                                  */ /* Also name BASE91_D */\
    IF_USE( CRC_USE_CRC32_ISO_HDLC,    X_CRC( CRC32_ISO_HDLC,    0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0xCBF43926                                                                                  */ /* Also name ADCCP, ISO 3309, V.42, CRC-32-IEEEE, ETHERNET, XZ, PKZIP */\
    IF_USE( CRC_USE_CRC32_JAMCRC,      X_CRC( CRC32_JAMCRC,      0x04C11DB7, 0xFFFFFFFF, 0x00000000, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0x340BC6D9                                                                                  */ \
    IF_USE( CRC_USE_CRC32_MEF,         X_CRC( CRC32_MEF,         0x741B8CD7, 0xFFFFFFFF, 0x00000000, CRC_INPUTDATA_INVERSION_BYTE, CRC_OUTPUTDATA_INVERSION_ENABLE,  CRC_POLYLENGTH_32B)) /* 0xD2C22F51                                                                                  */ \
    IF_USE( CRC_USE_CRC32_Q,           X_CRC( CRC32_Q,           0x814141AB, 0x00000000, 0x00000000, CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_32B)) /* 0x3010BF7F                                                                                  */ /* Also name AIXM */\
    IF_USE( CRC_USE_CRC32_XFER,        X_CRC( CRC32_XFER,        0x000000AF, 0x00000000, 0x00000000, CRC_INPUTDATA_INVERSION_NONE, CRC_OUTPUTDATA_INVERSION_DISABLE, CRC_POLYLENGTH_32B)) /* 0xBD0BE338                                                                                  */ \

#if (CRC_USE_CRC32_ISO_HDLC == DEF_ENABLED)
#define CRC32_ETHERNET  CRC32_ISO_HDLC
#endif

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

//-------------------------------------------------------------------------------------------------
// Typedef(s)
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
    uint32_t    Mode;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CRC_Driver
{
    public:

                    CRC_Driver          ();
        
        void        Start               (CRC_Type_e Type);
        uint32_t    GetValue            (void);
        void        AddByte             (uint8_t Value);
        void        AddBuffer           (const uint32_t* pBuffer, size_t Length);
        uint32_t    CalculateBuffer     (const uint32_t* pBuffer, size_t Length, CRC_Type_e Type);

    private:
    
        const CRC_Info_t*            m_pMethod;
        static const CRC_Info_t      m_MethodList       [NUMBER_OF_CRC_METHOD];
        
        #if(CRC_USE_CRC32_PARKER_F4 == DEF_ENABLED)
        CRC_Type_e                   m_Type;
        #endif
        
        #ifdef USE_FREERTOS
        static xSemaphoreHandle      m_Sem;
        static bool                  m_SemIsInitialize;
        #endif
};

//-------------------------------------------------------------------------------------------------

#endif // __cplusplus
