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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MEM_DBG_DEF(X_MEM_DBG) \
    X_MEM_DBG(MEM_DBG_NONE)                                     \
    X_MEM_DBG(MEM_DBG_STR1)                                     \
    X_MEM_DBG(MEM_DBG_STR2)                                     \
    /* Unique ID for lib_fifo                                 */\
    X_MEM_DBG(MEM_DBG_FIFO1)                                    \
    /* Unique ID for Comm CLI                                 */\
\
\
  /*---------------------------------------------------------*/ \
  IF_USE(DIGINI_USE_CMD_LINE,                                   \
  /*---------------------------------------------------------*/ \
    X_MEM_DBG(MEM_DBG_CLI1)                                     \
  /*---------------------------------------------------------*/ \
  ) /* End of DIGINI_USE_CMD_LINE */                            \
  /*---------------------------------------------------------*/ \
\
\
  /*---------------------------------------------------------*/ \
  IF_USE(DIGINI_USE_CONSOLE,                                    \
  /*---------------------------------------------------------*/ \
    /* Unique ID for Comm Console                             */\
    X_MEM_DBG(MEM_DBG_CON1)                                     \
    X_MEM_DBG(MEM_DBG_CON2)                                     \
    X_MEM_DBG(MEM_DBG_CON3)                                     \
  /*---------------------------------------------------------*/ \
  ) /* End of DIGINI_USE_CONSOLE */                             \
  /*---------------------------------------------------------*/ \
\
\
  /*---------------------------------------------------------*/ \
  IF_USE(DIGINI_USE_VT100_MENU,                                 \
  /*---------------------------------------------------------*/ \
    /* Unique ID for Comm VT100                               */\
    X_MEM_DBG(MEM_DBG_VT100_1)                                  \
    X_MEM_DBG(MEM_DBG_VT100_2)                                  \
    X_MEM_DBG(MEM_DBG_VT100_3)                                  \
    /* Unique ID for lib_vt100_callback.cpp                   */\
    X_MEM_DBG(MEM_DBG_VTCB1)                                    \
    X_MEM_DBG(MEM_DBG_VTCB2)                                    \
    X_MEM_DBG(MEM_DBG_VTCB3)                                    \
    X_MEM_DBG(MEM_DBG_VTCB4)                                    \
    X_MEM_DBG(MEM_DBG_VTCB5)                                    \
    X_MEM_DBG(MEM_DBG_VTCB6)                                    \
    X_MEM_DBG(MEM_DBG_VTCB7)                                    \
    X_MEM_DBG(MEM_DBG_LOG)                                      \
  /*---------------------------------------------------------*/ \
  ) /* End of DIGINI_USE_VT100_MENU */                          \
  /*---------------------------------------------------------*/ \
\
\
  /*---------------------------------------------------------*/ \
  IF_USE(DIGINI_USE_ETHERNET,                                   \
  /*---------------------------------------------------------*/ \
    /* Unique ID for NanoIP                                   */\
    X_MEM_DBG(MEM_DBG_TSKNET1)                                  \
    X_MEM_DBG(MEM_DBG_TSKNET2)                                  \
    X_MEM_DBG(MEM_DBG_ETHERIF1)                                 \
    X_MEM_DBG(MEM_DBG_ETHDMARX1)                                \
    X_MEM_DBG(MEM_DBG_ETHDMARX2)                                \
    X_MEM_DBG(MEM_DBG_ETHDMATX)                                 \
    X_MEM_DBG(MEM_DBG_IPPKT)                                    \
    X_MEM_DBG(MEM_DBG_ICMP)                                     \
    X_MEM_DBG(MEM_DBG_ICMPDT)                                   \
    X_MEM_DBG(MEM_DBG_SOCKET)                                   \
    X_MEM_DBG(MEM_DBG_SOCKTCP)                                  \
    X_MEM_DBG(MEM_DBG_SOCKUDP)                                  \
    X_MEM_DBG(MEM_DBG_ARPRX)                                    \
    X_MEM_DBG(MEM_DBG_ARPDTRX)                                  \
    X_MEM_DBG(MEM_DBG_ARPTX)                                    \
    X_MEM_DBG(MEM_DBG_ARPDTTX)                                  \
    X_MEM_DBG(MEM_DBG_ARPPO)                                    \
    X_MEM_DBG(MEM_DBG_ARPDTPO)                                  \
    X_MEM_DBG(MEM_DBG_UDP)                                      \
    X_MEM_DBG(MEM_DBG_UDPDT)                                    \
    X_MEM_DBG(MEM_DBG_DHCP)                                     \
    X_MEM_DBG(MEM_DBG_DHCPTX)                                   \
    X_MEM_DBG(MEM_DBG_DNSTX)                                    \
    X_MEM_DBG(MEM_DBG_DNSRX)                                    \
    X_MEM_DBG(MEM_DBG_SNTPTX)                                   \
    X_MEM_DBG(MEM_DBG_SNTPRX)                                   \
    X_MEM_DBG(MEM_DBG_TCP)                                      \
    X_MEM_DBG(MEM_DBG_TCPDT)                                    \
    X_MEM_DBG(MEM_DBG_TCPR)                                     \
    X_MEM_DBG(MEM_DBG_TCPRDT)                                   \
    X_MEM_DBG(MEM_DBG_TCPCPY)                                   \
  /*---------------------------------------------------------*/ \
  ) /* End of DIGINI_USE_ETHERNET */                            \
  /*---------------------------------------------------------*/ \
