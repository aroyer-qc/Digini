//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_adc.cpp
//
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define ADC_DRIVER_GLOBAL
#include "BSP_board.h"
#undef  ADC_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const ADC_ChannelInfo_t ADC_Driver::m_ConstChannelInfo[NB_OF_ADC_CHANNEL] =
{
    ADC_CHANNEL_DEF(EXPAND_X_ADC_CHANNEL_AS_STRUCT_DATA)
};

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------
ADC_Driver *ADC_Driver::m_pDriver[NB_OF_ADC_DRIVER] = { nullptr };
bool     ADC_Driver::m_CommonIsInitialized = false;
uint16_t ADC_Driver::s_VrefintCal         = 0;
uint16_t ADC_Driver::s_TsCal1             = 0;
uint16_t ADC_Driver::s_TsCal2             = 0;

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
    m_IsInitialized = false;
    m_ADC_ID        = ADC_ID;
    m_pInfo         = &ADC_Info[ADC_ID];

    m_CurrentFreeChannel = 0;

    #if (ADC_DRIVER_SUPPORT_INJ_CONV_CFG == DEF_ENABLED)
      m_NumberOfInjectedChannel  = 0;
    #endif

    m_pCallback    = nullptr;
    m_CallBackType = ADC_CALLBACK_NONE;

    m_pDriver[ADC_ID] = this;
    m_State = SYS_DEVICE_NOT_PRESENT;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:   ReadFactoryCalibration
