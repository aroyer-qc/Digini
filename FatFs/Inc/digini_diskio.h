//-------------------------------------------------------------------------------------------------
//
//  File : digini_diskio.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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
//
// note(s)
//
// Those are example
//      #define FAT_FS_DRIVE_DEF(X_DRIVE)
//      / *  Will create x-macro for defined drive only  * /
//    X_DRIVE( DISK_SPI_FLASH,    FatFS_SPI_Flash,      SPI_Flash,    &mySPI_FLASH)
//    X_DRIVE( SPI_SD_CARD,       FatFS_SD_Card,        SPI_SD_Card,  &mySPI_SD   )
//    X_DRIVE( DISK_SDIO_SD_CARD, FatFS_SDIO,           SDIO_SD_Card, &mySDIO     )
//    X_DRIVE( DISK_USB_KEY,      FatFS_USB,            USB_Key,      &myUSB      )
//    X_DRIVE( DISK_RAM_DISK,     FatFS_RAM_Disk,       RAM_Disk,     nullptr     )
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Expanding Macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_DRIVE_AS_ENUM(ENUM_ID, CLASS_, DISK_OBJ, ARG)                          ENUM_ID,
#define EXPAND_X_DRIVE_AS_DISK_CLASS_OBJECT_DECLARATION(ENUM_ID, CLASS_, DISK_OBJ, ARG) CLASS_ DISK_OBJ;

//-------------------------------------------------------------------------------------------------
// X-Macro example
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

enum DiskMedia_e
{
    FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_ENUM)
    NUMBER_OF_DISK,
  //  FF_VOLUMES = NUMBER_OF_DISK,
	INVALID_DISK,
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------


class DiskIO    // Singleton
{
    public:

        static DiskIO& 		GetInstance			(void)						{ static DiskIO instance; return instance; }

        DSTATUS             Initialize          (DiskMedia_e Disk);
        DSTATUS             Status              (DiskMedia_e Disk);
        DRESULT             Read                (DiskMedia_e Disk, uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
        DRESULT             Write               (DiskMedia_e Disk, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
         DRESULT            IO_Ctrl             (DiskMedia_e Disk, uint8_t Command, void* pBuffer);

    private:

        static DiskIO_DeviceInterface*   		pDiskList    		[NUMBER_OF_DISK];
        static void*                            pParameterList      [NUMBER_OF_DISK];

        FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_DISK_CLASS_OBJECT_DECLARATION)
};
#endif

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
  #if (DIGINI_USE_FATFS == DEF_ENABLED)
    #ifdef  DISKIO_GLOBAL
      class DiskIO&     FatFS_DiskIO = DiskIO::GetInstance();

      // Use 'Super Floppy Disk' to remove unused MBR sector
  	  const MKFS_PARM 	MKFS_Option =
	  {
		  /*.fmt     =*/ FM_ANY | FM_SFD,
		  /*.n_fat   =*/ 0,          		// 0 = Auto-select number of FAT tables (usually 2)
		  /*.align   =*/ 0,          		// 0 = Auto-align clusters to data area block size
		  /*.n_root  =*/ 0,         		// 0 = Auto-select root directory entries (for FAT12/16)
		  /*.au_size =*/ 0,        			// 0 = Auto-select cluster size based on disk capacity
	  };

    #else

      extern class DiskIO& FatFS_DiskIO;
  	  extern const MKFS_PARM MKFS_Option;

    #endif
  #endif
#endif

//-------------------------------------------------------------------------------------------------
