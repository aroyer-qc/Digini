//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_cdc.h
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

#if (USE_USB_DRIVER == DEF_ENABLED) && (USE_USB_CDC_DRIVER == DEF_ENABLED)

#include "lib_class_usb_class_base.h"
#include "../../Digini/inc/lib_typedef.h"
#include "../../Digini/inc/lib_class_cbi.h"
#include "../../Peripheral/inc/interface/lib_class_serial_port_interface.h"
#include "../../Utility/inc/lib_fifo.h"

#if (DIGINI_USE_STATIC_MEMORY_ALLOC != DEF_ENABLED)
#error USB CDC requires the Digini memory pool
#endif

#define USB_CDC_PACKET_SIZE 64
#define USB_CDC_BUFFER_SIZE 512
#define USB_CDC_LINE_CODING_SIZE 7

class USB_CDC_ACM_Driver : public USB_ClassBase, public SerialPortInterface
{
    public:

                            USB_CDC_ACM_Driver      (USB_DeviceBackend& Backend);
                            USB_CDC_ACM_Driver      (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity);

        void                InitializeTransport     (void) override;
        void                SetReceiveBuffer        (uint8_t* pBuffer, size_t Capacity) override;
        SystemState_e       SendData                (const uint8_t* pData, size_t* pSize) override;
        void                RegisterReceiveCallback (CallbackInterface* pCallback) override;
        void                EnableReceiveCallbacks  (int CallbackTypes) override;

        bool                IsDTR                   (void) const;
        bool                IsRTS                   (void) const;
        uint32_t            GetBaudRate             (void) const;
        uint16_t            Available               (void) const;
        uint16_t            Read                    (uint8_t* pData, uint16_t Length);
        uint16_t            Write                   (const uint8_t* pData, uint16_t Length);

    protected:

        bool                InitializeClass         (void) override;
        void                ResetClassState         (void) override;
        void                ConfigureClassEndpoints (void) override;
        const uint8_t*      GetConfiguration        (uint16_t* pLength) const override;
        void                GetDeviceClass          (uint8_t* pClass, uint8_t* pSubClass, uint8_t* pProtocol) const override;
        bool                HandleClassSetup        (uint8_t Type, uint8_t Request, uint16_t Value,
                                                     uint16_t Index, uint16_t Length) override;
        void                ControlReceiveComplete  (uint16_t Length) override;
        void                HandleClassEndpointRx   (uint8_t Endpoint) override;
        void                HandleClassEndpointTx   (uint8_t Endpoint) override;
        void                HandleEndpointHaltCleared (uint8_t Endpoint) override;
        bool                IsEndpointAvailable     (uint8_t Endpoint, bool In) const override;
        void                ProcessClass            (void) override;

    private:

        void                StartTransmit           (void);
        void                Receive                 (void);

        volatile bool       m_TxBusy;
        volatile uint16_t   m_ControlLines;
        FIFO_Buffer         m_RxFifo;
        FIFO_Buffer         m_TxFifo;
        static const uint8_t m_DefaultLineCoding[USB_CDC_LINE_CODING_SIZE];
        uint8_t             m_LineCoding[USB_CDC_LINE_CODING_SIZE];
        bool                m_TxZLP;
        uint16_t            m_TxPacketLength;
        uint8_t             m_PendingLineCoding[USB_CDC_LINE_CODING_SIZE];
        uint8_t*            m_pReceiveBuffer;
        size_t              m_ReceiveBufferCapacity;
        size_t              m_ReceiveBufferHead;
        SerialTransfer_t    m_ReceiveTransfer;
        CallbackInterface*  m_pReceiveCallback;
        int                 m_ReceiveCallbackTypes;
};

#endif // USB CDC driver
