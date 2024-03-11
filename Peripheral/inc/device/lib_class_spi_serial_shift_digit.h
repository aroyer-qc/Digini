//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_serial_shift_digit.h
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

#define DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE          10

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct DigitConfig_t
{
    SPI_ID_e    SPI_ID
    IO_ID_e     LoadPin;
    IO_ID_e     BlankPin;
    uint8_t     NumberOfDigit;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------
class DigitDriverSerial
{
    public:

                    DigitDriverSerial       ();

        void        Initialize              (const DigitConfig_t* pConfig);
        
        void        Write                   (const uint8_t* pBuffer);                               // Update all digit from string. Supporting dot and send load command
        void        Write                   (uint8_t Value, uint8_t Offset, bool Dot = false);      // Change value at offset 
        void        WriteEncodedValue       (uint8_t Value, uint8_t Offset);                        // Change encoded value at offset.. Control raw digit
        void        Load                    (void);
        void        Blank                   (bool IsItEnabled);
        void        Dim                     (uint8_t Percent);

    private:

        DigitConfig_t*              m_pConfig;
        uint8_t*                    m_pDigitStream;
        const uint8_t               m_EncodedValue[DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE];
};

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)

//example :
/*
    uint8_t CustomDigit[10];


const uint8_t CustomDigit[3] =
{
    0xC6,       // °
    0x9C,       // C
    0x8E,       // F
}




*/

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DIGIT_DOT_VALUE             0x80

const uint8_t DigitDriverSerial::m_EncodedValue[DIGIT_NUMBER_OF_STANDARD_ENCODED_VALUE] =
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
}



DigitDriverSerial::DigitDriverSerial()
{
    m_pDigitStream = nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   DigitConfig_t*       Configuration located in device_var.h
//
//  Return:         void
//
//  Description:    Initialize the Serial shift for Digit using SPI interface
//
//-------------------------------------------------------------------------------------------------
void DigitDriverSerial::Initialize(const DigitConfig_t* pConfig)
{
    m_pConfig      = pConfig;
    m_pDigitStream = (uint8_t*) pMemoryPool->AllocAndClear(pConfig->NumberOfDigit);     // No digit ON

    IO_PinInit(m_pConfig->LoadPin);
    
    pConfig->pSPI // configure the SPI....
}

void DigitDriverSerial::Write(const uint8_t* pBuffer)
{
}

void DigitDriverSerial::Write(uint8_t Value, uint8_t Offset, bool Dot)
{
    if((Value >= '0') && (Value <= '9'))
    {
        Value -='0';                                // Remove Offset
        WriteEncodedValue(m_EncodedValue[Value] | (Dot == true ) ? DIGIT_DOT_VALUE : 0);
    }
}

// Change encoded value at offset.. Control raw digit
void DigitDriverSerial::WriteEncodedValue(uint8_t EncodedValue, uint8_t Offset)
{
    if(m_pDigitStream != nullptr)
    {
        if(Offset < m_pConfig->NumberOfDigit)
        {
            m_pDigitStream[Offset] = EncodedValue;
        }
    }
}

// Toggle the loading of the Encoded value
void DigitDriverSerial::Load(void)
{
    IO_SetPinHigh(m_pConfig->LoadPin);
    IO_SetPinLow(m_pConfig->LoadPin);
}

void DigitDriverSerial::Blank(bool IsItEnabled)
{
    // Stop PWM
    // or 
    // Start
}

void DigitDriverSerial::Dim(uint8_t Percent)
{
    // use Timer PWM channel
}
