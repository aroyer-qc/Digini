//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_i2c_DS3502.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class DS3502
{
    public:

        SystemState_e   Initialize                      (I2C_Driver* pI2C, uint8_t DeviceAddress);
        void            Reset                           (void);
        void            SetWiperIV                      (uint8_t InitialValue);
        void            SetWiper                        (uint8_t WiperValue);
        void            Up                               (void);
        void            Down                            (void);
        void            SetMaxValue                     (uint8_t MaxValue);

    private:

        I2C_Driver*     m_pI2C;
        uint8_t         m_DeviceAddress;
        uint8_t         m_WiperPos;
        uint8_t         m_WiperIV;
        uint8_t         m_MaxValue;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

extern class   DS3502                      DS3502_Volume;

#ifdef LIB_DS3502_GLOBAL
 class   DS3502                            DS3502_Volume;
#endif

//-------------------------------------------------------------------------------------------------

#else // (USE_I2C_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for I2C must be enable and configure to use this device driver")

#endif // (USE_I2C_DRIVER == DEF_ENABLED)


