//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7_system_clock.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "stm32h7xx.h"
#include <stdbool.h>
#include "./Digini/inc/lib_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CFG_LSI_VALUE                               32000
#define CFG_HSI_VALUE                               64000000
#define CFG_CSI_VALUE                               4000000
#define CFG_RC48_VALUE                              48000000

/*
// Own define for register bit value without cast (Do not change)
#define CFG_RCC_CFGR_SW_HSI                         0x00000000U
#define CFG_RCC_CFGR_SW_HSE                         0x00000001U
#define CFG_RCC_CFGR_SW_PLL                         0x00000002U
                #define RCC_CFGR_SW_Pos                        (0U)
                #define RCC_CFGR_SW_Msk                        (0x7UL << RCC_CFGR_SW_Pos)           // 0x00000007
                #define RCC_CFGR_SW                            RCC_CFGR_SW_Msk                     // SW[2:0] bits (System clock Switch)
                #define RCC_CFGR_SW_0                          (0x1UL << RCC_CFGR_SW_Pos)           // 0x00000001
                #define RCC_CFGR_SW_1                          (0x2UL << RCC_CFGR_SW_Pos)           // 0x00000002
                #define RCC_CFGR_SW_2                          (0x4UL << RCC_CFGR_SW_Pos)           // 0x00000004


#define CFG_RCC_PLLCFGR_PLLSRC_HSE                  0x00400000U
#define CFG_RCC_PLLCFGR_PLLSRC_HSI                  0x00000000U


#define CFG_RCC_PLL1DIVR
#define CFG_RCC_PLL1DIVP
#define CFG_RCC_PLL1DIVQ

#define CFG_RCC_PLL2DIVR
#define CFG_RCC_PLL2DIVP
#define CFG_RCC_PLL2DIVQ

#define CFG_RCC_PLL3DIVR
#define CFG_RCC_PLL3DIVP
#define CFG_RCC_PLL3DIVQ



#define CFG_RCC_CFGR_D1HPRE_DIV1                     ?? 0x00000000U
#define CFG_RCC_CFGR_D1HPRE_DIV2                     ?? 0x00000080U
#define CFG_RCC_CFGR_D1HPRE_DIV4                     ?? 0x00000090U
#define CFG_RCC_CFGR_D1HPRE_DIV8                     ?? 0x000000A0U
#define CFG_RCC_CFGR_D1HPRE_DIV16                    ?? 0x000000B0U
#define CFG_RCC_CFGR_D1HPRE_DIV64                    ?? 0x000000C0U
#define CFG_RCC_CFGR_D1HPRE_DIV128                   ?? 0x000000D0U
#define CFG_RCC_CFGR_D1HPRE_DIV256                   ?? 0x000000E0U
#define CFG_RCC_CFGR_D1HPRE_DIV512                   ?? 0x000000F0U

#define CFG_RCC_CFGR_D1PPRE_DIV1                     ?? 0x00000000U
#define CFG_RCC_CFGR_D1PPRE_DIV2                     ?? 0x00000080U
#define CFG_RCC_CFGR_D1PPRE_DIV4                     ?? 0x00000090U
#define CFG_RCC_CFGR_D1PPRE_DIV8                     ?? 0x000000A0U
#define CFG_RCC_CFGR_D1PPRE_DIV16                    ?? 0x000000B0U
#define CFG_RCC_CFGR_D1PPRE_DIV64                    ?? 0x000000C0U
#define CFG_RCC_CFGR_D1PPRE_DIV128                   ?? 0x000000D0U
#define CFG_RCC_CFGR_D1PPRE_DIV256                   ?? 0x000000E0U
#define CFG_RCC_CFGR_D1PPRE_DIV512                   ?? 0x000000F0U

#define CFG_RCC_CFGR_D2PPRE1_DIV1                     ?? 0x00000000U
#define CFG_RCC_CFGR_D2PPRE1_DIV2                     ?? 0x00000080U
#define CFG_RCC_CFGR_D2PPRE1_DIV4                     ?? 0x00000090U
#define CFG_RCC_CFGR_D2PPRE1_DIV8                     ?? 0x000000A0U
#define CFG_RCC_CFGR_D2PPRE1_DIV16                    ?? 0x000000B0U
#define CFG_RCC_CFGR_D2PPRE1_DIV64                    ?? 0x000000C0U
#define CFG_RCC_CFGR_D2PPRE1_DIV128                   ?? 0x000000D0U
#define CFG_RCC_CFGR_D2PPRE1_DIV256                   ?? 0x000000E0U
#define CFG_RCC_CFGR_D2PPRE1_DIV512                   ?? 0x000000F0U

#define CFG_RCC_CFGR_D2PPRE2_DIV1                     ?? 0x00000000U
#define CFG_RCC_CFGR_D2PPRE2_DIV2                     ?? 0x00000080U
#define CFG_RCC_CFGR_D2PPRE2_DIV4                     ?? 0x00000090U
#define CFG_RCC_CFGR_D2PPRE2_DIV8                     ?? 0x000000A0U
#define CFG_RCC_CFGR_D2PPRE2_DIV16                    ?? 0x000000B0U
#define CFG_RCC_CFGR_D2PPRE2_DIV64                    ?? 0x000000C0U
#define CFG_RCC_CFGR_D2PPRE2_DIV128                   ?? 0x000000D0U
#define CFG_RCC_CFGR_D2PPRE2_DIV256                   ?? 0x000000E0U
#define CFG_RCC_CFGR_D2PPRE2_DIV512                   ?? 0x000000F0U

#define CFG_RCC_CFGR_D3PPRE_DIV1                     ?? 0x00000000U
#define CFG_RCC_CFGR_D3PPRE_DIV2                     ?? 0x00000080U
#define CFG_RCC_CFGR_D3PPRE_DIV4                     ?? 0x00000090U
#define CFG_RCC_CFGR_D3PPRE_DIV8                     ?? 0x000000A0U
#define CFG_RCC_CFGR_D3PPRE_DIV16                    ?? 0x000000B0U
#define CFG_RCC_CFGR_D3PPRE_DIV64                    ?? 0x000000C0U
#define CFG_RCC_CFGR_D3PPRE_DIV128                   ?? 0x000000D0U
#define CFG_RCC_CFGR_D3PPRE_DIV256                   ?? 0x000000E0U
#define CFG_RCC_CFGR_D3PPRE_DIV512                   ?? 0x000000F0U

#define CFG_RCC_CFGR_MCO1PRE_NO_DIV                 0x00000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV2                   0x04000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV3                   0x05000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV4                   0x06000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV5                   0x07000000U

            #define RCC_CFGR_MCO1PRE_Pos                   (18U)
            #define RCC_CFGR_MCO1PRE_Msk                   (0xFUL << RCC_CFGR_MCO1PRE_Pos)
            #define RCC_CFGR_MCO1PRE                       RCC_CFGR_MCO1PRE_Msk                    // 0x003C0000
            #define RCC_CFGR_MCO1PRE_0                     (0x1UL << RCC_CFGR_MCO1PRE_Pos)          // 0x00040000
            #define RCC_CFGR_MCO1PRE_1                     (0x2UL << RCC_CFGR_MCO1PRE_Pos)          // 0x00080000
            #define RCC_CFGR_MCO1PRE_2                     (0x4UL << RCC_CFGR_MCO1PRE_Pos)          // 0x00100000
            #define RCC_CFGR_MCO1PRE_3                     (0x8UL << RCC_CFGR_MCO1PRE_Pos)          // 0x00200000

            #define RCC_CFGR_MCO2PRE_Pos                   (25U)
            #define RCC_CFGR_MCO2PRE_Msk                   (0xFUL << RCC_CFGR_MCO2PRE_Pos)
            #define RCC_CFGR_MCO2PRE                       RCC_CFGR_MCO2PRE_Msk                    // 0x1E000000
            #define RCC_CFGR_MCO2PRE_0                     (0x1UL << RCC_CFGR_MCO2PRE_Pos)          // 0x02000000
            #define RCC_CFGR_MCO2PRE_1                     (0x2UL << RCC_CFGR_MCO2PRE_Pos)          // 0x04000000
            #define RCC_CFGR_MCO2PRE_2                     (0x4UL << RCC_CFGR_MCO2PRE_Pos)          // 0x08000000
            #define RCC_CFGR_MCO2PRE_3                     (0x8UL << RCC_CFGR_MCO2PRE_Pos)          // 0x10000000

#define CFG_RCC_CFGR_MCO1_HSI                       0x00000000U
#define CFG_RCC_CFGR_MCO1_LSE                       0x00200000U
#define CFG_RCC_CFGR_MCO1_HSE                       0x00400000U
#define CFG_RCC_CFGR_MCO1_PLL                       0x00600000U

#define CFG_RCC_CFGR_MCO2PRE_NO_DIV                 0x00000000U
#define CFG_RCC_CFGR_MCO2PRE_DIV2                   0x20000000U
#define CFG_RCC_CFGR_MCO2PRE_DIV3                   0x28000000U
#define CFG_RCC_CFGR_MCO2PRE_DIV4                   0x30000000U
#define CFG_RCC_CFGR_MCO2PRE_DIV5                   0x38000000U

#define CFG_RCC_CFGR_MCO2_SYSCLOCK                  0x00000000U
#define CFG_RCC_CFGR_MCO2_PLLI2S                    0x40000000U
#define CFG_RCC_CFGR_MCO2_HSE                       0x80000000U
#define CFG_RCC_CFGR_MCO2_PLL                       0xC0000000U

*/

