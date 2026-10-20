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

#define USB_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  USB_DRIVER_GLOBAL

#if (USE_USB_DRIVER == DEF_ENABLED)

#if (CFG_ENABLE_PLL_Q == 0) || (CFG_PLL_Q_FREQUENCY != 48000000ULL)
#error USB requires a 48 MHz PLLQ clock
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define USB_REGS reinterpret_cast<USB_TypeDef*>(USB_BASE)
#define USB_ENDPOINT(Number) (*reinterpret_cast<volatile uint16_t*>(USB_BASE + 4U * (Number)))
#define USB_TABLE reinterpret_cast<volatile uint16_t*>(USB_PMAADDR)

#define USB_CDC_DEFAULT_BAUD_RATE        115200UL
#define USB_CDC_STOP_BITS_1              0
#define USB_CDC_PARITY_NONE              0
#define USB_CDC_DATA_BITS_8              8

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           USB_Driver
// Parameter(s):   void
// Return:         void
//
// Description:    Initialize generic USB port state
//
//-------------------------------------------------------------------------------------------------
USB_Driver::USB_Driver(void)
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
}

//-------------------------------------------------------------------------------------------------
//
// Name:           USB_CDC_ACM_Driver
// Parameter(s):   void
// Return:         void
//
// Description:    Initialize CDC ACM class state
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
USB_CDC_ACM_Driver::USB_CDC_ACM_Driver(void)
{
    m_TxBusy                     = false;
    m_ControlLines               = 0;
    m_TxZLP                      = false;
    m_TxPacketLength             = 0;
    m_ExpectLineCoding           = false;
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           InitializeClass
// Parameter(s):   void
// Return:         bool - true when both CDC FIFOs have storage
//
// Description:    Allocate CDC buffers from the memory pool before enabling USB interrupts
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
bool USB_CDC_ACM_Driver::InitializeClass(void)
{

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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           InitializeTransport
// Parameter(s):   void
// Return:         void
//
// Description:    Initialize the USB-backed serial transport
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::InitializeTransport(void)
{
    (void)Initialize();
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           SetReceiveBuffer
// Parameter(s):   pBuffer - circular receive buffer
//                 Capacity - buffer capacity
// Return:         void
//
// Description:    Attach the Console receive buffer to the CDC packet stream
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::SetReceiveBuffer(uint8_t* pBuffer, size_t Capacity)
{
    m_pReceiveBuffer          = pBuffer;
    m_ReceiveBufferCapacity   = Capacity;
    m_ReceiveBufferHead       = 0;
    m_ReceiveTransfer.pBuffer = pBuffer;
    m_ReceiveTransfer.u.Head  = 0;
    m_ReceiveTransfer.StaticSize = Capacity;
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           SendData
// Parameter(s):   pData - data to send
//                 pSize - requested size and accepted size on return
// Return:         SystemState_e - ready, busy, or device error
//
// Description:    Queue serial data through the CDC ACM transmit ring
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           RegisterReceiveCallback
// Parameter(s):   pCallback - serial receive callback
// Return:         void
//
// Description:    Register the Console callback for CDC receive events
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::RegisterReceiveCallback(CallbackInterface* pCallback)
{
    m_pReceiveCallback = pCallback;
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           EnableReceiveCallbacks
// Parameter(s):   CallbackTypes - enabled serial callback types
// Return:         void
//
// Description:    Select CDC receive events to forward to the registered callback
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::EnableReceiveCallbacks(int CallbackTypes)
{
    m_ReceiveCallbackTypes = CallbackTypes;
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Initialize
//
// Parameter(s):   void
// Return:         bool - true when the USB device is initialized
//
// Description:    Initialize the USB peripheral and enable its interrupt
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::Initialize(void)
{
    if(m_Initialized == true)
    {
        return true;
    }

    if(((RCC->CR & RCC_CR_PLLRDY) == 0) || ((RCC->PLLCFGR & RCC_PLLCFGR_PLLQEN) == 0))
    {
        return false;
    }

    if(InitializeClass() == false)
    {
        return false;
    }

    RCC->APB1ENR1  |= RCC_APB1ENR1_PWREN;
    PWR->CR2       |= PWR_CR2_USV;
    RCC->CCIPR      = (RCC->CCIPR & ~RCC_CCIPR_CLK48SEL) | RCC_CCIPR_CLK48SEL_1;
    RCC->APB1ENR1  |= RCC_APB1ENR1_USBFSEN;
    (void)RCC->APB1ENR1;
    RCC->APB1RSTR1 |= RCC_APB1RSTR1_USBFSRST;
    RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_USBFSRST;
    USB_REGS->BCDR = 0;
    USB_REGS->CNTR = USB_CNTR_FRES;
    LIB_Delay_uSec(10);
    USB_REGS->CNTR = 0;
    USB_REGS->ISTR = 0;
    Reset();
    USB_REGS->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_SUSPM | USB_CNTR_WKUPM;
    NVIC_ClearPendingIRQ(USB_IRQn);
    NVIC_SetPriority(USB_IRQn, 5);
    m_Initialized = true;
    NVIC_EnableIRQ(USB_IRQn);
    USB_REGS->BCDR = USB_BCDR_DPPU;
    return true;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           SetEndpointStatus
//
// Parameter(s):   Endpoint - USB endpoint number
//                 Mask - endpoint status fields to update
//                 Status - requested endpoint status
// Return:         void
//
// Description:    Update endpoint status fields while preserving other register bits
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::SetEndpointStatus(uint8_t Endpoint, uint16_t Mask, uint16_t Status)
{
    uint16_t Value = USB_ENDPOINT(Endpoint);
    USB_ENDPOINT(Endpoint) = (Value & (USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD)) | USB_EP_CTR_RX | USB_EP_CTR_TX | ((Value ^ Status) & Mask);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ClearEndpointEvent
//
// Parameter(s):   Endpoint - USB endpoint number
//                 Event - endpoint event flags to clear
// Return:         void
//
// Description:    Clear selected endpoint events without changing its status fields
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ClearEndpointEvent(uint8_t Endpoint, uint16_t Event)
{
    USB_ENDPOINT(Endpoint) = (USB_ENDPOINT(Endpoint) & (USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD)) | ((USB_EP_CTR_RX | USB_EP_CTR_TX) & ~Event);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ConfigureEndpoint
// Parameter(s):   Endpoint - endpoint number
//                 Type - endpoint transfer type
//                 TxAddress - PMA transmit buffer address
//                 RxAddress - PMA receive buffer address
// Return:         void
//
// Description:    Configure an endpoint and its packet memory buffer addresses
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ConfigureEndpoint(uint8_t Endpoint, uint16_t Type, uint16_t TxAddress, uint16_t RxAddress)
{
    USB_TABLE[Endpoint * 4] = TxAddress;
    USB_TABLE[Endpoint * 4 + 1] = 0;
    USB_TABLE[Endpoint * 4 + 2] = RxAddress;
    USB_TABLE[Endpoint * 4 + 3] = 0;
    USB_ENDPOINT(Endpoint) = Type | Endpoint;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           IsEndpointTxReady
// Parameter(s):   Endpoint - endpoint number
// Return:         bool - true when the endpoint can accept a transmit packet
//
// Description:    Check whether an endpoint transmit status is NAK
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::IsEndpointTxReady(uint8_t Endpoint) const
{
    return (USB_ENDPOINT(Endpoint) & USB_EPTX_STAT) == USB_EP_TX_NAK;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           IsEndpointTxStalled
// Parameter(s):   Endpoint - endpoint number
// Return:         bool - true when the endpoint transmit direction is stalled
//
// Description:    Check whether the endpoint transmit direction is stalled
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::IsEndpointTxStalled(uint8_t Endpoint) const
{
    return (USB_ENDPOINT(Endpoint) & USB_EPTX_STAT) == USB_EP_TX_STALL;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           IsEndpointRxBlocked
// Parameter(s):   Endpoint - endpoint number
// Return:         bool - true when the endpoint receive direction is NAK
//
// Description:    Check whether the endpoint receive direction is blocked
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::IsEndpointRxBlocked(uint8_t Endpoint) const
{
    return (USB_ENDPOINT(Endpoint) & USB_EPRX_STAT) == USB_EP_RX_NAK;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           EnableEndpointRx
// Parameter(s):   Endpoint - endpoint number
//                 Capacity - receive buffer capacity in bytes
// Return:         void
//
// Description:    Set the PMA receive capacity and enable endpoint reception
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::EnableEndpointRx(uint8_t Endpoint, uint16_t Capacity)
{
    uint16_t Allocation = 0;

    if(Capacity > 62)
    {
        uint16_t Blocks = static_cast<uint16_t>((Capacity + 31) / 32);
        Allocation = static_cast<uint16_t>(0x8000 | ((Blocks - 1) << 10));
    }
    else
    {
        Allocation = static_cast<uint16_t>((Capacity + 1) / 2);
    }

    USB_TABLE[Endpoint * 4 + 3] = Allocation;
    SetEndpointStatus(Endpoint, USB_EPRX_STAT, USB_EP_RX_VALID);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetEndpointRxLength
// Parameter(s):   Endpoint - endpoint number
// Return:         uint16_t - received packet length
//
// Description:    Read the received byte count from endpoint packet memory
//
//-------------------------------------------------------------------------------------------------
uint16_t USB_Driver::GetEndpointRxLength(uint8_t Endpoint) const
{
    return USB_TABLE[Endpoint * 4 + 3] & 0x03FF;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ReadEndpointRx
// Parameter(s):   Endpoint - endpoint number
//                 pData - destination buffer
//                 Length - number of bytes to read
// Return:         void
//
// Description:    Copy an endpoint receive packet from PMA to a class buffer
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ReadEndpointRx(uint8_t Endpoint, uint8_t* pData, uint16_t Length)
{
    ReadPMA(USB_TABLE[Endpoint * 4 + 2], pData, Length);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           WritePMA
//
// Parameter(s):   Address - packet memory address
//                 pData - source data
//                 Length - number of bytes to write
// Return:         void
//
// Description:    Copy data to USB packet memory using 16-bit accesses
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::WritePMA(uint16_t Address, const uint8_t* pData, uint16_t Length)
{
    volatile uint16_t* pMemory = reinterpret_cast<volatile uint16_t*>(USB_PMAADDR + Address);
    
    for(uint16_t Offset = 0; Offset < Length; Offset += 2)
    {
        uint16_t Value = pData[Offset];

        if((Offset + 1) < Length)
        {
            Value |= static_cast<uint16_t>(pData[Offset + 1]) << 8;
        }

        *pMemory++ = Value;
    }
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ReadPMA
//
// Parameter(s):   Address - packet memory address
//                 pData   - destination buffer
//                 Length  - number of bytes to read
// Return:         void
//
// Description:    Copy data from USB packet memory using 16-bit accesses
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ReadPMA(uint16_t Address, uint8_t* pData, uint16_t Length)
{
    volatile uint16_t* pMemory = reinterpret_cast<volatile uint16_t*>(USB_PMAADDR + Address);
    
    for(uint16_t Offset = 0; Offset < Length; Offset += 2)
    {
        uint16_t Value = *pMemory++;
        pData[Offset] = static_cast<uint8_t>(Value);
        
        if((Offset + 1) < Length)
        {
            pData[Offset + 1] = static_cast<uint8_t>(Value >> 8);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
// Name:           Transmit
//
// Parameter(s):   Endpoint - USB endpoint number
//                 pData - source data
//                 Length - number of bytes to transmit
// Return:         void
//
// Description:    Write endpoint data to packet memory and start transmission
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::Transmit(uint8_t Endpoint, const uint8_t* pData, uint16_t Length)
{
    WritePMA(USB_TABLE[Endpoint * 4], pData, Length);
    USB_TABLE[Endpoint * 4 + 1] = Length;
    SetEndpointStatus(Endpoint, USB_EPTX_STAT, USB_EP_TX_VALID);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           Reset
// Parameter(s):   void
// Return:         void
//
// Description:    Reset USB state and initialize the control endpoint
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::Reset(void)
{
    m_Configured = false;
    m_Suspended = false;
    m_ControlRemaining = 0;
    m_ControlZLP = false;
    m_AddressPending = false;
    m_Configuration = 0;
    USB_REGS->BTABLE = 0;

    ResetClassState();

    for(uint8_t Endpoint = 0; Endpoint < 8; ++Endpoint)
    {
        USB_ENDPOINT(Endpoint) = 0;
        SetEndpointStatus(Endpoint, USB_EPRX_STAT | USB_EPTX_STAT, 0);
        uint16_t Toggles = USB_ENDPOINT(Endpoint) & (USB_EP_DTOG_RX | USB_EP_DTOG_TX);
        USB_ENDPOINT(Endpoint) = Toggles;
    }

    USB_TABLE[0] = 0x40;
    USB_TABLE[1] = 0;
    USB_TABLE[2] = 0x80;
    USB_TABLE[3] = 0x8400;
    USB_ENDPOINT(0) = USB_EP_CONTROL;
    SetEndpointStatus(0, USB_EPRX_STAT | USB_EPTX_STAT, USB_EP_RX_VALID | USB_EP_TX_NAK);
    USB_REGS->DADDR = USB_DADDR_EF;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ConfigureEndpoints
//
// Parameter(s):   void
// Return:         void
//
// Description:    Configure CDC endpoints for the current USB configuration
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ConfigureEndpoints(void)
{
    m_Configured = false;

    ResetClassState();

    for(uint8_t Endpoint = 1; Endpoint < 8; ++Endpoint)
    {
        USB_ENDPOINT(Endpoint) = Endpoint;
        SetEndpointStatus(Endpoint, USB_EPRX_STAT | USB_EPTX_STAT, 0);
        uint16_t Toggles = USB_ENDPOINT(Endpoint) & (USB_EP_DTOG_RX | USB_EP_DTOG_TX);
        USB_ENDPOINT(Endpoint) = Endpoint | Toggles;
    }

    if(m_Configuration == 0)
    {
        return;
    }

    ConfigureClassEndpoints();
    m_Configured = true;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ResetClassState
// Parameter(s):   void
// Return:         void
//
// Description:    Reset CDC buffers and class-specific transfer state
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::ResetClassState(void)
{
    m_TxBusy = false;
    m_TxZLP = false;
    m_TxPacketLength = 0;
    m_RxFifo.Flush(m_RxFifo.CheckUsedSpace());
    m_TxFifo.Flush(m_TxFifo.CheckUsedSpace());
    m_ControlLines = 0;
    m_ExpectLineCoding = false;
    m_ReceiveBufferHead = 0;
    m_ReceiveTransfer.u.Head = 0;

    if((m_pReceiveCallback != nullptr) &&
       ((m_ReceiveCallbackTypes & SERIAL_CALLBACK_RX_IDLE) != 0))
    {
        m_pReceiveCallback->CallbackFunction(SERIAL_CALLBACK_RX_IDLE, &m_ReceiveTransfer);
    }
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           ConfigureClassEndpoints
// Parameter(s):   void
// Return:         void
//
// Description:    Configure the CDC notification and bulk endpoints
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::ConfigureClassEndpoints(void)
{
    ConfigureEndpoint(1, USB_EP_INTERRUPT, 0xC0, 0);
    SetEndpointStatus(1, USB_EPTX_STAT, USB_EP_TX_NAK);

    ConfigureEndpoint(2, USB_EP_BULK, 0xD0, 0x110);
    EnableEndpointRx(2, USB_CDC_PACKET_SIZE);
    SetEndpointStatus(2, USB_EPTX_STAT, USB_EP_TX_NAK);
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           GetDescriptor
//
// Parameter(s):   Value - descriptor type and index
//                 Requested - requested descriptor length
// Return:         bool - true when the descriptor is supported
//
// Description:    Select and send a USB device, configuration, or string descriptor
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
bool USB_CDC_ACM_Driver::GetClassDescriptor(uint16_t Value, uint16_t Requested)
{
    static const uint8_t Device[] = {
        18, 1, 0x00, 0x02, 0xEF, 2, 1, 64,
        USB_CDC_VID & 0xFF, USB_CDC_VID >> 8, USB_CDC_PID & 0xFF, USB_CDC_PID >> 8,
        0x00, 0x01, 1, 2, 3, 1
    };
    
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

    if(Value == 0x0100)
    {
        ControlSend(Device, sizeof(Device), Requested);
    }
    else if(Value == 0x0200)
    {
        ControlSend(Configuration, sizeof(Configuration), Requested);
    }
    else if((Value >> 8) == 3)
    {
        uint8_t Index = static_cast<uint8_t>(Value);

        if(Index == 0)
        {
            static const uint8_t Language[] = { 4, 3, 0x09, 0x04 };
            ControlSend(Language, sizeof(Language), Requested);
            return true;
        }

        const char* pText = nullptr;
        char Serial[25];

        if(Index == 1)
        {
            pText = USB_CDC_MANUFACTURER;
        }
        else if(Index == 2)
        {
            pText = USB_CDC_PRODUCT;
        }
        else if(Index == 3)
        {
            static const char Hex[] = "0123456789ABCDEF";
            const volatile uint32_t* pUID = reinterpret_cast<const volatile uint32_t*>(UID_BASE);

            for(uint8_t Word = 0; Word < 3; ++Word)
            {
                uint32_t Unique = pUID[Word];
                for(uint8_t Digit = 0; Digit < 8; ++Digit)
                {
                    Serial[Word * 8 + Digit] = Hex[(Unique >> (28 - Digit * 4)) & 15];
                }
            }

            Serial[24] = 0;
            pText = Serial;
        }
        else
        {
            return false;
        }

        uint16_t Characters = static_cast<uint16_t>(strlen(pText));

        if(Characters > ((sizeof(m_ControlBuffer) - 2) / 2))
        {
            Characters = (sizeof(m_ControlBuffer) - 2) / 2;
        }

        m_ControlBuffer[0] = static_cast<uint8_t>(2 + Characters * 2);
        m_ControlBuffer[1] = 3;

        for(uint16_t Position = 0; Position < Characters; ++Position)
        {
            m_ControlBuffer[2 + Position * 2] = static_cast<uint8_t>(pText[Position]);
            m_ControlBuffer[3 + Position * 2] = 0;
        }

        ControlSend(m_ControlBuffer, m_ControlBuffer[0], Requested);
    }
    else
    {
        return false;
    }

    return true;
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)
    
//-------------------------------------------------------------------------------------------------
//
// Name:           ControlSend
//
// Parameter(s):   pData - control response data
//                 Length - response length
//                 Requested - host-requested length
// Return:         void
//
// Description:    Prepare a control response and start its first packet
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ControlSend(const uint8_t* pData, uint16_t Length, uint16_t Requested)
{
    m_pControlData = pData;

    m_ControlZLP = (Length < Requested) && ((Length % USB_CONTROL_PACKET_SIZE) == 0);
    m_ControlRemaining = Length < Requested ? Length : Requested;
    ControlIn();
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ControlIn
//
// Parameter(s):   void
// Return:         void
//
// Description:    Send the next packet of a control response
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::ControlIn(void)
{

    if(m_AddressPending == true)
    {
        USB_REGS->DADDR = USB_DADDR_EF | m_PendingAddress;
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
// Name:           StallControl
//
// Parameter(s):   void
// Return:         void
//
// Description:    Stall the control endpoint and cancel any pending control transfer
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::StallControl(void)
{
    m_ControlRemaining = 0;
    m_ControlZLP = false;
    CancelClassControl();
    SetEndpointStatus(0, USB_EPRX_STAT | USB_EPTX_STAT, USB_EP_RX_STALL | USB_EP_TX_STALL);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           CancelClassControl
// Parameter(s):   void
// Return:         void
//
// Description:    Cancel any pending CDC-specific control request
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::CancelClassControl(void)
{
    m_ExpectLineCoding = false;
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Setup
//
// Parameter(s):   pSetup - received USB setup packet
// Return:         void
//
// Description:    Decode standard and CDC class setup requests
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::Setup(const uint8_t* pSetup)
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
    SetEndpointStatus(0, USB_EPRX_STAT | USB_EPTX_STAT, USB_EP_RX_VALID | USB_EP_TX_NAK);

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
            (Length == 0) && ((USB_REGS->DADDR & USB_DADDR_ADD) != 0))
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
                uint16_t Mask = (Index & 0x80) ? USB_EPTX_STAT : USB_EPRX_STAT;
                uint16_t Halt = (Index & 0x80) ? USB_EP_TX_STALL : USB_EP_RX_STALL;
                m_ControlBuffer[0] = (USB_ENDPOINT(Index & 15) & Mask) == Halt;
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
        uint16_t Mask = (Index & 0x80) ? USB_EPTX_STAT : USB_EPRX_STAT;
        uint16_t Status = (Index & 0x80) ? USB_EP_TX_STALL : USB_EP_RX_STALL;

        if(Request == 1)
        {
            uint16_t Toggle = (Index & 0x80) ? USB_EP_DTOG_TX : USB_EP_DTOG_RX;
            uint16_t Register = USB_ENDPOINT(Endpoint);
            USB_ENDPOINT(Endpoint) = (Register & (USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD | Toggle)) |
                USB_EP_CTR_RX | USB_EP_CTR_TX;
            Status = (Index & 0x80) ? USB_EP_TX_NAK : USB_EP_RX_NAK;
        }

        SetEndpointStatus(Endpoint, Mask, Status);

        if(Request == 1)
        {
            HandleEndpointHaltCleared(Endpoint);
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
// Name:           HandleClassSetup
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
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
        m_ExpectLineCoding = true;
        return true;
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           ControlOut
//
// Parameter(s):   pData - received control data
//                 Length - received data length
// Return:         void
//
// Description:    Validate and accept CDC line coding data
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::HandleClassControlOut(const uint8_t* pData, uint16_t Length)
{
    if(m_ExpectLineCoding == true)
    {
        m_ExpectLineCoding = false;

        if((Length != 7) || (pData[4] > 2) || (pData[5] > 4) ||
            ((((pData[6] >= 5) && (pData[6] <= 8)) || (pData[6] == 16)) == false) ||
            ((pData[0] | pData[1] | pData[2] | pData[3]) == 0))
        {
            StallControl();
            return;
        }

        memcpy(m_LineCoding, pData, sizeof(m_LineCoding));
        ControlSend(nullptr, 0, 0);
    }
    else
    {
        StallControl();
    }
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           StartTransmit
//
// Parameter(s):   void
// Return:         void
//
// Description:    Start the next queued CDC packet when the transmit endpoint is available
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Receive
//
// Parameter(s):   void
// Return:         void
//
// Description:    Receive one bulk packet into the RX ring buffer
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Process
//
// Parameter(s):   void
// Return:         void
//
// Description:    Re-enable RX when buffer space is available and advance queued TX data
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::Process(void)
{
    if(m_Initialized == false)
    {
        return;
    }

    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();

    ProcessClass();

    __set_PRIMASK(InterruptState);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           ProcessClass
// Parameter(s):   void
// Return:         void
//
// Description:    Service CDC receive flow control and pending transmissions
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           IsConfigured
//
// Parameter(s):   void
// Return:         bool - true when configured and not suspended
//
// Description:    Report whether the USB device can currently transfer data
//
//-------------------------------------------------------------------------------------------------
bool USB_Driver::IsConfigured(void) const
{
    return (m_Configured == true) && (m_Suspended == false);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetInterfaceCount
// Parameter(s):   void
// Return:         uint8_t - number of CDC interfaces
//
// Description:    Report the number of interfaces in the CDC ACM configuration
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
uint8_t USB_CDC_ACM_Driver::GetInterfaceCount(void) const
{
    return 2;
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           IsEndpointAvailable
// Parameter(s):   Endpoint - endpoint number
//                 In - true for an IN endpoint
// Return:         bool - true when the endpoint direction is used by CDC
//
// Description:    Validate an endpoint address against the CDC endpoint map
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
bool USB_CDC_ACM_Driver::IsEndpointAvailable(uint8_t Endpoint, bool In) const
{
    return ((Endpoint == 1) && (In == true)) || (Endpoint == 2);
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           HandleEndpointHaltCleared
// Parameter(s):   Endpoint - endpoint number
// Return:         void
//
// Description:    Resume CDC transmission when the host clears a bulk endpoint halt
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::HandleEndpointHaltCleared(uint8_t Endpoint)
{
    if(Endpoint == 2)
    {
        StartTransmit();
    }
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           HandleClassEndpointRx
// Parameter(s):   Endpoint - endpoint number
// Return:         void
//
// Description:    Dispatch received CDC endpoint data
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
void USB_CDC_ACM_Driver::HandleClassEndpointRx(uint8_t Endpoint)
{
    if((Endpoint == 2) && (m_Configured == true))
    {
        Receive();
    }
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           HandleClassEndpointTx
// Parameter(s):   Endpoint - endpoint number
// Return:         void
//
// Description:    Complete a CDC bulk IN packet and schedule the next one
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           IsDTR
//
// Parameter(s):   void
// Return:         bool - true when the host asserts DTR
//
// Description:    Report the current CDC DTR control-line state
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
bool USB_CDC_ACM_Driver::IsDTR(void) const
{
    return (IsConfigured() == true) && ((m_ControlLines & 1) != 0);
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           IsRTS
//
// Parameter(s):   void
// Return:         bool - true when the host asserts RTS
//
// Description:    Report the current CDC RTS control-line state
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
bool USB_CDC_ACM_Driver::IsRTS(void) const
{
    return (IsConfigured() == true) && ((m_ControlLines & 2) != 0);
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           GetBaudRate
//
// Parameter(s):   void
// Return:         uint32_t - host-requested baud rate
//
// Description:    Read the baud rate from the most recently accepted line coding
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
uint32_t USB_CDC_ACM_Driver::GetBaudRate(void) const
{
    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    uint32_t Baud = m_LineCoding[0] | (static_cast<uint32_t>(m_LineCoding[1]) << 8) |
        (static_cast<uint32_t>(m_LineCoding[2]) << 16) | (static_cast<uint32_t>(m_LineCoding[3]) << 24);
    __set_PRIMASK(InterruptState);
    return Baud;
}
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Available
//
// Parameter(s):   void
// Return:         uint16_t - number of bytes waiting in the RX buffer
//
// Description:    Return the number of buffered receive bytes
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Read
//
// Parameter(s):   pData - destination buffer (cannot be nullptr)
//                 Length - maximum number of bytes to read
// Return:         uint16_t - number of bytes copied
//
// Description:    Read buffered CDC data and resume reception when space is available
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Write
//
// Parameter(s):   pData - source buffer
//                 Length - number of bytes to queue
// Return:         uint16_t - number of bytes accepted
//
// Description:    Queue CDC data for transmission to the host
//
//-------------------------------------------------------------------------------------------------
#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
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
#endif // (USE_USB_CDC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           IRQHandler
//
// Parameter(s):   void
// Return:         void
//
// Description:    Handle USB reset, suspend, wakeup, and endpoint events
//
//-------------------------------------------------------------------------------------------------
void USB_Driver::IRQHandler(void)
{
    uint16_t Status = USB_REGS->ISTR;

    if((Status & USB_ISTR_RESET) != 0)
    {
        USB_REGS->CNTR &= ~(USB_CNTR_FSUSP | USB_CNTR_LPMODE);
        USB_REGS->ISTR = static_cast<uint16_t>(~(USB_ISTR_RESET | USB_ISTR_SUSP | USB_ISTR_WKUP));
        Reset();
    }
    else
    {
        if((Status & USB_ISTR_SUSP) != 0)
        {
            USB_REGS->CNTR |= USB_CNTR_FSUSP;
            m_Suspended = true;
            USB_REGS->ISTR = static_cast<uint16_t>(~USB_ISTR_SUSP);
        }

        if((Status & USB_ISTR_WKUP) != 0)
        {
            USB_REGS->CNTR &= ~(USB_CNTR_FSUSP | USB_CNTR_LPMODE);
            m_Suspended = false;
            USB_REGS->ISTR = static_cast<uint16_t>(~USB_ISTR_WKUP);
        }
    }

    while((USB_REGS->ISTR & USB_ISTR_CTR) != 0)
    {
        uint8_t Endpoint = USB_REGS->ISTR & USB_ISTR_EP_ID;
        uint16_t Register = USB_ENDPOINT(Endpoint);

        if((Register & USB_EP_CTR_RX) != 0)
        {
            if(Endpoint == 0)
            {
                uint8_t Packet[USB_CONTROL_PACKET_SIZE];
                uint16_t Length = USB_TABLE[3] & 0x3FF;
                bool IsSetup = (Register & USB_EP_SETUP) != 0;

                if(Length <= USB_CONTROL_PACKET_SIZE)
                {
                    ReadPMA(USB_TABLE[2], Packet, Length);
                }

                ClearEndpointEvent(0, USB_EP_CTR_RX | (IsSetup ? USB_EP_CTR_TX : 0));
                USB_TABLE[3] = 0x8400;

                if((IsSetup == true) && (Length == 8))
                {
                    Setup(Packet);
                }
                else if((IsSetup == false) && (Length <= USB_CONTROL_PACKET_SIZE))
                {
                    if(Length == 0)
                    {
                        m_ControlRemaining = 0;
                        m_ControlZLP = false;
                        SetEndpointStatus(0, USB_EPTX_STAT, USB_EP_TX_NAK);
                    }
                    else
                    {
                        HandleClassControlOut(Packet, Length);
                    }
                }
                else
                {
                    StallControl();
                }

                if((USB_ENDPOINT(0) & USB_EPRX_STAT) != USB_EP_RX_STALL)
                {
                    SetEndpointStatus(0, USB_EPRX_STAT, USB_EP_RX_VALID);
                }

                if(IsSetup == true)
                {
                    continue;
                }
            }
            else
            {
                ClearEndpointEvent(Endpoint, USB_EP_CTR_RX);

                HandleClassEndpointRx(Endpoint);
            }
        }

        if((USB_ENDPOINT(Endpoint) & USB_EP_CTR_TX) != 0)
        {
            ClearEndpointEvent(Endpoint, USB_EP_CTR_TX);

            if(Endpoint == 0)
            {
                if((m_AddressPending == true) || (m_ControlRemaining != 0) || (m_ControlZLP == true))
                {
                    ControlIn();
                }
            }
            else
            {
                HandleClassEndpointTx(Endpoint);
            }
        }
    }

    Process();
}

#endif
