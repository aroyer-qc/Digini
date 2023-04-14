//-------------------------------------------------------------------------------------------------
//
//  File : diskio.h
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_typedef.h"
#include "lib_advanced_macro.h"
#include "ff.h"
#include "FatFs_cfg.h"
#include "diskio_interface.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Command code for disk_ioctrl()
// Generic command
#define CTRL_SYNC                       0       // Mandatory for write functions
#define GET_SECTOR_COUNT                1       // Mandatory for only f_mkfs()
#define GET_SECTOR_SIZE                 2
#define GET_BLOCK_SIZE                  3       // Mandatory for only f_mkfs()
#define CTRL_ERASE_SECTOR               4        // Force erased a block of sectors (for only _USE_ERASE)

#define EXPAND_X_DRIVE_AS_ENUM(ENUM_ID, CLASS_, DISK_OBJ)              ENUM_ID,
#define EXPAND_X_DRIVE_AS_OBJ_CREATION(ENUM_ID, CLASS_, DISK_OBJ)      class CLASS_ DISK_OBJ;
#define EXPAND_X_DRIVE_AS_OBJ_DECLARATION(ENUM_ID, CLASS_, DISK_OBJ)   extern class CLASS_ DISK_OBJ;
#define EXPAND_X_DRIVE_AS_OBJ_CONST_IN_DISK(ENUM_ID, CLASS_, DISK_OBJ) dynamic_cast<DiskIO_DeviceInterface*>(&DISK_OBJ),

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Status of Disk Functions
// They are mapped in Digini errors systems.
typedef SystemState_e   DSTATUS;

typedef enum
{
    FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_ENUM)
    NUMBER_OF_DISK,
} DiskMedia_e;


// Results of Disk Functions
typedef enum
{
    RES_OK = 0,             // 0: Successful
    RES_ERROR,              // 1: R/W Error
    RES_WRPRT,              // 2: Write Protected
    RES_NOTRDY,             // 3: Not Ready
    RES_PARERR              // 4: Invalid Parameter
} DRESULT;

#ifdef __cplusplus
}
#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes for disk control functions needed by FatFS
DSTATUS             disk_initialize         (uint8_t Drive);
DSTATUS             disk_status             (uint8_t Drive);
DRESULT             disk_read               (uint8_t Drive, uint8_t*, uint32_t, uint16_t);
#if _USE_WRITE == 1
DRESULT             disk_write              (uint8_t Drive, const uint8_t*, uint32_t, uint16_t);
#endif
#if _USE_IOCTL == 1
DRESULT             disk_ioctl              (uint8_t Drive, uint8_t, void*);
#endif
uint32_t            get_fattime             (void);

DWORD               ff_convert              (DWORD wch, UINT dir);
DWORD               ff_wtoupper             (DWORD wch);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class DiskIO    // Singleton
{
    public:
        static DiskIO& GetInstance()
        {
            static DiskIO instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
            return instance;
        }


        //void                RegisterDisk        (DiskMedia_e Disk, DiskIO_DeviceInterface* pDisk);

        DSTATUS             Initialize          (DiskMedia_e Disk);
        DSTATUS             Status              (DiskMedia_e Disk);
        DRESULT             Read                (DiskMedia_e Disk, uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
      #if _USE_WRITE == 1
        DRESULT             Write               (DiskMedia_e Disk, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
      #endif
      #if _USE_IOCTL == 1
         DRESULT            IO_Ctrl             (DiskMedia_e Disk, uint8_t Command, void* pBuffer);
      #endif

    //    void                Sync                (DiskMedia_e Disk);
   //     uint32_t            GetSectorCount      (DiskMedia_e Disk);
   //     uint32_t            GetSectorSize       (DiskMedia_e Disk);
   //     uint32_t            GetEraseBlockSize   (DiskMedia_e Disk);

    private:

                            DiskIO              () {}                                              // Constructor? (the {} brackets) are needed here.

    public:
                            // C++ 11
                            // =======
                            // We can use the better technique of deleting the methods we don't want.
                            // Note: Scott Meyers mentions in his Effective Modern
                            //       C++ book, that deleted functions should generally
                            //       be public as it results in better error messages
                            //       due to the compilers behavior to check accessibility
                            //       before deleted status
                            DiskIO(DiskIO const&)               = delete;
        void                operator=(DiskIO const&)            = delete;

        static /*const*/ DiskIO_DeviceInterface*    pDiskList          [NUMBER_OF_DISK];              // Number of Disk is define in the FatFs_cfg.h of the application config directory

};
#endif

#ifdef __cplusplus
 #ifdef DIGINI_USE_FATFS
  #ifdef  DISKIO_GLOBAL
   class DiskIO&     FatFS_DiskIO = DiskIO::GetInstance();
   // Create all CFatFS_... object
   FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_OBJ_CREATION)
  #else
   FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_OBJ_DECLARATION)
  #endif
 #endif
#endif
//-------------------------------------------------------------------------------------------------
