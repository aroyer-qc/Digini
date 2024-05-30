//-------------------------------------------------------------------------------------------------
//
//  File    : lib_class_spi_DACx3508.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DACX3508_DEV_CFG_PDN_ALL            0x0100          // using this define will disable all output
#define DACX3508_DEV_CFG_PDN_H              0x0080
#define DACX3508_DEV_CFG_PDN_G              0x0040
#define DACX3508_DEV_CFG_PDN_F              0x0020
#define DACX3508_DEV_CFG_PDN_E              0x0010
#define DACX3508_DEV_CFG_PDN_D              0x0008
#define DACX3508_DEV_CFG_PDN_C              0x0004
#define DACX3508_DEV_CFG_PDN_B              0x0002
#define DACX3508_DEV_CFG_PDN_A              0x0001

#define DAC43508_STA_TRG_SW_RESET_VALUE     0x000A          // Reset code

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum DAC_X3508_Cmd_e
{
    DACX3508_DEVICE_CONFIG_REG     = 0x01,
    DACX3508_STATUS_TRIGGER_REG    = 0x02,
    DACX3508_BROADCAST_REG         = 0x03,    	    // Writing to the BRDCAST register forces the DAC channel to update the active register data to BRDCAST_DATA
    DACX3508_DACA_DATA_REG         = 0x08,
    DACX3508_DACA_DATB_REG         = 0x09,
    DACX3508_DACA_DATC_REG         = 0x0A,
    DACX3508_DACA_DATD_REG         = 0x0B,
    DACX3508_DACA_DATE_REG         = 0x0C,
    DACX3508_DACA_DATF_REG         = 0x0D,
    DACX3508_DACA_DATG_REG         = 0x0E,
    DACX3508_DACA_DATH_REG         = 0x0F,
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class DAC_X3508_Driver
{
    public:

                        DAC_X3508_Driver        ();

        SystemState_e   Initialize              (SPI_Driver* pSPI, IO_ID_e ChipSelectIO);
        SystemState_e   WriteDAC                (int Channel, uint16_t Value);

    private:

        SystemState_e   Send3Bytes              (DAC_X3508_Cmd_e Command, uint16_t Data);

        bool            isItInitialized;
        SPI_Driver*     m_pSPI;
        IO_ID_e         m_ChipSelectIO;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_DAC_X3508__
#include "device_var.h"
#undef  __CLASS_DAC_X3508__

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
