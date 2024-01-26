//-------------------------------------------------------------------------------------------------
//
//  File : lib_node_list.h
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
//------ Note(s) ----------------------------------------------------------------------------------
//
//      Special class to handle node list.
//
//          This class will reserved the memory for the node to be used by the client and also
//          handle the memory release.
//          This class used the static memory block allocation 'lib_memory.h'
//          Node list information is integrated into the allocated memory block, but the
//          pointer it return is offseted for client data only.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_typedef.h"

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct NodeList_t
{
    uint16_t        NodeID;                 // Use to identified the node by external client
    void*           pClientData;            // Can be function, Data Struct, Class object etc...
    NodeList_t*     pPreviousNode;
    NodeList_t*     pNextNode;
};

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class NodeList
{
    public:

                        NodeList            (size_t NodeDataSize);                      // All node data size will be created with this size + node overhead

        SystemState_e   Alloc               (size_t Size);                              // Will create as many node as necessary to hold the provided size

        SystemState_e   AddNode             (uint16_t NodeID, void** pData);            // This reserve the memory for the node and insert it into the chain
        SystemState_e   RemoveNode          (uint16_t NodeID);                          // This will also free the memory of the node
        SystemState_e   RemoveAllNode       (void);
        SystemState_e   GetNodeDataPointer  (uint16_t NodeID, void** pData);            // Get the node data from the client ID
        uint16_t        GetNumberOfNode     (void);

        void            SetNodeSize         (size_t NodeDataSize);
        size_t          GetNodeSize         (void);                                     // Get Node data pointer and increment node pointer

        // Special method to scan all node
        SystemState_e   ResetScanNode       (void);                                     // Reset the ScanNode pointer to beginning
        SystemState_e   GetNextNode         (uint16_t* NodeID, void** pData);           // Get the Next node pointer and chainID

    private:

        SystemState_e   GetNodePointer      (uint16_t NodeID, NodeList_t** pNode);     // Get the node pointer from the client ID
        void*           GetNodeDataAddress  (NodeList_t* pNode);

        NodeList_t*     m_pFirstNode;
        NodeList_t*     m_pLastNode;
        NodeList_t*     m_pScanNode;
        uint16_t        m_NumberOfNode;
        size_t          m_NodeSize;
};

// upgrade?
// Add sorting of the NodeList using ID, need to create a class using bubble sort
// Add search with successive approximation using ID;

//-------------------------------------------------------------------------------------------------

#endif  // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
