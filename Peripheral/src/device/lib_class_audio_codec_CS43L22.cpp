//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_audio_codec_CS43L22.cpp
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
//    Note(s) This file provides a set of functions needed to drive the CS43L22 audio codec.
//
//-------------------------------------------------------------------------------------------------

// TODO to complete one day!!


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_CS43L22_GLOBAL
#include "./Digini/Peripheral/inc/device/lib_class_audio_codec_CS43L22.h"
#undef  LIB_CS43L22_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)
#if (USE_I2S_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// I2C Slave address of of the audio codec Cirrus Logic CS43L22
#define CS43L22_I2C_SLAVE_ADDRESS           ((uint8_t)0x94)

// Mask for the bit EN of the I2S CFGR register
#define I2S_ENABLE_MASK                     0x0400

// Delay for the Codec to be correctly reset
#define CODEC_RESET_DELAY                   2

#define VOLUME_CONVERT(Volume)              (((Volume) > 100)? 100:((uint8_t)(((Volume) * 255) / 100)))


// CS43l22 Registers
#define   CS43L22_REG_ID                    0x01
#define   CS43L22_REG_POWER_CTL1            0x02
#define   CS43L22_REG_POWER_CTL2            0x04
#define   CS43L22_REG_CLOCKING_CTL          0x05
#define   CS43L22_REG_INTERFACE_CTL1        0x06
#define   CS43L22_REG_INTERFACE_CTL2        0x07
#define   CS43L22_REG_PASSTHR_A_SELECT      0x08
#define   CS43L22_REG_PASSTHR_B_SELECT      0x09
#define   CS43L22_REG_ANALOG_ZC_SR_SETT     0x0A
#define   CS43L22_REG_PASSTHR_GANG_CTL      0x0C
#define   CS43L22_REG_PLAYBACK_CTL1         0x0D
#define   CS43L22_REG_MISC_CTL              0x0E
#define   CS43L22_REG_PLAYBACK_CTL2         0x0F
#define   CS43L22_REG_PASSTHR_A_VOL         0x14
#define   CS43L22_REG_PASSTHR_B_VOL         0x15
#define   CS43L22_REG_PCMA_VOL              0x1A
#define   CS43L22_REG_PCMB_VOL              0x1B
#define   CS43L22_REG_BEEP_FREQ_ON_TIME     0x1C
#define   CS43L22_REG_BEEP_VOL_OFF_TIME     0x1D
#define   CS43L22_REG_BEEP_TONE_CFG         0x1E
#define   CS43L22_REG_TONE_CTL              0x1F
#define   CS43L22_REG_MASTER_A_VOL          0x20
#define   CS43L22_REG_MASTER_B_VOL          0x21
#define   CS43L22_REG_HEADPHONE_A_VOL       0x22
#define   CS43L22_REG_HEADPHONE_B_VOL       0x23
#define   CS43L22_REG_SPEAKER_A_VOL         0x24
#define   CS43L22_REG_SPEAKER_B_VOL         0x25
#define   CS43L22_REG_CH_MIXER_SWAP         0x26
#define   CS43L22_REG_LIMIT_CTL1            0x27
#define   CS43L22_REG_LIMIT_CTL2            0x28
#define   CS43L22_REG_LIMIT_ATTACK_RATE     0x29
#define   CS43L22_REG_OVF_CLK_STATUS        0x2E
#define   CS43L22_REG_BATT_COMPENSATION     0x2F
#define   CS43L22_REG_VP_BATTERY_LEVEL      0x30
#define   CS43L22_REG_SPEAKER_STATUS        0x31
#define   CS43L22_REG_TEMPMONITOR_CTL       0x32
#define   CS43L22_REG_THERMAL_FOLDBACK      0x33
#define   CS43L22_REG_CHARGE_PUMP_FREQ      0x34

#define   CS43L22_ID_MASK                   0xF8
#define   CS43L22_ID_OFFSET                 0x03

