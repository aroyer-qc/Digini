//-------------------------------------------------------------------------------------------------
//
//  File : lib_4bcddec.cpp
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

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_4BcdDec
//
//   Parameter(s):  uint16_t     BCD_Value       BCD Value
//   Return:        uint16_t                     0 - 9999 decimal value
//
//   Description:   Extract from 4 BCD to decimal value uint16_t
//
//-------------------------------------------------------------------------------------------------
uint16_t LIB_4BcdDec(uint16_t BCD_Value)
{
    uint16_t Value;

    Value  = LIB_2BcdDec(uint8_t(BCD_Value >> 8)) * 100;
    Value += LIB_2BcdDec(uint8_t(BCD_Value));

    return Value;
}

//-------------------------------------------------------------------------------------------------