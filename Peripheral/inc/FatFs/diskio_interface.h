//-------------------------------------------------------------------------------------------------
//
//  File : diskio_interface.h
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

class DiskIO_DeviceInterface
{
    public:

        virtual                    ~DiskIO_DeviceInterface (){}


        virtual DSTATUS             Initialize              (void)                                  = 0;
        virtual DSTATUS             Status                  (void)                                  = 0;
        virtual DRESULT             Read                    (uint8_t*, uint32_t, uint16_t)          = 0;
      #if _USE_WRITE == 1
        virtual DRESULT             Write                   (const uint8_t*, uint32_t, uint16_t)    = 0;
      #endif
      #if _USE_IOCTL == 1
        virtual DRESULT             IO_Ctrl                 (uint8_t, void*)                        = 0;
      #endif

        //virtual void                Sync                    (void)                                  = 0;
        //virtual uint32_t            GetSectorCount          (void)                                  = 0;
        //virtual uint32_t            GetSectorSize           (void)                                  = 0;
        //virtual uint32_t            GetEraseBlockSize       (void)                                  = 0;
};

#endif

//-------------------------------------------------------------------------------------------------
