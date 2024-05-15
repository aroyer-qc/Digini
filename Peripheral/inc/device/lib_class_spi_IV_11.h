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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE          13
#define DIGIT_SERIAL_SHIFT_PADDING                      4
#define DIGIT_LOW_NIBBLE_MASK                           0x0F
#define DIGIT_HIGH_NIBBLE_MASK                          0xF0

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct IV_11_Config_t
{
    SPI_ID_e    SPI_ID;
    IO_ID_e     LoadPin;
    IO_ID_e     BlankPin;
    uint8_t     NumberOfDigit;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------
class IV_11_DigitDriver
{
    public:

                    IV_11_DigitDriver       ();

        void        Initialize              (const IV_11_Config_t* pConfig);

        void        Write                   (const char* pBuffer);                                  // Update all digit from string. Supporting dot and send load command
        void        Write                   (uint8_t Value, uint8_t Offset, bool Dot = false);      // Change value at offset
        void        WriteEncodedValue       (uint8_t Value, uint8_t Offset);                        // Change encoded value at offset.. Control raw digit
        void        Load                    (void);
        void        Blank                   (bool IsItEnabled);
        void        Dim                     (uint8_t Percent);

    private:

        const IV_11_Config_t*       m_pConfig;
        SPI_Driver*                 m_pSPI;
        uint8_t*                    m_pDigitStream;
        uint16_t                    m_Padding;
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
