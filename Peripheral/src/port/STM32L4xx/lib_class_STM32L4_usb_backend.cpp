//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32L4_usb_backend.cpp
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

#define USB_DRIVER_GLOBAL
#include "lib_digini.h"
#undef USB_DRIVER_GLOBAL

#if (USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)

extern "C" void DebugTrap(int32_t IRQ_Number);

//-------------------------------------------------------------------------------------------------
//
//  Function:       STM32L4_USB_PMA_Backend
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Initialize the bridge without accessing the USB peripheral.
//
//-------------------------------------------------------------------------------------------------
STM32L4_USB_PMA_Backend::STM32L4_USB_PMA_Backend(void)
{
    m_pEventSink = nullptr;
    m_NextPMAAddress = 0xC0U;
    m_Address = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CheckResult
//
//  Parameter(s):   Result
//  Return:         None
//
//  Description:    Trap a programming or controller error in the void device-backend contract.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::CheckResult(SystemState_e Result) const
{
    if(Result != SYS_READY)
    {
        DebugTrap(USB_IRQn);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ReadUniqueId
//
//  Parameter(s):   pWords
//  Return:         None
//
//  Description:    Read the MCU identifier used by the USB serial-number descriptor.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::ReadUniqueId(uint32_t* pWords) const
{
    const volatile uint32_t* pId = reinterpret_cast<const volatile uint32_t*>(UID_BASE);
    for(uint8_t Index = 0; Index < 3U; Index++)
    {
        pWords[Index] = pId[Index];
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   None
//  Return:         bool - True when the controller and event callbacks are initialized
//
//  Description:    Power and reset USB, then initialize the Digini controller while disconnected.
//
//-------------------------------------------------------------------------------------------------
bool STM32L4_USB_PMA_Backend::Initialize(void)
{
    NVIC_DisableIRQ(USB_IRQn);
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    PWR->CR2 |= PWR_CR2_USV;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USBFSEN;
    (void)RCC->APB1ENR1;
    RCC->APB1RSTR1 |= RCC_APB1RSTR1_USBFSRST;
    RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_USBFSRST;
    LIB_Delay_uSec(10);
    USB_Info_t Info = {USB_ENDPOINT_COUNT, false, false};
    if(m_Controller.Initialize(&Info) != SYS_READY)
    {
        return false;
    }
    const USB_Callback_e Events[] =
    {
        USB_CALLBACK_RESET, USB_CALLBACK_SETUP, USB_CALLBACK_SUSPEND, USB_CALLBACK_RESUME
    };
    for(USB_Callback_e Event : Events)
    {
        if(m_Controller.RegisterCallback(Event, this) != SYS_READY)
        {
            return false;
        }
    }
    return (m_Controller.RegisterDataInStageCallback(this) == SYS_READY) &&
           (m_Controller.RegisterDataOutStageCallback(this) == SYS_READY);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Start
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the Digini USB IRQ and connect the controller to the host.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::Start(void)
{
    NVIC_ClearPendingIRQ(USB_IRQn);
    CheckResult(m_Controller.Start());
    ISR_Init(USB_IRQn, 5);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleInterrupt
//
//  Parameter(s):   EventSink
//  Return:         None
//
//  Description:    Dispatch controller callbacks to the device stack during this interrupt.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::HandleInterrupt(USB_DeviceEventSink& EventSink)
{
    m_pEventSink = &EventSink;
    m_Controller.IRQHandler();
    EventSink.OnUsbInterruptComplete();
    m_pEventSink = nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResetEndpoints
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Reset endpoint allocations and open both control-endpoint directions.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::ResetEndpoints(void)
{
    ResetNonControlEndpoints();
    CheckResult(m_Controller.CloseEndpoint(0x80));
    CheckResult(m_Controller.CloseEndpoint(0));
    CheckResult(m_Controller.ConfigurePMA(0x80, USB_SINGLE_BUFFER, 0x40));
    CheckResult(m_Controller.ConfigurePMA(0, USB_SINGLE_BUFFER, 0x80));
    CheckResult(m_Controller.OpenEndpoint(0x80, USB_CONTROL_PACKET_SIZE, USB_ENDPOINT_CONTROL));
    CheckResult(m_Controller.OpenEndpoint(0, USB_CONTROL_PACKET_SIZE, USB_ENDPOINT_CONTROL));
    EnableEndpointRx(0, USB_CONTROL_PACKET_SIZE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResetNonControlEndpoints
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Close data endpoints and reclaim their PMA after reset or configuration changes.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::ResetNonControlEndpoints(void)
{
    for(uint8_t Endpoint = 1; Endpoint < USB_ENDPOINT_COUNT; Endpoint++)
    {
        CheckResult(m_Controller.CloseEndpoint(Endpoint));
        CheckResult(m_Controller.CloseEndpoint(Endpoint | 0x80U));
    }
    m_NextPMAAddress = 0xC0U;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetAddress
//
//  Parameter(s):   Address
//  Return:         None
//
//  Description:    Apply the device stack's address when its control status stage completes.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::SetAddress(uint8_t Address)
{
    CheckResult(m_Controller.SetAddress(Address));
    m_Address = Address;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsAddressed
//
//  Parameter(s):   None
//  Return:         bool
//
//  Description:    Report whether the host has assigned a nonzero device address.
//
//-------------------------------------------------------------------------------------------------
bool STM32L4_USB_PMA_Backend::IsAddressed(void) const
{
    return m_Address != 0U;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetAddress
//
//  Parameter(s):   Endpoint, Direction
//  Return:         uint8_t
//
//  Description:    Convert a device-stack endpoint and direction to a controller endpoint address.
//
//-------------------------------------------------------------------------------------------------
uint8_t STM32L4_USB_PMA_Backend::GetAddress(uint8_t Endpoint, USB_DeviceDirection_e Direction) const
{
    return Endpoint | ((Direction == USB_DEVICE_DIRECTION_IN) ? 0x80U : 0U);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ConfigureEndpoint
//
//  Parameter(s):   Endpoint, Direction, Type, MaxPacketSize
//  Return:         None
//
//  Description:    Allocate one packet of PMA and open the endpoint through USB_Driver.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::ConfigureEndpoint(uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                              USB_DeviceEndpointType_e Type, uint16_t MaxPacketSize)
{
    if((Endpoint >= USB_ENDPOINT_COUNT) || (MaxPacketSize > USB_CONTROL_PACKET_SIZE))
    {
        CheckResult(SYS_WRONG_VALUE);
        return;
    }
    uint8_t Address = GetAddress(Endpoint, Direction);
    uint8_t EndpointType;
    switch(Type)
    {
        case USB_DEVICE_ENDPOINT_CONTROL:     EndpointType = USB_ENDPOINT_CONTROL; break;
        case USB_DEVICE_ENDPOINT_ISOCHRONOUS: EndpointType = USB_ENDPOINT_ISOCHRONOUS; break;
        case USB_DEVICE_ENDPOINT_BULK:        EndpointType = USB_ENDPOINT_BULK; break;
        case USB_DEVICE_ENDPOINT_INTERRUPT:   EndpointType = USB_ENDPOINT_INTERRUPT; break;
        default: CheckResult(SYS_WRONG_VALUE); return;
    }
    CheckResult(m_Controller.ConfigurePMA(Address, USB_SINGLE_BUFFER, m_NextPMAAddress));
    CheckResult(m_Controller.OpenEndpoint(Address, MaxPacketSize, EndpointType));
    m_NextPMAAddress += (MaxPacketSize + 1U) & ~1U;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetEndpointState
//
//  Parameter(s):   Endpoint, Direction, State
//  Return:         None
//
//  Description:    Translate endpoint state without bypassing the controller's register handling.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::SetEndpointState(uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                             USB_DeviceEndpointState_e State)
{
    uint16_t Status;
    switch(State)
    {
        case USB_DEVICE_ENDPOINT_DISABLED: Status = USB_EP_TX_DIS; break;
        case USB_DEVICE_ENDPOINT_STALL:    Status = USB_EP_TX_STALL; break;
        case USB_DEVICE_ENDPOINT_NAK:      Status = USB_EP_TX_NAK; break;
        case USB_DEVICE_ENDPOINT_READY:    Status = USB_EP_TX_VALID; break;
        default: CheckResult(SYS_WRONG_VALUE); return;
    }
    if(Direction == USB_DEVICE_DIRECTION_OUT)
    {
        Status <<= 8;
    }
    CheckResult(m_Controller.SetEndpointStatus(GetAddress(Endpoint, Direction), Status));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetStatus
//
//  Parameter(s):   Endpoint, Direction
//  Return:         uint16_t
//
//  Description:    Read the selected direction's status through USB_Driver.
//
//-------------------------------------------------------------------------------------------------
uint16_t STM32L4_USB_PMA_Backend::GetStatus(uint8_t Endpoint, USB_DeviceDirection_e Direction) const
{
    uint16_t Status;
    CheckResult(m_Controller.GetEndpointStatus(GetAddress(Endpoint, Direction), &Status));
    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearEndpointHalt
//
//  Parameter(s):   Endpoint, Direction
//  Return:         None
//
//  Description:    Clear halt and the data toggle, leaving reception paused for FIFO flow control.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::ClearEndpointHalt(uint8_t Endpoint, USB_DeviceDirection_e Direction)
{
    CheckResult(m_Controller.ClearStall(GetAddress(Endpoint, Direction)));
    SetEndpointState(Endpoint, Direction, USB_DEVICE_ENDPOINT_NAK);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointStalled
//
//  Parameter(s):   Endpoint, Direction
//  Return:         bool
//
//  Description:    Check whether the selected direction is stalled.
//
//-------------------------------------------------------------------------------------------------
bool STM32L4_USB_PMA_Backend::IsEndpointStalled(uint8_t Endpoint, USB_DeviceDirection_e Direction) const
{
    return GetStatus(Endpoint, Direction) ==
           ((Direction == USB_DEVICE_DIRECTION_IN) ? USB_EP_TX_STALL : USB_EP_RX_STALL);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointTxReady
//
//  Parameter(s):   Endpoint
//  Return:         bool
//
//  Description:    Check whether the IN direction can accept another packet.
//
//-------------------------------------------------------------------------------------------------
bool STM32L4_USB_PMA_Backend::IsEndpointTxReady(uint8_t Endpoint) const
{
    return GetStatus(Endpoint, USB_DEVICE_DIRECTION_IN) == USB_EP_TX_NAK;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointTxStalled
//
//  Parameter(s):   Endpoint
//  Return:         bool
//
//  Description:    Check the transmit halt state.
//
//-------------------------------------------------------------------------------------------------
bool STM32L4_USB_PMA_Backend::IsEndpointTxStalled(uint8_t Endpoint) const
{
    return IsEndpointStalled(Endpoint, USB_DEVICE_DIRECTION_IN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointRxBlocked
//
//  Parameter(s):   Endpoint
//  Return:         bool
//
//  Description:    Check whether receive flow control currently NAKs packets.
//
//-------------------------------------------------------------------------------------------------
bool STM32L4_USB_PMA_Backend::IsEndpointRxBlocked(uint8_t Endpoint) const
{
    return GetStatus(Endpoint, USB_DEVICE_DIRECTION_OUT) == USB_EP_RX_NAK;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableEndpointRx
//
//  Parameter(s):   Endpoint, Capacity
//  Return:         None
//
//  Description:    Arm one packet into bridge-owned storage that survives the interrupt.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::EnableEndpointRx(uint8_t Endpoint, uint16_t Capacity)
{
    if((Endpoint >= USB_ENDPOINT_COUNT) || (Capacity > USB_CONTROL_PACKET_SIZE))
    {
        CheckResult(SYS_WRONG_VALUE);
        return;
    }
    CheckResult(m_Controller.Receive(Endpoint, m_ReceiveBuffer[Endpoint], Capacity));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetEndpointRxLength
//
//  Parameter(s):   Endpoint
//  Return:         uint16_t
//
//  Description:    Return the length of the last completed receive packet.
//
//-------------------------------------------------------------------------------------------------
uint16_t STM32L4_USB_PMA_Backend::GetEndpointRxLength(uint8_t Endpoint) const
{
    uint32_t Count;
    CheckResult(m_Controller.GetReceiveCount(Endpoint, &Count));
    return uint16_t(Count);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ReadEndpointRx
//
//  Parameter(s):   Endpoint, pData, Length
//  Return:         None
//
//  Description:    Copy a completed packet from bridge storage to the CDC FIFO staging buffer.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::ReadEndpointRx(uint8_t Endpoint, uint8_t* pData, uint16_t Length)
{
    if((Endpoint >= USB_ENDPOINT_COUNT) || (Length > USB_CONTROL_PACKET_SIZE) || (pData == nullptr))
    {
        CheckResult(SYS_WRONG_VALUE);
        return;
    }
    memcpy(pData, m_ReceiveBuffer[Endpoint], Length);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Transmit
//
//  Parameter(s):   Endpoint, pData, Length
//  Return:         None
//
//  Description:    Retain a stack-generated packet until the controller reports its completion.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::Transmit(uint8_t Endpoint, const uint8_t* pData, uint16_t Length)
{
    if((Endpoint >= USB_ENDPOINT_COUNT) || (Length > USB_CONTROL_PACKET_SIZE) ||
       ((Length != 0U) && (pData == nullptr)))
    {
        CheckResult(SYS_WRONG_VALUE);
        return;
    }
    if(Length != 0U)
    {
        memcpy(m_TransmitBuffer[Endpoint], pData, Length);
    }
    CheckResult(m_Controller.Transmit(Endpoint | 0x80U, m_TransmitBuffer[Endpoint], Length));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CallbackFunction
//
//  Parameter(s):   Type, pContext
//  Return:         None
//
//  Description:    Translate controller completions to device-stack events and rearm control OUT.
//
//-------------------------------------------------------------------------------------------------
void STM32L4_USB_PMA_Backend::CallbackFunction(int Type, void* pContext)
{
    if(m_pEventSink == nullptr)
    {
        CheckResult(SYS_DEVICE_ERROR);
        return;
    }
    const USB_CallbackInfo_t* pInfo = static_cast<const USB_CallbackInfo_t*>(pContext);
    uint8_t Endpoint = uint8_t(pInfo->Value);
    switch(Type)
    {
        case USB_CALLBACK_RESET:
            m_pEventSink->OnUsbBusReset();
            break;

        case USB_CALLBACK_SUSPEND:
            m_pEventSink->OnUsbSuspend();
            break;

        case USB_CALLBACK_RESUME:
            m_pEventSink->OnUsbWakeup();
            break;

        case USB_CALLBACK_SETUP:
            CheckResult(m_Controller.Abort(0x80));
            SetEndpointState(0, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_NAK);
            SetEndpointState(0, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_NAK);
            m_pEventSink->OnUsbSetupPacket(m_Controller.GetSetup());
            break;

        case USB_CALLBACK_DATA_IN:
            m_pEventSink->OnUsbEndpointTx(Endpoint);
            break;

        case USB_CALLBACK_DATA_OUT:
            if(Endpoint == 0U)
            {
                m_pEventSink->OnUsbControlOut(m_ReceiveBuffer[0], GetEndpointRxLength(0));
            }
            else
            {
                m_pEventSink->OnUsbEndpointRx(Endpoint);
            }
            break;

        default:
            CheckResult(SYS_WRONG_VALUE);
            return;
    }
    if(((Type == USB_CALLBACK_SETUP) || ((Type == USB_CALLBACK_DATA_OUT) && (Endpoint == 0U))) &&
       (IsEndpointStalled(0, USB_DEVICE_DIRECTION_OUT) == false))
    {
        EnableEndpointRx(0, USB_CONTROL_PACKET_SIZE);
    }
}

#endif // (USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)