//-------------------------------------------------------------------------------------------------
//
//  Name:           InitializeLowLevel
//
//  Parameter(s):
//
//  Return:
//
//  Description:    Perform low level chip initialization
//
//  Note(s):        ----!!! WARNING !!!----
//                  CS43L22.InitializeLowLevel must be called before the I2C Initialize port
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::InitializeLowLevel(IO_ID_e RST_Pin)
{
    m_IO_ResetPinID = RST_Pin;
    this->Reset();
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void* pArg          Pointer on the type of driver use by this class
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Initializes the audio codec and the control interface.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
// uint32_t cs43l22_Init(uint16_t DeviceAddr, uint16_t OutputMode, uint8_t Volume, uint32_t AudioFreq)
SystemState_e CS43L22::Initialize(void* pArg)
{
    bool    Status = true;
   // uint8_t Standard;

    m_pI2C = (I2C_Driver*)pArg;                                             // I2C already initialize.
    m_IsItStopped = true;

    Status &= Write(CS43L22_REG_POWER_CTL1,     0x01);                      // Keep CODEC powered OFF.
    Status &= SetOutputMode(CS43L22_OUTPUT_DEVICE);                         // Save Output device for mute ON/OFF procedure.
    Status &= Write(CS43L22_REG_POWER_CTL2,     CS43L22_OUTPUT_DEVICE);
    Status &= Write(CS43L22_REG_CLOCKING_CTL,   0x81);                      // Clock configuration: Auto detection.
    Status &= Write(CS43L22_REG_INTERFACE_CTL1, CS43L22_CODEC_STANDARD);    // Set the Slave Mode and the audio Standard.
   // Status &= SetVolume(Volume);                                          // Set the Master volume.

    // If the Speaker is enabled, set the Mono mode and volume attenuation level
  #if CS43L22_OUTPUT_DEVICE == CS43L22_OUTPUT_DEVICE_HEADPHONE
    Status &= Write(CS43L22_REG_PLAYBACK_CTL2, 0x06);           // Set the Speaker Mono mode.
    Status &= Write(CS43L22_REG_SPEAKER_A_VOL, 0x00);           // Set the Speaker attenuation level.
    Status &= Write(CS43L22_REG_SPEAKER_B_VOL, 0x00);
  #endif

    // Additional configuration for the CODEC.
    //      These configurations are done to reduce the time needed for the CODEC to power off.
    //      If these configurations are removed, then a long delay should be added between powering off the CODEC and switching
    //      off the I2S peripheral MCLK clock (which is the operating clock for CODEC).
    //      If this delay is not inserted, then the CODEC will not shut down properly and it results in high noise after shut down.

    Status &= Write(CS43L22_REG_ANALOG_ZC_SR_SETT, 0x00);           // Disable the analog soft ramp.
    Status &= Write(CS43L22_REG_MISC_CTL,          0x04);           // Disable the digital soft ramp.
    Status &= Write(CS43L22_REG_LIMIT_CTL1,        0x00);           // Disable the limiter attack level.
    Status &= Write(CS43L22_REG_TONE_CTL,          0x0F);           // Adjust Bass and Treble levels.
    Status &= Write(CS43L22_REG_PCMA_VOL,          0x0A);           // Adjust PCM volume level.
    Status &= Write(CS43L22_REG_PCMB_VOL,          0x0A);

    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadID
//
//  Parameter(s):   None
//
//  Return:         ID
//
//  Description:    Get the CS43L22 ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint16_t CS43L22::ReadID(void)
{
    uint8_t Value;

    // Initialize the Control interface of the Audio Codec
   // AUDIO_IO_Init();
    m_pI2C->ReadRegister(CS43L22_REG_ID, &Value, m_DeviceAddress);
    Value = (Value & CS43L22_ID_MASK) >> CS43L22_ID_OFFSET;
    return int(Value);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Play
//
//  Parameter(s):    not used parameter
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Start the audio codec.
//
//  Note(s):        For this codec no Play options are required.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::Play(uint16_t* pBuffer, uint16_t Size)
{
    bool Status = true;

    if(m_IsItStopped == true)
    {
        Status &= Write(CS43L22_REG_MISC_CTL, 0x06);            // Enable the digital soft ramp
       // Status &= cs43l22_SetMute(AUDIO_MUTE_OFF);              // Enable Output device
        Status &= Write(CS43L22_REG_POWER_CTL1, 0x9E);          // Power on the Codec
        m_IsItStopped = false;
    }

    // Return communication control value
    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Pause
//
//  Parameter(s):   DeviceAddr: Device address on communication Bus.
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:       Pauses playing on the audio codec.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::Pause(void)
{
    bool Status = true;

    Status &= SetMute(AUDIO_MUTE_ON);                   // Mute the output first
    Status &= Write(CS43L22_REG_POWER_CTL1, 0x01);      // Put the codec in power save mode
    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Resume
//
//  Parameter(s):   None
//
//  Return:         true or false
//
//  Description:    Resumes playing on the audio codec.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::Resume(void)
{
    bool Status = true;

    Status &= SetMute(AUDIO_MUTE_OFF);                  // Unmute the output first
    for(uint32_t index = 0x00; index < 0xFF; index++);
    Status &= Write(CS43L22_REG_POWER_CTL2, m_OutputConfig);
    Status &= Write(CS43L22_REG_POWER_CTL1, 0x9E);              // Exit the Power save mode
    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Stop
//
//  Parameter(s):   uint32_t CodecPdwnMode        Selects the  power down mode.
//  *                                              - CODEC_PDWN_HW: Physically power down the codec.
//
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Stops audio Codec playing. It powers down the codec.
//
//  Note(s):        When resuming from this mode, the codec is set to default configuration
//                    User should re-Initialize the codec in order to play again the audio stream.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::Stop(uint32_t CodecPdwnMode)
{
    bool Status = true;

    VAR_UNUSED(CodecPdwnMode);
    Status &= SetMute(AUDIO_MUTE_ON);                           // Mute the output first
    Status &= Write(CS43L22_REG_MISC_CTL, 0x04);                // Disable the digital soft ramp
    Status &= Write(CS43L22_REG_POWER_CTL1, 0x9F);              // Disable the digital soft ramp
    m_IsItStopped = true;
    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetVolume
//
//  Parameter(s):   uint8_t     Volume:     Byte value from 0 to 255.
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Sets higher or lower the codec volume level.
//
//  Note(s):        Value are adapted to the codec in use
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::SetVolume(uint8_t Volume)
{
    bool Status = true;
    uint8_t convertedvol = VOLUME_CONVERT(Volume);

    // Set the Master volume
    if(Volume > 0xE6)
    {
        Status &= Write(CS43L22_REG_MASTER_A_VOL, convertedvol - 0xE7);
        Status &= Write(CS43L22_REG_MASTER_B_VOL, convertedvol - 0xE7);
    }
    else
    {
        Status &= Write(CS43L22_REG_MASTER_A_VOL, convertedvol + 0x19);
        Status &= Write(CS43L22_REG_MASTER_B_VOL, convertedvol + 0x19);
    }

    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetFrequency
//
//  Parameter(s):   uint32_t AudioFrequency     Audio frequency used to play the audio stream.
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Sets new frequency.
//
//  Note(s):        Feature not supported
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::SetFrequency(uint32_t AudioFrequency)
{
    VAR_UNUSED(AudioFrequency);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetMute
//
//  Parameter(s):   uint32_t Command        AUDIO_MUTE_ON to enable the mute
//                                          AUDIO_MUTE_OFF to disable the
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Enables or disables the mute feature on the audio codec.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::SetMute(uint32_t Command)
{
    bool Status = true;

    // Set the Mute mode
    if(Command == AUDIO_MUTE_ON)
    {
        Status &= Write(CS43L22_REG_POWER_CTL2,      0xFF);
        Status &= Write(CS43L22_REG_HEADPHONE_A_VOL, 0x01);
        Status &= Write(CS43L22_REG_HEADPHONE_B_VOL, 0x01);
    }
    else // AUDIO_MUTE_OFF Disable the Mute
    {
        Status &= Write(CS43L22_REG_HEADPHONE_A_VOL, 0x00);
        Status &= Write(CS43L22_REG_HEADPHONE_B_VOL, 0x00);
        Status &= Write(CS43L22_REG_POWER_CTL2,      m_OutputConfig);
    }

    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetOutputMode
//
//  Parameter(s):   uint8_t Mode    Specifies the audio output target:      OUTPUT_DEVICE_SPEAKER
//                                                                          OUTPUT_DEVICE_HEADPHONE
//                                                                          OUTPUT_DEVICE_BOTH
//                                                                          OUTPUT_DEVICE_AUTO
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Switch dynamically the output target (speaker or headphone).
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::SetOutputMode(uint8_t Mode)
{
    switch(Mode)
    {
        case CS43L22_OUTPUT_DEVICE_SPEAKER:   m_OutputConfig = 0xFA; break; // Speaker ON  & head phone OFF
        case CS43L22_OUTPUT_DEVICE_HEADPHONE: m_OutputConfig = 0xAF; break; // Speaker OFF & head phone ON
        case CS43L22_OUTPUT_DEVICE_BOTH:      m_OutputConfig = 0xAA; break; // Speaker ON  & head phone ON
        default:                              m_OutputConfig = 0x05; break; // Detect automatically
    }

    return Write(CS43L22_REG_POWER_CTL2, m_OutputConfig);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):    Register             Register to write
//                    Data                Data for register
//
//  Return:         SYS_READY or SYS_DEVICE_ERROR
//
//  Description:    Write a single data.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::Write(uint8_t Register, uint8_t Data)
{
    bool Status = true;

    m_pI2C->WriteRegister(Register, Data, m_DeviceAddress);

  #ifdef CS43L22_VERIFY_WRITTEN_DATA
    // Verify that the data has been correctly written
    uint8_t TempValue;
    m_pI2C->ReadRegister(Register, &TempValue, m_DeviceAddress);
    Status &= (TempValue == Register) ? true : false;
  #endif // CS43L22_VERIFY_WRITTEN_DATA

    return (Status) ? SYS_READY : SYS_DEVICE_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Reset
//
//  Parameter(s):   None
//
//  Return:         SYS_READY
//
//  Description:    Resets CS43L22.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e CS43L22::Reset(void)
{
    IO_SetPinLow(m_IO_ResetPinID);      // Initialize and clear the PIN. Default (LOW) is set in bsp_io_def.h
    LIB_Delay_mSec(CODEC_RESET_DELAY);
    IO_SetPinHigh(m_IO_ResetPinID);     // Release Reset
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2S_DRIVER == DEF_ENABLED)
#endif // (USE_I2C_DRIVER == DEF_ENABLED)

#if 0
/*==============================================================================================================================
                                             User NOTES
1. How To use this driver:
--------------------------
   - Configure the options in file stm32f4_discovery_audio_codec.h in the section CONFIGURATION.
      Refer to the sections 2 and 3 to have more details on the possible configurations.

   - Call the function EVAL_AUDIO_Init( OutputDevice: physical output mode (OUTPUT_DEVICE_SPEAKER,
                                        OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_AUTO or OUTPUT_DEVICE_BOTH)
                                        Volume: initial volume to be set (0 is min (mute), 100 is max (100%)
                                        AudioFreq: Audio frequency in Hz (8000, 16000, 22500, 32000 ...) this parameter is relative to the audio file/stream type.                                  )

      This function configures all the hardware required for the audio application (codec, I2C, I2S,
      GPIOs, DMA and interrupt if needed). This function returns 0 if configuration is OK.
      if the returned value is different from 0 or the function is stuck then the communication with
      the codec (try to un-plug the power or reset device in this case).
      + OUTPUT_DEVICE_SPEAKER: only speaker will be set as output for the audio stream.
      + OUTPUT_DEVICE_HEADPHONE: only headphones will be set as output for the audio stream.
      + OUTPUT_DEVICE_AUTO: Selection of output device is made through external switch (implemented
         into the audio jack on the evaluation board). When the Headphone is connected it is used
         as output. When the headphone is disconnected from the audio jack, the output is
         automatically switched to Speaker.
      + OUTPUT_DEVICE_BOTH: both Speaker and Headphone are used as outputs for the audio stream
         at the same time.

   - Call the function EVAL_AUDIO_Play(    pBuffer: pointer to the audio data file address
                                        Size: size of the buffer to be sent in Bytes)
      to start playing (for the first time) from the audio file/stream.

   - Call the function EVAL_AUDIO_PauseResume(
                                         Cmd: AUDIO_PAUSE (or 0) to pause playing or AUDIO_RESUME (or
                                               any value different from 0) to resume playing.
                                         )
       Note. After calling EVAL_AUDIO_PauseResume() function for pause, only EVAL_AUDIO_PauseResume() should be called
          for resume (it is not allowed to call EVAL_AUDIO_Play() in this case).
       Note. This function should be called only when the audio file is played or paused (not stopped).

   - For each mode, you may need to implement the relative callback functions into your code.
      The Callback functions are named EVAL_AUDIO_XXX_CallBack() and only their prototypes are declared in
      the stm32f4_discovery_audio_codec.h file. (refer to the example for more details on the callbacks implementations)

   - To Stop playing, to modify the volume level or to mute, use the functions
       EVAL_AUDIO_Stop(), EVAL_AUDIO_VolumeCtl() and EVAL_AUDIO_Mute().

   - The driver API and the callback functions are at the end of the stm32f4_discovery_audio_codec.h file.


 Driver architecture:
 --------------------
 This driver is composed of three main layers:
   o High Audio Layer: consists of the function API exported in the stm32f4_discovery_audio_codec.h file
     (EVAL_AUDIO_Init(), EVAL_AUDIO_Play() ...)
   o Codec Control layer: consists of the functions API controlling the audio codec (CS43L22) and
     included as local functions in file stm32f4_discovery_audio_codec.c (Codec_Init(), Codec_Play() ...)
   o Media Access Layer (MAL): which consists of functions allowing to access the media containing/
     providing the audio file/stream. These functions are also included as local functions into
     the stm32f4_discovery_audio_codec.c file (Audio_MAL_Init(), Audio_MAL_Play() ...)
  Each set of functions (layer) may be implemented independently of the others and customized when
  needed.

2. Modes description:
---------------------
     + AUDIO_MAL_MODE_NORMAL : is suitable when the audio file is in a memory location.
     + AUDIO_MAL_MODE_CIRCULAR: is suitable when the audio data are read either from a
        memory location or from a device at real time (double buffer could be used).

3. DMA interrupts description:
------------------------------
     + EVAL_AUDIO_IT_TC_ENABLE: Enable this define to use the DMA end of transfer interrupt.
        then, a callback should be implemented by user to perform specific actions
        when the DMA has finished the transfer.
     + EVAL_AUDIO_IT_HT_ENABLE: Enable this define to use the DMA end of half transfer interrupt.
        then, a callback should be implemented by user to perform specific actions
        when the DMA has reached the half of the buffer transfer (generally, it is useful
        to load the first half of buffer while DMA is loading from the second half).
     + EVAL_AUDIO_IT_ER_ENABLE: Enable this define to manage the cases of error on DMA transfer.

4. Known Limitations:
---------------------
   1- When using the Speaker, if the audio file quality is not high enough, the speaker output
      may produce high and uncomfortable noise level. To avoid this issue, to use speaker
      output properly, try to increase audio file sampling rate (typically higher than 48KHz).
      This operation will lead to larger file size.
   2- Communication with the audio codec (through I2C) may be corrupted if it is interrupted by some
      user interrupt routines (in this case, interrupts could be disabled just before the start of
      communication then re-enabled when it is over). Note that this communication is only done at
      the configuration phase (EVAL_AUDIO_Init() or EVAL_AUDIO_Stop()) and when Volume control modification is
      performed (EVAL_AUDIO_VolumeCtl() or EVAL_AUDIO_Mute()). When the audio data is played, no communication is
      required with the audio codec.
  3- Parsing of audio file is not implemented (in order to determine audio file properties: Mono/Stereo, Data size,
     File size, Audio Frequency, Audio Data header size ...). The configuration is fixed for the given audio file.
  4- Mono audio streaming is not supported (in order to play mono audio streams, each data should be sent twice
     on the I2S or should be duplicated on the source buffer. Or convert the stream in stereo before playing).
  5- Supports only 16-bit audio data size.
===============================================================================================================================*/



/* This structure is declared global because it is handled by two different functions */
DMA_InitTypeDef DMA_InitStructure;
DMA_InitTypeDef AUDIO_MAL_DMA_InitStructure;

uint32_t AudioTotalSize = 0xFFFF; /* This variable holds the total size of the audio file */
uint32_t AudioRemSize   = 0xFFFF; /* This variable holds the remaining data in audio file */
uint16_t *CurrentPos ;             /* This variable holds the current position of audio pointer */

__IO uint32_t  CODECTimeout = CODEC_LONG_TIMEOUT;
__IO uint8_t OutputDev = 0;


__IO uint32_t CurrAudioInterface = AUDIO_INTERFACE_I2S; //AUDIO_INTERFACE_DAC

static void Audio_MAL_IRQHandler(void);
/*-----------------------------------
                           Audio Codec functions
                                    ------------------------------------------*/
/* High Layer codec functions */
static uint32_t Codec_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
static uint32_t Codec_DeInit(void);
static uint32_t Codec_Play(void);
static uint32_t Codec_PauseResume(uint32_t Cmd);
static uint32_t Codec_Stop(uint32_t Cmd);
static uint32_t Codec_VolumeCtrl(uint8_t Volume);
static uint32_t Codec_Mute(uint32_t Cmd);
/* Low layer codec functions */
static void     Codec_CtrlInterface_Init(void);
static void     Codec_CtrlInterface_DeInit(void);
static void     Codec_AudioInterface_Init(uint32_t AudioFreq);
static void     Codec_AudioInterface_DeInit(void);
static void     Codec_Reset(void);
static uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue);
static uint32_t Codec_ReadRegister(uint8_t RegisterAddr);
static void     Codec_GPIO_Init(void);
static void     Codec_GPIO_DeInit(void);

/*-----------------------------------
                   MAL (Media Access Layer) functions
                                    ------------------------------------------*/
/* Peripherals configuration functions */
static void     Audio_MAL_Init(void);
static void     Audio_MAL_DeInit(void);
static void     Audio_MAL_PauseResume(uint32_t Cmd, uint32_t Addr);
static void     Audio_MAL_Stop(void);
/*----------------------------------------------------------------------------*/

 /* DMA Stream definitions */
 uint32_t AUDIO_MAL_DMA_CLOCK    = AUDIO_I2S_DMA_CLOCK;
 DMA_Stream_TypeDef * AUDIO_MAL_DMA_STREAM   = AUDIO_I2S_DMA_STREAM ;
 uint32_t AUDIO_MAL_DMA_DREG     = AUDIO_I2S_DMA_DREG;
 uint32_t AUDIO_MAL_DMA_CHANNEL  = AUDIO_I2S_DMA_CHANNEL;
 uint32_t AUDIO_MAL_DMA_IRQ      = AUDIO_I2S_DMA_IRQ  ;
 uint32_t AUDIO_MAL_DMA_FLAG_TC  = AUDIO_I2S_DMA_FLAG_TC;
 uint32_t AUDIO_MAL_DMA_FLAG_HT  = AUDIO_I2S_DMA_FLAG_HT;
 uint32_t AUDIO_MAL_DMA_FLAG_FE  = AUDIO_I2S_DMA_FLAG_FE;
 uint32_t AUDIO_MAL_DMA_FLAG_TE  = AUDIO_I2S_DMA_FLAG_TE;
 uint32_t AUDIO_MAL_DMA_FLAG_DME = AUDIO_I2S_DMA_FLAG_DME;

/**
  * @brief  Set the current audio interface (I2S or DAC).
  * @param  Interface: AUDIO_INTERFACE_I2S or AUDIO_INTERFACE_DAC
  * @retval None
  */
void EVAL_AUDIO_SetAudioInterface(uint32_t Interface)
{
  CurrAudioInterface = Interface;

  if (CurrAudioInterface == AUDIO_INTERFACE_I2S)
  {
    /* DMA Stream definitions */
    AUDIO_MAL_DMA_CLOCK    = AUDIO_I2S_DMA_CLOCK;
    AUDIO_MAL_DMA_STREAM   = AUDIO_I2S_DMA_STREAM;
    AUDIO_MAL_DMA_DREG     = AUDIO_I2S_DMA_DREG;
    AUDIO_MAL_DMA_CHANNEL  = AUDIO_I2S_DMA_CHANNEL;
    AUDIO_MAL_DMA_IRQ      = AUDIO_I2S_DMA_IRQ  ;
    AUDIO_MAL_DMA_FLAG_TC  = AUDIO_I2S_DMA_FLAG_TC;
    AUDIO_MAL_DMA_FLAG_HT  = AUDIO_I2S_DMA_FLAG_HT;
    AUDIO_MAL_DMA_FLAG_FE  = AUDIO_I2S_DMA_FLAG_FE;
    AUDIO_MAL_DMA_FLAG_TE  = AUDIO_I2S_DMA_FLAG_TE;
    AUDIO_MAL_DMA_FLAG_DME = AUDIO_I2S_DMA_FLAG_DME;
  }
  else if (Interface == AUDIO_INTERFACE_DAC)
  {
    /* DMA Stream definitions */
    AUDIO_MAL_DMA_CLOCK    = AUDIO_DAC_DMA_CLOCK;
    AUDIO_MAL_DMA_STREAM   = AUDIO_DAC_DMA_STREAM;
    AUDIO_MAL_DMA_DREG     = AUDIO_DAC_DMA_DREG;
    AUDIO_MAL_DMA_CHANNEL  = AUDIO_DAC_DMA_CHANNEL;
    AUDIO_MAL_DMA_IRQ      = AUDIO_DAC_DMA_IRQ  ;
    AUDIO_MAL_DMA_FLAG_TC  = AUDIO_DAC_DMA_FLAG_TC;
    AUDIO_MAL_DMA_FLAG_HT  = AUDIO_DAC_DMA_FLAG_HT;
    AUDIO_MAL_DMA_FLAG_FE  = AUDIO_DAC_DMA_FLAG_FE;
    AUDIO_MAL_DMA_FLAG_TE  = AUDIO_DAC_DMA_FLAG_TE;
    AUDIO_MAL_DMA_FLAG_DME = AUDIO_DAC_DMA_FLAG_DME;
  }
}

/**
  * @brief  Configure the audio peripherals.
  * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  /* Perform low layer Codec initialization */
  if (Codec_Init(OutputDevice, VOLUME_CONVERT(Volume), AudioFreq) != 0)
  {
    return 1;
  }
  else
  {
    /* I2S data transfer preparation:
    Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
    Audio_MAL_Init();

    /* Return 0 when all operations are OK */
    return 0;
  }
}

/**
  * @brief  Starts playing audio stream from a data buffer for a determined size.
  * @param  pBuffer: Pointer to the buffer
  * @param  Size: Number of audio data BYTES.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Play(uint16_t* pBuffer, uint32_t Size)
{
  /* Set the total number of data to be played (count in half-word) */
  AudioTotalSize = Size;

  /* Call the audio Codec Play function */
  Codec_Play();

  /* Update the Media layer and enable it for play */
  Audio_MAL_Play((uint32_t)pBuffer, (uint32_t)(DMA_MAX(Size/4)));

  /* Update the remaining number of data to be played */
  AudioRemSize = (Size/2) - DMA_MAX(AudioTotalSize);

  /* Update the current audio pointer position */
  CurrentPos = pBuffer + DMA_MAX(AudioTotalSize);

  return 0;
}

/**
  * @brief  This function Pauses or Resumes the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used. In all cases the I2S
  *         peripheral is disabled.
  *
  * @WARNING When calling EVAL_AUDIO_PauseResume() function for pause, only
  *          this function should be called for resume (use of EVAL_AUDIO_Play()
  *          function for resume could lead to unexpected behavior).
  *
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_PauseResume(uint32_t Cmd)
{
  /* Call the Audio Codec Pause/Resume function */
  if (Codec_PauseResume(Cmd) != 0)
  {
    return 1;
  }

    /* Call the Media layer pause/resume function */
    Audio_MAL_PauseResume(Cmd, 0);

    /* Return 0 if all operations are OK */
    return 0;
}

/**
  * @brief  Stops audio playing and Power down the Audio Codec.
  * @param  Option: could be one of the following parameters
  *           - CODEC_PDWN_SW: for software power off (by writing registers).
  *                            Then no need to reconfigure the Codec after power on.
  *           - CODEC_PDWN_HW: completely shut down the codec (physically).
  *                            Then need to reconfigure the Codec after power on.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Stop(uint32_t Option)
{
  /* Call Audio Codec Stop function */
  if (Codec_Stop(Option) != 0)
  {
    return 1;
  }
    /* Call Media layer Stop function */
    Audio_MAL_Stop();

    /* Update the remaining data number */
    AudioRemSize = AudioTotalSize;

    /* Return 0 when all operations are correctly done */
    return 0;
}

/**
  * @brief  Controls the current audio volume level.
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for
  *         Mute and 100 for Max volume level).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_VolumeCtl(uint8_t Volume)
{
  /* Call the codec volume control function with converted volume value */
  return (Codec_VolumeCtrl(VOLUME_CONVERT(Volume)));
}

/**
  * @brief  Enables or disables the MUTE mode by software
  * @param  Command: could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to
  *         unmute the codec and restore previous volume level.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t EVAL_AUDIO_Mute(uint32_t Cmd)
{
  /* Call the Codec Mute function */
  return (Codec_Mute(Cmd));
}

/**
  * @brief  This function handles main Media layer interrupt.
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
static void Audio_MAL_IRQHandler(void)
{
#ifndef AUDIO_MAL_MODE_NORMAL
  uint16_t *pAddr = (uint16_t *)CurrentPos;
  uint32_t Size = AudioRemSize;
#endif /* AUDIO_MAL_MODE_NORMAL */

#ifdef AUDIO_MAL_DMA_IT_TC_EN
  /* Transfer complete interrupt */
  if (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC) != RESET)
  {
 #ifdef AUDIO_MAL_MODE_NORMAL
    /* Check if the end of file has been reached */
    if (AudioRemSize > 0)
    {
      /* Wait the DMA Stream to be effectively disabled */
      while (DMA_GetCmdStatus(AUDIO_MAL_DMA_STREAM) != DISABLE)
      {}

      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);

      /* Re-Configure the buffer address and size */
      DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) CurrentPos;
      DMA_InitStructure.DMA_BufferSize = (uint32_t) (DMA_MAX(AudioRemSize));

      /* Configure the DMA Stream with the new parameters */
      DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);

      /* Enable the I2S DMA Stream*/
      DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);

      /* Update the current pointer position */
      CurrentPos += DMA_MAX(AudioRemSize);

      /* Update the remaining number of data to be played */
      AudioRemSize -= DMA_MAX(AudioRemSize);
        /* Enable the I2S DMA Stream*/
      DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
    }
    else
    {
      /* Disable the I2S DMA Stream*/
      DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);

      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);

      /* Manage the remaining file size and new address offset: This function
      should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */
      EVAL_AUDIO_TransferComplete_CallBack((uint32_t)CurrentPos, 0);
    }

 #elif defined(AUDIO_MAL_MODE_CIRCULAR)
    /* Manage the remaining file size and new address offset: This function
       should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */
    EVAL_AUDIO_TransferComplete_CallBack(pAddr, Size);

    /* Clear the Interrupt flag */
    DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);
 #endif /* AUDIO_MAL_MODE_NORMAL */
  }
