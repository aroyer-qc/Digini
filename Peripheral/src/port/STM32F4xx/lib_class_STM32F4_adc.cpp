//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_adc.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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
        ADC->CCR |= ADC_COMMON_CLOCK_PRESCALER_CFG;

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

    //MODIFY_REG(m_State, ADC_STATE_REG_BUSY | ADC_STATE_INJ_BUSY, ADC_STATE_BUSY_INTERNAL);       // Set ADC state
    //----------------------------------

    // Set ADC scan mode and resolution, Discontinuous mode, Number of channels to be converted in discontinuous mode
    m_pInfo->pADCx->CR1 &= ~(ADC_CR1_SCAN | ADC_CR1_RES | ADC_CR1_DISCEN | ADC_CR1_DISCNUM);
    m_pInfo->pADCx->CR1 |= m_pInfo->CR1_Common;

    // Set ADC data alignment, external trigger, external trigger polarity, continuous conversion mode, EOC selection, DMA continuous request
    m_pInfo->pADCx->CR2 &= ~(ADC_CR2_ALIGN | ADC_CR2_EXTSEL | ADC_CR2_EXTEN | ADC_CR2_CONT | ADC_CR2_EOCS | ADC_CR2_DDS);
    m_pInfo->pADCx->CR2 |= m_pInfo->CR2_Common;

    // Set ADC number of conversion
    m_pInfo->pADCx->SQR1 &= ~(ADC_SQR1_L);
    m_pInfo->pADCx->SQR1 |= m_pInfo->NumberOfConversion;

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
//   IRQ Function:  ADC_VBAT_Control
//
//   Description:   Enables or disables the temperature sensor and Vrefint channels.
//
//   Parameter(s):  NewState        New state of the temperature sensor and Vrefint channels.
//                                  DEF_ENABLE or DEF_DISABLE
//   Return Value:  None
//
//   Note(s):       The Battery voltage measured is equal to VBAT/2 on STM32F40xx and STM32F41xx
//                  devices and equal to VBAT/4 on STM32F42xx and STM32F43xx devices.
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::TempSensorVrefintControl(bool NewState)
{
    if(NewState == DEF_ENABLE)
    {
        // Enable the temperature sensor and Vrefint channel
        ADC->CCR |= (uint32_t)ADC_CCR_TSVREFE;
    }
    else
    {
        // Disable the temperature sensor and Vrefint channel
        ADC->CCR &= (uint32_t)(~ADC_CCR_TSVREFE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   IRQ Function:  VBAT_Control
//
//   Description:   Enables or disables the VBAT (Voltage Battery) channel.
//
//   Parameter(s):  NewState     New state of the VBAT channel.
//                                  DEF_ENABLE or DEF_DISABLE
//   Return Value:  None
//
//   Note(s):       The Battery voltage measured is equal to VBAT/2 on STM32F40xx and STM32F41xx
//                  devices and equal to VBAT/4 on STM32F42xx and STM32F43xx devices.
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::VBAT_Control(bool NewState)
{
    if(NewState == DEF_ENABLE)
    {
        // Enable the VBAT channel
        ADC->CCR |= (uint32_t)ADC_CCR_VBATE;
    }
    else
    {
        // Disable the VBAT channel
        ADC->CCR &= (uint32_t)(~ADC_CCR_VBATE);
    }
}

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

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   ADC_Driver
//
//   Parameter(s):  ADC_ChannelID
//
//   Description:   Initializes the channel specified by ADC_Channel ID
//
//-------------------------------------------------------------------------------------------------
ADC_ChannelDriver::ADC_ChannelDriver(ADC_ChannelID_e ADC_ChannelID)
{
  //  m_IsItInitialize = false;
    m_pChannelInfo   = &ADC_ChannelInfo[ADC_ChannelID];
    m_pADC_Info      = &ADC_Info[m_pChannelInfo->ADC_ID];
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Description:
//
//   Parameter(s):
//   Return Value:  None
//
//   Note(s):
//
//
//-------------------------------------------------------------------------------------------------
void ADC_ChannelDriver::Initialize()
{
    IO_PinInit(m_pChannelInfo->IO_ID);
  // get channel ID of pointer
}

/**
  * @param  Rank: The rank in the regular group sequencer.
  *          This parameter must be between 1 to 16.
  * @param  ADC_SampleTime: The sample time value to be set for the selected channel.
  *          This parameter can be one of the following values:
  *            @arg ADC_SampleTime_3Cycles: Sample time equal to 3 cycles
  *            @arg ADC_SampleTime_15Cycles: Sample time equal to 15 cycles
  *            @arg ADC_SampleTime_28Cycles: Sample time equal to 28 cycles
  *            @arg ADC_SampleTime_56Cycles: Sample time equal to 56 cycles
  *            @arg ADC_SampleTime_84Cycles: Sample time equal to 84 cycles
  *            @arg ADC_SampleTime_112Cycles: Sample time equal to 112 cycles
  *            @arg ADC_SampleTime_144Cycles: Sample time equal to 144 cycles
  *            @arg ADC_SampleTime_480Cycles: Sample time equal to 480 cycles
  * @retval None
  */
void ADC_ChannelDriver::Config(uint8_t Rank, uint8_t ADC_SampleTime)
{
    uint32_t tmpreg1 = 0;
    uint32_t tmpreg2 = 0;

    /* if ADC_Channel_10 ... ADC_Channel_18 is selected */
    if(m_pChannelInfo->Channel > ADC_CHANNEL_9)
    {
        /* Get the old register value */
        tmpreg1 = m_pADC_Info->pADCx->SMPR1;

        /* Calculate the mask to clear */
        tmpreg2 = SMPR_SMP_SET << (3 * (m_pChannelInfo->Channel - 10));

        /* Clear the old sample time */
        tmpreg1 &= ~tmpreg2;

        /* Calculate the mask to set */
        tmpreg2 = (uint32_t)ADC_SampleTime << (3 * (m_pChannelInfo->Channel - 10));

        /* Set the new sample time */
        tmpreg1 |= tmpreg2;

        /* Store the new register value */
        m_pADC_Info->pADCx->SMPR1 = tmpreg1;
    }
    else /* ADC_Channel include in ADC_Channel_[0..9] */
    {
            /* Get the old register value */
        tmpreg1 = m_pADC_Info->pADCx->SMPR2;

        /* Calculate the mask to clear */
        tmpreg2 = SMPR_SMP_SET << (3 * m_pChannelInfo->Channel);

        /* Clear the old sample time */
        tmpreg1 &= ~tmpreg2;

        /* Calculate the mask to set */
        tmpreg2 = (uint32_t)ADC_SampleTime << (3 * m_pChannelInfo->Channel);

        /* Set the new sample time */
        tmpreg1 |= tmpreg2;

        /* Store the new register value */
        m_pADC_Info->pADCx->SMPR2 = tmpreg1;
    }

    /* For Rank 1 to 6 */
    if (Rank < 7)
    {
        /* Get the old register value */
        tmpreg1 = m_pADC_Info->pADCx->SQR3;

        /* Calculate the mask to clear */
        tmpreg2 = SQR_SQ_SET << (5 * (Rank - 1));

        /* Clear the old SQx bits for the selected rank */
        tmpreg1 &= ~tmpreg2;

        /* Calculate the mask to set */
        tmpreg2 = m_pChannelInfo->Channel << (5 * (Rank - 1));

        /* Set the SQx bits for the selected rank */
        tmpreg1 |= tmpreg2;

        /* Store the new register value */
        m_pADC_Info->pADCx->SQR3 = tmpreg1;
    }
    /* For Rank 7 to 12 */
    else if (Rank < 13)
    {
        /* Get the old register value */
        tmpreg1 = m_pADC_Info->pADCx->SQR2;

        /* Calculate the mask to clear */
        tmpreg2 = SQR_SQ_SET << (5 * (Rank - 7));

        /* Clear the old SQx bits for the selected rank */
        tmpreg1 &= ~tmpreg2;

        /* Calculate the mask to set */
        tmpreg2 = m_pChannelInfo->Channel << (5 * (Rank - 7));

        /* Set the SQx bits for the selected rank */
        tmpreg1 |= tmpreg2;

        /* Store the new register value */
        m_pADC_Info->pADCx->SQR2 = tmpreg1;
    }
    /* For Rank 13 to 16 */
    else
    {
        /* Get the old register value */
        tmpreg1 = m_pADC_Info->pADCx->QR1;

        /* Calculate the mask to clear */
        tmpreg2 = SQR_SQ_SET << (5 * (Rank - 13));

        /* Clear the old SQx bits for the selected rank */
        tmpreg1 &= ~tmpreg2;

        /* Calculate the mask to set */
        tmpreg2 = m_pChannelInfo->Channel << (5 * (Rank - 13));

        /* Set the SQx bits for the selected rank */
        tmpreg1 |= tmpreg2;

        /* Store the new register value */
        m_pADC_Info->pADCx->SQR1 = tmpreg1;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Description:   Enables the selected ADC software start conversion of the regular channels.
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Note(s):
//
//
//-------------------------------------------------------------------------------------------------
void ADC_ChannelDriver::StartConversion(void)
{
    // Enable the selected ADC conversion for regular group
    m_pADC_Info->pADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_ADC_DRIVER == DEF_ENABLED)
