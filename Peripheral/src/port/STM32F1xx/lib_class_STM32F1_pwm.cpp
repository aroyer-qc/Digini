//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_pwm.cpp
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

// TODO compare to last F4 and F7 to sync methodology


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

// TODO simplify mode setting and also allow configuration into tim_var.h
#define TIM_CCMR1_OC1_MODE_1_PWM               ((uint16_t)0x0060)
#define TIM_CCMR1_OC1_MODE_2_PWM               ((uint16_t)0x0060)
#define TIM_CCMR1_OC2_MODE_1_PWM               ((uint16_t)0x6000)
#define TIM_CCMR1_OC2_MODE_2_PWM               ((uint16_t)0x6000)
#define TIM_CCMR2_OC3_MODE_1_PWM               ((uint16_t)0x0060)
#define TIM_CCMR2_OC3_MODE_2_PWM               ((uint16_t)0x0060)
#define TIM_CCMR2_OC4_MODE_1_PWM               ((uint16_t)0x6000)
#define TIM_CCMR2_OC4_MODE_2_PWM               ((uint16_t)0x6000)

#define TIM_CCER_OC1_MASK                      ((uint16_t)0x000F)
#define TIM_CCER_OC2_MASK                      ((uint16_t)0x00F0)
#define TIM_CCER_OC3_MASK                      ((uint16_t)0x0F00)
#define TIM_CCER_OC4_MASK                      ((uint16_t)0xF000)

#define TIM_CCMR1_OC1_MASK                     ((uint16_t)0x00FF)
#define TIM_CCMR1_OC2_MASK                     ((uint16_t)0xFF00)
#define TIM_CCMR2_OC3_MASK                     ((uint16_t)0x00FF)
#define TIM_CCMR2_OC4_MASK                     ((uint16_t)0xFF00)

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
    m_pTimer = new TIM_Driver(m_pInfo->TimID);
    m_pTimer->Initialize();
    m_pTim   = m_pTimer->GetTimerPointer();
    IO_PinInit(m_pInfo->PinID);

    switch(m_pInfo->Channel)
    {
        case PWM_CHANNEL_1:
        {
            m_pTim->CCER  &= ~TIM_CCER_OC1_MASK;            // Clear config for OC1
            m_pTim->CCMR1 |= (TIM_CCMR1_OC1_MODE_PWM);      // Set PWM mode
            m_pTim->CCR1   = m_pInfo->InitialDuty;          // Set the Capture Compare Register value
        }
        break;

        case PWM_CHANNEL_2:
        {
            m_pTim->CCER  &= ~TIM_CCER_OC2_MASK;            // Clear config for OC2
            m_pTim->CCMR1 |= (TIM_CCMR1_OC2_MODE_PWM);      // Set PWM mode
            m_pTim->CCR2   = m_pInfo->InitialDuty;          // Set the Capture Compare Register value
        }
        break;

        case PWM_CHANNEL_3:
        {
            m_pTim->CCER  &= ~TIM_CCER_OC3_MASK;            // Clear config for OC3
            m_pTim->CCMR2 |= (TIM_CCMR2_OC3_MODE_PWM);      // Set PWM mode
            m_pTim->CCR3   = m_pInfo->InitialDuty;          // Set the Capture Compare Register value
        }
        break;

        case PWM_CHANNEL_4:
        {
            m_pTim->CCER  &= ~TIM_CCER_OC4_MASK;             // Clear config for OC4
            m_pTim->CCMR2 |= (TIM_CCMR2_OC4_MODE_PWM);       // Set PWM mode
            m_pTim->CCR4   = m_pInfo->InitialDuty;           // Set the Capture Compare Register value
        }
        break;

        default: return;
    }

    m_pTim->CCER |=  m_pInfo->Output;                        // Set the Output
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
    //uint32_t     Period = m_pTimer->ARR; ?? should calculate it from ARR
    switch(m_pInfo->Channel)
    {
        case PWM_CHANNEL_1: { m_pTim->CCR1 = Duty; } break;
        case PWM_CHANNEL_2: { m_pTim->CCR2 = Duty; } break;
        case PWM_CHANNEL_3: { m_pTim->CCR3 = Duty; } break;
        case PWM_CHANNEL_4: { m_pTim->CCR4 = Duty; } break;
        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Start
//
//  Parameter(s):   none
//  Return:         void
//
//  Description:    Start the PWM
//
//-------------------------------------------------------------------------------------------------
void PWM_Driver::Start(void)
{
    m_pTimer->Start();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Stop
//
//  Parameter(s):   none
//  Return:         void
//
//  Description:    Stop the PWM
//
//-------------------------------------------------------------------------------------------------
void PWM_Driver::Stop(void)
{
    m_pTimer->Stop();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetCompareRegisterPointer
//
//  Parameter(s):   uint32_t*
//
//  Description:    Return the address pointer of the compare register
//
//  Note(s):        Useful for DMA configuration
//
//-------------------------------------------------------------------------------------------------
volatile uint32_t* PWM_Driver::GetCompareRegisterPointer(void)
{
    switch(m_pInfo->Channel)
    {
        case (PWM_CHANNEL_1): return &m_pTim->CCR1;
        case (PWM_CHANNEL_2): return &m_pTim->CCR2;
        case (PWM_CHANNEL_3): return &m_pTim->CCR3;
        case (PWM_CHANNEL_4): return &m_pTim->CCR4;
        default:              return nullptr;
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_PWM_DRIVER == DEF_ENABLED)
