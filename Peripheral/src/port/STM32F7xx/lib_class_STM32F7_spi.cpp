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
#include "./lib_digini.h"
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
#define SPI_CR1_BR_OFFSET                   3

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
    m_Mutex      = NULL;
    m_pDriver[SPI_ID] = this;

  #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
    m_DMA_Status = SYS_UNKNOWN;
  #endif  
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
    uint32_t PCLK_Frequency;

    nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);

    IO_PinInit(m_pInfo->PinCLK);
    IO_PinInit(m_pInfo->PinMOSI);
    IO_PinInit(m_pInfo->PinMISO);
    IO_PinInit(m_pInfo->PinNSS);

    switch(uint32_t(m_SPI_ID))
    {
      #if (SPI_DRIVER_SUPPORT_SPI1_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI1_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI1RST;             // Enable SPI1 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;             // Release SPI1 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI1EN;               // Enable SPI_PORT clock
            PCLK_Frequency =  SYS_APB2_CLOCK_FREQUENCY;
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
            PCLK_Frequency =  SYS_APB1_CLOCK_FREQUENCY;
        }
        break;
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI3_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI3_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1RSTR |=  RCC_APB1RSTR_SPI3RST;             // Enable SPI3 reset state
            RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI3RST;             // Release SPI3 from reset state
            RCC->APB1ENR  |=  RCC_APB1ENR_SPI3EN;               // Enable SPI_PORT clock
            PCLK_Frequency =  SYS_APB1_CLOCK_FREQUENCY;
        }
        break;
      #endif
      
      #if (SPI_DRIVER_SUPPORT_SPI4_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI4_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI4RST;             // Enable SPI4 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI4RST;             // Release SPI4 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI4EN;               // Enable SPI_PORT clock
            PCLK_Frequency =  SYS_APB2_CLOCK_FREQUENCY;
        }
        break;
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI5_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI5_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI5RST;             // Enable SPI5 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI5RST;             // Release SPI5 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI5EN;               // Enable SPI_PORT clock
            PCLK_Frequency =  SYS_APB2_CLOCK_FREQUENCY;
        }
        break;
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI6_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI6_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI6RST;             // Enable SPI6 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI6RST;             // Release SPI6 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI6EN;               // Enable SPI_PORT clock
            PCLK_Frequency =  SYS_APB2_CLOCK_FREQUENCY;
        }
        break;
      #endif
    }

    //---------------------------- SPIx CR2 Configuration ------------------------

    // Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register)
    //CLEAR_BIT(pSPIx->I2SCFGR, SPI_I2SCFGR_I2SMOD);

    //---------------------------- SPIx CR1 Configuration and enable module ------
    
    CLEAR_BIT(m_pInfo->pSPIx->CR1, SPI_CR1_SPE);                // Disable SPIx
    MODIFY_REG(m_pInfo->pSPIx->CR1, SPI_CFG_CR1_CLEAR_MASK, m_pInfo->Config);
    SetPrescalerFromSpeed(m_pInfo->Speed, PCLK_Frequency);
    SET_BIT(m_pInfo->pSPIx->CR1, SPI_CR1_SPE);                  // Enable SPIx

