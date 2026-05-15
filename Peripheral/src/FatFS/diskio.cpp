//-------------------------------------------------------------------------------------------------
//
//  File : diskio.cpp
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

#define DISKIO_GLOBAL
#include "./lib_digini.h"
#undef  DISKIO_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_FATFS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Expanding Macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_DRIVE_AS_DISK_OBJ(ENUM_ID, CLASS_, DISK_OBJ, ARG)  &DiskIO::GetInstance().DISK_OBJ,
#define EXPAND_X_DRIVE_AS_DISK_ARG(ENUM_ID, CLASS_, DISK_OBJ, ARG)  ARG,

//-------------------------------------------------------------------------------------------------
// Local variable(s) or Class
//-------------------------------------------------------------------------------------------------

DiskIO_DeviceInterface* DiskIO::pDiskList[NUMBER_OF_DISK] =
{
    FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_DISK_OBJ)
};

void* DiskIO::pParameterList[NUMBER_OF_DISK] =
{
    FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_DISK_ARG)
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           DiskIO::Initialize
//
//  Parameter(s):   DiskMedia_e Disk        - Media identifier (0..NUMBER_OF_DISK-1)
//
//  Return:         DSTATUS                 - STA_OK if initialized, STA_NOINIT otherwise
//
//  Description:    Initializes the specified media by calling the low-level driver
//                  registered in pDiskList[Disk]. This function is invoked by FatFS
//                  through disk_initialize().
//
//  Note(s):        The concrete driver (USB, SD, RAM, etc.) must implement Initialize().
//
//-------------------------------------------------------------------------------------------------
DSTATUS DiskIO::Initialize(DiskMedia_e Disk)
{
    return pDiskList[Disk]->Initialize(DiskIO::pParameterList[Disk]);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DiskIO::Status
//
//  Parameter(s):   DiskMedia_e Disk        - Media identifier
//
//  Return:         DSTATUS                  - Media status (STA_OK, STA_NOINIT, STA_PROTECT)
//
//  Description:    Returns the current status of the media through the low-level driver.
//
//  Note(s):        Used by FatFS to validate media state before read/write operations.
//
//-------------------------------------------------------------------------------------------------
DSTATUS DiskIO::Status(DiskMedia_e Disk)
{
    return DiskIO::pDiskList[Disk]->Status();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DiskIO::Read
//
//  Parameter(s):   DiskMedia_e Disk     - Media identifier
//                  uint8_t* pBuffer     - Destination buffer
//                  uint32_t Sector      - Sector number (LBA)
//                  uint16_t Count       - Number of sectors to read
//
//  Return:         DRESULT              - RES_OK on success, RES_ERROR otherwise
//
//  Description:    Reads one or more sectors from the media. Calls the Read() method
//                  of the associated low-level driver.
//
//  Note(s):        FatFS later converts DRESULT into FRESULT.
//
//-------------------------------------------------------------------------------------------------
DRESULT DiskIO::Read(DiskMedia_e Disk, uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
    return DiskIO::pDiskList[Disk]->Read(pBuffer, Sector, Count);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DiskIO::Write
//
//  Parameter(s):   DiskMedia_e Disk       - Media identifier
//                  const uint8_t* pBuffer - Data to write
//                  uint32_t Sector        - Sector number (LBA)
//                  uint16_t Count         - Number of sectors to write
//
//  Return:         DRESULT                - RES_OK on success, RES_ERROR otherwise
//
//  Description:    Writes one or more sectors to the media. Calls the Write() method
//                  of the associated low-level driver.
//
//  Note(s):        Available only when _USE_WRITE == 1.
//
//-------------------------------------------------------------------------------------------------
#if _USE_WRITE == 1
DRESULT DiskIO::Write(DiskMedia_e Disk, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
    return DiskIO::pDiskList[Disk]->Write(pBuffer, Sector, Count);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           DiskIO::IO_Ctrl
//
//  Parameter(s):   DiskMedia_e Disk     - Media identifier
//                  uint8_t Command      - FatFS control command (CTRL_SYNC, GET_SECTOR_COUNT, etc.)
//                  void* pBuffer        - Optional buffer for command data
//
//  Return:         DRESULT              - RES_OK on success, RES_PARERR on invalid command
//
//  Description:    Control interface for special operations required by FatFS.
//                  Redirects to the IO_Ctrl() method of the low-level driver.
//
//  Note(s):        Required when _USE_IOCTL == 1.
//
//-------------------------------------------------------------------------------------------------
DRESULT DiskIO::IO_Ctrl(DiskMedia_e Disk, uint8_t Command, void* pBuffer)
{
    if(Command == CTRL_FORMAT)
    {
        // Working buffer (FatFS need at least FF_MAX_SS bytes
        uint8_t* pSector = (uint8_t*)pMemoryPool->Alloc(FF_MAX_SS);

        if(pSector == nullptr)
        {
            return RES_ERROR;
        }

        // Build drive string: "0:", "1:", etc.
        char DriveStr[3];
        DriveStr[0] = '0' + uint8_t(Disk);
        DriveStr[1] = ':';
        DriveStr[2] = '\0';

        // Format
        FRESULT Result = f_mkfs(DriveStr, &MKFS_Option, pSector, FF_MAX_SS);

        // Free working buffer
        pMemoryPool->Free((void**)&pSector);

        if(Result == FR_OK)
        {
            return RES_OK;
        }

        return RES_ERROR;
    }

    return DiskIO::pDiskList[Disk]->IO_Ctrl(Command, pBuffer);
}

#if 0
//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void DiskIO::Sync(DiskMedia_e Disk)
{
    return pDiskList[Disk]->Sync();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t DiskIO::GetSectorCount(DiskMedia_e Disk)
{
    return pDiskList[Disk]->GetSectorCount();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t DiskIO::GetSectorSize(DiskMedia_e Disk)
{
    return pDiskList[Disk]->GetSectorSize();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):
//  Return:
//
//  Description:
//
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t DiskIO::GetEraseBlockSize(DiskMedia_e Disk)
{
    return pDiskList[Disk]->GetEraseBlockSize();
}
#endif

//-------------------------------------------------------------------------------------------------
// Global function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_initialize
//
//  Parameter(s):   uint8_t Disk         - Physical media number (0..)
//
//  Return:         DSTATUS              - Media status after initialization
//
//  Description:    C wrapper called by FatFS. Redirects to DiskIO::Initialize().
//
//  Note(s):        Allows FatFS (pure C) to call C++ code.
//
//-------------------------------------------------------------------------------------------------
extern "C" DSTATUS disk_initialize(uint8_t Disk)
{
    return FatFS_DiskIO.GetInstance().Initialize(DiskMedia_e(Disk));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_status
//
//  Parameter(s):   uint8_t Disk         - Physical media number
//
//  Return:         DSTATUS              - Current media status
//
//  Description:    C wrapper for DiskIO::Status().
//
//  Note(s):        Used by FatFS to check media readiness.
//
//-------------------------------------------------------------------------------------------------
extern "C" DSTATUS disk_status(uint8_t Disk)
{
    return FatFS_DiskIO.GetInstance().Status(DiskMedia_e(Disk));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_read
//
//  Parameter(s):   uint8_t Disk         - Physical media number
//                  uint8_t* pBuffer     - Destination buffer
//                  uint32_t Sector      - Sector number (LBA)
//                  uint16_t Count       - Number of sectors to read
//
//  Return:         DRESULT              - Low-level result code
//
//  Description:    C wrapper for DiskIO::Read().
//
//  Note(s):        FatFS converts DRESULT into FRESULT.
//
//-------------------------------------------------------------------------------------------------
extern "C" DRESULT disk_read(uint8_t Disk, uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
   return FatFS_DiskIO.GetInstance().Read(DiskMedia_e(Disk), pBuffer, Sector, Count);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_write
//
//  Parameter(s):   uint8_t Disk           - Physical media number
//                  const uint8_t* pBuffer - Data to write
//                  uint32_t Sector        - Sector number (LBA)
//                  uint16_t Count         - Number of sectors to write
//
//  Return:         DRESULT                - Low-level result code
//
//  Description:    C wrapper for DiskIO::Write().
//
//  Note(s):        Available only when _USE_WRITE == 1.
//
//-------------------------------------------------------------------------------------------------
#if _USE_WRITE == 1
extern "C" DRESULT disk_write(uint8_t Disk, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
   return FatFS_DiskIO.GetInstance().Write(DiskMedia_e(Disk), pBuffer, Sector, Count);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_ioctl
//
//  Parameter(s):   uint8_t Disk         - Physical drive number (0..)
//                  uint8_t Control      - Control code (CTRL_SYNC, GET_SECTOR_COUNT, etc.)
//                  void* pBuffer        - Optional buffer for control data
//
//  Return:         DRESULT              - RES_OK on success, RES_PARERR or RES_ERROR otherwise
//
//  Description:    Dispatches miscellaneous control operations requested by FatFS.
//                  Redirects the command to DiskIO::IO_Ctrl() for the selected media.
//
//  Note(s):        Required when _USE_IOCTL == 1. FatFS uses this for non-read/write
//                  operations such as synchronization, geometry queries, and device info.
//
//-------------------------------------------------------------------------------------------------
extern "C" DRESULT disk_ioctl(uint8_t Disk, uint8_t Control, void* pBuffer)
{
   return FatFS_DiskIO.GetInstance().IO_Ctrl(DiskMedia_e(Disk), Control, pBuffer);
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           get_fattime
//
//  Parameter(s):   None
//
//  Return:         uint32_t              - Encoded FAT timestamp
//
//  Description:    Returns the current timestamp encoded in FAT format.
//                  The FAT timestamp packs year, month, day, hour, minute, and second/2
//                  into a 32-bit value.
//
//  Note(s):        This implementation currently returns 0 (no timestamp).
//                  Should be linked to a real-time clock for proper file time stamping.
//
//-------------------------------------------------------------------------------------------------
extern "C" uint32_t get_fattime()
{
    uint32_t time;

  /*
    time = ((uint32_t(34) << 25) |
            (uint32_t(4)  << 21) |
            (uint32_t(25) << 16) |
            (uint32_t(7)  << 11));

    // TO DO link to real time clock

*/ time = 0;

    return time;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ff_convert
//
//  Parameter(s):   WCHAR wch     - Input character
//                  UINT dir      - Conversion direction (0: Unicode -> OEM, 1: OEM -> Unicode)
//
//  Return:         WCHAR         - Converted character or 0 if unsupported
//
//  Description:    Character conversion function required by FatFS when Unicode support
//                  is enabled. This implementation only supports ASCII characters.
//
//  Note(s):        Unicode is not supported here due to memory constraints.
//
//-------------------------------------------------------------------------------------------------
WCHAR ff_convert(WCHAR wch, UINT dir)
{
    if(wch < 0x80)
    {
        // ASCII Char
        return wch;
    }

    // No support for unicode it is too big!
    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ff_wtoupper
//
//  Parameter(s):   WCHAR wch     - Input character
//
//  Return:         WCHAR         - Uppercase version of the character
//
//  Description:    Converts a character to uppercase for filename comparison.
//                  Only ASCII characters are supported.
//
//  Note(s):        Unicode uppercase conversion is not supported.
//
//-------------------------------------------------------------------------------------------------
WCHAR ff_wtoupper(WCHAR wch)
{
    if(wch < 0x80)
    {
        // ASCII Char
        if (wch >= 'a' && wch <= 'z')
        {
            wch &= ~0x20;
        }
        return wch;
    }

    // No support for unicode as it is too big!
    return 0;
}
//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_FATFS
