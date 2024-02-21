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

#include "stm32f1xxxx.h"
#include <stdbool.h>
#include "lib_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CFG_HSI_VALUE                               8000000
#define CFG_HSI_PLL_VALUE                           4000000             // HSI for PLL is always divide by 2

#define RCC_CFGR_PLL_MULTIPLIER_POS                 18
#define RCC_CFGR_PLL_DIVIDER_POS                    17

#define RCC_CFGR_PLL_SRC_MASK                       RCC_CFGR_PLLSRC

// Own define for register bit value without cast (Do not change)

#define CFG_CLOCK_SRC_HSI                           0x00000000U
#define CFG_CLOCK_SRC_HSE                           0x00000001U
#define CFG_CLOCK_SRC_PLL                           0x00000002U

#define CFG_CLOCK_PLL_SRC_HSE                       0x00400000U
#define CFG_CLOCK_PLL_SRC_HSI                       0x00000000U

// RCC_CFGR_HPRE -> AHB prescaler
#define CFG_CLOCK_AHB_DIV1                          0x00000000U
#define CFG_CLOCK_AHB_DIV2                          0x00000080U
#define CFG_CLOCK_AHB_DIV4                          0x00000090U
#define CFG_CLOCK_AHB_DIV8                          0x000000A0U
#define CFG_CLOCK_AHB_DIV16                         0x000000B0U
#define CFG_CLOCK_AHB_DIV64                         0x000000C0U
#define CFG_CLOCK_AHB_DIV128                        0x000000D0U
#define CFG_CLOCK_AHB_DIV256                        0x000000E0U
#define CFG_CLOCK_AHB_DIV512                        0x000000F0U

// RCC_CFGR_PPRE1 -> APB1 prescaler
#define CFG_CLOCK_APB1_DIV1                         0x00000000U
#define CFG_CLOCK_APB1_DIV2                         0x00001000U
#define CFG_CLOCK_APB1_DIV4                         0x00001400U
#define CFG_CLOCK_APB1_DIV8                         0x00001800U
#define CFG_CLOCK_APB1_DIV16                        0x00001C00U

// RCC_CFGR_PPRE2 -> APB2 prescaler
#define CFG_CLOCK_APB2_DIV1                         0x00000000U
#define CFG_CLOCK_APB2_DIV2                         0x00008000U
#define CFG_CLOCK_APB2_DIV4                         0x0000A000U
#define CFG_CLOCK_APB2_DIV8                         0x0000C000U
#define CFG_CLOCK_APB2_DIV16                        0x0000E000U

//RCC_CFGR_ADCPRE -> ADC prescaler
#define CFG_CLOCK_ADC_DIV2                          0x00000000U
#define CFG_CLOCK_ADC_DIV4                          0x00004000U
#define CFG_CLOCK_ADC_DIV6                          0x00008000U
#define CFG_CLOCK_ADC_DIV8                          0x0000C000U

//-------------------------------------------------------------------------------------------------
// Configuration file(s)
//-------------------------------------------------------------------------------------------------

#include "clock_cfg.h"

//-------------------------------------------------------------------------------------------------
// Auto define configuration
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Auto configuration value for the clock

#if (CFG_SYS_CLOCK_MUX == CFG_CLOCK_SRC_PLL)

  #if (CFG_PLL_DIVIDER < 1) || (CFG_PLL_DIVIDER > 2)
    #pragma message "XSTR(CFG_PLL_DIVIDER)"
    #error PLL divider is out of range
  #else
    #define CFG_RCC_CFGR_PLL_DIVIDER                        (CFG_PLL_DIVIDER << RCC_CFGR_PLL_DIVIDER_POS)
  #endif

  #if (CFG_PLL_MULTIPLIER < 2) || (CFG_PLL_MULTIPLIER > 16)
    #pragma message "XSTR(CFG_PLL_MULTIPLIER)"
    #error PLL multiplier is out of range
  #else
    #define CFG_RCC_CFGR_PLL_MULTIPLIER                     (CFG_PLL_MULTIPLIER << (RCC_CFGR_PLL_MULTIPLIER_POS >> 1))
  #endif

  #if (CFG_PLL_SOURCE_MUX == CFG_CLOCK_PLL_SRC_HSI)
    #define CFG_PLL_CLK_FREQUENCY                           (CFG_HSI_PLL_VALUE * CFG_PLL_MULTIPLIER)
  #else // CFG_CLOCK_PLL_SRC_HSE
    #define CFG_PLL_CLK_FREQUENCY                           ((CFG_HSE_VALUE / CFG_PLL_DIVIDER) * CFG_PLL_MULTIPLIER)
  #endif

  #define CFG_RCC_PLL_CFGR                                  (CFG_RCC_CFGR_PLL_DIVIDER    | \
                                                             CFG_RCC_CFGR_PLL_MULTIPLIER | \
                                                             CFG_PLL_SOURCE_MUX)
  #endif

  #define SYS_CPU_CORE_CLOCK_FREQUENCY                      CFG_PLL_CLK_FREQUENCY

