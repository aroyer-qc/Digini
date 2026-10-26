//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_device.h
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

#pragma once

#if (USE_USB_DRIVER == DEF_ENABLED)

#if (USE_USB_AUDIO_DRIVER == DEF_ENABLED)
#error USB Audio device class is not implemented
#endif

#include "./lib_usb_device_backend.h"

#define USB_CONTROL_PACKET_SIZE 64
#define DIGINI_USB_DEVICE_CLASSES ((USE_USB_CDC_DRIVER == DEF_ENABLED) || (USE_USB_HID_DRIVER == DEF_ENABLED) || \
    (USE_USB_CUSTOM_HID_DRIVER == DEF_ENABLED) || (USE_USB_MSC_DRIVER == DEF_ENABLED) || \
    (USE_USB_DFU_DRIVER == DEF_ENABLED) || (USE_USB_DFU_RUNTIME_DRIVER == DEF_ENABLED))

class USB_DeviceDriver : public USB_DeviceEventSink
{
    public:

                            USB_DeviceDriver        (USB_DeviceBackend& Backend);
        virtual            ~USB_DeviceDriver        (void) = default;

        bool                Initialize              (void);
        void                Process                 (void);
        void                IRQHandler              (void);
        bool                IsConfigured            (void) const;

        void                OnUsbBusReset           (void) override;
        void                OnUsbSuspend            (void) override;
        void                OnUsbWakeup             (void) override;
        void                OnUsbSetupPacket        (const uint8_t* pSetup) override;
        void                OnUsbControlOut         (const uint8_t* pData, uint16_t Length) override;
        void                OnUsbEndpointRx         (uint8_t Endpoint) override;
        void                OnUsbEndpointTx         (uint8_t Endpoint) override;
        void                OnUsbInterruptComplete  (void) override;

    protected:

        virtual bool        InitializeClass         (void) = 0;
        virtual void        ResetClassState         (void) = 0;
        virtual void        ConfigureClassEndpoints (void) = 0;
        virtual bool        GetClassDescriptor      (uint16_t Value, uint16_t Requested) = 0;
        virtual bool        HandleClassSetup        (uint8_t Type, uint8_t Request, uint16_t Value,
                                                     uint16_t Index, uint16_t Length) = 0;
        virtual void        CancelClassControl      (void) = 0;
        virtual void        HandleClassControlOut   (const uint8_t* pData, uint16_t Length) = 0;
        virtual void        HandleClassEndpointRx   (uint8_t Endpoint) = 0;
        virtual void        HandleClassEndpointTx   (uint8_t Endpoint) = 0;
        virtual void        HandleEndpointHaltCleared (uint8_t Endpoint) = 0;
        virtual bool        IsEndpointAvailable     (uint8_t Endpoint, bool In) const = 0;
        virtual uint8_t     GetInterfaceCount       (void) const = 0;
        virtual void        ProcessClass            (void) = 0;
#if DIGINI_USB_DEVICE_CLASSES
        virtual void        ControlTransmitComplete (void) {}
        virtual bool        ControlReceivePending   (void) const { return false; }
        virtual void        ProcessTask             (void) {}
#endif

        void                SetEndpointState        (uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                                     USB_DeviceEndpointState_e State);
        void                ConfigureEndpoint       (uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                                     USB_DeviceEndpointType_e Type, uint16_t MaxPacketSize);
        bool                IsEndpointTxReady       (uint8_t Endpoint) const;
        bool                IsEndpointTxStalled     (uint8_t Endpoint) const;
        bool                IsEndpointRxBlocked     (uint8_t Endpoint) const;
        void                EnableEndpointRx       (uint8_t Endpoint, uint16_t Capacity);
        uint16_t            GetEndpointRxLength     (uint8_t Endpoint) const;
        void                ReadEndpointRx          (uint8_t Endpoint, uint8_t* pData, uint16_t Length);
        void                Transmit                (uint8_t Endpoint, const uint8_t* pData, uint16_t Length);
        void                ControlSend             (const uint8_t* pData, uint16_t Length, uint16_t Requested);
        void                StallControl            (void);

        USB_DeviceBackend&  m_Backend;
        volatile bool       m_Configured;
        volatile bool       m_Suspended;
        uint8_t             m_ControlBuffer[128];
        uint16_t            m_ControlRemaining;
        bool                m_ControlZLP;
#if DIGINI_USB_DEVICE_CLASSES
        bool                m_InInterrupt;
#endif

    private:

        void                Reset                   (void);
        void                ConfigureEndpoints      (void);
        void                Setup                   (const uint8_t* pSetup);
        void                ControlIn               (void);

        const uint8_t*      m_pControlData;
        bool                m_AddressPending;
        uint8_t             m_PendingAddress;
        uint8_t             m_Configuration;
        bool                m_Initialized;
};

#endif