//-------------------------------------------------------------------------------------------------
// Multiplexer for peripheral clock source

// register to config
// RCC_D1CCIPR
// RCC_D2CCIP1R
// RCC_D2CCIP2R
// RCC_D3CCIPR
// RCC_BDCR_RTCSEL_0+ other config

// Multiplexer define to be used for ADC clock source                                                   -> CFG_ADC_SOURCE_MUX           default: PLL2P
#define CFG_RCC_D3CCIPR_ADC_PLL2P               0
#define CFG_RCC_D3CCIPR_ADC_PLL3R               RCC_D3CCIPR_ADCSEL_0
#define CFG_RCC_D3CCIPR_ADC_PER_CLK             RCC_D3CCIPR_ADCSEL_1

// Multiplexer define to be used for HDMI CEC clock source                                              -> CFG_CEC_SOURCE_MUX           default: LSE
#define CFG_RCC_D2CCIP2R_CEC_LSE                0
#define CFG_RCC_D2CCIP2R_CEC_LSI                RCC_D2CCIP2R_CECSEL_0
#define CFG_RCC_D2CCIP2R_CEC_CSI                RCC_D2CCIP2R_CECSEL_1

// Multiplexer define to be used for DFSDM clock source                                                 -> CFG_DFSDM_SOURCE_MUX         default: PCLK2
#define CFG_RCC_D2CCIP1R_DFSDM_PLCK2            0
#define CFG_RCC_D2CCIP1R_DFSDM_SYSCLK           RCC_D2CCIP1R_DFSDM1SEL

