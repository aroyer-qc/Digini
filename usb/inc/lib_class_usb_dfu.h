//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_dfu.h
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 DFU.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#pragma once

#if (USE_USB_DRIVER == DEF_ENABLED) && \
    ((USE_USB_DFU_DRIVER == DEF_ENABLED) || (USE_USB_DFU_RUNTIME_DRIVER == DEF_ENABLED))

#include "lib_class_usb_class_base.h"

class USB_DFU_Base : public USB_ClassBase
{
    protected:

                            USB_DFU_Base            (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity, bool Runtime);
        void                ConfigureClassEndpoints (void) override;
        const uint8_t*      GetConfiguration        (uint16_t* pLength) const override;
        void                HandleClassEndpointRx   (uint8_t Endpoint) override;
        void                HandleClassEndpointTx   (uint8_t Endpoint) override;
        bool                IsEndpointAvailable     (uint8_t Endpoint, bool In) const override;
        void                ProcessClass            (void) override;

    private:

        uint8_t             m_Descriptor[27];
};

#if (USE_USB_DFU_RUNTIME_DRIVER == DEF_ENABLED)

class USB_DFU_RuntimeInterface
{
    public:

        virtual void        Detach                  (uint16_t TimeoutMilliseconds) = 0;

    protected:

        virtual            ~USB_DFU_RuntimeInterface(void) = default;
};

class USB_DFU_RuntimeDriver : public USB_DFU_Base
{
    public:

                            USB_DFU_RuntimeDriver   (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
                                                     USB_DFU_RuntimeInterface& Application);

    protected:

        void                ResetClassState         (void) override;
        bool                HandleClassSetup        (uint8_t Type, uint8_t Request, uint16_t Value,
                                                     uint16_t Index, uint16_t Length) override;
        void                ControlReceiveComplete  (uint16_t Length) override;
        void                ControlTransmitComplete (void) override;
        void                CancelClassControl      (void) override;
        void                ProcessTask             (void) override;

    private:

        USB_DFU_RuntimeInterface& m_Application;
        uint16_t            m_Timeout;
        bool                m_DetachAcknowledgement;
        volatile bool       m_DetachPending;
        bool                m_Detaching;
};

#endif

#if (USE_USB_DFU_DRIVER == DEF_ENABLED)

static_assert((USB_DFU_TRANSFER_SIZE >= 64) && (USB_DFU_TRANSFER_SIZE <= 4096) &&
              ((USB_DFU_TRANSFER_SIZE % 64) == 0), "DFU transfer size must be a multiple of 64, from 64 to 4096");

struct USB_DFU_Target_t
{
    uint32_t StartAddress;
    uint32_t Size;
    uint32_t EraseSize;
    uint32_t PollMilliseconds;
    const char* pMemoryLayout;
};

class USB_DFU_MemoryInterface
{
    public:

        virtual bool        Read                    (uint32_t Address, uint8_t* pData, uint16_t Length) = 0;
        virtual bool        Write                   (uint32_t Address, const uint8_t* pData, uint16_t Length) = 0;
        virtual bool        Erase                   (uint32_t Address) = 0;
        virtual bool        Manifest                (void) = 0;

    protected:

        virtual            ~USB_DFU_MemoryInterface (void) = default;
};

class USB_DFU_Driver : public USB_DFU_Base
{
    public:

                            USB_DFU_Driver          (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
                                                     USB_DFU_MemoryInterface& Memory, const USB_DFU_Target_t& Target);

    protected:

        bool                InitializeClass         (void) override;
        void                ResetClassState         (void) override;
        bool                GetClassDescriptor      (uint16_t Value, uint16_t Requested) override;
        void                HandleClassControlOut   (const uint8_t* pData, uint16_t Length) override;
        bool                HandleClassSetup        (uint8_t Type, uint8_t Request, uint16_t Value,
                                                     uint16_t Index, uint16_t Length) override;
        void                ControlReceiveComplete  (uint16_t Length) override;
        void                ControlTransmitComplete (void) override;
        void                CancelClassControl      (void) override;
        void                ProcessTask             (void) override;

    private:

        enum State_e { Idle = 2, DownloadSync, DownloadBusy, DownloadIdle, ManifestSync,
                       Manifest, WaitReset, UploadIdle, Error };
        enum Action_e { NoAction, SetAddress, EraseMemory, WriteMemory, ManifestMemory, ReadMemory };

        bool                IsRangeValid            (uint32_t Address, uint32_t Length) const;
        bool                GetBlockAddress         (uint16_t Block, uint32_t* pAddress) const;
        bool                RejectRequest           (uint8_t Status);
        void                SendStatus              (void);

        USB_DFU_MemoryInterface& m_Memory;
        USB_DFU_Target_t    m_Target;
        volatile State_e    m_State;
        uint8_t             m_Error;
        uint16_t            m_BlockNumber;
        uint16_t            m_Length;
        uint32_t            m_AddressPointer;
        uint32_t            m_OperationAddress;
        uint8_t             m_Buffer[USB_DFU_TRANSFER_SIZE];
        uint8_t             m_WorkBuffer[USB_DFU_TRANSFER_SIZE];
        volatile Action_e   m_Action;
        bool                m_AcknowledgeAction;
        volatile bool       m_ActionReady;
        uint32_t            m_Generation;
        uint32_t            m_ControlGeneration;
};

#endif
#endif
