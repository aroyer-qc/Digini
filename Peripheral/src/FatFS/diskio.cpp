//-------------------------------------------------------------------------------------------------
//
//  File : diskio.cpp
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

#define DISKIO_GLOBAL
#include "./Digini/lib_digini.h"
#undef  DISKIO_GLOBAL

#if (DIGINI_USE_FATFS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Local variable(s) or Class
//-------------------------------------------------------------------------------------------------

DiskIO_DeviceInterface* DiskIO::pDiskList[NUMBER_OF_DISK] =
{
    FAT_FS_DRIVE_DEF(EXPAND_X_DRIVE_AS_OBJ_CONST_IN_DISK)
};

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
//void DiskIO::RegisterDisk(DiskMedia_e Disk, DiskIO_DeviceInterface* pDisk)
//{
//    pDiskList[Disk] = pDisk;
//}

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
DSTATUS DiskIO::Initialize(DiskMedia_e Disk)
{
    return pDiskList[Disk]->Initialize();
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
DSTATUS DiskIO::Status(DiskMedia_e Disk)
{
    return pDiskList[Disk]->Status();
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
DRESULT DiskIO::Read(DiskMedia_e Disk, uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
    return pDiskList[Disk]->Read(pBuffer, Sector, Count);
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
#if _USE_WRITE == 1
DRESULT DiskIO::Write(DiskMedia_e Disk, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
    return pDiskList[Disk]->Write(pBuffer, Sector, Count);
}
#endif

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
DRESULT DiskIO::IO_Ctrl(DiskMedia_e Disk, uint8_t Command, void* pBuffer)
{
    return pDiskList[Disk]->IO_Ctrl(Command, pBuffer);
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
//  Parameter(s):   uint8_t        Device
//  Return:         DSTATUS
//
//  Description:    Initialize a device
//
//
//  Note(s):
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
//  Parameter(s):   uint8_t        Disk
//  Return:         DSTATUS
//
//  Description:    Return Status
//
//
//  Note(s):
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
//  Parameter(s):   uint8_t        Disk     Physical device number (0..)
//                  uint8_t*       pBuffer  Data buffer to store read data
//                  uint32_t       Sector   Sector number (LBA)
//                  uint16_t       Count    Sector count (1..65535)
//  Return:         DRESULT
//
//  Description:    Read Sector(s)
//
//  Note(s):
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
//  Parameter(s):   uint8_t        Disk        Physical drive number (0..)
//                  uint8_t*       pBuffer     Data buffer to be written
//                  uint32_t       Sector      Sector number (LBA)
//                  uint16_t       Count       Sector count (1..65535)
//  Return:         DRESULT
//
//  Description:    Write Sector(s)
//
//  Note(s):
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
//  Parameter(s):   uint8_t        Disk        Physical drive number (0..)
//                  uint8_t        Control     Control code
//                  uint8_t*       pBuffer     Buffer to send/receive control data
//  Return:         DRESULT
//
//  Description:    Miscellaneous Functions
//
//  Note(s):
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
//  Return:         uint32_t
//
//  Description:    Coded time stamp
//
//  Note(s):
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


WCHAR ff_convert(WCHAR wch, UINT dir)
{
    if(wch < 0x80)
    {
        /* ASCII Char */
        return wch;
    }

    /* I don't support unicode it is too big! */
    return 0;
}

WCHAR ff_wtoupper(WCHAR wch)
{
    if(wch < 0x80)
    {
        /* ASCII Char */
        if (wch >= 'a' && wch <= 'z')
        {
            wch &= ~0x20;
        }
        return wch;
    }

    /* I don't support unicode it is too big! */
    return 0;
}
//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_FATFS
