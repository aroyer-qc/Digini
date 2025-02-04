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
// Macro(s)
//-------------------------------------------------------------------------------------------------

//#define XSTR(x) STR(x)
//#define STR(x) #x

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#ifdef STM32H735xx
    #define MAX_SYS_HCLK                        550000000
    #define MAX_SYS_AXI                         275000000
    #define MAX_PCLK1                           137500000
    #define MAX_PCLK2                           137500000
    #define MAX_PCLK3                           137500000
    #define MAX_PCLK4                           137500000
    #define MAX_AHB4                            275000000
    #define VCOSEL_FREQUENCY                    2000000                         // Higher than this PLLx_VCOSEL is 0, otherwise it is 1

    #define MIN_DIVM1                           1
    #define MAX_DIVM1                           63
    #define MIN_MULN1                           4
    #define MAX_MULN1                           512
    #define MIN_DIVP1                           1
    #define MAX_DIVP1                           128
    #define MIN_DIVQ1                           1
    #define MAX_DIVQ1                           128
    #define MIN_DIVR1                           1
    #define MAX_DIVR1                           128

    #define MIN_DIVM2                           1
    #define MAX_DIVM2                           63
    #define MIN_MULN2                           4
    #define MAX_MULN2                           512
    #define MIN_DIVP2                           1
    #define MAX_DIVP2                           128
    #define MIN_DIVQ2                           1
    #define MAX_DIVQ2                           128
    #define MIN_DIVR2                           1
    #define MAX_DIVR2                           128

    #define MIN_DIVM3                           1
    #define MAX_DIVM3                           63
    #define MIN_MULN3                           4
    #define MAX_MULN3                           512
    #define MIN_DIVP3                           1
    #define MAX_DIVP3                           128
    #define MIN_DIVQ3                           1
    #define MAX_DIVQ3                           128
    #define MIN_DIVR3                           1
    #define MAX_DIVR3                           128
#endif

#ifdef STM32H745xx
    #define MAX_SYS_HCLK                        480000000
    #define MAX_SYS_AXI                         240000000
    #define MAX_PCLK1                           120000000
    #define MAX_PCLK2                           120000000
    #define MAX_PCLK3                           120000000
    #define MAX_PCLK4                           120000000
    #define MAX_AHB4                            240000000
    #define VCOSEL_FREQUENCY                    2000000

    #define MIN_DIVM1                           1
    #define MAX_DIVM1                           63
    #define MIN_MULN1                           4
    #define MAX_MULN1                           512
    #define MIN_DIVP1                           2
    #define MAX_DIVP1                           128
    #define MIN_DIVQ1                           1
    #define MAX_DIVQ1                           128
    #define MIN_DIVR1                           1
    #define MAX_DIVR1                           128

    #define MIN_DIVM2                           1
    #define MAX_DIVM2                           63
    #define MIN_MULN2                           4
    #define MAX_MULN2                           512
    #define MIN_DIVP2                           1
    #define MAX_DIVP2                           128
    #define MIN_DIVQ2                           1
    #define MAX_DIVQ2                           128
    #define MIN_DIVR2                           1
    #define MAX_DIVR2                           128

    #define MIN_DIVM3                           1
    #define MAX_DIVM3                           63
    #define MIN_MULN3                           4
    #define MAX_MULN3                           512
    #define MIN_DIVP3                           1
    #define MAX_DIVP3                           128
    #define MIN_DIVQ3                           1
    #define MAX_DIVQ3                           128
    #define MIN_DIVR3                           1
    #define MAX_DIVR3                           128
#endif

#ifdef STM32H7B3xx
    #define MAX_SYS_HCLK                        280000000
    #define MAX_SYS_AXI                         140000000
    #define MAX_PCLK1                           140000000
    #define MAX_PCLK2                           140000000
    #define MAX_PCLK3                           140000000
    #define MAX_PCLK4                           140000000
    #define MAX_AHB4                            140000000
    #define VCOSEL_FREQUENCY                    2000000

    #define MIN_DIVM1                           1
    #define MAX_DIVM1                           63
    #define MIN_MULN1                           8
    #define MAX_MULN1                           420
    #define MIN_DIVP1                           2
    #define MAX_DIVP1                           128
    #define MIN_DIVQ1                           1
    #define MAX_DIVQ1                           128
    #define MIN_DIVR1                           1
    #define MAX_DIVR1                           128

    #define MIN_DIVM2                           1
    #define MAX_DIVM2                           63
    #define MIN_MULN2                           8
    #define MAX_MULN2                           420
    #define MIN_DIVP2                           1
    #define MAX_DIVP2                           128
    #define MIN_DIVQ2                           1
    #define MAX_DIVQ2                           128
    #define MIN_DIVR2                           1
    #define MAX_DIVR2                           128

    #define MIN_DIVM3                           1
    #define MAX_DIVM3                           63
    #define MIN_MULN3                           8
    #define MAX_MULN3                           420
    #define MIN_DIVP3                           1
    #define MAX_DIVP3                           128
    #define MIN_DIVQ3                           1
    #define MAX_DIVQ3                           128
    #define MIN_DIVR3                           1
    #define MAX_DIVR3                           128
#endif


#define LSI_CLOCK_FREQUENCY                     32000
#define HSI_RC_CLOCK_FREQUENCY                  64000000
#define CSI_CLOCK_FREQUENCY                     4000000
#define RC48_CLOCK_FREQUENCY                    48000000

//-------------------------------------------------------------------------------------------------
// Multiplexer for peripheral clock source

// System clock mux selection
#define CFG_RCC_CFGR_SW_HSI                     0x00000000
#define CFG_RCC_CFGR_SW_CSI                     0x00000001
#define CFG_RCC_CFGR_SW_HSE                     0x00000002
#define CFG_RCC_CFGR_SW_PLL1                    0x00000003

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
#define CFG_RCC_D2CCIP1R_FDCAN_HSE              0
#define CFG_RCC_D2CCIP1R_FDCAN_PLL1Q            RCC_D2CCIP1R_FDCANSEL_0
#define CFG_RCC_D2CCIP1R_FDCAN_PLL2Q            RCC_D2CCIP1R_FDCANSEL_1

// Multiplexer define to be used for FMC clock source                                                   -> CFG_FMC_SOURCE_MUX           default: HCLK3
#define CFG_RCC_D1CCIPR_FMC_HCLK3               0
#define CFG_RCC_D1CCIPR_FMC_PLL1Q               RCC_D1CCIPR_FMCSEL_0
#define CFG_RCC_D1CCIPR_FMC_PLL2R               RCC_D1CCIPR_FMCSEL_1
#define CFG_RCC_D1CCIPR_FMC_PER_CLK             (RCC_D1CCIPR_FMCSEL_1 | RCC_D1CCIPR_FMCSEL_0)

#if defined (RCC_D2CCIP2R_I2C123SEL)
  // Multiplexer define to be used for I2C123 clock source                                              -> CFG_I2C123_SOURCE_MUX        default: PCLK1
  #define CFG_RCC_D2CCIP2R_I2C123_PCLK1         0
  #define CFG_RCC_D2CCIP2R_I2C123_PLL3R         RCC_D2CCIP2R_I2C123SEL_0
  #define CFG_RCC_D2CCIP2R_I2C123_HSI           RCC_D2CCIP2R_I2C123SEL_1
  #define CFG_RCC_D2CCIP2R_I2C123_CSI           (RCC_D2CCIP2R_I2C123SEL_1 | RCC_D2CCIP2R_I2C123SEL_0)
