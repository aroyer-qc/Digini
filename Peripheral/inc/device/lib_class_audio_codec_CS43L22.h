//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_audio_codec_C43L22.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"
#include "lib_class_audio_interface.h"
#include "device_cfg.h"

//-------------------------------------------------------------------------------------------------
// define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CS43L22 : public AudioInterface

{
    public:

        SystemState_e       Initialize            (void* pArg);
        //SystemState_e       Initialize            (void* pArgControl, void* pArgData);
        //SystemState_e     DeInitialize          (void);
        uint16_t            ReadID                (void);
        SystemState_e       Reset                 (void);

        SystemState_e       Play                  (uint16_t* pBuffer, uint16_t Size);
        SystemState_e       Stop                  (uint32_t CodecPdwnMode);
        SystemState_e       Pause                 (void);
        SystemState_e       Resume                (void);

        SystemState_e       SetVolume             (uint8_t Volume);
        SystemState_e       SetFrequency          (uint32_t AudioFrequency);
        SystemState_e       SetMute               (uint32_t Command);
        SystemState_e       SetOutputMode         (uint8_t Mode);

    private:

        SystemState_e       Write                 (uint8_t Register, uint8_t Data);

        uint8_t             m_DeviceAddress;
        uint8_t             m_OutputConfig;
        bool                m_IsItStopped;

        I2C_Driver*         m_pI2C;
        I2S_Driver*         m_pI2S;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

extern class CS43L22                     CS43L22_AudioCodec;

#ifdef LIB_CS43L22_GLOBAL
 class CS43L22                           CS43L22_AudioCodec;
#endif

//-------------------------------------------------------------------------------------------------

#if 0


/*------------------------------------
    CONFIGURATION: Audio Codec Driver Configuration parameters
----------------------------------------*/
// Audio Transfer mode (I2S Interrupt)
//#define I2S_INTERRUPT                 // Uncomment this line to enable audio transfert with I2S interrupt

// Audio Transfer mode (DMA, Interrupt or Polling)
#define AUDIO_MAL_MODE_NORMAL         // Uncomment this line to enable the audio Transfer using DMA
// #define AUDIO_MAL_MODE_CIRCULAR    // Uncomment this line to enable the audio Transfer using DMA

// For the DMA modes select the interrupt that will be used
#define AUDIO_MAL_DMA_IT_TC_EN        // Uncomment this line to enable DMA Transfer Complete interrupt
// #define AUDIO_MAL_DMA_IT_HT_EN     // Uncomment this line to enable DMA Half Transfer Complete interrupt
// #define AUDIO_MAL_DMA_IT_TE_EN     // Uncomment this line to enable DMA Transfer Error interrupt

// Select the interrupt preemption priority and subpriority for the DMA interrupt
#define EVAL_AUDIO_IRQ_PREPRIO           0   // Select the preemption priority level(0 is the highest)
#define EVAL_AUDIO_IRQ_SUBRIO            0   // Select the sub-priority level (0 is the highest)

/* Uncomment the following line to use the default Codec_TIMEOUT_UserCallback()
   function implemented in stm32f4_discovery_audio_codec.c file.
   Codec_TIMEOUT_UserCallback() function is called whenever a timeout condition
   occurs during communication (waiting on an event that doesn't occur, bus
   errors, busy devices ...). */
/* #define USE_DEFAULT_TIMEOUT_CALLBACK */

/* Enable this define to use the I2S DMA for writing into DAC register */
//#define DAC_USE_I2S_DMA
/*----------------------------------------------------------------------------*/

/*------------------------------------
                    OPTIONAL Configuration defines parameters
*/
/* Uncomment the defines below to select if the Master clock mode should be
  enabled or not */
#define CODEC_MCLK_ENABLED
/* #deine CODEC_MCLK_DISABLED */

/* Uncomment this line to enable verifying data sent to codec after each write
  operation */
#define VERIFY_WRITTENDATA
/*----------------------------------------------------------------------------*/

/*-----------------------------------
                    Hardware Configuration defines parameters
                                     -----------------------------------------*/
/* Audio Reset Pin definition */
#define AUDIO_RESET_GPIO_CLK           RCC_AHB1Periph_GPIOD
#define AUDIO_RESET_PIN                GPIO_Pin_4
#define AUDIO_RESET_GPIO               GPIOD

/* I2S peripheral configuration defines */
#define CODEC_I2S                      SPI3
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_ADDRESS              0x40003C0C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI3
#define CODEC_I2S_IRQ                  SPI3_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOA)
#define CODEC_I2S_WS_PIN               GPIO_Pin_4
#define CODEC_I2S_SCK_PIN              GPIO_Pin_10
#define CODEC_I2S_SD_PIN               GPIO_Pin_12
#define CODEC_I2S_MCK_PIN              GPIO_Pin_7
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource4
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource10
#define CODEC_I2S_SD_PINSRC            GPIO_PinSource12
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource7
#define CODEC_I2S_GPIO                 GPIOC
#define CODEC_I2S_WS_GPIO              GPIOA
#define CODEC_I2S_MCK_GPIO             GPIOC
#define Audio_I2S_IRQHandler           SPI3_IRQHandler


 #define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
 #define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord
 #define DMA_MAX_SIZE                   0xFFFF


 #define DAC_DHR12L1_ADDRESS            0x4000740C
 #define DAC_DHR12R1_ADDRESS            0x40007408
 #define DAC_DHR8R1_ADDRESS             0x40007410
 #define AUDIO_DAC_CHANNEL              DAC_Channel_1

 /* I2S DMA Stream definitions */
 #define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
 #define AUDIO_I2S_DMA_STREAM           DMA1_Stream7
 #define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
 #define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
 #define AUDIO_I2S_DMA_IRQ              DMA1_Stream7_IRQn
 #define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF7
 #define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF7
 #define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF7
 #define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF7
 #define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF7

 #define Audio_MAL_I2S_IRQHandler       DMA1_Stream7_IRQHandler


 /* DAC DMA Stream definitions */
 #define AUDIO_DAC_DMA_CLOCK            RCC_AHB1Periph_DMA1
 #define AUDIO_DAC_DMA_STREAM           DMA1_Stream0
 #define AUDIO_DAC_DMA_DREG             DAC_DHR12L1_ADDRESS
 #define AUDIO_DAC_DMA_CHANNEL          DMA_Channel_0
 #define AUDIO_DAC_DMA_IRQ              DMA1_Stream0_IRQn
 #define AUDIO_DAC_DMA_FLAG_TC          DMA_FLAG_TCIF0
 #define AUDIO_DAC_DMA_FLAG_HT          DMA_FLAG_HTIF0
 #define AUDIO_DAC_DMA_FLAG_FE          DMA_FLAG_FEIF0
 #define AUDIO_DAC_DMA_FLAG_TE          DMA_FLAG_TEIF0
 #define AUDIO_DAC_DMA_FLAG_DME         DMA_FLAG_DMEIF0

 #define Audio_MAL_DAC_IRQHandler       DMA1_Stream0_IRQHandler


/* I2C peripheral configuration defines (control interface of the audio codec) */
#define CODEC_I2C                      I2C1
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C1
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C1
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_6
#define CODEC_I2C_SDA_PIN              GPIO_Pin_9
#define CODEC_I2S_SCL_PINSRC           GPIO_PinSource6
#define CODEC_I2S_SDA_PINSRC           GPIO_PinSource9

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define CODEC_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))
/*----------------------------------------------------------------------------*/

