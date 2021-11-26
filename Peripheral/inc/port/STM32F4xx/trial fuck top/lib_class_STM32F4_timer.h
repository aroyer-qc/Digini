//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_timer.h
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


#pragma once

#ifdef STM32F4_TIMER_GLOBAL
    #define STM32F4_TIMER_EXTERN
#else
    #define STM32F4_TIMER_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "bsp_cfg.h"
#include "stm32f4xx.h"

#ifdef TIMER_DEF

//-------------------------------------------------------------------------------------------------
// struct(s) and enum(s)
//-------------------------------------------------------------------------------------------------

enum TIMER_e
{
  #define X_TIMER(ENUM_ID, HARD_PORT, MODE, PRESCALER, FREQUENCY) ENUM_ID,
    TIMER_DEF
  #undef  X_TIMER
    NB_TIMER_CONST
};

enum TIMER_port_e
{
    TIM1_HARD_PORT = 0,
    TIM2_HARD_PORT,
    TIM3_HARD_PORT,
    TIM4_HARD_PORT,
    TIM5_HARD_PORT,
    TIM6_HARD_PORT,
    TIM7_HARD_PORT,
    TIM8_HARD_PORT,
    TIM9_HARD_PORT,
    TIM10_HARD_PORT,
    TIM11_HARD_PORT,
    TIM12_HARD_PORT,
    TIM13_HARD_PORT,
    TIM14_HARD_PORT,
    NBR_OF_HARD_TIMER,
};

struct TIMER_Config_t
{
    TIMER_port_e    HardwarePort;
    uint16_t        CounterMode;
    uint16_t        Prescaler;
    uint16_t        ClockDivision;
    uint32_t        Period;
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

class TIMER_Driver
{
    public:

        void                            Initialize          (const TIMER_Config_t* pConfig);
        void                            Initialize          (TIMER_e Timer);
        uint16_t                        SetFrequency        (uint32_t Frequency);

        static  void                    Configure           (TIMER_e Timer);
        static  void                    Configure           (const TIMER_Config_t* pConfig);

        static const TIM_TypeDef*       m_pTimer[NBR_OF_HARD_TIMER];
        static const uint32_t           m_BitMask[NBR_OF_HARD_TIMER];
        static const TIMER_Config_t     m_Config[NB_TIMER_CONST];

    private:

        uint32_t                        GetGranularity      (void);

        const TIMER_Config_t*           m_pConfig;
};

//-------------------------------------------------------------------------------------------------

#endif