#else
  // Multiplexer define to be used for I2C1235 clock source                                             -> CFG_I2C1235_SOURCE_MUX       default: PCLK1
  #define CFG_RCC_D2CCIP2R_I2C1235_PCLK1        0
  #define CFG_RCC_D2CCIP2R_I2C1235_PLL3R        RCC_D2CCIP2R_I2C1235SEL_0
  #define CFG_RCC_D2CCIP2R_I2C1235_HSI          RCC_D2CCIP2R_I2C1235SEL_1
  #define CFG_RCC_D2CCIP2R_I2C1235_CSI          (RCC_D2CCIP2R_I2C1235SEL_1 | RCC_D2CCIP2R_I2C1235SEL_0)
#endif

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

#if defined(QUADSPI)
// Multiplexer define to be used for QUADSPI clock source                                               -> CFG_QSPI_SOURCE_MUX          default: PLL1Q
  #define CFG_RCC_D1CCIPR_QSPI_HCLK3            0
  #define CFG_RCC_D1CCIPR_QSPI_PLL1Q            RCC_D1CCIPR_QSPISEL_0
  #define CFG_RCC_D1CCIPR_QSPI_PLL2R            RCC_D1CCIPR_QSPISEL_1
  #define CFG_RCC_D1CCIPR_QSPI_PER_CLK          (RCC_D1CCIPR_QSPISEL_1 | RCC_D1CCIPR_QSPISEL_0)
#endif

#if defined(OCTOSPI1) || defined(OCTOSPI2)
// Multiplexer define to be used for OCTOSPI clock source                                               -> CFG_OSPI_SOURCE_MUX          default: PLL1Q
  #define CFG_RCC_D1CCIPR_OSPI_HCLK3            0
  #define CFG_RCC_D1CCIPR_OSPI_PLL1Q            RCC_D1CCIPR_OCTOSPISEL_0
  #define CFG_RCC_D1CCIPR_OSPI_PLL2R            RCC_D1CCIPR_OCTOSPISEL_1
  #define CFG_RCC_D1CCIPR_OSPI_PER_CLK          (RCC_D1CCIPR_OCTOSPISEL_1 | RCC_CDCCIPR_OCTOSPISEL_0)
#endif

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

// Multiplexer define to be used for UART1, UART6 clock source                                          -> CFG_UART16_SOURCE_MUX        default: PCLK2
#if defined(RCC_D2CCIP2R_USART16SEL)
  #define CFG_RCC_D2CCIP2R_USART16_PCLK2        0
  #define CFG_RCC_D2CCIP2R_USART16_PLL2Q        RCC_D2CCIP2R_USART16SEL_0
  #define CFG_RCC_D2CCIP2R_USART16_PLL3Q        RCC_D2CCIP2R_USART16SEL_1
  #define CFG_RCC_D2CCIP2R_USART16_HSI          (RCC_D2CCIP2R_USART16SEL_1 | RCC_D2CCIP2R_USART16SEL_0)
  #define CFG_RCC_D2CCIP2R_USART16_CSI          RCC_D2CCIP2R_USART16SEL_2
  #define CFG_RCC_D2CCIP2R_USART16_LSE          (RCC_D2CCIP2R_USART16SEL_2 | RCC_D2CCIP2R_USART16SEL_0)
#else
// Multiplexer define to be used for UART1, UART6, UART9, UART10 clock source                           -> CFG_UART16910_SOURCE_MUX     default: PCLK2
  #define CFG_RCC_D2CCIP2R_USART16910_PCLK2     0
  #define CFG_RCC_D2CCIP2R_USART16910_PLL2Q     RCC_D2CCIP2R_USART16910SEL_0
  #define CFG_RCC_D2CCIP2R_USART16910_PLL3Q     RCC_D2CCIP2R_USART16910SEL_1
  #define CFG_RCC_D2CCIP2R_USART16910_HSI       (RCC_D2CCIP2R_USART16910SEL_1 | RCC_D2CCIP2R_USART16910SEL_0)
  #define CFG_RCC_D2CCIP2R_USART16910_CSI       RCC_D2CCIP2R_USART16910SEL_2
  #define CFG_RCC_D2CCIP2R_USART16910_LSE       (RCC_D2CCIP2R_USART16910SEL_2 | RCC_D2CCIP2R_USART16910SEL_0)
#endif

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

// Divider for all Clock

#if defined(RCC_D1CFGR_D1CPRE_DIV1)
  #define CFG_SYSCLK_DIV1                       RCC_D1CFGR_D1CPRE_DIV1
  #define CFG_SYSCLK_DIV2                       RCC_D1CFGR_D1CPRE_DIV2
  #define CFG_SYSCLK_DIV4                       RCC_D1CFGR_D1CPRE_DIV4
  #define CFG_SYSCLK_DIV8                       RCC_D1CFGR_D1CPRE_DIV8
  #define CFG_SYSCLK_DIV16                      RCC_D1CFGR_D1CPRE_DIV16
  #define CFG_SYSCLK_DIV64                      RCC_D1CFGR_D1CPRE_DIV64
  #define CFG_SYSCLK_DIV128                     RCC_D1CFGR_D1CPRE_DIV128
  #define CFG_SYSCLK_DIV256                     RCC_D1CFGR_D1CPRE_DIV256
  #define CFG_SYSCLK_DIV512                     RCC_D1CFGR_D1CPRE_DIV512
#else
  #define CFG_SYSCLK_DIV1                       RCC_CDCFGR1_CDCPRE_DIV1
  #define CFG_SYSCLK_DIV2                       RCC_CDCFGR1_CDCPRE_DIV2
  #define CFG_SYSCLK_DIV4                       RCC_CDCFGR1_CDCPRE_DIV4
  #define CFG_SYSCLK_DIV8                       RCC_CDCFGR1_CDCPRE_DIV8
  #define CFG_SYSCLK_DIV16                      RCC_CDCFGR1_CDCPRE_DIV16
  #define CFG_SYSCLK_DIV64                      RCC_CDCFGR1_CDCPRE_DIV64
  #define CFG_SYSCLK_DIV128                     RCC_CDCFGR1_CDCPRE_DIV128
  #define CFG_SYSCLK_DIV256                     RCC_CDCFGR1_CDCPRE_DIV256
  #define CFG_SYSCLK_DIV512                     RCC_CDCFGR1_CDCPRE_DIV512
#endif

#if defined(RCC_D1CFGR_HPRE_DIV1)
  #define CFG_HCLK_DIV1                         RCC_D1CFGR_HPRE_DIV1
  #define CFG_HCLK_DIV2                         RCC_D1CFGR_HPRE_DIV2
  #define CFG_HCLK_DIV4                         RCC_D1CFGR_HPRE_DIV4
  #define CFG_HCLK_DIV8                         RCC_D1CFGR_HPRE_DIV8
  #define CFG_HCLK_DIV16                        RCC_D1CFGR_HPRE_DIV16
  #define CFG_HCLK_DIV64                        RCC_D1CFGR_HPRE_DIV64
  #define CFG_HCLK_DIV128                       RCC_D1CFGR_HPRE_DIV128
  #define CFG_HCLK_DIV256                       RCC_D1CFGR_HPRE_DIV256
  #define CFG_HCLK_DIV512                       RCC_D1CFGR_HPRE_DIV512
