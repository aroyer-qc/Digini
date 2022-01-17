//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_tim.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define TIM_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  TIM_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_TIM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// define(s)
//-------------------------------------------------------------------------------------------------

#define TIM_GLOBAL
#define TIM_BACK_OFFSET_RESET_REGISTER             0x20
#define TIM_16_BITS_MASK                           0xFFFF
#define TIM_16_BITS_MAX_COMPARE                    0xFFFF
#define TIM_32_BITS_MAX_COMPARE                    0xFFFFFFFF
#define TIM_CCER_OUTPUT_ENABLE_SPACING_SHIFT       4
#define TIM_CCMRx_MODE_SHIFT                       8
#define TIM_CCMRx_MODE_PWM_MASK                    0x0060

//-------------------------------------------------------------------------------------------------
//
//   Class: TIM_Driver
//
//
//   Description:   Class to handle basic TIM functionality
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   TIM_Driver
//
//   Parameter(s):  TimID                  ID for the data to use for this class
//
//   Description:   Initializes the TIM_Driver class
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
TIM_Driver::TIM_Driver(TIM_ID_e TimID)
{
    m_pInfo = (TIM_Info_t*)&TIM_Info[TimID];
    m_pTim  = m_pInfo->pTIMx;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   pCallBack       Callback for application.
//  Return:         None
//
//  Description:    Configure the module
//
//-------------------------------------------------------------------------------------------------
void TIM_Driver::Initialize(void)
{
    ISR_Prio_t ISR_Prio;

    // Reset and Enable clock module
    *(uint32_t*)((uint32_t)m_pInfo->RCC_APBxEN_Register - TIM_BACK_OFFSET_RESET_REGISTER) |=  m_pInfo->RCC_APBxPeriph;
    *(uint32_t*)((uint32_t)m_pInfo->RCC_APBxEN_Register - TIM_BACK_OFFSET_RESET_REGISTER) &= ~m_pInfo->RCC_APBxPeriph;
    *(m_pInfo->RCC_APBxEN_Register) |= m_pInfo->RCC_APBxPeriph;

    // Set the prescaler value
    m_pTim->PSC = m_pInfo->Prescaler;

    // Set the auto reload register
  #if (TIM_DRIVER_SUPPORT_16_BITS_TIM_CFG == DEF_ENABLED)
    if((m_pTim != TIM2) && (m_pTim != TIM5))
    {
        MODIFY_REG(m_pInfo->Reload, TIM_16_BITS_MASK, m_pInfo->Reload); // Keep only low 16 bits
    }
  #endif

    m_pTim->ARR = m_pInfo->Reload;
    m_pTim->CNT = m_pInfo->Reload - 1;      // Prevent PWM from been active on activation

    // Set mode and Auto-reload preload enable
  #if (TIM_DRIVER_SUPPORT_BASIC_TIM_CFG == DEF_ENABLED)
    if((m_pTim == TIM6) || (m_pTim == TIM7))
    {
        Mode = 0;
    }
  #endif

    ((TIM_TypeDef*)m_pTim)->CR1 = m_pInfo->Mode | TIM_CR1_ARPE;

    // Set the update interrupt enable
    if(m_pInfo->EnableUpdateIRQ == true)
    {
         CLEAR_BIT(((TIM_TypeDef*)m_pTim)->SR, TIM_SR_UIF);
        ((TIM_TypeDef*)m_pTim)->DIER = TIM_DIER_UIE;
    }

    // Configure interrupt priority for TIM
    ISR_Prio.PriorityGroup     = NVIC_GetPriorityGrouping();
    ISR_Prio.PremptionPriority = m_pInfo->PreempPrio;
    ISR_Prio.SubPriority       = 0;
    ISR_Init(m_pInfo->IRQn_Channel, &ISR_Prio);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterCallBack
//
//  Parameter(s):   pCallBack       Callback for application.
//  Return:         None
//
//  Description:    Set callback for application.
//
//-------------------------------------------------------------------------------------------------
void TIM_Driver::RegisterCallBack(TIM_CallBack_t pCallBack)
{
    m_pCallBack = pCallBack;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TimeBaseToPrescaler
//
//  Parameter(s):   TimeBase        Timebase precision required in 1 uSec.
//  Return:         int32_t         Prescaler value
//
//  Description:    Calculate prescaler for the specified time base
//
//-------------------------------------------------------------------------------------------------
uint32_t TIM_Driver::TimeBaseToPrescaler(uint32_t TimeBase)
{
    VAR_UNUSED(TimeBase);
    // if requested
    // TODO (Alain#2#) a function to calculate right value for a requested time period
    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Start the timer
//
//-------------------------------------------------------------------------------------------------
void TIM_Driver::Start(void)
{
    SET_BIT(m_pTim->CR1, TIM_CR1_CEN);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReStart
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Restart the timer from beginning
//
//-------------------------------------------------------------------------------------------------
void TIM_Driver::ReStart(void)
{
    CLEAR_BIT(m_pTim->CR1, TIM_CR1_CEN);                          // Stop the timer
    m_pTim->CNT = 0;                                              // Reset counter
    SET_BIT(m_pTim->CR1, TIM_CR1_CEN);                            // Start the timer
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Stop
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Stop the timer
//
//-------------------------------------------------------------------------------------------------
void TIM_Driver::Stop(void)
{
    CLEAR_BIT(m_pTim->CR1, TIM_CR1_CEN);                            // Stop any other timer
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetReload
//
//  Parameter(s):   Value           New reload value
//  Return:         void
//
//  Description:    Reload a new period for timer and restart it
//
//-------------------------------------------------------------------------------------------------
void TIM_Driver::SetReload(uint32_t Value)
{
    CLEAR_BIT(m_pTim->CR1, TIM_CR1_CEN);                            // Stop the timer
    m_pTim->ARR = Value;                                            // Reload a new value
    m_pTim->CNT = 0;                                                // Reset counter
    SET_BIT(m_pTim->CR1, TIM_CR1_CEN);                              // Start the timer
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetReload
//
//  Parameter(s):   Value           New reload value
//  Return:         void
//
//  Description:    Reload a new period for timer and restart it
//
//-------------------------------------------------------------------------------------------------
uint32_t TIM_Driver::GetReload(void)
{
    return m_pTim->ARR;                                             // Get Reload value
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetCompare
//
//  Parameter(s):   Channel         STM32F4: Compare 1 to 4
//                                        - TIM_COMPARE_CH1
//                                        - TIM_COMPARE_CH2
//                                        - TIM_COMPARE_CH3
//                                        - TIM_COMPARE_CH4
//                  Value           Compare value (must be lower than reload value)
//  Return:         void
//
//  Description:    Configure one of the compare register (Give more time slice option)
//
//  Note(s):        Callback for this channel will be enabled
//                  Compare 1 to 4 exist for TIM1 to TIM5, TIM8
//                  Compare 1 and 2 exist for TIM9, TIM12
//                  No compare on TIM6 and TIM7
//                  Only ONE compare on timer TIM10, TIM11, TIM13, TIM14
//
//-------------------------------------------------------------------------------------------------
#if (TIM_DRIVER_SUPPORT_COMPARE_FEATURE_CFG == DEF_ENABLED)
void TIM_Driver::SetCompare(TIM_Channel_e Channel, uint32_t Value)
{
    uint32_t     BitMask;

  #if ((TIM_DRIVER_SUPPORT_TIM1_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM1_COMPARE_CFG == DEF_ENABLED)) || \
      ((TIM_DRIVER_SUPPORT_TIM8_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM8_COMPARE_CFG == DEF_ENABLED))
    ISR_Prio_t  ISR_Prio;

    // Configure interrupt priority for TIM
    ISR_Prio.PriorityGroup     = NVIC_GetPriorityGrouping();
    ISR_Prio.PremptionPriority = 6;
    ISR_Prio.SubPriority       = 0;
  #endif

    switch(int(m_pTim))
    {

      #if ((TIM_DRIVER_SUPPORT_TIM1_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM1_COMPARE_CFG == DEF_ENABLED))
        case int(TIM1):
        {
            // Init compare IRQ for TIM1 Compare
            ISR_Init(TIM1_CC_IRQn, &ISR_Prio);
        }
        break;
      #endif

      #if (TIM_DRIVER_SUPPORT_TIM6_CFG == DEF_ENABLED)
        case int(TIM6):
      #endif
      #if (TIM_DRIVER_SUPPORT_TIM7_CFG == DEF_ENABLED)
        case int(TIM7):
      #endif
      #if ((TIM_DRIVER_SUPPORT_TIM6_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM7_CFG == DEF_ENABLED))
        {
            return;  // no compare for TIM6 and TIM7
        }
      #endif

      #if ((TIM_DRIVER_SUPPORT_TIM8_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM8_COMPARE_CFG == DEF_ENABLED))
        case int(TIM8):
        {
            // Init compare IRQ for TIM8 Compare
            ISR_Init(TIM8_CC_IRQn, &ISR_Prio);
        }
        break;
      #endif

      #if ((TIM_DRIVER_SUPPORT_TIM9_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM9_OR_TIM12_COMPARE_CFG == DEF_ENABLED))
        case int(TIM9):
      #endif
      #if ((TIM_DRIVER_SUPPORT_TIM12_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM9_OR_TIM12_COMPARE_CFG == DEF_ENABLED))
        case int(TIM12):
      #endif
      #if (((TIM_DRIVER_SUPPORT_TIM9_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM12_CFG == DEF_ENABLED)) && \
      (TIM_DRIVER_SUPPORT_TIM9_OR_TIM12_COMPARE_CFG  == DEF_ENABLED))
        {
            if(Channel > TIM_COMPARE_CH2)
            {
                return;
            }
        }
        break;
      #endif

      #if ((TIM_DRIVER_SUPPORT_TIM10_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM10_COMPARE_CFG == DEF_ENABLED))
        case int(TIM10):
      #endif
      #if ((TIM_DRIVER_SUPPORT_TIM11_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM11_COMPARE_CFG == DEF_ENABLED))
        case int(TIM11):
      #endif
      #if ((TIM_DRIVER_SUPPORT_TIM13_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM13_COMPARE_CFG == DEF_ENABLED))
        case int(TIM13):
      #endif
      #if ((TIM_DRIVER_SUPPORT_TIM14_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM14_COMPARE_CFG == DEF_ENABLED))
        case int(TIM14):
      #endif
      #if (((TIM_DRIVER_SUPPORT_TIM10_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM10_COMPARE_CFG == DEF_ENABLED)) || \
           ((TIM_DRIVER_SUPPORT_TIM11_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM11_COMPARE_CFG == DEF_ENABLED)) || \
           ((TIM_DRIVER_SUPPORT_TIM13_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM13_COMPARE_CFG == DEF_ENABLED)) || \
           ((TIM_DRIVER_SUPPORT_TIM14_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM14_COMPARE_CFG == DEF_ENABLED)))
        {
            if(Channel > TIM_COMPARE_CH1)
            {
                return;
            }
        }
        break;
      #endif
    }

    switch(Channel)
    {
        case TIM_COMPARE_CH1:
        {
            BitMask = TIM_DIER_CC1IE;
            m_pTim->CCR1 = Value;
        }
        break;

        case TIM_COMPARE_CH2:
        {
            BitMask = TIM_DIER_CC2IE;
            m_pTim->CCR2 = Value;
        }
        break;

        case TIM_COMPARE_CH3:
        {
            BitMask = TIM_DIER_CC3IE;
            m_pTim->CCR3 = Value;
        }
        break;

        case TIM_COMPARE_CH4:
        {
            BitMask = TIM_DIER_CC4IE;
            m_pTim->CCR4 = Value;
        }
        break;

        default: break;
    }

    MODIFY_REG(m_pTim->SR, BitMask, 0);
    SET_BIT(m_pTim->DIER, BitMask);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetCounterValue
//
//  Parameter(s):   None
//  Return:         Value
//
//  Description:    Get the actual counter value
//
//-------------------------------------------------------------------------------------------------
uint32_t TIM_Driver::GetCounterValue(void)
{
    return m_pTim->CNT;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetCounterValue
//
//  Parameter(s):   TimeBase        Timebase precision required in 1 uSec.
//  Return:         void
//
//  Description:    Configure a PWM channel
//
//-------------------------------------------------------------------------------------------------
#if (TIM_DRIVER_SUPPORT_PWM_FEATURE_CFG == DEF_ENABLED)
void TIM_Driver::ConfigPWM_Channel(TIM_Channel_e Channel)
{
    int IntegerChannel;

    IntegerChannel = ((int)Channel - 1);
    m_pTim->CCER |= (1 << (IntegerChannel * TMR_CCER_OUTPUT_ENABLE_SPACING_SHIFT));

    switch(Channel)
    {
        case TIM_COMPARE_CH1:
        case TIM_COMPARE_CH2:
        {
            IntegerChannel = Channel - TIM_COMPARE_CH1;
            m_pTim->CCMR1 |= (TMR_CCMRx_MODE_PWM_MASK << (IntegerChannel * TMR_CCMRx_MODE_SHIFT));
        }
        break;

        case TIM_COMPARE_CH3:
        case TIM_COMPARE_CH4:
        {
            IntegerChannel = Channel - TIM_COMPARE_CH3;
            m_pTim->CCMR2 |= (TMR_CCMRx_MODE_PWM_MASK << (IntegerChannel * TMR_CCMRx_MODE_SHIFT));
        }
        break;
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CallBack
//
//  Parameter(s):   ProcessUpdate   Do we process the update flag (special case for TIM1 and TIM8)
//  Return:         void
//
//  Description:    Call each compare IRQ for each interrupt flag set
//
//-------------------------------------------------------------------------------------------------
void TIM_Driver::CallBack(bool ProcessUpdate)
{
    uint32_t Status = m_pTim->SR;
    uint32_t Enable = m_pTim->DIER;

  #if (TIM_DRIVER_SUPPORT_COMPARE_FEATURE_CFG != DEF_ENABLED)
    VAR_UNUSED(ProcessUpdate);
  #endif

    if(m_pCallBack != nullptr)
    {
        Status &= Enable;

      #if (TIM_USE_COMPARE_FEATURE_CFG == DEF_ENABLED)
        if(((Status & TIM_SR_CC1IF) && (m_pTim->DIER & TIM_DIER_CC1IE)) != 0)
        {
            m_pCallBack(TIM_MATCH_CH1);
            MODIFY_REG(m_pTim->SR, TIM_SR_CC1IF, 0);
        }

        if(((Status & TIM_SR_CC2IF) && (m_pTim->DIER & TIM_DIER_CC2IE)) != 0)
        {
            m_pCallBack(TIM_MATCH_CH2);
            MODIFY_REG(m_pTim->SR, TIM_SR_CC2IF, 0);
        }

        if(((Status & TIM_SR_CC3IF) && (m_pTim->DIER & TIM_DIER_CC3IE)) != 0)
        {
            m_pCallBack(TIM_MATCH_CH3);
            MODIFY_REG(m_pTim->SR, TIM_SR_CC3IF, 0);
        }

        if(((Status & TIM_SR_CC4IF) && (m_pTim->DIER & TIM_DIER_CC4IE)) != 0)
        {
            m_pCallBack(TIM_MATCH_CH4);
            MODIFY_REG(m_pTim->SR, TIM_SR_CC4IF, 0);
        }
      #endif

        if((Status & TIM_SR_UIF) != 0)
        {
          #if (TIM_USE_COMPARE_FEATURE_CFG == DEF_ENABLED)
            if(ProcessUpdate == true)
          #endif
            {
                m_pCallBack(TIM_MATCH_UPDATE);
                MODIFY_REG(m_pTim->SR, TIM_SR_UIF, 0);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetTimerPointer
//
//  Parameter(s):   TimID           ID of the timer to get the pointer
//  Return:         TIM_TypeDef*    \pointer on the timer module
//
//  Description:    Return the pointer on the timer use by this ID
//
//-------------------------------------------------------------------------------------------------
TIM_TypeDef* TIM_Driver::GetTimerPointer(TIM_ID_e TimID)
{
    return TIM_Info[TimID].pTIMx;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_TIM_DRIVER == DEF_ENABLED)
