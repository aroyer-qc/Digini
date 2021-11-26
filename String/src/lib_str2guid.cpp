//-------------------------------------------------------------------------------------------------
//
//  File : lib_str2guid.cpp
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

#if 0
//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_str2GUID
//  Parameter(s):
//
//  Return:
//
//  Description:    Extract a UID from string, stripping all irrelevant character
//
//  Note(s):		ex. "1ABC-45F7-3A6D-9BC8-00"  will become 1ABC45F73A6D9BC800
//
//-------------------------------------------------------------------------------------------------
uint8_t LIB_str2UID(void *pSrc, uint8_t *pGUID)
{
    char *pcSrc = (char *)pSrc;
    uint8_t byCount;

    byCount = 0;
    while((*pcSrc != '\0') && (byCount < 16))
    {
        if(((*pcSrc >= '0') && (*pcSrc <= '9')) || ((*pcSrc >= 'A') && (*pcSrc <= 'F')))
        {
            pGUID[byCount] = LIB_2AscHex(pcSrc);
            pcSrc += 2;
            byCount++;
        }
        else
        {
            pcSrc++;
        }
    }

    return byCount;
}

#endif
