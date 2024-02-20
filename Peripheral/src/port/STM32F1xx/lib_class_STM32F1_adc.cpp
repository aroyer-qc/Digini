//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_adc.cpp
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

#if 0

single conversion

handle start
offer polling, but if call back is set then enable the interrupt
External trigger


single conversion, continuous mode

offer polling, but if call back is set then enable the interrupt


multiple conversion, continuous mode

Channel-wise programmable sampling time
DMA!!!
Configurable delay




#endif


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
// const(s)
//-------------------------------------------------------------------------------------------------

const ADC_Info_t        ADC_Driver::m_ConstInfo[NB_OF_ADC_DRIVER] =
{
    ADC_DEF(EXPAND_X_ADC_AS_STRUCT_DATA)
};

const ADC_ChannelInfo_t ADC_Driver::m_ConstChannelInfo[NB_OF_ADC_CHANNEL] =
{
    ADC_CHANNEL_DEF(EXPAND_X_ADC_CHANNEL_AS_STRUCT_DATA)
};

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

bool ADC_Driver::m_CommonIsItInitialize = false;

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

    m_NumberOfChannel          = 0;
    m_NumberOfInjectedChannel  = 0;
    m_CurrentFreeChannel       = 0;
    m_pCallback                = nullptr;
    m_CallBackType             = ADC_CALLBACK_NONE;

    m_pInfo = (ADC_Info_t*)&m_ConstInfo[ADC_ID];
    m_State = SYS_DEVICE_NOT_PRESENT;
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
//                      - Clock Prescaler
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
    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;

        // Set the ADC clock prescaler
        ADC->CCR &= ~(ADC_CCR_ADCPRE);
        ADC->CCR |= ADC_COMMON_CLOCK_PRESCALER_CFG;

        SET_BIT(RCC->APB2ENR, m_pInfo->RCC_APB2_En);            // Enable clock for this ADC

        // No channel and no ranking in both regular and injected conversion
        m_pInfo->pADCx->SQR1 = 0;
        m_pInfo->pADCx->SQR2 = 0;
        m_pInfo->pADCx->SQR3 = 0;
        m_pInfo->pADCx->JSQR = 0;

        // No channel and no ranking in both regular and injected conversion
        m_pInfo->pADCx->SMPR1 = 0;
        m_pInfo->pADCx->SMPR2 = 0;

        // Set ADC resolution, Discontinuous mode, Number of channels to be converted in discontinuous mode
        m_pInfo->pADCx->CR1 = m_pInfo->CR1_Common | ADC_SCAN_MODE_DISABLE ;//| .. TODO set default value;

        // Set ADC data alignment, external trigger, external trigger polarity, continuous conversion mode, EOC selection, DMA continuous request
        m_pInfo->pADCx->CR2 = m_pInfo->CR2_Common | ADC_REG_TRIG_EXT_NONE | ADC_REG_TRIG_EXT_EDGE_NONE;
    }

    if(m_CommonIsItInitialize == false)
    {
        m_CommonIsItInitialize = true;

        // ADC interrupt are common to all module
        ISR_Init(ADC_IRQn, 0, 0);
    }

    // m_State = SYS_READY;                   // Initialize ADC error code
    // MODIFY_REG(m_State, ADC_STATE_REG_BUSY | ADC_STATE_INJ_BUSY, ADC_STATE_BUSY_INTERNAL);       // Set ADC state
    // MODIFY_REG(m_State, ADC_STATE_BUSY_INTERNAL, HAL_ADC_STATE_READY);                       // Set the ADC state
}


//  To be set by ConfigConversionTrigger ConfigInjectedConversionTrigger
//ADC_REG_TRIG_EXT_EDGE_NONE
//ADC_REG_TRIG_EXT_NONE


