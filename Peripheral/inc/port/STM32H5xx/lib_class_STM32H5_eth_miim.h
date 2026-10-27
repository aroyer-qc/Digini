//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_eth_miim.h
//
//  MII Management Interface (AKA MMD)
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
#include <cstdint>
#include "lib_define.h"
#include "lib_class_STM32H5_eth.h"
#include "IPhyMmd.h"
#include "Freertos.h"
#include "semphr.h"


//-------------------------------------------------------------------------------------------------
// define(s)
//-------------------------------------------------------------------------------------------------
#define PHY_ADDR_MIN            0
#define PHY_ADDR_MAX            31

#define MMD_REGISTER_WIDTH_BITS 16
#define MMD_REGISTER_ADDR_MIN   0
#define MMD_REGISTER_ADDR_MAX   31

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------
// This is a singleton class

class EthMiim : public IPhyMmd
{
  public:
    EthMiim(const EthMiim& obj)               = delete;  // Deleting the copy constructor to prevent copies
    EthMiim&        operator=(const EthMiim&) = delete;  // Deleting the copy assignment operator to prevent copies

    static EthMiim& GetInstance()
    {
        static EthMiim instance;
        return instance;
    }

    SystemState_e      Initialize(ETH_Driver& EthernetDriver);
    SystemState_e      Deinitialize(void);

    SystemState_e      ReadRegister(uint8_t phyAddr, uint8_t phyRegAddr, uint16_t* pRegValue)  override;
    SystemState_e      WriteRegister(uint8_t phyAddr, uint8_t phyRegAddr, uint16_t regValue)  override;


  private:
    EthMiim() = default;

    ETH_HandleTypeDef*         m_pETH_Handle;
    bool                       m_IsInitialized;
    SemaphoreHandle_t          m_Mutex;  // MIIM resource protection mutex
};

//-------------------------------------------------------------------------------------------------
#endif  // __cplusplus