#elif (CFG_SYS_CLOCK_MUX == RCC_CFGR_SW_HSE)
    
  #define SYS_CPU_CORE_CLOCK_FREQUENCY                      HSE_VALUE
  
#else //(CFG_SYS_CLOCK_MUX == RCC_CFGR_SW_HSI)

  #define SYS_CPU_CORE_CLOCK_FREQUENCY                      HSI_VALUE
  
#endif


#if   CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV1
    #define SYS_HCLK_CLOCK_FREQUENCY                        SYS_CPU_CORE_CLOCK_FREQUENCY
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV2
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 2)
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV4
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 4)
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV8
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 8)
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV6
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 16)
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV64
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 64)
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV128
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 128)
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV256
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 256)
#elif CFG_AHB_CLK_DIVIDER == CFG_CLOCK_AHB_DIV512
    #define SYS_HCLK_CLOCK_FREQUENCY                        (SYS_CPU_CORE_CLOCK_FREQUENCY / 512)
#endif
#define SYS_HCLK_CFG                                        CFG_AHB_CLK_DIVIDER

#define SYSTEM_CORE_CLOCK                                   SYS_CPU_CORE_CLOCK_FREQUENCY

#if   CFG_APB1_CLK_DIVIDER == CFG_CLOCK_APB1_DIV1
    #define SYS_APB1_CLOCK_FREQUENCY                        SYS_HCLK_CLOCK_FREQUENCY
#elif CFG_APB1_CLK_DIVIDER == CFG_CLOCK_APB1_DIV2
    #define SYS_APB1_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif CFG_APB1_CLK_DIVIDER == CFG_CLOCK_APB1_DIV4
    #define SYS_APB1_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif CFG_APB1_CLK_DIVIDER == CFG_CLOCK_APB1_DIV8
    #define SYS_APB1_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif CFG_APB1_CLK_DIVIDER == CFG_CLOCK_APB1_DIV16
    #define SYS_APB1_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif
#define SYS_APB1_CFG                                        CFG_APB1_CLK_DIVIDER
#define SYS_APB1_TIMER_CLOCK_FREQUENCY                      (SYS_APB1_CLOCK_FREQUENCY * 2)

#if   CFG_APB2_CLK_DIVIDER == CFG_CLOCK_APB2_DIV1
    #define SYS_APB2_CLOCK_FREQUENCY                        SYS_HCLK_CLOCK_FREQUENCY
#elif CFG_APB2_CLK_DIVIDER == CFG_CLOCK_APB2_DIV2
    #define SYS_APB2_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif CFG_APB2_CLK_DIVIDER == CFG_CLOCK_APB2_DIV4
    #define SYS_APB2_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif CFG_APB2_CLK_DIVIDER == CFG_CLOCK_APB2_DIV8
    #define SYS_APB2_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif CFG_APB2_CLK_DIVIDER == CFG_CLOCK_APB2_DIV16
    #define SYS_APB2_CLOCK_FREQUENCY                        (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif
#define SYS_APB2_CFG                                        CFG_APB2_CLK_DIVIDER
#define SYS_APB2_TIMER_CLOCK_FREQUENCY                      SYS_APB2_CLOCK_FREQUENCY

// Flash Latency configuration for Voltage from 2.7V to 3.6V
#if   (SYS_CPU_CORE_CLOCK_FREQUENCY <= 24000000)
#define FLASH_LATENCY_CFG                                   FLASH_ACR_LATENCY_0
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 48000000)
#define FLASH_LATENCY_CFG                                   FLASH_ACR_LATENCY_1
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 72000000)
#define FLASH_LATENCY_CFG                                   FLASH_ACR_LATENCY_2
#endif

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

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SystemInit              (void);

//-------------------------------------------------------------------------------------------------




