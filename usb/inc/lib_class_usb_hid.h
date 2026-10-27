//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_hid.h
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 HID and CustomHID.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#pragma once

#if (USE_USB_DRIVER == DEF_ENABLED) && \
    ((USE_USB_HID_DRIVER == DEF_ENABLED) || (USE_USB_CUSTOM_HID_DRIVER == DEF_ENABLED))

#include "lib_class_usb_class_base.h"

class USB_HID_ReportInterface
{
    public:

        virtual bool        GetReport               (uint8_t Type, uint8_t Id, uint8_t* pData,
                                                     uint16_t* pLength) = 0;
        virtual bool        SetReport               (uint8_t Type, uint8_t Id, const uint8_t* pData,
                                                     uint16_t Length) = 0;

    protected:

        virtual            ~USB_HID_ReportInterface (void) = default;
};

class USB_HID_Base : public USB_ClassBase
{
    public:

        SystemState_e       SendReport              (const uint8_t* pData, uint16_t Length);

    protected:

                            USB_HID_Base            (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
                                                     const uint8_t* pDescriptor, uint16_t DescriptorLength,
                                                     USB_HID_ReportInterface* pReports, bool Mouse);
        bool                InitializeClass         (void) override;
        void                ResetClassState         (void) override;
        void                ConfigureClassEndpoints (void) override;
        const uint8_t*      GetConfiguration        (uint16_t* pLength) const override;
        bool                HandleClassSetup        (uint8_t Type, uint8_t Request, uint16_t Value,
                                                     uint16_t Index, uint16_t Length) override;
        void                ControlReceiveComplete  (uint16_t Length) override;
        void                HandleClassEndpointRx   (uint8_t Endpoint) override;
        void                HandleClassEndpointTx   (uint8_t Endpoint) override;
        void                HandleEndpointHaltCleared(uint8_t Endpoint) override;
        bool                IsEndpointAvailable     (uint8_t Endpoint, bool In) const override;
        void                ProcessClass            (void) override;
        void                ProcessTask             (void) override;
        void                CancelClassControl      (void) override;

    private:

        const uint8_t*      m_pReportDescriptor;
        uint16_t            m_ReportDescriptorLength;
        USB_HID_ReportInterface* m_pReports;
        bool                m_Mouse;
        uint8_t             m_ConfigurationDescriptor[41];
        uint8_t             m_TxReport[64];
        uint16_t            m_TxLength;
        volatile bool       m_TxBusy;
        uint8_t             m_RxReport[64];
        uint8_t             m_OutReport[64];
        uint16_t            m_OutLength;
        volatile bool       m_OutPending;
        uint8_t             m_ReportType;
        uint8_t             m_ReportId;
        uint16_t            m_ReportLength;
        volatile bool       m_ReportPending;
        uint8_t             m_Idle;
        uint8_t             m_Protocol;
        uint8_t             m_Feature;
        uint32_t            m_ControlGeneration;
        uint32_t            m_Generation;
        uint32_t            m_LastReportTick;
};

#if (USE_USB_HID_DRIVER == DEF_ENABLED)
class USB_HID_Driver : public USB_HID_Base
{
    public:

                            USB_HID_Driver          (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity);
};
#endif

#if (USE_USB_CUSTOM_HID_DRIVER == DEF_ENABLED)
class USB_CustomHID_Driver : public USB_HID_Base
{
    public:

                            USB_CustomHID_Driver    (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
                                                     const uint8_t* pDescriptor, uint16_t DescriptorLength,
                                                     USB_HID_ReportInterface& Reports);
};
#endif

#endif
