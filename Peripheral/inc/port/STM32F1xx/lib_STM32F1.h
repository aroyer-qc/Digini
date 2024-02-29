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
// Note(s)
//-------------------------------------------------------------------------------------------------

// No support yet for TIM9 to TIM17 add as necessary
// Not all CPU are define, add has necessary

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#if defined (STM32F100xB)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_DISABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_DISABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F100xE)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_ENABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_ENABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F101x6)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_DISABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_DISABLED

    #define TIM1_SUPPORT        DEF_DISABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_DISABLED
    #define TIM5_SUPPORT        DEF_DISABLED
    #define TIM6_SUPPORT        DEF_DISABLED
    #define TIM7_SUPPORT        DEF_DISABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F101xB)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_DISABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_DISABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_DISABLED
    #define TIM6_SUPPORT        DEF_DISABLED
    #define TIM7_SUPPORT        DEF_DISABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F101xE)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_ENABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_DISABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_ENABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_DISABLED
 #endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F101xG)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_ENABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_DISABLED
    #define TIM2_SUPPORT        DEF_DISABLED
    #define TIM3_SUPPORT        DEF_DISABLED
    #define TIM4_SUPPORT        DEF_DISABLED
    #define TIM5_SUPPORT        DEF_ENABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F102x6)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_DISABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_DISABLED

    #define TIM1_SUPPORT        DEF_DISABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_DISABLED
    #define TIM5_SUPPORT        DEF_DISABLED
    #define TIM6_SUPPORT        DEF_DISABLED
    #define TIM7_SUPPORT        DEF_DISABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F102xB)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_DISABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_DISABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_DISABLED
    #define TIM6_SUPPORT        DEF_DISABLED
    #define TIM7_SUPPORT        DEF_DISABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F103x6)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_DISABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_DISABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_DISABLED
    #define TIM5_SUPPORT        DEF_DISABLED
    #define TIM6_SUPPORT        DEF_DISABLED
    #define TIM7_SUPPORT        DEF_DISABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F103xB)
    #define NUMBER_OF_IO_PORT   6
    #define DMA2_SUPPORT        DEF_DISABLED
    
    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_DISABLED
    #define TIM6_SUPPORT        DEF_DISABLED
    #define TIM7_SUPPORT        DEF_DISABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F103xE)
    #define NUMBER_OF_IO_PORT   8
    #define DMA2_SUPPORT        DEF_ENABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_ENABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F103xG)
    #define NUMBER_OF_IO_PORT   8
    #define DMA2_SUPPORT        DEF_ENABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_ENABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F105xC)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_ENABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_ENABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------

#if defined (STM32F107xC)
    #define NUMBER_OF_IO_PORT   
    #define DMA2_SUPPORT        DEF_ENABLED

    #define I2C1_SUPPORT        DEF_ENABLED
    #define I2C2_SUPPORT        DEF_ENABLED

    #define TIM1_SUPPORT        DEF_ENABLED
    #define TIM2_SUPPORT        DEF_ENABLED
    #define TIM3_SUPPORT        DEF_ENABLED
    #define TIM4_SUPPORT        DEF_ENABLED
    #define TIM5_SUPPORT        DEF_ENABLED
    #define TIM6_SUPPORT        DEF_ENABLED
    #define TIM7_SUPPORT        DEF_ENABLED
    #define TIM8_SUPPORT        DEF_DISABLED
#endif

//-------------------------------------------------------------------------------------------------
