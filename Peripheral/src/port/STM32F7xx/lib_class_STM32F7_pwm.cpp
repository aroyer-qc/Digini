//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_pwm.cpp
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define PWM_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  PWM_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_PWM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TIM_CCMR1_OC1_MODE_PWM                 ((uint16_t)0x0060)
#define TIM_CCMR1_OC2_MODE_PWM                 ((uint16_t)0x6000)
#define TIM_CCMR2_OC3_MODE_PWM                 ((uint16_t)0x0060)
#define TIM_CCMR2_OC4_MODE_PWM                 ((uint16_t)0x6000)
#define TIM_CCER_OC1_MASK                      ((uint16_t)0x000F)
#define TIM_CCER_OC2_MASK                      ((uint16_t)0x00F0)
#define TIM_CCER_OC3_MASK                      ((uint16_t)0x0F00)
#define TIM_CCER_OC4_MASK                      ((uint16_t)0xF000)

//-------------------------------------------------------------------------------------------------
//
//   Class: PWM_Driver
//
//
//   Description:   Class to handle basic PWM functionality
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   PWM_Driver
//
//   Parameter(s):  PWM_ID                  ID for the data to use for this class
//
//   Description:   Initializes the PWM_Driver class
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
PWM_Driver::PWM_Driver(PWM_ChannelID_e PWM_ID)
{
    m_pInfo = &PWM_Info[PWM_ID];
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetDuty
//
//  Parameter(s):   Duty                New duty value
//                  Granularity         Calculate from this granularity
//  Return:         none
//
//  Description:    Set a new duty to the PWM
//
//-------------------------------------------------------------------------------------------------
void PWM_Driver::Initialize(void)
{
    m_pTimer = TIM_Driver::GetTimerPointer(m_pInfo->TimID);
    IO_PinInit(m_pInfo->PinID);

    switch(m_pInfo->Channel)
    {
        case TIM_CHANNEL_1:
        {
            m_pTimer->CCER  &= ~TIM_CCER_OC1_MASK;                            // Clear config for OC1
            m_pTimer->CCMR1 |=  (TIM_CCMR1_OC1_MODE_PWM | TIM_CCMR1_OC1PE);   // Set PWM mode and enable output
            m_pTimer->CCR1   =  m_pInfo->InitialDuty;                         // Set the Capture Compare Register value
            m_pTimer->CCER  |=  TIM_CCER_CC1E;                                // Set the Output State
        }
        break;

        case TIM_CHANNEL_2:
        {
            m_pTimer->CCER  &= ~TIM_CCER_OC2_MASK;                            // Clear config for OC2
            m_pTimer->CCMR1 |=  (TIM_CCMR1_OC2_MODE_PWM | TIM_CCMR1_OC2PE);   // Set PWM mode and enable output
            m_pTimer->CCR2   =  m_pInfo->InitialDuty;                         // Set the Capture Compare Register value
            m_pTimer->CCER  |=  TIM_CCER_CC2E;                                // Set the Output State
        }
        break;

        case TIM_CHANNEL_3:
        {
            m_pTimer->CCER  &= ~TIM_CCER_OC3_MASK;                            // Clear config for OC3
            m_pTimer->CCMR2 |=  (TIM_CCMR2_OC3_MODE_PWM | TIM_CCMR2_OC3PE);   // Set PWM mode and enable output
            m_pTimer->CCR3   =  m_pInfo->InitialDuty;                         // Set the Capture Compare Register value
            m_pTimer->CCER  |=  TIM_CCER_CC3E;                                // Set the Output State
        }
        break;

        case TIM_CHANNEL_4:
        {
            m_pTimer->CCER  &= ~TIM_CCER_OC4_MASK;                            // Clear config for OC4
            m_pTimer->CCMR2 |=  (TIM_CCMR2_OC4_MODE_PWM | TIM_CCMR2_OC4PE);   // Set PWM mode and enable output
            m_pTimer->CCR4   =  m_pInfo->InitialDuty;                         // Set the Capture Compare Register value
            m_pTimer->CCER  |=  TIM_CCER_CC4E;                                // Set the Output State
        }
        break;

        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetDuty
//
//  Parameter(s):   Duty                New duty value
//  Return:         none
//
//  Description:    Set a new duty to the PWM
//
//-------------------------------------------------------------------------------------------------
void PWM_Driver::SetDuty(uint16_t Duty)
{
    //uint32_t     Period = m_pTimer->ARR; ?? should calculate it from ARR

    switch(m_pInfo->Channel)
    {
        case TIM_CHANNEL_1: { m_pTimer->CCR1 = Duty; } break;
        case TIM_CHANNEL_2: { m_pTimer->CCR2 = Duty; } break;
        case TIM_CHANNEL_3: { m_pTimer->CCR3 = Duty; } break;
        case TIM_CHANNEL_4: { m_pTimer->CCR4 = Duty; } break;
        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Start
//
//  Parameter(s):   none
//  Return:         none
//
//  Description:    Start the PWM
//
//-------------------------------------------------------------------------------------------------
void PWM_Driver::Start(void)
{
    switch(m_pInfo->Channel)
    {
        case TIM_CHANNEL_1: { m_pTimer->CCER |= TIM_CCER_CC1E; } break;
        case TIM_CHANNEL_2: { m_pTimer->CCER |= TIM_CCER_CC2E; } break;
        case TIM_CHANNEL_3: { m_pTimer->CCER |= TIM_CCER_CC3E; } break;
        case TIM_CHANNEL_4: { m_pTimer->CCER |= TIM_CCER_CC4E; } break;
        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Stop
//
//  Parameter(s):   none
//  Return:         none
//
//  Description:    Stop the PWM
//
//-------------------------------------------------------------------------------------------------
void PWM_Driver::Stop(void)
{
    switch(m_pInfo->Channel)
    {
        case TIM_CHANNEL_1: { m_pTimer->CCER &= ~TIM_CCER_CC1E; } break;
        case TIM_CHANNEL_2: { m_pTimer->CCER &= ~TIM_CCER_CC2E; } break;
        case TIM_CHANNEL_3: { m_pTimer->CCER &= ~TIM_CCER_CC3E; } break;
        case TIM_CHANNEL_4: { m_pTimer->CCER &= ~TIM_CCER_CC4E; } break;
        default: break;
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_PWM_DRIVER == DEF_ENABLED)
