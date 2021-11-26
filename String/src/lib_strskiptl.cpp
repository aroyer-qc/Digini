//-------------------------------------------------------------------------------------------------
//
//  File : Lib_strskiptl.c
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
//   Function Name: LIB_strskiptl
//
//   Parameter(s):  void*           pStr
//                  const void*     pBase
//   Return Value:  void*
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
void* LIB_strskiptl(void* pStr, const void* pBase)
{
    if(Flag.b.UseWORD_StringSize == NO)
    {
        return (void*)LIB_strskiptl_B((char*)pStr, (const char*)pBase);
    }
    else
    {
        return (void*)LIB_strskiptl_W((uint16_t*)pStr, (const uint16_t*)pBase);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strskiptl_B
//
//   Parameter(s):  char*           pStr
//                  const char*     pBase
//   Return Value:  char*
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
char* LIB_strskiptl_B(char* pStr, const char* pBase)
{
    char* pStr1  = pStr;
    char* pBase1 = (char*)pBase;

    while((pStr1 > pBase1) && ((*(pStr1 - 1) == ' ') || (*(pStr1 - 1) == '\0')))
    {
        pStr1--;
    }
    return pStr1;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strskiptl_W
//
//   Parameter(s):  uint16_t*           pStr
//                  const uint16_t*     pBase
//   Return Value:  uint16_t*
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
uint16_t* LIB_strskiptl_W(uint16_t* pStr, const uint16_t* pBase)
{
    uint16_t* pStr1  = pStr;
    uint16_t* pBase1 = (uint16_t*)pBase;

    while((pStr1 > pBase1) && ((*(pStr1 - 1) == ' ') || (*(pStr1 - 1) == '\0')))
    {
        pStr1--;
    }
    return pStr1;
}

#endif
