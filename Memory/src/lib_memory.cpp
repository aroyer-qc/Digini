//-------------------------------------------------------------------------------------------------
//
//  File : lib_memory.cpp
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

#define MEM_GLOBAL
#include "./lib_digini.h"
#undef  MEM_GLOBAL

//-------------------------------------------------------------------------------------------------

#ifdef MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_MEM_BLOCK_AS_ARRAY(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)                         \
                                                                                                                    \
                                                            m_pBufferArray[i++] = (void*)&m_##GROUP_NAME[0][0];

#define EXPAND_X_MEM_BLOCK_AS_INITIALIZE_ARRAY(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)                                   \
                                                                                                                                         \
                                                            memset(&m_nOS_MemArray[i], 0, sizeof(nOS_Mem));                              \
                                                            nOS_MemCreate(&m_nOS_MemArray[i], m_pBufferArray[i], BLOCK_SIZE, BLOCK_MAX); \
                                                            i++;

// For debug block trace
#define EXPAND_X_MEM_BLOCK_AS_DEBUG_INFO_ARRAY(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)              \
                                                                                                                    \
                                                            m_pDebugInfoArray[i++] = &m_DebugTrace ## GROUP_NAME[0];

#define EXPAND_X_MEM_BLOCK_AS_INITIALIZE_DEBUG_INFO_ARRAY(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)        \
                                                                                                                         \
                                                            memset(&m_DebugTrace ## GROUP_NAME[i++], 0, sizeof(nOS_Mem));

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   MemPoolDriver
//
//   Parameter(s):  None
//
//   Description:   Initializes the memory allocation table and private support data
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------

MemPoolDriver::MemPoolDriver()
{
    int i;


  #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
    m_UsedMemory = 0;

    for(i = 0; i < MEM_BLOCK_GROUP_QTS; i++)
    {
        m_BlockUsed[i]    = 0;
        m_BlockHighest[i] = 0;
    }
  #endif

    m_TimeOut = NOS_WAIT_INFINITE;


    // Get the address of each group in pointer array
    i = 0;
    MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_ARRAY)

    //Initialize nOS Memory array structure
    i = 0;
    MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_INITIALIZE_ARRAY)

  #if (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)

    // Get the address of each group of debug info stats data
    i = 0;
    MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_DEBUG_INFO_ARRAY)

    //Initialize info array with the MEM_DBG_NONE
    i = 0;
    MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_INITIALIZE_DEBUG_INFO_ARRAY)

  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Destructor:    ~MemPoolDriver
