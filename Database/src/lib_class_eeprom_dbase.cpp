//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_eeprom_dbase.cpp
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

#if (DIGINI_USE_EEPROM == DEF_ENABLED)
#if defined(EEPROM_DBASE_DEF)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_EEPROM_DBASE_AS_DRIVER(ENUM_ID, DRIVER, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE)        &DRIVER,
#define EXPAND_X_EEPROM_DBASE_AS_ITEMS_QTY(ENUM_ID, DRIVER, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE)     ITEMS_QTY,
#define EXPAND_X_EEPROM_DBASE_AS_ITEMS_SUB_QTY(ENUM_ID, DRIVER, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE) ITEMS_SubQTY,
#define EXPAND_X_EEPROM_DBASE_AS_ITEM_SIZE(ENUM_ID, DRIVER, ITEMS_QTY, ITEMS_SubQTY, ITEM_SIZE)     ITEM_SIZE,

//-------------------------------------------------------------------------------------------------
//
//   Class: E2_DataBase
//
//
//   Description:   Class E2_DataBase
//
//-------------------------------------------------------------------------------------------------

// Create Driver list for each record item  // TODO create interface for database driver has the type may be from another driver type
MemoryDriverInterface* E2_DataBase::m_Driver[NB_EEPROM_DBASE_ITEMS_CONST] =
{
    EEPROM_DBASE_DEF(EXPAND_X_EEPROM_DBASE_AS_DRIVER)
};

// Create Quantity list for each record item
const uint16_t E2_DataBase::m_ItemsQTY[NB_EEPROM_DBASE_ITEMS_CONST] =                       // Array[THIS][]
{
    EEPROM_DBASE_DEF(EXPAND_X_EEPROM_DBASE_AS_ITEMS_QTY)
};

// Create SUB Quantity list for each record item
const uint16_t E2_DataBase::m_ItemsSubQTY[NB_EEPROM_DBASE_ITEMS_CONST] =                    // Array[][THIS]
{
    EEPROM_DBASE_DEF(EXPAND_X_EEPROM_DBASE_AS_ITEMS_SUB_QTY)
};

