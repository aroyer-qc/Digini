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
//   Constructor:   NodeList
//
//   Parameter(s):  size_t      NodeDataSize
//
//   Description:   Initializes internal chain list pointer and size of data struct
//
//-------------------------------------------------------------------------------------------------
NodeList::NodeList(size_t NodeDataSize)
{
    m_pFirstNode    = nullptr;
    m_pLastNode     = nullptr;
    m_pScanNode     = nullptr;
    m_NumberOfNode  = 0;
    SetNodeSize(NodeDataSize);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNodeDataAddress
//
//   Parameter(s):  NodeList_t* pNode      Address of the node
//   Return:        void*                   Address of data for the node
//
//   Description:   Private function to return the data address pointer
//
//-------------------------------------------------------------------------------------------------
void* NodeList::GetNodeDataAddress(NodeList_t* pNode)
{
    uint32_t AddressData;
    void*    pData;

    AddressData  = sizeof(NodeList_t);
    AddressData += uint32_t(reinterpret_cast<uint8_t*>(pNode));
    pData        = reinterpret_cast<void*>(AddressData);

    return pData;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: AddNode
//
//   Parameter(s):  uint16_t    NodeID         Client ID for this node
//                  void**      pData           Pointer on data for the node
//   Return:        SystemState_e               SYS_READY if node is added
//                                              SYS_ID_ALREADY_EXIST if node already exist
//                                              SYS_POOL_NOT_ALLOCATED_ERROR If no memory allocated
//
//   Description:   This reserve the memory for the node and insert it into the chain
//
//-------------------------------------------------------------------------------------------------
SystemState_e NodeList::AddNode(uint16_t NodeID, void** pData)
{
    SystemState_e State = SYS_READY;
    NodeList_t* pNode;

    if(GetNodePointer(NodeID, &pNode) == SYS_READY)      // First check if this ID already exist in chain
    {
        State = SYS_ID_ALREADY_EXIST;
    }
    else
    {
        // Important to clear the allocation, to get 'nullptr' everywhere
        pNode = static_cast<NodeList_t*>(pMemoryPool->AllocAndClear(m_NodeSize));

        if(pNode != nullptr)
        {
            pNode->NodeID = NodeID;

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
//   Parameter(s):  uint16_t    NodeID         Client ID for this node
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
SystemState_e NodeList::RemoveNode(uint16_t NodeID)
{
    SystemState_e State;
    NodeList_t*  pNode;

    if((State = GetNodePointer(NodeID, &pNode)) == SYS_READY)
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
//   Function name: RemoveAllNode
//
//   Parameter(s):  None
//
//   Return:        SystemState_e               Success it return SYS_READY, otherwise may return
//                                              SYS_ID_DOES_NOT_EXIST, SYS_FAIL_MEMORY_DEALLOCATION
//
//   Description:   Free memory for all node and remove them
//
//-------------------------------------------------------------------------------------------------
SystemState_e NodeList::RemoveAllNode(void)
{
    SystemState_e State = SYS_READY;

    do
    {
        if(RemoveNode(m_pFirstNode->NodeID) != SYS_READY)
        {
                State = SYS_NODE_UNKNOWN_ERROR;
        }
    }
    while(m_pFirstNode != nullptr);

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNodePointer
//
//   Parameter(s):  uint16_t      NodeID       Client ID for this node
//                  NodeList_t** pNode
//
//   Return:        SystemState_e               If node exist than it returned a pointer and
//                                              State = SYS_READY
//                                              If not pointer is null and
//                                              State = SYS_ID_DOES_NOT_EXIST
//
//   Description:   Get the node pointer from the client ID
//
//-------------------------------------------------------------------------------------------------
SystemState_e NodeList::GetNodePointer(uint16_t NodeID, NodeList_t** pNode)
{
    SystemState_e State = SYS_ID_DOES_NOT_EXIST;
    NodeList_t*  pScanNode;

    *pNode = nullptr;

    if(m_NumberOfNode != 0)
    {
        pScanNode = m_pFirstNode;

        do
        {
            if(pScanNode->NodeID == NodeID)
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
//   Parameter(s):  uint16_t        NodeID     Client ID for this node
//                  void**          pData      Pointer on data for the node or nullptr
//
//   Return:        SystemState_e   If node exist than it returned a pointer and state = SYS_READY
//                                  If not pointer is null and state = SYS_ID_DOES_NOT_EXIST
//
//   Description:   Get the node data pointer from the client ID
//
//-------------------------------------------------------------------------------------------------
SystemState_e NodeList::GetNodeDataPointer(uint16_t NodeID, void** pData)
{
    SystemState_e State;
    NodeList_t*  pNode;

    State  = GetNodePointer(NodeID, &pNode);

    if(State == SYS_READY)
    {
        *pData = static_cast<void*>(reinterpret_cast<uint8_t*>(pNode) + sizeof(NodeList_t));
    }
    else
    {
        *pData = nullptr;
    }

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
uint16_t NodeList::GetNumberOfNode(void)
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
//                                  If list is empty state = SYS_NODE_LIST_IS_EMPTY
//
//   Description:   Reset the m_pScanNode pointer to beginning.
//
//-------------------------------------------------------------------------------------------------
SystemState_e NodeList::ResetScanNode(void)
{
    SystemState_e State = SYS_READY;

    m_pScanNode = m_pFirstNode;

    if(m_pScanNode == nullptr)
    {
        State = SYS_NODE_LIST_IS_EMPTY;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNextNode
//
//   Parameter(s):  uint16_t*   pChain          The returned pointer is for this returned node ID
//                  void**      pData           Pointer on data for the node
//
//   Return:        SystemState_e   If node exist than it returned a pointer and State = SYS_READY
//                                  If node list is empty State = SYS_NODE_LIST_IS_EMPTY
//                                  If scan reach end of list, State = SYS_REACH_END_OF_LIST
//
//   Description:   Get the next node pointer and NodeID.
//
//-------------------------------------------------------------------------------------------------
SystemState_e NodeList::GetNextNode(uint16_t* pChainID, void** pData)
{
    SystemState_e State = SYS_READY;

    if(m_pScanNode != nullptr)
    {
        *pData      = GetNodeDataAddress(m_pScanNode);      // Get data pointer
        *pChainID   = m_pScanNode->NodeID;                  // Get NodeID
        m_pScanNode = m_pScanNode->pNextNode;               // Get Pointer of next node
    }
    else
    {
        *pData   = nullptr;
        *pChainID = 0;

        if(m_pFirstNode == nullptr)
        {
            State =  SYS_NODE_LIST_IS_EMPTY;
        }
        else
        {
            State = SYS_REACH_END_OF_LIST;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SetNodeSize
//
//   Parameter(s):  size_t          NodeDataSize        Total size of data node needed
//   Return:        void
//
//   Description:   Set the node size required.
//
//-------------------------------------------------------------------------------------------------
void NodeList::SetNodeSize(size_t NodeDataSize)
{
    m_NodeSize = NodeDataSize + sizeof(NodeList_t);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNodeSize
//
//   Parameter(s):  None
//   Return:        size_t    NodeDataSize        Total size of data node needed
//
//   Description:   Get the node size.
//
//-------------------------------------------------------------------------------------------------
size_t NodeList::GetNodeSize(void)
{
    return m_NodeSize - sizeof(NodeList_t);
}


//-------------------------------------------------------------------------------------------------

#endif // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------