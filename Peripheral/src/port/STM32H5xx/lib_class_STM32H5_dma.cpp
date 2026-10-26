//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_dma.cpp
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define DMA_DRIVER_GLOBAL
#include "BSP_board.h"
#include "lib_class_STM32H5_dma.h"
#include <cstring>
#undef DMA_DRIVER_GLOBAL

DMA_Driver::DMA_Driver() :
    m_pChannel(nullptr),
    m_IsCircular(false),
    m_IsEnable(false),
    m_DataWidthShift(0),
    m_Direction(0),
    m_Request(0),
    m_IRQn_Channel(GPDMAx_NONE_IRQn),
    m_PreempPrio(0),
    m_Flag(0),
    m_pCallback(nullptr)
#if USE_DMA_LINKLIST == DEF_ENABLED
    ,
    m_UseLinkedList(false)
#endif
{
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   const DMA_Info_t*           pInfo
//  Return:         None
//
//  Description:    Initialize the DMA object
//
//  Note(s):        Add in direction support for M2M
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Initialize(const DMA_Info_t* pInfo)
{
    uint32_t Register1 = 0;
    uint32_t Register2;

    m_pChannel     = pInfo->pChannel;
    m_Flag         = pInfo->Flag;
    m_IRQn_Channel = pInfo->IRQn_Channel;
    m_Direction    = pInfo->Direction;
    m_PreempPrio   = pInfo->PreempPrio;
#if USE_DMA_LINKLIST == DEF_ENABLED
    m_UseLinkedList = pInfo->UseLinkedList;
#endif
    m_Request    = pInfo->Request;
    m_IsEnable   = false;
    m_IsCircular = false;
    m_pCallback  = nullptr;

    if(pInfo->Mode == BSP_DMA_MODE_CIRCULAR)
    {
        m_IsCircular = true;
    }

    EnableClock();

    SET_BIT(m_pChannel->CCR, DMA_CCR_RESET);

    m_pChannel->CCR = pInfo->Priority;

    // We only support transfers of identical size (i.e. source data width == destination data width)
    // The transfer size in the CBR1 register must be the number of bytes to transfer (i.e. number of elements * size of each element)
    switch(pInfo->SrcAndDestDataWidth)
    {
        case BSP_DMA_TRANFER_DATA_SIZE_8_BITS:
            m_DataWidthShift = 0;
            break;
        case BSP_DMA_TRANFER_DATA_SIZE_16_BITS:
            m_DataWidthShift = 1;
            break;
        case BSP_DMA_TRANFER_DATA_SIZE_32_BITS:
            m_DataWidthShift = 2;
            break;
        default:
            ASSERT(0);
    }

    switch(m_Direction)  // See ST DMA overview doc
    {
        case BSP_DMA_PERIPHERAL_TO_MEMORY:
            Register1 = (BSP_DMA_SOURCE_ALLOCATED_PORT0 | BSP_DMA_DESTINATION_ALLOCATED_PORT1);
            break;
        case BSP_DMA_MEMORY_TO_PERIPHERAL:
            Register1 = (BSP_DMA_SOURCE_ALLOCATED_PORT1 | BSP_DMA_DESTINATION_ALLOCATED_PORT0);
            break;
        case BSP_DMA_MEMORY_TO_MEMORY:
            Register1 = (BSP_DMA_SOURCE_ALLOCATED_PORT1 | BSP_DMA_DESTINATION_ALLOCATED_PORT1);
            break;
        default:
            ASSERT(0);
    }

    Register1 |= (pInfo->SrcAndDestIncrement | pInfo->SrcAndDestDataWidth | ((pInfo->SourceBurstLength - 1U) << DMA_CTR1_SBL_1_Pos) | ((pInfo->DestinationBurstLength - 1U) << DMA_CTR1_DBL_1_Pos));

    Register2 = (pInfo->Request | pInfo->BlockHardwareRequest | pInfo->TransferEventMode | pInfo->Direction | pInfo->Mode);

#if USE_DMA_LINKLIST == DEF_ENABLED
    if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
    {
        std::memset(m_NodeInfo.LLRegisters, 0, sizeof(m_NodeInfo.LLRegisters));  // Reset Linked-List temporary array
        m_NodeInfo.LLRegisters[DMA_NODE_CTR1_IDX] = Register1;
        m_NodeInfo.LLRegisters[DMA_NODE_CTR2_IDX] = Register2;
    }
    else
#endif
    {
        m_pChannel->CTR1  = Register1;
        m_pChannel->CTR2  = Register2;
        m_pChannel->CLBAR = 0;
        m_pChannel->CLLR  = 0;
    }

    if(m_IRQn_Channel != GPDMAx_NONE_IRQn)
    {
        EnableIRQ();
        EnableInterrupt(m_Flag);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetTransfer, SetSource, SetDestination, SetLength
//
//  Parameter(s):   Source
//                  Destination
//                  Length
//  Return:         None
//
//  Description:    Setup transfer from source to destination. according to configuration
//
//  Note(s):        Add in direction support for M2M
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetTransfer(void* pSource, void* pDestination, size_t Length)
{
    SetSource(pSource);
    SetDestination(pDestination);
    SetLength(Length);

#if USE_DMA_LINKLIST == DEF_ENABLED
    // Once transfer has been configured, we can write all the transfer registers
    if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
    {
        WriteTransferRegisters();
    }
#endif
}

void DMA_Driver::SetSource(void* pSource)
{
#if USE_DMA_LINKLIST == DEF_ENABLED
    if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
    {
        m_NodeInfo.LLRegisters[DMA_NODE_CSAR_IDX] = uint32_t(pSource);  // Save temporarily until transfer is started
    }
    else
#endif
    {
        m_pChannel->CSAR = uint32_t(pSource);
    }
}

void DMA_Driver::SetDestination(void* pDestination)
{
#if USE_DMA_LINKLIST == DEF_ENABLED
    if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
    {
        m_NodeInfo.LLRegisters[DMA_NODE_CDAR_IDX] = uint32_t(pDestination);  // Save temporarily until transfer is started
    }
    else
#endif
    {
        m_pChannel->CDAR = uint32_t(pDestination);
    }
}

void DMA_Driver::SetLength(size_t Length)
{
#if USE_DMA_LINKLIST == DEF_ENABLED
    if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
    {
        m_NodeInfo.LLRegisters[DMA_NODE_CBR1_IDX] = uint32_t(Length << m_DataWidthShift);
    }
    else
#endif
    {
        m_pChannel->CBR1 = uint32_t(Length << m_DataWidthShift);
    }
}

void DMA_Driver::SetFastDestinationAndLength(void* pDestination, size_t Length)
{
    SET_BIT(m_pChannel->CCR, DMA_CCR_SUSP);
    SET_BIT(m_pChannel->CCR, DMA_CCR_RESET);  // Reset the channel
    m_pChannel->CFCR = DMA_ALL_FLAG;          // Clear all status flags
    m_pChannel->CDAR = uint32_t(pDestination);
    m_pChannel->CBR1 = uint32_t(Length << m_DataWidthShift);
    SET_BIT(m_pChannel->CCR, DMA_CCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       WriteTransferRegisters
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Write registers according to transfer configuration (i.e. transfer must already be configured)
//
//-------------------------------------------------------------------------------------------------

#if USE_DMA_LINKLIST == DEF_ENABLED
void DMA_Driver::WriteTransferRegisters(void)
{
    uint32_t* pLinkedListNode;

    if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
    {
        m_pChannel->CTR1 = m_NodeInfo.LLRegisters[DMA_NODE_CTR1_IDX];
        m_pChannel->CTR2 = m_NodeInfo.LLRegisters[DMA_NODE_CTR2_IDX];
        m_pChannel->CBR1 = m_NodeInfo.LLRegisters[DMA_NODE_CBR1_IDX];
        m_pChannel->CSAR = m_NodeInfo.LLRegisters[DMA_NODE_CSAR_IDX];
        m_pChannel->CDAR = m_NodeInfo.LLRegisters[DMA_NODE_CDAR_IDX];
    }

    if((m_IsCircular == false) || (m_UseLinkedList == DMA_LINKED_LIST_DISABLE))
    {
        m_pChannel->CLBAR = 0;
        m_pChannel->CLLR  = 0;
    }
    else
    {
        pLinkedListNode                           = &m_NodeInfo.LLRegisters[0];
        m_NodeInfo.LLRegisters[DMA_NODE_CLLR_IDX] = (uint32_t(pLinkedListNode) & BSP_DMA_LINKED_LIST_ADDRESS_OFFSET_MASK) | BSP_DMA_LINKED_LIST_CTR1_UPDATE | BSP_DMA_LINKED_LIST_CTR2_UPDATE | BSP_DMA_LINKED_LIST_CBR1_UPDATE | BSP_DMA_LINKED_LIST_CSAR_UPDATE | BSP_DMA_LINKED_LIST_CDAR_UPDATE;

        // Configure the linked list registers
        m_pChannel->CLLR                          = m_NodeInfo.LLRegisters[DMA_NODE_CLLR_IDX];
        m_pChannel->CLBAR                         = (uint32_t(pLinkedListNode) & BSP_DMA_LINKED_LIST_BASE_ADDRESS_MASK);
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       Disable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Make sure to disable the specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Disable(void)
{
    SET_BIT(m_pChannel->CCR, DMA_CCR_SUSP);
    SET_BIT(m_pChannel->CCR, DMA_CCR_RESET);  // Reset the channel
    ClearAllFlag();                           // Clear all status flags
    m_IsEnable = false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CheckFlag
//
//  Parameter(s):   Flag        Flag to check
//  Return:         bool        If true then flag is set.
//
//  Description:    Check flag for specific DMA stream.
//-------------------------------------------------------------------------------------------------
bool DMA_Driver::CheckFlag(uint32_t Flag)
{
    return (m_pChannel->CSR & Flag) != 0U;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableClock
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the associated DMA module clock
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableClock(void)
{
    if(uintptr_t(m_pChannel) < GPDMA2_BASE)
    {
        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPDMA1EN);
    }
    else
    {
        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPDMA2EN);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetMemoryIncrement
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Perform emory increment
//
//  Notes(s) :      Only used this for transfer from or to a peripheral, mainly used for dummy TX
//                  or RX
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetMemoryIncrement(void)
{
    if(m_Direction == BSP_DMA_PERIPHERAL_TO_MEMORY)
    {
#if USE_DMA_LINKLIST == DEF_ENABLED
        if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
        {
            SET_BIT(m_NodeInfo.LLRegisters[DMA_NODE_CTR1_IDX], DMA_CTR1_DINC);
        }
        else
#endif
        {
            SET_BIT(m_pChannel->CTR1, DMA_CTR1_DINC);
        }
    }
    else
    {
#if USE_DMA_LINKLIST == DEF_ENABLED
        if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
        {
            SET_BIT(m_NodeInfo.LLRegisters[DMA_NODE_CTR1_IDX], DMA_CTR1_SINC);
        }
        else
#endif
        {
            SET_BIT(m_pChannel->CTR1, DMA_CTR1_SINC);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetNoMemoryIncrement
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Do not perform emory increment
//
//  Notes(s) :      Only used this for transfer from or to a peripheral, mainly used for dummy TX
//                  or RX
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetNoMemoryIncrement(void)
{
    if(m_Direction == BSP_DMA_PERIPHERAL_TO_MEMORY)
    {
#if USE_DMA_LINKLIST == DEF_ENABLED
        if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
        {
            CLEAR_BIT(m_NodeInfo.LLRegisters[DMA_NODE_CTR1_IDX], DMA_CTR1_DINC);
        }
        else
#endif
        {
            CLEAR_BIT(m_pChannel->CTR1, DMA_CTR1_DINC);
        }
    }
    else
    {
#if USE_DMA_LINKLIST == DEF_ENABLED
        if(m_UseLinkedList == DMA_LINKED_LIST_ENABLE)
        {
            CLEAR_BIT(m_NodeInfo.LLRegisters[DMA_NODE_CTR1_IDX], DMA_CTR1_SINC);
        }
        else
#endif
        {
            CLEAR_BIT(m_pChannel->CTR1, DMA_CTR1_SINC);
        }
    }
}

void DMA_Driver::Enable(void)
{
    SET_BIT(m_pChannel->CCR, DMA_CCR_EN);
    m_IsEnable = true;
}

bool DMA_Driver::IsItEnable(void)
{
    return m_IsEnable;
}

void DMA_Driver::EnableCircular(void)
{
    m_IsCircular = true;
}

void DMA_Driver::DisableCircular(void)
{
    m_IsCircular = false;
}

void DMA_Driver::ClearFlag(void)
{
    m_pChannel->CFCR = m_Flag;
}

void DMA_Driver::ClearAllFlag(void)
{
    m_pChannel->CFCR = DMA_ALL_FLAG;
}

size_t DMA_Driver::GetRemainder(void)
{
    return size_t((m_pChannel->CBR1 & DMA_CBR1_BNDT) >> m_DataWidthShift);
}

void DMA_Driver::SetFifoControl(uint32_t Control)
{
    m_pChannel->CSR = Control;
}

void DMA_Driver::EnableIRQ(void)
{
    ISR_Init(m_IRQn_Channel, m_PreempPrio);
}

void DMA_Driver::EnableInterrupt(uint32_t Interrupt)
{
    SET_BIT(m_pChannel->CCR, Interrupt);
}

void DMA_Driver::DisableInterrupt(uint32_t Interrupt)
{
    CLEAR_BIT(m_pChannel->CCR, Interrupt);
}

void DMA_Driver::EnableTransmitCompleteInterrupt(void)
{
    SET_BIT(m_pChannel->CCR, DMA_CCR_TCIE);
}

void DMA_Driver::DisableTransmitCompleteInterrupt(void)
{
    CLEAR_BIT(m_pChannel->CCR, DMA_CCR_TCIE);
}

void DMA_Driver::RegisterCallback(DmaCallback_t pCallback)
{
    m_pCallback = pCallback;
}