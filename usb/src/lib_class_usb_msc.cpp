//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_msc.cpp
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 MSC BOT and SCSI.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

#if (USE_USB_DRIVER == DEF_ENABLED) && (USE_USB_MSC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_MSC_ReadBigEndian
//  Parameter(s):   pData
//  Return:         uint32_t
//  Description:    Decode SCSI fields without alignment or host byte-order assumptions.
//
//-------------------------------------------------------------------------------------------------
static uint32_t USB_MSC_ReadBigEndian(const uint8_t* pData)
{
    return (uint32_t(pData[0]) << 24) | (uint32_t(pData[1]) << 16) |
           (uint32_t(pData[2]) << 8) | pData[3];
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_MSC_ReadLittleEndian
//  Parameter(s):   pData
//  Return:         uint32_t
//  Description:    Decode a BOT wrapper field.
//
//-------------------------------------------------------------------------------------------------
static uint32_t USB_MSC_ReadLittleEndian(const uint8_t* pData)
{
    return uint32_t(pData[0]) | (uint32_t(pData[1]) << 8) |
           (uint32_t(pData[2]) << 16) | (uint32_t(pData[3]) << 24);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_MSC_WriteBigEndian
//  Parameter(s):   pData, Value
//  Return:         None
//  Description:    Encode a SCSI response field.
//
//-------------------------------------------------------------------------------------------------
static void USB_MSC_WriteBigEndian(uint8_t* pData, uint32_t Value)
{
    pData[0] = uint8_t(Value >> 24);
    pData[1] = uint8_t(Value >> 16);
    pData[2] = uint8_t(Value >> 8);
    pData[3] = uint8_t(Value);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_MSC_Driver
//  Parameter(s):   Backend, Identity, Storage
//  Return:         None
//  Description:    Bind a single-LUN, 512-byte block device without imposing any media.
//
//-------------------------------------------------------------------------------------------------
USB_MSC_Driver::USB_MSC_Driver(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
    USB_MSC_StorageInterface& Storage) : USB_ClassBase(Backend, Identity), m_Storage(Storage)
{
    m_Generation = 0;
    m_TaskGeneration = 0;
    ResetClassState();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResetClassState
//  Parameter(s):   None
//  Return:         None
//  Description:    Cancel transport state without performing storage operations in the ISR.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::ResetClassState(void)
{
    m_Generation++;
    CancelClassControl();
    m_Stage = Command;
    m_Residue = 0;
    m_Address = 0;
    m_Blocks = 0;
    m_Offset = 0;
    m_PacketLength = 0;
    m_ReplyLength = 0;
    m_SenseKey = 0;
    m_SenseCode = 0;
    m_PreventRemoval = false;
    m_Ejected = false;
    m_ResetRecovery = false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ConfigureClassEndpoints
//  Parameter(s):   None
//  Return:         None
//  Description:    Configure the two bulk pipes used by BOT.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::ConfigureClassEndpoints(void)
{
    ConfigureEndpoint(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_BULK, 64);
    ConfigureEndpoint(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_BULK, 64);
    SetEndpointState(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_NAK);
    EnableEndpointRx(1, 64);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetConfiguration
//  Parameter(s):   pLength
//  Return:         const uint8_t*
//  Description:    Advertise SCSI transparent command set with bulk-only transport.
//
//-------------------------------------------------------------------------------------------------
const uint8_t* USB_MSC_Driver::GetConfiguration(uint16_t* pLength) const
{
    static const uint8_t Descriptor[] = {9, 2, 32, 0, 1, 1, 0, 0x80, 50,
        9, 4, 0, 0, 2, 8, 6, 0x50, 0, 7, 5, 0x81, 2, 64, 0, 0, 7, 5, 1, 2, 64, 0, 0};
    static_assert(sizeof(Descriptor) == 32, "Invalid MSC configuration length");
    *pLength = sizeof(Descriptor);
    return Descriptor;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassSetup
//  Parameter(s):   Type, Request, Value, Index, Length
//  Return:         bool
//  Description:    Process GET_MAX_LUN and the mandatory BOT reset recovery sequence.
//
//-------------------------------------------------------------------------------------------------
bool USB_MSC_Driver::HandleClassSetup(uint8_t Type, uint8_t Request, uint16_t Value,
    uint16_t Index, uint16_t Length)
{
    if((m_Configured == false) || (Index != 0) || (Value != 0))
    {
        return false;
    }
    if((Type == 0xA1) && (Request == 0xFE) && (Length == 1))
    {
        m_ControlBuffer[0] = 0;
        ControlSend(m_ControlBuffer, 1, Length);
    }
    else if((Type == 0x21) && (Request == 0xFF) && (Length == 0))
    {
        m_Stage = Recovery;
        m_Generation++;
        m_ResetRecovery = true;
        SetEndpointState(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_STALL);
        SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
        ControlSend(nullptr, 0, 0);
    }
    else
    {
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ControlReceiveComplete
//  Parameter(s):   Length
//  Return:         None
//  Description:    MSC has no class-specific control OUT data stage.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::ControlReceiveComplete(uint16_t Length)
{
    (void)Length;
    StallControl();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassEndpointRx
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Validate CBW framing or assemble one bounded media block.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::HandleClassEndpointRx(uint8_t Endpoint)
{
    if(Endpoint != 1)
    {
        return;
    }
    uint16_t Length = GetEndpointRxLength(1);
    uint8_t Packet[64];
    if(Length > sizeof(Packet))
    {
        PhaseError();
        return;
    }
    ReadEndpointRx(1, Packet, Length);
    if(m_Stage == Command)
    {
        if((Length != 31) || (USB_MSC_ReadLittleEndian(Packet) != 0x43425355U) ||
           ((Packet[12] & 0x7F) != 0) || (Packet[13] != 0) || (Packet[14] == 0) || (Packet[14] > 16))
        {
            m_Generation++;
            m_Stage = Recovery;
            m_ResetRecovery = false;
            SetEndpointState(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_STALL);
            SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
            return;
        }
        memcpy(m_Command, Packet, 31);
        m_Residue = USB_MSC_ReadLittleEndian(Packet + 8);
        m_Stage = Decode;
    }
    else if(m_Stage == ReceiveBlock)
    {
        if(Length != 64)
        {
            PhaseError();
            return;
        }
        memcpy(m_Block + m_Offset, Packet, Length);
        m_Offset += Length;
        if(m_Offset == sizeof(m_Block))
        {
            m_Stage = WriteBlock;
        }
        else
        {
            if(!m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
            {
                EnableEndpointRx(1, 64);
            }
        }
    }
    else
    {
        PhaseError();
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassEndpointTx
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Account acknowledged data and advance the BOT IN phase.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::HandleClassEndpointTx(uint8_t Endpoint)
{
    if(Endpoint != 1)
    {
        return;
    }
    if(m_Stage == SendStatus)
    {
        if(m_Status[12] == 2)
        {
            m_Stage = Recovery;
            m_ResetRecovery = false;
        }
        else
        {
            m_Stage = Command;
            if(!m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
            {
                EnableEndpointRx(1, 64);
            }
        }
    }
    else if(m_Stage == SendZLP)
    {
        SendCommandStatus(m_Status[12]);
    }
    else if((m_Stage == SendBlock) || (m_Stage == SendReply))
    {
        m_Offset += m_PacketLength;
        m_Residue -= m_PacketLength;
        SendNextPacket();
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SendNextPacket
//  Parameter(s):   None
//  Return:         None
//  Description:    Split class data into backend-sized packets without borrowing stack storage.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::SendNextPacket(void)
{
    if(IsEndpointTxStalled(1))
    {
        return;
    }
    uint16_t Total = m_Stage == SendBlock ? 512 : m_ReplyLength;
    if(m_Offset < Total)
    {
        m_PacketLength = Total - m_Offset;
        if(m_PacketLength > 64)
        {
            m_PacketLength = 64;
        }
        Transmit(1, m_Block + m_Offset, m_PacketLength);
    }
    else if(m_Stage == SendBlock)
    {
        m_Address++;
        if(--m_Blocks != 0)
        {
            m_Stage = ReadBlock;
        }
        else
        {
            SendCommandStatus(0);
        }
    }
    else if((m_Residue != 0) && ((m_ReplyLength & 63) == 0))
    {
        m_Stage = SendZLP;
        m_Status[12] = 0;
        Transmit(1, nullptr, 0);
    }
    else
    {
        SendCommandStatus(0);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SendCommandStatus
//  Parameter(s):   Status
//  Return:         None
//  Description:    Emit a packed CSW with the original tag and unprocessed-byte residue.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::SendCommandStatus(uint8_t Status)
{
    m_Status[0] = 0x55;
    m_Status[1] = 0x53;
    m_Status[2] = 0x42;
    m_Status[3] = 0x53;
    memcpy(m_Status + 4, m_Command + 4, 4);
    for(uint8_t Byte = 0; Byte < 4; Byte++)
    {
        m_Status[8 + Byte] = uint8_t(m_Residue >> (Byte * 8));
    }
    m_Status[12] = Status;
    m_Stage = SendStatus;
    if(!IsEndpointTxStalled(1))
    {
        Transmit(1, m_Status, sizeof(m_Status));
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       FailCommand
//  Parameter(s):   Key, Code
//  Return:         None
//  Description:    Save sense information and terminate the data phase before the failed CSW.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::FailCommand(uint8_t Key, uint8_t Code)
{
    m_SenseKey = Key;
    m_SenseCode = Code;
    if((m_Residue == 0) || ((m_Stage == WriteBlock) && (m_Blocks == 1)))
    {
        SendCommandStatus(1);
    }
    else if((m_Command[12] & 0x80) != 0)
    {
        m_Stage = SendZLP;
        m_Status[12] = 1;
        if(!IsEndpointTxStalled(1))
        {
            Transmit(1, nullptr, 0);
        }
    }
    else
    {
        m_Stage = WaitHalt;
        m_Status[12] = 1;
        SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       PhaseError
//  Parameter(s):   None
//  Return:         None
//  Description:    Mark a host/device transfer disagreement; require reset after its CSW.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::PhaseError(void)
{
    m_Generation++;
    m_Status[12] = 2;
    m_Stage = WaitHalt;
    m_ResetRecovery = false;
    SetEndpointState(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_STALL);
    SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CheckDataPhase
//  Parameter(s):   In, Length
//  Return:         bool
//  Description:    Validate fixed-length block and no-data command phases.
//
//-------------------------------------------------------------------------------------------------
bool USB_MSC_Driver::CheckDataPhase(bool In, uint32_t Length)
{
    if((m_Residue != Length) || ((Length != 0) && (((m_Command[12] & 0x80) != 0) != In)))
    {
        PhaseError();
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SendReplyData
//  Parameter(s):   Length, Allocation
//  Return:         None
//  Description:    Respect both the CDB allocation length and CBW transfer direction.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::SendReplyData(uint16_t Length, uint32_t Allocation)
{
    uint32_t Count = Length < Allocation ? Length : Allocation;
    if((m_Residue < Count) || ((Count != 0) && ((m_Command[12] & 0x80) == 0)))
    {
        PhaseError();
        return;
    }
    m_ReplyLength = uint16_t(Count);
    m_Offset = 0;
    m_Stage = SendReply;
    SendNextPacket();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DecodeCommand
//  Parameter(s):   None
//  Return:         None
//  Description:    Execute SCSI discovery, media control and bounded block commands in task context.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::DecodeCommand(void)
{
    uint8_t CDB[16];
    memcpy(CDB, m_Command + 15, sizeof(CDB));
    const uint8_t* pCDB = CDB;
    bool Result;
    uint8_t Operation = pCDB[0];
    uint8_t Required = (Operation >= 0x80) ? ((Operation >= 0xA0) ? 12 : 16) :
                       ((Operation >= 0x20) ? 10 : 6);
    if(m_Command[14] < Required)
    {
        FailCommand(5, 0x24);
        return;
    }
    memset(m_Block, 0, sizeof(m_Block));
    if(Operation == 0x12)
    {
        if((pCDB[1] != 0) || (pCDB[2] != 0))
        {
            FailCommand(5, 0x24);
            return;
        }
        m_Block[1] = 0x80;
        m_Block[2] = 2;
        m_Block[3] = 2;
        m_Block[4] = 31;
        memcpy(m_Block + 8, "DIGINI  USB BLOCK DEVICE1.00", 28);
        SendReplyData(36, pCDB[4]);
        return;
    }
    if(Operation == 3)
    {
        m_Block[0] = 0x70;
        m_Block[2] = m_SenseKey;
        m_Block[7] = 10;
        m_Block[12] = m_SenseCode;
        m_SenseKey = 0;
        m_SenseCode = 0;
        SendReplyData(18, pCDB[4]);
        return;
    }
    if(Operation == 0xA0)
    {
        m_Block[3] = 8;
        SendReplyData(16, USB_MSC_ReadBigEndian(pCDB + 6));
        return;
    }
    if(Operation == 0x1E)
    {
        if(CheckDataPhase(false, 0))
        {
            m_PreventRemoval = (pCDB[4] & 1) != 0;
            SendCommandStatus(0);
        }
        return;
    }
    if(Operation == 0x1B)
    {
        if(CheckDataPhase(false, 0) == false)
        {
            return;
        }
        bool Start = (pCDB[4] & 1) != 0;
        bool Eject = (pCDB[4] & 2) != 0;
        if(Eject && !Start && m_PreventRemoval)
        {
            FailCommand(5, 0x53);
            return;
        }
        if(!CallStorage([&]() { return m_Storage.StartStop(Start, Eject); }, &Result))
        {
            return;
        }
        if(!Result)
        {
            FailCommand(5, 0x53);
            return;
        }
        if(Start)
        {
            m_Ejected = false;
        }
        else if(Eject)
        {
            m_Ejected = true;
        }
        SendCommandStatus(0);
        return;
    }
    if(!CallStorage([&]() { return m_Storage.IsReady(); }, &Result))
    {
        return;
    }
    if(m_Ejected || !Result)
    {
        FailCommand(2, 0x3A);
        return;
    }
    if((Operation == 0) || (Operation == 0x35))
    {
        if(CheckDataPhase(false, 0) == false)
        {
            return;
        }
        if(Operation == 0x35)
        {
            if(!CallStorage([&]() { return m_Storage.Synchronize(); }, &Result))
            {
                return;
            }
            if(!Result)
            {
                FailCommand(3, 0x0C);
                return;
            }
        }
        SendCommandStatus(0);
        return;
    }
    if((Operation == 0x1A) || (Operation == 0x5A))
    {
        if(((pCDB[2] & 0x3F) != 0x3F) && ((pCDB[2] & 0x3F) != 0))
        {
            FailCommand(5, 0x24);
            return;
        }
        bool Ten = Operation == 0x5A;
        m_Block[Ten ? 1 : 0] = Ten ? 6 : 3;
        if(!CallStorage([&]() { return m_Storage.IsReadOnly(); }, &Result))
        {
            return;
        }
        m_Block[Ten ? 3 : 2] = Result ? 0x80 : 0;
        SendReplyData(Ten ? 8 : 4, Ten ? (uint16_t(pCDB[7]) << 8) | pCDB[8] : pCDB[4]);
        return;
    }
    uint32_t Capacity = 0;
    if(!CallStorage([&]() { return m_Storage.GetCapacity(&Capacity); }, &Result))
    {
        return;
    }
    if(!Result || (Capacity == 0))
    {
        FailCommand(2, 0x3A);
        return;
    }
    if(Operation == 0x2F)
    {
        uint32_t Address = USB_MSC_ReadBigEndian(pCDB + 2);
        uint32_t Blocks = (uint16_t(pCDB[7]) << 8) | pCDB[8];
        if((pCDB[1] & 2) != 0)
        {
            FailCommand(5, 0x24);
        }
        else if((Address > Capacity) || (Blocks > Capacity - Address))
        {
            FailCommand(5, 0x21);
        }
        else if(CheckDataPhase(false, 0))
        {
            SendCommandStatus(0);
        }
    }
    else if(Operation == 0x25)
    {
        USB_MSC_WriteBigEndian(m_Block, Capacity - 1);
        USB_MSC_WriteBigEndian(m_Block + 4, 512);
        SendReplyData(8, 8);
    }
    else if(Operation == 0x23)
    {
        m_Block[3] = 8;
        USB_MSC_WriteBigEndian(m_Block + 4, Capacity);
        m_Block[8] = 2;
        m_Block[10] = 2;
        SendReplyData(12, (uint16_t(pCDB[7]) << 8) | pCDB[8]);
    }
    else if((Operation == 0x9E) && ((pCDB[1] & 31) == 0x10))
    {
        USB_MSC_WriteBigEndian(m_Block + 4, Capacity - 1);
        USB_MSC_WriteBigEndian(m_Block + 8, 512);
        SendReplyData(32, USB_MSC_ReadBigEndian(pCDB + 10));
    }
    else if((Operation == 0x28) || (Operation == 0x2A) || (Operation == 0xA8) || (Operation == 0xAA))
    {
        m_Address = USB_MSC_ReadBigEndian(pCDB + 2);
        m_Blocks = (Operation >= 0xA0) ? USB_MSC_ReadBigEndian(pCDB + 6) :
                   (uint16_t(pCDB[7]) << 8) | pCDB[8];
        if((m_Address > Capacity) || (m_Blocks > Capacity - m_Address))
        {
            FailCommand(5, 0x21);
            return;
        }
        uint64_t Bytes = uint64_t(m_Blocks) * 512U;
        bool In = (Operation == 0x28) || (Operation == 0xA8);
        if(Bytes > UINT32_MAX)
        {
            PhaseError();
        }
        else if(CheckDataPhase(In, uint32_t(Bytes)))
        {
            if(m_Blocks == 0)
            {
                SendCommandStatus(0);
            }
            else if(In)
            {
                m_Stage = ReadBlock;
            }
            else
            {
                if(!CallStorage([&]() { return m_Storage.IsReadOnly(); }, &Result))
                {
                    return;
                }
                if(Result)
                {
                    FailCommand(7, 0x27);
                    return;
                }
                m_Offset = 0;
                m_Stage = ReceiveBlock;
                if(!m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
                {
                    EnableEndpointRx(1, 64);
                }
            }
        }
    }
    else
    {
        FailCommand(5, 0x20);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleEndpointHaltCleared
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Enforce BOT recovery and resume a failed command after OUT halt clearing.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::HandleEndpointHaltCleared(uint8_t Endpoint)
{
    if(Endpoint != 1)
    {
        return;
    }
    if(m_Stage == Recovery)
    {
        if(m_ResetRecovery && !m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_IN) &&
           !m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
        {
            m_ResetRecovery = false;
            m_Stage = Command;
            EnableEndpointRx(1, 64);
        }
        else if(m_ResetRecovery == false)
        {
            SetEndpointState(1, USB_DEVICE_DIRECTION_IN, USB_DEVICE_ENDPOINT_STALL);
            SetEndpointState(1, USB_DEVICE_DIRECTION_OUT, USB_DEVICE_ENDPOINT_STALL);
        }
    }
    else if((m_Stage == WaitHalt) && !IsEndpointTxStalled(1) &&
            !m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
    {
        SendCommandStatus(m_Status[12]);
    }
    else if(IsEndpointTxReady(1))
    {
        if((m_Stage == SendBlock) || (m_Stage == SendReply))
        {
            SendNextPacket();
        }
        else if(m_Stage == SendStatus)
        {
            Transmit(1, m_Status, sizeof(m_Status));
        }
        else if(m_Stage == SendZLP)
        {
            Transmit(1, nullptr, 0);
        }
    }
    if(((m_Stage == Command) || (m_Stage == ReceiveBlock)) &&
       !m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
    {
        EnableEndpointRx(1, 64);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointAvailable
//  Parameter(s):   Endpoint, In
//  Return:         bool
//  Description:    Validate the single BOT endpoint pair.
//
//-------------------------------------------------------------------------------------------------
bool USB_MSC_Driver::IsEndpointAvailable(uint8_t Endpoint, bool In) const
{
    (void)In;
    return Endpoint == 1;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ProcessClass
//  Parameter(s):   None
//  Return:         None
//  Description:    Storage processing is reserved for task context.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::ProcessClass(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ProcessTask
//  Parameter(s):   None
//  Return:         None
//  Description:    Service one media operation while the relevant bulk endpoint remains NAKed.
//
//-------------------------------------------------------------------------------------------------
void USB_MSC_Driver::ProcessTask(void)
{
    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    m_TaskGeneration = m_Generation;
    bool Result;
    if(m_Stage == Decode)
    {
        DecodeCommand();
    }
    else if(m_Stage == ReadBlock)
    {
        uint32_t Address = m_Address;
        if(!CallStorage([&]() { return m_Storage.ReadBlock(Address, m_Block); }, &Result))
        {
            __set_PRIMASK(InterruptState);
            return;
        }
        if(!Result)
        {
            FailCommand(3, 0x11);
        }
        else
        {
            m_Offset = 0;
            m_Stage = SendBlock;
            SendNextPacket();
        }
    }
    else if(m_Stage == WriteBlock)
    {
        uint32_t Address = m_Address;
        if(!CallStorage([&]() { return m_Storage.WriteBlock(Address, m_Block); }, &Result))
        {
            __set_PRIMASK(InterruptState);
            return;
        }
        if(!Result)
        {
            FailCommand(3, 0x0C);
        }
        else
        {
            m_Residue -= 512;
            m_Address++;
            m_Blocks--;
            if(m_Blocks == 0)
            {
                if((m_Command[16] & 8) != 0)
                {
                    if(!CallStorage([&]() { return m_Storage.Synchronize(); }, &Result))
                    {
                        __set_PRIMASK(InterruptState);
                        return;
                    }
                    if(!Result)
                    {
                        FailCommand(3, 0x0C);
                        __set_PRIMASK(InterruptState);
                        return;
                    }
                }
                SendCommandStatus(0);
            }
            else
            {
                m_Offset = 0;
                m_Stage = ReceiveBlock;
                if(!m_Backend.IsEndpointStalled(1, USB_DEVICE_DIRECTION_OUT))
                {
                    EnableEndpointRx(1, 64);
                }
            }
        }
    }
    __set_PRIMASK(InterruptState);
}

#endif
