//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_fatfs_spi_flash_disk.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_FATFS_USE_SPI_FLASH_CHIP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const MKFS_PARM FatFS_SPI_FlashDisk::m_MKFS_Option =
{
    /*.fmt     =*/ FM_ANY | FM_SFD,  
    /*.n_fat   =*/ 0,          		// 0 = Auto-select number of FAT tables (usually 2)
    /*.align   =*/ 0,          		// 0 = Auto-align clusters to data area block size
    /*.n_root  =*/ 0,         		// 0 = Auto-select root directory entries (for FAT12/16)
    /*.au_size =*/ 0,        		// 0 = Auto-select cluster size based on disk capacity
};

//-------------------------------------------------------------------------------------------------
//
//   Class: FatFS_SPI_FlashDisk
//
//
//   Description:   Class to handle FatFS for flash disk
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function:      Configure
//
//   Parameter(s):  uint8_t*    pBuffer         Data buffer allocated for flash disk
//                  size_t      Size            Size of the buffer
//
//   Return value:  None
//
//-------------------------------------------------------------------------------------------------
void FatFS_SPI_FlashDisk::Configure(uint8_t* pBuffer, size_t Size)
{
    m_pBuffer        = pBuffer;
    m_Size           = Size;
    m_IsItInitialize = true;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Initialize flash disk
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_SPI_FlashDisk::Initialize(void)
{
    if(m_IsItInitialize == true)
    {
        m_Status = STA_OK;
    }

// do

    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Status
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Get Status from Flash disk device.
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_SPI_FlashDisk::Status(void)
{
    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Read
//
//   Parameter(s):  uint8_t*  pBuffer
//                  uint32_t  Sector
//                  uint16_t   NumberOfSectors
//   Return value:  DRESULT
//
//   Description:   Read From flash disk device.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_SPI_FlashDisk::Read(uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    DRESULT Result;

    if((Result = CheckError(Sector, NumberOfSectors)) == RES_OK)
    {
        memcpy(pBuffer, m_pBuffer + (Sector * FLASH_DISK_SECTOR_SIZE), NumberOfSectors * FLASH_DISK_SECTOR_SIZE);
        return RES_OK;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Write
//
//   Parameter(s):  const uint8_t*  pBuffer
//                  uint32_t        Sector
//                  uint16_t        NumberOfSectors
//   Return value:  DRESULT
//
//   Description:   Write to the flash disk device
//
//-------------------------------------------------------------------------------------------------
#if _USE_WRITE == 1
DRESULT FatFS_SPI_FlashDisk::Write(const uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    DRESULT Result;

    if((Result = CheckError(Sector, NumberOfSectors)) == RES_OK)
    {
        memcpy(m_pBuffer + (Sector * RAM_DISK_SECTOR_SIZE), pBuffer, NumberOfSectors * RAM_DISK_SECTOR_SIZE);
        return RES_OK;
    }

    return Result;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name: IO_Control
//
//   Parameter(s):  uint8_t    Control        Control code
//                  void*      pBuffer        Buffer to send/receive control data
//   Return value:  DRESULT
//
//   Description:   Control FatFs required functions
//
//-------------------------------------------------------------------------------------------------
#if _USE_IOCTL == 1
DRESULT FatFS_SPI_FlashDisk::IO_Ctrl(uint8_t Control, void *pBuffer)
{
    DRESULT res = RES_ERROR;

    if(m_Status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch(Control)
    {
        case CTRL_SYNC:                                                         // Make sure that no pending write process
        {
            res = RES_OK;
        }
		break;

        case GET_SECTOR_COUNT:                                                  // Get number of sectors on the disk (unit32_t)
        {
            *(uint32_t*)pBuffer = pSPI_Flash->GetFlashSize() / FF_MAX_SS;
            res = RES_OK;
        }
		break;

        case GET_SECTOR_SIZE:                                                   // Get R/W sector size (uii16_t)
        {
            *(uint16_t*)pBuffer = FF_MAX_SS;
            res = RES_OK;
        }
		break;

        case GET_BLOCK_SIZE:                                                    // Get erase block size (In Flash this is sector)
        {
            *(uint32_t*)pBuffer = pSPI_Flash->GetSectorEraseSize();
        }
		break;

        case CTRL_FORMAT:
        {
			FATFS fs;
			FRESULT res;
			
			// Allocate the mandatory working buffer for formatting Must be at least FF_MAX_SS
			uint8_t* pSector = (uint8_t*)pMemoryPool->Alloc(FF_MAX_SS);

			// Execute formatting using the 4-parameter API "0:" indicates the logical drive number
			//m_ThisDisk;
			res = f_mkfs("0:", &FatFS_SPI_FlashDisk::m_MKFS_Option, FF_MAX_SS, FF_MAX_SS);

			//if(res == FR_OK)
			//{
				// Mount the drive immediately (1 = Force mount check)
				//res = f_mount(&fs, "0:", 1);
			//}
			
			// Free memory used by f_mkfs
			pMemoryPool->Free((void**)&pSector);
		}
		break;
		
        default:
        {
            res = RES_PARERR;
        }
    }

    return res;
}
#endif


#if 0 

        SystemState_e               EraseSector             (uint32_t SectorAddress);
        SystemState_e               BulkErase               (void);
        SystemState_e               Read                    (void* pBuffer, uint32_t Address, size_t Length);
        SystemState_e               Write                   (const void* pBuffer, uint32_t Address, size_t Length);

        uint32_t                    GetPageSize             (void)          { return m_FlashInfo.PageSize; }
        uint64_t                    GetFlashSize            (void)          { return m_FlashInfo.NumberOfPages * m_FlashInfo.PageSize; }
        uint32_t                    GetSectorEraseSize      (void)          { return m_FlashInfo.SectorEraseSize; }
        uint32_t                    GetSectorSize           (void)          { return m_FlashInfo.SectorSize; }
        uint32_t                    GetFlashID              (void)          { return m_FlashInfo.FlashID; }
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name: CheckError
//
//   Parameter(s):  uint32_t        Sector
//                  uint8_t         NumberOfBlocks
//   Return value:  DRESULT
//
//   Description:   Check for parameter error and flash boundary violation
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_SPI_FlashDisk::CheckError(uint32_t Sector, uint16_t NumberOfSectors)
{
    if(m_Status & (STA_NOINIT | STA_NODISK))
    {
        return RES_NOTRDY;
    }

    if(NumberOfSectors == 0)
    {
       return RES_PARERR;
    }

    if((Sector + ((NumberOfSectors - 1) * FLASH_DISK_SECTOR_SIZE)) >= (m_Size / FLASH_DISK_SECTOR_SIZE))
    {
       return RES_PARERR;
    }

    return RES_OK;
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_FATFS_USE_RAM_DISK


#if 0

// how to
void format_super_floppy(void)
{
    FATFS fs;
    FRESULT res;
    
    // Allocate the mandatory working buffer for formatting
    // Must be at least _MAX_SS (Sector Size, typically 512 or 4096)
    BYTE work_buffer[FF_MAX_SS]; 

    // Initialize the configuration structure
    MKFS_PARM opt;
    
    // Set format type to any valid FAT type (FAT/FAT32/exFAT) AND combine it with the Super Floppy Disk flag (FM_SFD, no MBR)
    opt.fmt = FM_ANY | FM_SFD;  
    
    opt.au_size = 0;        	// 0 = Auto-select cluster size based on disk capacity
    opt.align   = 0;          	// 0 = Auto-align clusters to data area block size
    opt.n_fat   = 0;          	// 0 = Auto-select number of FAT tables (usually 2)
    opt.n_root  = 0;         	// 0 = Auto-select root directory entries (for FAT12/16)

    // Execute formatting using the 4-parameter API "0:" indicates the logical drive number
    res = f_mkfs("0:", &opt, work_buffer, sizeof(work_buffer));

    if(res == FR_OK)
	{
        // Mount the drive immediately (1 = Force mount check)
        res = f_mount(&fs, "0:", 1);
    }
}
#endif