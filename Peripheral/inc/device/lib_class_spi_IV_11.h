//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_IV_11.h
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
//
//  Note(s) This driver assume the grid is connected through the driver and follow the 8 bits for
//          each segment
//
//          Support IV-6, IV-8, IV-11, IV-22
//
//          Need modification other tube are to be supported  IV-3, IV-12, IV-17,
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE          14

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class IV_11_DigitDriver
{
    public:

                    IV_11_DigitDriver       (VFD_Driver* pDriver, const uint16_t* pInfo, uint8_t NumberOfTubes);

        void        GridControl             (bool IsItDriven);
        void        Write                   (const char* pBuffer);                                  // Update all digit from string. Supporting dot and send load command
        void        Write                   (uint8_t Value, uint8_t Offset, bool Dot = false);      // Change value at offset
        void        WriteEncodedValue       (uint8_t Value, uint8_t Offset);                        // Change encoded value at offset.. Control raw digit

        void        Blank                   (bool IsItBlank)   {m_pDriver->Blank(IsItBlank); }
        void        Dim                     (uint8_t DimValue) {m_pDriver->Dim(DimValue);    }
        void        Send                    (void)             {m_pDriver->Send();           }

    private:

        const uint16_t*             m_pInfo;
        VFD_Driver*                 m_pDriver;
        uint8_t                     m_NumberOfTubes;
        static const uint8_t        m_EncodedValue[DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_DIGIT_IV_11__
#include "device_var.h"
#undef  __CLASS_DIGIT_IV_11__

//-------------------------------------------------------------------------------------------------

#else // (USE_SPI_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for SPI must be enable and configure to use this device driver")

#endif // (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
