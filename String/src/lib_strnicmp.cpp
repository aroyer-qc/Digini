//-------------------------------------------------------------------------------------------------
//
//  File : Lib_strnicmp.c
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

//------ Note(s) ----------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
#define LIB_002_GLOBAL
#undef LIB_002_GLOBAL
#include <lib_string.h>

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strnicmp_B
//
//   Parameter(s):  pS1      Pointer string 1
//                  pS2      Pointer string 2
//                  n        Number of byte to compare
//
//   Return Value:
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
int16_t LIB_strnicmp_B(void* pS1, const void* pS2, uint32_t n)
{
    char c1;
    char c2;
    char *pPtr1 = (char*)pS1;
    const char *pPtr2 = (const char*)pS2;

    while((n-- != 0) && (*pPtr1 || *pPtr2))
    {
        c1 = *pPtr1;
        pPtr1++;

        if(('a' <= c1) && (c1 <= 'z'))
        {
            c1 += ('A' - 'a');
        }

        c2 = *pPtr2++;
        if(('a' <= c2) && (c2 <= 'z'))
        {
            c2 += ('A' - 'a');
        }

        if(c1 != c2)
        {
            return c1 - c2;
        }
  }
  return 0;
}

