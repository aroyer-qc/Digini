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

#if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

extern const uint32_t _estack;
extern const uint32_t _Min_Stack_Size;

//-------------------------------------------------------------------------------------------------
//
// Name:           STACK_GetUsage
//
// Parameter(s):   uint32_t*           pStack
//                 size_t              Size
// Return:         size_t              Used Size
//
// Description:    Return the use size of a specific stack.
//
//-------------------------------------------------------------------------------------------------
size_t STACK_GetUsage(const uint32_t* pStack, size_t Size)
{
    size_t UseSize = 0;

    pStack -= (Size / 4);

    // Use -1 here, because if we reach -1 we now the stack was busted
    while((UseSize < Size) && (*pStack == 0xFFFFFFFFUL))
    {
        UseSize += 4;
        pStack  += 4;
    }

    return UseSize;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           STACK_GetPercent
//
// Parameter(s):   const  uint32_t*    pStack
//                 size_t              Size
// Return:         int32_t             Percentage of the specific stack.
//
// Description:    Return the use size of a specific stack
//
//-------------------------------------------------------------------------------------------------
int32_t STACK_GetPercent(const uint32_t* pStack, size_t Size)
{
    int32_t Percent;

    Percent = int32_t(STACK_GetUsage(pStack, Size));

    if(Percent == -1)
    {
        return Percent;
    }

    Percent = Percent * 100;
    Percent = (Percent / Size) + ((((Percent * 10) % Size) > 5) ? 1 : 0);      // Add 1 if .6% and more

    return Percent;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           STACK_FillIdle
//
// Parameter(s):   void
// Return:         None
//
// Description:    Fill the Idle stack with 0xFF
//
//-------------------------------------------------------------------------------------------------
void STACK_FillIdle(void)
{
    int32_t Stack;
    int32_t StackUsed;
    int32_t StackLeft;

    __asm(  "mov %[result], sp" : [result] "=r" (Stack));

    StackUsed = uint32_t(&_estack) - Stack;
    StackLeft = uint32_t(&_Min_Stack_Size) - StackUsed;

    for(; StackLeft > 0; StackLeft -= 4)
    {
        Stack -= 4;
        *((uint32_t*)Stack) = 0xFFFFFFFFUL;
    }
}

#endif
