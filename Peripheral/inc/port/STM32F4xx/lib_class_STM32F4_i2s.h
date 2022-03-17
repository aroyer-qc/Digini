//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_i2s.h
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
//
//  use one of the define in device_cfg.h
//            I2S_STANDARD_PHILLIPS
//            I2S_STANDARD_MSB
//            I2S_STANDARD_LSB
//

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_I2S_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// typedef(s)
//-------------------------------------------------------------------------------------------------

enum I2S_ID_e
{
    #if (I2S_DRIVER_SUPPORT_I2S1_CFG == DEF_ENABLED)
        DRIVER_I2S1_ID,
    #endif

    #if (I2S_DRIVER_SUPPORT_I2S2_CFG == DEF_ENABLED)
        DRIVER_I2S2_ID,
    #endif

    #if (I2S_DRIVER_SUPPORT_I2S3_CFG == DEF_ENABLED)
        DRIVER_I2S3_ID,
    #endif

    NB_OF_I2S_DRIVER,
};

enum I2S_Frequency_e
{   I2S_8000_HZ,
    I2S_11025_HZ,
    I2S_16000_HZ,
    I2S_22050_HZ,
    I2S_32000_HZ,
    I2S_44100_HZ,
    I2S_48000_HZ,
    I2S_96000_HZ,
    NB_OF_I2S_FREQUENCY,
};

struct I2S_Init_t
{
    uint16_t I2S_Mode;          // Specifies the I2S operating mode.                            This Parameter can be a value of @ref I2S_Mode
    uint16_t I2S_Standard;      // Specifies the standard used for the I2S communication.       This Parameter can be a value of @ref I2S_Standard
    uint16_t I2S_DataFormat;    // Specifies the data format for the I2S communication.         This Parameter can be a value of @ref I2S_Data_Format
    uint16_t I2S_MCLKOutput;    // Specifies whether the I2S MCLK output is enabled or not.     This Parameter can be a value of @ref I2S_MCLK_Output
    uint32_t I2S_AudioFreq;     // Specifies the frequency selected for the I2S communication.  This Parameter can be a value of @ref I2S_Audio_Frequency
    uint16_t I2S_CPOL;          // Specifies the idle state of the I2S clock.                   This Parameter can be a value of @ref I2S_Clock_Polarity
};

typedef struct
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
} I2S_TypeDef;

struct I2S_Info_t
{
    I2S_ID_e            I2S_ID;
    I2S_TypeDef*        pI2Sx;
    IO_ID_e             SCK;
    IO_ID_e             SD;
    IRQn_Type           I2S_DMA_IRQn;

    // DMA
    DMA_TypeDef*        pDMAx;
    uint32_t            DMA_Channel;
    uint32_t            IT_Flag;
    DMA_Stream_TypeDef* DMA_Stream;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class I2S_Driver
{
    public:

                        I2S_Driver          (I2S_ID_e* I2S_ID);

        SystemState_e   GetStatus           (void);

        void            Initialize          (void);
        SystemState_e   SetFrequency        (I2S_Frequency_e Frequency);

    private:

        void            ChangeBuffer        (uint16_t *pData, uint16_t Size);

        I2S_Info_t*                         m_pInfo;

        //volatile size_t                   m_Size;
        //volatile uint8_t*                 m_pBuffer;

        volatile size_t                     m_Size;

        volatile SystemState_e              m_Status;
        volatile uint8_t                    m_Timeout;

        static const uint32_t               m_PLLN[NB_OF_I2S_FREQUENCY];
        static const uint32_t               m_PLLR[NB_OF_I2S_FREQUENCY];
};


#if 0
/* Initialization and Configuration functions *********************************/
void I2S_Cmd(SPI_TypeDef* SPIx, FunctionalState NewState);
void I2S_FullDuplexConfig(SPI_TypeDef* I2Sxext, I2S_InitTypeDef* I2S_InitStruct);
/* Data transfers functions ***************************************************/
void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data);
uint16_t SPI_I2S_ReceiveData(SPI_TypeDef* SPIx);

/* DMA transfers management functions *****************************************/
void SPI_I2S_DMACmd(SPI_TypeDef* SPIx, uint16_t SPI_I2S_DMAReq, FunctionalState NewState);
/* Interrupts and flags management functions **********************************/
void SPI_I2S_ITConfig(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT, FunctionalState NewState);
FlagStatus SPI_I2S_GetFlagStatus(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG);
void SPI_I2S_ClearFlag(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG);
ITStatus SPI_I2S_GetITStatus(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT);
void SPI_I2S_ClearITPendingBit(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT);
#endif

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "i2s_var.h"

//-------------------------------------------------------------------------------------------------

#endif // STM32F4_I2S_GLOBAL

//-------------------------------------------------------------------------------------------------




