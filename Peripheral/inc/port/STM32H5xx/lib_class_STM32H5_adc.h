//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_adc.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "adc_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define BSP_ADC_CLOCK_PRESC_DIV_NONE                ((uint32_t)0x00000000)
#define BSP_ADC_CLOCK_PRESC_DIV_2                   ((uint32_t)ADC_CCR_PRESC_0)
#define BSP_ADC_CLOCK_PRESC_DIV_4                   ((uint32_t)ADC_CCR_PRESC_1)
#define BSP_ADC_CLOCK_PRESC_DIV_6                   ((uint32_t)ADC_CCR_PRESC_1 | ADC_CCR_PRESC_0)
#define BSP_ADC_CLOCK_PRESC_DIV_8                   ((uint32_t)ADC_CCR_PRESC_2)
#define BSP_ADC_CLOCK_PRESC_DIV_10                  ((uint32_t)ADC_CCR_PRESC_2 | ADC_CCR_PRESC_0)
#define BSP_ADC_CLOCK_PRESC_DIV_12                  ((uint32_t)ADC_CCR_PRESC_2 | ADC_CCR_PRESC_1)
#define BSP_ADC_CLOCK_PRESC_DIV_16                  ((uint32_t)ADC_CCR_PRESC_2 | ADC_CCR_PRESC_1 | ADC_CCR_PRESC_0)
#define BSP_ADC_CLOCK_PRESC_DIV_32                  ((uint32_t)ADC_CCR_PRESC_3)
#define BSP_ADC_CLOCK_PRESC_DIV_64                  ((uint32_t)ADC_CCR_PRESC_3 | ADC_CCR_PRESC_0)
#define BSP_ADC_CLOCK_PRESC_DIV_128                 ((uint32_t)ADC_CCR_PRESC_3 | ADC_CCR_PRESC_1)
#define BSP_ADC_CLOCK_PRESC_DIV_256                 ((uint32_t)ADC_CCR_PRESC_3 | ADC_CCR_PRESC_1 | ADC_CCR_PRESC_0)

#define BSP_ADC_RESOLUTION_12_BITS                  ((uint32_t)0x00000000)
#define BSP_ADC_RESOLUTION_10_BITS                  ((uint32_t)ADC_CFGR_RES_0)
#define BSP_ADC_RESOLUTION_8_BITS                   ((uint32_t)ADC_CFGR_RES_1)
#define BSP_ADC_RESOLUTION_6_BITS                   ((uint32_t)ADC_CFGR_RES)

#define BSP_ADC_DATA_ALIGN_RIGHT                    ((uint32_t)0x00000000)
#define BSP_ADC_DATA_ALIGN_LEFT                     ((uint32_t)ADC_CFGR_ALIGN)

#define BSP_ADC_OVERRUN_MODE_PRESERVE               ((uint32_t)0x00000000)
#define BSP_ADC_OVERRUN_MODE_OVERWRITE              ((uint32_t)ADC_CFGR_OVRMOD)

#define BSP_ADC_REG_TRIG_SOFTWARE                   ((uint32_t)0x00000000UL)
#define BSP_ADC_REG_TRIG_EXT_TIM1_CH1               ((uint32_t)0x00000000UL)
#define BSP_ADC_REG_TRIG_EXT_TIM1_CH2               ((uint32_t)ADC_CFGR_EXTSEL_0)
#define BSP_ADC_REG_TRIG_EXT_TIM1_CH3               ((uint32_t)ADC_CFGR_EXTSEL_1)
#define BSP_ADC_REG_TRIG_EXT_TIM2_CH2               ((uint32_t)ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0)
#define BSP_ADC_REG_TRIG_EXT_TIM3_TRGO              ((uint32_t)ADC_CFGR_EXTSEL_2)
#define BSP_ADC_REG_TRIG_EXT_EXTI_LINE11            ((uint32_t)ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_1)
#define BSP_ADC_REG_TRIG_EXT_TIM1_TRGO              ((uint32_t)ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_0)
#define BSP_ADC_REG_TRIG_EXT_TIM1_TRGO2             ((uint32_t)ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_1)
#define BSP_ADC_REG_TRIG_EXT_TIM2_TRGO              ((uint32_t)ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0)
#define BSP_ADC_REG_TRIG_EXT_TIM6_TRGO              ((uint32_t)ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_0)
#define BSP_ADC_REG_TRIG_EXT_TIM3_CH4               ((uint32_t)ADC_CFGR_EXTSEL_3 | ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0)
#define BSP_ADC_REG_TRIG_EXT_EXTI_LINE15            ((uint32_t)ADC_CFGR_EXTSEL_4 )
#define BSP_ADC_REG_TRIG_EXT_LPTIM1_CH1             ((uint32_t)ADC_CFGR_EXTSEL_4 | ADC_CFGR_EXTSEL_1)
#define BSP_ADC_REG_TRIG_EXT_LPTIM2_CH1             ((uint32_t)ADC_CFGR_EXTSEL_4 | ADC_CFGR_EXTSEL_1 | ADC_CFGR_EXTSEL_0 )

