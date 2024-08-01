//-------------------------------------------------------------------------------------------------
//
//  File : lib_memory.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_STATIC_MEMORY_ALLOC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

enum MEM_DebugListOfID_e
{
    MEM_DBG_NONE,
    MEM_DBG_NO_INIT,                    // Used as default if user did not provide an ID

    // Unique ID for lib_string
    MEM_DBG_LIB_STRING_1,
    MEM_DBG_LIB_STRING_2,

    // Unique ID for lib_fifo
    MEM_DBG_LIB_FIFO_1,

    // Unique ID for lib_memory_node and lib_memory_node_list
    MEM_NODE_1,
    MEM_NODE_2,
    MEM_NODE_LIST_1,

    // Unique ID for Comm CLI
    MEM_DBG_CLI_1,

    // Unique ID for Comm Console
    MEM_DBG_CONSOLE_1,
    MEM_DBG_CONSOLE_2,
    MEM_DBG_CONSOLE_3,

    // Unique ID for Comm VT100
    MEM_DBG_VT100_1,
    MEM_DBG_VT100_2,
    MEM_DBG_VT100_3,

    // Unique ID for lib_vt100_callback.cpp
    MEM_DBG_VT100_CB_SYSSET_1,
    MEM_DBG_VT100_CB_SYSSET_2,
    MEM_DBG_VT100_CB_SYSSET_3,
    MEM_DBG_VT100_CB_SYSSET_4,
    MEM_DBG_VT100_CB_SYSSET_5,
    MEM_DBG_VT100_CB_SYSSET_6,

    // Unique ID for NanoIP
    MEM_DBG_TASK_NETWORK_1,
    MEM_DBG_TASK_NETWORK_2,
    MEM_DBG_CLASS_ETHERNETIF_1,
    MEM_DBG_CLASS_IP_MANAGER_1,

    // UniqueID for included in Digini external module
    MEM_DBG_SPI_VFD_1,

    // UniqueID for lib_service
    MEM_DBG_SERVICE,

    // Application debug ID from user
    MEMORY_USER_DEBUG_ID
};

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_MEM_BLOCK_AS_ENUM(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)        ENUM_ID,
#define EXPAND_X_MEM_BLOCK_AS_ARRAY_DECL(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)  uint8_t m_##GROUP_NAME[BLOCK_MAX][BLOCK_SIZE] __attribute__ ((aligned (4)));
#define EXPAND_X_MEM_BLOCK_AS_TOTAL(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)       (BLOCK_MAX * BLOCK_SIZE) +

// For debug block trace
#define EXPAND_X_MEM_BLOCK_AS_ARRAY_DEBUG_BLOCK_TRACE(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)  MEM_DebugListOfID_e m_DebugTrace ## GROUP_NAME[BLOCK_MAX];

//-------------------------------------------------------------------------------------------------
// MEM_BLOCK list declaration section
//-------------------------------------------------------------------------------------------------

// To found how many block type there is
enum MEM_BlockList_e
{
    MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_ENUM)
    MEM_BLOCK_GROUP_QTS
};

//-------------------------------------------------------------------------------------------------

class MemPoolDriver
{
    public:

                    MemPoolDriver               ();
                   ~MemPoolDriver               ();

        void*       Alloc                       (size_t SizeRequired, MEM_DebugListOfID_e = MEM_DBG_NO_INIT);
        void*       AllocAndClear               (size_t SizeRequired, MEM_DebugListOfID_e = MEM_DBG_NO_INIT);
        void*       AllocAndSet                 (size_t SizeRequired, uint8_t FillValue, MEM_DebugListOfID_e = MEM_DBG_NO_INIT);

        void        OverrideNextTimeOut         (TickCount_t TimeOut);                          // This will override the default time out for next allocation. Code reduction as most of the time the default is used. and reduce number of overload.

        bool        Free                        (void** pBlock);
        bool        IsAvailable                 (size_t SizeRequired);
        nOS_Error   GetLastError                (void);
      #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
        uint32_t    GetTotalSizeReserved        (void);
        uint32_t    GetUsedMemory               (void);
        uint32_t    GetNumberOfPool             (void);
        uint32_t    GetPoolNumberOfBlock        (uint32_t PoolNumber);
        uint32_t    GetPoolBlockSize            (uint32_t PoolNumber);
        uint32_t    GetPoolBlockUsed            (uint32_t PoolNumber);
        uint32_t    GetPoolBlockHighPoint       (uint32_t PoolNumber);
      #endif

      #if (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
        // TODO add function to get info on the block trace data
      #endif

    private:

        TickCount_t                 m_TimeOut;
        nOS_Mem                     m_nOS_MemArray      [MEM_BLOCK_GROUP_QTS];                  // handler to give to nOS_Mem... function
        void*                       m_pBufferArray      [MEM_BLOCK_GROUP_QTS];                  // pointer array of the memory block
        nOS_Error                   m_LastError;
      #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
        nOS_MemCounter              m_BlockUsed         [MEM_BLOCK_GROUP_QTS];
        nOS_MemCounter              m_BlockHighest      [MEM_BLOCK_GROUP_QTS];
        uint32_t                    m_UsedMemory;
      #endif

        MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_ARRAY_DECL)

      #if (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
        MEM_DebugListOfID_e*        m_pDebugInfoArray   [MEM_BLOCK_GROUP_QTS];                  // pointer array of the memory block
        MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_ARRAY_DEBUG_BLOCK_TRACE)
      #endif
};


// ----- Memory allocation(s) ------
#ifdef MEM_GLOBAL
class MemPoolDriver                       _MemoryPool;
class MemPoolDriver*                      pMemoryPool     = &_MemoryPool;
#else
extern class MemPoolDriver*               pMemoryPool;
#endif

//-------------------------------------------------------------------------------------------------

#else

#pragma message("DIGINI use memory pool instead of the malloc library. please define you memblock into memory_cfg.h")

#endif  // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_STATIC_MEMORY_ALLOC == DEF_ENABLED)
