//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32L4_system_clock.h
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
//
// Note(s) 		only configured for STM32L412CB at this time
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "stm32L4xx.h"
#include <stdbool.h>
#include "./Digini/inc/lib_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define LSI_CLOCK_FREQUENCY                         32000
#define HSI_CLOCK_FREQUENCY                         16000000

// Own define for register bit value without cast (Do not change)
#define CFG_RCC_CFGR_SW_MSI                         0x00000000U
#define CFG_RCC_CFGR_SW_HSI16                       0x00000001U
#define CFG_RCC_CFGR_SW_HSE                         0x00000002U
#define CFG_RCC_CFGR_SW_PLL                         0x00000003U

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
#define CFG_RCC_CFGR_PPRE1_DIV2                     0x00000400U
#define CFG_RCC_CFGR_PPRE1_DIV4                     0x00000500U
#define CFG_RCC_CFGR_PPRE1_DIV8                     0x00000600U
#define CFG_RCC_CFGR_PPRE1_DIV16                    0x00000700U

#define CFG_RCC_CFGR_PPRE2_DIV1                     0x00000000U
#define CFG_RCC_CFGR_PPRE2_DIV2                     0x00002000U
#define CFG_RCC_CFGR_PPRE2_DIV4                     0x00002800U
#define CFG_RCC_CFGR_PPRE2_DIV8                     0x00003000U
#define CFG_RCC_CFGR_PPRE2_DIV16                    0x00003800U

#define CFG_RCC_CFGR_MCOSEL_DISABLE                 0x00000000U
#define CFG_RCC_CFGR_MCOSEL_SYSCLK                  0x01000000U
#define CFG_RCC_CFGR_MCOSEL_MSI                     0x02000000U
#define CFG_RCC_CFGR_MCOSEL_HSI16                   0x03000000U
#define CFG_RCC_CFGR_MCOSEL_HSE                     0x04000000U
#define CFG_RCC_CFGR_MCOSEL_PLL                     0x05000000U
#define CFG_RCC_CFGR_MCOSEL_LSI                     0x06000000U
#define CFG_RCC_CFGR_MCOSEL_LSE                     0x07000000U
#define CFG_RCC_CFGR_MCOSEL_HSI48                   0x08000000U

#define CFG_RCC_CFGR_MCOPRE_NO_DIV                  0x00000000U
#define CFG_RCC_CFGR_MCOPRE_DIV2                    0x10000000U
#define CFG_RCC_CFGR_MCOPRE_DIV4                    0x20000000U
#define CFG_RCC_CFGR_MCOPRE_DIV8                    0x30000000U
#define CFG_RCC_CFGR_MCOPRE_DIV16                   0x40000000U

#define CFG_RCC_PLLCFGR_PLLSRC_MSI                  0x00000001U
#define CFG_RCC_PLLCFGR_PLLSRC_HSI                  0x00000002U
#define CFG_RCC_PLLCFGR_PLLSRC_HSE                  0x00000003U

#define CFG_RCC_CCIPR_CLK48SEL_HSI48                0x00000000U
#define CFG_RCC_CCIPR_CLK48SEL_PLLQ                 0x00000001U

#define CFG_RCC_BDCR_RTCSEL_NO_CLOCK                0x00000000U
#define CFG_RCC_BDCR_RTCSEL_LSE                     0x00000100U
#define CFG_RCC_BDCR_RTCSEL_LSI                     0x00000200U
#define CFG_RCC_BDCR_RTCSEL_HSE                     0x00000300U

#define CFG_MAX_CPU_SYS_HCLK_CLOCK_FREQUENCY        80000000U
#define CFG_MAX_APB1_CLOCK_FREQUENCY                80000000U
#define CFG_MAX_APB2_CLOCK_FREQUENCY                80000000U

//-------------------------------------------------------------------------------------------------
// Multiplexer for peripheral clock source (TODO missing one)

