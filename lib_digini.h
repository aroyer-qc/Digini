//-------------------------------------------------------------------------------------------------
//
//  File : lib_digini.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DIGINI_VERSION              "V2.01"

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

#include "./Digini/inc/lib_define.h"
#include "./Digini/inc/lib_macro.h"
#include "./Digini/inc/lib_advanced_macro.h"
#include "./Digini/inc/lib_typedef.h"

//-------------------------------------------------------------------------------------------------
// Real Time OS

#include "nOS.h"

//-------------------------------------------------------------------------------------------------
// Common configuration header
//

#include "project_def.h"

//#include "clock_cfg.h"
#include "digini_cfg.h"

// Need to integrate a dependency check for all DEF_ENABLED

#include "driver_cfg.h"
#include "bsp_io_def.h"

#if (DIGINI_USE_CONSOLE == DEF_ENABLED)
#include "console_cfg.h"
#endif

#if (DIGINI_USE_CRC == DEF_ENABLED)
#include "crc_cfg.h"
#endif

#if (DIGINI_USE_STATIC_MEMORY_ALLOC == DEF_ENABLED)
#include "memory_cfg.h"
#endif

 #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
#include "vt100_cfg.h"
 #endif
 #if (DIGINI_USE_CMD_LINE == DEF_ENABLED)
#include "cli_cfg.h"
 #endif

#if (DIGINI_USE_LABEL == DEF_ENABLED)
#include "label_cfg.h"
#endif

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)
#include "ip_cfg.h"
#endif

//#include "project_def.h"

#if (DIGINI_USE_DATABASE == DEF_ENABLED)
#include "database_cfg.h"
#endif

//-------------------------------------------------------------------------------------------------
// Stack check and statistic
//
#include "./Utility/inc/LIB_stacktistic.h"

//-------------------------------------------------------------------------------------------------
// System Clock
//
#include "./Peripheral/inc/port/lib_system_clock.h"

//-------------------------------------------------------------------------------------------------
// Digini library
//
#include "./Peripheral/inc/port/lib_cpu_specific.h"
#include "./Digini/inc/lib_assert.h"
#include "./Database/inc/lib_class_database.h"
#include "./Digini/inc/lib_class_cbi.h"                      // Callback interface
#include "./Digini/inc/lib_label.h"
#include "./String/inc/lib_string.h"
#include "./RTOS_Wrapper/inc/nOS/lib_class_queue.h"
#include "./RTOS_Wrapper/inc/nOS/lib_class_timer.h"
#include "./Utility/inc/lib_utility.h"
#include "./Peripheral/inc/port/lib_isr.h"
#include "./Utility/inc/lib_pid.h"
#include "./Utility/inc/lib_sunset.h"
#include "./Peripheral/inc/port/lib_io.h"
#include "./Utility/inc/lib_fifo.h"
#include "./Peripheral/inc/port/lib_class_dma.h"
#include "./Peripheral/inc/port/lib_dma.h"                  // todo remove when DMA porting is done
#include "./Peripheral/inc/port/lib_rng.h"
#include "./NanoIP/inc/lib_ethernet_typedef.h"
#include "./Utility/inc/lib_crc.h"
#include "./Memory/inc/lib_memory.h"
#include "./Memory/inc/lib_node_list.h"
#include "./Memory/inc/lib_memory_node.h"

#if (DIGINI_MPU_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_mpu.h"                  // TODO 
#endif

//-------------------------------------------------------------------------------------------------
// Interface
//
//#include "./Peripheral/inc/port/interface/lib_class_driver_interface.h"


//-------------------------------------------------------------------------------------------------
// Driver
//

#if (USE_ADC_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_adc.h"
#endif

#if (USE_CAN_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_can.h"
#endif

#if (USE_CRC_DRIVER == DEF_ENABLED)                         // This is for hardware CRC calculation support
#include "./Peripheral/inc/port/lib_class_hardware_crc.h"
#endif

#if (USE_DAC_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_dac.h"
#endif

#if (USE_ETH_DRIVER == DEF_ENABLED) && (DIGINI_USE_ETHERNET == DEF_ENABLED)
#include "./NanoIP/inc/interface/lib_class_ethernet_interface.h"
#include "./Peripheral/inc/port/lib_class_eth.h"
#include PHY_DRIVER_INCLUDE
#endif

#if (USE_I2C_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_i2c.h"
#endif

#if (USE_I2S_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_i2s.h"
#endif

#if (USE_QSPI_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_qspi.h"
#endif

#if (USE_RTC_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_rtc.h"
#endif

#if (USE_SAI_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_sai.h"
#endif

#if (USE_SDIO_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_sdio.h"
#endif

#if (USE_SPI_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_spi.h"
#endif

#if (USE_TIM_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_tim.h"
#endif

#if (USE_PWM_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_pwm.h"
#endif

#if (USE_UART_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_uart.h"
#endif

#if (USE_USB_DRIVER == DEF_ENABLED)
#include "./Peripheral/inc/port/lib_class_usb.h"
#endif

//-------------------------------------------------------------------------------------------------
// Digini included functionality
//

#include "./Comm/Console/inc/lib_console.h"
#include "./Comm/CmdLine/inc/lib_cli.h"
#include "./Comm/VT100/inc/lib_vt100.h"
#include "./Comm/Comm/inc/lib_comm.h"
#include "./NanoIP/inc/lib_class_ethernetif.h"
#include "./NanoIP/inc/lib_Class_IP_Manager.h"

//-------------------------------------------------------------------------------------------------
// High level Peripheral
//
// Note(s) All specific driver or high level device driver must be include in device_cfg.h

#include "device_cfg.h"

#if (DIGINI_USE_FATFS == DEF_ENABLED)

#include "./Peripheral/inc/FatFs/diskio_def.h"
#include "./Peripheral/inc/FatFs/diskio_interface.h"

#if (DIGINI_FATFS_USE_RAM_DISK == DEF_ENABLED)
#include "./Peripheral/inc/FatFs/lib_class_fatfs_ram_disk.h"
#endif

#if (DIGINI_FATFS_USE_SDIO_SD_CARD == DEF_ENABLED)
#include "./Peripheral/inc/port/FatFs/lib_class_fatfs_sdio.h"
#endif

#if (DIGINI_FATFS_USE_SPI_FLASH_CHIP == DEF_ENABLED)
#include "./Peripheral/inc/port/FatFs/lib_class_fatfs_spi_flash_chip.h"     // TODO
#endif

#if (DIGINI_FATFS_USE_SPI_SD_CARD == DEF_ENABLED)
#include "./Peripheral/inc/port/FatFs/lib_class_fatfs_spi_sd_card.h"     // TODO
#endif

#if (DIGINI_FATFS_USE_USB_KEY == DEF_ENABLED)
#include "./Peripheral/inc/port/FatFs/lib_class_fatfs_usb.h"
#endif

#include "./Peripheral/inc/FatFs/digini_diskio.h"
#include "diskio_drv_cfg.h"                     // This will add any custom driver

#endif // (DIGINI_USE_FATFS == DEF_ENABLED)

#include "./Grafx/inc/lib_grafx.h"

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)
#include "task_network.h"
#endif

//-------------------------------------------------------------------------------------------------
// Functions prototypes
//-------------------------------------------------------------------------------------------------

SystemState_e       DIGINI_Initialize       (void);
SystemState_e       DIGINI_PostInitialize   (void);

//-------------------------------------------------------------------------------------------------
