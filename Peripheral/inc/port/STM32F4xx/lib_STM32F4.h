//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2022 Alain Royer.
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

#define I2S2                ((I2S_TypeDef *) SPI2_BASE)
#define I2S3                ((I2S_TypeDef *) SPI3_BASE)

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct I2S_TypeDef
{
    uint32_t      RESERVED0;    //                                  Reserved:       0x00
    __IO uint16_t CR2;          // SPI/I2S control register 2,      Address offset: 0x04
    uint16_t      RESERVED1;    //                                  Reserved:       0x06
    __IO uint16_t SR;           // SPI/I2S status register,         Address offset: 0x08
    uint16_t      RESERVED2;    //                                  Reserved:       0x0A
    __IO uint16_t DR;           // SPI/I2S data register,           Address offset: 0x0C
    uint16_t      RESERVED3;    //                                  Reserved:       0x0E
    uint32_t      RESERVED4;    //                                  Reserved:       0x10
    uint32_t      RESERVED5;    //                                  Reserved:       0x14
    uint32_t      RESERVED6;    //                                  Reserved:       0x18
    __IO uint16_t I2SCFGR;      // SPI_I2S configuration register,  Address offset: 0x1C
    uint16_t      RESERVED7;    //                                  Reserved:       0x1E
    __IO uint16_t I2SPR;        // SPI_I2S prescaler register,      Address offset: 0x20
    uint16_t      RESERVED8;    //                                  Reserved:       0x22
};

//-------------------------------------------------------------------------------------------------
