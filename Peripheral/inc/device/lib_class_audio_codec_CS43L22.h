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
//
// Note(s)
//          If data need to be verified after write to the codec
//                      Put #define CS443L22_VERIFY_WRITTEN_DATA in device_cfg.h
//
//          Mandatory in device_cfg.h : #define CS43L22_CODEC_STANDARD    CS43L22_STANDARD_xxxx
//                      Avaliable option are:
//                          CS443L22_STANDARD_PHILLIPS
//                          CS443L22_STANDARD_MSB
//                          CS443L22_STANDARD_LSB
//
//          Mandatory in device_cfg.h : #define CS43L22_DEFAULT_VOLUME    0-255
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Pre define(s) used in "device_cfg.h"
//-------------------------------------------------------------------------------------------------

#define CS43L22_STANDARD_MSB                0x00
#define CS43L22_STANDARD_PHILLIPS           0x04
#define CS43L22_STANDARD_LSB                0x08

// Codec output DEVICE
#define CS43L22_OUTPUT_DEVICE_SPEAKER       1
#define CS43L22_OUTPUT_DEVICE_HEADPHONE     2
#define CS43L22_OUTPUT_DEVICE_BOTH          3
#define CS43L22_OUTPUT_DEVICE_AUTO          4

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"
#include "./Peripheral/inc/interface/lib_class_audio_interface.h"
#include "device_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define AUDIO_PAUSE                     0
#define AUDIO_RESUME                    1

// MUTE commands
#define AUDIO_MUTE_ON                   1
#define AUDIO_MUTE_OFF                  0

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CS43L22 : public AudioDriverInterface
{
    public:

        SystemState_e       InitializeLowLevel    (IO_ID_e RST_Pin);

        SystemState_e       Initialize            (void* pArg);
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
        IO_ID_e             m_IO_ResetPinID;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_CS43L22__
#include "device_var.h"
#undef  __CLASS_CS43L22__

//-------------------------------------------------------------------------------------------------

// Codec audio Standards
#ifndef CS43L22_CODEC_STANDARD
 #error "Error: No audio communication standard defined in device_cfg.h"
#endif // CS43L22_CODEC_STANDARD

#ifndef CS43L22_OUTPUT_DEVICE
 #define CS43L22_OUTPUT_DEVICE = CS43L22_OUTPUT_DEVICE_HEADPHONE
#endif // CS43L22_OUTPUT_DEVICE

#ifndef CS43L22_DEFAULT_VOLUME
 #error "Error: No default volume found in device_cfg.h"
#endif // CS43L22_DEFAULT_VOLUME

//-------------------------------------------------------------------------------------------------



#if 0


// Audio Transfer mode (DMA, Interrupt or Polling)
#define AUDIO_MAL_MODE_NORMAL         // Uncomment this line to enable the audio Transfer using DMA
// #define AUDIO_MAL_MODE_CIRCULAR    // Uncomment this line to enable the audio Transfer using DMA

// For the DMA modes select the interrupt that will be used
#define AUDIO_MAL_DMA_IT_TC_EN        // Uncomment this line to enable DMA Transfer Complete interrupt
// #define AUDIO_MAL_DMA_IT_HT_EN     // Uncomment this line to enable DMA Half Transfer Complete interrupt
// #define AUDIO_MAL_DMA_IT_TE_EN     // Uncomment this line to enable DMA Transfer Error interrupt


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
/* Uncomment this line to enable verifying data sent to codec after each write
  operation */
/*----------------------------------------------------------------------------*/

#define CODEC_I2S_ADDRESS              0x40003C0C
#define CODEC_I2S_IRQ                  SPI3_IRQn
#define Audio_I2S_IRQHandler           SPI3_IRQHandler

 #define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
 #define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord
 #define DMA_MAX_SIZE                   0xFFFF

 #define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
 #define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF7
 #define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF7
 #define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF7
 #define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF7
 #define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF7

 #define Audio_MAL_I2S_IRQHandler       DMA1_Stream7_IRQHandler

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
/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04
/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2
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
