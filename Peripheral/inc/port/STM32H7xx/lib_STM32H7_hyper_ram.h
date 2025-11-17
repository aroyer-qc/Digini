//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7_hyper_ram.h
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

#if (USE_HYPER_RAM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define OSPI_SAMPLE_SHIFTING_NONE       ((uint32_t)0x00000000)          // No shift
#define OSPI_SAMPLE_SHIFTING_HALFCYCLE  ((uint32_t)OCTOSPI_TCR_SSHIFT)  // 1/2 cycle shift

#define OSPI_DHQC_DISABLE               ((uint32_t)0x00000000)          // No Delay
#define OSPI_DHQC_ENABLE                ((uint32_t)OCTOSPI_TCR_DHQC)    // Delay Hold 1/4 cycle

#define OSPI_DELAY_BLOCK_USED           ((uint32_t)0x00000000)          // Sampling clock is delayed by the delay block
#define OSPI_DELAY_BLOCK_BYPASSED       ((uint32_t)OCTOSPI_DCR1_DLYBYP) // Delay block is bypassed

#define OSPI_LATENCY_ON_WRITE           ((uint32_t)0x00000000)          // Latency on write accesses
#define OSPI_NO_LATENCY_ON_WRITE        ((uint32_t)OCTOSPI_HLCR_WZL)    // No latency on write accesses

#define OSPI_VARIABLE_LATENCY           ((uint32_t)0x00000000)          // Variable initial latency
#define OSPI_FIXED_LATENCY              ((uint32_t)OCTOSPI_HLCR_LM)     // Fixed latency

#define OSPI_DQS_DISABLE                ((uint32_t)0x00000000)          // DQS disabled
#define OSPI_DQS_ENABLE                 ((uint32_t)OCTOSPI_CCR_DQSE)    // DQS enabled

#define OSPI_TIMEOUT_COUNTER_DISABLE    ((uint32_t)0x00000000)          // Timeout counter disabled, nCS remains active
#define OSPI_TIMEOUT_COUNTER_ENABLE     ((uint32_t)OCTOSPI_CR_TCEN)     // Timeout counter enabled, nCS released when timeout expires

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct HYPER_RAM_Info_t
{
    OCTOSPI_TypeDef* pInstance;

    uint32_t         FifoThreshold;
    uint32_t         DeviceSize;
    uint32_t         ChipSelectHighTime;
    uint32_t         ClockPrescaler;
    uint32_t         SampleShifting;
    uint32_t         DelayHoldQuarterCycle;
    uint32_t         DelayBlockBypass;
    uint32_t         ChipSelectBoundary;
    uint32_t         Refresh;

    uint32_t         RW_RecoveryTime;
    uint32_t         AccessTime;
    uint32_t         WriteZeroLatency;
    uint32_t         LatencyMode;

    uint32_t         DQS_Mode;
    uint32_t         Address;
    uint32_t         NbData;

    uint32_t         TimeOutActivation;
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

SystemState_e   HYPER_RAM_Initialize    (void);

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "ram_var.h"

//-------------------------------------------------------------------------------------------------

#endif // USE_HYPER_RAM_DRIVER