#else
  #define CFG_HCLK_DIV1                         RCC_CDCFGR1_HPRE_DIV1
  #define CFG_HCLK_DIV2                         RCC_CDCFGR1_HPRE_DIV2
  #define CFG_HCLK_DIV4                         RCC_CDCFGR1_HPRE_DIV4
  #define CFG_HCLK_DIV8                         RCC_CDCFGR1_HPRE_DIV8
  #define CFG_HCLK_DIV16                        RCC_CDCFGR1_HPRE_DIV16
  #define CFG_HCLK_DIV64                        RCC_CDCFGR1_HPRE_DIV64
  #define CFG_HCLK_DIV128                       RCC_CDCFGR1_HPRE_DIV128
  #define CFG_HCLK_DIV256                       RCC_CDCFGR1_HPRE_DIV256
  #define CFG_HCLK_DIV512                       RCC_CDCFGR1_HPRE_DIV512
#endif

#if defined (RCC_D2CFGR_D2PPRE1_DIV1)
  #define CFG_APB1_DIV1                         RCC_D2CFGR_D2PPRE1_DIV1
  #define CFG_APB1_DIV2                         RCC_D2CFGR_D2PPRE1_DIV2
  #define CFG_APB1_DIV4                         RCC_D2CFGR_D2PPRE1_DIV4
  #define CFG_APB1_DIV8                         RCC_D2CFGR_D2PPRE1_DIV8
  #define CFG_APB1_DIV16                        RCC_D2CFGR_D2PPRE1_DIV16
#else
  #define CFG_APB1_DIV1                         RCC_CDCFGR2_CDPPRE1_DIV1
  #define CFG_APB1_DIV2                         RCC_CDCFGR2_CDPPRE1_DIV2
  #define CFG_APB1_DIV4                         RCC_CDCFGR2_CDPPRE1_DIV4
  #define CFG_APB1_DIV8                         RCC_CDCFGR2_CDPPRE1_DIV8
  #define CFG_APB1_DIV16                        RCC_CDCFGR2_CDPPRE1_DIV16
#endif

#if defined (RCC_D2CFGR_D2PPRE2_DIV1)
  #define CFG_APB2_DIV1                         RCC_D2CFGR_D2PPRE2_DIV1
  #define CFG_APB2_DIV2                         RCC_D2CFGR_D2PPRE2_DIV2
  #define CFG_APB2_DIV4                         RCC_D2CFGR_D2PPRE2_DIV4
  #define CFG_APB2_DIV8                         RCC_D2CFGR_D2PPRE2_DIV8
  #define CFG_APB2_DIV16                        RCC_D2CFGR_D2PPRE2_DIV16
#else
  #define CFG_APB2_DIV1                         RCC_CDCFGR2_CDPPRE2_DIV1
  #define CFG_APB2_DIV2                         RCC_CDCFGR2_CDPPRE2_DIV2
  #define CFG_APB2_DIV4                         RCC_CDCFGR2_CDPPRE2_DIV4
  #define CFG_APB2_DIV8                         RCC_CDCFGR2_CDPPRE2_DIV8
  #define CFG_APB2_DIV16                        RCC_CDCFGR2_CDPPRE2_DIV16
#endif

#if defined (RCC_D1CFGR_D1PPRE_DIV1)
  #define CFG_APB3_DIV1                         RCC_D1CFGR_D1PPRE_DIV1
  #define CFG_APB3_DIV2                         RCC_D1CFGR_D1PPRE_DIV2
  #define CFG_APB3_DIV4                         RCC_D1CFGR_D1PPRE_DIV4
  #define CFG_APB3_DIV8                         RCC_D1CFGR_D1PPRE_DIV8
  #define CFG_APB3_DIV16                        RCC_D1CFGR_D1PPRE_DIV16
#else
  #define CFG_APB3_DIV1                         RCC_CDCFGR1_CDPPRE_DIV1
  #define CFG_APB3_DIV2                         RCC_CDCFGR1_CDPPRE_DIV2
  #define CFG_APB3_DIV4                         RCC_CDCFGR1_CDPPRE_DIV4
  #define CFG_APB3_DIV8                         RCC_CDCFGR1_CDPPRE_DIV8
  #define CFG_APB3_DIV16                        RCC_CDCFGR1_CDPPRE_DIV16
#endif

#if defined(RCC_D3CFGR_D3PPRE_DIV1)
  #define CFG_APB4_DIV1                         RCC_D3CFGR_D3PPRE_DIV1
  #define CFG_APB4_DIV2                         RCC_D3CFGR_D3PPRE_DIV2
  #define CFG_APB4_DIV4                         RCC_D3CFGR_D3PPRE_DIV4
  #define CFG_APB4_DIV8                         RCC_D3CFGR_D3PPRE_DIV8
  #define CFG_APB4_DIV16                        RCC_D3CFGR_D3PPRE_DIV16
#else
  #define CFG_APB4_DIV1                         RCC_SRDCFGR_SRDPPRE_DIV1
  #define CFG_APB4_DIV2                         RCC_SRDCFGR_SRDPPRE_DIV2
  #define CFG_APB4_DIV4                         RCC_SRDCFGR_SRDPPRE_DIV4
  #define CFG_APB4_DIV8                         RCC_SRDCFGR_SRDPPRE_DIV8
  #define CFG_APB4_DIV16                        RCC_SRDCFGR_SRDPPRE_DIV16
#endif

// Valid for PLL1, PLL2, PLL3
#define CFG_RCC_PLLDIVR_PLL_N_POS               0
#define CFG_RCC_PLLDIVR_PLL_P_POS               9
#define CFG_RCC_PLLDIVR_PLL_Q_POS               16
#define CFG_RCC_PLLDIVR_PLL_R_POS               24

// PLL Clock Source and POS
#define CFG_RCC_PLLCKSELR_PLLSRC_HSI            0x00000000
#define CFG_RCC_PLLCKSELR_PLLSRC_CSI            0x00000001
#define CFG_RCC_PLLCKSELR_PLLSRC_HSE            0x00000002
#define CFG_RCC_PLLCKSELR_PLLSRC_POS            0
#define CFG_RCC_PLLCKSELR_PLL1_DIV_M_POS        4
#define CFG_RCC_PLLCKSELR_PLL2_DIV_M_POS        12
#define CFG_RCC_PLLCKSELR_PLL3_DIV_M_POS        20

// Enable for PLL output bit pos
#define RCC_PLLCFGR_PLL1P_POS                   16
#define RCC_PLLCFGR_PLL1Q_POS                   17
#define RCC_PLLCFGR_PLL1R_POS                   18
#define RCC_PLLCFGR_PLL2P_POS                   19
#define RCC_PLLCFGR_PLL2Q_POS                   20
#define RCC_PLLCFGR_PLL2R_POS                   21
#define RCC_PLLCFGR_PLL3P_POS                   22
#define RCC_PLLCFGR_PLL3Q_POS                   23
#define RCC_PLLCFGR_PLL3R_POS                   24

// Power regulator
#define CFG_PWR_REGULATOR_VOLTAGE_SCALE0        (0)
#define CFG_PWR_REGULATOR_VOLTAGE_SCALE1        (PWR_D3CR_VOS_1 | PWR_D3CR_VOS_0)
#define CFG_PWR_REGULATOR_VOLTAGE_SCALE2        (PWR_D3CR_VOS_1)
#define CFG_PWR_REGULATOR_VOLTAGE_SCALE3        (PWR_D3CR_VOS_0)

//-------------------------------------------------------------------------------------------------
// Configuration file(s)
//-------------------------------------------------------------------------------------------------