//
//   Description:   Free up any resources if any
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
MemPoolDriver::~MemPoolDriver()
{
  #if (NOS_CONFIG_MEM_DELETE_ENABLE > 0)
    for(i = 0; i < APP_NB_MEM_BLOCK_GROUP; i++)
    {
        nOS_MemDelete(&m_nOS_MemArray[i]);
    }
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Alloc
//
//   Parameter(s):  size_t                  SizeRequired        Size of the needed data block
//                  MEM_DebugListOfID_e     DebugID             Owner of the memory block
//
//   Return:        void*          Pointer to allocated block of memory.
//                                 nullptr = No Block available
//
//   Description:   Try to take one block from any of the memory array when the size is equal or
//                  more then the requested amount of data. If no block available, calling thread
//                  will be removed from list of SYS_READY to run threads and be placed in list of
//                  waiting threads for number of ticks specified by TimeOut. If a block of memory
//                  is freed before end of timeout, thread will be awoken and pointer to memory
//                  block will be returned.
//
//   Note(s):       This class wrap the nOS Function
//
//-------------------------------------------------------------------------------------------------
void* MemPoolDriver::Alloc(size_t SizeRequired, MEM_DebugListOfID_e DebugID)
{
    void*           MemPtr = nullptr;
    size_t          SizeBlock;
  #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
    nOS_StatusReg   sr;
  #endif

  #if (MEMORY_POOL_RESTRICT_ALLOC_TO_BLOCK_SIZE == DEF_DISABLED)
    // First loop will check for any block available, so we don't wait to be freed
    for(uint8_t GroupID = 0; GroupID < MEM_BLOCK_GROUP_QTS; GroupID++)
    {
        SizeBlock = m_nOS_MemArray[GroupID].bsize;

        if(SizeBlock >= SizeRequired)
        {
            MemPtr = nOS_MemAlloc(&m_nOS_MemArray[GroupID], NOS_NO_WAIT);

            if(MemPtr != nullptr)
            {
              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED) | (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                nOS_EnterCritical(sr);
              #endif

              #if (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                m_pDebugInfoArray[GroupID][m_BlockUsed[GroupID]] = DebugID;         // Tag this block with the owner ID
              #endif

              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
                m_UsedMemory += SizeBlock;
                m_BlockUsed[GroupID]++;

                if(m_BlockUsed[GroupID] > m_BlockHighest[GroupID])
                {
                    m_BlockHighest[GroupID] = m_BlockUsed[GroupID];
                }

              #endif
              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED) || (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                nOS_LeaveCritical(sr);
              #endif

                m_TimeOut = NOS_WAIT_INFINITE;          // Reset to default

                return MemPtr;
            }
        }
    }
  #endif

    // If we reach here then we did not succeed to get a block, so we will wait
    for(uint8_t GroupID = 0; GroupID < MEM_BLOCK_GROUP_QTS; GroupID++)
    {
        SizeBlock = m_nOS_MemArray[GroupID].bsize;

        if(SizeBlock >= SizeRequired)
        {
            MemPtr = nOS_MemAlloc(&m_nOS_MemArray[GroupID], m_TimeOut);

            if(MemPtr != nullptr)
            {
              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED) || (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                nOS_EnterCritical(sr);
              #endif

              #if (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                m_pDebugInfoArray[GroupID][m_BlockUsed[GroupID]] = DebugID;         // Tag this block with the owner ID
              #endif

              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
                m_UsedMemory += SizeBlock;
                m_BlockUsed[GroupID]++;

                if(m_BlockUsed[GroupID] > m_BlockHighest[GroupID])
                {
                    m_BlockHighest[GroupID] = m_BlockUsed[GroupID];
                }
              #endif

              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED) || (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                nOS_LeaveCritical(sr);
              #endif
            }

            m_TimeOut = NOS_WAIT_INFINITE;              // Reset to default
            return MemPtr;
        }
    }

    m_TimeOut = NOS_WAIT_INFINITE;                      // Reset to default
    return MemPtr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: AllocAndClear
//
//   Parameter(s):  size_t                  SizeRequired,       Size of the needed data block
//                  MEM_DebugListOfID_e     DebugID             Owner of the memory block
//
//   Return:        void*           Pointer to allocated block of memory.
//                                  nullptr = No Block available
//
//   Description:   Try to take one block from any of the memory array when the size is equal or
//                  more then the requested amount of data. If no block available, calling thread
//                  will be removed from list of SYS_READY to run threads and be placed in list of
//                  waiting threads for number of ticks specified by TimeOut. If a block of memory
//                  is freed before end of timeout, thread will be awoken and pointer to memory
//                  block will be returned. Also it clear it's content
//
//   Note(s):       This class wrap the nOS Function
//
//-------------------------------------------------------------------------------------------------
void* MemPoolDriver::AllocAndClear(size_t SizeRequired, MEM_DebugListOfID_e DebugID)
{
    void* MemPtr;

    MemPtr = Alloc(SizeRequired, DebugID);

    if(MemPtr != nullptr)
    {
        memset(MemPtr, 0, SizeRequired);
    }

    return MemPtr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: AllocAndClear
//
//   Parameter(s):  size_t                  SizeRequired,       Size of the needed data block
//                  MEM_DebugListOfID_e     DebugID             Owner of the memory block
//
//   Return:        void*           Pointer to allocated block of memory.
//                                  nullptr = No Block available
//
//   Description:   Try to take one block from any of the memory array when the size is equal or
//                  more then the requested amount of data. If no block available, calling thread
//                  will be removed from list of SYS_READY to run threads and be placed in list of
//                  waiting threads for number of ticks specified by TimeOut. If a block of memory
//                  is freed before end of timeout, thread will be awoken and pointer to memory
//                  block will be returned. Also it fill the buffer with the fill value
//
//   Note(s):       This class wrap the nOS Function
//
//-------------------------------------------------------------------------------------------------
void* MemPoolDriver::AllocAndSet(size_t SizeRequired, uint8_t FillValue, MEM_DebugListOfID_e DebugID)
{
    void* MemPtr;

    MemPtr = Alloc(SizeRequired, DebugID);

    if(MemPtr != nullptr)
    {
        memset(MemPtr, int(FillValue), SizeRequired);
    }

    return MemPtr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: OverrideNextTimeOut
//
//   Parameter(s):  TickCount_t    TimeOut             How long it can wait for the data block
//
//                                 NOS_NO_WAIT:                  Don't wait if no blocks available.
//                                 0 > tout < NOS_WAIT_INFINITE: Maximum number of ticks to wait
//                                                               until a block became available.
//                                 NOS_WAIT_INFINITE:            Wait indefinitely until a block
//                                                               became available.
//   Return:        None
//
//   Description:   This will override the default time out for next allocation and reduce number
//                  of Alloc overloaded method.
//
//   Note(s):       This help code reduction as most of the time the default is used.
//
//-------------------------------------------------------------------------------------------------
void MemPoolDriver::OverrideNextTimeOut(TickCount_t TimeOut)
{
    m_TimeOut = TimeOut;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Free
//
//   Parameter(s):  void* pBlock    Pointer to previously allocated block.
//   Return:        bool            true  : Memory block has been freed with success.
//                                  false : Some error occurred
//
//   Description:   Free a previously allocated block of memory.
//
//   Note(s):       if exact result code is necessary, you can call GetLastError();
//
//-------------------------------------------------------------------------------------------------
bool MemPoolDriver::Free(void** pBlock)
{
    uint8_t         GroupID;
  #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
    nOS_StatusReg   sr;
  #endif

    for(GroupID = 0; GroupID < MEM_BLOCK_GROUP_QTS; GroupID++)
    {
        if((*pBlock >= (void*)((uint8_t*)m_nOS_MemArray[GroupID].buffer)) &&
           (*pBlock < ((void*)((uint8_t*)m_nOS_MemArray[GroupID].buffer + (m_nOS_MemArray[GroupID].bsize * m_nOS_MemArray[GroupID].bmax)))))
        {
            m_LastError = nOS_MemFree(&m_nOS_MemArray[GroupID], *pBlock);

            if(m_LastError == NOS_OK)
            {
              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED) || (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                nOS_EnterCritical(sr);
              #endif

              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED)
                m_UsedMemory -= m_nOS_MemArray[GroupID].bsize;
                m_BlockUsed[GroupID]--;
              #endif

              #if (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                m_pDebugInfoArray[GroupID][m_BlockUsed[GroupID]] = MEM_DBG_NONE;            // Mark block as free
              #endif

                *pBlock = nullptr;

              #if (MEMORY_POOL_USE_STAT == DEF_ENABLED) || (MEMORY_POOL_USE_DEBUG_BLOCK_TRACE == DEF_ENABLED)
                nOS_LeaveCritical(sr);
              #endif

                return true;
            }

            return false;
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: IsAvailable
//
//   Parameter(s):  size_t          SizeRequired,       Size of the needed data block
//   Return::       bool                                Block availability.
//                                  false : No block of memory is currently available.
//                                  true  : At least one block of memory is available.
//
//   Description:   Check if at least one block of memory is available.
//
//-------------------------------------------------------------------------------------------------
bool MemPoolDriver::IsAvailable(size_t SizeRequired)
{
    for(uint8_t GroupID = 0; GroupID < MEM_BLOCK_GROUP_QTS; GroupID++)
    {
        if(m_nOS_MemArray[GroupID].bsize <= SizeRequired)
        {
            if(nOS_MemIsAvailable(&m_nOS_MemArray[GroupID]) == true)
            {
                return true;
            }

          #if (DIGINI_RESTRICT_MEMORY_ALLOC_TO_BLOCK_SIZE == DEF_ENABLED)
            return false;
          #endif
        }
    }
    return false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetLastError
//
//   Parameter(s):  None
//   Return::       nOS_Error
//
//   Description:   Return the last known error
//
//-------------------------------------------------------------------------------------------------
nOS_Error MemPoolDriver::GetLastError(void)
{
    return m_LastError;
}

//-------------------------------------------------------------------------------------------------

#if (MEMORY_POOL_USE_STAT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetTotalSizeReserved
//
//   Parameter(s):  None
//   Return::       uint32_t        Used memory in bytes
//
//   Description:   Return the total used memory in pool
//
//-------------------------------------------------------------------------------------------------
uint32_t MemPoolDriver::GetUsedMemory(void)
{
    return m_UsedMemory;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetTotalSizeReserved
//
//   Parameter(s):  None
//   Return::       uint32_t        Total size of pool
//
//   Description:   Return the last known error
//
//-------------------------------------------------------------------------------------------------
uint32_t MemPoolDriver::GetTotalSizeReserved(void)
{
    return MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_TOTAL) + 0;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetNumberOfPool
//
//   Parameter(s):  None
//   Return::       uint32_t        Number of pool
//
//   Description:   Return the number of different pool size
//
//-------------------------------------------------------------------------------------------------
uint32_t MemPoolDriver::GetNumberOfPool(void)
{
    return MEM_BLOCK_GROUP_QTS;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetPoolBlockSize
//
//   Parameter(s):  int             GroupID
//   Return::       uint32_t        Pool Size
//
//   Description:   Return the pool block size
//
//-------------------------------------------------------------------------------------------------
uint32_t MemPoolDriver::GetPoolBlockSize(uint32_t GroupID)
{
    if(GroupID < MEM_BLOCK_GROUP_QTS)
    {
        return m_nOS_MemArray[GroupID].bsize;
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetPoolNumberOfBlock
//
//   Parameter(s):  int             GroupID
//   Return::       uint32_t        Pool Size
//
//   Description:   Return the pool size
//
//-------------------------------------------------------------------------------------------------
uint32_t MemPoolDriver::GetPoolNumberOfBlock(uint32_t GroupID)
{
    if(GroupID < MEM_BLOCK_GROUP_QTS)
    {
        return m_nOS_MemArray[GroupID].bmax;
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetPoolBlockUsed
//
//   Parameter(s):  int             GroupID
//   Return::       uint32_t        Number of block used
//
//   Description:   Return the number of block actually used in a pool
//
//-------------------------------------------------------------------------------------------------
uint32_t MemPoolDriver::GetPoolBlockUsed(uint32_t GroupID)
{
    if(GroupID < MEM_BLOCK_GROUP_QTS)
    {
        return uint32_t(m_BlockUsed[GroupID]);
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetPoolBlockHighPoint
//
//   Parameter(s):  int             GroupID
//   Return::       uint32_t        Highest number of block used
//
//   Description:   Return the highest number of block used in a pool
//
//-------------------------------------------------------------------------------------------------
uint32_t MemPoolDriver::GetPoolBlockHighPoint(uint32_t GroupID)
{
    if(GroupID < MEM_BLOCK_GROUP_QTS)
    {
        return uint32_t(m_BlockHighest[GroupID]);
    }

    return 0;
}

#endif

//-------------------------------------------------------------------------------------------------

#endif // MEM_BLOCK_DEF