#endif /* AUDIO_MAL_DMA_IT_TC_EN */

#ifdef AUDIO_MAL_DMA_IT_HT_EN
  /* Half Transfer complete interrupt */
  if (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_HT) != RESET)
  {
    /* Manage the remaining file size and new address offset: This function
       should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */
    EVAL_AUDIO_HalfTransfer_CallBack((uint32_t)pAddr, Size);

    /* Clear the Interrupt flag */
    DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_HT);
  }
#endif /* AUDIO_MAL_DMA_IT_HT_EN */

#ifdef AUDIO_MAL_DMA_IT_TE_EN
  /* FIFO Error interrupt */
  if((DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TE) != RESET) ||
     (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_FE) != RESET) ||
     (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_DME) != RESET))

  {
    /* Manage the error generated on DMA FIFO: This function
       should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */
    EVAL_AUDIO_Error_CallBack((uint32_t*)&pAddr);

    /* Clear the Interrupt flag */
    DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TE | AUDIO_MAL_DMA_FLAG_FE | AUDIO_MAL_DMA_FLAG_DME);
  }
#endif /* AUDIO_MAL_DMA_IT_TE_EN */
}

/**
  * @brief  I2S interrupt management
  * @param  None
  * @retval None
  */
void Audio_I2S_IRQHandler(void)
{
  /* Check on the I2S TXE flag */
  if (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != RESET)
  {
    if (CurrAudioInterface == AUDIO_INTERFACE_DAC)
    {
      /* Wirte data to the DAC interface */
      DAC_SetChannel1Data(DAC_Align_12b_L, EVAL_AUDIO_GetSampleCallBack());
    }

    /* Send dummy data on I2S to avoid the underrun condition */
    SPI_I2S_SendData(CODEC_I2S, EVAL_AUDIO_GetSampleCallBack());
  }
}
/*========================

                CS43L22 Audio Codec Control Functions
                                                ==============================*/
