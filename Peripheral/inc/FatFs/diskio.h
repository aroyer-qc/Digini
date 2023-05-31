//-------------------------------------------------------------------------------------------------
//
//  File : diskio.h
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

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "diskio_def.h"
//#include "ff.h"

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

// Patch..
typedef unsigned int	UINT;	/* int must be 16-bit or 32-bit */
typedef uint32_t		DWORD;	/* 32-bit unsigned integer */

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

// Prototypes for disk control functions needed by FatFS
DSTATUS             disk_initialize         (uint8_t Drive);
DSTATUS             disk_status             (uint8_t Drive);
DRESULT             disk_read               (uint8_t Drive, uint8_t*, uint32_t, uint16_t);
#if _USE_WRITE == 1
DRESULT             disk_write              (uint8_t Drive, const uint8_t*, uint32_t, uint16_t);
#endif
#if _USE_IOCTL == 1
DRESULT             disk_ioctl              (uint8_t Drive, uint8_t, void*);
#endif
uint32_t            get_fattime             (void);

DWORD               ff_convert              (DWORD wch, UINT dir);
DWORD               ff_wtoupper             (DWORD wch);

#ifdef __cplusplus
}
#endif

//-------------------------------------------------------------------------------------------------
