//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_class_base.h
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 Core and class descriptor handling.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#pragma once

#include "../../Peripheral/inc/port/interface/lib_class_usb_device.h"

#if (USE_USB_DRIVER == DEF_ENABLED) && DIGINI_USB_DEVICE_CLASSES

struct USB_DeviceIdentity_t
{
    uint16_t VendorId;
    uint16_t ProductId;
    const char* pManufacturer;
    const char* pProduct;
};

class USB_ClassBase : public USB_DeviceDriver
{
    public:

                            USB_ClassBase           (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity);

    protected:

        bool                InitializeClass         (void) override;
        bool                GetClassDescriptor      (uint16_t Value, uint16_t Requested) override;
        uint8_t             GetInterfaceCount       (void) const override;
        void                HandleClassControlOut   (const uint8_t* pData, uint16_t Length) override;
        void                CancelClassControl      (void) override;
        bool                ControlReceivePending   (void) const override;
        void                HandleEndpointHaltCleared(uint8_t Endpoint) override;
        bool                BeginControlReceive     (uint8_t* pBuffer, uint16_t Length, uint16_t Capacity);
        bool                SendString              (const char* pText, uint16_t Requested);
        virtual void        GetDeviceClass          (uint8_t* pClass, uint8_t* pSubClass, uint8_t* pProtocol) const;
        virtual const uint8_t* GetConfiguration     (uint16_t* pLength) const = 0;
        virtual void        ControlReceiveComplete  (uint16_t Length) = 0;

        USB_DeviceIdentity_t m_Identity;

    private:

        uint8_t*            m_pControlReceive;
        uint16_t            m_ControlReceiveLength;
        uint16_t            m_ControlReceiveCount;
};

#endif