/**
  * @brief  Initializes the audio codec and all related interfaces (control
  *         interface: I2C and audio interface: I2S)
  * @param  OutputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  uint32_t counter = 0;

  /* Initialize the Control interface of the Audio Codec */
  Codec_CtrlInterface_Init();

  /* Keep Codec powered OFF */
  counter += Codec_WriteRegister(0x02, 0x01);

  counter += Codec_WriteRegister(0x04, 0xAF); /* SPK always OFF & HP always ON */
  OutputDev = 0xAF;

  /* Clock configuration: Auto detection */
  counter += Codec_WriteRegister(0x05, 0x81);

  /* Set the Slave Mode and the audio Standard */
  counter += Codec_WriteRegister(0x06, CODEC_STANDARD);

  /* Set the Master volume */
  Codec_VolumeCtrl(Volume);

  if (CurrAudioInterface == AUDIO_INTERFACE_DAC)
  {
    /* Enable the PassThrough on AIN1A and AIN1B */
    counter += Codec_WriteRegister(0x08, 0x01);
    counter += Codec_WriteRegister(0x09, 0x01);

    /* Route the analog input to the HP line */
    counter += Codec_WriteRegister(0x0E, 0xC0);

    /* Set the Passthough volume */
    counter += Codec_WriteRegister(0x14, 0x00);
    counter += Codec_WriteRegister(0x15, 0x00);
  }

  /* Power on the Codec */
  counter += Codec_WriteRegister(0x02, 0x9E);

  /* Additional configuration for the CODEC. These configurations are done to reduce
      the time needed for the Codec to power off. If these configurations are removed,
      then a long delay should be added between powering off the Codec and switching
      off the I2S peripheral MCLK clock (which is the operating clock for Codec).
      If this delay is not inserted, then the codec will not shut down properly and
      it results in high noise after shut down. */

  /* Disable the analog soft ramp */
  counter += Codec_WriteRegister(0x0A, 0x00);
  if (CurrAudioInterface != AUDIO_INTERFACE_DAC)
  {
    /* Disable the digital soft ramp */
    counter += Codec_WriteRegister(0x0E, 0x04);
  }
  /* Disable the limiter attack level */
  counter += Codec_WriteRegister(0x27, 0x00);
  /* Adjust Bass and Treble levels */
  counter += Codec_WriteRegister(0x1F, 0x0F);
  /* Adjust PCM volume level */
  counter += Codec_WriteRegister(0x1A, 0x0A);
  counter += Codec_WriteRegister(0x1B, 0x0A);

  /* Configure the I2S peripheral */
  Codec_AudioInterface_Init(AudioFreq);

  /* Return communication control value */
  return counter;
}

