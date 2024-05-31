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

#include "./Digini/Digini/inc/lib_typedef.h"
#include "FatFs_cfg.h"

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
#define MMC_GET_TYPE                    5        // FatFS definition N/U
#define MMC_GET_CSD                     6        // FatFS definition N/U... use Formatted struct version GET_CSD_STRUCT
#define MMC_GET_CID                     7        // FatFS definition N/U... use Formatted struct version GET_CID_STRUCT
#define MMC_GET_OCR                     8
#define ATA_GET_REV                     9        // FatFS definition N/U
#define ATA_GET_MODEL                   10       // FatFS definition N/U
#define ATA_GET_SN                      11       // FatFS definition N/U .. we use CID

// Custom definition
#define GET_CID_STRUCT                  20
#define GET_CSD_STRUCT                  21
#define GET_SCR_STRUCT                  22
#define GET_CARD_CAPACITY               23

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Status of Disk Functions
// They are mapped in Digini errors systems.
//typedef SystemState_e DSTATUS;

// Results of Disk Functions
typedef enum
{
    RES_OK = 0,             // 0: Successful
    RES_ERROR,              // 1: R/W Error
    RES_WRPRT,              // 2: Write Protected
    RES_NOTRDY,             // 3: Not Ready
    RES_PARERR              // 4: Invalid Parameter
} DRESULT;

typedef SystemState_e DSTATUS;

#ifdef __cplusplus
}
#endif

//-------------------------------------------------------------------------------------------------
