//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_dma.h
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
#include "stm32h5xx.h"
#include "lib_STM32H5_isr.h"

//-------------------------------------------------------------------------------------------------
// Define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

#define USE_DMA_LINKLIST                         DEF_ENABLED
#define GPDMAx_NONE_IRQn                         ((IRQn_Type)255)

// CCR Definition
#define BSP_DMA_PRIORITY_LOW_PRIO_WEIGHT_LOW     ((uint32_t)0x0000000)       // Low Priority level : Low weight
#define BSP_DMA_PRIORITY_LOW_PRIO_WEIGHT_MEDIUM  ((uint32_t)DMA_CCR_PRIO_0)  // Low Priority level : Medium weight
#define BSP_DMA_PRIORITY_LOW_PRIO_WEIGHT_HIGH    ((uint32_t)DMA_CCR_PRIO_1)  // Low Priority level : High weight
#define BSP_DMA_PRIORITY_HIGH_PRIO               ((uint32_t)DMA_CCR_PRIO)    // High Priority level

// CTR1 Definition
#define BSP_DMA_SOURCE_NO_INCREMENT              ((uint32_t)0x00000000)     // Source increment mode Disable
#define BSP_DMA_SOURCE_INCREMENT                 ((uint32_t)DMA_CTR1_SINC)  // Source increment mode Enable

#define BSP_DMA_DESTINATION_NO_INCREMENT         ((uint32_t)0x00000000)     // Destination increment mode Disable
#define BSP_DMA_DESTINATION_INCREMENT            ((uint32_t)DMA_CTR1_DINC)  // Destination increment mode Enable

#define BSP_DMA_SOURCE_SIZE_8_BITS               ((uint32_t)0x00000000)           // Source data width : uint8_t
#define BSP_DMA_SOURCE_SIZE_16_BITS              ((uint32_t)DMA_CTR1_SDW_LOG2_0)  // Source data width : uint16_t
#define BSP_DMA_SOURCE_SIZE_32_BITS              ((uint32_t)DMA_CTR1_SDW_LOG2_1)  // Source data width : uint32_t

#define BSP_DMA_DESTINATION_SIZE_8_BITS          ((uint32_t)0x00000000)           // Destination data width : uint8_t
#define BSP_DMA_DESTINATION_SIZE_16_BITS         ((uint32_t)DMA_CTR1_DDW_LOG2_0)  // Destination data width : uint16_t
#define BSP_DMA_DESTINATION_SIZE_32_BITS         ((uint32_t)DMA_CTR1_DDW_LOG2_1)  // Destination data width : uint32_t

#define BSP_DMA_TRANFER_DATA_SIZE_8_BITS         (BSP_DMA_SOURCE_SIZE_8_BITS  | BSP_DMA_DESTINATION_SIZE_8_BITS)
#define BSP_DMA_TRANFER_DATA_SIZE_16_BITS        (BSP_DMA_SOURCE_SIZE_16_BITS | BSP_DMA_DESTINATION_SIZE_16_BITS)
#define BSP_DMA_TRANFER_DATA_SIZE_32_BITS        (BSP_DMA_SOURCE_SIZE_32_BITS | BSP_DMA_DESTINATION_SIZE_32_BITS)

// CTR2 Definition
#define BSP_DMA_PERIPHERAL_TO_MEMORY             ((uint32_t)0x00000000U)     // Peripheral to memory direction
#define BSP_DMA_MEMORY_TO_PERIPHERAL             ((uint32_t)DMA_CTR2_DREQ)   // Memory to peripheral direction
#define BSP_DMA_MEMORY_TO_MEMORY                 ((uint32_t)DMA_CTR2_SWREQ)  // Memory to memory direction

#define BSP_DMA_BREQ_SINGLE_BURST                ((uint32_t)0x00000000U)    // Hardware request protocol at a single / burst level
#define BSP_DMA_BLOCK_HARDWARE_REQUEST           ((uint32_t)DMA_CTR2_BREQ)  // Hardware request protocol at a block level

#define BSP_DMA_MODE_NORMAL                      ((uint32_t)0x00000000)      // Normal Mode
#define BSP_DMA_MODE_CIRCULAR                    ((uint32_t)0x00000001)
#define BSP_DMA_MODE_PERIPHERAL_FLOW_CTRL        ((uint32_t)DMA_CTR2_PFREQ)  // Peripheral flow control mode

#define BSP_DMA_SOURCE_ALLOCATED_PORT0           ((uint32_t)0x00000000U)
#define BSP_DMA_SOURCE_ALLOCATED_PORT1           ((uint32_t)DMA_CTR1_SAP)
#define BSP_DMA_DESTINATION_ALLOCATED_PORT0      ((uint32_t)0x00000000U)
#define BSP_DMA_DESTINATION_ALLOCATED_PORT1      ((uint32_t)DMA_CTR1_DAP)

#define BSP_DMA_TC_EVENT_BLOCK_TRANSFER          ((uint32_t)0x00000000U)      // TC at end of each block
#define BSP_DMA_TC_EVENT_REPEATED_BLOCK_TRANSFER ((uint32_t)DMA_CTR2_TCEM_0)  // TC at end of repeated block