//
//   Description: Reads VREFINT and temperature-sensor factory calibration constants from system
//                flash (0x08FFF8xx) into RAM-resident static variables.  Must be called BEFORE
//                HAL_ICACHE_Enable(); reading those addresses through ICACHE may return aliased
//                (stale / wrong) data causing incorrect Vdda and temperature calculations.
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::ReadFactoryCalibration(void)
{
    s_VrefintCal = *((volatile const uint16_t*)0x08FFF810);
    s_TsCal1     = *((volatile const uint16_t*)0x08FFF814);
    s_TsCal2     = *((volatile const uint16_t*)0x08FFF818);
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
    if(m_IsInitialized == false)
    {
        m_IsInitialized = true;

        // Clock Selection done in system_stm32h5xx.c

        // Enable the peripheral clock
        SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADCEN);

        // Disable deep power down mode
        if ((m_pInfo->pADCx->CR & ADC_CR_DEEPPWD) != 0)
        {
            CLEAR_BIT(m_pInfo->pADCx->CR, (ADC_CR_DEEPPWD | BSP_ADC_CR_PROPERTY_BITS));
        }

        // Enable the internal regulator
        if ((m_pInfo->pADCx->CR & ADC_CR_ADVREGEN) == 0)
        {
            MODIFY_REG(m_pInfo->pADCx->CR, BSP_ADC_CR_PROPERTY_BITS, ADC_CR_ADVREGEN);

            // Wait for ADC regulator startup before launching calibration.
            HAL_Delay(1);
        }

        // Set the ADC clock prescaler BEFORE calibration so that calibration runs at
        // the same clock frequency used for normal conversions.  At reset CCR.PRESC=0
        // (no prescaler) which would drive the ADC at full HCLK (250 MHz — 5× over spec),
        // producing a wrong CALFACT that causes a systematic offset on every conversion.
        // This can only be done while the ADC is not enabled (ADEN=0), which is true here.
        CLEAR_BIT(ADC12_COMMON->CCR, ADC_CCR_CKMODE);
        ADC12_COMMON->CCR &= ~(ADC_CCR_PRESC);
        ADC12_COMMON->CCR |= m_pInfo->CCR_Common;

        // Run ADC calibration only after DEEPPWD is cleared, ADVREGEN is enabled,
        // and the ADC clock is set to its operational frequency.
        InitCalibration();

        // No channel and no ranking in regular conversion
        m_pInfo->pADCx->SQR1 = 0;
        m_pInfo->pADCx->SQR2 = 0;
        m_pInfo->pADCx->SQR3 = 0;
        m_pInfo->pADCx->SQR4 = 0;
        m_pInfo->pADCx->JSQR = 0;

        // Disable offset for all channels
        m_pInfo->pADCx->OFR1 = 0;
        m_pInfo->pADCx->OFR2 = 0;
        m_pInfo->pADCx->OFR3 = 0;
        m_pInfo->pADCx->OFR4 = 0;

        // No channel and no ranking in regular conversions
        m_pInfo->pADCx->SMPR1 = 0;
        m_pInfo->pADCx->SMPR2 = 0;

        // All channels are in single-ended mode
        m_pInfo->pADCx->DIFSEL = 0;

        // Set resolution, data alignment, continuous mode and discontinuous mode
        m_pInfo->pADCx->CFGR = m_pInfo->CFGR_Common;
        m_pInfo->pADCx->CFGR2 = m_pInfo->CFGR2_Common;

        // Configure the DMA for the given ADC
        m_DMA_ADC.Initialize(&m_pInfo->DMA_ADC);
        m_DMA_ADC.EnableCircular();

        m_DMA_ADC.SetSource((void*)&m_pInfo->pADCx->DR); // Configure ADC data register
        m_DMA_ADC.EnableTransmitCompleteInterrupt();
        m_DMA_ADC.EnableIRQ();                           // NVIC Setup for ADC DMA channels interrupt request
    }

    if(m_CommonIsInitialized == false)
    {
        m_CommonIsInitialized = true;

        // ADC interrupt are common to all module
        ISR_Init(m_pInfo->IRQn, 4);
    }
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           InitCalibration
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Initialize ADC calibration
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::InitCalibration(void)
{
    uint32_t wait_loop_index = 0UL;

    // Start ADC calibration in mode single-ended (ADC_CALFACT_CALFACT_S == single-ended)
    MODIFY_REG(m_pInfo->pADCx->CR, ADC_CR_ADCALDIF | ADC_CR_BITS_PROPERTY_RS, ADC_CR_ADCAL | (ADC_CALFACT_CALFACT_S & ADC_SINGLEDIFF_CALIB_START_MASK));

    // Wait for calibration completion
    while(READ_BIT(m_pInfo->pADCx->CR, ADC_CR_ADCAL) == ADC_CR_ADCAL)
    {
        wait_loop_index++;

        if(wait_loop_index >= ADC_CALIBRATION_TIMEOUT)
        {
            m_State = SYS_TIME_OUT;
            return;
        }
    }

    m_State = SYS_READY;
}

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
void ADC_Driver::RegisterCallBack(DmaCallback_t pCallback)
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
      case ADC_CALLBACK_CONVERSION_COMPLETED:
      {
        m_pContextConversionCompleted = pContext;
        m_CallBackType |= CallBackType;
      }
      break;

      #if (ADC_DRIVER_SUPPORT_INJ_CONV_CFG == DEF_ENABLED)
        case ADC_CALLBACK_INJECTED_CONVERSION_COMPLETED:
        {
            m_pContextConversionInjectionCompleted = pContext;
            m_CallBackType |= CallBackType;
        }
        break;
      #endif

      #if (ADC_DRIVER_SUPPORT_ERROR_CFG == DEF_ENABLED)
        case ADC_CALLBACK_ERROR:
        {
            m_pContextERROR = pContext;
            m_CallBackType |= CallBackType;
        }
        break;
      #endif

        default:
          // We should not get here
          ASSERT(0);
        break;

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
//   Parameter(s):  ChannelID           The ID of the channel to add in the sequence
//   Return Value:  SystemState_e
//
//   Description:   Appends the given channel to the sequence of conversions
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e ADC_Driver::AddChannelToGroup(ADC_ChannelID_e ChannelID)
{
    volatile uint32_t* pRegister;
    uint32_t           Offset = 0;
    uint32_t           Shift  = 0;
    ADC_ChannelInfo_t* pChannel;
    uint32_t           Channel;

    ASSERT(ChannelID < NB_OF_ADC_CHANNEL);

    if(m_CurrentFreeChannel >= ADC_NUMBER_OF_RANK_FOR_CONVERSION)
    {
        ASSERT(0);
        return SYS_POOL_FULL;
    }

    m_pChannelInfo[m_CurrentFreeChannel] = (ADC_ChannelInfo_t*)&m_ConstChannelInfo[uint32_t(ChannelID)];
    pChannel = m_pChannelInfo[m_CurrentFreeChannel];
    
    if(pChannel->IO_ID != IO_NOT_DEFINED)
    {
        IO_PinInit(pChannel->IO_ID);
    }

    // Extract channel number
    Channel = pChannel->Channel & ~(BSP_ADC_CHANNEL_INTERNAL_MSK);

    // For channel ADC1_INP0, we need to enable the GPIO switch
    if(Channel == BSP_ADC_CHANNEL_0)
    {    
        SET_BIT(ADC1->OR, ADC_OR_OP0);
    }

    // if ADC_Channel_10 ... ADC_Channel_18 is selected
    if(Channel > BSP_ADC_CHANNEL_9)
    {
        Offset    = 10;
        pRegister = &m_pInfo->pADCx->SMPR2;
    }
    else // ADC_Channel include in ADC_Channel_[0..9]
    {
        Offset = 0;
        pRegister = &m_pInfo->pADCx->SMPR1;
    }

    Shift = 3 * (Channel - Offset);
    MODIFY_REG(*pRegister, SMPR_SMP_SET << Shift, uint32_t(pChannel->SampleTime) << Shift);    // Set new sample time

    // Configure regular conversion sequence
    if(m_CurrentFreeChannel < 4)                        // For Rank 1 to 4
    {
        pRegister = &m_pInfo->pADCx->SQR1;
        // In SQR1, the 6 LSB bits contain the number of conversions
        Offset    = -1;
    }
    else if(m_CurrentFreeChannel < 9)                  // For Rank 5 to 9
    {
        pRegister = &m_pInfo->pADCx->SQR2;
        Offset    = 4;
    }
    else if(m_CurrentFreeChannel < 14)                  // For Rank 10 to 14
    {
        pRegister = &m_pInfo->pADCx->SQR3;
        Offset    = 9;
    }
    else
    {
        pRegister = &m_pInfo->pADCx->SQR4;
        Offset    = 14;
    }

    Shift = 6 * (m_CurrentFreeChannel - Offset);
    MODIFY_REG(*pRegister, SQR_SQ_SET << Shift, Channel << Shift);       // Set the SQx bits for the selected rank

    if((pChannel->Channel & BSP_ADC_CHANNEL_INTERNAL_MSK) == BSP_ADC_CHANNEL_INTERNAL_MSK)
    {
        // Specific configurations for Vrefint, VBAT and temperature sensor
        if(pChannel->Channel == BSP_ADC_CHANNEL_VREFINT)
        {
            VrefintControl(DEF_ENABLED);
        }
        else if(pChannel->Channel == BSP_ADC_CHANNEL_VBAT)
        {
            VBAT_Control(DEF_ENABLED);
        }
        else if(pChannel->Channel == BSP_ADC_CHANNEL_TEMP) 
        {
            TempSensorControl(DEF_ENABLED);
        }
    }

    // Update the number of channels in scan mode
    m_pInfo->pADCx->SQR1 &= ~(ADC_SQR1_L);
    m_pInfo->pADCx->SQR1 |= (m_CurrentFreeChannel & ADC_SQR1_L);

    m_CurrentFreeChannel++;

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
void ADC_Driver::StartConversion(uint32_t* buffer, size_t size)
{
    bool needsRegulatorStabilization = false;

    if((m_pInfo->pADCx->CR & ADC_CR_ADEN) == 0)
    {
        uint32_t wait_loop_index = 0UL;

        // Clear ADRDY then enable ADC and wait until it is ready.
        m_pInfo->pADCx->ISR = ADC_ISR_ADRDY;
        MODIFY_REG(m_pInfo->pADCx->CR, BSP_ADC_CR_PROPERTY_BITS, ADC_CR_ADEN);

        while(READ_BIT(m_pInfo->pADCx->ISR, ADC_ISR_ADRDY) == 0)
        {
            wait_loop_index++;
            if(wait_loop_index >= ADC_CALIBRATION_TIMEOUT)
            {
                m_State = SYS_TIME_OUT;
                return;
            }
        }

        needsRegulatorStabilization = true;
    }

    m_State = SYS_READY;

    if(needsRegulatorStabilization == true)
    {
        // Wait for ADC regulator stabilization (min ~10µs per STM32H5 datasheet)
        // Using busy-wait since HAL_Delay may not be available during early init (systick not running)
        for(volatile uint32_t i = 0; i < 100; i++) { }  // Busy-wait ~10µs
    }

    // Clear regular conversion flags
    m_pInfo->pADCx->ISR = (ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR);

    m_DMA_ADC.SetDestination(buffer);
    m_DMA_ADC.SetLength(size);
    m_DMA_ADC.WriteTransferRegisters();
    m_DMA_ADC.Enable();
    m_DMA_ADC.ClearFlag();

    // Start the ADC conversions for regular group
    MODIFY_REG(m_pInfo->pADCx->CR, BSP_ADC_CR_PROPERTY_BITS, ADC_CR_ADSTART);
}

#if (ADC_DRIVER_SUPPORT_INJ_CONV_CFG == DEF_ENABLED)
SystemState_e ADC_Driver::AddInjectedChannelToGroup(ADC_ChannelID_e Channel, uint8_t Rank)
{
    // TODO
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
#endif // #if (ADC_DRIVER_SUPPORT_INJ_CONV_CFG == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   IRQ Function:  VrefintControl
//
//   Parameter(s):  NewState        New state of the VrefInt hannel.
//                                  DEF_ENABLE or DEF_DISABLE
//   Return Value:  None
//
//   Description:   Enables or disables the VrefInt  sensor channel.
//
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::VrefintControl(bool NewState)
{
    if(NewState == DEF_ENABLED)
    {
        SET_BIT(ADC12_COMMON->CCR, ADC_CCR_VREFEN);         // Enable the Vrefint channel
    }
    else
    {
        CLEAR_BIT(ADC12_COMMON->CCR, ADC_CCR_VREFEN);       // Disable the Vrefint channel
    }
}

//-------------------------------------------------------------------------------------------------
//
//   IRQ Function:  TempSensorControl
//
//   Parameter(s):  NewState        New state of the temperature sensorchannel.
//                                  DEF_ENABLE or DEF_DISABLE
//   Return Value:  None
//
//   Description:   Enables or disables the temperature sensor channel.
//
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::TempSensorControl(bool NewState)
{
    if(NewState == DEF_ENABLED)
    {
        // Enable the temperature sensor channel
        SET_BIT(ADC12_COMMON->CCR, ADC_CCR_TSEN);
    }
    else
    {
        // Disable the temperature sensor channel
        CLEAR_BIT(ADC12_COMMON->CCR, ADC_CCR_TSEN);
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
    if(NewState == DEF_ENABLED)
    {
        // Enable the VBAT channel
        SET_BIT(ADC12_COMMON->CCR, ADC_CCR_VBATEN);
    }
    else
    {
        // Disable the VBAT channel
        CLEAR_BIT(ADC12_COMMON->CCR, ADC_CCR_VBATEN);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    IRQHandler
//
//  Parameter(s):  ADC_ID     ID of the ADC that triggered the interrupt
//
//  Return Value:  None
//
//  Description:    This function handles ADCx DMA interrupt request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void ADC_Driver::DmaIRQHandler(ADC_ID_e ADC_ID)
{
    ADC_Driver* pAdcDriver;
    DMA_Driver* pDmaDriver;

    ASSERT(ADC_ID < NB_OF_ADC_DRIVER);

    pAdcDriver = m_pDriver[ADC_ID];
    pDmaDriver = &(pAdcDriver->m_DMA_ADC);

    #if (ADC_DRIVER_SUPPORT_ERROR_CFG == DEF_ENABLED)
        bool error;
        error = pDmaDriver->CheckFlag(DMA_CSR_DTEF |
                                      DMA_CSR_USEF |
                                      DMA_CSR_ULEF |
                                      DMA_CSR_TOF);
        
        if (error)
        {
            if (pAdcDriver->m_pCallback)
            {
                pAdcDriver->m_pCallback(ADC_CALLBACK_ERROR, pAdcDriver->m_pContextERROR);
            }
            
            // Clear flags before returning
            pDmaDriver->ClearFlag();
            return;
        }
    #endif
    
    if (pDmaDriver->CheckFlag(DMA_CSR_TCF))
    {
        pDmaDriver->ClearFlag();
        if (pAdcDriver->m_pCallback)
        {
            pAdcDriver->m_pCallback(ADC_CALLBACK_CONVERSION_COMPLETED, pAdcDriver->m_pContextConversionCompleted);
        }
    
    #if (ADC_DRIVER_SUPPORT_INJ_CONV_CFG == DEF_ENABLED)
        // Not supported for now
        //if (pAdcDriver->m_pCallback)
        //{
        //    pAdcDriver->m_pCallback(ADC_CALLBACK_INJECTED_CONVERSION_COMPLETED, m_pContextConversionInjectionCompleted);
        //}
    #endif
    }
}

//-------------------------------------------------------------------------------------------------

extern "C"
{
// ADC1 DMA IRQ handler
void GPDMA2_Channel1_IRQHandler(void)
{
    ADC_Driver::DmaIRQHandler(DRIVER_ADC1_ID);
}

} // extern "C"

//-------------------------------------------------------------------------------------------------
