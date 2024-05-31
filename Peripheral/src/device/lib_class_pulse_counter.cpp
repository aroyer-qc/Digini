//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_pulse_counter.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_PULSE_COUNTER_GLOBAL
#include "./Digini/Peripheral/inc/device/lib_class_pulse_counter.h"
#undef  LIB_PULSE_COUNTER_GLOBAL

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

//#define EXPAND_X_PULSE_COUNTER_AS_STRUCT_DATA(ENUM_ID, IO_CLK, IO_DATA, IO_PUSH) {IO_CLK, IO_DATA, IO_PUSH},

//-------------------------------------------------------------------------------------------------
//
//   Class: PULSE_Counter
//
//
//   Description:   Class to handle pulse counting
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// static variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

//PULSE_CounterInfo_t PULSE_Counter::m_Info[NB_OF_PULSE_COUNTER] =  // i think i don't need this
//{
    //QUAD_ENCODER_DEF(EXPAND_X_QUAD_ENCODER_AS_STRUCT_DATA)
//};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   PULSE_Counter
//
//   Description:   Initializes a Quad encoder object
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
PULSE_Counter::PULSE_Counter(TIM_Driver* pTimer)
{
    m_pTimer = pTimer;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Reset
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Reset state of the counter
//
//-------------------------------------------------------------------------------------------------
void PULSE_Counter::Initialize()
{
    Reset();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Reset
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Reset state of the counter
//
//-------------------------------------------------------------------------------------------------
void PULSE_Counter::Reset(void)
{
    m_Overflow   = 0;
    m_PulseCount = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterChangeCallback
//
//  Parameter(s):   pCallback       Callback pointer
//  Return:         None
//
//  Description:    Register callback for user code
//
//-------------------------------------------------------------------------------------------------
void PULSE_Counter::RegisterCallback(PULSE_CounterCallBack_t pCallback)
{
    m_pCallback = pCallback;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EncoderISR
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    ISR for the push button
//
//-------------------------------------------------------------------------------------------------
void PULSE_Counter::PulseISR(void)
{
    //??value

    //m_pCallback(Change);
}

//-------------------------------------------------------------------------------------------------