//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterCallback
//
//  Parameter(s):   pCallback       Callback pointer
//                  None
//
//  Description:    Register callback for user code in ISR
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::RegisterCallBack(CallbackInterface* pCallback)
{
    m_pCallback = pCallback;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableCallbackType
//
//  Parameter(s):   CallBackType    Type if the ISR callback
//                  pContext        Context for ISR
//  Return:         None
//
//  Description:    Enable the type of interrupt for the callback.
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::EnableCallbackType(int CallBackType, void* pContext)
{
    switch(CallBackType)
    {
      //#if (??? == DEF_ENABLED)
        case ADC_CALLBACK_CONVERSION_COMPLETED:
        {
            m_pContextConversionCompleted = pContext;
            m_CallBackType |= CallBackType;
        }
        break;
      //#endif

      //#if (??? == DEF_ENABLED)
        case ADC_CALLBACK_INJECTED_CONVERSION_COMPLETED:
        {
            m_pContextConversionInjectionCompleted = pContext;
            m_CallBackType |= CallBackType;
        }
        break;
      //#endif

      //#if (??? == DEF_ENABLED)
        case ADC_CALLBACK_ERROR:
        {
            m_pContextERROR = pContext;
            m_CallBackType |= CallBackType;
        }
        break;
      //#endif

    }
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
//   Function:      AddChannelToGroup
//
//   Parameter(s):  ChannelID
//                  Rank                The rank in the regular group sequencer.
//                                      This parameter must be between 1 to 16.
//   Return Value:  SystemState_e
//
//   Description:   Initializes the channel specified by ADC_Channel ID
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e ADC_Driver::AddChannelToGroup(ADC_ChannelID_e ChannelID, uint8_t Rank)
{
    volatile uint32_t* pRegister;
    uint32_t           Offset = 0;
    uint32_t           Shift  = 0;
    ADC_ChannelInfo_t* pChannel;

    m_pChannelInfo[m_CurrentFreeChannel] = (ADC_ChannelInfo_t*)&m_ConstChannelInfo[uint32_t(ChannelID)];;
    pChannel = m_pChannelInfo[m_CurrentFreeChannel];
    m_pInfo  = (ADC_Info_t*)&m_ConstInfo[uint32_t(pChannel->ADC_ID)];
    IO_PinInit(pChannel->IO_ID);

// To be set by AddChannelToGroup and AddInjectedChannelToGroup when more than 1 channel in the group
//ADC_SCAN_MODE_ENABLE

    // if ADC_Channel_10 ... ADC_Channel_18 is selected
    if(pChannel->Channel > ADC_CHANNEL_9)
    {
        Offset    = 10;
        pRegister = &m_pInfo->pADCx->SMPR1;
    }
    else // ADC_Channel include in ADC_Channel_[0..9]
    {
        pRegister = &m_pInfo->pADCx->SMPR2;
    }

    Shift = 3 * (pChannel->Channel - Offset);
    MODIFY_REG(*pRegister, SMPR_SMP_SET << Shift, uint32_t(pChannel->SampleTime) << Shift);    // Set new sample time

    // For Rank 1 to 6
    if (Rank < 7)
    {
        pRegister = &m_pInfo->pADCx->SQR3;
        Offset    = 1;
    }
    // For Rank 7 to 12
    else if (Rank < 13)
    {
        pRegister = &m_pInfo->pADCx->SQR2;
        Offset    = 7;
    }
    // For Rank 13 to 16
    else
    {
        pRegister = &m_pInfo->pADCx->SQR1;
        Offset    = 13;
    }

    Shift = 5 * (Rank - Offset);
    MODIFY_REG(*pRegister, SQR_SQ_SET << Shift, pChannel->Channel << Shift);       // Set the SQx bits for the selected rank

    m_CurrentFreeChannel++; // TODO add protection

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      StartConversion
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Enables the selected ADC software start conversion of the regular channels.
//
//   Note(s):
//
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::StartConversion(void)
{
    // Enable the selected ADC conversion for regular group
    SET_BIT(m_pInfo->pADCx->CR2, ADC_CR2_SWSTART);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      StartInjectedConversion
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Enables the selected ADC software start conversion of the injected channels.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::StartInjectedConversion(void)
{
    // Enable the selected ADC conversion for regular group
    // SET_BIT(m_pADC_Info->pADCx->CR2, ADC_CR2_SWSTART);
}

//-------------------------------------------------------------------------------------------------
//
//   IRQ Function:  TempSensorVrefintControl
//
//   Parameter(s):  NewState        New state of the temperature sensor and Vrefint channels.
//                                  DEF_ENABLE or DEF_DISABLE
//   Return Value:  None
//
//   Description:   Enables or disables the temperature sensor and Vrefint channels.
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
        SET_BIT(ADC->CCR, ADC_CCR_TSVREFE);
    }
    else
    {
        // Disable the temperature sensor and Vrefint channel
        CLEAR_BIT(ADC->CCR, ADC_CCR_TSVREFE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   IRQ Function:  VBAT_Control
//
//   Parameter(s):  NewState     New state of the VBAT channel.
//                                  DEF_ENABLE or DEF_DISABLE
//   Return Value:  None
//
//   Description:   Enables or disables the VBAT (Voltage Battery) channel.
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
        SET_BIT(ADC->CCR, ADC_CCR_VBATE);
    }
    else
    {
        // Disable the VBAT channel
        CLEAR_BIT(ADC->CCR, ADC_CCR_VBATE);
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
    // TODO check flag and call the right function
    m_pCallback->CallbackFunction(ADC_CALLBACK_CONVERSION_COMPLETED,          m_pContextConversionCompleted);
    m_pCallback->CallbackFunction(ADC_CALLBACK_INJECTED_CONVERSION_COMPLETED, m_pContextConversionInjectionCompleted);
    m_pCallback->CallbackFunction(ADC_CALLBACK_ERROR,                         m_pContextERROR);
}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

#endif // (USE_ADC_DRIVER == DEF_ENABLED)
