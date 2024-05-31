//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_spi.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#define SPI_DRIVER_GLOBAL
#include "./Digini/lib_digini.h"
#undef  SPI_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SPI_DMA_TRANSFER_TIMEOUT            1000

#define SPI_DMA_SxCR_8_OR_16_BITS_MASK      DMA_SxCR_MSIZE | DMA_SxCR_PSIZE
#define SPI_DMA_SxCR_8_BITS_CFG             0
#define SPI_DMA_SxCR_16_BITS_CFG            DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0

//-------------------------------------------------------------------------------------------------
//
//   Class: SPI_Driver
//
//
//   Description:   Class to handle SPI
//
//-------------------------------------------------------------------------------------------------

SPI_Driver* SPI_Driver::m_pDriver[NB_OF_SPI_DRIVER] = {nullptr};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   SPI_Driver
//
//   Parameter(s):  SPI_ID          ID of the SPI info
//
//   Description:   Initializes the SPIx peripheral according to the specified Parameters
//
//-------------------------------------------------------------------------------------------------
SPI_Driver::SPI_Driver(SPI_ID_e SPI_ID)
{
    m_SPI_ID     = SPI_ID;
    m_Device     = IO_NOT_DEFINED;
    m_pInfo      = &SPI_Info[SPI_ID];
    m_Status     = SYS_UNKNOWN;
    m_DMA_Status = SYS_UNKNOWN;
    m_pDriver[SPI_ID] = this;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Initialize this SPI port
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::Initialize(void)
{
    nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);

    IO_PinInit(m_pInfo->PinCLK);
    IO_PinInit(m_pInfo->PinMOSI);
    IO_PinInit(m_pInfo->PinMISO);

    switch(uint32_t(m_SPI_ID))
    {
      #if (SPI_DRIVER_SUPPORT_SPI1_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI1_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI1RST;             // Enable SPI1 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;             // Release SPI1 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI1EN;               // Enable SPI_PORT clock
        }
        break;
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI2_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI2_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1RSTR |=  RCC_APB1RSTR_SPI2RST;             // Enable SPI2 reset state
            RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;             // Release SPI2 from reset state
            RCC->APB1ENR  |=  RCC_APB1ENR_SPI2EN;               // Enable SPI_PORT clock
        }
        break;
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI3_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI3_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1RSTR |=  (RCC_APB1RSTR_SPI3RST;            // Enable SPI3 reset state
            RCC->APB1RSTR &= ~(RCC_APB1RSTR_SPI3RST;            // Release SPI3 from reset state
            RCC->APB1ENR  |=  (RCC_APB1ENR_SPI3EN;              // Enable SPI_PORT clock
        }
        break;
      #endif
    }

      #if (SPI_DRIVER_SUPPORT_SPI4_CFG == DEF_ENABLED)
        case DRIVER_SPI4_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI4RST;             // Enable SPI4 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI4RST;             // Release SPI4 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI4EN;               // Enable SPI_PORT clock
            break;
        }
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI5_CFG == DEF_ENABLED)
        case DRIVER_SPI5_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI5RST;	        // Enable SPI5 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI5RST;             // Release SPI5 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI5EN;	        // Enable SPI_PORT clock
            break;
        }
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI6_CFG == DEF_ENABLED)
        case DRIVER_SPI6_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI5RST;	        // Enable SPI6 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI5RST;             // Release SPI6 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI5EN;	        // Enable SPI_PORT clock
            break;
        }
      #endif
    }

    //---------------------------- SPIx CR2 Configuration ------------------------

    // Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register)
    //CLEAR_BIT(pSPIx->I2SCFGR, SPI_I2SCFGR_I2SMOD);

    //---------------------------- SPIx CR1 Configuration and enable module ------

    Config(SPI_CFG_CR1_CLEAR_MASK, SPI_FULL_DUPLEX      |
                                   SPI_MODE_MASTER      |
                                   SPI_DATA_WIDTH_8_BIT |
                                   SPI_POLARITY_LOW     |
                                   SPI_PHASE_1_EDGE     |
                                   SPI_NSS_SOFT         |   // This driver doesn't use NSS function of the module
                                   SPI_MSB_FIRST        |
                                   m_pInfo->Speed);

    m_DMA_Status = SYS_IDLE;
    m_NoMemoryIncrement = false;

    // Pre inititialize register that won't change
    m_DMA_TX.Initialize(&m_pInfo->DMA_TX);
    m_DMA_TX.SetDestination((void*)&m_pInfo->pSPIx->DR);          // Configure transmit data register

    m_DMA_RX.Initialize(&m_pInfo->DMA_RX);
    m_DMA_RX.SetSource((void*)&m_pInfo->pSPIx->DR);          // Configure transmit data register

    //ISR_Init(m_pInfo->TX_IRQn, 6);                   // NVIC Setup for TX DMA channels interrupt request
    //ISR_Init(m_pInfo->RX_IRQn, 6);                   // NVIC Setup for RX DMA channels interrupt request
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockToDevice
//
//  Parameter(s):   IO_ID_e        Device
//  Return:         SystemState_e  Status
//
//  Description:    This routine will lock it to a specific device so any other access to the port
//                  will be block until unlock
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::LockToDevice(IO_ID_e Device)
{
    if(Device == IO_NOT_DEFINED)
    {
        while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK){};
        m_Device = Device;
        m_Status = SYS_READY;
    }

    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UnlockFromDevice