/*-----------------------------------
                        Audio Codec User defines
                                     -----------------------------------------*/
/* Audio interface : I2S or DAC */
#define AUDIO_INTERFACE_I2S           1
#define AUDIO_INTERFACE_DAC           2

/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER         1
#define OUTPUT_DEVICE_HEADPHONE       2
#define OUTPUT_DEVICE_BOTH            3
#define OUTPUT_DEVICE_AUTO            4

/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04

#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

/*----------------------------------------------------------------------------*/
#define VOLUME_CONVERT(x)    ((Volume > 100)? 100:((uint8_t)((Volume * 255) / 100)))
#define DMA_MAX(x)           (((x) <= DMA_MAX_SIZE)? (x):DMA_MAX_SIZE)


// User Callbacks: user has to implement these functions in his code if they are needed.

uint16_t EVAL_AUDIO_GetSampleCallBack(void);

/* This function is called when the requested data has been completely transferred.
   In Normal mode (when  the define AUDIO_MAL_MODE_NORMAL is enabled) this function
   is called at the end of the whole audio file.
   In circular mode (when  the define AUDIO_MAL_MODE_CIRCULAR is enabled) this
   function is called at the end of the current buffer transmission. */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size);

/* This function is called when half of the requested buffer has been transferred
   This callback is useful in Circular mode only (when AUDIO_MAL_MODE_CIRCULAR
   define is enabled)*/
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size);

/* This function is called when an Interrupt due to transfer error on or peripheral
   error occurs. */
void EVAL_AUDIO_Error_CallBack(void* pData);

/* Codec_TIMEOUT_UserCallback() function is called whenever a timeout condition
   occurs during communication (waiting on an event that doesn't occur, bus
   errors, busy devices ...) on the Codec control interface (I2C).
   You can use the default timeout callback implementation by uncommenting the
   define USE_DEFAULT_TIMEOUT_CALLBACK in stm32f4_discovery_audio_codec.h file.
   Typically the user implementation of this callback should reset I2C peripheral
   and re-initialize communication or in worst case reset all the application. */
uint32_t Codec_TIMEOUT_UserCallback(void);

#endif
