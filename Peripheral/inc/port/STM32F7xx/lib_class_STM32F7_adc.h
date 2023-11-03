//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_adc.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

#define ADC_CLOCK_SYNC_PCLK_DIV_2                   ((uint32_t)0x00000000)
#define ADC_CLOCK_SYNC_PCLK_DIV_4                   ((uint32_t)ADC_CCR_ADCPRE_0)
#define ADC_CLOCK_SYNC_PCLK_DIV_6                   ((uint32_t)ADC_CCR_ADCPRE_1)
#define ADC_CLOCK_SYNC_PCLK_DIV_8                   ((uint32_t)ADC_CCR_ADCPRE)

#define ADC_RESOLUTION_12_BITS                      ((uint32_t)0x00000000)
#define ADC_RESOLUTION_10_BITS                      ((uint32_t)ADC_CR1_RES_0)
#define ADC_RESOLUTION_8_BITS                       ((uint32_t)ADC_CR1_RES_1)
#define ADC_RESOLUTION_6_BITS                       ((uint32_t)ADC_CR1_RES)

#define ADC_DATA_ALIGN_RIGHT                        ((uint32_t)0x00000000)
#define ADC_DATA_ALIGN_LEFT                         ((uint32_t)ADC_CR2_ALIGN)

#define ADC_REG_TRIG_EXT_NONE                       ((uint32_t)0x00000000)
#define ADC_REG_TRIG_EXT_TIM1_CH1                   ((uint32_t)0x00000000)
#define ADC_REG_TRIG_EXT_TIM1_CH2                   ((uint32_t)0x01000000)
#define ADC_REG_TRIG_EXT_TIM1_CH3                   ((uint32_t)0x02000000)
#define ADC_REG_TRIG_EXT_TIM2_CH2                   ((uint32_t)0x03000000)
#define ADC_REG_TRIG_EXT_TIM2_CH3                   ((uint32_t)0x04000000)
#define ADC_REG_TRIG_EXT_TIM2_CH4                   ((uint32_t)0x05000000)
#define ADC_REG_TRIG_EXT_TIM2_TRGO                  ((uint32_t)0x06000000)
#define ADC_REG_TRIG_EXT_TIM3_CH1                   ((uint32_t)0x07000000)
#define ADC_REG_TRIG_EXT_TIM3_TRGO                  ((uint32_t)0x08000000)
#define ADC_REG_TRIG_EXT_TIM4_CH4                   ((uint32_t)0x09000000)
#define ADC_REG_TRIG_EXT_TIM5_CH1                   ((uint32_t)0x0A000000)
#define ADC_REG_TRIG_EXT_TIM5_CH2                   ((uint32_t)0x0B000000)
#define ADC_REG_TRIG_EXT_TIM5_CH3                   ((uint32_t)0x0C000000)
#define ADC_REG_TRIG_EXT_TIM8_CH1                   ((uint32_t)0x0D000000)
#define ADC_REG_TRIG_EXT_TIM8_TRGO                  ((uint32_t)0x0E000000)
#define ADC_REG_TRIG_EXT_EXTI_LINE11                ((uint32_t)0x0F000000)

#define ADC_REG_TRIG_EXT_EDGE_NONE                  ((uint32_t)0x00000000)
#define ADC_REG_TRIG_EXT_EDGE_RISING                ((uint32_t)0x10000000)
#define ADC_REG_TRIG_EXT_EDGE_FALLING               ((uint32_t)0x20000000)
#define ADC_REG_TRIG_EXT_EDGE_BOTH                  ((uint32_t)0x30000000)

#define ADC_SCAN_MODE_DISABLE                       ((uint32_t)0x00000000)
#define ADC_SCAN_MODE_ENABLE                        ((uint32_t)ADC_CR1_SCAN)

#define ADC_DISCONTINUOUS_MODE_DISABLE              ((uint32_t)0x00000000)
#define ADC_DISCONTINUOUS_MODE_ENABLE               ((uint32_t)ADC_CR1_DISCEN)

#define ADC_CONVERSION_SINGLE                       ((uint32_t)0x00000000)
#define ADC_CONVERSION_CONTINUOUS                   ((uint32_t)ADC_CR2_CONT)

