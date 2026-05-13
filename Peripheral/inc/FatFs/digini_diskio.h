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
#include "diskio_def.h"

//-------------------------------------------------------------------------------------------------
// Expanding Macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_DRIVE_AS_ENUM(ENUM_ID, CLASS_, DISK_OBJ, ARG)              ENUM_ID,
#define EXPAND_X_DRIVE_AS_OBJ_CONST_IN_DISK(ENUM_ID, CLASS_, DISK_OBJ, ARG) &DISK_OBJ

//-------------------------------------------------------------------------------------------------
// X-Macro example
//-------------------------------------------------------------------------------------------------

#if 0
// Those are example
#define FAT_FS_DRIVE_DEF(X_DRIVE)\
/*  Will create x-macro for defined drive only  */\
    IF_USE( DIGINI_FATFS_USE_SPI_FLASH_CHIP, X_DRIVE( DISK_SPI_FLASH,    FatFS_SPI_Flash,      SPI_Flash,    &mySPI_FLASH))/*  class object those should be set in config file as more than one instance can exist*/ \
    IF_USE( DIGINI_FATFS_USE_SPI_SD_CARD,    X_DRIVE( SPI_SD_CARD,       FatFS_SD_Card,        SPI_SD_Card,  &mySPI_SD   ))\
    IF_USE( DIGINI_FATFS_USE_SDIO_SD_CARD,   X_DRIVE( DISK_SDIO_SD_CARD, FatFS_SDIO,           SDIO_SD_Card, &mySDIO     ))\
    IF_USE( DIGINI_FATFS_USE_USB_KEY,        X_DRIVE( DISK_USB_KEY,      FatFS_USB,            USB_Key,      &myUSB      ))\
    IF_USE( DIGINI_FATFS_USE_RAM_DISK,       X_DRIVE( DISK_RAM_DISK,     FatFS_RAM_Disk,       RAM_Disk,     nullptr     ))\

#endif

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
	INVALID_DISK,
} DiskMedia_e;

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

class DiskIO    // Singleton
{
    public:

        static DiskIO& 		GetInstance			(void)						{ static DiskIO instance; return instance; }

        DSTATUS             Initialize          (DiskMedia_e Disk);
        DSTATUS             Status              (DiskMedia_e Disk);
        DRESULT             Read                (DiskMedia_e Disk, uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
      #if _USE_WRITE == 1
        DRESULT             Write               (DiskMedia_e Disk, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
      #endif
      #if _USE_IOCTL == 1
         DRESULT            IO_Ctrl             (DiskMedia_e Disk, uint8_t Command, void* pBuffer);
      #endif

    private:

        static DiskIO_DeviceInterface*   		pDiskList    		[NUMBER_OF_DISK];

};
#endif

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
  #if (DIGINI_USE_FATFS == DEF_ENABLED)
    #ifdef  DISKIO_GLOBAL
      class DiskIO&     FatFS_DiskIO = DiskIO::GetInstance();

  	  const MKFS_PARM 	MKFS_Option =
	  {
		  /*.fmt     =*/ FM_ANY | FM_SFD,
		  /*.n_fat   =*/ 0,          		// 0 = Auto-select number of FAT tables (usually 2)
		  /*.align   =*/ 0,          		// 0 = Auto-align clusters to data area block size
		  /*.n_root  =*/ 0,         		// 0 = Auto-select root directory entries (for FAT12/16)
		  /*.au_size =*/ 0,        			// 0 = Auto-select cluster size based on disk capacity
	  };

    #else

  	  extern const MKFS_PARM MKFS_Option;

    #endif
  #endif
#endif

//-------------------------------------------------------------------------------------------------
