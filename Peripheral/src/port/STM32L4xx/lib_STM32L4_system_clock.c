//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32L4_system_clock.c
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

uint32_t SystemCoreClock = SYSTEM_CORE_CLOCK;

static void WaitForClockRegister(volatile uint32_t *Register, uint32_t Mask, uint32_t Value)
{
  uint32_t Retry = CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY;

  while((*Register & Mask) != Value)
  {
    if(Retry == 0U)
    {
      while(1) { __NOP(); }
    }
    Retry--;
  }
}

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
//                  Clock readiness timeout halts initialization instead of running with a wrong clock.
//                  Changing an existing RTC source requires an explicit backup-domain reset by the caller.
//
//-------------------------------------------------------------------------------------------------
void SystemInit(void)
{
    __asm volatile("cpsid i");                                                          // Disable IRQ

    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    (void)READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN);
    MODIFY_REG(PWR->CR1, PWR_CR1_VOS, POWER_REGULATOR_CFG);
    WaitForClockRegister(&PWR->SR2, PWR_SR2_VOSF, 0U);

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SET_BIT(SCB->CPACR, ((3UL << 10 * 2) | (3UL << 11 * 2)));   // Set CP10 and CP11 Full Access
  #endif

    SET_BIT(RCC->CR, RCC_CR_HSION);
    WaitForClockRegister(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);
    WaitForClockRegister(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_HSI);
    CLEAR_BIT(RCC->CR, RCC_CR_CSSON | RCC_CR_PLLON);
    WaitForClockRegister(&RCC->CR, RCC_CR_PLLRDY, 0U);

    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, CFG_FLASH_LATENCY);
    WaitForClockRegister(&FLASH->ACR, FLASH_ACR_LATENCY, CFG_FLASH_LATENCY);

    MODIFY_REG(RCC->CFGR,
               RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2 | RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE,
               CFG_SYS_HCLK | CFG_SYS_APB1 | CFG_SYS_APB2 | CFG_MCO_SOURCE_MUX | CFG_MCO_CLK_DIVIDER);

  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_MSI) || \
      (CFG_USE_PLL && (CFG_RCC_PLLCFGR_PLLSRC == CFG_RCC_PLLCFGR_PLLSRC_MSI)) || \
      (CFG_MCO_SOURCE_MUX == CFG_RCC_CFGR_MCOSEL_MSI)
    SET_BIT(RCC->CR, RCC_CR_MSION);
    WaitForClockRegister(&RCC->CR, RCC_CR_MSIRDY, RCC_CR_MSIRDY);
    MODIFY_REG(RCC->CR, RCC_CR_MSIRANGE | RCC_CR_MSIRGSEL,
               RCC_CR_MSIRANGE_1 | RCC_CR_MSIRANGE_2 | RCC_CR_MSIRGSEL);
    WaitForClockRegister(&RCC->CR, RCC_CR_MSIRDY, RCC_CR_MSIRDY);
  #endif

  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSE) || \
      (CFG_USE_PLL && (CFG_RCC_PLLCFGR_PLLSRC == CFG_RCC_PLLCFGR_PLLSRC_HSE)) || \
      (CFG_RTC_CLOCK_SOURCE == CFG_RCC_BDCR_RTCSEL_HSE) || \
      (CFG_MCO_SOURCE_MUX == CFG_RCC_CFGR_MCOSEL_HSE)
    if(READ_BIT(RCC->CR, RCC_CR_HSEON) == 0U)
    {
        CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);
    }
    SET_BIT(RCC->CR, RCC_CR_HSEON);
    WaitForClockRegister(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY);
  #endif

  #if CFG_USE_PLL
    RCC->PLLCFGR = CFG_RCC_PLLCFGR_CFG;
    SET_BIT(RCC->CR, RCC_CR_PLLON);
    WaitForClockRegister(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY);
  #endif

  #if (CFG_USB_CLK48_SOURCE == CFG_RCC_CCIPR_CLK48SEL_PLLQ)
    #if (CFG_ENABLE_PLL_Q == 0) || (CFG_PLL_Q_FREQUENCY != 48000000ULL)
      #error "USB PLLQ must be enabled and configured to 48 MHz in clock_cfg.h"
    #endif
  #elif (CFG_USB_CLK48_SOURCE == CFG_RCC_CCIPR_CLK48SEL_HSI48)
    SET_BIT(RCC->CRRCR, RCC_CRRCR_HSI48ON);
    WaitForClockRegister(&RCC->CRRCR, RCC_CRRCR_HSI48RDY, RCC_CRRCR_HSI48RDY);
  #else
    #error "Unsupported USB 48 MHz source in clock_cfg.h"
  #endif
    MODIFY_REG(RCC->CCIPR, RCC_CCIPR_CLK48SEL, CFG_USB_CLK48_SOURCE);

    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, CFG_SYS_CLOCK_MUX);
    WaitForClockRegister(&RCC->CFGR, RCC_CFGR_SWS, CFG_SYS_CLOCK_MUX << RCC_CFGR_SWS_Pos);
    SystemCoreClock = SYSTEM_CORE_CLOCK;

  #if (CFG_MCO_SOURCE_MUX == CFG_RCC_CFGR_MCOSEL_LSI) || (CFG_RTC_CLOCK_SOURCE == CFG_RCC_BDCR_RTCSEL_LSI)
    SET_BIT(RCC->CSR, RCC_CSR_LSION);
    WaitForClockRegister(&RCC->CSR, RCC_CSR_LSIRDY, RCC_CSR_LSIRDY);
  #endif

  #if (CFG_MCO_SOURCE_MUX == CFG_RCC_CFGR_MCOSEL_HSI48)
    SET_BIT(RCC->CRRCR, RCC_CRRCR_HSI48ON);
    WaitForClockRegister(&RCC->CRRCR, RCC_CRRCR_HSI48RDY, RCC_CRRCR_HSI48RDY);
  #endif