#define ADC_DMA_CONTINUOUS_REQ_DISABLE              ((uint32_t)0x00000000)
#define ADC_DMA_CONTINUOUS_REQ_ENABLE               ((uint32_t)ADC_CR2_DDS)

#define ADC_EOC_SELECT_SEQUENCE                     ((uint32_t)0x00000000)
#define ADC_EOC_SELECT_SINGLE                       ((uint32_t)ADC_CR2_EOCS)

#define ADC_DISCONTINUOUS_NB_CHANNEL_NONE           ((uint32_t)0x00000000)
#define ADC_DISCONTINUOUS_NB_CHANNEL_1              ((uint32_t)0x00000000)
#define ADC_DISCONTINUOUS_NB_CHANNEL_2              ((uint32_t)ADC_CR1_DISCNUM_0)
#define ADC_DISCONTINUOUS_NB_CHANNEL_3              ((uint32_t)ADC_CR1_DISCNUM_1)
#define ADC_DISCONTINUOUS_NB_CHANNEL_4              ((uint32_t)(ADC_CR1_DISCNUM_1 | ADC_CR1_DISCNUM_0))
#define ADC_DISCONTINUOUS_NB_CHANNEL_5              ((uint32_t)ADC_CR1_DISCNUM_2)
#define ADC_DISCONTINUOUS_NB_CHANNEL_6              ((uint32_t)(ADC_CR1_DISCNUM_2 | ADC_CR1_DISCNUM_0))
#define ADC_DISCONTINUOUS_NB_CHANNEL_7              ((uint32_t)(ADC_CR1_DISCNUM_2 | ADC_CR1_DISCNUM_1))
#define ADC_DISCONTINUOUS_NB_CHANNEL_8              ((uint32_t)(ADC_CR1_DISCNUM_2 | ADC_CR1_DISCNUM_1 | ADC_CR1_DISCNUM_0))

/*
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
*/

// ADC SMPx mask
#define SMPR_SMP_SET                                ((uint32_t)0x00000007)
// ADC SQx mask
#define SQR_SQ_SET                                  ((uint32_t)0x0000001F)

#define ADC_NUMBER_OF_RANK_FOR_CONVERSION           16
#define ADC_NUMBER_OF_RANK_FOR_INJECTED_CONVERSION  4

// Callback type in bit position
#define ADC_CALLBACK_NONE                           0x00
#define ADC_CALLBACK_CONVERSION_COMPLETED           0x01
#define ADC_CALLBACK_ERROR                          0x02
#define ADC_CALLBACK_INJECTED_CONVERSION_COMPLETED  0x04
// ??? ADC_CALLBACK_CONVERSION_HALF_COMPLETED,
// ??? ADC_CALLBACK_LEVEL_OUT_OF_WINDOWS

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

// I have remove SCAN_MODE, EXTERNAL_TRIGGER, EXTERNAL_TRIGGER_EDGE
#define EXPAND_X_ADC_AS_ENUM(ENUM_ID, ADCx, RCC_APB2ENR_ADCxEN, RESOLUTION, CONVERSION, DISCONTINUOUS, DISCONTINUOUS_NUMBER, ADC_DATA_ALIGNMENT, EOC_SELECTION, CONTINUOUS_DMA_REQ)  ENUM_ID,
#define EXPAND_X_ADC_AS_STRUCT_DATA(ENUM_ID, ADCx, RCC_APB2ENR_ADCxEN, RESOLUTION, CONVERSION, DISCONTINUOUS, DISCONTINUOUS_NUMBER, ADC_DATA_ALIGNMENT, EOC_SELECTION, CONTINUOUS_DMA_REQ) \
                                   { ADCx, RCC_APB2ENR_ADCxEN,                                                                                                          \
              /* CR1 */              (RESOLUTION | DISCONTINUOUS | DISCONTINUOUS_NUMBER),                                                                                                            \
              /* CR2 */              (ADC_DATA_ALIGNMENT | CONVERSION | EOC_SELECTION)},

