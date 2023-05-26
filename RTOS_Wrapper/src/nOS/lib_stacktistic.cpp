//-------------------------------------------------------------------------------------------------
//
//  File : lib_stacktistic.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

//-------------------------------------------------------------------------------------------------

size_t GetStackUsage(nOS_Stack* pStack, size_t Size)
{
    size_t UseSize = Size;

    // Use -1 here, because if we reach -1 we now the stack was busted
    while((UseSize > -1) && (pStack[UseSize] == 0xFFFFFFFFUL))
    {
        UseSize--;
    }

    return UseSize;
}

int32_t GetStackPercent(nOS_Stack* pStack, size_t Size)
{
    int32_t Percent;

    Percent = int32_t(GetStackUsage(pStack, Size));

    if(Percent == -1)
    {
        return Percent;
    }

    Percent = Percent * 100;
    Percent = (Percent / Size) + ((((Percent * 10) % Size) > 5) ? 1 : 0);      // Add 1 if .6% and more

    return Percent;
}