#include "clock_cfg.h"

//-------------------------------------------------------------------------------------------------
// Auto define configuration
//-------------------------------------------------------------------------------------------------

#define HSI_CLOCK_FREQUENCY                         (HSI_RC_CLOCK_FREQUENCY / CFG_HSI_DIVIDER)


/// --------------------------------------------------------------------------------------------------------------------------------
/// Autoconfig value for PLL

#if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL1)

  #define SYS_PLL_CLK_FREQUENCY                     (((CFG_FREQ_PLL_SOURCE / CFG_PLL1_M_DIVIDER) * CFG_PLL1_N_MULTIPLIER) / CFG_PLL1_P_DIVIDER)

#endif // CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL1

/// -------------------------------------------------------------------------------------------------------------------------------
/// PLL1 Configuration
///
/// need to account example: 2 =  1 in selection for all divider!!!!

#if (CFG_PLL1_M_DIVIDER < MIN_DIVM1) || (CFG_PLL1_M_DIVIDER > MAX_DIVM1)
  #pragma message XSTR(CFG_PLL1_M_DIVIDER)
  #error PLL1M is out of range
#else
  #define CFG_RCC_PLLCKSELR_PLL1_M                  (CFG_PLL1_M_DIVIDER << CFG_RCC_PLLCKSELR_PLL1_DIV_M_POS)
#endif

#if (CFG_PLL1_N_MULTIPLIER < MIN_MULN1) || (CFG_PLL1_N_MULTIPLIER > MAX_MULN1)
  #pragma message "XSTR(CFG_PLL1_N_MULTIPLIER)"
  #error PLL1N is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL1_N                    (CFG_PLL1_N_MULTIPLIER << CFG_RCC_PLLDIVR_PLL_N_POS)
#endif

#if (CFG_PLL1_P_DIVIDER < MIN_DIVP1) || (CFG_PLL1_P_DIVIDER > MAX_DIVP1)
  #pragma message "XSTR(CFG_PLL1_P_DIVIDER)"
  #error PLL1P is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL1_P                    (CFG_PLL1_P_DIVIDER << CFG_RCC_PLLDIVR_PLL_P_POS)
#endif

#if (CFG_PLL1_Q_DIVIDER < MIN_DIVQ1) || (CFG_PLL1_Q_DIVIDER > MAX_DIVQ1)
  #pragma message "XSTR(CFG_PLL1_Q_DIVIDER)"
  #error PLL1Q is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL1_Q                    (CFG_PLL1_Q_DIVIDER << CFG_RCC_PLLDIVR_PLL_Q_POS)
#endif

#if (CFG_PLL1_R_DIVIDER < MIN_DIVR1) || (CFG_PLL1_R_DIVIDER > MAX_DIVR1)
  #pragma message "XSTR(CFG_PLL1_R_DIVIDER)"
  #error PLL1R is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL1_R                    (CFG_PLL1_R_DIVIDER << CFG_RCC_PLLDIVR_PLL_R_POS)
#endif

#define CFG_RCC_PLL1_DIVR                           (CFG_RCC_PLLDIVR_PLL1_N |   \
                                                     CFG_RCC_PLLDIVR_PLL1_P |   \
                                                     CFG_RCC_PLLDIVR_PLL1_Q |   \
                                                     CFG_RCC_PLLDIVR_PLL1_R)

#define PLL1_CLOCK_FREQUENCY                        ((CFG_FREQ_PLL_SOURCE / CFG_PLL1_M_DIVIDER) * CFG_PLL1_N_MULTIPLIER)
#define PLL1P_CLOCK_FREQUENCY                       (PLL1_CLOCK_FREQUENCY / CFG_PLL1_P_DIVIDER)
#define PLL1Q_CLOCK_FREQUENCY                       (PLL1_CLOCK_FREQUENCY / CFG_PLL1_Q_DIVIDER)
#define PLL1R_CLOCK_FREQUENCY                       (PLL1_CLOCK_FREQUENCY / CFG_PLL1_R_DIVIDER)

/// -------------------------------------------------------------------------------------------------------------------------------
/// PLL2 Configuration
///

#if (CFG_PLL2_M_DIVIDER < MIN_DIVM2) || (CFG_PLL2_M_DIVIDER > MAX_DIVM2)
  #pragma message XSTR(CFG_PLL2_M_DIVIDER)
  #error PLL2M is out of range
#else
  #define CFG_RCC_PLLCKSELR_PLL2_M                  (CFG_PLL2_M_DIVIDER << CFG_RCC_PLLCKSELR_PLL2_DIV_M_POS)
#endif

#if (CFG_PLL2_N_MULTIPLIER < MIN_MULN2) || (CFG_PLL2_N_MULTIPLIER > MAX_MULN2)
  #pragma message "XSTR(CFG_PLL2_N_MULTIPLIER)"
  #error PLL2N is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL2_N                    (CFG_PLL2_N_MULTIPLIER << CFG_RCC_PLLDIVR_PLL_N_POS)
#endif

#if (CFG_PLL2_P_DIVIDER < MIN_DIVP2) || (CFG_PLL2_P_DIVIDER > MAX_DIVP2)
  #pragma message "XSTR(CFG_PLL2_P_DIVIDER)"
  #error PLL2P is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL2_P                    (CFG_PLL2_P_DIVIDER << CFG_RCC_PLLDIVR_PLL_P_POS)
#endif

#if (CFG_PLL2_Q_DIVIDER < MIN_DIVQ2) || (CFG_PLL2_Q_DIVIDER > MAX_DIVQ2)
  #pragma message "XSTR(CFG_PLL2_Q_DIVIDER)"
  #error PLL2Q is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL2_Q                    (CFG_PLL2_Q_DIVIDER << CFG_RCC_PLLDIVR_PLL_Q_POS)
#endif

#if (CFG_PLL2_R_DIVIDER < MIN_DIVR2) || (CFG_PLL2_R_DIVIDER > MAX_DIVR2)
  #pragma message "XSTR(CFG_PLL2_R_DIVIDER)"
  #error PLL2R is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL2_R                    (CFG_PLL2_R_DIVIDER << CFG_RCC_PLLDIVR_PLL_R_POS)
#endif

#define CFG_RCC_PLL2_DIVR                           (CFG_RCC_PLLDIVR_PLL2_N |   \
                                                     CFG_RCC_PLLDIVR_PLL2_P |   \
                                                     CFG_RCC_PLLDIVR_PLL2_Q |   \
                                                     CFG_RCC_PLLDIVR_PLL2_R)

#define PLL2_CLOCK_FREQUENCY                        ((CFG_FREQ_PLL_SOURCE / CFG_PLL2_M_DIVIDER) * CFG_PLL2_N_MULTIPLIER)
#define PLL2P_CLOCK_FREQUENCY                       (PLL2_CLOCK_FREQUENCY / CFG_PLL2_P_DIVIDER)
#define PLL2Q_CLOCK_FREQUENCY                       (PLL2_CLOCK_FREQUENCY / CFG_PLL2_Q_DIVIDER)
#define PLL2R_CLOCK_FREQUENCY                       (PLL2_CLOCK_FREQUENCY / CFG_PLL2_R_DIVIDER)

/// -------------------------------------------------------------------------------------------------------------------------------
/// PLL3 Configuration
///

#if (CFG_PLL3_M_DIVIDER < MIN_DIVM3) || (CFG_PLL3_M_DIVIDER > MAX_DIVM3)
  #pragma message XSTR(CFG_PLL3_M_DIVIDER)
  #error PLL3M is out of range
