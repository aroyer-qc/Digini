//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_i2c_si4703.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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
//  Notes:  Library header file for the radio library to control the si4703 radio chip.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum SI4703_Seek_e
{
    SI4703_SEEK_DOWN = 0,           // Direction used for seeking. Default is down
    SI4703_SEEK_UP   = 1,
};

// A structure that contains information about the radio features from the chip.
struct SI4703_RadioInfo_t
{
    //bool    Active;                     // Receiving is active.
    uint8_t RSSI;                       // Radio Station Strength Information.
    //uint8_t SNR;                        // Signal Noise Ratio.
    bool    Tuned;                      // A stable frequency is tuned.
    bool    Mono;                       // Mono mode is on.
    bool    Stereo;                     // Stereo audio is available
    bool    Mute;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SI4703
{
    public:

        SystemState_e       InitializeLowLevel      (IO_ID_e RST_Pin, IO_ID_e SDIO_Pin);    // Initialize chip at low level (2 Wires).
        SystemState_e       Initialize              (I2C_Driver* pI2C);                     // Initialize the chip.
        void                SetVolume               (uint32_t Volume); 	                    // 0 to 15
        void                SetChannel              (uint32_t channel);  	                // 3 digit channel number
        SystemState_e       SetMono                 (bool Mono);                            // Control mono/stereo mode of the radio chip
        SystemState_e       SetMute                 (bool Mute);                            // Control the mute function of the radio chip
        //SystemState_e       SetBand                 (void);

        //SystemState_e       FindStations            (uint8_t SearchLevel, TEA5767_Freq_t* pFrequencyArray, uint16_t* pStationCount);
        uint32_t            SeekUp                  (void);                                 // Start seek mode upwards
        uint32_t            SeekDown                (void);                                 // Start seek mode downwards
        SystemState_e       GetRadioInfo            (SI4703_RadioInfo_t* pInfo);
        void                GetRDS                  (char* Message, TickCount_t TimeOut);
									// message should be at least 9 chars
									// result will be null terminated
									// timeout in milliseconds

    private:

        SystemState_e       ReadRegisters           (void);                               // Read all status & data registers
        SystemState_e       SaveRegisters           (void);                               // Save writable registers back to the chip
        uint32_t            Seek                    (SI4703_Seek_e Seek);
        uint32_t            GetChannel              (void);

        I2C_Driver*         m_pI2C;
        uint16_t            m_Registers[16];        // Store the current values of the registers for controlling the radio chip.
 };

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_SI4703__
#include "device_var.h"
#undef  __CLASS_SI4703__

//-------------------------------------------------------------------------------------------------

#else // (USE_I2C_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for I2C must be enable and configure to use this device driver")

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
