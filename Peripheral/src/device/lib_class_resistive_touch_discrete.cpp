//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_custom_resistive_touch.cpp
//
//-------------------------------------------------------------------------------------------------
// Copyright(c) 2026 Alain Royer.
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

#define LIB_RESISTIVE_TOUCH_GLOBAL
#include "./lib_digini.h"
#undef  LIB_RESISTIVE_TOUCH_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TOUCH_RESOLUTION_12_BITS        4095
#define TOUCH_MIN_X_INITIAL_CORRECTION

//-------------------------------------------------------------------------------------------------

CustomResistiveTouchDriver::CustomResistiveTouchDriver()
{
    m_MinX_Correction = 190;
    m_MaxX_Correction = 3900;
    m_MinY_Correction = 180;
    m_MaxY_Correction = 3850;
    m_X = -1;
    m_Y = -1;
    m_IsCalibrated = false;
    m_State        = TOUCH_ENERGIZE_Y_AXIS;
}

//-------------------------------------------------------------------------------------------------

SystemState_e CustomResistiveTouchDriver::Initialize(void* pArg)
{
    m_ProcessTick = GetTick() + PDI_PROCESS_LOOP_DELAY;
    VAR_UNUSED(pArg);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::Process(void)
{
    if(myADC2_Driver.GetStatusSingleConversion() == SYS_READY)
    {
        if(TickHasTimeOut(m_ProcessTick, PDI_PROCESS_LOOP_DELAY) == true)
        {
            m_ProcessTick = GetTick();

            switch(m_State)
            {
                case TOUCH_ENERGIZE_Y_AXIS:
                {
                    DisableAllAxis();
                    EnableY_Axis();
                }
                break;

                case TOUCH_START_ADC_X1:
                {
                    myADC2_Driver.StartSingleConversion(ADC_CHANNEL_X1);
                }
                break;

                case TOUCH_READ_X1:
                {
                    m_RawX1 = TOUCH_RESOLUTION_12_BITS - myADC2_Driver.ReadSingleConversionValue();
                }
                break;

                case TOUCH_START_ADC_X2:
                {
                    myADC2_Driver.StartSingleConversion(ADC_CHANNEL_X2);
                }
                break;

                case TOUCH_READ_X2:
                {
                    m_RawX2 = TOUCH_RESOLUTION_12_BITS - myADC2_Driver.ReadSingleConversionValue();
                }
                break;

                case TOUCH_ENERGIZE_X_AXIS:
                {
                    DisableAllAxis();
                    EnableX_Axis();
                }
                break;

                case TOUCH_START_ADC_Y1:
                {
                    myADC2_Driver.StartSingleConversion(ADC_CHANNEL_Y1);
                }
                break;

                case TOUCH_READ_Y1:
                {
                    m_RawY1 = TOUCH_RESOLUTION_12_BITS - myADC2_Driver.ReadSingleConversionValue();
                }
                break;

                case TOUCH_START_ADC_Y2:
                {
                    myADC2_Driver.StartSingleConversion(ADC_CHANNEL_Y2);
                }
                break;

                case TOUCH_READ_Y2:
                {
                    m_RawY2 = TOUCH_RESOLUTION_12_BITS - myADC2_Driver.ReadSingleConversionValue();
                }
                break;

                case TOUCH_CALCULATE_XY:
                {
                    // Average the two raw samples
                    uint16_t RawX = (m_RawX1 + m_RawX2) >> 1;
                    uint16_t RawY = (m_RawY1 + m_RawY2) >> 1;

                    // Compute spans
                    int32_t SpanX = m_MaxX_Correction - m_MinX_Correction;
                    int32_t SpanY = m_MaxY_Correction - m_MinY_Correction;

                    // Safety
                    if(SpanX <= 0 || SpanY <= 0)
                    {
                        m_X = -1;
                        m_Y = -1;
                        break;
                    }

                    // Detect "no touch" if raw values fall outside the extended valid range
                    if((RawX < m_MinX_Correction) ||
                       (RawX > m_MaxX_Correction) ||
                       (RawY < m_MinY_Correction) ||
                       (RawY > m_MaxY_Correction))
                    {
                        m_X = -1;   // No touch detected
                        m_Y = -1;
                        break;
                    }

                    // Apply calibration
                    int32_t x = RawX - m_MinX_Correction;
                    int32_t y = RawY - m_MinY_Correction;

                    // Scale to grid
                    x = (x * GRAFX_TOUCH_SIZE_X) / SpanX;
                    y = (y * GRAFX_TOUCH_SIZE_Y) / SpanY;

                    // Clamp
                    if(x < 0) x = 0;
                    if(x >= GRAFX_TOUCH_SIZE_X) x = GRAFX_TOUCH_SIZE_X - 1;

                    if(y < 0) y = 0;
                    if(y >= GRAFX_TOUCH_SIZE_Y) y = GRAFX_TOUCH_SIZE_Y - 1;

                    // Output
                    m_X = (int16_t)x;
                    m_Y = (int16_t)y;
                }
                break;
            }

            m_State = (m_State == TOUCH_CALCULATE_XY) ? TOUCH_ENERGIZE_Y_AXIS : TouchState_e(int(m_State) + 1);
        }
    }
}

//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::EnableX_Axis(void)
{
    IO_SetPinHigh(IO_TOUCH_X1_SWITCH);
    IO_SetPinLow(IO_TOUCH_X2_SWITCH);
}

//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::EnableY_Axis(void)
{
    IO_SetPinLow(IO_TOUCH_Y1_SWITCH);
    IO_SetPinHigh(IO_TOUCH_Y2_SWITCH);
}

//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::DisableAllAxis(void)
{
    IO_SetPinLow(IO_TOUCH_X1_SWITCH);
    IO_SetPinHigh(IO_TOUCH_X2_SWITCH);
    IO_SetPinHigh(IO_TOUCH_Y1_SWITCH);
    IO_SetPinLow(IO_TOUCH_Y2_SWITCH);
}

//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::Reset(void)
{
}

//-------------------------------------------------------------------------------------------------

uint8_t CustomResistiveTouchDriver::DetectEvent(void)
{
    return ((m_X == -1) || (m_Y == -1)) ? 0 : 1;
}

//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::GetXY(Cartesian_t* pCartesian)
{
    pCartesian->X = m_X;
    pCartesian->Y = m_Y;
}

//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::SetTouchCorrection(Cartesian_t Correction)
{
   // m_X_Correction = Correction.X;
   // m_Y_Correction = Correction.Y;
    m_IsCalibrated = true;
}

//-------------------------------------------------------------------------------------------------

uint16_t CustomResistiveTouchDriver::GetRawADC(uint32_t Number)
{
    switch(Number)
    {
        case 0: return m_RawX1;
        case 1: return m_RawX2;
        case 2: return m_RawY1;
        case 3: return m_RawY2;
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------

