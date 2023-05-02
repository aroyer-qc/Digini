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

#include "stm32f746xx.h"
#include <stdbool.h>
#include "lib_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Own define for register bit value without cast (Do not change)
#define CFG_RCC_CFGR_SW_HSI                         0x00000000U
#define CFG_RCC_CFGR_SW_HSE                         0x00000001U
#define CFG_RCC_CFGR_SW_PLL                         0x00000002U

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

#define FLASH_LATENCY_0                             FLASH_ACR_LATENCY_0WS       // FLASH Zero Latency cycle
#define FLASH_LATENCY_1                             FLASH_ACR_LATENCY_1WS       // FLASH One Latency cycle
#define FLASH_LATENCY_2                             FLASH_ACR_LATENCY_2WS       // FLASH Two Latency cycles
#define FLASH_LATENCY_3                             FLASH_ACR_LATENCY_3WS       // FLASH Three Latency cycles
#define FLASH_LATENCY_4                             FLASH_ACR_LATENCY_4WS       // FLASH Four Latency cycles
#define FLASH_LATENCY_5                             FLASH_ACR_LATENCY_5WS       // FLASH Five Latency cycles
#define FLASH_LATENCY_6                             FLASH_ACR_LATENCY_6WS       // FLASH Six Latency cycles
#define FLASH_LATENCY_7                             FLASH_ACR_LATENCY_7WS       // FLASH Seven Latency cycles
#define FLASH_LATENCY_8                             FLASH_ACR_LATENCY_8WS       // FLASH Eight Latency cycles
#define FLASH_LATENCY_9                             FLASH_ACR_LATENCY_9WS       // FLASH Nine Latency cycles
#define FLASH_LATENCY_10                            FLASH_ACR_LATENCY_10WS      // FLASH Ten Latency cycles
#define FLASH_LATENCY_11                            FLASH_ACR_LATENCY_11WS      // FLASH Eleven Latency cycles
#define FLASH_LATENCY_12                            FLASH_ACR_LATENCY_12WS      // FLASH Twelve Latency cycles
#define FLASH_LATENCY_13                            FLASH_ACR_LATENCY_13WS      // FLASH Thirteen Latency cycles
#define FLASH_LATENCY_14                            FLASH_ACR_LATENCY_14WS      // FLASH Fourteen Latency cycles
#define FLASH_LATENCY_15                            FLASH_ACR_LATENCY_15WS      // FLASH Fifteen Latency cycles

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

