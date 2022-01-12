//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_adc.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_ADC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define ADC_CLOCK_SYNC_PCLK_DIV_2                   0
#define ADC_CLOCK_SYNC_PCLK_DIV_4                   ((uint32_t)ADC_CCR_ADCPRE_0)
#define ADC_CLOCK_SYNC_PCLK_DIV_6                   ((uint32_t)ADC_CCR_ADCPRE_1)
#define ADC_CLOCK_SYNC_PCLK_DIV_8                   ((uint32_t)ADC_CCR_ADCPRE)

#define ADC_RESOLUTION_12_BITS                      0
#define ADC_RESOLUTION_10_BITS                      ((uint32_t)ADC_CR1_RES_0)
#define ADC_RESOLUTION_8_BITS                       ((uint32_t)ADC_CR1_RES_1)
#define ADC_RESOLUTION_6_BITS                       ((uint32_t)ADC_CR1_RES)

#define ADC_DATA_ALIGN_RIGHT                        0
#define ADC_DATA_ALIGN_LEFT                         ((uint32_t)ADC_CR2_ALIGN)

#define ADC_SEQ_SCAN_DISABLE
#define ADC_SEQ_SCAN_ENABLE

#define ADC_REG_TRIG_SOFTWARE
#define ADC_REG_TRIG_EXT_TIM1_CH1
#define ADC_REG_TRIG_EXT_TIM1_CH2
#define ADC_REG_TRIG_EXT_TIM1_CH3
#define ADC_REG_TRIG_EXT_TIM2_CH2
#define ADC_REG_TRIG_EXT_TIM2_CH3
#define ADC_REG_TRIG_EXT_TIM2_CH4
#define ADC_REG_TRIG_EXT_TIM2_TRGO
#define ADC_REG_TRIG_EXT_TIM3_CH1
#define ADC_REG_TRIG_EXT_TIM3_TRGO
#define ADC_REG_TRIG_EXT_TIM4_CH4
#define ADC_REG_TRIG_EXT_TIM5_CH1
#define ADC_REG_TRIG_EXT_TIM5_CH2
#define ADC_REG_TRIG_EXT_TIM5_CH3
#define ADC_REG_TRIG_EXT_TIM8_CH1
#define ADC_REG_TRIG_EXT_TIM8_TRGO
#define ADC_REG_TRIG_EXT_EXTI_LINE11

#define ADC_REG_CONV_SINGLE
#define ADC_REG_CONV_CONTINUOUS

#define ADC_REG_DMA_TRANSFER_NONE
#define ADC_REG_DMA_TRANSFER_LIMITED
#define ADC_REG_DMA_TRANSFER_UNLIMITED

#define ADC_REG_FLAG_EOC_SEQUENCE_CONV
#define ADC_REG_FLAG_EOC_UNITARY_CONV

#define ADC_REG_SEQ_SCAN_DISABLE
#define ADC_REG_SEQ_SCAN_ENABLE_2_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_3_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_4_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_5_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_6_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_7_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_8_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_9_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_10_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_11_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_12_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_13_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_14_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_15_RANKS
#define ADC_REG_SEQ_SCAN_ENABLE_16_RANKS

#define ADC_REG_SEQ_DISCONT_DISABLE
#define ADC_REG_SEQ_DISCONT_1_RANK
#define ADC_REG_SEQ_DISCONT_2_RANKS
#define ADC_REG_SEQ_DISCONT_3_RANKS
#define ADC_REG_SEQ_DISCONT_4_RANKS
#define ADC_REG_SEQ_DISCONT_5_RANKS
#define ADC_REG_SEQ_DISCONT_6_RANKS
#define ADC_REG_SEQ_DISCONT_7_RANKS
#define ADC_REG_SEQ_DISCONT_8_RANKS

#define ADC_INJ_TRIG_SOFTWARE
#define ADC_INJ_TRIG_EXT_TIM1_CH4
#define ADC_INJ_TRIG_EXT_TIM1_TRGO
#define ADC_INJ_TRIG_EXT_TIM2_CH1
#define ADC_INJ_TRIG_EXT_TIM2_TRGO
#define ADC_INJ_TRIG_EXT_TIM3_CH2
#define ADC_INJ_TRIG_EXT_TIM3_CH4
#define ADC_INJ_TRIG_EXT_TIM4_CH1
#define ADC_INJ_TRIG_EXT_TIM4_CH2
#define ADC_INJ_TRIG_EXT_TIM4_CH3
#define ADC_INJ_TRIG_EXT_TIM4_TRGO
#define ADC_INJ_TRIG_EXT_TIM5_CH4
#define ADC_INJ_TRIG_EXT_TIM5_TRGO
#define ADC_INJ_TRIG_EXT_TIM8_CH2
#define ADC_INJ_TRIG_EXT_TIM8_CH3
#define ADC_INJ_TRIG_EXT_TIM8_CH4
#define ADC_INJ_TRIG_EXT_EXTI_LINE15

