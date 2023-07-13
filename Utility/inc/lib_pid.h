//-------------------------------------------------------------------------------------------------
//
//  File : lib_pid.h
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
//
// PID controller
//  Is a system control in which a correctional output is generated to guide the system toward a
//  desired setpoint (aka target). The PID calculates the output based on the following factors:
//      Gains (proportional, integral, and derivative)
//      The gains act as multipliers for their corresponding correction of PID.
//
// Proportional
//  The Proportional control introduces a linear relationship between the error and the output.
//  As the feedback grows further away from the target, the output grows proportionally stronger.
//      Proportional Correction = (P Gain) * (error)
//                              = (P Gain) * (target - feedback)
//
// Integral
//  The purpose of introducing the Integral control is to address the static error in the process.
//      Integral Correction     = (I Gain) * Integral of error over time
//      In this implementation, Integral is calculated with a running summation of the system's error, updated at each tick.
//
// Derivative
//  The Derivative control measures the rate of change of the feedback.
//      Derivative Correction   = (D Gain) * ((change in error) / (Tick change in time))
//                              = (D Gain) * ((error - lastError) / (ActualTick - lastTick))
//
// PID Equation
//  The output of a PID Controller is defined as:
//      PID Output = Proportional Correction + Integral Correction + Derivative Correction
//
// Anti-Integral Windup (TODO)
//
//  One of the common pitfalls with PID control is dealing with Integral windup. Windup is the
//  effect of accumulating too great of an error sum when the Process Variable is approaching
//  the Setpoint from far away. It results in a significant amount of overshoot and inefficient
//  control. To understand why windup occurs, it is helpful to better understand the deeper
//  mechanism of the Integral control.
//
//  The Integral component is the only part of the PID control process with long-term memory.
//  While the Proportional and Derivative components help react to live errors, the Integral
//  component is the only factor for improving the control quality over time.
//  Integral control is designed to accumulate an error sum that will offset any static error in
//  the controlled process. That is, for every process environment, there is an “ideal” amount of
//  Integral component that can perfectly offset this static error.
//
//  The natural goal and behavior for every PID controller is to adjust the error sum to the
//  “ideal” value that, once combined with the Integral constant, perfectly offsets the static
//  error.
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef float (*PID_Source_t)(void);
typedef void  (*PID_Output_t)(float);

enum PID_Direction_e
{
    PID_DIR_NORMAL,
    PID_DIR_REVERSE,
};

//-------------------------------------------------------------------------------------------------
// class
//-------------------------------------------------------------------------------------------------


class PID_Control
{
    public:

        void    Initialize                  (float Kp, float Ki, float Kd, float SetPoint, PID_Source_t PID_Source, PID_Output_t PID_Output);
        void    Process                     (void);
        void    Reset                       (void);

        void    Tune                        (float kp, float ki, float kd);

        // Getter
        float   GetProportionalValue        (void);
        float   GetIntegralValue            (void);
        float   GetDerivativeValue          (void);

        // Setter
        void    SetEnable                   (bool Enabled);
        void    SetDirection                (PID_Direction_e Direction);
        void    SetSetpoint                 (float Setpoint);

        void    SetMaxIntegralCumulation    (float Max);
        void    SetInputLimits              (float LowerLimit, float UpperLimit);
        void    SetOutputLimits             (float LowerLimit, float UpperLimit);

    private:

        void    ReverseK                    (void);

        double          m_Kp;
        double          m_Ki;
        double          m_Kd;

        bool            m_IsEnable;
        bool            m_ApplyLimitToInput;
        bool            m_ApplyLimitToOutput;
        PID_Direction_e m_Direction;

        TickCount_t     m_CurrentTick;
        TickCount_t     m_LastTick;

        float           m_Setpoint;
        float           m_Output;

        float           m_CurrentFeedback;
        float           m_LastFeedback;
        float           m_Error;
        float           m_LastError;
        float           m_IntegralCumulation;
        float           m_MaxCumulation;
        float           m_CycleDerivative;

        float           m_InputLowerLimit;
        float           m_InputUpperLimit;
        float           m_OutputLowerLimit;
        float           m_OutputUpperLimit;

        PID_Source_t    m_PID_Source;
        PID_Output_t    m_PID_Output;
};

//-------------------------------------------------------------------------------------------------
