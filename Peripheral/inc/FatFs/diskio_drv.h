//-------------------------------------------------------------------------------------------------
//
//  File : diskio_drv.h
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

#include "diskio_drv_cfg.h"                     // This will add any custom driver

//-------------------------------------------------------------------------------------------------

#if DIGINI_FATFS_USE_RAM_DISK == DEF_ENABLED
#include "lib_class_fatfs_ram_disk.h"
#endif

#if DIGINI_FATFS_USE_SDIO_SD_CARD == DEF_ENABLED
#include "lib_class_fatfs_sdio.h"
#endif

#if DIGINI_FATFS_USE_SPI_FLASH_CHIP == DEF_ENABLED
#include "lib_class_fatfs_spi_flash_chip.h"
#endif

#if DIGINI_FATFS_USE_SPI_SD_CARD == DEF_ENABLED
#include "lib_class_fatfs_spi_sd_card.h"
#endif

#if DIGINI_FATFS_USE_USB_KEY == DEF_ENABLED
#include "lib_class_fatfs_usb.h"
#endif

//-------------------------------------------------------------------------------------------------

