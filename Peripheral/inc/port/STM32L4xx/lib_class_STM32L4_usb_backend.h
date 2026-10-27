//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32L4_usb_backend.h
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

#if (USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_class_STM32L4_usb.h"
#include "../interface/lib_class_usb_device.h"
#include "../../../../usb/inc/lib_class_usb_cdc.h"

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class STM32L4_USB_PMA_Backend : public USB_DeviceBackend, public CallbackInterface
{
    public:

                        STM32L4_USB_PMA_Backend      (void);
        void            ReadUniqueId                (uint32_t* pWords) const override;
        bool            Initialize                  (void) override;
        void            Start                       (void) override;
        void            HandleInterrupt             (USB_DeviceEventSink& EventSink) override;
        void            ResetEndpoints              (void) override;
        void            ResetNonControlEndpoints    (void) override;
        void            SetAddress                  (uint8_t Address) override;
        bool            IsAddressed                 (void) const override;
        void            ConfigureEndpoint           (uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                                     USB_DeviceEndpointType_e Type, uint16_t MaxPacketSize) override;
        void            SetEndpointState            (uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                                     USB_DeviceEndpointState_e State) override;
        void            ClearEndpointHalt           (uint8_t Endpoint, USB_DeviceDirection_e Direction) override;
        bool            IsEndpointStalled           (uint8_t Endpoint, USB_DeviceDirection_e Direction) const override;
        bool            IsEndpointTxReady           (uint8_t Endpoint) const override;
        bool            IsEndpointTxStalled         (uint8_t Endpoint) const override;
        bool            IsEndpointRxBlocked         (uint8_t Endpoint) const override;
        void            EnableEndpointRx            (uint8_t Endpoint, uint16_t Capacity) override;
        uint16_t        GetEndpointRxLength         (uint8_t Endpoint) const override;
        void            ReadEndpointRx              (uint8_t Endpoint, uint8_t* pData, uint16_t Length) override;
        void            Transmit                    (uint8_t Endpoint, const uint8_t* pData, uint16_t Length) override;
        void            CallbackFunction            (int Type, void* pContext) override;

    private:

        void            CheckResult                 (SystemState_e Result) const;
        uint8_t         GetAddress                  (uint8_t Endpoint, USB_DeviceDirection_e Direction) const;
        uint16_t        GetStatus                   (uint8_t Endpoint, USB_DeviceDirection_e Direction) const;

        USB_Driver              m_Controller;
        USB_DeviceEventSink*    m_pEventSink;
        uint16_t                m_NextPMAAddress;
        uint8_t                 m_Address;
        uint8_t                 m_ReceiveBuffer[USB_ENDPOINT_COUNT][USB_CONTROL_PACKET_SIZE];
        uint8_t                 m_TransmitBuffer[USB_ENDPOINT_COUNT][USB_CONTROL_PACKET_SIZE];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#if (USE_USB_CDC_DRIVER == DEF_ENABLED)
  #ifdef USB_DRIVER_GLOBAL
    STM32L4_USB_PMA_Backend USB_DeviceBackendL4;
    USB_CDC_ACM_Driver myUSB_CDC(USB_DeviceBackendL4);
  #else
    extern STM32L4_USB_PMA_Backend USB_DeviceBackendL4;
    extern USB_CDC_ACM_Driver myUSB_CDC;
  #endif
#endif

#endif // (USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)
