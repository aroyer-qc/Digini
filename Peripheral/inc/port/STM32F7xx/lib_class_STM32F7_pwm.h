//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_pwm.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#if (USE_PWM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// typedef Typedef(s)
//-------------------------------------------------------------------------------------------------

struct PWM_Info_t
{
    IO_ID_e         PinID;
    TIM_ID_e        TimID;
    TIM_Compare_e   Channel;
    uint16_t        InitialDuty;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class PWM_Driver
{
    public:

                                        PWM_Driver              (PWM_ChannelID_e PWM_ID, TIM_Driver* pTimer);

        void                            Initialize              (void);
        void                            SetDuty                 (uint16_t Duty);
        void                            Start                   (void);
        void                            Stop                    (void);

    private:

        const PWM_Info_t*               m_pInfo;
        TIM_Driver*                     m_pTimer;
        TIM_TypeDef*                    m_pTim;
        IO_ID_e                         m_IO_Pin;
        uint16_t                        m_Duty;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "pwm_var.h"         // Project variable

extern const PWM_Info_t PWM_Info[NB_OF_PWM_CHANNEL];

//-------------------------------------------------------------------------------------------------

#endif // (USE_PWM_DRIVER == DEF_ENABLED)
