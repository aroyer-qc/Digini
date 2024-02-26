//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F1.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#if (defined (STM32F100xB) || \
     defined (STM32F101x6) || defined (STM32F101xB) || \
     defined (STM32F102x6) || defined (STM32F102xB) || \
     defined (STM32F103x6) || defined (STM32F103xB))

#define DMA2_SUPPORT        DEF_DISABLED

#endif

#if (defined (STM32F100xE) || \
     defined (STM32F101xE) || defined (STM32F101xG) || \
     defined (STM32F103xE) || defined (STM32F103xG) || \
     defined (STM32F105xC) || defined (STM32F107xC))

#define DMA2_SUPPORT        DEF_ENABLED

#endif

// Add CPU spec as project need then

#if defined (STM32F103xB)

#define NUMBER_OF_IO_PORT               6

#endif

#if (defined (STM32F103xE) || defined (STM32F103xG))

#define NUMBER_OF_IO_PORT               8

#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void ISR_Initialize     (void);
void ISR_Init           (IRQn_Type Channel, const ISR_Prio_t* pPrio);

//-------------------------------------------------------------------------------------------------
