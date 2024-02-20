//-------------------------------------------------------------------------------------------------
//
//  File : system_clock.h
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
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "stm32f1xx.h"
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

//#define RCC_PLLCFGR_PLL_M_POS           0
//#define RCC_PLLCFGR_PLL_N_POS           6
//#define RCC_PLLCFGR_PLL_P_POS           16
//#define RCC_PLLCFGR_PLL_Q_POS           24

//-------------------------------------------------------------------------------------------------
// Auto configuration value for PLL

#if (SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)
  // ---------
  // HSI Check
  #define SYS_HSI_PLL_CLK_FREQUENCY                 (((HSI_PLL_SOURCE / HSI_PLL_M_DIVIDER) * HSI_PLL_N_MULTIPLIER) / HSI_PLL_P_DIVIDER)
  #define SYS_HSI_PLL_Q_FREQUENCY                   (((HSI_PLL_SOURCE / HSI_PLL_M_DIVIDER) * HSI_PLL_N_MULTIPLIER) / HSI_PLL_Q_DIVIDER)

  #if (HSI_PLL_M_DIVIDER < 2) || (HSI_PLL_M_DIVIDER > 63)
    #pragma message "XSTR(HSI_PLL_M_DIVIDER)"
    #error PLL_M for HSI is out of range
  #else
    #define RCC_PLL_CFGR_HSI_PLL_M_CFG              (HSI_PLL_M_DIVIDER << RCC_PLLCFGR_PLL_M_POS)
  #endif

  #if (HSI_PLL_N_MULTIPLIER < 50) || (HSI_PLL_N_MULTIPLIER > 432)
    #pragma message "XSTR(HSI_PLL_N_MULTIPLIER)"
    #error PLL_N for HSI is out of range
  #else
    #define RCC_PLL_CFGR_HSI_PLL_N_CFG              (HSI_PLL_N_MULTIPLIER << RCC_PLLCFGR_PLL_N_POS)
  #endif

  #define RCC_HSI_PLL_CFGR_CFG                      (RCC_PLL_CFGR_HSI_PLL_M_CFG |   \
                                                     RCC_PLL_CFGR_HSI_PLL_N_CFG |   \
                                                     CFG_RCC_PLLCFGR_PLLSRC_HSI)

  // ---------
  // HSE Check

  #if (HSE_PLL_M_DIVIDER < 2) || (HSE_PLL_M_DIVIDER > 63)
    #pragma message "XSTR(HSE_PLL_M_DIVIDER)"
    #error PLL_M for HSE is out of range
  #else
    #define RCC_PLL_CFGR_HSE_PLL_M_CFG              (HSE_PLL_M_DIVIDER << RCC_PLLCFGR_PLL_M_POS)
  #endif

  #if (HSE_PLL_N_MULTIPLIER < 50) || (HSE_PLL_N_MULTIPLIER > 432)
    #pragma message "XSTR(HSE_PLL_N_MULTIPLIER)"
    #error PLL_N for HSE is out of range
  #else
    #define RCC_PLL_CFGR_HSE_PLL_N_CFG              (HSE_PLL_N_MULTIPLIER << RCC_PLLCFGR_PLL_N_POS)
  #endif

  #define RCC_HSE_PLL_CFGR_CFG                      (RCC_PLL_CFGR_HSE_PLL_M_CFG |   \
                                                     RCC_PLL_CFGR_HSE_PLL_N_CFG |   \
                                                     CFG_RCC_PLLCFGR_PLLSRC_HSE)
// --------------------
// HSI versus HSE Check
#ifdef HSE_AND_HSI_MUST_BE_MATCHED
  #if SYS_HSI_PLL_CLK_FREQUENCY == SYS_HSE_PLL_CLK_FREQUENCY
    #define SYS_PLL_CLK_FREQUENCY                  SYS_HSE_PLL_CLK_FREQUENCY
  #else
    #pragma message "XSTR(SYS_HSI_PLL_CLK_FREQUENCY != SYS_HSE_PLL_CLK_FREQUENCY)"
    #error Both HSI anf HSE configuration must give in the same system frequency
  #endif
#else
  #if SYS_PLL_MUX == RCC_PLLCFGR_PLLSRC_HSE
    #define SYS_PLL_CLK_FREQUENCY = SYS_HSE_PLL_CLK_FREQUENCY
  #else
    #define SYS_PLL_CLK_FREQUENCY = SYS_HSI_PLL_CLK_FREQUENCY
  #endif
#endif


#endif // SYS_CLOCK_MUX == RCC_CFGR_SW_PLL

// --------------------------------------------------------------------------------------------------------------------------------

#if (SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              SYS_PLL_CLK_FREQUENCY
#elif (SYS_CLOCK_MUX == RCC_CFGR_SW_HSE)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              HSE_VALUE
#else //(SYS_CLOCK_MUX == RCC_CFGR_SW_HSI)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              HSI_VALUE
#endif


#if   AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV1
    #define SYS_HCLK_CLOCK_FREQUENCY                SYS_CPU_CORE_CLOCK_FREQUENCY
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV2
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 2)
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV4
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 4)
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV8
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 8)
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV16
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 16)
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV64
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 64)
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV128
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 128)
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV256
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 256)
#elif AHB_CLK_DIVIDER == CFG_RCC_CFGR_HPRE_DIV512
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 512)
#endif
#define SYS_HCLK_CFG                                AHB_CLK_DIVIDER

#define SYSTEM_CORE_CLOCK                           SYS_CPU_CORE_CLOCK_FREQUENCY

#if   APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV1
    #define SYS_APB1_CLOCK_FREQUENCY                SYS_HCLK_CLOCK_FREQUENCY
#elif APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV2
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV4
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV8
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV16
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif
#define SYS_APB1_CFG                                APB1_CLK_DIVIDER
#define SYS_APB1_TIMER_CLOCK_FREQUENCY              (SYS_APB1_CLOCK_FREQUENCY * 2)

#if   APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV1
    #define SYS_APB2_CLOCK_FREQUENCY                SYS_HCLK_CLOCK_FREQUENCY
#elif APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV2
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV4
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV8
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV16
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif
#define SYS_APB2_CFG                                APB2_CLK_DIVIDER
#define SYS_APB2_TIMER_CLOCK_FREQUENCY              SYS_APB2_CLOCK_FREQUENCY * 2

//  F1 OK
// Flash Latency configuration for Voltage from 2.7V to 3.6V
#if   (SYS_CPU_CORE_CLOCK_FREQUENCY <= 24000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_0
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 48000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_1
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 72000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_2
#endif

//  F1 OK
// Verification
#if SYS_HCLK_CLOCK_FREQUENCY > 72000000
 #pragma message "XSTR(SYS_HCLK_CLOCK_FREQUENCY)"
 #error CPU Core frequency exceed maximum allowed!
#endif

#if SYS_APB1_CLOCK_FREQUENCY > 36000000
 #pragma message "XSTR(SYS_APB1_CLOCK_FREQUENCY)"
 #error APB1 frequency exceed maximum allowed!
#endif

#if SYS_APB2_CLOCK_FREQUENCY > 72000000
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

#if (HSE_AND_HSI_MUST_BE_MATCHED == DEF_ENABLED)
SYSTEM_EXTERN   bool            SystemHSE_ClockFailure;
#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SystemInit              (void);

//-------------------------------------------------------------------------------------------------