// Multiplexer define to be used for FDCAN clock source                                                 -> CFG_FDCAN_SOURCE_MUX         default: HSE
#define CFG_RCC_D2CCIP1R_FDCAN_HSE             0
#define CFG_RCC_D2CCIP1R_FDCAN_PLL1Q           RCC_D2CCIP1R_FDCANSEL_0
#define CFG_RCC_D2CCIP1R_FDCAN_PLL2Q           RCC_D2CCIP1R_FDCANSEL_1

// Multiplexer define to be used for FMC clock source                                                   -> CFG_FMC_SOURCE_MUX           default: HCLK3
#define CFG_RCC_D1CCIPR_FMC_HCLK3              0
#define CFG_RCC_D1CCIPR_FMC_PLL1Q              RCC_D1CCIPR_FMCSEL_0
#define CFG_RCC_D1CCIPR_FMC_PLL2R              RCC_D1CCIPR_FMCSEL_1
#define CFG_RCC_D1CCIPR_FMC_PER_CLK            (RCC_D1CCIPR_FMCSEL_1 | RCC_D1CCIPR_FMCSEL_0)

// Multiplexer define to be used for I2C123 clock source                                                -> CFG_I2C123_SOURCE_MUX        default: PCLK1
#define CFG_RCC_D2CCIP2R_I2C123_PCLK1           0
#define CFG_RCC_D2CCIP2R_I2C123_PLL3R           RCC_D2CCIP2R_I2C123SEL_0
#define CFG_RCC_D2CCIP2R_I2C123_HSI             RCC_D2CCIP2R_I2C123SEL_1
#define CFG_RCC_D2CCIP2R_I2C123_CSI             (RCC_D2CCIP2R_I2C123SEL_1 | RCC_D2CCIP2R_I2C123SEL_0)

// Multiplexer define to be used for I2C4 clock source                                                  -> CFG_I2C4_SOURCE_MUX          default: PCLK4
#define CFG_RCC_D3CCIPR_I2C4_PCLK4              0
#define CFG_RCC_D3CCIPR_I2C4_PLL3R              RCC_D3CCIPR_I2C4SEL_0
#define CFG_RCC_D3CCIPR_I2C4_HSI                RCC_D3CCIPR_I2C4SEL_1
#define CFG_RCC_D3CCIPR_I2C4_CSI                (RCC_D3CCIPR_I2C4SEL_1 | RCC_D3CCIPR_I2C4SEL_0)

// Multiplexer define to be used for HRTIM clock source                                                 -> CFG_HRTIM_SOURCE_MUX         default: TIM_CLK
#define CFG_RCC_CFGR_HRTIM_TIM_CLK              0
#define CFG_RCC_CFGR_HRTIM_CPU1_CLK             RCC_CFGR_HRTIMSEL

// Multiplexer define to be used for LPTIM1 clock source                                                -> CFG_LPTIM1_SOURCE_MUX        default: PCLK1
#define CFG_RCC_D2CCIP2R_LPTIM1_PCLK1           0
#define CFG_RCC_D2CCIP2R_LPTIM1_PLL2P           RCC_D2CCIP2R_LPTIM1SEL_0
#define CFG_RCC_D2CCIP2R_LPTIM1_PLL3R           RCC_D2CCIP2R_LPTIM1SEL_1
#define CFG_RCC_D2CCIP2R_LPTIM1_LSE             (RCC_D2CCIP2R_LPTIM1SEL_1 | RCC_D2CCIP2R_LPTIM1SEL_0)
#define CFG_RCC_D2CCIP2R_LPTIM1_LSI             RCC_D2CCIP2R_LPTIM1SEL_2
#define CFG_RCC_D2CCIP2R_LPTIM1_PER_CLK         (RCC_D2CCIP2R_LPTIM1SEL_2 | RCC_D2CCIP2R_LPTIM1SEL_0)

