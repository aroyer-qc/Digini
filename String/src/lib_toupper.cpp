//-------------------------------------------------------------------------------------------------
//
//  File : LIB_toupper.c
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
#include "lib_string.h"
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
//   Function Name: LIB_toupper
//
//   Parameter(s):
//
//
//   Return Value:
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
void LIB_toupper(void* pStr)
{
    if(Flag.b.UseWORD_StringSize == NO)
    {
        LIB_toupper_B((char*)pStr);
    }
    else
    {
        LIB_toupper_W((uint16_t*)pStr);
    }
}
//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_toupper_B
//
//   Parameter(s):
//
//
//   Return Value:
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
void LIB_toupper_B(char* pStr)
{
    while(*pStr)
    {
        if((*pStr >= 'a') && (*pStr <= 'z'))
        {
            *pStr -= 0x30;
        }
        pStr++;
    }
}
//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_toupper_W
//
//   Parameter(s):
//
//
//   Return Value:
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
void LIB_toupper_W(uint16_t* pStr)
{
    while(*pStr)
    {
        if((*pStr >= 'a') && (*pStr <= 'z'))
        {
            *pStr -= 0x30;
        }
        pStr++;
    }
}
#endif
