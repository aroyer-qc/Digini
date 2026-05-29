//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7_hyper_ram.cpp
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

#define HYPER_RAM_GLOBAL
#include "./lib_digini.h"
#undef  HYPER_RAM_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_HYPER_RAM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define OSPI_MEMORY_TYPE_HYPERBUS           ((uint32_t)OCTOSPI_DCR1_MTYP_2)
#define OSPI_TIMEOUT_DEFAULT_VALUE          ((uint32_t)5000)                // 5 Seconds
#define OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  ((uint32_t)OCTOSPI_CR_FMODE)    // Memory-mapped mode

//-------------------------------------------------------------------------------------------------
//
//   Function:      HYPER_RAM_Initialize
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Initialize the OSPI module for Hyper Ram usage
//
//-------------------------------------------------------------------------------------------------
SystemState_e HYPER_RAM_Initialize(void)
{
    TickCount_t      TickStart;
    OCTOSPI_TypeDef* pInstance = HYPER_RAM_Info.pInstance;

    // Enable clock on OSPI module
    if(pInstance == OCTOSPI1)
    {
        RCC->AHB3RSTR |=   RCC_AHB3RSTR_OSPI1RST;
        RCC->AHB3RSTR &= ~(RCC_AHB3RSTR_OSPI1RST);
        RCC->AHB3ENR  |= RCC_AHB3ENR_OSPI1EN;
    }
    else
    {
        RCC->AHB3RSTR |=   RCC_AHB3RSTR_OSPI2RST;
        RCC->AHB3RSTR &= ~(RCC_AHB3RSTR_OSPI2RST);
        RCC->AHB3ENR  |= RCC_AHB3ENR_OSPI2EN;
    }

    TickStart = GetTick();

    pInstance->DCR1 = (OSPI_MEMORY_TYPE_HYPERBUS                                          |     // Configure memory type, device size, chip select high time, delay block bypass, clock mode
                       ((HYPER_RAM_Info.DeviceSize - 1) << OCTOSPI_DCR1_DEVSIZE_Pos)      |
                       ((HYPER_RAM_Info.ChipSelectHighTime - 1) << OCTOSPI_DCR1_CSHT_Pos) |
                       HYPER_RAM_Info.DelayBlockBypass);
    pInstance->DCR2 = 0;                                                                        // Reset to default
    pInstance->DCR3 = (HYPER_RAM_Info.ChipSelectBoundary << OCTOSPI_DCR3_CSBOUND_Pos);          // Configure chip select boundary and maximum transfer
    pInstance->DCR4 = HYPER_RAM_Info.Refresh;                                                   // Configure refresh
    pInstance->CR   = ((HYPER_RAM_Info.FifoThreshold - 1) << OCTOSPI_CR_FTHRES_Pos);            // Configure FIFO threshold

    while((pInstance->SR &  OCTOSPI_SR_BUSY) != 0)                                              // Wait until flag is reset
    {
        if(TickHasTimeOut(TickStart, OSPI_TIMEOUT_DEFAULT_VALUE) == true)
        {
            return SYS_ERROR;
        }
    }

    pInstance->DCR2 = ((HYPER_RAM_Info.ClockPrescaler - 1) << OCTOSPI_DCR2_PRESCALER_Pos);      // Configure clock prescaler
    pInstance->TCR = (HYPER_RAM_Info.SampleShifting | HYPER_RAM_Info.DelayHoldQuarterCycle);    // Configure sample shifting and delay hold quarter cycle
    SET_BIT(pInstance->CR, OCTOSPI_CR_EN);                                                      // Enable OctoSPI

    TickStart = GetTick();

    while((pInstance->SR &  OCTOSPI_SR_BUSY) != 0)                                              // Wait until flag is reset
    {
        if(TickHasTimeOut(TickStart, OSPI_TIMEOUT_DEFAULT_VALUE) == true)
        {
            return SYS_ERROR;
        }
    }

    WRITE_REG(pInstance->HLCR, ((HYPER_RAM_Info.RW_RecoveryTime << OCTOSPI_HLCR_TRWR_Pos) |     // Configure Hyperbus configuration Latency register
                                (HYPER_RAM_Info.AccessTime      << OCTOSPI_HLCR_TACC_Pos) |
                                 HYPER_RAM_Info.WriteZeroLatency                          |
                                 HYPER_RAM_Info.LatencyMode));

    TickStart = GetTick();

    while((pInstance->SR &  OCTOSPI_SR_BUSY) != 0)                                              // Wait until flag is reset
    {
        if(TickHasTimeOut(TickStart, OSPI_TIMEOUT_DEFAULT_VALUE) == true)
        {
            return SYS_ERROR;
        }
    }

    // Configure the CCR and WCCR registers with the address size and the following configuration :
    // - DQS signal enabled (used as RWDS)
    // - DTR mode enabled on address and data
    // - address and data on 8 lines
    pInstance->CCR  = HYPER_RAM_Info.DQS_Mode | OCTOSPI_CCR_DDTR  | OCTOSPI_CCR_DMODE_2  | OCTOSPI_CCR_ABSIZE  | OCTOSPI_CCR_ADDTR  | OCTOSPI_CCR_ADMODE_2;
    pInstance->WCCR = HYPER_RAM_Info.DQS_Mode | OCTOSPI_WCCR_DDTR | OCTOSPI_WCCR_DMODE_2 | OCTOSPI_WCCR_ABSIZE | OCTOSPI_WCCR_ADDTR | OCTOSPI_WCCR_ADMODE_2;
    pInstance->DLR  = HYPER_RAM_Info.NbData - 1;                                                        // Configure the DLR register with the number of data
    pInstance->AR   = HYPER_RAM_Info.Address;                                                           // Configure the AR register with the address value

    TickStart = GetTick();

    while((pInstance->SR & OCTOSPI_SR_BUSY) != 0)                                              // Wait until flag is reset
    {
        if(TickHasTimeOut(TickStart, OSPI_TIMEOUT_DEFAULT_VALUE) == true)
        {
            return SYS_ERROR;
        }
    }

    MODIFY_REG(pInstance->CR,
               (OCTOSPI_CR_TCEN | OCTOSPI_CR_FMODE),
               (HYPER_RAM_Info.TimeOutActivation | OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED));                // Configure CR register with functional mode as memory-mapped

    return SYS_READY;                                                                           // Return function status
}

//-------------------------------------------------------------------------------------------------

#endif // USE_HYPER_RAM_DRIVER
