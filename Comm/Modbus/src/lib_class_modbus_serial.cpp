//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_modbus_serial.cpp
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
// ModbusRTU rtu1(&Console1, 1, 10);   // gère UnitID 1 à 10
// ModbusRTU rtu2(&Console2, 20, 30);  // gère UnitID 20 à 30
// ModbusRTU rtu3(&Console3, 100, 100); // gère seulement UnitID 100
//
//
//  define for MODBUS_RTU_SILENT_INTERVAL_MSEC 
//  +----------------+-------------------+----------------------+
//  | Baudrate (bps) | 1 char (ms)       | 3.5 chars (ms)       |
//  +----------------+-------------------+----------------------+
//  |     1200       |     9.166 ms      |     32.083 ms        |
//  |     2400       |     4.583 ms      |     16.041 ms        |
//  |     4800       |     2.291 ms      |      8.020 ms        |
//  |     9600       |     1.146 ms      |      4.010 ms        |
//  |    19200       |     0.573 ms      |      2.005 ms        |
//  |    38400       |     0.286 ms      |      1.002 ms        |
//  |    57600       |     0.191 ms      |      0.669 ms        |
//  |   115200       |     0.095 ms      |      0.333 ms        |
//  +----------------+-------------------+----------------------+
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_RTU_SILENT_INTERVAL_MSEC   1    // <- Put this into the config file for modbus

//-------------------------------------------------------------------------------------------------

void ModbusRTU::Initialize(MODBUS_Manager* pManager, Console* pConsole, uint8_t MinID, uint8_t MaxID)
{
    m_pManager  = pManager;
    m_pConsole  = pConsole;
    m_MinUnitID = MinID;
    m_MaxUnitID = MaxID;
}

//-------------------------------------------------------------------------------------------------

void ModbusRTU::IF_Process(void)
{
    switch(m_State)
    {
        case MODBUS_IDLE:
        {
            // No pending command → nothing to do
            if(m_HasPending == false)
            {
                return;
            }

            m_State = MODBUS_BUILD_FRAME;
        }
        break;
        
        case MODBUS_BUILD_FRAME:
        {
            m_pTxBuf = (uint8_t*)pMemoryPool->Alloc(MAX_MODBUS_FRAME_SIZE, 0);

            if(m_pTxBuf == nullptr)
            {
                m_State = MODBUS_ERROR;
                return;
            }

            // Ask the manager to build the RTU frame
            int FrameLen = m_pManager->BuildFrame(m_Command, m_pTxBuf, MAX_MODBUS_FRAME_SIZE);

            if(FrameLen <= 0)
            {
                // Invalid command or buffer too small
                m_State = MODBUS_ERROR;
                return;
            }

            m_TxLen = (size_t)FrameLen;

            // Next step: send the frame
            m_State = MODBUS_SEND_FRAME;
        }
        break;
        
        case MODBUS_SEND_FRAME:
        {
            int Sent = Send(m_pTxBuf, m_TxLen);

            if(Sent < 0)
            {
                m_State = MODBUS_ERROR;
                return;
            }

            // Start silent interval timer
            m_SilentTick = GetTick();

            m_State = MODBUS_WAIT_SILENT;
        }
        break;
        
        case MODBUS_WAIT_SILENT:
        {
            // Wait for the required silent interval before receiving
            if(TickHasTimeOut(m_SilentTick, MODBUS_RTU_SILENT_INTERVAL_MSEC))
            {
                m_RxLen     = 0;
                m_StartTick = GetTick();

                m_State = MODBUS_WAIT_RESPONSE;
            }
        }
        break;
        
        case MODBUS_WAIT_RESPONSE:
        {
            uint8_t Byte;
            int     Result;

            // Non-blocking read (timeout = 0)
            Result = Received(&Byte, 1, 0);

            if(Result < 0)
            {
                m_State = MODBUS_ERROR;
                return;
            }

            if(Result > 0)
            {
                if(m_RxLen < MAX_MODBUS_FRAME_SIZE)
                {
                    m_pRxBuf[m_RxLen++] = Byte;
                }
                else
                {
                    // RX buffer overflow
                    m_State = MODBUS_ERROR;
                    return;
                }

                // Check if the RTU frame is complete
                if(IsEndOfRTU_Frame(m_pRxBuf, m_RxLen))
                {
                    m_State = MODBUS_PARSE_RESPONSE;
                    return;
                }
            }

            // Global response timeout
            if(TickHasTimeOut(m_StartTick, m_Command.TimeoutMsec))
            {
                m_State = MODBUS_ERROR;
                return;
            }
        }
        break;
        
        case MODBUS_PARSE_RESPONSE:
        {
            int Status = m_pManager->ParseResponse(m_Command,
                                                   m_pRxBuf,
                                                   m_RxLen);

            if(Status < 0)
            {
                m_State = MODBUS_ERROR;
                return;
            }

            m_State = MODBUS_DONE;
        }
        break;
        
        case MODBUS_DONE:
        {
            // Command completed successfully
            m_HasPending = false;
            m_State      = MODBUS_IDLE;
        }
        break;

        case MODBUS_ERROR:
        {
            // Error occurred → reset state
            m_HasPending = false;
            m_State      = MODBUS_IDLE;
        }
        break;

        default:
            break;
    }
}

