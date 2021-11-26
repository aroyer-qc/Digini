//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_io_bus.h
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
// substantial portions of the Software.bandsaw
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_stm32f7_io.h"
#include "lib_class_stm32f7_io_port.h"

//-------------------------------------------------------------------------------------------------
// struct(s) and enum(s)
//-------------------------------------------------------------------------------------------------

enum IO_Bus_e
{
  #define X_IO_BUS(ENUM_ID, BUS_TYPE, RD_OR_E, WR_OR_RW, CHIP_SELECT) ENUM_ID,
    IO_BUS_DEF
  #undef X_IO_BUS
    NB_BUS_CONST
};

enum IO_BusType_e
{
    IO_BUS_MODE_INTEL,
    IO_BUS_MODE_MOTOROLA,
};

struct IO_Bus_t
{
    IO_BusType_e Type;

    union
    {
        struct
        {
            IO_ID_e     RD;
            IO_ID_e     WR;
        } Intel;
        struct
        {
            IO_ID_e     E;
            IO_ID_e     RW;
        } Motorola;
    } u;
    IO_ID_e             ChipSelect;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class IO_BusDriver
{
    public:

        void                                Initialize              (IO_Bus_e Bus);
        uint32_t                            Read                    (void);
        void                                Write                   (uint32_t Data);

    private:

        IO_Bus_t*                           m_pBus;
        static IO_Bus_t                     m_Bus[NB_BUS_CONST];
        IO_PortDriver                       m_Port;
};

//-------------------------------------------------------------------------------------------------