/**
  * @brief  Start the audio Codec play feature.
  * @note   For this codec no Play options are required.
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_Play(void)
{
  /*
     No actions required on Codec level for play command
     */

  /* Return communication control value */
  return 0;
}

/**
  * @brief  Pauses and resumes playing on the audio codec.
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume.
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_PauseResume(uint32_t Cmd)
{
  uint32_t counter = 0;

  /* Pause the audio file playing */
  if (Cmd == AUDIO_PAUSE)
  {
    /* Mute the output first */
    counter += Codec_Mute(AUDIO_MUTE_ON);

    /* Put the Codec in Power save mode */
    counter += Codec_WriteRegister(0x02, 0x01);
  }
  else /* AUDIO_RESUME */
  {
    /* Unmute the output first */
    counter += Codec_Mute(AUDIO_MUTE_OFF);

    counter += Codec_WriteRegister(0x04, OutputDev);

    /* Exit the Power save mode */
    counter += Codec_WriteRegister(0x02, 0x9E);
  }

  return counter;
}

/**
  * @brief  Stops audio Codec playing. It powers down the codec.
  * @param  CodecPdwnMode: selects the  power down mode.
  *          - CODEC_PDWN_SW: only mutes the audio codec. When resuming from this
  *                           mode the codec keeps the previous initialization
  *                           (no need to re-Initialize the codec registers).
  *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from this
  *                           mode, the codec is set to default configuration
  *                           (user should re-Initialize the codec in order to
  *                            play again the audio stream).
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_Stop(uint32_t CodecPdwnMode)
{
  uint32_t counter = 0;

  /* Mute the output first */
  Codec_Mute(AUDIO_MUTE_ON);

  if (CodecPdwnMode == CODEC_PDWN_SW)
  {
    /* Power down the DAC and the speaker (PMDAC and PMSPK bits)*/
    counter += Codec_WriteRegister(0x02, 0x9F);
  }
  else /* CODEC_PDWN_HW */
  {
    /* Power down the DAC components */
    counter += Codec_WriteRegister(0x02, 0x9F);

    /* Wait at least 100us */
    Delay(0xFFF);

    /* Reset The pin */
    GPIO_WriteBit(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, Bit_RESET);
  }

  return counter;
}

