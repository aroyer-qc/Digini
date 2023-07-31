//-------------------------------------------------------------------------------------------------
//
//  File : lib_pid.cpp
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

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   Kp          Proportional constant
//                  Ki          Proportional integral
//                  KD          Proportional Derivative
//                  Setpoint    Target of the PID
//                  PID_Source  Callback to get the input of the PID
//                  PID_Output  Output of the PID to the control
//  Return:         None
//
//  Description:    Initialize the PID
//
//-------------------------------------------------------------------------------------------------
void PID_Control::Initialize(float Kp, float Ki, float Kd, float Setpoint, PID_Source_t PID_Source, PID_Output_t PID_Output)
{
    // PID Constant
	m_Kp = Kp;
	m_Ki = Ki;
	m_Kd = Kd;

    // Input and output value
    m_Setpoint = Setpoint;

    // Configuration mode
    m_IsEnable           = false;
	m_ApplyLimitToInput  = false;
	m_ApplyLimitToOutput = false;
	m_Direction          = PID_DIR_NORMAL;

    // Reset on going running state
    Reset();
	m_MaxCumulation = 30000;

    // Limits for input and output
    m_InputLowerLimit  = 0;
	m_InputUpperLimit  = 0;
	m_OutputLowerLimit = 0;
	m_OutputUpperLimit = 0;

    // Callback for PID client
    m_PID_Source = PID_Source;
	m_PID_Output = PID_Output;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Reset
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Reset the PID
//
//-------------------------------------------------------------------------------------------------
void PID_Control::Reset(void)
{
    // Time tick
	m_CurrentTick        = 0;
	m_LastTick           = 0;

    // On going state
    m_CurrentFeedback    = 0;
	m_LastFeedback       = 0;
	m_LastError          = 0;
	m_IntegralCumulation = 0;
    m_Output             = 0;
}

void PID_Control::Tune(float kp, float ki, float kd)
{
	// Check for validity
	if(kp < 0 || ki < 0 || kd < 0)
    {
    	return;
    }

	m_Kp = kp;
	m_Ki = ki;
	m_Kd = kd;

	if(m_Direction == PID_DIR_REVERSE)
    {
        ReverseK();
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Calculate the PID output
//
//  Note(s):        This method should be run as fast as the source of the feedback in order to
//                  provide the highest resolution of control.
//
//-------------------------------------------------------------------------------------------------
void PID_Control::Process(void)
{
	if(m_IsEnable == true)
    {
		// Retrieve system feedback from user callback.
		m_CurrentFeedback = m_PID_Source();

        // Apply input limits if necessary.
		if(m_ApplyLimitToInput == true)
        {
            m_CurrentFeedback = AbsMax(m_CurrentFeedback, m_InputUpperLimit);
            m_CurrentFeedback = AbsMin(m_CurrentFeedback, m_InputLowerLimit);
		}

        // Calculate the error between the feedback and the target.
        m_Error = m_Setpoint - m_CurrentFeedback;

        m_CurrentTick = GetTick();
        TickCount_t ElapseTick = m_CurrentTick - m_LastTick;

        int CycleIntegral = ((m_LastError + m_Error) / 2) * ElapseTick;         // Calculate the integral of the feedback data since last cycle.
        m_IntegralCumulation += CycleIntegral;                                  // Add this cycle's integral to the integral cumulation.
        m_CycleDerivative = (m_Error - m_LastError) / ElapseTick;               // Calculate the slope of the line with data from the current and last cycles.
        m_LastTick = m_CurrentTick;                               			    // Save time data for next iteration.

		// Prevent the integral cumulation from becoming overwhelmingly huge.
        m_IntegralCumulation = AbsMax(m_IntegralCumulation, m_MaxCumulation);
        m_IntegralCumulation = AbsMin(m_IntegralCumulation, -m_MaxCumulation);

		// Calculate the system output based on data and PID gains.
		m_Output = (int)((m_Error * m_Kp) + (m_IntegralCumulation * m_Ki) + (m_CycleDerivative * m_Kd));

		// Save a record of this iteration's data.
		m_LastFeedback = m_CurrentFeedback;
		m_LastError    = m_Error;

		// Trim the output to the limits if needed.
		if(m_ApplyLimitToOutput == true)
        {
            m_Output = AbsMax(m_Output, m_OutputUpperLimit);
            m_Output = AbsMin(m_Output, m_OutputLowerLimit);
		}

		m_PID_Output(m_Output);
	}

	m_PID_Output(0);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetEnable
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Enable or disable this PID.
//
//-------------------------------------------------------------------------------------------------
void PID_Control::SetEnable(bool Enable)
{
	// If the PID was enabled and is being disabled.
	if((Enable == false) && (m_IsEnable == true))
    {
		Reset();
	}

    m_IsEnable = Enable;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetProportionalValue
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Return the value that the proportional control contributing to the output.
//
//-------------------------------------------------------------------------------------------------
float PID_Control::GetProportionalValue(void)
{
	return (m_Error * m_Kp);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetIntegralValue
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Return the value that the Integral control contributing to the output.
//
//-------------------------------------------------------------------------------------------------
float PID_Control::GetIntegralValue(void)
{
	return (m_IntegralCumulation * m_Ki);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetDerivativeValue
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Return the value that the Derivative control contributing to the output.
//
//-------------------------------------------------------------------------------------------------
float PID_Control::GetDerivativeValue(void)
{
	return (m_CycleDerivative * m_Kd);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetMaxIntegralCumulation
//
//  Parameter(s):   Max         The maximum value of the integral cumulation.
//  Return:         None
//
//  Description:    Set the maximum value that the integral cumulation can reach.
//
//-------------------------------------------------------------------------------------------------
void PID_Control::SetMaxIntegralCumulation(float Max)
{
    Max = ABS(Max);                         // If the new max value is less than 0, invert to make positive.
	if(Max > 1) m_MaxCumulation = Max;      // If the new max is not more than 1 then the cumulation is useless.
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetInputLimits
//
//  Parameter(s):   LowerLimit           The lower limit of the input.
//                  UpperLimit           The upper limit of the input.
//  Return:         None
//
//  Description:    Set the lower and upper input limit of the PID.
//
//-------------------------------------------------------------------------------------------------
void PID_Control::SetInputLimits(float LowerLimit, float UpperLimit)
{
	if(UpperLimit > LowerLimit)
    {
		m_ApplyLimitToInput = true;
		m_InputLowerLimit = LowerLimit;
		m_InputUpperLimit = UpperLimit;
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetOutputLimits
//
//  Parameter(s):   LowerLimit           The lower limit of the output.
//                  UpperLimit           The upper limit of the output.
//  Return:         None
//
//  Description:    Set the lower and upper output limit of the PID.
//
//-------------------------------------------------------------------------------------------------
void PID_Control::SetOutputLimits(float LowerLimit, float UpperLimit)
{
	if(UpperLimit > LowerLimit)
    {
		m_ApplyLimitToOutput = true;
		m_OutputLowerLimit = LowerLimit;
		m_OutputUpperLimit = UpperLimit;
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetDirection
//
//  Parameter(s):   Direction            Can be PID_DIR_NORMAL or PID_DIR_REVERSE
//  Return:         None
//
//  Description:    Set the direction of the PID.
//
//-------------------------------------------------------------------------------------------------
void PID_Control::SetDirection(PID_Direction_e Direction)
{
	if(m_Direction != Direction)
    {
        ReverseK();
	}

	m_Direction = Direction;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReverseK
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Private method to reverse the value of the K constant
//
//-------------------------------------------------------------------------------------------------
void PID_Control::ReverseK(void)
{
    m_Kp = 0 - m_Kp;
    m_Ki = 0 - m_Ki;
    m_Kd = 0 - m_Kd;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetSetpoint
//
//  Parameter(s):   Setpoint        New setpoint.
//  Return:         None
//
//  Description:    Set a new setpoint for the PID
//
//-------------------------------------------------------------------------------------------------
void PID_Control::SetSetpoint(float Setpoint)
{
    m_Setpoint = Setpoint;
}

//-------------------------------------------------------------------------------------------------

