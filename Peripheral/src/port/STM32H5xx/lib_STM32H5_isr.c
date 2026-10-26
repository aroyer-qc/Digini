//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_isr.c
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "BSP_board.h"

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
    PRIVILEGE_EXEC(
        NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
        ISR_Enable();
    );
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ISR_FullInit
//
//  Parameter(s):   Channel     ISR Channel to init
//                  pPrio       Pointer on the structure containing ISR priority configuration
//  Return:         none
//
//  Description:    Initialization of ISR vector
//
//-------------------------------------------------------------------------------------------------
void ISR_FullInit(IRQn_Type Channel, const ISR_Prio_t* pPrio)
{
    // Enable interrupt
    if((int)Channel != (int)ISR_IRQn_NONE)
    {
        if(pPrio != NULL)
        {
            PRIVILEGE_EXEC(
                NVIC_SetPriority(Channel, NVIC_EncodePriority(pPrio->PriorityGroup,
                                        pPrio->PreemptionPriority,
                                        pPrio->SubPriority));
                NVIC_EnableIRQ(Channel);
            );
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ISR_Init
//
//  Parameter(s):   Channel             ISR Channel to init
//                  PremptionPriority
//  Return:         none
//
//  Description:    Initialization of ISR vector
//
//-------------------------------------------------------------------------------------------------
void ISR_Init(IRQn_Type Channel, uint8_t PreemptionPriority)
{
    ISR_Prio_t          ISR_Prio;
    uint32_t            PriorityGroup;

    PRIVILEGE_EXEC(PriorityGroup = NVIC_GetPriorityGrouping(););
    ISR_Prio.PriorityGroup      = PriorityGroup;
    ISR_Prio.SubPriority        = 0;
    ISR_Prio.PreemptionPriority = PreemptionPriority;

    ISR_FullInit(Channel, &ISR_Prio);
}

//-------------------------------------------------------------------------------------------------