#define BSP_ADC_REG_TRIG_EXT_EDGE_NONE              ((uint32_t)0x00000000)
#define BSP_ADC_REG_TRIG_EXT_EDGE_RISING            ((uint32_t)ADC_CFGR_EXTEN_0)
#define BSP_ADC_REG_TRIG_EXT_EDGE_FALLING           ((uint32_t)ADC_CFGR_EXTEN_1)
#define BSP_ADC_REG_TRIG_EXT_EDGE_BOTH              ((uint32_t)ADC_CFGR_EXTEN)

#define BSP_ADC_INJECTED_QUEUE_ENABLE               ((uint32_t)0x00000000)
#define BSP_ADC_INJECTED_QUEUE_DISABLE              ((uint32_t)ADC_CFGR_JQDIS)

#define BSP_ADC_DISCONTINUOUS_MODE_DISABLE          ((uint32_t)0x00000000)
#define BSP_ADC_DISCONTINUOUS_MODE_ENABLE           ((uint32_t)ADC_CFGR_DISCEN)

#define BSP_ADC_CONVERSION_SINGLE                   ((uint32_t)0x00000000)
#define BSP_ADC_CONVERSION_CONTINUOUS               ((uint32_t)ADC_CFGR_CONT)

#define BSP_ADC_DMA_MODE_DISABLE                    ((uint32_t)0x00000000)
#define BSP_ADC_DMA_MODE_ENABLE                     ((uint32_t) ADC_CFGR_DMAEN)

#define BSP_ADC_DMA_CONTINUOUS_REQ_DISABLE          ((uint32_t)0x00000000)
#define BSP_ADC_DMA_CONTINUOUS_REQ_ENABLE           ((uint32_t)ADC_CFGR_DMACFG)

#define BSP_ADC_SAMPLING_MODE_NORMAL                ((uint32_t)0x00000000UL)
#define BSP_ADC_SAMPLING_MODE_BULB                  ((uint32_t)ADC_CFGR2_BULB)
#define BSP_ADC_SAMPLING_MODE_TRIGGER_CONTROLED     ((uint32_t)ADC_CFGR2_SMPTRIG)

// Oversampling mode
// The oversampling options are currently not supported
#define BSP_ADC_REG_OVERSAMPLING_DISABLE            ((uint32_t)0x00000000)
#define BSP_ADC_REG_OVERSAMPLING_ENABLE             ((uint32_t)ADC_CFGR2_ROVSE)

#define BSP_ADC_EOC_SELECT_SEQUENCE                 ((uint32_t)0x00000000)
#define BSP_ADC_EOC_SELECT_SINGLE                   ((uint32_t)ADC_CR2_EOCS)

#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_NONE       ((uint32_t)0x00000000)
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_1          ((uint32_t)0x00000000)
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_2          ((uint32_t)ADC_CFGR_DISCNUM_0)
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_3          ((uint32_t)ADC_CFGR_DISCNUM_1)
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_4          ((uint32_t)(ADC_CFGR_DISCNUM_1 | ADC_CR1_DISCNUM_0))
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_5          ((uint32_t)ADC_CFGR_DISCNUM_2)
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_6          ((uint32_t)(ADC_CFGR_DISCNUM_2 | ADC_CR1_DISCNUM_0))
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_7          ((uint32_t)(ADC_CFGR_DISCNUM_2 | ADC_CR1_DISCNUM_1))
#define BSP_ADC_DISCONTINUOUS_NB_CHANNEL_8          ((uint32_t)(ADC_CFGR_DISCNUM_2 | ADC_CR1_DISCNUM_1 | ADC_CR1_DISCNUM_0))

