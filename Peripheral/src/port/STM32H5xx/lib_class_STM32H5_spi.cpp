//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_spi.cpp
//
//-------------------------------------------------------------------------------------------------

// This driver use the most common configuration for accessing SPI devices.

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define SPI_DRIVER_GLOBAL
#include "BSP_board.h"
#undef  SPI_DRIVER_GLOBAL
#include "CalcInterval.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SPI_DMA_TRANSFER_TIMEOUT            100

#define SPI_SPEED_MASK                      (SPI_CFG1_MBR)

// As per PDF RM0481
#define SPI_CR1_DEFAULT                     0x00000000
#define SPI_CR2_DEFAULT                     0x00000000             
#define SPI_CFG1_DEFAULT                    0x00070007
#define SPI_CFG2_DEFAULT                    (SPI_CFG2_SSOE | SPI_CFG2_AFCNTR)
#define SPI_CFG1_BR_OFFSET                  28

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
    m_SPI_ID          = SPI_ID;
    m_Device          = IO_NOT_DEFINED;
    m_pInfo           = &SPI_Info[SPI_ID];
    m_Status          = SYS_UNKNOWN;
    m_DMA_Status      = SYS_UNKNOWN;
    m_IsItUsingDMA_RX = false;
  #ifdef USE_FREERTOS
    m_Mutex           = nullptr;
  #else
    m_IsItInUse       = false;
  #endif
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
    uint32_t CLK_Frequency = 0;

  #ifdef USE_FREERTOS
    m_Mutex             = xSemaphoreCreateMutex();          // Create a mutex type semaphore.
	m_SignalISRToThread = xSemaphoreCreateBinary();         // Create Signal to thread.
  #endif

    // Init defined IO's
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
            CLK_Frequency = HAL_RCC_GetPCLK2Freq();
        }
        break;
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI2_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI2_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1LRSTR |=  RCC_APB1LRSTR_SPI2RST;           // Enable SPI2 reset state
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_SPI2RST;           // Release SPI2 from reset state
            RCC->APB1LENR  |=  RCC_APB1LENR_SPI2EN;             // Enable SPI_PORT clock
            CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            
            // According to Errata:
            // With fast software execution (high frequency) and slow SPI (low SCK frequency), the SPI disable occurring
            // too fast may result in truncating the SPI output signals.
            //  Workaround
            //            • Decrease the ratio between CLK to SPI and SCK frequencies
            /* For debug to slow down test 
            MODIFY_REG(RCC->CCIPR5, RCC_CCIPR5_CKERPSEL_Msk, 0x00);                 // make sure per_clk is set for HSI
            MODIFY_REG(RCC->CCIPR3, RCC_CCIPR3_SPI4SEL_Msk, RCC_CCIPR3_SPI2SEL_2);  // We use Per_clk as clock and it is fix at 64 MHz
            CLK_Frequency = HSI_VALUE;
            */
        }
        break;
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI3_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_SPI3_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1LRSTR |=  RCC_APB1LRSTR_SPI3RST;           // Enable SPI3 reset state
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_SPI3RST;           // Release SPI3 from reset state
            RCC->APB1LENR  |=  RCC_APB1LENR_SPI3EN;             // Enable SPI_PORT clock
            CLK_Frequency = HAL_RCC_GetPCLK1Freq();
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
            CLK_Frequency = HAL_RCC_GetPCLK2Freq();

            // According to Errata:
            // With fast software execution (high frequency) and slow SPI (low SCK frequency), the SPI disable occurring
            // too fast may result in truncating the SPI output signals.
            //  Workaround
            //            • Decrease the ratio between CLK to SPI and SCK frequencies
            /* For debug to slow down test
            MODIFY_REG(RCC->CCIPR3, RCC_CCIPR3_SPI4SEL_Msk, (RCC_CCIPR3_SPI4SEL_1 | RCC_CCIPR3_SPI4SEL_0));  // We use HSI as clock and it is fix at 64 MHz
            CLK_Frequency = HSI_VALUE;
            */
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
            CLK_Frequency = HAL_RCC_GetPCLK2Freq();
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
            CLK_Frequency = HAL_RCC_GetPCLK2Freq();
        }
        break;
      #endif
    }

    //---------------------------- SPIx Configuration and enable module ------
    
    m_pInfo->pSPIx->CR1  = SPI_CR1_DEFAULT;                      // Disable SPIx
    m_pInfo->pSPIx->CR2  = SPI_CR2_DEFAULT;
    m_pInfo->pSPIx->CFG1 = SPI_CFG1_DEFAULT;
    m_pInfo->pSPIx->CFG2 = SPI_CFG2_DEFAULT;

    m_pInfo->pSPIx->CFG1 |= m_pInfo->DataWidth;

    m_pInfo->pSPIx->CFG2 |= (m_pInfo->Mode     | m_pInfo->CLK_Polarity | m_pInfo->CLK_Phase |
                             m_pInfo->FirstBit | m_pInfo->NSS_Handling | m_pInfo->Duplex);

    SetPrescalerFromSpeed(m_pInfo->Speed, CLK_Frequency);

    //----------------------------------------------------------------------------

    // There is always a TX DMA
    m_DMA_Status = SYS_IDLE;
    m_NoMemoryIncrement = false;
    m_DMA_TX.Initialize(&m_pInfo->DMA_TX);
    m_DMA_TX.SetDestination((void*)&m_pInfo->pSPIx->TXDR);      // Configure transmit data register

    if(m_pInfo->DMA_RX.pChannel != SPI_DMA_DISABLED)
    {
        m_IsItUsingDMA_RX = true;
        m_DMA_RX.Initialize(&m_pInfo->DMA_RX);
        m_DMA_RX.SetSource((void*)&m_pInfo->pSPIx->RXDR);       // Configure transmit data register
    }

    ClearAllFlag();
    m_pInfo->pSPIx->IER |= SPI_IER_EOTIE;                       // Enable end of transmission IRQ
    ISR_Init(m_pInfo->IRQn, m_pInfo->PreempPrio);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockToDevice
