//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_fatfs_usb_key.h
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
// Define(s)
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "diskio_interface.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_FATFS_USE_USB_KEY == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class FatFS_USB_Key : public DiskIO_DeviceInterface
{
    public:

								FatFS_USB_Key ();
							   ~FatFS_USB_Key (){}

        //void            		Configure           (uint8_t* pBuffer, size_t Size);

		// Mandatory function required by FatFs
        DSTATUS         		Initialize          (void);
        DSTATUS         		Status              (void);
        DRESULT         		Read                (uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors);
      #if _USE_WRITE == 1
        DRESULT         		Write               (const uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors);
      #endif
      #if _USE_IOCTL == 1
        DRESULT         		IO_Ctrl             (uint8_t Control, void* pBuffer);
      #endif

    private:

        //bool            		m_IsItInitialize    = false;
        DSTATUS         		m_Status            = STA_NODISK;
		DiskMedia_e				m_ThisDisk			= INVALID_DISK;

        //uint8_t*        		m_pBuffer           = nullptr;
        //size_t          		m_Size              = 0;                       // size of the disk, is a multiple of 512
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_FATFS_USE_USB_KEY == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#endif // __cplusplus

