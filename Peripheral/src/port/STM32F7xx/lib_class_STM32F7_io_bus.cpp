//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_io_bus.cpp
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
#include "lib_class_STM32F7_io_bus.h"

//-------------------------------------------------------------------------------------------------
// Define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

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
    IO_PinInit(m_pBus->u.Intel.ChipSelect);

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
        Data = m_Port.Read();
        IO_SetPinHigh(m_pBus->ChipSelect);
        IO_SetPinHigh(m_pBus->u.Intel.RD);
    }
    else // IO_BUS_MODE_MOTOROLA
    {
        IO_SetPinHigh(m_pBus->u.Motorola.RW);
        IO_SetPinLow(m_pBus->ChipSelect;
        IO_SetPinHigh(m_pBus->u.Motorola.E);
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
        __asm("nop");
        IO_SetPinHigh(m_pBus->ChipSelect);
        IO_SetPinHigh(m_pBus->u.Intel.WR);
    }
    else // IO_BUS_MODE_MOTOROLA
    {
        IO_SetPinLow(m_pBus->u.Motorola.RW);
        IO_SetPinLow(m_pBus->ChipSelect;
        IO_SetPinHigh(m_pBus->u.Motorola.E);
        __asm("nop");
        IO_SetPinLow(m_pBus->u.Motorola.E);
        IO_SetPinHigh(m_pBus->ChipSelect);
    }

    m_Port.SetDirection(IO_PORT_INPUT);
}

//-------------------------------------------------------------------------------------------------

