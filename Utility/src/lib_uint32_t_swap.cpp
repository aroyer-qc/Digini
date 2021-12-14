//-------------------------------------------------------------------------------------------------
//
//  File : lib_uint32_t_swap.c
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_uint32_t_Swap
//
//   Parameter(s):  uint32_t*       pSwap
//   Return Value:  None
//
//   Description:   Swap all bytes in a 32 bits value
//
//-------------------------------------------------------------------------------------------------
void LIB_uint32_t_Swap(uint32_t* pSwap)
{
    s32_t Value;

    Value.u_8.u0 = ((s32_t*)pSwap)->u_8.u3;
    Value.u_8.u1 = ((s32_t*)pSwap)->u_8.u2;
    Value.u_8.u2 = ((s32_t*)pSwap)->u_8.u1;
    Value.u_8.u3 = ((s32_t*)pSwap)->u_8.u0;
    *pSwap = Value.u_32;
}

//-------------------------------------------------------------------------------------------------
