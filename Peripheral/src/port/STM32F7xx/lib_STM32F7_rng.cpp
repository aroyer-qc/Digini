//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F7_rng.cpp
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

#define RNG_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  RNG_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_RNG_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   Initializes the random number generator
//
//-------------------------------------------------------------------------------------------------
void RNG_Initialize(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;      // Enable clock to module
    RNG->CR      |= RNG_CR_RNGEN;           // Enable the RNG Peripheral
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: RNG_GetRandom
//
//   Parameter(s):  None
//   Return value:  uint32_t Random value
//
//   Description:   Return a ramdom value from 0x00000000 to 0xFFFFFFFF
//
//-------------------------------------------------------------------------------------------------
uint32_t RNG_GetRandom(void)
{
    while((RNG->SR & RNG_SR_DRDY) == 0){};      // Should never jam;

    return RNG->DR;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: RNG_GetRandomFromRange
//
//   Parameter(s):  uint32_t    Minimum Value
//                  uint32_t    Maximum value
//   Return value:  uint32_t    Random value
//
//   Description:   Return a ramdom value in the provide range
//
//-------------------------------------------------------------------------------------------------
uint32_t RNG_GetRandomFromRange(uint32_t Min, uint32_t Max)
{
    uint32_t CalculatedRandom;
    uint32_t RandomValue;

    CalculatedRandom = (Max - Min) + 1;             // This is the range
    while((RNG->SR & RNG_SR_DRDY) == 0){};          // Should never jam;
    RandomValue = RNG->DR;

    return uint32_t(uint64_t(RandomValue * uint64_t(CalculatedRandom)) >> sizeof(uint32_t)) + Min;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_RNG_DRIVER == DEF_ENABLED)
