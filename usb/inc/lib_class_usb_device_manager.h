//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_device_manager.h
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

#include "../../Peripheral/inc/port/interface/lib_class_usb_device.h"

//-------------------------------------------------------------------------------------------------

#if (USE_USB_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class USB_DeviceManager
{
    public:

                        USB_DeviceManager       (void);

        // Task
        void            Run                     (void);
        SystemState_e   Initialize              (USB_DeviceDriver& Device);

    private:

        nOS_Thread        m_Handle;
        nOS_Stack         m_Stack[USB_DEVICE_MANAGER_STACK_SIZE];
        USB_DeviceDriver* m_pDevice;
        volatile bool     m_Initialized;
        bool              m_ThreadStarted;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef USB_DEVICE_MANAGER_GLOBAL
    USB_DeviceManager myUSB_DeviceManager;
#else
    extern USB_DeviceManager myUSB_DeviceManager;
#endif

//-------------------------------------------------------------------------------------------------

#endif // (USE_USB_DRIVER == DEF_ENABLED)
