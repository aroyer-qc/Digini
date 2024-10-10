//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_crc.h
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
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum CRC_InputType_e
{
    CRC_INPUT_DATA_FORMAT_BYTES         = 0,
    CRC_INPUT_DATA_FORMAT_HALF_WORDS,
    CRC_INPUT_DATA_FORMAT_WORDS,
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CRC_Driver
{
    public:

        void        Initialize      (CRC_InputType_e Type);
        void        Reset           (void);
        uint32_t    GetValue        (void);
        uint32_t    AddBuffer       (const void *pBuffer, size_t Length);

    private:

        void        HandleBytes     (uint8_t* pBuffer, size_t Length);
        void        HandleHalfWords (uint16_t* pBuffer, size_t Length);

        CRC_InputType_e     m_Type;
};


//-------------------------------------------------------------------------------------------------