#define ADC_INJ_TRIG_EXT_RISING
#define ADC_INJ_TRIG_EXT_FALLING
#define ADC_INJ_TRIG_EXT_RISINGFALLING

#define ADC_INJ_TRIG_INDEPENDENT
#define ADC_INJ_TRIG_FROM_GRP_REGULAR

#define ADC_INJ_SEQ_SCAN_DISABLE
#define ADC_INJ_SEQ_SCAN_ENABLE_2_RANKS
#define ADC_INJ_SEQ_SCAN_ENABLE_3_RANKS
#define ADC_INJ_SEQ_SCAN_ENABLE_4_RANKS

#define ADC_INJ_SEQ_DISCONT_DISABLE
#define ADC_INJ_SEQ_DISCONT_1_RANK

#ifdef ADC_MULTIMODE_SUPPORT
#define ADC_MULTI_INDEPENDENT
#define ADC_MULTI_DUAL_REG_SIMULT
#define ADC_MULTI_DUAL_REG_INTERL
#define ADC_MULTI_DUAL_INJ_SIMULT
#define ADC_MULTI_DUAL_INJ_ALTERN
#define ADC_MULTI_DUAL_REG_SIM_INJ_SIM
#define ADC_MULTI_DUAL_REG_SIM_INJ_ALT
#define ADC_MULTI_DUAL_REG_INT_INJ_SIM
#ifdef ADC3
#define ADC_MULTI_TRIPLE_REG_SIM_INJ_SIM
#define ADC_MULTI_TRIPLE_REG_SIM_INJ_ALT
#define ADC_MULTI_TRIPLE_INJ_SIMULT
#define ADC_MULTI_TRIPLE_REG_SIMULT
#define ADC_MULTI_TRIPLE_REG_INTERL
#define ADC_MULTI_TRIPLE_INJ_ALTERN
#endif
#endif

#define ADC_MULTI_REG_DMA_EACH_ADC
#define ADC_MULTI_REG_DMA_LIMIT_1
#define ADC_MULTI_REG_DMA_LIMIT_2
#define ADC_MULTI_REG_DMA_LIMIT_3
#define ADC_MULTI_REG_DMA_UNLMT_1
#define ADC_MULTI_REG_DMA_UNLMT_2
#define ADC_MULTI_REG_DMA_UNLMT_3

#define ADC_MULTI_TWOSMP_DELAY_5_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_6_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_7_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_8_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_9_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_10_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_11_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_12_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_13_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_14_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_15_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_16_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_17_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_18_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_19_CYCLES
#define ADC_MULTI_TWOSMP_DELAY_20_CYCLES

#define ADC_MULTI_MASTER
#define ADC_MULTI_SLAVE
#define ADC_MULTI_MASTER_SLAVE

#define ADC_CHANNEL_0                               0x00000000U
#define ADC_CHANNEL_1                               ((uint32_t)ADC_CR1_AWDCH_0)
#define ADC_CHANNEL_2                               ((uint32_t)ADC_CR1_AWDCH_1)
#define ADC_CHANNEL_3                               ((uint32_t)(ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_4                               ((uint32_t)ADC_CR1_AWDCH_2)
#define ADC_CHANNEL_5                               ((uint32_t)(ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_6                               ((uint32_t)(ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1))
#define ADC_CHANNEL_7                               ((uint32_t)(ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_8                               ((uint32_t)ADC_CR1_AWDCH_3)
#define ADC_CHANNEL_9                               ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_10                              ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_1))
#define ADC_CHANNEL_11                              ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_12                              ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2))
#define ADC_CHANNEL_13                              ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_14                              ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1))
#define ADC_CHANNEL_15                              ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_16                              ((uint32_t)ADC_CR1_AWDCH_4)
#define ADC_CHANNEL_17                              ((uint32_t)(ADC_CR1_AWDCH_4 | ADC_CR1_AWDCH_0))
#define ADC_CHANNEL_18                              ((uint32_t)(ADC_CR1_AWDCH_4 | ADC_CR1_AWDCH_1))
#define ADC_CHANNEL_VREFINT                         ((uint32_t)ADC_CHANNEL_17)
#define ADC_CHANNEL_VBAT                            ((uint32_t)ADC_CHANNEL_18)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_ADC_CHANNEL_AS_ENUM(ENUM_ID, ADC_ID, CHANNEL_NUMBER) ENUM_ID,
#define EXPAND_X_ADC_CHANNEL_AS_STRUCT_DATA(ENUM_ID, ADC_ID, CHANNEL_NUMBER) \
                                                   { ADC_ID, CHANNEL_NUMBER },

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum ADC_ID_e
{
    #if (ADC_DRIVER_SUPPORT_ADC1_CFG == DEF_ENABLED)
        DRIVER_ADC1_ID,
    #endif

    #if (ADC_DRIVER_SUPPORT_ADC2_CFG == DEF_ENABLED)
        DRIVER_ADC2_ID,
    #endif

    #if (ADC_DRIVER_SUPPORT_ADC3_CFG == DEF_ENABLED)
        DRIVER_ADC2_ID,
    #endif

    NB_OF_ADC_DRIVER,
};

