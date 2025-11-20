//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F7_system_clock.h
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
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "stm32f7xx.h"
#include <stdbool.h>
#include "./Digini/inc/lib_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define ETH_MTU_SIZE                                1500

#define LSI_CLOCK_FREQUENCY                         32000
#define HSI_CLOCK_FREQUENCY                         16000000
#define LSE_CLOCK_FREQUENCY                         32768

// Own define for register bit value without cast (Do not change)
#define CFG_RCC_CFGR_SW_HSI                         0x00000000U
#define CFG_RCC_CFGR_SW_HSE                         0x00000001U
#define CFG_RCC_CFGR_SW_PLL                         0x00000002U

#define CFG_RCC_PLLCFGR_PLLSRC_HSE                  0x00400000U
#define CFG_RCC_PLLCFGR_PLLSRC_HSI                  0x00000000U

#define CFG_RCC_CFGR_HPRE_DIV1                      0x00000000U
#define CFG_RCC_CFGR_HPRE_DIV2                      0x00000080U
#define CFG_RCC_CFGR_HPRE_DIV4                      0x00000090U
#define CFG_RCC_CFGR_HPRE_DIV8                      0x000000A0U
#define CFG_RCC_CFGR_HPRE_DIV16                     0x000000B0U
#define CFG_RCC_CFGR_HPRE_DIV64                     0x000000C0U
#define CFG_RCC_CFGR_HPRE_DIV128                    0x000000D0U
#define CFG_RCC_CFGR_HPRE_DIV256                    0x000000E0U
#define CFG_RCC_CFGR_HPRE_DIV512                    0x000000F0U

#define CFG_RCC_CFGR_PPRE1_DIV1                     0x00000000U
#define CFG_RCC_CFGR_PPRE1_DIV2                     0x00001000U
#define CFG_RCC_CFGR_PPRE1_DIV4                     0x00001400U
#define CFG_RCC_CFGR_PPRE1_DIV8                     0x00001800U
#define CFG_RCC_CFGR_PPRE1_DIV16                    0x00001C00U

#define CFG_RCC_CFGR_PPRE2_DIV1                     0x00000000U
#define CFG_RCC_CFGR_PPRE2_DIV2                     0x00008000U
#define CFG_RCC_CFGR_PPRE2_DIV4                     0x0000A000U
#define CFG_RCC_CFGR_PPRE2_DIV8                     0x0000C000U
#define CFG_RCC_CFGR_PPRE2_DIV16                    0x0000E000U

#define CFG_RCC_CFGR_MCO1PRE_NO_DIV                 0x00000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV2                   0x04000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV3                   0x05000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV4                   0x06000000U
#define CFG_RCC_CFGR_MCO1PRE_DIV5                   0x07000000U

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

#define CFG_RCC_DCKCFGR1_PLLSAI_Q_DIV(x)            ((x - 1) << RCC_DCKCFGR1_PLLSAIDIVQ_Pos)

#define CFG_RCC_DCKCFGR1_PLLSAI_R_DIV2              0x00000000U
#define CFG_RCC_DCKCFGR1_PLLSAI_R_DIV4              RCC_DCKCFGR1_PLLSAIDIVR_0
#define CFG_RCC_DCKCFGR1_PLLSAI_R_DIV8              RCC_DCKCFGR1_PLLSAIDIVR_1
#define CFG_RCC_DCKCFGR1_PLLSAI_R_DIV16             (RCC_DCKCFGR1_PLLSAIDIVR_1 | RCC_DCKCFGR1_PLLSAIDIVR_0)

#define CFG_RCC_DCKCFGR1_PLLI2S_Q_DIV(x)            ((x - 1) << RCC_DCKCFGR1_PLLI2SDIVQ_Pos)

//-------------------------------------------------------------------------------------------------
// Multiplexer for peripheral clock source (TODO missing one)

#define CFG_CEC_LSE                                 0
#define CFG_CEC_HSI_DIV_488                         RCC_DCKCFGR2_CECSEL_0

#define CFG_CK48M_PLLQ                              0
#define CFG_CK48M_PLLSAIP                           RCC_DCKCFGR2_CK48MSEL_0

