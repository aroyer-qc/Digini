//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_dac.h
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

#if (USE_DAC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DAC_CHANNEL_1                      uint32_t(0x00000000)
#define DAC_CHANNEL_2                      uint32_t(0x00000010)

// DAC Data Alignment
#define DAC_ALIGN_12B_RIGHT                 uint32_t(0x00000000)
#define DAC_ALIGN_12B_LEFT                  uint32_t(0x00000004)
#define DAC_ALIGN_8B_RIGHT                  uint32_t(0x00000008)

// DAC Trigger
#define DAC_TRIGGER_NONE                    uint32_t(0x00000000)                                        // Conversion start when the DAC1_DHRxxxx register has been loaded.
#define DAC_TRIGGER_T2_TRGO                 uint32_t(DAC_CR_TSEL1_2 | DAC_CR_TEN1)                      // TIM2 TRGO selected as external trigger.
#define DAC_TRIGGER_T4_TRGO                 uint32_t(DAC_CR_TSEL1_2 | DAC_CR_TSEL1_0 | DAC_CR_TEN1)     // TIM4 TRGO selected as external trigger.
#define DAC_TRIGGER_T5_TRGO                 uint32_t(DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0 | DAC_CR_TEN1)     // TIM5 TRGO selected as external trigger.
#define DAC_TRIGGER_T6_TRGO                 uint32_t(DAC_CR_TEN1)                                       // TIM6 TRGO selected as external trigger.
#define DAC_TRIGGER_T7_TRGO                 uint32_t(DAC_CR_TSEL1_1 | DAC_CR_TEN1)                      // TIM7 TRGO selected as external trigger.
#define DAC_TRIGGER_T8_TRGO                 uint32_t(DAC_CR_TSEL1_0 | DAC_CR_TEN1)                      // TIM8 TRGO selected as external trigger.
#define DAC_TRIGGER_EXT_IT9                 uint32_t(DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TEN1)     // EXTI Line9 event selected as external trigger.
#define DAC_TRIGGER_SOFTWARE                uint32_t(DAC_CR_TSEL1 | DAC_CR_TEN1)                        // Conversion started by software trigger.

#define DAC_OUTPUT_BUFFER_ENABLE            uint32_t(0x00000000)
#define DAC_OUTPUT_BUFFER_DISABLE           uint32_t(DAC_CR_BOFF1)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct DAC_Info_t
{
    uint32_t            Alignment;
    uint32_t            TriggerSource;
    uint32_t            OutputBuffer;

  #if (DAC_DRIVER_CHANNEL_1_CFG == DEF_ENABLED)
    IO_ID_e             IO_Channel1;
  #endif
  #if (DAC_DRIVER_CHANNEL_2_CFG == DEF_ENABLED)
    IO_ID_e             IO_Channel2;
  #endif

  #if (DAC_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
   #if (DAC_DRIVER_CHANNEL_1_CFG == DEF_ENABLED)
    uint32_t            DMA_Flag_CH1;
   #endif

   #if (DAC_DRIVER_CHANNEL_2_CFG == DEF_ENABLED)
    uint32_t            DMA_Flag_CH2;
   #endif
  #endif
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class DAC_Driver
{
    public:

        void            Initialize                      (void);
        void            RegisterCallBack                (CallbackInterface* pCallback);
        void            EnableCallbackType              (int CallBackType, void* pContext = nullptr);

        void            Start                           (uint8_t Channel);
        void            Stop                            (uint8_t Channel);
        void            SetValue                        (uint8_t Channel, uint16_t Data, uint32_t Alignment = DAC_ALIGN_12B_RIGHT);

        void            Start_DMA                       (uint8_t Channel, uint16_t *pData, size_t Length, uint32_t Alignment = DAC_ALIGN_12B_RIGHT);
        void            Stop_DMA                        (uint8_t Channel);

        SystemState_e   GetStatus                       (void);

        void            IRQHandler                      ();

    private:

        static const DAC_Info_t         m_Info;
        SystemState_e                   m_State;
        bool                            m_IsItInitialize;

        CallbackInterface*              m_pCallback;
        int                             m_CallBackType;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef DAC_DRIVER_GLOBAL

class ADC_Driver myDAC_Driver();

#else // DAC_DRIVER_GLOBAL

extern class DAC_Driver myDAC_Driver;

#endif // DAC_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#endif // (USE_DAC_DRIVER == DEF_ENABLED)
