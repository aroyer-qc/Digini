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
#include "./Digini/lib_digini.h"
#undef  TIM_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_TIM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TIM_BACK_OFFSET_RESET_REGISTER             0x0C

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

    m_pTim->PSC = m_pInfo->Prescaler - 1;                           // Set the prescaler value
    m_pTim->ARR = m_pInfo->Reload;                                  // Set the auto reload register
    m_pTim->CNT = 0;//m_pInfo->Reload - 1;                              // Prevent PWM from been active on activation
    m_pTim->CR1 = m_pInfo->Mode | TIM_CR1_ARPE;

    // Set the update interrupt enable
    if((m_pInfo->IRQ_DMA_SourceEnable & (TIM_IRQ_UPDATE | TIM_DMA_UPDATE)) != 0)
    {
         CLEAR_BIT(((TIM_TypeDef*)m_pTim)->SR, TIM_SR_UIF);
        ((TIM_TypeDef*)m_pTim)->DIER = TIM_DIER_UIE;
    }

    ((TIM_TypeDef*)m_pTim)->DIER |= m_pInfo->IRQ_DMA_SourceEnable;  // Enable all source according to configuration

    // Configure interrupt priority for TIM
    if(m_pInfo->IRQn_Channel != ISR_IRQn_NONE)
    {
        ISR_Init(m_pInfo->IRQn_Channel, m_pInfo->PreempPrio);    // Configure interrupt priority for TIM
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
//  Name:           GetTimerPointer
//
//  Parameter(s):   None
//  Return:         TIM_TypeDef*    Pointer on the timer module
//
//  Description:    Return the pointer on the timer
//
//-------------------------------------------------------------------------------------------------
TIM_TypeDef* TIM_Driver::GetTimerPointer(void)
{
    return m_pTim;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_TIM_DRIVER == DEF_ENABLED)
