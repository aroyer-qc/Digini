//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_modbus_app.h
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
//
//  Note(s):
//
//  +---------------------+
//  |     ModbusAPP       |  <-- Application layer
//  |  X-macro table      |      - callbacks
//  |  callbacks          |      - passthru rules
//  |                     |      - dynamic entries (later)
//  +---------------------+
//           ^
//           |
//           v
//  +---------------------+
//  |   MODBUS_Manager    |  <-- Modbus protocol logic (stateless)
//  |  BuildFrame()       |
//  |  ParseResponse()    |
//  +---------------------+
//           ^
//           |
//           v
//  +---------------------+
//  |   MODBUS_Router     |  <-- choose RTU or TCP
//  |  passthru rules     |      - need a struct to define behavior of a passthru with address change (later)
//  +---------------------+
//      ^            ^
//      |            |
//      v            v
//  +-----------+   +-----------+
//  | ModbusRTU |   | ModbusTCP |
//  | Process() |   | Process() |
//  +-----------+   +-----------+
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_MAX_BACKENDS   8   // Pour le config plus tard!!


#define MAKE_ENTRY(ID, FUNC, CB, PT) { ID, FUNC, CB, PT },

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct MODBUS_AppEntry_t
{
    uint8_t         UnitID;
    uint8_t         Function;
    void            (*Callback)(const MODBUS_Command_t&, MODBUS_Response_t&);
};


/*
#define MODBUS_APP_TABLE(X) \
    X(1, 0x03, ReadHoldingRegs) \
    X(1, 0x06, WriteSingleReg) \
    X(2, 0x03, Poutine) \
    X(3, 0x10, WriteMultipleRegs)
*/

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class ModbusAPP
{
    public:
        
        bool Process(const MODBUS_Command_t& Command, MODBUS_Response_t& Response);

    private:

        const ModbusAppEntry* Find(uint8_t UnitID, uint8_t Function);
};
