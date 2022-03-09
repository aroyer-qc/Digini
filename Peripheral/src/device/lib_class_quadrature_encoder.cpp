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

Quad_Info_t QUAD_Encoder::m_QuadInfo[NB_OF_QUADRATURE_ENCODER] =
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
QUAD_Encoder::QUAD_Encoder(QuadEncoderID_e QuadID)
{
    if((QuadID > QUAD_ENCODER_NONE) && (QuadID < NB_OF_QUADRATURE_ENCODER))
    {
        m_pQuadInfo = &m_QuadInfo[QuadID];
    }
    else
    {
        m_pQuadInfo = nullptr;
    }

    m_Position = 0;
    m_pCallback = nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetNumberOfPosition
//
//  Parameter(s):   NumberOfPosition        number of possible selection
//  Return:         None
//
//  Description:    Set the number of position to offload application of handling this
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::SetNumberOfPosition(uint32_t NumberOfPosition)
{
    m_NumberOfPosition = NumberOfPosition;

    if(m_Position >= m_NumberOfPosition)
    {
        m_Position = m_NumberOfPosition - 1;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetPosition
//
//  Parameter(s):   Position        New position
//  Return:         None
//
//  Description:    Set position of the quadrature encoder
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::SetPosition(uint32_t Position)
{
    if(Position < m_NumberOfPosition)
    {
        m_Position = Position;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetPosition
//
//  Parameter(s):   None
//  Return:         Position        Actual position
//
//  Description:    Get the actual position of the quadrature encoder
//
//-------------------------------------------------------------------------------------------------
uint32_t QUAD_Encoder::GetPosition(void)
{
    return m_Position;
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
uint32_t QUAD_Encoder::GetPushButtonState(void)
{
    if(m_pQuadInfo != nullptr)
    {
        return IO_GetInputPin(IO_GetIO_ID(m_pQuadInfo->IO_Push));
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnablePushButtonISR
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the ISR for the push button
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::EnablePushButtonISR(void)
{
    IO_EnableIRQ(m_pQuadInfo->IO_Push);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisablePushButtonISR
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable the ISR for the push button
//
//-------------------------------------------------------------------------------------------------
void QUAD_Encoder::DisablePushButtonISR(void)
{
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
void QUAD_Encoder::RegisterChangeCallback(void* pCallback)
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
    // Need to do process for Pressed and released, maybe superkey??
    //m_pCallback
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
    // Need to do process for QUAD_INCREMENTED and QUAD_DECREMENTED
    //m_pCallback
}

//-------------------------------------------------------------------------------------------------

#endif // QUAD_ENCODER_DEF
