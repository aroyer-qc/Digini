//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_dac.h
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

#if (USE_DAC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DAC_TRIGGER_NONE                   0x00000000   // Conversion is automatic once the DAC1_DHRxxxx register has been loaded, and not by external trigger
#define DAC_TRIGGER_T2_TRGO                0x00000024   // TIM2 TRGO selected as external conversion trigger for DAC channel
#define DAC_TRIGGER_T4_TRGO                0x0000002C   // TIM4 TRGO selected as external conversion trigger for DAC channel
#define DAC_TRIGGER_T5_TRGO                0x0000001C   // TIM5 TRGO selected as external conversion trigger for DAC channel
#define DAC_TRIGGER_T6_TRGO                0x00000004   // TIM6 TRGO selected as external conversion trigger for DAC channel
#define DAC_TRIGGER_T7_TRGO                0x00000014   // TIM7 TRGO selected as external conversion trigger for DAC channel
#define DAC_TRIGGER_T8_TRGO                0x0000000C   // TIM8 TRGO selected as external conversion trigger for DAC channel
#define DAC_TRIGGER_EXTI_9                 0x00000034   // EXTI Line9 event selected as external conversion trigger for DAC channel
#define DAC_TRIGGER_SOFTWARE               0x0000003C   // Conversion started by software trigger for DAC channel

#define DAC_WAVE_GENERATION_NONE           0x00000000
#define DAC_WAVE_GENERATION_NOISE          0x00000040
#define DAC_WAVE_GENERATION_TRIANGLE       0x00000080

// Unmask DAC channel LFSR for noise wave generation
#define DAC_LFSR_UNMASK_BIT_0              0x00000000   // LFSR bit0
#define DAC_LFSR_UNMASK_BITS_1_0           0x00000100   // LFSR bit[1:0]
#define DAC_LFSR_UNMASK_BITS_2_0           0x00000200   // LFSR bit[2:0]
#define DAC_LFSR_UNMASK_BITS_3_0           0x00000300   // LFSR bit[3:0]
#define DAC_LFSR_UNMASK_BITS_4_0           0x00000400   // LFSR bit[4:0]
#define DAC_LFSR_UNMASK_BITS_5_0           0x00000500   // LFSR bit[5:0]
#define DAC_LFSR_UNMASK_BITS_6_0           0x00000600   // LFSR bit[6:0]
#define DAC_LFSR_UNMASK_BITS_7_0           0x00000700   // LFSR bit[7:0]
#define DAC_LFSR_UNMASK_BITS_8_0           0x00000800   // LFSR bit[8:0]
#define DAC_LFSR_UNMASK_BITS_9_0           0x00000900   // LFSR bit[9:0]
#define DAC_LFSR_UNMASK_BITS_10_0          0x00000A00   // LFSR bit[10:0]
#define DAC_LFSR_UNMASK_BITS_11_0          0x00000B00   // LFSR bit[11:0]

// Select max triangle amplitude
#define DAC_TRIANGLE_AMPLITUDE_1           0x00000000   // Amplitude of 1
#define DAC_TRIANGLE_AMPLITUDE_3           0x00000100   // Amplitude of 3
#define DAC_TRIANGLE_AMPLITUDE_7           0x00000200   // Amplitude of 7
#define DAC_TRIANGLE_AMPLITUDE_15          0x00000300   // Amplitude of 15
#define DAC_TRIANGLE_AMPLITUDE_31          0x00000400   // Amplitude of 31
#define DAC_TRIANGLE_AMPLITUDE_63          0x00000500   // Amplitude of 63
#define DAC_TRIANGLE_AMPLITUDE_127         0x00000600   // Amplitude of 127
#define DAC_TRIANGLE_AMPLITUDE_255         0x00000700   // Amplitude of 255
#define DAC_TRIANGLE_AMPLITUDE_511         0x00000800   // Amplitude of 511
#define DAC_TRIANGLE_AMPLITUDE_1023        0x00000900   // Amplitude of 1023
#define DAC_TRIANGLE_AMPLITUDE_2047        0x00000A00   // Amplitude of 2047
#define DAC_TRIANGLE_AMPLITUDE_4095        0x00000B00   // Amplitude of 4095

// DAC_output_buffer
#define DAC_OUTPUT_BUFFER_ENABLE           0x00000000
#define DAC_OUTPUT_BUFFER_DISABLE          0x00000002

// DAC Dual Channels SWTRIG masks
#define DAC_DUAL_SWTRIG                    (DAC_SWTRIGR_SWTRIG1 | DAC_SWTRIGR_SWTRIG2)


