//-------------------------------------------------------------------------------------------------
//
//  File : diskio_def.h
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

#include "./Digini/inc/lib_typedef.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Command code for disk_ioctrl()
// Generic command
#define CTRL_SYNC                       0       // Mandatory for write functions
#define GET_SECTOR_COUNT                1       // Mandatory for only f_mkfs()
#define GET_SECTOR_SIZE                 2
#define GET_BLOCK_SIZE                  3        // Mandatory for only f_mkfs()
#define CTRL_ERASE_SECTOR               4        // Force erased a block of sectors (for only _USE_ERASE)

// Example of optional ioctl command
#define CTRL_FORMAT						5		// Formatting the drive is done via IO_Ctrl
#define MMC_GET_TYPE                    6        // FatFS definition N/U
#define MMC_GET_CSD                     7        // FatFS definition N/U... use Formatted struct version GET_CSD_STRUCT
#define MMC_GET_CID                     8        // FatFS definition N/U... use Formatted struct version GET_CID_STRUCT
#define MMC_GET_OCR                     9
#define ATA_GET_REV                     10        // FatFS definition N/U
#define ATA_GET_MODEL                   11       // FatFS definition N/U
#define ATA_GET_SN                      12       // FatFS definition N/U .. we use CID

// Custom definition
#define MMC_CID_STRUCT                  20
#define MMC_CSD_STRUCT                  21
#define MMC_GET_SCR                     22
#define GET_CARD_CAPACITY               23

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Patch..
typedef unsigned int	UINT;	        // int must be 16-bit or 32-bit
typedef uint32_t		DWORD;	        // 32-bit unsigned integer
typedef SystemState_e   DSTATUS;        // Status of Disk Functions. They are mapped in Digini errors systems.

// Results of Disk Functions
typedef enum
{
    RES_OK = 0,             // 0: Successful
    RES_ERROR,              // 1: R/W Error
    RES_WRPRT,              // 2: Write Protected
    RES_NOTRDY,             // 3: Not Ready
    RES_PARERR              // 4: Invalid Parameter
} DRESULT;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
struct SPI_Param_t			// SPI device, Flash, Eeprom SD card
{
    SPI_Driver*    pDriver;
    IO_ID_e        IO_ChipSelect;
};
#endif

//-------------------------------------------------------------------------------------------------
