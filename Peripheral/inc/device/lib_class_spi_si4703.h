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
//  Notes:  Library header file for the radio library to control the TEA5767 radio chip.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

// Frequency data type.
typedef uint16_t SI4703_Freq_t;        // Only 16 bits are used for any frequency value (not the real one)

// A structure that contains information about the radio features from the chip.
struct SI4703_Info_t
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

class SI4703
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

extern class   SI4703                   SI4703_Radio;

#ifdef LIB_SI4703_GLOBAL
 class   SI4703                         SI4703_Radio;
#endif

//-------------------------------------------------------------------------------------------------


#ifndef SparkFunSi4703_h
#define SparkFunSi4703_h

#include "Arduino.h"



class Si4703_Breakout
{
  public:
    Si4703_Breakout(int resetPin, int sdioPin, int sclkPin);
    void powerOn();					// call in setup
	void setChannel(int channel);  	// 3 digit channel number
	int seekUp(); 					// returns the tuned channel or 0
	int seekDown(); 				
	void setVolume(int volume); 	// 0 to 15
	void readRDS(char* message, long timeout);	
									// message should be at least 9 chars
									// result will be null terminated
									// timeout in milliseconds
  private:
    int  _resetPin;
	int  _sdioPin;
	int  _sclkPin;
	void si4703_init();
	void readRegisters();
	byte updateRegisters();
	int seek(byte seekDirection);
	int getChannel();
	uint16_t si4703_registers[16]; //There are 16 registers, each 16 bits large
	static const uint16_t  FAIL = 0;
	static const uint16_t  SUCCESS = 1;

	static const int  SI4703 = 0x10; //0b._001.0000 = I2C address of Si4703 - note that the Wire function assumes non-left-shifted I2C address, not 0b.0010.000W
	static const uint16_t  I2C_FAIL_MAX = 10; //This is the number of attempts we will try to contact the device before erroring out
	static const uint16_t  SEEK_DOWN = 0; //Direction used for seeking. Default is down
	static const uint16_t  SEEK_UP = 1;

	//Define the register names
	static const uint16_t  DEVICEID = 0x00;
	static const uint16_t  CHIPID = 0x01;
	static const uint16_t  POWERCFG = 0x02;
	static const uint16_t  CHANNEL = 0x03;
	static const uint16_t  SYSCONFIG1 = 0x04;
	static const uint16_t  SYSCONFIG2 = 0x05;
	static const uint16_t  STATUSRSSI = 0x0A;
	static const uint16_t  READCHAN = 0x0B;
	static const uint16_t  RDSA = 0x0C;
	static const uint16_t  RDSB = 0x0D;
	static const uint16_t  RDSC = 0x0E;
	static const uint16_t  RDSD = 0x0F;

	//Register 0x02 - POWERCFG
	static const uint16_t  SMUTE = 15;
	static const uint16_t  DMUTE = 14;
	static const uint16_t  SKMODE = 10;
	static const uint16_t  SEEKUP = 9;
	static const uint16_t  SEEK = 8;

	//Register 0x03 - CHANNEL
	static const uint16_t  TUNE = 15;

	//Register 0x04 - SYSCONFIG1
	static const uint16_t  RDS = 12;
	static const uint16_t  DE = 11;

	//Register 0x05 - SYSCONFIG2
	static const uint16_t  SPACE1 = 5;
	static const uint16_t  SPACE0 = 4;

	//Register 0x0A - STATUSRSSI
	static const uint16_t  RDSR = 15;
	static const uint16_t  STC = 14;
	static const uint16_t  SFBL = 13;
	static const uint16_t  AFCRL = 12;
	static const uint16_t  RDSS = 11;
	static const uint16_t  STEREO = 8;
};

#endif