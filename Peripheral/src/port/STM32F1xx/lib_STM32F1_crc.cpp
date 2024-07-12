//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F1_crc.cpp
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_CRC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           CRC_Reset
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    This function init the hardware for crc calculation.
//
//-------------------------------------------------------------------------------------------------
void CRC_Reset(void)
{
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
    CRC->CR = 1;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CRC_GetValue
//
//  Parameter(s):   void
//  Return:         uint32_t
//
//  Description:    This function return the 32 bits CRC value.
//
//-------------------------------------------------------------------------------------------------
uint32_t CRC_GetValue(void)
{
    return CRC->DR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CRC_AddByte
//
//  Parameter(s):   uint8_t Byte
//  Return:         void
//
//  Description:    Add a byte to calculation of on going CRC sequence.
//
//-------------------------------------------------------------------------------------------------
void CRC_AddByte(const uint8_t Byte)
{
    CRC->DR = Byte;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CRC_AddBuffer
//
//  Parameter(s):   pBuffer             Calculate CRC on this buffer
//                  Length              Length of the buffer
//  Return:         void
//
//  Description:    Add buffer to calculation of on going CRC sequence.
//
//-------------------------------------------------------------------------------------------------
void CRC_AddBuffer(const uint8_t *pBuffer, size_t Length)
{
    // Enter Data to the CRC calculator
    for(size_t i = 0; i < Length; i++)
    {
        CRC->DR = pBuffer[i];
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_CRC_DRIVER == DEF_ENABLED)