//    Config(SPI_CFG_CR1_CLEAR_MASK, SPI_MODE_MASTER      |
//                                   SPI_DATA_WIDTH_8_BIT |
//                                   SPI_POLARITY_LOW     |
//                                   SPI_PHASE_1_EDGE     |
//                                   SPI_NSS_SOFT         |   // This driver doesn't use NSS function of the module
//                                   SPI_MSB_FIRST        |
//                                   m_pInfo->Control     |   // TODO should use DMA method to merge all settings.
//                                   m_pInfo->Speed);

    //----------------------------------------------------------------------------
    

  #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
    m_DMA_Status = SYS_IDLE;
    m_NoMemoryIncrement = false;

    if(m_pInfo->DMA_TX.ConfigAndChannel != SPI_DMA_DISABLED)
    {
        m_IsItUsingDMA_TX = true;
        m_DMA_TX.Initialize(&m_pInfo->DMA_TX);
        m_DMA_TX.SetDestination((void*)&m_pInfo->pSPIx->DR);    // Configure transmit data register
        m_DMA_TX.EnableTransmitCompleteInterrupt();
        m_DMA_TX.EnableIRQ();                                   // NVIC Setup for TX DMA channels interrupt request
    }

    if(m_pInfo->DMA_RX.ConfigAndChannel != SPI_DMA_DISABLED)
    {
        m_IsItUsingDMA_RX = true;
        m_DMA_RX.Initialize(&m_pInfo->DMA_RX);
        m_DMA_RX.SetSource((void*)&m_pInfo->pSPIx->DR);         // Configure transmit data register
        m_DMA_RX.EnableTransmitCompleteInterrupt();
        m_DMA_RX.EnableIRQ();                                   // NVIC Setup for RX DMA channels interrupt request
    }
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockToDevice
//
//  Parameter(s):   IO_ID_e        Device           It use the unique ID of the CS pin as device ID
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
    if(m_Device == IO_NOT_DEFINED)
    {
        while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK){};
        m_Device = Device;
        m_Status = SYS_READY;
        IO_SetPinLow(Device);
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
    if(m_Device == Device)
    {
        nOS_MutexUnlock(&m_Mutex);
        m_Device = IO_NOT_DEFINED;
        m_Status = SYS_DEVICE_NOT_PRESENT;
        IO_SetPinHigh(Device);
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
//  Function:       SetPrescalerFromSpeed
//
//  Parameter(s):   uint32_t Speed
//                  uint32_t PCLK_Frequency
//  Return:         None
//
//  Description:    Set the calculated value of the prescaler from requested speed.
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::SetPrescalerFromSpeed(uint32_t Speed, uint32_t PCLK_Frequency)
{
    uint16_t Prescaler;
    uint16_t IdealPrescaler;
    int BaudRate = 0;

    IdealPrescaler = PCLK_Frequency / Speed;

    for(int i = 0; i <= 7; i++)
    {
        Prescaler = (1 << (i + 1));

        if(IdealPrescaler >= Prescaler)
        {
            BaudRate = i << SPI_CR1_BR_OFFSET;
        }
        else
        {
            MODIFY_REG(m_pInfo->pSPIx->CR1, SPI_SPEED_MASK, BaudRate);
            return;
        }
    }
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
	SPI_TypeDef*  pSPIx;
    uint32_t      Dummy;
    SystemState_e State = SYS_READY;

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
          #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
            
            if(m_IsItUsingDMA_TX == true)
            {                
                // TX DMA
                m_DMA_Status = SYS_BUSY_TX;                                 // Set flag to busy in TX
                m_DMA_TX.SetSource(pTX_Data);                               // Set DMA source
                m_DMA_TX.SetLength(TX_Size);                                // Set size of the TX

                if(m_NoMemoryIncrement == false)
                {
                    m_DMA_TX.SetMemoryIncrement();                          // Enable memory increment
                }
                else
                {
                    m_DMA_TX.SetNoMemoryIncrement();
                    m_NoMemoryIncrement = false;
                }

                m_DMA_TX.Enable();                                          // Enable the DMA module
                m_DMA_TX.ClearFlag();                                       // Clear IRQ DMA flag

                SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                           // Enable SPI
                SET_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);                       // Enable DMA TX
                State = WaitDMA();
                CLEAR_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);                     // Deactivate DMA
                m_DMA_TX.Disable();                                         // Disable the DMA module
                CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                         // Disable SPI
            }
            else
          #endif
            
            {
                // IRQ method
            }
        }

        // ----------------------------------------------------------------------------------------
        // RX setup
        if(State == SYS_READY)
        {
            if((pRX_Data != nullptr) && (RX_Size != 0))
            {
                while((pSPIx->SR & SPI_SR_RXNE) != 0)
                {
                    Dummy = pSPIx->DR;
                }

              #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
                if(m_IsItUsingDMA_RX == true)
                {                
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

                    SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                           // Enable SPI
                    SET_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));   // Start the process

                    State = WaitDMA();

                    // Deactivate DMA
                    CLEAR_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));
                    m_DMA_RX.Disable();                                         // Disable the DMA RX module
                    m_DMA_TX.Disable();                                         // Disable the DMA TX module
                }
                else
              #endif
                {
                    // IRQ method
                }
            }
        }

        // ----------------------------------------------------------------------------------------

        CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                             // Disable SPI

        m_DMA_Status = SYS_IDLE;

        if(State == SYS_ERROR)
        {
            State = SYS_TIME_OUT;
        }

        return State;
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
#if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
void SPI_Driver::OverrideMemoryIncrement(void)
{
    m_NoMemoryIncrement = true;
}
#endif

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

#if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)

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

#endif // (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
