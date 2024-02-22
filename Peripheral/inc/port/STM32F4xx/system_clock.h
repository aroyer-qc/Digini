//-------------------------------------------------------------------------------------------------
//
//  File : system_clock.h
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

#include "stm32f4xx.h"
#include <stdbool.h>
#include "lib_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------
// Configuration file(s)
//-------------------------------------------------------------------------------------------------

#include "clock_cfg.h"

//-------------------------------------------------------------------------------------------------
// Auto define configuration
//-------------------------------------------------------------------------------------------------

#define CFG_RCC_PLLCFGR_PLL_M_POS               0
#define CFG_RCC_PLLCFGR_PLL_N_POS               6
#define CFG_RCC_PLLCFGR_PLL_P_POS               16
#define CFG_RCC_PLLCFGR_PLL_Q_POS               24

//-------------------------------------------------------------------------------------------------
// Auto configuration value for PLL
#if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

  #define CFG_PLL_CLK_FREQUENCY                     (((CFG_PLL_SOURCE / CFG_PLL_M_DIVIDER) * CFG_PLL_N_MULTIPLIER) / CFG_PLL_P_DIVIDER)
  #define CFG_PLL_Q_FREQUENCY                       (((CFG_PLL_SOURCE / CFG_PLL_M_DIVIDER) * CFG_PLL_N_MULTIPLIER) / CFG_PLL_Q_DIVIDER)

  #if (CFG_PLL_M_DIVIDER < 2) || (CFG_PLL_M_DIVIDER > 63)
    #pragma message "XSTR(CFG_PLL_M_DIVIDER)"
    #error PLL_M is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLL_M                  (CFG_PLL_M_DIVIDER << CFG_RCC_PLLCFGR_PLL_M_POS)
  #endif

  #if (CFG_PLL_N_MULTIPLIER < 50) || (CFG_PLL_N_MULTIPLIER > 432)
    #pragma message "XSTR(CFG_PLL_N_MULTIPLIER)"
    #error PLL_N is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLL_N                  (CFG_PLL_N_MULTIPLIER << CFG_RCC_PLLCFGR_PLL_N_POS)
  #endif

  #if ((CFG_PLL_P_DIVIDER / 2) < 1) || ((CFG_PLL_P_DIVIDER / 2) > 4)   // (tested for 2,4,6,8)
    #pragma message "XSTR(CFG_PLL_P_DIVIDER)"
    #error PLL_P is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLL_P                  (((CFG_PLL_P_DIVIDER / 2) - 1) << CFG_RCC_PLLCFGR_PLL_P_POS)
  #endif

  #if (CFG_PLL_Q_DIVIDER < 2) || (CFG_PLL_Q_DIVIDER > 15)
    #pragma message "XSTR(CFG_PLL_Q_DIVIDER)"
    #error PLL_Q is out of range
  #else
    #define CFG_RCC_PLL_CFGR_PLL_Q                  (CFG_PLL_Q_DIVIDER << CFG_RCC_PLLCFGR_PLL_Q_POS)
  #endif

  #define CFG_RCC_PLL_CFGR_CFG                      (CFG_RCC_PLL_CFGR_HSI_PLL_M |   \
                                                     CFG_RCC_PLL_CFGR_HSI_PLL_N |   \
                                                     CFG_RCC_PLL_CFGR_HSI_PLL_P |   \
                                                     CFG_RCC_PLL_CFGR_HSI_PLL_Q |   \
                                                     CFG_RCC_PLLCFGR_PLLSRC)

  #define CFG_SYS_PLL_CLK_FREQUENCY                 CFG_PLL_CLK_FREQUENCY


#endif // CFG_SYS_CLOCK_MUX == RCC_CFGR_SW_PLL

// --------------------------------------------------------------------------------------------------------------------------------

#if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_SYS_PLL_CLK_FREQUENCY
#elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSE)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_HSE_VALUE
#else //(CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSI)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_HSI_VALUE
#endif


#if   CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV1
    #define SYS_HCLK_CLOCK_FREQUENCY                SYS_CPU_CORE_CLOCK_FREQUENCY
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV2
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 2)
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV4
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 4)
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV8
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 8)
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV16
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 16)
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV64
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 64)
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV128
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 128)
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV256
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 256)
#elif CFG_AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV512
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 512)
#endif
#define CFG_SYS_HCLK                                CFG_AHB_CLK_DIVIDER

#define SYSTEM_CORE_CLOCK                           SYS_CPU_CORE_CLOCK_FREQUENCY

#if   CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV1
    #define SYS_APB1_CLOCK_FREQUENCY                SYS_HCLK_CLOCK_FREQUENCY
#elif CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV2
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV4
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV8
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV16
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif
#define CFG_SYS_APB1                                CFG_APB1_CLK_DIVIDER
#define SYS_APB1_TIMER_CLOCK_FREQUENCY              (SYS_APB1_CLOCK_FREQUENCY * 2)

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
#define CFG_SYS_APB2                                CFG_APB2_CLK_DIVIDER
#define SYS_APB2_TIMER_CLOCK_FREQUENCY              SYS_APB2_CLOCK_FREQUENCY * 2


// Power scaling according to speed
#if (SYS_CPU_CORE_CLOCK_FREQUENCY <= 144000000)
#define POWER_REGULATOR_CFG                         0
#else // (SYS_CPU_CORE_CLOCK_FREQUENCY <= 168000000)
#define POWER_REGULATOR_CFG                         PWR_CR_VOS_0
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
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 168000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_5WS
#endif

// Verification
#if SYS_HCLK_CLOCK_FREQUENCY > 168000000
 #pragma message "XSTR(SYS_HCLK_CLOCK_FREQUENCY)"
 #error CPU Core frequency exceed maximum allowed!
#endif

#if SYS_APB1_CLOCK_FREQUENCY > 42000000
 #pragma message "XSTR(SYS_APB1_CLOCK_FREQUENCY)"
 #error APB1 frequency exceed maximum allowed!
#endif

#if SYS_APB2_CLOCK_FREQUENCY > 84000000
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
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

SYSTEM_EXTERN   uint32_t        SystemCoreClock;

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SystemInit              (void);

//-------------------------------------------------------------------------------------------------