//--------------------------------------------------------------------------
//  RTC init clock
    
  #if (CFG_RTC_CLOCK_SOURCE != CFG_RCC_BDCR_RTCSEL_NO_CLOCK) || (CFG_MCO_SOURCE_MUX == CFG_RCC_CFGR_MCOSEL_LSE)
    SET_BIT(PWR->CR1, PWR_CR1_DBP);
    WaitForClockRegister(&PWR->CR1, PWR_CR1_DBP, PWR_CR1_DBP);

   #if (CFG_RTC_CLOCK_SOURCE == CFG_RCC_BDCR_RTCSEL_LSE) || (CFG_MCO_SOURCE_MUX == CFG_RCC_CFGR_MCOSEL_LSE)
    SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
    WaitForClockRegister(&RCC->BDCR, RCC_BDCR_LSERDY, RCC_BDCR_LSERDY);
   #endif

   #if (CFG_RTC_CLOCK_SOURCE != CFG_RCC_BDCR_RTCSEL_NO_CLOCK)
    MODIFY_REG(RCC->BDCR, RCC_BDCR_RTCSEL, CFG_RTC_CLOCK_SOURCE);
    WaitForClockRegister(&RCC->BDCR, RCC_BDCR_RTCSEL, CFG_RTC_CLOCK_SOURCE);
    SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
   #endif
    CLEAR_BIT(PWR->CR1, PWR_CR1_DBP);
  #endif

//--------------------------------------------------------------------------

    RCC->CIER = 0;
    RCC->CICR = RCC_CICR_LSIRDYC | RCC_CICR_LSERDYC | RCC_CICR_MSIRDYC |
          RCC_CICR_HSIRDYC | RCC_CICR_HSERDYC | RCC_CICR_PLLRDYC |
          RCC_CICR_CSSC | RCC_CICR_HSI48RDYC;

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
