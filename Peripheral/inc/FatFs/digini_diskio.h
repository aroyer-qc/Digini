//-------------------------------------------------------------------------------------------------
//
//  File : digini_diskio.h
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
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "diskio.h"

//-------------------------------------------------------------------------------------------------
// Expanding Macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_DRIVE_AS_ENUM(ENUM_ID, CLASS_, DISK_OBJ, ARG)              ENUM_ID,
#define EXPAND_X_DRIVE_AS_OBJ_CREATION(ENUM_ID, CLASS_, DISK_OBJ, ARG)      class CLASS_ DISK_OBJ (ARG);
#define EXPAND_X_DRIVE_AS_OBJ_DECLARATION(ENUM_ID, CLASS_, DISK_OBJ, ARG)   extern class CLASS_ DISK_OBJ;
#define EXPAND_X_DRIVE_AS_OBJ_CONST_IN_DISK(ENUM_ID, CLASS_, DISK_OBJ, ARG) &DISK_OBJ

//-------------------------------------------------------------------------------------------------
// X-Macro
//-------------------------------------------------------------------------------------------------

#define FAT_FS_DRIVE_DEF(X_DRIVE)\
/*  Will create x-macro for defined drive only  */\
    IF_USE( DIGINI_FATFS_USE_SPI_FLASH_CHIP, X_DRIVE( DISK_SPI_FLASH,    FatFS_SPI_Flash,      SPI_Flash,    &mySPI_FLASH))/*  class object those should be set in config file as more than one instance can exist*/ \
    IF_USE( DIGINI_FATFS_USE_SPI_SD_CARD,    X_DRIVE( SPI_SD_CARD,       FatFS_SD_Card,        SPI_SD_Card,  &mySPI_SD   ))\
    IF_USE( DIGINI_FATFS_USE_SDIO_SD_CARD,   X_DRIVE( DISK_SDIO_SD_CARD, FatFS_SDIO,           SDIO_SD_Card, &mySDIO     ))\
    IF_USE( DIGINI_FATFS_USE_USB_KEY,        X_DRIVE( DISK_USB_KEY,      FatFS_USB,            USB_Key,      &myUSB      ))\
    IF_USE( DIGINI_FATFS_USE_RAM_DISK,       X_DRIVE( DISK_RAM_DISK,     FatFS_RAM_Disk,       RAM_Disk,     nullptr     ))\

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef enum
{
    FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_ENUM)
  #ifdef FAT_FS_CUSTOM_DRIVE_DEF 
    FAT_FS_CUSTOM_DRIVE_DEF(EXPAND_X_DRIVE_AS_ENUM)
  #endif  
    NUMBER_OF_DISK,
  //  FF_VOLUMES = NUMBER_OF_DISK,
} DiskMedia_e;

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

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

        static DiskIO_DeviceInterface*    pDiskList          [NUMBER_OF_DISK];              // Number of Disk is define in the FatFs_cfg.h of the application config directory

};
#endif

#ifdef __cplusplus
  #if (DIGINI_USE_FATFS == DEF_ENABLED)
    #ifdef  DISKIO_GLOBAL
      class DiskIO&     FatFS_DiskIO = DiskIO::GetInstance();
      // Create all FatFS_... object
      FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_OBJ_CREATION)
     #ifdef FAT_FS_CUSTOM_DRIVE_DEF 
      FAT_FS_CUSTOM_DRIVE_DEF(EXPAND_X_DRIVE_AS_OBJ_CREATION)
     #endif  
    #else
      FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_OBJ_DECLARATION)
     #ifdef FAT_FS_CUSTOM_DRIVE_DEF 
      FAT_FS_CUSTOM_DRIVE_DEF(EXPAND_X_DRIVE_AS_OBJ_DECLARATION)
     #endif  
    #endif
  #endif
#endif

//-------------------------------------------------------------------------------------------------
