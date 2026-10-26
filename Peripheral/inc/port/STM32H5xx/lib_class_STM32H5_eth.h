//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_eth.h
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Ethernet MAC Frame Transmit Flags
#define ETH_MAC_TX_FRAME_FRAGMENT   (1UL << 0)  // Indicate frame fragment
#define ETH_MAC_TX_FRAME_EVENT      (1UL << 1)  // Generate event when frame is transmitted
#define ETH_MAC_TX_FRAME_TIMESTAMP  (1UL << 2)  // Capture frame time stamp

// Ethernet MAC VLAN Filter Flag
#define ETH_MAC_VLAN_FILTER_ID_ONLY (1UL << 16)  // Compare only the VLAN Identifier (12-bit)

// Ethernet MAC Event
#define ETH_MAC_EVENT_RX_FRAME      (1UL << 0)  // Frame Received
#define ETH_MAC_EVENT_TX_FRAME      (1UL << 1)  // Frame Transmitted
#define ETH_MAC_EVENT_WAKEUP        (1UL << 2)  // Wake-up (on Magic Packet)
#define ETH_MAC_EVENT_TIMER_ALARM   (1UL << 3)  // Timer Alarm

// EMAC Driver state flags
#define ETH_EMAC_FLAG_POWER         (1 << 0)  // Driver power on
#define ETH_EMAC_FLAG_DMA_INIT      (1 << 1)  // DMA Initialized

// MAC address configuration.
#define MAC_ADDR0                   (0x00)
#define MAC_ADDR1                   (((char*)0x08FFF800)[0])
#define MAC_ADDR2                   (((char*)0x08FFF800)[1])
#define MAC_ADDR3                   (((char*)0x08FFF800)[2])
#define MAC_ADDR4                   (((char*)0x08FFF800)[3])
#define MAC_ADDR5                   (((char*)0x08FFF800)[4])

//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
#include <cstdint>
#include "lib_define.h"
#include "stm32h5xx.h"
#include "stm32h5xx_hal_eth.h"
#include "Legacy/stm32_hal_legacy.h"
#include "phy_driver_interface.h"

extern "C"
{

    // Interface functions called by the IP Stack
    void OnRxBufferAvailable(void);

    // HAL Callback functions implemented in this driver
    void HAL_ETH_MspInit(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_MspDeInit(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_ErrorCallback(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_RxAllocateCallback(uint8_t** buff);
    void HAL_ETH_RxLinkCallback(void** pStart, void** pEnd, uint8_t* buff, uint16_t Length);
    void HAL_ETH_TxFreeCallback(uint32_t* buff);

}  // extern "C"


//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

// Ethernet MAC Time
struct ETH_MAC_Time_t
{
    uint32_t nSec;  // Nano seconds
    uint32_t Sec;   // Seconds
};

// Ethernet MAC Address
struct ETH_MAC_Address_t
{
    uint8_t MAC[6];  // MAC Address (6 bytes), MSB first
};

typedef void (*bufferAllocatorFct)(uint8_t** buff);
typedef void (*bufferLinkerFct)(void** pStart, void** pEnd, uint8_t* buff, uint16_t Length);
typedef void (*bufferFreeFct)(uint32_t* buff);

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------
// This is a singleton class

class ETH_Driver
{
  public:
    ETH_Driver(const ETH_Driver& obj)               = delete;  // Deleting the copy constructor to prevent copies
    ETH_Driver&        operator=(const ETH_Driver&) = delete;  // Deleting the copy assignment operator to prevent copies

    static ETH_Driver& GetInstance()
    {
        static ETH_Driver instance;
        return instance;
    }

    SystemState_e      Initialize(const ETH_MediaInterfaceTypeDef phyIfMode, PHY_DriverInterface* pPhy);
    SystemState_e      Deinitialize(void);

    void               RegisterStackCallback(bufferAllocatorFct rxBuffAllocator, bufferLinkerFct rxBuffLink, bufferFreeFct txBuffFree);
    SystemState_e      GetMacAddress(ETH_MAC_Address_t* pAddress) const;
    SystemState_e      SetMacAddress(const ETH_MAC_Address_t* pAddress);
    SystemState_e      SetAddressFilter(const ETH_MAC_Address_t* pAddress, uint32_t NumberOfAddress);  // Configure Address Filter.
    SystemState_e      SendFrame(ETH_BufferTypeDef* txBuffers, uint32_t len, void* appPtr);
    SystemState_e      ReadFrame(void** ppBuf);
    SystemState_e      WaitRxFrame(uint32_t timeoutMs);
    SystemState_e      MAC_Receiver(const FunctionalState state);    // Enable or disable the receiver
    SystemState_e      ConfigurePhyIfMode(const PHY_Status_e Mode);  // Configure the mode (SPEED and DUPLEX)

    static void        IRQHandlerImpl();  // Global Ethernet Interrupt handler
    static void        MspInit();
    static void        MspDeInit();
    static void        RxCpltCallback(ETH_HandleTypeDef* pETH_Handle);
    static void        TxCpltCallback(ETH_HandleTypeDef* pETH_Handle);
    static void        ErrorCallback(ETH_HandleTypeDef* pETH_Handle);
    static void        SignalRxBufferAvailable(void);

    ETH_HandleTypeDef* GetEthHandle()
    {
        return &m_ETH_Handle;
    }

  private:
    ETH_Driver() = default;
    static void                GetMAC_From_GUID(uint8_t* pMAC);

    ETH_HandleTypeDef          m_ETH_Handle;
    ETH_BufferTypeDef          m_RX_Buffer;  // Receive buffer pointers
    ETH_MACFilterConfigTypeDef m_MAC_Filter;
    ETH_MACConfigTypeDef       m_MAC_Config;
    PHY_DriverInterface*       m_pPhy = nullptr;  // Pointer to the PHY
    uint8_t                    m_Flags;
    bool                       m_IsInitialized;
};

//-------------------------------------------------------------------------------------------------
#endif  // __cplusplus
