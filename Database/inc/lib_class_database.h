//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_database.h
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

#if (DIGINI_USE_DATABASE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./Database/inc/lib_database_wrapper.h"
#include "./Digini/inc/lib_typedef.h"
#include "project_def.h"
#include "./Digini/inc/lib_label.h"
#include "database_cfg.h"
#if (DIGINI_USE_GRAFX == DEF_ENABLED)
  #include "./Grafx/inc/lib_grafx_dbase.h"
#endif
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Global Macro
//-------------------------------------------------------------------------------------------------

#ifdef LIB_CLASS_DATABASE_GLOBAL
    #define EXTERN_LIB_CLASS_DATABASE
#else
    #define EXTERN_LIB_CLASS_DATABASE extern
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define     DBASE_INTERFACE_POOL                7      // Size of the interface pool (must fit next define)

#define     DBASE_INDEX_BKPREG_RANGE            0
#define     DBASE_INDEX_EEPROM_RANGE            1000
#define     DBASE_INDEX_HARD_RANGE              2000
#define     DBASE_INDEX_RAM_RANGE               3000
#define     DBASE_INDEX_ROM_RANGE               4000
#define     DBASE_INDEX_SYSTEM_RANGE            5000
#define     DBASE_INDEX_QSPI_RANGE              6000

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_DBASE_AS_ENUM(ENUM_ID, RAM_RECORD, ROM_RECORD, INTERVAL, START_TIME) ENUM_ID,

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

enum DBaseItemList_e
{
    START_DBASE_INDEX = - 1,
  #ifdef DBASE_DEF
    DBASE_DEF(EXPAND_X_DBASE_AS_ENUM)
  #endif
    END_DBASE_INDEX,
    NB_DBASE_ITEMS_CONST = ((END_DBASE_INDEX - START_DBASE_INDEX) - 1)
};

struct DBaseInfo_t
{
    uint16_t ItemsQTY;
    uint16_t SubItemsQTY;
    uint32_t Size;
};

//-------------------------------------------------------------------------------------------------
//  class
//-------------------------------------------------------------------------------------------------

class CDataBaseInterface
{
    public:

                                CDataBaseInterface       (){};
        virtual SystemState_e   Initialize               (void* pConfig,      size_t ObjectSize) { VAR_UNUSED(pConfig); VAR_UNUSED(ObjectSize); return SYS_READY;}; // Initialize DataBase
        virtual SystemState_e   Get                      (void*       pData,  uint16_t Record, uint16_t Number, uint16_t SubNumber)  = 0;                           // Read a Record
        virtual SystemState_e   Set                      (const void* pData,  uint16_t Record, uint16_t Number, uint16_t SubNumber)  = 0;                           // Write a Record
        virtual uint16_t        GetDriverIndex           (Range_e Range)                                                             = 0;
        virtual SystemState_e   GetSize                  (size_t* pSize,      uint16_t Record)                                       = 0;                           // Get Record Size
        virtual SystemState_e   GetInfo                  (DBaseInfo_t* pInfo, uint16_t Record)                                       = 0;                           // Get Record Info
        virtual SystemState_e   GetPointer               (void** pAddress,    uint16_t Record, uint16_t Number, uint16_t SubNumber)  = 0;                           // Get Record address if it is memory location
        virtual SystemState_e   SetDB_Address            (void** pAddress)                                                           = 0;                           // Set database address if supported by module
};

class CDataBase
{
    public:

                                 CDataBase               ();
        SystemState_e            RegisterDriver          (CDataBaseInterface* DataBaseInterface);                                                               // Register a hardware access database
        SystemState_e            Initialize              (void);                                                                                                // Initialize DataBase
        SystemState_e            Get                     (void*       pData,  uint16_t Record, uint16_t Number = 0, uint16_t SubNumber = 0);                    // Read a Record
        SystemState_e            Set                     (const void* pData,  uint16_t Record, uint16_t Number = 0, uint16_t SubNumber = 0);                    // Write a Record
        SystemState_e            GetAll                  (void*       pData,  uint16_t Record);                                                                 // Read a full set of the Record
        SystemState_e            SetAll                  (const void* pData,  uint16_t Record);                                                                 // Write a full set of the Record
        SystemState_e            Fill                    (uint8_t Value,      uint16_t Record, uint16_t Number = 0, uint16_t SubNumber = 0);                    // Fill a record with specific value
        SystemState_e            GetSize                 (size_t* pSize,      uint16_t Record);                                                                 // Get Record Size
        SystemState_e            GetInfo                 (DBaseInfo_t* pInfo, uint16_t Record);                                                                 // Get Record Info
        SystemState_e            GetPointer              (void** pAddress, uint16_t Record, uint16_t Number = 0, uint16_t SubNumber   = 0);                     // Read a record pointer address
        SystemState_e            SetDB_Address           (void** pAddress, uint16_t Record);                                                                    // Set database address if supported by module
       #ifdef DBASE_DEF
        SystemState_e            ForceUpdate             (uint16_t Record, uint16_t Number, uint16_t SubNumber);
       #endif
        void                     TimerCallBack           (void);

    private:

        CDataBaseInterface*      GetDriver               (uint16_t  Record);

        uint8_t                  m_PoolCount;
        CDataBaseInterface*      m_pDriver               [DBASE_INTERFACE_POOL];
        uint16_t                 m_FreeIndexPointer;

       #ifdef DBASE_DEF
        bool                     m_IsItInSync            [NB_DBASE_ITEMS_CONST];
        static const uint16_t    m_RamRecord             [NB_DBASE_ITEMS_CONST];
        static const uint16_t    m_RomRecord             [NB_DBASE_ITEMS_CONST];
        static const uint32_t    m_Interval              [NB_DBASE_ITEMS_CONST];
        static const uint32_t    m_StartTime             [NB_DBASE_ITEMS_CONST];
       #endif
};

//-------------------------------------------------------------------------------------------------
// Global Variable(s)
//-------------------------------------------------------------------------------------------------

EXTERN_LIB_CLASS_DATABASE class CDataBase DB_Central;

//-------------------------------------------------------------------------------------------------
// Post Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./Digini/Database/inc/lib_class_eeprom_dbase.h"
#include "./Digini/Database/inc/lib_class_qspi_dbase.h"
#include "./Digini/Database/inc/lib_class_hard_dbase.h"
#include "./Digini/Database/inc/lib_class_ram_dbase.h"
#include "./Digini/Database/inc/lib_class_rom_dbase.h"
#include "./Digini/Database/inc/port/lib_class_bkpreg_dbase.h"
#include "./Digini/Database/inc/lib_database_wrapper.h"

//-------------------------------------------------------------------------------------------------

#endif //(DIGINI_USE_DATABASE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
