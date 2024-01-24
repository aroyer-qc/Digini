//-------------------------------------------------------------------------------------------------
//
//  File : lib_chain_list.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   ChainList
//
//   Parameter(s):  size_t      NodeDataSize
//
//   Description:   Initializes internal chain list pointer and size of data struct
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
ChainList::ChainList(size_t NodeDataSize)
{
    m_pFirstNode    = nullptr;
    m_pLastNode     = nullptr;
    m_pScanNode     = nullptr;
    m_NumberOfNode  = 0;
    m_NodeDataSize  = NodeDataSize + sizeof(ChainList_t);
}

//-------------------------------------------------------------------------------------------------
//
//   Destructor:    ~ChainList
//
//   Description:   Free up any resources
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
ChainList::~ChainList()
{
    // Free memory for all node
    do
    {
        RemoveNode(m_pFirstNode->ChainID);
    }
    while(m_pFirstNode != nullptr);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNodeDataAddress
//
//   Parameter(s):  ChainList_t* pNode      Address of the node
//   Return:        void*                   Address of data for the node
//
//   Description:   Private function to return the data address pointer
//
//-------------------------------------------------------------------------------------------------
void* ChainList::GetNodeDataAddress(ChainList_t* pNode)
{
    uint32_t AddressData;
    void*    pData;

    AddressData  = sizeof(ChainList_t);
    AddressData += uint32_t(reinterpret_cast<uint8_t*>(pNode));
    pData        = reinterpret_cast<void*>(AddressData);

    return pData;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Alloc
//
//   Parameter(s):  size_t      Size            Total size to create all necessary node
//   Return:        SystemState_e               SYS_READY if all node are added
//                                              SYS_POOL_NOT_ALLOCATED_ERROR If no memory allocated
//
//   Description:   Will create as many node as necessary to hold the provided size
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::Alloc(size_t Size)
{
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: AddNode
//
//   Parameter(s):  uint16_t    ChainID         Client ID for this node
//                  void**      pData           Pointer on data for the node
//   Return:        SystemState_e               SYS_READY if node is added
//                                              SYS_ID_ALREADY_EXIST if node already exist
//                                              SYS_POOL_NOT_ALLOCATED_ERROR If no memory allocated
//
//   Description:   This reserve the memory for the node and insert it into the chain
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::AddNode(uint16_t ChainID, void** pData)
{
    SystemState_e State = SYS_READY;
    ChainList_t* pNode;

    if(GetNodePointer(ChainID, &pNode) == SYS_READY)      // First check if this ID already exist in chain
    {
        State = SYS_ID_ALREADY_EXIST;
    }
    else
    {
        // Important to clear the allocation, to get 'nullptr' everywhere
        pNode = static_cast<ChainList_t*>(pMemoryPool->AllocAndClear(m_NodeDataSize));

        if(pNode != nullptr)
        {
            pNode->ChainID = ChainID;

            if(m_pFirstNode == nullptr)
            {
                m_pFirstNode = pNode;
            }

            if(m_pLastNode != nullptr)
            {
                m_pLastNode->pNextNode = pNode;
                pNode->pPreviousNode = m_pLastNode;
            }

            m_pLastNode  = pNode;
            *pData       = GetNodeDataAddress(pNode);
            m_NumberOfNode++;
        }
        else
        {
            State = SYS_POOL_NOT_ALLOCATED_ERROR;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: RemoveNode
//
//   Parameter(s):  uint16_t    ChainID         Client ID for this node
//
//   Return:        SystemState_e               If node exist than it returned  state = SYS_READY
//                                              If not state = SYS_ID_DOES_NOT_EXIST
//                                              It might return another kind of error.
//                                                  See: lib_memory
//
//   Description:   Remove the node entry in the chainlist using the client ID
//
//
//   Note(s):       This will also free the memory of the node.
//                  The Node is still remove even if the free block doesn't work.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::RemoveNode(uint16_t ChainID)
{
    SystemState_e State;
    ChainList_t*  pNode;

    if((State = GetNodePointer(ChainID, &pNode)) == SYS_READY)
    {
        // Put the next node pointer into the previous node
        if(pNode->pPreviousNode != nullptr)
        {
            (pNode->pPreviousNode)->pNextNode = pNode->pNextNode;
        }

        // Put the previous node pointer into the next node
        if(pNode->pNextNode != nullptr)
        {
            (pNode->pNextNode)->pPreviousNode = pNode->pPreviousNode;
        }

        // If this node was the first, than update the first node pointer with next node entry
        if(m_pFirstNode == pNode)
        {
            m_pFirstNode = pNode->pNextNode;
        }

        // If this node was the last, than update the last node pointer with previous node entry
        if(m_pLastNode == pNode)
        {
            m_pLastNode = pNode->pPreviousNode;
        }

        if(pMemoryPool->Free((void**)&pNode) != true)
        {
            State = SYS_FAIL_MEMORY_DEALLOCATION;
        }


        m_NumberOfNode--;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNodePointer
//
//   Parameter(s):  uint16_t    ChainID         Client ID for this node
//                  void**      pNode
//
//   Return:        SystemState_e               If node exist than it returned a pointer and
//                                              State = SYS_READY
//                                              If not pointer is null and
//                                              State = SYS_ID_DOES_NOT_EXIST
//
//   Description:   Get the node pointer from the client ID
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::GetNodePointer(uint16_t ChainID, ChainList_t** pNode)
{
    SystemState_e State = SYS_ID_DOES_NOT_EXIST;
    ChainList_t*  pScanNode;

    *pNode = nullptr;

    if(m_NumberOfNode != 0)
    {
        pScanNode = m_pFirstNode;

        do
        {
            if(pScanNode->ChainID == ChainID)
            {
                *pNode    = pScanNode;
                pScanNode = nullptr;
                State     = SYS_READY;
            }
            else
            {
                pScanNode = pScanNode->pNextNode;
            }
        }
        while(pScanNode != nullptr);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNodeDataPointer
//
//   Parameter(s):  uint16_t        ChainID     Client ID for this node
//                  void**          pData       Pointer on data for the node
//
//   Return:        SystemState_e   If node exist than it returned a pointer and state = SYS_READY
//                                  If not pointer is null and state = SYS_ID_DOES_NOT_EXIST
//
//   Description:   Get the node data pointer from the client ID
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::GetNodeDataPointer(uint16_t ChainID, void** pData)
{
    SystemState_e State;
    ChainList_t*  pNode;

    State  = GetNodePointer(ChainID, &pNode);
    *pData = static_cast<void*>(reinterpret_cast<uint8_t*>(pNode) + sizeof(ChainList_t));

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNumberOfNode
//
//   Parameter(s):  None
//
//   Return:        uint16_t
//
//   Description:   This return the number of node
//
//-------------------------------------------------------------------------------------------------
uint16_t ChainList::GetNumberOfNode(void)
{
    return m_NumberOfNode;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: ResetScanNode
//
//   Parameter(s):  None
//
//   Return:        SystemState_e   If node exist than it returned a pointer and state = SYS_READY
//                                  If list is empty state = SYS_CHAIN_LIST_IS_EMPTY
//
//   Description:   Reset the m_pScanNode pointer to beginning
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::ResetScanNode(void)
{
    SystemState_e State = SYS_READY;

    m_pScanNode = m_pFirstNode;

    if(m_pScanNode == nullptr)
    {
        State = SYS_CHAIN_LIST_IS_EMPTY;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNextNode
//
//   Parameter(s):  uint16_t*   pChain          The returned pointer is for this returned chain ID
//                  void**      pData           Pointer on data for the node
//
//   Return:        SystemState_e   If node exist than it returned a pointer and State = SYS_READY
//                                  If Chain list is empty State = SYS_CHAIN_LIST_IS_EMPTY
//                                  If scan reach end of list, State = SYS_REACH_END_OF_LIST
//
//   Description:   Get the Next node pointer and chainID
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::GetNextNode(uint16_t* pChainID, void** pData)
{
    SystemState_e State = SYS_READY;

    if(m_pScanNode != nullptr)
    {
        *pData      = GetNodeDataAddress(m_pScanNode);      // Get data pointer
        *pChainID   = m_pScanNode->ChainID;                 // Get ChainID
        m_pScanNode = m_pScanNode->pNextNode;               // Get Pointer of next node
    }
    else
    {
        *pData   = nullptr;
        *pChainID = 0;

        if(m_pFirstNode == nullptr)
        {
            State =  SYS_CHAIN_LIST_IS_EMPTY;
        }
        else
        {
            State = SYS_REACH_END_OF_LIST;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------

#endif // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
