//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_io_bus.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

#define IO_BUS_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  IO_BUS_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_IO_BUS_DRIVER == DEF_ENABLED)

#define CFG_CHIP_SELECT_LOW_TIME            3  // Chip select active time for 6800/8080 bus

#define m_E         m_RD
#define m_RW        m_WR

//-------------------------------------------------------------------------------------------------
// Static variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

IO_Bus_t IO_BusDriver::m_Bus[NB_BUS_CONST] =
{
  #define X_IO_BUS(ENUM_ID, BUS_TYPE,  RD_OR_E, WR_OR_RW,  CHIP_SELECT) \
                           {BUS_TYPE, {RD_OR_E, WR_OR_RW}, CHIP_SELECT},
      IO_BUS_DEF
  #undef X_IO_BUS
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   IO_Bus_e Bus      Specified the pin to configure
//  Return:         None
//
//  Description:    Initialize selected GPIO as input/output/function
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_BusDriver::Initialize(IO_Bus_e Bus)
{
    m_pBus = &m_Bus[Bus];

    IO_PinInit(m_pBus->u.Intel.RD);
    IO_PinInit(m_pBus->u.Intel.WR);
    IO_PinInit(m_pBus->ChipSelect);

    if(m_pBus->Type == IO_BUS_MODE_MOTOROLA)
    {
        IO_SetPinLow(m_pBus->u.Motorola.E);    // E signal is inactive low
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_Read
//
//  Parameter(s):   None
//  Return:         uint32_t	Bus value
//
//  Description:    Read a value from a bus according to the configuration
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t IO_BusDriver::Read(void)
{
    uint32_t Data;

    if(m_pBus->Type == IO_BUS_MODE_INTEL)
    {
        IO_SetPinLow(m_pBus->u.Intel.RD);
        IO_SetPinLow(m_pBus->ChipSelect);
        for(int i = 0; i < CFG_CHIP_SELECT_LOW_TIME; i++) { __asm("nop"); }         // todo fix that!!
        for(int i = 0; i < CFG_CHIP_SELECT_LOW_TIME; i++) { __asm("nop"); }
        for(int i = 0; i < CFG_CHIP_SELECT_LOW_TIME; i++) { __asm("nop"); }
        Data = m_Port.Read();
        IO_SetPinHigh(m_pBus->ChipSelect);
        IO_SetPinHigh(m_pBus->u.Intel.RD);
    }
    else // IO_BUS_MODE_MOTOROLA
    {
        IO_SetPinHigh(m_pBus->u.Motorola.RW);
        IO_SetPinLow(m_pBus->ChipSelect);
        IO_SetPinHigh(m_pBus->u.Motorola.E);
        for(int i = 0; i < CFG_CHIP_SELECT_LOW_TIME; i++) { __asm("nop"); }         // todo fix that!!
        Data = m_Port.Read();
        IO_SetPinLow(m_pBus->u.Motorola.E);
        IO_SetPinHigh(m_pBus->ChipSelect);
    }

    return Data;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_Write
//
//  Parameter(s):   uint32_t	Bus value
//  Return:         none
//
//  Description:    Write a value to a bus according to the configuration
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_BusDriver::Write(uint32_t Data)
{
    m_Port.SetDirection(IO_PORT_OUTPUT);
    m_Port.Write(Data);

    if(m_pBus->Type == IO_BUS_MODE_INTEL)
    {
        IO_SetPinLow(m_pBus->u.Intel.WR);
        IO_SetPinLow(m_pBus->ChipSelect);
        for(int i = 0; i < CFG_CHIP_SELECT_LOW_TIME; i++) { __asm("nop"); }        // todo fix that!!
        IO_SetPinHigh(m_pBus->ChipSelect);
        IO_SetPinHigh(m_pBus->u.Intel.WR);
    }
    else // IO_BUS_MODE_MOTOROLA
    {
        IO_SetPinLow(m_pBus->u.Motorola.RW);
        IO_SetPinLow(m_pBus->ChipSelect);
        IO_SetPinHigh(m_pBus->u.Motorola.E);
        for(int i = 0; i < CFG_CHIP_SELECT_LOW_TIME; i++) { __asm("nop"); }        // todo fix that!!
        IO_SetPinLow(m_pBus->u.Motorola.E);
        IO_SetPinHigh(m_pBus->ChipSelect);
    }

    m_Port.SetDirection(IO_PORT_INPUT);
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_IO_BUS_DRIVER == DF_ENABLED)
