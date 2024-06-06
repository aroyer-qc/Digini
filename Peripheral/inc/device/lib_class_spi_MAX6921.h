//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_VFD.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct MAX6921_PinStruct_t
{
    IO_ID_e     IO_DOut;
    IO_ID_e     IO_Clk;
    IO_ID_e     IO_Load;
    IO_ID_e     IO_Blank;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class MAX6921_Driver
{
    public:

        SystemState_e   Initialize                      (void* pArg);
        void            Send                            (void);
        void            Callback                        (void);

    private:

        MAX6921_PinStruct_t*            m_pPinStruct;
        uint8_t                         m_ChainSize;
       
        //IRQ related variables
        volatile uint16_t               m_BitCounter;
        volatile uint8_t                m_BitMask;
        uint8_t*                        m_pDataToSend;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_MAX6921__
#include "device_var.h"
#undef  __CLASS_MAX6921__

//-------------------------------------------------------------------------------------------------

