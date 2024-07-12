//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_hard_dbase.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if defined(RTC_DBASE_DEF) || defined(HARD_DBASE_DEF)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_HARD_DBASE_AS_ITEMS_QTY(ENUM_ID, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE, CALLBACK)     ITEMS_QTY,
#define EXPAND_X_HARD_DBASE_AS_ITEMS_SUB_QTY(ENUM_ID, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE, CALLBACK) ITEMS_SubQTY,
#define EXPAND_X_HARD_DBASE_AS_ITEM_SIZE(ENUM_ID, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE, CALLBACK)     ITEM_SIZE,
#define EXPAND_X_HARD_DBASE_AS_CALLBACK(ENUM_ID, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE, CALLBACK)      CALLBACK,

//-------------------------------------------------------------------------------------------------
//
//   Class: HARD_DataBase
//
//
//   Description:   Class HARD_DataBase
//
//-------------------------------------------------------------------------------------------------

// Create Quantity list for each record item
const uint16_t HARD_DataBase::m_ItemsQTY[NB_HARD_DBASE_ITEMS_CONST] =                                      // Array[THIS][]
{
  #ifdef HARD_DBASE_DEF
      HARD_DBASE_DEF(EXPAND_X_HARD_DBASE_AS_ITEMS_QTY)
  #endif
};

// Create SUB Quantity list for each record item
const uint16_t HARD_DataBase::m_ItemsSubQTY[NB_HARD_DBASE_ITEMS_CONST] =                                   // Array[][THIS]
{
  #ifdef HARD_DBASE_DEF
      HARD_DBASE_DEF(EXPAND_X_HARD_DBASE_AS_ITEMS_SUB_QTY)
  #endif
};

// Create size list for each record item
const size_t HARD_DataBase::m_ItemSize[NB_HARD_DBASE_ITEMS_CONST] =                                        // sizeof()
{
  #ifdef HARD_DBASE_DEF
      HARD_DBASE_DEF(EXPAND_X_HARD_DBASE_AS_ITEM_SIZE)
  #endif
};

// Create Get/Set function pointer list for each record item
const Func_DatabaseCallBack HARD_DataBase::m_CallBack[NB_HARD_DBASE_ITEMS_CONST] =
{
  #ifdef HARD_DBASE_DEF
      HARD_DBASE_DEF(EXPAND_X_HARD_DBASE_AS_CALLBACK)
  #endif
};

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  void*       pConfig
//                  size_t      ObjectSize
//   Return:        SystemState_e
//
//   Description:   Initialize Hardware database
//
//-------------------------------------------------------------------------------------------------
SystemState_e HARD_DataBase::Initialize(void* pConfig, size_t ObjectSize)
{
    // Nothing in this driver to init
    VAR_UNUSED(pConfig);
    VAR_UNUSED(ObjectSize);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetDriverIndex
//
//   Parameter(s):  Range_e     Range           RANGE_MIN or RANGE_MAX
//   Return:        uint16_t
//
//   Description:   Return min or the maximum  for the range of index
//
//-------------------------------------------------------------------------------------------------
uint16_t HARD_DataBase::GetDriverIndex(Range_e Range)
{
    if(Range == RANGE_MIN) return START_HARD_DBASE + 1;
    return END_HARD_DBASE - 1;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Get
//
//   Parameter(s):  void*       pData
//                  uint16_t    Record
//                  uint16_t    Number
//                  uint16_t    SubNumber
//   Return:        SystemState_e
//
//   Description:   Get the record for HARD type database
//
//-------------------------------------------------------------------------------------------------
SystemState_e HARD_DataBase::Get(void* pData, uint16_t Record, uint16_t Number, uint16_t SubNumber)
{
    SystemState_e    State;

    Record -= DBASE_INDEX_HARD_RANGE;
    if((State = CheckRange(Record, Number, SubNumber)) != SYS_READY)
    {
        return State;
    }

    m_CallBack[Record](ACCESS_READ, pData, Number, SubNumber);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Set
//
//   Parameter(s):  const void*     pData
//                  uint16_t        Record
//                  uint16_t        Number
//                  uint16_t        SubNumber
//   Return:        SystemState_e
//
//   Description:   Set the record for HARD type database
//
//-------------------------------------------------------------------------------------------------
SystemState_e HARD_DataBase::Set(const void* pData, uint16_t Record, uint16_t Number, uint16_t SubNumber)
{
    SystemState_e    State;

    Record -= DBASE_INDEX_HARD_RANGE;
    if((State = CheckRange(Record, Number, SubNumber)) != SYS_READY)
    {
        return State;
    }

    m_CallBack[Record](ACCESS_WRITE, (void*)pData, Number, SubNumber);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetSize
//
//   Parameter(s):  size_t*     pSize
//                  uint16_t    Record
//
//   Return:        SystemState_e
//
//   Description:   Return the size of record
//
//-------------------------------------------------------------------------------------------------
SystemState_e HARD_DataBase::GetSize(size_t* pSize, uint16_t Record)
{
    Record -= DBASE_INDEX_HARD_RANGE;
    *pSize = m_ItemSize[Record];
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetInfo
//
//   Parameter(s):  DBaseInfo_t*    pInfo
//                  uint16_t        Record
//   Return:        uint32_t*       SystemState_e
//
//   Description:   Return the all info for this of record
//
//-------------------------------------------------------------------------------------------------
SystemState_e HARD_DataBase::GetInfo(DBaseInfo_t* pInfo, uint16_t Record)
{
    Record -= DBASE_INDEX_HARD_RANGE;
    pInfo->ItemsQTY    = m_ItemsQTY[Record];
    pInfo->SubItemsQTY = m_ItemsSubQTY[Record];
    pInfo->Size        = m_ItemSize[Record];
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetPointer
//
//   Parameter(s):  void**          pPointer
//                  uint16_t        Record
//                  uint16_t        Number
//                  uint16_t        SubNumber
//   Return:        SystemState_e
//
//   Description:   Return the memory address from item information
//
//-------------------------------------------------------------------------------------------------
SystemState_e HARD_DataBase::GetPointer(void** pPointer, uint16_t Record, uint16_t Number, uint16_t SubNumber)
{
    *pPointer = nullptr;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: CheckRange
//
//   Parameter(s):  uint16_t        Record
//                  uint16_t        Number
//                  uint16_t        SubNumber
//   Return:        SystemState_e
//
//   Description:   Check the range provided if not out of bound
//
//-------------------------------------------------------------------------------------------------
SystemState_e HARD_DataBase::CheckRange(uint16_t Record, uint16_t Number, uint16_t SubNumber)
{
    if(Number < m_ItemsQTY[Record])
    {
        if(SubNumber < m_ItemsSubQTY[Record])
        {
            return SYS_READY;
        }
    }

    return SYS_OUT_OF_RANGE;
}

//-------------------------------------------------------------------------------------------------

#endif // #if defined(RTC_DBASE_DEF) || defined(HARD_DBASE_DEF)
