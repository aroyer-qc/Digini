//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_isr.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define NVIC_PRIORITYGROUP_0    ((uint32_t)0x7U) // 0 bits pre-emption priority 4 bits subpriority
#define NVIC_PRIORITYGROUP_1    ((uint32_t)0x6U) // 1 bits pre-emption priority 3 bits subpriority
#define NVIC_PRIORITYGROUP_2    ((uint32_t)0x5U) // 2 bits pre-emption priority 2 bits subpriority
#define NVIC_PRIORITYGROUP_3    ((uint32_t)0x4U) // 3 bits pre-emption priority 1 bits subpriority
#define NVIC_PRIORITYGROUP_4    ((uint32_t)0x3U) // 4 bits pre-emption priority 0 bits subpriority

#define ISR_Enable()            __asm volatile("cpsie i")
#define ISR_Disable()           __asm volatile("cpsid i")

// TO DO come from F7, I don't know yet if it is relevant
// Get Pending Interrupt
#define ISR_GetPendingIRQ(IRQ)   (((NVIC->ISPR[0] & (uint32_t(1) << uint32_t(int32_t(IRQ) & 0x1F))) != 0) ? true : false)
// Set Pending Interrupt
#define ISR_SetPendingIRQ(IRQ)   (NVIC->ISPR[0] = (uint32_t(1) << ((uint32_t)(int32_t(IRQ) & 0x1F))))
// Clear Pending Interrupt
#define ISR_ClearPendingIRQ(IRQ) (NVIC->ICPR[0] = (uint32_t(1) << ((uint32_t)(int32_t(IRQ) & 0x1L))))

#define ISR_IRQn_NONE               IRQn_Type(0x7F)

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct ISR_Prio_t
{
    uint32_t PriorityGroup;
    uint8_t  PremptionPriority;
    uint8_t  SubPriority;
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void ISR_Initialize     (void);
void ISR_Init           (IRQn_Type Channel, const ISR_Prio_t* pPrio);
void ISR_Init           (IRQn_Type Channel, uint8_t SubPriority, uint8_t PremptionPriority);

//-------------------------------------------------------------------------------------------------