// Multiplexer define to be used for LPTIM2 clock source                                                -> CFG_LPTIM2_SOURCE_MUX        default: PCLK4
#define CFG_RCC_D3CCIPR_LPTIM2_PCLK4            0
#define CFG_RCC_D3CCIPR_LPTIM2_PLL2P            RCC_D3CCIPR_LPTIM2SEL_0
#define CFG_RCC_D3CCIPR_LPTIM2_PLL3R            RCC_D3CCIPR_LPTIM2SEL_1
#define CFG_RCC_D3CCIPR_LPTIM2_LSE              (RCC_D3CCIPR_LPTIM2SEL_1 | RCC_D3CCIPR_LPTIM2SEL_0)
#define CFG_RCC_D3CCIPR_LPTIM2_LSI              RCC_D3CCIPR_LPTIM2SEL_2
#define CFG_RCC_D3CCIPR_LPTIM2_PER_CLK          (RCC_D3CCIPR_LPTIM2SEL_2 | RCC_D3CCIPR_LPTIM2SEL_0)

// Multiplexer define to be used for LPTIM3, LPTIM4, LPTIM5 clock source                                -> CFG_LPTIM345_SOURCE_MUX      default:PCLK4
#define CFG_RCC_D3CCIPR_LPTIM345_PCLK4          0       
#define CFG_RCC_D3CCIPR_LPTIM345_PLL2P          RCC_D3CCIPR_LPTIM345SEL_0
#define CFG_RCC_D3CCIPR_LPTIM345_PLL3R          RCC_D3CCIPR_LPTIM345SEL_1
#define CFG_RCC_D3CCIPR_LPTIM345_LSE            (RCC_D3CCIPR_LPTIM345SEL_1 | RCC_D3CCIPR_LPTIM345SEL_0)
#define CFG_RCC_D3CCIPR_LPTIM345_LSI            RCC_D3CCIPR_LPTIM345SEL_2
#define CFG_RCC_D3CCIPR_LPTIM345_PER_CLK        (RCC_D3CCIPR_LPTIM345SEL_2 | RCC_D3CCIPR_LPTIM345SEL_0)

// Multiplexer define to be used for LPUART1 clock source                                               -> CFG_LPUART1_SOURCE_MUX       default: PCLK3
#define CFG_RCC_D3CCIPR_LPUART1_PCLK3           0
#define CFG_RCC_D3CCIPR_LPUART1_PLL2Q           RCC_D3CCIPR_LPUART1SEL_0
#define CFG_RCC_D3CCIPR_LPUART1_PLL3Q           RCC_D3CCIPR_LPUART1SEL_1
#define CFG_RCC_D3CCIPR_LPUART1_HSI             (RCC_D3CCIPR_LPUART1SEL_1 | RCC_D3CCIPR_LPUART1SEL_0)
#define CFG_RCC_D3CCIPR_LPUART1_CSI             RCC_D3CCIPR_LPUART1SEL_2
#define CFG_RCC_D3CCIPR_LPUART1_LSE             (RCC_D3CCIPR_LPUART1SEL_2 | RCC_D3CCIPR_LPUART1SEL_0)

// Multiplexer define to be used for MCO1 clock source                                                  -> CFG_MCO1_SOURCE_MUX          default: SYS_CLK
#define CFG_RCC_CFGR_MCO1_SYS_CLK               0
#define CFG_RCC_CFGR_MCO1_HSI                   RCC_CFGR_MCO1SEL_0
#define CFG_RCC_CFGR_MCO1_LSE                   RCC_CFGR_MCO1SEL_1
#define CFG_RCC_CFGR_MCO1_HSE                   (RCC_CFGR_MCO1SEL_1 | RCC_CFGR_MCO1SEL_0)
#define CFG_RCC_CFGR_MCO1_PLL1Q                 RCC_CFGR_MCO1SEL_2
#define CFG_RCC_CFGR_MCO1_HSI48                 (RCC_CFGR_MCO1SEL_2 | RCC_CFGR_MCO1SEL_0)

// Multiplexer define to be used for MCO2 clock source                                                  -> CFG_MCO2_SOURCE_MUX          default: SYS_CLK
#define CFG_RCC_CFGR_MCO2_SYS_CLK               0
#define CFG_RCC_CFGR_MCO2_PLL2P                 RCC_CFGR_MCO2SEL_0
#define CFG_RCC_CFGR_MCO2_HSE                   RCC_CFGR_MCO2SEL_1
#define CFG_RCC_CFGR_MCO2_PLL1P                 (RCC_CFGR_MCO2SEL_1 | RCC_CFGR_MCO2SEL_0)
#define CFG_RCC_CFGR_MCO2_CSI                   RCC_CFGR_MCO2SEL_2
#define CFG_RCC_CFGR_MCO2_LSI                   (RCC_CFGR_MCO2SEL_2 | RCC_CFGR_MCO2SEL_0)

