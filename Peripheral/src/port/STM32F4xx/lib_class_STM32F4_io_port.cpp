//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_io_port.cpp
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

#include <stdint.h>
#include "lib_class_STM32F4_io_port.h"

//-------------------------------------------------------------------------------------------------

#ifdef IO_PORT_DEF

//-------------------------------------------------------------------------------------------------
// define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

#define IO_PORT_ALL_INPUT_MASK                  0x00000000
#define IO_PORT_ALL_OUTPUT_MASK                 0x55555555
#define IO_PORT_2_BITS_MASK                     0x00000003

//-------------------------------------------------------------------------------------------------
// static variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

const IO_Port_t IO_PortDriver::m_Port[NB_IO_PORT_CONST] __attribute__((used)) =
{
  #define X_IO_PORT(ENUM_ID, PORT, PORT_MASK, PORT_SHIFT, MODE_OUTPUT, MODE_INPUT, SPEED) \
                            {PORT, PORT_MASK, PORT_SHIFT, MODE_OUTPUT, MODE_INPUT, SPEED},
    IO_PORT_DEF
  #undef  X_IO_PORT
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   IO_Port_e Port      Specified the port to configure
//  Return:         None
//
//  Description:    Initialize selected GPIO as IO Input by default
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PortDriver::Initialize(IO_Port_e Port)
{
    m_pPort     = &m_Port[Port];
    m_2BitsMask = 0;

    for(uint32_t Pin = 0; Pin < NUMBER_OF_PIN_PER_PORT; Pin++)
    {
        if((m_pPort->Mask & (1 << Pin)) != 0)       // only init pin enable in the mask
        {
            IO_PinInit(m_pPort->pPort,
                       Pin,
                       IO_MODE_INPUT,
                       (m_pPort->TypeOutput | m_pPort->TypeInput),
                       m_pPort->Speed,
                       0);                  // Don't care for state
            m_2BitsMask |= (IO_PORT_2_BITS_MASK << (Pin << 1));
        }
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
uint32_t IO_PortDriver::Read(void)
{
    uint32_t Data;

    SetDirection(IO_PORT_INPUT);
    Data   = m_pPort->pPort->IDR;
    Data  &= m_pPort->Mask;
    Data >>= m_pPort->Shift;
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
void IO_PortDriver::Write(uint32_t Data)
{
    uint32_t RegData;

    RegData  = m_pPort->pPort->ODR;                 // Get current port data
    RegData &= ~(m_pPort->Mask);                    // keep the bit's not on our care
    Data   <<= m_pPort->Shift;                      // Shift our data to the proper position
    Data    &= m_pPort->Mask;                       // Make sure no other bit's are set in our data
    RegData |= Data;                                // insert our data into register data
    m_pPort->pPort->ODR = RegData;                  // Write back the new data
    SetDirection(IO_PORT_OUTPUT);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:      SetDirection
//
//  Parameter(s):   uint32_t	Bus value
//  Return:         none
//
//  Description:    Set the port in input or output
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PortDriver::SetDirection(IO_PortDir_e Direction)
{
    uint32_t ModeMask;
    uint32_t PortMask;
    uint32_t RegData;

    ModeMask  = (Direction == IO_PORT_INPUT) ? IO_PORT_ALL_INPUT_MASK : IO_PORT_ALL_OUTPUT_MASK;    // Apply input or output mask
    RegData   = m_pPort->pPort->MODER;                                                              // Get the MODER to save non port bits
    PortMask  = m_pPort->Mask;                                                                      // Get the port mask from config
    RegData  &= ~m_2BitsMask;                                                                       // keep all other IO mode value
    RegData  |= (ModeMask & m_2BitsMask);                                                           // Apply mode change to port
    m_pPort->pPort->MODER =  RegData;                                                               // Apply port new direction and other unchanged IO
}

//-------------------------------------------------------------------------------------------------

#endif // IO_PORT_DEF


