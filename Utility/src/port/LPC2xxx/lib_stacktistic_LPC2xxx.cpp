//-------------------------------------------------------------------------------------------------
//
//  File : lib_stacktistic_LPC2xxx.cpp
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

#error This code is not validated yet

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)

extern const uint32_t __StackTop;       // maybe use standard embitz way's _estack 
extern const uint32_t Stack_Size;       // _Min_Stack_Size

//-------------------------------------------------------------------------------------------------
//
// Name:           InitializePort
//
// Parameter(s):   None
// Return:         None
//
// Description:    Initialize the stack monitoring tool on the specific CPU port.
//
//-------------------------------------------------------------------------------------------------
void StackCheck::InitializePort(void)
{
    uint32_t*           pStackBottom;

    m_Size[0]         = uint32_t(&Stack_Size);
    pStackBottom      = (uint32_t*)&__StackTop - m_Size[0];
    m_pStackBottom[0] = (uint32_t*)pStackBottom;
    m_pStackName[0]   = "TaskIdle";

    m_NumberOfStack = 1;

    // Start by filling the idle stack (main stack)
    __asm( "mov     r1,     %0"     :: "r" (pStackBottom));
    __asm( "mov     r2,     sp                                                  \n"
           "movs    r0,     " STRINGIFY(DIGINI_STACKTISTIC_WATER_MARK_CODE) "   \n"
           "b       LoopFillStack                                               \n"
           "FillStack:                                                          \n"
           "strlo   r0,     [r1],   #4                                          \n"
           "LoopFillStack:                                                      \n"
           "cmp     r1,     r2                                                  \n"
           "blo     FillStack                                                   \n" );
}

//-------------------------------------------------------------------------------------------------

#endif