#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None
  * @retval None
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{
  /* Block communication and all processes */
  while (1)
  {
  }
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */
/*========================

                Audio MAL Interface Control Functions

                                                ==============================*/

/**
  * @brief  Initializes and prepares the Media to perform audio data transfer
  *         from Media to the I2S peripheral.
  * @param  None
  * @retval None
  */
static void Audio_MAL_Init(void)
{

#ifdef I2S_INTERRUPT
  NVIC_InitTypeDef   NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = SPI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_TXE, ENABLE);

  I2S_Cmd(SPI3, ENABLE);
#else
#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
  NVIC_InitTypeDef NVIC_InitStructure;
#endif

  if (CurrAudioInterface == AUDIO_INTERFACE_I2S)
  {
    /* Enable the DMA clock */
    RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, ENABLE);

    /* Configure the DMA Stream */
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
    DMA_DeInit(AUDIO_MAL_DMA_STREAM);
    /* Set the parameters to be configured */
    DMA_InitStructure.DMA_Channel = AUDIO_MAL_DMA_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_MAL_DMA_DREG;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = AUDIO_MAL_DMA_PERIPH_DATA_SIZE;
    DMA_InitStructure.DMA_MemoryDataSize = AUDIO_MAL_DMA_MEM_DATA_SIZE;
#ifdef AUDIO_MAL_MODE_NORMAL
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
#else
#error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);

    /* Enable the selected DMA interrupts (selected in "stm32f4_discovery_eval_audio_codec.h" defines) */
#ifdef AUDIO_MAL_DMA_IT_TC_EN
    DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TC_EN */
#ifdef AUDIO_MAL_DMA_IT_HT_EN
    DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_HT_EN */
#ifdef AUDIO_MAL_DMA_IT_TE_EN
    DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TE_EN */

#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
    /* I2S DMA IRQ Channel configuration */
    NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
  }

