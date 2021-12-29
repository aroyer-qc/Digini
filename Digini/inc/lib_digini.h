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
// Standard header
//
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
//#include <cstddef>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Common configuration header
//
#include "clock_cfg.h"
#include "digini_cfg.h"
#include "driver_cfg.h"
#include "bsp_io_def.h"
#include "app_cfg.h"
#include "label_cfg.h"

//-------------------------------------------------------------------------------------------------
// Digini library
//
#include "lib_assert.h"
#include "lib_memory.h"
#include "lib_class_database.h"
#include "lib_label.h"
#include "lib_macro.h"
#include "lib_define.h"
#include "lib_typedef.h"
#include "lib_string.h"
#include "lib_CQueue.h"
#include "lib_utility.h"
#include "lib_io.h"
#include "lib_isr.h"
#include "lib_dma.h"

//-------------------------------------------------------------------------------------------------
// Operating system
//
#include "nOS.h"

//-------------------------------------------------------------------------------------------------
// Driver
//
#if (USE_ADC_DRIVER == DEF_ENABLED)
#include "lib_class_adc.h"
#endif

#if (USE_DAC_DRIVER == DEF_ENABLED)
#include "lib_class_dac.h"
#endif

#if (USE_I2C_DRIVER == DEF_ENABLED)
#include "i2c_cfg.h"
#include "lib_class_i2c.h"
#endif

#if (USE_I2S_DRIVER == DEF_ENABLED)
#include "lib_class_i2s.h"
#endif

#if (USE_PWM_DRIVER == DEF_ENABLED)
#include "lib_class_pwm.h"
#endif

#if (USE_QSPI_DRIVER == DEF_ENABLED)
 #include "qspi_cfg.h"
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
#include "spi_cfg.h"
#include "lib_class_spi.h"
#endif

#if (USE_TIM_DRIVER == DEF_ENABLED)
#include "tim_cfg.h"
#include "lib_class_tim.h"
#endif

#if (USE_UART_DRIVER == DEF_ENABLED)
#include "uart_cfg.h"
#include "lib_class_uart.h"
#endif

#if (USE_USB_DRIVER == DEF_ENABLED)
#include "lib_class_usb.h"
#endif

//-------------------------------------------------------------------------------------------------
// High level Peripheral
//

#ifdef DIGINI_USE_EEPROM
#include "eeprom_cfg.h"
#include "lib_class_I2C_EEprom.h"
#endif


#ifdef DIGINI_USE_FATFS
#include "ff.h"
#include "lib_fatfs_disk.h"
#endif

#include "lib_grafx.h"

//-------------------------------------------------------------------------------------------------
// Functions prototypes
//-------------------------------------------------------------------------------------------------

SystemState_e       DIGINI_Initialize       (void);
SystemState_e       DIGINI_PostInitialize   (void);

//-------------------------------------------------------------------------------------------------