#define BSP_ADC_CHANNEL_0                           ((uint32_t)0x00000000U)
#define BSP_ADC_CHANNEL_1                           ((uint32_t)0x00000001U)
#define BSP_ADC_CHANNEL_2                           ((uint32_t)0x00000002U)
#define BSP_ADC_CHANNEL_3                           ((uint32_t)0x00000003U)
#define BSP_ADC_CHANNEL_4                           ((uint32_t)0x00000004U)
#define BSP_ADC_CHANNEL_5                           ((uint32_t)0x00000005U)
#define BSP_ADC_CHANNEL_6                           ((uint32_t)0x00000006U)
#define BSP_ADC_CHANNEL_7                           ((uint32_t)0x00000007U)
#define BSP_ADC_CHANNEL_8                           ((uint32_t)0x00000008U)
#define BSP_ADC_CHANNEL_9                           ((uint32_t)0x00000009U)
#define BSP_ADC_CHANNEL_10                          ((uint32_t)0x0000000AU)
#define BSP_ADC_CHANNEL_11                          ((uint32_t)0x0000000BU)
#define BSP_ADC_CHANNEL_12                          ((uint32_t)0x0000000CU)
#define BSP_ADC_CHANNEL_13                          ((uint32_t)0x0000000DU)
#define BSP_ADC_CHANNEL_14                          ((uint32_t)0x0000000EU)
#define BSP_ADC_CHANNEL_15                          ((uint32_t)0x0000000FU)
#define BSP_ADC_CHANNEL_16                          ((uint32_t)0x00000010U)
#define BSP_ADC_CHANNEL_17                          ((uint32_t)0x00000011U)
#define BSP_ADC_CHANNEL_18                          ((uint32_t)0x00000012U)
#define BSP_ADC_CHANNEL_19                          ((uint32_t)0x00000013U)

// Internal channels
#define BSP_ADC_CHANNEL_INTERNAL_MSK                ((uint32_t)0x80000000)
#define BSP_ADC_CHANNEL_TEMP                        ((uint32_t)BSP_ADC_CHANNEL_16 | BSP_ADC_CHANNEL_INTERNAL_MSK)
#define BSP_ADC_CHANNEL_VREFINT                     ((uint32_t)BSP_ADC_CHANNEL_17 | BSP_ADC_CHANNEL_INTERNAL_MSK)
#define BSP_ADC_CHANNEL_VBAT                        ((uint32_t)BSP_ADC_CHANNEL_2  | BSP_ADC_CHANNEL_INTERNAL_MSK)


#define BSP_ADC_SAMPLETIME_2CYCLES_5                0x00000000U
#define BSP_ADC_SAMPLETIME_6CYCLES_5                ((uint32_t)ADC_SMPR1_SMP0_0)
#define BSP_ADC_SAMPLETIME_12CYCLES_5               ((uint32_t)ADC_SMPR1_SMP0_1)
#define BSP_ADC_SAMPLETIME_24CYCLES_5               ((uint32_t)(ADC_SMPR1_SMP0_1 | ADC_SMPR1_SMP0_0))
#define BSP_ADC_SAMPLETIME_47CYCLES_5               ((uint32_t)ADC_SMPR1_SMP0_2)
#define BSP_ADC_SAMPLETIME_92CYCLES_5               ((uint32_t)(ADC_SMPR1_SMP0_2 | ADC_SMPR1_SMP0_0))
#define BSP_ADC_SAMPLETIME_247CYCLES_5              ((uint32_t)(ADC_SMPR1_SMP0_2 | ADC_SMPR1_SMP0_1))
#define BSP_ADC_SAMPLETIME_640CYCLES_5              ((uint32_t)(ADC_SMPR1_SMP0_2 | ADC_SMPR1_SMP0_1 | ADC_SMPR1_SMP0_0))

#define BSP_ADC_CR_PROPERTY_BITS                    (ADC_CR_ADCAL | ADC_CR_ADEN | ADC_CR_ADDIS | ADC_CR_JADSTART | ADC_CR_JADSTP | ADC_CR_ADSTART | ADC_CR_ADSTP)

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

#define ADC_CALIBRATION_TIMEOUT                     (633600000UL)                       // ADC calibration time-out value

// Internal voltage reference VrefInt
// NOTE: macros redirect to RAM-cached copies (pre-read before ICACHE via ReadFactoryCalibration())
#define ADC_VREFINT_CAL_ADDR                        ADC_Driver::s_VrefintCal  // 0x08FFF810 — read before ICACHE
#define ADC_VREFINT_CAL_VREF                        3300                                    // Analog voltage reference (Vref+)

// Internal Temp sensor reference
#define TS_CAL1_ADDR 		                        ADC_Driver::s_TsCal1      // 0x08FFF814 — read before ICACHE
#define TS_CAL2_ADDR 		                        ADC_Driver::s_TsCal2      // 0x08FFF818 — read before ICACHE
#define TS_TEMP_OFFSET 		                        float(30)                               // Temp offset
#define TS_TEMP_RANGE 		                        float(80)                               // Temp Range

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_ADC_CHANNEL_AS_ENUM(ENUM_ID, ADC_ID, CHANNEL_NUMBER, IO_ID, SAMPLE_TIME) ENUM_ID,
#define EXPAND_X_ADC_CHANNEL_AS_STRUCT_DATA(ENUM_ID, ADC_ID, CHANNEL_NUMBER, IO_ID, SAMPLE_TIME) \
    { \
        DRIVER_ ## ADC_ID ## _ID, \
        BSP_ADC_CHANNEL_ ## CHANNEL_NUMBER, \
        IO_ID, \
        SAMPLE_TIME \
    },

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

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

    NB_OF_ADC_DRIVER,
};

