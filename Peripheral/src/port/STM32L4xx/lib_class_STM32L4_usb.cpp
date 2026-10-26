//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32L4_usb.cpp
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

#include "lib_digini.h"

#if(USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)

#include <cstring>

//-------------------------------------------------------------------------------------------------
// Define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

#define USB_BTABLE_ADDRESS 0x000U
#define USB_PMA_ACCESS 1U
#define USB_CNTRX_NBLK_MSK (0x1FUL << 10)
#define USB_CNTRX_BLSIZE (0x1UL << 15)

//-------------------------------------------------------------------------------------------------
// Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_Driver
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Initialize the controller state without accessing the peripheral registers.
//
//-------------------------------------------------------------------------------------------------
USB_Driver::USB_Driver(void)
{
    m_pUSB                          = USB;
    m_Info                          = {};
    m_Address                       = 0;
    m_Locked                        = false;
    m_State                         = USB_STATE_RESET;
    m_ErrorCode                     = 0;
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
    m_LPM_State                     = USB_LPM_L0;
    m_BESL                          = 0;
#endif
    m_FrameNumber                   = 0;
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
    m_LPM_Active                    = 0;
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
    m_BCD_Active                    = 0;
#endif
    m_pSOFCallback                  = nullptr;
    m_pSetupStageCallback           = nullptr;
    m_pResetCallback                = nullptr;
    m_pSuspendCallback              = nullptr;
    m_pResumeCallback               = nullptr;
    m_pConnectCallback              = nullptr;
    m_pDisconnectCallback           = nullptr;
    m_pHardwareInitializeCallback   = nullptr;
    m_pHardwareDeInitializeCallback = nullptr;
    m_pDataOutStageCallback         = nullptr;
    m_pDataInStageCallback          = nullptr;
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
    m_pISOOUTIncompleteCallback     = nullptr;
    m_pISOINIncompleteCallback      = nullptr;
#endif
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
    m_pLPMCallback                  = nullptr;
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
    m_pBCDCallback                  = nullptr;
#endif

    for(uint8_t Index = 0; Index < USB_ENDPOINT_COUNT; Index++)
    {
        m_InEndpoint[Index]  = {};
        m_OutEndpoint[Index] = {};
    }
    for(uint8_t Index = 0; Index < 12U; Index++)
    {
        m_Setup[Index] = 0;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   pInfo
//  Return:         SystemState_e
//
//  Description:    Initialize the USB FS device controller from its configuration.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Initialize(const USB_Info_t* pInfo)
{
    if((pInfo == nullptr) || (pInfo->EndpointCount == 0U) || (pInfo->EndpointCount > 8U))
    {
        return SYS_WRONG_VALUE;
    }

#if (DIGINI_USE_USB_LPM != DEF_ENABLED)
    if(pInfo->LPM_Enable == true)
    {
        return SYS_WRONG_VALUE;
    }
#endif
    m_Info = *pInfo;

    uint8_t Index;

    if(m_State == USB_STATE_RESET)
    {
        m_Locked = false;

        m_pSOFCallback              = nullptr;
        m_pSetupStageCallback       = nullptr;
        m_pResetCallback            = nullptr;
        m_pSuspendCallback          = nullptr;
        m_pResumeCallback           = nullptr;
        m_pConnectCallback          = nullptr;
        m_pDisconnectCallback       = nullptr;
        m_pDataOutStageCallback     = nullptr;
        m_pDataInStageCallback      = nullptr;
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
        m_pISOOUTIncompleteCallback = nullptr;
        m_pISOINIncompleteCallback  = nullptr;
#endif
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        m_pLPMCallback              = nullptr;
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
        m_pBCDCallback              = nullptr;
#endif

        HardwareInitialize();
    }

    m_State = USB_STATE_BUSY;

    (void)DisableInterrupts();
    std::memset(m_InEndpoint, 0, sizeof(m_InEndpoint));
    std::memset(m_OutEndpoint, 0, sizeof(m_OutEndpoint));
    m_ErrorCode  = 0;
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
    m_LPM_State  = USB_LPM_L0;
    m_LPM_Active = 0;
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
    m_BCD_Active = 0;
#endif

    for(Index = 0U; Index < m_Info.EndpointCount; Index++)
    {
        m_InEndpoint[Index].IsIn   = 1U;
        m_InEndpoint[Index].Number = Index;

        m_InEndpoint[Index].Type            = USB_ENDPOINT_CONTROL;
        m_InEndpoint[Index].MaxPacketSize   = 0U;
        m_InEndpoint[Index].pTransferBuffer = 0U;
        m_InEndpoint[Index].TransferLength  = 0U;
    }

    for(Index = 0U; Index < m_Info.EndpointCount; Index++)
    {
        m_OutEndpoint[Index].IsIn   = 0U;
        m_OutEndpoint[Index].Number = Index;

        m_OutEndpoint[Index].Type            = USB_ENDPOINT_CONTROL;
        m_OutEndpoint[Index].MaxPacketSize   = 0U;
        m_OutEndpoint[Index].pTransferBuffer = 0U;
        m_OutEndpoint[Index].TransferLength  = 0U;
    }

    if(InitializeDevice() != SYS_READY)
    {
        m_State = USB_STATE_ERROR;
        return SYS_ERROR;
    }

    m_Address = 0U;
    m_State   = USB_STATE_READY;

#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
    if(m_Info.LPM_Enable == 1U)
    {
        (void)ActivateLPM();
    }
    else
    {
        (void)DeActivateLPM();
    }
#else
    m_pUSB->LPMCSR &= uint16_t(~(USB_LPMCSR_LMPEN | USB_LPMCSR_LPMACK));
#endif

    (void)DisconnectDevice();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DeInitialize
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Stop the USB device and release its hardware resources.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::DeInitialize(void)
{
    m_State = USB_STATE_BUSY;

    if(StopDevice() != SYS_READY)
    {
        return SYS_ERROR;
    }

    HardwareDeInitialize();

    m_State = USB_STATE_RESET;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HardwareInitialize
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Hardware Initialize.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::HardwareInitialize(void)
{
    Notify(m_pHardwareInitializeCallback, USB_CALLBACK_HARDWARE_INITIALIZE, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HardwareDeInitialize
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Hardware De Initialize.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::HardwareDeInitialize(void)
{
    Notify(m_pHardwareDeInitializeCallback, USB_CALLBACK_HARDWARE_DEINITIALIZE, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       RegisterCallback
//
//  Parameter(s):   CallbackID, pCallback
//  Return:         SystemState_e
//
//  Description:    Register Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::RegisterCallback(USB_Callback_e CallbackID, CallbackInterface* pCallback)
{
    SystemState_e Status = SYS_READY;

    if(pCallback == nullptr)
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;
        return SYS_ERROR;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        switch(CallbackID)
        {
        case USB_CALLBACK_SOF:
            m_pSOFCallback = pCallback;
            break;

        case USB_CALLBACK_SETUP:
            m_pSetupStageCallback = pCallback;
            break;

        case USB_CALLBACK_RESET:
            m_pResetCallback = pCallback;
            break;

        case USB_CALLBACK_SUSPEND:
            m_pSuspendCallback = pCallback;
            break;

        case USB_CALLBACK_RESUME:
            m_pResumeCallback = pCallback;
            break;

        case USB_CALLBACK_CONNECT:
            m_pConnectCallback = pCallback;
            break;

        case USB_CALLBACK_DISCONNECT:
            m_pDisconnectCallback = pCallback;
            break;

        case USB_CALLBACK_HARDWARE_INITIALIZE:
            m_pHardwareInitializeCallback = pCallback;
            break;

        case USB_CALLBACK_HARDWARE_DEINITIALIZE:
            m_pHardwareDeInitializeCallback = pCallback;
            break;

        default:

            m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

            Status = SYS_ERROR;
            break;
        }
    }
    else if(m_State == USB_STATE_RESET)
    {
        switch(CallbackID)
        {
        case USB_CALLBACK_HARDWARE_INITIALIZE:
            m_pHardwareInitializeCallback = pCallback;
            break;

        case USB_CALLBACK_HARDWARE_DEINITIALIZE:
            m_pHardwareDeInitializeCallback = pCallback;
            break;

        default:

            m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

            Status = SYS_ERROR;
            break;
        }
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();
    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       UnRegisterCallback
//
//  Parameter(s):   CallbackID
//  Return:         SystemState_e
//
//  Description:    Un Register Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::UnRegisterCallback(USB_Callback_e CallbackID)
{
    SystemState_e Status = SYS_READY;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        switch(CallbackID)
        {
        case USB_CALLBACK_SOF:
            m_pSOFCallback = nullptr;
            break;

        case USB_CALLBACK_SETUP:
            m_pSetupStageCallback = nullptr;
            break;

        case USB_CALLBACK_RESET:
            m_pResetCallback = nullptr;
            break;

        case USB_CALLBACK_SUSPEND:
            m_pSuspendCallback = nullptr;
            break;

        case USB_CALLBACK_RESUME:
            m_pResumeCallback = nullptr;
            break;

        case USB_CALLBACK_CONNECT:
            m_pConnectCallback = nullptr;
            break;

        case USB_CALLBACK_DISCONNECT:
            m_pDisconnectCallback = nullptr;
            break;

        case USB_CALLBACK_HARDWARE_INITIALIZE:
            m_pHardwareInitializeCallback = nullptr;
            break;

        case USB_CALLBACK_HARDWARE_DEINITIALIZE:
            m_pHardwareDeInitializeCallback = nullptr;
            break;

        default:

            m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

            Status = SYS_ERROR;
            break;
        }
    }
    else if(m_State == USB_STATE_RESET)
    {
        switch(CallbackID)
        {
        case USB_CALLBACK_HARDWARE_INITIALIZE:
            m_pHardwareInitializeCallback = nullptr;
            break;

        case USB_CALLBACK_HARDWARE_DEINITIALIZE:
            m_pHardwareDeInitializeCallback = nullptr;
            break;

        default:

            m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

            Status = SYS_ERROR;
            break;
        }
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();
    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       RegisterDataOutStageCallback
//
//  Parameter(s):   pCallback
//  Return:         SystemState_e
//
//  Description:    Register Data Out Stage Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::RegisterDataOutStageCallback(CallbackInterface* pCallback)
{
    SystemState_e Status = SYS_READY;

    if(pCallback == nullptr)
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        return SYS_ERROR;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pDataOutStageCallback = pCallback;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       UnRegisterDataOutStageCallback
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Un Register Data Out Stage Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::UnRegisterDataOutStageCallback(void)
{
    SystemState_e Status = SYS_READY;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pDataOutStageCallback = nullptr;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       RegisterDataInStageCallback
//
//  Parameter(s):   pCallback
//  Return:         SystemState_e
//
//  Description:    Register Data In Stage Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::RegisterDataInStageCallback(CallbackInterface* pCallback)
{
    SystemState_e Status = SYS_READY;

    if(pCallback == nullptr)
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        return SYS_ERROR;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pDataInStageCallback = pCallback;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       UnRegisterDataInStageCallback
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Un Register Data In Stage Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::UnRegisterDataInStageCallback(void)
{
    SystemState_e Status = SYS_READY;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pDataInStageCallback = nullptr;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       RegisterIsoOutIncpltCallback
//
//  Parameter(s):   pCallback
//  Return:         SystemState_e
//
//  Description:    Register Iso Out Incplt Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::RegisterIsoOutIncpltCallback(CallbackInterface* pCallback)
{
    SystemState_e Status = SYS_READY;

    if(pCallback == nullptr)
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        return SYS_ERROR;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pISOOUTIncompleteCallback = pCallback;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       UnRegisterIsoOutIncpltCallback
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Un Register Iso Out Incplt Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::UnRegisterIsoOutIncpltCallback(void)
{
    SystemState_e Status = SYS_READY;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pISOOUTIncompleteCallback = nullptr;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       RegisterIsoInIncpltCallback
//
//  Parameter(s):   pCallback
//  Return:         SystemState_e
//
//  Description:    Register Iso In Incplt Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::RegisterIsoInIncpltCallback(CallbackInterface* pCallback)
{
    SystemState_e Status = SYS_READY;

    if(pCallback == nullptr)
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        return SYS_ERROR;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pISOINIncompleteCallback = pCallback;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       UnRegisterIsoInIncpltCallback
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Un Register Iso In Incplt Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::UnRegisterIsoInIncpltCallback(void)
{
    SystemState_e Status = SYS_READY;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pISOINIncompleteCallback = nullptr;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       RegisterBcdCallback
//
//  Parameter(s):   pCallback
//  Return:         SystemState_e
//
//  Description:    Register Bcd Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::RegisterBcdCallback(CallbackInterface* pCallback)
{
    SystemState_e Status = SYS_READY;

    if(pCallback == nullptr)
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        return SYS_ERROR;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pBCDCallback = pCallback;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       UnRegisterBcdCallback
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Un Register Bcd Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::UnRegisterBcdCallback(void)
{
    SystemState_e Status = SYS_READY;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pBCDCallback = nullptr;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       RegisterLpmCallback
//
//  Parameter(s):   pCallback
//  Return:         SystemState_e
//
//  Description:    Register Lpm Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::RegisterLpmCallback(CallbackInterface* pCallback)
{
    SystemState_e Status = SYS_READY;

    if(pCallback == nullptr)
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        return SYS_ERROR;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pLPMCallback = pCallback;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       UnRegisterLpmCallback
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Un Register Lpm Callback.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::UnRegisterLpmCallback(void)
{
    SystemState_e Status = SYS_READY;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    if(m_State == USB_STATE_READY)
    {
        m_pLPMCallback = nullptr;
    }
    else
    {
        m_ErrorCode |= USB_ERROR_INVALID_CALLBACK;

        Status = SYS_ERROR;
    }

    Unlock();

    return Status;
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Function:       Start
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Start.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Start(void)
{
    if(Lock() == false)
    {
        return SYS_BUSY;
    }
    (void)EnableInterrupts();
    (void)ConnectDevice();
    Unlock();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Stop
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Stop.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Stop(void)
{
    if(Lock() == false)
    {
        return SYS_BUSY;
    }
    (void)DisableInterrupts();
    (void)DisconnectDevice();

    Unlock();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IRQHandler
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Dispatch USB FS interrupts and completed endpoint transfers.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::IRQHandler(void)
{
    uint32_t Interrupt = ReadInterrupts();

    if((Interrupt & USB_ISTR_CTR) == USB_ISTR_CTR)
    {
        (void)EndpointIRQHandler();

        return;
    }

    if((Interrupt & USB_ISTR_RESET) == USB_ISTR_RESET)
    {
        m_pUSB->ISTR = uint16_t(~(USB_ISTR_RESET | USB_ISTR_SUSP | USB_ISTR_WKUP));
        m_pUSB->CNTR &= uint16_t(~(USB_CNTR_FSUSP | USB_CNTR_LPMODE));
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        m_LPM_State = USB_LPM_L0;
        m_BESL = 0U;
#endif

        ResetCallback();

        (void)SetAddress(0U);

        return;
    }

    if((Interrupt & USB_ISTR_PMAOVR) == USB_ISTR_PMAOVR)
    {
        m_pUSB->ISTR = uint16_t(~(USB_ISTR_PMAOVR));

        return;
    }

    if((Interrupt & USB_ISTR_ERR) == USB_ISTR_ERR)
    {
        m_pUSB->ISTR = uint16_t(~(USB_ISTR_ERR));

        return;
    }

    if((Interrupt & USB_ISTR_WKUP) == USB_ISTR_WKUP)
    {
        m_pUSB->CNTR &= (uint16_t) ~(USB_CNTR_LPMODE);
        m_pUSB->CNTR &= (uint16_t) ~(USB_CNTR_FSUSP);

#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        if(m_LPM_State == USB_LPM_L1)
        {
            m_LPM_State = USB_LPM_L0;
            LPM_Callback(USB_LPM_L0_ACTIVE);
        }
#endif

        ResumeCallback();

        m_pUSB->ISTR = uint16_t(~(USB_ISTR_WKUP));

        return;
    }

    if((Interrupt & USB_ISTR_SUSP) == USB_ISTR_SUSP)
    {
        m_pUSB->CNTR |= (uint16_t)USB_CNTR_FSUSP;

        m_pUSB->ISTR = uint16_t(~(USB_ISTR_SUSP));

        m_pUSB->CNTR |= (uint16_t)USB_CNTR_LPMODE;

        SuspendCallback();

        return;
    }

#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
    if((Interrupt & USB_ISTR_L1REQ) == USB_ISTR_L1REQ)
    {
        m_pUSB->ISTR = uint16_t(~(USB_ISTR_L1REQ));
        if(m_LPM_State == USB_LPM_L0)
        {
            m_pUSB->CNTR |= (uint16_t)USB_CNTR_LPMODE;
            m_pUSB->CNTR |= (uint16_t)USB_CNTR_FSUSP;

            m_LPM_State = USB_LPM_L1;
            m_BESL      = ((uint32_t)m_pUSB->LPMCSR & USB_LPMCSR_BESL) >> 4;
            LPM_Callback(USB_LPM_L1_ACTIVE);
        }
        else
        {
            SuspendCallback();
        }

        return;
    }
#endif

    if((Interrupt & USB_ISTR_SOF) == USB_ISTR_SOF)
    {
        m_pUSB->ISTR = uint16_t(~(USB_ISTR_SOF));

        SOFCallback();

        return;
    }

    if((Interrupt & USB_ISTR_ESOF) == USB_ISTR_ESOF)
    {
        m_pUSB->ISTR = uint16_t(~(USB_ISTR_ESOF));

        return;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DataOutStageCallback
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Data Out Stage Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::DataOutStageCallback(uint8_t Endpoint)
{
    Notify(m_pDataOutStageCallback, USB_CALLBACK_DATA_OUT, Endpoint);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DataInStageCallback
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Data In Stage Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::DataInStageCallback(uint8_t Endpoint)
{
    Notify(m_pDataInStageCallback, USB_CALLBACK_DATA_IN, Endpoint);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetupStageCallback
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Setup Stage Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetupStageCallback(void)
{
    Notify(m_pSetupStageCallback, USB_CALLBACK_SETUP, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SOFCallback
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    SOF Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SOFCallback(void)
{
    Notify(m_pSOFCallback, USB_CALLBACK_SOF, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResetCallback
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Reset Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ResetCallback(void)
{
    Notify(m_pResetCallback, USB_CALLBACK_RESET, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SuspendCallback
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Suspend Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SuspendCallback(void)
{
    Notify(m_pSuspendCallback, USB_CALLBACK_SUSPEND, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResumeCallback
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Resume Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ResumeCallback(void)
{
    Notify(m_pResumeCallback, USB_CALLBACK_RESUME, 0U);
}

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       ISOOUTIncompleteCallback
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    ISOOUT Incomplete Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ISOOUTIncompleteCallback(uint8_t Endpoint)
{
    Notify(m_pISOOUTIncompleteCallback, USB_CALLBACK_ISO_OUT_INCOMPLETE, Endpoint);
}
#endif


#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       ISOINIncompleteCallback
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    ISOIN Incomplete Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ISOINIncompleteCallback(uint8_t Endpoint)
{
    Notify(m_pISOINIncompleteCallback, USB_CALLBACK_ISO_IN_INCOMPLETE, Endpoint);
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Function:       ConnectCallback
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Connect Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ConnectCallback(void)
{
    Notify(m_pConnectCallback, USB_CALLBACK_CONNECT, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisconnectCallback
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disconnect Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::DisconnectCallback(void)
{
    Notify(m_pDisconnectCallback, USB_CALLBACK_DISCONNECT, 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Connect
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Connect.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Connect(void)
{
    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    (void)ConnectDevice();
    Unlock();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Disconnect
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Disconnect.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Disconnect(void)
{
    if(Lock() == false)
    {
        return SYS_BUSY;
    }
    (void)DisconnectDevice();

    Unlock();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetAddress
//
//  Parameter(s):   Address
//  Return:         SystemState_e
//
//  Description:    Set Address.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::SetAddress(uint8_t Address)
{
    if(Address > 127U)
    {
        return SYS_WRONG_VALUE;
    }
    if(Lock() == false)
    {
        return SYS_BUSY;
    }
    m_Address = Address;
    (void)SetDeviceAddress(Address);
    Unlock();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       OpenEndpoint
//
//  Parameter(s):   EndpointAddress, MaxPacketSize, Type
//  Return:         SystemState_e
//
//  Description:    Activate an endpoint using its configured packet-memory buffers.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::OpenEndpoint(uint8_t EndpointAddress, uint16_t MaxPacketSize, uint8_t Type)
{
#if (DIGINI_USE_USB_ISOCHRONOUS != DEF_ENABLED)
    if(Type == USB_ENDPOINT_ISOCHRONOUS)
    {
        return SYS_WRONG_VALUE;
    }
#endif
    if((IsEndpointValid(EndpointAddress) == false) || (Type > USB_ENDPOINT_INTERRUPT) || (MaxPacketSize == 0U) ||
       (MaxPacketSize > ((Type == USB_ENDPOINT_ISOCHRONOUS) ? 1023U : 64U)))
    {
        return SYS_WRONG_VALUE;
    }
    if((((Type == USB_ENDPOINT_CONTROL) || (Type == USB_ENDPOINT_BULK)) && (MaxPacketSize != 8U) &&
        (MaxPacketSize != 16U) && (MaxPacketSize != 32U) && (MaxPacketSize != 64U)) ||
       (((EndpointAddress & USB_ENDPOINT_MASK) == 0U) && (Type != USB_ENDPOINT_CONTROL)))
    {
        return SYS_WRONG_VALUE;
    }

    SystemState_e   Status = SYS_READY;
    USB_Endpoint_t* pEndpoint;

    if((EndpointAddress & 0x80U) == 0x80U)
    {
        pEndpoint       = &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
        pEndpoint->IsIn = 1U;
    }
    else
    {
        pEndpoint       = &m_OutEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
        pEndpoint->IsIn = 0U;
    }

    if((IsPMA_Valid(pEndpoint, MaxPacketSize) == false)
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
       || ((pEndpoint->DoubleBuffer != 0U) && ((Type == USB_ENDPOINT_CONTROL) || (Type == USB_ENDPOINT_INTERRUPT)))
#endif
       )
    {
        return SYS_WRONG_VALUE;
    }

    pEndpoint->Number        = EndpointAddress & USB_ENDPOINT_MASK;
    pEndpoint->MaxPacketSize = (uint32_t)MaxPacketSize & 0x7FFU;
    pEndpoint->Type          = Type;

    if(Type == USB_ENDPOINT_BULK)
    {
        pEndpoint->DataPID = 0U;
    }

    if(Lock() == false)
    {
        return SYS_BUSY;
    }
    Status = ActivateEndpoint(pEndpoint);
    Unlock();

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CloseEndpoint
//
//  Parameter(s):   EndpointAddress
//  Return:         SystemState_e
//
//  Description:    Close Endpoint.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::CloseEndpoint(uint8_t EndpointAddress)
{
    if(IsEndpointValid(EndpointAddress) == false)
    {
        return SYS_WRONG_VALUE;
    }

    USB_Endpoint_t* pEndpoint;

    if((EndpointAddress & 0x80U) == 0x80U)
    {
        pEndpoint       = &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
        pEndpoint->IsIn = 1U;
    }
    else
    {
        pEndpoint       = &m_OutEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
        pEndpoint->IsIn = 0U;
    }
    pEndpoint->Number = EndpointAddress & USB_ENDPOINT_MASK;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }
    (void)DeactivateEndpoint(pEndpoint);
    Unlock();
    pEndpoint->MaxPacketSize = 0U;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Receive
//
//  Parameter(s):   EndpointAddress, pBuffer, Length
//  Return:         SystemState_e
//
//  Description:    Arm an OUT transfer; the caller retains the buffer until completion.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Receive(uint8_t EndpointAddress, uint8_t* pBuffer, uint32_t Length)
{
    if((Length != 0U) && (pBuffer == nullptr))
    {
        return SYS_WRONG_VALUE;
    }

    if(IsEndpointValid(EndpointAddress) == false)
    {
        return SYS_WRONG_VALUE;
    }

    USB_Endpoint_t* pEndpoint;

    pEndpoint = &m_OutEndpoint[EndpointAddress & USB_ENDPOINT_MASK];

    if(pEndpoint->MaxPacketSize == 0U)
    {
        return SYS_DEVICE_ERROR;
    }

    pEndpoint->pTransferBuffer = pBuffer;
    pEndpoint->TransferLength  = Length;
    pEndpoint->TransferCount   = 0U;
    pEndpoint->IsIn            = 0U;
    pEndpoint->Number          = EndpointAddress & USB_ENDPOINT_MASK;

    (void)StartTransfer(pEndpoint);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetReceiveCount
//
//  Parameter(s):   EndpointAddress, pCount
//  Return:         SystemState_e
//
//  Description:    Get Receive Count.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::GetReceiveCount(uint8_t EndpointAddress, uint32_t* pCount) const
{
    if((IsEndpointValid(EndpointAddress) == false) || (pCount == nullptr))
    {
        return SYS_WRONG_VALUE;
    }
    *pCount = m_OutEndpoint[EndpointAddress & USB_ENDPOINT_MASK].TransferCount;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Transmit
//
//  Parameter(s):   EndpointAddress, pBuffer, Length
//  Return:         SystemState_e
//
//  Description:    Start an IN transfer; the caller retains the buffer until completion.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Transmit(uint8_t EndpointAddress, uint8_t* pBuffer, uint32_t Length)
{
    if((Length != 0U) && (pBuffer == nullptr))
    {
        return SYS_WRONG_VALUE;
    }

    if(IsEndpointValid(EndpointAddress) == false)
    {
        return SYS_WRONG_VALUE;
    }

    USB_Endpoint_t* pEndpoint;

    pEndpoint = &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK];

    if(pEndpoint->MaxPacketSize == 0U)
    {
        return SYS_DEVICE_ERROR;
    }

    pEndpoint->pTransferBuffer    = pBuffer;
    pEndpoint->TransferLength     = Length;
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    pEndpoint->DoubleBufferFill   = 1U;
    pEndpoint->DoubleBufferLength = Length;
#endif
    pEndpoint->TransferCount      = 0U;
    pEndpoint->IsIn               = 1U;
    pEndpoint->Number             = EndpointAddress & USB_ENDPOINT_MASK;

    (void)StartTransfer(pEndpoint);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetStall
//
//  Parameter(s):   EndpointAddress
//  Return:         SystemState_e
//
//  Description:    Set Stall.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::SetStall(uint8_t EndpointAddress)
{
    if(IsEndpointValid(EndpointAddress) == false)
    {
        return SYS_WRONG_VALUE;
    }

    USB_Endpoint_t* pEndpoint;

    if((0x80U & EndpointAddress) == 0x80U)
    {
        pEndpoint       = &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
        pEndpoint->IsIn = 1U;
    }
    else
    {
        pEndpoint       = &m_OutEndpoint[EndpointAddress];
        pEndpoint->IsIn = 0U;
    }

    pEndpoint->IsStall = 1U;
    pEndpoint->Number  = EndpointAddress & USB_ENDPOINT_MASK;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }

    (void)SetEndpointStall(pEndpoint);

    Unlock();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearStall
//
//  Parameter(s):   EndpointAddress
//  Return:         SystemState_e
//
//  Description:    Clear Stall.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ClearStall(uint8_t EndpointAddress)
{
    if(IsEndpointValid(EndpointAddress) == false)
    {
        return SYS_WRONG_VALUE;
    }

    USB_Endpoint_t* pEndpoint;

    if((0x80U & EndpointAddress) == 0x80U)
    {
        pEndpoint       = &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
        pEndpoint->IsIn = 1U;
    }
    else
    {
        pEndpoint       = &m_OutEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
        pEndpoint->IsIn = 0U;
    }

    pEndpoint->IsStall = 0U;
    pEndpoint->Number  = EndpointAddress & USB_ENDPOINT_MASK;

    if(Lock() == false)
    {
        return SYS_BUSY;
    }
    (void)ClearEndpointStall(pEndpoint);
    Unlock();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Abort
//
//  Parameter(s):   EndpointAddress
//  Return:         SystemState_e
//
//  Description:    Abort.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Abort(uint8_t EndpointAddress)
{
    if(IsEndpointValid(EndpointAddress) == false)
    {
        return SYS_WRONG_VALUE;
    }

    SystemState_e   Status;
    USB_Endpoint_t* pEndpoint;

    if((0x80U & EndpointAddress) == 0x80U)
    {
        pEndpoint = &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
    }
    else
    {
        pEndpoint = &m_OutEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
    }

    Status = StopTransfer(pEndpoint);

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Flush
//
//  Parameter(s):   EndpointAddress
//  Return:         SystemState_e
//
//  Description:    Flush.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::Flush(uint8_t EndpointAddress)
{
    if(IsEndpointValid(EndpointAddress) == false)
    {
        return SYS_WRONG_VALUE;
    }
    // USB FS has no FIFO separate from PMA; use Abort() to cancel a transfer.
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ActivateRemoteWakeup
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Activate Remote Wakeup.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ActivateRemoteWakeup(void)
{
    return (StartRemoteWakeup());
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DeActivateRemoteWakeup
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    De Activate Remote Wakeup.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::DeActivateRemoteWakeup(void)
{
    return (StopRemoteWakeup());
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetState
//
//  Parameter(s):   None
//  Return:         USB_State_e
//
//  Description:    Get State.
//
//-------------------------------------------------------------------------------------------------
USB_State_e USB_Driver::GetState(void)
{
    return m_State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EndpointIRQHandler
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Endpoint IRQ Handler.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::EndpointIRQHandler(void)
{
    USB_Endpoint_t* pEndpoint;
    uint16_t        Count;
    uint16_t        Interrupt;
    uint16_t        EndpointValue;
    uint16_t        PacketSize;
    uint8_t         EndpointIndex;

    while((m_pUSB->ISTR & USB_ISTR_CTR) != 0U)
    {
        Interrupt = m_pUSB->ISTR;

        EndpointIndex = (uint8_t)(Interrupt & USB_ISTR_EP_ID);

        if(EndpointIndex == 0U)
        {
            if((Interrupt & USB_ISTR_DIR) == 0U)
            {
                ClearEndpointTxEvent(0U);
                pEndpoint = &m_InEndpoint[0];

                pEndpoint->TransferCount = GetEndpointTxCount(pEndpoint->Number);
                if(pEndpoint->TransferCount != 0U)
                {
                    pEndpoint->pTransferBuffer += pEndpoint->TransferCount;
                }

                DataInStageCallback(0U);

                if((m_Address > 0U) && (pEndpoint->TransferLength == 0U))
                {
                    m_pUSB->DADDR = ((uint16_t)m_Address | USB_DADDR_EF);
                    m_Address     = 0U;
                }
            }
            else
            {
                pEndpoint     = &m_OutEndpoint[0];
                EndpointValue = ReadEndpointRegister(0U);

                if((EndpointValue & USB_EP_SETUP) != 0U)
                {
                    pEndpoint->TransferCount = GetEndpointRxCount(pEndpoint->Number);

                    if(pEndpoint->TransferCount != 8U)
                    {
                        SetEndpointRxStatus(0U, USB_EP_RX_STALL);
                        SetEndpointTxStatus(0U, USB_EP_TX_STALL);

                        ClearEndpointRxEvent(0U);

                        return SYS_READY;
                    }

                    ReadPMA((uint8_t*)m_Setup, pEndpoint->PMA_Address, (uint16_t)pEndpoint->TransferCount);

                    ClearEndpointRxEvent(0U);
                    ClearEndpointTxEvent(0U);
                    m_Address = 0U;
                    m_InEndpoint[0].IsStall = 0U;
                    m_OutEndpoint[0].IsStall = 0U;

                    SetupStageCallback();
                }
                else if((EndpointValue & USB_EP_CTR_RX) != 0U)
                {
                    ClearEndpointRxEvent(0U);

                    pEndpoint->TransferCount = GetEndpointRxCount(pEndpoint->Number);

                    if(pEndpoint->TransferCount == 0U)
                    {
                        SetEndpointRxStatus(0U, USB_EP_RX_VALID);
                        DataOutStageCallback(0U);
                    }
                    else
                    {
                        if(pEndpoint->pTransferBuffer != 0U)
                        {
                            ReadPMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address,
                                    (uint16_t)pEndpoint->TransferCount);

                            pEndpoint->pTransferBuffer += pEndpoint->TransferCount;

                            DataOutStageCallback(0U);
                        }
                    }
                }
            }
        }
        else
        {
            EndpointValue = ReadEndpointRegister(EndpointIndex);

            if((EndpointValue & USB_EP_CTR_RX) != 0U)
            {
                ClearEndpointRxEvent(EndpointIndex);
                pEndpoint = &m_OutEndpoint[EndpointIndex];

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
                if(pEndpoint->DoubleBuffer == 0U)
#endif
                {
                    Count = (uint16_t)GetEndpointRxCount(pEndpoint->Number);

                    if(Count != 0U)
                    {
                        ReadPMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address, Count);
                    }
                }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
                else
                {
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
                    if(pEndpoint->Type == USB_ENDPOINT_BULK)
#endif
                    {
                        Count = ReceiveDoubleBuffer(pEndpoint, EndpointValue);
                    }
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
                    else
                    {
                        ReleaseBuffer(pEndpoint->Number, 0U);

                        if((ReadEndpointRegister(pEndpoint->Number) & USB_EP_DTOG_RX) != 0U)
                        {
                            Count = (uint16_t)GetEndpointBuffer0Count(pEndpoint->Number);

                            if(Count != 0U)
                            {
                                ReadPMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address0, Count);
                            }
                        }
                        else
                        {
                            Count = (uint16_t)GetEndpointBuffer1Count(pEndpoint->Number);

                            if(Count != 0U)
                            {
                                ReadPMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address1, Count);
                            }
                        }
                    }
#endif
                }
#endif

                pEndpoint->TransferCount += Count;

                if((pEndpoint->TransferLength == 0U) || (Count < pEndpoint->MaxPacketSize))
                {
                    DataOutStageCallback(pEndpoint->Number);
                }
                else
                {
                    pEndpoint->pTransferBuffer += Count;
                    (void)StartTransfer(pEndpoint);
                }
            }

            if((EndpointValue & USB_EP_CTR_TX) != 0U)
            {
                pEndpoint = &m_InEndpoint[EndpointIndex];

                ClearEndpointTxEvent(EndpointIndex);

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
                if(pEndpoint->Type == USB_ENDPOINT_ISOCHRONOUS)
                {
                    pEndpoint->TransferLength = 0U;

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
                    if(pEndpoint->DoubleBuffer != 0U)
                    {
                        if((EndpointValue & USB_EP_DTOG_TX) != 0U)
                        {
                            SetEndpointBuffer0Count(pEndpoint->Number, pEndpoint->IsIn, 0U);
                        }
                        else
                        {
                            SetEndpointBuffer1Count(pEndpoint->Number, pEndpoint->IsIn, 0U);
                        }
                    }
#endif

                    DataInStageCallback(pEndpoint->Number);
                }
                else
#endif
                {
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
                    if((EndpointValue & USB_EP_KIND) == 0U)
#endif
                    {
                        PacketSize = (uint16_t)GetEndpointTxCount(pEndpoint->Number);

                        if(pEndpoint->TransferLength > PacketSize)
                        {
                            pEndpoint->TransferLength -= PacketSize;
                        }
                        else
                        {
                            pEndpoint->TransferLength = 0U;
                        }

                        if(pEndpoint->TransferLength == 0U)
                        {
                            DataInStageCallback(pEndpoint->Number);
                        }
                        else
                        {
                            pEndpoint->pTransferBuffer += PacketSize;
                            pEndpoint->TransferCount += PacketSize;
                            (void)StartTransfer(pEndpoint);
                        }
                    }

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
                    else
                    {
                        (void)TransmitDoubleBuffer(pEndpoint, EndpointValue);
                    }
#endif
                }
            }
        }
    }

    return SYS_READY;
}

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       ReceiveDoubleBuffer
//
//  Parameter(s):   pEndpoint, EndpointValue
//  Return:         uint16_t
//
//  Description:    Receive Double Buffer.
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_Driver::ReceiveDoubleBuffer(USB_Endpoint_t* pEndpoint, uint16_t EndpointValue)
{
    uint16_t Count;

    if((EndpointValue & USB_EP_DTOG_RX) != 0U)
    {
        Count = (uint16_t)GetEndpointBuffer0Count(pEndpoint->Number);

        if(pEndpoint->TransferLength >= Count)
        {
            pEndpoint->TransferLength -= Count;
        }
        else
        {
            pEndpoint->TransferLength = 0U;
        }

        if(pEndpoint->TransferLength == 0U)
        {
            SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_NAK);
        }

        if((EndpointValue & USB_EP_DTOG_TX) != 0U)
        {
            ReleaseBuffer(pEndpoint->Number, 0U);
        }

        if(Count != 0U)
        {
            ReadPMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address0, Count);
        }
    }

    else
    {
        Count = (uint16_t)GetEndpointBuffer1Count(pEndpoint->Number);

        if(pEndpoint->TransferLength >= Count)
        {
            pEndpoint->TransferLength -= Count;
        }
        else
        {
            pEndpoint->TransferLength = 0U;
        }

        if(pEndpoint->TransferLength == 0U)
        {
            SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_NAK);
        }

        if((EndpointValue & USB_EP_DTOG_TX) == 0U)
        {
            ReleaseBuffer(pEndpoint->Number, 0U);
        }

        if(Count != 0U)
        {
            ReadPMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address1, Count);
        }
    }

    return Count;
}
#endif


#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       TransmitDoubleBuffer
//
//  Parameter(s):   pEndpoint, EndpointValue
//  Return:         SystemState_e
//
//  Description:    Transmit Double Buffer.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::TransmitDoubleBuffer(USB_Endpoint_t* pEndpoint, uint16_t EndpointValue)
{
    uint32_t Length;
    uint16_t PacketSize;

    if((EndpointValue & USB_EP_DTOG_TX) != 0U)
    {
        PacketSize = (uint16_t)GetEndpointBuffer0Count(pEndpoint->Number);

        if(pEndpoint->TransferLength > PacketSize)
        {
            pEndpoint->TransferLength -= PacketSize;
        }
        else
        {
            pEndpoint->TransferLength = 0U;
        }

        if(pEndpoint->TransferLength == 0U)
        {
            SetEndpointBuffer0Count(pEndpoint->Number, pEndpoint->IsIn, 0U);
            SetEndpointBuffer1Count(pEndpoint->Number, pEndpoint->IsIn, 0U);

            if(pEndpoint->Type == USB_ENDPOINT_BULK)
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_NAK);
            }

            DataInStageCallback(pEndpoint->Number);

            if((EndpointValue & USB_EP_DTOG_RX) != 0U)
            {
                ReleaseBuffer(pEndpoint->Number, 1U);
            }

            return SYS_READY;
        }
        else
        {
            if((EndpointValue & USB_EP_DTOG_RX) != 0U)
            {
                ReleaseBuffer(pEndpoint->Number, 1U);
            }

            if(pEndpoint->DoubleBufferFill == 1U)
            {
                pEndpoint->pTransferBuffer += PacketSize;
                pEndpoint->TransferCount += PacketSize;

                if(pEndpoint->DoubleBufferLength >= pEndpoint->MaxPacketSize)
                {
                    Length = pEndpoint->MaxPacketSize;
                    pEndpoint->DoubleBufferLength -= Length;
                }
                else if(pEndpoint->DoubleBufferLength == 0U)
                {
                    Length                      = PacketSize;
                    pEndpoint->DoubleBufferFill = 0U;
                }
                else
                {
                    pEndpoint->DoubleBufferFill   = 0U;
                    Length                        = pEndpoint->DoubleBufferLength;
                    pEndpoint->DoubleBufferLength = 0U;
                }

                SetEndpointBuffer0Count(pEndpoint->Number, pEndpoint->IsIn, Length);

                WritePMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address0, (uint16_t)Length);
            }
        }
    }
    else
    {
        PacketSize = (uint16_t)GetEndpointBuffer1Count(pEndpoint->Number);

        if(pEndpoint->TransferLength >= PacketSize)
        {
            pEndpoint->TransferLength -= PacketSize;
        }
        else
        {
            pEndpoint->TransferLength = 0U;
        }

        if(pEndpoint->TransferLength == 0U)
        {
            SetEndpointBuffer0Count(pEndpoint->Number, pEndpoint->IsIn, 0U);
            SetEndpointBuffer1Count(pEndpoint->Number, pEndpoint->IsIn, 0U);

            if(pEndpoint->Type == USB_ENDPOINT_BULK)
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_NAK);
            }

            DataInStageCallback(pEndpoint->Number);

            if((EndpointValue & USB_EP_DTOG_RX) == 0U)
            {
                ReleaseBuffer(pEndpoint->Number, 1U);
            }

            return SYS_READY;
        }
        else
        {
            if((EndpointValue & USB_EP_DTOG_RX) == 0U)
            {
                ReleaseBuffer(pEndpoint->Number, 1U);
            }

            if(pEndpoint->DoubleBufferFill == 1U)
            {
                pEndpoint->pTransferBuffer += PacketSize;
                pEndpoint->TransferCount += PacketSize;

                if(pEndpoint->DoubleBufferLength >= pEndpoint->MaxPacketSize)
                {
                    Length = pEndpoint->MaxPacketSize;
                    pEndpoint->DoubleBufferLength -= Length;
                }
                else if(pEndpoint->DoubleBufferLength == 0U)
                {
                    Length                      = PacketSize;
                    pEndpoint->DoubleBufferFill = 0U;
                }
                else
                {
                    Length                        = pEndpoint->DoubleBufferLength;
                    pEndpoint->DoubleBufferLength = 0U;
                    pEndpoint->DoubleBufferFill   = 0;
                }

                SetEndpointBuffer1Count(pEndpoint->Number, pEndpoint->IsIn, Length);

                WritePMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address1, (uint16_t)Length);
            }
        }
    }

    SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_VALID);

    return SYS_READY;
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Function:       ConfigurePMA
//
//  Parameter(s):   EndpointAddress, BufferMode, PMA_Address
//  Return:         SystemState_e
//
//  Description:    Assign single or double packet-memory buffers to an endpoint.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ConfigurePMA(uint16_t EndpointAddress, uint16_t BufferMode, uint32_t PMA_Address)
{
#if (DIGINI_USE_USB_DOUBLE_BUFFER != DEF_ENABLED)
    if(BufferMode != USB_SINGLE_BUFFER)
    {
        return SYS_WRONG_VALUE;
    }
#endif
    uint16_t Address0 = uint16_t(PMA_Address);
    uint16_t Address1 = uint16_t(PMA_Address >> 16);
    if((EndpointAddress > 0xFFU) || (IsEndpointValid(uint8_t(EndpointAddress)) == false) ||
       (BufferMode > USB_DOUBLE_BUFFER) || ((Address0 & 1U) != 0U) || (Address0 < USB_ENDPOINT_COUNT * 8U) ||
       (Address0 >= USB_PMA_SIZE) || ((BufferMode == USB_SINGLE_BUFFER) && (Address1 != 0U)) ||
       ((BufferMode == USB_DOUBLE_BUFFER) && (((Address1 & 1U) != 0U) || (Address1 < USB_ENDPOINT_COUNT * 8U) ||
                                              (Address1 >= USB_PMA_SIZE) || (Address0 == Address1))))
    {
        return SYS_WRONG_VALUE;
    }

    USB_Endpoint_t* pEndpoint;

    if((0x80U & EndpointAddress) == 0x80U)
    {
        pEndpoint = &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
    }
    else
    {
        pEndpoint = &m_OutEndpoint[EndpointAddress];
    }

    if(pEndpoint->MaxPacketSize != 0U)
    {
        return SYS_BUSY;
    }

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    if(BufferMode == USB_SINGLE_BUFFER)
    {
        pEndpoint->DoubleBuffer = 0U;
#endif
        pEndpoint->PMA_Address = (uint16_t)PMA_Address;
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    }
    else
    {
        pEndpoint->DoubleBuffer = 1U;

        pEndpoint->PMA_Address0 = (uint16_t)(PMA_Address & 0xFFFFU);
        pEndpoint->PMA_Address1 = (uint16_t)((PMA_Address & 0xFFFF0000U) >> 16);
    }
#endif

    return SYS_READY;
}

#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       ActivateBCD
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Activate BCD.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ActivateBCD(void)
{
    m_BCD_Active = 1U;

    m_pUSB->BCDR &= ~(USB_BCDR_PDEN);
    m_pUSB->BCDR &= ~(USB_BCDR_SDEN);

    m_pUSB->BCDR |= USB_BCDR_BCDEN;

    return SYS_READY;
}
#endif


#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       DeActivateBCD
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    De Activate BCD.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::DeActivateBCD(void)
{
    m_BCD_Active = 0U;

    m_pUSB->BCDR &= ~(USB_BCDR_BCDEN);

    return SYS_READY;
}
#endif


#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       DetectBatteryCharging
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Detect a standard, charging or dedicated downstream port.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::DetectBatteryCharging(void)
{
    nOS_TickCounter StartTick = nOS_GetTickCount();

    LIB_Delay_mSec(350U);

    m_pUSB->BCDR |= (USB_BCDR_PDEN);
    LIB_Delay_mSec(50U);

    if((m_pUSB->BCDR & USB_BCDR_PDET) == USB_BCDR_PDET)
    {
        m_pUSB->BCDR &= ~(USB_BCDR_PDEN);
        LIB_Delay_mSec(50U);
        m_pUSB->BCDR |= (USB_BCDR_SDEN);
        LIB_Delay_mSec(50U);

        if((m_pUSB->BCDR & USB_BCDR_SDET) == USB_BCDR_SDET)
        {
            BCD_Callback(USB_BCD_DEDICATED_CHARGING_PORT);
        }
        else
        {
            BCD_Callback(USB_BCD_CHARGING_DOWNSTREAM_PORT);
        }
    }
    else
    {
        BCD_Callback(USB_BCD_STD_DOWNSTREAM_PORT);
    }

    (void)DeActivateBCD();

    if(nOS_TickCounter(nOS_GetTickCount() - StartTick) > NOS_CONFIG_TICKS_PER_SECOND)
    {
        BCD_Callback(USB_BCD_ERROR);
    }
    else
    {
        BCD_Callback(USB_BCD_DISCOVERY_COMPLETED);
    }
}
#endif


#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       ActivateLPM
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Activate LPM.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ActivateLPM(void)
{
    m_LPM_Active = 1U;
    m_LPM_State  = USB_LPM_L0;

    m_pUSB->LPMCSR |= USB_LPMCSR_LMPEN;
    m_pUSB->LPMCSR |= USB_LPMCSR_LPMACK;

    return SYS_READY;
}
#endif


#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       DeActivateLPM
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    De Activate LPM.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::DeActivateLPM(void)
{
    m_LPM_Active = 0U;

    m_pUSB->LPMCSR &= ~(USB_LPMCSR_LMPEN);
    m_pUSB->LPMCSR &= ~(USB_LPMCSR_LPMACK);

    return SYS_READY;
}
#endif


#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       LPM_Callback
//
//  Parameter(s):   Message
//  Return:         None
//
//  Description:    Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::LPM_Callback(USB_LPM_Message_e Message)
{
    Notify(m_pLPMCallback, USB_CALLBACK_LPM, Message);
}
#endif


#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       BCD_Callback
//
//  Parameter(s):   Message
//  Return:         None
//
//  Description:    Callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::BCD_Callback(USB_BCD_Message_e Message)
{
    Notify(m_pBCDCallback, USB_CALLBACK_BCD, Message);
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableInterrupts
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device Enable Global Int.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::EnableInterrupts(void)
{
    uint32_t InterruptMask;

    m_pUSB->ISTR = 0U;

    InterruptMask = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM | USB_CNTR_SOFM | USB_CNTR_ESOFM |
                    USB_CNTR_RESETM | USB_CNTR_L1REQM;
    if(m_Info.SOF_Enable == false)
    {
        InterruptMask &= ~USB_CNTR_SOFM;
    }
#if (DIGINI_USE_USB_LPM != DEF_ENABLED)
    InterruptMask &= ~USB_CNTR_L1REQM;
#endif

    m_pUSB->CNTR = (uint16_t)InterruptMask;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisableInterrupts
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device Disable Global Int.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::DisableInterrupts(void)
{
    uint32_t InterruptMask;

    InterruptMask = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM | USB_CNTR_SOFM | USB_CNTR_ESOFM |
                    USB_CNTR_RESETM | USB_CNTR_L1REQM;

    m_pUSB->CNTR &= (uint16_t)(~InterruptMask);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       InitializeDevice
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device Dev Init.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::InitializeDevice(void)
{
    m_pUSB->CNTR = (uint16_t)USB_CNTR_FRES;

    m_pUSB->CNTR = 0U;

    m_pUSB->ISTR = 0U;

    m_pUSB->BTABLE = USB_BTABLE_ADDRESS;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ActivateEndpoint
//
//  Parameter(s):   pEndpoint
//  Return:         SystemState_e
//
//  Description:    Device Activate Endpoint.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ActivateEndpoint(USB_Endpoint_t* pEndpoint)
{
    SystemState_e Status = SYS_READY;
    uint16_t      EndpointValue;

    EndpointValue = ReadEndpointRegister(pEndpoint->Number) & USB_EP_T_MASK;

    switch(pEndpoint->Type)
    {
    case USB_ENDPOINT_CONTROL:
        EndpointValue |= USB_EP_CONTROL;
        break;

    case USB_ENDPOINT_BULK:
        EndpointValue |= USB_EP_BULK;
        break;

    case USB_ENDPOINT_INTERRUPT:
        EndpointValue |= USB_EP_INTERRUPT;
        break;

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
    case USB_ENDPOINT_ISOCHRONOUS:
        EndpointValue |= USB_EP_ISOCHRONOUS;
        break;
#endif

    default:
        Status = SYS_ERROR;
        break;
    }

    WriteEndpointRegister(pEndpoint->Number, (EndpointValue | USB_EP_CTR_RX | USB_EP_CTR_TX));

    SetEndpointAddress(pEndpoint->Number, pEndpoint->Number);

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    if(pEndpoint->DoubleBuffer == 0U)
#endif
    {
        if(pEndpoint->IsIn != 0U)
        {
            SetEndpointTxAddress(pEndpoint->Number, pEndpoint->PMA_Address);
            ClearEndpointTxToggle(pEndpoint->Number);

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            if(pEndpoint->Type != USB_ENDPOINT_ISOCHRONOUS)
#endif
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_NAK);
            }
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            else
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_DIS);
            }
#endif
        }
        else
        {
            SetEndpointRxAddress(pEndpoint->Number, pEndpoint->PMA_Address);

            SetEndpointRxCount(pEndpoint->Number, pEndpoint->MaxPacketSize);
            ClearEndpointRxToggle(pEndpoint->Number);

            if(pEndpoint->Number == 0U)
            {
                SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_VALID);
            }
            else
            {
                SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_NAK);
            }
        }
    }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    else
    {
        if(pEndpoint->Type == USB_ENDPOINT_BULK)
        {
            SetBulkDoubleBuffer(pEndpoint->Number);
        }
        else
        {
            ClearEndpointKind(pEndpoint->Number);
        }

        SetEndpointBufferAddress(pEndpoint->Number, pEndpoint->PMA_Address0, pEndpoint->PMA_Address1);

        if(pEndpoint->IsIn == 0U)
        {
            ClearEndpointRxToggle(pEndpoint->Number);
            ClearEndpointTxToggle(pEndpoint->Number);

            SetEndpointBufferCount(pEndpoint->Number, pEndpoint->IsIn, pEndpoint->MaxPacketSize);

            SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_VALID);
            SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_DIS);
        }
        else
        {
            ClearEndpointRxToggle(pEndpoint->Number);
            ClearEndpointTxToggle(pEndpoint->Number);

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            if(pEndpoint->Type != USB_ENDPOINT_ISOCHRONOUS)
#endif
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_NAK);
            }
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            else
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_DIS);
            }
#endif

            SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_DIS);
        }
    }
#endif

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DeactivateEndpoint
//
//  Parameter(s):   pEndpoint
//  Return:         SystemState_e
//
//  Description:    Device Deactivate Endpoint.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::DeactivateEndpoint(USB_Endpoint_t* pEndpoint)
{
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    if(pEndpoint->DoubleBuffer == 0U)
#endif
    {
        if(pEndpoint->IsIn != 0U)
        {
            ClearEndpointTxToggle(pEndpoint->Number);

            SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_DIS);
        }

        else
        {
            ClearEndpointRxToggle(pEndpoint->Number);

            SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_DIS);
        }
    }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    else
    {
        if(pEndpoint->IsIn == 0U)
        {
            ClearEndpointRxToggle(pEndpoint->Number);
            ClearEndpointTxToggle(pEndpoint->Number);

            ToggleEndpointTx(pEndpoint->Number);

            SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_DIS);
            SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_DIS);
        }
        else
        {
            ClearEndpointRxToggle(pEndpoint->Number);
            ClearEndpointTxToggle(pEndpoint->Number);
            ToggleEndpointRx(pEndpoint->Number);

            SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_DIS);
            SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_DIS);
        }
    }
#endif

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    if((pEndpoint->DoubleBuffer != 0U) || (pEndpoint->IsIn != 0U))
#else
    if(pEndpoint->IsIn != 0U)
#endif
    {
        ClearEndpointTxEvent(pEndpoint->Number);
    }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    if((pEndpoint->DoubleBuffer != 0U) || (pEndpoint->IsIn == 0U))
#else
    if(pEndpoint->IsIn == 0U)
#endif
    {
        ClearEndpointRxEvent(pEndpoint->Number);
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       StartTransfer
//
//  Parameter(s):   pEndpoint
//  Return:         SystemState_e
//
//  Description:    Device EP Start Xfer.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::StartTransfer(USB_Endpoint_t* pEndpoint)
{
    uint32_t Length;
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    uint16_t PMA_Address;
    uint16_t EndpointValue;
#endif

    if(pEndpoint->IsIn == 1U)
    {
        if(pEndpoint->TransferLength > pEndpoint->MaxPacketSize)
        {
            Length = pEndpoint->MaxPacketSize;
        }
        else
        {
            Length = pEndpoint->TransferLength;
        }

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        if(pEndpoint->DoubleBuffer == 0U)
#endif
        {
            WritePMA(pEndpoint->pTransferBuffer, pEndpoint->PMA_Address, (uint16_t)Length);
            SetEndpointTxCount(pEndpoint->Number, Length);
        }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        else
        {
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            if(pEndpoint->Type == USB_ENDPOINT_BULK)
#endif
            {
                if(pEndpoint->DoubleBufferLength > pEndpoint->MaxPacketSize)
                {
                    SetBulkDoubleBuffer(pEndpoint->Number);

                    pEndpoint->DoubleBufferLength -= Length;

                    if((ReadEndpointRegister(pEndpoint->Number) & USB_EP_DTOG_TX) != 0U)
                    {
                        SetEndpointBuffer1Count(pEndpoint->Number, pEndpoint->IsIn, Length);
                        PMA_Address = pEndpoint->PMA_Address1;

                        WritePMA(pEndpoint->pTransferBuffer, PMA_Address, (uint16_t)Length);
                        pEndpoint->pTransferBuffer += Length;

                        if(pEndpoint->DoubleBufferLength > pEndpoint->MaxPacketSize)
                        {
                            pEndpoint->DoubleBufferLength -= Length;
                        }
                        else
                        {
                            Length                        = pEndpoint->DoubleBufferLength;
                            pEndpoint->DoubleBufferLength = 0U;
                        }

                        SetEndpointBuffer0Count(pEndpoint->Number, pEndpoint->IsIn, Length);
                        PMA_Address = pEndpoint->PMA_Address0;

                        WritePMA(pEndpoint->pTransferBuffer, PMA_Address, (uint16_t)Length);
                    }
                    else
                    {
                        SetEndpointBuffer0Count(pEndpoint->Number, pEndpoint->IsIn, Length);
                        PMA_Address = pEndpoint->PMA_Address0;

                        WritePMA(pEndpoint->pTransferBuffer, PMA_Address, (uint16_t)Length);
                        pEndpoint->pTransferBuffer += Length;

                        if(pEndpoint->DoubleBufferLength > pEndpoint->MaxPacketSize)
                        {
                            pEndpoint->DoubleBufferLength -= Length;
                        }
                        else
                        {
                            Length                        = pEndpoint->DoubleBufferLength;
                            pEndpoint->DoubleBufferLength = 0U;
                        }

                        SetEndpointBuffer1Count(pEndpoint->Number, pEndpoint->IsIn, Length);
                        PMA_Address = pEndpoint->PMA_Address1;

                        WritePMA(pEndpoint->pTransferBuffer, PMA_Address, (uint16_t)Length);
                    }
                }

                else
                {
                    Length = pEndpoint->DoubleBufferLength;

                    ClearBulkDoubleBuffer(pEndpoint->Number);

                    SetEndpointTxCount(pEndpoint->Number, Length);
                    PMA_Address = pEndpoint->PMA_Address0;

                    WritePMA(pEndpoint->pTransferBuffer, PMA_Address, (uint16_t)Length);
                }
            }
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            else
            {
                pEndpoint->DoubleBufferLength -= Length;

                if((ReadEndpointRegister(pEndpoint->Number) & USB_EP_DTOG_TX) != 0U)
                {
                    SetEndpointBuffer1Count(pEndpoint->Number, pEndpoint->IsIn, Length);
                    PMA_Address = pEndpoint->PMA_Address1;

                    WritePMA(pEndpoint->pTransferBuffer, PMA_Address, (uint16_t)Length);
                }
                else
                {
                    SetEndpointBuffer0Count(pEndpoint->Number, pEndpoint->IsIn, Length);
                    PMA_Address = pEndpoint->PMA_Address0;

                    WritePMA(pEndpoint->pTransferBuffer, PMA_Address, (uint16_t)Length);
                }
            }
#endif
        }
#endif

        SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_VALID);
    }
    else
    {
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        if(pEndpoint->DoubleBuffer == 0U)
#endif
        {
            if((pEndpoint->TransferLength == 0U) && (pEndpoint->Type == USB_ENDPOINT_CONTROL))
            {
                SetOutStatus(pEndpoint->Number);
            }
            else
            {
                ClearOutStatus(pEndpoint->Number);
            }

            if(pEndpoint->TransferLength > pEndpoint->MaxPacketSize)
            {
                pEndpoint->TransferLength -= pEndpoint->MaxPacketSize;
            }
            else
            {
                pEndpoint->TransferLength = 0U;
            }
        }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        else
        {
            if(pEndpoint->Type == USB_ENDPOINT_BULK)
            {
                if(pEndpoint->TransferCount != 0U)
                {
                    EndpointValue = ReadEndpointRegister(pEndpoint->Number);

                    if((((EndpointValue & USB_EP_DTOG_RX) != 0U) && ((EndpointValue & USB_EP_DTOG_TX) != 0U)) ||
                       (((EndpointValue & USB_EP_DTOG_RX) == 0U) && ((EndpointValue & USB_EP_DTOG_TX) == 0U)))
                    {
                        ReleaseBuffer(pEndpoint->Number, 0U);
                    }
                }
            }

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            else if(pEndpoint->Type == USB_ENDPOINT_ISOCHRONOUS)
            {
                pEndpoint->TransferLength = 0U;
            }
#endif
            else
            {
                return SYS_ERROR;
            }
        }
#endif

        SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_VALID);
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointStall
//
//  Parameter(s):   pEndpoint
//  Return:         SystemState_e
//
//  Description:    Device EP Set Stall.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::SetEndpointStall(USB_Endpoint_t* pEndpoint)
{
    if(pEndpoint->IsIn != 0U)
    {
        SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_STALL);
    }
    else
    {
        SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_STALL);
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearEndpointStall
//
//  Parameter(s):   pEndpoint
//  Return:         SystemState_e
//
//  Description:    Device EP Clear Stall.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ClearEndpointStall(USB_Endpoint_t* pEndpoint)
{
    if(pEndpoint->IsIn != 0U)
    {
        ClearEndpointTxToggle(pEndpoint->Number);

#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
        if(pEndpoint->Type != USB_ENDPOINT_ISOCHRONOUS)
#endif
        {
            SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_NAK);
        }
    }
    else
    {
        ClearEndpointRxToggle(pEndpoint->Number);

        SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_VALID);
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       StopTransfer
//
//  Parameter(s):   pEndpoint
//  Return:         SystemState_e
//
//  Description:    Device EP Stop Xfer.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::StopTransfer(USB_Endpoint_t* pEndpoint)
{
    if(pEndpoint->IsIn == 1U)
    {
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        if(pEndpoint->DoubleBuffer == 0U)
#endif
        {
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            if(pEndpoint->Type != USB_ENDPOINT_ISOCHRONOUS)
#endif
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_NAK);
            }
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            else
            {
                SetEndpointTxStatus(pEndpoint->Number, USB_EP_TX_DIS);
            }
#endif
        }
    }
    else
    {
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        if(pEndpoint->DoubleBuffer == 0U)
#endif
        {
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            if(pEndpoint->Type != USB_ENDPOINT_ISOCHRONOUS)
#endif
            {
                SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_NAK);
            }
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
            else
            {
                SetEndpointRxStatus(pEndpoint->Number, USB_EP_RX_DIS);
            }
#endif
        }
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       StopDevice
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device Stop Device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::StopDevice(void)
{
    m_pUSB->CNTR = (uint16_t)USB_CNTR_FRES;

    m_pUSB->ISTR = 0U;

    m_pUSB->CNTR = (uint16_t)(USB_CNTR_FRES | USB_CNTR_PDWN);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetDeviceAddress
//
//  Parameter(s):   Address
//  Return:         SystemState_e
//
//  Description:    Device Set Dev Address.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::SetDeviceAddress(uint8_t Address)
{
    if(Address == 0U)
    {
        m_pUSB->DADDR = (uint16_t)USB_DADDR_EF;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ConnectDevice
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device Dev Connect.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::ConnectDevice(void)
{
    m_pUSB->BCDR |= (uint16_t)USB_BCDR_DPPU;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisconnectDevice
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device Dev Disconnect.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::DisconnectDevice(void)
{
    m_pUSB->BCDR &= (uint16_t)(~(USB_BCDR_DPPU));

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ReadInterrupts
//
//  Parameter(s):   None
//  Return:         uint32_t
//
//  Description:    Device Read Interrupts.
//
//-------------------------------------------------------------------------------------------------
uint32_t USB_Driver::ReadInterrupts(void)
{
    uint32_t Value;

    Value = m_pUSB->ISTR;
    return Value;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       StartRemoteWakeup
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device Activate Remote Wakeup.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::StartRemoteWakeup(void)
{
    m_pUSB->CNTR |= (uint16_t)USB_CNTR_RESUME;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       StopRemoteWakeup
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Device De Activate Remote Wakeup.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::StopRemoteWakeup(void)
{
    m_pUSB->CNTR &= (uint16_t)(~USB_CNTR_RESUME);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       WritePMA
//
//  Parameter(s):   pBuffer, PMA_Address, Length
//  Return:         None
//
//  Description:    Copy bytes to packet memory without reading past an odd-length buffer.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::WritePMA(const uint8_t* pBuffer, uint16_t PMA_Address, uint16_t Length)
{
    volatile uint16_t* pMemory = reinterpret_cast<volatile uint16_t*>(uintptr_t(m_pUSB) + 0x400U + PMA_Address);
    while(Length >= 2U)
    {
        *pMemory++ = uint16_t(pBuffer[0]) | (uint16_t(pBuffer[1]) << 8);
        pBuffer += 2;
        Length -= 2U;
    }
    if(Length != 0U)
    {
        *pMemory = pBuffer[0];
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ReadPMA
//
//  Parameter(s):   pBuffer, PMA_Address, Length
//  Return:         None
//
//  Description:    Device Read PMA.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ReadPMA(uint8_t* pBuffer, uint16_t PMA_Address, uint16_t Length)
{
    volatile const uint16_t* pMemory =
        reinterpret_cast<volatile const uint16_t*>(uintptr_t(m_pUSB) + 0x400U + PMA_Address);
    while(Length >= 2U)
    {
        uint16_t Value = *pMemory++;
        *pBuffer++     = uint8_t(Value);
        *pBuffer++     = uint8_t(Value >> 8);
        Length -= 2U;
    }
    if(Length != 0U)
    {
        *pBuffer = uint8_t(*pMemory);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       WriteEndpointRegister
//
//  Parameter(s):   Endpoint, Value
//  Return:         None
//
//  Description:    Set Endpoint.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::WriteEndpointRegister(uint16_t Endpoint, uint16_t Value)
{
    (*(__IO uint16_t*)(&(m_pUSB)->EP0R + ((Endpoint)*2U)) = (uint16_t)(Value));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ReadEndpointRegister
//
//  Parameter(s):   Endpoint
//  Return:         uint16_t
//
//  Description:    Get Endpoint.
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_Driver::ReadEndpointRegister(uint16_t Endpoint)
{
    return (*(__IO uint16_t*)(&(m_pUSB)->EP0R + ((Endpoint)*2U)));
}

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       ReleaseBuffer
//
//  Parameter(s):   Endpoint, Direction
//  Return:         None
//
//  Description:    Free User Buffer.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ReleaseBuffer(uint16_t Endpoint, uint16_t Direction)
{
    if((Direction) == 0U)
    {
        ToggleEndpointTx((Endpoint));
    }
    else if((Direction) == 1U)
    {
        ToggleEndpointRx((Endpoint));
    }
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointTxStatus
//
//  Parameter(s):   Endpoint, State
//  Return:         None
//
//  Description:    Set Ep Tx Status.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointTxStatus(uint16_t Endpoint, uint16_t State)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint)) & USB_EPTX_DTOGMASK;
    if((USB_EPTX_DTOG1 & (State)) != 0U)
    {
        RegisterValue ^= USB_EPTX_DTOG1;
    }
    if((USB_EPTX_DTOG2 & (State)) != 0U)
    {
        RegisterValue ^= USB_EPTX_DTOG2;
    }
    WriteEndpointRegister((Endpoint), (RegisterValue | USB_EP_CTR_RX | USB_EP_CTR_TX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointRxStatus
//
//  Parameter(s):   Endpoint, State
//  Return:         None
//
//  Description:    Set Ep Rx Status.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointRxStatus(uint16_t Endpoint, uint16_t State)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint)) & USB_EPRX_DTOGMASK;
    if((USB_EPRX_DTOG1 & (State)) != 0U)
    {
        RegisterValue ^= USB_EPRX_DTOG1;
    }
    if((USB_EPRX_DTOG2 & (State)) != 0U)
    {
        RegisterValue ^= USB_EPRX_DTOG2;
    }
    WriteEndpointRegister((Endpoint), (RegisterValue | USB_EP_CTR_RX | USB_EP_CTR_TX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointKind
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Set Ep Kind.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointKind(uint16_t Endpoint)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint)) & USB_EPREG_MASK;
    WriteEndpointRegister((Endpoint), (RegisterValue | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_KIND));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearEndpointKind
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Clear Ep Kind.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearEndpointKind(uint16_t Endpoint)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint)) & USB_EPKIND_MASK;
    WriteEndpointRegister((Endpoint), (RegisterValue | USB_EP_CTR_RX | USB_EP_CTR_TX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetOutStatus
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Set Out Status.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetOutStatus(uint16_t Endpoint)
{
    SetEndpointKind((Endpoint));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearOutStatus
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Clear Out Status.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearOutStatus(uint16_t Endpoint)
{
    ClearEndpointKind((Endpoint));
}

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       SetBulkDoubleBuffer
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Set Bulk Ep Dbuf.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetBulkDoubleBuffer(uint16_t Endpoint)
{
    SetEndpointKind((Endpoint));
}
#endif


#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearBulkDoubleBuffer
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Clear Bulk Ep Dbuf.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearBulkDoubleBuffer(uint16_t Endpoint)
{
    ClearEndpointKind((Endpoint));
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearEndpointRxEvent
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Clear Rx Ep Ctr.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearEndpointRxEvent(uint16_t Endpoint)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint)) & (0x7FFFU & USB_EPREG_MASK);
    WriteEndpointRegister((Endpoint), (RegisterValue | USB_EP_CTR_TX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearEndpointTxEvent
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Clear Tx Ep Ctr.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearEndpointTxEvent(uint16_t Endpoint)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint)) & (0xFF7FU & USB_EPREG_MASK);
    WriteEndpointRegister((Endpoint), (RegisterValue | USB_EP_CTR_RX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ToggleEndpointRx
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Rx Dtog.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ToggleEndpointRx(uint16_t Endpoint)
{
    uint16_t EndpointValue;
    EndpointValue = ReadEndpointRegister((Endpoint)) & USB_EPREG_MASK;
    WriteEndpointRegister((Endpoint), (EndpointValue | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_RX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ToggleEndpointTx
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Tx Dtog.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ToggleEndpointTx(uint16_t Endpoint)
{
    uint16_t EndpointValue;
    EndpointValue = ReadEndpointRegister((Endpoint)) & USB_EPREG_MASK;
    WriteEndpointRegister((Endpoint), (EndpointValue | USB_EP_CTR_RX | USB_EP_CTR_TX | USB_EP_DTOG_TX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearEndpointRxToggle
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Clear Rx Dtog.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearEndpointRxToggle(uint16_t Endpoint)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint));
    if((RegisterValue & USB_EP_DTOG_RX) != 0U)
    {
        ToggleEndpointRx((Endpoint));
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearEndpointTxToggle
//
//  Parameter(s):   Endpoint
//  Return:         None
//
//  Description:    Clear Tx Dtog.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearEndpointTxToggle(uint16_t Endpoint)
{
    uint16_t RegisterValue;
    RegisterValue = ReadEndpointRegister((Endpoint));
    if((RegisterValue & USB_EP_DTOG_TX) != 0U)
    {
        ToggleEndpointTx((Endpoint));
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointAddress
//
//  Parameter(s):   Endpoint, Address
//  Return:         None
//
//  Description:    Set Ep Address.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointAddress(uint16_t Endpoint, uint16_t Address)
{
    uint16_t RegisterValue;
    RegisterValue = (ReadEndpointRegister((Endpoint)) & USB_EPREG_MASK) | (Address);
    WriteEndpointRegister((Endpoint), (RegisterValue | USB_EP_CTR_RX | USB_EP_CTR_TX));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointTxCountRegister
//
//  Parameter(s):   Endpoint
//  Return:         volatile uint16_t*
//
//  Description:    Ep Tx Cnt.
//
//-------------------------------------------------------------------------------------------------
volatile uint16_t* USB_Driver::GetEndpointTxCountRegister(uint16_t Endpoint)
{
    return ((volatile uint16_t*)((((uint32_t)m_pUSB->BTABLE + ((uint32_t)(Endpoint)*8U) + 2U) * USB_PMA_ACCESS) +
                                 (uintptr_t(m_pUSB) + 0x400U)));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointRxCountRegister
//
//  Parameter(s):   Endpoint
//  Return:         volatile uint16_t*
//
//  Description:    Ep Rx Cnt.
//
//-------------------------------------------------------------------------------------------------
volatile uint16_t* USB_Driver::GetEndpointRxCountRegister(uint16_t Endpoint)
{
    return ((volatile uint16_t*)((((uint32_t)m_pUSB->BTABLE + ((uint32_t)(Endpoint)*8U) + 6U) * USB_PMA_ACCESS) +
                                 (uintptr_t(m_pUSB) + 0x400U)));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointTxAddress
//
//  Parameter(s):   Endpoint, Address
//  Return:         None
//
//  Description:    Set Ep Tx Address.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointTxAddress(uint16_t Endpoint, uint16_t Address)
{
    __IO uint16_t* RegisterValue;
    uintptr_t      RegisterBase = uintptr_t(m_pUSB);
    RegisterBase += (uint32_t)m_pUSB->BTABLE;
    RegisterValue  = (__IO uint16_t*)(RegisterBase + 0x400U + (((uint32_t)(Endpoint)*8U) * USB_PMA_ACCESS));
    *RegisterValue = ((Address) >> 1) << 1;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointRxAddress
//
//  Parameter(s):   Endpoint, Address
//  Return:         None
//
//  Description:    Set Ep Rx Address.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointRxAddress(uint16_t Endpoint, uint16_t Address)
{
    __IO uint16_t* RegisterValue;
    uintptr_t      RegisterBase = uintptr_t(m_pUSB);
    RegisterBase += (uint32_t)m_pUSB->BTABLE;
    RegisterValue  = (__IO uint16_t*)(RegisterBase + 0x400U + ((((uint32_t)(Endpoint)*8U) + 4U) * USB_PMA_ACCESS));
    *RegisterValue = ((Address) >> 1) << 1;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointRxBuffer0Count
//
//  Parameter(s):   Endpoint, Count
//  Return:         None
//
//  Description:    Set Ep Rx Dbuf Cnt.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointRxBuffer0Count(uint16_t Endpoint, uint16_t Count)
{
    uintptr_t RegisterBase = uintptr_t(m_pUSB);
    __IO uint16_t* pRegister;
    RegisterBase += (uint32_t)m_pUSB->BTABLE;
    pRegister = (__IO uint16_t*)(RegisterBase + 0x400U + ((((uint32_t)(Endpoint)*8U) + 2U) * USB_PMA_ACCESS));
    SetReceiveCountRegister(pRegister, (Count));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointTxCount
//
//  Parameter(s):   Endpoint, Count
//  Return:         None
//
//  Description:    Set Ep Tx Cnt.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointTxCount(uint16_t Endpoint, uint16_t Count)
{
    uintptr_t RegisterBase = uintptr_t(m_pUSB);
    __IO uint16_t* RegisterValue;
    RegisterBase += (uint32_t)m_pUSB->BTABLE;
    RegisterValue  = (__IO uint16_t*)(RegisterBase + 0x400U + ((((uint32_t)(Endpoint)*8U) + 2U) * USB_PMA_ACCESS));
    *RegisterValue = (uint16_t)(Count);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointRxCount
//
//  Parameter(s):   Endpoint, Count
//  Return:         None
//
//  Description:    Set Ep Rx Cnt.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointRxCount(uint16_t Endpoint, uint16_t Count)
{
    uintptr_t RegisterBase = uintptr_t(m_pUSB);
    __IO uint16_t* RegisterValue;
    RegisterBase += (uint32_t)m_pUSB->BTABLE;
    RegisterValue = (__IO uint16_t*)(RegisterBase + 0x400U + ((((uint32_t)(Endpoint)*8U) + 6U) * USB_PMA_ACCESS));
    SetReceiveCountRegister(RegisterValue, (Count));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointTxCount
//
//  Parameter(s):   Endpoint
//  Return:         uint16_t
//
//  Description:    Get Ep Tx Cnt.
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_Driver::GetEndpointTxCount(uint16_t Endpoint)
{
    return ((uint32_t)(*GetEndpointTxCountRegister((Endpoint))) & 0x3ffU);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointRxCount
//
//  Parameter(s):   Endpoint
//  Return:         uint16_t
//
//  Description:    Get Ep Rx Cnt.
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_Driver::GetEndpointRxCount(uint16_t Endpoint)
{
    return ((uint32_t)(*GetEndpointRxCountRegister((Endpoint))) & 0x3ffU);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointBuffer0Address
//
//  Parameter(s):   Endpoint, Address0
//  Return:         None
//
//  Description:    Set Ep Dbuf Addr.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointBuffer0Address(uint16_t Endpoint, uint16_t Address0)
{
    SetEndpointTxAddress((Endpoint), (Address0));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointBuffer1Address
//
//  Parameter(s):   Endpoint, Address1
//  Return:         None
//
//  Description:    Set Ep Dbuf Addr.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointBuffer1Address(uint16_t Endpoint, uint16_t Address1)
{
    SetEndpointRxAddress((Endpoint), (Address1));
}

#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointBufferAddress
//
//  Parameter(s):   Endpoint, Address0, Address1
//  Return:         None
//
//  Description:    Set Ep Dbuf Addr.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointBufferAddress(uint16_t Endpoint, uint16_t Address0, uint16_t Address1)
{
    SetEndpointBuffer0Address((Endpoint), (Address0));
    SetEndpointBuffer1Address((Endpoint), (Address1));
}
#endif


#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointBuffer0Count
//
//  Parameter(s):   Endpoint, Direction, Count
//  Return:         None
//
//  Description:    Set Ep Dbuf Cnt.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointBuffer0Count(uint16_t Endpoint, uint16_t Direction, uint16_t Count)
{
    if((Direction) == 0U)
    {
        SetEndpointRxBuffer0Count((Endpoint), (Count));
    }
    else
    {
        if((Direction) == 1U)
        {
            SetEndpointTxCount((Endpoint), (Count));
        }
    }
}
#endif


#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointBuffer1Count
//
//  Parameter(s):   Endpoint, Direction, Count
//  Return:         None
//
//  Description:    Set Ep Dbuf Cnt.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointBuffer1Count(uint16_t Endpoint, uint16_t Direction, uint16_t Count)
{
    uintptr_t BaseAddress = uintptr_t(m_pUSB);
    __IO uint16_t* pRegister;
    if((Direction) == 0U)
    {
        SetEndpointRxCount((Endpoint), (Count));
    }
    else
    {
        if((Direction) == 1U)
        {
            BaseAddress += (uint32_t)m_pUSB->BTABLE;
            pRegister  = (__IO uint16_t*)(BaseAddress + 0x400U + ((((uint32_t)(Endpoint)*8U) + 6U) * USB_PMA_ACCESS));
            *pRegister = (uint16_t)(Count);
        }
    }
}
#endif


#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointBufferCount
//
//  Parameter(s):   Endpoint, Direction, Count
//  Return:         None
//
//  Description:    Set Ep Dbuf Cnt.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointBufferCount(uint16_t Endpoint, uint16_t Direction, uint16_t Count)
{
    SetEndpointBuffer0Count((Endpoint), (Direction), (Count));
    SetEndpointBuffer1Count((Endpoint), (Direction), (Count));
}
#endif


#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointBuffer0Count
//
//  Parameter(s):   Endpoint
//  Return:         uint16_t
//
//  Description:    Get Ep Dbuf Cnt.
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_Driver::GetEndpointBuffer0Count(uint16_t Endpoint)
{
    return (GetEndpointTxCount((Endpoint)));
}
#endif


#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointBuffer1Count
//
//  Parameter(s):   Endpoint
//  Return:         uint16_t
//
//  Description:    Get Ep Dbuf Cnt.
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_Driver::GetEndpointBuffer1Count(uint16_t Endpoint)
{
    return (GetEndpointRxCount((Endpoint)));
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Function:       SetReceiveCountRegister
//
//  Parameter(s):   pRegister, Count
//  Return:         None
//
//  Description:    Set Receive Count.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetReceiveCountRegister(volatile uint16_t* pRegister, uint16_t Count)
{
    uint16_t Blocks;
    uint16_t Value = *pRegister & 0x03FFU;
    if(Count == 0U)
    {
        Value |= USB_CNTRX_BLSIZE;
    }
    else if(Count <= 62U)
    {
        Blocks = (Count + 1U) >> 1;
        Value |= Blocks << 10;
    }
    else
    {
        Blocks = (Count - 1U) >> 5;
        Value |= (Blocks << 10) | USB_CNTRX_BLSIZE;
    }
    *pRegister = Value;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Lock
//
//  Parameter(s):   None
//  Return:         bool - True when the controller lock was acquired
//
//  Description:    Acquire the lock while preserving the caller's interrupt mask.
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::Lock(void)
{
    uint32_t Mask = __get_PRIMASK();
    __disable_irq();
    bool Available = (m_Locked == false);
    if(Available)
    {
        m_Locked = true;
    }
    __set_PRIMASK(Mask);
    return Available;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Unlock
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Release the controller lock.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::Unlock(void)
{
    m_Locked = false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointValid
//
//  Parameter(s):   EndpointAddress
//  Return:         bool - True when the address identifies a configured endpoint number
//
//  Description:    Check the endpoint address bits and configured endpoint count.
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::IsEndpointValid(uint8_t EndpointAddress) const
{
    return ((EndpointAddress & 0x78U) == 0U) && ((EndpointAddress & USB_ENDPOINT_MASK) < m_Info.EndpointCount);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsPMA_Valid
//
//  Parameter(s):   pEndpoint, MaxPacketSize
//  Return:         bool - True when the endpoint buffers fit in packet memory
//
//  Description:    Check packet-memory bounds and double-buffer overlap with RX size rounding.
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::IsPMA_Valid(const USB_Endpoint_t* pEndpoint, uint16_t MaxPacketSize) const
{
    uint32_t Size = (MaxPacketSize + 1U) & ~1U;
    if((pEndpoint->IsIn == 0U) && (MaxPacketSize > 62U))
    {
        Size = (MaxPacketSize + 31U) & ~31U;
    }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    uint32_t Address0 = (pEndpoint->DoubleBuffer != 0U) ? pEndpoint->PMA_Address0 : pEndpoint->PMA_Address;
#else
    uint32_t Address0 = pEndpoint->PMA_Address;
#endif
    if((Address0 < USB_ENDPOINT_COUNT * 8U) || ((Address0 + Size) > USB_PMA_SIZE))
    {
        return false;
    }
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    if(pEndpoint->DoubleBuffer != 0U)
    {
        uint32_t Address1 = pEndpoint->PMA_Address1;
        if((Address1 < USB_ENDPOINT_COUNT * 8U) || ((Address1 + Size) > USB_PMA_SIZE) ||
           ((Address0 < (Address1 + Size)) && (Address1 < (Address0 + Size))))
        {
            return false;
        }
    }
#endif
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpoint
//
//  Parameter(s):   EndpointAddress
//  Return:         const USB_Endpoint_t* - Endpoint state or nullptr for an invalid address
//
//  Description:    Return read-only transfer state for the selected endpoint direction.
//
//-------------------------------------------------------------------------------------------------
const USB_Endpoint_t* USB_Driver::GetEndpoint(uint8_t EndpointAddress) const
{
    if(IsEndpointValid(EndpointAddress) == false)
    {
        return nullptr;
    }
    return ((EndpointAddress & 0x80U) != 0U) ? &m_InEndpoint[EndpointAddress & USB_ENDPOINT_MASK]
                                             : &m_OutEndpoint[EndpointAddress & USB_ENDPOINT_MASK];
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointStatus
//
//  Parameter(s):   EndpointAddress, Status
//  Return:         SystemState_e
//
//  Description:    Set one endpoint direction's status while preserving pending transfer events.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::SetEndpointStatus(uint8_t EndpointAddress, uint16_t Status)
{
    bool In = (EndpointAddress & 0x80U) != 0U;
    uint16_t Mask = In ? USB_EPTX_STAT : USB_EPRX_STAT;
    if((IsEndpointValid(EndpointAddress) == false) || ((Status & ~Mask) != 0U))
    {
        return SYS_WRONG_VALUE;
    }
    if(In)
    {
        SetEndpointTxStatus(EndpointAddress & USB_ENDPOINT_MASK, Status);
    }
    else
    {
        SetEndpointRxStatus(EndpointAddress & USB_ENDPOINT_MASK, Status);
    }
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointStatus
//
//  Parameter(s):   EndpointAddress, pStatus
//  Return:         SystemState_e
//
//  Description:    Read the selected endpoint direction's hardware status.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_Driver::GetEndpointStatus(uint8_t EndpointAddress, uint16_t* pStatus) const
{
    if((IsEndpointValid(EndpointAddress) == false) || (pStatus == nullptr))
    {
        return SYS_WRONG_VALUE;
    }
    const volatile uint16_t* pRegister = reinterpret_cast<const volatile uint16_t*>(
        uintptr_t(m_pUSB) + (EndpointAddress & USB_ENDPOINT_MASK) * 4U);
    *pStatus = *pRegister & (((EndpointAddress & 0x80U) != 0U) ? USB_EPTX_STAT : USB_EPRX_STAT);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Notify
//
//  Parameter(s):   pCallback, Event, Value
//  Return:         None
//
//  Description:    Dispatch an event with a temporary context valid only during the callback.
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::Notify(CallbackInterface* pCallback, USB_Callback_e Event, uint32_t Value)
{
    if(pCallback != nullptr)
    {
        USB_CallbackInfo_t Info = {this, Value};
        pCallback->CallbackFunction(Event, &Info);
    }
}

#endif // (USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)
