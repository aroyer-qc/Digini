//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F1_isr.cpp
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

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
// public function
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           ISR_Initialize
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Initialize the ISR
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void ISR_Initialize(void)
{
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    ISR_Enable();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ISR_Init
//
//  Parameter(s):   Channel     ISR Channel to init
//                  pPrio       Pointer on the structure containing ISR priority configuration
//  Return:         none
//
//  Description:    Initialization of ISR vector
//
//-------------------------------------------------------------------------------------------------
void ISR_Init(IRQn_Type Channel, const ISR_Prio_t* pPrio)
{
    // Enable interrupt
    if(pPrio != nullptr)
    {
        NVIC_SetPriority(Channel, NVIC_EncodePriority(pPrio->PriorityGroup,
                                  pPrio->PremptionPriority,
                                  pPrio->SubPriority));
        NVIC_EnableIRQ(Channel);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ISR_Init
//
//  Parameter(s):   Channel             ISR Channel to init
//                  Subpriority
//                  PremptionPriority
//  Return:         none
//
//  Description:    Initialization of ISR vector
//
//-------------------------------------------------------------------------------------------------
void ISR_Init(IRQn_Type Channel, uint8_t SubPriority, uint8_t PremptionPriority)
{
    ISR_Prio_t          ISR_Prio;
    uint32_t            PriorityGroup;

    PriorityGroup = NVIC_GetPriorityGrouping();
    ISR_Prio.PriorityGroup     = PriorityGroup;
    ISR_Prio.SubPriority       = SubPriority;
    ISR_Prio.PremptionPriority = PremptionPriority;

    ISR_Init(Channel, &ISR_Prio);
}

//-------------------------------------------------------------------------------------------------