//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32L4_usb.h
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

#if(USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "../../device/port/STM32L4xx/stm32l4xx.h"
#include "../../../../Digini/inc/lib_typedef.h"
#include "../../../../Digini/inc/lib_class_cbi.h"

#if !defined(USB)
#error USB_Driver requires the STM32L4 USB FS/PMA controller
#endif

//-------------------------------------------------------------------------------------------------
// Define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

#define USB_ENDPOINT_COUNT 8U
#define USB_PMA_SIZE 1024U
#define USB_ERROR_INVALID_CALLBACK 0x00000010U
#define USB_ENDPOINT_MASK 0x07U
#define USB_ENDPOINT_CONTROL 0U
#define USB_ENDPOINT_ISOCHRONOUS 1U
#define USB_ENDPOINT_BULK 2U
#define USB_ENDPOINT_INTERRUPT 3U
#define USB_SINGLE_BUFFER 0U
#define USB_DOUBLE_BUFFER 1U

//-------------------------------------------------------------------------------------------------
// Type definition(s), enum(s) and structure(s)
//-------------------------------------------------------------------------------------------------

enum USB_State_e
{
    USB_STATE_RESET   = 0x00,
    USB_STATE_READY   = 0x01,
    USB_STATE_ERROR   = 0x02,
    USB_STATE_BUSY    = 0x03,
    USB_STATE_TIMEOUT = 0x04
};

enum USB_LPM_State_e
{
    USB_LPM_L0 = 0x00,
    USB_LPM_L1 = 0x01,
    USB_LPM_L2 = 0x02,
    USB_LPM_L3 = 0x03,
};

enum USB_LPM_Message_e
{
    USB_LPM_L0_ACTIVE = 0x00,
    USB_LPM_L1_ACTIVE = 0x01,
};

enum USB_BCD_Message_e
{
    USB_BCD_ERROR                    = 0xFF,
    USB_BCD_CONTACT_DETECTION        = 0xFE,
    USB_BCD_STD_DOWNSTREAM_PORT      = 0xFD,
    USB_BCD_CHARGING_DOWNSTREAM_PORT = 0xFC,
    USB_BCD_DEDICATED_CHARGING_PORT  = 0xFB,
    USB_BCD_DISCOVERY_COMPLETED      = 0x00,
};

enum USB_Callback_e
{
    USB_CALLBACK_SOF        = 0x01,
    USB_CALLBACK_SETUP      = 0x02,
    USB_CALLBACK_RESET      = 0x03,
    USB_CALLBACK_SUSPEND    = 0x04,
    USB_CALLBACK_RESUME     = 0x05,
    USB_CALLBACK_CONNECT    = 0x06,
    USB_CALLBACK_DISCONNECT = 0x07,

    USB_CALLBACK_HARDWARE_INITIALIZE   = 0x08,
    USB_CALLBACK_HARDWARE_DEINITIALIZE = 0x09,
    USB_CALLBACK_DATA_OUT,
    USB_CALLBACK_DATA_IN,
    USB_CALLBACK_ISO_OUT_INCOMPLETE,
    USB_CALLBACK_ISO_IN_INCOMPLETE,
    USB_CALLBACK_LPM,
    USB_CALLBACK_BCD,
};

struct USB_Info_t
{
    uint8_t EndpointCount;
    bool    SOF_Enable;
    bool    LPM_Enable;
};

struct USB_Endpoint_t
{
    uint8_t  Number;
    uint8_t  IsIn;
    uint8_t  IsStall;
    uint8_t  Type;
    uint8_t  DataPID;
    uint16_t PMA_Address;
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    uint16_t PMA_Address0;
    uint16_t PMA_Address1;
    uint8_t  DoubleBuffer;
#endif
    uint32_t MaxPacketSize;
    uint8_t* pTransferBuffer;
    uint32_t TransferLength;
    uint32_t TransferCount;
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
    uint32_t DoubleBufferLength;
    uint8_t  DoubleBufferFill;
#endif
};

class USB_Driver;