//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum DAC_Channel_e
{
    DAC_CHANNEL_1 = 0x00000000,
    DAC_CHANNEL_2 = 0x00000010
};

enum DAC_Wave_e
{
    DAC_WAVE_NONE     = 0x00000000,
    DAC_WAVE_NOISE    = 0x00000040,
    DAC_WAVE_TRIANGLE = 0x00000080
};

enum DAC_Alignment_e
{
    DAC_12_BITS_RIGHT = 0x00000000,
    DAC_12_BITS_LEFT  = 0x00000004,
    DAC_8_BITS_RIGHT  = 0x00000008,
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class DAC_Driver
{
    public:

        SystemState_e       Initialize                      (void);

        void                Enable                          (DAC_Channel_e Channel)                                 { SET_BIT(DAC->CR, DAC_CR_EN1 << uint32_t(Channel)); }
        void                Disable                         (DAC_Channel_e Channel)                                 { CLEAR_BIT(DAC->CR, DAC_CR_EN1 << uint32_t(Channel)); }
        void                EnableSoftwareTrigger           (DAC_Channel_e Channel)                                 { SET_BIT(DAC->SWTRIGR, DAC_SWTRIGR_SWTRIG1 <<(uint32_t(Channel) >> 4)); }
        void                DisableSoftwareTrigger          (DAC_Channel_e Channel)                                 { CLEAR_BIT(DAC->SWTRIGR, DAC_SWTRIGR_SWTRIG1 << (uint32_t(Channel) >> 4)); }
        void                EnableDualSoftwareTrigger       (void)                                                  { SET_BIT(DAC->SWTRIGR, DAC_DUAL_SWTRIG); }
        void                DisableDualSoftwareTrigger      (void)                                                  { CLEAR_BIT(DAC->SWTRIGR, DAC_DUAL_SWTRIG); }
        void                EnableWaveGeneration            (DAC_Channel_e Channel, DAC_Wave_e Wave)                { SET_BIT(DAC->CR, Wave << uint32_t(Channel)); }
        void                DisableWaveGeneration           (DAC_Channel_e Channel, DAC_Wave_e Wave)                { CLEAR_BIT(DAC->CR, Wave << uint32_t(Channel)); }

      #if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_1 ==  DEF_ENABLED) || (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_2 ==  DEF_ENABLED)
        void                DMA_Config                      (DAC_Channel_e Channel, void* pBuffer, size_t Length);
        void                DMA_Enable                      (DAC_Channel_e Channel)                                 { SET_BIT(DAC->CR, DAC_CR_DMAEN1 << uint32_t(Channel)); }
        void                DMA_Disable                     (DAC_Channel_e Channel)                                 { CLEAR_BIT(DAC->CR, DAC_CR_DMAEN1 << uint32_t(Channel)); }
      #endif

        void                IT_Enable                       (DAC_Channel_e Channel, uint32_t Interrupt)             { SET_BIT(DAC->CR, Interrupt << uint32_t(Channel)); }
        void                IT_Disable                      (DAC_Channel_e Channel, uint32_t Interrupt)             { CLEAR_BIT(DAC->CR, Interrupt << uint32_t(Channel)); }

        void                SetChannel_1                    (DAC_Alignment_e DAC_Align, uint16_t Data);
        void                SetChannel_2                    (DAC_Alignment_e DAC_Align, uint16_t Data);
        uint16_t            GetDataOutputValue              (DAC_Channel_e Channel);
        void                SetDualChannelData              (DAC_Alignment_e DAC_Align, uint16_t Data2, uint16_t Data1);

        void                ClearFlag                       (DAC_Channel_e Channel, uint32_t Flag)                  { SET_BIT(DAC->SR, Flag << uint32_t(Channel)); }
        bool                GetFlagStatus                   (DAC_Channel_e Channel, uint32_t Flag);
        bool                GetIT_Status                    (DAC_Channel_e Channel, uint32_t IT_Source);

        void                IRQHandler                      ();

    private:

      #if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_1 ==  DEF_ENABLED)
        DMA_Driver                  m_DMA_Channel_1;
        static const DMA_Info_t     m_DMA_InfoChannel_1;
      #endif

      #if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_2 ==  DEF_ENABLED)
        DMA_Driver                  m_DMA_Channel_2;
        static const DMA_Info_t     m_DMA_InfoChannel_2;
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef DAC_DRIVER_GLOBAL

class DAC_Driver myDAC_Driver;

#else // DAC_DRIVER_GLOBAL

extern class DAC_Driver myDAC_Driver;

#endif // DAC_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#endif // (USE_DAC_DRIVER == DEF_ENABLED)
