//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_hid.cpp
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 HID and CustomHID.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

#if (USE_USB_DRIVER == DEF_ENABLED) && \
    ((USE_USB_HID_DRIVER == DEF_ENABLED) || (USE_USB_CUSTOM_HID_DRIVER == DEF_ENABLED))

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_HID_Base
//  Parameter(s):   Backend, Identity, pDescriptor, DescriptorLength, pReports, Mouse
//  Return:         None
//  Description:    Build the standalone full-speed HID interface.
//
//-------------------------------------------------------------------------------------------------
USB_HID_Base::USB_HID_Base(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
    const uint8_t* pDescriptor, uint16_t DescriptorLength, USB_HID_ReportInterface* pReports, bool Mouse)
    : USB_ClassBase(Backend, Identity)
{
    m_pReportDescriptor = pDescriptor;
    m_ReportDescriptorLength = DescriptorLength;
    m_pReports = pReports;
    m_Mouse = Mouse;
    const uint8_t Descriptor[] = {9, 2, uint8_t(Mouse ? 34 : 41), 0, 1, 1, 0, 0x80, 50,
        9, 4, 0, 0, uint8_t(Mouse ? 1 : 2), 3, uint8_t(Mouse ? 1 : 0), uint8_t(Mouse ? 2 : 0), 0,
        9, 0x21, 0x11, 1, 0, 1, 0x22, uint8_t(DescriptorLength), uint8_t(DescriptorLength >> 8),
        7, 5, 0x81, 3, uint8_t(Mouse ? 4 : 64), 0, 10,
        7, 5, 0x01, 3, 64, 0, 10};
    static_assert(sizeof(Descriptor) == 41, "Invalid HID configuration length");
    memcpy(m_ConfigurationDescriptor, Descriptor, sizeof(Descriptor));
    m_ReportType = 0;
    m_ControlGeneration = 0;
    m_Generation = 0;
    ResetClassState();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       InitializeClass
//  Parameter(s):   None
//  Return:         bool
//  Description:    Require persistent descriptors and a Custom HID report provider.
//
//-------------------------------------------------------------------------------------------------
bool USB_HID_Base::InitializeClass(void)
{
    return USB_ClassBase::InitializeClass() && (m_pReportDescriptor != nullptr) &&
           (m_ReportDescriptorLength != 0) && (m_Mouse || (m_pReports != nullptr));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResetClassState
//  Parameter(s):   None
//  Return:         None
//  Description:    Discard transfers and restore report protocol defaults.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::ResetClassState(void)
{
    m_Generation++;
    m_ReportType = 0;
    CancelClassControl();
    m_TxBusy = false;
    m_TxLength = m_Mouse ? 4 : 0;
    memset(m_TxReport, 0, sizeof(m_TxReport));
    m_ReportPending = false;
    m_OutPending = false;
    m_OutLength = 0;
    m_ReportLength = 0;
    m_Idle = 0;
    m_Protocol = 1;
    m_Feature = 0;
    m_LastReportTick = uint32_t(GetTick());
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ConfigureClassEndpoints
//  Parameter(s):   None
//  Return:         None
//  Description:    Allocate interrupt endpoints; mouse reports need IN only.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::ConfigureClassEndpoints(void)
{
    ConfigureEndpoint(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_INTERRUPT, m_Mouse ? 4 : 64);
    SetEndpointState(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_NAK);
    if(m_Mouse == false)
    {
        ConfigureEndpoint(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_INTERRUPT, 64);
        EnableEndpointRx(1, 64);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetConfiguration
//  Parameter(s):   pLength
//  Return:         const uint8_t*
//  Description:    Return the instance-owned HID configuration.
//
//-------------------------------------------------------------------------------------------------
const uint8_t* USB_HID_Base::GetConfiguration(uint16_t* pLength) const
{
    *pLength = m_Mouse ? 34 : 41;
    return m_ConfigurationDescriptor;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SendReport
//  Parameter(s):   pData, Length
//  Return:         SystemState_e
//  Description:    Copy one report and retain it until interrupt IN completion.
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_HID_Base::SendReport(const uint8_t* pData, uint16_t Length)
{
    if((pData == nullptr) || (Length == 0) || (Length > 64) || (m_Mouse && (Length != 4)))
    {
        return SYS_WRONG_VALUE;
    }
    uint32_t State = __get_PRIMASK();
    __disable_irq();
    SystemState_e Result = SYS_BUSY;
    if(IsConfigured() && (m_TxBusy == false) && (IsEndpointTxStalled(1) == false))
    {
        memcpy(m_TxReport, pData, Length);
        m_TxLength = Length;
        m_TxBusy = true;
        Transmit(1, m_TxReport, (m_Mouse && !m_Protocol) ? 3 : Length);
        Result = SYS_READY;
    }
    __set_PRIMASK(State);
    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassSetup
//  Parameter(s):   Type, Request, Value, Index, Length
//  Return:         bool
//  Description:    Handle HID descriptors, report, idle and boot protocol requests.
//
//-------------------------------------------------------------------------------------------------
bool USB_HID_Base::HandleClassSetup(uint8_t Type, uint8_t Request, uint16_t Value,
    uint16_t Index, uint16_t Length)
{
    if(Index != 0)
    {
        return false;
    }
    if((Type == 0x81) && (Request == 6) && (Length != 0) &&
       ((Value == 0x2100) || (Value == 0x2200)))
    {
        ControlSend(Value == 0x2100 ? m_ConfigurationDescriptor + 18 : m_pReportDescriptor,
            Value == 0x2100 ? 9 : m_ReportDescriptorLength, Length);
        return true;
    }
    if(m_Configured == false)
    {
        return false;
    }
    if((Type == 0xA1) && (Request == 2) && (Value == 0) && (Length == 1))
    {
        m_ControlBuffer[0] = m_Idle;
        ControlSend(m_ControlBuffer, 1, Length);
    }
    else if((Type == 0x21) && (Request == 10) && ((Value & 255) == 0) && (Length == 0))
    {
        m_Idle = uint8_t(Value >> 8);
        ControlSend(nullptr, 0, 0);
    }
    else if((Type == 0xA1) && (Request == 3) && (Value == 0) && (Length == 1) && m_Mouse)
    {
        m_ControlBuffer[0] = m_Protocol;
        ControlSend(m_ControlBuffer, 1, Length);
    }
    else if((Type == 0x21) && (Request == 11) && (Value <= 1) && (Length == 0) && m_Mouse)
    {
        m_Protocol = uint8_t(Value);
        ControlSend(nullptr, 0, 0);
    }
    else if((Type == 0xA1) && (Request == 1) && (Length != 0) && (Value >= 0x0100) && (Value < 0x0400))
    {
        if(m_Mouse)
        {
            if(Value == 0x0100)
            {
                memcpy(m_ControlBuffer, m_TxReport, 4);
                // Boot mouse protocol omits the wheel.
                ControlSend(m_ControlBuffer, m_Protocol ? 4 : 3, Length);
            }
            else if(Value == 0x0300)
            {
                m_ControlBuffer[0] = m_Feature;
                ControlSend(m_ControlBuffer, 1, Length);
            }
            else
            {
                return false;
            }
        }
        else
        {
            if(m_ReportPending)
            {
                return false;
            }
            m_ReportType = uint8_t(Value >> 8);
            m_ReportId = uint8_t(Value);
            m_ReportLength = Length;
            m_ReportPending = true;
            // NAK EP0 until the task has obtained the application report.
        }
    }
    else if((Type == 0x21) && (Request == 9) && (Length != 0) && (Length <= 64) &&
            ((Value >> 8) >= 2) && ((Value >> 8) <= 3) && (m_ReportPending == false))
    {
        if(m_Mouse && ((Value != 0x0300) || (Length != 1)))
        {
            return false;
        }
        m_ReportType = uint8_t(Value >> 8);
        m_ReportId = uint8_t(Value);
        return BeginControlReceive(m_RxReport, Length, sizeof(m_RxReport));
    }
    else
    {
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ControlReceiveComplete
//  Parameter(s):   Length
//  Return:         None
//  Description:    Queue SET_REPORT for task execution before acknowledging its status.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::ControlReceiveComplete(uint16_t Length)
{
    m_ReportLength = Length;
    m_ReportType |= 0x80;
    m_ReportPending = true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassEndpointRx
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Keep interrupt OUT NAKed until the task consumes its report.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::HandleClassEndpointRx(uint8_t Endpoint)
{
    if((Endpoint == 1) && m_OutPending)
    {
        SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
        return;
    }
    if((Endpoint == 1) && (m_Mouse == false) && (m_OutPending == false))
    {
        m_OutLength = GetEndpointRxLength(1);
        if(m_OutLength > sizeof(m_OutReport))
        {
            SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
            return;
        }
        ReadEndpointRx(1, m_OutReport, m_OutLength);
        m_OutPending = true;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassEndpointTx
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Release the retained interrupt IN report.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::HandleClassEndpointTx(uint8_t Endpoint)
{
    if(Endpoint == 1)
    {
        m_TxBusy = false;
        m_LastReportTick = uint32_t(GetTick());
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleEndpointHaltCleared
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Release an aborted IN transfer and rearm an idle OUT report pipe.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::HandleEndpointHaltCleared(uint8_t Endpoint)
{
    if(Endpoint == 1)
    {
        if(IsEndpointTxReady(1))
        {
            m_TxBusy = false;
        }
        if(!m_Mouse && !m_OutPending &&
           !m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
        {
            EnableEndpointRx(1, 64);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointAvailable
//  Parameter(s):   Endpoint, In
//  Return:         bool
//  Description:    Validate endpoint recipients for standard requests.
//
//-------------------------------------------------------------------------------------------------
bool USB_HID_Base::IsEndpointAvailable(uint8_t Endpoint, bool In) const
{
    return (Endpoint == 1) && (In || (m_Mouse == false));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ProcessClass
//  Parameter(s):   None
//  Return:         None
//  Description:    Application report callbacks are deliberately left to ProcessTask.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::ProcessClass(void)
{
    uint32_t Period = (uint32_t(m_Idle) * 4U * TICKS_PER_SECOND + 999U) / 1000U;
    if(IsConfigured() && (m_Idle != 0) && !m_TxBusy && (m_TxLength != 0) &&
       !IsEndpointTxStalled(1) && (uint32_t(GetTick()) - m_LastReportTick >= Period))
    {
        // Relative mouse movement must not be repeated when only the idle timer expires.
        if(m_Mouse)
        {
            m_TxReport[1] = m_TxReport[2] = m_TxReport[3] = 0;
        }
        m_TxBusy = true;
        Transmit(1, m_TxReport, (m_Mouse && !m_Protocol) ? 3 : m_TxLength);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ProcessTask
//  Parameter(s):   None
//  Return:         None
//  Description:    Execute report callbacks outside the USB interrupt.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::ProcessTask(void)
{
    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    if(!m_ReportPending && !m_OutPending)
    {
        __set_PRIMASK(InterruptState);
        return;
    }
    bool OutEndpoint = m_OutPending;
    bool Set = OutEndpoint || ((m_ReportType & 0x80) != 0);
    uint32_t Generation = m_Generation;
    uint32_t ControlGeneration = m_ControlGeneration;
    uint8_t ReportType = OutEndpoint ? 2 : (m_ReportType & 3);
    uint8_t ReportId = OutEndpoint ? 0 : m_ReportId;
    uint16_t Requested = OutEndpoint ? m_OutLength : m_ReportLength;
    uint16_t Capacity = sizeof(m_ControlBuffer);
    uint8_t Buffer[128];
    if(Set)
    {
        memcpy(Buffer, OutEndpoint ? m_OutReport : m_RxReport, Requested);
    }
    __set_PRIMASK(InterruptState);
    bool Result;
    if(m_Mouse)
    {
        Result = true;
    }
    else if(Set)
    {
        Result = m_pReports->SetReport(ReportType, ReportId, Buffer, Requested);
    }
    else
    {
        Result = m_pReports->GetReport(ReportType, ReportId, Buffer, &Capacity);
        if(Capacity > sizeof(m_ControlBuffer))
        {
            Result = false;
        }
    }
    __disable_irq();
    if((Generation != m_Generation) || (!OutEndpoint && (ControlGeneration != m_ControlGeneration)))
    {
        __set_PRIMASK(InterruptState);
        return;
    }
    if(Result == false)
    {
        if(OutEndpoint)
        {
            SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
        }
        else
        {
            StallControl();
        }
    }
    else if(Set && !OutEndpoint)
    {
        if(m_Mouse)
        {
            m_Feature = Buffer[0] & 3;
        }
        ControlSend(nullptr, 0, 0);
    }
    else if(!Set)
    {
        memcpy(m_ControlBuffer, Buffer, Capacity);
        ControlSend(m_ControlBuffer, Capacity, Requested);
    }
    if(OutEndpoint)
    {
        m_OutPending = false;
    }
    else
    {
        m_ReportPending = false;
    }
    if(!m_Mouse && !m_OutPending && !m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
    {
        EnableEndpointRx(1, 64);
    }
    __set_PRIMASK(InterruptState);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CancelClassControl
//  Parameter(s):   None
//  Return:         None
//  Description:    Cancel EP0 work without losing an already accepted interrupt OUT report.
//
//-------------------------------------------------------------------------------------------------
void USB_HID_Base::CancelClassControl(void)
{
    USB_ClassBase::CancelClassControl();
    m_ControlGeneration++;
    m_ReportPending = false;
}

#if (USE_USB_HID_DRIVER == DEF_ENABLED)
static const uint8_t USB_MouseReport[] = {
    0x05, 0x01, 0x09, 0x02, 0xA1, 0x01, 0x09, 0x01, 0xA1, 0x00, 0x05, 0x09,
    0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x95, 0x03, 0x75, 0x01,
    0x81, 0x02, 0x95, 0x01, 0x75, 0x05, 0x81, 0x01, 0x05, 0x01, 0x09, 0x30,
    0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7F, 0x75, 0x08, 0x95, 0x03,
    0x81, 0x06, 0xC0, 0x09, 0x3C, 0x05, 0xFF, 0x09, 0x01, 0x15, 0x00, 0x25,
    0x01, 0x75, 0x01, 0x95, 0x02, 0xB1, 0x22, 0x75, 0x06, 0x95, 0x01, 0xB1,
    0x01, 0xC0};
static_assert(sizeof(USB_MouseReport) == 74, "Invalid mouse report descriptor length");

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_HID_Driver
//  Parameter(s):   Backend, Identity
//  Return:         None
//  Description:    Construct the ST three-button, X/Y/wheel mouse HID profile.
//
//-------------------------------------------------------------------------------------------------
USB_HID_Driver::USB_HID_Driver(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity)
    : USB_HID_Base(Backend, Identity, USB_MouseReport, sizeof(USB_MouseReport), nullptr, true)
{
}
#endif

#if (USE_USB_CUSTOM_HID_DRIVER == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_CustomHID_Driver
//  Parameter(s):   Backend, Identity, pDescriptor, DescriptorLength, Reports
//  Return:         None
//  Description:    Bind an application report descriptor and provider.
//
//-------------------------------------------------------------------------------------------------
USB_CustomHID_Driver::USB_CustomHID_Driver(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
    const uint8_t* pDescriptor, uint16_t DescriptorLength, USB_HID_ReportInterface& Reports)
    : USB_HID_Base(Backend, Identity, pDescriptor, DescriptorLength, &Reports, false)
{
}
#endif

#endif
