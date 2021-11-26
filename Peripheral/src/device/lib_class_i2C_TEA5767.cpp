//-------------------------------------------------------------------------------------------------
//
//  File    : lib_class_i2c_TEA5767.cpp
//  Version : 001
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_TEA5767_GLOBAL
#include "lib_class_i2c_TEA5767.h"
#undef  LIB_TEA5767_GLOBAL

//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_USE_GRAFX
#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// ----- Definitions for the Wire communication

// might be define BSP
#define TEA5767_I2C_ADR         0xC0

// ----- Radio chip specific definitions including the registers

#define QUARTZ                  32768
#define FILTER                  225000


// Number of registers
#define NB_OF_WRITE_REGISTER    (5)
#define NB_OF_READ_REGISTER     (4)

// Define the registers
#define REG_1                   uint8_t(0x00)
#define REG_2                   uint8_t(0x01)
#define REG_3                   uint8_t(0x02)
#define REG_4                   uint8_t(0x03)
#define REG_5                   uint8_t(0x04)

#define REG_1_MUTE              uint8_t(0x80)
#define REG_1_SM                uint8_t(0x40)
#define REG_1_PLL               uint8_t(0x3F)

#define REG_2_PLL               uint8_t(0xFF)

#define REG_3_MS                uint8_t(0x08)
#define REG_3_SSL_LO            uint8_t(0x20)
#define REG_3_SSL_MID           uint8_t(0x40)
#define REG_3_SSL_HI            uint8_t(0x60)
#define REG_3_SSL_MASK          uint8_t(REG_3_SSL_LO | REG_3_SSL_MID |REG_3_SSL_HI)
#define REG_3_SUD               uint8_t(0x80)

#define REG_4_SMUTE             uint8_t(0x08)
#define REG_4_XTAL              uint8_t(0x10)
#define REG_4_BL                uint8_t(0x20)
#define REG_4_STBY              uint8_t(0x40)

#define REG_5_PLLREF            uint8_t(0x80)
#define REG_5_DTC               uint8_t(0x40)

#define STAT_3_STEREO           uint8_t(0x80)
#define STAT_4_ADC              uint8_t(0xF0)
#define STAT_4_ADC_SHIFT        (4)

#define LOW_FREQ                uint16_t(8750)
#define HIGH_FREQ               uint16_t(10800)