#if (SYS_CLOCK_MUX == RCC_CFGR_SW_PLL)

  // -----------------
  // HSI Check
  #define SYS_HSI_PLL_CLK_FREQUENCY                 (((HSI_PLLM_SOURCE / HSI_PLLM_DIVIDER) * HSI_PLLM_N_MULTIPLIER) / HSI_PLLM_P_DIVIDER)
  #define SYS_HSI_PLL_Q_FREQUENCY                   (((HSI_PLLM_SOURCE / HSI_PLLM_DIVIDER) * HSI_PLLM_N_MULTIPLIER) / HSI_PLLM_Q_DIVIDER)

  #if (HSI_PLLM_DIVIDER < 2) || (HSI_PLLM_DIVIDER > 63)
    #pragma message XSTR(HSI_PLLM_DIVIDER)
    #error PLLM for HSI is out of range
  #else
    #define RCC_PLL_CFGR_HSI_PLLM_CFG               (HSI_PLLM_DIVIDER << RCC_PLLCFGR_PLL_M_POS)
  #endif

  #if (HSI_PLLM_N_MULTIPLIER < 50) || (HSI_PLLM_N_MULTIPLIER > 432)
    #pragma message "XSTR(HSI_PLLM_N_MULTIPLIER)"
    #error PLLN for HSI is out of range
  #else
    #define RCC_PLL_CFGR_HSI_PLLN_CFG               (HSI_PLLM_N_MULTIPLIER << RCC_PLLCFGR_PLL_N_POS)
  #endif

  #if ((HSI_PLLM_P_DIVIDER / 2) < 1) || ((HSI_PLLM_P_DIVIDER / 2) > 4)   // (tested for 2,4,6,8)
    #pragma message "XSTR(HSI_PLLM_P_DIVIDER)"
    #error PLLP for HSI is out of range
  #else
    #define RCC_PLL_CFGR_HSI_PLLP_CFG               (((HSI_PLLM_P_DIVIDER / 2) - 1) << RCC_PLLCFGR_PLL_P_POS)
  #endif

  #if (HSI_PLLM_Q_DIVIDER < 2) || (HSI_PLLM_Q_DIVIDER > 15)
    #pragma message "XSTR(HSI_PLLM_Q_DIVIDER)"
    #error PLLQ for HSI is out of range
  #else
    #define RCC_PLL_CFGR_HSI_PLLQ_CFG               (HSI_PLLM_Q_DIVIDER << RCC_PLLCFGR_PLL_Q_POS)
  #endif

  #define RCC_HSI_PLL_CFGR_CFG                      (RCC_PLL_CFGR_HSI_PLLM_CFG |   \
                                                     RCC_PLL_CFGR_HSI_PLLN_CFG |   \
                                                     RCC_PLL_CFGR_HSI_PLLP_CFG |   \
                                                     RCC_PLL_CFGR_HSI_PLLQ_CFG |   \
                                                     RCC_PLLCFGR_PLLSRC_HSI)

  // -----------------
  // HSE Check
  #define SYS_HSE_PLL_CLK_FREQUENCY                 (((HSE_PLLM_SOURCE / HSE_PLLM_DIVIDER) * HSE_PLLM_N_MULTIPLIER) / HSE_PLLM_P_DIVIDER)
  #define SYS_HSE_PLL_Q_FREQUENCY                   (((HSE_PLLM_SOURCE / HSE_PLLM_DIVIDER) * HSE_PLLM_N_MULTIPLIER) / HSE_PLLM_Q_DIVIDER)

  #if (HSE_PLLM_DIVIDER < 2) || (HSE_PLLM_DIVIDER > 63)
    #pragma message XSTR(HSE_PLLM_DIVIDER)
    #error PLLM for HSE is out of range
  #else
    #define RCC_PLL_CFGR_HSE_PLLM_CFG               (HSE_PLLM_DIVIDER << RCC_PLLCFGR_PLL_M_POS)
  #endif

  #if (HSE_PLLM_N_MULTIPLIER < 50) || (HSE_PLLM_N_MULTIPLIER > 432)
    #pragma message "XSTR(HSE_PLLM_N_MULTIPLIER)"
    #error PLLN for HSE is out of range
  #else
    #define RCC_PLL_CFGR_HSE_PLLN_CFG               (HSE_PLLM_N_MULTIPLIER << RCC_PLLCFGR_PLL_N_POS)
  #endif

  #if ((HSE_PLLM_P_DIVIDER / 2) < 1) || ((HSE_PLLM_P_DIVIDER / 2) > 4)   // (tested for 2,4,6,8)
    #pragma message "XSTR(HSE_PLLM_P_DIVIDER)"
    #error PLLP for HSE is out of range
  #else
    #define RCC_PLL_CFGR_HSE_PLLP_CFG               (((HSE_PLLM_P_DIVIDER / 2) - 1) << RCC_PLLCFGR_PLL_P_POS)
  #endif

  #if (HSE_PLLM_Q_DIVIDER < 2) || (HSE_PLLM_Q_DIVIDER > 15)
    #pragma message "XSTR(HSE_PLLM_Q_DIVIDER)"
    #error PLLQ for HSE is out of range
  #else
    #define RCC_PLL_CFGR_HSE_PLLQ_CFG               (HSE_PLLM_Q_DIVIDER << RCC_PLLCFGR_PLL_Q_POS)
  #endif

  #define RCC_HSE_PLL_CFGR_CFG                      (RCC_PLL_CFGR_HSE_PLLM_CFG |   \
                                                     RCC_PLL_CFGR_HSE_PLLN_CFG |   \
                                                     RCC_PLL_CFGR_HSE_PLLP_CFG |   \
                                                     RCC_PLL_CFGR_HSE_PLLQ_CFG |   \
                                                     RCC_PLLCFGR_PLLSRC_HSE)

