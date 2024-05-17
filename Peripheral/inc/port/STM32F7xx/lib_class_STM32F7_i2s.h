//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_i2s.h
//
// Imported from F4, might not be compatible
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

#if (USE_I2S_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// I2S_CallBack type in bit position
#define I2S_CALLBACK_NONE               0x00
#define I2S_CALLBACK_HALF_COMPLETE      0x01
#define I2S_CALLBACK_COMPLETE           0x02
#define I2S_CALLBACK_ERROR              0x04

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
    I2S_192000_HZ,
    NB_OF_I2S_FREQUENCY,
};

// I2S_Mode
enum I2S_Mode_e
{
    I2S_MODE_SLAVE_TX       = 0x0000,
    I2S_MODE_SLAVE_RX       = 0x0100,
    I2S_MODE_MASTER_TX      = 0x0200,
    I2S_MODE_MASTER_RX      = 0x0300,
};

// I2S_Standard
enum I2S_Standard_e
{
    I2S_STANDARD_PHILLIPS   = 0x0000,
    I2S_STANDARD_MSB        = 0x0010,
    I2S_STANDARD_LSB        = 0x0020,
    I2S_STANDARD_PCM_SHORT  = 0x0030,
    I2S_STANDARD_PCM_LONG   = 0x00B0,
};

// I2S_Data Format
enum I2S_DataFormat_e
{
    I2S_DATAFORMAT_16B          = 0,
    I2S_DATAFORMAT_16B_EXTENDED = 1,
    I2S_DATAFORMAT_24B          = 3,
    I2S_DATAFORMAT_32B          = 5,
};

// I2S_MCLK_Output
enum I2S_MCLK_Output_e
{
    I2S_MCLK_OUTPUT_ENABLE  = 0x0200,
    I2S_MCLK_OUTPUT_DISABLE = 0x0000,
};

// I2S_Clock_Polarity
enum I2S_CPOL_Level_e
{
    I2S_CPOL_LOW            = 0x0000,
    I2S_CPOL_HIGH           = 0x0008,
};

struct I2S_Info_t
{
    I2S_ID_e            I2S_ID;
    I2S_TypeDef*        pI2Sx;
    uint32_t            RCC_APB1xPeriph;
    IO_ID_e             MCLK;
    IO_ID_e             CK;
    IO_ID_e             SD;
    IO_ID_e             WS;
    I2S_Mode_e          Mode;           // Specifies the I2S operating mode.
    I2S_DataFormat_e    DataFormat;     // Specifies the data format for the I2S communication.
    I2S_Standard_e      Standard;       // Specifies the standard used for the I2S communication.
    I2S_Frequency_e     Frequency;
    I2S_MCLK_Output_e   CLK_Output;     // Specifies whether the I2S MCLK output is enabled or not.
    I2S_CPOL_Level_e    CPOL_Level;     // Specifies the idle state of the I2S clock.
    // DMA
    
    DMA_Info_t          DMA_I2S;

