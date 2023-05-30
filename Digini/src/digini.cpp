//-------------------------------------------------------------------------------------------------
//
//  File : digini.cpp
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

#define DIGINI_GLOBAL
#include "lib_digini.h"
#undef  DIGINI_GLOBAL
#include "bsp.h"

//-------------------------------------------------------------------------------------------------
// local Class
//-------------------------------------------------------------------------------------------------

#if defined(EEPROM_DBASE_DEF)
  class EEPROM_DataBase             DB_Eeprom;
#endif

#if defined(ROM_DBASE_DEF)
  class ROM_DataBase                DB_Rom;
#endif

#if defined(RAM_DBASE_DEF) || defined(GFX_RAM_DBASE_DEF) || defined(NV_RAM_DBASE_DEF)
  class RAM_DataBase                DB_Ram;
#endif

#if defined(BACKUP_DBASE_DEF)
  class BKPREG_DataBase             DB_BackupRegister;
#endif

#if defined(HARD_DBASE_DEF)
  class HARD_DataBase               DB_Hardware;               // Time, ADC, etc...
#endif

//-------------------------------------------------------------------------------------------------
//
// Name:           DIGINI_Initialize
// Parameter(s):   void
// Return:         SystemState_e
//
// Description:    Initialize DIGINI Lib
//
// Note(s):        Here we create the task that will start the GRAFX if used
//                 Also initialize other lib
//
//-------------------------------------------------------------------------------------------------
SystemState_e DIGINI_Initialize(void)
{
  #if (USE_RTC_DRIVER == DEF_ENABLED)
    myRTC.Initialize(RTC_CLOCK_MODE_LSI);            // RTC module object
  #endif

    // Register all database driver

  #if defined(EEPROM_DBASE_DEF)
     DB_Eeprom.Initialize(nullptr, 0);
     DB_Central.RegisterDriver(&DB_Eeprom);
  #endif

  #if defined(ROM_DBASE_DEF)
    DB_Central.RegisterDriver(&DB_Rom);
  #endif

  #if defined(RAM_DBASE_DEF) || defined(GFX_RAM_DBASE_DEF) || defined(NV_RAM_DBASE_DEF)
    RAM_DBaseRegionPointer_t DBaseRegion;

   #if defined(RAM_DBASE_DEF)
    DBaseRegion.pRam = reinterpret_cast<uint8_t*>(RAM_DBASE_ADDRESS);
   #endif

   #if defined(GFX_RAM_DBASE_DEF)
    DBaseRegion.pGFX_Ram = reinterpret_cast<uint8_t*>(GFX_RAM_ADDRESS);
   #endif

   #if defined(NV_RAM_DBASE_DEF)
    DBaseRegion.pNV_Ram = NV_RAM_DBASE_ADDRESS;
   #endif

    DB_Ram.Initialize((void *)&DBaseRegion, sizeof(void*));
    DB_Central.RegisterDriver(&DB_Ram);
  #endif

  #if defined(BACKUP_DBASE_DEF)
    if(DB_BackupRegister.Inititialize(BSP_pRTC, 0) != SYS_READY)
    {
        assert_failed((const char*)__FILE__, __LINE__);
    }
    DB_Central.RegisterDriver(&DB_BackupRegister);
  #endif

   #if defined(HARD_DBASE_DEF)
    DB_Central.RegisterDriver(&DB_Hardware);
   #endif

  #if (DIGINI_USE_GRAFX == DEF_ENABLED)
    GRAFX_Initialize();
  #endif

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           DIGINI_PostInitialize
// Parameter(s):   void
// Return:         SystemState_e
//
// Description:    Initialize DIGINI Lib
//
// Note(s):        Here we create the task that will start the GRAFX if used
//                 Also initialize other lib
//
//-------------------------------------------------------------------------------------------------
SystemState_e DIGINI_PostInitialize(void)
{
    //nOS_Error Error;
    //SystemState_e State;

  #if (USE_USB_DRIVER == DEF_ENABLED)
    USB.Initialize();
  #endif // (USE_USB_DRIVER == DEF_ENABLED)

  #if (DIGINI_USE_FATFS == DEF_ENABLED)
    //FatFS_DiskIO.Initialize();
  #endif

  #if (DIGINI_USE_GRAFX == DEF_ENABLED)
    GRAFX_PostInitialize();
  #endif

    return SYS_READY;
}
