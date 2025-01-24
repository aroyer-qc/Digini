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
//                      - Setup the micro controller system.
//                      - Initialize the FPU setting
//                      - vector table locations configuration.
//
//-------------------------------------------------------------------------------------------------
#ifdef CORE_CM7
void SystemInit(void)
{
    __asm volatile("cpsid i");                                                  // Disable IRQ

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3 << (10 * 2)) | (3 << (11 * 2)));                          // Set CP10 and CP11 Full Access
  #endif

    // SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt signal is detectable by the CPU after a WFI/WFE instruction.
    SCB->SCR |= SCB_SCR_SEVONPEND_Pos;

//according th PDF VOS0 shoul be chossen for 240Mhz AXI clock with 4 WS
    SET_BIT(PWR->CR3, PWR_CR3_SMPSEN);                                          // Set the power supply configuration
    while((PWR->D3CR & PWR_D3CR_VOSRDY) != PWR_D3CR_VOSRDY){};                  // Wait till voltage level flag is set

    // Configure the main internal regulator output voltage
    CLEAR_BIT(SYSCFG->PWRCR, SYSCFG_PWRCR_ODEN);                                // Disable the PWR overdrive
    MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, PWR_REGULATOR_VOLTAGE_SCALE3);          // Configure the main internal regulator output voltage
    SET_BIT(RCC->APB4ENR, RCC_APB4ENR_SYSCFGEN);

    // Reset the RCC clock configuration to the default reset state
    RCC->CR        |= RCC_CR_HSION;                                             // Set HSION bit
    RCC->CFGR       = 0x00000000;                                               // Reset CFGR register

    CLEAR_BIT(RCC->CR, (RCC_CR_HSEON   | RCC_CR_CSSHSEON | RCC_CR_CSION  |      // Reset HSEON, CSSON, CSION, RC48ON, CSIKERON, PLL1ON, PLL2ON and PLL3ON bits
                        RCC_CR_HSI48ON | RCC_CR_CSIKERON | RCC_CR_PLL1ON |
                        RCC_CR_PLL2ON | RCC_CR_PLL3ON));

    RCC->D1CFGR     = CFG_RCC_D1CFGR;
    RCC->D2CFGR     = CFG_RCC_D2CFGR;
    RCC->D3CFGR     = CFG_RCC_D3CFGR;
    RCC->D1CCIPR    = CFG_RCC_D1CCIPR;
    RCC->D2CCIP1R   = CFG_RCC_D2CCIP1R;
    RCC->D2CCIP2R   = CFG_RCC_D2CCIP2R;
    RCC->D3CCIPR    = CFG_RCC_D3CCIPR;

    RCC->PLLCKSELR  = CFG_RCC_PLLCKSELR;
    RCC->PLLCFGR    = CFG_RCC_PLLCFGR;
    RCC->PLL1DIVR   = CFG_RCC_PLL1_DIVR;
    RCC->PLL1FRACR  = (CFG_PLL1_FRACTIONAL_VALUE << RCC_PLL1FRACR_FRACN1_Pos);
    RCC->PLL2DIVR   = CFG_RCC_PLL2_DIVR;
    RCC->PLL2FRACR  = (CFG_PLL2_FRACTIONAL_VALUE << RCC_PLL2FRACR_FRACN2_Pos);
    RCC->PLL3DIVR   = CFG_RCC_PLL3_DIVR;
    RCC->PLL3FRACR  = (CFG_PLL2_FRACTIONAL_VALUE << RCC_PLL3FRACR_FRACN3_Pos);
    CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP;                                           // Reset HSEBYP bit
    RCC->CIER       = 0x00000000;                                               // Disable all interrupts
    SET_BIT(EXTI_D2->EMR3, EXTI_EMR3_EM78);                                     // Enable CortexM7 HSEM EXTI line (line 78)



    if((DBGMCU->IDCODE & 0xFFFF0000) < 0x20000000)
    {
        // if stm32h7 revY
        // Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7)
        *((__IO uint32_t*)0x51008108) = 0x000000001;
    }


// start here the clcok config PLL HSE, etc....






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

#if 0


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






#endif







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
