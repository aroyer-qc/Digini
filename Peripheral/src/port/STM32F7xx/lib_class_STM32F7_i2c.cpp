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

//#define I2C_ADDRESS_7_BITS                          ((uint16_t)0x4000)
//#define I2C_ADDRESS_10_BITS                         ((uint16_t)0xC000)

//#define CR1_CLEAR_MASK                              ((uint16_t)0xFBF5)          // I2C registers Masks
#define FLAG_MASK                                   ((uint32_t)0x00FFBFFF)      // I2C FLAG mask
#define I2C_TIME_OUT                                100                         // 100 Milliseconds
//#define I2C_NACK                                    ((uint32_t)0x00001000)
//#define I2C_EVENT_MASTER_MODE_SELECT                ((uint32_t)0x00030001)      // BUSY, MSL and SB flag
//#define I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED  ((uint32_t)0x00070082)      // BUSY, MSL, ADDR, TXE and TRA flags
//#define I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED     ((uint32_t)0x00030002)      // BUSY, MSL and ADDR flags
//#define I2C_EVENT_MASTER_BYTE_RECEIVED              ((uint32_t)0x00030040)      // BUSY, MSL and RXNE flags
//#define I2C_EVENT_MASTER_BYTE_TRANSMITTING          ((uint32_t)0x00070080)      // TRA, BUSY, MSL, TXE flags
//#define I2C_EVENT_MASTER_BYTE_TRANSMITTED           ((uint32_t)0x00070084)      // TRA, BUSY, MSL, TXE and BTF flags
//#define I2C_DEVICE_READ                             0x01

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
//   Description:   Initializes the I2C_ID peripheral according to the specified Parameters
//
//-------------------------------------------------------------------------------------------------
I2C_Driver::I2C_Driver(I2C_ID_e I2C_ID)
{
    m_IsItInitialize = false;
    m_pInfo          = &I2C_Info[I2C_ID];
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
    ISR_Prio_t          ISR_Prio;
    uint32_t            PriorityGroup;
    I2C_TypeDef*        pI2Cx;

    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;
        Error = nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
        VAR_UNUSED(Error);
    }

    pI2Cx     = m_pInfo->pI2Cx;
    m_Timeout = 0;

    NVIC_DisableIRQ(m_pInfo->EV_IRQn);
    NVIC_DisableIRQ(m_pInfo->ER_IRQn);

    // ---- Module configuration ----
    RCC->APB1RSTR |=  m_pInfo->RCC_APB1_En;             // Reset I2C
    RCC->APB1RSTR &= ~m_pInfo->RCC_APB1_En;             // Release reset signal of I2C
    RCC->APB1ENR  |=  m_pInfo->RCC_APB1_En;

    // ---- GPIO configuration ----
    IO_PinInit(m_pInfo->SCL);
    IO_PinInit(m_pInfo->SDA);

