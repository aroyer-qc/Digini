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
    
    m_NumberofChannel          = 0;
    m_NumberofInjectedChannel  = 0;
    
    
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
    ISR_Prio_t    ISR_Prio;
    uint32_t      PriorityGroup;
    //SystemState_e State         = SYS_READY;

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
        Mm_pInfo->pADCx->CR1 = m_pInfo->CR1_Common | ADC_SCAN_MODE_DISABLE | ... TODO set default value;
    
        // Set ADC data alignment, external trigger, external trigger polarity, continuous conversion mode, EOC selection, DMA continuous request
        m_pInfo->pADCx->CR2 = m_pInfo->CR2_Common | ADC_REG_TRIG_EXT_NONE | ADC_REG_TRIG_EXT_EDGE_NONE;
    }

    if(m_CommonIsItInitialize == false)
    {
        m_CommonIsItInitialize = true;

        // ADC interrupt are common to all module
        PriorityGroup              = NVIC_GetPriorityGrouping();
        ISR_Prio.PriorityGroup     = PriorityGroup;
        ISR_Prio.SubPriority       = 0;
        ISR_Prio.PremptionPriority = 0;
        ISR_Init(ADC_IRQn, &ISR_Prio);
    }

    // m_State = SYS_READY;                   // Initialize ADC error code
    // MODIFY_REG(m_State, ADC_STATE_REG_BUSY | ADC_STATE_INJ_BUSY, ADC_STATE_BUSY_INTERNAL);       // Set ADC state
    // MODIFY_REG(m_State, ADC_STATE_BUSY_INTERNAL, HAL_ADC_STATE_READY);                       // Set the ADC state
}


//  To be set by ConfigConversionTrigger ConfigInjectedConversionTrigger
ADC_REG_TRIG_EXT_EDGE_NONE
ADC_REG_TRIG_EXT_NONE

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
//
//   Parameter(s):  Rank                The rank in the regular group sequencer.
//                                      This parameter must be between 1 to 16.
//   Return Value:  None
//
//   Description:
//
//
//   Note(s):
//
//
//
//   Description:   Initializes the channel specified by ADC_Channel ID
//
//-------------------------------------------------------------------------------------------------
SystemState_e ADC_Driver::AddChannelToGroup(ADC_ChannelID_e ChannelID, uint8_t Rank)
{
  //  m_IsItInitialize = false;
    m_pChannelInfo   = &ADC_ChannelInfo[ChannelID];
    m_pADC_Info      = &ADC_Info[m_pChannelInfo->ADC_ID];


    IO_PinInit(m_pChannelInfo->IO_ID);        //  TODO validate



// To be set by AddChannelToGroupand AddInjectedChannelToGroup when more than 1 channel in the group
ADC_SCAN_MODE_ENABLE


    volatile uint32_t* pRegister;
    uint32_t  Offset = 0;
    uint32_t  Shift  = 0;

    // if ADC_Channel_10 ... ADC_Channel_18 is selected
    if(m_pChannelInfo->Channel > ADC_CHANNEL_9)
    {
        Offset    = 10;
        pRegister = &m_pADC_Info->pADCx->SMPR1;
    }
    else // ADC_Channel include in ADC_Channel_[0..9]
    {
        pRegister = &m_pADC_Info->pADCx->SMPR2;
    }

    Shift = 3 * (m_pChannelInfo->Channel - Offset);
    MODIFY_REG(*pRegister, SMPR_SMP_SET << Shift, uint32_t(ADC_SampleTime) << Shift);    // Set new sample time



    // For Rank 1 to 6
    if (Rank < 7)
    {
        pRegister = &m_pADC_Info->pADCx->SQR3;
        Offset    = 1;
    }
    // For Rank 7 to 12
    else if (Rank < 13)
    {
        pRegister = &m_pADC_Info->pADCx->SQR2;
        Offset    = 7;
    }
    // For Rank 13 to 16
    else
    {
        pRegister = &m_pADC_Info->pADCx->SQR1;
        Offset    = 13;
    }

    Shift = 5 * (Rank - Offset);
    MODIFY_REG(*pRegister, SQR_SQ_SET << Shift, m_pChannelInfo->Channel << Shift);       // Set the SQx bits for the selected rank
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      StartConversion
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
void ADC_Driver::StartConversion(void)
{
    // Enable the selected ADC conversion for regular group
    m_pADC_Info->pADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      StartInjectedConversion
//
//   Description:   Enables the selected ADC software start conversion of the injected channels.
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Note(s):
//
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::StartInjectedConversion(void)
{
    // Enable the selected ADC conversion for regular group
    //m_pADC_Info->pADCx->CR2 |= (uint32_t)ADC_CR2_SWSTART;
}

//-------------------------------------------------------------------------------------------------
//
//   IRQ Function:  TempSensorVrefintControl
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

#endif // (USE_ADC_DRIVER == DEF_ENABLED)
