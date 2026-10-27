//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_device_manager.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define USB_DEVICE_MANAGER_GLOBAL
#include "lib_digini.h"
#undef USB_DEVICE_MANAGER_GLOBAL

#if (USE_USB_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
// Function:       USB_DeviceManager
// Parameter(s):   void
// Return:         N/A
//
// Description:    Initialize task state without starting the USB peripheral
//
//-------------------------------------------------------------------------------------------------
USB_DeviceManager::USB_DeviceManager(void)
{
    m_pDevice = nullptr;
    m_Initialized = false;
    m_ThreadStarted = false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       TaskUSB_Device
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    Entry point for the generic USB device service task
//
//-------------------------------------------------------------------------------------------------
extern "C" void TaskUSB_Device(void* pvParameters)
{
    (static_cast<USB_DeviceManager*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   Device - USB device stack to service for the task lifetime
//  Return:         SystemState_e - USB and task initialization result
//
//  Description:    Start one service task and initialize the attached USB device
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_DeviceManager::Initialize(USB_DeviceDriver& Device)
{
    if((m_ThreadStarted == true) && (m_pDevice != &Device))
    {
        return SYS_BUSY;
    }
    if(m_Initialized == true)
    {
        return SYS_READY;
    }

    m_pDevice = &Device;

    if(m_ThreadStarted == false)
    {
        nOS_Error Error = nOS_ThreadCreate(&m_Handle,
                                 TaskUSB_Device,
                                 this,
                                 &m_Stack[0],
                                 USB_DEVICE_MANAGER_STACK_SIZE,
                                 USB_DEVICE_MANAGER_PRIO,
                                 "USB Device");

        if(Error != NOS_OK)
        {
            return SYS_ERROR;
        }

        m_ThreadStarted = true;
    }

    if(Device.Initialize() == false)
    {
        return SYS_DEVICE_ERROR;
    }

    m_Initialized = true;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Run
//
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Service the attached device independently of its USB class
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceManager::Run(void)
{
    for(;;)
    {
        if(m_Initialized == true)
        {
            m_pDevice->Process();
        }

        nOS_Sleep(USB_DEVICE_MANAGER_POLL_TICKS);
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_USB_DRIVER == DEF_ENABLED)
