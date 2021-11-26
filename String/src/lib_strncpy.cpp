//-------------------------------------------------------------------------------------------------
//
//  File : Lib_strncpy.cpp
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

#include <stdint.h>
#include "lib_string.h"
#include "lib_typedef.h"
#include "grafx_cfg.h"

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strncpy
//
//   Parameter(s):  char*               pDest       Destination buffer
//                  const char*         pSrc        Source Buffer
//   Return Value:  char*               pDest       Destination buffer is return
//
//   Description:   Copy a string to a new location until
//                          nullptr is encounter ( in char size )     or
//                          max number of copy is reach
//
//-------------------------------------------------------------------------------------------------
char* LIB_strncpy(char* pDest, const char* pSrc, size_t Size)
{
    char*   pSrcPtr;
    char*   pDstPtr;
    size_t  Count;

    pSrcPtr = (char*)pSrc;
    pDstPtr = pDest;
    Count   = 0;

    if(Size > DIGINI_MAX_PRINT_SIZE)
    {
        Size = DIGINI_MAX_PRINT_SIZE;
    }

    while((*pSrcPtr != (char)ASCII_NULL) && (Count < Size))
    {
        *pDstPtr = *pSrcPtr;
        pDstPtr++;
        pSrcPtr++;
        Count++;
    }
    *pDstPtr = *pSrcPtr;

    return pDest;
}

