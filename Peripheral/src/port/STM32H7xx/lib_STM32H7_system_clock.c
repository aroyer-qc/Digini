//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7_system_clock.c
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define SYSTEM_GLOBAL
#include "./Peripheral/inc/port/lib_system_clock.h"
#undef  SYSTEM_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define RCC_PLLCFGR_RESET_VALUE             0x01FF0000


// Vector Table base offset field. This value must be a multiple of 0x200.
#define VECT_TAB_OFFSET                     0x00000000

//-------------------------------------------------------------------------------------------------
//
//  Name:           SystemInit
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Configures CM7 Core
//                      - Reset the CPU Setting
//                      - Setup the microcontroller system.
//                      - Initialize the FPU setting
//                      - vector table locations configuration.
//
//-------------------------------------------------------------------------------------------------
#ifdef CORE_CM7
void SystemInit(void)
{
    __asm volatile("cpsid i");                              // Disable IRQ

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3 << (10 * 2)) | (3 << (11 * 2)));      // Set CP10 and CP11 Full Access
  #endif

    // SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt signal is detectable by the CPU after a WFI/WFE instruction.
    SCB->SCR |= SCB_SCR_SEVONPEND_Pos;


    /*
    /* Supply configuration update enable */
    HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

    /* Configure the main internal regulator output voltage */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

__HAL_RCC_SYSCFG_CLK_ENABLE();


    // Reset the RCC clock configuration to the default reset state
    RCC->CR        |= RCC_CR_HSION;                         // Set HSION bit
    RCC->CFGR       = 0x00000000;                           // Reset CFGR register

    // Reset HSEON, CSSON, CSION,RC48ON, CSIKERON PLL1ON, PLL2ON and PLL3ON bits
    RCC->CR        &= 0xEAF6ED7F;
    RCC->D1CFGR     = 0x00000000;                           // Reset D1CFGR register
    RCC->D2CFGR     = 0x00000000;                           // Reset D2CFGR register
    RCC->D3CFGR     = 0x00000000;                           // Reset D3CFGR register
    RCC->PLLCKSELR  = 0x00000000;                           // Reset PLLCKSELR register
    RCC->PLLCFGR    = 0x00000000;                           // Reset PLLCFGR register
    RCC->PLL1DIVR   = 0x00000000;                           // Reset PLL1DIVR register
    RCC->PLL1FRACR  = 0x00000000;                           // Reset PLL1FRACR register
    RCC->PLL2DIVR   = 0x00000000;                           // Reset PLL2DIVR register
    RCC->PLL2FRACR  = 0x00000000;                           // Reset PLL2FRACR register
    RCC->PLL3DIVR   = 0x00000000;                           // Reset PLL3DIVR register
    RCC->PLL3FRACR  = 0x00000000;                           // Reset PLL3FRACR register
    RCC->CR        &= 0xFFFBFFFF;                           // Reset HSEBYP bit
    RCC->CIER       = 0x00000000;                           // Disable all interrupts
    EXTI_D2->EMR3  |= 0x4000;                               // Enable CortexM7 HSEM EXTI line (line 78)

    if((DBGMCU->IDCODE & 0xFFFF0000) < 0x20000000)
    {
        // if stm32h7 revY
        // Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7)
        *((__IO uint32_t*)0x51008108) = 0x000000001;
    }

  /* Configure the Vector Table location add offset address ------------------*/
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = D1_AXISRAM_BASE  | VECT_TAB_OFFSET; // Vector Table Relocation in Internal D1 AXI-RAM
  #else
    SCB->VTOR = FLASH_BANK1_BASE | VECT_TAB_OFFSET; // Vector Table Relocation in Internal FLASH
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SystemInit
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Configures CM4 Core
//                      - Initialize the FPU setting
//                      - vector table locations configuration.
//
//-------------------------------------------------------------------------------------------------
#ifdef CORE_CM4
void SystemInit(void)
{
    __asm volatile("cpsid i");                        // Disable IRQ

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3 << (10 * 2)) | (3 << (11 * 2)));  // set CP10 and CP11 full access
  #endif

    // SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt signal is detectable
    // by the CPU after a WFI/WFE instruction.
    SCB->SCR |= SCB_SCR_SEVONPEND_Pos;


    // Configure the Vector Table location add offset address
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = D2_AHBSRAM_BASE | VECT_TAB_OFFSET;  // Vector Table Relocation in Internal SRAM
  #else
    SCB->VTOR = FLASH_BANK2_BASE | VECT_TAB_OFFSET; // Vector Table Relocation in Internal FLASH
  #endif
}
#endif

//-------------------------------------------------------------------------------------------------




Disable IRQ
SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
MODIFY_REG(PWR->CR, PWR_CR_VOS, POWER_REGULATOR_CFG);

// FPU settings
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
SET_BIT(SCB->CPACR, ((3UL << 10 * 2) | (3UL << 11 * 2)));   // Set CP10 and CP11 Full Access
endif

// Reset the RCC clock configuration to the default reset state ------------
// Set HSION bit
SET_BIT(RCC->CR, RCC_CR_HSION);

// Set CFGR register
RCC->CFGR = (CFG_SYS_HCLK | CFG_SYS_APB1 | CFG_SYS_APB2 | CFG_MCO_1 | CFG_MCO_2);

// Reset HSEBYP, CSSON and PLLON bits
CLEAR_BIT(RCC->CR, (RCC_CR_CSSON | RCC_CR_PLLON | RCC_CR_HSEBYP));














  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

   #if (CFG_PLL_SOURCE == CFG_HSE_VALUE)
    SET_BIT(RCC->CR, RCC_CR_HSEON);

    Retry = 0;
    while((READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0) && (Retry < CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY))
    {
        Retry++;
    };

   #else
    // Wait for HSI to be ready B4 enabling PLL
    while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0) {};
   #endif

    // Set PLLCFGR register
    RCC->PLLCFGR = CFG_RCC_PLLCFGR_CFG;

   #if (CFG_PLL_SOURCE == CFG_HSE_VALUE)
    // Reset HSION bit to reduce consumption
    CLEAR_BIT(RCC->CR, RCC_CR_HSION);
   #endif

    // Set flash latency
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, CFG_FLASH_LATENCY);

    // Enable PLL
    SET_BIT(RCC->CR, RCC_CR_PLLON);

    // Wait for PLL to be ready B4 enabling PLL
    while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0) {};

    // Switch to PLL
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL);

  #endif // (SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

    // Disable all interrupts
    RCC->CIR = 0;

    // Configure the Vector Table location add offset address ------------------
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;    // Vector Table Relocation in Internal SRAM
  #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;   // Vector Table Relocation in Internal FLASH
  #endif
