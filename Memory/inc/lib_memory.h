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
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "lib_define.h"

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_MEM_BLOCK_AS_ENUM(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)        ENUM_ID,
#define EXPAND_X_MEM_BLOCK_AS_ARRAY_DECL(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)  uint8_t m_##GROUP_NAME[BLOCK_MAX][BLOCK_SIZE] __attribute__ ((aligned (4)));
#define EXPAND_X_MEM_BLOCK_AS_TOTAL(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)       (BLOCK_MAX * BLOCK_SIZE) +

//-------------------------------------------------------------------------------------------------
// MEM_BLOCK list declaration section
//-------------------------------------------------------------------------------------------------

// To found how many block type there is
enum MEM_BlockList_e
{
    MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_ENUM)
    MEM_BLOCK_GROUP_SIZE
};

//-------------------------------------------------------------------------------------------------

class MemPoolDriver
{
    public:

                    MemPoolDriver               ();
                   ~MemPoolDriver               ();

        void*       Alloc                       (size_t SizeRequired, TickCount_t TimeOut = NOS_WAIT_INFINITE);
        void*       AllocAndClear               (size_t SizeRequired, TickCount_t TimeOut = NOS_WAIT_INFINITE);
        void*       AllocAndSet                 (size_t SizeRequired, uint8_t FillValue, TickCount_t TimeOut = NOS_WAIT_INFINITE);
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

    private:

        nOS_Mem                     m_nOS_MemArray      [MEM_BLOCK_GROUP_SIZE];                  // handler to give to nOS_Mem... function
        void*                       m_pBufferArray      [MEM_BLOCK_GROUP_SIZE];                  // pointer array of the memory block
        nOS_Error                   m_LastError;
      #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
        nOS_MemCounter              m_BlockUsed         [MEM_BLOCK_GROUP_SIZE];
        nOS_MemCounter              m_BlockHighest      [MEM_BLOCK_GROUP_SIZE];
        uint32_t                    m_UsedMemory;
      #endif

        MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_ARRAY_DECL)
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
