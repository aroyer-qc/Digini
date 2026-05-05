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
//  |  MasterParseResponse()    |
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
// Global Macro
//-------------------------------------------------------------------------------------------------

#ifdef MODBUS_APP_GLOBAL
    #define MODBUS_APP_EXTERN
#else
    #define MODBUS_APP_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_SLAVE_CALLBACK(SLAVE_ID, FUNCTION, ADDRESS, MAX_QUANTITY, HANDLER) \
        extern void HANDLER(const MODBUS_Command_t& Command, MODBUS_SlaveResponse_t& Response);
#define EXPAND_MASTER_CALLBACK( REQUEST_TO_ID, FUNCTION, ADDRESS, MAX_QUANTITY, TIMEOUT, HANDLER) \
        extern void HANDLER(const MODBUS_MasterResponse_t& Response);

//-------------------------------------------------------------------------------------------------
// Declare external handler
//-------------------------------------------------------------------------------------------------

MODBUS_APP_SLAVE_TABLE(EXPAND_SLAVE_CALLBACK)
MODBUS_APP_MASTER_TABLE(EXPAND_MASTER_CALLBACK)

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class MODBUS_Application
{
    public:

											MODBUS_Application 		    ();

		bool 								Process						(MODBUS_Command_t& Command, MODBUS_SlaveResponse_t& Response);
        void                                SetManager                  (MODBUS_Manager* pManager) 					{ m_pManager = pManager; }

		// SLAVE side
		bool 								SlaveRegisterCommand 		(const MODBUS_SlaveCommandEntry_t& Entry);
		MODBUS_SlaveCommandEntry_t*			SlaveFindHandler			(uint8_t DeviceAddress, uint8_t Function);

		size_t                              SlaveGetCount               (void)        	{ return m_ModbusAppSlaveCount; }
		const MODBUS_SlaveCommandEntry_t*   SlaveGetEntry               (size_t Index)  { return (Index < m_ModbusAppSlaveCount) ? &m_ModbusAppSlaveTable[Index] : nullptr; }

		//Master side
		uint16_t 							MasterRegisterRequest		(const MODBUS_MasterEntry_t& Entry);
		bool 								MasterRequest				(uint32_t RequestID, uint16_t Quantity);
		MODBUS_MasterEntry_t* 				MasterFindRequest			(uint32_t RequestID);
		size_t 								MasterGetCount				(void) 										{ return m_ModbusAppMasterCount; }


	private:

        MODBUS_Manager*                     m_pManager = nullptr;

		// SLAVE table
		static MODBUS_SlaveCommandEntry_t 	m_ModbusAppSlaveTable    	[MODBUS_MAX_SLAVE_COMMAND_ENTRY];
		static size_t            			m_ModbusAppSlaveCount;   	// Number of used entries (static + dynamic)

		// MASTER table
		static MODBUS_MasterEntry_t       	m_ModbusAppMasterTable		[MODBUS_MAX_MASTER_REQUEST_ENTRY];
		static size_t                     	m_ModbusAppMasterCount;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

MODBUS_APP_EXTERN class MODBUS_Application 		myMODBUS_Application;

//-------------------------------------------------------------------------------------------------

#endif //(DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