enum ADC_ChannelID_e
{
    ADC_CHANNEL_DEF(EXPAND_X_ADC_CHANNEL_AS_ENUM)
    NB_OF_ADC_CHANNEL,
};

struct ADC_Info_t
{
    ADC_TypeDef*        pADCx;
    
    // IRQ number
    IRQn_Type           IRQn;

    // ADC Configuration registers
    uint32_t            CFGR_Common;
    uint32_t            CFGR2_Common;
    uint32_t            CCR_Common;

    DMA_Info_t          DMA_ADC;
};

struct ADC_ChannelInfo_t
{
    ADC_ID_e            ADC_ID;
    uint32_t            Channel;
    IO_ID_e             IO_ID;
    uint32_t            SampleTime;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class ADC_Driver
{
    public:

                        ADC_Driver                      (ADC_ID_e ADC_ID);

        void            Initialize                      (void);
        void            RegisterCallBack                (DmaCallback_t pCallback);
        void            EnableCallbackType              (int CallBackType, void* pContext = nullptr);

        // Normal conversion group (Up to 16 Channel)
        SystemState_e   AddChannelToGroup               (ADC_ChannelID_e Channel);
        //SystemState_e   RemoveChannelFromGroup          (ADC_ChannelID_e Channel, uint8_t Rank);
        void            StartConversion                 (uint32_t* buffer, size_t size); // Start converting regular channels
        //void            ConfigConversionTrigger         (void);

        // Injected group (Up to 4 Channel)
        #if (ADC_DRIVER_SUPPORT_INJ_CONV_CFG == DEF_ENABLED)
          SystemState_e   AddInjectedChannelToGroup       (ADC_ChannelID_e Channel, uint8_t Rank);        // Do a single conversion on one or more injected  channel
          //SystemState_e   RemoveInjectedChannelFromGroup  (ADC_ChannelID_e Channel, uint8_t Rank);
          void            StartInjectedConversion         (void);
          //void            ConfigInjectedConversionTrigger (void);
        #endif

        SystemState_e   GetStatus                       (void);

        static void     DmaIRQHandler                   (ADC_ID_e ADC_ID);

        // Must be called ONCE before HAL_ICACHE_Enable() to capture factory
        // calibration values from system flash (0x08FFF8xx) into RAM.
        // Reading those addresses after ICACHE is active can return aliased data.
        static void     ReadFactoryCalibration          (void);

        // RAM-cached factory calibration values (populated by ReadFactoryCalibration)
        static uint16_t s_VrefintCal;  // raw at 30°C / 3.3 V (0x08FFF810)
        static uint16_t s_TsCal1;      // temp sensor raw at 30°C  (0x08FFF814)
        static uint16_t s_TsCal2;      // temp sensor raw at 110°C (0x08FFF818)

    private:

        void            InitCalibration                 (void);
        void            VrefintControl                  (bool NewState);
        void            TempSensorControl               (bool NewState);
        void            VBAT_Control                    (bool NewState);

        ADC_ID_e                        m_ADC_ID;
        ADC_Info_t*                     m_pInfo;
        SystemState_e                   m_State;
        bool                            m_IsInitialized;
        static bool                     m_CommonIsInitialized;

        DmaCallback_t                   m_pCallback;
        int                             m_CallBackType;

        void*                           m_pContextConversionCompleted;
        uint8_t                         m_CurrentFreeChannel;
        ADC_ChannelInfo_t*              m_pChannelInfo[ADC_NUMBER_OF_RANK_FOR_CONVERSION];                     // this should represent the rank pointer on channel info

        #if (ADC_DRIVER_SUPPORT_INJ_CONV_CFG == DEF_ENABLED)
          void*                           m_pContextConversionInjectionCompleted;
          uint8_t                         m_NumberOfInjectedChannel;
          ADC_ChannelInfo_t*              m_pInjectedChannelInfo[ADC_NUMBER_OF_RANK_FOR_INJECTED_CONVERSION];   // this should represent the rank pointer on channel info
        #endif

        #if (ADC_DRIVER_SUPPORT_ERROR_CFG == DEF_ENABLED)
          void*                           m_pContextERROR;
        #endif

        DMA_Driver                      m_DMA_ADC;

        static ADC_Driver*              m_pDriver[NB_OF_ADC_DRIVER];
        static const ADC_ChannelInfo_t  m_ConstChannelInfo[NB_OF_ADC_CHANNEL];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "adc_var.h"

//-------------------------------------------------------------------------------------------------

#endif // __cplusplus