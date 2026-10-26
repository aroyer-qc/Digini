//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_cdc.cpp
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

#include "../../lib_digini.h"

#if (USE_USB_DRIVER == DEF_ENABLED) && (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define USB_CDC_DEFAULT_BAUD_RATE           115200UL
#define USB_CDC_STOP_BITS_1                 0
#define USB_CDC_PARITY_NONE                 0
#define USB_CDC_DATA_BITS_8                 8
#define USB_CDC_VID                         0x0483U
#define USB_CDC_PID                         0x5740U


//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

const uint8_t USB_CDC_ACM_Driver::m_DefaultLineCoding[USB_CDC_LINE_CODING_SIZE] =
{
    static_cast<uint8_t>(USB_CDC_DEFAULT_BAUD_RATE),
    static_cast<uint8_t>(USB_CDC_DEFAULT_BAUD_RATE >> 8),
    static_cast<uint8_t>(USB_CDC_DEFAULT_BAUD_RATE >> 16),
    static_cast<uint8_t>(USB_CDC_DEFAULT_BAUD_RATE >> 24),
    USB_CDC_STOP_BITS_1,
    USB_CDC_PARITY_NONE,
    USB_CDC_DATA_BITS_8
};

//-------------------------------------------------------------------------------------------------
//
// Function:       USB_CDC_ACM_Driver
// Parameter(s):   Backend - device controller backend
// Return:         void
// Description:    Preserve the existing application identity and public constructor.
//
//-------------------------------------------------------------------------------------------------
USB_CDC_ACM_Driver::USB_CDC_ACM_Driver(USB_DeviceBackend& Backend)
    : USB_CDC_ACM_Driver(Backend, USB_DeviceIdentity_t{USB_CDC_VID, USB_CDC_PID,
                        OUR_FIRMWARE_DIGINI_NAME, OUR_FIRMWARE_NAME})
{
}

//-------------------------------------------------------------------------------------------------
//
// Function:       USB_CDC_ACM_Driver
// Parameter(s):   Backend - device controller backend
// Return:         void
//
// Description:    Initialize CDC ACM class state
//
//-------------------------------------------------------------------------------------------------
USB_CDC_ACM_Driver::USB_CDC_ACM_Driver(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity)
    : USB_ClassBase(Backend, Identity)
{
    m_TxBusy                     = false;
    m_ControlLines               = 0;
    m_TxZLP                      = false;
    m_TxPacketLength             = 0;
    memset(m_PendingLineCoding, 0, sizeof(m_PendingLineCoding));
    m_pReceiveBuffer             = nullptr;
    m_ReceiveBufferCapacity      = 0;
    m_ReceiveBufferHead          = 0;
    m_ReceiveTransfer.pBuffer    = nullptr;
    m_ReceiveTransfer.u.Head     = 0;
    m_ReceiveTransfer.StaticSize = 0;
    m_pReceiveCallback           = nullptr;
    m_ReceiveCallbackTypes       = 0;

    memcpy(m_LineCoding, m_DefaultLineCoding, sizeof(m_LineCoding));
}

