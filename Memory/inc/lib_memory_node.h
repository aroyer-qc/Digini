//-------------------------------------------------------------------------------------------------
//
//  File : lib_memory_node.h
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
//------ Note(s) ----------------------------------------------------------------------------------
//
//      Special class to handle memory node.
//
//          It does the same thing as 'lib_memory.h' but can realloc more data on request. and work
//          on chunk of allocated data not linearly distributed in memory called node.
//          Used the static memory block allocation 'lib_memory.h'
//          Also used of the node list library 'lib_node_list.h'
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_STATIC_MEMORY_ALLOC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class MemoryNode
{
    public:


        void                Create              (size_t NodeDataSize);
        SystemState_e       Alloc               (size_t Size);                              // This reserve the memory for entire buffer  and clear the data.
        SystemState_e       Free                (void);                                     // Will free all node data allocation and node allocation

        void                SetNodeSize         (size_t NodeDataSize);
        size_t              GetNodeSize         (void);                                     // Get Node data pointer and increment node pointer

        size_t              GetTotalSize        (void) { return m_TotalSize; }              // Get memory size requested in Alloc

        void                Begin               (void) { m_NodePtr = 0; }                   // Set Node pointer to first node
        void*               GetNext             (void);                                     // Get Node data pointer and increment node pointer

        static MemoryNode*  AllocNode           (size_t Size, size_t NodeDataSize);         // Create and return a MemoryNode
        static void         FreeNode            (MemoryNode* pMemoryNode);                  // Free the memory allocated by the memory node and the MemoryNode itself

    private:

        NodeList*       m_pNodeList;
        uint16_t        m_NodePtr;
        size_t          m_TotalSize;
};

//-------------------------------------------------------------------------------------------------

#endif  // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_STATIC_MEMORY_ALLOC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

