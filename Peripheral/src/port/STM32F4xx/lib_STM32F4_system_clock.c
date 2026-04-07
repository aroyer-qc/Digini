//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_system_clock.c
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define SYSTEM_GLOBAL
#include "./Peripheral/inc/port/lib_system_clock.h"
#undef  SYSTEM_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Vector Table base offset field. This value must be a multiple of 0x200.
#define VECT_TAB_OFFSET     0x00

//-------------------------------------------------------------------------------------------------
// Variables(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           SystemInit
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Reset the CPU Setting
//                  Configures : Vector table relocation
//                               System clock source
//                               PLL Multiplier and Divider factors
//                               AHB/APBx prescalers and Flash settings
//
//  Note(s):        This function should be called only once the RCC clock configuration is reset
//                  to the default reset state (done in BSP_SystemReset function).
//
//-------------------------------------------------------------------------------------------------
void SystemInit(void)
{
  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)
    uint32_t Retry;
  #endif

    __asm volatile("cpsid i");                                                          // Disable IRQ

    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    MODIFY_REG(PWR->CR, PWR_CR_VOS, POWER_REGULATOR_CFG);

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SET_BIT(SCB->CPACR, ((3UL << 10 * 2) | (3UL << 11 * 2)));   // Set CP10 and CP11 Full Access
  #endif

    // Reset the RCC clock configuration to the default reset state ------------
    SET_BIT(RCC->CR, RCC_CR_HSION);                                                     // Set HSION bit
	RCC->CFGR = (CFG_SYS_HCLK         |
                 CFG_SYS_APB1         |
                 CFG_SYS_APB2         |
                 CFG_MCO1_SOURCE_MUX  |
                 CFG_MCO2_SOURCE_MUX  |
                 CFG_MCO1_CLK_DIVIDER |
                 CFG_MCO2_CLK_DIVIDER);                                                 // Set CFGR register
    CLEAR_BIT(RCC->CR, (RCC_CR_CSSON | RCC_CR_PLLON | RCC_CR_HSEBYP));                  // Reset HSEBYP, CSSON and PLLON bits

  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

   #if (CFG_PLL_SOURCE == CFG_HSE_VALUE)
    SET_BIT(RCC->CR, RCC_CR_HSEON);

    Retry = 0;
    while((READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0) && (Retry < CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY))
    {
        Retry++;
    };

   #else
    while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0) {};                                    // Wait for HSI to be ready B4 enabling PLL
   #endif

    RCC->PLLCFGR = CFG_RCC_PLLCFGR_CFG;                                                 // Set PLLCFGR register

   #if (CFG_PLL_SOURCE == CFG_HSE_VALUE)
    CLEAR_BIT(RCC->CR, RCC_CR_HSION);                                                   // Reset HSION bit to reduce consumption
   #endif

    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, CFG_FLASH_LATENCY);                       // Set flash latency
    SET_BIT(RCC->CR, RCC_CR_PLLON);                                                     // Enable PLL
    while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0) {};                                    // Wait for PLL to be ready B4 enabling PLL
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL);                                                // Switch to PLL

  #endif // (SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

   //------------ Dedicated Clocks Configuration Register 1 -------------------

  #if defined(STM32F427xx) || defined(STM32F429xx) || defined(STM32F437xx) || defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx)
    uint32_t DCKCFGR_Value = (
    CFG_SAI2_SOURCE_MUX   |
    CFG_SAI1_SOURCE_MUX   |
    CFG_PLLSAI_DIV_R      |
    CFG_PLLSAI_DIV_Q      |
    CFG_PLLI2S_DIV_Q);
    SET_BIT(RCC->DCKCFGR, DCKCFGR_Value);
  #endif

//--------------------------------------------------------------------------

  #if defined(STM32F427xx) || defined(STM32F429xx) || defined(STM32F437xx) || defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx)
   #ifdef CFG_ENABLE_PLLSAI
    RCC->PLLSAICFGR  = (CFG_PLLSAI_N_MULTIPLIER << RCC_PLLSAICFGR_PLLSAIN_Pos) |
                       (CFG_PLLSAI_Q_DIVIDER    << RCC_PLLSAICFGR_PLLSAIQ_Pos) |
                       (CFG_PLLSAI_R_DIVIDER    << RCC_PLLSAICFGR_PLLSAIR_Pos);


    // TODO need to use the enable for this
    RCC->CR |= RCC_CR_PLLSAION;
    while((RCC->CR & RCC_CR_PLLSAIRDY) == 0);
   #endif
  #endif

//--------------------------------------------------------------------------

  #ifdef CFG_ENABLE_PLLI2S
    RCC->PLLI2SCFGR  = (CFG_PLLI2S_N_MULTIPLIER << RCC_PLLI2SCFGR_PLLI2SN_Pos) |
   #if defined(STM32F427xx) || defined(STM32F429xx) || defined(STM32F437xx) || defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx)
                       (CFG_PLLI2S_Q_DIVIDER    << RCC_PLLI2SCFGR_PLLI2SQ_Pos) |
   #endif
                       (CFG_PLLI2S_R_DIVIDER    << RCC_PLLI2SCFGR_PLLI2SR_Pos);

    // TODO need to use the enable for this
    RCC->CR |= RCC_CR_PLLI2SON;
    while((RCC->CR & RCC_CR_PLLI2SRDY) == 0);
  #endif

//--------------------------------------------------------------------------
//  RTC init clock
    
  #if (CFG_RTC_CLOCK_SOURCE != CFG_RCC_BDCR_RTCSEL_NO_CLOCK)
   #if (CFG_RTC_CLOCK_SOURCE == CFG_RCC_BDCR_RTCSEL_HSE)
    MODIFY_REG(RCC->CFGR, RCC_CFGR_RTCPRE, CFG_RCC_CFGR_RTCPRE);
   #endif
  
   #if (CFG_RTC_CLOCK_SOURCE == CFG_RCC_BDCR_RTCSEL_LSE)
    SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);                                                 // External 32.768 KHz oscillator ON
    while((RCC->BDCR & RCC_BDCR_LSERDY) == 0);
   #endif

   #if (CFG_RTC_CLOCK_SOURCE == CFG_RCC_BDCR_RTCSEL_LSI)
    SET_BIT(RCC->CSR, RCC_CSR_LSION;                                                    // Internal 32 KHz oscillator ON
    while((RCC->CSR & RCC_CSR_LSIRDY) == 0);
   #endif

    MODIFY_REG(RCC->BDCR, RCC_BDCR_RTCSEL, CFG_RTC_CLOCK_SOURCE);
    SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
  #endif 

//--------------------------------------------------------------------------

    RCC->CIR = 0;                                                                       // Disable all interrupts

//--------------------------------------------------------------------------

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;                                     // Enable DWT
    DWT->CYCCNT = 0;                                                                    // Reset cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                                                // Start cycle counter

//--------------------------------------------------------------------------

    // Configure the Vector Table location add offset address ------------------
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;                                            // Vector Table Relocation in Internal SRAM
  #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;                                           // Vector Table Relocation in Internal FLASH
  #endif
}

//-------------------------------------------------------------------------------------------------