//-------------------------------------------------------------------------------------------------
//
// Function:       InitializeClass
// Parameter(s):   void
// Return:         bool - true when both CDC FIFOs have storage
//
// Description:    Allocate CDC buffers from the memory pool before enabling USB interrupts
//
//-------------------------------------------------------------------------------------------------
bool USB_CDC_ACM_Driver::InitializeClass(void)
{
    if(USB_ClassBase::InitializeClass() == false)
    {
        return false;
    }

    if(m_RxFifo.GetBufferPointer() == nullptr)
    {

        if(pMemoryPool->IsAvailable(USB_CDC_BUFFER_SIZE) == false)
        {
            return false;
        }

        m_RxFifo.Initialize(USB_CDC_BUFFER_SIZE);
    }

    if(m_TxFifo.GetBufferPointer() == nullptr)
    {

        if(pMemoryPool->IsAvailable(USB_CDC_BUFFER_SIZE) == false)
        {
            return false;
        }

        m_TxFifo.Initialize(USB_CDC_BUFFER_SIZE);
    }

    return (m_RxFifo.GetBufferPointer() != nullptr) && (m_TxFifo.GetBufferPointer() != nullptr);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       InitializeTransport
// Parameter(s):   void
// Return:         void
//
// Description:    Initialize the USB-backed serial transport
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::InitializeTransport(void)
{
    (void)Initialize();
}

//-------------------------------------------------------------------------------------------------
//
// Function:       SetReceiveBuffer
// Parameter(s):   pBuffer - circular receive buffer
//                 Capacity - buffer capacity
// Return:         void
//
// Description:    Attach the Console receive buffer to the CDC packet stream
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::SetReceiveBuffer(uint8_t* pBuffer, size_t Capacity)
{
    m_pReceiveBuffer          = pBuffer;
    m_ReceiveBufferCapacity   = Capacity;
    m_ReceiveBufferHead       = 0;
    m_ReceiveTransfer.pBuffer = pBuffer;
    m_ReceiveTransfer.u.Head  = 0;
    m_ReceiveTransfer.StaticSize = Capacity;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       SendData
// Parameter(s):   pData - data to send
//                 pSize - requested size and accepted size on return
// Return:         SystemState_e - ready, busy, or device error
//
// Description:    Queue serial data through the CDC ACM transmit ring
//
//-------------------------------------------------------------------------------------------------
SystemState_e USB_CDC_ACM_Driver::SendData(const uint8_t* pData, size_t* pSize)
{
    if((pSize == nullptr) || ((*pSize != 0) && (pData == nullptr)))
    {
        return SYS_WRONG_VALUE;
    }

    size_t Requested = *pSize;
    size_t Accepted = 0;
    const size_t MaxChunk = static_cast<size_t>(UINT16_MAX);

    while(Accepted < Requested)
    {
        size_t Remaining = Requested - Accepted;
        uint16_t Chunk = static_cast<uint16_t>((Remaining > MaxChunk) ? MaxChunk : Remaining);
        uint16_t Written = Write(pData + Accepted, Chunk);
        Accepted += Written;

        if(Written < Chunk)
        {
            break;
        }
    }

    *pSize = Accepted;

    if(Accepted == Requested)
    {
        return SYS_READY;
    }

    if(IsConfigured() == true)
    {
        return SYS_BUSY;
    }

    return SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       RegisterReceiveCallback
// Parameter(s):   pCallback - serial receive callback
// Return:         void
//
// Description:    Register the Console callback for CDC receive events
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::RegisterReceiveCallback(CallbackInterface* pCallback)
{
    m_pReceiveCallback = pCallback;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       EnableReceiveCallbacks
// Parameter(s):   CallbackTypes - enabled serial callback types
// Return:         void
//
// Description:    Select CDC receive events to forward to the registered callback
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::EnableReceiveCallbacks(int CallbackTypes)
{
    m_ReceiveCallbackTypes = CallbackTypes;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ResetClassState
// Parameter(s):   void
// Return:         void
//
// Description:    Reset CDC buffers and class-specific transfer state
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::ResetClassState(void)
{
    m_TxBusy = false;
    m_TxZLP = false;
    m_TxPacketLength = 0;
    m_RxFifo.Flush(m_RxFifo.CheckUsedSpace());
    m_TxFifo.Flush(m_TxFifo.CheckUsedSpace());
    m_ControlLines = 0;
    USB_ClassBase::CancelClassControl();
    m_ReceiveBufferHead = 0;
    m_ReceiveTransfer.u.Head = 0;

    if((m_pReceiveCallback != nullptr) &&
       ((m_ReceiveCallbackTypes & SERIAL_CALLBACK_RX_IDLE) != 0))
    {
        m_pReceiveCallback->CallbackFunction(SERIAL_CALLBACK_RX_IDLE, &m_ReceiveTransfer);
    }
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ConfigureClassEndpoints
// Parameter(s):   void
// Return:         void
//
// Description:    Configure the CDC notification and bulk endpoints
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::ConfigureClassEndpoints(void)
{
    ConfigureEndpoint(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_INTERRUPT, 16);
    SetEndpointState(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_NAK);
    ConfigureEndpoint(2, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_BULK, USB_CDC_PACKET_SIZE);
    ConfigureEndpoint(2, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_BULK, USB_CDC_PACKET_SIZE);
    EnableEndpointRx(2, USB_CDC_PACKET_SIZE);
    SetEndpointState(2, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_NAK);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       GetConfiguration
// Parameter(s):   pLength - configuration descriptor length
// Return:         const uint8_t* - persistent descriptor
// Description:    Supply the two-interface CDC ACM configuration to the common class base.
//
//-------------------------------------------------------------------------------------------------
const uint8_t* USB_CDC_ACM_Driver::GetConfiguration(uint16_t* pLength) const
{
    static const uint8_t Configuration[] =
    {
        9, 2, 75, 0, 2, 1, 0, 0x80, 50,
        8, 11, 0, 2, 2, 2, 1, 0,
        9, 4, 0, 0, 1, 2, 2, 1, 0,
        5, 0x24, 0, 0x10, 0x01,
        5, 0x24, 1, 0, 1,
        4, 0x24, 2, 2,
        5, 0x24, 6, 0, 1,
        7, 5, 0x81, 3, 16, 0, 16,
        9, 4, 1, 0, 2, 0x0A, 0, 0, 0,
        7, 5, 0x02, 2, 64, 0, 0,
        7, 5, 0x82, 2, 64, 0, 0
    };
    static_assert(sizeof(Configuration) == 75, "Invalid CDC configuration length");

    *pLength = sizeof(Configuration);
    return Configuration;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       GetDeviceClass
// Parameter(s):   pClass, pSubClass, pProtocol
// Return:         void
// Description:    Preserve the CDC IAD device class triplet in the shared device descriptor.
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::GetDeviceClass(uint8_t* pClass, uint8_t* pSubClass, uint8_t* pProtocol) const
{
    *pClass = 0xEF;
    *pSubClass = 2;
    *pProtocol = 1;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       HandleClassSetup
// Parameter(s):   Type - request type
//                 Request - request code
//                 Value - request value
//                 Index - interface index
//                 Length - request data length
// Return:         bool - true when the CDC request is supported
//
// Description:    Handle CDC ACM class-specific control requests
//
//-------------------------------------------------------------------------------------------------
bool USB_CDC_ACM_Driver::HandleClassSetup(uint8_t Type, uint8_t Request, uint16_t Value,
                                          uint16_t Index, uint16_t Length)
{
    if((Index == 0) && (m_Configured == true) && (Value == 0) &&
       (Type == 0xA1) && (Request == 0x21) && (Length == 7))
    {
        memcpy(m_ControlBuffer, m_LineCoding, sizeof(m_LineCoding));
        ControlSend(m_ControlBuffer, 7, Length);
        return true;
    }
    else if((Index == 0) && (m_Configured == true) && (Value == 0) &&
            (Type == 0x21) && (Request == 0x20) && (Length == 7))
    {
        return BeginControlReceive(m_PendingLineCoding, Length, sizeof(m_PendingLineCoding));
    }
    else if((Index == 0) && (m_Configured == true) && (Type == 0x21) &&
            (Request == 0x22) && (Length == 0) && ((Value & ~3U) == 0))
    {
        m_ControlLines = Value;
        uint8_t Notification[10] = { 0xA1, 0x20, 0, 0, 0, 0, 2, 0,
            static_cast<uint8_t>((Value & 1) ? 3 : 0), 0 };

        if(IsEndpointTxReady(1) == true)
        {
            Transmit(1, Notification, sizeof(Notification));
        }

        ControlSend(nullptr, 0, 0);
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ControlReceiveComplete
// Parameter(s):   Length - complete line coding payload length
// Return:         void
// Description:    Validate the staged payload before changing the active CDC line coding.
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::ControlReceiveComplete(uint16_t Length)
{
    const uint8_t* pData = m_PendingLineCoding;
    if((Length != USB_CDC_LINE_CODING_SIZE) || (pData[4] > 2) || (pData[5] > 4) ||
       !(((pData[6] >= 5) && (pData[6] <= 8)) || (pData[6] == 16)) ||
       ((pData[0] | pData[1] | pData[2] | pData[3]) == 0))
    {
        StallControl();
        return;
    }
    memcpy(m_LineCoding, pData, sizeof(m_LineCoding));
    ControlSend(nullptr, 0, 0);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       StartTransmit
//
// Parameter(s):   void
// Return:         void
//
// Description:    Start the next queued CDC packet when the transmit endpoint is available
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::StartTransmit(void)
{

     if((m_Configured == false) || (m_Suspended == true) || (m_TxBusy == true) ||
         (IsEndpointTxStalled(2) == true))
    {
        return;
    }

    uint16_t Length = static_cast<uint16_t>(m_TxFifo.CheckUsedSpace());

    if(Length == 0)
    {

        if(m_TxZLP == true)
        {
            m_TxZLP = false;
            m_TxPacketLength = 0;
            m_TxBusy = true;
            Transmit(2, nullptr, 0);
        }
        return;
    }

    if(Length > USB_CDC_PACKET_SIZE)
    {
        Length = USB_CDC_PACKET_SIZE;
    }

    uint8_t Packet[USB_CDC_PACKET_SIZE];

    for(uint16_t Position = 0; Position < Length; ++Position)
    {
        Packet[Position] = static_cast<uint8_t>(m_TxFifo.At(Position));
    }

    m_TxZLP = Length == USB_CDC_PACKET_SIZE;
    m_TxPacketLength = Length;
    m_TxBusy = true;
    Transmit(2, Packet, Length);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Receive
//
// Parameter(s):   void
// Return:         void
//
// Description:    Receive one bulk packet into the RX ring buffer
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::Receive(void)
{
    uint16_t Length = GetEndpointRxLength(2);
    size_t Free = m_RxFifo.CheckFreeSpace();
    bool InternalBufferHasSpace = Length <= Free;
    bool ExternalBufferAttached = (m_pReceiveBuffer != nullptr) && (m_ReceiveBufferCapacity != 0);

    if((Length <= USB_CDC_PACKET_SIZE) &&
       ((InternalBufferHasSpace == true) || (ExternalBufferAttached == true)))
    {
        uint8_t Packet[USB_CDC_PACKET_SIZE];
        ReadEndpointRx(2, Packet, Length);

        if(InternalBufferHasSpace == true)
        {
            m_RxFifo.Write(Packet, Length);
        }

        for(uint16_t Position = 0; Position < Length; ++Position)
        {
            if(ExternalBufferAttached == true)
            {
                m_pReceiveBuffer[m_ReceiveBufferHead] = Packet[Position];
                m_ReceiveBufferHead++;

                if(m_ReceiveBufferHead == m_ReceiveBufferCapacity)
                {
                    m_ReceiveBufferHead = 0;
                }
            }
        }

        if((m_pReceiveCallback != nullptr) && (m_pReceiveBuffer != nullptr) &&
           (m_ReceiveBufferCapacity != 0))
        {
            m_ReceiveTransfer.u.Head = m_ReceiveBufferHead;

            if((m_ReceiveCallbackTypes & SERIAL_CALLBACK_RX_IDLE) != 0)
            {
                m_pReceiveCallback->CallbackFunction(SERIAL_CALLBACK_RX_IDLE, &m_ReceiveTransfer);
            }
            else if((m_ReceiveCallbackTypes & SERIAL_CALLBACK_RX_NOT_EMPTY) != 0)
            {
                m_pReceiveCallback->CallbackFunction(SERIAL_CALLBACK_RX_NOT_EMPTY, &m_ReceiveTransfer);
            }
        }
    }

    Process();
}

//-------------------------------------------------------------------------------------------------
//
// Function:       ProcessClass
// Parameter(s):   void
// Return:         void
//
// Description:    Service CDC receive flow control and pending transmissions
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::ProcessClass(void)
{
    bool InternalBufferHasSpace = m_RxFifo.CheckFreeSpace() >= USB_CDC_PACKET_SIZE;
    bool ExternalBufferAttached = (m_pReceiveBuffer != nullptr) && (m_ReceiveBufferCapacity != 0);

    if((m_Configured == true) && (m_Suspended == false) &&
       (IsEndpointRxBlocked(2) == true) &&
       ((InternalBufferHasSpace == true) || (ExternalBufferAttached == true)))
    {
        EnableEndpointRx(2, USB_CDC_PACKET_SIZE);
    }

    StartTransmit();
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IsEndpointAvailable
// Parameter(s):   Endpoint - endpoint number
//                 In - true for an IN endpoint
// Return:         bool - true when the endpoint direction is used by CDC
//
// Description:    Validate an endpoint address against the CDC endpoint map
//
//-------------------------------------------------------------------------------------------------
bool USB_CDC_ACM_Driver::IsEndpointAvailable(uint8_t Endpoint, bool In) const
{
    return ((Endpoint == 1) && (In == true)) || (Endpoint == 2);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       HandleEndpointHaltCleared
// Parameter(s):   Endpoint - endpoint number
// Return:         void
//
// Description:    Resume CDC transmission when the host clears a bulk endpoint halt
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::HandleEndpointHaltCleared(uint8_t Endpoint)
{
    if(Endpoint == 2)
    {
        StartTransmit();
    }
}

//-------------------------------------------------------------------------------------------------
//
// Function:       HandleClassEndpointRx
// Parameter(s):   Endpoint - endpoint number
// Return:         void
//
// Description:    Dispatch received CDC endpoint data
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::HandleClassEndpointRx(uint8_t Endpoint)
{
    if((Endpoint == 2) && (m_Configured == true))
    {
        Receive();
    }
}

//-------------------------------------------------------------------------------------------------
//
// Function:       HandleClassEndpointTx
// Parameter(s):   Endpoint - endpoint number
// Return:         void
//
// Description:    Complete a CDC bulk IN packet and schedule the next one
//
//-------------------------------------------------------------------------------------------------
void USB_CDC_ACM_Driver::HandleClassEndpointTx(uint8_t Endpoint)
{
    if(Endpoint == 2)
    {
        m_TxFifo.Flush(m_TxPacketLength);
        m_TxPacketLength = 0;
        m_TxBusy = false;
        StartTransmit();
    }
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IsDTR
//
// Parameter(s):   void
// Return:         bool - true when the host asserts DTR
//
// Description:    Report the current CDC DTR control-line state
//
//-------------------------------------------------------------------------------------------------
bool USB_CDC_ACM_Driver::IsDTR(void) const
{
    return (IsConfigured() == true) && ((m_ControlLines & 1) != 0);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       IsRTS
//
// Parameter(s):   void
// Return:         bool - true when the host asserts RTS
//
// Description:    Report the current CDC RTS control-line state
//
//-------------------------------------------------------------------------------------------------
bool USB_CDC_ACM_Driver::IsRTS(void) const
{
    return (IsConfigured() == true) && ((m_ControlLines & 2) != 0);
}

//-------------------------------------------------------------------------------------------------
//
// Function:       GetBaudRate
//
// Parameter(s):   void
// Return:         uint32_t - host-requested baud rate
//
// Description:    Read the baud rate from the most recently accepted line coding
//
//-------------------------------------------------------------------------------------------------
uint32_t USB_CDC_ACM_Driver::GetBaudRate(void) const
{
    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    uint32_t Baud = m_LineCoding[0] | (static_cast<uint32_t>(m_LineCoding[1]) << 8) |
        (static_cast<uint32_t>(m_LineCoding[2]) << 16) | (static_cast<uint32_t>(m_LineCoding[3]) << 24);
    __set_PRIMASK(InterruptState);
    return Baud;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Available
//
// Parameter(s):   void
// Return:         uint16_t - number of bytes waiting in the RX buffer
//
// Description:    Return the number of buffered receive bytes
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_CDC_ACM_Driver::Available(void) const
{
    if(m_RxFifo.GetBufferPointer() == nullptr)
    {
        return 0;
    }

    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    uint16_t Count = static_cast<uint16_t>(m_RxFifo.CheckUsedSpace());
    __set_PRIMASK(InterruptState);
    return Count;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Read
//
// Parameter(s):   pData - destination buffer (cannot be nullptr)
//                 Length - maximum number of bytes to read
// Return:         uint16_t - number of bytes copied
//
// Description:    Read buffered CDC data and resume reception when space is available
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_CDC_ACM_Driver::Read(uint8_t* pData, uint16_t Length)
{
    if(pData == nullptr)
    {
        return 0;
    }

    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    uint16_t Count = static_cast<uint16_t>(m_RxFifo.Read(pData, Length));

    Process();
    __set_PRIMASK(InterruptState);
    return Count;
}

//-------------------------------------------------------------------------------------------------
//
// Function:       Write
//
// Parameter(s):   pData - source buffer
//                 Length - number of bytes to queue
// Return:         uint16_t - number of bytes accepted
//
// Description:    Queue CDC data for transmission to the host
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_CDC_ACM_Driver::Write(const uint8_t* pData, uint16_t Length)
{
    if(pData == nullptr)
    {
        return 0;
    }

    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    uint16_t Count = 0;

    if(IsConfigured() == true)
    {
        Count = static_cast<uint16_t>(m_TxFifo.Write(pData, Length));

        StartTransmit();
    }

    __set_PRIMASK(InterruptState);
    return Count;
}

#endif // USB CDC driver