#else
  #define CFG_RCC_PLLCKSELR_PLL3_M                  (CFG_PLL3_M_DIVIDER << CFG_RCC_PLLCKSELR_PLL3_DIV_M_POS)
#endif

#if (CFG_PLL3_N_MULTIPLIER < MIN_MULN3) || (CFG_PLL3_N_MULTIPLIER > MAX_MULN3)
  #pragma message "XSTR(CFG_PLL3_N_MULTIPLIER)"
  #error PLL3N is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL3_N                    (CFG_PLL3_N_MULTIPLIER << CFG_RCC_PLLDIVR_PLL_N_POS)
#endif

#if (CFG_PLL3_P_DIVIDER < MIN_DIVP3) || (CFG_PLL3_P_DIVIDER > MAX_DIVP3)
  #pragma message "XSTR(CFG_PLL3_P_DIVIDER)"
  #error PLL3P is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL3_P                    (CFG_PLL3_P_DIVIDER << CFG_RCC_PLLDIVR_PLL_P_POS)
#endif

#if (CFG_PLL3_Q_DIVIDER < MIN_DIVQ3) || (CFG_PLL3_Q_DIVIDER > MAX_DIVQ3)
  #pragma message "XSTR(CFG_PLL3_Q_DIVIDER)"
  #error PLL3Q is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL3_Q                    (CFG_PLL3_Q_DIVIDER << CFG_RCC_PLLDIVR_PLL_Q_POS)
#endif

#if (CFG_PLL3_R_DIVIDER < MIN_DIVR3) || (CFG_PLL3_R_DIVIDER > MAX_DIVR3)
  #pragma message "XSTR(CFG_PLL3_R_DIVIDER)"
  #error PLL3R is out of range
#else
  #define CFG_RCC_PLLDIVR_PLL3_R                    (CFG_PLL3_R_DIVIDER << CFG_RCC_PLLDIVR_PLL_R_POS)
#endif

#define CFG_RCC_PLL3_DIVR                           (CFG_RCC_PLLDIVR_PLL3_N |   \
                                                     CFG_RCC_PLLDIVR_PLL3_P |   \
                                                     CFG_RCC_PLLDIVR_PLL3_Q |   \
                                                     CFG_RCC_PLLDIVR_PLL3_R)

#define PLL3_CLOCK_FREQUENCY                        ((CFG_FREQ_PLL_SOURCE / CFG_PLL3_M_DIVIDER) * CFG_PLL3_N_MULTIPLIER)
#define PLL3P_CLOCK_FREQUENCY                       (PLL3_CLOCK_FREQUENCY / CFG_PLL3_P_DIVIDER)
#define PLL3Q_CLOCK_FREQUENCY                       (PLL3_CLOCK_FREQUENCY / CFG_PLL3_Q_DIVIDER)
#define PLL3R_CLOCK_FREQUENCY                       (PLL3_CLOCK_FREQUENCY / CFG_PLL3_R_DIVIDER)

/// -------------------------------------------------------------------------------------------------------------------------------
/// PLLCKSELR
///

#define CFG_RCC_PLLCKSELR                           (CFG_RCC_PLLCKSELR_PLL1_M | \
                                                     CFG_RCC_PLLCKSELR_PLL2_M | \
                                                     CFG_RCC_PLLCKSELR_PLL3_M | \
                                                     CFG_MUX_PLL_SOURCE )

/// -------------------------------------------------------------------------------------------------------------------------------
/// PLLCFGR
///

#define CFG_PLLCFGR_OUTPUT_ENABLE                   ((CFG_ENABLE_PLL1P << RCC_PLLCFGR_PLL1P_POS) | \
                                                     (CFG_ENABLE_PLL1Q << RCC_PLLCFGR_PLL1Q_POS) | \
                                                     (CFG_ENABLE_PLL1R << RCC_PLLCFGR_PLL1R_POS) | \
                                                     (CFG_ENABLE_PLL2P << RCC_PLLCFGR_PLL2P_POS) | \
                                                     (CFG_ENABLE_PLL2Q << RCC_PLLCFGR_PLL2Q_POS) | \
                                                     (CFG_ENABLE_PLL2R << RCC_PLLCFGR_PLL2R_POS) | \
                                                     (CFG_ENABLE_PLL3P << RCC_PLLCFGR_PLL3P_POS) | \
                                                     (CFG_ENABLE_PLL3Q << RCC_PLLCFGR_PLL3Q_POS) | \
                                                     (CFG_ENABLE_PLL3R << RCC_PLLCFGR_PLL3R_POS))


#define PLL1_INPUT_CLOCK_FREQUENCY                  (CFG_FREQ_PLL_SOURCE / CFG_PLL1_M_DIVIDER)
#define PLL2_INPUT_CLOCK_FREQUENCY                  (CFG_FREQ_PLL_SOURCE / CFG_PLL2_M_DIVIDER)
#define PLL3_INPUT_CLOCK_FREQUENCY                  (CFG_FREQ_PLL_SOURCE / CFG_PLL3_M_DIVIDER)

#if (PLL1_INPUT_CLOCK_FREQUENCY < VCOSEL_FREQUENCY)
  #define CFG_PLLCFGR_PLL1VCOSEL                    RCC_PLLCFGR_PLL1VCOSEL
#else
  #define CFG_PLLCFGR_PLL1VCOSEL                    0
#endif

#if (PLL2_INPUT_CLOCK_FREQUENCY < VCOSEL_FREQUENCY)
  #define CFG_PLLCFGR_PLL2VCOSEL                    RCC_PLLCFGR_PLL2VCOSEL
#else
  #define CFG_PLLCFGR_PLL2VCOSEL                    0
#endif

#if (PLL3_INPUT_CLOCK_FREQUENCY < VCOSEL_FREQUENCY)
  #define CFG_PLLCFGR_PLL3VCOSEL                    RCC_PLLCFGR_PLL3VCOSEL
#else
  #define CFG_PLLCFGR_PLL3VCOSEL                    0
#endif

#if   (PLL1_INPUT_CLOCK_FREQUENCY >= 1000000) && (PLL1_INPUT_CLOCK_FREQUENCY <= 2000000)
  #define CFG_PLLCFGR_PLL1RGE                       RCC_PLLCFGR_PLL1RGE_0
#elif (PLL1_INPUT_CLOCK_FREQUENCY >  2000000) && (PLL1_INPUT_CLOCK_FREQUENCY <= 4000000)
  #define CFG_PLLCFGR_PLL1RGE                       RCC_PLLCFGR_PLL1RGE_1
#elif (PLL1_INPUT_CLOCK_FREQUENCY >  4000000) && (PLL1_INPUT_CLOCK_FREQUENCY <= 8000000)
  #define CFG_PLLCFGR_PLL1RGE                       RCC_PLLCFGR_PLL1RGE_2
#elif (PLL1_INPUT_CLOCK_FREQUENCY >  8000000) && (PLL1_INPUT_CLOCK_FREQUENCY <= 1600000)
  #define CFG_PLLCFGR_PLL1RGE                       RCC_PLLCFGR_PLL1RGE_3
#else
 #error PLL1 input frequency outside of allowed range!
#endif

#if   (PLL2_INPUT_CLOCK_FREQUENCY >= 1000000) && (PLL2_INPUT_CLOCK_FREQUENCY <= 2000000)
  #define CFG_PLLCFGR_PLL2RGE                       RCC_PLLCFGR_PLL2RGE_0
