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
//
// Note(s):     Support only interrupt at this time
//
//
//       Those timing setting are from the RM0385 manual from ST. You can use STM32CubeMX to
//       calculate other values.
//       _______________________________________________________________________
//      |             |                         |             |                |
//      |  Parameter  |      Standard mode      |  Fast mode  | Fast mode plus |
//      |             |_________________________|_____________|________________|
//      |  fI2CCLK =  |            |            |             |                |
//      |    8 MHz    |  10 kHz    |  100 kHz   |   400 kHz   |      1 MHz     |
//      |_____________|____________|____________|_____________|________________|
//      |    PRESC    |    0x1     |   0x1      |    0x0      |      0x0       |
//      |    SCLDEL   |    0x4     |   0x4      |    0x3      |      0x1       |
//      |    SDADEL   |    0x2     |   0x2      |    0x1      |      0x0       |
//      |    SCLH     |    0xC3    |   0x0F     |    0x03     |      0x03      |
//      |    SCLL     |    0xC7    |   0x13     |    0x09     |      0x06      |
//      |_____________|____________|____________|_____________|________________|
//      |    TIMING   | 0x1042C7C3 | 0x1042130F | 0x00310309  |   0x00100306   |
//      |_____________|____________|____________|_____________|________________|
//
//       _______________________________________________________________________
//      |             |                         |             |                |
//      |  Parameter  |      Standard mode      |  Fast mode  | Fast mode plus |
//      |             |_________________________|_____________|________________|
//      |  fI2CCLK =  |            |            |             |                |
//      |    16 Mhz   |  10 kHz    |  100 kHz   |   400 kHz   |      1 MHz     |
//      |_____________|____________|____________|_____________|________________|
//      |    PRESC    |    0x3     |    0x3     |    0x1      |      0x0       |
//      |    SCLDEL   |    0x4     |    0x4     |    0x3      |      0x2       |
//      |    SDADEL   |    0x2     |    0x2     |    0x2      |      0x0       |
//      |    SCLH     |    0xC3    |    0x0F    |    0x03     |      0x03      |
//      |    SCLL     |    0xC7    |    0x13    |    0x09     |      0x06      |
//      |_____________|____________|____________|_____________|________________|
//      |    TIMING   | 0x3042C3C7 | 0x30420F13 | 0x10320309  |   0x00200306   |
//      |_____________|____________|____________|_____________|________________|
//
//       _______________________________________________________________________
//      |             |                         |             |                |
//      |  Parameter  |      Standard mode      |  Fast mode  | Fast mode plus |
//      |             |_________________________|_____________|________________|
//      |  fI2CCLK =  |            |            |             |                |
//      |    48 MHz   |  10 kHz    |  100 kHz   |   400 kHz   |      1 MHz     |
//      |_____________|____________|____________|_____________|________________|
//      |    PRESC    |    0xB     |   0xB      |    0x5      |      0x5       |
//      |    SCLDEL   |    0x4     |   0x4      |    0x3      |      0x1       |
//      |    SDADEL   |    0x2     |   0x2      |    0x3      |      0x0       |
//      |    SCLH     |    0xC3    |   0x0F     |    0x03     |      0x01      |
//      |    SCLL     |    0xC7    |   0x13     |    0x09     |      0x03      |
//      |_____________|____________|____________|_____________|________________|
//      |    TIMING   | 0xB042C3C7 | 0xB0420F13 | 0x50330309  |   0x50100103   |
//      |_____________|____________|____________|_____________|________________|
//
//       _______________________________________________________________________
//      |             |                         |             |                |
//      |  Parameter  |      Standard mode      |  Fast mode  | Fast mode plus |
//      |             |_________________________|_____________|________________|
//      |  fI2CCLK =  |            |            |             |                |
//      |    54 MHz   |  10 kHz    |  100 kHz   |   400 kHz   |      1 MHz     |
//      |_____________|____________|____________|_____________|________________|
//      |    PRESC    |    0xA     |   0x2      |    0x6      |      0x0       |
//      |    SCLDEL   |    0x1     |   0x4      |    0x0      |      0x2       |
//      |    SDADEL   |    0x0     |   0x0      |    0x0      |      0x0       |
//      |    SCLH     |    0xE9    |   0x47     |    0x03     |      0x09      |
//      |    SCLL     |    0xFF    |   0x68     |    0x0D     |      0x22      |
//      |_____________|____________|____________|_____________|________________|
//      |    TIMING   | 0xA010E9FF | 0x20404768 | 0x6000030D  |   0x50100922   |
//      |_____________|____________|____________|_____________|________________|
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