//#define CFG_MCO_SYS_CLK                            	0
//#define CFG_MCO_PLLI2S_R                           	RCC_CFGR_MCO_0
//#define CFG_MCO_HSE                                	RCC_CFGR_MCO_1
//#define CFG_MCO_PLL_CLK                            	(RCC_CFGR_MCO_1 | RCC_CFGR_MCO_0)

//-------------------------------------------------------------------------------------------------
// Configuration file(s)
//-------------------------------------------------------------------------------------------------

#include "clock_cfg.h"

//-------------------------------------------------------------------------------------------------
// default
//-------------------------------------------------------------------------------------------------

#ifdef CFG_LSE_VALUE
  #define LSE_CLOCK_FREQUENCY                       CFG_LSE_VALUE
#else
  #define LSE_CLOCK_FREQUENCY                       32768
#endif

//-------------------------------------------------------------------------------------------------
// Auto define configuration
//-------------------------------------------------------------------------------------------------

#define CFG_RCC_PLLCFGR_PLL_M_POS                   4
#define CFG_RCC_PLLCFGR_PLL_N_POS                   8
#define CFG_RCC_PLLCFGR_PLL_R_POS                   25
#define CFG_RCC_PLLCFGR_PLL_Q_POS                   21

#define CFG_HSI_VALUE                              HSI_CLOCK_FREQUENCY
#define CFG_MSI_VALUE                              4000000U
// +++++++++++++
// Keep PLL initialization enabled when USB selects PLLQ, even if SYSCLK/MCO use another source.
// +++++++++++++
#define CFG_USE_PLL                                ((CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL) || \
                                                    (CFG_MCO_SOURCE_MUX == CFG_RCC_CFGR_MCOSEL_PLL) || \
                                                    (CFG_USB_CLK48_SOURCE == CFG_RCC_CCIPR_CLK48SEL_PLLQ))

#if (CFG_RTC_CLOCK_SOURCE != CFG_RCC_BDCR_RTCSEL_NO_CLOCK) && (CFG_RTC_CLOCK_SOURCE != CFG_RCC_BDCR_RTCSEL_LSE) && (CFG_RTC_CLOCK_SOURCE != CFG_RCC_BDCR_RTCSEL_LSI) && (CFG_RTC_CLOCK_SOURCE != CFG_RCC_BDCR_RTCSEL_HSE)
  #error Invalid RTC clock source
#endif
#if (CFG_MCO_SOURCE_MUX < CFG_RCC_CFGR_MCOSEL_DISABLE) || (CFG_MCO_SOURCE_MUX > CFG_RCC_CFGR_MCOSEL_HSI48) || ((CFG_MCO_SOURCE_MUX & 0x00FFFFFFU) != 0U)
  #error Invalid MCO clock source
#endif
#if (CFG_MCO_CLK_DIVIDER != CFG_RCC_CFGR_MCOPRE_NO_DIV) && (CFG_MCO_CLK_DIVIDER != CFG_RCC_CFGR_MCOPRE_DIV2) && (CFG_MCO_CLK_DIVIDER != CFG_RCC_CFGR_MCOPRE_DIV4) && (CFG_MCO_CLK_DIVIDER != CFG_RCC_CFGR_MCOPRE_DIV8) && (CFG_MCO_CLK_DIVIDER != CFG_RCC_CFGR_MCOPRE_DIV16)
  #error Invalid MCO prescaler
#endif
#if (CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY < 1)
  #error Clock readiness retry count must be positive
#endif

#if (CFG_RCC_PLLCFGR_PLLSRC == CFG_RCC_PLLCFGR_PLLSRC_HSE)
  #define CFG_PLL_SOURCE                           CFG_HSE_VALUE
#elif (CFG_RCC_PLLCFGR_PLLSRC == CFG_RCC_PLLCFGR_PLLSRC_HSI)
  #define CFG_PLL_SOURCE                           CFG_HSI_VALUE
#elif (CFG_RCC_PLLCFGR_PLLSRC == CFG_RCC_PLLCFGR_PLLSRC_MSI)
  #define CFG_PLL_SOURCE                           CFG_MSI_VALUE
