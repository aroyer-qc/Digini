//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_i2c.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define I2C_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  I2C_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define I2C_TIME_OUT                100                         // 100 Milliseconds

//-------------------------------------------------------------------------------------------------
//
//   Class: I2C_Driver
//
//   Description:   Class to handle I2C_Driver
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   I2C_Driver
//
//   Parameter(s):  I2C_ID_e
//
//   Description:   Initialize the I2Cx peripheral according to the specified Parameters
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
I2C_Driver::I2C_Driver(I2C_ID_e I2C_ID)
{
    m_IsItInitialize = false;
    m_pInfo          = const_cast<I2C_Info_t*>(&I2C_Info[I2C_ID]);
    m_Device         = -1;
    m_State          = SYS_DEVICE_NOT_PRESENT;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   None
//  Return:         none
//
//  Description:    It start the initialize process from configuration in struct
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::Initialize(void)
{
    uint16_t            Result;
    uint16_t            Register;
    uint16_t            FreqRange;
    nOS_Error           Error;
    uint32_t            PriorityGroup;
    I2C_TypeDef*        pI2Cx;

    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;
    	Error = nOS_MutexCreate(&this->m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
        VAR_UNUSED(Error);
    }

    pI2Cx     = m_pInfo->pI2Cx;
    m_Timeout = 0;

    NVIC_DisableIRQ(m_pInfo->EV_IRQn);
    NVIC_DisableIRQ(m_pInfo->ER_IRQn);

    // ---- GPIO configuration ----
    RCC->APB1RSTR |=  m_pInfo->RCC_APB1_En;             // Reset I2C
    RCC->APB1RSTR &= ~m_pInfo->RCC_APB1_En;             // Release reset signal of I2C
    RCC->APB1ENR  |=  m_pInfo->RCC_APB1_En;

    IO_PinInit(m_pInfo->SCL);
    IO_PinInit(m_pInfo->SDA);

    // ---- Configure event interrupt ----
    PriorityGroup = NVIC_GetPriorityGrouping();
    NVIC_SetPriority(m_pInfo->EV_IRQn, NVIC_EncodePriority(PriorityGroup, 5, 0));
    NVIC_SetPriority(m_pInfo->ER_IRQn, NVIC_EncodePriority(PriorityGroup, 5, 0));

    // ---- Peripheral software reset ----
    //pI2Cx->CR1  =  I2C_CR1_SWRST;                                                                   // Peripheral software reset
    //pI2Cx->CR1 &= ~I2C_CR1_SWRST;

    // Configure I2C module Frequency
    pI2Cx->TIMINGR = m_pInfo->Timing;

    // ---- I2Cx CR1 Configuration ----
    pI2Cx->CR1 |= I2C_CR1_PE;                                                                       // Enable the selected I2C peripheral

    // ---- Enable I2C event interrupt ----
    NVIC_EnableIRQ(m_pInfo->EV_IRQn);
    NVIC_EnableIRQ(m_pInfo->ER_IRQn);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      GetStatus
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Return general status of the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::GetStatus(void)
{
    return m_State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockToDevice
//
//  Parameter(s):   uint8_t        Device
//  Return:         SystemState_e  State
//
//  Description:    This routine will configure the I2C port to work with a specific device and
//                  lock it, so any other access to the port will be block until unlock
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::LockToDevice(uint8_t Device)
{
    SystemState_e State = SYS_NOT_LOCK_TO_DEVICE;
    nOS_StatusReg   sr;

    nOS_EnterCritical(sr);

    if(m_Device == -1)
    {
        m_Device = Device;
        this->Lock();
        State = SYS_READY;
    }

    nOS_LeaveCritical(sr);
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UnlockFromDevice
//
//  Parameter(s):   uint8_t         Device
//  Return:         SystemState_e   State
//
//  Description:    This routine will unlock I2C port from a specific device
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//                  if lock and no write at all if not lock to a device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::UnlockFromDevice(uint8_t Device)
{
    SystemState_e State = SYS_WRONG_DEVICE;
    nOS_StatusReg   sr;

    nOS_EnterCritical(sr);

    if(Device == m_Device)
    {
        this->Unlock();
        m_Device = -1;
        State = SYS_READY;
    }
    else if(m_Device == -1)
    {
        State = SYS_NOT_LOCK_TO_DEVICE;
    }

    nOS_LeaveCritical(sr);
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Transfer
//
//  Parameter(s):   uint32_t        Address
//                  uint32_t        AddressSize
//                  void*           pTxBuffer
//                  size_t          TxSize
//                  void*           pRxBuffer
//                  size_t          RxSize
//
//  Return:         SystemState_e   State
//
//  Description:    Read or writes data from/to I2C device.
//
//  Note(s):        This is the overloaded version for multiple operation on the device
//                  Use must call LockToDevice() prior of using this function.
//                  Call to UnlockFromDevice() after multiple operation are done
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Transfer(uint32_t Address, uint32_t AddressSize, const void* pTxBuffer, size_t TxSize, const void* pRxBuffer, size_t RxSize)
{
    uint32_t        RegisterCR1;
    uint32_t        RegisterCR2;
    nOS_TickCounter TimeOut;
    I2C_TypeDef*    pI2Cx;

    if(m_Device != -1)
    {
		pI2Cx      = m_pInfo->pI2Cx;
		m_State    = SYS_BUSY;
        pI2Cx->CR1 = 0;                        // Stop I2C module

        // Setup I2C transfer record
        m_Address     = Address;
        m_AddressSize = AddressSize;
        m_TxSize      = (pTxBuffer != nullptr) ? TxSize : 0;   // If TX buffer is null, this make sure size is 0
        m_RxSize      = (pRxBuffer != nullptr) ? RxSize : 0;   // If RX buffer is null, this make sure size is 0
        m_pTxBuffer   = (uint8_t*)pTxBuffer;
        m_pRxBuffer   = (uint8_t*)pRxBuffer;

        if((m_TxSize != 0) || (m_AddressSize != 0))
        {
            RegisterCR1 = I2C_CR1_PE     |
                          I2C_CR1_TXIE   |
                          I2C_CR1_TCIE   |
                          I2C_CR1_STOPIE;

            RegisterCR2 = ((m_TxSize + m_AddressSize) << 16)  | // Size of transfer in NBYTE
                          ((uint32_t)m_Device << 1);            // Device slave address

            if(m_RxSize == 0)                                   // if we have data to receive then IRQ on reload
            {
                RegisterCR2 |= I2C_CR2_AUTOEND;                 // STOP condition is automatically sent when NBYTES data are transferred.
            }
            else
            {
                RegisterCR1 |= I2C_CR1_RXIE;
            }

            pI2Cx->CR1  = RegisterCR1;
            pI2Cx->CR2  = RegisterCR2;
        }
        else // RX Only
        {
            pI2Cx->CR1 = I2C_CR1_PE    |
                         I2C_CR1_RXIE  |
                         I2C_CR1_STOPIE;
            pI2Cx->CR2 = I2C_CR2_AUTOEND   |
                         I2C_CR2_RD_WRN    |
                         (m_RxSize << 16)  |
                         ((uint32_t)m_Device << 1);    // Device slave address
        }

        pI2Cx->CR2 |= I2C_CR2_START;

        // Wait here until I2C transaction is completed or time out
        TimeOut = GetTick();
        do
        {
            nOS_Yield();

            if(TickHasTimeOut(TimeOut, I2C_TIME_OUT) == true)
            {
                // We need to reset module if I2C is jammed or in error
                this->Initialize();
                m_State = SYS_TIME_OUT;
            }
        }
        while(((pI2Cx->ISR & I2C_ISR_TC) == 0) && (m_State == SYS_BUSY));

        return m_State;
    }

    return SYS_NOT_LOCK_TO_DEVICE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Transfer
//
//  Parameter(s):   uint32_t        Address
//                  uint32_t        AddressSize
//                  void*           pTxBuffer
//                  size_t          TxSize
//                  void*           pRxBuffer
//                  size_t          RxSize
//                  uint8_t         Device
//
//  Return:         SystemState_e   State
//
//  Description:    Read or writes data to I2C device.
//
//  Note(s):        This is the overloaded version for single operation on the device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Transfer(uint32_t Address, uint32_t AddressSize, const void* pTxBuffer, size_t TxSize, const void* pRxBuffer, size_t RxSize, uint8_t Device)
{
    SystemState_e State;

    if((State = this->LockToDevice(Device)) == SYS_READY)
    {
        State = this->Transfer(Address, AddressSize, pTxBuffer, TxSize, pRxBuffer, RxSize);
        this->UnlockFromDevice(Device);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   void*           pBuffer
//                  size_t          Size
//
//  Return:         SystemState_e   State
//
//  Description:    writes data to I2C device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Write(const void* pBuffer, size_t Size)
{
    return Transfer(0, 0, pBuffer, Size, nullptr, 0);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   void*           pBuffer
//                  size_t          Size
//                  uint8_t         Device
//
//  Return:         SystemState_e   State
//
//  Description:    writes data to I2C device.
//
//  Note(s):        This is the overloaded version for multiple operation on the device
//                  Use must call LockToDevice() prior of using this function.
//                  Call to UnlockFromDevice() after multiple operation are done
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Write(const void* pBuffer, size_t Size, uint8_t Device)
{
    return Transfer(0, 0, pBuffer, Size, nullptr, 0, Device);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Read
//
//  Parameter(s):   void*           pBuffer
//                  size_t          Size
//
//  Return:         SystemState_e   State
//
//  Description:    Read data from I2C device.
//
//  Note(s):        This is the overloaded version for multiple operation on the device
//                  Use must call LockToDevice() prior of using this function.
//                  Call to UnlockFromDevice() after multiple operation are done
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Read(const void* pBuffer, size_t Size)
{
    return Transfer(0, 0, nullptr, 0, pBuffer, Size);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Read
//
//  Parameter(s):   void*           pBuffer
//                  size_t          Size
//                  uint8_t         Device
//
//  Return:         SystemState_e   State
//
//  Description:    Read data from I2C device.
//
//  Note(s):        This is the overloaded version for multiple operation on the device
//                  Use must call LockToDevice() prior of using this function.
//                  Call to UnlockFromDevice() after multiple operation are done
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Read(const void* pBuffer, size_t Size, uint8_t Device)
{
    return Transfer(0, 0, nullptr, 0, pBuffer, Size, Device);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Lock the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::Lock(void)
{
    while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK){};
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Unlock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Unlock the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::Unlock(void)
{
    nOS_MutexUnlock(&m_Mutex);
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    EV_IRQHandler
//
//  Description:    This function handles I2Cx Event interrupt request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::EV_IRQHandler()
{
    uint32_t     Status;
    I2C_TypeDef* pI2Cx;

    pI2Cx  = m_pInfo->pI2Cx;

    Status = pI2Cx->ISR;                                            // Get I2C Status

    if(Status & I2C_ISR_ARLO)                                       // --- Master Lost Arbitration ---
    {
        m_State  = SYS_ARBITRATION_LOST;                                // Set transfer status as Arbitration Lost
        pI2Cx->ICR = I2C_ICR_ARLOCF;                                    // Clear Status Flags
    }
    else if(Status & I2C_ISR_BERR)                                  // --- Master Start Stop Error ---
    {
        m_State  = SYS_BUS_ERROR;                                       // Set transfer status as Bus Error
        pI2Cx->ICR = I2C_ICR_BERRCF;                                    // Clear Status Flags
    }

    if(pI2Cx->CR1 & I2C_CR1_TXIE)
    {
        if(Status & I2C_ISR_TXE)                                    // ... Master Transmit available ...
        {
            if(m_AddressSize != 0)
            {
                pI2Cx->TXDR = uint8_t(m_Address >> ((m_AddressSize - 1) << 3));
                m_AddressSize--;
            }
            else if(m_TxSize == 0)
            {
                pI2Cx->CR1  &= ~I2C_CR1_TXIE;
                if(m_RxSize != 0)
                {
                    pI2Cx->CR2 &= ~I2C_CR2_NBYTES;                      // Flush the NBYTES
                    pI2Cx->CR2 |= (I2C_CR2_START  |
                                   I2C_CR2_RD_WRN |
                                   (m_RxSize << 16));                   // Request a read
                }
                else
                {
                    pI2Cx->CR2 |= I2C_CR2_STOP;
                }
            }
            else
            {
                pI2Cx->TXDR = *m_pTxBuffer;                            // If TX data available transmit data and continue
                m_pTxBuffer++;
                m_TxSize--;
            }
        }
    }

    if(Status & I2C_ISR_TC)
    {
        pI2Cx->CR2 |= I2C_CR2_AUTOEND;
    }

    if(pI2Cx->CR1 & I2C_CR1_RXIE)
    {
        if(Status & I2C_ISR_RXNE)                                   // ... Master Receive data available ...
        {
            *m_pRxBuffer++ = pI2Cx->RXDR;
        }
    }

    if(Status & I2C_ISR_STOPF)                                      // ... STOP is sent
    {
        m_State     = SYS_READY;
        pI2Cx->CR1 &= ~(uint32_t)I2C_CR1_STOPIE;
        pI2Cx->ICR  =  I2C_ICR_STOPCF;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    ER_IRQHandler
//
//  Description:    This function is called in I2C_EV_IRQHandler
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::ER_IRQHandler()
{
    m_Timeout  = 0;
    m_State    = SYS_READY;                                 // We're done!
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
