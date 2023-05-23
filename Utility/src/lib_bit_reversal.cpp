//-------------------------------------------------------------------------------------------------
//
//  File : lib_bit_reversal.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
//
//  Name:           BitReversal
//
//  Parameter(s):   CrcType
//  Return:         void
//
//  Description:    This function init the conversion with the CRC type selected.
//
//-------------------------------------------------------------------------------------------------
uint32_t LIB_BitReversal(uint32_t Value)
{
    uint32_t Result;

  #if (__CORTEX_M >= 0x03U) || (__CORTEX_SC >= 300U)
    __ASM volatile ("rbit %0, %1" : "=r" (Result) : "r" (Value));
  #else
    int32_t Shift = sizeof(uint32_t) * 8 - 1;             // extra shift needed at end

    Result = Value;                                       // Result will be reversed bits of Value; first get LSB of Value

    for(Value >>= 1; Value; Value >>= 1)
    {
        Result <<= 1;
        Result |= (Value & 1);
        Shift--;
    }

    Result <<= Shift;                                     // shift when v's highest bits are zero
  #endif

    return Result;
}

//-------------------------------------------------------------------------------------------------