#else
  #error Invalid PLL clock source
#endif

//-------------------------------------------------------------------------------------------------
// Auto configuration value for PLL
#if CFG_USE_PLL

  #define CFG_PLL_CLK_FREQUENCY                     ((CFG_PLL_SOURCE * 1ULL * CFG_PLL_N_MULTIPLIER) / (CFG_PLL_M_DIVIDER * CFG_PLL_R_DIVIDER))
  #define CFG_PLL_Q_FREQUENCY                       ((CFG_PLL_SOURCE * 1ULL * CFG_PLL_N_MULTIPLIER) / (CFG_PLL_M_DIVIDER * CFG_PLL_Q_DIVIDER))

  #if (CFG_PLL_M_DIVIDER < 1) || (CFG_PLL_M_DIVIDER > 8)
    #error PLL_M is out of range
  #else
    #define CFG_RCC_PLLCFGR_PLL_M                   ((CFG_PLL_M_DIVIDER - 1) << CFG_RCC_PLLCFGR_PLL_M_POS)
  #endif

  #if (CFG_PLL_N_MULTIPLIER < 8) || (CFG_PLL_N_MULTIPLIER > 86)
    #error PLL_N is out of range
  #else
    #define CFG_RCC_PLLCFGR_PLL_N                   (CFG_PLL_N_MULTIPLIER << CFG_RCC_PLLCFGR_PLL_N_POS)
  #endif

  #if (CFG_PLL_R_DIVIDER != 2) && (CFG_PLL_R_DIVIDER != 4) && (CFG_PLL_R_DIVIDER != 6) && (CFG_PLL_R_DIVIDER != 8)
    #error PLL_R must be 2, 4, 6 or 8
  #else
    #define CFG_RCC_PLLCFGR_PLL_R                   (((CFG_PLL_R_DIVIDER / 2) - 1) << CFG_RCC_PLLCFGR_PLL_R_POS)
  #endif

  #if (CFG_PLL_Q_DIVIDER != 2) && (CFG_PLL_Q_DIVIDER != 4) && (CFG_PLL_Q_DIVIDER != 6) && (CFG_PLL_Q_DIVIDER != 8)
    #error PLL_Q is out of range
  #else
    #define CFG_RCC_PLLCFGR_PLL_Q                   (((CFG_PLL_Q_DIVIDER / 2) - 1) << CFG_RCC_PLLCFGR_PLL_Q_POS)
  #endif

  #if (CFG_PLL_SOURCE < (4000000ULL * CFG_PLL_M_DIVIDER)) || (CFG_PLL_SOURCE > (16000000ULL * CFG_PLL_M_DIVIDER))
    #error PLL input frequency must be between 4 MHz and 16 MHz
  #endif
  #if (CFG_PLL_SOURCE * 1ULL * CFG_PLL_N_MULTIPLIER < (64000000ULL * CFG_PLL_M_DIVIDER)) || (CFG_PLL_SOURCE * 1ULL * CFG_PLL_N_MULTIPLIER > (344000000ULL * CFG_PLL_M_DIVIDER))
    #error PLL VCO frequency must be between 64 MHz and 344 MHz
  #endif
  #if (CFG_PLL_SOURCE * 1ULL * CFG_PLL_N_MULTIPLIER > (80000000ULL * CFG_PLL_M_DIVIDER * CFG_PLL_R_DIVIDER))
    #error PLLR output frequency exceeds 80 MHz
  #endif

  #if CFG_ENABLE_PLL_Q
    #if (CFG_PLL_SOURCE * 1ULL * CFG_PLL_N_MULTIPLIER > (80000000ULL * CFG_PLL_M_DIVIDER * CFG_PLL_Q_DIVIDER))
      #error PLLQ output frequency exceeds 80 MHz
    #endif
    #define CFG_RCC_PLLCFGR_PLL_Q_ENABLE            RCC_PLLCFGR_PLLQEN
  #else
    #define CFG_RCC_PLLCFGR_PLL_Q_ENABLE            0U
  #endif

  #define CFG_RCC_PLLCFGR_CFG                       (CFG_RCC_PLLCFGR_PLL_M |   \
                                                     CFG_RCC_PLLCFGR_PLL_N |   \
                                                     CFG_RCC_PLLCFGR_PLL_R |   \
                                                     RCC_PLLCFGR_PLLREN   |   \
                                                     CFG_RCC_PLLCFGR_PLL_Q |   \
                                                     CFG_RCC_PLLCFGR_PLL_Q_ENABLE | \
                                                     CFG_RCC_PLLCFGR_PLLSRC)

  #define CFG_SYS_PLL_CLK_FREQUENCY                 CFG_PLL_CLK_FREQUENCY


