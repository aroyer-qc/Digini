//-------------------------------------------------------------------------------------------------
//
//  File : Lib_stristr.c
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

#define LIB_STR_GLOBAL
#undef LIB_STR_GLOBAL
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
//   Function Name: LIB_stristr
//
//   Parameter(s):  const void* pS1     Pointer on the string to scan
//                  const void* pS2     Pointer on the string containing the sequence to
//                                      match
//   Return Value:  char*
//
//   Description:   Search string inside another string, ignoring the case according to the swap
//                  table loaded in ram from PSK
//
//   note:          no swap in uint16_t size string
//
//-------------------------------------------------------------------------------------------------
void* LIB_stristr(const void* pS1, const void* pS2)
{
    if(Flag.b.UseWORD_StringSize == NO)
    {
        return (void*)LIB_strstric_B((const char*)pS1, (const char*)pS2);
    }
    else
    {
        return (void*)LIB_strstric_W((const uint16_t*)pS1, (const uint16_t*)pS2);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_stristr_B
//
//   Parameter(s):  const void* pS1     Pointer on the string to scan
//                  const void* pS2     Pointer on the string containing the sequence to
//                                      match
//   Return Value:  char*
//
//   Description:   Search string inside another string, ignoring the case according to the swap
//                  table loaded in ram from PSK
//
//   note:          no swap in uint16_t size string
//
//-------------------------------------------------------------------------------------------------
char* LIB_stristr_B(const char* pS1, const char* pS2)
{
    char* pPtr;
    char* s1;
    char* s2;

    pPtr = (char*)pS1;

    for(; *pPtr != '\0'; pPtr++)
    {
        s1 = pPtr;
        s2 = (char*)pS2;

        while(*s2 != '\0')
        {
            if((*s1 != *s2) && (g_LIB_AsciiSwapTable[*s1] != *s2))
            {
                break;
            }

            s1++;
            s2++;

            if(*s2 == '\0')
            {
                return pPtr;
            }
        }
    }

    return nullptr;
}
//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_stristr_W
//
//   Parameter(s):  const void* pS1     Pointer on the string to scan
//                  const void* pS2     Pointer on the string containing the sequence to
//                                      match
//   Return Value:  char*
//
//   Description:   Search string inside another string, ignoring the case according to the swap
//                  table loaded in ram from PSK
//
//   note:          no swap in uint16_t size string
//
//-------------------------------------------------------------------------------------------------
uint16_t* LIB_stristr_W(const uint16_t* pS1, const uint16_t* pS2)
{
    uint16_t* pPtr;
    uint16_t* s1;
    uint16_t* s2;

    pPtr = (uint16_t*)pS1;

    for(; *pPtr != '\0'; pPtr++)
    {
        s1 = pPtr;
        s2 = (uint16_t*)pS2;

        while(*s2 != '\0')
        {
            if(*s1 != *s2)
            {
                break;
            }

            s1++;
            s2++;

            if(*s2 == '\0')
            {
                return pPtr;
            }
        }
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:      LIB_strinstr_B
//
//   Parameter(s):       char*                   Pointer on the string to scan
//                       const char*             Pointer on the string containing the sequence to
//                                               match
//   Return:             char*                   Pointer on the first occurence in the main string
//
//   Description:        Find a subtring in another string but ignore case
//                       For file name only, not for labels)
//
//
//   Notes:             Only use for filename, so no uint16_t version
//
//-------------------------------------------------------------------------------------------------
char* LIB_strinstr_B(char* pS1, const char* pS2)
{
    char*   s1;
    char*   s2;
    uint8_t    c1;
    uint8_t    c2;

    for(; *pS1 != '\0'; pS1++)
    {
        s1 = pS1;
        s2 = (char*)pS2;

        while(*s2 != '\0')
        {
            c1 = *(uint8_t *)s1;
            c2 = *(uint8_t *)s2;

            if('a' <= c1 && c1 <= 'z')
            {
                c1 += ('A' - 'a');
            }
            if('a' <= c2 && c2 <= 'z')
            {
                c2 += ('A' - 'a');
            }

            if(*s1 != *s2)
            {
                break;          // exit while loop
            }

            s1++;
            s2++;

            if(*s2 == '\0')
            {
                return pS1;
            }
        }
    }
    return nullptr;
}

#endif