//-------------------------------------------------------------------------------------------------
//
//   Class: TEA5767
//
//
//   Description:   Class to handle radio TEA5767
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   I2C_Driver*     pI2C
//
//  Return:         SystemState_e   State
//
//  Description:    Perform chip initialization
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::Initialize(I2C_Driver* pI2C)
{
    TEA5767_Freq_t Stations[20];


    m_pI2C = pI2C;

    // if(this->IsOperational() == false)                                          // Read IO TEA5767 ID
    //{
    //  return false;
    //}

    m_Registers[REG_1] = 0x00;
    m_Registers[REG_2] = 0x00;
    m_Registers[REG_3] = REG_3_SSL_MID;
    m_Registers[REG_4] = REG_4_XTAL | REG_4_SMUTE;
    m_Registers[REG_5] = 0x00;//REG_5_DTC;                             // 75 ms Europe setup


    this->SaveRegisters();



    this->FindStations(REG_3_SSL_LO, &Stations[0], 20);



//    return this->SaveRegisters();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetMono
//
//  Parameter(s):   bool            SwitchOn
//
//  Return:         SystemState_e   State
//
//  Description:    Force mono receiving mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::SetMono(bool SwitchOn)
{
    //m_Mono = SwitchOn;

    if(SwitchOn) m_Registers[REG_3] |=  REG_3_MS;
    else         m_Registers[REG_3] &= ~REG_3_MS;

    return this->SaveRegisters();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetMute
//
//  Parameter(s):   bool            SwitchOn
//
//  Return:         SystemState_e   State
//
//  Description:    Force mute mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::SetMute(bool SwitchOn)
{
    //m_Mute = SwitchOn;

    if(SwitchOn) m_Registers[REG_1] |=  REG_1_MUTE;
    else         m_Registers[REG_1] &= ~REG_1_MUTE;

    return this->SaveRegisters();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetBand
//
//  Parameter(s):   None
//
//  Return:         SystemState_e   State
//
//  Description:    Tune to new a band.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::SetBand(void)
{
    //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
    m_Registers[REG_4] |= REG_4_BL;

    return this->SaveRegisters();

    // FM mixer for conversion to IF of the US/Europe (87.5 MHz to 108 MHz)
    //                                  and Japanese  (76 MHz to 91 MHz)
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetFrequency
//
//  Parameter(s):   TEA5767_Freq*     Return the current frequency.
//
//  Return:         SystemState_e     State
//
//  Description:    Retrieve the real frequency from the chip after automatic tuning.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::GetFrequency(TEA5767_Freq_t* pFrequency)
{
    SystemState_e State;
    uint32_t      _Frequency;

    if((State = this->ReadRegisters()) == SYS_READY)
    {
        _Frequency = ((m_Status[REG_1] & REG_1_PLL) << 8) | m_Status[REG_2];
        _Frequency = ((_Frequency * QUARTZ / 4) - FILTER) / 10000;
        *pFrequency = TEA5767_Freq_t(_Frequency);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetFrequency
//
//  Parameter(s):   TEA5767_Freq        Frequency
//
//  Return:         SystemState_e       State
//
//  Description:    Change the frequency in the chip.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::SetFrequency(TEA5767_Freq_t Frequency)
{
    uint32_t _Frequency;

//unsigned int freqC = (frequency * 1000000 + 225000) / 8192;

    _Frequency = 4 * ((uint32_t(Frequency) * 10000L) + FILTER) / QUARTZ;
    m_Registers[REG_1] = uint8_t(_Frequency >> 8);
    m_Registers[REG_2] = uint8_t(_Frequency);
    return this->SaveRegisters();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindStations
//
//  Parameter(s):   uint8_t             searchLevel
//                  TEA5767_Freq_t**    pFrequencyArray
//                  uint16_t*           StationCount    On call this is the max station to scan
//                                                      Upon return it the number of station found
//
//  Return:         SystemState_e       State
//
//  Description:    Scan for station.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::FindStations(uint8_t SearchLevel, TEA5767_Freq_t* pFrequencyArray, uint16_t* pStationCount)
{
	SystemState_e  State;
	uint8_t        Level;
	bool           Stereo;
	size_t         Count = 0;
	TEA5767_Freq_t Frequency;
	TEA5767_Freq_t LastStation;

	for(TEA5767_Freq_t Frequency = LOW_FREQ;
	    (Frequency <= HIGH_FREQ) && (Count < *pStationCount);
	    Frequency += 20)
    {
	    State = this->SetFrequency(Frequency);
		nOS_Sleep(2000);
		/*
		State  = this->GetFrequency(&Frequency);
        Stereo =(m_Status[REG_3] & STAT_3_STEREO) ? true : false;
        Level  = m_Status[REG_4] >> STAT_4_ADC_SHIFT;

		if((Level >= SearchLevel) && (Stereo == true))
        {
			pFrequencyArray[Count] = Frequency;

			if(LastStation >= (Frequency - 300))        // Make sure it is not a lock outside frequency band
            {
				if(Level <= SearchLevel)
                {
                    pFrequencyArray[Count - 1] = Frequency;
                }
			}
			else
			{
                Count++;
			}

			LastStation = Frequency;
			Level = SearchLevel;
		}
		*/
	}

	if(Count > 0)
    {
        SetFrequency(pFrequencyArray[0]);
	}

	*pStationCount = Count;

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SeekUp
//
//  Parameter(s):   uint8_t SearchLevel
//
//  Return:         SystemState_e       State
//
//  Description:    Start seek mode upwards
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::SeekUp(uint8_t SearchLevel)
{
    m_Registers[REG_1] |=  REG_1_SM;
    m_Registers[REG_3] |=  REG_3_SUD;
    m_Registers[REG_3] &= ~REG_3_SSL_MASK;
    m_Registers[REG_3] |=  SearchLevel;
    this->SaveRegisters();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SeekDown
//
//  Parameter(s):   uint8_t SearchLevel
//
//  Return:         SystemState_e       State
//
//  Description:    Start seek mode downwards
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::SeekDown(uint8_t SearchLevel)
{
    m_Registers[REG_1] |=   REG_1_SM;
    m_Registers[REG_3] &= ~(REG_3_SSL_MASK | REG_3_SUD);
    m_Registers[REG_3] |=   SearchLevel;
    this->SaveRegisters();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetRadioInfo
//
//  Parameter(s):   EA5767_Info_t*  pInfo
//
//  Return:         SystemState_e   State
//
//  Description:    Get the radio information.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::GetRadioInfo(TEA5767_Info_t* pInfo)
{
    SystemState_e State;

    if((State = this->ReadRegisters()) == SYS_READY)
    {
        pInfo->Stereo = (m_Status[REG_3] & STAT_3_STEREO) ? true : false;
        pInfo->RSSI   = (m_Status[REG_4] & STAT_4_ADC) >> 4;
        //pInfo->Mute   = m_Mute;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadRegisters
//
//  Parameter(s):   None
//  Return:         SystemState_e       State
//
//  Description:    Load all status requested registers from to the chip.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::ReadRegisters(void)
{
    return m_pI2C->Read(&m_Status[0], NB_OF_READ_REGISTER, TEA5767_I2C_ADR);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SaveRegisters
//
//  Parameter(s):   None
//
//  Return:         SystemState_e       State
//
//  Description:    Save requested writable registers back to the chip.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e TEA5767::SaveRegisters(void)
{
    return m_pI2C->Write(&m_Registers[0], NB_OF_WRITE_REGISTER, TEA5767_I2C_ADR);
}

//-------------------------------------------------------------------------------------------------
#endif // USE_I2C_DRIVER == DEF_ENABLED
#endif // DIGINI_USE_GRAFX

