//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_dac.h
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DAC_CHANNEL_1                                     0
#define DAC_CHANNEL_2                                     1

// DAC data alignment
#define DAC_ALIGN_12B_R                                   0x00000000U
#define DAC_ALIGN_12B_L                                   0x00000004U
#define DAC_ALIGN_8B_R                                    0x00000008U

// DAC Sample And Hold
#define DAC_SAMPLE_AND_HOLD_DISABLE                       (0x00000000)
#define DAC_SAMPLE_AND_HOLD_ENABLE                        (DAC_MCR_MODE1_2 | DAC_MCR_MODE2_2)
  
// DAC output buffer
#define DAC_OUTPUT_BUFFER_ENABLE                          (0x00000000)
#define DAC_OUTPUT_BUFFER_DISABLE                         (DAC_MCR_MODE1_1 | DAC_MCR_MODE2_1)

// DAC high frequency interface mode
#define DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE         0x00000000UL          // High frequency interface mode disabled
#define DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ     (DAC_MCR_HFSEL_0)     // High frequency interface mode compatible to AHB>80MHz enabled
#define DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_160MHZ    (DAC_MCR_HFSEL_1)     // High frequency interface mode compatible to AHB>160MHz enabled

#define DAC_SAMPLE_TIME                                   20                    // Imported from ST example for basic DAC output, Specifies the Sample time for the selected channel. This parameter applies when DAC_SampleAndHold is DAC_SAMPLEANDHOLD_ENABLE. This parameter must be a number between Min_Data = 0 and Max_Data = 1023
#define DAC_HOLD_TIME                                     10                    // Imported from ST example for basic DAC output, Specifies the hold time for the selected channel This parameter applies when DAC_SampleAndHold is DAC_SAMPLEANDHOLD_ENABLE. This parameter must be a number between Min_Data = 0 and Max_Data = 1023
#define DAC_REFRESH_TIME                                  5                     // Imported from ST example for basic DAC output, Specifies the refresh time for the selected channel This parameter applies when DAC_SampleAndHold is DAC_SAMPLEANDHOLD_ENABLE. This parameter must be a number between Min_Data = 0 and Max_Data = 255

#define DAC_BOTH_CHANNEL_ENABLE                           (DAC_CR_EN1 | DAC_CR_EN2)

#define DAC_CFG_SAMPLE_AND_HOLD                           DAC_SAMPLE_AND_HOLD_DISABLE
#define DAC_CFG_OUTPUT_BUFFER                             DAC_OUTPUT_BUFFER_ENABLE
#define DAC_CFG_ALIGNMENT                                 DAC_ALIGN_12B_R
#define DAC_CFG_HIGH_FREQUENCY_INTERFACE                  DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_160MHZ

#define DAC_CHANNEL_1_BASE                                (DAC1_BASE_NS + 0x00008UL)
#define DAC_CHANNEL_2_BASE                                (DAC1_BASE_NS + 0x00014UL)

#define DAC_CHANNEL1                                     ((DAC_Channel_t *) DAC_CHANNEL_1_BASE)
#define DAC_CHANNEL2                                     ((DAC_Channel_t *) DAC_CHANNEL_2_BASE)

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

//-------------------------------------------------------------------------------------------------

struct DAC_Channel_t
{
    volatile uint32_t DHR12;
    volatile uint32_t DHL12;
    volatile uint32_t DHR8;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class DAC_Driver
{
    public:

                        DAC_Driver          ();

        SystemState_e   Initialize          (void);
        SystemState_e   WriteDAC            (uint32_t channel, uint32_t Value);

    private:

        bool                    m_IsItInitialize;
};

//-------------------------------------------------------------------------------------------------

#endif // __cplusplus
