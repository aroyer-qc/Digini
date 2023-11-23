//-------------------------------------------------------------------------------------------------
//
//  File : lib_chain_list.h
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
//      Special class to handle chain list.
//
//          This class will reserved the memory for the node to be used by the client and also
//          handle the memory release.
//          This class used the static memory block allocation 'lib_memory.h'
//          Node chain list information is integrated into the allocated memory block, but the
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

struct ChainList_t
{
    uint16_t        ChainID;                    // Use to identified the node by external client
    void*           pClientData;                // Can be function, Data Struct, Class object etc...
    ChainList_t*    pPreviousNode;
    ChainList_t*    pNextNode;
};

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class ChainList
{
    public:

                        ChainList           (size_t NodeDataSize);
                       ~ChainList           ();

        SystemState_e   AddNode             (uint16_t ChainID, void** pData);           // This reserve the memory for the node and insert it into the chain
        SystemState_e   RemoveNode          (uint16_t ChainID);                         // This will also free the memory of the node
        SystemState_e   GetNodeDataPointer  (uint16_t ChainID, void** pData);           // Get the node data from the client ID
        uint16_t        GetNumberOfNode     (void);

    private:

        SystemState_e   GetNodePointer      (uint16_t ChainID, ChainList_t** pData);    // Get the node pointer from the client ID


        ChainList_t*    m_pFirstNode;
        ChainList_t*    m_pLastNode;
        uint8_t         m_NumberOfNode;
        size_t          m_NodeDataSize;
};

//-------------------------------------------------------------------------------------------------

#endif  // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
