//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F7_system_clock.c
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define SYSTEM_GLOBAL
#include "./Peripheral/inc/port/lib_system_clock.h"
#undef  SYSTEM_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define RCC_PLLCFGR_RESET_VALUE             0x24003010


// Vector Table base offset field. This value must be a multiple of 0x200.
#define VECT_TAB_OFFSET  0x00

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
    __asm volatile("cpsid i");                                              // Disable IRQ

    // Reset the RCC clock configuration to the default reset state
    RCC->CR |= RCC_CR_HSION;                                                // Set HSION bit
    RCC->CFGR = 0;                                                          // Reset CFGR register
    RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);               // Reset HSEON, CSSON and PLLON bits
    RCC->PLLCFGR = RCC_PLLCFGR_RESET_VALUE;                                 // Reset PLLCFGR register
    RCC->CR &= ~RCC_CR_HSEBYP;                                              // Reset HSEBYP bit
    RCC->CIR = 0;                                                           // Disable all interrupts

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));                      // Set CP10 and CP11 Full Access
  #endif

    //----------------------------- HSE Configuration --------------------------

  #if (CFG_SYS_PLL_MUX == CFG_RCC_PLLCFGR_PLLSRC_HSE) || (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSE)
    RCC->CR|=  RCC_CR_HSEON;                                                // Enable the External High Speed oscillator (HSE).
    while((RCC->CR & RCC_CR_HSERDY) == 0);                                  // Wait till HSE is ready
  #endif

    //-------------------------------- Power Control ---------------------------

    // Enable Power Control clock
    RCC->APB1ENR = RCC_APB1ENR_PWREN;

    // Activate the OverDrive to reach the 216 Mhz Frequency
    PWR->CR1 |= PWR_CR1_ODEN;
    while((PWR->CSR1 & PWR_CSR1_ODRDY) == 0);
    PWR->CR1 |= PWR_CR1_ODSWEN;
    while((PWR->CSR1 & PWR_CSR1_ODSWRDY) == 0);

    // The voltage scaling allows optimizing the power consumption when the device is
    // clocked below the maximum system frequency, to update the voltage scaling value
    // regarding system frequency refer to product datasheet.
    PWR->CR1 |= POWER_REGULATOR_CFG;

    //-------------------------------- PLL Configuration -----------------------

  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)
    // Disable the main PLL.
    RCC->CR &= ~RCC_CR_PLLON;

    // Configure the main PLL clock source, multiplication and division factors.
    RCC->PLLCFGR = CFG_PLL_M_DIVIDER                                          |
                   (CFG_PLL_N_MULTIPLIER << RCC_PLL_CFGR_PLL_N_POS)           |
                   (((CFG_PLL_P_DIVIDER >> 1) - 1) << RCC_PLL_CFGR_PLL_P_POS) |
                   (CFG_SYS_PLL_MUX)                                          |
                   (CFG_PLL_Q_DIVIDER << RCC_PLL_CFGR_PLL_Q_POS);

    RCC->CR |= RCC_CR_PLLON;                                            // Enable the main PLL.
    while((RCC->CR & RCC_CR_PLLRDY) == 0);                              // Wait till PLL is ready
    FLASH->ACR = FLASH_ACR_PRFTEN | CFG_FLASH_LATENCY;                  // Configure Flash prefetch and wait state
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | CFG_RCC_CFGR_SW_PLL;   // Select the main PLL as system clock source
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);              // Wait till the main PLL is used as system clock source
  #endif

    RCC->DCKCFGR2 |= RCC_DCKCFGR2_CK48MSEL;

  #ifdef CFG_ENABLE_PLLSAI
    RCC->PLLSAICFGR  = (CFG_PLLSAI_N_MULTIPLIER << RCC_PLLSAI_CFGR_PLL_N_POS) |
                       (CFG_PLLSAI_P_DIVIDER    << RCC_PLLSAI_CFGR_PLL_P_POS) |
                       (CFG_PLLSAI_Q_DIVIDER    << RCC_PLLSAI_CFGR_PLL_Q_POS) |
                       (CFG_PLLSAI_R_DIVIDER    << RCC_PLLSAI_CFGR_PLL_R_POS);

    MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_PLLSAIDIVR, CFG_LCD_CLOCK_DIVIDER);

    // TODO need to use the enable for this
    RCC->CR |= RCC_CR_PLLSAION;
    while((RCC->CR & RCC_CR_PLLSAIRDY) == 0);
  #endif

  #ifdef CFG_ENABLE_PLLI2S
    RCC->PLLI2SCFGR  = (CFG_PLLI2S_N_MULTIPLIER << RCC_PLLI2S_CFGR_PLL_N_POS) |
                       (CFG_PLLI2S_P_DIVIDER    << RCC_PLLI2S_CFGR_PLL_P_POS) |
                       (CFG_PLLI2S_Q_DIVIDER    << RCC_PLLI2S_CFGR_PLL_Q_POS) |
                       (CFG_PLLI2S_R_DIVIDER    << RCC_PLLI2S_CFGR_PLL_R_POS);

    // TODO need to use the enable for this
    RCC->CR |= RCC_CR_PLLI2SON;
    while((RCC->CR & RCC_CR_PLLI2SRDY) == 0);
  #endif

    // Set Source for USB
  #if (USE_USB_DRIVER == DEF_ENABLED)
    SET_BIT(RCC->DCKCFGR2, CFG_USB_SOURCE_MUX);
  #endif

    // AHB,APB1,APB2 CLOCK
    RCC->CFGR |= (CFG_SYS_HCLK | CFG_SYS_APB1 | CFG_SYS_APB2 | CFG_MCO_1 | CFG_MCO_2);

    // Configure the Vector Table location add offset address
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM1_BASE | VECT_TAB_OFFSET; // Vector Table Relocation in Internal SRAM
  #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; // Vector Table Relocation in Internal FLASH
  #endif
}

//-------------------------------------------------------------------------------------------------