    //uint32_t            DMA_Channel;
    //DMA_Stream_TypeDef* pDMA_Stream;
    //IRQn_Type           I2S_DMA_IRQn;
    //uint32_t            IT_Flag;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class I2S_Driver
{
    public:

                        I2S_Driver              (I2S_ID_e I2S_ID);

        SystemState_e   GetStatus               (void);

        void            Initialize              (void);
        void            RegisterCallback        (CallbackInterface* pCallback);
        void            EnableCallbackType      (int CallbackType, void* pContext = nullptr);
        SystemState_e   SetFrequency            (I2S_Frequency_e Frequency);
        SystemState_e   Transmit                (uint16_t* pBuffer, size_t Size);
        SystemState_e   Callback                (void);

    private:

        I2S_Info_t*                             m_pInfo;
        volatile uint16_t*                      m_pBuffer;
        volatile size_t                         m_Size;
        volatile SystemState_e                  m_Status;
        volatile uint8_t                        m_Timeout;
        static const uint32_t                   m_PLLN[NB_OF_I2S_FREQUENCY];
        static const uint32_t                   m_PLLR[NB_OF_I2S_FREQUENCY];

        DMA_Driver                              m_DMA;
        CallbackInterface*                      m_pCallback;
        int                                     m_CallBackType;
        void*                                   m_pContextHalf;
        void*                                   m_pContext;
        void*                                   m_pContextERROR;

        size_t                                  m_TX_transfertSize;
        size_t                                  m_TX_transfertCount;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "i2s_var.h"         // Project variable

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2S_DRIVER == DEF_ENABLED)



#if 0

example call for
  EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);            Initialize I2S interface
  EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, volume, AudioFreq );      Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...)
        Codec_Init(OutputDevice, VOLUME_CONVERT(Volume), AudioFreq)
            Codec_GPIO_Init();          // Configure the Codec related IOs  only reset to init
            Codec_Reset();              // Reset the Codec Registers
            Codec_CtrlInterface_Init(); // Initialize the Control interface of the Audio Codec
            counter += Codec_WriteRegister(0x02, 0x01)      // Keep Codec powered OFF
            counter += Codec_WriteRegister(0x04, 0xAF);     // SPK always OFF & HP always ON
            OutputDev = 0xAF;
            counter += Codec_WriteRegister(0x05, 0x81);           // Clock configuration: Auto detection
            counter += Codec_WriteRegister(0x06, CODEC_STANDARD); // Set the Slave Mode and the audio Standard
            Codec_VolumeCtrl(Volume);                             // Set the Master volume

            if (CurrAudioInterface == AUDIO_INTERFACE_DAC)
            {
                counter += Codec_WriteRegister(0x08, 0x01);       // Enable the PassThrough on AIN1A and AIN1B
                counter += Codec_WriteRegister(0x09, 0x01);
                counter += Codec_WriteRegister(0x0E, 0xC0);       // Route the analog input to the HP line
                counter += Codec_WriteRegister(0x14, 0x00);       // Set the Passthough volume
                counter += Codec_WriteRegister(0x15, 0x00);
            }

            counter += Codec_WriteRegister(0x02, 0x9E);             // Power on the Codec

            /* Additional configuration for the CODEC. These configurations are done to reduce
                the time needed for the Codec to power off. If these configurations are removed,
                then a long delay should be added between powering off the Codec and switching
                off the I2S peripheral MCLK clock (which is the operating clock for Codec).
                If this delay is not inserted, then the codec will not shut down properly and
                it results in high noise after shut down. */

            counter += Codec_WriteRegister(0x0A, 0x00);             // Disable the analog soft ramp

            if (CurrAudioInterface != AUDIO_INTERFACE_DAC)
            {
                counter += Codec_WriteRegister(0x0E, 0x04);         // Disable the digital soft ramp
            }
            counter += Codec_WriteRegister(0x27, 0x00);             // Disable the limiter attack level
            counter += Codec_WriteRegister(0x1F, 0x0F);             // Adjust Bass and Treble levels
            counter += Codec_WriteRegister(0x1A, 0x0A);             // Adjust PCM volume level
            counter += Codec_WriteRegister(0x1B, 0x0A);

            Codec_AudioInterface_Init(AudioFreq);                   // Configure the I2S peripheral
                RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);      // Enable the CODEC_I2S peripheral clock

                SPI_I2S_DeInit(CODEC_I2S);                          // CODEC_I2S peripheral configuration
                I2S_InitStructure.I2S_AudioFreq = AudioFreq;
                I2S_InitStructure.I2S_Standard = I2S_STANDARD;
                I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
                I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
                if (CurrAudioInterface == AUDIO_INTERFACE_DAC)
                {
                    I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;
                }
                else
                {
                    I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
                }
                #ifdef CODEC_MCLK_ENABLED
                I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
                #elif defined(CODEC_MCLK_DISABLED)
                I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
                #else
                  #error "No selection for the MCLK output has been defined !"
                #endif /* CODEC_MCLK_ENABLED */

                I2S_Init(CODEC_I2S, &I2S_InitStructure);            // Initialize the I2S peripheral with the structure above
                if(CurrAudioInterface == AUDIO_INTERFACE_DAC)       // Configure the DAC interface
                {
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); // DAC Periph clock enable
                DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;   // DAC channel1 Configuration
                DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
                DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
                DAC_Init(AUDIO_DAC_CHANNEL, &DAC_InitStructure);
                DAC_Cmd(AUDIO_DAC_CHANNEL, ENABLE);                 // Enable DAC Channel1
              }

              // The I2S peripheral will be enabled only in the EVAL_AUDIO_Play() function or by user functions if DMA mode not enabled


        Audio_MAL_Init();  // this should not be in the driver..
#endif

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