#define CFG_I2C1_PCLK1                              0
#define CFG_I2C1_SYS_CLOCK                          RCC_DCKCFGR2_I2C1SEL_0
#define CFG_I2C1_HSI                                RCC_DCKCFGR2_I2C1SEL_1

#define CFG_I2C2_PCLK1                              0
#define CFG_I2C2_SYS_CLOCK                          RCC_DCKCFGR2_I2C2SEL_0
#define CFG_I2C2_HSI                                RCC_DCKCFGR2_I2C2SEL_1

#define CFG_I2C3_PCLK1                              0
#define CFG_I2C3_SYS_CLOCK                          RCC_DCKCFGR2_I2C3SEL_0
#define CFG_I2C3_HSI                                RCC_DCKCFGR2_I2C3SEL_1

#define CFG_I2C4_PCLK1                              0
#define CFG_I2C4_SYS_CLOCK                          RCC_DCKCFGR2_I2C4SEL_0
#define CFG_I2C4_HSI                                RCC_DCKCFGR2_I2C4SEL_1

#define CFG_LPTIM1_PCLK1                            0
#define CFG_LPTIM1_LSI                              RCC_DCKCFGR2_LPTIM1SEL_0
#define CFG_LPTIM1_HSI                              RCC_DCKCFGR2_LPTIM1SEL_1
#define CFG_LPTIM1_LSE                              (RCC_DCKCFGR2_LPTIM1SEL_1 | RCC_DCKCFGR2_LPTIM1SEL_0)

#define CFG_SAI1_PLLSAIQ                            0
#define CFG_SAI1_PLLI2SQ                            RCC_DCKCFGR1_SAI1SEL_0
#define CFG_SAI1_I2S_CKIN                           RCC_DCKCFGR1_SAI1SEL_1

#define CFG_SAI2_PLLSAIQ                            0
#define CFG_SAI2_PLLI2SQ                            RCC_DCKCFGR1_SAI2SEL_0
#define CFG_SAI2_I2S_CKIN                           RCC_DCKCFGR1_SAI2SEL_1

#define CFG_SDMMC1_PLL48_CLOCK                      0
#define CFG_SDMMC1_SYS_CLOCK                        RCC_DCKCFGR2_SDMMC1SEL_0

#define CFG_USART1_PCLK2                            0
#define CFG_USART1_SYS_CLOCK                        RCC_DCKCFGR2_USART1SEL_0
#define CFG_USART1_HSI                              RCC_DCKCFGR2_USART1SEL_1
#define CFG_USART1_LSE                              (RCC_DCKCFGR2_USART1SEL_1 | RCC_DCKCFGR2_USART1SEL_0)

#define CFG_USART2_PCLK1                            0
#define CFG_USART2_SYS_CLOCK                        RCC_DCKCFGR2_USART2SEL_0
#define CFG_USART2_HSI                              RCC_DCKCFGR2_USART2SEL_1
#define CFG_USART2_LSE                              (RCC_DCKCFGR2_USART2SEL_1 | RCC_DCKCFGR2_USART2SEL_0)

#define CFG_USART3_PCLK1                            0
#define CFG_USART3_SYS_CLOCK                        RCC_DCKCFGR2_USART3SEL_0
#define CFG_USART3_HSI                              RCC_DCKCFGR2_USART3SEL_1
#define CFG_USART3_LSE                              (RCC_DCKCFGR2_USART3SEL_1 | RCC_DCKCFGR2_USART3SEL_0)

#define CFG_UART4_PCLK1                             0
#define CFG_UART4_SYS_CLOCK                         RCC_DCKCFGR2_UART4SEL_0
#define CFG_UART4_HSI                               RCC_DCKCFGR2_UART4SEL_1
#define CFG_UART4_LSE                               (RCC_DCKCFGR2_UART4SEL_1 | RCC_DCKCFGR2_UART4SEL_0)

#define CFG_UART5_PCLK1                             0
#define CFG_UART5_SYS_CLOCK                         RCC_DCKCFGR2_UART5SEL_0
#define CFG_UART5_HSI                               RCC_DCKCFGR2_UART5SEL_1
#define CFG_UART5_LSE                               (RCC_DCKCFGR2_UART5SEL_1 | RCC_DCKCFGR2_UART5SEL_0)

