//-------------------------------------------------------------------------------------------------
//
//  File : Lib_strcmp.c
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

#if 0
//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
// ...

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

// ...

//-------------------------------------------------------------------------------------------------
// Private function(s)
//-------------------------------------------------------------------------------------------------

// ...

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strcmp
//
//   Parameter(s):  const void *pS1             String 1
//                  const void *pS2             String 2
//   Return Value:  long                        Difference between *p1 and *p2
//
//   Description:   compare the string pS1 to the string pS2
//                  This function starts comparing the character of each strings until a pair differ
//                  or until a terminating nullptr character is reached.
//
//   Notes :
//
//-------------------------------------------------------------------------------------------------
long LIB_strcmp(const void* pS1, const void* pS2)
{
    if(Flag.b.UseWORD_StringSize == NO)
    {
        return (long)LIB_strcmp_B((const char*)pS1, (const char*)pS2);
    }
    else
    {
        return LIB_strcmp_W((const uint16_t*)pS1, (const uint16_t*)pS2);
    }
}
//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strcmp_W
//
//   Parameter(s):  const uint16_t *pS1             String 1
//                  const uint16_t *pS2             String 2
//   Return Value:  long                        Difference between *p1 and *p2
//
//   Description:   compare the string pS1 to the string pS2
//                  This function starts comparing the caracter of each strings until a pair differ
//                  or until a terminating NULL caracter is reached.
//
//   Notes :
//
//-------------------------------------------------------------------------------------------------
long LIB_strcmp_W(const uint16_t *pS1, const uint16_t *pS2)
{
    uint16_t* p1 = (uint16_t*)pS1;
    uint16_t* p2 = (uint16_t*)pS2;
    long r   = 0;

    while((r == 0) && (*p1 != 0) && (*p2 != 0))
    {
        if(r = (*p1 - *p2))
        {
            return r;
        }
        p1++;
        p2++;
    };

    (r = *p1 - *p2);

    return r;
}

#endif

