//-------------------------------------------------------------------------------------------------
//
//  File : usbh_conf.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "stm32f4xx.h"
#include "stm32f4xx_ll_usb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./Digini/Memory/inc/lib_c_memory_wrapper.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define USBH_MAX_NUM_ENDPOINTS              2U
#define USBH_MAX_NUM_INTERFACES             2U
#define USBH_MAX_NUM_CONFIGURATION          1U
#define USBH_KEEP_CFG_DESCRIPTOR            1U
#define USBH_MAX_NUM_SUPPORTED_CLASS        1U
#define USBH_MAX_SIZE_CONFIGURATION         0x200U
#define USBH_MAX_DATA_BUFFER                0x200U
#define USBH_DEBUG_LEVEL                    0U                // No Log
#define USBH_USE_OS                         0U				// Not compatible with nOS
#define USBH_IN_NAK_PROCESS                 0

// Memory management macros
#define USBH_malloc(SIZE)         			(MSC_HandleTypeDef*)MemoryPool_Alloc(SIZE)
#define USBH_free(PTR)            			MemoryPool_Free(PTR)
#define USBH_memset               			memset
#define USBH_memcpy              		 	memcpy
#define HAL_Delay(d)

// DEBUG macros
#if (USBH_DEBUG_LEVEL > 0U)
#define  USBH_UsrLog(...)   do { \
                                 printf(__VA_ARGS__); \
                                 printf("\n"); \
                               } while (0)
#else
#define USBH_UsrLog(...) do {} while (0)
#endif

#if (USBH_DEBUG_LEVEL > 1U)

#define  USBH_ErrLog(...) do { \
                               printf("ERROR: "); \
                               printf(__VA_ARGS__); \
                               printf("\n"); \
                             } while (0)
#else
#define USBH_ErrLog(...) do {} while (0)
#endif

#if (USBH_DEBUG_LEVEL > 2U)
#define  USBH_DbgLog(...)   do { \
                                 printf("DEBUG : "); \
                                 printf(__VA_ARGS__); \
                                 printf("\n"); \
                               } while (0)
#else
#define USBH_DbgLog(...) do {} while (0)
#endif

#ifdef __cplusplus
}
#endif
