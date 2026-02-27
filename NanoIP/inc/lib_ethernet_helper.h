//-------------------------------------------------------------------------------------------------
//
//  File : lib_ethernet_helper.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Macro(s)
//-------------------------------------------------------------------------------------------------

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

    #define IP_ADDRESS(A,B,C,D)                 (uint32_t(A) + (uint32_t(B) << 8) + (uint32_t(C) << 16) + (uint32_t(D) << 24))
    #define IP_A(IP)                            uint8_t(IP)
    #define IP_B(IP)                            uint8_t(IP >> 8)
    #define IP_C(IP)                            uint8_t(IP >> 16)
    #define IP_D(IP)                            uint8_t(IP >> 24)

    // Use on static value to save code space
    #define HTONS(V)                            uint16_t(uint16_t(V) << 8 | uint16_t(V) >> 8)
    #define HTONL(V)                            (uint32_t((((V) & 0x000000FF) << 24) | (((V) & 0x0000FF00) << 8 ) | (((V) & 0x00FF0000) >> 8 ) | (((V) & 0xFF000000) >> 24)))

#else

    #define IP_ADDRESS(A,B,C,D)                 (uint32_t(D) + (uint32_t(C) << 8) + (uint32_t(B) << 16) + (uint32_t(A) << 24))
    #define IP_A(IP)                            uint8_t(IP >> 24)
    #define IP_B(IP)                            uint8_t(IP >> 16)
    #define IP_C(IP)                            uint8_t(IP >> 8)
    #define IP_D(IP)                            uint8_t(IP)

    #define HTONS(V)                            uint16_t(V)
    #define HTONL(V)                            uint32_t(V)

#endif

//-------------------------------------------------------------------------------------------------
// Inline function(s)
//-------------------------------------------------------------------------------------------------

inline uint16_t htons(uint16_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return static_cast<uint16_t>(__REV16(x));
  #endif

    // ADD other CPU fast method to reverse 16 Bits order
}

inline uint16_t ntohs(uint16_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return static_cast<uint16_t>(__REV16(x));
  #endif

    // ADD other CPU fast method to reverse 16 Bits order
}

inline uint32_t htonl(uint32_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return __REV(x);
  #endif

    // ADD other CPU fast method to reverse 32 Bits order
}

inline uint32_t ntohl(uint32_t x)
{
  #if defined(__ARM_ARCH_6M__)   || defined(__ARM_ARCH_7M__)   || defined(__ARM_ARCH_7EM__)  || defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
    return __REV(x);
  #endif

    // ADD other CPU fast method to reverse 32 Bits order
}


//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