// Multiplexer define to be used for PERIPHERAL clock source                                            -> CFG_PER_SOURCE_MUX           default: HSI
#define CFG_RCC_D1CCIPR_PER_HSI_KER             0              
#define CFG_RCC_D1CCIPR_PER_CSI_KER             RCC_D1CCIPR_CKPERSEL_0
#define CFG_RCC_D1CCIPR_PER_HSE_KER             RCC_D1CCIPR_CKPERSEL_1

// Multiplexer define to be used for QUADSPI clock source                                               -> CFG_QSPI_SOURCE_MUX          default: PLL1Q
#define CFG_RCC_D1CCIPR_QSPI_HCLK3              0
#define CFG_RCC_D1CCIPR_QSPI_PLL1Q              RCC_D1CCIPR_QSPISEL_0
#define CFG_RCC_D1CCIPR_QSPI_PLL2R              RCC_D1CCIPR_QSPISEL_1
#define CFG_RCC_D1CCIPR_QSPI_PER_CLK            (RCC_D1CCIPR_QSPISEL_1 | RCC_D1CCIPR_QSPISEL_0)

// Multiplexer define to be used for RTC clock source                                                   -> CFG_QSPI_SOURCE_MUX          default: None
#define CFG_RCC_BDCR_RTC_LSE                    RCC_BDCR_RTCSEL_0
#define CFG_RCC_BDCR_RTC_LSI                    RCC_BDCR_RTCSEL_1
#define CFG_RCC_BDCR_RTC_HSE                    (RCC_BDCR_RTCSEL_1 | RCC_BDCR_RTCSEL_0)

// Multiplexer define to be used for RNG clock source                                                   -> CFG_RNG_SOURCE_MUX           default: HSI48
#define CFG_RCC_D2CCIP2R_RNG_HSI48              0
#define CFG_RCC_D2CCIP2R_RNG_PLL1Q              RCC_D2CCIP2R_RNGSEL_0
#define CFG_RCC_D2CCIP2R_RNG_LSE                RCC_D2CCIP2R_RNGSEL_1
#define CFG_RCC_D2CCIP2R_RNG_LSI                (RCC_D2CCIP2R_RNGSEL_1 | RCC_D2CCIP2R_RNGSEL_0)

// Multiplexer define to be used for SAI1 clock source                                                  -> CFG_SAI1_SOURCE_MUX          default: PLL1Q
#define CFG_RCC_D2CCIP1R_SAI1_PLL1Q             0
#define CFG_RCC_D2CCIP1R_SAI1_PLL2P             RCC_D2CCIP1R_SAI1SEL_0
#define CFG_RCC_D2CCIP1R_SAI1_PLL3P             RCC_D2CCIP1R_SAI1SEL_1
#define CFG_RCC_D2CCIP1R_SAI1_I2SCKIN           (RCC_D2CCIP1R_SAI1SEL_1 | RCC_D2CCIP1R_SAI1SEL_0)
#define CFG_RCC_D2CCIP1R_SAI1_PER_CLK           RCC_D2CCIP1R_SAI1SEL_2

// Multiplexer define to be used for SAI2, SAI3 clock source                                            -> CFG_SAI23_SOURCE_MUX         default: PLL1Q
#define CFG_RCC_D2CCIP1R_SAI23_PLL1Q            0
#define CFG_RCC_D2CCIP1R_SAI23_PLL2P            RCC_D2CCIP1R_SAI23SEL_0
#define CFG_RCC_D2CCIP1R_SAI23_PLL3P            RCC_D2CCIP1R_SAI23SEL_1
#define CFG_RCC_D2CCIP1R_SAI23_I2SCKIN          (RCC_D2CCIP1R_SAI23SEL_1 | RCC_D2CCIP1R_SAI23SEL_0)
#define CFG_RCC_D2CCIP1R_SAI23_PER_CLK          RCC_D2CCIP1R_SAI23SEL_2

// Multiplexer define to be used for SAI4A clock source                                                 -> CFG_SAI4B_SOURCE_MUX         default: PLL1Q
#define CFG_RCC_D3CCIPR_SAI4A_PLL1Q             0
#define CFG_RCC_D3CCIPR_SAI4A_PLL2P             RCC_D3CCIPR_SAI4ASEL_0
#define CFG_RCC_D3CCIPR_SAI4A_PLL3P             RCC_D3CCIPR_SAI4ASEL_1
#define CFG_RCC_D3CCIPR_SAI4A_I2SCKIN           (RCC_D3CCIPR_SAI4ASEL_1 | RCC_D3CCIPR_SAI4ASEL_0)
#define CFG_RCC_D3CCIPR_SAI4A_PER_CLK           RCC_D3CCIPR_SAI4ASEL_2

// Multiplexer define to be used for SAI4B clock source                                                 -> CFG_SAI4B_SOURCE_MUX         default: PLL1Q
#define CFG_RCC_D3CCIPR_SAI4B_PLL1Q             0
#define CFG_RCC_D3CCIPR_SAI4B_PLL2P             RCC_D3CCIPR_SAI4BSEL_0
#define CFG_RCC_D3CCIPR_SAI4B_PLL3P             RCC_D3CCIPR_SAI4BSEL_1
#define CFG_RCC_D3CCIPR_SAI4B_I2SCKIN           (RCC_D3CCIPR_SAI4BSEL_1 | RCC_D3CCIPR_SAI4BSEL_0)
#define CFG_RCC_D3CCIPR_SAI4B_PER_CLK           RCC_D3CCIPR_SAI4BSEL_2

