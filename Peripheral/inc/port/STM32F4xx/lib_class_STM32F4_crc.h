//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_crc.h
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
// Note(s)
//-------------------------------------------------------------------------------------------------
//
// This hardware CRC is for CRC-32 (MPEG-2) polynomial: 0x4C11DB7
//
// With some adjustment four CRC-32 method can be use with the hardware of STM32F4
// The reverse init value is the value after reset and it is needed in the CRC module to start with
// the real init value that is required for the calculation.
//
// Field 'Init' is not used in the expanding macro it is there for reference.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_CRC_AS_ENUM(ENUM_ID, INIT, REVERSE_INIT, XOR_OUT, WIDTH)          ENUM_ID,
#define EXPAND_X_CRC_AS_CLASS_CONST(ENUM_ID, INIT, REVERSE_INIT, XOR_OUT, WIDTH)   {REVERSE_INIT, XOR_OUT, WIDTH},

#define CRC_32_HW_METHOD_DEF(X_CRC)   /* Check is base on string "123456789" */ \
/*                                                Algorithm	        Input Type,                     Init        Rev init,   XorOut           Check                   */ \
    IF_USE( DIGINI_USE_HW_CRC_32_BZIP2,    X_CRC( CRC_32_BZIP2,     CRC_INPUT_DATA_FORMAT_BYTES,    0xFFFFFFFF, 0xB9509BB6, 0xFFFFFFFF )) /* 0xFC891918              */ /* Also name AAl5, DECT-B, B-CRC-32 */\
    IF_USE( DIGINI_USE_HW_CRC_32_MPEG_2,   X_CRC( CRC_32_MPEG_2,    CRC_INPUT_DATA_FORMAT_BYTES,    0xFFFFFFFF, 0xB9509BB6, 0x00000000 )) /* 0x0376E6E7              */ /* used by CRC module of STM32F1/F2/F4/L1 */\
    IF_USE( DIGINI_USE_HW_CRC_32_POSIX,    X_CRC( CRC_32_POSIX,     CRC_INPUT_DATA_FORMAT_BYTES,    0x00000000, 0xFFFFFFFF, 0xFFFFFFFF )) /* 0x765E7680              */ /* Also name CKSUM */\
    IF_USE( DIGINI_USE_HW_CRC_32_SATA,     X_CRC( CRC_32_SATA,      CRC_INPUT_DATA_FORMAT_BYTES,    0x52325032, 0x0E0DE6FB, 0x00000000 )) /* 0xCF72AFE8              */ \

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum CRC_Type_e
{
    CRC_32_HW_METHOD_DEF(EXPAND_X_CRC_AS_ENUM)
    NUMBER_OF_HW_CRC_METHOD,
};

enum CRC_InputType_e
{
    CRC_INPUT_DATA_FORMAT_BYTES         = 0,
    CRC_INPUT_DATA_FORMAT_HALF_WORDS,
    CRC_INPUT_DATA_FORMAT_WORDS,
};

struct CRC_HW_Info_t
{
    uint32_t    RevInit;
    uint32_t    XorOut;
    uint8_t     Width;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CRC_Driver
{
    public:

        void        Initialize          (CRC_Type_e Type);
        void        Start               (void);
        uint32_t    GetValue            (void);
        uint32_t    AddByte             (uint8_t Value);
        uint32_t    AddBuffer           (const uint8_t* pBuffer, size_t Length);
        uint32_t    CalculateBuffer     (const uint8_t* pBuffer, size_t Length);

    private:

        void        HandleBytes         (uint8_t* pBuffer, size_t Length);
        void        HandleHalfWords     (uint16_t* pBuffer, size_t Length);

        CRC_Type_e                      m_Type;
        static const CRC_HW_Info_t      m_MethodList       [NUMBER_OF_CRC_METHOD];
};

//-------------------------------------------------------------------------------------------------

