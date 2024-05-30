//-------------------------------------------------------------------------------------------------
//
//  File : lib_io.h
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

// General define for IO_PIN
#define IO_PIN_MASK_0                   ((uint16_t)0x0001)
#define IO_PIN_MASK_1                   ((uint16_t)0x0002)
#define IO_PIN_MASK_2                   ((uint16_t)0x0004)
#define IO_PIN_MASK_3                   ((uint16_t)0x0008)
#define IO_PIN_MASK_4                   ((uint16_t)0x0010)
#define IO_PIN_MASK_5                   ((uint16_t)0x0020)
#define IO_PIN_MASK_6                   ((uint16_t)0x0040)
#define IO_PIN_MASK_7                   ((uint16_t)0x0080)
#define IO_PIN_MASK_8                   ((uint16_t)0x0100)
#define IO_PIN_MASK_9                   ((uint16_t)0x0200)
#define IO_PIN_MASK_10                  ((uint16_t)0x0400)
#define IO_PIN_MASK_11                  ((uint16_t)0x0800)
#define IO_PIN_MASK_12                  ((uint16_t)0x1000)
#define IO_PIN_MASK_13                  ((uint16_t)0x2000)
#define IO_PIN_MASK_14                  ((uint16_t)0x4000)
#define IO_PIN_MASK_15                  ((uint16_t)0x8000)
#define IO_PIN_MASK_All                 ((uint16_t)0xFFFF)

#if defined STM32F1xx
 #include "./Peripheral/inc/port/STM32F1xx/lib_STM32F1_io.h"
#elif defined STM32F4xx
 #include "./Peripheral/inc/port/STM32F4xx/lib_STM32F4_io.h"
#elif defined STM32F7xx
 #include "./Peripheral/inc/port/STM32F7xx/lib_STM32F7_io.h"
#endif

//-------------------------------------------------------------------------------------------------