enum ADC_ChannelID_e
{
    ADC_CHANNEL_DEF(EXPAND_X_ADC_CHANNEL_AS_ENUM)
    NB_OF_ADC_CHANNEL,
};

struct ADC_Info_t
{
    ADC_ID_e            ADC_ID;
    ADC_TypeDef*        pADCx;
    uint32_t            RCC_APB2_En;
    uint32_t            Resolution;                   // ADC resolution in bits.

    uint32_t            ScanConversionMode;           // Configures the sequencer of regular and injected groups. This parameter can be associated to parameter 'DiscontinuousConvMode' to have main sequence subdivided in successive parts.
    uint32_t            DataAlign;                    // Specifies ADC data alignment, can be set to ENABLE or DISABLE
    uint8_t             PreempPrio;


    //hadc1.Init.ContinuousConvMode    = ENABLE;
    //hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    //hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    //hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    //hadc1.Init.NbrOfConversion       = 1;
    //hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  uint32_t EOCSelection;                 /*!< Specifies what EOC (End Of Conversion) flag is used for conversion by polling and interruption: end of conversion of each rank or complete sequence.
                                              This parameter can be a value of @ref ADC_EOCSelection.
                                              Note: For injected group, end of conversion (flag&IT) is raised only at the end of the sequence.
                                                    Therefore, if end of conversion is set to end of each conversion, injected group should not be used with interruption (HAL_ADCEx_InjectedStart_IT)
                                                    or polling (HAL_ADCEx_InjectedStart and HAL_ADCEx_InjectedPollForConversion). By the way, polling is still possible since driver will use an estimated timing for end of injected conversion.
                                              Note: If overrun feature is intended to be used, use ADC in mode 'interruption' (function HAL_ADC_Start_IT() ) with parameter EOCSelection set to end of each conversion or in mode 'transfer by DMA' (function HAL_ADC_Start_DMA()).
                                                    If overrun feature is intended to be bypassed, use ADC in mode 'polling' or 'interruption' with parameter EOCSelection must be set to end of sequence */
  uint32_t NbrOfConversion;              /*!< Specifies the number of ranks that will be converted within the regular group sequencer.
                                              To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                              This parameter must be a number between Min_Data = 1 and Max_Data = 16. */
  uint32_t ExternalTrigConv;             /*!< Selects the external event used to trigger the conversion start of regular group.
                                              If set to ADC_SOFTWARE_START, external triggers are disabled.
                                              If set to external trigger source, triggering is on event rising edge by default.
                                              This parameter can be a value of @ref ADC_External_trigger_Source_Regular */
  uint32_t ExternalTrigConvEdge;         /*!< Selects the external trigger edge of regular group.
                                              If trigger is set to ADC_SOFTWARE_START, this parameter is discarded.
                                              This parameter can be a value of @ref ADC_External_trigger_edge_Regular */
/*  FunctionalState DMAContinuousRequests; /!< Specifies whether the DMA requests are performed in one shot mode (DMA transfer stop when number of conversions is reached)
											  or in Continuous mode (DMA transfer unlimited, whatever number of conversions).
											  Note: In continuous mode, DMA must be configured in circular mode. Otherwise an overrun will be triggered when DMA buffer maximum pointer is reached.
											  Note: This parameter must be modified when no conversion is on going on both regular and injected groups (ADC disabled, or ADC enabled without continuous mode or external trigger that could launch a conversion).
											  This parameter can be set to ENABLE or DISABLE. */


};

struct ADC_ChannelInfo_t
{
    ADC_ID_e            ADC_ID;
    uint32_t            Channel;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class ADC_Driver
{
    public:

                        ADC_Driver              (ADC_ID_e ADC_ID);

        void            Initialize              (void);
        SystemState_e   GetStatus               (void);

        void            IRQHandler              ();

    private:

        const ADC_Info_t*       m_pInfo;
        SystemState_e           m_State;
        bool                    m_IsItInitialize;

        static bool             m_CommonInitialize_IsItDone;
        nOS_Mutex               m_Mutex;

};

class ADC_ChannelDriver
{
    public:

                        ADC_ChannelDriver       (ADC_ID_e ADC_ID);

        void            Initialize              (void);

    private:

        ADC_ChannelInfo_t*      m_pInfo;
        SystemState_e           m_State;

};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "adc_var.h"

#ifdef ADC_DRIVER_GLOBAL

const ADC_ChannelInfo_t ADC_ChannelInfo[NB_OF_ADC_CHANNEL] =
{
    ADC_CHANNEL_DEF(EXPAND_X_ADC_CHANNEL_AS_STRUCT_DATA)
}

#else

extern const ADC_ChannelInfo_t ADC_ChannelInfo[NB_OF_ADC_CHANNEL];

#endif

//-------------------------------------------------------------------------------------------------

#endif // (USE_ADC_DRIVER == DEF_ENABLED)
