//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_tim.cpp
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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TIM_BACK_OFFSET_RESET_REGISTER             0x20
#define TIM_16_BITS_MASK                           0xFFFF

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

  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(m_pTim == LPTIM1)
    {
        ((LPTIM_TypeDef*)m_pTim)->ARR = m_pInfo->Reload;
    }
    else
  #endif
    {
        m_pTim->ARR = m_pInfo->Reload;
        m_pTim->CNT = m_pInfo->Reload - 1;      // Prevent PWM from been active on activation
    }

    // Set mode and Auto-reload preload enable
  #if (TIM_DRIVER_SUPPORT_BASIC_TIM_CFG == DEF_ENABLED)
    if((m_pTim == TIM6) || (m_pTim == TIM7))
    {
        Mode = 0;
    }
  #endif

  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(m_pTim == LPTIM1)
    {
        if m_pInfo->Mode == TMR_OPMODE_REPETITIVE)
        {
            m_LPTIM1_ContinuousMode = true;
        }
        else
        {
            m_LPTIM1_ContinuousMode = false;
        }

        SET_BIT(LPTIM1->CFGR, LPTIM_CFGR_PRELOAD);
    }
    else
  #endif
    {
        ((TIM_TypeDef*)m_pTim)->CR1 = m_pInfo->Mode | TIM_CR1_ARPE;
    }

    // Set the update interrupt enable
    if(m_pInfo->IRQ_DMA_SourceEnable & (TIM_IRQ_UPDATE | TIM_DMA_UPDATE) != 0)
//    if(m_pInfo->EnableUpdateIRQ == true)
    {
      #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
        if(m_pTim == LPTIM1)
        {
            SET_BIT(LPTIM1->ICR, LPTIM_ICR_ARRMCF);
            SET_BIT(LPTIM1->IER, LPTIM_IER_ARRMIE);
        }
        else
      #endif
        {
             CLEAR_BIT(((TIM_TypeDef*)m_pTim)->SR, TIM_SR_UIF);
            ((TIM_TypeDef*)m_pTim)->DIER = TIM_DIER_UIE;
        }
    }

    // Configure interrupt priority for TIM
    if(m_pInfo->IRQn_Channel != ISR_IRQn_NONE)
    {
        ISR_Init(m_pInfo->IRQn_Channel, m_pInfo->PreempPrio);
    }
}

#if 0 //not in F4.. Does not mean we don't keep it
//-------------------------------------------------------------------------------------------------
//
//  Name:           IsItRunning
//
//  Parameter(s):   None
//  Return:         true or false
//
//  Description:    Check if timer is enable.
//
//-------------------------------------------------------------------------------------------------
bool TIM_Driver::IsItRunning(void)
{
    return (LPTIM1->CR & LPTIM_CR_ENABLE) ? true : false;
}
#endif

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
  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(((LPTIM_TypeDef*)m_pTim) == LPTIM1)
    {
        SET_BIT(LPTIM1->CR, LPTIM_CR_ENABLE);                   // Start the timer LPTIM1

        if(m_LPTIM1_ContinuousMode == true)
        {
            SET_BIT(LPTIM1->CR, LPTIM_CR_CNTSTRT);
        }
        else
        {
            SET_BIT(LPTIM1->CR, LPTIM_CR_SNGSTRT);
        }
    }
    else
  #endif
    {
      #if (TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG == DEF_ENABLED)
        SET_BIT(m_pTim->CR1, TIM_CR1_CEN);                    // Start any other timer
      #endif
    }
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
  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(((LPTIM_TypeDef*)m_pTim) == LPTIM1)
    {
        CLEAR_BIT(LPTIM1->CR, LPTIM_CR_ENABLE);                         // Stop the timer LPTIM1
        LPTIM1->CNT = 0;
        SET_BIT(LPTIM1->CR, LPTIM_CR_ENABLE);                           // Start the timer LPTIM1
    }
    else
  #endif
    {
      #if (TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pTim->CR1, TIM_CR1_CEN);                          // Stop the timer
        m_pTim->CNT = 0;                                              // Reset counter
        SET_BIT(m_pTim->CR1, TIM_CR1_CEN);                            // Start the timer
      #endif
    }
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
  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(((LPTIM_TypeDef*)m_pTim) == LPTIM1)
    {
        CLEAR_BIT(LPTIM1->CR, LPTIM_CR_ENABLE);                         // Stop the timer LPTIM1
    }
    else
  #endif
    {
      #if (TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pTim->CR1, TIM_CR1_CEN);                            // Stop any other timer
      #endif
    }
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
  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(((LPTIM_TypeDef*)m_pTim) == LPTIM1)
    {
        CLEAR_BIT(LPTIM1->CR, LPTIM_CR_ENABLE);                         // Stop the timer LPTIM1
        LPTIM1->ARR = Value;                                            // Reload a new value
        LPTIM1->CNT = 0;                                                // Reset counter
        SET_BIT(LPTIM1->CR, LPTIM_CR_ENABLE);                           // Start the timer LPTIM1
    }
    else
  #endif
    {
      #if (TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pTim->CR1, TIM_CR1_CEN);                            // Stop the timer
        m_pTim->ARR = Value;                                            // Reload a new value
        m_pTim->CNT = 0;                                                // Reset counter
        SET_BIT(m_pTim->CR1, TIM_CR1_CEN);                              // Start the timer
      #endif
    }
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
  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(((LPTIM_TypeDef*)m_pTim) == LPTIM1)
    {
	return LPTIM1->ARR;                                             // Get Reload value
    }
    else
  #endif
    {
      #if (TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG == DEF_ENABLED)
        return m_pTim->ARR;                                                 // Get Reload value
      #endif
    }
}

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
  #if (TIM_DRIVER_SUPPORT_LPTIM1_CFG == DEF_ENABLED)
    if(((LPTIM_TypeDef*)m_pTim) == LPTIM1)
    {
        return LPTIM1->CNT;
    }
    else
  #endif
    {
      #if (TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG == DEF_ENABLED)
        return m_pTim->CNT;
      #endif
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
