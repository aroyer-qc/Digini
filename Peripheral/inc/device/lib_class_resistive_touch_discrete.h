//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_custom_resistive_touc.h
//
//-------------------------------------------------------------------------------------------------
//
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

#pragma once

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define PDI_NUMBER_OF_EVENT             1               // Number of event supported by this device
#define PDI_PROCESS_LOOP_DELAY          2               // scan at a rate of 10 machine state = 60 Hz

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum TouchState_e
{
    TOUCH_ENERGIZE_Y_AXIS,
    TOUCH_START_ADC_X1,
    TOUCH_READ_X1,
    TOUCH_START_ADC_X2,
    TOUCH_READ_X2,
    TOUCH_ENERGIZE_X_AXIS,
    TOUCH_START_ADC_Y1,
    TOUCH_READ_Y1,
    TOUCH_START_ADC_Y2,
    TOUCH_READ_Y2,
    TOUCH_CALCULATE_XY,
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CustomResistiveTouchDriver : public PointingDeviceInterface
{
    public:

                        CustomResistiveTouchDriver      ();

        SystemState_e   Initialize                      (void* pArg);
        void            Process                         (void);
        void            Reset                           (void);
        uint8_t         DetectEvent                     (void);
        void            GetXY                           (Cartesian_t* pCartesian);
        void            SetTouchCorrection              (Cartesian_t Correction);

        uint16_t        GetRawADC                       (uint32_t Number);

    private:

        void            EnableX_Axis                    (void);
        void            EnableY_Axis                    (void);
        void            DisableAllAxis                  (void);

        Box_t           m_TouchSize;
        uint16_t        m_RawX1;
        uint16_t        m_RawX2;
        uint16_t        m_RawY1;
        uint16_t        m_RawY2;
        int16_t         m_X;
        int16_t         m_Y;
        int16_t         m_MinX_Correction;
        int16_t         m_MinY_Correction;
        int16_t         m_MaxX_Correction;
        int16_t         m_MaxY_Correction;
        uint32_t        m_TimeStamp;            // Timestamp for settle delay
        TouchState_e    m_State;
        bool            m_IsCalibrated;
        TickCount_t     m_ProcessTick;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

extern class CustomResistiveTouchDriver     PDI_ResistiveTouch;
extern class PointingDeviceInterface*       PDI_pDriver;

#ifdef LIB_RESISTIVE_TOUCH_GLOBAL
 class   CustomResistiveTouchDriver         PDI_ResistiveTouch;
 class   PointingDeviceInterface*           PDI_pDriver = &PDI_ResistiveTouch;
#endif

//-------------------------------------------------------------------------------------------------
