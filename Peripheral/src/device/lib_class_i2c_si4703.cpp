//-------------------------------------------------------------------------------------------------
//
//  File    : lib_class_i2c_si4703.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_SI4703_GLOBAL
#include "lib_class_i2c_si4703.h"
#undef  LIB_SI4703_GLOBAL

//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_USE_GRAFX
#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SI4703_ADDRESS              0x20        // I2C address of Si4703


#define SI4703_I2C_FAIL_MAX         10          // This is the number of attempts we will try connect to the device before returning error


// Register 0x02 - POWER_CFG
#define SI4703_DSMUTE_MASK          0x8000
#define SI4703_DMUTE_MASK           0x4000
#define SI4703_MONO_MASK            0x2000
#define SI4703_SK_MODE_MASK         0x0400
#define SI4703_SEEK_UP_MASK         0x0200
#define SI4703_SEEK_MASK            0x0100

// Register 0x03 - CHANNEL
#define SI4703_TUNE_MASK            0x8000
#define SI4703_CHANNEL_MASK         0x03FF

// Register 0x04 - SYS_CONFIG1
#define SI4703_RDS_MASK             0x1000
#define SI4703_DE_MASK              0x0800
#define SI4703_VOLUME_MASK          0x000F

// Register 0x05 - SYS_CONFIG2
#define SI4703_SPACE1_MASK          0x0020
#define SI4703_SPACE0_MASK          0x0010

// Register 0x0A - STATUS
#define SI4703_RDSR_MASK            0x8000
#define SI4703_STC_MASK             0x4000
#define SI4703_SFBL_MASK            0x2000
#define SI4703_AFCRL_MASK           0x1000
#define SI4703_RDSS_MASK            0x0800
#define SI4703_STEREO_MASK          0x0100
#define SI4703_RSSI_MASK            0x00FF

#define SI4703_NB_OF_READ_REGISTER  16
#define SI4703_NB_OF_WRITE_REGISTER 6

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum SI4703_Register_e  // Register names
{
    SI4703_STATUS            = 0,  // 0x0A
    SI4703_READ_CHANNEL      = 1,  // 0x0B
    SI4703_RDSA              = 2,  // 0x0C
    SI4703_RDSB              = 3,  // 0x0D
    SI4703_RDSC              = 4,  // 0x0E
    SI4703_RDSD              = 5,  // 0x0F
    SI4703_DEVICE_ID         = 6,  // 0x00
    SI4703_CHIP_ID           = 7,  // 0x01
    SI4703_POWER_CFG         = 8,  // 0x02
    SI4703_CHANNEL           = 9,  // 0x03
    SI4703_SYS_CONFIG1       = 10, // 0x04
    SI4703_SYS_CONFIG2       = 11, // 0x05
    SI4703_REGISTER_6        = 12, // 0x06
    SI4703_REGISTER_7        = 13, // 0x07
    SI4703_REGISTER_8        = 14, // 0x08
    SI4703_REGISTER_9        = 15, // 0x09
};

