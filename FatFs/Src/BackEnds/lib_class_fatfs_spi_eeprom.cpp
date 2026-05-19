//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_fatfs_spi_eeprom.cpp
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

#if (DIGINI_FATFS_USE_SPI_EEPROM == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Class: FatFS_SPI_EEprom
//
//
//   Description:   Class to handle FatFS for eeprom disk
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Initialize EEprom disk
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_SPI_EEprom::Initialize(void* pParameter)
{
    SPI_Param_t* pParam = ((SPI_Param_t*)pParameter);
    m_SPI_EEprom.Initialize(pParam->pDriver, pParam->IO_ChipSelect);

    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Status
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Get Status from EEprom disk device.
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_SPI_EEprom::Status(void)
{
    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Read
//
//   Parameter(s):  uint8_t*    pBuffer
//                  uint32_t    Sector
//                  uint16_t    NumberOfSectors
//   Return value:  DRESULT
//
//   Description:   Read From EEprom disk device.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_SPI_EEprom::Read(uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    uint32_t Address = Sector * FF_MAX_SS;
    size_t   Length  = (size_t)NumberOfSectors * FF_MAX_SS;
    SystemState_e State = m_SPI_EEprom.Read(pBuffer, Address, Length);

    if(State == SYS_READY)
    {
        return RES_OK;
    }

    return RES_ERROR;
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
//   Description:   Write to the EEprom disk device
//
//-------------------------------------------------------------------------------------------------
#if _USE_WRITE == 1
DRESULT FatFS_SPI_EEprom::Write(const uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    uint32_t Address = Sector * FF_MAX_SS;
    size_t   Length  = (size_t)NumberOfSectors * FF_MAX_SS;

    // Low-level flash write
    SystemState_e State = m_SPI_EEprom.Write(pBuffer, Address, Length);

    if(State == SYS_READY)
    {
        return RES_OK;
    }

    return RES_ERROR;
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
DRESULT FatFS_SPI_EEprom::IO_Ctrl(uint8_t Control, void *pBuffer)
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
            *(uint32_t*)pBuffer = m_SPI_EEprom.GetFlashSize() / FF_MAX_SS;
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
           *(uint32_t*)pBuffer = m_SPI_EEprom.GetSectorEraseSize()/ FF_MAX_SS;
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

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_FATFS_USE_SPI_EEPROM