// uncommented is ok

#define I2C_STATE_IE_SHIFT          23

#define ISR_FLAG_MASK               ((uint32_t)(I2C_ISR_OVR_Msk    | I2C_ISR_BERR_Msk  | I2C_ISR_TCR_Msk  | I2C_ISR_TC_Msk     | \
                                                I2C_ISR_STOPF_Msk  | I2C_ISR_NACKF_Msk | I2C_ISR_ADDR_Msk | I2C_ISR_RXNE_Msk   | \
                                                I2C_ISR_TXIS_Msk | I2C_ISR_TXE_Msk))
#define ISR_SOURCE_FLAG_MASK        ((uint32_t)(I2C_CR1_ADDRIE_Msk | I2C_CR1_ERRIE_Msk | I2C_CR1_NACKIE_Msk | I2C_CR1_RXIE_Msk | \
                                                I2C_CR1_STOPIE_Msk | I2C_CR1_TCIE_Msk  | I2C_CR1_TXIE_Msk))
#define I2C_TIME_OUT                100                         // 100 Milliseconds


#define I2C_STATE_TX_EMPTY          ((uint32_t)(I2C_ISR_TXIS_Msk  | ( I2C_CR1_TXIE_Msk   << I2C_STATE_IE_SHIFT)))
#define I2C_STATE_RX_NOT_EMPTY      ((uint32_t)(I2C_ISR_RXNE_Msk  | ( I2C_CR1_RXIE_Msk   << I2C_STATE_IE_SHIFT)))
#define I2C_STATE_MASTER_RX_NACK    ((uint32_t)(I2C_ISR_NACKF_Msk | ( I2C_CR1_NACKIE_Msk << I2C_STATE_IE_SHIFT)))
#define I2C_STATE_MASTER_STOP       ((uint32_t)(I2C_ISR_STOPF_Msk | ( I2C_CR1_STOPIE_Msk << I2C_STATE_IE_SHIFT)))

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
    nOS_Error           Error;
    ISR_Prio_t          ISR_Prio;
    uint32_t            PriorityGroup;

    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;
        Error = nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
        VAR_UNUSED(Error);
    }

    m_pI2Cx   = m_pInfo->pI2Cx;
    m_Timeout = 0;

    NVIC_DisableIRQ(m_pInfo->EV_IRQn);
    NVIC_DisableIRQ(m_pInfo->ER_IRQn);

  #if (I2C_DRIVER_SUPPORT_CLK_SELECTION_CFG == DEF_ENABLED)
    uint32_t Register = RCC->DCKCFGR2;

    switch(uint32_t(m_pInfo->I2C_ID))
    {
      #if (I2C_DRIVER_SUPPORT_I2C1_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C1_ID):  Register &= ~RCC_DCKCFGR2_I2C1SEL_Msk; break;
      #endif
      #if (I2C_DRIVER_SUPPORT_I2C2_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C2_ID):  Register &= ~RCC_DCKCFGR2_I2C2SEL_Msk; break;
      #endif
      #if (I2C_DRIVER_SUPPORT_I2C3_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C3_ID):  Register &= ~RCC_DCKCFGR2_I2C3SEL_Msk; break;
      #endif
      #if (I2C_DRIVER_SUPPORT_I2C4_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C4_ID):  Register &= ~RCC_DCKCFGR2_I2C4SEL_Msk; break;
      #endif
        default: break;
    }

    Register      |= m_pInfo->ClockSelection;
    RCC->DCKCFGR2  = Register;
  #endif

    // ---- Module configuration ----
    RCC->APB1RSTR |=  m_pInfo->RCC_APB1_En;             // Reset I2C
    RCC->APB1RSTR &= ~m_pInfo->RCC_APB1_En;             // Release reset signal of I2C
    RCC->APB1ENR  |=  m_pInfo->RCC_APB1_En;

    // Disable I2C and all options
    m_pI2Cx->CR1 = 0;

    // ---- GPIO configuration ----
    IO_PinInit(m_pInfo->SCL);
    IO_PinInit(m_pInfo->SDA);

    // ---- TIMINGR Configuration ----
    m_pI2Cx->TIMINGR = m_pInfo->Timing;                                    // Configure the SDA setup, hold time, SCL high, low period, Frequency range

    // ---- OAR1 Configuration ----

  #if (I2C_DRIVER_SUPPORT_ADVANCED_MODE_CFG == DEF_ENABLED)

    m_pI2Cx->OAR1 = (I2C_OAR1_OA1EN | m_pInfo->OwnAddress_1)      // Configure OwnAddress1 and ack mode

    if(m_pInfo->AddressingMode == I2C_ADDRESSING_MODE_10_BIT)
    {
        m_pI2Cx->OAR1 |= I2C_OAR1_OA1MODE;
    }
  #else
    m_pI2Cx->OAR1 = 0;                                            // Disable Own Address 1
  #endif

    // ---- CR2 Configuration ----

    // Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process)
    m_pI2Cx->CR2 = (I2C_CR2_AUTOEND | I2C_CR2_NACK);

  #if (I2C_DRIVER_SUPPORT_ADVANCED_MODE_CFG == DEF_ENABLED)
    if(m_pInfo-AddressingMode == I2C_ADDRESSING_MODE_10_BIT)       // Addressing Master mode
    {
      m_pI2Cx->CR2 |= I2C_CR2_ADD10;
    }

    // ---- OAR2 Configuration
    m_pI2Cx->OAR2 &= ~I2C_DUAL_ADDRESS_ENABLE;                    // Disable Own Address2 before set the Own Address2 configuration

    m_pI2Cx->OAR2 = (m_pInfo->DualAddressMode | m_pInfo->OwnAddress2 | (m_pInfo->OwnAddress2Masks << 8));  // Configure Dual mode and Own Address2

    // ---- I2Cx CR1 Configuration ----
    m_pI2Cx->CR1 = (m_pInfo->GeneralCallMode | m_pInfo->NoStretchMode);   // GeneralCall and NoStretchMode
  #endif


