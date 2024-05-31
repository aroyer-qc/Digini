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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_IV_11_GLOBAL
#include "./Digini/Peripheral/inc/device/lib_class_spi_IV_11.h"
#undef  LIB_IV_11_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DIGIT_DOT_VALUE             0x80

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const uint8_t IV_11_DigitDriver::m_EncodedValue[DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE] =
{
    0xFC,   // 0
    0x60,   // 1
    0xDA,   // 2
    0xF2,   // 3
    0x66,   // 4
    0xB6,   // 5
    0xBE,   // 6
    0xE0,   // 7
    0xFE,   // 8
    0xF6,   // 9
    0xC6,   // ° Degree
    0x9C,   // C Celsius
    0x8E,   // F fahrenheit
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Constructor
//
//  Parameter(s):   None
//
//  Description:    Preinit pointer
//
//-------------------------------------------------------------------------------------------------
IV_11_DigitDriver::IV_11_DigitDriver()
{
    m_pDigitStream = nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   IV_11_Config_t*     Configuration located in device_var.h
//
//  Return:         void
//
//  Description:    Initialize the Serial shift for Digit using SPI interface
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::Initialize(const IV_11_Config_t* pConfig)
{
    size_t RequiredBytes;

    m_pConfig = pConfig;

    // IV-11 need 20 Bits for each pair of IV-11.
    // There is 4 dummy bits for each pair, but for the last pair those bits do not need to be sent.
    // There is padding bits to add in the beginning of the stream if not multiple of 8 bits
    RequiredBytes  = pConfig->NumberOfDigit;                                    // Number of bytes
    RequiredBytes += (((RequiredBytes / 2) - 1) / 2);                           // Remove one pair size, than add 4 bits for all remain pair.
    m_pDigitStream = (uint8_t*) pMemoryPool->AllocAndClear(RequiredBytes);      // No digit ON

    // If number of bit per pair is divisible by 8, then we need padding so all bit needed are fetch.
    // If not divisible by 8, the no need for padding since all bit will be fetch
    m_Padding = (((pConfig->NumberOfDigit / 2) % 8) == 0) ? 4 : 0;

    IO_PinInit(m_pConfig->LoadPin);

    m_pSPI = new SPI_Driver(pConfig->SPI_ID);
    m_pSPI->Initialize();
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
    uint8_t EncodedOffset;
    uint8_t Value;
    size_t  Length;
    bool    Dot;

    EncodedOffset = 0;
    Length = strlen(pBuffer);

    for(size_t i = 0; i < Length; i++)
    {
        Value = pBuffer[i];
        Dot = false;

        if((i + 1) != Length)   // Do not check dot past the length of the string.
        {
            if(pBuffer[i + 1] == '.')
            {
                Dot = true;
            }
        }

        Write(Value, EncodedOffset, Dot);
        EncodedOffset++;

        if(Dot == true)
        {
            i++;
        }

        if(EncodedOffset >= m_pConfig->NumberOfDigit)
        {
            return;
        }
    }


    m_pSPI->Transfer(m_pDigitStream, 7, nullptr, 0, (SPI_DeviceInfo_t*)this);







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

    WriteEncodedValue(m_EncodedValue[Value] | (Dot == true ) ? DIGIT_DOT_VALUE : 0, Offset);
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
//  Description:    Change encoded value at offset.. Control raw digit.
//
//  Note(s)         Invalid character will be ignored
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::WriteEncodedValue(uint8_t EncodedValue, uint8_t Offset)
{
    uint16_t BitOffset;

    if(m_pDigitStream != nullptr)
    {
        if(Offset < m_pConfig->NumberOfDigit)
        {

            // Each pair are separated by 20 bits, so calculate bit offset.
            // Add padding if any.
            BitOffset = (((uint16_t(Offset) / 2) * 20) + uint16_t((Offset % 2) * 8)) + m_Padding;

            Offset = uint8_t(BitOffset / 8);

            if((BitOffset % 8) == 0)
            {
                m_pDigitStream[Offset] = EncodedValue;
            }
            else
            {
                CLEAR_BIT(m_pDigitStream[Offset], DIGIT_LOW_NIBBLE_MASK);
                SET_BIT(m_pDigitStream[Offset], EncodedValue >> 4);

                CLEAR_BIT(m_pDigitStream[Offset + 1], DIGIT_HIGH_NIBBLE_MASK);
                SET_BIT(m_pDigitStream[Offset + 1], EncodedValue << 4);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Load
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Toggle the loading of the encoded value
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::Load(void)
{
    IO_SetPinHigh(m_pConfig->LoadPin);
    IO_SetPinLow(m_pConfig->LoadPin);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Blank
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Blank the tube
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::Blank(bool IsItEnabled)
{
    // Stop PWM
    // or
    // Start
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Dim
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    control the brightness on the tube
//
//-------------------------------------------------------------------------------------------------
void IV_11_DigitDriver::Dim(uint8_t Percent)
{
    // use Timer PWM channel
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
