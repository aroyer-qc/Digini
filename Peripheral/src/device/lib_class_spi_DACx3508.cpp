//-------------------------------------------------------------------------------------------------
//
//  File    : lib_class_spi_DACx3508.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

//#define DAC_x3508_GLOBAL
#include "lib_digini.h"
//#undef  DAC_x3508_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Class: DAC_X3508_Driver
//
//
//   Description:   Class to handle DAC43508 or DAC53508 using SPI
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   DAC_X3508_Driver
//
//   Parameter(s):  
//
//   Description:   Initializes the 
//
//-------------------------------------------------------------------------------------------------
DAC_X3508_Driver::DAC_X3508_Driver()
{
    isItInitialized = false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Initialize this DACX3508
//
//-------------------------------------------------------------------------------------------------
SystemState_e DAC_X3508_Driver::Initialize(SPI_Driver* pSPI, IO_ID_e ChipSelectIO)
{
    if(DAC43508_isInitialized != true)
	{
        m_pSPI         = pSPI;
        m_ChipSelectIO = ChipSelectIO;
        
        IO_PinInitialize(m_pConfig->ChipSelectIO);

        // Initialize SPI Driver
        m_pSPI->Initialize();

        Send3Bytes(DAC43508_STATUS_TRIGGER_REG, DAC43508_STA_TRG_SW_RESET_VALUE);	    // Reset the DAC
        //    need a delay here Minimum 5 mSec!!
        nOS_Sleep(10);

        // Put default value into DAC output.
        Send3Bytes(DAC43508_DEVICE_CONFIG_REG, 0);		                                // The broadcast will affect all DAC
        Send3Bytes(DAC43508_BRDCAST_REG, 0);								            // The default value on all DAC
        isItInitialized = true;
	}
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteDAC
//
//   Parameter(s):  int                 Channel
//                  uint16_t            Value
//   Return Value:  None
//
//   Description:   Write value into specific channel
//
//-------------------------------------------------------------------------------------------------
void DAC_X3508_Driver::WriteDAC(int Channel, uint16_t Value)
{
	if(isItInitialized == true)
	{
        // Write data to the register for specific channel
        Send3Bytes(Channel + DAC43508_DACA_DATA_REG, Value);
	}
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Send3Bytes
//
//   Parameter(s):  DAC_X3508_Cmd_e     Command
//                  uint16_t            Data
//   Return Value:  None
//
//   Description:   Send the command to the DAC with data
//
//-------------------------------------------------------------------------------------------------
void DAC_X3508_Driver::Send3Bytes(DAC_X3508_Cmd_e Command, uint16_t Data)
{
	bool doBusyWait = true;
	uint8_t ByteBuffer[3];

	ByteBuffer[0] = CMD;
	ByteBuffer[1] = (uint8_t)(Data >> 8);
	ByteBuffer[2] = (uint8_t)Data;


    m_pSPI->LockToDevice(m_ChipSelectIO)
    IO_ResetPinHigh(m_pConfig->ChipSelect);
	m_pSPI->Transfer(&byteBuffer, NULL, 3);
    IO_SetPinHigh(m_pConfig->ChipSelect);
}
