//-------------------------------------------------------------------------------------------------
//
//  File : lib_fast_memcpy.cpp
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

#include "./Digini/lib_digini.h"

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_FastMemcpy
//
//   Parameter(s):  const uint8_t*    pSrc
//                  uint8_t*          pDst
//                  size_t            Length
//   Return:        uint8_t                             4 Bit's value
//
//   Description:   Fast copy memory block to another location with a faster method.
//
//   Note(s):       Overloaded function
//
//-------------------------------------------------------------------------------------------------
void LIB_FastMemcpy(const uint8_t* pSrc, uint8_t* pDst, size_t Length)
{
    // Fast-copy data fragments tobuffer

    for(; Length > 0; pDst++, pSrc++, Length--)
    {
        *pDst = *pSrc;
    }

/*
    for(; Length > 7; pDst += 8, pSrc += 8, Length -= 8)
    {
        UNALIGNED_UINT32_WRITE(&pDst[0], UNALIGNED_UINT32_READ(&pSrc[0]));
        UNALIGNED_UINT32_WRITE(&pDst[4], UNALIGNED_UINT32_READ(&pSrc[4]));
    }

    // Copy remaining up to 6 bytes
    for(; Length > 1; pDst += 2, pSrc += 2, Length -= 2)
    {
        UNALIGNED_UINT16_WRITE(&pDst[0], UNALIGNED_UINT16_READ(&pSrc[0]));
    }

    // Copy remaining byte if any
    if(Length > 0)
    {
        pDst++[0] = pSrc++[0];
    }
*/
}

//-------------------------------------------------------------------------------------------------