/* replaced by timing
    // ---- I2Cx CR2 Configuration ----
    FreqRange  = (uint16_t)(SYS_APB1_CLOCK_FREQUENCY / 1000000);
    Register   = pI2Cx->CR2;                                                                        // Get the I2Cx CR2 value
    Register  &= (uint16_t)~(I2C_CR2_FREQ);                                                         // Clear frequency FREQ[5:0] bits
    Register  |= FreqRange;
    pI2Cx->CR2  = Register;                                                                         // Write to I2Cx CR2

    // ---- I2Cx CCR Configuration ----
    Register = pI2Cx->CCR;

    if(m_pInfo->Speed <= 100000)                                                                    // Configure speed in standard mode
    {
        Result = (uint16_t)(SYS_APB1_CLOCK_FREQUENCY / (m_pInfo->Speed << 1));                      // Standard mode speed calculate
        Result = AbsMin(Result, 4);                                                                 // Test if CCR value is under 0x04
        Register |= Result;                                                                         // Set speed value for standard mode
        pI2Cx->TRISE = FreqRange + 1;                                                               // Set Maximum Rise Time for standard mode
    }
    else                                                                                            // (m_pInfo->Speed <= 400000) Configure speed in fast mode
    {                                                                                               // To use the I2C at 400 KHz (in fast mode), the PCLK1 frequency (I2C peripheral input clock) must be a multiple of 10 MHz
        Result = (uint16_t)(SYS_APB1_CLOCK_FREQUENCY / (m_pInfo->Speed * 3));                       // Fast mode speed calculate: Tlow/Thigh = 2

        if((Result & I2C_CCR_CCR) == 0)                                                             // Test if CCR value is under 0x1
        {
            Result |= (uint16_t)0x0001;                                                             // Set minimum allowed value
        }

        Register |= (Result | I2C_CCR_FS);                                                          // Set speed value and set F/S bit for fast mode
        pI2Cx->TRISE = (((FreqRange * 300) / 1000) + 1);                                            // Set Maximum Rise Time for fast mode
    }

    pI2Cx->CCR  = Register;                                                                         // Write to I2Cx CCR
*/

    // ---- I2Cx CR1 Configuration ----
    pI2Cx->CR1 |= I2C_CR1_PE;                                                                       // Enable the selected I2C peripheral

    // ---- Enable I2C event interrupt ----
    PriorityGroup = NVIC_GetPriorityGrouping();
    ISR_Prio.PriorityGroup     = PriorityGroup;
    ISR_Prio.SubPriority       = 0;
    ISR_Prio.PremptionPriority = 5;

    // NVIC Setup for TX DMA channels interrupt request
    ISR_Init(m_pInfo->EV_IRQn, &ISR_Prio);

    // NVIC Setup for RX DMA channels interrupt request
    ISR_Init(m_pInfo->ER_IRQn, &ISR_Prio);
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
    SystemState_e  State = SYS_NOT_LOCK_TO_DEVICE;
    nOS_StatusReg  sr;

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
//
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
    nOS_StatusReg sr;

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
    TickCount_t  TickStart;
    I2C_TypeDef* pI2Cx;

    if(m_Device != -1)
	{
		pI2Cx   = m_pInfo->pI2Cx;
		m_State = SYS_BUSY;

		// Setup I2C transfer record
		//m_DeviceAddress = Device;
		m_Address       = Address;
		m_AddressSize   = AddressSize;
		m_pTxBuffer     = (uint8_t*)pTxBuffer;
		m_TxSize        = (pTxBuffer != nullptr) ? TxSize : 0;    // If TX buffer is null, this make sure size is 0
		m_pRxBuffer     = (uint8_t*)pRxBuffer;
		m_RxSize        = (pRxBuffer != nullptr) ? RxSize : 0;    // If RX buffer is null, this make sure size is 0

		// Enable the selected I2C interrupts
		pI2Cx->CR2 &= ~I2C_CR2_ITBUFEN;
		pI2Cx->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);

		// Generate a START condition
		pI2Cx->CR2 |= I2C_CR2_START;

		// Wait here until I2C transaction is completed or time out
		TickStart = GetTick();

		do
		{
			nOS_Yield();

			if(TickHasTimeOut(TickStart, I2C_TIME_OUT) == true)
			{
				// We need to reset module if I2C is jammed or in error
				this->Initialize();
				m_State = SYS_ERROR;
				return SYS_TIME_OUT;
			}
		}
		while(m_State == SYS_BUSY);

		return SYS_READY;
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
//                  uint8_t         DeviceAddress
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
//  Name:           ReadRegister
//
//  Parameter(s):   uint8_t         Register
//                  uint8_t*        pValue
//                  uint8_t         Device
//
//  Return:         SystemState_e   State
//
//  Description:    Read data from a specific register in the I2C device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::ReadRegister(uint8_t Register, uint8_t* pValue, uint8_t Device)
{
    return Transfer(0, 0, &Register, sizeof(uint8_t), pValue, sizeof(uint8_t), Device);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadRegister
//
//  Parameter(s):   uint8_t         Register
//                  uint8_t*        pValue
//
//  Return:         SystemState_e   State
//
//  Description:    Read data from a specific register in the I2C device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::ReadRegister(uint8_t Register, uint8_t* pValue)
{
    return Transfer(0, 0, &Register, sizeof(uint8_t), pValue, sizeof(uint8_t));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteRegister
//
//  Parameter(s):   uint8_t         Register
//                  uint8_t         Value
//
//  Return:         SystemState_e   State
//
//  Description:    Write data to a specific register in the I2C device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::WriteRegister(uint8_t Register, uint8_t Value, uint8_t Device)
{
    uint8_t Buffer[2];

    Buffer[0] = Register;
    Buffer[1] = Value;

    return Transfer(0, 0, &Buffer[0], 2, nullptr, 0, Device);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteRegister
//
//  Parameter(s):   uint8_t         Register
//                  uint8_t         Value
//
//  Return:         SystemState_e   State
//
//  Description:    Write data to a specific register in the I2C device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::WriteRegister(uint8_t Register, uint8_t Value)
{
    uint8_t Buffer[2];

    Buffer[0] = Register;
    Buffer[1] = Value;

    return Transfer(0, 0, &Buffer[0], 2, nullptr, 0);
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
//  Name:           RegisterCallback
//
//  Parameter(s):   pCallback       Callback pointer
//  Return:         None
//
//  Description:    Register callback for user code in ISR
//
//-------------------------------------------------------------------------------------------------
#if (I2C_ISR_CFG == DEF_ENABLED)
void I2C_Driver::RegisterCallback(CallbackInterface* pCallback)
{
    m_pCallback = pCallback;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableCallbackType
//
//  Parameter(s):   CallBackType    Type if the ISR callback
//                  pContext        Context for ISR
//  Return:         None
//
//  Description:    Enable the type of interrupt for the callback.
//
//-------------------------------------------------------------------------------------------------
#if (I2C_ISR_CFG == DEF_ENABLED)
void I2C_Driver::EnableCallbackType(int CallBackType, void* pContext)
{
    switch(CallBackType)
    {
      #if (I2C_ISR_MASTER_TX_CFG == DEF_ENABLED)
        case I2C_CALLBACK_MASTER_TX_COMPLETED:
        {
            m_pContextMasterTX    = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_MASTER_RX_CFG == DEF_ENABLED)
        case I2C_CALLBACK_MASTER_RX_COMPLETED:
        {
            m_pContextMasterRX    = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

    #if (I2C_ISR_SLAVE_TX_CFG == DEF_ENABLED)
        case I2C_CALLBACK_SLAVE_TX_COMPLETED:
        {
            m_pContextSlaveTX     = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_SLAVE_RX_CFG == DEF_ENABLED)
        case I2C_CALLBACK_SLAVE_RX_COMPLETED:
        {
            m_pContextSlaveRX     = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_ADRRESS_CFG == DEF_ENABLED)
        case I2C_CALLBACK_ADDRESS:
        {
            m_pContextAddress     = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_TX_LISTEN_CFG == DEF_ENABLED)
        case I2C_CALLBACK_LISTEN_COMPLETED:
        {
            m_pContextListen      = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_MEMORY_TX_CFG == DEF_ENABLED)
        case I2C_CALLBACK_MEMORY_TX_COMPLETED:
        {
            m_pContextMemoryTX    = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_MEMORY_RX_CFG == DEF_ENABLED)
        case I2C_CALLBACK_MEMORY_RX_COMPLETED:
        {
            m_pContextMemoryRX    = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_ERROR_CFG == DEF_ENABLED)
        case I2C_CALLBACK_ERROR:
        {
            m_pContextError       = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif

      #if (I2C_ISR_ABORT_CFG == DEF_ENABLED)
        case I2C_CALLBACK_ABORT:
        {
            m_pContextAbort       = pContext;
            m_CallBackType       |= CallBackType;
        }
        break;
      #endif
    }
}
#endif

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
    I2C_TypeDef* pI2Cx;
    uint32_t     Status;

    pI2Cx  = m_pInfo->pI2Cx;
    Status = uint32_t(pI2Cx->ISR) & FLAG_MASK;

    switch(Status)
    {
        // ---- Master Mode ----
        case I2C_EVENT_MASTER_MODE_SELECT:
        {
            if((m_TxSize != 0) || (m_AddressSize != 0))
            {
                pI2Cx->TDR = m_Device;                                  // Send slave address for write
            }
            else
            {
                pI2Cx->TDR = m_Device | I2C_DEVICE_READ;                // Send slave address for read
            }
        }
        break;

        // ---- Master Transmitter ----
        case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
        {
            pI2Cx->CR2 |= I2C_CR2_ITBUFEN;                              // Next we send data buffer, enable buffer interrupt
        }
        // Pass through in write so no "break" here

        case I2C_EVENT_MASTER_BYTE_TRANSMITTING:                        // Without BTF
        {
            if(m_AddressSize != 0)
            {
                pI2Cx->TDR = *(reinterpret_cast<uint8_t*>(m_Address) + (2 - m_AddressSize));
                m_AddressSize--;
            }
            else if(m_TxSize != 0)
            {
                pI2Cx->TDR = *m_pTxBuffer;                               // If TX data available transmit data and continue
                m_pTxBuffer++;
                m_TxSize--;
            }
            else
            {
                pI2Cx->CR2 &= ~I2C_CR2_ITBUFEN;                         // Done sending data before reading data
            }
        }
        break;

        case I2C_EVENT_MASTER_BYTE_TRANSMITTED:
        {
            if(m_RxSize != 0)                                           // If was transmitting address bytes before read
            {
                pI2Cx->CR2 |= I2C_CR2_START;                            // Generate a START condition to read data
            }
            else                                                        // If we are done transmitting\A0
            {
                pI2Cx->CR2 |= I2C_CR2_STOP;                             // Generate a STOP condition
                pI2Cx->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);     // Disable the selected I2C interrupts
                m_State = SYS_READY;
            }
        }
        break;

        // Master Receiver -----------------------------------------------------

        case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
        {
            if(m_RxSize == 1)
            {
                pI2Cx->CR1 &= ~I2C_CR1_ACK;                             // Disable the acknowledgement
                pI2Cx->CR1 |=  I2C_CR1_STOP;                            // Generate a STOP condition
            }

            pI2Cx->CR2 |= I2C_CR2_ITBUFEN;                              // Next we receive data buffer
        }
        break;

        case I2C_EVENT_MASTER_BYTE_RECEIVED:
        {
            if(m_RxSize > 0)                                            // If waiting for bytes?
            {
                *m_pRxBuffer = (uint8_t)pI2Cx->DR;                      // Store I2Cx received data
                m_pRxBuffer++;
                m_RxSize--;

                if(m_RxSize == 1)                                       // One more byte to go?
                {
                    pI2Cx->CR2 |= I2C_CR2_NACK;                         // Disable the acknowledgment
                    pI2Cx->CR2 |=  I2C_CR2_STOP;                        // Generate a STOP condition
                }
                else if(m_RxSize == 0)                                  // last byte received?
                {
                    pI2Cx->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN); // Disable the selected I2C interrupts
                    m_State = SYS_READY;                                // We're done!
                }
            }
            else                                                        // Not waiting for bytes, not supposed to be here
            {
                pI2Cx->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);     // Disable the selected I2C interrupts
                m_State = SYS_READY;                                    // We're done!
            }
        }
        break;

        default:                                                        // Not supposed to be here
        {
            // Reset all flags : Do not modify configuration here
            // Timeout will be generated
            Status = pI2Cx->SR1 | pI2Cx->SR2 | pI2Cx->DR;               // Dummy read
            pI2Cx->SR1 = 0;
        }
        break;
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
    I2C_TypeDef* pI2Cx;
    uint32_t     Status;

    pI2Cx  = m_pInfo->pI2Cx;
    Status = pI2Cx->SR1;

    if((Status & I2C_NO_ACK) != 0)
    {
        pI2Cx->CR2 |= I2C_CR2_STOP;
    }

    Status     = pI2Cx->SR1 | pI2Cx->SR2 | pI2Cx->RDR;      // Dummy read

    pI2Cx->SR1 = 0;                                         // After a  NACK, transfer is done
    m_State    = SYS_READY;                                 // We're done!
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