// Create size list for each record item
const size_t E2_DataBase::m_ItemSize[NB_EEPROM_DBASE_ITEMS_CONST] =                         // sizeof()
{
    EEPROM_DBASE_DEF(EXPAND_X_EEPROM_DBASE_AS_ITEM_SIZE)
};

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  size_t          ObjectSize
//   Return:        SystemState_e   Always READY in this driver
//
//   Description:   Initialize E2 database
//
//-------------------------------------------------------------------------------------------------
SystemState_e E2_DataBase::Initialize(void* pConfig, size_t ObjectSize)
{
    uint16_t    i;
    size_t      RecordTotalSize;

    if(ObjectSize > sizeof(size_t)) return SYS_WRONG_SIZE;

// TODO ?? check this
  //  m_pDriver
//    m_pE2             = ((E2DBaseInfo_t*)pConfig)->pE2;
//    m_ItemsAddress[0] = ((E2DBaseInfo_t*)pConfig)->Address;

    for(i = 0; i < NB_EEPROM_DBASE_ITEMS_CONST; i++)
    {
        // Precalculate offset for each record
        RecordTotalSize       = (size_t)m_ItemsQTY[i] * (size_t)m_ItemsSubQTY[i] * m_ItemSize[i];
        m_ItemsAddress[i + 1] = ((size_t)(m_ItemsAddress[i]) + RecordTotalSize);
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Get
//
//   Parameter(s):  void*           pData
//                  uint16_t        Record
//                  uint16_t        Number
//                  uint16_t        SubNumber
//   Return:        SystemState_e    State
//
//   Description:   Get the record for EEPROM type database
//
//-------------------------------------------------------------------------------------------------
SystemState_e E2_DataBase::Get(void* pData, uint16_t Record, uint16_t Number, uint16_t SubNumber)
{
    SystemState_e            State;
    uint32_t                 Address;
    MemoryDriverInterface*   pDriver;

    Record -= DBASE_INDEX_EEPROM_RANGE;
    if((State = CheckRange(Record, Number, SubNumber)) != SYS_READY)
    {
        return State;
    }

    Address = GetAddress(Record, Number, SubNumber);
    pDriver = GetDriver(Record);
    pDriver->Read(Address, pData, m_ItemSize[Record]);

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
//   Return:        SystemState_e   State
//
//   Description:   Set the record for EEPROM type database
//
//-------------------------------------------------------------------------------------------------
SystemState_e E2_DataBase::Set(const void* pData, uint16_t Record, uint16_t Number, uint16_t SubNumber)
{
    SystemState_e          State;
    uint32_t               Address;
    MemoryDriverInterface* pDriver;

    Record -= DBASE_INDEX_EEPROM_RANGE;
    if((State = CheckRange(Record, Number, SubNumber)) != SYS_READY)
    {
        return State;
    }

    Address = GetAddress(Record, Number, SubNumber);
    pDriver = GetDriver(Record);
    pDriver->Write(Address, pData, m_ItemSize[Record]);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetDriverIndex
//
//   Parameter(s):  Range_e     Range           RANGE_MIN or RANGE_MAX
//   Return:        uint16_t    Index
//
//   Description:   Return min or the maximum for the range of index
//
//-------------------------------------------------------------------------------------------------
uint16_t E2_DataBase::GetDriverIndex(Range_e Range)
{
    if(Range == RANGE_MIN) return  START_EEPROM_DBASE + 1;
    return END_EEPROM_DBASE - 1;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetSize
//
//   Parameter(s):  size_t*     pSize
//                  uint16_t    Record
//   Return:        uint32_t*   SystemState_e
//
//   Description:   Return the size of record
//
//-------------------------------------------------------------------------------------------------
SystemState_e E2_DataBase::GetSize(size_t* pSize, uint16_t Record)
{
    Record -= DBASE_INDEX_EEPROM_RANGE;
    *pSize = m_ItemSize[Record];
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetInfo
//
//   Parameter(s):  DBaseInfo_t* pInfo
//                  uint16_t     Record
//   Return:        uint32_t*    SystemState_e
//
//   Description:   Return the all info for this of record
//
//-------------------------------------------------------------------------------------------------
SystemState_e E2_DataBase::GetInfo(DBaseInfo_t* pInfo, uint16_t Record)
{
    Record -= DBASE_INDEX_EEPROM_RANGE;
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
SystemState_e E2_DataBase::GetPointer(void** pPointer, uint16_t Record, uint16_t Number, uint16_t SubNumber)
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
//   Return:        SystemState_e    State
//
//   Description:   Check the range provided if it is not out of bound
//
//-------------------------------------------------------------------------------------------------
SystemState_e E2_DataBase::CheckRange(uint16_t Record, uint16_t Number, uint16_t SubNumber)
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
//
//   Function name: GetAddress
//
//   Parameter(s):  uint16_t        Record
//                  uint16_t        Number
//                  uint16_t        SubNumber
//   Return:        uint32_t        Address
//
//   Description:   return the memory address from item information
//
//-------------------------------------------------------------------------------------------------
uint32_t E2_DataBase::GetAddress(uint16_t Record, uint16_t Number, uint16_t SubNumber)
{
    uint32_t Offset;

    Offset  = (uint32_t(Number) * uint32_t(m_ItemsSubQTY[Record])) + uint32_t(SubNumber);       // Calculate offset in items QTY
    Offset *= m_ItemSize[Record];                                                               // Multiply bu the size of the item
    Offset += (uint32_t)m_ItemsAddress[Record];                                                 // Add it to the real pointer value

    return Offset;
}

//-------------------------------------------------------------------------------------------------

#endif // #if defined(EEPROM_DBASE_DEF)
#endif //(DIGINI_USE_EEPROM == DEF_ENABLED)