#elif (PLL2_INPUT_CLOCK_FREQUENCY >  2000000) && (PLL2_INPUT_CLOCK_FREQUENCY <= 4000000)
  #define CFG_PLLCFGR_PLL2RGE                       RCC_PLLCFGR_PLL2RGE_1
#elif (PLL2_INPUT_CLOCK_FREQUENCY >  4000000) && (PLL2_INPUT_CLOCK_FREQUENCY <= 8000000)
  #define CFG_PLLCFGR_PLL2RGE                       RCC_PLLCFGR_PLL2RGE_2
#elif (PLL2_INPUT_CLOCK_FREQUENCY >  8000000) && (PLL2_INPUT_CLOCK_FREQUENCY <= 16000000)
  #define CFG_PLLCFGR_PLL2RGE                       RCC_PLLCFGR_PLL2RGE_3
#else
 #error PLL2 input frequency outside of allowed range!
#endif

#if   (PLL3_INPUT_CLOCK_FREQUENCY >= 1000000) && (PLL3_INPUT_CLOCK_FREQUENCY <= 2000000)
  #define CFG_PLLCFGR_PLL3RGE                       RCC_PLLCFGR_PLL3RGE_0
#elif (PLL3_INPUT_CLOCK_FREQUENCY >  2000000) && (PLL3_INPUT_CLOCK_FREQUENCY <= 4000000)
  #define CFG_PLLCFGR_PLL3RGE                       RCC_PLLCFGR_PLL3RGE_1
#elif (PLL3_INPUT_CLOCK_FREQUENCY >  4000000) && (PLL3_INPUT_CLOCK_FREQUENCY <= 8000000)
  #define CFG_PLLCFGR_PLL3RGE                       RCC_PLLCFGR_PLL3RGE_2
#elif (PLL3_INPUT_CLOCK_FREQUENCY >  8000000) && (PLL3_INPUT_CLOCK_FREQUENCY <= 16000000)
  #define CFG_PLLCFGR_PLL3RGE                       RCC_PLLCFGR_PLL3RGE_3
#else
 #error PLL3 input frequency outside of allowed range!
#endif

#if (CFG_PLL1_FRACTIONAL_VALUE == 0)
  #define CFG_PLLCFGR_PLL1FRAEN                     0
#else
  #define CFG_PLLCFGR_PLL1FRAEN                     RCC_PLLCFGR_PLL1FRAEN
#endif

#if (CFG_PLL2_FRACTIONAL_VALUE == 0)
  #define CFG_PLLCFGR_PLL2FRAEN                     0
#else
  #define CFG_PLLCFGR_PLL2FRAEN                     RCC_PLLCFGR_PLL2FRAEN
#endif

#if (CFG_PLL3_FRACTIONAL_VALUE == 0)
  #define CFG_PLLCFGR_PLL3FRAEN                     0
#else
  #define CFG_PLLCFGR_PLL3FRAEN                     RCC_PLLCFGR_PLL3FRAEN
#endif

#define CFG_RCC_PLLCFGR                             (CFG_PLLCFGR_OUTPUT_ENABLE  | \
                                                     CFG_PLLCFGR_PLL1VCOSEL     | \
                                                     CFG_PLLCFGR_PLL2VCOSEL     | \
                                                     CFG_PLLCFGR_PLL3VCOSEL     | \
                                                     CFG_PLLCFGR_PLL1RGE        | \
                                                     CFG_PLLCFGR_PLL2RGE        | \
                                                     CFG_PLLCFGR_PLL3RGE        | \
                                                     CFG_PLLCFGR_PLL1FRAEN      | \
                                                     CFG_PLLCFGR_PLL2FRAEN      | \
                                                     CFG_PLLCFGR_PLL3FRAEN)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_CFGR
///

/// Missing RTEPRE

#define CFG_RCC_CFGR                                (CFG_MCO1_SOURCE_MUX        | \
                                                     CFG_MCO2_SOURCE_MUX        | \
                                                     CFG_HRTIM_SOURCE_MUX)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_D1CCIPR - RCC domain 1 kernel clock configuration register
///
/// missing DSI

#if defined(QUADSPI)
  #define CFG_RCC_D1CCIPR_CPU_DEPENDANCY            CFG_QSPI_SOURCE_MUX
#endif

#if defined(OCTOSPI1) || defined(OCTOSPI2)
  #define CFG_RCC_D1CCIPR_CPU_DEPENDANCY            CFG_OSPI_SOURCE_MUX
#endif

#define CFG_RCC_D1CCIPR                             (CFG_PER_SOURCE_MUX         | \
                                                     CFG_SDMMC_SOURCE_MUX       | \
                                                     CFG_FMC_SOURCE_MUX         | \
                                                     CFG_RCC_D1CCIPR_CPU_DEPENDANCY)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_D2CCIP1R - RCC domain 2 kernel clock configuration register
///

#define CFG_RCC_D2CCIP1R                            (CFG_SWP_SOURCE_MUX         | \
                                                     CFG_FDCAN_SOURCE_MUX       | \
                                                     CFG_DFSDM_SOURCE_MUX       | \
                                                     CFG_SPDIF_SOURCE_MUX       | \
                                                     CFG_SPI45_SOURCE_MUX       | \
                                                     CFG_SPI123_SOURCE_MUX      | \
                                                     CFG_SAI23_SOURCE_MUX       | \
                                                     CFG_SAI1_SOURCE_MUX)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_D2CCIP2R - RCC domain 2 kernel clock configuration register
///

#if defined(RCC_D2CCIP2R_USART16SEL)
  #define CFG_RCC_D2CCIP2R_CPU_DEPENDANCY_1         CFG_UART16_SOURCE_MUX
#else
  #define CFG_RCC_D2CCIP2R_CPU_DEPENDANCY_1         CFG_UART16910_SOURCE_MUX
#endif

#if defined(RCC_D2CCIP2R_I2C123SEL)
  #define CFG_RCC_D2CCIP2R_CPU_DEPENDANCY_2         CFG_I2C123_SOURCE_MUX
#else
  #define CFG_RCC_D2CCIP2R_CPU_DEPENDANCY_2         CFG_I2C1235_SOURCE_MUX
#endif

#define CFG_RCC_D2CCIP2R                            (CFG_LPTIM1_SOURCE_MUX              | \
                                                     CFG_CEC_SOURCE_MUX                 | \
                                                     CFG_USB_SOURCE_MUX                 | \
                                                     CFG_RNG_SOURCE_MUX                 | \
                                                     CFG_UART234578_SOURCE_MUX          | \
                                                     CFG_RCC_D2CCIP2R_CPU_DEPENDANCY_1  | \
                                                     CFG_RCC_D2CCIP2R_CPU_DEPENDANCY_2)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_D3CCIPR - RCC domain 3 kernel clock configuration register
///

#define CFG_RCC_D3CCIPR                             (CFG_SPI6_SOURCE_MUX        | \
                                                     CFG_SAI4B_SOURCE_MUX       | \
                                                     CFG_SAI4A_SOURCE_MUX       | \
                                                     CFG_ADC_SOURCE_MUX         | \
                                                     CFG_LPTIM345_SOURCE_MUX    | \
                                                     CFG_LPTIM2_SOURCE_MUX      | \
                                                     CFG_I2C4_SOURCE_MUX        | \
                                                     CFG_LPUART1_SOURCE_MUX)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_D1CFGR - RCC domain 1 clock configuration register
///

