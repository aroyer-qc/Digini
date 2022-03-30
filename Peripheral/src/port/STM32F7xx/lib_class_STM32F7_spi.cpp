//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_spi.cpp
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

#define SPI_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  SPI_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//#define SPI_USE_DMA_TRANSFERT       0

#define SPI_PORT_QTY                6
#define SPI_TICK_HOOK_TIME	        portTICK_RATE_MS	        // in mSec

#define SPI_CR1_DFF_8_BITS			0
#define SPI_CR1_DFF_16_BITS		    SPI_CR1_DFF

#define	SPI_CR1_CPOL_LOW    		0
#define SPI_CR1_CPOL_HIGH			SPI_CR1_CPOL

#define SPI_CR1_CPHA_1_EDGE			0
#define SPI_CR1_CPHA_2_EDGE			SPI_CR1_CPHA

#define SPI_CR1_SSM_DISABLE			0
#define SPI_CR1_SSM_ENABLE 			SPI_CR1_SSM

#define SPI_CR1_MSB_FIRST			0					//SPI_FirstBit_MSB
#define SPI_CR1_LSB_FIRST			SPI_CR1_LSBFIRST	//SPI_FirstBit_LSB

#define SPI_PRESCALER_MASK          0x0038

#define SPI_TRANSFERT_TIME_OUT      2


