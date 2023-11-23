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
//   Function name:
//
//   Parameter(s):  uint16_t    ChainID             Client ID for this none
//                  void**      pData               Pointer on data for the node
//   Return:        SystemState_e
//
//   Description:
//
//
//   Note(s):       This reserve the memory for the node and insert it into the chain
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::AddNode(uint16_t ChainID, void** pData)
{
    SystemState_e State = SYS_READY;
    ChainList_t* pNode;
    uint32_t     AddressData;

    if(GetNodePointer(ChainID, &pNode) == SYS_READY)      // First check if this ID already exist in chain
    {
        State = SYS_ID_ALREADY_EXIST;
    }
    else
    {
        pNode = static_cast<ChainList_t*>(pMemoryPool->AllocAndClear(m_NodeDataSize));
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
        AddressData  = sizeof(ChainList_t);
        AddressData += uint32_t(reinterpret_cast<uint8_t*>(pNode));
        *pData       = reinterpret_cast<void*>(AddressData);
        m_NumberOfNode++;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: RemoveNode
//
//   Parameter(s):  uint16_t    ChainID            Client ID for this node
//
//   Return:        Error       If node exist than it returned  state = SYS_OK
//                              if not state = SYS_ID_DOES_NOT_EXIST
//                              It might return another kind of error. See: lib_memory
//
//   Description:   Remove and entry in the chainlist using the client ID
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
        // Must get the node pointer from next node     // todo mixup definition
        // and put this pointer into the previous node
        if(pNode->pPreviousNode != nullptr)
        {
            (pNode->pPreviousNode)->pNextNode = pNode->pNextNode;
        }

        // Must get the node pointer from Previous node
        // and put this pointer into the previous node
        if(pNode->pNextNode != nullptr)
        {
            (pNode->pNextNode)->pPreviousNode = pNode->pPreviousNode;
        }

        // If this node was the first, than update the first node pointer with next entry
        if(m_pFirstNode == pNode)
        {
            m_pFirstNode = pNode->pNextNode;
        }

        // If this node was the last, than update the last node pointer with previous entry
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
//   Parameter(s):  uint16_t    ChainID            Client ID for this none
//                  void**      pNode
//
//   Return:        Error       If node exist than it returned a pointer and state = SYS_OK
//                              if not pointer is null and state = SYS_ID_DOES_NOT_EXIST
//
//   Description:   Get the node pointer from the client ID
//
//-------------------------------------------------------------------------------------------------
SystemState_e ChainList::GetNodePointer(uint16_t ChainID, ChainList_t** pNode)
{
    SystemState_e State = SYS_ID_DOES_NOT_EXIST;
    ChainList_t*  pScanNode;

    *pNode    = nullptr;

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
//   Parameter(s):  uint16_t    ChainID            Client ID for this none
//                  void**      pData              Pointer on data for the node
//
//   Return:        Error       If node exist than it returned a pointer and state = SYS_OK
//                              if not pointer is null and state = SYS_ID_DOES_NOT_EXIST
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

#endif // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