#define CFG_RCC_D1CFGR                              (CFG_SYSCLK_DIVIDER         | \
                                                     CFG_APB3_DIVIDER         | \
                                                     CFG_HCLK_DIVIDER)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_D2CFGR - RCC domain 2 clock configuration register
///

#define CFG_RCC_D2CFGR                              (CFG_APB2_DIVIDER        | \
                                                     CFG_APB1_DIVIDER)

/// -------------------------------------------------------------------------------------------------------------------------------
/// RCC_D3CFGR - RCC domain 3 clock configuration register
///

#define CFG_RCC_D3CFGR                              (CFG_APB4_DIVIDER)

/// -------------------------------------------------------------------------------------------------------------------------------
/// SYS Clock Mux
///

#if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL1)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              SYS_PLL_CLK_FREQUENCY
#elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSI)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              HSI_CLOCK_FREQUENCY
#elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_CSI)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_CSI_VALUE
#else //(CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSE)
  #define SYS_CPU_CORE_CLOCK_FREQUENCY              CFG_HSE_VALUE
#endif


#if   CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV1
  #define SYS_HCLK_CLOCK_FREQUENCY                  SYS_CPU_CORE_CLOCK_FREQUENCY
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV2
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 2)
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV4
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 4)
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV8
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 8)
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV16
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 16)
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV64
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 64)
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV128
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 128)
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV256
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 256)
#elif CFG_SYSCLK_DIVIDER == CFG_SYSCLK_DIV512
  #define SYS_HCLK_CLOCK_FREQUENCY                  (SYS_CPU_CORE_CLOCK_FREQUENCY / 512)
#endif


#if   CFG_HCLK_DIVIDER == CFG_HCLK_DIV1
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     SYS_HCLK_CLOCK_FREQUENCY
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV2
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 2)
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV4
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 4)
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV8
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 8)
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV16
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 16)
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV64
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 64)
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV128
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 128)
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV256
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 256)
#elif CFG_HCLK_DIVIDER == CFG_HCLK_DIV512
    #define SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY     (SYS_HCLK_CLOCK_FREQUENCY / 512)
#endif

#define AXI_CLOCK_FREQUENCY                         SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY
#define SYS_HCLK_CPU2_CLOCK_FREQUENCY               SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY             // If second core exist

#if   CFG_APB1_DIVIDER == CFG_APB1_DIV1
  #define PCLK1_CLOCK_FREQUENCY                     SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY
#elif CFG_APB1_DIVIDER == CFG_APB1_DIV2
  #define PCLK1_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 2)
#elif CFG_APB1_DIVIDER == CFG_APB1_DIV4
  #define PCLK1_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 4)
#elif CFG_APB1_DIVIDER == CFG_APB1_DIV8
  #define PCLK1_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 8)
#elif CFG_APB1_DIVIDER == CFG_APB1_DIV16
  #define PCLK1_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 16)
#endif

#if   CFG_APB2_DIVIDER == CFG_APB2_DIV1
  #define PCLK2_CLOCK_FREQUENCY                     SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY
#elif CFG_APB2_DIVIDER == CFG_APB2_DIV2
  #define PCLK2_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 2)
#elif CFG_APB2_DIVIDER == CFG_APB2_DIV4
  #define PCLK2_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 4)
#elif CFG_APB2_DIVIDER == CFG_APB2_DIV8
  #define PCLK2_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 8)
#elif CFG_APB2_DIVIDER == CFG_APB2_DIV16
  #define PCLK2_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 16)
#endif

#if   CFG_APB3_DIVIDER == CFG_APB3_DIV1
  #define PCLK3_CLOCK_FREQUENCY                     SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY
#elif CFG_APB3_DIVIDER == CFG_APB3_DIV2
  #define PCLK3_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 2)
#elif CFG_APB3_DIVIDER == CFG_APB3_DIV4
  #define PCLK3_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 4)
#elif CFG_APB3_DIVIDER == CFG_APB3_DIV8
  #define PCLK3_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 8)
#elif CFG_APB3_DIVIDER == CFG_APB3_DIV16
  #define PCLK3_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 16)
#endif

#if   CFG_APB4_DIVIDER == CFG_APB4_DIV1
  #define PCLK4_CLOCK_FREQUENCY                     SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY
#elif CFG_APB4_DIVIDER == CFG_APB4_DIV2
  #define PCLK4_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 2)
#elif CFG_APB4_DIVIDER == CFG_APB4_DIV4
  #define PCLK4_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 4)
#elif CFG_APB4_DIVIDER == CFG_APB4_DIV8
  #define PCLK4_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 8)
#elif CFG_APB4_DIVIDER == CFG_APB4_DIV16
  #define PCLK4_CLOCK_FREQUENCY                     (SYS_PERIPHERAL_MAIN_CLOCK_FREQUENCY / 16)
#endif

#define SYSTEM_CORE_CLOCK                           SYS_HCLK_CLOCK_FREQUENCY

/// -------------------------------------------------------------------------------------------------------------------------------
/// All Peripheral frequency ( TODO need to add all module clock here. )


// CFG_PER_SOURCE_MUX                          CFG_RCC_D1CCIPR_PER_HSI_KER
// CFG_ADC_SOURCE_MUX                          CFG_RCC_D3CCIPR_ADC_PLL2P
// CFG_CEC_SOURCE_MUX                          CFG_RCC_D2CCIP2R_CEC_LSE
// CFG_DFSDM_SOURCE_MUX                        CFG_RCC_D2CCIP1R_DFSDM_PLCK2
// CFG_FDCAN_SOURCE_MUX                        CFG_RCC_D2CCIP1R_FDCAN_HSE
// CFG_FMC_SOURCE_MUX                          CFG_RCC_D1CCIPR_FMC_PLL1Q
// CFG_I2C123_SOURCE_MUX                       CFG_RCC_D2CCIP2R_I2C123_PCLK1
// CFG_I2C4_SOURCE_MUX                         CFG_RCC_D3CCIPR_I2C4_PCLK4
// CFG_HRTIM_SOURCE_MUX                        CFG_RCC_CFGR_HRTIM_TIM_CLK
// CFG_LPTIM1_SOURCE_MUX                       CFG_RCC_D2CCIP2R_LPTIM1_PCLK1
// CFG_LPTIM2_SOURCE_MUX                       CFG_RCC_D3CCIPR_LPTIM2_PCLK4
// CFG_LPTIM345_SOURCE_MUX                     CFG_RCC_D3CCIPR_LPTIM345_PCLK4
// CFG_LPUART1_SOURCE_MUX                      CFG_RCC_D3CCIPR_LPUART1_PCLK3
// CFG_MCO1_SOURCE_MUX                         CFG_RCC_CFGR_MCO1_SYS_CLK
// CFG_MCO2_SOURCE_MUX                         CFG_RCC_CFGR_MCO2_SYS_CLK
// CFG_QSPI_SOURCE_MUX                         CFG_RCC_D1CCIPR_QSPI_PLL1Q           // for 745 only
// CFG_OSPI_SOURCE_MUX                         CFG_RCC_D1CCIPR_OCTOSPI_PLL1Q        // for 735 only
// CFG_RNG_SOURCE_MUX                          CFG_RCC_D2CCIP2R_RNG_HSI48
// CFG_RTC_SOURCE_MUX                          CFG_RCC_BDCR_RTC_LSE
// CFG_SAI1_SOURCE_MUX                         CFG_RCC_D2CCIP1R_SAI1_PLL1Q
// CFG_SAI23_SOURCE_MUX                        CFG_RCC_D2CCIP1R_SAI23_PLL1Q
// CFG_SAI4A_SOURCE_MUX                        CFG_RCC_D3CCIPR_SAI4A_PLL1Q
// CFG_SAI4B_SOURCE_MUX                        CFG_RCC_D3CCIPR_SAI4B_PLL1Q
// CFG_SDMMC_SOURCE_MUX                        CFG_RCC_D1CCIPR_SDMMC_PLL1Q
// CFG_SPDIF_SOURCE_MUX                        CFG_RCC_D2CCIP1R_SPDIF_PLL1Q
// CFG_SPI123_SOURCE_MUX                       CFG_RCC_D2CCIP1R_SPI123_PLL1Q
// CFG_SPI45_SOURCE_MUX                        CFG_RCC_D2CCIP1R_SPI45_PLCK2
// CFG_SPI6_SOURCE_MUX                         CFG_RCC_D3CCIPR_SPI6_PLCK4
// CFG_SWP_SOURCE_MUX                          CFG_RCC_D2CCIP1R_SWP_PCLK1

