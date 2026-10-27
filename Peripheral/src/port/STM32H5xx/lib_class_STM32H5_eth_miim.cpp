//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_eth_miim.cpp
//
//-------------------------------------------------------------------------------------------------
//
// # Limitations
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------
#include "BSP_board.h"
#include "lib_define.h"
#include "lib_class_STM32H5_eth_miim.h"
#include "stm32h5xx_hal_eth.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------
#define MIIM_WAIT_TIMEOUT_MS        500

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

// PHY Media Independent Interface Management (MIIM)
//-------------------------------------------------------------------------------------------------
//
//   Function:      MIIM_Initialize
//
//   Return Value:  Operation status
//                 - SYS_ERROR when the ethernet driver is powered down
//                 - SYS_NOT_INITIALIZE when driver initialization is not
//                 - SYS_READY indicates a frame is ready to be consummed
//                 - SYS_TIME_OUT a timeout while waiting or other errors
//
//
//   Description:  Initialize the MII management interface to read and write PHY registers
//
//-------------------------------------------------------------------------------------------------
SystemState_e EthMiim::Initialize(ETH_Driver& EthernetDriver)
{
    UNUSED(EthernetDriver);
    m_pETH_Handle = EthernetDriver.GetEthHandle();
    
    // create a binary semaphore used to serialize access to the MIIM (aka MMD) port
    m_Mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(m_Mutex);

    // ------------------ MDIO CSR Clock Range Configuration --------------------
    HAL_ETH_SetMDIOClockRange(m_pETH_Handle);

    // ------------------ Configure IO pins -------------------------------------
    IO_PinInit(IO_ETH_MDC);
    IO_PinInit(IO_ETH_MDIO);

    m_IsInitialized = true;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      MIIM_ReadRegister
//
//   Return Value:  Operation status
//                 - SYS_ERROR_NONE indicates successful read operation
//                 - SYS_ERROR when the ethernet driver is powered down
//                 - SYS_NOT_INITIALIZE when driver initialization is not
//                 - SYS_TIME_OUT a timeout while waiting or other errors
//
//
//   Description:  Read a PHY registers
//
//-------------------------------------------------------------------------------------------------
SystemState_e EthMiim::ReadRegister(uint8_t phyAddr, uint8_t phyRegAddr, uint16_t* pRegValue)
{
    if(!m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }
    else if(xSemaphoreTake(m_Mutex, MIIM_WAIT_TIMEOUT_MS) == pdFALSE)
    {
        return SYS_TIME_OUT;
    }

    uint32_t regVal;
    auto halRetVal = HAL_ETH_ReadPHYRegister(m_pETH_Handle, phyAddr, phyRegAddr, &regVal);
    *pRegValue = regVal;
    xSemaphoreGive(m_Mutex);
    if(HAL_ERROR == halRetVal)
    {
        return SYS_ERROR;
    }
    else
    {
        return SYS_ERROR_NONE;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      MIIM_WriteRegister
//
//   Return Value:  Operation status
//                 - SYS_ERROR_NONE indicates successful write operation
//                 - SYS_ERROR when the ethernet driver is powered down
//                 - SYS_NOT_INITIALIZE when driver initialization is not
//                 - SYS_TIME_OUT a timeout while waiting or other errors
//
//
//   Description:  Write a PHY registers
//
//-------------------------------------------------------------------------------------------------
SystemState_e EthMiim::WriteRegister(uint8_t phyAddr, uint8_t phyRegAddr, uint16_t regValue)
{
    if(!m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }
    else if(xSemaphoreTake(m_Mutex, MIIM_WAIT_TIMEOUT_MS) == pdFALSE)
    {
        return SYS_TIME_OUT;
    }

    auto halRetVal = HAL_ETH_WritePHYRegister(m_pETH_Handle, phyAddr, phyRegAddr, regValue);
    xSemaphoreGive(m_Mutex);
    if(HAL_ERROR == halRetVal)
    {
        return SYS_ERROR;
    }
    else
    {
        return SYS_ERROR_NONE;
    }
}
