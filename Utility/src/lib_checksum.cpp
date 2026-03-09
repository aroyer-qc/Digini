//-------------------------------------------------------------------------------------------------
//
//  File : lib_checksum.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

uint16_t LIB_Checksum16(const uint8_t* pBuffer, uint16_t Length)
{
    uint32_t Sum = 0;

    while(Length > 1)
    {
        uint16_t Word = ((uint16_t)pBuffer[0] << 8) | pBuffer[1];
        Sum += Word;

        if(Sum > 0xFFFF)
        {
            Sum = (Sum & 0xFFFF) + 1;
        }

        pBuffer += 2;
        Length  -= 2;
    }

    if(Length == 1)
    {
        uint16_t Word = ((uint16_t)pBuffer[0] << 8);
        Sum += Word;

        if(Sum > 0xFFFF)
        {
            Sum = (Sum & 0xFFFF) + 1;
        }
    }

    return (uint16_t)(~Sum & 0xFFFF);
}

//-------------------------------------------------------------------------------------------------

uint16_t LIB_HTONS_Checksum16(const uint8_t* pBuffer, uint16_t Length)
{
    return htons(LIB_Checksum16(pBuffer, Length));
}
