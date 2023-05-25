//-------------------------------------------------------------------------------------------------
//
//  File : system_clock.c
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
    uint32_t Retry;

    __asm volatile("cpsid i");                                              // Disable IRQ

    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
    MODIFY_REG(PWR->CR, PWR_CR_VOS, POWER_REGULATOR_CFG);

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SET_BIT(SCB->CPACR, ((3UL << 10 * 2) | (3UL << 11 * 2)));   // Set CP10 and CP11 Full Access
  #endif

    // Reset the RCC clock configuration to the default reset state ------------
    // Set HSION bit
    SET_BIT(RCC->CR, RCC_CR_HSION);

    // Set CFGR register
	RCC->CFGR = (SYS_HCLK_CFG | SYS_APB1_CFG | SYS_APB2_CFG);

    // Reset HSEBYP, CSSON and PLLON bits
	CLEAR_BIT(RCC->CR, (RCC_CR_CSSON | RCC_CR_PLLON | RCC_CR_HSEBYP));

  #if (SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL)

    SET_BIT(RCC->CR, RCC_CR_HSEON);

    // Wait for HSE to be ready B4 enabling PLL
    Retry = 0;
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

        // Set PLLCFGR register
        RCC->PLLCFGR = RCC_HSI_PLL_CFGR_CFG;

      #if (HSE_AND_HSI_MUST_BE_MATCHED == DEF_ENABLED)
        // Set a flag for main loop to signal HSE clock failure!
        SystemHSE_ClockFailure = true;
      #endif
    }
    else
    {
        // Set PLLCFGR register
        RCC->PLLCFGR = RCC_HSE_PLL_CFGR_CFG;

        // Reset HSION bit to reduce consumption
        CLEAR_BIT(RCC->CR, RCC_CR_HSION);

      #if (HSE_AND_HSI_MUST_BE_MATCHED == DEF_ENABLED)
        // Set a flag for main loop signal HSE is OK!
        SystemHSE_ClockFailure = false;
      #endif
    }

    // Set flash latency
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_LATENCY_CFG);

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
}

//-------------------------------------------------------------------------------------------------
