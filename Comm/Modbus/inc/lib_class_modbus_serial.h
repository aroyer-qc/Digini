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

#if (DIGINI_USE_MODBUS == DEF_ENABLED) && (DIGINI_USE_SERIAL_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_RTU_MAX_FRAME_SIZE       MODBUS_MAX_PDU_SIZE + 2   		// CRC16
#define MODBUS_SERIAL_OUT_SIZE          MODBUS_RTU_MAX_FRAME_SIZE

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum MODBUS_State_e
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

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class ModbusRTU : public ChildProcessInterface, public MODBUS_InterfaceBackEnd , public CallbackInterface
{
    public:

        void                IF_Process              (void);


//        void                Initialize              (MODBUS_Manager* pManager, Console* pConsole, uint8_t MinID, uint8_t MaxID);
        void                Initialize              (MODBUS_Manager* pManager, UART_Driver* pUartDriver, uint8_t MinID, uint8_t MaxID);

        int                 Send                    (const uint8_t* pData, size_t Length);
        int                 Received                (uint8_t* pBuffer, size_t MaxLength);
        bool                Queue                   (const MODBUS_Command_t& Command);
        bool                CanHandle               (uint8_t UnitID) override;

        void                CallbackFunction        (int Type, void* pContext);

    private:

        bool                IsEndOfRTU_Frame        (const uint8_t* pBuffer, size_t Length);


        //Console*            m_pConsole              = nullptr;
		UART_Driver*		m_pUartDriver           = nullptr;
        MODBUS_State_e      m_State                 = MODBUS_IDLE;
        MODBUS_Manager*     m_pManager              = nullptr;

        FIFO_Buffer         m_Fifo;
        nOS_Sem             m_RX_IdleSem;

        uint8_t             m_MinUnitID;
        uint8_t             m_MaxUnitID;

        uint8_t*            m_pTX_Buffer            = nullptr;
        size_t              m_pTX_Length            = 0;

        uint8_t*            m_pRX_Buffer            = nullptr;
        size_t              m_RX_Length             = 0;

        MODBUS_Command_t    m_Command;
        bool                m_HasPending            = false;

        uint32_t            m_StartTick             = 0;
        uint32_t            m_SilentTick            = 0;
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_MODBUS == DEF_ENABLED) && (DIGINI_USE_SERIAL_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
