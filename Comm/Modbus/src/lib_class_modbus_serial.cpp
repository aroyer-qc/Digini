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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_RTU_GLOBAL
#include "./lib_digini.h"
#undef  MODBUS_RTU_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_MODBUS == DEF_ENABLED) && (DIGINI_USE_SERIAL_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_RTU_SILENT_INTERVAL_MSEC   			1		// <- Put this into the config file for modbus
#define MODBUS_RTU_RX_NB_OF_SEMAPHORE_COUNT         8
#define MODBUS_RTU_FIFO_RX_SIZE         			MODBUS_RTU_MAX_FRAME_SIZE

//-------------------------------------------------------------------------------------------------

void ModbusRTU::Initialize(MODBUS_Manager* pManager, UART_Driver* pUartDriver, IO_ID_e RE_DE_ControlPin, uint8_t MinDeviceAddress, uint8_t MaxDeviceAddress)
{
    m_pManager         = pManager;
    m_pUartDriver      = pUartDriver;
    m_RE_DE_ControlPin = RE_DE_ControlPin;
    m_MinDeviceAddress     = MinDeviceAddress;
    m_MaxDeviceAddress     = MaxDeviceAddress;

    m_Fifo.Initialize(CON_FIFO_PARSER_RX_SIZE);
    m_pRX_Buffer = m_Fifo.GetBufferPointer();

    nOS_SemCreate(&m_RX_IdleSem, 0, MODBUS_RTU_RX_NB_OF_SEMAPHORE_COUNT);
    pUartDriver->DMA_ConfigRX(m_pRX_Buffer, MODBUS_RTU_FIFO_RX_SIZE);                // DMA will use the FIFO buffer allocated memory
    pUartDriver->RegisterCallback((CallbackInterface*)this);
    pUartDriver->EnableCallbackType(UART_CALLBACK_RX_IDLE | UART_CALLBACK_TX_COMPLETED | UART_CALLBACK_RX_ERROR);
}

//-------------------------------------------------------------------------------------------------

void ModbusRTU::Process(void)
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
            m_pTX_Buffer = (uint8_t*)pMemoryPool->Alloc(MODBUS_RTU_MAX_FRAME_SIZE, MEM_DBG_MB_SERIAL);

            if(m_pTX_Buffer == nullptr)
            {
                m_State = MODBUS_ERROR;
                return;
            }

            // Ask the manager to build the RTU frame
            int FrameLength = m_pManager->BuildFrame(m_Command, m_pTX_Buffer, MODBUS_RTU_MAX_FRAME_SIZE);

            if(FrameLength <= 0)
            {
                // Invalid command or buffer too small
                m_State = MODBUS_ERROR;
                return;
            }

            m_pTX_Length = (size_t)FrameLength;

            // Next step: send the frame
            m_State = MODBUS_SEND_FRAME;
        }
        break;

        case MODBUS_SEND_FRAME:
        {
            int Sent = Send(m_pTX_Buffer, m_pTX_Length);

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
                m_RX_Length = 0;
                m_StartTick = GetTick();

                m_State = MODBUS_WAIT_RESPONSE;
            }
        }
        break;

		case MODBUS_WAIT_RESPONSE:
		{
			// Wait for incoming data notification (non-blocking)
			if(nOS_SemTake(&m_RX_IdleSem, 0) == NOS_OK)
			{
				// Read all available bytes from the UART FIFO
				int Count = (int)m_Fifo.Read(&m_pRX_Buffer[m_RX_Length], MODBUS_RTU_MAX_FRAME_SIZE - m_RX_Length);

				if(Count < 0)
				{
					m_State = MODBUS_ERROR;
					return;
				}

				m_RX_Length += Count;

				// Check if the RTU frame is complete
				if(IsEndOfRTU_Frame(m_pRX_Buffer, m_RX_Length))
				{
					m_State = MODBUS_PARSE_RESPONSE;
					return;
				}
			}

			// Check global Modbus response timeout
			if(TickHasTimeOut(m_StartTick, m_Command.TimeoutMsec))
			{
				m_State = MODBUS_ERROR;
				return;
			}
		}
		break;

        case MODBUS_PARSE_RESPONSE:
        {
            int Status = m_pManager->ParseResponse(m_Command, m_pRX_Buffer, m_RX_Length);

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
    if(m_pUartDriver == nullptr)
    {
        return -1;
    }

    IO_SetPinHigh(m_RE_DE_ControlPin);
    return m_pUartDriver->SendData(pData, &Length);
}

//-------------------------------------------------------------------------------------------------