// do if else for HSE or HSE
    #define SYS_PLL_CLK_FREQUENCY                   SYS_HSE_PLL_CLK_FREQUENCY

#endif // SYS_CLOCK_MUX == RCC_CFGR_SW_PLL

// --------------------------------------------------------------------------------------------------------------------------------

#if (SYS_CLOCK_MUX == RCC_CFGR_SW_PLL)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              SYS_PLL_CLK_FREQUENCY
#elif (SYS_CLOCK_MUX == RCC_CFGR_SW_HSE)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              SYS_HSE_VALUE
#else //(SYS_CLOCK_MUX == RCC_CFGR_SW_HSI)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              SYS_HSI_VALUE
#endif

// --------------------------------------------------------------------------------------------------------------------------------

#define SYS_HCLK_CFG                                HCLK_DIVIDER

#if   HCLK_DIVIDER == RCC_CFGR_HPRE_DIV1
    #define SYS_HCLK_CLOCK_FREQUENCY                SYS_CPU_CORE_CLOCK_FREQUENCY
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV2
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 2)
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV4
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 4)
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV8
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 8)
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV16
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 16)
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV64
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 64)
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV128
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 128)
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV256
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 256)
#elif HCLK_DIVIDER == RCC_CFGR_HPRE_DIV512
    #define SYS_HCLK_CLOCK_FREQUENCY                (SYS_CPU_CORE_CLOCK_FREQUENCY / 512)
#endif

#define SYSTEM_CORE_CLOCK                           SYS_HCLK_CLOCK_FREQUENCY

// --------------------------------------------------------------------------------------------------------------------------------

#define SYS_APB1_CFG                                APB1_CLK_DIVIDER

#if   APB1_CLK_DIVIDER == RCC_CFGR_PPRE1_DIV1
    #define SYS_APB1_CLOCK_FREQUENCY                SYS_HCLK_CLOCK_FREQUENCY
#elif APB1_CLK_DIVIDER == RCC_CFGR_PPRE1_DIV2
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif APB1_CLK_DIVIDER == RCC_CFGR_PPRE1_DIV4
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif APB1_CLK_DIVIDER == RCC_CFGR_PPRE1_DIV8
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif APB1_CLK_DIVIDER == RCC_CFGR_PPRE1_DIV16
    #define SYS_APB1_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 16)
#endif

#define SYS_APB1_TIMER_CLOCK_FREQUENCY              (SYS_APB1_CLOCK_FREQUENCY * 2)

// --------------------------------------------------------------------------------------------------------------------------------

#define SYS_APB2_CFG                                APB2_CLK_DIVIDER

#if   APB2_CLK_DIVIDER == RCC_CFGR_PPRE2_DIV1
    #define SYS_APB2_CLOCK_FREQUENCY                SYS_HCLK_CLOCK_FREQUENCY
#elif APB2_CLK_DIVIDER == RCC_CFGR_PPRE2_DIV2
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif APB2_CLK_DIVIDER == RCC_CFGR_PPRE2_DIV4
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif APB2_CLK_DIVIDER == RCC_CFGR_PPRE2_DIV8
    #define SYS_APB2_CLOCK_FREQUENCY                (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif APB2_CLK_DIVIDER == RCC_CFGR_PPRE2_DIV16
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
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_0WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 60000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_1WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 90000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_2WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 120000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_3WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 150000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_4WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 180000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_5WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 210000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_6WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 216000000)
#define FLASH_LATENCY_CFG                           FLASH_ACR_LATENCY_7WS
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
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

extern uint32_t SystemCoreClock;
extern uint32_t APB1_Clock;

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void SystemInit(void);

//-------------------------------------------------------------------------------------------------


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
SYSTEM_EXTERN   uint32_t        APB1_Clock;

#if (HSE_AND_HSI_MUST_BE_MATCHED == DEF_ENABLED)                // TODO
SYSTEM_EXTERN   bool            SystemHSE_ClockFailure;
#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SystemInit              (void);

//-------------------------------------------------------------------------------------------------


