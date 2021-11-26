//-------------------------------------------------------------------------------------------------
//
//  File : Lib_strstriptl.c
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
//   Function Name: LIB_strstriptl
//
//   Parameter(s):  void* pStr          Pointer on the string to strip
//
//   Return Value:  char*               Pointer on the EOL
//
//   Description:   Will put a '\0' at the last encounter if = ' '
//
//-------------------------------------------------------------------------------------------------
void* LIB_strstriptl(void* p)
{
    if(Flag.b.UseWORD_StringSize == NO)
    {
        return (void*)LIB_strstriptl_B((char*)p);
    }
    else
    {
        return (void*)LIB_strstriptl_W((uint16_t*)p);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strstriptl_B
//
//   Parameter(s):  void* pStr          Pointer on the string to strip
//
//   Return Value:  char*               Pointer on the EOL
//
//   Description:   Will put a '\0' at the last encounter if = ' '
//
//-------------------------------------------------------------------------------------------------
char* LIB_strstriptl_B(char* p)
{
    uint8_t byLen;
    char* pStr    = p;
    char* pStrAdd = p;

    byLen = LIB_strlen_B(pStr);
    pStr += byLen;

    do
    {
        pStr--;
    }
    while(((*pStr == ' ') || (*pStr == '\0')) && (pStr >= pStrAdd));

    pStr++;
    *pStr = '\0';
    return pStr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strstriptl_W
//
//   Parameter(s):  void* pStr          Pointer on the string to strip
//
//   Return Value:  char*               Pointer on the EOL
//
//   Description:   Will put a '\0' at the last encounter if = ' '
//
//-------------------------------------------------------------------------------------------------
uint16_t* LIB_strstriptl_W(uint16_t* p)
{
    uint8_t byLen;
    uint16_t* pStr    = p;
    uint16_t* pStrAdd = p;

    byLen = LIB_strlen_W(pStr);
    pStr += byLen;

    do
    {
        pStr--;
    }
    while(((*pStr == ' ') || (*pStr == '\0')) && (pStr >= pStrAdd));

    pStr++;
    *pStr = '\0';
    return pStr;
}

#endif
