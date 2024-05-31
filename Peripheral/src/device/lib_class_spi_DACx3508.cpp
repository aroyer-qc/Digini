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

#define LIB_DAC_x3508_GLOBAL
#include "./Digini/lib_digini.h"
#undef  LIB_DAC_x3508_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DAC_NUMBER_OF_BYTES_PER_COMMAND         size_t(3)

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
   if(isItInitialized != true)
	{
        m_pSPI         = pSPI;
        m_ChipSelectIO = ChipSelectIO;

        IO_PinInit(ChipSelectIO);

        // Initialize SPI Driver
        m_pSPI->Initialize();

        Send3Bytes(DACX3508_STATUS_TRIGGER_REG, DAC43508_STA_TRG_SW_RESET_VALUE);	    // Reset the DAC
        //    need a delay here Minimum 5 mSec!!
        nOS_Sleep(10);

        // Put default value into DAC output.
        Send3Bytes(DACX3508_DEVICE_CONFIG_REG, 0);		                                // The broadcast will affect all DAC
        Send3Bytes(DACX3508_BROADCAST_REG, 0);								            // The default value on all DAC
        isItInitialized = true;
	}

	return SYS_READY;
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
SystemState_e DAC_X3508_Driver::WriteDAC(int Channel, uint16_t Value)
{
	if(isItInitialized == true)
	{
        // Write data to the register for specific channel
        Send3Bytes(DAC_X3508_Cmd_e(Channel + uint32_t(DACX3508_DACA_DATA_REG)), Value);
	}

	return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Send3Bytes
//
//   Parameter(s):  DAC_X3508_Cmd_e     Command
//                  uint16_t            Data
//   Return Value:  SystemState_e
//
//   Description:   Send the command to the DAC with data
//
//-------------------------------------------------------------------------------------------------
SystemState_e DAC_X3508_Driver::Send3Bytes(DAC_X3508_Cmd_e Command, uint16_t Data)
{
	SystemState_e State;
	uint8_t       ByteBuffer[3];

	ByteBuffer[0] = Command;
	ByteBuffer[1] = (uint8_t)(Data >> 8);
	ByteBuffer[2] = (uint8_t)Data;
	State = m_pSPI->Write(&ByteBuffer[0], DAC_NUMBER_OF_BYTES_PER_COMMAND, m_ChipSelectIO);

	return State;
}

//-------------------------------------------------------------------------------------------------

#endif
