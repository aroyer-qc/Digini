//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F74_io_port.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 20203 Alain Royer.
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "stm32f7xx.h"
#include "./Digini/Peripheral/inc/port/lib_io.h"

//-------------------------------------------------------------------------------------------------

#ifdef IO_PORT_DEF

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum IO_Port_e
{
  #define X_IO_PORT(ENUM_ID, PORT, PORT_MASK, PORT_SHIFT, MODE_OUTPUT, MODE_INPUT, SPEED) ENUM_ID,
    IO_PORT_DEF
  #undef X_IO_PORT
    NB_IO_PORT_CONST
};

enum IO_PortDir_e
{
    IO_PORT_INPUT,
    IO_PORT_OUTPUT,
};

struct IO_Port_t
{
    GPIO_TypeDef*       pPort;
    uint16_t            Mask;
    uint8_t             Shift;
    uint32_t            TypeOutput;
    uint32_t            TypeInput;
    uint32_t            Speed;
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

class IO_PortDriver
{
    public:

        void                                Initialize              (IO_Port_e Port);
        uint32_t                            Read                    (void);
        void                                Write                   (uint32_t Data);
        void                                SetDirection            (IO_PortDir_e Direction);

    private:

        IO_Port_t*                          m_pPort;
        static const IO_Port_t              m_Port[NB_IO_PORT_CONST];
        uint32_t                            m_2BitsMask;
};

//-------------------------------------------------------------------------------------------------

#endif // IO_PORT_DEF
