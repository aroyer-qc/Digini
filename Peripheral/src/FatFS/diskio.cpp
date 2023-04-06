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
#include "lib_digini.h"
#undef  DISKIO_GLOBAL
#ifdef DIGINI_USE_FATFS

//-------------------------------------------------------------------------------------------------
// Local variable(s) or Classe(s)
//-------------------------------------------------------------------------------------------------

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
void DiskIO::RegisterDisk(DiskMedia_e Disk, DiskIO_DeviceInterface* pDisk)
{
    pDiskList[Disk] = pDisk;
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



// Initialize
/*
  #ifdef DIGINI_FATFS_USE_SPI_FLASH_CHIP
    pSPI_Flash    = new CFatFS_SPI_Flash();
  #endif

  #ifdef DIGINI_FATFS_USE_SDIO_SD_CARD
    pSDIO         = new SDIO_Driver();
  #endif

  #ifdef DIGINI_FATFS_USE_SPI_SD_CARD
    pSPI_SD_Card  = new CFatFS_SD_Card(&SPI);
  #endif

  #ifdef DIGINI_FATFS_USE_USB_KEY
    pUSB_Key      = new CFatFS_USB();
  #endif

  #ifdef DIGINI_FATFS_USE_RAM_DRIVE
    pRAM_Drive    = new CFatFS_RAM_Drive());
  #endif
*/
/*
this is removed to be placed in their own class
    DSTATUS Status;

    Status = STA_NOINIT;

    switch((DiskMedia_e)Device)
    {
      #ifdef DIGINI_FATFS_USE_SPI_FLASH_CHIP
        case DISK_FLASH:
        {
            Status = pSPI_Flash->Initialize();          // SPI Flash
            break;
        }
      #endif // DIGINI_FATFS_USE_SPI_FLASH_CHIP

      #ifdef DIGINI_FATFS_USE_SDIO_SD_CARD
        case DISK_SDIO_SD_CARD:
        {
            Status = pSDIO->FatFS_Initialize();         // SDIO SD Card
            break;
        }
      #endif // DIGINI_FATFS_USE_SDIO_SD_CARD

      #ifdef DIGINI_FATFS_USE_SPI_SD_CARD
        case DISK_SPI_SD_CARD:
        {
            Status = pSPI_SD_Card->Initialize();          // SPI SD Card
            break;
        }
      #endif // DIGINI_FATFS_USE_SPI_SD_CARD

      #ifdef DIGINI_FATFS_USE_USB_KEY
        case DISK_USB_KEY:
        {
            Status = pUSB_Key->Initialize();              // USB Flash key
            break;
        }
      #endif // DIGINI_FATFS_USE_USB_KEY

      #ifdef DIGINI_FATFS_USE_RAM_DRIVE
        case DISK_RAM:
        {
            Status = BSP_pRAM_Drive->Initialize();      // Ram Drive
            break;
        }
      #endif // DIGINI_FATFS_USE_RAM_DRIVE

      default: break;
    }
*/
// disk read
/*
this is removed to be placed in their own class
    switch(Drive)
    {
      #ifdef DIGINI_FATFS_USE_SPI_FLASH_CHIP
        case DISK_FLASH:
            Result = pSPI_Flash->Read(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_SPI_FLASH_CHIP

      #ifdef DIGINI_FATFS_USE_SDIO_SD_CARD
        case DISK_SDIO_SD_CARD:
            Result = pSDIO->FatFS_Read(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_SDIO_SD_CARD

      #ifdef DIGINI_FATFS_USE_SPI_SD_CARD
        case DISK_SPI_SD_CARD:
            Result = pSPI_SD_Card->Read(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_SPI_SD_CARD

      #ifdef DIGINI_FATFS_USE_USB_KEY
        case DISK_USB_KEY:
            Result = pUSB_Key->Read(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_USB_KEY

      #ifdef DIGINI_FATFS_USE_RAM_DRIVE
        case DISK_RAM:
            Result = BSP_pRAM_Drive->Read(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_RAM_DRIVE
    }
*/
// Status
/*
this is removed to be placed in their own class
    DSTATUS Status;

    switch(Device)
    {
      #ifdef DIGINI_FATFS_USE_SPI_FLASH_CHIP
        case DISK_FLASH:
            Status = pSPI_Flash->Status();
            return Status;
      #endif // DIGINI_FATFS_USE_SPI_FLASH_CHIP

      #ifdef DIGINI_FATFS_USE_SDIO_SD_CARD
        case DISK_SDIO_SD_CARD:
            Status = pSDIO->FatFS_DiskStatus();
            return Status;
      #endif // DIGINI_FATFS_USE_SDIO_SD_CARD

      #ifdef DIGINI_FATFS_USE_SPI_SD_CARD
        case DISK_SPI_SD_CARD:
            Status = pSPI_SD_Card->Status();
            return Status;
      #endif // DIGINI_FATFS_USE_SPI_SD_CARD

      #ifdef DIGINI_FATFS_USE_USB_KEY
        case DISK_USB_KEY:
            Status = pUSB_Key->Status();
            return Status;
      #endif // DIGINI_FATFS_USE_USB_KEY

      #ifdef DIGINI_FATFS_USE_RAM_DRIVE
        case DISK_RAM:
            Status = BSP_pRAM_Drive->Status();
            return Status;
      #endif // DIGINI_FATFS_USE_RAM_DRIVE
    }
    return STA_NOINIT;
*/
// Write

/*
this is removed to be placed in their own class
    DRESULT Result;

    switch(Device)
    {
      #ifdef DIGINI_FATFS_USE_SPI_FLASH_CHIP
        case DISK_FLASH:
            Result = pSPI_Flash->Write(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_SPI_FLASH_CHIP

      #ifdef DIGINI_FATFS_USE_SDIO_SD_CARD
        case DISK_SDIO_SD_CARD:
            Result = pSDIO->FatFS_Write(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_SDIO_SD_CARD

      #ifdef DIGINI_FATFS_USE_SPI_SD_CARD
        case DISK_SPI_SD_CARD:
            Result = pSPI_SD_Card->Write(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_SPI_SD_CARD

      #ifdef DIGINI_FATFS_USE_USB_KEY
        case DISK_USB_KEY:
            Result = pUSB_Key->Write(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_USB_KEY

      #ifdef DIGINI_FATFS_USE_RAM_DRIVE
        case DISK_RAM:
            Result = BSP_pRAM_Drive->Write(pBuffer, Sector, Count);
            return Result;
      #endif // DIGINI_FATFS_USE_RAM_DRIVE
    }
    return RES_PARERR;
*/
// io_ctrl
/*
this is removed to be placed in their own class
    DRESULT Result;

    switch(Device)
    {
      #ifdef DIGINI_FATFS_USE_SPI_FLASH_CHIP
        case DISK_FLASH:
            Result = pSPI_Flash->IO_Control(Control, pBuffer);
            return Result;
      #endif // DIGINI_FATFS_USE_SPI_FLASH_CHIP

      #ifdef DIGINI_FATFS_USE_SDIO_SD_CARD
        case DISK_SDIO_SD_CARD:
            Result = pSDIO->FatFS_IO_Control(Control, pBuffer);
            return Result;
      #endif // DIGINI_FATFS_USE_SDIO_SD_CARD

      #ifdef DIGINI_FATFS_USE_SPI_SD_CARD
        case DISK_SPI_SD_CARD:
            Result = pSPI_SD_Card->IO_Control(Control, pBuffer);
            return Result;
      #endif // DIGINI_FATFS_USE_SPI_SD_CARD

      #ifdef DIGINI_FATFS_USE_USB_KEY
        case DISK_USB_KEY:
            Result = pUSB_Key->IO_Control(Control, pBuffer);
            return Result;
      #endif // DIGINI_FATFS_USE_USB_KEY

      #ifdef DIGINI_FATFS_USE_RAM_DRIVE
        case DISK_RAM:
            Result = BSP_pRAM_Drive->IO_Control(Control, pBuffer);
            return Result;
      #endif // DIGINI_FATFS_USE_RAM_DRIVE
    }
    return RES_PARERR;
*/