#define CFG_USART6_PCLK2                            0
#define CFG_USART6_SYS_CLOCK                        RCC_DCKCFGR2_USART6SEL_0
#define CFG_USART6_HSI                              RCC_DCKCFGR2_USART6SEL_1
#define CFG_USART6_LSE                              (RCC_DCKCFGR2_USART6SEL_1 | RCC_DCKCFGR2_USART6SEL_0)

#define CFG_UART7_PCLK1                             0
#define CFG_UART7_SYS_CLOCK                         RCC_DCKCFGR2_UART7SEL_0
#define CFG_UART7_HSI                               RCC_DCKCFGR2_UART7SEL_1
#define CFG_UART7_LSE                               (RCC_DCKCFGR2_UART7SEL_1 | RCC_DCKCFGR2_UART7SEL_0)

#define CFG_UART8_PCLK1                             0
#define CFG_UART8_SYS_CLOCK                         RCC_DCKCFGR2_UART8SEL_0
#define CFG_UART8_HSI                               RCC_DCKCFGR2_UART8SEL_1
#define CFG_UART8_LSE                               (RCC_DCKCFGR2_UART8SEL_1 | RCC_DCKCFGR2_UART8SEL_0)

#define CFG_USB_PLLQ                                0
#define CFG_USB_PLLSAI_P                            RCC_DCKCFGR2_CK48MSEL

//-------------------------------------------------------------------------------------------------
// Configuration file(s)
//-------------------------------------------------------------------------------------------------

#include "clock_cfg.h"

//-------------------------------------------------------------------------------------------------
// Auto define configuration
//-------------------------------------------------------------------------------------------------

#define RCC_PLL_CFGR_PLL_M_POS                      0
#define RCC_PLL_CFGR_PLL_N_POS                      6
#define RCC_PLL_CFGR_PLL_P_POS                      16
#define RCC_PLL_CFGR_PLL_Q_POS                      24

#define RCC_PLLSAI_CFGR_PLL_N_POS                   6
#define RCC_PLLSAI_CFGR_PLL_P_POS                   16
#define RCC_PLLSAI_CFGR_PLL_Q_POS                   24
#define RCC_PLLSAI_CFGR_PLL_R_POS                   28

#define RCC_PLLI2S_CFGR_PLL_N_POS                   6
#define RCC_PLLI2S_CFGR_PLL_P_POS                   16
#define RCC_PLLI2S_CFGR_PLL_Q_POS                   24
#define RCC_PLLI2S_CFGR_PLL_R_POS                   28

// --------------------------------------------------------------------------------------------------------------------------------
// Autoconfig value for PLL

  // Main PLL
  #define SYS_PLL_CLK_FREQUENCY                     (((CFG_PLL_SOURCE / CFG_PLL_M_DIVIDER) * CFG_PLL_N_MULTIPLIER) / CFG_PLL_P_DIVIDER)
  #define SYS_PLL_Q_FREQUENCY                       (((CFG_PLL_SOURCE / CFG_PLL_M_DIVIDER) * CFG_PLL_N_MULTIPLIER) / CFG_PLL_Q_DIVIDER)

  #if (CFG_PLL_M_DIVIDER < 2) || (CFG_PLL_M_DIVIDER > 63)
    #pragma message XSTR(CFG_PLLM_DIVIDER)
    #error PLLM is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLM                   (CFG_PLL_M_DIVIDER << RCC_PLL_CFGR_PLL_M_POS)
  #endif

  #if (CFG_PLL_N_MULTIPLIER < 50) || (CFG_PLL_N_MULTIPLIER > 432)
    #pragma message "XSTR(CFG_PLL_N_MULTIPLIER)"
    #error PLLN is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLN                   (CFG_PLL_N_MULTIPLIER << RCC_PLL_CFGR_PLL_N_POS)
  #endif

  #if ((CFG_PLL_P_DIVIDER / 2) < 1) || ((CFG_PLL_P_DIVIDER / 2) > 4)   // (tested for 2,4,6,8)
    #pragma message "XSTR(CFG_PLL_P_DIVIDER)"
    #error PLLP is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLP                   (((CFG_PLL_P_DIVIDER / 2) - 1) << RCC_PLL_CFGR_PLL_P_POS)
  #endif

  #if (CFG_PLL_Q_DIVIDER < 2) || (CFG_PLL_Q_DIVIDER > 15)
    #pragma message "XSTR(CFG_PLL_Q_DIVIDER)"
    #error PLLQ is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLLQ                   (CFG_PLL_Q_DIVIDER << RCC_PLL_CFGR_PLL_Q_POS)
  #endif

  #define CFG_RCC_PLL_CFGR                          (CFG_RCC_PLL_CFGR_PLLM |   \
                                                     CFG_RCC_PLL_CFGR_PLLN |   \
                                                     CFG_RCC_PLL_CFGR_PLLP |   \
                                                     CFG_RCC_PLL_CFGR_PLLQ |   \
                                                     CFG_RCC_PLLCFGR_PLLSRC)

  #define PLLP_CLOCK_FREQUENCY                      SYS_PLL_CLK_FREQUENCY
  #define PLLQ_CLOCK_FREQUENCY                      SYS_PLL_Q_FREQUENCY