struct USB_CallbackInfo_t
{
    USB_Driver* pDriver;
    uint32_t    Value;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class USB_Driver
{
    public:

                                USB_Driver                              (void);
        virtual                 ~USB_Driver                             (void) = default;
                                USB_Driver                              (const USB_Driver&) = delete;
        USB_Driver&             operator=                               (const USB_Driver&) = delete;

        SystemState_e           Initialize                              (const USB_Info_t* pInfo);
        SystemState_e           DeInitialize                            (void);
        SystemState_e           RegisterCallback                        (USB_Callback_e CallbackID, CallbackInterface* pCallback);
        SystemState_e           UnRegisterCallback                      (USB_Callback_e CallbackID);
        SystemState_e           RegisterDataOutStageCallback            (CallbackInterface* pCallback);
        SystemState_e           UnRegisterDataOutStageCallback          (void);
        SystemState_e           RegisterDataInStageCallback             (CallbackInterface* pCallback);
        SystemState_e           UnRegisterDataInStageCallback           (void);
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
        SystemState_e           RegisterIsoOutIncpltCallback            (CallbackInterface* pCallback);
        SystemState_e           UnRegisterIsoOutIncpltCallback          (void);
        SystemState_e           RegisterIsoInIncpltCallback             (CallbackInterface* pCallback);
        SystemState_e           UnRegisterIsoInIncpltCallback           (void);
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
        SystemState_e           RegisterBcdCallback                     (CallbackInterface* pCallback);
        SystemState_e           UnRegisterBcdCallback                   (void);
#endif
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        SystemState_e           RegisterLpmCallback                     (CallbackInterface* pCallback);
        SystemState_e           UnRegisterLpmCallback                   (void);
#endif
        SystemState_e           Start                                   (void);
        SystemState_e           Stop                                    (void);
        void                    IRQHandler                              (void);
        SystemState_e           Connect                                 (void);
        SystemState_e           Disconnect                              (void);
        SystemState_e           SetAddress                              (uint8_t Address);
        SystemState_e           OpenEndpoint                            (uint8_t EndpointAddress, uint16_t MaxPacketSize, uint8_t Type);
        SystemState_e           CloseEndpoint                           (uint8_t EndpointAddress);
        SystemState_e           Receive                                 (uint8_t EndpointAddress, uint8_t* pBuffer, uint32_t Length);
        SystemState_e           GetReceiveCount                         (uint8_t EndpointAddress, uint32_t* pCount) const;
        SystemState_e           Transmit                                (uint8_t EndpointAddress, uint8_t* pBuffer, uint32_t Length);
        SystemState_e           SetStall                                (uint8_t EndpointAddress);
        SystemState_e           ClearStall                              (uint8_t EndpointAddress);
        SystemState_e           SetEndpointStatus                       (uint8_t EndpointAddress, uint16_t Status);
        SystemState_e           GetEndpointStatus                       (uint8_t EndpointAddress, uint16_t* pStatus) const;
        SystemState_e           Abort                                   (uint8_t EndpointAddress);
        SystemState_e           Flush                                   (uint8_t EndpointAddress);
        SystemState_e           ActivateRemoteWakeup                    (void);
        SystemState_e           DeActivateRemoteWakeup                  (void);
        USB_State_e             GetState                                (void);
        SystemState_e           ConfigurePMA                            (uint16_t EndpointAddress, uint16_t BufferMode, uint32_t PMA_Address);
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
        SystemState_e           ActivateBCD                             (void);
        SystemState_e           DeActivateBCD                           (void);
        void                    DetectBatteryCharging                   (void);
#endif
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        SystemState_e           ActivateLPM                             (void);
        SystemState_e           DeActivateLPM                           (void);
#endif

        const uint8_t*          GetSetup                                (void) const { return reinterpret_cast<const uint8_t*>(m_Setup); }
        const USB_Endpoint_t*   GetEndpoint                             (uint8_t EndpointAddress) const;
        uint32_t                GetErrorCode                            (void) const { return m_ErrorCode; }
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        USB_LPM_State_e         GetLPM_State                            (void) const { return m_LPM_State; }
        uint32_t                GetBESL                                 (void) const { return m_BESL; }
        bool                    IsLPM_Active                            (void) const { return m_LPM_Active != 0U; }
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
        bool                    IsBCD_Active                            (void) const { return m_BCD_Active != 0U; }
#endif

    protected:

        virtual void            HardwareInitialize                      (void);
        virtual void            HardwareDeInitialize                    (void);
        virtual void            DataOutStageCallback                    (uint8_t Endpoint);
        virtual void            DataInStageCallback                     (uint8_t Endpoint);
        virtual void            SetupStageCallback                      (void);
        virtual void            SOFCallback                             (void);
        virtual void            ResetCallback                           (void);
        virtual void            SuspendCallback                         (void);
        virtual void            ResumeCallback                          (void);
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
        virtual void            ISOOUTIncompleteCallback                (uint8_t Endpoint);
        virtual void            ISOINIncompleteCallback                 (uint8_t Endpoint);
#endif
        virtual void            ConnectCallback                         (void);
        virtual void            DisconnectCallback                      (void);
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        virtual void            LPM_Callback                            (USB_LPM_Message_e Message);
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
        virtual void            BCD_Callback                            (USB_BCD_Message_e Message);
#endif

    private:

        SystemState_e           EndpointIRQHandler                      (void);
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        uint16_t                ReceiveDoubleBuffer                     (USB_Endpoint_t* pEndpoint, uint16_t EndpointValue);
        SystemState_e           TransmitDoubleBuffer                    (USB_Endpoint_t* pEndpoint, uint16_t EndpointValue);
#endif
        SystemState_e           EnableInterrupts                        (void);
        SystemState_e           DisableInterrupts                       (void);
        SystemState_e           InitializeDevice                        (void);
        SystemState_e           ActivateEndpoint                        (USB_Endpoint_t* pEndpoint);
        SystemState_e           DeactivateEndpoint                      (USB_Endpoint_t* pEndpoint);
        SystemState_e           StartTransfer                           (USB_Endpoint_t* pEndpoint);
        SystemState_e           SetEndpointStall                        (USB_Endpoint_t* pEndpoint);
        SystemState_e           ClearEndpointStall                      (USB_Endpoint_t* pEndpoint);
        SystemState_e           StopTransfer                            (USB_Endpoint_t* pEndpoint);
        SystemState_e           StopDevice                              (void);
        SystemState_e           SetDeviceAddress                        (uint8_t Address);
        SystemState_e           ConnectDevice                           (void);
        SystemState_e           DisconnectDevice                        (void);
        uint32_t                ReadInterrupts                          (void);
        SystemState_e           StartRemoteWakeup                       (void);
        SystemState_e           StopRemoteWakeup                        (void);
        void                    WritePMA                                (const uint8_t* pBuffer, uint16_t PMA_Address, uint16_t Length);
        void                    ReadPMA                                 (uint8_t* pBuffer, uint16_t PMA_Address, uint16_t Length);
        void                    WriteEndpointRegister                   (uint16_t Endpoint, uint16_t Value);
        uint16_t                ReadEndpointRegister                    (uint16_t Endpoint);
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        void                    ReleaseBuffer                           (uint16_t Endpoint, uint16_t Direction);
#endif
        void                    SetEndpointTxStatus                     (uint16_t Endpoint, uint16_t State);
        void                    SetEndpointRxStatus                     (uint16_t Endpoint, uint16_t State);
        void                    SetEndpointKind                         (uint16_t Endpoint);
        void                    ClearEndpointKind                       (uint16_t Endpoint);
        void                    SetOutStatus                            (uint16_t Endpoint);
        void                    ClearOutStatus                          (uint16_t Endpoint);
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        void                    SetBulkDoubleBuffer                     (uint16_t Endpoint);
        void                    ClearBulkDoubleBuffer                   (uint16_t Endpoint);
#endif
        void                    ClearEndpointRxEvent                    (uint16_t Endpoint);
        void                    ClearEndpointTxEvent                    (uint16_t Endpoint);
        void                    ToggleEndpointRx                        (uint16_t Endpoint);
        void                    ToggleEndpointTx                        (uint16_t Endpoint);
        void                    ClearEndpointRxToggle                   (uint16_t Endpoint);
        void                    ClearEndpointTxToggle                   (uint16_t Endpoint);
        void                    SetEndpointAddress                      (uint16_t Endpoint, uint16_t Address);
        volatile uint16_t*      GetEndpointTxCountRegister              (uint16_t Endpoint);
        volatile uint16_t*      GetEndpointRxCountRegister              (uint16_t Endpoint);
        void                    SetEndpointTxAddress                    (uint16_t Endpoint, uint16_t Address);
        void                    SetEndpointRxAddress                    (uint16_t Endpoint, uint16_t Address);
        void                    SetEndpointRxBuffer0Count               (uint16_t Endpoint, uint16_t Count);
        void                    SetEndpointTxCount                      (uint16_t Endpoint, uint16_t Count);
        void                    SetEndpointRxCount                      (uint16_t Endpoint, uint16_t Count);
        uint16_t                GetEndpointTxCount                      (uint16_t Endpoint);
        uint16_t                GetEndpointRxCount                      (uint16_t Endpoint);
        void                    SetEndpointBuffer0Address               (uint16_t Endpoint, uint16_t Address0);
        void                    SetEndpointBuffer1Address               (uint16_t Endpoint, uint16_t Address1);
#if (DIGINI_USE_USB_DOUBLE_BUFFER == DEF_ENABLED)
        void                    SetEndpointBufferAddress                (uint16_t Endpoint, uint16_t Address0, uint16_t Address1);
        void                    SetEndpointBuffer0Count                 (uint16_t Endpoint, uint16_t Direction, uint16_t Count);
        void                    SetEndpointBuffer1Count                 (uint16_t Endpoint, uint16_t Direction, uint16_t Count);
        void                    SetEndpointBufferCount                  (uint16_t Endpoint, uint16_t Direction, uint16_t Count);
        uint16_t                GetEndpointBuffer0Count                 (uint16_t Endpoint);
        uint16_t                GetEndpointBuffer1Count                 (uint16_t Endpoint);
#endif
        void                    SetReceiveCountRegister                 (volatile uint16_t* pRegister, uint16_t Count);

        bool                    Lock                                    (void);
        void                    Unlock                                  (void);
        bool                    IsEndpointValid                         (uint8_t EndpointAddress) const;
        bool                    IsPMA_Valid                             (const USB_Endpoint_t* pEndpoint, uint16_t MaxPacketSize) const;
        void                    Notify                                  (CallbackInterface* pCallback, USB_Callback_e Event, uint32_t Value);

        USB_TypeDef*         m_pUSB;
        USB_Info_t           m_Info;
        volatile uint8_t     m_Address;
        USB_Endpoint_t       m_InEndpoint[USB_ENDPOINT_COUNT];
        USB_Endpoint_t       m_OutEndpoint[USB_ENDPOINT_COUNT];
        volatile bool        m_Locked;
        volatile USB_State_e m_State;
        volatile uint32_t    m_ErrorCode;
        uint32_t             m_Setup[12];
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        USB_LPM_State_e      m_LPM_State;
        uint32_t             m_BESL;
#endif
        uint32_t             m_FrameNumber;
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        uint32_t             m_LPM_Active;
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
        uint32_t             m_BCD_Active;
#endif
        CallbackInterface*   m_pSOFCallback;
        CallbackInterface*   m_pSetupStageCallback;
        CallbackInterface*   m_pResetCallback;
        CallbackInterface*   m_pSuspendCallback;
        CallbackInterface*   m_pResumeCallback;
        CallbackInterface*   m_pConnectCallback;
        CallbackInterface*   m_pDisconnectCallback;
        CallbackInterface*   m_pHardwareInitializeCallback;
        CallbackInterface*   m_pHardwareDeInitializeCallback;
        CallbackInterface*   m_pDataOutStageCallback;
        CallbackInterface*   m_pDataInStageCallback;
#if (DIGINI_USE_USB_ISOCHRONOUS == DEF_ENABLED)
        CallbackInterface*   m_pISOOUTIncompleteCallback;
        CallbackInterface*   m_pISOINIncompleteCallback;
#endif
#if (DIGINI_USE_USB_LPM == DEF_ENABLED)
        CallbackInterface*   m_pLPMCallback;
#endif
#if (DIGINI_USE_USB_BCD == DEF_ENABLED)
        CallbackInterface*   m_pBCDCallback;
#endif
};

#endif // (USE_USB_DRIVER == DEF_ENABLED) && defined(STM32L4xx)