//-------------------------------------------------------------------------------------------------
//
//   Class: SI4703
//
//
//   Description:   Class to handle radio SI4703
//
//-------------------------------------------------------------------------------------------------

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
//  Note(s):        To get the Si4703 into 2 wires mode, SEN needs to be HIGH and SDIO (SDA) needs
//                  to be LOW after a reset.
//
//                  ----!!! WARNING !!!----
//                  SI4703_Radio.InitializeLowLevel must be called before the I2C Initialize port
//
//-------------------------------------------------------------------------------------------------
SystemState_e SI4703::InitializeLowLevel(IO_ID_e RST_Pin, IO_ID_e SDIO_Pin)
{
    IO_PinInit(RST_Pin);                // Initialize and clear the PIN. Default (LOW) is set in bsp_io_def.h
    IO_SetPinLow(SDIO_Pin);             // Make sure pin are high level
    IO_PinInitOutput(SDIO_Pin);         // Init pin to control chip 2 wires mode
    LIB_Delay_mSec(2);
    IO_SetPinHigh(RST_Pin);             // Release RST Pin
    LIB_Delay_mSec(2);
    IO_SetPinHigh(SDIO_Pin);            // Release SDIO Pin
    return SYS_READY;
}

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
SystemState_e SI4703::Initialize(I2C_Driver* pI2C)
{
    // if(this->IsOperational() == false)                                          // Read IO TEA5767 ID
    //{
    //  return false;
    //}

    m_pI2C = pI2C;


//
//The breakout board has SEN pulled high, but also has SDIO pulled high. Therefore, after a normal power up
// The Si4703 will be in an unknown state. RST must be controlled

    this->ReadRegisters();                              // Read the current register set
    // m_Registers[0x07] = 0xBC04;                      // Enable the oscillator, from AN230 page 9, rev 0.5 (DOES NOT WORK, wtf Silicon Labs datasheet?)
    m_Registers[0x07] = 0x8100;                         // Enable the oscillator, from AN230 page 9, rev 0.61 (works)
    this->SaveRegisters();                              // Update
    nOS_Sleep(500);                                     // Wait for clock to settle - from AN230 page 9

    this->ReadRegisters();                              // Read the current register set
    m_Registers[SI4703_POWER_CFG] = 0x4001;                    // Enable the IC
    // m_Registers[POWER_CFG] |= (1 << SI4703_SMUTE) | (1 << SI4703_DMUTE); //Disable Mute, disable softmute
    m_Registers[SI4703_SYS_CONFIG1] |= SI4703_RDS_MASK;        // Enable RDS

// TODO  Create a config page so this can be set at startup within the init parameter
    m_Registers[SI4703_SYS_CONFIG1] |=  SI4703_DE_MASK;        // 50kHz Europe setup
    m_Registers[SI4703_SYS_CONFIG2] |=  SI4703_SPACE0_MASK;    // 100kHz channel spacing for Europe
    m_Registers[SI4703_SYS_CONFIG2] &= ~SI4703_VOLUME_MASK;    // Clear volume bits
    m_Registers[SI4703_SYS_CONFIG2] |= 0x0001;                 // Set volume to lowest
    this->SaveRegisters();                              // Update
    nOS_Sleep(110);                                     // Max powerup time, from datasheet page 13
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetVolume
//
//  Parameter(s):   Volume          Volume Value from 0-15
//
//  Return:         SystemState_e   State
//
//  Description:    Set the volume on the SI4703.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SI4703::SetVolume(uint32_t Volume)
{
    this->ReadRegisters();                              //   Read the current register set

    if(Volume > 15)
    {
        Volume = 15;
    }

    m_Registers[SI4703_SYS_CONFIG2] &= ~SI4703_VOLUME_MASK;    // Clear volume bits
    m_Registers[SI4703_SYS_CONFIG2] |=  Volume;                // Set new volume
    this->SaveRegisters();                              // Update
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetChannel
//
//  Parameter(s):   Channel         Frequency value from 875 to 1080
//
//  Return:         SystemState_e   State
//
//  Description:    Set the frequency (station) on the SI4703.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SI4703::SetChannel(uint32_t Channel)
{
    uint32_t NewChannel;

    // Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
    // 97.3 = 0.2 * Chan + 87.5
    // 9.8 / 0.2 = 49
    NewChannel  = Channel * 10;     // 973 * 10 = 9730
    NewChannel -= 8750;             // 9730 - 8750 = 980
    NewChannel /= 10;               // 980 / 10 = 98

    // These steps come from AN230 page 20 rev 0.5
    this->ReadRegisters();
    m_Registers[SI4703_CHANNEL] &= ~SI4703_CHANNEL_MASK;       // Clear out the channel bits
    m_Registers[SI4703_CHANNEL] |=  NewChannel;                // Mask in the new channel
    m_Registers[SI4703_CHANNEL] |=  SI4703_TUNE_MASK;          // Set the TUNE bit to start
    this->SaveRegisters();

    //delay(60); //Wait 60ms - you can use or skip this delay

    // Poll to see if STC is set
    while(1)
    {
        this->ReadRegisters();

        if((m_Registers[SI4703_STATUS] & SI4703_STC_MASK) != 0)
        {
            break; //Tuning complete!
        }
    }

    this->ReadRegisters();
    m_Registers[SI4703_CHANNEL] &= ~SI4703_TUNE_MASK;   // Clear the tune after a tune has completed
    this->SaveRegisters();

    //Wait for the si4703 to clear the STC as well
    while(1)
    {
        this->ReadRegisters();

        if((m_Registers[SI4703_STATUS] & SI4703_STC_MASK) == 0)
        {
            break;                          // Tuning complete!
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetMono
//
//  Parameter(s):   bool            Mono
//
//  Return:         SystemState_e   State
//
//  Description:    Force mono receiving mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SI4703::SetMono(bool Mono)
{
    this->ReadRegisters(); // Read the current register set

    if(Mono == true) m_Registers[SI4703_POWER_CFG] |=  SI4703_MONO_MASK;
    else             m_Registers[SI4703_POWER_CFG] &= ~SI4703_MONO_MASK;

    return this->SaveRegisters();
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetMute
//
//  Parameter(s):   bool            Mute
//
//  Return:         SystemState_e   State
//
//  Description:    Force mute mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SI4703::SetMute(bool Mute)
{
    this->ReadRegisters(); // Read the current register set

    if(Mute == true) m_Registers[SI4703_POWER_CFG] |=  SI4703_DMUTE_MASK;
    else             m_Registers[SI4703_POWER_CFG] &= ~SI4703_DMUTE_MASK;

    return this->SaveRegisters();
    return SYS_READY;
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
/*
SystemState_e SI4703::SetBand(void)
{
    //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
    m_Registers[REG_4] |= REG_4_BL;

    return this->SaveRegisters();

    // FM mixer for conversion to IF of the US/Europe (87.5 MHz to 108 MHz)
    //                                  and Japanese  (76 MHz to 91 MHz)
    return SYS_READY;
}
*/

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindStations
//
//  Parameter(s):   uint8_t             SearchLevel
//                  TEA5767_Freq_t*    pFrequencyArray
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
/*
SystemState_e SI4703::FindStations(uint8_t SearchLevel, TEA5767_Freq_t* pFrequencyArray, uint16_t* pStationCount)
{
	SystemState_e  State;
	uint8_t        Level;
	bool           Stereo;
	size_t         Count = 0;
	SI4703_Freq_t Frequency;
	SI4703_Freq_t LastStation;

	for(SI4703_Freq_t Frequency = LOW_FREQ;
	    (Frequency <= HIGH_FREQ) && (Count < *pStationCount);
	    Frequency += 20)
    {
	    State = this->SetFrequency(Frequency);
		nOS_Sleep(2000);
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
	}

	if(Count > 0)
    {
        SetFrequency(pFrequencyArray[0]);
	}

	*pStationCount = Count;

	return State;
    return SYS_READY;
}
*/

//-------------------------------------------------------------------------------------------------
//
//  Name:           SeekUp
//
//  Parameter(s):   None
//
//  Return:         uint32_t
//
//  Description:    Start seek mode upwards
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t SI4703::SeekUp(void)
{
	return Seek(SI4703_SEEK_UP);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SeekDown
//
//  Parameter(s):   None
//
//  Return:         uint32_t
//
//  Description:    Start seek mode downwards
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t SI4703::SeekDown(void)
{
	return Seek(SI4703_SEEK_DOWN);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetRDS
//
//  Parameter(s):   pMessage        Buffer to put RDS info in.
//                  TimeOut         How long to wait for RDS string
//
//  Return:         SystemState_e   State
//
//  Description:    Get the radio RDS information.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SI4703::GetRDS(char* pMessage, TickCount_t TimeOut)
{
	TickCount_t StartTick;
    bool        Completed[4]   = {false, false, false, false};
    uint32_t    CompletedCount = 0;

    StartTick = GetTick();

    while((CompletedCount < 4) && TickHasTimeOut(StartTick, TimeOut))
    {
        this->ReadRegisters();

        if((m_Registers[SI4703_STATUS] & SI4703_RDSR_MASK) != 0)
        {
            // ls 2 bits of B determine the 4 letter pairs once we have a full set return
            // if you get nothing after 20 readings return with empty string
            uint16_t b     = m_Registers[SI4703_RDSB];
            int      Index = b & 0x03;

            if((Completed[Index] == 0) && (b < 500))
            {
                Completed[Index] = true;
                CompletedCount++;
                char Dh = char(m_Registers[SI4703_RDSD] >> 8);
                char Dl = char(m_Registers[SI4703_RDSD]);
                pMessage[Index * 2]       = Dh;
                pMessage[(Index * 2) + 1] = Dl;
                // Serial.print(si4703_registers[RDSD]); Serial.print(" ");
                // Serial.print(index);Serial.print(" ");
                // Serial.write(Dh);
                // Serial.write(Dl);
                // Serial.println();
            }

            nOS_Sleep(40); //Wait for the RDS bit to clear
        }
        else
        {
            nOS_Sleep(30); //From AN230, using the polling method 40ms should be sufficient amount of time between checks
        }
    }

    if(TickHasTimeOut(StartTick, TimeOut) == true)
    {
		pMessage[0] = '\0';
		return;
	}

    pMessage[8] = '\0';
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetRadioInfo
//
//  Parameter(s):   pMessage        Buffer to put RDS info in.
//                  TimeOut         How long to wait for RDS string
//
//  Return:         SystemState_e   State
//
//  Description:    Get the radio information, RSSI. Mono or Stereo, Mute, SNR?
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SI4703::GetRadioInfo(SI4703_RadioInfo_t* pInfo)
{
    SystemState_e State;

    if((State = this->ReadRegisters()) == SYS_READY)
    {
        pInfo->Stereo = (m_Registers[SI4703_STATUS] & SI4703_STEREO_MASK)   ? true : false;
        pInfo->RSSI   = uint8_t(m_Registers[SI4703_STATUS] & SI4703_RSSI_MASK);
        pInfo->Mute   = (m_Registers[SI4703_POWER_CFG] & SI4703_DMUTE_MASK) ? true : false;
        pInfo->Mono   = (m_Registers[SI4703_POWER_CFG] & SI4703_MONO_MASK)  ? true : false;
        pInfo->Tuned  = (m_Registers[SI4703_CHANNEL]   & SI4703_TUNE_MASK)  ? true : false;
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
SystemState_e SI4703::ReadRegisters(void)
{
    SystemState_e State;

    // Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
    if((State = m_pI2C->Read(&m_Registers[SI4703_STATUS], SI4703_NB_OF_READ_REGISTER, SI4703_ADDRESS)) == SYS_READY)
    {
      #if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        for(int i = int(SI4703_STATUS); i <= int(SI4703_REGISTER_9); i++)
        {
            LIB_uint16_t_Swap(&m_Registers[i]);
        }
      #endif
    }

    return State;
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
SystemState_e SI4703::SaveRegisters(void)
{
    SystemState_e State;

  #if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    for(int i = int(SI4703_POWER_CFG); i <= int(SI4703_REGISTER_7); i++)
    {
        LIB_uint16_t_Swap(&m_Registers[i]);
    }
  #endif

    // Write the current control registers (0x02 to 0x07) to the Si4703
    // The Si4703 assumes you are writing to 0x02 first, then increments
    State = m_pI2C->Write(&m_Registers[SI4703_POWER_CFG], SI4703_NB_OF_WRITE_REGISTER, SI4703_ADDRESS);

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Seek
//
//  Parameter(s):   Seek    Seek direction:
//                              SI4703_SEEK_DOWN
//                              SI4703_SEEK_UP
//
//  Return:         int       Return the frequency if found one, 0 if failed.
//
//  Description:    Seeks out the next available station.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t SI4703::Seek(SI4703_Seek_e Seek)
{
    int valueSFBL;

    this->ReadRegisters();
    // Set seek mode wrap bit
    m_Registers[SI4703_POWER_CFG] |= (SI4703_SK_MODE_MASK);         // Allow wrap
    //m_Registers[POWER_CFG] &= ~(1 << SI4703_SK_MODE_MASK);        // Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first

    if(Seek == SI4703_SEEK_DOWN)
    {
        m_Registers[SI4703_POWER_CFG] &= ~SI4703_SEEK_UP_MASK;      // Seek down is the default upon reset
    }
    else
    {
        m_Registers[SI4703_POWER_CFG] |= SI4703_SEEK_UP_MASK;       // Set the bit to seek up
    }

    m_Registers[SI4703_POWER_CFG] |= SI4703_SEEK_MASK;              // Start seek
    this->SaveRegisters();                                          // Seeking will now start

    // Poll to see if STC is set
    while(1)
    {
        this->ReadRegisters();

        if((m_Registers[SI4703_STATUS] & SI4703_STC_MASK) != 0)
        {
            break;                                                  // Tuning complete!
        }
    }

    this->ReadRegisters();
    valueSFBL = m_Registers[SI4703_STATUS] & SI4703_SFBL_MASK;      // Store the value of SFBL
    m_Registers[SI4703_POWER_CFG] &= ~SI4703_SEEK_MASK;             // Clear the seek bit after seek has completed
    this->SaveRegisters();

    // Wait for the si4703 to clear the STC as well
    while(1)
    {
        this->ReadRegisters();
        if((m_Registers[SI4703_STATUS] & SI4703_STC_MASK) == 0)
        {
            break;                                                  // Tuning complete!
        }
    }

    if(valueSFBL != 0)                                              // The bit was set indicating we hit a band limit or failed to find a station
    {
        return(0);
    }

    return GetChannel();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Seek
//
//  Parameter(s):   Seek    Seek direction:
//                              SI4703_SEEK_DOWN
//                              SI4703_SEEK_UP
//
//  Return:         Returns a number like 973 for 97.3MHz
//
//  Description:    Reads the current channel
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
    //
    // Returns a number like 973 for 97.3MHz
uint32_t SI4703::GetChannel(void)
{
    uint32_t Channel;

    this->ReadRegisters();
    Channel = m_Registers[SI4703_READ_CHANNEL] & SI4703_CHANNEL_MASK;  // Mask out everything but the lower 10 bits

    //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
    //X = 0.1 * Chan + 87.5
    Channel += 875; //98 + 875 = 973

    return Channel;
}
//-------------------------------------------------------------------------------------------------

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
#endif // DIGINI_USE_GRAFX

