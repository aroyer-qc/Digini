//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_fatfs_usb.h
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

//#include "diskio.h"
#if (USE_USB_DRIVER == DEF_ENABLED)

#include "lib_class_usbh.h"
#include "diskio_interface.h"

//-------------------------------------------------------------------------------------------------

class CFatFS_CUSB : public DiskIO_DeviceInterface
{
    public:

                    CFatFS_CUSB         (USB_HostInterface* pUSB);
                   ~CFatFS_CUSB         () {}

// TODO might need to add function for the passing of instance to

        DSTATUS     Initialize          (void);
        DSTATUS     Status              (void);
        DRESULT     Read                (uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfBlocks);
      #if _USE_WRITE == 1
        DRESULT     Write               (const uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfBlocks);
      #endif
      #if _USE_IOCTL == 1
        DRESULT     IO_Ctrl             (uint8_t Control, void *pBuffer);
      #endif

        void        Configure           (uint8_t* pBuffer, size_t Size);

    private:

        DRESULT     CheckError          (uint32_t Sector, uint16_t NumberOfSectors);

        bool                        m_Initialize;
        USB_HostInterface*          m_pUSB;
        DSTATUS                     m_Status;
};

//-------------------------------------------------------------------------------------------------

#endif // (USE_USB_DRIVER == DEF_ENABLED)