//
//  Parameter(s):   IO_ID_e       Device
//  Return:         SystemState_e Status
//
//  Description:    This routine will unlock SPI port from a specific device
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//                  if lock and no write at all if not lock to a device
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::UnlockFromDevice(IO_ID_e Device)
{
    if(Device == m_Device)
    {
        nOS_MutexUnlock(&m_Mutex);
        m_Device = IO_NOT_DEFINED;
        m_Status = SYS_DEVICE_NOT_PRESENT;
    }
    else
    {
        if(Device != m_Device) return SYS_WRONG_DEVICE;
        else                   return SYS_NOT_LOCK_TO_DEVICE;
    }
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetPrescalerFromSpeed
//
//  Parameter(s):   None
//  Return:         none
//
//  Description:    Calculate the prescaler value for requested speed
//
//-------------------------------------------------------------------------------------------------
/*
uint16_t SPI_Driver::GetPrescalerFromSpeed(uint32_t Speed)
{
    int i;
    uint16_t Prescaler;

    for(i = 0; i < 8; i++)
    {
        Prescaler = ((i + 1) << 1);

        if((APB2_CLK / Speed) < Prescaler)
        {
            return Prescaler;
        }
    }
    return i;     // 'i' is at an invalid value
}
*/

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Lock the driver
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::Lock(void)
{
    while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK) {};
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
//-------------------------------------------------------------------------------------------------
void SPI_Driver::Unlock(void)
{
    nOS_MutexUnlock(&m_Mutex);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Config
//
//  Parameter(s):   SPI_Speed_e     Speed
//  Return:         None
//
//  Description:    Set speed on the SPI port
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::Config(uint32_t Mask, uint32_t Config)
{
    CLEAR_BIT(m_pInfo->pSPIx->CR1, SPI_CR1_SPE);                // Disable SPIx
    MODIFY_REG(m_pInfo->pSPIx->CR1, Mask, Config);
    SET_BIT(m_pInfo->pSPIx->CR1, SPI_CR1_SPE);                  // Enable SPIx
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   Differ according to overloaded function
//
//                      - SystemState_e Write   (uint8_t* pBuffer, size_t Size)
//                      - SystemState_e Write   (uint8_t* pBuffer, size_t Size, IO_ID_e Device)
//                      - SystemState_e Write   (uint8_t  Data);
//                      - SystemState_e Write   (uint8_t  Data, IO_ID_e Device);
//
//  Return:         None
//
//  Description:    Write singke data of data buffer to SPI
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Write(uint8_t* pBuffer, size_t Size)
{
    return Transfer(pBuffer, Size, nullptr, 0);
}

SystemState_e SPI_Driver::Write(uint8_t* pBuffer, size_t Size, IO_ID_e Device)
{
    return Transfer(pBuffer, Size, nullptr, 0, Device);
}

SystemState_e SPI_Driver::Write(uint8_t  Data)
{
    return Transfer(&Data, 1, nullptr, 0);
}

SystemState_e SPI_Driver::Write(uint8_t  Data, IO_ID_e Device)
{
    return Transfer(&Data, 1, nullptr, 0, Device);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   Differ according to overloaded function
//
//                      - SystemState_e Read   (uint8_t* pBuffer, size_t Size)
//                      - SystemState_e Read   (uint8_t* pBuffer, size_t Size, IO_ID_e Device)
//                      - SystemState_e Read   (uint8_t* pData);
//                      - SystemState_e Read   (uint8_t* pData, IO_ID_e Device);
//
//  Return:         None
//
//  Description:    Write single data of data buffer to SPI
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Read(uint8_t* pBuffer, size_t Size)
{
    return Transfer(nullptr, 0, pBuffer, Size);
}

SystemState_e SPI_Driver::Read(uint8_t* pBuffer, size_t Size, IO_ID_e Device)
{
    return Transfer(nullptr, 0, pBuffer, Size, Device);
}

SystemState_e SPI_Driver::Read(uint8_t* pData)
{
    return Transfer(nullptr, 0, pData, 1);
}

SystemState_e SPI_Driver::Read(uint8_t* pData, IO_ID_e Device)
{
    return Transfer(nullptr, 0, pData, 1, Device);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Transfer
//
//  Parameter(s):   pTX_Data        Pointer on the data buffer to transfer.
//                  TX_Size			Number of byte to send.
//                  pRX_pData       Pointer on the data buffer where to put received data.
//                  RX_Size			Number of byte to receive.
//                  Device          IO_ID_e (it's a unique ID in the system)
//  Return:         SystemState_e   State
//
//  Description:    Read or writes data to SPI device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Transfer(uint8_t* pTX_Data, uint32_t TX_Size, uint8_t* pRX_Data, uint32_t RX_Size, IO_ID_e Device)
{
    SystemState_e State;

    if((State = LockToDevice(Device)) == SYS_READY)
    {
        State = Transfer(pTX_Data, TX_Size, pRX_Data, RX_Size);
        UnlockFromDevice(Device);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Transfer
//
//  Parameter(s):   pTX_Data        Pointer on the data buffer to transfer.
//                  TX_Size			Number of byte to send.
//                  pRX_pData       Pointer on the data buffer where to put received data.
//                  RX_Size			Number of byte to receive.
//  Return:         SystemState_e   State
//
//  Description:    Read or writes data to SPI device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Transfer(uint8_t* pTX_Data, uint32_t TX_Size, uint8_t* pRX_Data, uint32_t RX_Size)
{
	SPI_TypeDef* pSPIx;
    uint32_t     Dummy;

    if(((pTX_Data == nullptr) || (TX_Size == 0)) &&
       ((pRX_Data == nullptr) || (RX_Size == 0)))
    {
        return SYS_NULLPTR;
    }

    if(m_Status == SYS_READY)
    {
        CLEAR_BIT(m_pInfo->pSPIx->CR1, SPI_CR1_SPE);

        // ----------------------------------------------------------------------------------------
        // TX Setup

        // Note(s) needed by both TX and RX
        pSPIx = m_pInfo->pSPIx;

        if((pTX_Data != nullptr) && (TX_Size != 0))
        {
            // TX DMA
            m_DMA_Status = SYS_BUSY_TX;                                 // Set flag to busy in TX
            m_DMA_TX.SetSource(pTX_Data);                               // Set DMA source
            m_DMA_TX.SetLength(TX_Size);                                // Set size of the TX

            if(m_NoMemoryIncrement == false)
            {
                m_DMA_TX.SetMemoryIncrement();                          // Enable transfer complete and memory increment
            }
            else
            {
                m_DMA_TX.SetNoMemoryIncrement();
                m_NoMemoryIncrement = false;
            }

            m_DMA_TX.Enable();                                          // Enable the DMA module
            m_DMA_TX.ClearFlag();                                       // Clear IRQ DMA flag

            // SPI
            if(m_pInfo->Control == SPI_HALF_DUPLEX)
            {
                MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_HALF_DUPLEX_TX);
            }

            SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                           // Enable SPI
            SET_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);                       // Enable DMA TX

            if(WaitDMA() == SYS_ERROR)
            {
                UnlockFromDevice(m_Device);
                return SYS_TIME_OUT;
            }

            CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                         // Disable SPI

            // Deactivate DMA
            CLEAR_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);
            m_DMA_TX.Disable();                                         // Disable the DMA module
        }

        // ----------------------------------------------------------------------------------------
        // RX setup

        if((pRX_Data != nullptr) && (RX_Size != 0))
        {
            while((pSPIx->SR & SPI_SR_RXNE) != 0)
            {
                Dummy = pSPIx->DR;
            }

            m_DMA_Status = SYS_BUSY_RX;                                 // Set flag to busy in TX

            // TX DMA
            Dummy = 0xFF;                                               // Value to dummy TX
            m_DMA_TX.SetSource(&Dummy);                                 // Set DMA source for dummy TX
            m_DMA_TX.SetLength(RX_Size);                                // Set size of the TX using RX_Size for dummy TX to kick the RX
            m_DMA_TX.SetNoMemoryIncrement();
            m_DMA_TX.Enable();                                          // Enable the DMA module
            m_DMA_TX.ClearFlag();                                       // Clear IRQ DMA flag
            // RX DMA
            m_DMA_RX.SetDestination(pRX_Data);
            m_DMA_RX.SetLength(RX_Size);
            m_DMA_RX.Enable();                                          // Enable the DMA module
            m_DMA_RX.ClearFlag();

            // SPI
            if(m_pInfo->Control == SPI_HALF_DUPLEX)
            {
                MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_HALF_DUPLEX_RX);
            }

            SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                           // Enable SPI
            SET_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));   // Start the process

            if(WaitDMA() == SYS_ERROR)
            {
                UnlockFromDevice(m_Device);
                return SYS_TIME_OUT;
            }

            // Deactivate DMA
            CLEAR_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));
            m_DMA_RX.Disable();                                         // Disable the DMA RX module
            m_DMA_TX.Disable();                                         // Disable the DMA TX module
        }

        // ----------------------------------------------------------------------------------------

        CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                             // Disable SPI

        if(m_pInfo->Control == SPI_HALF_DUPLEX)
        {
            MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_FULL_DUPLEX);
        }

        m_DMA_Status = SYS_IDLE;
        return SYS_READY;
    }

    return SYS_BUSY;
}

