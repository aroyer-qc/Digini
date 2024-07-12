//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_pulse_counter.h
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

// Note(s)  When configuring a clock/data quadrature encoder only choose falling or rising for ISR.
//          If encoder is not incrementing in the right direction, then change only clock polarity.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"
#include "device_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef void (* PULSE_CounterCallBack_t)(uint32_t Count);  // use callback interface??

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class PULSE_Counter
{
    public:

                                PULSE_Counter           (TIM_Driver* pTimer);

        void                    Initialize              (void);
        void                    Reset                   (void);

        // Getter/Setter
        uint32_t                GetPulseCount           (void)              { return m_PulseCount;  }
        void                    SetPulseCount           (uint32_t Count)    { m_PulseCount = Count; }

        void                    RegisterCallback        (PULSE_CounterCallBack_t pCallback);  //?? use callback interface?

        void                    PulseISR                (void);

    private:

        uint32_t                        m_Overflow;
        uint32_t                        m_PulseCount;
        PULSE_CounterCallBack_t         m_pCallback;
        TIM_Driver*                     m_pTimer;

      //  static  PULSE_CounterInfo_t     m_Info[NB_OF_PULSE_COUNTER];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_PULSE_COUNTER__
#include "device_var.h"
#undef  __CLASS_PULSE_COUNTER__

//-------------------------------------------------------------------------------------------------