// TODO in structure concatenate all definition that work on same register to save code size..

    // ---- I2Cx CR1 Configuration ----
    m_pI2Cx->CR1 |= I2C_CR1_PE;                                                                       // Enable the selected I2C peripheral

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
    uint32_t     RegisterCR1;
    uint32_t     RegisterCR2;
    TickCount_t  TickStart;

    if(m_Device != -1)
	{
		m_pI2Cx = m_pInfo->pI2Cx;
		m_State = SYS_BUSY;
        m_pI2Cx->CR1 = 0;                                    // Stop I2C module

		// Setup I2C transfer record
		m_Address       = Address;
		m_AddressSize   = AddressSize;
		m_pTxBuffer     = (uint8_t*)pTxBuffer;
		m_TxSize        = (pTxBuffer != nullptr) ? TxSize : 0;    // If TX buffer is null, this make sure size is 0
		m_pRxBuffer     = (uint8_t*)pRxBuffer;
		m_RxSize        = (pRxBuffer != nullptr) ? RxSize : 0;    // If RX buffer is null, this make sure size is 0

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

            m_pI2Cx->CR1  = RegisterCR1;
            m_pI2Cx->CR2  = RegisterCR2;
        }
        else // RX Only
        {
            m_pI2Cx->CR1 = I2C_CR1_PE  |
                        I2C_CR1_RXIE   |
                        I2C_CR1_STOPIE;
            m_pI2Cx->CR2 = I2C_CR2_AUTOEND |
                        I2C_CR2_RD_WRN     |
                        (m_RxSize << 16)   |
                        ((uint32_t)m_Device << 1);    // Device slave address
        }

        m_pI2Cx->CR2 |= I2C_CR2_START;

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
		while(((m_pI2Cx->ISR & I2C_ISR_TC) == false) && (m_State == SYS_BUSY));

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
//  Status is compose of this:
//
//      _________________________________________________________________________
//      |   31   |   30   |   29   |   28   |   27   |   26   |   25   |   24   |
//      |        |  ERRIE |  TCIE  | STOPIE | NACKIE | ADDRIE |  RXIE  |  TXIE  |
//      |________|________|________|________|________|________|________|________|
//      |   23       22       21       20       19       18       17   |   16   |
//      |                         ADDCODE [6:0]                        |        |
//      |________ ________ ________ ________ ________ ________ ________|________|
//      |   15   |   14   |   13   |   12   |   11   |   10   |    9   |    8   |
//      |        |        |        |        |        |   OVR  |  ARLO  |  BERR  |
//      |________|________|________|________|________|________|________|________|
//      |    7   |    6   |    5   |    4   |    3   |    2   |    1   |    0   |
//      |   TCR  |   TC   |  STOPF |  NACKF |  ADDR  |  RXNE  |  TXIS  |   TXE  |
//      |________|________|________|________|________|________|________|________|
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::EV_IRQHandler()
{
    uint32_t     Status;

    Status = m_pI2Cx->ISR;                                            // Get I2C Status

    if(Status & I2C_ISR_ARLO)                                       // --- Master Lost Arbitration ---
    {
        m_State  = SYS_ARBITRATION_LOST;                                // Set transfer status as Arbitration Lost
        m_pI2Cx->ICR = I2C_ICR_ARLOCF;                                    // Clear Status Flags
    }
    else if(Status & I2C_ISR_BERR)                                  // --- Master Start Stop Error ---
    {
        m_State  = SYS_BUS_ERROR;                                       // Set transfer status as Bus Error
        m_pI2Cx->ICR = I2C_ICR_BERRCF;                                    // Clear Status Flags
    }

    if(m_pI2Cx->CR1 & I2C_CR1_TXIE)
    {
        if(Status & I2C_ISR_TXE)                                    // ... Master Transmit available ...
        {
            if(m_AddressSize != 0)
            {
                m_pI2Cx->TXDR = uint8_t(m_Address >> ((m_AddressSize - 1) << 3));
                m_AddressSize--;
            }
            else if(m_TxSize == 0)
            {
                m_pI2Cx->CR1  &= ~I2C_CR1_TXIE;
                if(m_RxSize != 0)
                {
                    m_pI2Cx->CR2 &= ~I2C_CR2_NBYTES;                     // Flush the NBYTES
                    m_pI2Cx->CR2 |= (I2C_CR2_START |
                                  I2C_CR2_RD_WRN    |
                                  (m_RxSize << 16));                   // Request a read
                }
                else
                {
                    m_pI2Cx->CR2 |= I2C_CR2_STOP;
                }
            }
            else
            {
                m_pI2Cx->TXDR = *m_pTxBuffer;                            // If TX data available transmit data and continue
                m_pTxBuffer++;
                m_TxSize--;
            }
        }
    }

    if(Status & I2C_ISR_TC)
    {
        m_pI2Cx->CR2 |= I2C_CR2_AUTOEND;
    }

    if(m_pI2Cx->CR1 & I2C_CR1_RXIE)
    {
        if(Status & I2C_ISR_RXNE)                                   // ... Master Receive data available ...
        {
            *m_pRxBuffer++ = m_pI2Cx->RXDR;
        }
    }

    if(Status & I2C_ISR_STOPF)                                      // ... STOP is sent
    {
        m_State     = SYS_READY;
        m_pI2Cx->CR1 &= ~(uint32_t)I2C_CR1_STOPIE;
        m_pI2Cx->ICR  =  I2C_ICR_STOPCF;
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

    m_pI2Cx->CR2 |= I2C_CR2_STOP;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
