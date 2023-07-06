///-------------------------------------------------------------------------------------------------
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

#define STK_CHK_GLOBAL
#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

extern const uint32_t _estack;
extern const uint32_t _Min_Stack_Size;

//-------------------------------------------------------------------------------------------------

void StackCheck::Initialize(void)
{
    int32_t Stack;
    int32_t StackUsed;
    int32_t StackLeft;

    // Start by filling the idle stack (main stack)
    __asm(  "mov %[result], sp" : [result] "=r" (Stack));

    StackUsed = uint32_t(&_estack) - Stack;
    StackLeft = uint32_t(&_Min_Stack_Size) - StackUsed;

    for(; StackLeft > 0; StackLeft -= 4)
    {
        Stack -= 4;
        *((uint32_t*)Stack) = 0xFFFFFFFFUL;
    }

    // Assign stackID 0 to idle stack
    m_Size[0]         = uint32_t(&_Min_Stack_Size);
    m_pStackBottom[0] = &_estack - m_Size[0];

    // Fill empty stack information
    for(int i = 1; i < DIGINI_STACKTISTIC_NUMBER_OF_STACK; i++)
    {
        m_Size[i]         = 0;
        m_pStackBottom[i] = nullptr;
    }

    m_FreeSlot = 0;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           Register
//
// Parameter(s):   pStack               Pointer on the stack to register
//                 STackSz              Stack Size in multiple of 4 bytes
// Return:         StackID              Return the stackID of the registration.
//                                      -1 mean it was not register.
//
// Description:    Return the use size of a specific stack.
//
//-------------------------------------------------------------------------------------------------
int StackCheck::Register(const uint32_t* pStack, size_t STackSz)
{
    if(m_FreeSlot < DIGINI_STACKTISTIC_NUMBER_OF_STACK)
    {
        STackSz--;
        m_pStackBottom[m_FreeSlot] = pStack;
        m_FreeSlot++;
        return m_FreeSlot - 1;
    }

    return -1;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetUsage
//
// Parameter(s):   StackID              Number attributed to stack when it was register
// Return:         size_t               Used Size
//
// Description:    Return the use size of a specific stack.
//
//-------------------------------------------------------------------------------------------------
size_t StackCheck::GetUsage(int StackID)
{
    size_t    UseSize = - 1;
    uint32_t* pStack;

    if(StackID != -1)
    {
        pStack = (uint32_t*)m_pStackBottom[StackID];

        // Use -1 here, because if we reach -1 we now know we busted the stack.
        while((UseSize < m_Size[StackID]) && (*pStack == DIGINI_STACKTISTIC_WATER_MARK_CODE))
        {
            UseSize += 4;
            pStack  += 4;
        }
    }

    return UseSize;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetPercent
//
// Parameter(s):   StackID              Number attributed to stack when it was register
// Return:         int32_t              Percentage of the specific stack.
//
// Description:    Return the use size of a specific stack
//
//-------------------------------------------------------------------------------------------------
int32_t StackCheck::GetPercent(int StackID)
{
    int32_t Percent;

    Percent = int32_t(GetUsage(StackID));

    if(Percent != -1)
    {
        Percent = Percent * 100;
        Percent = (Percent / m_Size[StackID]) + ((((Percent * 10) % m_Size[StackID]) > 5) ? 1 : 0);      // Add 1 if .6% and more
    }

    return Percent;
}

//-------------------------------------------------------------------------------------------------

#endif







#if 0
void StackCheck::Process(void)
{
    void* pScan;
    void  IsItFound;

    for(int i = 0; i < DIGINI_STACKTISTIC_NUMBER_OF_STACK; i++)
    {
        if(i < DIGINI_STACKTISTIC_NUMBER_OF_STACK)
        {
            IsItFound = false;

            for(pScan = m_pStackBottom; ((pScan <= m_pStackTop[i]) && (IsItFound == false)); pScan++)
            {
                if(*pScan != DIGINI_STACKTISTIC_WATER_MARK_CODE)
                {
                    if(pScan < m_pMaxReach)
                    {
                        IsItFound   = true;
                        m_pMaxReach = pScan;
                        m_Percent   = uint8_t((uint32_t((m_pStackTop) - uint32_t(pScan)) * 100) / m_Size[i]);
                    }
                }
            }
        }
    }
}
#endif

