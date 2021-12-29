//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_i2c_TEA5767.h
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
//  Notes:  Library header file for the radio library to control the TEA5767 radio chip.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "digini_cfg.h"
#include "lib_class_i2c.h"

//-------------------------------------------------------------------------------------------------
// typedef(s)
//-------------------------------------------------------------------------------------------------

// Frequency data type.
typedef uint16_t TEA5767_Freq_t;        // Only 16 bits are used for any frequency value (not the real one)

// A structure that contains information about the radio features from the chip.
struct TEA5767_Info_t
{
    bool    Active;                     // Receiving is active.
    uint8_t RSSI;                       // Radio Station Strength Information.
    uint8_t SNR;                        // Signal Noise Ratio.
    bool    Tuned;                      // A stable frequency is tuned.
    bool    Mono;                       // Mono mode is on.
    bool    Stereo;                     // Stereo audio is available
    bool    Mute;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class TEA5767
{
    public:

        SystemState_e       Initialize              (I2C_Driver* pI2C);         // Initialize the chip.
        SystemState_e       SetMono                 (bool SwitchOn);            // Control mono/stereo mode of the radio chip
        SystemState_e       SetMute                 (bool SwitchOn);            // Control the mute function of the radio chip
        SystemState_e       SetBand                 (void);

        SystemState_e       SetFrequency            (TEA5767_Freq_t Frequency);
        SystemState_e       GetFrequency            (TEA5767_Freq_t* pFrequency);
        SystemState_e       FindStations            (uint8_t SearchLevel, TEA5767_Freq_t* pFrequencyArray, uint16_t* pStationCount);
        SystemState_e       SeekUp                  (uint8_t SearchLevel);      // Start seek mode upwards
        SystemState_e       SeekDown                (uint8_t SearchLevel);      // Start seek mode downwards
        SystemState_e       GetRadioInfo            (TEA5767_Info_t *info);

    private:

        SystemState_e       ReadRegisters           (void);                     // Read all status & data registers
        SystemState_e       SaveRegisters           (void);                     // Save writable registers back to the chip

        I2C_Driver*         m_pI2C;

        uint8_t             m_Registers[5];                                     // Store the current values of the registers for controlling the radio chip.
        uint8_t             m_Status[5];                                        // Registers with the current status of the radio chip.
        //bool                m_Mono;
        //bool                m_Mute;
        //TEA5767_Freq_t      m_Freq;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

extern class   TEA5767                  TEA5767_Radio;

#ifdef LIB_TEA5767_GLOBAL
 class   TEA5767                        TEA5767_Radio;
#endif

//-------------------------------------------------------------------------------------------------


