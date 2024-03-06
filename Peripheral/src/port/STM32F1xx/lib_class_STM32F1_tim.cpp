//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_tim.cpp
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

#define TIM_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  TIM_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_TIM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TIM_BACK_OFFSET_RESET_REGISTER             0x20

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
    // Reset and Enable clock module
    *(uint32_t*)((uint32_t)m_pInfo->RCC_APBxEN_Register - TIM_BACK_OFFSET_RESET_REGISTER) |=  m_pInfo->RCC_APBxPeriph;
    *(uint32_t*)((uint32_t)m_pInfo->RCC_APBxEN_Register - TIM_BACK_OFFSET_RESET_REGISTER) &= ~m_pInfo->RCC_APBxPeriph;
    *(m_pInfo->RCC_APBxEN_Register) |= m_pInfo->RCC_APBxPeriph;

    m_pTim->PSC = m_pInfo->Prescaler;                               // Set the prescaler value
    m_pTim->ARR = m_pInfo->Reload;                                  // Set the auto reload register
    m_pTim->CNT = m_pInfo->Reload - 1;                              // Prevent PWM from been active on activation
    m_pTim->CR1 = m_pInfo->Mode | TIM_CR1_ARPE;

    // Set the update interrupt enable
    if((m_pInfo->IRQ_DMA_SourceEnable & (TIM_IRQ_UPDATE | TIM_DMA_UPDATE)) != 0)
    {
         CLEAR_BIT(((TIM_TypeDef*)m_pTim)->SR, TIM_SR_UIF);
        ((TIM_TypeDef*)m_pTim)->DIER = TIM_DIER_UIE;
    }

    ((TIM_TypeDef*)m_pTim)->DIER |= m_pInfo->IRQ_DMA_SourceEnable;  // Enable all source according to configuration

    if(m_pInfo->IRQn_Channel != ISR_IRQn_NONE)
    {
        ISR_Init(m_pInfo->IRQn_Channel, 0, m_pInfo->PreempPrio);    // Configure interrupt priority for TIM
    }
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
/*
uint32_t TIM_Driver::TimeBaseToPrescaler(uint32_t TimeBase)
{
    VAR_UNUSED(TimeBase);
    // if requested
    // TODO (Alain#2#) a function to calculate right value for a requested time period
    return 0;
}
*/
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
//  Name:           GetReload
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
//  Parameter(s):   Channel         STM32F1: Compare 1 to 4
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
//                  Compare 1 and 2 exist for TIM9, TIM12                       NO support yet
//                  No compare on TIM6 and TIM7
//                  Only ONE compare on timer TIM10, TIM11, TIM13, TIM14        NO support yet
//
//-------------------------------------------------------------------------------------------------
#if (TIM_DRIVER_SUPPORT_COMPARE_FEATURE_CFG == DEF_ENABLED)
void TIM_Driver::SetCompare(TIM_Compare_e Channel, uint32_t Value)
{
    uint32_t BitMask;

    switch(uintptr_t(m_pTim))
    {
        // Add other case for TIMx with compare IRQ as needed.

      #if ((TIM_DRIVER_SUPPORT_TIM1_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM1_COMPARE_CFG == DEF_ENABLED))
        case TIM1_BASE:
        {
          #if ( defined (STM32F100xB) || defined (STM32F100xE) ||                                                   \
                defined (STM32F103x6) || defined (STM32F103xB) || defined (STM32F103xE) || defined (STM32F103xG) || \
                defined (STM32F105xC) || defined (STM32F107xC) )
            // Init compare IRQ for TIM1 Compare
            ISR_Init(TIM1_CC_IRQn, 0, 6);
          #endif

          // Fill this for other CPU as needed
        }
        break;
      #endif

      #if ((TIM_DRIVER_SUPPORT_TIM8_CFG == DEF_ENABLED) && (TIM_DRIVER_SUPPORT_TIM8_COMPARE_CFG == DEF_ENABLED))
        case TIM8_BASE:
        {
          #if ( defined (STM32F103xE) || defined (STM32F103xG) )
            // Init compare IRQ for TIM8 Compare
            ISR_Init(TIM8_CC_IRQn, 0, 6);
          #endif

          // Fill this for other CPU as needed
        }
        break;
      #endif

        default:
        {
            return;
        }
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
//  Name:           GetPointerCompareRegister
//
//  Parameter(s):   Channel         STM32F1: Compare 1 to 4
//                                        - TIM_COMPARE_CH1
//                                        - TIM_COMPARE_CH2
//                                        - TIM_COMPARE_CH3
//                                        - TIM_COMPARE_CH4
//  Return:         uint32_t*
//
//  Description:    Return the address pointer of the compare register
//
//  Note(s):        Useful for DMA configuration
//
//-------------------------------------------------------------------------------------------------
#if (TIM_DRIVER_SUPPORT_COMPARE_FEATURE_CFG == DEF_ENABLED)
uint32_t* TIM_Driver::GetPointerCompareRegister(TIM_Compare_e Channel)
{
    switch(Channel)
    {
        case TIM_COMPARE_CH1: return &m_pTim->CCR1;
        case TIM_COMPARE_CH2: return &m_pTim->CCR2;
        case TIM_COMPARE_CH3: return &m_pTim->CCR3;
        case TIM_COMPARE_CH4: return &m_pTim->CCR4;
        default:              return nullptr;
    }
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

/*
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
void TIM_Driver::ConfigPWM_Channel(TIM_Compare_e Channel)
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
*/

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
