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
//
//  Note(s)
//
//              Voltage Scaling Mode (STM32H74x/STM32H75x)
//          -------------------------------------------------------------------
//          |  Voltage Range  |  Mode  |    CM7    |    CM4    |  Power Mode  |
//          |-----------------|--------|-----------|-----------|--------------|
//          |   Range 0       |  Run   |  480 MHz  |  240 MHz  |     LDO      |
//          |-----------------|--------|-----------|-----------|--------------|
//          |   Range 1       |  Run   |  400 MHz  |  200 MHz  |     SMPS     |
//          |-----------------|--------|-----------|-----------|--------------|
//          |   Range 2       |  Run   |  300 MHz  |  150 MHz  |     SMPS     |
//          |-----------------|--------|-----------|-----------|--------------|
//          |   Range 3       |  Run   |  200 MHz  |  100 MHz  |     SMPS     |
//          -------------------------------------------------------------------
//
//              Voltage Scaling Mode (STM32H72x/STM32H73x)
//          -------------------------------------------------------
//          |  Voltage Range  |  Mode  |    CM7    |  Power Mode  |
//          |-----------------|--------|-----------|--------------|
//          |   Range 0   +   |  Run   |  550 MHz  |              |
//          |  CPU_FREQ_BOOST |        |           |              |
//          |-----------------|--------|-----------|--------------|
//          |   Range 0       |  Run   |  520 MHz  |              |
//          |-----------------|--------|-----------|--------------|
//          |   Range 1       |  Run   |  400 MHz  |              |
//          |-----------------|--------|-----------|--------------|
//          |   Range 2       |  Run   |  300 MHz  |              |
//          |-----------------|--------|-----------|--------------|
//          |   Range 3       |  Run   |  170 MHz  |              |
//          -------------------------------------------------------
//
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
\

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
//  Note(s):         The activation of the I/O Compensation Cell is recommended with communication
//                   interfaces (GPIO, SPI, FMC, QSPI ...)  when operating at high frequencies
//
//-------------------------------------------------------------------------------------------------
#ifdef CORE_CM7
void SystemInit(void)
{
    uint32_t Retry;

    __asm volatile("cpsid i");                                                      // Disable IRQ

    // FPU settings
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3 << (10 * 2)) | (3 << (11 * 2)));                              // Set CP10 and CP11 Full Access
  #endif

    // SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt signal is detectable by the CPU after a WFI/WFE instruction.
    SCB->SCR |= SCB_SCR_SEVONPEND_Pos;

    // According the PDF VOS0 should be chosen for 240Mhz AXI clock with 4 WS
  #if defined (SMPS)
    SET_BIT(PWR->CR3, PWR_CR3_SMPSEN);                                              // Set the power supply configuration
  #endif

    // Configure the main internal regulator output voltage
  #if defined(PWR_SRDCR_VOS)                                                        // STM32H7Axxx and STM32H7Bxxx lines
    MODIFY_REG(PWR->SRDCR, PWR_SRDCR_VOS, CFG_PWR_REGULATOR_VOLTAGE);
  #else
   #if defined(SYSCFG_PWRCR_ODEN)                                                   // STM32H74xxx and STM32H75xxx lines
    uint32_t RegisterValue;

    if(CFG_PWR_REGULATOR_VOLTAGE == CFG_PWR_REGULATOR_VOLTAGE_SCALE0)
    {
// TODO not working with H745 at this time... when bypasssed it is OK!!
//        MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, CFG_PWR_REGULATOR_VOLTAGE_SCALE1);    // Configure the Voltage Scaling 1
//        RegisterValue = READ_BIT(PWR->D3CR, PWR_D3CR_VOS);                        // Delay after setting the voltage scaling
//        SET_BIT(SYSCFG->PWRCR, SYSCFG_PWRCR_ODEN);                                // Enable the PWR overdrive
//        RegisterValue = READ_BIT(SYSCFG->PWRCR, SYSCFG_PWRCR_ODEN);               // Delay after setting the syscfg boost setting
    }
    else
    {
        CLEAR_BIT(SYSCFG->PWRCR, SYSCFG_PWRCR_ODEN);                                // Disable the PWR overdrive
        RegisterValue = READ_BIT(SYSCFG->PWRCR, SYSCFG_PWRCR_ODEN);                 // Delay after setting the syscfg boost setting
        MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, CFG_PWR_REGULATOR_VOLTAGE);             // Configure the Voltage Scaling x
        RegisterValue = READ_BIT(PWR->D3CR, PWR_D3CR_VOS);                          // Delay after setting the voltage scaling
    }

    VAR_UNUSED(RegisterValue);
   #else                                                                            // STM32H72xxx and STM32H73xxx lines
    uint32_t RegisterValue;

    MODIFY_REG (PWR->D3CR, PWR_D3CR_VOS, CFG_PWR_REGULATOR_VOLTAGE);                // Configure the Voltage Scaling
    RegisterValue = READ_BIT(PWR->D3CR, PWR_D3CR_VOS);                              // Delay after setting the voltage scaling
    VAR_UNUSED(RegisterValue);
   #endif
  #endif

    // Wait till voltage level flag is set
  #if defined (DUAL_CORE) || defined (PWR_CPUCR_SBF_D2) || defined (SMPS)
    while((PWR->D3CR & PWR_D3CR_VOSRDY) != PWR_D3CR_VOSRDY){};
  #else // STM32H7Axx and STM32H7Bxx lines
    while((PWR->SRDCR & PWR_SRDCR_VOSRDY) != PWR_SRDCR_VOSRDY){};
  #endif

    SET_BIT(RCC->APB4ENR, RCC_APB4ENR_SYSCFGEN);                                    // Enable SYSCFG Clock

  #if defined(STM32H723xx) ||  defined(STM32H725xx) || defined(STM32H730xx) || defined(STM32H730xxQ) || defined(STM32H735xx)
    FLASH->OPTKEYR = 0x08192A3B;
    FLASH->OPTKEYR = 0x4C5D6E7F;

    if(SYS_HCLK_CLOCK_FREQUENCY > 520000000)
    {
        SET_BIT(FLASH->OPTSR2_PRG, FLASH_OPTSR2_CPUFREQ_BOOST);
    }
    else
    {
        CLEAR_BIT(FLASH->OPTSR2_PRG, FLASH_OPTSR2_CPUFREQ_BOOST);
    }
  #endif

    // Reset the RCC clock configuration to the default reset state
    RCC->CR   |= RCC_CR_HSION;                                                      // Set HSION bit
    RCC->CFGR  = 0x00000000;                                                        // Reset CFGR register

    CLEAR_BIT(RCC->CR, (RCC_CR_HSEON   | RCC_CR_CSSHSEON | RCC_CR_CSION  |          // Reset HSEON, CSSON, CSION, RC48ON, CSIKERON, PLL1ON, PLL2ON and PLL3ON bits
                        RCC_CR_HSI48ON | RCC_CR_CSIKERON | RCC_CR_PLL1ON |
                        RCC_CR_PLL2ON  | RCC_CR_PLL3ON));

  #if defined(RCC_D1CFGR_HPRE)
    RCC->D1CFGR     = CFG_RCC_D1CFGR;
    RCC->D2CFGR     = CFG_RCC_D2CFGR;
    RCC->D3CFGR     = CFG_RCC_D3CFGR;
    RCC->D1CCIPR    = CFG_RCC_D1CCIPR;
    RCC->D2CCIP1R   = CFG_RCC_D2CCIP1R;
    RCC->D2CCIP2R   = CFG_RCC_D2CCIP2R;
    RCC->D3CCIPR    = CFG_RCC_D3CCIPR;
  #else
    RCC->CDCFGR1    = CFG_RCC_CDCFGR1;
    RCC->CDCFGR2    = CFG_RCC_CDCFGR2;
    RCC->SRDCFGR    = CFG_RCC_SRDCFGR;
    RCC->CDCCIPR    = CFG_RCC_CDCCIPR;
    RCC->CDCCIP1R   = CFG_RCC_CDCCIP1R;
    RCC->CDCCIP2R   = CFG_RCC_CDCCIP2R;
    RCC->SRDCCIPR   = CFG_RCC_SRDCCIPR;
  #endif

    CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);                                              // Reset HSEBYP bit
    RCC->CIER       = 0x00000000;                                                   // Disable all interrupts
  #if defined(EXTI_EMR3_EM78)
    SET_BIT(EXTI_D2->EMR3, EXTI_EMR3_EM78);                                         // Enable CortexM7 HSEM EXTI line (line 78)
  #endif

    if((DBGMCU->IDCODE & 0xFFFF0000) < 0x20000000)
    {
        // if stm32h7 revY
        // Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7)
        *((__IO uint32_t*)0x51008108) = 0x000000001;
    }

    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, CFG_FLASH_LATENCY);                   // Set flash latency

    /// Config all PLL

  #if (CFG_MUX_PLL_SOURCE == CFG_RCC_PLLCKSELR_PLLSRC_HSE)

    Retry = 0;
    SET_BIT(RCC->CR, RCC_CR_HSEON);

    while((READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0) && (Retry < CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY))        // Wait for HSE to be ready B4 enabling PLL
    {
        Retry++;
    };

  #elif (CFG_MUX_PLL_SOURCE == CFG_RCC_PLLCKSELR_PLLSRC_CSI)

    Retry = 0;
    SET_BIT(RCC->CR, RCC_CR_CSION);

    while(READ_BIT(RCC->CR, RCC_CR_CSIRDY) == 0) && (Retry < CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY))         // Wait for CSI to be ready B4 enabling PLL
    {
        Retry++;
    };

  #else
    // HSI is the default PLL source
  #endif

    RCC->PLLCFGR   = CFG_RCC_PLLCFGR;
    RCC->PLL1DIVR  = CFG_RCC_PLL1_DIVR;
    RCC->PLL1FRACR = (CFG_PLL1_FRACTIONAL_VALUE << RCC_PLL1FRACR_FRACN1_Pos);
    RCC->PLL2DIVR  = CFG_RCC_PLL2_DIVR;
    RCC->PLL2FRACR = (CFG_PLL2_FRACTIONAL_VALUE << RCC_PLL2FRACR_FRACN2_Pos);
    RCC->PLL3DIVR  = CFG_RCC_PLL3_DIVR;
    RCC->PLL3FRACR = (CFG_PLL2_FRACTIONAL_VALUE << RCC_PLL3FRACR_FRACN3_Pos);
    RCC->PLLCKSELR = CFG_RCC_PLLCKSELR;

  #if ((CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSI) && (CFG_MUX_PLL_SOURCE != CFG_RCC_PLLCKSELR_PLLSRC_HSI) && (CFG_PER_SOURCE_MUX != CFG_RCC_D1CCIPR_PER_HSI_KER))
    CLEAR_BIT(RCC->CR, RCC_CR_HSION);                                                                   // Reset HSION bit to reduce consumption
  #endif

    SET_BIT(RCC->CR, (RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON));                                   // Enable All PLL

    // Wait for PLL to be ready B4 enabling PLL
    Retry = 0;
    while((READ_BIT(RCC->CR, RCC_CR_PLL1RDY) == 0) && (READ_BIT(RCC->CR, RCC_CR_PLL2RDY) == 0) && (READ_BIT(RCC->CR, RCC_CR_PLL3RDY) == 0))
    {
        Retry++;
    };

    /// Switch Main CPU to the configure source.

  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_PLL1)

    SET_BIT(RCC->CFGR, CFG_RCC_CFGR_SW_PLL1);                                                           // Switch to PLL

  #elif (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_HSE)
    Retry = 0;
    SET_BIT(RCC->CR, RCC_CR_HSEON);

    while((READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0) && (Retry < CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY))        // Wait for HSE to be ready B4 enabling PLL
    {
        Retry++;
    };

    SET_BIT(RCC->CFGR, CFG_RCC_CFGR_SW_HSE);                                                            // Switch to HSE
  #endif

    Retry = 0;
    SET_BIT(RCC->CR, RCC_CR_CSION);                 // CSI must be enabled for IO compensation cell

    while((READ_BIT(RCC->CR, RCC_CR_CSIRDY) == 0) && (Retry < CFG_SYSTEM_CLOCK_NUMBER_OF_RETRY))        // Wait for HSE to be ready B4 enabling PLL
    {
        Retry++;
    };

  #if (CFG_SYS_CLOCK_MUX == CFG_RCC_CFGR_SW_CSI)
    SET_BIT(RCC->CFGR, CFG_RCC_CFGR_SW_CSI);                                                            // Switch to HSE
  #endif

  SET_BIT(SYSCFG->CCCSR, SYSCFG_CCCSR_EN);          // Enables the I/O Compensation Cell

  /* Configure the Vector Table location add offset address ------------------*/
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = D1_AXISRAM_BASE  | VECT_TAB_OFFSET; // Vector Table Relocation in Internal D1 AXI-RAM
  #else
    SCB->VTOR = FLASH_BANK1_BASE | VECT_TAB_OFFSET; // Vector Table Relocation in Internal FLASH
  #endif
}
#endif

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