// --------------------------------------------------------------------------------------------------------------------------------

    // PLLSAI
  #if (CFG_PLLSAI_N_MULTIPLIER < 50) || (CFG_PLLSAI_N_MULTIPLIER > 432)
    #pragma message "XSTR(CFG_PLLSAI_N_MULTIPLIER)"
    #error SAI PLLN is out of range
  #else
    #define CFG_RCC_PLLSAI_CFGR_PLLN                (CFG_PLLSAI_N_MULTIPLIER << RCC_PLLSAI_CFGR_PLL_N_POS)
  #endif

  #if ((CFG_PLLSAI_P_DIVIDER / 2) < 1) || ((CFG_PLLSAI_P_DIVIDER / 2) > 4)   // (tested for 2,4,6,8)
    #pragma message "XSTR(CFG_PLLSAI_P_DIVIDER)"
    #error SAI PLLP is out of range
  #else
    #define CFG_RCC_PLLSAI_CFGR_PLLP                (((CFG_PLLSAI_P_DIVIDER / 2) - 1) << RCC_PLLSAI_CFGR_PLL_P_POS)
  #endif

  #if (CFG_PLLSAI_Q_DIVIDER < 2) || (CFG_PLLSAI_Q_DIVIDER > 15)
    #pragma message "XSTR(CFG_PLLSAI_Q_DIVIDER)"
    #error SAI PLLQ is out of range
  #else
    #define CFG_RCC_PLLSAI_CFGR_PLLQ                (CFG_PLLSAI_Q_DIVIDER << RCC_PLLSAI_CFGR_PLL_Q_POS)
  #endif

  #if (CFG_PLLSAI_R_DIVIDER < 2) || (CFG_PLLSAI_R_DIVIDER > 7)
    #pragma message "XSTR(CFG_PLLSAI_R_DIVIDER)"
    #error SAI PLLR is out of range
  #else
    #define CFG_RCC_PLLSAI_CFGR_PLLR                (CFG_PLLSAI_R_DIVIDER << RCC_PLLSAI_CFGR_PLL_R_POS)
  #endif

  #define CFG_RCC_PLLSAI_CFGR                       (CFG_RCC_PLLSAI_CFGR_PLLN |   \
                                                     CFG_RCC_PLLSAI_CFGR_PLLP |   \
                                                     CFG_RCC_PLLSAI_CFGR_PLLQ |   \
                                                     CFG_RCC_PLLSAI_CFGR_PLLQ)

  #define PLLSAIN_CLOCK_FREQUENCY                   ((CFG_PLL_SOURCE / CFG_PLL_M_DIVIDER) * CFG_PLLSAI_N_MULTIPLIER)
  #define PLLSAIP_CLOCK_FREQUENCY                   (PLLSAIN_CLOCK_FREQUENCY / CFG_PLLSAI_P_DIVIDER)
  #define PLLSAIQ_CLOCK_FREQUENCY                   (PLLSAIN_CLOCK_FREQUENCY / CFG_PLLSAI_Q_DIVIDER)
  #define PLLSAIR_CLOCK_FREQUENCY                   (PLLSAIN_CLOCK_FREQUENCY / CFG_PLLSAI_Q_DIVIDER)

