//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_fatfs_ram_disk.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_FATFS_USE_RAM_DISK == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// User can overrides those value if bigger RAM Disk are required

#ifndef RAM_DISK_SECTOR_SIZE
#define RAM_DISK_SECTOR_SIZE         512                 // Sector size is fixed at 512
#endif

#ifndef RAM_DISK_BLOCK_SIZE
#define RAM_DISK_BLOCK_SIZE          512                 // BlockSize size is fixed at 512
#endif

//-------------------------------------------------------------------------------------------------
//
//   Class: CFatFS_RAM_Disk
//
//
//   Description:   Class to handle FatFS for RAM Disk
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   CFatFS_RAM_Disk
//
//   Parameter(s):
//   Return Value:
//
//   Description:   Initializes the RAM Disk volatile peripheral
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
CFatFS_RAM_Disk::CFatFS_RAM_Disk()
{
    m_IsItInitialize = false;
    m_Status         = STA_NODISK;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Configure
//
//   Parameter(s):  uint8_t*    pBuffer         Data buffer allocated for RAM Disk
//                  size_t      Size            Size of the buffer
//   Return value:  None
//
//   Description:   Initialize RAM Disk
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void CFatFS_RAM_Disk::Configure(uint8_t* pBuffer, size_t Size)
{
    m_pBuffer        = pBuffer;
    m_Size           = Size;
    m_Status         = STA_NOINIT;
    m_IsItInitialize = true;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Initialize RAM Disk
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DSTATUS CFatFS_RAM_Disk::Initialize(void)
{
    if(m_IsItInitialize == true)
    {
        m_Status = STA_OK;
    }

    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Status
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Get Status from RAM Disk Device
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DSTATUS CFatFS_RAM_Disk::Status()
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
//   Description:   Read From RAM Disk Device
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT CFatFS_RAM_Disk::Read(uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    DRESULT Result;

    if((Result = CheckError(Sector, NumberOfSectors)) == RES_OK)
    {
        memcpy(pBuffer, m_pBuffer + (Sector * RAM_DISK_SECTOR_SIZE), NumberOfSectors * RAM_DISK_SECTOR_SIZE);
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
//   Description:   Write to the RAM Disk Device
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
#if _USE_WRITE == 1
DRESULT CFatFS_RAM_Disk::Write(const uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
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
//   Description:   Control
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
#if _USE_IOCTL == 1
DRESULT CFatFS_RAM_Disk::IO_Ctrl(uint8_t Control, void *pBuffer)
{
    DRESULT res = RES_ERROR;

    if(m_Status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch(Control)
    {
        case CTRL_SYNC:                                                         // Make sure that no pending write process, Impossible in RAM :)
        {
            res = RES_OK;
            break;
        }

        case GET_SECTOR_COUNT:                                                  // Get number of sectors on the disk (unit32_t)
        {
            *(uint32_t*)pBuffer = uint32_t(m_Size / RAM_DISK_SECTOR_SIZE);
            res = RES_OK;
            break;
        }

        case GET_SECTOR_SIZE:                                                   // Get R/W sector size (unit16_t)
        {
            *(uint16_t*)pBuffer = RAM_DISK_SECTOR_SIZE;
            res = RES_OK;
            break;
        }

        case GET_BLOCK_SIZE:                                                    // Get erase block size in unit of sector (unit32_t)
        {
            *(uint32_t*)pBuffer = RAM_DISK_BLOCK_SIZE / RAM_DISK_BLOCK_SIZE;
            break;
        }


        default:
        {
            res = RES_PARERR;
        }
    }

    return res;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name: CheckError
//
//   Parameter(s):  uint32_t        Sector
//                  uint8_t         NumberOfBlocks
//   Return value:  DRESULT
//
//   Description:   Check for parameter error and RAM boundary violation
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT CFatFS_RAM_Disk::CheckError(uint32_t Sector, uint16_t NumberOfSectors)
{
    if(m_Status & (STA_NOINIT | STA_NODISK))
    {
        return RES_NOTRDY;
    }

    if(NumberOfSectors == 0)
    {
       return RES_PARERR;
    }

    if((Sector + ((NumberOfSectors - 1) * RAM_DISK_SECTOR_SIZE)) >= (m_Size / RAM_DISK_SECTOR_SIZE))
    {
       return RES_PARERR;
    }

    return RES_OK;
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_FATFS_USE_RAM_DISK


