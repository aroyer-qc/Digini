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

#define MODBUS_APP_GLOBAL
#include "./lib_digini.h"
#undef  MODBUS_APP_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_MODBUS_INIT_ENTRY(DEVICE_ADDRESS, FUNCTION, HANDLER) m_ModbusAppTable[m_ModbusAppCount++] = { DEVICE_ADDRESS, FUNCTION, HANDLER };

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

MODBUS_AppEntry_t MODBUS_Application::m_ModbusAppTable[MODBUS_MAX_COMMAND_ENTRY];
size_t            MODBUS_Application::m_ModbusAppCount = 0;

//-------------------------------------------------------------------------------------------------
//
//  Name:           MODBUS_Application
//
//  Parameters:     None
//
//  Description:    Initializes the Modbus application command table. Static entries are loaded
//                  from the MODBUS_APP_TABLE macro, and the internal counter is set to the
//                  number of predefined commands. Dynamic entries may be registered afterward.
//
//-------------------------------------------------------------------------------------------------
MODBUS_Application::MODBUS_Application()
{
  #ifdef MODBUS_APP_TABLE
    MODBUS_APP_TABLE(EXPAND_MODBUS_INIT_ENTRY)
  #endif

	// Clear the table
	for(int Index = m_ModbusAppCount; Index < MODBUS_MAX_COMMAND_ENTRY; Index++)
	{
		m_ModbusAppTable[Index].DeviceAddress = 0;
		m_ModbusAppTable[Index].Function      = 0;
		m_ModbusAppTable[Index].pCallback     = nullptr;
	}
}

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
//  Description:    Locates the application handler associated with the requested DeviceAddress and
//                  Function code. If a matching entry exists and provides a callback, the
//                  callback is invoked to generate the Modbus response. Passthru routing is
//                  not handled here; it is performed by the Modbus manager before calling APP.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Application::Process(MODBUS_Command_t& Command, MODBUS_Response_t& Response)
{
    const MODBUS_AppEntry_t* pEntry = FindHandler(Command.Address, Command.Function);

    if(pEntry == nullptr)
    {
        return false;
    }

    if(pEntry->pCallback == nullptr)
    {
        return false;
    }

    // Appel du handler applicatif
    pEntry->pCallback(Command, Response);

    // Le callback doit remplir Response.Length
    if(Response.Length == 0)
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterCommand
//
//  Parameters:     Entry   - Application command entry to register
//
//  Returns:        true    - Entry successfully added
//                  false   - Table is full; entry not added
//
//  Description:    Adds a new Modbus application command entry to the dynamic portion of the
//                  application table. Static entries are loaded during initialization, and this
//                  function appends additional entries at runtime until the table reaches its
//                  maximum capacity.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Application::RegisterCommand(const MODBUS_AppEntry_t& Entry)
{
    if(m_ModbusAppCount >= MODBUS_MAX_COMMAND_ENTRY)
	{
        return false;
	}

    // Copy fields explicitly (safe)
    m_ModbusAppTable[m_ModbusAppCount].DeviceAddress = Entry.DeviceAddress;
    m_ModbusAppTable[m_ModbusAppCount].Function      = Entry.Function;
    m_ModbusAppTable[m_ModbusAppCount].pCallback     = Entry.pCallback;
    m_ModbusAppCount++;
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindHandler
//
//  Parameters:     DeviceAddress   - Modbus slave address to match
//                  Function    - Modbus function code to match
//
//  Returns:        Pointer to the matching MODBUS_AppEntry_t, or nullptr if no entry matches.
//
//  Description:    Performs a linear search in the application command table and returns
//                  the entry whose DeviceAddress and Function fields match the requested command.
//                  Used by MODBUS_Application to locate the app handler for a given request.
//
//-------------------------------------------------------------------------------------------------
MODBUS_AppEntry_t* MODBUS_Application::FindHandler(uint8_t DeviceAddress, uint8_t Function)
{
    for(size_t Index = 0; Index < m_ModbusAppCount; Index++)
    {
        MODBUS_AppEntry_t* Entry = &m_ModbusAppTable[Index];

        if(Entry->DeviceAddress == DeviceAddress)
        {
            if(Entry->Function == Function)
            {
                return Entry;
            }
        }
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
