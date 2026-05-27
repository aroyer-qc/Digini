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
//
//  Name:           Initialize
//
//  Parameters:     pUartDriver     	- Pointer to the UART driver used for RTU communication
//                  Mode            	- Initial operating mode (MASTER or SLAVE)
//                  RE_DE_ControlPin	- GPIO pin controlling RS‑485 driver direction (RE/DE)
//                  MinDeviceAddress	- Lowest valid slave address accepted in SLAVE mode
//                  MaxDeviceAddress	- Highest valid slave address accepted in SLAVE mode
//
//  Returns:        None
//
//  Description:    Initializes the Modbus RTU backend. This function configures the UART
//                  driver, sets up the RX FIFO buffer, registers the UART callback handler,
//                  and enables the required UART interrupt types (RX idle, TX completed,
//                  RX error). The RS‑485 direction‑control pin is stored for later use by
//                  the Send() routine. The backend mode (MASTER or SLAVE) is also stored,
//                  allowing the processing logic to adapt its behavior accordingly.
//
//-------------------------------------------------------------------------------------------------
void ModbusRTU::Initialize(UART_Driver* pUartDriver,  MODBUS_Mode_e Mode, IO_ID_e RE_DE_ControlPin, uint8_t MinDeviceAddress, uint8_t MaxDeviceAddress)
{
    m_pUartDriver      = pUartDriver;
    m_RE_DE_ControlPin = RE_DE_ControlPin;
	m_Mode             = Mode;

    m_Fifo.Initialize(MODBUS_RTU_FIFO_RX_SIZE);
    m_pRX_Buffer = m_Fifo.GetBufferPointer();

    nOS_SemCreate(&m_RX_IdleSem, 0, MODBUS_RTU_RX_NB_OF_SEMAPHORE_COUNT);

    pUartDriver->Initialize();
    pUartDriver->DMA_ConfigRX(m_pRX_Buffer, MODBUS_RTU_FIFO_RX_SIZE);
    pUartDriver->RegisterCallback(this);
    pUartDriver->EnableCallbackType(UART_CALLBACK_RX_IDLE | UART_CALLBACK_TX_COMPLETED | UART_CALLBACK_RX_ERROR);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameters:     None
//
//  Returns:        None
//
//  Description:    Main state machine of the Modbus RTU backend. This function handles both
//                  SLAVE and MASTER operation paths. In SLAVE mode, it monitors the RX idle
//                  semaphore to detect the end of an incoming RTU request, validates the frame,
//                  and signals the Router when a complete request is available. In MASTER mode,
//                  it builds the outgoing RTU frame, transmits it, enforces the mandatory silent
//                  interval, waits for the slave response, and delegates response parsing to the
//                  MODBUS manager. All transitions between IDLE, BUILD, SEND, WAIT, PARSE, DONE,
//                  and ERROR states are handled internally, ensuring non‑blocking operation and
//                  strict Modbus RTU timing compliance.
//
//-------------------------------------------------------------------------------------------------
void ModbusRTU::Process(void)
{
    switch(m_State)
    {
        case MODBUS_IDLE:
        {
            // --- SLAVE path: detect a complete RTU request ---
            if(nOS_SemTake(&m_RX_IdleSem, 0) == NOS_OK)
            {
                int Count = m_Fifo.Read(m_pRX_Buffer, MODBUS_RTU_MAX_FRAME_SIZE);

                if(Count > 0)
                {
                    m_RX_Length = (size_t)Count;

                    // Check if a complete RTU request has been received
                    if(IsEndOfRTU_Request(m_pRX_Buffer, m_RX_Length) == true)
                    {
                        // Is this a device address handled by this backend?
                        if(CanHandle(m_pRX_Buffer[0]))
                        {
                            // Notify the Router that a SLAVE request is ready
                            m_SlaveHasRequest = true;
                        }
                    }
                }

                // Do not block; exit Process()
                return;
            }

            // --- MASTER path: a command has been queued() ---
            if(m_MasterHasPending == false)
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

            // Ask the manager to build the RTU frame (MASTER)
            size_t FrameLength = MODBUS_RTU_MAX_FRAME_SIZE;
            SystemState_e State = m_pManager->MasterBuildFrame(m_Command, m_pTX_Buffer, &FrameLength);

            if(State != SYS_READY)
            {
                m_State = MODBUS_ERROR;
                return;
            }

            m_pTX_Length = FrameLength;

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

            m_SilentTick = GetTick();
            m_State      = MODBUS_WAIT_SILENT;
        }
        break;

        case MODBUS_WAIT_SILENT:
        {
            // Wait for the mandatory silent interval before listening for a response
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
            // Check if new RX data has arrived
            if(nOS_SemTake(&m_RX_IdleSem, 0) == NOS_OK)
            {
                int Count = (int)m_Fifo.Read(&m_pRX_Buffer[m_RX_Length], MODBUS_RTU_MAX_FRAME_SIZE - m_RX_Length);

                if(Count < 0)
                {
                    m_State = MODBUS_ERROR;
                    return;
                }

                m_RX_Length += (size_t)Count;

                // Check if the full RTU response frame has been received
                if(IsEndOfRTU_Frame(m_pRX_Buffer, m_RX_Length))
                {
                    m_State = MODBUS_PARSE_RESPONSE;
                    return;
                }
            }

            // Timeout waiting for the slave response
            if(TickHasTimeOut(m_StartTick, m_Command.TimeoutMsec))
            {
                // Notify the manager a MASTER timeout has occured.
                m_pManager->MasterTimeOut(m_Command.SlotIndex);
                m_State = MODBUS_ERROR;
                return;
            }
        }
        break;

        case MODBUS_PARSE_RESPONSE:
        {
            // Delegate response parsing to the manager
            m_pManager->MasterHandleResponse(m_pRX_Buffer, m_RX_Length);
            m_State = MODBUS_DONE;
        }
        break;

        case MODBUS_DONE:
        {
            // Clear MASTER and SLAVE flags and return to IDLE
            m_SlaveHasRequest  = false;
            m_MasterHasPending = false;
            m_State            = MODBUS_IDLE;
        }
        break;

        case MODBUS_ERROR:
        {
            // Reset state after any error
            m_SlaveHasRequest  = false;
            m_MasterHasPending = false;
            m_State            = MODBUS_IDLE;
        }
        break;

        default:
            break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameters:     pData       - Pointer to the buffer containing the RTU frame to transmit
//                  Length      - Number of bytes to send
//
//  Returns:        >= 0        - Number of bytes accepted by the UART driver
//                  < 0         - Transmission could not be started (UART not initialized)
//
//  Description:    Sends a Modbus RTU frame over the UART interface. Before transmission, the
//                  RS‑485 driver‑enable pin (RE/DE) is asserted to switch the transceiver into
//                  transmit mode. The actual byte transfer is delegated to the UART driver,
//                  which may use DMA or interrupt‑driven transmission depending on its
//                  implementation. The RE/DE pin is released later in the UART callback once
//                  transmission is fully completed.
//
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
//
//  Name:           Received
//
//  Parameters:     pBuffer     - Destination buffer where received UART bytes will be copied
//                  MaxLength   - Maximum number of bytes that can be written into pBuffer
//
//  Returns:        >= 0        - Number of bytes read from the internal FIFO
//                  < 0         - UART driver not initialized
//
//  Description:    Retrieves all currently available bytes from the Modbus RTU receive FIFO.
//                  This function is fully non‑blocking: it simply drains the FIFO up to
//                  MaxLength bytes and returns the number of bytes copied. No timeout logic is
//                  applied here; Modbus RTU timing (silence detection, frame boundaries) is
//                  handled elsewhere in the state machine.
//
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
//
//  Name:           Queue
//
//  Parameters:     Command     - Modbus master request to enqueue for transmission
//
//  Returns:        true        - The command was accepted and the state machine was started
//                  false       - A command is already pending (busy)
//
//  Description:    Queues a Modbus master request for processing. If no other request is currently
//                  pending, the command is copied into the internal buffer, the pending flag is set,
//                  and the Modbus RTU master state machine is initialized to the BUILD_FRAME state.
//                  Only one request can be active at a time; additional requests are rejected until
//                  the current transaction completes.
//
//-------------------------------------------------------------------------------------------------
bool ModbusRTU::Queue(MODBUS_Command_t& Command)
{
    if(m_MasterHasPending == true)                						// Already busy?
    {
        return false;
    }

    m_Command          = Command;                 						// Accept command
    m_MasterHasPending = true;
    m_State            = MODBUS_BUILD_FRAME;      						// Start state machine

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsEndOfRTU_Frame
//
//  Parameters:     pBuffer     - Pointer to the partially received Modbus RTU frame
//                  Length      - Number of bytes currently accumulated in the buffer
//
//  Returns:        true        - The buffer contains a complete Modbus RTU **response** frame
//                  false       - The frame is incomplete or the function code does not match
//                                any known fixed‑length or byte‑count‑based format
//
//  Description:    Determines whether a Modbus RTU **response** frame (MASTER side) has been fully
//                  received based solely on length rules defined by the Modbus specification.
//                  No CRC validation is performed here; this function is used by the RTU state
//                  machine to detect frame boundaries.
//
//                  The logic follows the Modbus PDU formats:
//
//                      • Read functions (0x01, 0x02, 0x03, 0x04):
//                            [Addr][Func][ByteCount][Data...][CRC_L][CRC_H]
//                            -> ByteCount determines total frame length.
//
//                      • Write Single (0x05, 0x06):
//                            Fixed 8‑byte response.
//
//                      • Write Multiple (0x0F, 0x10):
//                            Fixed 8‑byte response.
//
//                      • Exception responses (Func | 0x80):
//                            Always 5 bytes.
//
//                  The function returns true only when the accumulated bytes match or exceed the
//                  expected length for the detected function code.
//
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
//
//  Name:           IsEndOfRTU_Request
//
//  Parameters:     pBuffer     - Pointer to the partially received Modbus RTU **request** frame
//                  Length      - Number of bytes currently accumulated in the buffer
//
//  Returns:        true        - The buffer contains a complete Modbus RTU **request** frame
//                  false       - The frame is incomplete or does not match the expected format
//
//  Description:    Determines whether a full Modbus RTU **request** frame (SLAVE side) has been
//                  received. Unlike response frames, request frames have fixed lengths for most
//                  function codes:
//
//                      • Read functions (0x01, 0x02, 0x03, 0x04):
//                            Always 8 bytes.
//
//                      • Write Single (0x05, 0x06):
//                            Always 8 bytes.
//
//                      • Write Multiple (0x0F, 0x10):
//                            Variable length:
//                                [Addr][Func][Addr_H][Addr_L][Qty_H][Qty_L][ByteCount][Data...]
//                                [CRC_L][CRC_H]
//
//                  The function uses the ByteCount field (when applicable) to compute the exact
//                  expected frame length. CRC is **not** validated here; this function is used
//                  strictly for RTU framing and end‑of‑frame detection.
//
//-------------------------------------------------------------------------------------------------
bool ModbusRTU::IsEndOfRTU_Request(const uint8_t* pBuffer, size_t Length)
{
    // Minimum: addr + func + CRC(2)
    if(Length < 4)
    {
        return false;
    }

    uint8_t Function = pBuffer[1];

    switch(Function)
    {
        // READ functions: always 8 bytes request
        case MODBUS_READ_COILS:
        case MODBUS_READ_DISCRETE_INPUTS:
        case MODBUS_READ_HOLDING_REGISTERS:
        case MODBUS_READ_INPUT_REGISTERS:
        {
            return (Length == 8);
        }

        // WRITE SINGLE: always 8 bytes request
        case MODBUS_WRITE_SINGLE_COIL:
        case MODBUS_WRITE_SINGLE_REGISTER:
        {
            return (Length == 8);
        }

        // WRITE MULTIPLE: variable length
        case MODBUS_WRITE_MULTIPLE_COILS:
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            if(Length < 7)
            {
                return false;
            }

            uint8_t ByteCount = pBuffer[6];
            size_t Expected = 7 + ByteCount + 2;   // header + bytecount + data + CRC

            return (Length == Expected);
        }

        default:
            return false;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetRequest
//
//  Parameters:     ppRX        - Output: pointer to the internal RX buffer containing the request
//                  pLength     - Output: number of bytes stored in the RX buffer
//
//  Returns:        true        - A pending request was available and has been returned
//                  false       - No pending request or invalid output pointers
//
//  Description:    Retrieves the next fully received Modbus RTU **request** accumulated by the
//                  SLAVE backend. This function is called by the Router once IsEndOfRTU_Request()
//                  has detected a complete frame. The function exposes the internal RX buffer and
//                  its length, then clears the pending flag so the backend can accept the next
//                  incoming request. No parsing or validation is performed here; this function
//                  simply hands off the raw frame to the upper layers.
//
//-------------------------------------------------------------------------------------------------
bool ModbusRTU::GetRequest(const uint8_t** ppRX, size_t* pLength)
{
    if(ppRX == nullptr || pLength == nullptr)
    {
        return false;
    }

    if(m_SlaveHasRequest == false)
    {
        return false;
    }

    *ppRX   = m_pRX_Buffer;
    *pLength = m_RX_Length;

    // The request has been consume by the router
    m_SlaveHasRequest = false;

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CanHandle
//
//  Parameters:     SlaveID     - Modbus slave address extracted from the incoming request
//
//  Returns:        true        - This RTU backend is responsible for this slave address
//                  false       - The address is outside the configured slave address range
//
//  Description:    Determines whether this ModbusRTU instance is assigned to handle the specified
//                  Modbus slave address. The backend is configured with a contiguous range defined
//                  by FirstSlaveAddress and LastSlaveAddress, allowing it to represent either a
//                  single slave (first == last) or a block of consecutive slave IDs. The Router
//                  uses this check to select the appropriate backend when multiple RTU interfaces
//                  are active.
//
//-------------------------------------------------------------------------------------------------
bool ModbusRTU::CanHandle(uint8_t SlaveID)
{
    return (SlaveID == MODBUS_RTU_SLAVE_ID);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CallbackFunction
//
//  Parameters:     Type        - UART callback type (TX completed, RX idle, etc.)
//                  pContext    - Driver‑specific context pointer (DMA transfer, RX info, etc.)
//
//  Returns:        None
//
//  Description:    UART interrupt callback used by the ModbusRTU backend. This function handles
//                  UART TX completion (DMA or interrupt mode), releases the memory block associated
//                  with the transmission, and drives the RS‑485 RE/DE pin LOW to return to receive
//                  mode. For RX events, the function updates the FIFO head position when the UART
//                  reports an IDLE condition and signals the receive semaphore so the RTU state
//                  machine can process the newly received bytes.
//
//                  No Modbus parsing is performed here — this callback only handles UART‑level
//                  events and buffer management.
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
        }
        break;
      #endif

        // TX from uart is completed then release memory.
      #if (UART_DRIVER_TX_COMPLETED_CFG == DEF_ENABLED)
        case UART_CALLBACK_TX_COMPLETED:
        {
            IO_SetPinLow(m_RE_DE_ControlPin);
            m_Fifo.Flush(MODBUS_RTU_MAX_FRAME_SIZE);
        }
        break;
      #endif

      #if (UART_DRIVER_RX_NOT_EMPTY_CFG == DEF_ENABLED) // Don't know if we need to keep this... this mode is never use!! on STM32 will need to see on other CPU
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
