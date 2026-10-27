//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_usb_dfu.cpp
//
//  Copyright (c) 2015 STMicroelectronics. All rights reserved.
//  C++ adaptation (c) 2026 Alain Royer.
//  Adapted from stm32-mw-usb-device v2.11.6 DFU.
//  Licensed under SLA0044 Rev6; see ../LICENSE_ST_USB.md.
//  Use and execution restricted to STMicroelectronics processing units.
//
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

#if (USE_USB_DRIVER == DEF_ENABLED) && \
    ((USE_USB_DFU_DRIVER == DEF_ENABLED) || (USE_USB_DFU_RUNTIME_DRIVER == DEF_ENABLED))

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_DFU_Base
//  Parameter(s):   Backend, Identity, Runtime
//  Return:         None
//  Description:    Construct a standalone DFU 1.1 interface using EP0 only.
//
//-------------------------------------------------------------------------------------------------
USB_DFU_Base::USB_DFU_Base(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
    bool Runtime) : USB_ClassBase(Backend, Identity)
{
    const uint8_t Descriptor[] = {9, 2, 27, 0, 1, 1, 0, 0x80, 50,
        9, 4, 0, 0, 0, 0xFE, 1, uint8_t(Runtime ? 1 : 2), uint8_t(Runtime ? 0 : 4),
        9, 0x21, 0x0B, 0xE8, 3, uint8_t(USB_DFU_TRANSFER_SIZE),
        uint8_t(USB_DFU_TRANSFER_SIZE >> 8), uint8_t(Runtime ? 0x10 : 0x1A), 1};
    static_assert(sizeof(Descriptor) == 27, "Invalid DFU configuration length");
    memcpy(m_Descriptor, Descriptor, sizeof(Descriptor));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ConfigureClassEndpoints
//  Parameter(s):   None
//  Return:         None
//  Description:    No non-control endpoint is required by DFU.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Base::ConfigureClassEndpoints(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetConfiguration
//  Parameter(s):   pLength
//  Return:         const uint8_t*
//  Description:    Return the instance-owned runtime or DfuSe configuration.
//
//-------------------------------------------------------------------------------------------------
const uint8_t* USB_DFU_Base::GetConfiguration(uint16_t* pLength) const
{
    *pLength = sizeof(m_Descriptor);
    return m_Descriptor;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassEndpointRx
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Reject impossible non-control endpoint notifications.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Base::HandleClassEndpointRx(uint8_t Endpoint)
{
    (void)Endpoint;
    StallControl();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassEndpointTx
//  Parameter(s):   Endpoint
//  Return:         None
//  Description:    Reject impossible non-control endpoint notifications.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Base::HandleClassEndpointTx(uint8_t Endpoint)
{
    (void)Endpoint;
    StallControl();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsEndpointAvailable
//  Parameter(s):   Endpoint, In
//  Return:         bool
//  Description:    DFU exposes no non-control endpoints.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_Base::IsEndpointAvailable(uint8_t Endpoint, bool In) const
{
    (void)Endpoint;
    (void)In;
    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ProcessClass
//  Parameter(s):   None
//  Return:         None
//  Description:    Memory and application operations execute only in ProcessTask.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Base::ProcessClass(void)
{
}

#if (USE_USB_DFU_RUNTIME_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_DFU_RuntimeDriver
//  Parameter(s):   Backend, Identity, Application
//  Return:         None
//  Description:    Bind the application-owned transition to DFU mode.
//
//-------------------------------------------------------------------------------------------------
USB_DFU_RuntimeDriver::USB_DFU_RuntimeDriver(USB_DeviceBackend& Backend,
    const USB_DeviceIdentity_t& Identity, USB_DFU_RuntimeInterface& Application)
    : USB_DFU_Base(Backend, Identity, true), m_Application(Application)
{
    ResetClassState();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResetClassState
//  Parameter(s):   None
//  Return:         None
//  Description:    Restore appIDLE and discard an unacknowledged detach request.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_RuntimeDriver::ResetClassState(void)
{
    m_Timeout = 0;
    m_DetachAcknowledgement = false;
    m_DetachPending = false;
    m_Detaching = false;
    USB_ClassBase::CancelClassControl();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassSetup
//  Parameter(s):   Type, Request, Value, Index, Length
//  Return:         bool
//  Description:    Support runtime DETACH, GETSTATUS and GETSTATE only.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_RuntimeDriver::HandleClassSetup(uint8_t Type, uint8_t Request, uint16_t Value,
    uint16_t Index, uint16_t Length)
{
    if(Index != 0)
    {
        return false;
    }
    if((Type == 0x81) && (Request == 6) && (Value == 0x2100) && (Length != 0))
    {
        uint16_t DescriptorLength;
        const uint8_t* pDescriptor = GetConfiguration(&DescriptorLength);
        ControlSend(pDescriptor + 18, 9, Length);
        return true;
    }
    if(!m_Configured)
    {
        return false;
    }
    if((Type == 0x21) && (Request == 0) && (Length == 0) && (Value <= 1000) && !m_Detaching)
    {
        m_Timeout = Value;
        m_DetachAcknowledgement = true;
        ControlSend(nullptr, 0, 0);
    }
    else if((Type == 0xA1) && (Request == 3) && (Value == 0) && (Length == 6))
    {
        memset(m_ControlBuffer, 0, 6);
        m_ControlBuffer[4] = m_Detaching ? 1 : 0;
        ControlSend(m_ControlBuffer, 6, Length);
    }
    else if((Type == 0xA1) && (Request == 5) && (Value == 0) && (Length == 1))
    {
        m_ControlBuffer[0] = m_Detaching ? 1 : 0;
        ControlSend(m_ControlBuffer, 1, Length);
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
//  Description:    Runtime DFU never accepts control OUT payloads.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_RuntimeDriver::ControlReceiveComplete(uint16_t Length)
{
    (void)Length;
    StallControl();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ControlTransmitComplete
//  Parameter(s):   None
//  Return:         None
//  Description:    Defer detach until the host has acknowledged its status packet.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_RuntimeDriver::ControlTransmitComplete(void)
{
    if(m_DetachAcknowledgement)
    {
        m_DetachAcknowledgement = false;
        m_Detaching = true;
        m_DetachPending = true;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CancelClassControl
//  Parameter(s):   None
//  Return:         None
//  Description:    Cancel only detach requests whose status was not acknowledged.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_RuntimeDriver::CancelClassControl(void)
{
    USB_ClassBase::CancelClassControl();
    m_DetachAcknowledgement = false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ProcessTask
//  Parameter(s):   None
//  Return:         None
//  Description:    Let the application select its bootloader; never jump to an imposed address.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_RuntimeDriver::ProcessTask(void)
{
    uint32_t State = __get_PRIMASK();
    __disable_irq();
    bool Pending = m_DetachPending;
    uint16_t Timeout = m_Timeout;
    m_DetachPending = false;
    __set_PRIMASK(State);
    if(Pending)
    {
        m_Application.Detach(Timeout);
    }
}
#endif

#if (USE_USB_DFU_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Function:       USB_DFU_Driver
//  Parameter(s):   Backend, Identity, Memory, Target
//  Return:         None
//  Description:    Bind one explicitly bounded application memory target.
//
//-------------------------------------------------------------------------------------------------
USB_DFU_Driver::USB_DFU_Driver(USB_DeviceBackend& Backend, const USB_DeviceIdentity_t& Identity,
    USB_DFU_MemoryInterface& Memory, const USB_DFU_Target_t& Target)
    : USB_DFU_Base(Backend, Identity, false), m_Memory(Memory)
{
    m_Target = Target;
    m_Generation = 0;
    m_ControlGeneration = 0;
    ResetClassState();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       InitializeClass
//  Parameter(s):   None
//  Return:         bool
//  Description:    Validate the advertised memory map and polling timeout before connecting.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_Driver::InitializeClass(void)
{
    return USB_ClassBase::InitializeClass() && (m_Target.Size != 0) &&
           (m_Target.Size <= UINT32_MAX - m_Target.StartAddress) && (m_Target.EraseSize != 0) &&
           ((m_Target.StartAddress % m_Target.EraseSize) == 0) &&
           ((m_Target.Size % m_Target.EraseSize) == 0) && (m_Target.pMemoryLayout != nullptr) &&
           (strlen(m_Target.pMemoryLayout) <= 63) && (m_Target.PollMilliseconds != 0) &&
           (m_Target.PollMilliseconds <= 0xFFFFFF);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ResetClassState
//  Parameter(s):   None
//  Return:         None
//  Description:    Invalidate pending task completions and return to dfuIDLE.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Driver::ResetClassState(void)
{
    m_Generation++;
    m_State = Idle;
    m_Error = 0;
    m_BlockNumber = 0;
    m_Length = 0;
    m_AddressPointer = m_Target.StartAddress;
    m_OperationAddress = m_Target.StartAddress;
    m_Action = NoAction;
    m_ActionReady = false;
    CancelClassControl();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetClassDescriptor
//  Parameter(s):   Value, Requested
//  Return:         bool
//  Description:    Add the application's DfuSe memory-layout string.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_Driver::GetClassDescriptor(uint16_t Value, uint16_t Requested)
{
    return Value == 0x0304 ? SendString(m_Target.pMemoryLayout, Requested) :
           USB_ClassBase::GetClassDescriptor(Value, Requested);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassControlOut
//  Parameter(s):   pData, Length
//  Return:         None
//  Description:    Report malformed download packets through the DFU status state machine.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Driver::HandleClassControlOut(const uint8_t* pData, uint16_t Length)
{
    USB_ClassBase::HandleClassControlOut(pData, Length);
    if(m_Backend.IsEndpointStalled(0, USB_DEVICE_DIRECTION_OUT))
    {
        RejectRequest(0x0F);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IsRangeValid
//  Parameter(s):   Address, Length
//  Return:         bool
//  Description:    Check ranges by subtraction to prevent integer wraparound.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_Driver::IsRangeValid(uint32_t Address, uint32_t Length) const
{
    return (Address >= m_Target.StartAddress) &&
           (Address - m_Target.StartAddress <= m_Target.Size) &&
           (Length <= m_Target.Size - (Address - m_Target.StartAddress));
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetBlockAddress
//  Parameter(s):   Block, pAddress
//  Return:         bool
//  Description:    Apply the DfuSe block-two origin without overflowing a physical address.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_Driver::GetBlockAddress(uint16_t Block, uint32_t* pAddress) const
{
    if(Block < 2)
    {
        return false;
    }
    uint32_t Offset = uint32_t(Block - 2U) * USB_DFU_TRANSFER_SIZE;
    if(Offset > UINT32_MAX - m_AddressPointer)
    {
        return false;
    }
    *pAddress = m_AddressPointer + Offset;
    return IsRangeValid(*pAddress, 0);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       RejectRequest
//  Parameter(s):   Status
//  Return:         bool
//  Description:    Enter dfuERROR and explicitly stall a malformed or unsafe request.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_Driver::RejectRequest(uint8_t Status)
{
    m_Generation++;
    m_Error = Status;
    m_State = Error;
    m_Action = NoAction;
    m_ActionReady = false;
    StallControl();
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SendStatus
//  Parameter(s):   None
//  Return:         None
//  Description:    Advance synchronization states and advertise the application's busy poll time.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Driver::SendStatus(void)
{
    if(m_State == DownloadSync)
    {
        m_State = m_Action == NoAction ? DownloadIdle : DownloadBusy;
    }
    else if(m_State == ManifestSync)
    {
        m_State = Manifest;
        m_Action = ManifestMemory;
    }
    bool Busy = (m_State == DownloadBusy) || (m_State == Manifest);
    if(Busy && !m_ActionReady)
    {
        m_AcknowledgeAction = true;
    }
    uint32_t Poll = Busy ? m_Target.PollMilliseconds : 0;
    m_ControlBuffer[0] = m_Error;
    m_ControlBuffer[1] = uint8_t(Poll);
    m_ControlBuffer[2] = uint8_t(Poll >> 8);
    m_ControlBuffer[3] = uint8_t(Poll >> 16);
    m_ControlBuffer[4] = uint8_t(m_State);
    m_ControlBuffer[5] = 0;
    ControlSend(m_ControlBuffer, 6, 6);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       HandleClassSetup
//  Parameter(s):   Type, Request, Value, Index, Length
//  Return:         bool
//  Description:    Validate DFU state transitions and queue memory requests without executing them.
//
//-------------------------------------------------------------------------------------------------
bool USB_DFU_Driver::HandleClassSetup(uint8_t Type, uint8_t Request, uint16_t Value,
    uint16_t Index, uint16_t Length)
{
    if(Index != 0)
    {
        return false;
    }
    if((Type == 0x81) && (Request == 6) && (Value == 0x2100) && (Length != 0))
    {
        uint16_t DescriptorLength;
        const uint8_t* pDescriptor = GetConfiguration(&DescriptorLength);
        ControlSend(pDescriptor + 18, 9, Length);
        return true;
    }
    if(!m_Configured)
    {
        return false;
    }
    if((Type == 0xA1) && (Request == 3) && (Value == 0) && (Length == 6))
    {
        SendStatus();
        return true;
    }
    if((Type == 0xA1) && (Request == 5) && (Value == 0) && (Length == 1))
    {
        m_ControlBuffer[0] = uint8_t(m_State);
        ControlSend(m_ControlBuffer, 1, 1);
        return true;
    }
    if((Type == 0x21) && (Value == 0) && (Length == 0) &&
       (((Request == 4) && (m_State == Error)) ||
        ((Request == 6) && ((m_State == Idle) || (m_State == DownloadSync) ||
                           (m_State == DownloadIdle) || (m_State == UploadIdle)))))
    {
        m_Generation++;
        m_State = Idle;
        m_Error = 0;
        m_Action = NoAction;
        m_ActionReady = false;
        CancelClassControl();
        ControlSend(nullptr, 0, 0);
        return true;
    }
    if((Type == 0x21) && (Request == 1) && ((m_State == Idle) || (m_State == DownloadIdle)))
    {
        if(Length == 0)
        {
            m_State = ManifestSync;
            ControlSend(nullptr, 0, 0);
        }
        else if((Length <= USB_DFU_TRANSFER_SIZE) && (Value != 1))
        {
            m_BlockNumber = Value;
            m_Length = Length;
            BeginControlReceive(m_Buffer, Length, sizeof(m_Buffer));
        }
        else
        {
            return RejectRequest(0x0F);
        }
        return true;
    }
    if((Type == 0xA1) && (Request == 2) && ((m_State == Idle) || (m_State == UploadIdle)) &&
       (Length <= USB_DFU_TRANSFER_SIZE))
    {
        if(Length == 0)
        {
            m_State = Idle;
            ControlSend(nullptr, 0, 0);
        }
        else if(Value == 0)
        {
            static const uint8_t Commands[] = {0, 0x21, 0x41};
            m_State = Length > sizeof(Commands) ? Idle : UploadIdle;
            ControlSend(Commands, sizeof(Commands), Length);
        }
        else if(GetBlockAddress(Value, &m_OperationAddress))
        {
            m_BlockNumber = Value;
            m_Length = Length;
            m_Action = ReadMemory;
            m_ActionReady = true;
        }
        else
        {
            return RejectRequest(8);
        }
        return true;
    }
    if((Type == 0x21) || (Type == 0xA1))
    {
        return RejectRequest(0x0F);
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ControlReceiveComplete
//  Parameter(s):   Length
//  Return:         None
//  Description:    Decode bounded DfuSe commands or a complete firmware download block.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Driver::ControlReceiveComplete(uint16_t Length)
{
    m_Length = Length;
    if(m_BlockNumber == 0)
    {
        if((Length == 1) && (m_Buffer[0] == 0))
        {
            m_Action = NoAction;
        }
        else if((Length == 5) && ((m_Buffer[0] == 0x21) || (m_Buffer[0] == 0x41)))
        {
            m_OperationAddress = uint32_t(m_Buffer[1]) | (uint32_t(m_Buffer[2]) << 8) |
                                 (uint32_t(m_Buffer[3]) << 16) | (uint32_t(m_Buffer[4]) << 24);
            bool Erase = m_Buffer[0] == 0x41;
            if(!IsRangeValid(m_OperationAddress, Erase ? m_Target.EraseSize : 1) ||
               (Erase && (((m_OperationAddress - m_Target.StartAddress) % m_Target.EraseSize) != 0)))
            {
                RejectRequest(8);
                return;
            }
            m_Action = Erase ? EraseMemory : SetAddress;
        }
        else
        {
            RejectRequest(0x0F);
            return;
        }
    }
    else if(GetBlockAddress(m_BlockNumber, &m_OperationAddress) && IsRangeValid(m_OperationAddress, Length))
    {
        m_Action = WriteMemory;
    }
    else
    {
        RejectRequest(8);
        return;
    }
    m_ActionReady = false;
    m_State = DownloadSync;
    ControlSend(nullptr, 0, 0);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ControlTransmitComplete
//  Parameter(s):   None
//  Return:         None
//  Description:    Execute a busy operation only after GETSTATUS has reached the host.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Driver::ControlTransmitComplete(void)
{
    if(m_AcknowledgeAction)
    {
        m_AcknowledgeAction = false;
        m_ActionReady = true;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CancelClassControl
//  Parameter(s):   None
//  Return:         None
//  Description:    Cancel incomplete EP0 work while preserving acknowledged flash operations.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Driver::CancelClassControl(void)
{
    USB_ClassBase::CancelClassControl();
    m_ControlGeneration++;
    m_AcknowledgeAction = false;
    if(m_Action == ReadMemory)
    {
        m_Action = NoAction;
        m_ActionReady = false;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ProcessTask
//  Parameter(s):   None
//  Return:         None
//  Description:    Perform application memory operations with interrupts enabled and discard stale completions.
//
//-------------------------------------------------------------------------------------------------
void USB_DFU_Driver::ProcessTask(void)
{
    uint32_t InterruptState = __get_PRIMASK();
    __disable_irq();
    if(!m_ActionReady)
    {
        __set_PRIMASK(InterruptState);
        return;
    }
    Action_e Action = m_Action;
    uint32_t Generation = m_Generation;
    uint32_t ControlGeneration = m_ControlGeneration;
    uint32_t Address = m_OperationAddress;
    uint16_t Length = m_Length;
    uint16_t Requested = Length;
    if(Action == ReadMemory)
    {
        uint32_t Available = m_Target.Size - (Address - m_Target.StartAddress);
        if(Length > Available)
        {
            Length = uint16_t(Available);
        }
    }
    else if(Action == WriteMemory)
    {
        memcpy(m_WorkBuffer, m_Buffer, Length);
    }
    __set_PRIMASK(InterruptState);

    bool Result = true;
    if(Action == ReadMemory)
    {
        Result = (Length == 0) || m_Memory.Read(Address, m_WorkBuffer, Length);
    }
    else if(Action == WriteMemory)
    {
        Result = m_Memory.Write(Address, m_WorkBuffer, Length);
    }
    else if(Action == EraseMemory)
    {
        Result = m_Memory.Erase(Address);
    }
    else if(Action == ManifestMemory)
    {
        Result = m_Memory.Manifest();
    }

    __disable_irq();
    if((Generation == m_Generation) &&
       ((Action != ReadMemory) || (ControlGeneration == m_ControlGeneration)))
    {
        m_ActionReady = false;
        m_AcknowledgeAction = false;
        m_Action = NoAction;
        if(!Result)
        {
            m_Error = Action == EraseMemory ? 4 : (Action == WriteMemory ? 3 : 0x0B);
            m_State = Error;
            if(Action == ReadMemory)
            {
                StallControl();
            }
        }
        else if(Action == ReadMemory)
        {
            memcpy(m_Buffer, m_WorkBuffer, Length);
            m_State = Length < Requested ? Idle : UploadIdle;
            ControlSend(m_Buffer, Length, Requested);
        }
        else if(Action == ManifestMemory)
        {
            m_State = WaitReset;
        }
        else
        {
            if(Action == SetAddress)
            {
                m_AddressPointer = Address;
            }
            m_State = DownloadSync;
        }
    }
    __set_PRIMASK(InterruptState);
}
#endif
#endif
