//-------------------------------------------------------------------------------------------------
//
//  File : lib_digini.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Standard header
//
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Real Time OS

#include "nOS.h"

//-------------------------------------------------------------------------------------------------
// Common configuration header
//
//#include "clock_cfg.h"
#include "digini_cfg.h"
#include "driver_cfg.h"
#include "bsp_io_def.h"
#include "memory_cfg.h"
#include "label_cfg.h"
#include "project_def.h"

//-------------------------------------------------------------------------------------------------
// Stack Check and statistic
//
#include "LIB_stacktistic.h"

//-------------------------------------------------------------------------------------------------
// System Clock
//
#include "system_clock.h"

//-------------------------------------------------------------------------------------------------
// Digini library
//
#include "lib_assert.h"
#include "lib_memory.h"
#include "lib_class_database.h"
#include "lib_class_cbi.h"                      // Callback interface
#include "lib_label.h"
#include "lib_macro.h"
#include "lib_advanced_macro.h"
#include "lib_define.h"
#include "lib_typedef.h"
#include "lib_string.h"
#include "lib_class_queue.h"
#include "lib_class_timer.h"
#include "lib_stacktistic.h"
#include "lib_utility.h"
#include "lib_pid.h"
#include "lib_io.h"
#include "lib_isr.h"
#include "lib_dma.h"
#include "lib_fifo.h"
#include "lib_cpu_specific.h"

#if (DIGINI_USE_CRC == DEF_ENABLED)
#include "lib_crc.h"
#endif

//-------------------------------------------------------------------------------------------------
// Driver
//

//#include "lib_class_driver_interface.h"

#if (USE_ADC_DRIVER == DEF_ENABLED)
#include "lib_class_adc.h"
#endif

#if (USE_CAN_DRIVER == DEF_ENABLED)
#include "lib_class_can.h"
#endif

#if (USE_DAC_DRIVER == DEF_ENABLED)
#include "lib_class_dac.h"
#endif

#if (USE_ETH_DRIVER == DEF_ENABLED)
#include "lib_ethernet_typedef.h"
#include "lib_class_eth.h"
#include "lib_class_ethernet_phy_interface.h"
#include PHY_DRIVER_INCLUDE
#endif

#if (USE_I2C_DRIVER == DEF_ENABLED)
#include "lib_class_i2c.h"
#endif

#if (USE_I2S_DRIVER == DEF_ENABLED)
#include "lib_class_i2s.h"
#endif

#if (USE_QSPI_DRIVER == DEF_ENABLED)
#include "lib_class_qspi.h"
#endif

#if (USE_RTC_DRIVER == DEF_ENABLED)
#include "lib_class_rtc.h"
#endif

#if (USE_SAI_DRIVER == DEF_ENABLED)
#include "lib_class_sai.h"
#endif

#if (USE_SDIO_DRIVER == DEF_ENABLED)
#include "lib_class_sdio.h"
#endif

#if (USE_SPI_DRIVER == DEF_ENABLED)
#include "lib_class_spi.h"
#endif

#if (USE_TIM_DRIVER == DEF_ENABLED)
#include "lib_class_tim.h"
#endif

#if (USE_PWM_DRIVER == DEF_ENABLED)
#include "lib_class_pwm.h"
#endif

#if (USE_UART_DRIVER == DEF_ENABLED)
#include "lib_class_uart.h"
#endif

#if (USE_USB_DRIVER == DEF_ENABLED)
#include "lib_class_usb.h"
#endif

//-------------------------------------------------------------------------------------------------
// Digini included functionality
//

#include "lib_console.h"
#include "lib_cli.h"
#include "lib_vt100.h"

//-------------------------------------------------------------------------------------------------
// High level Peripheral
//
// Note(s) All specific driver or high level device driver must be include in device_cfg.h

#include "device_cfg.h"

#if (DIGINI_USE_FATFS == DEF_ENABLED)

#include "diskio_def.h"
#include "diskio_interface.h"

#if (DIGINI_FATFS_USE_RAM_DISK == DEF_ENABLED)
#include "lib_class_fatfs_ram_disk.h"
#endif

#if (DIGINI_FATFS_USE_SDIO_SD_CARD == DEF_ENABLED)
#include "lib_class_fatfs_sdio.h"
#endif

#if (DIGINI_FATFS_USE_SPI_FLASH_CHIP == DEF_ENABLED)
#include "lib_class_fatfs_spi_flash_chip.h"
#endif

#if (DIGINI_FATFS_USE_SPI_SD_CARD == DEF_ENABLED)
#include "lib_class_fatfs_spi_sd_card.h"
#endif

#if (DIGINI_FATFS_USE_USB_KEY == DEF_ENABLED)
#include "lib_class_fatfs_usb.h"
#endif

#include "digini_diskio.h"
#include "diskio_drv_cfg.h"                     // This will add any custom driver

#endif

#if (DIGINI_USE_GRAFX == DEF_ENABLED)
#include "lib_grafx.h"
#endif

//-------------------------------------------------------------------------------------------------
// Functions prototypes
//-------------------------------------------------------------------------------------------------

SystemState_e       DIGINI_Initialize       (void);
SystemState_e       DIGINI_PostInitialize   (void);

//-------------------------------------------------------------------------------------------------
