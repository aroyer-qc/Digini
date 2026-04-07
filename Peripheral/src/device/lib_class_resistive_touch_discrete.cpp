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

SystemState_e CustomResistiveTouchDriver::Initialize(void* pArg)
{
    m_ProcessTick = GetTick() + PDI_PROCESS_LOOP_DELAY;

    m_X_Correction = 0;
    m_Y_Correction = 0;
    m_IsCalibrated = false;

    VAR_UNUSED(pArg);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

uint16_t X1[4];
uint16_t X2[4];
uint16_t Y1[4];
uint16_t Y2[4];


void CustomResistiveTouchDriver::Process(void)
{
    if(TickHasTimeOut(m_ProcessTick, PDI_PROCESS_LOOP_DELAY) == true)
    {
        m_ProcessTick = GetTick();


        // from IDLE -> Begin X-axis drive
        DisableAllAxis();
        EnableX_Axis();
        LIB_Delay_uSec(30);                     // Wait time (we are in the idle task)

        myADC2_Driver.StartSingleConversion(ADC_CHANNEL_X1);
        while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}
        X1[0] = myADC2_Driver.ReadSingleConversionValue();

myADC2_Driver.StartSingleConversion(ADC_CHANNEL_X2);
while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}
X2[0] = myADC2_Driver.ReadSingleConversionValue();

myADC2_Driver.StartSingleConversion(ADC_CHANNEL_Y1);
while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}
Y1[0] = myADC2_Driver.ReadSingleConversionValue();

myADC2_Driver.StartSingleConversion(ADC_CHANNEL_Y2);
while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}
Y2[0] = myADC2_Driver.ReadSingleConversionValue();

        DisableAllAxis();
        EnableY_Axis();
        LIB_Delay_uSec(30);                     // Wait time (we are in the idle task)
        myADC2_Driver.StartSingleConversion(ADC_CHANNEL_X1);
        while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}

        X1[1] = myADC2_Driver.ReadSingleConversionValue();

myADC2_Driver.StartSingleConversion(ADC_CHANNEL_X2);
while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}
X2[1] = myADC2_Driver.ReadSingleConversionValue();

myADC2_Driver.StartSingleConversion(ADC_CHANNEL_Y1);
while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}
Y1[1] = myADC2_Driver.ReadSingleConversionValue();

myADC2_Driver.StartSingleConversion(ADC_CHANNEL_Y2);
while(myADC2_Driver.GetStatusSingleConversion() != SYS_READY) {}
Y2[1] = myADC2_Driver.ReadSingleConversionValue();

        //if((m_rawX < TOUCH_MIN) || (m_rawY < TOUCH_MIN))
        //{
        //    m_X = 0;
        //    m_Y = 0;
        //}
        //else
        DisableAllAxis();

        {
            __asm("nop");
       //     m_X = m_rawX + m_X_Correction;
       //     m_Y = m_rawY + m_Y_Correction;
        }
    }
}

void CustomResistiveTouchDriver::EnableX_Axis(void)
{
    IO_SetPinLow(IO_TOUCH_X1_SWITCH);      // Disable Y
    IO_SetPinLow(IO_TOUCH_Y1_SWITCH);
}
void CustomResistiveTouchDriver::EnableY_Axis(void)
{
    IO_SetPinLow(IO_TOUCH_X2_SWITCH);      // Disable Y
    IO_SetPinLow(IO_TOUCH_Y2_SWITCH);
}
void CustomResistiveTouchDriver::DisableAllAxis(void)
{
    // TO Be on the safe side.. they are all OD so fully disabled
    IO_SetPinHigh(IO_TOUCH_Y1_SWITCH);      // Disable Y
    IO_SetPinHigh(IO_TOUCH_Y2_SWITCH);
    IO_SetPinHigh(IO_TOUCH_X1_SWITCH);
    IO_SetPinHigh(IO_TOUCH_X2_SWITCH);      // Enable X
}


//-------------------------------------------------------------------------------------------------

void CustomResistiveTouchDriver::Reset(void)
{
}

//-------------------------------------------------------------------------------------------------

uint8_t CustomResistiveTouchDriver::DetectEvent(void)
{
    return 0;
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
    m_X_Correction = Correction.X;
    m_Y_Correction = Correction.Y;
    m_IsCalibrated = true;
}

//-------------------------------------------------------------------------------------------------

