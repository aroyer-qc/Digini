//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F7_crc.h
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
// Define(s)
//-------------------------------------------------------------------------------------------------

// Definition for void CRC_Initialize -> Mode
// Example Mode = (CRC_POLYNOMIAL_LENGTH_32_BITS | CRC_INPUT_DATA_INVERSION_HALFWORD | CRC_OUTPUT_DATA_INVERSION_ENABLE)


#define CRC_POLYNOMIAL_LENGTH_7_BITS            0x00000018
#define CRC_POLYNOMIAL_LENGTH_8_BITS            0x00000010
#define CRC_POLYNOMIAL_LENGTH_16_BITS           0x00000008
#define CRC_POLYNOMIAL_LENGTH_32_BITS           0x00000000
                                              
#define CRC_INPUT_DATA_INVERSION_NONE           0x00000000
#define CRC_INPUT_DATA_INVERSION_BYTE           0x00000020
#define CRC_INPUT_DATA_INVERSION_HALFWORD       0x00000040
#define CRC_INPUT_DATA_INVERSION_WORD           0x00000060

#define CRC_OUTPUT_DATA_INVERSION_DISABLE       0x00000000
#define CRC_OUTPUT_DATA_INVERSION_ENABLE        0x00000080

//-------------------------------------------------------------------------------------------------
// function definition(s)
//-------------------------------------------------------------------------------------------------

void        CRC_Reset       (void);
uint32_t    CRC_GetValue    (void);
void        CRC_AddByte     (const uint8_t Byte);
void        CRC_AddBuffer   (const uint8_t *pBuffer, size_t Length);

//-------------------------------------------------------------------------------------------------

