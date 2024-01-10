//-------------------------------------------------------------------------------------------------
//
//  File : lib_macro.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define VAR_UNUSED(v)                       ((void)(v))

#define AbsMin(V,T)                         (((V) < (T)) ? (T) : (V))
#define AbsMax(V,T)                         (((V) > (T)) ? (T) : (V))
#define Toggle(A)                           (A = (uint8_t)(1 - A))
#define CheckOption(A,B)                    (((B) & (A)) > 0)
#define sizearray(A)                        (sizeof(A) / sizeof(A[0]))
#define ABS(X)                              ((X) >= 0 ? (X) : -(X))

#define swap8(A)                            ((A << 4) | (A >> 4))
#define swap16(A)                           ((A << 8) | (A >> 8))
#define swap32(A)                           ((A << 16) | (A >> 16))

// Macro to convert 1 ASCII character that represent a Byte in text to a real HEX value
#define AscHex(A)                           ((((A-=48)>9?(A-=7):A)>15)?(A-=32):A)
// Macro to convert 2 ASCII character that represent a Byte in text to a real HEX value
#define Asc2Hex(A,B)                        (AscHex(A)<<4)+AscHex(B)

// Bit operation on uint8_t
#define CheckBit(DATA, BT)                  (DATA &   (0x01 << BT))
#define ToggleBit(DATA, BT)                 (DATA ^=  (0x01 << BT))
#define BIT(n)                              (1 << n)
#define BIT_TRUE(x, mask)                   (x) |= (mask)
#define BIT_FALSE(x, mask)                  (x) &= ~(mask)
#define BIT_IS_TRUE(x, mask)                ((x & mask) != 0)
#define BIT_IS_FALSE(x, mask)               ((x & mask) == 0)
//#define BIT_TRUE_ATOMIC(x, mask)            BIT_TRUE(x,mask)
//#define BIT_FALSE_ATOMIC(x, mask)           BIT_FALSE(x,mask)
//#define BIT_TOGGLE_ATOMIC(x, mask)          (x) ^= (mask)



// Create uint32_t value from 4 uint8_t value or ascii char
#define U32MACRO(A,B,C,D)                   ((uint32_t(D) << 24) + (uint32_t(C) << 16) + (uint32_t(B) << 8) + uint32_t(A))

// Create uint32_t value from 4 uint8_t value or ascii char
#define U16MACRO(A,B)                       ((uint16_t(B) << 8) + uint16_t(A))

#define GetTick()                           nOS_GetTickCount()
#define TickHasTimeOut(start_time,delay)    ((GetTick() - (start_time)) > ((TickCount_t)delay))

#define CVT_HOUR_TO_SECOND(H)               ((H) * 3600)

#ifndef M_PI
 #define M_PI                               3.14159265358979323846
#endif // M_PI

#define MM_PER_INCH                         (25.40)
#define INCH_PER_MM                         (0.0393701)

#ifndef   PACKED_STRUCT
  #define PACKED_STRUCT                     struct __attribute__((packed, aligned(1)))
#endif

#ifndef UNALIGNED_UINT16_WRITE
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpacked"
  #pragma GCC diagnostic ignored "-Wattributes"
  PACKED_STRUCT T_UINT16_WRITE { uint16_t v; };
  #pragma GCC diagnostic pop
  #define UNALIGNED_UINT16_WRITE(addr, val)    (void)((((struct T_UINT16_WRITE *)(void *)(addr))->v) = (val))
#endif
#ifndef   UNALIGNED_UINT16_READ
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpacked"
  #pragma GCC diagnostic ignored "-Wattributes"
  PACKED_STRUCT T_UINT16_READ { uint16_t v; };
  #pragma GCC diagnostic pop
  #define UNALIGNED_UINT16_READ(addr)          (((const struct T_UINT16_READ *)(const void *)(addr))->v)
#endif
#ifndef   UNALIGNED_UINT32_WRITE
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpacked"
  #pragma GCC diagnostic ignored "-Wattributes"
  PACKED_STRUCT T_UINT32_WRITE { uint32_t v; };
  #pragma GCC diagnostic pop
  #define UNALIGNED_UINT32_WRITE(addr, val)    (void)((((struct T_UINT32_WRITE *)(void *)(addr))->v) = (val))
#endif
#ifndef   UNALIGNED_UINT32_READ
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpacked"
  #pragma GCC diagnostic ignored "-Wattributes"
  PACKED_STRUCT T_UINT32_READ { uint32_t v; };
  #pragma GCC diagnostic pop
  #define UNALIGNED_UINT32_READ(addr)          (((const struct T_UINT32_READ *)(const void *)(addr))->v)
#endif