#define EXPAND_X_ADC_CHANNEL_AS_ENUM(ENUM_ID, ADC_ID, CHANNEL_NUMBER, IO_ID, SAMPLE_TIME) ENUM_ID,
#define EXPAND_X_ADC_CHANNEL_AS_STRUCT_DATA(ENUM_ID, ADC_ID, CHANNEL_NUMBER, IO_ID, SAMPLE_TIME) \
                                                   { ADC_ID, CHANNEL_NUMBER, IO_ID, SAMPLE_TIME},

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum ADC_ID_e
{
    ADC_DEF(EXPAND_X_ADC_AS_ENUM)
    NB_OF_ADC_DRIVER,
};

enum ADC_Channel_e
{
    ADC_CHANNEL_0       = 0,
    ADC_CHANNEL_1       = ((uint32_t)ADC_CR1_AWDCH_0),
    ADC_CHANNEL_2       = ((uint32_t)ADC_CR1_AWDCH_1),
    ADC_CHANNEL_3       = ((uint32_t)(ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_4       = ((uint32_t)ADC_CR1_AWDCH_2),
    ADC_CHANNEL_5       = ((uint32_t)(ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_6       = ((uint32_t)(ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1)),
    ADC_CHANNEL_7       = ((uint32_t)(ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_8       = ((uint32_t)ADC_CR1_AWDCH_3),
    ADC_CHANNEL_9       = ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_10      = ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_1)),
    ADC_CHANNEL_11      = ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_12      = ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2)),
    ADC_CHANNEL_13      = ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_14      = ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1)),
    ADC_CHANNEL_15      = ((uint32_t)(ADC_CR1_AWDCH_3 | ADC_CR1_AWDCH_2 | ADC_CR1_AWDCH_1 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_16      = ((uint32_t)ADC_CR1_AWDCH_4),
    ADC_CHANNEL_17      = ((uint32_t)(ADC_CR1_AWDCH_4 | ADC_CR1_AWDCH_0)),
    ADC_CHANNEL_18      = ((uint32_t)(ADC_CR1_AWDCH_4 | ADC_CR1_AWDCH_1)),
    ADC_CHANNEL_VREFINT = ((uint32_t)ADC_CHANNEL_17),
    ADC_CHANNEL_VREFINT = ((uint32_t)ADC_CHANNEL_17),
    ADC_CHANNEL_VBAT    = ((uint32_t)ADC_CHANNEL_18),
};

enum ADC_SampleTime_e
{
    ADC_SAMPLE_TIME_3_CYCLES   = 0x00,                     // Sample time equal to 3   cycles
    ADC_SAMPLE_TIME_15_CYCLES  = 0x01,                     // Sample time equal to 15  cycles
    ADC_SAMPLE_TIME_28_CYCLES  = 0x02,                     // Sample time equal to 28  cycles
    ADC_SAMPLE_TIME_56_CYCLES  = 0x03,                     // Sample time equal to 56  cycles
    ADC_SAMPLE_TIME_84_CYCLES  = 0x04,                     // Sample time equal to 84  cycles
    ADC_SAMPLE_TIME_112_CYCLES = 0x05,                     // Sample time equal to 112 cycles
    ADC_SAMPLE_TIME_144_CYCLES = 0x06,                     // Sample time equal to 144 cycles
    ADC_SAMPLE_TIME_480_CYCLES = 0x07,                     // Sample time equal to 480 cycles
};

enum ADC_ChannelID_e
{
    ADC_CHANNEL_DEF(EXPAND_X_ADC_CHANNEL_AS_ENUM)
    NB_OF_ADC_CHANNEL,
};

struct ADC_Info_t
{
    ADC_TypeDef*        pADCx;
    uint32_t            RCC_APB2_En;
    uint32_t            CR1_Common;                 // CR1 Resolution and Scan Conversion Mode
    uint32_t            CR2_Common;                  // CR2 ADC data alignment, Trigger, trigger edge, continuous conversion mode

    // todo CONTINUOUS_DMA_REQ not set or handle
    //uint32_t            NumberOfConversion;   // now this is config by adding channel to convert
    uint8_t             PreempPrio;
/*
// TODO verify the macro for those!!
    hadc1.Init.ContinuousConvMode    = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
*/
};

struct ADC_ChannelInfo_t
{
    ADC_ID_e            ADC_ID;
    uint32_t            Channel;
    IO_ID_e             IO_ID;
    ADC_SampleTime_e    SampleTime;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class ADC_Driver
{
    public:

                        ADC_Driver                      (ADC_ID_e ADC_ID);

        void            Initialize                      (void);
        void            RegisterCallBack                (CallbackInterface* pCallback);
        void            EnableCallbackType              (int CallBackType, void* pContext = nullptr);

        // Normal conversion group (Up to 16 Channel)
        SystemState_e   AddChannelToGroup               (ADC_ChannelID_e Channel, uint8_t Rank);        // TODO use config to define channel information!!
        SystemState_e   RemoveChannelFromGroup          (ADC_ChannelID_e Channel, uint8_t Rank);
        void            StartConversion                 (void);                         // Do a single conversion on one or more channel
        void            ConfigConversionTrigger         (void);

        // Injected group (Up to 4 Channel)
        SystemState_e   AddInjectedChannelToGroup       (ADC_ChannelID_e Channel, uint8_t Rank);        // Do a single conversion on one or more injected  channel
        SystemState_e   RemoveInjectedChannelFromGroup  (ADC_ChannelID_e Channel, uint8_t Rank);
        void            StartInjectedConversion         (void);
        void            ConfigInjectedConversionTrigger (void);

        SystemState_e   GetStatus                       (void);
        void            TempSensorVrefintControl        (bool NewState);
        void            VBAT_Control                    (bool NewState);

        void            IRQHandler                      ();

    private:

        ADC_Info_t*                     m_pInfo;
        SystemState_e                   m_State;
        bool                            m_IsItInitialize;

        CallbackInterface*              m_pCallback;
        int                             m_CallBackType;
        void*                           m_pContextConversionCompleted;
        void*                           m_pContextConversionInjectionCompleted;
        void*                           m_pContextERROR;

        uint8_t                         m_NumberOfChannel;
        uint8_t                         m_CurrentFreeChannel;
        ADC_ChannelInfo_t*              m_pChannelInfo[ADC_NUMBER_OF_RANK_FOR_CONVERSION];                     // this should represent the rank pointer on channel info
        uint8_t                         m_NumberOfInjectedChannel;
        ADC_ChannelInfo_t*              m_pInjectedChannelInfo[ADC_NUMBER_OF_RANK_FOR_INJECTED_CONVERSION];             // this should represent the rank pointer on channel info
        static bool                     m_CommonIsItInitialize;

        static const ADC_Info_t         m_ConstInfo[NB_OF_ADC_DRIVER];
        static const ADC_ChannelInfo_t  m_ConstChannelInfo[NB_OF_ADC_CHANNEL];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef ADC_DRIVER_GLOBAL

#if (ADC_DRIVER_SUPPORT_ADC1_CFG == DEF_ENABLED)
class ADC_Driver myADC1_Driver(DRIVER_ADC1_ID);
#endif

#if (ADC_DRIVER_SUPPORT_ADC2_CFG == DEF_ENABLED)
class ADC_Driver myADC2_Driver(DRIVER_ADC2_ID);
#endif

#if (ADC_DRIVER_SUPPORT_ADC3_CFG == DEF_ENABLED)
class ADC_Driver myADC3_Driver(DRIVER_ADC3_ID);
#endif

#else // ADC_DRIVER_GLOBAL

#if (ADC_DRIVER_SUPPORT_ADC1_CFG == DEF_ENABLED)
extern class ADC_Driver myADC1_Driver;
#endif

#if (ADC_DRIVER_SUPPORT_ADC2_CFG == DEF_ENABLED)
extern class ADC_Driver myADC2_Driver;
#endif

#if (ADC_DRIVER_SUPPORT_ADC3_CFG == DEF_ENABLED)
extern class ADC_Driver myADC3_Driver;
#endif

#endif // ADC_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#endif // (USE_ADC_DRIVER == DEF_ENABLED)
