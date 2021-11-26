//-------------------------------------------------------------------------------------------------
//
//  File : Lib_strnlen.cpp
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

#include "lib_string.h"
#include "grafx_cfg.h"

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strnlen
//
//   Parameter(s):  char* or wchar_t*       pString     String pointer
//                  uint16_t                wSizeMax
//   Return Value:  uint16_t                wSize       Size of the string
//
//   Description:   Calculate the string lenght
//
//   note(s):       Overloaded function
//
//-------------------------------------------------------------------------------------------------
size_t LIB_strnlen(char* pStr, size_t Size)
{
    size_t  Count = 0;
    char*   pPtr;

    pPtr = (char*)pStr;

    if(Size > DIGINI_MAX_PRINT_SIZE)
    {
        Size = DIGINI_MAX_PRINT_SIZE;
    }

    while((*pPtr != (char)'\0') && (Count <= Size))
    {
        pPtr++;
        Count++;
    }

    return Count;
}