//-------------------------------------------------------------------------------------------------
//
//   Class: SPI_Driver
//
//
//   Description:   Class to handle SPI
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   SPI_Driver
//
//   Parameter(s):  SPI_ID          ID of the SPI info
//
//   Description:   Initializes the SPIx peripheral according to the specified Parameters
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SPI_Driver::SPI_Driver(SPI_PortInfo_t* pPort)
{
//    m_pPort   = pPort;
    m_pDevice = nullptr;
    m_pInfo   = &SPI_Info[SPI_ID];
    m_State   = SYS_DEVICE_NOT_PRESENT;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      GetStatus
//
//   Parameter(s):
//   Return Value:
//
//   Description:    SystemState_e  Return general status of the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::GetStatus(void)
{
    return m_State;
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::Initialize(void)
{
    ISR_Prio_t          ISR_Prio;
    uint32_t            PriorityGroup;

  #if (SPI_DRIVER_SUPPORT_DMA == DEF_ENABLED)
    DMA_Stream_TypeDef* pDMA;
  #endif

    nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);

    IO_PinInit(m_pInfo->PinCLK);
    IO_PinInit(m_pInfo->PinMOSI);
    IO_PinInit(m_pInfo->PinMISO);
    IO_PinInit(m_pInfo->PinNSS);

    switch(m_pInfo->SPI_ID)
    {
      #if (SPI_DRIVER_SUPPORT_SPI1 == DEF_ENABLE)
        case DRIVER_SPI1_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI1RST;                                                    // Enable SPI1 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;                                                 // Release SPI1 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI1EN;                                                    // Enable SPI_PORT clock
            RCC->AHB1ENR  |=  RCC_AHB1ENR_DMA2EN;
            break;
        }
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI2 == DEF_ENABLE)
        case DRIVER_SPI2_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1RSTR |=  RCC_APB1RSTR_SPI2RST;	            // Enable SPI2 reset state
            RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;             // Release SPI2 from reset state
            RCC->APB1ENR  |=  RCC_APB1ENR_SPI2EN;               // Enable SPI_PORT clock
            RCC->AHB1ENR  |=  RCC_AHB1ENR_DMA1EN;
            break;
        }
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI3 == DEF_ENABLE)
        case DRIVER_SPI3_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1RSTR |=  RCC_APB1RSTR_SPI3RST;	            // Enable SPI3 reset state
            RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI3RST;             // Release SPI3 from reset state
            RCC->APB1ENR  |=  RCC_APB1ENR_SPI3EN;	            // Enable SPI_PORT clock
            RCC->AHB1ENR  |=  RCC_AHB1ENR_DMA1EN;
            break;
        }
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI4 == DEF_ENABLE)
        case DRIVER_SPI4_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI4RST;                                                    // Enable SPI4 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI4RST;                                                 // Release SPI4 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI4EN;                                                    // Enable SPI_PORT clock
            RCC->AHB1ENR  |=  RCC_AHB1ENR_DMA2EN;
            break;
        }
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI5 == DEF_ENABLE)
        case DRIVER_SPI5_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI5RST;	            // Enable SPI5 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI5RST;             // Release SPI5 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI5EN;	            // Enable SPI_PORT clock
            RCC->AHB1ENR  |=  RCC_AHB1ENR_DMA2EN;
            break;
        }
      #endif

      #if (SPI_DRIVER_SUPPORT_SPI6 == DEF_ENABLE)
        case DRIVER_SPI6_ID:
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB2RSTR |=  RCC_APB2RSTR_SPI5RST;	            // Enable SPI6 reset state
            RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI5RST;             // Release SPI6 from reset state
            RCC->APB2ENR  |=  RCC_APB2ENR_SPI5EN;	            // Enable SPI_PORT clock
            RCC->AHB1ENR  |=  RCC_AHB1ENR_DMA2EN;
            break;
        }
      #endif
    }

    // ---- Configure and enable SPI interrupt ----
    if(m_pInfo->CallBackISR != nullptr)
    {
        uint32_t Priority = NVIC_EncodePriority(NVIC_GetPriorityGrouping(), m_pInfo->PreempPrio, 0x00);
        NVIC_SetPriority(m_pInfo->IRQn, Priority);
        NVIC_EnableIRQ(m_pInfo->IRQn);
    }

    m_pInfo->pSPIx->CR1      = m_pInfo->portConfig;                                                 // Configuration for SPIx Port
    m_pInfo->pSPIx->I2SCFGR &= ~SPI_I2SCFGR_I2SMOD;                                                 // Activate the SPIx mode (Reset I2SMOD bit in I2SCFGR register)
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockToDevice
//
//  Parameter(s):   SPI_DeviceInfo_t*    pDevice
//  Return:         SystemState_e        Status
//
//  Description:    This routine will configure the SPI port to work with a specific device and
//                  lock it, so any other access to the port will be block until unlock
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::LockToDevice(SPI_DeviceInfo_t* pDevice)
{
    SystemState_e State = SYS_NOT_LOCK_TO_DEVICE;
    nOS_StatusReg sr;

    nOS_EnterCritical(sr);

    if(m_pDevice == nullptr)
    {
        m_pDevice       = pDevice;
        this->Lock();
        m_SlowSpeed     = GetPrescalerFromSpeed(m_pDevice->SlowSpeed);
        m_FastSpeed     = GetPrescalerFromSpeed(m_pDevice->FastSpeed);
        this->Config(LOW_SPEED);
        State = SYS_READY;
        m_State
    }

    nOS_LeaveCritical(sr);
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UnlockFromDevice
//
//  Parameter(s):   SPI_DeviceInfo_t*    pDevice
//  Return:         SystemState_e        Status
//
//  Description:    This routine will unlock SPI port from a specific device
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//                  if lock and no write at all if not lock to a device
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::UnlockFromDevice(SPI_DeviceInfo_t* pDevice)
{
    SystemState_e State = SYS_WRONG_DEVICE;
    nOS_StatusReg sr;

    nOS_EnterCritical(sr);

    if(pDevice == m_pDevice)
    {
        this->Unlock();
        m_pDevice = nullptr;
        m_State   = SYS_DEVICE_NOT_PRESENT;
        State     = SYS_READY
    }
    else if(m_pDevice == nullptr)
    {
        State = SYS_NOT_LOCK_TO_DEVICE;
    }

    nOS_LeaveCritical(sr);
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Read
//
//  Parameter(s):   ?                   See overload
//                  size_t  Size        Number of byte to read
//
//  Return:         SystemState_e        Error status SPI_PASS, SPI_PASS_NACK or SPI_FAIL
//
//  Description:    This routine perform read.
//
//  Note(s):        Multiple overload available
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Read(uint8_t* pBuffer, size_t Size)
{
    if(m_State == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    return this->Request(ACCESS_READ, pBuffer, Size);
}

SystemState_e SPI_Driver::Read(uint8_t* pData)
{
    if(m_State == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    return this->Request(ACCESS_READ, (uint8_t*)pData, sizeof(uint8_t));
}

SystemState_e SPI_Driver::Read(uint16_t* pData)
{
    SystemState_e State;

    if(m_State == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    State = this->Request(ACCESS_READ,  (uint8_t*)pData, sizeof(uint16_t));
    LIB_uint16_t_Swap(pData);
    return State;
}

SystemState_e SPI_Driver::Read(uint32_t* pData)
{
    SystemState_e State;

    if(m_Status == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    State = this->Request(ACCESS_READ, (uint8_t*)pData, sizeof(uint32_t));
    LIB_uint32_t_Swap(pData);
    return State;
}

// Lock version
SystemState_e SPI_Driver::Read(uint8_t* pBuffer, size_t Size, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_READ, pBuffer, Size);
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
}

SystemState_e SPI_Driver::Read(uint8_t* pData, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_READ, (uint8_t*)pData, sizeof(uint8_t));
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
}

SystemState_e SPI_Driver::Read(uint16_t* pData, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_READ, (uint8_t*)pData, sizeof(uint16_t));
    LIB_uint16_t_Swap(pData);
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
}

SystemState_e SPI_Driver::Read(uint32_t* pData, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_READ, (uint8_t*)pData, sizeof(uint32_t));
    LIB_uint32_t_Swap(pData);
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   ?                   See overload
//  Return:         SystemState_e        Status
//
//  Description:    This routine writes to the SPI bus.
//
//  Note(s):        Multiple overload available
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Write(const uint8_t* pBuffer, size_t Size)
{
    if(m_State == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    return this->Request(ACCESS_WRITE, (uint8_t*)pBuffer, Size);
}

SystemState_e SPI_Driver::Write(uint8_t Data)
{
    if(m_State == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    return this->Request(ACCESS_WRITE, (uint8_t*)&Data, sizeof(uint8_t));
}

SystemState_e SPI_Driver::Write(uint16_t Data)
{
    LIB_uint16_t_Swap(&Data);
    if(m_State == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    return this->Request(ACCESS_WRITE, (uint8_t*)&Data, sizeof(uint16_t));
}

SystemState_e SPI_Driver::Write(uint32_t Data)
{
    LIB_uint32_t_Swap(&Data);
    if(m_State == SYS_DEVICE_NOT_PRESENT) return SYS_DEVICE_NOT_PRESENT;
    return this->Request(ACCESS_WRITE, (uint8_t*)&Data, sizeof(uint32_t));
}

// Lock Version
SystemState_e SPI_Driver::Write(const uint8_t* pBuffer, size_t Size, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_WRITE, (uint8_t*)pBuffer, Size);
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
}

SystemState_e SPI_Driver::Write(uint8_t Data, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_WRITE, (uint8_t*)&Data, sizeof(uint8_t));
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
}

SystemState_e SPI_Driver::Write(uint16_t Data, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    LIB_uint16_t_Swap(&Data);
    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_WRITE, (uint8_t*)&Data, sizeof(uint16_t));
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
}

SystemState_e SPI_Driver::Write(uint32_t Data, SPI_DeviceInfo_t* pDevice)
{
    SystemState_e PriorityState;
    SystemState_e State;

    LIB_uint32_t_Swap(&Data);
    if((State     = this->LockToDevice(pDevice)) != SYS_READY) return State;
    PriorityState = this->Request(ACCESS_WRITE, (uint8_t*)&Data, sizeof(uint32_t));
    State         = this->UnlockFromDevice(pDevice);
    if(PriorityState != SYS_READY) State = PriorityState;

    return State;
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
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):
//   Return Value:
//
//   Description:   Lock the driver
//
//   Note(s):
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
//   Parameter(s):
//   Return Value:
//
//   Description:   Unlock the driver
//
//   Note(s):
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
//  Parameter(s):   DeviceSpeed_e  Speed
//  Return:         None
//
//  Description:    Set speed on the SPI port
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::Config(uint32_t Speed)
{
    m_pInfo->pSPIx->CR1 &= ~SPI_CR1_SPE;                                                            // Disable SPIx
    m_pInfo->pSPIx->CR1 &= ~SPI_PRESCALER_MASK;                                                     // Clear prescaler
//i have remove enum... so create it locally
//    m_pInfo->pSPIx->CR1 |= (Speed == FAST_SPEED) ? m_FastSpeed : m_SlowSpeed;                       // Apply prescaler
    m_pInfo->pSPIx->CR1 |= SPI_CR1_SPE;                                                             // Enable SPIx
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Transfer
//
//  Parameter(s):   pTX_Data        Pointer on the data buffer to transfer.
//                  TX_Size			Number of byte to send.
//                  pRX_pData       Pointer on the data buffer where to put received data.
//                  RX_Size			Number of byte to receive.
//                  pDevice         Pointer of the device using SPI (it's a unique ID in the system)
//  Return:         SystemState_e   State
//
//  Description:    Read or writes data to SPI device.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Transfer(const uint8_t* pTX_Data, uint32_t TX_Size, uint8_t* pRX_Data, uint32_t RX_Size, void* pDevice)
{
    SystemState_e State;

    if((State = LockToDevice(pDevice)) == SYS_READY)
    {
        State = Transfer(pTX_Data, TX_Size, pRX_Data, RX_Size);
        UnlockFromDevice(pDevice);
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
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (SPI_DRIVER_SUPPORT_DMA == DEF_ENABLED)
SystemState_e SPI_Driver::Transfer(const uint8_t* pTX_Data, uint32_t TX_Size, uint8_t* pRX_Data, uint32_t RX_Size)
{
	SPI_TypeDef*              pSPIx;
    DMA_Stream_TypeDef*       pDMA;
    uint32_t                  Flag;
    uint32_t                  Dummy;

    if(m_pDevice != nullptr)
    {
        if(((pTX_Data == NULL) || (TX_Size == 0)) &&
           ((pRX_Data == NULL) || (RX_Size == 0)))
        {
            return SYS_NULLPTR;
    }

        if(m_Status == SYS_READY)
    {
            CLEAR_BIT(m_pInfo->pSPIx->CR1, SPI_CR1_SPE);

            if(m_pInfo->PinNSS != IO_NOT_DEFINED)
            {
                IO_SetPinLow(m_pInfo->PinNSS);                              // Select the NSS pin
    }

            // ----------------------------------------------------------------------------------------
            // TX Setup

            // Note(s) needed by both TX and RX
            pDMA  = m_pInfo->DMA_StreamTX;                                  // In all case the first DMA set is the TX
            pSPIx = m_pInfo->pSPIx;
            Flag  = m_pInfo->TX_IT_Flag;

            if((pTX_Data != NULL) && (TX_Size != 0))
            {
                // TX DMA
                m_DMA_Status = SYS_BUSY_TX;                                 // Set flag to busy in TX
                pDMA->M0AR = ((uint32_t)pTX_Data);                          // Set DMA source
                pDMA->NDTR = TX_Size;                                       // Set size of the TX

                if(m_NoMemoryIncrement == false)
                {
                    SET_BIT(pDMA->CR, DMA_SxCR_MINC);                       // Enable transfer complete and memory increment
                }
                else
                {
                    CLEAR_BIT(pDMA->CR, DMA_SxCR_MINC);
                    m_NoMemoryIncrement == false;
                }

                SET_BIT(pDMA->CR, DMA_SxCR_EN);                             // Enable the DMA module
                DMA_ClearFlag(pDMA, Flag);                                  // Clear IRQ DMA flag

                // SPI
                if(m_pInfo->Control == SPI_HALF_DUPLEX)
                {
                    MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_HALF_DUPLEX_TX);
}

                SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                           // Enable SPI
                SET_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);                       // Enable DMA TX

                if(WaitDMA() == SYS_ERROR)
                {
                    if(m_pInfo->PinNSS != IO_NOT_DEFINED)
                    {
                        IO_SetPinHigh(m_pInfo->PinNSS);
                    }

                    return SYS_TIME_OUT;
                }

                CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                         // Disable SPI

                // Deactivate DMA
                CLEAR_BIT(pSPIx->CR2, SPI_CR2_TXDMAEN);
                CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);                           // Disable the DMA module
            }

            // ----------------------------------------------------------------------------------------
            // RX setup

            if((pRX_Data != NULL) && (RX_Size != 0))
            {

                while((pSPIx->SR & SPI_SR_RXNE) != 0)
                {
                    Dummy = pSPIx->DR;
                }

                m_DMA_Status = SYS_BUSY_RX;                                 // Set flag to busy in TX

                // TX DMA
                Dummy = 0xFF;                                               // Value to dummy TX
                pDMA->M0AR = (uint32_t)&Dummy;                              // Set DMA source for dummy TX
                pDMA->NDTR = RX_Size;                                       // Set size of the TX
                CLEAR_BIT(pDMA->CR, DMA_SxCR_MINC);
                SET_BIT(pDMA->CR, DMA_SxCR_EN);                             // Enable the DMA module
                DMA_ClearFlag(pDMA, Flag);                                  // Clear IRQ DMA flag
                // RX DMA
                pDMA = m_pInfo->DMA_StreamRX;                               // Now configure RX DMA
                Flag = m_pInfo->RX_IT_Flag;
                pDMA->M0AR = (uint32_t)pRX_Data;
                pDMA->NDTR = RX_Size;
                SET_BIT(pDMA->CR, DMA_SxCR_EN);                             // Enable the DMA module
                DMA_ClearFlag(pDMA, Flag);

                // SPI
                if(m_pInfo->Control == SPI_HALF_DUPLEX)
                {
                    MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_HALF_DUPLEX_RX);
                }

                SET_BIT(pSPIx->CR1, SPI_CR1_SPE);                           // Enable SPI
                SET_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));   // Start the process

                if(WaitDMA() == SYS_ERROR)
                {
                    if(m_pInfo->PinNSS != IO_NOT_DEFINED)
                    {
                        IO_SetPinHigh(m_pInfo->PinNSS);                     // There is an error then release NSS
                    }

                    return SYS_TIME_OUT;
                }


                // Deactivate DMA
                CLEAR_BIT(pSPIx->CR2, (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN));
                CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);                           // Disable the DMA RX module
                pDMA = m_pInfo->DMA_StreamTX;                               // In all case the first DMA set is the TX
                CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);                           // Disable the DMA TX module
            }

            // ----------------------------------------------------------------------------------------

            CLEAR_BIT(pSPIx->CR1, SPI_CR1_SPE);                             // Disable SPI
            if(m_pInfo->Control == SPI_HALF_DUPLEX)
            {
                MODIFY_REG(pSPIx->CR1, SPI_DUPLEX_MASK, SPI_FULL_DUPLEX);
            }


            if(m_pInfo->PinNSS != IO_NOT_DEFINED)
            {
                IO_SetPinHigh(m_pInfo->PinNSS);                             // Select the NSS pin
            }

            m_DMA_Status = SYS_IDLE;
            return SYS_READY;
        }

        return SYS_BUSY;
    }

    return SYS_DEVICE_NOT_PRESENT;
}
#endif // (SPI_DRIVER_SUPPORT_DMA == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           Request
//
//  Parameter(s):   AccessRequest_e  Request
//                  void*           pBuffer
//                  size_t          Size
//  Return:         SPI_eStatus     Status
//
//  Description:    Read or writes data to SPI device.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_Driver::Request(AccessRequest_e Request, uint8_t* pBuffer, size_t Size)
{
    if(m_pDevice != nullptr)
    {
      #if (SPI_DRIVER_SUPPORT_DMA == DEF_DEFINED)

        uint8_t    DummyTX = 0xFF;
        bool    bTransmitCompleted;

        this->Lock();

        DMA_DeInit(m_pInfo->pDMA_RX);
        m_pInfo->pDMA_RX->NDTR = Size;
        m_pInfo->pDMA_RX->PAR  = (uint32_t)&(m_pInfo->pSPIx->DR);

        DMA_DeInit(m_pInfo->pDMA_TX);
        m_pInfo->pDMA_TX->NDTR = Size;
        m_pInfo->pDMA_TX->PAR  = (uint32_t)&(m_pInfo->pSPIx->DR);

        m_pInfo->pDMA_RX->CR = m_pInfo->IRQ_Source        | m_pInfo->DMA_ChannelRX |
                               DMA_DIR_PeripheralToMemory | DMA_Mode_Normal        | DMA_FIFOMode_Disable        | DMA_Priority_VeryHigh    | DMA_PeripheralInc_Disable |
                               DMA_PeripheralBurst_Single | DMA_MemoryBurst_Single | DMA_PeripheralDataSize_Byte | DMA_MemoryDataSize_Byte;

        m_pInfo->pDMA_TX->CR = m_pInfo->IRQ_Source        | m_pInfo->DMA_ChannelTX |
                               DMA_DIR_MemoryToPeripheral | DMA_Mode_Normal        | DMA_FIFOMode_Disable        | DMA_Priority_VeryHigh    | DMA_PeripheralInc_Disable |
                               DMA_PeripheralBurst_Single | DMA_MemoryBurst_Single | DMA_PeripheralDataSize_Byte | DMA_MemoryDataSize_Byte;

        switch(Request)
        {
            case ACCESS_READ:
            {
                // DMA Channel configuration SPIx RX
                m_pInfo->pDMA_RX->CR   |= DMA_MemoryInc_Enable;
                m_pInfo->pDMA_RX->M0AR  = (uint32_t)pBuffer;

                // DMA Channel configuration SPIx TX
                m_pInfo->pDMA_TX->CR   |= DMA_MemoryInc_Disable;
                m_pInfo->pDMA_TX->M0AR  = (uint32_t)&DummyTX;
                break;
            }

            case ACCESS_WRITE:
            {
                // DMA Channel configuration SPIx RX
                m_pInfo->pDMA_RX->CR   |= DMA_MemoryInc_Disable;
                m_pInfo->pDMA_RX->M0AR  = (uint32_t)&DummyTX;
                // DMA Channel configuration SPIx TX
                m_pInfo->pDMA_TX->CR   |= DMA_MemoryInc_Enable;
                m_pInfo->pDMA_TX->M0AR  = (uint32_t)pBuffer;
                break;
            }
        }
        //dummy read to clear data register
        void(m_pInfo->pSPIx->DR);

        // Enable RX & TX Channel
        m_pInfo->pDMA_RX->CR |= DMA_SxCR_EN;
        m_pInfo->pDMA_TX->CR |= DMA_SxCR_EN;

        // Enable SPI TX/RX request
        m_Timeout = m_pDevice->TimeOut;
        m_pInfo->pSPIx->CR2 |= (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        do
        {
            nOS_Yield();

            if(m_pInfo->HardwarePort < 4) bTransmitCompleted = (m_pInfo->pDMA->LISR & m_pInfo->DMA_Flag_TC_RX) ? true : false;
            else                          bTransmitCompleted = (m_pInfo->pDMA->HISR & m_pInfo->DMA_Flag_TC_RX) ? true : false;
            m_Timeout--;
        }
        while((m_Timeout > 0) && (bTransmitCompleted == false));

        // Disable DMA RX & TX Channel
        m_pInfo->pDMA_RX->CR &= (uint16_t)(~DMA_SxCR_EN);
        m_pInfo->pDMA_TX->CR &= (uint16_t)(~DMA_SxCR_EN);

        // Disable SPIx RX/TX request
        m_pInfo->pSPIx->CR2 &= (uint16_t)~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

        this->Unlock();

        if(bTransmitCompleted != true) return SYS_TIME_OUT;

      #else // (SPI_DRIVER_SUPPORT_DMA == DEF_DEFINED)

        this->Lock();

        switch(Request)
        {
            case ACCESS_READ:
            {
                //dummy read to clear data register
                ((uint8_t*)pBuffer)[0] = m_pInfo->pSPIx->DR;

                for(size_t i = 0; i < Size; i++)
                {
                    m_pInfo->pSPIx->DR = 0xFF;
                    if(WaitReady() == false) return SYS_HUNG;
                    ((uint8_t*)pBuffer)[i] = m_pInfo->pSPIx->DR;
                }
                break;
            }
            case ACCESS_WRITE:
            {
                for(size_t i = 0; i < Size; i++)
                {
                    m_pInfo->pSPIx->DR = ((uint8_t*)pBuffer)[i];
                    WaitReady();
                }
                break;
            }
        }

        this->Unlock();

      #endif //(SPI_DRIVER_SUPPORT_DMA == DEF_DEFINED)
        return SYS_READY;
    }
    return SYS_DEVICE_NOT_PRESENT;
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

    m_Timeout = m_pDevice->TimeOut;
    while((m_pInfo->pSPIx->SR & SPI_I2S_FLAG_TXE)  == 0)                // Wait until transmit complete
    {
        nOS_Yield();
        if(m_Timeout == 0) return SYS_HUNG;
    }

    m_Timeout = m_pDevice->TimeOut;
    while((m_pInfo->pSPIx->SR & SPI_I2S_FLAG_RXNE) == 0)                // Wait until receive complete
    {
        nOS_Yield();
        if(m_Timeout == 0) return SYS_HUNG;
    }

    m_Timeout = m_pDevice->TimeOut;
    while((m_pInfo->pSPIx->SR & SPI_I2S_FLAG_BSY)  != 0)                // wait until SPI is not busy anymore
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
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SPI_Driver::IRQHandler(void)
{
    if(m_pInfo->CallBackISR != nullptr)
    {
        m_pInfo->CallBackISR();
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
