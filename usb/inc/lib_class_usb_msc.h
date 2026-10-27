//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_msc.h
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 MSC BOT and SCSI.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#pragma once

#if (USE_USB_DRIVER == DEF_ENABLED) && (USE_USB_MSC_DRIVER == DEF_ENABLED)

#include "lib_class_usb_class_base.h"

class USB_MSC_StorageInterface
{
    public:

        virtual bool        IsReady                 (void) = 0;
        virtual bool        IsReadOnly              (void) = 0;
        virtual bool        GetCapacity             (uint32_t* pBlockCount) = 0;
        virtual bool        ReadBlock               (uint32_t Block, uint8_t* pData) = 0;
        virtual bool        WriteBlock              (uint32_t Block, const uint8_t* pData) = 0;
        virtual bool        Synchronize             (void) = 0;
        virtual bool        StartStop               (bool Start, bool Eject) = 0;

    protected:

        virtual            ~USB_MSC_StorageInterface(void) = default;
};

class USB_MSC_Driver : public USB_ClassBase
{
    public:

                            USB_MSC_Driver          (USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
                                                     USB_MSC_StorageInterface& Storage);

    protected:

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

    private:

        enum Stage_e { Command, Decode, ReadBlock, SendBlock, ReceiveBlock, WriteBlock,
                       SendReply, SendZLP, SendStatus, WaitHalt, Recovery };

        void                DecodeCommand           (void);
        void                SendNextPacket          (void);
        void                SendCommandStatus       (uint8_t Status);
        void                FailCommand             (uint8_t Key, uint8_t Code);
        void                PhaseError              (void);
        void                SendReplyData           (uint16_t Length, uint32_t Allocation);
        bool                CheckDataPhase          (bool In, uint32_t Length);

        //-----------------------------------------------------------------------------------------
        //
        //  Function:       CallStorage
        //  Parameter(s):   Callback, pResult
        //  Return:         bool - transport generation is still current
        //  Description:    Unlock IRQs around application code, then reject reset-time completions.
        //
        //-----------------------------------------------------------------------------------------
        template<typename Action>
        bool CallStorage(Action Callback, bool* pResult)
        {
            __set_PRIMASK(0);
            *pResult = Callback();
            __disable_irq();
            return (m_TaskGeneration == m_Generation) && m_Configured;
        }

        USB_MSC_StorageInterface& m_Storage;
        volatile Stage_e    m_Stage;
        uint8_t             m_Command[31];
        uint8_t             m_Block[512];
        uint8_t             m_Status[13];
        uint32_t            m_Residue;
        uint32_t            m_Address;
        uint32_t            m_Blocks;
        uint16_t            m_Offset;
        uint16_t            m_PacketLength;
        uint16_t            m_ReplyLength;
        uint8_t             m_SenseKey;
        uint8_t             m_SenseCode;
        bool                m_PreventRemoval;
        bool                m_Ejected;
        bool                m_ResetRecovery;
        uint32_t            m_Generation;
        uint32_t            m_TaskGeneration;
};

#endif