// Multiplexer define to be used for SDMMC clock source                                                 -> CFG_SDMMC_SOURCE_MUX         default: PLLQ1
#define CFG_RCC_D1CCIPR_SDMMC_PLL1Q             0
#define CFG_RCC_D1CCIPR_SDMMC_PLL2R             RCC_D1CCIPR_SDMMCSEL

// Multiplexer define to be used for SPDIF clock source                                                 -> CFG_SPDIF_SOURCE_MUX         default: PLL1Q
#define CFG_RCC_D2CCIP1R_SPDIF_PLL1Q            0
#define CFG_RCC_D2CCIP1R_SPDIF_PLL2R            RCC_D2CCIP1R_SPDIFSEL_0
#define CFG_RCC_D2CCIP1R_SPDIF_PLL3R            RCC_D2CCIP1R_SPDIFSEL_1
#define CFG_RCC_D2CCIP1R_SPDIF_HSI              (RCC_D2CCIP1R_SPDIFSEL_1 | RCC_D2CCIP1R_SPDIFSEL_0)

// Multiplexer define to be used for SPI1, SPI2, SPI3 clock source                                      -> CFG_SPI123_SOURCE_MUX        default: PLL1Q
#define CFG_RCC_D2CCIP1R_SPI123_PLL1Q           0
#define CFG_RCC_D2CCIP1R_SPI123_PLL2P           RCC_D2CCIP1R_SPI123SEL_0
#define CFG_RCC_D2CCIP1R_SPI123_PLL3P           RCC_D2CCIP1R_SPI123SEL_1
#define CFG_RCC_D2CCIP1R_SPI123_I2SCKIN         (RCC_D2CCIP1R_SPI123SEL_1 | RCC_D2CCIP1R_SPI123SEL_0)
#define CFG_RCC_D2CCIP1R_SPI123_PER_CLK         RCC_D2CCIP1R_SPI123SEL_2

// Multiplexer define to be used for SPI4, SPI5 clock source                                            -> CFG_SPI45_SOURCE_MUX         default: PCLK2
#define CFG_RCC_D2CCIP1R_SPI45_PLCK2            0
#define CFG_RCC_D2CCIP1R_SPI45_PLL2Q            RCC_D2CCIP1R_SPI45SEL_0
#define CFG_RCC_D2CCIP1R_SPI45_PLL3Q            RCC_D2CCIP1R_SPI45SEL_1
#define CFG_RCC_D2CCIP1R_SPI45_HSI              (RCC_D2CCIP1R_SPI45SEL_1 | RCC_D2CCIP1R_SPI45SEL_0)
#define CFG_RCC_D2CCIP1R_SPI45_CSI              RCC_D2CCIP1R_SPI45SEL_2
#define CFG_RCC_D2CCIP1R_SPI45_HSE              (RCC_D2CCIP1R_SPI45SEL_2 | RCC_D2CCIP1R_SPI45SEL_0)

// Multiplexer define to be used for SPI6 clock source                                                  -> CFG_SPI6_SOURCE_MUX          default: PCLK4
#define CFG_RCC_D3CCIPR_SPI6_PLCK4              0
#define CFG_RCC_D3CCIPR_SPI6_PLL2Q              RCC_D3CCIPR_SPI6SEL_0
#define CFG_RCC_D3CCIPR_SPI6_PLL3Q              RCC_D3CCIPR_SPI6SEL_1
#define CFG_RCC_D3CCIPR_SPI6_HSI                (RCC_D3CCIPR_SPI6SEL_1 | RCC_D3CCIPR_SPI6SEL_0)
#define CFG_RCC_D3CCIPR_SPI6_CSI                RCC_D3CCIPR_SPI6SEL_2
#define CFG_RCC_D3CCIPR_SPI6_HSE                (RCC_D3CCIPR_SPI6SEL_2 | RCC_D3CCIPR_SPI6SEL_0)

// Multiplexer define to be used for SWP clock source                                                   -> CFG_SWP_SOURCE_MUX           default: PCLK1
#define CFG_RCC_D2CCIP1R_SWP_PCLK1              0
#define CFG_RCC_D2CCIP1R_SWP_HSI                RCC_D2CCIP1R_SWPSEL

// Multiplexer define to be used for PER clock source 
#define CFG_TRACE_SOURCE_MUX                        HSI //???

//not found !!