// --------------------------------------------------------------------------------------------------------------------------------
// PLLI2S
  #if (CFG_PLLI2S_N_MULTIPLIER < 50) || (CFG_PLLI2S_N_MULTIPLIER > 432)
    #pragma message "XSTR(CFG_PLLSAI_N_MULTIPLIER)"
    #error I2S PLLN is out of range
  #else
    #define CFG_RCC_PLLI2S_CFGR_PLLN                (CFG_PLLI2S_N_MULTIPLIER << RCC_PLLI2S_CFGR_PLL_N_POS)
  #endif

  #if ((CFG_PLLI2S_P_DIVIDER / 2) < 1) || ((CFG_PLLI2S_P_DIVIDER / 2) > 4)   // (tested for 2,4,6,8)
    #pragma message "XSTR(CFG_PLLSAI_P_DIVIDER)"
    #error I2S PLLP is out of range
  #else
    #define CFG_RCC_PLLI2S_CFGR_PLLP                (((CFG_PLLI2S_P_DIVIDER / 2) - 1) << RCC_PLLI2S_CFGR_PLL_P_POS)
  #endif

  #if (CFG_PLLI2S_Q_DIVIDER < 2) || (CFG_PLLI2S_Q_DIVIDER > 15)
    #pragma message "XSTR(CFG_PLLI2S_Q_DIVIDER)"
    #error I2S PLLQ is out of range
  #else
    #define CFG_RCC_PLLI2S_CFGR_PLLQ                (CFG_PLLI2S_Q_DIVIDER << RCC_PLLI2S_CFGR_PLL_Q_POS)
  #endif

  #if (CFG_PLLI2S_R_DIVIDER < 2) || (CFG_PLLI2S_R_DIVIDER > 7)
    #pragma message "XSTR(CFG_PLLI2S_R_DIVIDER)"
    #error I2S PLLR is out of range
  #else
    #define CFG_RCC_PLLI2S_CFGR_PLLR                (CFG_PLLI2S_R_DIVIDER << RCC_PLLI2S_CFGR_PLL_R_POS)
  #endif

  #define CFG_RCC_PLLI2S_CFGR                       (CFG_RCC_PLLI2S_CFGR_PLLN |   \
                                                     CFG_RCC_PLLI2S_CFGR_PLLP |   \
                                                     CFG_RCC_PLLI2S_CFGR_PLLQ |   \
                                                     CFG_RCC_PLLI2S_CFGR_PLLQ)

  #define PLLI2SN_CLOCK_FREQUENCY                   ((CFG_PLL_SOURCE / CFG_PLL_M_DIVIDER) * CFG_PLLI2S_N_MULTIPLIER)
  #define PLLI2SP_CLOCK_FREQUENCY                   (PLLI2SN_CLOCK_FREQUENCY / CFG_PLLI2S_P_DIVIDER)
  #define PLLI2SQ_CLOCK_FREQUENCY                   (PLLI2SN_CLOCK_FREQUENCY / CFG_PLLI2S_Q_DIVIDER)
  #define PLLI2SR_CLOCK_FREQUENCY                   (PLLI2SN_CLOCK_FREQUENCY / CFG_PLLI2S_Q_DIVIDER)

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

#if   CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV1
    #define SYS_HCLK_CLOCK_FREQUENCY                SYS_CPU_CORE_CLOCK_FREQUENCY
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV2
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 2)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV4
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 4)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV8
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 8)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV16
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 16)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV64
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 64)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV128
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 128)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV256
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 256)
#elif CFG_HCLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV512
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 512)
#endif

#define SYSTEM_CORE_CLOCK                           SYS_HCLK_CLOCK_FREQUENCY
#define SYS_CLOCK_FREQUENCY                         SYSTEM_CORE_CLOCK

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
#define PCLK1_CLOCK_FREQUENCY                       SYS_APB1_CLOCK_FREQUENCY

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
#define PCLK2_CLOCK_FREQUENCY                       SYS_APB2_CLOCK_FREQUENCY

/// -------------------------------------------------------------------------------------------------------------------------------
/// All Peripheral frequency ( TODO need to add all module clock here. )

/// CEC clock frequency
  #if   (CFG_CEC_SOURCE_MUX == CFG_CEC_LSE)
    #define CEC_CLOCK_FREQUENCY                         LSE_CLOCK_FREQUENCY
  #elif (CFG_CEC_SOURCE_MUX == CFG_CEC_HSI_DIV_488)
    #define CEC_CLOCK_FREQUENCY                         HSI_CLOCK_FREQUENCY / 488
  #else
    #error  CEC_CLOCK_FREQUENCY not defined
  #endif