/// USART 1,6 clock frequency
#if   (CFG_UART16_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART16_PCLK2)
  #define USART16_CLOCK_FREQUENCY           PCLK2_CLOCK_FREQUENCY
#elif (CFG_UART16_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART16_PLL2Q)
  #define USART16_CLOCK_FREQUENCY           PLL2Q_CLOCK_FREQUENCY
#elif (CFG_UART16_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART16_PLL3Q)
  #define USART16_CLOCK_FREQUENCY           PLL3Q_CLOCK_FREQUENCY
#elif (CFG_UART16_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART16_HSI)
  #define USART16_CLOCK_FREQUENCY           HSI_CLOCK_FREQUENCY
#elif (CFG_UART16_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART16_CSI)
  #define USART16_CLOCK_FREQUENCY           CSI_CLOCK_FREQUENCY
#elif (CFG_UART16_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART16_LSE)
  #define USART16_CLOCK_FREQUENCY           LSE_CLOCK_FREQUENCY
#endif

/// USART 2,3,4,5,7,8 clock frequency
#if   (CFG_UART234578_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART234578_PCLK1)
  #define USART234578_CLOCK_FREQUENCY           PCLK1_CLOCK_FREQUENCY
#elif (CFG_UART234578_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART234578_PLL2Q)
  #define USART234578_CLOCK_FREQUENCY           PLL2Q_CLOCK_FREQUENCY
#elif (CFG_UART234578_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART234578_PLL3Q)
  #define USART234578_CLOCK_FREQUENCY           PLL3Q_CLOCK_FREQUENCY
#elif (CFG_UART234578_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART234578_HSI)
  #define USART234578_CLOCK_FREQUENCY           HSI_CLOCK_FREQUENCY
#elif (CFG_UART234578_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART234578_CSI)
  #define USART234578_CLOCK_FREQUENCY           CSI_CLOCK_FREQUENCY
#elif (CFG_UART234578_SOURCE_MUX == CFG_RCC_D2CCIP2R_USART234578_LSE)
  #define USART234578_CLOCK_FREQUENCY           LSE_CLOCK_FREQUENCY
#endif

// CFG_USB_SOURCE_MUX                          CFG_RCC_D2CCIP2R_USB_PLL3Q

// --------------------------------------------------------------------------------------------------------------------------------
/*
// Power scaling according to speed
#if (SYS_CPU_CORE_CLOCK_FREQUENCY <= 70000000)
//#define CFG_POWER_REGULATOR                         PWR_CR1_VOS_0   // Voltage scale 3
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 140000000)
//#define CFG_POWER_REGULATOR                         PWR_CR1_VOS_1   // Voltage scale 2
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY <= 216000000)
#define CFG_POWER_REGULATOR                         PWR_CR1_VOS     // Voltage scale 1
#endif
*/
// Flash Latency configuration for Voltage from 2.7V to 3.6V
#if   (SYS_CPU_CORE_CLOCK_FREQUENCY < 80000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_0WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY < 160000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_1WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY < 240000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_2WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY < 320000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_3WS
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY < 480000000)
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_4WS
#else                                                                                                           // todo need to add WS for 735 up to 550MHz..
#define CFG_FLASH_LATENCY                           FLASH_ACR_LATENCY_5WS
#endif

// --------------------------------------------------------------------------------------------------------------------------------
// see page 3151 of the PDF: rm0399-stm32h745755-and-stm32h747757-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
// This configuration is for MDIO with a Max frequency of 2.5 MHz (specified in the IEEE 802.3)
//
#if   (SYS_CPU_CORE_CLOCK_FREQUENCY > 155000000)
  #define ETH_MACMDIOAR_CR_DIVIDER                  ETH_MACMIIAR_CR_Div102
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY > 105000000)
  #define ETH_MACMDIOAR_CR_DIVIDER                  ETH_MACMIIAR_CR_Div62
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY > 65000000)
  #define ETH_MACMDIOAR_CR_DIVIDER                  ETH_MACMIIAR_CR_Div42
#elif (SYS_CPU_CORE_CLOCK_FREQUENCY > 40000000)
  #define ETH_MACMDIOAR_CR_DIVIDER                  ETH_MACMIIAR_CR_Div26
#else
  #define ETH_MACMDIOAR_CR_DIVIDER                  ETH_MACMIIAR_CR_Div16
#endif

// Verification
#if SYS_HCLK_CLOCK_FREQUENCY > MAX_SYS_HCLK
 #pragma message "XSTR(SYS_HCLK_CLOCK_FREQUENCY)"
 #error CPU Core frequency exceed maximum allowed!
#endif

#if SYS_AXI_CLOCK_FREQUENCY > MAX_SYS_AXI
 #pragma message "XSTR(SYS_APB1_CLOCK_FREQUENCY)"
 #error APB1 frequency exceed maximum allowed!
#endif


#if PCLK1_CLOCK_FREQUENCY > MAX_PCLK1
 #pragma message XSTR(PCLK1_CLOCK_FREQUENCY)
 #error PCLK1 frequency exceed maximum allowed!
#endif

#if PCLK2_CLOCK_FREQUENCY > MAX_PCLK2
 #pragma message "XSTR(PCLK2_CLOCK_FREQUENCY)"
 #error PLCK2 frequency exceed maximum allowed!
#endif

#if PCLK3_CLOCK_FREQUENCY > MAX_PCLK3
 #pragma message "XSTR(PCKL3_CLOCK_FREQUENCY)"
 #error PLCK3 frequency exceed maximum allowed!
#endif

#if PCLK4_CLOCK_FREQUENCY > MAX_PCLK4
 #pragma message "XSTR(PLCK4_CLOCK_FREQUENCY)"
 #error PCLK4 frequency exceed maximum allowed!
#endif

#if AHB4_CLOCK_FREQUENCY > MAX_AHB4
 #pragma message "XSTR(AHB4_CLOCK_FREQUENCY)"
 #error PCLK4 frequency exceed maximum allowed!
#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SystemInit              (void);

//-------------------------------------------------------------------------------------------------

#define CFG_RTC_SOURCE_MUX                          CFG_RCC_BDCR_RTC_LSE


// Define the MUX for module that you will need.        See lib_STM32H7_system_clock for clock selection option.

//#define CFG_TRACE_SOURCE_MUX                        HSI //???