int ModbusRTU::Received(uint8_t* pBuffer, size_t MaxLength)
{
    if(m_pUartDriver == nullptr)
	{
        return -1;
	}

    // Read all that is available from the fifo
    return (int)m_Fifo.Read(pBuffer, MaxLength);
}

//-------------------------------------------------------------------------------------------------

bool ModbusRTU::Queue(MODBUS_Command_t& Command)
{
    if(m_HasPending == true)                						// Already busy?
    {
        return false;
    }

    m_Command    = Command;                 						// Accept command
    m_HasPending = true;
    m_State      = MODBUS_BUILD_FRAME;      						// Start state machine

    return true;
}

//-------------------------------------------------------------------------------------------------

bool ModbusRTU::IsEndOfRTU_Frame(const uint8_t* pBuffer, size_t Length)
{
    if(Length < 4)													// Need at least: address + function + CRC(2)
    {
        return false;
    }

    uint8_t Function = pBuffer[1];

    switch(Function)
    {
        // Functions with field ByteCount
        case MODBUS_READ_COILS:
        case MODBUS_READ_DISCRETE_INPUTS:
        case MODBUS_READ_HOLDING_REGISTERS:
        case MODBUS_READ_INPUT_REGISTERS:
        {
            if(Length < 3)											// Need: address + function + bytecount
            {
                return false;
            }

            uint8_t ByteCount = pBuffer[2];
            size_t Expected = 3 + ByteCount + 2;					// addr + func + bytecount + data + CRC

            return (Length >= Expected);
        }

        // Functions Write Single
        case MODBUS_WRITE_SINGLE_COIL: 								// Write Single Coil
        case MODBUS_WRITE_SINGLE_REGISTER: 							// Write Single Register
        {
            return (Length >= 8); 									// addr + func + addr_hi + addr_lo + val_hi + val_lo + CRC(2)
        }

        // Functions Write Multiple
        case MODBUS_WRITE_MULTIPLE_COILS:
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            return (Length >= 8);                                  	// addr + func + addr_hi + addr_lo + qty_hi + qty_lo + CRC(2)
        }

        // Exception responses
        default:
        {
            if(Function & MODBUS_EXCEPTION_RESPONSE)
            {
                return (Length >= 5);                              	// addr + func + exception_code + CRC(2)
            }
        }
        break;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

bool ModbusRTU::CanHandle(uint8_t Address)
{
    return (Address >= m_MinDeviceAddress) && (Address <= m_MaxDeviceAddress);
}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           CallbackFunction
//
//  Parameter(s):   void
//
//  Return:         None
//
//  Description:    Check if password is valid parsing the FIFO
//
//-------------------------------------------------------------------------------------------------
void ModbusRTU::CallbackFunction(int Type, void* pContext)
{
    switch(Type)
    {
        // When DMA transfert is complete.
      #if (UART_DRIVER_DMA_TX_COMPLETED_CFG == DEF_ENABLED)
        case UART_CALLBACK_TX_DMA:
        {
            pMemoryPool->Free((void**)&pContext);
            IO_SetPinLow(m_RE_DE_ControlPin);
        }
        break;
      #endif

        // TX from uart is completed then release memory.
      #if (UART_DRIVER_TX_COMPLETED_CFG == DEF_ENABLED)
        case UART_CALLBACK_TX_COMPLETED:
        {
            pMemoryPool->Free((void**)&pContext);
            IO_SetPinLow(m_RE_DE_ControlPin);
        }
        break;
      #endif

      #if (UART_DRIVER_RX_NOT_EMPTY_CFG == DEF_ENABLED)                         // Don't know if we need to keep this... this mode is never use!!
        case UART_CALLBACK_RX_NOT_EMPTY:
        {
            uint8_t* pData = (uint8_t*)pContext;
            //m_Fifo.Write(pData, 1);
            nOS_SemGive(&m_RX_IdleSem);
        }
        break;
      #endif

      #if (UART_DRIVER_RX_IDLE_CFG == DEF_ENABLED)
        case UART_CALLBACK_RX_IDLE:
        {
            UART_Transfer_t* pTransfer = (UART_Transfer_t*)pContext;
            m_Fifo.SetNewHeadPosition(pTransfer->u.Head);
            nOS_SemGive(&m_RX_IdleSem);
        }
        break;
      #endif

        case UART_CALLBACK_RX_ERROR:
        {
            __asm("nop");
            // nothing so far
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------

#endif  // (DIGINI_USE_MODBUS == DEF_ENABLED) && (DIGINI_USE_SERIAL_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
