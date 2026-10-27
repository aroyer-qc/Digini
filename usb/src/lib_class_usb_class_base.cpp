//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_class_base.cpp
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 Core and class descriptor handling.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

#if (USE_USB_DRIVER == DEF_ENABLED) && DIGINI_USB_DEVICE_CLASSES

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_ClassBase
//  Parameter(s):   Backend, Identity
//  Return:         None
//  Description:    Bind a class to its backend and application-owned identity strings.
//
//-------------------------------------------------------------------------------------------------
USB_ClassBase::USB_ClassBase(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity)
    : USB_DeviceDriver(Backend)
{
    m_Identity = Identity;
    m_pControlReceive = nullptr;
    m_ControlReceiveLength = 0;
    m_ControlReceiveCount = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       InitializeClass
//  Parameter(s):   None
//  Return:         bool
//  Description:    Validate the application descriptor identity.
//
//-------------------------------------------------------------------------------------------------
bool USB_ClassBase::InitializeClass(void)
{
    return (m_Identity.VendorId != 0) && (m_Identity.pManufacturer != nullptr) &&
           (m_Identity.pProduct != nullptr);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SendString
//  Parameter(s):   pText, Requested
//  Return:         bool
//  Description:    Encode a bounded ASCII string as a USB UTF-16LE descriptor.
//
//-------------------------------------------------------------------------------------------------
bool USB_ClassBase::SendString(const char* pText, uint16_t Requested)
{
    if(pText == nullptr)
    {
        return false;
    }
    uint16_t Count = 0;
    while((pText[Count] != 0) && (Count < (sizeof(m_ControlBuffer) - 2U) / 2U))
    {
        m_ControlBuffer[2U + Count * 2U] = uint8_t(pText[Count]);
        m_ControlBuffer[3U + Count * 2U] = 0;
        Count++;
    }
    m_ControlBuffer[0] = uint8_t(2U + Count * 2U);
    m_ControlBuffer[1] = 3;
    ControlSend(m_ControlBuffer, m_ControlBuffer[0], Requested);
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetClassDescriptor
//  Parameter(s):   Value, Requested
//  Return:         bool
//  Description:    Supply full-speed device, configuration and identity descriptors.
//
//-------------------------------------------------------------------------------------------------
bool USB_ClassBase::GetClassDescriptor(uint16_t Value, uint16_t Requested)
{
    if(Value == 0x0100)
    {
        const uint8_t Descriptor[] = {18, 1, 0, 2, 0, 0, 0, 64,
            uint8_t(m_Identity.VendorId), uint8_t(m_Identity.VendorId >> 8),
            uint8_t(m_Identity.ProductId), uint8_t(m_Identity.ProductId >> 8), 0, 1, 1, 2, 3, 1};
        memcpy(m_ControlBuffer, Descriptor, sizeof(Descriptor));
        GetDeviceClass(&m_ControlBuffer[4], &m_ControlBuffer[5], &m_ControlBuffer[6]);
        ControlSend(m_ControlBuffer, sizeof(Descriptor), Requested);
        return true;
    }
    if(Value == 0x0200)
    {
        uint16_t Length = 0;
        const uint8_t* pDescriptor = GetConfiguration(&Length);
        if((pDescriptor == nullptr) || (Length < 9))
        {
            return false;
        }
        ControlSend(pDescriptor, Length, Requested);
        return true;
    }
    if(Value == 0x0300)
    {
        static const uint8_t Language[] = {4, 3, 9, 4};
        ControlSend(Language, sizeof(Language), Requested);
        return true;
    }
    if(Value == 0x0301)
    {
        return SendString(m_Identity.pManufacturer, Requested);
    }
    if(Value == 0x0302)
    {
        return SendString(m_Identity.pProduct, Requested);
    }
    if(Value == 0x0303)
    {
        uint32_t Id[3];
        char Serial[25];
        static const char Hex[] = "0123456789ABCDEF";
        m_Backend.ReadUniqueId(Id);
        for(uint8_t Word = 0; Word < 3; Word++)
        {
            for(uint8_t Digit = 0; Digit < 8; Digit++)
            {
                Serial[Word * 8U + Digit] = Hex[(Id[Word] >> (28U - Digit * 4U)) & 15U];
            }
        }
        Serial[24] = 0;
        return SendString(Serial, Requested);
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetDeviceClass
//  Parameter(s):   pClass, pSubClass, pProtocol
//  Return:         None
//  Description:    Default to interface-level class identification; CDC overrides for its IAD.
//
//-------------------------------------------------------------------------------------------------
void USB_ClassBase::GetDeviceClass(uint8_t* pClass, uint8_t* pSubClass, uint8_t* pProtocol) const
{
    *pClass = 0;
    *pSubClass = 0;
    *pProtocol = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetInterfaceCount
//  Parameter(s):   None
//  Return:         uint8_t
//  Description:    Use the same interface count as the class configuration descriptor.
//
//-------------------------------------------------------------------------------------------------
uint8_t USB_ClassBase::GetInterfaceCount(void) const
{
    uint16_t Length = 0;
    const uint8_t* pDescriptor = GetConfiguration(&Length);
    return ((pDescriptor != nullptr) && (Length >= 9)) ? pDescriptor[4] : 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       BeginControlReceive
//  Parameter(s):   pBuffer, Length, Capacity
//  Return:         bool
//  Description:    Collect a bounded control OUT transfer across full-size packets.
//
//-------------------------------------------------------------------------------------------------
bool USB_ClassBase::BeginControlReceive(uint8_t* pBuffer, uint16_t Length, uint16_t Capacity)
{
    if((pBuffer == nullptr) || (Length == 0) || (Length > Capacity))
    {
        StallControl();
        return false;
    }
    m_pControlReceive = pBuffer;
    m_ControlReceiveLength = Length;
    m_ControlReceiveCount = 0;
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassControlOut
//  Parameter(s):   pData, Length
//  Return:         None
//  Description:    Validate packet lengths and deliver only complete control transfers.
//
//-------------------------------------------------------------------------------------------------
void USB_ClassBase::HandleClassControlOut(const uint8_t* pData, uint16_t Length)
{
    if((m_pControlReceive == nullptr) || (Length == 0) ||
       (Length > m_ControlReceiveLength - m_ControlReceiveCount))
    {
        StallControl();
        return;
    }
    memcpy(m_pControlReceive + m_ControlReceiveCount, pData, Length);
    m_ControlReceiveCount += Length;
    if(m_ControlReceiveCount == m_ControlReceiveLength)
    {
        uint16_t Count = m_ControlReceiveCount;
        m_pControlReceive = nullptr;
        ControlReceiveComplete(Count);
    }
    else if(Length < USB_CONTROL_PACKET_SIZE)
    {
        StallControl();
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CancelClassControl
//  Parameter(s):   None
//  Return:         None
//  Description:    Discard an incomplete request when superseded by SETUP or a stall.
//
//-------------------------------------------------------------------------------------------------
void USB_ClassBase::CancelClassControl(void)
{
    m_pControlReceive = nullptr;
    m_ControlReceiveLength = 0;
    m_ControlReceiveCount = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ControlReceivePending
//  Parameter(s):   None
//  Return:         bool
//  Description:    Distinguish a premature short OUT packet from an IN status acknowledgement.
//
//-------------------------------------------------------------------------------------------------
bool USB_ClassBase::ControlReceivePending(void) const
{
    return m_pControlReceive != nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleEndpointHaltCleared
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Classes with transport recovery override this hook.
//
//-------------------------------------------------------------------------------------------------
void USB_ClassBase::HandleEndpointHaltCleared(uint8_t Endpoint)
{
    (void)Endpoint;
}

#endif