#ifdef DAC_USE_I2S_DMA
  else
  {
    /* Enable the DMA clock */
    RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, ENABLE);

    /* Configure the DMA Stream */
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
    DMA_DeInit(AUDIO_MAL_DMA_STREAM);
    /* Set the parameters to be configured */
    DMA_InitStructure.DMA_Channel = AUDIO_MAL_DMA_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_MAL_DMA_DREG;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = AUDIO_MAL_DMA_PERIPH_DATA_SIZE;
    DMA_InitStructure.DMA_MemoryDataSize = AUDIO_MAL_DMA_MEM_DATA_SIZE;
#ifdef AUDIO_MAL_MODE_NORMAL
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
#else
#error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);

    /* Enable the selected DMA interrupts (selected in "stm32f4_discovery_eval_audio_codec.h" defines) */
#ifdef AUDIO_MAL_DMA_IT_TC_EN
    DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TC_EN */
#ifdef AUDIO_MAL_DMA_IT_HT_EN
    DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_HT_EN */
#ifdef AUDIO_MAL_DMA_IT_TE_EN
    DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TE_EN */

#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
    /* I2S DMA IRQ Channel configuration */
    NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
  }
#endif /* DAC_USE_I2S_DMA */

    /* Enable the I2S DMA request */
    SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);
