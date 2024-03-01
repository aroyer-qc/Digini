//-------------------------------------------------------------------------------------------------
//
//  File : system_clock.c
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
#include "system_clock.h"
#undef  SYSTEM_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Vector Table base offset field. This value must be a multiple of 0x200.
#define VECT_TAB_OFFSET                 0x00
#define SYSTEM_CLOCK_NUMBER_OF_RETRY    10000

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
    __asm volatile("cpsid i");                                              // Disable IRQ

    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);                              // Enable alternate function I/O clock

    // Reset the RCC clock configuration to the default reset state ------------
    // Set HSION bit
    SET_BIT(RCC->CR, RCC_CR_HSION);

    // Set CFGR register
	RCC->CFGR = (CFG_SYS_HCLK | CFG_SYS_APB1 | CFG_SYS_APB2 | CFG_MCO_OUTPUT_SELECT);

    // Reset HSEBYP, CSSON and PLLON bits
	CLEAR_BIT(RCC->CR, (RCC_CR_CSSON | RCC_CR_PLLON | RCC_CR_HSEBYP));

  #if (CFG_SYS_CLOCK_MUX == CFG_CLOCK_SRC_PLL)

    SET_BIT(RCC->CR, RCC_CR_HSEON);

    // Wait for HSE to be ready B4 enabling PLL
    uint32_t  Retry = 0;
    while((READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0) && (Retry < SYSTEM_CLOCK_NUMBER_OF_RETRY))
    {
        Retry++;
    };

    // If HSE not ready, will will switch to HSI
    if(READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0)
    {
        CLEAR_BIT(RCC->CR, RCC_CR_HSEON);

        // Wait for HSI to be ready B4 enabling PLL
        while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0) {};
    }
    else
    {
        // Set PLL src in CFGR register
        MODIFY_REG(RCC->CFGR, RCC_CFGR_PLL_SRC_MASK, CFG_RCC_PLL_CFGR);
    }

    // Enable Prefetch Buffer
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    // Set flash latency
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, CFG_FLASH_LATENCY);

    // Enable PLL
    SET_BIT(RCC->CR, RCC_CR_PLLON);

    // Wait for PLL to be ready B4 enabling PLL
    while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0) {};

    // Switch to PLL
    SET_BIT(RCC->CFGR, RCC_CFGR_SW_PLL);

    if(READ_BIT(RCC->CR, RCC_CR_HSERDY) != 0)
    {
        // Reset HSION bit to reduce consumption
        CLEAR_BIT(RCC->CR, RCC_CR_HSION);
    }

  #endif // (SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

    // Disable and clear all interrupts
    RCC->CIR = 0;

    // Configure the Vector Table location add offset address ------------------
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;    // Vector Table Relocation in Internal SRAM
  #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;   // Vector Table Relocation in Internal FLASH
  #endif
}

//-------------------------------------------------------------------------------------------------

