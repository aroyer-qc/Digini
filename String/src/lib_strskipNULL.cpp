//-------------------------------------------------------------------------------------------------
//
//  File : Lib_strskipNULL.c
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
//   Function name:      LIB_strskipNULL
//
//   Parameter(s):       void*   Pointer on the string to test
//                       void*   Pointer on the end of the string to test
//   Return:             char*   Pointer on the last character different than null
//
//   Description:        Return pointer on last character different of the value null '0x00'
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void* LIB_strskipNULL(void* pStr, void* pBase)
{
    if(Flag.b.UseWORD_StringSize == NO)
    {
        return (void*)LIB_strskipNULL_B((char*)pStr, (char*)pBase);
    }
    else
    {
        return (void*)LIB_strskipNULL_W((uint16_t*)pStr, (uint16_t*)pBase);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:      LIB_strskipNULL_B
//
//   Parameter(s):       char*   Pointer on the string to test
//                       char*   Pointer on the end of the string to test
//   Return:             char*   Pointer on the last character different than null
//
//   Description:        Return pointer on last character different of the value null '0x00'
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
char* LIB_strskipNULL_B(char* pStr, char* pBase)
{
    char* pS =(char*)pStr;
    char* pB =(char*)pBase;

    while((pS > pB) && (*(pS - 1) == '\0'))
    {
        pS--;
    }

    return pS;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:      LIB_strskipNULL_W
//
//   Parameter(s):       uint16_t*   Pointer on the string to test
//                       uint16_t*   Pointer on the end of the string to test
//   Return:             uint16_t*   Pointer on the last character different than null
//
//   Description:        Return pointer on last character different of the value null '0x0000'
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
uint16_t* LIB_strskipNULL_W(uint16_t* pStr, uint16_t* pBase)
{
    uint16_t* pS = pStr;
    uint16_t* pB = pBase;

    while((pS > pB) && (*(pS - 1) == '\0'))
    {
        pS--;
    }

    return pS;
}

#endif