//
//  Parameter(s):   IO_ID_e        Device           It use the unique ID of the CS pin as device ID
//                  bool           HandleCS         Handle or not the Chip Select
//  Return:         SystemState_e  Status
//
//  Description:    This routine will lock it to a specific device so any other access to the port
//                  will be block until unlock
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::LockToDevice(IO_ID_e Device, bool HandleCS)
{
  #ifdef USE_FREERTOS
    while(m_Device != IO_NOT_DEFINED)
    {
        vTaskDelay(1);
    }
  #endif

  #ifndef USE_FREERTOS
    if(m_Device == IO_NOT_DEFINED)
    {
  #endif
      #ifdef USE_FREERTOS
        xSemaphoreTake(m_Mutex, portMAX_DELAY);
      #endif    
        m_Device = Device;
        m_Status = SYS_READY;

        if(HandleCS == true)
        {
            IO_SetPinLow(Device);
        }
  #ifndef USE_FREERTOS
    }
    else
    {
        return SYS_NOT_LOCK_TO_DEVICE;
    }
  #endif
  
    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UnlockFromDevice
//
//  Parameter(s):   IO_ID_e       Device
//                  bool          HandleCS          Handle or not the Chip Select
//  Return:         SystemState_e Status
//
//  Description:    This routine will unlock SPI port from a specific device
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//                  if lock and no write at all if not lock to a device
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::UnlockFromDevice(IO_ID_e Device, bool HandleCS)
{
    if(m_Device == Device)
    {
        if(HandleCS == true)
        {
            IO_SetPinHigh(Device);
        }

      #ifdef USE_FREERTOS
        xSemaphoreGive(m_Mutex);
      #endif
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
//  Name:           SelectChip
//
//  Parameter(s):   IO_ID_e       Device
//  Return:         SystemState_e Status
//
//  Description:    This routine activate the chip select
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::SelectChip(IO_ID_e Device)
{ 
    if(m_Device == Device)
    {
        IO_SetPinLow(Device);
    }
    else
    {
        if(Device != m_Device) return SYS_WRONG_DEVICE;
    }
    
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DeSelectChip
//
//  Parameter(s):   IO_ID_e       Device
//  Return:         SystemState_e Status
//
//  Description:    This routine de-activate the chip select
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::DeSelectChip(IO_ID_e Device)
{
    if(m_Device == Device)
    {
        IO_SetPinHigh(Device);
    }
    else
    {
        if(Device != m_Device) return SYS_WRONG_DEVICE;
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
            BaudRate = i << SPI_CFG1_BR_OFFSET;
        }
        else
        {
            MODIFY_REG(m_pInfo->pSPIx->CFG1, SPI_SPEED_MASK, BaudRate);
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearAllFlag
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear all pending flag
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::ClearAllFlag(void)
{
    m_pInfo->pSPIx->IFCR = (SPI_IFCR_SUSPC | SPI_IFCR_MODFC | SPI_IFCR_TIFREC | SPI_IFCR_CRCEC |
                            SPI_IFCR_OVRC  | SPI_IFCR_UDRC  | SPI_IFCR_TXTFC  | SPI_IFCR_EOTC);
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
//  Description:    Write single data of data buffer to SPI
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
//                  TX_Size			    Number of byte to send.
//                  pRX_pData       Pointer on the data buffer where to put received data.
//                  RX_Size			    Number of byte to receive.
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
//                  TX_Size			    Number of byte to send.
//                  pRX_pData       Pointer on the data buffer where to put received data.
//                  RX_Size			    Number of byte to receive.
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
        pSPIx = m_pInfo->pSPIx;

        // ----------------------------------------------------------------------------------------
        // TX Setup
        // Note(s) needed by both TX and RX

        if((pTX_Data != nullptr) && (TX_Size != 0))
        {
            #ifndef USE_FREERTOS
            m_IsItInUse = true;
            #endif
            
            // Configure TX DMA
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
            
            // Activate SPI & DMA
            ClearAllFlag();
            MODIFY_REG(pSPIx->CR2, SPI_CR2_TSIZE, TX_Size);
            SET_BIT(pSPIx->CFG1, SPI_CFG1_TXDMAEN);                     // Enable DMA TX
            MODIFY_REG(pSPIx->CFG2, SPI_CFG2_COMM_Msk, SPI_CFG2_COMM_0);// simplex transmitter
            SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                           // Enable SPI
            SET_BIT(pSPIx->CR1, SPI_CR1_CSTART);                        // Master Transfert Start
            
            // Complete transaction and stop SPI and DMA
            State = WaitEndOfTransfer();
            m_DMA_TX.Disable();                                         // Disable the DMA module
            CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                         // Disable SPI
            CLEAR_BIT(pSPIx->CFG1, SPI_CFG1_TXDMAEN);
        }

        // ----------------------------------------------------------------------------------------
        // RX setup
        if(State == SYS_READY)
        {
            if((pRX_Data != nullptr) && (RX_Size != 0))
            {
              #ifndef USE_FREERTOS
                m_IsItInUse = true;
              #endif

                while((pSPIx->SR & SPI_SR_RXWNE) != 0)
                {
                    Dummy = uint8_t(pSPIx->RXDR);
                }

                VAR_UNUSED(Dummy);

                if(m_IsItUsingDMA_RX == true)
                {                
                    m_DMA_Status = SYS_BUSY_RX;                                     // Set flag to busy in TX

                    // Configure RX DMA
                    m_DMA_RX.SetDestination(pRX_Data);
                    m_DMA_RX.SetLength(RX_Size);
                    m_DMA_RX.Enable();                                              // Enable the DMA module
                    m_DMA_RX.ClearFlag();
                    
                    // Activate SPI & DMA
                    ClearAllFlag();
                    MODIFY_REG(pSPIx->CR2, SPI_CR2_TSIZE, RX_Size);
                    SET_BIT(pSPIx->CFG1, (SPI_CFG1_RXDMAEN));                       // Enable RX SPI DMA
                    MODIFY_REG(pSPIx->CFG2, SPI_CFG2_COMM_Msk, SPI_CFG2_COMM_1);    // simplex receiver
                    SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                               // Enable SPI
                    SET_BIT(pSPIx->CR1, SPI_CR1_CSTART);                            // Master Transfert Start

                    // Complete transaction and stop SPI and DMA
                    State = WaitEndOfTransfer();
                    m_DMA_RX.Disable();                                             // Disable the DMA module
                    CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                             // Disable SPI
                    CLEAR_BIT(pSPIx->CFG1, SPI_CFG1_RXDMAEN);
                }
            }
        }

        // ----------------------------------------------------------------------------------------

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
void SPI_Driver::OverrideMemoryIncrement(void)
{
    m_NoMemoryIncrement = true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WaitEndOfTransfer
//
//  Parameter(s):   None
//  Return:         SystemState_e   State
//
//  Description:    Wait for SPI transfer to conclude
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::WaitEndOfTransfer(void)
{
    SystemState_e State = SYS_READY;

  #ifndef USE_FREERTOS
  uint32_t TickStart  = GetTick();
  uint32_t ElapseTick;
  #endif

  #ifdef USE_FREERTOS
	// Do a busy wait if 
    //           1) We're not in FreeRTOS context
    //           2) or in an interrupt
    if((xTaskGetCurrentTaskHandle() == 0) || (__get_IPSR() != 0))
    {
        State = SYS_BUSY;
    }

    if(xSemaphoreTake(m_SignalISRToThread, SPI_DMA_TRANSFER_TIMEOUT) == pdFALSE)
    {
        State = SYS_TIME_OUT;
    }
  #else
    do
    {
        //State = TickHasTimeOut(TickStart, SPI_DMA_TRANSFER_TIMEOUT) ? SYS_READY : SYS_TIME_OUT;
        __asm("nop");
    }
    while((m_IsItInUse == true) && (State == SYS_READY));
  #endif

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReleaseFromISR
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Get status of the SPI and DMA transfer
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::ReleaseFromISR(void)
{
    m_pInfo->pSPIx->IFCR = SPI_IFCR_EOTC;        // Clear the flag

  #ifdef USE_FREERTOS
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(m_SignalISRToThread, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  #else
    m_IsItInUse = false;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IRQ_Handler
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Signal application -> the transfer is completed
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::IRQ_Handler(SPI_ID_e SPI_ID)
{
    SPI_Driver::m_pDriver[SPI_ID]->ReleaseFromISR();
}

//-------------------------------------------------------------------------------------------------

extern "C"
{
  #if (SPI_DRIVER_SUPPORT_SPI2_CFG == DEF_ENABLED)
    void SPI2_IRQHandler()
    {
        SPI_Driver::IRQ_Handler(DRIVER_SPI2_ID);
    }
  #endif

  #if (SPI_DRIVER_SUPPORT_SPI4_CFG == DEF_ENABLED)
    void SPI4_IRQHandler()
    {
        SPI_Driver::IRQ_Handler(DRIVER_SPI4_ID);
    }
  #endif

  #if (SPI_DRIVER_SUPPORT_SPI6_CFG == DEF_ENABLED)
    void SPI6_IRQHandler()
    {
        SPI_Driver::IRQ_Handler(DRIVER_SPI6_ID);
    }
  #endif
}

//-------------------------------------------------------------------------------------------------
