//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_pwm.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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
    TIM_Compare_e Channel = m_pInfo->Channel;
    
    m_pTimer = TIM_Driver::GetTimerPointer(m_pInfo->TimID);
    m_pTimer->Initialize();
    IO_PinInit(m_pInfo->PinID);
    ClearConfigCompareChannel(Channel);

    // Set PWM mode and enable output
    switch(Channel)
    {
        case TIM_CHANNEL_1: { m_pTim->CCMR1 |= (TIM_CCMR1_OC1_MODE_PWM | TIM_CCMR1_OC1PE); } break;
        case TIM_CHANNEL_2: { m_pTim->CCMR1 |= (TIM_CCMR1_OC2_MODE_PWM | TIM_CCMR1_OC2PE); } break;
        case TIM_CHANNEL_3: { m_pTim->CCMR2 |= (TIM_CCMR2_OC3_MODE_PWM | TIM_CCMR2_OC3PE); } break;
        case TIM_CHANNEL_4: { m_pTim->CCMR2 |= (TIM_CCMR2_OC4_MODE_PWM | TIM_CCMR2_OC4PE); } break;
        default: break;
    }
    
    m_pTimer->SetCompareChannel(Channel, m_pInfo->InitialDuty);
    m_pTimer->EnableCompareChannel(Channel);
    m_pTim->BDTR |= TIM_BDTR_MOE;
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
    m_pTimer->SetCompareChannel(m_pInfo->Channel, Duty);
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
    m_pTimer->EnableCompareChannel(m_pInfo->Channel);
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
    m_pTimer->DisableCompareChannel(m_pInfo->Channel);
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_PWM_DRIVER == DEF_ENABLED)
