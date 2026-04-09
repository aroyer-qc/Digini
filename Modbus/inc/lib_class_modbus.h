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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_MAX_BACKENDS   8   // Pour le config plus tard!!

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

enum MODBUS_Backend_e
{
    MODBUS_BACKEND_LOCAL,
    MODBUS_BACKEND_TCP,
    MODBUS_BACKEND_RTU,
};

struct MODBUS_Command_t
{
    MODBUS_Backend_e   BackEnd;         // RTU, TCP, etc.
    uint8_t            UnitID;          // Slave address
    MODBUS_Function_e  Function;        // Function code
    uint16_t           Address;         // Starting address
    uint16_t           Quantity;        // Number of items
    uint16_t           Value;           // For single write
    uint16_t*          Data;            // For multiple write
    uint8_t*           ResultBuffer;    // For coils / discrete inputs
    uint16_t*          ResultRegisters; // For registers
    size_t             ResultLength;    // Number of bytes or registers
};

struct MODBUS_PassthruRule_t
{
    uint8_t     SrcUnitID;
    uint8_t     DstUnitID;
};

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class MODBUS_InterfaceBackEnd
{
    public:
        
        virtual             ~MODBUS_InterfaceBackEnd    ()                              {}

        virtual bool        Queue                       (const ModbusCommand& Command)  = 0;
        virtual void        Process                     (void)                          = 0;
        virtual bool        IsBusy                      (void)                          = 0;
        virtual bool        CanHandle                   (uint8_t UnitID)                = 0;
};

class MODBUS_Manager
{
    public:
    
        int                 BuildFrame                  (const MODBUS_Command_t& Command, uint8_t* pOut, size_t MaxLength);
        int                 ParseResponse               (const MODBUS_Command_t& Command, const uint8_t* pIn, size_t Length);
        int                 ParsePayload                (const MODBUS_Command_t& Command, uint8_t Function, const uint8_t* pIn, size_t Length);

    private:

        bool                ValidateCRC                 (const uint8_t* pData, size_t Length);
};

class MODBUS_Router
{
    public:

                            ModbusRouter            () = default;

        bool                RegisterEndpoint        (IModbusBackend* pBackEnd);

        bool                Queue                   (const ModbusCommand_t& Command);
        void                Process                 (void);
        bool                IsBusy                  (void);
        bool                CanHandle               (uint8_t UnitID);

    private:

        MODBUS_InterfaceBackEnd*        m_BackEnds[MODBUS_MAX_BACKENDS];
        MODBUS_PassthruRule_t           m_PassthruRules[MODBUS_MAX_RULES];
};