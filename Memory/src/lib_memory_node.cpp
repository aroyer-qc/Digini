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

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   MemoryNode
//
//   Parameter(s):  None
//
//   Description:   Initializes internal memory node.
//
//-------------------------------------------------------------------------------------------------
MemoryNode::MemoryNode()
{
    m_pNodeList = (NodeList*)pMemoryPool->AllocAndClear(sizeof(NodeList));
}

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   MemoryNode
//
//   Parameter(s):  size_t      NodeDataSize
//
//   Description:   Initializes internal memory node.
//
//-------------------------------------------------------------------------------------------------
MemoryNode::MemoryNode(size_t NodeDataSize)
{
    MemoryNode();
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
    SystemState_e State;
    uint16_t      NeededNumberOfNode;
    size_t        NodeSize;
    void*         pDummy;

    State              = SYS_READY;
    NodeSize           = m_pNodeList->GetNodeSize();
    NeededNumberOfNode = Size / NodeSize;
    m_TotalSize        = Size;

    if((Size % NodeSize) != 0)
    {
        NeededNumberOfNode++;
    }


    if(Size > (NodeSize * NeededNumberOfNode))              // Need to allocate more memory node.
    {
        for(uint16_t i = m_pNodeList->GetNumberOfNode(); (i < NeededNumberOfNode) && (State == SYS_READY); i++)
        {
            m_pNodeList->AddNode(i, &pDummy);
        }
    }
    else if(Size < (NodeSize * NeededNumberOfNode))         // Need to free up some memory node.
    {
//        for(int16_t i = (Size - NeededNumberOfNode); i


        //for(int i = NeededNumberOfNodem_NumberOfNode; (i < NeededNumberOfNode) && (State == SYS_READY); i++)
        //{
            //m_pNodeList->AddNode(i, &pDummy);
        //}
    }
    else
    {
        // It has, already, the memory node reserved for the need of the request
    }

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
//                  unneeded node.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MemoryNode::Free(void)
{
    return m_pNodeList->RemoveAllNode();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SetNodeSize
//
//   Parameter(s):  size_t          NodeDataSize        Total size of data node needed
//   Return:        void
//
//   Description:   Set the node size required for this memopy pool of node.
//
//-------------------------------------------------------------------------------------------------
void MemoryNode::SetNodeSize(size_t NodeDataSize)
{
    m_pNodeList->SetNodeSize(NodeDataSize);
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

#endif // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
