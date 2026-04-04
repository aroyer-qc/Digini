//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_custom_resistive_touch.h
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
// Define(s)
//-------------------------------------------------------------------------------------------------

// Number of event supported by this device
#define PDI_NUMBER_OF_EVENT             1

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CustomResistiveTouch : PointingDeviceInterface
{
    public:

        SystemState_e   Initialize                      (void* pArg)                { return SYS_READY; }
        void            Reset                           (void)                      {};
        uint8_t         DetectEvent                     (void)                      { return 0; }
        void            GetXY                           (Cartesian_t* pCartesian)   { VAR_UNUSED(pCartesian); }
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

extern class CustomResistiveTouch        PDI_ResistiveTouch;
extern class PointingDeviceInterface*    PDI_pDriver;

#ifdef LIB_CUSTOM_RSESISTIVE_TOUCH_GLOBAL
 class   CustomResistiveTouch              PDI_ResistiveTouch;
 class   PointingDeviceInterface*          PDI_pDriver = &PDI_ResistiveTouch;
#endif
