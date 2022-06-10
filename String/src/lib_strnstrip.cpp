//-------------------------------------------------------------------------------------------------
//
//  File : lib_strnstrip.cpp
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
#ifdef DIGINI_USE_GRAFX    // remove this and use more common define for DIGINI_MAX_PRINT_SIZE

//-------------------------------------------------------------------------------------------------
//
//   Function Name: STR_strnstrip
//
//   Parameter(s):  char*   pString
//                  size_t  nSize
//   Return Value:  None
//
//   Description:   Strip the trailing space starting at nSize
//
//-------------------------------------------------------------------------------------------------
void STR_strnstrip(char* pString, size_t nSize)
{
    char* End;

    End = pString + (nSize - 1);
    while((End >= pString))
    {
        if(*End == ' ')
        {
            *End = '\0';
        }
        else
        {
            return;
        }
        End--;
    }
}

//-------------------------------------------------------------------------------------------------

#endif