\
\
  /*---------------------------------------------------------*/ \
  IF_USE(IP_USE_MQTT,                                           \
  /*---------------------------------------------------------*/ \
    X_MEM_DBG(MEM_DBG_MQTT1)                                    \
    X_MEM_DBG(MEM_DBG_MQTT2)                                    \
    X_MEM_DBG(MEM_DBG_MQTT3)                                    \
    X_MEM_DBG(MEM_DBG_MQTT4)                                    \
    X_MEM_DBG(MEM_DBG_MQTT5)                                    \
    X_MEM_DBG(MEM_DBG_MQTT6)                                    \
    X_MEM_DBG(MEM_DBG_MQTTSUB)                                  \
    X_MEM_DBG(MEM_DBG_MQTTMSG)                                  \
    X_MEM_DBG(MEM_DBG_MQTTTOPIC)                                \
    X_MEM_DBG(MEM_DBG_MQTTLOAD)                                 \
  /*---------------------------------------------------------*/ \
  ) /* End of IP_USE_MQTT */                                    \
  /*---------------------------------------------------------*/ \
    X_MEM_DBG(MEM_DBG_SERVICE)  /*grafx service*/               \

\
\

    /* UniqueID for lib_service                               */   // actually not used

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_MEM_BLOCK_AS_ENUM(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)        ENUM_ID,
#define EXPAND_X_MEM_BLOCK_AS_ARRAY_DECL(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)  uint8_t m_##GROUP_NAME[BLOCK_MAX][BLOCK_SIZE] __attribute__ ((aligned (4)));
#define EXPAND_X_MEM_BLOCK_AS_TOTAL(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)       (BLOCK_MAX * BLOCK_SIZE) +

// For debug block trace
#define EXPAND_X_MEM_BLOCK_AS_ARRAY_DEBUG_BLOCK_TRACE(ENUM_ID, GROUP_NAME, ALLOC_NAME, BLOCK_MAX, BLOCK_SIZE)  MEM_DebugListOfID_e m_DebugTrace ## GROUP_NAME[BLOCK_MAX];

#define EXPAND_X_MEM_DBG_AS_ENUM(ENUM_ID)       ENUM_ID,
#define EXPAND_X_MEM_DBG_AS_LBL_ENUM(ENUM_ID)   LBL_##ENUM_ID,
#define EXPAND_X_MEM_DBG_AS_DATA(ENUM_ID)       {STRINGIFY(ENUM_ID) , nullptr},

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum MEM_DebugListOfID_e
{
    MEM_DBG_FREE            = 0,

    MEM_DBG_DEF(EXPAND_X_MEM_DBG_AS_ENUM)

  #if MEM_USER_DBG_DEF
    MEM_USER_DBG_DEF(EXPAND_X_MEM_DBG_AS_ENUM)
  #endif

    NUMBER_OF_MEM_DBG,
};

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

        void*           Alloc                       (size_t SizeRequired, MEM_DebugListOfID_e DbgID = MEM_DBG_NONE);
        void*           AllocAndClear               (size_t SizeRequired, MEM_DebugListOfID_e DbgID = MEM_DBG_NONE);
        void*           AllocAndSet                 (size_t SizeRequired, uint8_t FillValue, MEM_DebugListOfID_e DbgID = MEM_DBG_NONE);
        bool            Free                        (void** pBlock);
        bool            IsAvailable                 (size_t SizeRequired);
        nOS_Error       GetLastError                (void);
      #if (MEMORY_POOL_USE_DEBUG_STAT == DEF_ENABLED)
        uint32_t        GetTotalSizeReserved        (void);
        uint32_t        GetUsedMemory               (void);
        uint32_t        GetNumberOfPool             (void);
        uint32_t        GetPoolNumberOfBlock        (uint32_t PoolNumber);
        uint32_t        GetPoolBlockSize            (uint32_t PoolNumber);
        uint32_t        GetPoolBlockUsed            (uint32_t PoolNumber);
        uint32_t        GetPoolBlockHighPoint       (uint32_t PoolNumber);
        uint32_t        GetAllocCount               (MEM_DebugListOfID_e DbgID)     { return m_AllocCount[DbgID];           }
        uint32_t        GetMaxDebugID               (void)                          { return uint32_t(NUMBER_OF_MEM_DBG);   }
        bool            ChangeDebugID               (void* MemBlock, MEM_DebugListOfID_e OriginalDebugID, MEM_DebugListOfID_e NewDebugID);
      #endif


    private:

        void*           TryAllocFromGroup           (uint8_t GroupID, size_t SizeRequired, MEM_DebugListOfID_e DebugID, nOS_TickCounter TimeOut);

        nOS_Mem                     m_nOS_MemArray      [MEM_BLOCK_GROUP_QTS];                  // handler to give to nOS_Mem... function
        void*                       m_pBufferArray      [MEM_BLOCK_GROUP_QTS];                  // pointer array of the memory block
        nOS_Error                   m_LastError;
      #if (MEMORY_POOL_USE_DEBUG_STAT == DEF_ENABLED)
        nOS_MemCounter              m_BlockUsed         [MEM_BLOCK_GROUP_QTS];
        nOS_MemCounter              m_BlockHighest      [MEM_BLOCK_GROUP_QTS];
        uint32_t                    m_UsedMemory;
        uint32_t                    m_AllocCount        [NUMBER_OF_MEM_DBG];
        MEM_DebugListOfID_e*        m_pDebugInfoArray   [MEM_BLOCK_GROUP_QTS];                  // pointer array of the memory block
        MEM_BLOCK_DEF(EXPAND_X_MEM_BLOCK_AS_ARRAY_DEBUG_BLOCK_TRACE)
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

#ifndef MEM_BLOCK_DEF

#pragma message("DIGINI use memory pool instead of the malloc library. please define you memblock into memory_cfg.h")

#endif  // MEM_BLOCK_DEF

//-------------------------------------------------------------------------------------------------
