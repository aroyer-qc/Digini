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
#include "lib_class_spi_si4703.h"
#undef  LIB_SI4703_GLOBAL

//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_USE_GRAFX
#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define  SI4703 = 0x10; //0b._001.0000 = I2C address of Si4703 - note that the Wire function assumes non-left-shifted I2C address, not 0b.0010.000W
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
SystemState_e SI4703::Initialize(SPI_Driver* pSPI)
{
// done in init of SPI.. but need to do SEN
/*
Si4703_Breakout::Si4703_Breakout(int resetPin, int sdioPin, int sclkPin)
{
  _resetPin = resetPin;
  _sdioPin = sdioPin;
  _sclkPin = sclkPin;
}
*/

/*
    SI4703_Freq_t Stations[20];
uint16_t NumberOfStation = 20;

    m_pSPI = pSPI;

    // if(this->IsOperational() == false)                                          // Read IO TEA5767 ID
    //{
    //  return false;
    //}


    this->SaveRegisters();
    this->FindStations(REG_3_SSL_LO, &Stations[0], &NumberOfStation);
*/  
  return SYS_READY;//this->SaveRegisters();
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
SystemState_e SI4703::SetMono(bool SwitchOn)
{
/*
    //m_Mono = SwitchOn;

    if(SwitchOn) m_Registers[REG_3] |=  REG_3_MS;
    else         m_Registers[REG_3] &= ~REG_3_MS;

    return this->SaveRegisters();
    */ 
    return SYS_READY;
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
SystemState_e SI4703::SetMute(bool SwitchOn)
{
/*
    //m_Mute = SwitchOn;

    if(SwitchOn) m_Registers[REG_1] |=  REG_1_MUTE;
    else         m_Registers[REG_1] &= ~REG_1_MUTE;

    return this->SaveRegisters();
    */ 
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
SystemState_e SI4703::SetBand(void)
{
/*
    //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
    m_Registers[REG_4] |= REG_4_BL;

    return this->SaveRegisters();

    // FM mixer for conversion to IF of the US/Europe (87.5 MHz to 108 MHz)
    //                                  and Japanese  (76 MHz to 91 MHz)
    */ 
    return SYS_READY;
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
SystemState_e SI4703::GetFrequency(TEA5767_Freq_t* pFrequency)
{
    /*
    SystemState_e State;
    uint32_t      _Frequency;

    if((State = this->ReadRegisters()) == SYS_READY)
    {
        _Frequency = ((m_Status[REG_1] & REG_1_PLL) << 8) | m_Status[REG_2];
        _Frequency = ((_Frequency * QUARTZ / 4) - FILTER) / 10000;
        *pFrequency = TEA5767_Freq_t(_Frequency);
    }

    return State;
    */ 
    return SYS_READY;
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
SystemState_e SI4703::SetFrequency(TEA5767_Freq_t Frequency)
{
/*
    uint32_t _Frequency;

//unsigned int freqC = (frequency * 1000000 + 225000) / 8192;

    _Frequency = 4 * ((uint32_t(Frequency) * 10000L) + FILTER) / QUARTZ;
    m_Registers[REG_1] = uint8_t(_Frequency >> 8);
    m_Registers[REG_2] = uint8_t(_Frequency);
    return this->SaveRegisters();
    */ 
    return SYS_READY;
}

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
SystemState_e SI4703::FindStations(uint8_t SearchLevel, TEA5767_Freq_t* pFrequencyArray, uint16_t* pStationCount)
{
/*
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
    */ 
    return SYS_READY;
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
SystemState_e SI4703::SeekUp(uint8_t SearchLevel)
{
/*
    m_Registers[REG_1] |=  REG_1_SM;
    m_Registers[REG_3] |=  REG_3_SUD;
    m_Registers[REG_3] &= ~REG_3_SSL_MASK;
    m_Registers[REG_3] |=  SearchLevel;
    return this->SaveRegisters();
    */ 
    return SYS_READY;
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
SystemState_e SI4703::SeekDown(uint8_t SearchLevel)
{
    /*
    m_Registers[REG_1] |=   REG_1_SM;
    m_Registers[REG_3] &= ~(REG_3_SSL_MASK | REG_3_SUD);
    m_Registers[REG_3] |=   SearchLevel;
    return this->SaveRegisters();
    */ 
    return SYS_READY;
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
SystemState_e SI4703::GetRadioInfo(TEA5767_Info_t* pInfo)
{
/*
    SystemState_e State;

    if((State = this->ReadRegisters()) == SYS_READY)
    {
        pInfo->Stereo = (m_Status[REG_3] & STAT_3_STEREO) ? true : false;
        pInfo->RSSI   = (m_Status[REG_4] & STAT_4_ADC) >> 4;
        //pInfo->Mute   = m_Mute;
    }

    return State;
*/
    return SYS_READY;
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
    //return m_pSPI->Read(&m_Status[0], NB_OF_READ_REGISTER, SI4703_I2C_ADR);
    return SYS_READY;
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
    //return m_pI2C->Write(&m_Registers[0], NB_OF_WRITE_REGISTER, SI4703_I2C_ADR);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
#endif // (USE_SPI_DRIVER == DEF_ENABLED)
#endif // DIGINI_USE_GRAFX

void Si4703_Breakout::powerOn()
{
    si4703_init();
}

void Si4703_Breakout::setChannel(int channel)
{
  //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
  //97.3 = 0.2 * Chan + 87.5
  //9.8 / 0.2 = 49
  int newChannel = channel * 10; //973 * 10 = 9730
  newChannel -= 8750; //9730 - 8750 = 980
  newChannel /= 10; //980 / 10 = 98

  //These steps come from AN230 page 20 rev 0.5
  readRegisters();
  si4703_registers[CHANNEL] &= 0xFE00; //Clear out the channel bits
  si4703_registers[CHANNEL] |= newChannel; //Mask in the new channel
  si4703_registers[CHANNEL] |= (1<<TUNE); //Set the TUNE bit to start
  updateRegisters();

  //delay(60); //Wait 60ms - you can use or skip this delay

  //Poll to see if STC is set
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) != 0) break; //Tuning complete!
  }

  readRegisters();
  si4703_registers[CHANNEL] &= ~(1<<TUNE); //Clear the tune after a tune has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
  }
}

int Si4703_Breakout::seekUp()
{
	return seek(SEEK_UP);
}

int Si4703_Breakout::seekDown()
{
	return seek(SEEK_DOWN);
}

void Si4703_Breakout::setVolume(int volume)
{
  readRegisters(); //Read the current register set
  if(volume < 0) volume = 0;
  if (volume > 15) volume = 15;
  si4703_registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  si4703_registers[SYSCONFIG2] |= volume; //Set new volume
  updateRegisters(); //Update
}

void Si4703_Breakout::readRDS(char* buffer, long timeout)
{ 
	long endTime = millis() + timeout;
  boolean completed[] = {false, false, false, false};
  int completedCount = 0;
  while(completedCount < 4 && millis() < endTime) {
	readRegisters();
	if(si4703_registers[STATUSRSSI] & (1<<RDSR)){
		// ls 2 bits of B determine the 4 letter pairs
		// once we have a full set return
		// if you get nothing after 20 readings return with empty string
	  uint16_t b = si4703_registers[RDSB];
	  int index = b & 0x03;
	  if (! completed[index] && b < 500)
	  {
		completed[index] = true;
		completedCount ++;
	  	char Dh = (si4703_registers[RDSD] & 0xFF00) >> 8;
      	char Dl = (si4703_registers[RDSD] & 0x00FF);
		buffer[index * 2] = Dh;
		buffer[index * 2 +1] = Dl;
		// Serial.print(si4703_registers[RDSD]); Serial.print(" ");
		// Serial.print(index);Serial.print(" ");
		// Serial.write(Dh);
		// Serial.write(Dl);
		// Serial.println();
      }
      delay(40); //Wait for the RDS bit to clear
	}
	else {
	  delay(30); //From AN230, using the polling method 40ms should be sufficient amount of time between checks
	}
  }
	if (millis() >= endTime) {
		buffer[0] ='\0';
		return;
	}

  buffer[8] = '\0';
}




//To get the Si4703 inito 2-wire mode, SEN needs to be high and SDIO needs to be low after a reset
//The breakout board has SEN pulled high, but also has SDIO pulled high. Therefore, after a normal power up
//The Si4703 will be in an unknown state. RST must be controlled
void Si4703_Breakout::si4703_init() 
{
  pinMode(_resetPin, OUTPUT);
  pinMode(_sdioPin, OUTPUT); //SDIO is connected to A4 for I2C
  digitalWrite(_sdioPin, LOW); //A low SDIO indicates a 2-wire interface
  digitalWrite(_resetPin, LOW); //Put Si4703 into reset
  delay(1); //Some delays while we allow pins to settle
  digitalWrite(_resetPin, HIGH); //Bring Si4703 out of reset with SDIO set to low and SEN pulled high with on-board resistor
  delay(1); //Allow Si4703 to come out of reset

  Wire.begin(); //Now that the unit is reset and I2C inteface mode, we need to begin I2C

  readRegisters(); //Read the current register set
  //si4703_registers[0x07] = 0xBC04; //Enable the oscillator, from AN230 page 9, rev 0.5 (DOES NOT WORK, wtf Silicon Labs datasheet?)
  si4703_registers[0x07] = 0x8100; //Enable the oscillator, from AN230 page 9, rev 0.61 (works)
  updateRegisters(); //Update

  delay(500); //Wait for clock to settle - from AN230 page 9

  readRegisters(); //Read the current register set
  si4703_registers[POWERCFG] = 0x4001; //Enable the IC
  //  si4703_registers[POWERCFG] |= (1<<SMUTE) | (1<<DMUTE); //Disable Mute, disable softmute
  si4703_registers[SYSCONFIG1] |= (1<<RDS); //Enable RDS

  si4703_registers[SYSCONFIG1] |= (1<<DE); //50kHz Europe setup
  si4703_registers[SYSCONFIG2] |= (1<<SPACE0); //100kHz channel spacing for Europe

  si4703_registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  si4703_registers[SYSCONFIG2] |= 0x0001; //Set volume to lowest
  updateRegisters(); //Update

  delay(110); //Max powerup time, from datasheet page 13
}

//Read the entire register control set from 0x00 to 0x0F
void Si4703_Breakout::readRegisters(){

  //Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
  Wire.requestFrom(SI4703, 32); //We want to read the entire register set from 0x0A to 0x09 = 32 bytes.

  while(Wire.available() < 32) ; //Wait for 16 words/32 bytes to come back from slave I2C device
  //We may want some time-out error here

  //Remember, register 0x0A comes in first so we have to shuffle the array around a bit
  for(int x = 0x0A ; ; x++) { //Read in these 32 bytes
    if(x == 0x10) x = 0; //Loop back to zero
    si4703_registers[x] = Wire.read() << 8;
    si4703_registers[x] |= Wire.read();
    if(x == 0x09) break; //We're done!
  }
}

//Write the current 9 control registers (0x02 to 0x07) to the Si4703
//It's a little weird, you don't write an I2C addres
//The Si4703 assumes you are writing to 0x02 first, then increments
byte Si4703_Breakout::updateRegisters() {

  Wire.beginTransmission(SI4703);
  //A write command automatically begins with register 0x02 so no need to send a write-to address
  //First we send the 0x02 to 0x07 control registers
  //In general, we should not write to registers 0x08 and 0x09
  for(int regSpot = 0x02 ; regSpot < 0x08 ; regSpot++) {
    byte high_byte = si4703_registers[regSpot] >> 8;
    byte low_byte = si4703_registers[regSpot] & 0x00FF;

    Wire.write(high_byte); //Upper 8 bits
    Wire.write(low_byte); //Lower 8 bits
  }

  //End this transmission
  byte ack = Wire.endTransmission();
  if(ack != 0) { //We have a problem! 
    return(FAIL);
  }

  return(SUCCESS);
}

//Seeks out the next available station
//Returns the freq if it made it
//Returns zero if failed
int Si4703_Breakout::seek(byte seekDirection){
  readRegisters();
  //Set seek mode wrap bit
  si4703_registers[POWERCFG] |= (1<<SKMODE); //Allow wrap
  //si4703_registers[POWERCFG] &= ~(1<<SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first
  if(seekDirection == SEEK_DOWN) si4703_registers[POWERCFG] &= ~(1<<SEEKUP); //Seek down is the default upon reset
  else si4703_registers[POWERCFG] |= 1<<SEEKUP; //Set the bit to seek up

  si4703_registers[POWERCFG] |= (1<<SEEK); //Start seek
  updateRegisters(); //Seeking will now start

  //Poll to see if STC is set
  while(1) {
    readRegisters();
    if((si4703_registers[STATUSRSSI] & (1<<STC)) != 0) break; //Tuning complete!
  }

  readRegisters();
  int valueSFBL = si4703_registers[STATUSRSSI] & (1<<SFBL); //Store the value of SFBL
  si4703_registers[POWERCFG] &= ~(1<<SEEK); //Clear the seek bit after seek has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
  }

  if(valueSFBL) { //The bit was set indicating we hit a band limit or failed to find a station
    return(0);
  }
return getChannel();
}

//Reads the current channel from READCHAN
//Returns a number like 973 for 97.3MHz
int Si4703_Breakout::getChannel() {
  readRegisters();
  int channel = si4703_registers[READCHAN] & 0x03FF; //Mask out everything but the lower 10 bits
  //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
  //X = 0.1 * Chan + 87.5
  channel += 875; //98 + 875 = 973
  return(channel);
}