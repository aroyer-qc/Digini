//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_IV_11.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_IV_11_GLOBAL
#include "./Digini/lib_digini.h"
#undef  LIB_IV_11_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DIGIT_DOT_VALUE             0x80
#define NUMBER_OF_BIT_IN_DIGIT      8

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const uint8_t IV_11_DigitDriver::m_EncodedValue[DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE] =
{
    0x7E,   // 0
    0x30,   // 1
    0x6D,   // 2
    0x79,   // 3
    0x33,   // 4
    0x5B,   // 5
    0x5F,   // 6
    0x70,   // 7
    0x7F,   // 8
    0x7B,   // 9
    0x63,   // ° Degree
    0x4E,   // C Celsius
    0x47,   // F fahrenheit
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Constructor
//
//  Parameter(s):   VFD_Driver*         pDriver             VFD_Driver to use
//                  const uint16_t*     pInfo               Info on position of each tube in stream
//                  uint8_t             NumberOfTubes       Number of tube connected on the stream
//
//  Description:    Initialize configuration
//
//-------------------------------------------------------------------------------------------------
IV_11_DigitDriver::IV_11_DigitDriver(VFD_Driver* pDriver, const uint16_t* pInfo, uint8_t NumberOfTubes)
{
    m_pDriver       = pDriver;
    m_pInfo         = pInfo;
    m_NumberOfTubes = NumberOfTubes;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   const uint8_t* pBuffer      Buffer to write on Digit
//
//  Return:         void
//
//  Description:    Display a string on IV-11 digit.
//
//  Note(s)         Invalid character will be ignored.
//                  no support yet for custom character from string (maybe later)
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::Write(const char* pBuffer)
{
    // TODO
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   uint8_t     Value       Value to display on the digit
//                  uint8_t     Offset      Which digit to change
//                  bool        Dot         If the dot is ON (true or false)
//
//  Return:         void
//
//  Description:    Write a value on a specific digit.
//
//  Note(s)         Invalid character will be ignored
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::Write(uint8_t Value, uint8_t Offset, bool Dot)
{
    uint8_t EncodedValue;

    if((Value >= '0') && (Value <= '9'))
    {
        Value -='0';                                // Remove Offset
    }
    else if (Value == '\xBA')
    {
        Value = 10;
    }
    else if (Value == 'C')
    {
        Value = 11;
    }
    else if (Value == 'F')
    {
        Value = 12;
    }
    else
    {
        return;
    }

    EncodedValue  = m_EncodedValue[Value];
    EncodedValue |= ((Dot == true ) ? DIGIT_DOT_VALUE : 0);
    WriteEncodedValue(EncodedValue, Offset);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   uint8_t     Value       Value to display on the digit
//                  uint8_t     Offset      Which digit to change
//
//  Return:         void
//
//  Description:    Change encoded value at offset.. Control raw digit.
//
//  Note(s)         Invalid character will be ignored
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::WriteEncodedValue(uint8_t EncodedValue, uint8_t Offset)
{
    uint32_t StreamIndex;

    if(Offset < m_NumberOfTubes)
    {
        StreamIndex = m_pInfo[Offset];

        m_pDriver->Set(StreamIndex, &EncodedValue, NUMBER_OF_BIT_IN_DIGIT);
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