// Multiplexer define to be used for UART1, UART6 clock source                                          -> CFG_UART16_SOURCE_MUX        default: PCLK2
#define CFG_RCC_D2CCIP2R_USART16_PCLK2          0
#define CFG_RCC_D2CCIP2R_USART16_PLL2Q          RCC_D2CCIP2R_USART16SEL_0
#define CFG_RCC_D2CCIP2R_USART16_PLL3Q          RCC_D2CCIP2R_USART16SEL_1
#define CFG_RCC_D2CCIP2R_USART16_HSI            (RCC_D2CCIP2R_USART16SEL_1 | RCC_D2CCIP2R_USART16SEL_0)
#define CFG_RCC_D2CCIP2R_USART16_CSI            RCC_D2CCIP2R_USART16SEL_2
#define CFG_RCC_D2CCIP2R_USART16_LSE            (RCC_D2CCIP2R_USART16SEL_2 | RCC_D2CCIP2R_USART16SEL_0)

// Multiplexer define to be used for UART2, UART3, UART4, UART5, UART7 and UART8 clock source           -> CFG_UART234578_SOURCE_MUX    default: PCLK1
#define CFG_RCC_D2CCIP2R_USART234578_PCLK1      0
#define CFG_RCC_D2CCIP2R_USART234578_PLL2Q      RCC_D2CCIP2R_USART28SEL_0
#define CFG_RCC_D2CCIP2R_USART234578_PLL3Q      RCC_D2CCIP2R_USART28SEL_1
#define CFG_RCC_D2CCIP2R_USART234578_HSI        (RCC_D2CCIP2R_USART28SEL_1 | RCC_D2CCIP2R_USART28SEL_0)
#define CFG_RCC_D2CCIP2R_USART234578_CSI        RCC_D2CCIP2R_USART28SEL_2
#define CFG_RCC_D2CCIP2R_USART234578_LSE        (RCC_D2CCIP2R_USART28SEL_2 | RCC_D2CCIP2R_USART28SEL_0)

// Multiplexer define to be used for USB clock source                                                   -> CFG_USB_SOURCE_MUX           default: PLL1Q
#define CFG_RCC_D2CCIP2R_USB_PLL1Q              0
#define CFG_RCC_D2CCIP2R_USB_PLL3Q              RCC_D2CCIP2R_USBSEL_0
#define CFG_RCC_D2CCIP2R_USB_HSI48              RCC_D2CCIP2R_USBSEL_1

//-------------------------------------------------------------------------------------------------
// Configuration file(s)
//-------------------------------------------------------------------------------------------------

#include "clock_cfg.h"

//-------------------------------------------------------------------------------------------------
// Auto define configuration
//-------------------------------------------------------------------------------------------------

#define RCC_PLLCFGR_PLL_M_POS           0
#define RCC_PLLCFGR_PLL_N_POS           6
#define RCC_PLLCFGR_PLL_P_POS           16
#define RCC_PLLCFGR_PLL_Q_POS           24

// --------------------------------------------------------------------------------------------------------------------------------
// Autoconfig value for PLL

#if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

  #define SYS_PLL_CLK_FREQUENCY                     (((CFG_PLLM_SOURCE / CFG_PLLM_DIVIDER) * CFG_PLLM_N_MULTIPLIER) / CFG_PLLM_P_DIVIDER)
  #define SYS_PLL_Q_FREQUENCY                       (((CFG_PLLM_SOURCE / CFG_PLLM_DIVIDER) * CFG_PLLM_N_MULTIPLIER) / CFG_PLLM_Q_DIVIDER)

  #if (CFG_PLLM_DIVIDER < 2) || (CFG_PLLM_DIVIDER > 63)
    #pragma message XSTR(CFG_PLLM_DIVIDER)
    #error PLLM is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLM                   (CFG_PLLM_DIVIDER << RCC_PLLCFGR_PLL_M_POS)
  #endif

  #if (CFG_PLLM_N_MULTIPLIER < 50) || (CFG_PLLM_N_MULTIPLIER > 432)
    #pragma message "XSTR(CFG_PLLM_N_MULTIPLIER)"
    #error PLLN is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLN                   (CFG_PLLM_N_MULTIPLIER << RCC_PLLCFGR_PLL_N_POS)
  #endif

  #if ((CFG_PLLM_P_DIVIDER / 2) < 1) || ((CFG_PLLM_P_DIVIDER / 2) > 4)   // (tested for 2,4,6,8)
    #pragma message "XSTR(CFG_PLLM_P_DIVIDER)"
    #error PLLP is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLP                   (((CFG_PLLM_P_DIVIDER / 2) - 1) << RCC_PLLCFGR_PLL_P_POS)
  #endif

  #if (CFG_PLLM_Q_DIVIDER < 2) || (CFG_PLLM_Q_DIVIDER > 15)
    #pragma message "XSTR(CFG_PLLM_Q_DIVIDER)"
    #error PLLQ is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLQ                   (CFG_PLLM_Q_DIVIDER << RCC_PLLCFGR_PLL_Q_POS)
  #endif

  #define CFG_RCC_PLL_CFGR                          (CFG_RCC_PLL_CFGR_PLLM |   \
                                                     CFG_RCC_PLL_CFGR_PLLN |   \
                                                     CFG_RCC_PLL_CFGR_PLLP |   \
                                                     CFG_RCC_PLL_CFGR_PLLQ |   \
                                                     CFG_RCC_PLLCFGR_PLLSRC)