/// CK48M clock frequency
  #if   (CFG_CK48M_SOURCE_MUX == CFG_SDMMC1_PLLQ)
    #define CK48M_CLOCK_FREQUENCY                       PLLQ_CLOCK_FREQUENCY
  #elif (CFG_CK48M_SOURCE_MUX == CFG_SDMMC1_PLLSAIP)
    #define CK48M_CLOCK_FREQUENCY                       PLLSAIP_CLOCK_FREQUENCY
  #else
    #error  CK48M_CLOCK_FREQUENCY not defined
  #endif
/// I2C1 clock frequency
  #if   (CFG_I2C1_SOURCE_MUX == CFG_I2C1_PLCK1)
    #define I2C1_CLOCK_FREQUENCY                        PLCK1_CLOCK_FREQUENCY
  #elif (CFG_I2C1_SOURCE_MUX == CFG_I2C1_SYS_CLOCK)
    #define I2C1_CLOCK_FREQUENCY                        SYS_CLOCK_CLOCK_FREQUENCY
  #elif (CFG_I2C1_SOURCE_MUX == CFG_I2C1_HSI)
    #define I2C1_CLOCK_FREQUENCY                        HSI_CLOCK_FREQUENCY
  #else
    #error  I2C1_CLOCK_FREQUENCY not defined
  #endif
/// I2C2 clock frequency
  #if   (CFG_I2C2_SOURCE_MUX == CFG_I2C2_PLCK1)
    #define I2C2_CLOCK_FREQUENCY                        PLCK1_CLOCK_FREQUENCY
  #elif (CFG_I2C2_SOURCE_MUX == CFG_I2C2_SYS_CLOCK)
    #define I2C2_CLOCK_FREQUENCY                        SYS_CLOCK_CLOCK_FREQUENCY
  #elif (CFG_I2C2_SOURCE_MUX == CFG_I2C2_HSI)
    #define I2C2_CLOCK_FREQUENCY                        HSI_CLOCK_FREQUENCY
  #else
    #error  I2C2_CLOCK_FREQUENCY not defined
  #endif
/// I2C3 clock frequency
  #if   (CFG_I2C3_SOURCE_MUX == CFG_I2C3_PLCK1)
    #define I2C3_CLOCK_FREQUENCY                        PLCK1_CLOCK_FREQUENCY
  #elif (CFG_I2C3_SOURCE_MUX == CFG_I2C3_SYS_CLOCK)
    #define I2C3_CLOCK_FREQUENCY                        SYS_CLOCK_CLOCK_FREQUENCY
  #elif (CFG_I2C3_SOURCE_MUX == CFG_I2C3_HSI)
    #define I2C3_CLOCK_FREQUENCY                        HSI_CLOCK_FREQUENCY
  #else
    #error  I2C3_CLOCK_FREQUENCY not defined
  #endif
/// I2C4 clock frequency
  #if   (CFG_I2C4_SOURCE_MUX == CFG_I2C4_PLCK1)
    #define I2C4_CLOCK_FREQUENCY                        PLCK1_CLOCK_FREQUENCY
  #elif (CFG_I2C4_SOURCE_MUX == CFG_I2C4_SYS_CLOCK)
    #define I2C4_CLOCK_FREQUENCY                        SYS_CLOCK_CLOCK_FREQUENCY
  #elif (CFG_I2C4_SOURCE_MUX == CFG_I2C4_HSI)
    #define I2C4_CLOCK_FREQUENCY                        HSI_CLOCK_FREQUENCY
  #else
    #error  I2C4_CLOCK_FREQUENCY not defined
  #endif
