//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_crc.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_typedef.h"
#include "util_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

enum CrcType_e                      // Poly         Init Value      XOR Value
{
    CRC_8,
    CRC_8_EBU,                      // 0x1D
    CRC_16_MODBUS,                  // 0x8005       0xFFFF          0x0000
    CRC_16_CCITT,                   // 0x1021       0xFFFF          0x0000
    CRC_32_IEEE,                    // 0x04C11DB7	0xFFFFFFFF      0xFFFFFFFF
    CRC_NORMAL,
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CCRC
{
    public:

        void        Init        (CrcType_e CrcType, uint32_t Width, uint32_t Polynomial, uint32_t Xor);
        void        Init        (CrcType_e CrcType);
        uint32_t    Done        (void);
        void        Byte        (const uint8_t Byte);
        void        Buffer      (const uint8_t *pBuffer, uint32_t Len);

    private:

        CrcType_e  m_CrcType;
        uint32_t  m_Polynomial;
        uint32_t  m_Xor;
        uint32_t  m_Width;
        uint32_t  m_Mask;
        uint32_t  m_Remainder;
};
//-------------------------------------------------------------------------------------------------