//-------------------------------------------------------------------------------------------------
//  Name:           OverrideMemoryIncrement
//
//  Description:    This set override memory increment for DMA transfer.
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Note(s):        This is valid only for the next transaction
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::OverrideMemoryIncrement(void)
{
    m_NoMemoryIncrement = true;
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    WaitReady
//
//  Description:    This function verify if the SPI is ready
//
//  Note(s):        return  SYS_READY if available
//                          SYS_HUNG  if busy pass timeout
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::WaitReady(void)
{

    m_Timeout = 100;
    while((m_pInfo->pSPIx->SR & SPI_SR_TXE)  == 0)          // Wait until transmit complete
    {
        nOS_Yield();
        if(m_Timeout == 0) return SYS_HUNG;
    }

    m_Timeout = 100;
    while((m_pInfo->pSPIx->SR & SPI_SR_RXNE) == 0)          // Wait until receive complete
    {
        nOS_Yield();
        if(m_Timeout == 0) return SYS_HUNG;
    }

    m_Timeout = 100;
    while((m_pInfo->pSPIx->SR & SPI_SR_BSY)  != 0)          // wait until SPI is not busy anymore
    {
        nOS_Yield();
        if(m_Timeout == 0) return SYS_HUNG;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    IRQHandler
//
//  Description:    This function handles SPIx interrupt request.
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::IRQHandler(void)
{
    //if(m_pInfo->CallBackISR != nullptr)
    //{
    //    m_pInfo->CallBackISR();
    //}
}

//-------------------------------------------------------------------------------------------------
//  DDDDDD  MM     MM  AAAAA
//  DD   DD MMMM MMMM AA   AA
//  DD   DD MM MMM MM AAAAAAA
//  DD   DD MM  M  MM AA   AA
//  DDDDDD  MM     MM AA   AA
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           WaitDMA
//
//  Parameter(s):   None
//  Return:         SystemState_e   State
//
//  Description:    Wait for DMA to conclude
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::WaitDMA(void)
{
    TickCount_t  TickStart;
    SPI_TypeDef* pSPIx;

    pSPIx = m_pInfo->pSPIx;

    // Wait for DMA to conclude
    TickStart = GetTick();
    while(m_DMA_Status != SYS_BUSY_B4_RELEASE)
    {
        if(TickHasTimeOut(TickStart, SPI_DMA_TRANSFER_TIMEOUT) == true)
        {
            return SYS_TIME_OUT;
        }

        nOS_Yield();
    };

    // Wait for SPI to conclude
    TickStart = GetTick();
    while((pSPIx->SR & SPI_SR_BSY) != 0)
    {
        if(TickHasTimeOut(TickStart, SPI_DMA_TRANSFER_TIMEOUT) == true)
        {
            return SYS_TIME_OUT;
        }

        nOS_Yield();
    };

    return SYS_IDLE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DMA_TX_IRQ_Handler
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Get status of the SPI DMA transfer
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::DMA_TX_IRQ_Handler(SPI_ID_e SPI_ID)
{
    SPI_Driver* pDriver;

    pDriver = SPI_Driver::m_pDriver[SPI_ID];
    pDriver->m_DMA_Status = SYS_BUSY_B4_RELEASE;
    pDriver->m_DMA_TX.ClearFlag();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DMA_RX_IRQ_Handler
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Get status of the SPI DMA transfer
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::DMA_RX_IRQ_Handler(SPI_ID_e SPI_ID)
{
    SPI_Driver* pDriver;

    pDriver = m_pDriver[SPI_ID];
    pDriver->m_DMA_Status = SYS_BUSY_B4_RELEASE;
    pDriver->m_DMA_RX.ClearFlag();
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)



























// might be reinstated in future !!!

#if 0
//-------------------------------------------------------------------------------------------------
//
//  Name:           Transfer
//
//  Parameter(s):   pTX_Data        Pointer on the data buffer (uint16_t*) to transfer.
//                  TX_Size			Number of byte to send.
//                  pRX_pData       Pointer on the data buffer (uint16_t*) where to put received data.
//                  RX_Size			Number of byte to receive.
//                  Device          IO_ID_e (it's a unique ID in the system)
//  Return:         SystemState_e   State
//
//  Description:    Read or writes data to SPI device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Transfer(uint16_t* pTX_Data, uint32_t TX_Size, uint16_t* pRX_Data, uint32_t RX_Size, IO_ID_e Device)
{
    SystemState_e State;

    if((State = LockToDevice(Device)) == SYS_READY)
    {
        State = Transfer(pTX_Data, TX_Size, pRX_Data, RX_Size);
        UnlockFromDevice(Device);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Transfer
//
//  Parameter(s):   pTX_Data        Pointer on the data buffer (uint16_t*) to transfer.
//                  TX_Size			Number of byte to send.
//                  pRX_pData       Pointer on the data buffer (uint16_t*) where to put received data.
//                  RX_Size			Number of byte to receive.
//  Return:         SystemState_e   State
//
//  Description:    Read or writes data to SPI device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Transfer(uint16_t* pTX_Data, uint32_t TX_Size, uint16_t* pRX_Data, uint32_t RX_Size)
{
	SPI_TypeDef*        pSPIx;
    DMA_Stream_TypeDef* pDMA;
    uint32_t            Flag;
    uint32_t            Dummy;

    if(((pTX_Data == nullptr) || (TX_Size == 0)) &&
       ((pRX_Data == nullptr) || (RX_Size == 0)))
    {
        return SYS_NULLPTR;
    }

    if(m_Status == SYS_READY)
    {
        CLEAR_BIT(m_pInfo->pSPIx->CR1, SPI_CR1_SPE);

        // ----------------------------------------------------------------------------------------
        // TX Setup

        // Note(s) needed by both TX and RX
        pDMA  = m_pInfo->DMA_StreamTX;                                                      // In all case the first DMA set is the TX
        pSPIx = m_pInfo->pSPIx;
        Flag  = m_pInfo->TX_IT_Flag;

        if((pTX_Data != nullptr) && (TX_Size != 0))
        {
            // TX DMA
            m_DMA_Status = SYS_BUSY_TX;                                                     // Set flag to busy in TX
            pDMA->M0AR = ((uint32_t)pTX_Data);                                              // Set DMA source
            pDMA->NDTR = TX_Size;                                                           // Set size of the TX
            MODIFY_REG(pDMA->CR, SPI_DMA_SxCR_8_OR_16_BITS_MASK, SPI_DMA_SxCR_16_BITS_CFG);

            if(m_NoMemoryIncrement == false)
            {
                SET_BIT(pDMA->CR, DMA_SxCR_MINC);                                           // Enable transfer complete and memory increment
            }
            else
            {
                CLEAR_BIT(pDMA->CR, DMA_SxCR_MINC);
                m_NoMemoryIncrement = false;
            }

            SET_BIT(pDMA->CR, DMA_SxCR_EN);                                                 // Enable the DMA module
            DMA_ClearFlag(pDMA, Flag);                                                      // Clear IRQ DMA flag

            // SPI
            if(m_pInfo->Control == SPI_HALF_DUPLEX)
            {
                MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_HALF_DUPLEX_TX);
            }

            SET_BIT(pSPIx->CR1, SPI_CR1_DFF | SPI_CR1_SPE);                                 // Enable SPI at 16 Bits
            SET_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);                                           // Enable DMA TX

            if(WaitDMA() == SYS_ERROR)
            {
                UnlockFromDevice(m_Device);
                return SYS_TIME_OUT;
            }

            CLEAR_BIT(pSPIx->CR1, SPI_CR1_DFF | SPI_CR1_SPE);                               // Disable SPI and put back to 8 Bits default frame format

            // Deactivate DMA
            CLEAR_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);
            CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);                                               // Disable the DMA module
        }

        // ----------------------------------------------------------------------------------------
        // RX setup

        if((pRX_Data != nullptr) && (RX_Size != 0))
        {

            while((pSPIx->SR & SPI_SR_RXNE) != 0)
            {
                Dummy = pSPIx->DR;
            }

            m_DMA_Status = SYS_BUSY_RX;                                                     // Set flag to busy in TX

            // TX DMA
            Dummy = 0xFF;                                                                   // Value to dummy TX
            pDMA->M0AR = (uint32_t)&Dummy;                                                  // Set DMA source for dummy TX
            pDMA->NDTR = RX_Size;                                                           // Set size of the TX
            MODIFY_REG(pDMA->CR, SPI_DMA_SxCR_8_OR_16_BITS_MASK, SPI_DMA_SxCR_16_BITS_CFG); // Set 16 Bits Transfer
            CLEAR_BIT(pDMA->CR, DMA_SxCR_MINC);
            SET_BIT(pDMA->CR, DMA_SxCR_EN);                                                 // Enable the DMA module
            DMA_ClearFlag(pDMA, Flag);                                                      // Clear IRQ DMA flag

            // RX DMA
            pDMA = m_pInfo->DMA_StreamRX;                                                   // Now configure RX DMA
            Flag = m_pInfo->RX_IT_Flag;
            pDMA->M0AR = (uint32_t)pRX_Data;
            pDMA->NDTR = RX_Size;
            MODIFY_REG(pDMA->CR, SPI_DMA_SxCR_8_OR_16_BITS_MASK, SPI_DMA_SxCR_16_BITS_CFG); // Set 16 Bits Transfer
            SET_BIT(pDMA->CR, DMA_SxCR_EN);                                                 // Enable the DMA module
            DMA_ClearFlag(pDMA, Flag);

            // SPI
            if(m_pInfo->Control == SPI_HALF_DUPLEX)
            {
                MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_HALF_DUPLEX_RX);
            }

            SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                                               // Enable SPI
            SET_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));                       // Start the process

            if(WaitDMA() == SYS_ERROR)
            {
                UnlockFromDevice(m_Device);
                return SYS_TIME_OUT;
            }

            // Deactivate DMA
            CLEAR_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));
            CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);                                               // Disable the DMA RX module
            MODIFY_REG(pDMA->CR, SPI_DMA_SxCR_8_OR_16_BITS_MASK, SPI_DMA_SxCR_8_BITS_CFG);  // Put back to 8 bits default
            pDMA = m_pInfo->DMA_StreamTX;                                                   // In all case the first DMA set is the TX
            CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);                                               // Disable the DMA TX module
        }

        MODIFY_REG(pDMA->CR, SPI_DMA_SxCR_8_OR_16_BITS_MASK, SPI_DMA_SxCR_8_BITS_CFG);      // Put back to 8 bits default for TX ( also use on RX

        // ----------------------------------------------------------------------------------------

        CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                                                 // Disable SPI

        if(m_pInfo->Control == SPI_HALF_DUPLEX)
        {
            MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_FULL_DUPLEX);
        }

        m_DMA_Status = SYS_IDLE;
        return SYS_READY;
    }

    return SYS_BUSY;
}


#endif
