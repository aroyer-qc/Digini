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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameters:     Command     - Parsed Modbus request (validated by the manager)
//                  Response    - Structure to be filled by the application callback
//
//  Returns:        true        - A matching entry was found and its callback executed
//                  false       - No entry found; the manager will generate an exception
//
//  Description:    Locates the application handler associated with the requested UnitID and
//                  Function code. If a matching entry exists and provides a callback, the
//                  callback is invoked to generate the Modbus response. Passthru routing is
//                  not handled here; it is performed by the Modbus manager before calling APP.
//
//-------------------------------------------------------------------------------------------------
bool ModbusAPP::Process(const MODBUS_Command_t& Command, MODBUS_Response_t& Response)
{
    const ModbusAppEntry_t* Entry = Find(Command.UnitID, Command.Function);

    if(Entry == nullptr)
    {
        return false; // Manager will return an exception
    }

    if(Entry->Callback != nullptr)
    {
        Entry->Callback(Command, Response);
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Find
//
//  Parameters:     UnitID      - Modbus slave address to match
//                  Function    - Modbus function code to match
//
//  Returns:        Pointer to the matching MODBUS_AppEntry_t, or nullptr if no entry matches.
//
//  Description:    Performs a linear search in the application command table and returns
//                  the entry whose UnitID and Function fields match the requested command.
//                  Used by ModbusAPP to locate the application handler for a given request.
//
//-------------------------------------------------------------------------------------------------
const MODBUS_AppEntry_t* ModbusAPP::Find(uint8_t UnitID, uint8_t Function)
{
    for(size_t Index = 0; Index < m_ModbusAppTableCount; Index++)
    {
        const MODBUS_AppEntry_t* Entry = &m_ModbusAppTable[Index];

        if(Entry->UnitID == UnitID)
        {
            if(Entry->Function == Function)
            {
                return Entry;
            }
        }
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