// Register definitions for circular mode
// Only circular mode with one (1) item is supported for linked lists
#define BSP_DMA_LINKED_LIST_REGISTER_COUNT       8

#define BSP_DMA_LINKED_LIST_BASE_ADDRESS_MASK    ((uint32_t)0xFFFF0000)
#define BSP_DMA_LINKED_LIST_ADDRESS_OFFSET_MASK  ((uint32_t)0x0000FFFC)

#define BSP_DMA_LINKED_LIST_CTR1_UPDATE          ((uint32_t)DMA_CLLR_UT1)
#define BSP_DMA_LINKED_LIST_CTR2_UPDATE          ((uint32_t)DMA_CLLR_UT2)
#define BSP_DMA_LINKED_LIST_CBR1_UPDATE          ((uint32_t)DMA_CLLR_UB1)
#define BSP_DMA_LINKED_LIST_CSAR_UPDATE          ((uint32_t)DMA_CLLR_USA)
#define BSP_DMA_LINKED_LIST_CDAR_UPDATE          ((uint32_t)DMA_CLLR_UDA)

#define DMA_LINKED_LIST_ENABLE                   true
#define DMA_LINKED_LIST_DISABLE                  false

#define DMA_ALL_FLAG                             (DMA_CSR_IDLEF | DMA_CSR_TCF | DMA_CSR_HTF | DMA_CSR_DTEF | DMA_CSR_ULEF | DMA_CSR_USEF | DMA_CSR_SUSPF | DMA_CSR_TOF | DMA_CSR_FIFOL)

#ifdef __cplusplus

// Legacy-compatible callback typedef (pointer)
typedef void (*DmaCallback_t)(int Type, void* pContext);

// Typedef(s)
struct DMA_Info_t
{
    // Configuration
    uint32_t             Mode;
    uint32_t             Request;
    uint32_t             BlockHardwareRequest;
    uint32_t             Direction;
    uint32_t             SrcAndDestIncrement;
    uint32_t             SrcAndDestDataWidth;
    uint32_t             Priority;
    uint32_t             SourceBurstLength;
    uint32_t             DestinationBurstLength;
    uint32_t             TransferEventMode;
    DMA_Channel_TypeDef* pChannel;
    uint32_t             Flag;
    IRQn_Type            IRQn_Channel;
    uint8_t              PreempPrio;
#if USE_DMA_LINKLIST == DEF_ENABLED
    bool                 UseLinkedList;
#endif
};

enum DMA_NodeRegisterIndex_e
{
    DMA_NODE_CTR1_IDX = 0,
    DMA_NODE_CTR2_IDX = 1,
    DMA_NODE_CBR1_IDX = 2,
    DMA_NODE_CSAR_IDX = 3,
    DMA_NODE_CDAR_IDX = 4,
    DMA_NODE_CLLR_IDX = 5
};

// This structure represents the linked list structure.
// The order of the enum above must match the order of the registers in the GPDMA peripheral of the STM32H5 MCU
struct DMA_NodeInfo_t
{
    // All registers are required but only the first 6 are used (up to CxLLR)
    uint32_t LLRegisters[BSP_DMA_LINKED_LIST_REGISTER_COUNT];
};

// Class Definitions
class DMA_Driver
{
  public:
    DMA_Driver();

    void Initialize(const DMA_Info_t* pInfo);
    void SetTransfer(void* pSource, void* pDestination, size_t Length);
#if USE_DMA_LINKLIST == DEF_ENABLED
    void WriteTransferRegisters(void);
#endif
    bool CheckFlag(uint32_t Flag);

    void SetMemoryIncrement(void);
    void SetNoMemoryIncrement(void);
    void Disable(void);

    void SetSource(void* pSource);
    void SetDestination(void* pDestination);
    void SetLength(size_t Length);

    // Legacy fast helper (now implemented in .cpp)
    void SetFastDestinationAndLength(void* pDestination, size_t Length);

    void Enable(void);
    bool IsItEnable(void);

    void EnableCircular(void);
    void DisableCircular(void);

    void ClearFlag(void);
    void ClearAllFlag(void);

    size_t GetRemainder(void);
    void   SetFifoControl(uint32_t Control);

    void EnableIRQ(void);
    void EnableInterrupt(uint32_t Interrupt);
    void DisableInterrupt(uint32_t Interrupt);
    void EnableTransmitCompleteInterrupt(void);
    void DisableTransmitCompleteInterrupt(void);

    // Legacy callback support (kept for compatibility, even if not used everywhere)
    void RegisterCallback(DmaCallback_t pCallback);

  private:
    void EnableClock(void);

    DMA_Channel_TypeDef* m_pChannel;
    DMA_NodeInfo_t       m_NodeInfo;
    bool                 m_IsCircular;
    bool                 m_IsEnable;
    uint32_t             m_DataWidthShift;
    uint32_t             m_Direction;
    uint32_t             m_Request;
    IRQn_Type            m_IRQn_Channel;
    uint8_t              m_PreempPrio;
    uint32_t             m_Flag;
    DmaCallback_t        m_pCallback;
#if USE_DMA_LINKLIST == DEF_ENABLED
    bool                 m_UseLinkedList;
#endif
};

#endif  // __cplusplus