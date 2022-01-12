//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_adc.cpp
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

#define ADC_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  ADC_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_ADC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

bool ADC_Driver::m_CommonInitialize_IsItDone = false;

//-------------------------------------------------------------------------------------------------
//
//   Class: ADC_Driver
//
//
//   Description:   Class to handle ADC_Driver
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   ADC_Driver
//
//   Parameter(s):  ADC_ID_e
//   Return Value:
//
//   Description:   Initializes the ADC_ID peripheral according to the specified Parameters
//
//-------------------------------------------------------------------------------------------------
ADC_Driver::ADC_Driver(ADC_ID_e ADC_ID)
{
    m_IsItInitialize = false;
    m_pInfo          = &ADC_Info[ADC_ID];
    m_State          = SYS_DEVICE_NOT_PRESENT;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   None
//  Return:         none
//
//  Description:    Initializes the ADCx peripheral according to the specified parameters in the
//                  ADC configuration file.
//  Note(s):        This function is used to configure the global features of the ADC
//                      - ClockPrescaler
//                      - Resolution
//                      - Data Alignment
//                      - Number of conversion
//                  however, the rest of the configuration parameters are specific to the regular
//                  channels group (scan mode activation, continuous mode activation, External
//                  trigger source and edge, DMA continuous request after the last transfer and End
//                  of conversion selection).
//
//-------------------------------------------------------------------------------------------------

void ADC_Driver::Initialize(void)
{
    ISR_Prio_t    ISR_Prio;
    uint32_t      PriorityGroup;
    //SystemState_e State         = SYS_READY;

    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;
        // Allocate lock resource and initialize it
        /*Error =*/ nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
    }

    if(m_CommonInitialize_IsItDone == false)
    {
        // ADC interrupt are common to all module
        PriorityGroup = NVIC_GetPriorityGrouping();
        ISR_Prio.PriorityGroup     = PriorityGroup;
        ISR_Prio.SubPriority       = 0;
        ISR_Prio.PremptionPriority = 0;
        ISR_Init(ADC_IRQn, &ISR_Prio);

        // Set the ADC clock prescaler
        ADC->CCR &= ~(ADC_CCR_ADCPRE);
        ADC->CCR |= ADC_COMMON_CLOCK_PRESCALER_CONFIG;

        m_CommonInitialize_IsItDone = true;
    }

    SET_BIT(RCC->APB2ENR, m_pInfo->RCC_APB2_En);         // Enable lock for this ADC





/*
if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    // Init the ADC Callback settings
    hadc->ConvCpltCallback              = HAL_ADC_ConvCpltCallback;                 // Legacy weak callback
    hadc->ConvHalfCpltCallback          = HAL_ADC_ConvHalfCpltCallback;             // Legacy weak callback
    hadc->LevelOutOfWindowCallback      = HAL_ADC_LevelOutOfWindowCallback;         // Legacy weak callback
    hadc->ErrorCallback                 = HAL_ADC_ErrorCallback;                    // Legacy weak callback
    hadc->InjectedConvCpltCallback      = HAL_ADCEx_InjectedConvCpltCallback;       // Legacy weak callback
    if (hadc->MspInitCallback == NULL)
    {
      hadc->MspInitCallback = HAL_ADC_MspInit; // Legacy weak MspInit
    }

    // Init the low level hardware
    hadc->MspInitCallback(hadc);
endif // USE_HAL_ADC_REGISTER_CALLBACKS
*/
    m_State = SYS_READY;                   // Initialize ADC error code

//        MODIFY_REG(ADCx->CR1, ADC_CR1_RES | ADC_CR1_SCAN, ADC_InitStruct->Resolution | ADC_InitStruct->SequencersScanMode); // Set ADC data resolution
//        MODIFY_REG(ADCx->CR2, ADC_CR2_ALIGN, ADC_InitStruct->DataAlignment);                          // Set ADC conversion data alignment

 //   MODIFY_REG(m_State, ADC_STATE_REG_BUSY | ADC_STATE_INJ_BUSY, ADC_STATE_BUSY_INTERNAL);       // Set ADC state
    //----------------------------------


    // Set ADC scan mode
    m_pInfo->pADCx->CR1 &= ~(ADC_CR1_SCAN);
   // m_pInfo->pADCx->CR1 |=  ADC_CR1_SCANCONV(hadc->Init.ScanConvMode);

    // Set ADC resolution
    m_pInfo->pADCx->CR1 &= ~(ADC_CR1_RES);
   // m_pInfo->pADCx->CR1 |=  hadc->Init.Resolution;

    // Set ADC data alignment
    m_pInfo->pADCx->CR2 &= ~(ADC_CR2_ALIGN);
   // m_pInfo->pADCx->CR2 |= hadc->Init.DataAlign;

    // Enable external trigger if trigger selection is different of software start.
    // Note: This configuration keeps the hardware feature of parameter ExternalTrigConvEdge "trigger edge none" equivalent to software start.
    //if(hadc->Init.ExternalTrigConv != ADC_SOFTWARE_START)
    {
        // Select external trigger to start conversion
        m_pInfo->pADCx->CR2 &= ~(ADC_CR2_EXTSEL);
      //  m_pInfo->pADCx->CR2 |= hadc->Init.ExternalTrigConv;

        // Select external trigger polarity
        m_pInfo->pADCx->CR2 &= ~(ADC_CR2_EXTEN);
      //  m_pInfo->pADCx->CR2 |= hadc->Init.ExternalTrigConvEdge;
    }
    //else
    {
        // Reset the external trigger
        m_pInfo->pADCx->CR2 &= ~(ADC_CR2_EXTSEL);
        m_pInfo->pADCx->CR2 &= ~(ADC_CR2_EXTEN);
    }

    // Enable or disable ADC continuous conversion mode
    m_pInfo->pADCx->CR2 &= ~(ADC_CR2_CONT);
   // m_pInfo->pADCx->CR2 |= ADC_CR2_CONTINUOUS((uint32_t)hadc->Init.ContinuousConvMode);

 //   if(hadc->Init.DiscontinuousConvMode != DISABLE)
    {
        // Enable the selected ADC regular discontinuous mode
        m_pInfo->pADCx->CR1 |= (uint32_t)ADC_CR1_DISCEN;

        // Set the number of channels to be converted in discontinuous mode
        m_pInfo->pADCx->CR1 &= ~(ADC_CR1_DISCNUM);
      //  m_pInfo->pADCx->CR1 |=  ADC_CR1_DISCONTINUOUS(hadc->Init.NbrOfDiscConversion);
    }
 //   else
    {
        // Disable the selected ADC regular discontinuous mode
        m_pInfo->pADCx->CR1 &= ~(ADC_CR1_DISCEN);
    }

    // Set ADC number of conversion
    m_pInfo->pADCx->SQR1 &= ~(ADC_SQR1_L);
  //  m_pInfo->pADCx->SQR1 |=  ADC_SQR1(hadc->Init.NbrOfConversion);

    // Enable or disable ADC DMA continuous request
    m_pInfo->pADCx->CR2 &= ~(ADC_CR2_DDS);
  //  m_pInfo->pADCx->CR2 |= ADC_CR2_DMAContReq((uint32_t)hadc->Init.DMAContinuousRequests);

    // Enable or disable ADC end of conversion selection
    m_pInfo->pADCx->CR2 &= ~(ADC_CR2_EOCS);
  //  m_pInfo->pADCx->CR2 |= ADC_CR2_EOCSelection(hadc->Init.EOCSelection);


    //----------------------------------
  // MODIFY_REG(m_State, ADC_STATE_BUSY_INTERNAL, HAL_ADC_STATE_READY);                       // Set the ADC state
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      GetStatus
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Return general status of the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e ADC_Driver::GetStatus(void)
{
    return m_State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Lock the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
//void ADC_Driver::Lock(void)
//{
    //while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK){};
//}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Unlock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Unlock the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
//void ADC_Driver::Unlock(void)
//{
    //nOS_MutexUnlock(&m_Mutex);
//}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    EV_IRQHandler
//
//  Description:    This function handles ADCx interrupt request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::IRQHandler()
{
}

//-------------------------------------------------------------------------------------------------

void ADC_ChannelDriver::Initialize(void)
{
    IO_PinInit(IO_TEMPERATURE);
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_ADC_DRIVER == DEF_ENABLED)
