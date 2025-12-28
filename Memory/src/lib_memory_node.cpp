//-------------------------------------------------------------------------------------------------
//
//  File : lib_memory_node.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   Create
//
//   Parameter(s):  size_t      NodeDataSize
//
//   Description:   Initializes internal memory node.
//
//-------------------------------------------------------------------------------------------------
void MemoryNode::Create(size_t NodeDataSize)
{
    m_pNodeList = (NodeList*)pMemoryPool->AllocAndClear(sizeof(NodeList), MEM_NODE_1);

    if (m_pNodeList != nullptr)
    {
        m_pNodeList->Initialize(NodeDataSize);
    }

    SetNodeSize(NodeDataSize);
    m_TotalSize = 0;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Alloc
//
//   Parameter(s):  size_t          Size        Total size of data node needed
//   Return:        SystemState_e               SYS_READY if all node are added
//                                              SYS_POOL_NOT_ALLOCATED_ERROR If no memory allocated
//
//   Description:   Create as many node as necessary to hold the provided size or remove
//                  unneeded node.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MemoryNode::Alloc(size_t Size)
{
    void*           pDummy;
    size_t          NodeSize;
    uint16_t        Current;
    uint16_t        Needed;
    SystemState_e   State;

    State       = SYS_READY;
    NodeSize    = m_pNodeList->GetNodeSize();
    Current     = m_pNodeList->GetNumberOfNode();
    Needed      = (Size + NodeSize - 1) / NodeSize;
    m_TotalSize = Size;

    // Allocate missing nodes
    for(uint16_t i = Current; i < Needed; i++)
    {
        State = m_pNodeList->AddNode(i, &pDummy);

        if(State != SYS_READY)
        {
            return State;
        }
   }

    // Free extra nodes
    for(uint16_t i = Current; i > Needed; i--)
    {
        State = m_pNodeList->RemoveNode(i - 1);

        if(State != SYS_READY)
        {
            return State;
        }
    }

    // Reset iterator
    m_NodePtr = 0;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Free
//
//   Parameter(s):  None
//   Return:        SystemState_e               SYS_READY if all node are added
//                                              SYS_POOL_NOT_ALLOCATED_ERROR If no memory allocated
//
//   Description:   Free all node memory and the node themselve.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MemoryNode::Free(void)
{
//    return m_pNodeList->RemoveAllNode();
    SystemState_e State = SYS_READY;

    if(m_pNodeList != nullptr)
    {
        State = m_pNodeList->RemoveAllNode();

        // Free the NodeList object itself
        if(pMemoryPool->Free((void**)&m_pNodeList) != true)
        {
            State = SYS_FAIL_MEMORY_DEALLOCATION;
        }

        m_pNodeList = nullptr;
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
//   Description:   Set the node size required for this memory pool of node.
//
//-------------------------------------------------------------------------------------------------
void MemoryNode::SetNodeSize(size_t NodeDataSize)
{
    m_pNodeList->SetNodeSize(NodeDataSize);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNodeSize
//
//   Parameter(s):  None
//   Return:        size_t
//
//   Description:   Get the node size from this memory pool of node.
//
//-------------------------------------------------------------------------------------------------
size_t MemoryNode::GetNodeSize(void)
{
    return m_pNodeList->GetNodeSize();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNext
//
//   Parameter(s):  None
//   Return:        void* pData
//
//   Description:   Get the data pointer for the node and increment the node pointer to next
//                  available.
//
//-------------------------------------------------------------------------------------------------
void* MemoryNode::GetNext(void)
{
    void* pData = nullptr;

    if(m_pNodeList->GetNodeDataPointer(m_NodePtr, &pData) == SYS_READY)
    {
        m_NodePtr++;
    }

    return pData;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: AllocNode
//
//   Parameter(s):  size_t      Size                    Total size for this node
//                  size_t      NodeDataSize            Set granularity of the node
//   Return:        MemoryNode* Node pointer            Pointer on the created Memory Node object
//
//   Description:   Allocate memory the the MemoryNode object
//
//-------------------------------------------------------------------------------------------------
MemoryNode* MemoryNode::AllocNode(size_t Size, size_t NodeDataSize)
{
    MemoryNode* pMemoryNode;

    pMemoryNode = (MemoryNode*)pMemoryPool->AllocAndClear(sizeof(MemoryNode), MEM_NODE_2);

    if(pMemoryNode != nullptr)
    {
        pMemoryNode->Create(NodeDataSize);
        pMemoryNode->Alloc(Size);
    }

    return pMemoryNode;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: FreeNode
//
//   Parameter(s):  MemoryNode*     Node pointer       Pointer on the Memory Node to be freed
//   Return:        None
//
//   Description:   Free the memory allocated by the memory node and the MemoryNode itself
//
//-------------------------------------------------------------------------------------------------

void  MemoryNode::FreeNode(MemoryNode* pMemoryNode)
{
    if(pMemoryNode != nullptr)
    {
        pMemoryNode->Free();                     // Free all nodes + NodeList
        pMemoryPool->Free((void**)&pMemoryNode); // Free this MemoryNode
        // local pointer copy is now invalid, but caller's pointer is unchanged
    }

//    if(pMemoryNode != nullptr)
//    {
//        pMemoryNode->Free();                        // Free all node inside the object
//        pMemoryPool->Free((void**)&pMemoryNode);    // Free this MemoryNode
//        pMemoryNode = nullptr;
//    }
}

//-------------------------------------------------------------------------------------------------

#endif // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