#endif
}

/**
  * @brief  Starts playing audio stream from the audio Media.
  * @param  None
  * @retval None
  */
void Audio_MAL_Play(uint32_t Addr, uint32_t Size)
{
  if (CurrAudioInterface == AUDIO_INTERFACE_I2S)
  {
    /* Configure the buffer address and size */
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Addr;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)Size/2;

    /* Configure the DMA Stream with the new parameters */
    DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);

    /* Enable the I2S DMA Stream*/
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
  }
#ifndef DAC_USE_I2S_DMA
  else
  {
    /* Configure the buffer address and size */
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Addr;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)Size;

    /* Configure the DMA Stream with the new parameters */
    DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);

    /* Enable the I2S DMA Stream*/
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
  }
#endif /* DAC_USE_I2S_DMA */

  /* If the I2S peripheral is still not enabled, enable it */
  if ((CODEC_I2S->I2SCFGR & I2S_ENABLE_MASK) == 0)
  {
    I2S_Cmd(CODEC_I2S, ENABLE);
  }
}

/**
  * @brief  Pauses or Resumes the audio stream playing from the Media.
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *              from 0) to resume.
  * @param  Addr: Address from/at which the audio stream should resume/pause.
  * @retval None
  */
static void Audio_MAL_PauseResume(uint32_t Cmd, uint32_t Addr)
{
  /* Pause the audio file playing */
  if (Cmd == AUDIO_PAUSE)
  {
    /* Disable the I2S DMA request */
    SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, DISABLE);

    /* Pause the I2S DMA Stream
        Note. For the STM32F40x devices, the DMA implements a pause feature,
              by disabling the stream, all configuration is preserved and data
              transfer is paused till the next enable of the stream.
              This feature is not available on STM32F40x devices. */
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
  }
  else /* AUDIO_RESUME */
  {
    /* Enable the I2S DMA request */
    SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);

    /* Resume the I2S DMA Stream
        Note. For the STM32F40x devices, the DMA implements a pause feature,
              by disabling the stream, all configuration is preserved and data
              transfer is paused till the next enable of the stream.
              This feature is not available on STM32F40x devices. */
    DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);

    /* If the I2S peripheral is still not enabled, enable it */
    if ((CODEC_I2S->I2SCFGR & I2S_ENABLE_MASK) == 0)
    {
      I2S_Cmd(CODEC_I2S, ENABLE);
    }
  }
}

/**
  * @brief  Stops audio stream playing on the used Media.
  * @param  None
  * @retval None
  */
static void Audio_MAL_Stop(void)
{
  /* Stop the Transfer on the I2S side: Stop and disable the DMA stream */
  DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);

  /* Clear all the DMA flags for the next transfer */
  DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC |AUDIO_MAL_DMA_FLAG_HT | \
                                  AUDIO_MAL_DMA_FLAG_FE | AUDIO_MAL_DMA_FLAG_TE);

  /*
           The I2S DMA requests are not disabled here.
                                                            */

  /* In all modes, disable the I2S peripheral */
  I2S_Cmd(CODEC_I2S, DISABLE);
}


#endif
