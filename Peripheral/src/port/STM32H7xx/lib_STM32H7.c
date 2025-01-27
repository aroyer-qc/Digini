//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7.c
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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

#include <stdio.h>

// that is not the real CPUID, this what cortex lib is defined to ..

#define CM7_CPUID        ((uint32_t)0x00000003)

#if defined(DUAL_CORE)
#define CM4_CPUID        ((uint32_t)0x00000001)
#endif


//-------------------------------------------------------------------------------------------------
//
//  Name:           GetCurrentCPUID
//
//  Parameter(s):   None
//  Return:         CPU identifier
//
//  Description:    Returns the current CPU ID
//
//-------------------------------------------------------------------------------------------------
#if defined(DUAL_CORE)
uint32_t GetCurrentCPUID(void)
{
    if(((SCB->CPUID & 0x000000F0) >> 4) == 0x7)
    {
        return CM7_CPUID;
    }
    else
    {
        return CM4_CPUID;
    }
}

#else

uint32_t GetCurrentCPUID(void)
{
    return CM7_CPUID;
}

#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetUniqueCPUID
//
//  Parameter(s):   None
//  Return:         CPU identifier
//
//  Description:    Returns the current CPU ID
//
//-------------------------------------------------------------------------------------------------
uint32_t GetUniqueCPUID(void)
{
    return 0;
}