/// LPTIM1 clock frequency
  #if   (CFG_LPTIM1_SOURCE_MUX == CFG_LPTIM1_PLCK1)
    #define LPTIM1_CLOCK_FREQUENCY                        PLCK1_CLOCK_FREQUENCY
  #elif (CFG_LPTIM1_SOURCE_MUX == CFG_LPTIM1_LSI)
    #define LPTIM1_CLOCK_FREQUENCY                        LSI_CLOCK_FREQUENCY
  #elif (CFG_LPTIM1_SOURCE_MUX == CFG_LPTIM1_HSI)
    #define LPTIM1_CLOCK_FREQUENCY                        HSI_CLOCK_FREQUENCY
  #elif (CFG_LPTIM1_SOURCE_MUX == CFG_LPTIM1_LSE)
    #define LPTIM1_CLOCK_FREQUENCY                        LSE_CLOCK_FREQUENCY
  #else
    #error  LPTIM1_CLOCK_FREQUENCY not defined
  #endif
/// SDMMC 1 clock frequency
  #if   (CFG_SDMMC1_SOURCE_MUX == CFG_SDMMC1_PLL48_CLOCK)
    #define SDMMC1_CLOCK_FREQUENCY                      PLL48_CLOCK_FREQUENCY
  #elif (CFG_SDMMC1_SOURCE_MUX == CFG_SDMMC1_SYS_CLOCK)
    #define SDMMC1_CLOCK_FREQUENCY                      SYS_CLOCK_FREQUENCY
  #else
    #error  SDMMC1_CLOCK_FREQUENCY not defined
  #endif
/// USART 1 clock frequency
  #if   (CFG_USART1_SOURCE_MUX == CFG_USART1_PCLK2)
    #define USART1_CLOCK_FREQUENCY                      PCLK2_CLOCK_FREQUENCY
  #elif (CFG_USART1_SOURCE_MUX == CFG_USART1_SYS_CLOCK)
    #define USART1_CLOCK_FREQUENCY                      SYS_CLOCK_FREQUENCY
  #elif (CFG_USART1_SOURCE_MUX == CFG_USART1_HSI)
    #define USART1_CLOCK_FREQUENCY                      HSI_CLOCK_FREQUENCY
  #elif (CFG_USART1_SOURCE_MUX == CFG_USART1_LSE)
    #define USART1_CLOCK_FREQUENCY                      LSE_CLOCK_FREQUENCY
  #else
    #error  USART1_CLOCK_FREQUENCY not defined
  #endif
/// USART 2 clock frequency
  #if   (CFG_USART2_SOURCE_MUX == CFG_USART2_PCLK1)
    #define USART2_CLOCK_FREQUENCY                      PCLK1_CLOCK_FREQUENCY
  #elif (CFG_USART2_SOURCE_MUX == CFG_USART2_SYS_CLOCK)
    #define USART2_CLOCK_FREQUENCY                      SYS_CLOCK_FREQUENCY
  #elif (CFG_USART2_SOURCE_MUX == CFG_USART2_HSI)
    #define USART2_CLOCK_FREQUENCY                      HSI_CLOCK_FREQUENCY
  #elif (CFG_USART2_SOURCE_MUX == CFG_USART2_LSE)
    #define USART2_CLOCK_FREQUENCY                      LSE_CLOCK_FREQUENCY
  #else
    #error  USART2_CLOCK_FREQUENCY not defined
  #endif

/// USART 3 clock frequency
  #if   (CFG_USART3_SOURCE_MUX == CFG_USART3_PCLK1)
    #define USART3_CLOCK_FREQUENCY                      PCLK1_CLOCK_FREQUENCY
  #elif (CFG_USART3_SOURCE_MUX == CFG_USART3_SYS_CLOCK)
    #define USART3_CLOCK_FREQUENCY                      SYS_CLOCK_FREQUENCY
  #elif (CFG_USART3_SOURCE_MUX == CFG_USART3_HSI)
    #define USART3_CLOCK_FREQUENCY                      HSI_CLOCK_FREQUENCY
  #elif (CFG_USART3_SOURCE_MUX == CFG_USART3_LSE)
    #define USART3_CLOCK_FREQUENCY                      LSE_CLOCK_FREQUENCY
  #else
    #error  UART3_CLOCK_FREQUENCY not defined
  #endif

