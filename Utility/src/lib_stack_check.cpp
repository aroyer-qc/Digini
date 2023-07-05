///-------------------------------------------------------------------------------------------------
//
//  File : lib_stack_check.cpp
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
#include "lib_stack_check.h"

//-------------------------------------------------------------------------------------------------

void StackCheck::Initialize(void)
{
    for(int = 0; i < STACKCHECK_NUMBER_OF_STACK; i++)
    {
        m_pStackTop[i]    = nullptr;
        m_pStackBottom[i] = nullptr;
        m_pMaxReach[i]    = nullptr;
        m_Size            = 0;
    }
    
    m_FreeSlot = 0;
}

void StackCheck::RegisterStack(void* pStack, size_t STackSz)
{
    if(m_FreeSlot < STACKCHECK_NUMBER_OF_STACK)
    {
        STackSz--;
        m_pStackBottom[m_FreeSlot] = pStack;
        m_pStackTop   [m_FreeSlot] = static_cast<void*>(uint32_t(pStack) + uint32_t(StackSz));
        m_pMaxReach   [m_FreeSlot] = m_pStackTop[m_FreeSlot];
        m_FreeSlot++;
    }
}

void StackCheck::Process(void)
{
    void* pScan;
    void  IsItFound;
    
    for(int i = 0; i < STACKCHECK_NUMBER_OF_STACK; i++)
    {
        if(i < STACKCHECK_NUMBER_OF_STACK)
        {
            IsItFound = false;
            
            for(pScan = m_pStackBottom; ((pScan <= m_pStackTop[i]) && (IsItFound == false)); pScan++)
            {
                if(*pScan != STACKCHECK_HIGH_WATER_MARK_CODE)
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
        // os yield not needed on idle task
    }
}