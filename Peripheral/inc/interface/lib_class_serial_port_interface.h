//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_serial_port_interface.h
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "./Digini/inc/lib_typedef.h"
#include "./Digini/inc/lib_class_cbi.h"

//-------------------------------------------------------------------------------------------------
// Type definition(s)
//-------------------------------------------------------------------------------------------------

enum SerialCallbackType_e
{
    SERIAL_CALLBACK_RX_ERROR       = 0x01,
    SERIAL_CALLBACK_RX_IDLE        = 0x02,
    SERIAL_CALLBACK_RX_NOT_EMPTY   = 0x04,
    SERIAL_CALLBACK_TX_COMPLETED   = 0x08
};

struct SerialTransfer_t
{
    uint8_t* pBuffer;
    union
    {
        size_t Size;
        size_t Head;
    } u;
    size_t StaticSize;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SerialPortInterface
{
    public:

        virtual void InitializeTransport(void) = 0;
        virtual void SetReceiveBuffer(uint8_t* pBuffer, size_t Capacity) = 0;
        virtual SystemState_e SendData(const uint8_t* pData, size_t* pSize) = 0;
        virtual void RegisterReceiveCallback(CallbackInterface* pCallback) = 0;
        virtual void EnableReceiveCallbacks(int CallbackTypes) = 0;
};

//-------------------------------------------------------------------------------------------------