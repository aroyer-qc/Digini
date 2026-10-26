#pragma once

#include "../../../../Digini/inc/lib_typedef.h"

enum USB_DeviceDirection_e
{
    USB_DEVICE_DIRECTION_OUT,
    USB_DEVICE_DIRECTION_IN
};

enum USB_DeviceEndpointType_e
{
    USB_DEVICE_ENDPOINT_CONTROL,
    USB_DEVICE_ENDPOINT_ISOCHRONOUS,
    USB_DEVICE_ENDPOINT_BULK,
    USB_DEVICE_ENDPOINT_INTERRUPT
};

enum USB_DeviceEndpointState_e
{
    USB_DEVICE_ENDPOINT_DISABLED,
    USB_DEVICE_ENDPOINT_STALL,
    USB_DEVICE_ENDPOINT_NAK,
    USB_DEVICE_ENDPOINT_READY
};

class USB_DeviceEventSink
{
    public:

        virtual void OnUsbBusReset(void) = 0;
        virtual void OnUsbSuspend(void) = 0;
        virtual void OnUsbWakeup(void) = 0;
        virtual void OnUsbSetupPacket(const uint8_t* pSetup) = 0;
        virtual void OnUsbControlOut(const uint8_t* pData, uint16_t Length) = 0;
        virtual void OnUsbEndpointRx(uint8_t Endpoint) = 0;
        virtual void OnUsbEndpointTx(uint8_t Endpoint) = 0;
        virtual void OnUsbInterruptComplete(void) = 0;

    protected:

        virtual ~USB_DeviceEventSink(void) = default;
};

class USB_DeviceBackend
{
    public:

        virtual void ReadUniqueId(uint32_t* pWords) const = 0;
        virtual bool Initialize(void) = 0;
        virtual void Start(void) = 0;
        virtual void HandleInterrupt(USB_DeviceEventSink& EventSink) = 0;
        virtual void ResetEndpoints(void) = 0;
        virtual void ResetNonControlEndpoints(void) = 0;
        virtual void SetAddress(uint8_t Address) = 0;
        virtual bool IsAddressed(void) const = 0;
        virtual void ConfigureEndpoint(uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                       USB_DeviceEndpointType_e Type, uint16_t MaxPacketSize) = 0;
        virtual void SetEndpointState(uint8_t Endpoint, USB_DeviceDirection_e Direction,
                                      USB_DeviceEndpointState_e State) = 0;
        virtual void ClearEndpointHalt(uint8_t Endpoint, USB_DeviceDirection_e Direction) = 0;
        virtual bool IsEndpointStalled(uint8_t Endpoint, USB_DeviceDirection_e Direction) const = 0;
        virtual bool IsEndpointTxReady(uint8_t Endpoint) const = 0;
        virtual bool IsEndpointTxStalled(uint8_t Endpoint) const = 0;
        virtual bool IsEndpointRxBlocked(uint8_t Endpoint) const = 0;
        virtual void EnableEndpointRx(uint8_t Endpoint, uint16_t Capacity) = 0;
        virtual uint16_t GetEndpointRxLength(uint8_t Endpoint) const = 0;
        virtual void ReadEndpointRx(uint8_t Endpoint, uint8_t* pData, uint16_t Length) = 0;
        virtual void Transmit(uint8_t Endpoint, const uint8_t* pData, uint16_t Length) = 0;

    protected:

        virtual ~USB_DeviceBackend(void) = default;
};