#endif // CFG_USE_PLL

// --------------------------------------------------------------------------------------------------------------------------------

#if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_SYS_PLL_CLK_FREQUENCY
#elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSE)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_HSE_VALUE
#elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSI16)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_HSI_VALUE
#elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_MSI)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_MSI_VALUE
#else
  #error Invalid system clock source
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
#else
  #error Invalid AHB prescaler
#endif
#define CFG_SYS_HCLK                                CFG_AHB_CLK_DIVIDER

#define SYSTEM_CORE_CLOCK                           SYS_HCLK_CLOCK_FREQUENCY

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
#else
  #error Invalid APB1 prescaler
#endif
#define CFG_SYS_APB1                                CFG_APB1_CLK_DIVIDER
#if CFG_APB1_CLK_DIVIDER == CFG_RCC_CFGR_PPRE1_DIV1
#define SYS_APB1_TIMER_CLOCK_FREQUENCY              SYS_APB1_CLOCK_FREQUENCY
#else
#define SYS_APB1_TIMER_CLOCK_FREQUENCY              (SYS_APB1_CLOCK_FREQUENCY * 2)
#endif

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
#else
  #error Invalid APB2 prescaler
#endif
#define CFG_SYS_APB2                                CFG_APB2_CLK_DIVIDER
#if CFG_APB2_CLK_DIVIDER == CFG_RCC_CFGR_PPRE2_DIV1
#define SYS_APB2_TIMER_CLOCK_FREQUENCY              SYS_APB2_CLOCK_FREQUENCY
#else
#define SYS_APB2_TIMER_CLOCK_FREQUENCY              (SYS_APB2_CLOCK_FREQUENCY * 2)
#endif


#define POWER_REGULATOR_CFG                         PWR_CR1_VOS_0

#if   (SYS_HCLK_CLOCK_FREQUENCY <= 16000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_0WS
#elif (SYS_HCLK_CLOCK_FREQUENCY <= 32000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_1WS
#elif (SYS_HCLK_CLOCK_FREQUENCY <= 48000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_2WS
#elif (SYS_HCLK_CLOCK_FREQUENCY <= 64000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_3WS
#elif (SYS_HCLK_CLOCK_FREQUENCY <= 80000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_4WS
#else
 #error HCLK exceeds the supported Flash frequency
#endif

// Verification
#if SYS_CPU_CORE_CLOCK_FREQUENCY > CFG_MAX_CPU_SYS_HCLK_CLOCK_FREQUENCY
 #error SYSCLK frequency exceeds maximum allowed!
#endif

#if SYS_HCLK_CLOCK_FREQUENCY > CFG_MAX_CPU_SYS_HCLK_CLOCK_FREQUENCY
 #error CPU Core frequency exceed maximum allowed!
#endif

#if SYS_APB1_CLOCK_FREQUENCY > CFG_MAX_APB1_CLOCK_FREQUENCY
 #error APB1 frequency exceed maximum allowed!
#endif

#if SYS_APB2_CLOCK_FREQUENCY > CFG_MAX_APB2_CLOCK_FREQUENCY
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

extern uint32_t SystemCoreClock;

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SystemInit              (void);

//-------------------------------------------------------------------------------------------------
