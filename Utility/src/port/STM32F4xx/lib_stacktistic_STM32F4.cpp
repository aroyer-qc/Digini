//-------------------------------------------------------------------------------------------------
//
//  File : lib_stacktistic_STM32F4.cpp
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)

extern const uint32_t _estack;
extern const uint32_t _Min_Stack_Size;

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

    m_Size[0]         = uint32_t(&_Min_Stack_Size) / 4;
    m_pStackBottom[0] = (uint32_t*)&_estack - m_Size[0];
    m_pStackName[0]   = "Task Idle";

    m_NumberOfStack = 1;

    // Copy the following code into the startup file after the call to  "bl  __libc_init_array"

    /// /* Fill the stack with watermark */
    ///   ldr  r3, =_StackBottom;
    ///   mov   r2, sp
    ///   movs  r4, 0xFFFFFFFF
    ///   b     LoopFillStack
    /// FillStack:
    ///   str   r4,     [r3],   #4
    /// LoopFillStack:
    ///   cmp  r2,     r3
    ///   bne  FillStack
}

//-------------------------------------------------------------------------------------------------

#endif
