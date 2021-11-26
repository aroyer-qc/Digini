//-------------------------------------------------------------------------------------------------
//
//  File : lib_peripheral_cfg.h
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "nOS.h"
#include "lib_macro.h"
#include "lib_define.h"
#include "lib_typedef.h"
#include "assert.h"
#include "lib_io.h"
#include "lib_isr.h"
#include "clock_cfg.h"
#include "driver_cfg.h"

#if (USE_DAC_DRIVER == DEF_ENABLED)
#include "dac_cfg.h"
#endif

#if (USE_ADC_DRIVER == DEF_ENABLED)           // TODO
#include "adc_cfg.h"
#endif

#if (USE_I2C_DRIVER == DEF_ENABLED)
#include "i2c_cfg.h"
#endif

#if (USE_I2S_DRIVER == DEF_ENABLED)
#include "i2s_cfg.h"
#endif

#if (USE_PWM_DRIVER == DEF_ENABLED)
#include "pwm_cfg.h"
#endif

#if (USE_QSPI_DRIVER == DEF_ENABLED)
 #include "qspi_cfg.h"
#endif

#if (USE_RTC_DRIVER == DEF_ENABLED)
#include "rtc_cfg.h"
#endif

#if (USE_SAI_DRIVER == DEF_ENABLED)           // TODO
#include "sai_cfg.h"
#endif

#if (USE_SDIO_DRIVER == DEF_ENABLED)
#include "sdio_cfg.h"
#endif

#if (USE_SPI_DRIVER == DEF_ENABLED)
#include "spi_cfg.h"
#endif

#if (USE_TIM_DRIVER == DEF_ENABLED)
#include "tim_cfg.h"
#endif

#if (USE_UART_DRIVER == DEF_ENABLED)
#include "uart_cfg.h"
#endif

#if (USE_USB_DRIVER == DEF_ENABLED)
#include "usb_cfg.h"
#endif

//-------------------------------------------------------------------------------------------------
