//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_quadrature_encoder.h
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

// Note(s)  When configuring a clock/data quadrature encoder only choose falling or rising for ISR.
//          If encoder is not incrementing in the right direction, then change only clock polarity.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"
#include "device_cfg.h"

//-------------------------------------------------------------------------------------------------

#ifdef QUAD_ENCODER_GLOBAL
    #define QUAD_ENCODER_EXTERN
#else
    #define QUAD_ENCODER_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define QUAD_SUPERKEY_TIMEOUT               1000

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_QUAD_ENCODER_AS_ENUM(ENUM_ID, IO_CLK, IO_DATA, IO_PUSH) ENUM_ID,

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

#ifdef QUAD_ENCODER_DEF
enum QUAD_EncoderID_e
{
    QUAD_ENCODER_NONE = -1,
    QUAD_ENCODER_DEF(EXPAND_X_QUAD_ENCODER_AS_ENUM)
    NB_OF_QUADRATURE_ENCODER,
};

enum QUAD_EncoderChange_e
{
    QUAD_IDLE,
    QUAD_PRESSED,
    QUAD_RELEASED,
    QUAD_SUPERKEY,
    QUAD_DECREMENTED,
    QUAD_INCREMENTED,
};


struct QUAD_EncoderInfo_t
{
    IO_IrqID_e  IO_Clk;
    IO_ID_e     IO_Data;
    IO_IrqID_e  IO_Push;

};

typedef void (* QUAD_EncoderCallBack_t) (QUAD_EncoderChange_e Change);

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class QUAD_Encoder
{
    public:

                                QUAD_Encoder            (QUAD_EncoderID_e QuadID);

        void                    Process                 (void);
        void                    RegisterChangeCallback  (QUAD_EncoderCallBack_t pCallback);
        void                    Enable                  (void);
        void                    Disable                 (void);

        void                    PushButtonISR           (void);
        void                    EncoderISR              (void);

    private:

                QUAD_EncoderChange_e    m_State;
                QUAD_EncoderChange_e    m_PreviousState;
                nOS_TickCounter         m_StateTick;
                QUAD_EncoderInfo_t*     m_pQuadInfo;
                QUAD_EncoderCallBack_t  m_pCallback;
        static  QUAD_EncoderInfo_t      m_QuadInfo[NB_OF_QUADRATURE_ENCODER];
};

//-------------------------------------------------------------------------------------------------

#endif



