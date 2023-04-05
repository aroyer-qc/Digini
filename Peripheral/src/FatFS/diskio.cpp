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
void DiskIO_Device::RegisterDrive(DiskMedia_e Drive, void* pDriveDevice)
{
    pDrivesList[Drive] = pDriveDevice;
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
SystemState_e DiskIO_Device::Initialize(DiskMedia_e Drive);
{
    return pDrivesList[Drive]->Initialize();
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
SystemState_e DiskIO_Device::Status(uint8_t Drive);
{
    return pDrivesList[Drive]->Status();
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
SystemState_e DiskIO_Device::Read(uint8_t  Drive, uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
{
    return pDrivesList[Drive]->Read(pBuffer, Sector, Count);
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
SystemState_e DiskIO_Device::Write(uint8_t Drive, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count);
{
   return pDrivesList[Drive]->Write(pBuffer, Sector, Count);
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
SystemState_e DiskIO_Device::IO_Ctrl(uint8_t Drive, uint8_t Command, void* pBuffer);
{
    return pDrivesList[Drive]->IO_Ctrl(Command, pBuffer);
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
void DiskIO_Device::Sync(uint8_t Drive);
{
    return pDrivesList[Drive]->Sync(Drive);
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
uint32_t DiskIO_Device::GetSectorCount(uint8_t Drive);
{
    return pDrivesList[Drive]->GetSectorCount(Drive);
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
uint32_t DiskIO_Device::GetSectorSize(uint8_t Drive);
{
    return pDrivesList[Drive]->GetSectorSize(Drive);
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
uint32_t DiskIO_Device::GetEraseBlockSize(uint8_t Drive);
{
    return pDrivesList[Drive]->GetEraseBlockSize(Drive);
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
extern "C" DSTATUS disk_initialize(uint8_t Drive)
{
    return DiskIO_Device.getInstance().Initialize(Drive);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_status
//
//  Parameter(s):   uint8_t        Device
//  Return:         DSTATUS
//
//  Description:    Return Status
//
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" DSTATUS disk_status(uint8_t Device)
{
    return DiskIO_Device.getInstance().Status(Drive);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_read
//
//  Parameter(s):   uint8_t        Drive   Physical device number (0..)
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
extern "C" DRESULT disk_read(uint8_t Drive, uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
   return DiskIO_Device.getInstance().Read(Drive, pBuffer, Sector, Count);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_write
//
//  Parameter(s):   uint8_t        Drive       Physical drive number (0..)
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
extern "C" DRESULT disk_write(uint8_t Drive, const uint8_t* pBuffer, uint32_t Sector, uint16_t Count)
{
   return DiskIO_Device.getInstance().Write(Drive, pBuffer, Sector, Count);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           disk_ioctl
//
//  Parameter(s):   uint8_t        Drive       Physical drive number (0..)
//                  uint8_t        Control     Control code
//                  uint8_t*       pBuffer     Buffer to send/receive control data
//  Return:         DRESULT
//
//  Description:    Miscellaneous Functions
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" DRESULT disk_ioctl(uint8_t Drive, uint8_t Control, void* pBuffer)
{
   return DiskIO_Device.getInstance().IO_Ctrl(Drive, pBuffer, Sector, Count);
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
*/}
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