#endif // CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL

// --------------------------------------------------------------------------------------------------------------------------------

#if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              SYS_PLL_CLK_FREQUENCY
#elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSE)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_HSE_VALUE
#else //(CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSI)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_HSI_VALUE
#endif

// --------------------------------------------------------------------------------------------------------------------------------

#define CFG_SYS_HCLK                                CFG_HCLK_DIVIDER

#if   CFG_HCLK_DIVIDER == CFG_CFG_RCC_CFGR_HPRE_DIV1
    #define SYS_HCLK_CLOCK_FREQUENCY                SYS_CPU_CORE_CLOCK_FREQUENCY
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV2
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 2)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV4
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 4)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV8
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 8)
#elif CFG_HCLK_DIVIDER == CFG_CFG_RCC_CFGR_HPRE_DIV16
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 16)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV64
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 64)
#elif CFG_HCLK_DIVIDER == CFG_CFG_RCC_CFGR_HPRE_DIV128
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 128)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV256
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 256)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV512
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 512)
#endif

#define SYSTEM_CORE_CLOCK                           SYS_HCLK_CLOCK_FREQUENCY

// --------------------------------------------------------------------------------------------------------------------------------

#define CFG_SYS_APB1                                CFG_APB1_CLK_DIVIDER

#if   CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV1
    #define SYS_APB1_CLOCK_FREQUENCY                SYS_HCLK_CLOCK_FREQUENCY
#elif CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV2
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV4
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV8
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV16
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif

#define SYS_APB1_TIMER_CLOCK_FREQUENCY              (SYS_APB1_CLOCK_FREQUENCY * 2)

// --------------------------------------------------------------------------------------------------------------------------------

#define CFG_SYS_APB2                                CFG_APB2_CLK_DIVIDER

#if   CFG_APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV1
    #define SYS_APB2_CLOCK_FREQUENCY                SYS_HCLK_CLOCK_FREQUENCY
#elif CFG_APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV2
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif CFG_APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV4
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif CFG_APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV8
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif CFG_APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV16
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif

#define SYS_APB2_TIMER_CLOCK_FREQUENCY              (SYS_APB2_CLOCK_FREQUENCY * 2)

// --------------------------------------------------------------------------------------------------------------------------------

// Power scaling according to speed
#if (SYS_CPU_CORE_CLOCK_FREQUENCY <= 151000000)
#define POWER_REGULATOR_CFG                         PWR_CR1_VOS_0   // Voltage scale 3
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 180000000)
#define POWER_REGULATOR_CFG                         PWR_CR1_VOS_1   // Voltage scale 2
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 216000000)
#define POWER_REGULATOR_CFG                         PWR_CR1_VOS     // Voltage scale 1
#endif

// Flash Latency configuration for Voltage from 2.7V to 3.6V
#if   (SYS_CPU_CORE_CLOCK_FREQUENCY <= 30000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_0WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 60000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_1WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 90000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_2WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 120000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_3WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 150000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_4WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 180000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_5WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 210000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_6WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 216000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_7WS
#endif

// --------------------------------------------------------------------------------------------------------------------------------

#if   (SYS_CPU_CORE_CLOCK_FREQUENCY >= 150000000)
#define ETH_MACIIAR_CR_DIVIDER                      ETH_MACMIIAR_CR_Div102
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY >= 100000000)
#define ETH_MACIIAR_CR_DIVIDER                      ETH_MACMIIAR_CR_Div62
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY >= 60000000)
#define ETH_MACIIAR_CR_DIVIDER                      ETH_MACMIIAR_CR_Div42
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY >= 35000000)
#define ETH_MACIIAR_CR_DIVIDER                      ETH_MACMIIAR_CR_Div26
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY >= 25000000)
#define ETH_MACIIAR_CR_DIVIDER                      ETH_MACMIIAR_CR_Div16
#endif

// Verification
#if SYS_HCLK_CLOCK_FREQUENCY > 216000000
 #pragma message "XSTR(SYS_HCLK_CLOCK_FREQUENCY)"
 #error CPU Core frequency exceed maximum allowed!
#endif

#if SYS_APB1_CLOCK_FREQUENCY > 54000000
 #pragma message "XSTR(SYS_APB1_CLOCK_FREQUENCY)"
 #error APB1 frequency exceed maximum allowed!
#endif

#if SYS_APB2_CLOCK_FREQUENCY > 108000000
 #pragma message "XSTR(SYS_APB2_CLOCK_FREQUENCY)"
 #error APB2 frequency exceed maximum allowed!
#endif

//-------------------------------------------------------------------------------------------------
// Global Macro
//-------------------------------------------------------------------------------------------------

#ifdef SYSTEM_GLOBAL
    #define SYSTEM_EXTERN
#else
    #define SYSTEM_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SystemInit              (void);

//-------------------------------------------------------------------------------------------------


