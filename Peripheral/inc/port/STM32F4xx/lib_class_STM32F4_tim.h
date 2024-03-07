//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_tim.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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
//
//  Notes:   This is incomplete driver, and evolution is as the need arise
//
//              In this file -> basic timer with callback when specify time has elapse
//                  - Start
//                  - ReStart
//                  - Stop
//                  - Reload
//                  - SetCompare
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_TIM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TIM_SR_UPDATE_AND_COMPARE_MASK              (TIM_SR_UIF | TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF | TIM_SR_CC4IF)

#define TIM_OP_MODE_SINGLE_UP_COUNT                 TIM_CR1_OPM
#define TIM_OP_MODE_SINGLE_DOWN_COUNT               (TIM_CR1_OPM | TIM_CR1_DIR)
#define TIM_OP_MODE_REPETITIVE_UP_COUNT             0
#define TIM_OP_MODE_REPETITIVE_DOWN_COUNT           TIM_CR1_DIR

#if (TIM_DRIVER_SUPPORT_TIM1_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM3_CFG  == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM4_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM8_CFG  == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM9_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM10_CFG == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM11_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM12_CFG == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM13_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM14_CFG == DEF_ENABLED)
  #define TIM_DRIVER_SUPPORT_16_BITS_TIM_CFG        DEF_ENABLED
#else
  #define TIM_DRIVER_SUPPORT_16_BITS_TIM_CFG        DEF_DISABLED
#endif

#if (TIM_DRIVER_SUPPORT_TIM2_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM5_CFG == DEF_ENABLED)
  #define TIM_DRIVER_SUPPORT_32_BITS_TIM_CFG        DEF_ENABLED   // nothing yet in support for it
#else
  #define TIM_DRIVER_SUPPORT_32_BITS_TIM_CFG        DEF_DISABLED
#endif

#if (TIM_DRIVER_SUPPORT_TIM6_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM7_CFG == DEF_ENABLED)
  #define TIM_DRIVER_SUPPORT_BASIC_TIM_CFG          DEF_ENABLED
#else
  #define TIM_DRIVER_SUPPORT_BASIC_TIM_CFG          DEF_DISABLED
#endif

#if (TIM_DRIVER_SUPPORT_TIM1_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM2_CFG  == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM3_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM4_CFG  == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM5_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM6_CFG  == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM7_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM8_CFG  == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM9_CFG  == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM10_CFG == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM11_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM12_CFG == DEF_ENABLED) || \
    (TIM_DRIVER_SUPPORT_TIM13_CFG == DEF_ENABLED) || (TIM_DRIVER_SUPPORT_TIM14_CFG == DEF_ENABLED)

  #define TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG  DEF_ENABLED
#else
  #define TIM_DRIVER_SUPPORT_ANY_TIM1_TO_TIM14_CFG  DEF_DISABLED
#endif

#define TIM_IRQ_DMA_NO_SOURCE                       0x0000
#define TIM_IRQ_UPDATE                              0x0001
#define TIM_IRQ_CAPTURE_COMPARE_1                   0x0002
#define TIM_IRQ_CAPTURE_COMPARE_2                   0x0004
#define TIM_IRQ_CAPTURE_COMPARE_3                   0x0008
#define TIM_IRQ_CAPTURE_COMPARE_4                   0x0010
#define TIM_IRQ_COM                                 0x0020
#define TIM_IRQ_TRIGGER                             0x0040
#define TIM_IRQ_BREAK                               0x0080
#define TIM_DMA_UPDATE                              0x0100
#define TIM_DMA_CAPTURE_COMPARE_1                   0x0200
#define TIM_DMA_CAPTURE_COMPARE_2                   0x0400
#define TIM_DMA_CAPTURE_COMPARE_3                   0x0800
#define TIM_DMA_CAPTURE_COMPARE_4                   0x1000
#define TIM_DMA_COM                                 0x2000
#define TIM_DMA_TRIGGER                             0x4000

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

enum TIM_ID_e
{
    #if (TIM_DRIVER_SUPPORT_TIM1_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_1,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM2_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_2,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM3_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_3,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM4_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_4,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM5_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_5,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM6_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_6,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM7_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_7,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM8_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_8,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM9_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_9,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM10_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_10,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM11_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_11,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM12_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_12,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM13_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_13,
    #endif

    #if (TIM_DRIVER_SUPPORT_TIM14_CFG == DEF_ENABLED)
        TIM_DRIVER_ID_14,
    #endif

     NB_OF_TIM_DRIVER,
};

struct TIM_Info_t
{
    TIM_TypeDef*        pTIMx;
    uint32_t            RCC_APBxPeriph;
    volatile uint32_t*  RCC_APBxEN_Register;
    IRQn_Type           IRQn_Channel;
    uint8_t             PreempPrio;
    uint32_t            Mode;
    uint32_t            Prescaler;
    uint32_t            Reload;
    uint32_t            IRQ_DMA_SourceEnable;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class TIM_Driver
{
    public:

                            TIM_Driver              (TIM_ID_e TimID);

        void                Initialize              (void);
        uint32_t            GetCounterValue         (void);
        uint32_t            TimeBaseToPrescaler     (uint32_t TimeBase);
        void                Start                   (void);
        void                ReStart                 (void);
        void                Stop                    (void);
        void                SetReload               (uint32_t Value);
        uint32_t            GetReload               (void);
        static TIM_TypeDef* GetTimerPointer         (TIM_ID_e TimID);

    private:

        TIM_TypeDef*        m_pTim;
        TIM_Info_t*         m_pInfo;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "tim_var.h"         // Project variable

extern const TIM_Info_t TIM_Info[NB_OF_TIM_DRIVER];

//-------------------------------------------------------------------------------------------------

#endif // (USE_TIM_DRIVER == DEF_ENABLED)
