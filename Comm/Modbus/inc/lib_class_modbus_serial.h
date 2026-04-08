//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_modbus_serial.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

enum class MODBUS_State_e
{
    MODBUS_IDLE,
    MODBUS_BUILD_FRAME,
    MODBUS_SEND_FRAME,
    MODBUS_WAIT_SILENT,
    MODBUS_WAIT_RESPONSE,
    MODBUS_PARSE_RESPONSE,
    MODBUS_DONE,
    MODBUS_ERROR
};


class ModbusRTU : public ChildProcessInterface
{
    public:

        void            IF_Process                  (void);

        void            Initialize                  (ModbusCentral* pCentral, Console* pConsole);
        int             Send                        (const uint8_t* pData, size_t Length);
        int             Received                    (uint8_t* pBuffer, size_t MaxLength, TickCount_t TimeOutMsec);

        bool            Queue                       (const MODBUS_Command& Command);

    private:

        bool            IsEndOfRTU_Frame            (const uint8_t* pBuffer, size_t Length);


        Console*            m_pConsole              = nullptr;
        MODBUS_State_e      m_State                 = MODBUS_IDLE;
        ModbusCentral*      m_pCentral              = nullptr;

        MODBUS_Command      m_Command;
        bool                m_HasPending            = false;
        
        uint32_t            m_StartTick             = 0;
        uint32_t            m_SilentTick            = 0;
};

//-------------------------------------------------------------------------------------------------