//-------------------------------------------------------------------------------------------------

int ModbusRTU::Send(const uint8_t* pData, size_t Length)
{
    if(m_pConsole == nullptr)
    {
        return -1;
    }

    return m_pConsole->Write(pData, Length);
}

//-------------------------------------------------------------------------------------------------

int ModbusRTU::Received(uint8_t* pBuffer, size_t MaxLength, TickCount_t TimeOutMsec)
{
    if(m_pConsole == nullptr)
    {
        return -1;
    }

    return m_pConsole->Read(pBuffer, MaxLength, TimeOutMsec);    
}

//-------------------------------------------------------------------------------------------------

bool ModbusRTU::Queue(const MODBUS_Command_t& Command)
{
    if(m_HasPending == true)                // Already busy?
    {
        return false;
    }

    m_Command    = Command;                 // Accept command
    m_HasPending = true;
    m_State      = MODBUS_BUILD_FRAME;      // Start state machine

    return true;
}

//-------------------------------------------------------------------------------------------------

bool ModbusRTU::IsEndOfRTU_Frame(const uint8_t* pBuf, size_t Len)
{
    if(Len < 4)
    {
        return false;                       // Address + function + CRC(2)
    }

    uint8_t function = pBuf[1];

    switch(function)
    {
        // Functions with field ByteCount
        case MODBUS_READ_COILS:
        case MODBUS_READ_DISCRETE_INPUTS:
        case MODBUS_READ_HOLDING_REGISTERS:
        case MODBUS_READ_INPUT_REGISTERS:
        {
            if(Len < 3)
            {
                return false;
            }

            uint8_t byteCount = pBuf[2];
            size_t expected = 3 + byteCount + 2; // addr + func + bytecount + data + CRC

            return (Len >= expected);
        }

        // Functions Write Single
        case MODBUS_WRITE_SINGLE_COIL: // Write Single Coil
        case MODBUS_WRITE_SINGLE_REGISTER: // Write Single Register
        {
            return (Len >= 8); // addr + func + addr_hi + addr_lo + val_hi + val_lo + CRC(2)
        }

        // Functions Write Multiple
        case MODBUS_WRITE_MULTIPLE_COILS:
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            return (Len >= 8);                                  // addr + func + addr_hi + addr_lo + qty_hi + qty_lo + CRC(2)
        }

        // Exception responses
        default:
        {
            if(function & 0x80)
            {
                return (Len >= 5);                              // addr + func + exception_code + CRC(2)
            }
        }
        break;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool ModbusRTU::CanHandle(uint8_t UnitID)
{
    return (UnitID >= m_MinUnitID) && (UnitID <= m_MaxUnitID); 
}

//-------------------------------------------------------------------------------------------------
