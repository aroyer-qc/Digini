//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_device.cpp
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

#include "../../../../lib_digini.h"

#if (USE_USB_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Function:       USB_DeviceDriver
// Parameter(s):   Backend - device controller backend
// Return:         void
//
// Description:    Initialize generic USB port state
//
//-------------------------------------------------------------------------------------------------
USB_DeviceDriver::USB_DeviceDriver(USB_DeviceBackend& Backend)
    : m_Backend(Backend)
{
    m_Configured        = false;
    m_Suspended         = false;
    m_ControlRemaining  = 0;
    m_ControlZLP        = false;
    m_pControlData      = nullptr;
    m_AddressPending    = false;
    m_PendingAddress    = 0;
    m_Configuration     = 0;
    m_Initialized       = false;
#if DIGINI_USB_DEVICE_CLASSES
    m_InInterrupt       = false;
#endif
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Initialize
//
// Parameter(s):   void
// Return:         bool - true when the USB device is initialized
//
// Description:    Initialize the USB peripheral and enable its interrupt
//
//-------------------------------------------------------------------------------------------------
bool USB_DeviceDriver::Initialize(void)
{
    if(m_Initialized == true)
    {
        return true;
    }

    if(InitializeClass() == false)
    {
        return false;
    }

    if(m_Backend.Initialize() == false)
    {
        return false;
    }

    Reset();
    m_Initialized = true;
    m_Backend.Start();
    return true;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       SetEndpointState
//
// Parameter(s):   Endpoint - USB endpoint number
//                 Direction - endpoint direction
//                 State - requested endpoint state
// Return:         void
//
// Description:    Set endpoint state through the device backend
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::SetEndpointState(uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                  USB_DeviceEndpointState_e State)
{
    m_Backend.SetEndpointState(Endpoint, Direction, State);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ConfigureEndpoint
// Parameter(s):   Endpoint - endpoint number
//                 Direction - endpoint direction
//                 Type - endpoint transfer type
//                 MaxPacketSize - maximum packet size in bytes
// Return:         void
//
// Description:    Configure an endpoint through the device backend
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::ConfigureEndpoint(uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                   USB_DeviceEndpointType_e Type, uint16_t MaxPacketSize)
{
    m_Backend.ConfigureEndpoint(Endpoint, Direction, Type, MaxPacketSize);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IsEndpointTxReady
// Parameter(s):   Endpoint - endpoint number
// Return:         bool - true when the endpoint can accept a transmit packet
//
// Description:    Check whether an endpoint transmit status is NAK
//
//-------------------------------------------------------------------------------------------------
bool USB_DeviceDriver::IsEndpointTxReady(uint8_t Endpoint) const
{
    return m_Backend.IsEndpointTxReady(Endpoint);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IsEndpointTxStalled
// Parameter(s):   Endpoint - endpoint number
// Return:         bool - true when the endpoint transmit direction is stalled
//
// Description:    Check whether the endpoint transmit direction is stalled
//
//-------------------------------------------------------------------------------------------------
bool USB_DeviceDriver::IsEndpointTxStalled(uint8_t Endpoint) const
{
    return m_Backend.IsEndpointTxStalled(Endpoint);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IsEndpointRxBlocked
// Parameter(s):   Endpoint - endpoint number
// Return:         bool - true when the endpoint receive direction is NAK
//
// Description:    Check whether the endpoint receive direction is blocked
//
//-------------------------------------------------------------------------------------------------
bool USB_DeviceDriver::IsEndpointRxBlocked(uint8_t Endpoint) const
{
    return m_Backend.IsEndpointRxBlocked(Endpoint);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       EnableEndpointRx
// Parameter(s):   Endpoint - endpoint number
//                 Capacity - receive buffer capacity in bytes
// Return:         void
//
// Description:    Set the PMA receive capacity and enable endpoint reception
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::EnableEndpointRx(uint8_t Endpoint, uint16_t Capacity)
{
    m_Backend.EnableEndpointRx(Endpoint, Capacity);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       GetEndpointRxLength
// Parameter(s):   Endpoint - endpoint number
// Return:         uint16_t - received packet length
//
// Description:    Read the received byte count from endpoint packet memory
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_DeviceDriver::GetEndpointRxLength(uint8_t Endpoint) const
{
    return m_Backend.GetEndpointRxLength(Endpoint);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ReadEndpointRx
// Parameter(s):   Endpoint - endpoint number
//                 pData - destination buffer
//                 Length - number of bytes to read
// Return:         void
//
// Description:    Copy an endpoint receive packet from PMA to a class buffer
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::ReadEndpointRx(uint8_t Endpoint, uint8_t* pData, uint16_t Length)
{
    m_Backend.ReadEndpointRx(Endpoint, pData, Length);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Transmit
//
// Parameter(s):   Endpoint - USB endpoint number
//                 pData - source data
//                 Length - number of bytes to transmit
// Return:         void
//
// Description:    Write endpoint data to packet memory and start transmission
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::Transmit(uint8_t Endpoint, const uint8_t* pData, uint16_t Length)
{
    m_Backend.Transmit(Endpoint, pData, Length);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Reset
// Parameter(s):   void
// Return:         void
//
// Description:    Reset USB state and initialize the control endpoint
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::Reset(void)
{
    m_Configured = false;
    m_Suspended = false;
    m_ControlRemaining = 0;
    m_ControlZLP = false;
    m_AddressPending = false;
    m_Configuration = 0;
    ResetClassState();
    m_Backend.ResetEndpoints();
    m_Backend.SetAddress(0);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ConfigureEndpoints
//
// Parameter(s):   void
// Return:         void
//
// Description:    Configure class endpoints for the current USB configuration
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::ConfigureEndpoints(void)
{
    m_Configured = false;

    ResetClassState();

    m_Backend.ResetNonControlEndpoints();

    if(m_Configuration == 0)
    {
        return;
    }

    ConfigureClassEndpoints();
    m_Configured = true;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ControlSend
//
// Parameter(s):   pData - control response data
//                 Length - response length
//                 Requested - host-requested length
// Return:         void
//
// Description:    Prepare a control response and start its first packet
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::ControlSend(const uint8_t* pData, uint16_t Length, uint16_t Requested)
{
    m_pControlData = pData;

    m_ControlZLP = (Length < Requested) && ((Length % USB_CONTROL_PACKET_SIZE) == 0);
    m_ControlRemaining = Length < Requested ? Length : Requested;
    ControlIn();
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ControlIn
//
// Parameter(s):   void
// Return:         void
//
// Description:    Send the next packet of a control response
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::ControlIn(void)
{

    if(m_AddressPending == true)
    {
        m_Backend.SetAddress(m_PendingAddress);
        m_AddressPending = false;
        return;
    }

    uint16_t Length = m_ControlRemaining > USB_CONTROL_PACKET_SIZE ?
        static_cast<uint16_t>(USB_CONTROL_PACKET_SIZE) : m_ControlRemaining;
    Transmit(0, m_pControlData, Length);

    if(Length != 0)
    {
        m_pControlData += Length;
    }

    m_ControlRemaining -= Length;

    if(Length == 0)
    {
        m_ControlZLP = false;
    }
}

//-------------------------------------------------------------------------------------------------
//
// Function:       StallControl
//
// Parameter(s):   void
// Return:         void
//
// Description:    Stall the control endpoint and cancel any pending control transfer
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::StallControl(void)
{
    m_ControlRemaining = 0;
    m_ControlZLP = false;
    CancelClassControl();
    m_Backend.SetEndpointState(0, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
    m_Backend.SetEndpointState(0, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_STALL);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Setup
//
// Parameter(s):   pSetup - received USB setup packet
// Return:         void
//
// Description:    Decode standard USB requests and dispatch class-specific requests
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::Setup(const uint8_t* pSetup)
{
    uint8_t Type = pSetup[0];
    uint8_t Request = pSetup[1];
    uint16_t Value = pSetup[2] | (static_cast<uint16_t>(pSetup[3]) << 8);
    uint16_t Index = pSetup[4] | (static_cast<uint16_t>(pSetup[5]) << 8);
    uint16_t Length = pSetup[6] | (static_cast<uint16_t>(pSetup[7]) << 8);
    m_AddressPending = false;
    CancelClassControl();
    m_ControlRemaining = 0;
    m_ControlZLP = false;
    m_pControlData = nullptr;
    SetEndpointState(0, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_READY);
    SetEndpointState(0, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_NAK);

    if((Type == 0x80) && (Request == 6) && (Length != 0))
    {
        if(GetClassDescriptor(Value, Length) == true)
        {
            return;
        }
    }
    else if((Type == 0) && (Request == 5) && (Value <= 127) && (Index == 0) &&
            (Length == 0) && (m_Configured == false))
    {
        Transmit(0, nullptr, 0);
        m_PendingAddress = static_cast<uint8_t>(Value);
        m_AddressPending = true;
        return;
    }
    else if((Type == 0) && (Request == 9) && (Value <= 1) && (Index == 0) &&
            (Length == 0) && (m_Backend.IsAddressed() == true))
    {
        m_Configuration = static_cast<uint8_t>(Value);
        ConfigureEndpoints();
        ControlSend(nullptr, 0, 0);
        return;
    }
    else if((Type == 0x80) && (Request == 8) && (Value == 0) && (Index == 0) && (Length == 1))
    {
        m_ControlBuffer[0] = m_Configuration;
        ControlSend(m_ControlBuffer, 1, Length);
        return;
    }
    else if((Type == 0x81) && (Request == 10) && (Value == 0) &&
            (Index < GetInterfaceCount()) && (Length == 1) && (m_Configured == true))
    {
        m_ControlBuffer[0] = 0;
        ControlSend(m_ControlBuffer, 1, Length);
        return;
    }
    else if((Type == 1) && (Request == 11) && (Value == 0) &&
            (Index < GetInterfaceCount()) && (Length == 0) && (m_Configured == true))
    {
        ControlSend(nullptr, 0, 0);
        return;
    }
    else if((Request == 0) && (Value == 0) && (Length == 2))
    {
        bool IsSupported = false;

        if(Type == 0x80)
        {
            IsSupported = (Index == 0);
        }
        else if(Type == 0x81)
        {
            IsSupported = (m_Configured == true) && (Index < GetInterfaceCount());
        }
        else if((Type == 0x82) && (Index <= 0x008F) && ((Index & 0x0070) == 0))
        {
            uint8_t Endpoint = static_cast<uint8_t>(Index & 0x0F);
            bool In = (Index & 0x80) != 0;

            IsSupported = (Endpoint == 0) ||
                ((m_Configured == true) && (IsEndpointAvailable(Endpoint, In) == true));
        }

        if(IsSupported == true)
        {
            m_ControlBuffer[0] = 0;
            m_ControlBuffer[1] = 0;

            if(Type == 0x82)
            {
                USB_DeviceDirection_e Direction = (Index & 0x80) ? USB_DEVICE_DIRECTION_IN : USB_DEVICE_DIRECTION_OUT;
                m_ControlBuffer[0] = m_Backend.IsEndpointStalled(static_cast<uint8_t>(Index & 15), Direction);
            }

            ControlSend(m_ControlBuffer, 2, Length);
            return;
        }
    }
    else if((Type == 2) && ((Request == 1) || (Request == 3)) && (Value == 0) &&
            (Length == 0) && (m_Configured == true) && (Index <= 0x008F) &&
            ((Index & 0x0070) == 0) &&
            (IsEndpointAvailable(static_cast<uint8_t>(Index & 0x0F), (Index & 0x80) != 0) == true))
    {
        uint8_t Endpoint = static_cast<uint8_t>(Index & 0x0F);
        USB_DeviceDirection_e Direction = (Index & 0x80) ? USB_DEVICE_DIRECTION_IN : USB_DEVICE_DIRECTION_OUT;

        if(Request == 1)
        {
            m_Backend.ClearEndpointHalt(Endpoint, Direction);
            HandleEndpointHaltCleared(Endpoint);
        }
        else
        {
            SetEndpointState(Endpoint, Direction, USB_DEVICE_ENDPOINT_STALL);
        }

        Process();
        ControlSend(nullptr, 0, 0);
        return;
    }
    else if(HandleClassSetup(Type, Request, Value, Index, Length) == true)
    {
        return;
    }

    StallControl();
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Process
//
// Parameter(s):   void
// Return:         void
//
// Description:    Re-enable RX when buffer space is available and advance queued TX data
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::Process(void)
{
    if(m_Initialized == false)
    {
        return;
    }

    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();

    ProcessClass();

    __set_PRIMASK(InterruptState);
#if DIGINI_USB_DEVICE_CLASSES
    if((m_InInterrupt == false) && (InterruptState == 0) && (IsConfigured() == true))
    {
        ProcessTask();
    }
#endif
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IsConfigured
//
// Parameter(s):   void
// Return:         bool - true when configured and not suspended
//
// Description:    Report whether the USB device can currently transfer data
//
//-------------------------------------------------------------------------------------------------
bool USB_DeviceDriver::IsConfigured(void) const
{
    return (m_Configured == true) && (m_Suspended == false);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbBusReset
//
// Parameter(s):   void
// Return:         void
//
// Description:    Reset device and class state after a USB bus reset
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbBusReset(void)
{
    Reset();
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbSuspend
// Parameter(s):   void
// Return:         void
//
// Description:    Pause class processing while the USB bus is suspended
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbSuspend(void)
{
    m_Suspended = true;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbWakeup
// Parameter(s):   void
// Return:         void
//
// Description:    Resume class processing after USB bus wakeup
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbWakeup(void)
{
    m_Suspended = false;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbSetupPacket
// Parameter(s):   pSetup - eight-byte SETUP packet
// Return:         void
//
// Description:    Dispatch a new control request
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbSetupPacket(const uint8_t* pSetup)
{
    Setup(pSetup);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbControlOut
// Parameter(s):   pData - completed control packet
//                 Length - received byte count
// Return:         void
//
// Description:    Complete a control status stage or deliver class request data
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbControlOut(const uint8_t* pData, uint16_t Length)
{
    if(Length == 0)
    {
#if DIGINI_USB_DEVICE_CLASSES
        if(ControlReceivePending() == true)
        {
            HandleClassControlOut(pData, Length);
            return;
        }
#endif
        m_ControlRemaining = 0;
        m_ControlZLP = false;
        SetEndpointState(0, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_NAK);
    }
    else if(pData != nullptr)
    {
        HandleClassControlOut(pData, Length);
    }
    else
    {
        StallControl();
    }
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbEndpointRx
// Parameter(s):   Endpoint - completed OUT endpoint
// Return:         void
//
// Description:    Deliver a received packet to the device class
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbEndpointRx(uint8_t Endpoint)
{
    HandleClassEndpointRx(Endpoint);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbEndpointTx
// Parameter(s):   Endpoint - completed IN endpoint
// Return:         void
//
// Description:    Continue a control transfer or acknowledge a class packet
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbEndpointTx(uint8_t Endpoint)
{
    if(Endpoint == 0)
    {
        if((m_AddressPending == true) || (m_ControlRemaining != 0) || (m_ControlZLP == true))
        {
            ControlIn();
        }
#if DIGINI_USB_DEVICE_CLASSES
        else
        {
            ControlTransmitComplete();
        }
#endif
    }
    else
    {
        HandleClassEndpointTx(Endpoint);
    }
}

//-------------------------------------------------------------------------------------------------
//
// Function:       OnUsbInterruptComplete
// Parameter(s):   void
// Return:         void
//
// Description:    Service class queues after controller events have been dispatched
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::OnUsbInterruptComplete(void)
{
    Process();
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IRQHandler
// Parameter(s):   void
// Return:         void
//
// Description:    Dispatch the peripheral interrupt through the device backend
//
//-------------------------------------------------------------------------------------------------
void USB_DeviceDriver::IRQHandler(void)
{
#if DIGINI_USB_DEVICE_CLASSES
    m_InInterrupt = true;
#endif
    m_Backend.HandleInterrupt(*this);
#if DIGINI_USB_DEVICE_CLASSES
    m_InInterrupt = false;
#endif
}

#endif
