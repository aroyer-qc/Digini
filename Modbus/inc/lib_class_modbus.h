//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_modbus.h
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
//  |   MODBUS_Manager    |  <-- Modbus logic (stateless)
//  |  BuildFrame()       |
//  |  ParseResponse()    |
//  +---------------------+
//           ^
//           |
//           v
//  +---------------------+
//  |   MODBUS_Router     |  <-- choose RTU or TCP
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

#ifdef MODBUS_GLOBAL
    #define MODBUS_EXTERN
#else
    #define MODBUS_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------

#include "modbus_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_MAX_PDU_SIZE   			252
#define MODBUS_EXCEPTION_RESPONSE		0x80

#define TASK_MODBUS_PRIO                7
#define TASK_MODBUS_STACK_SIZE          256

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum MODBUS_Function_e
{
    MODBUS_READ_COILS                  = 0x01,
    MODBUS_READ_DISCRETE_INPUTS        = 0x02,
    MODBUS_READ_HOLDING_REGISTERS      = 0x03,
    MODBUS_READ_INPUT_REGISTERS        = 0x04,
    MODBUS_WRITE_SINGLE_COIL           = 0x05,
    MODBUS_WRITE_SINGLE_REGISTER       = 0x06,
    MODBUS_WRITE_MULTIPLE_COILS        = 0x0F,
    MODBUS_WRITE_MULTIPLE_REGISTERS    = 0x10
};

enum  MODBUS_ExceptionCode_e
{
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION         = 0x01,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS     = 0x02,
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE       = 0x03,
    MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE     = 0x04,
    MODBUS_EXCEPTION_ACKNOWLEDGE              = 0x05,
    MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY        = 0x06,
    MODBUS_EXCEPTION_MEMORY_PARITY_ERROR      = 0x08,
    MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE = 0x0A,
    MODBUS_EXCEPTION_GATEWAY_TARGET_FAILED    = 0x0B
};

enum MODBUS_Backend_e
{
    MODBUS_BACKEND_LOCAL,
    MODBUS_BACKEND_TCP,
    MODBUS_BACKEND_RTU,
};

struct MODBUS_Command_t
{
    MODBUS_Backend_e   BackEnd;
    uint8_t            DeviceAddress;
    MODBUS_Function_e  Function;

    uint8_t*           pData;
    size_t             DataLength;

    uint16_t           Address;
    uint16_t           Quantity;
    uint16_t           Value;

    size_t             ResultLength;

    TickCount_t        TimeoutMsec;
};

struct MODBUS_Response_t
{
    uint8_t     Function;        // Function code (or function | 0x80 for exception)
    uint8_t*    pPayload;        // Pointer to external TX buffer
    size_t      MaxSize;         // Max size of external TX buffer
    size_t      Length;          // Number of bytes written into Payload
    bool        IsException;     // True if exception frame
    uint8_t     ExceptionCode;   // Only valid if IsException = true
};

struct MODBUS_PassThruRule_t
{
    uint8_t     SrcDeviceAddress;
    uint8_t     DstDeviceAddress;
};

struct MODBUS_AppEntry_t
{
    uint8_t     DeviceAddress;
    uint8_t     Function;
    void        (*pCallback)(const MODBUS_Command_t&, MODBUS_Response_t&);
};

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class MODBUS_InterfaceBackEnd
{
	public:

		virtual            ~MODBUS_InterfaceBackEnd	() {}

		// --- MASTER path ---
		virtual bool 		Queue					(MODBUS_Command_t& Command) 			= 0;   	// Queue a command for transmission
		virtual bool 		IsBusy					(void) 									= 0;    // Backend is executing a command

		// --- SLAVE path ---
		virtual bool		HasRequest				(void) const							= 0;    // A complete RTU/TCP request is ready
		virtual bool		GetRequest				(const uint8_t** ppRX, size_t* pLength) = 0; 	// Retrieve the request buffer

		// --- Address filtering ---
		virtual bool 		CanHandle				(uint8_t DeviceAddress) 				= 0;   	// Backend handles this address range

		// --- Processing ---
		virtual void 		Process					(void) 									= 0;    // Non-blocking state machine
        virtual int         Send                    (const uint8_t* pData, size_t Length)   = 0;


		// --- Manager injection ---
		virtual void 		SetManager				(class MODBUS_Manager* pManager) 		= 0;

		// --- TX buffer access (SLAVE response) ---
		virtual uint8_t*	GetTXBuffer				(void) 									= 0;
		virtual size_t   	GetTXBufferSize			(void) const 							= 0;
};

//-------------------------------------------------------------------------------------------------

class MODBUS_Manager
{
    public:

        int                 BuildFrame                  (MODBUS_Command_t& Command, uint8_t* pOut, size_t MaxLength);
        int                 ParseResponse               (MODBUS_Command_t& Command, const uint8_t* pIn, size_t Length);
		int 				HandleRequest				(const uint8_t* pRX, size_t RX_Length, uint8_t* pTX, size_t TX_Max, size_t* pTX_Length);
		int 				ParseRequest				(const uint8_t* pRX, size_t RX_Length, MODBUS_Command_t& Command);
        int                 ParsePayload                (MODBUS_Command_t& Command, uint8_t Function, const uint8_t* pIn, size_t Length);
		void 				SetApplication				(class MODBUS_Application* pApp)		{ m_pApplication = pApp; }

    private:

		int 				BuildException				(uint8_t Address, uint8_t Function, uint8_t ExceptionCode, uint8_t* pTX, size_t TX_MaxLength, size_t* pTX_Length);
        bool                ValidateCRC                 (const uint8_t* pData, size_t Length);
        int                 BuildPayload                (MODBUS_Command_t& Command, uint8_t* pOut, size_t MaxLength);
		MODBUS_AppEntry_t* 	FindHandler					(uint8_t DeviceAddress, uint8_t Function);

		MODBUS_Application* m_pApplication;
};

//-------------------------------------------------------------------------------------------------

class MODBUS_Router
{
    public:

        nOS_Error       	Initialize         			(void);
        void            	Run                			(void);

        bool                RegisterEndpoint            (MODBUS_InterfaceBackEnd* pBackEnd);
      #if (MODBUS_USE_ROUTER_PASSTHRU == DEF_DISABLED)
		bool				RegisterPassThru         	(const MODBUS_PassThruRule_t& PassThruRule);
      #endif
        bool                Queue                       (MODBUS_Command_t& Command);
        bool                IsBusy                      (void);
        bool                CanHandle                   (uint8_t DeviceAddress);

    private:

        nOS_Thread      					m_Handle;
        nOS_Stack       					m_Stack[TASK_MODBUS_STACK_SIZE];
		MODBUS_Manager 						m_Manager;

        MODBUS_InterfaceBackEnd*        	m_BackEnds          [MODBUS_MAX_BACKENDS];

      #if (MODBUS_USE_ROUTER_PASSTHRU == DEF_DISABLED)
		static 	MODBUS_PassThruRule_t 		m_PassThruRules     [MODBUS_MAX_PASSTHRU_RULES];
		static  size_t            			m_PassThruCount;   	// Number of used passthru entries (static + dynamic)
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

MODBUS_EXTERN class MODBUS_Router	  	myMODBUS_Router;

//-------------------------------------------------------------------------------------------------

#endif //(DIGINI_USE_MODBUS == DEF_ENABLED)
