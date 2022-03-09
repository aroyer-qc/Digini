//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_quadrature_encoder.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_QUAD_ENCODER_GLOBAL
#include "lib_class_quadrature_encoder.h"
#undef  LIB_QUAD_ENCODER_GLOBAL

//-------------------------------------------------------------------------------------------------

#ifdef QUAD_ENCODER_DEF

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_QUAD_ENCODER_AS_STRUCT_DATA(ENUM_ID, IO_CLK, IO_DATA, IO_PUSH) \
                                                     {IO_CLK, IO_DATA, IO_PUSH},

//-------------------------------------------------------------------------------------------------
//
//   Class: QUAD_Encoder
//
//
//   Description:   Class to handle Quadrature encoder
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// static variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

QUAD_EncoderInfo_t QUAD_Encoder::m_QuadInfo[NB_OF_QUADRATURE_ENCODER] =
{
    QUAD_ENCODER_DEF(EXPAND_X_QUAD_ENCODER_AS_STRUCT_DATA)
};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   QUAD_Encoder
//
//   Description:   Initializes a Quad encoder object
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
QUAD_Encoder::QUAD_Encoder(QUAD_EncoderID_e QuadID)
{
    if((QuadID > QUAD_ENCODER_NONE) && (QuadID < NB_OF_QUADRATURE_ENCODER))
    {
        m_pQuadInfo = &m_QuadInfo[QuadID];
    }
    else
    {
        m_pQuadInfo = nullptr;
    }

    m_pCallback = nullptr;
    m_State     = QUAD_IDLE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetPushButtonState
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Return the state od the push button
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::Process(void)
{
    switch(int(m_State))
    {
        case int(QUAD_PRESSED):
        {
            if( m_PreviousState != QUAD_PRESSED)
            {
                m_PreviousState = QUAD_PRESSED;
                m_StateTick = GetTick();
            }
            else
            {
                if(TickHasTimeOut(m_StateTick, QUAD_SUPERKEY_TIMEOUT) == true)
                {
                    m_State         = QUAD_SUPERKEY;
                    m_PreviousState = QUAD_SUPERKEY;
                    m_pCallback(QUAD_SUPERKEY);
                }
            }
        }
        break;

        case int(QUAD_RELEASED):
        {
            m_State         = QUAD_IDLE;
            m_PreviousState = QUAD_IDLE;
        }
        break;

        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetPushButtonState
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Return the state od the push button
//
//-------------------------------------------------------------------------------------------------
/*uint32_t QUAD_Encoder::GetPushButtonState(void)
{
    if(m_pQuadInfo != nullptr)
    {
        return IO_GetInputPin(IO_GetIO_ID(m_pQuadInfo->IO_Push));
    }

    return 0;
}
*/
//-------------------------------------------------------------------------------------------------
//
//  Name:           Enable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the ISR for the push button and encoder clock
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::Enable(void)
{
    IO_EnableIRQ(m_pQuadInfo->IO_Clk);
    IO_EnableIRQ(m_pQuadInfo->IO_Push);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Disable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable the ISR for the push button and encoder clock
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::Disable(void)
{
    IO_DisableIRQ(m_pQuadInfo->IO_Clk);
    IO_DisableIRQ(m_pQuadInfo->IO_Push);
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
void QUAD_Encoder::RegisterChangeCallback(QUAD_EncoderCallBack_t pCallback)
{
    m_pCallback = (QUAD_EncoderCallBack_t)pCallback;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PushButtonISR
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    ISR for the push button
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::PushButtonISR(void)
{
    m_State = (IO_GetInputPin(IO_GetIO_ID(m_pQuadInfo->IO_Push)) == 1) ? QUAD_PRESSED : QUAD_RELEASED;
    m_pCallback(m_State);
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
void QUAD_Encoder::EncoderISR(void)
{
    QUAD_EncoderChange_e Change;

    Change = (IO_GetInputPin(IO_GetIO_ID(m_pQuadInfo->IO_Clk)) == 1) ? QUAD_INCREMENTED : QUAD_DECREMENTED;
    m_pCallback(Change);
}

//-------------------------------------------------------------------------------------------------

#endif // QUAD_ENCODER_DEF