/// UART 4 clock frequency
  #if   (CFG_UART4_SOURCE_MUX == CFG_UART4_PCLK1)
    #define UART4_CLOCK_FREQUENCY                       PCLK1_CLOCK_FREQUENCY
  #elif (CFG_UART4_SOURCE_MUX == CFG_UART4_SYS_CLOCK)
    #define UART4_CLOCK_FREQUENCY                       SYS_CLOCK_FREQUENCY
  #elif (CFG_UART4_SOURCE_MUX == CFG_UART4_HSI)
    #define UART4_CLOCK_FREQUENCY                       HSI_CLOCK_FREQUENCY
  #elif (CFG_UART4_SOURCE_MUX == CFG_UART4_LSE)
    #define UART4_CLOCK_FREQUENCY                       LSE_CLOCK_FREQUENCY
  #else
    #error  UART4_CLOCK_FREQUENCY not defined
#endif

/// USART 5 clock frequency
  #if   (CFG_UART5_SOURCE_MUX == CFG_UART5_PCLK1)
    #define UART5_CLOCK_FREQUENCY                       PCLK1_CLOCK_FREQUENCY
  #elif (CFG_UART5_SOURCE_MUX == CFG_UART5_SYS_CLOCK)
    #define UART5_CLOCK_FREQUENCY                       SYS_CLOCK_FREQUENCY
  #elif (CFG_UART5_SOURCE_MUX == CFG_UART5_HSI)
    #define UART5_CLOCK_FREQUENCY                       HSI_CLOCK_FREQUENCY
  #elif (CFG_UART5_SOURCE_MUX == CFG_UART5_LSE)
    #define UART5_CLOCK_FREQUENCY                       LSE_CLOCK_FREQUENCY
  #else
    #error  UART5_CLOCK_FREQUENCY not defined
  #endif

/// USART 6 clock frequency
  #if   (CFG_USART6_SOURCE_MUX == CFG_USART6_PCLK2)
    #define USART6_CLOCK_FREQUENCY                      PCLK2_CLOCK_FREQUENCY
  #elif (CFG_USART6_SOURCE_MUX == CFG_USART6_SYS_CLOCK)
    #define USART6_CLOCK_FREQUENCY                      SYS_CLOCK_FREQUENCY
  #elif (CFG_USART6_SOURCE_MUX == CFG_USART6_HSI)
    #define USART6_CLOCK_FREQUENCY                      HSI_CLOCK_FREQUENCY
  #elif (CFG_USART6_SOURCE_MUX == CFG_USART6_LSE)
    #define USART6_CLOCK_FREQUENCY                      LSE_CLOCK_FREQUENCY
  #else
    #error  USART6_CLOCK_FREQUENCY not defined
  #endif

/// USART 7 clock frequency
  #if   (CFG_UART7_SOURCE_MUX == CFG_UART7_PCLK1)
    #define UART7_CLOCK_FREQUENCY                       PCLK1_CLOCK_FREQUENCY
  #elif (CFG_UART7_SOURCE_MUX == CFG_UART7_SYS_CLOCK)
    #define UART7_CLOCK_FREQUENCY                       SYS_CLOCK_FREQUENCY
  #elif (CFG_UART7_SOURCE_MUX == CFG_UART7_HSI)
    #define UART7_CLOCK_FREQUENCY                       HSI_CLOCK_FREQUENCY
  #elif (CFG_UART7_SOURCE_MUX == CFG_UART7_LSE)
    #define UART7_CLOCK_FREQUENCY                       LSE_CLOCK_FREQUENCY
  #else
    #error  UART7_CLOCK_FREQUENCY not defined
  #endif

/// USART 8 clock frequency
  #if   (CFG_UART8_SOURCE_MUX == CFG_UART8_PCLK1)
    #define UART8_CLOCK_FREQUENCY                       PCLK1_CLOCK_FREQUENCY
  #elif (CFG_UART8_SOURCE_MUX == CFG_UART8_SYS_CLOCK)
    #define UART8_CLOCK_FREQUENCY                       SYS_CLOCK_FREQUENCY
  #elif (CFG_UART8_SOURCE_MUX == CFG_UART8_HSI)
    #define UART8_CLOCK_FREQUENCY                       HSI_CLOCK_FREQUENCY
  #elif (CFG_UART8_SOURCE_MUX == CFG_UART8_LSE)
    #define UART8_CLOCK_FREQUENCY                       LSE_CLOCK_FREQUENCY
  #endif


// CFG_USB_SOURCE_MUX                          CFG_RCC_D2CCIP2R_USB_PLL3Q

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


