//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_modbus.cpp
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

#define MODBUS_GLOBAL
#include "./lib_digini.h"
#undef  MODBUS_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Macro(s)
//-------------------------------------------------------------------------------------------------

#define MODBUS_INIT_PASSTHRU_ENTRY(DEV_SRC_ADDRESS, DEV_DST_ADDRESS) m_PassThruRules[m_PassThruCount++] = { DEV_SRC_ADDRESS, DEV_DST_ADDRESS };

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildFrameSlave
//
//  Parameters:     Command     - Parsed Modbus request received from the master
//                  Response    - Application-generated response (normal or exception)
//                  pOut        - Output buffer where the complete Modbus RTU frame is written
//                  MaxLength   - Maximum number of bytes available in pOut
//
//  Returns:        > 0         - Total frame length written to pOut
//                  0           - Frame could not be built (invalid payload or insufficient space)
//
//  Description:    Builds a complete Modbus RTU slave response frame. The function writes the
//                  DeviceAddress, the response Function code (normal or exception), the response
//                  payload, and finally appends the Modbus CRC16. If the payload does not fit in
//                  the output buffer, the function returns 0 to indicate failure.
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::BuildFrameSlave(const MODBUS_Command_t& Command, const MODBUS_SlaveResponse_t& Response, uint8_t* pOut, size_t MaxLength)
{
    size_t Index = 0;

    // Address + Function
    pOut[Index++] = Command.DeviceAddress;
    pOut[Index++] = Response.IsException ? (Command.Function | 0x80) : Command.Function;

    // Copy payload
    if(Index + Response.PayloadLength + 2 > MaxLength)
	{
        return 0;
	}

    memcpy(&pOut[Index], Response.pPayload, Response.PayloadLength);
    Index += Response.PayloadLength;

    // CRC
    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = ModbusCRC.CalculateBuffer(pOut, Index);

    pOut[Index++] = (uint8_t)(ComputedCRC & 0xFF);
    pOut[Index++] = (uint8_t)(ComputedCRC >> 8);

    return (int)Index;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           MasterRequest
//
//  Parameters:     RequestID   - Unique identifier of the registered master request entry.
//                  Quantity    - Quantity field to apply to the Modbus command associated with
//                                the specified RequestID.
//
//  Returns:        true        - The request entry was found, updated, and marked as pending.
//                  false       - Invalid RequestID or the request is already pending.
//
//  Description:    Retrieves the MODBUS_MasterEntry_t structure associated with the provided
//                  RequestID, updates its command parameters (including the Quantity field), and
//                  marks the request as pending with a timestamp for timeout supervision. This
//                  function does not build or transmit the Modbus RTU frame itself; frame
//                  construction, buffer allocation, and transmission are performed later by the
//                  backend/Router using the information stored in the master request entry.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Manager::MasterRequest(uint32_t RequestID, uint16_t Quantity)
{
    MODBUS_MasterEntry_t* pEntry = m_pApplication->FindMasterRequest(RequestID);

    if(pEntry == nullptr)
    {
        return false;
    }

    if(pEntry->IsPending == true)
    {
        return false;
    }

    pEntry->Command.Quantity = Quantity;
    pEntry->IsPending        = true;
    pEntry->TimestampStart   = GetTick();

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildFrameMaster
//
//  Parameters:     Command     - Modbus request to serialize (read/write operation)
//                  pOut        - Output buffer where the complete Modbus RTU frame is written
//                  MaxLength   - Maximum number of bytes available in pOut
//
//  Returns:        > 0         - Total frame length written to pOut
//                  0           - Frame could not be built (invalid payload or insufficient space)
//
//  Description:    Builds a complete Modbus RTU master request frame. The function writes the
//                  DeviceAddress, Function code, and the request payload generated by
//                  BuildPayload().
//                  It then appends the Modbus CRC16. If the payload cannot be generated or the
//                  output buffer is too small, the function returns 0 to indicate failure.
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::BuildFrameMaster(MODBUS_Command_t& Command, uint8_t* pOut, size_t MaxLength)
{
    size_t Index = 0;

    // Address + Function
    pOut[Index++] = Command.DeviceAddress;
    pOut[Index++] = Command.Function;

    // Request payload (address, quantity, value…)
    int PayloadLength = BuildPayload(Command, &pOut[Index], MaxLength - Index);

	if(PayloadLength < 0)
    {
        return 0;
    }

    Index += PayloadLength;

    // CRC
    if(Index + 2 > MaxLength)
    {
        return 0;
    }

    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = ModbusCRC.CalculateBuffer(pOut, Index);

    pOut[Index++] = (uint8_t)(ComputedCRC & 0xFF);     // CRC Low
    pOut[Index++] = (uint8_t)(ComputedCRC >> 8);       // CRC High

    return (int)Index;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HandleResponse
//
//  Parameters:     pRX         - Pointer to the received MODBUS RTU frame
//                  RX_Length   - Total number of bytes in the received frame
//
//  Returns:        true        - The response frame was valid, successfully parsed, and the
//                                corresponding master request callback was invoked.
//                  false       - Invalid frame (CRC failure, malformed payload, insufficient
//                                length), unknown RequestID, or no matching pending request.
//
//  Description:    Processes a complete Modbus RTU response received from the Router/backend.
//                  The function delegates frame decoding to ParseResponse(), which validates the
//                  CRC, extracts the function code, identifies Modbus exceptions, retrieves the
//                  RequestID, and copies the payload into a MODBUS_Response_t structure.
//
//                  Once parsed, the RequestID is used to locate the corresponding master request
//                  entry via MODBUS_Application::FindMasterRequest(). If a matching entry is
//                  found, the request is marked as completed (IsPending cleared), and the
//                  application‑provided callback is invoked with the decoded response.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Manager::HandleResponse(const uint8_t* pRX, size_t RX_Length)
{
    MODBUS_MasterResponse_t Response;
    int Status = ParseResponse(pRX, RX_Length, Response);

    if(Status < 0)
    {
        return false;
    }

    // Find matching master request
    MODBUS_MasterEntry_t* pEntry = m_pApplication->FindMasterRequest(Response.RequestID);

    if(pEntry == nullptr)
    {
        return false;
    }

    // Clear pending state
    pEntry->IsPending = false;

    // Invoke application callback
    if(pEntry->pCallback != nullptr)
    {
        pEntry->pCallback(pEntry->RequestID, Response);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseResponse
//
//  Parameters:     pIn       - Pointer to the received Modbus RTU frame
//                  Length    - Total number of bytes in the received frame
//                  Response  - Structure filled with decoded response information
//
//  Returns:        >= 0      - Payload length (excluding RequestID and CRC) successfully parsed
//                  < 0       - Modbus exception code (returned as a negative value)
//                  -1        - Invalid frame (CRC failure, malformed payload, insufficient length)
//
//  Description:    Validates and interprets a received Modbus RTU response for master operation.
//                  The function checks minimal frame length, validates the CRC, and decodes the
//                  function code. If the function code indicates a Modbus exception
//                  (function | 0x80), the exception code is extracted and returned as a negative
//                  value, and the Response structure is marked as an exception. For normal
//                  responses, the first two bytes of the payload are interpreted as the RequestID,
//                  and the remaining bytes are copied into Response.pPayload. The decoded fields
//                  (DeviceAddress, Function, RequestID, exception status, and payload) are stored
//                  in the MODBUS_Response_t structure.
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::ParseResponse(const uint8_t* pIn, size_t Length, MODBUS_MasterResponse_t& Response)
{
    if(Length < 4)
    {
        return -1;
    }

    size_t Index = 0;

    Response.DeviceAddress = pIn[Index++];
    uint8_t Function       = pIn[Index++];

    if(ValidateCRC(pIn, Length) == false)
    {
        return -1;
    }

    // Exception response ?
    if(Function & MODBUS_EXCEPTION_RESPONSE)
    {
        Response.Function      = (uint8_t)(Function & ~MODBUS_EXCEPTION_RESPONSE);
        Response.IsException   = true;

        if(Length < 5) // Addr + Func + ExCode + CRC(2)
        {
            return -1;
        }

        Response.ExceptionCode = pIn[Index];
        Response.PayloadLength = 0;

        return -(int)Response.ExceptionCode;
    }

    Response.Function    = Function;
    Response.IsException = false;

    // Payload length excluding CRC
    size_t PayloadLength = Length - Index - 2;
    if(PayloadLength < 2) // Need at least RequestID (2 bytes)
    {
        return -1;
    }

    // First 2 bytes of payload = RequestID
    Response.RequestID = (uint16_t)((pIn[Index] << 8) | pIn[Index + 1]);
    Index        += 2;
    PayloadLength -= 2;

    if(PayloadLength > Response.MaxPayloadLength)
    {
        return -1;
    }

    if(PayloadLength > 0 && Response.pPayload != nullptr)
    {
        memcpy(Response.pPayload, &pIn[Index], PayloadLength);
    }

    Response.PayloadLength = (uint16_t)PayloadLength;

    return (int)PayloadLength;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildPayload
//
//  Parameters:     Command     - Modbus master request describing the operation to encode
//                  pOut        - Output buffer where the PDU payload will be written
//                  MaxLength   - Maximum number of bytes available in pOut
//
//  Returns:        >= 0        - Number of payload bytes written
//                  < 0         - Payload could not be generated (unsupported function or insufficient space)
//
//  Description:    Encodes the Modbus PDU payload for master requests. This function serializes
//                  the address, quantity, value, and optional write‑multiple data depending on the
//                  function code. Only the PDU payload is generated here; the caller is responsible
//                  for writing the device address, function code, and CRC. For unsupported functions
//                  or when the output buffer is too small, a negative value is returned.
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::BuildPayload(MODBUS_Command_t& Command, uint8_t* pOut, size_t MaxLength)
{
    size_t Index = 0;

    switch(Command.Function)
    {
        // 0x03 - Read Holding Registers
        case MODBUS_READ_HOLDING_REGISTERS:
        {
            if(MaxLength < 4)
            {
                return -1;
            }

            pOut[Index++] = uint8_t(Command.Address >> 8);
            pOut[Index++] = uint8_t(Command.Address);

            pOut[Index++] = uint8_t(Command.Quantity >> 8);
            pOut[Index++] = uint8_t(Command.Quantity);

            return (int)Index;
        }

        // 0x06 - Write Single Register
        case MODBUS_WRITE_SINGLE_REGISTER:
        {
            if(MaxLength < 4)
            {
                return -1;
            }

            pOut[Index++] = uint8_t(Command.Address >> 8);
            pOut[Index++] = uint8_t(Command.Address);

            pOut[Index++] = uint8_t(Command.Value >> 8);
            pOut[Index++] = uint8_t(Command.Value);

            return (int)Index;
        }

        // 0x10 - Write Multiple Registers
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            size_t ByteCount = Command.Quantity * 2;

            if(MaxLength < (5 + ByteCount))
            {
                return -1;
            }

            pOut[Index++] = uint8_t(Command.Address >> 8);
            pOut[Index++] = uint8_t(Command.Address);

            pOut[Index++] = uint8_t(Command.Quantity >> 8);
            pOut[Index++] = uint8_t(Command.Quantity);

            pOut[Index++] = uint8_t(ByteCount);

            for(size_t i = 0; i < Command.Quantity; i++)
            {
                uint16_t Register = Command.pPayload[i];
                pOut[Index++] = uint8_t(Register >> 8);
                pOut[Index++] = uint8_t(Register);
            }

            return int(Index);
        }

        // Unsupported function
        default:
        {
            return -1;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ValidateCRC
//
//  Parameters:     pData       - Pointer to the received Modbus RTU frame
//                  Length      - Total number of bytes in the frame, including CRC
//
//  Returns:        true        - CRC is valid and matches the computed Modbus CRC16
//                  false       - Frame too short or CRC mismatch
//
//  Description:    Extracts the CRC16 value appended to the end of a Modbus RTU frame and
//                  recomputes the CRC over all preceding bytes. The function compares the
//                  received CRC with the computed one and returns true only if they match.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Manager::ValidateCRC(const uint8_t* pData, size_t Length)
{
    if(Length < 3)
    {
        return false;
    }

    uint16_t ReceivedCRC = (uint16_t)pData[Length - 2] | ((uint16_t)pData[Length - 1] << 8);
    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = (uint16_t)ModbusCRC.CalculateBuffer(pData, Length - 2);
    return (ReceivedCRC == ComputedCRC);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParsePayload
//
//  Parameters:     Command     - Original Modbus command containing buffers for parsed results
//                  Function    - Function code extracted from the response frame
//                  pIn         - Pointer to the payload of the response (after DeviceAddress/Function)
//                  Length      - Number of bytes available in the payload buffer
//
//  Returns:        0           - Payload successfully parsed
//                  -1          - Invalid or incomplete payload format
//                  < 0         - Reserved for exception codes (handled earlier in ParseResponse)
//
//  Description:    Interprets the function-specific payload of a Modbus RTU response. The function
//                  validates the payload length according to the expected format of the specified
//                  function code and extracts coil states or register values into the buffers
//                  provided in the Command structure. For write-type functions, the response is an
//                  echo of the request and requires no data extraction.
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::ParsePayload(MODBUS_Command_t& Command, uint8_t Function, const uint8_t* pIn, size_t Length)
{
    switch(Function)
    {
        case MODBUS_READ_COILS:
        case MODBUS_READ_DISCRETE_INPUTS:
        {
            if(Length < 1)
            {
                return -1;
            }

            uint8_t ByteCount = pIn[0];

            if(Length < (size_t)(1 + ByteCount))
            {
                return -1;
            }

            // Copy raw bytes into pPayload
            memcpy(Command.pPayload, &pIn[1], ByteCount);
            Command.ResultLength = ByteCount;
            return 0;
        }

        case MODBUS_READ_HOLDING_REGISTERS:
        case MODBUS_READ_INPUT_REGISTERS:
        {
            if(Length < 1)
            {
                return -1;
            }

            uint8_t ByteCount = pIn[0];

            if(Length < (size_t)(1 + ByteCount))
            {
                return -1;
            }

            // Interpret pData as uint16_t*
            uint16_t* pRegs = (uint16_t*)Command.pPayload;

            for(size_t i = 0; i < (ByteCount / 2); i++)
            {
                uint16_t Value =
                    ((uint16_t)pIn[1 + (i * 2)] << 8) |
                     (uint16_t)pIn[2 + (i * 2)];

                pRegs[i] = Value;
            }

            Command.ResultLength = ByteCount / 2;   // number of registers
            return 0;
        }

        case MODBUS_WRITE_SINGLE_COIL:
        case MODBUS_WRITE_SINGLE_REGISTER:
        {
            if(Length < 4)
            {
                return -1;
            }

            // Echo only — nothing to extract
            Command.ResultLength = 0;
            return 0;
        }

        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            if(Length < 4)
            {
                return -1;
            }

            // Echo address + quantity — nothing else to extract
            Command.ResultLength = 0;
            return 0;
        }
    }

    return -1;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HandleRequest
//
//  Parameters:     pRX         - Pointer to the received Modbus RTU request frame
//                  RX_Length   - Number of bytes in the received request
//                  pTX         - Output buffer where the slave response frame will be written
//                  TX_Max      - Maximum number of bytes available in pTX
//                  pTX_Length  - Output: total number of bytes written to pTX
//
//  Returns:        > 0         - Response successfully built and stored in pTX
//                  0           - No response generated (invalid request or internal error)
//
//  Description:    Processes a complete MODBUS request on the SLAVE side. The function first
//                  parses the incoming frame using ParseRequest(). If the request is invalid,
//                  a MODBUS exception frame is generated. Otherwise, the appropriate application
//                  handler is located via FindSlaveHandler(), and the handler is invoked to fill
//                  the MODBUS_Response_t structure. Finally, BuildFrameSlave() serializes the
//                  response (normal or exception) into the output buffer.
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::HandleRequest(const uint8_t* pRX, size_t RX_Length, uint8_t* pTX, size_t TX_Max, size_t* pTX_Length)
{
    *pTX_Length = 0;

    MODBUS_Command_t Command;
    int Status = ParseRequest(pRX, RX_Length, Command);

    if(Status < 0)
    {
        return BuildException(pRX[0], pRX[1], MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, pTX, TX_Max, pTX_Length);
    }

    MODBUS_SlaveCommandEntry_t* pHandler = FindSlaveHandler(Command.DeviceAddress, Command.Function);

	if(pHandler == nullptr)
    {
        return BuildException(Command.DeviceAddress, Command.Function, MODBUS_EXCEPTION_ILLEGAL_FUNCTION, pTX, TX_Max, pTX_Length);
    }

    // Validate quantity limit for this handler (if configured)
    if(pHandler->MaxQuantity > 0 && Command.Quantity > pHandler->MaxQuantity)
    {
        return BuildException(Command.DeviceAddress, Command.Function, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, pTX, TX_Max, pTX_Length);
    }

    // Payload = pTX + 2
    MODBUS_SlaveResponse_t Response;
    Response.Function = Command.Function;
    Response.pPayload = &pTX[2];
    Response.MaxSize  = TX_Max - 2;
    Response.PayloadLength   = 0;
    Response.IsException = false;

    // Application fillup Response
    pHandler->pCallback(Command, Response);

    // Build complete frame (SLAVE)
    *pTX_Length = BuildFrameSlave(Command, Response, pTX, TX_Max);

    return (*pTX_Length > 0);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseRequest
//
//  Parameters:     pRX         - Pointer to the received Modbus RTU request frame
//                  RX_Length   - Total number of bytes in the received frame
//                  Command     - Output structure populated with parsed request fields
//
//  Returns:        0           - Request successfully parsed
//                  < 0         - Invalid frame (null pointer, insufficient length, CRC failure)
//
//  Description:    Parses a Modbus RTU request on the SLAVE side. The function validates minimal
//                  frame length, checks CRC integrity, and extracts the DeviceAddress and Function
//                  code. The remaining bytes (excluding the CRC) are exposed as the raw PDU payload
//                  through Command.pPayload and Command.PayloadLength. No semantic validation of
//                  the payload is performed here; higher‑level handlers are responsible for
//                  interpreting the function‑specific fields.
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::ParseRequest(const uint8_t* pRX, size_t RX_Length, MODBUS_Command_t& Command)
{
    if(pRX == nullptr)
    {
        return -1;
    }

    if(RX_Length < 4)   // Addr + Func + CRC(2)
    {
        return -2;
    }

    // 1) Vérifier CRC
    if(ValidateCRC(pRX, RX_Length) == false)
    {
        return -3;
    }

    // 2) Extract address + function
    Command.DeviceAddress = pRX[0];
    Command.Function      = MODBUS_Function_e(pRX[1]);

    // 3) Extract payload (without CRC)
    Command.pPayload      = (uint8_t*)&pRX[2];
    Command.PayloadLength = RX_Length - 4;

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildException
//
//  Parameters:     Address         - Device address to include in the exception frame
//                  Function        - Original Modbus function code (before exception bit is added)
//                  ExceptionCode   - Modbus exception code to return (0x01–0x0B)
//                  pTX             - Output buffer where the exception frame will be written
//                  TX_MaxLength    - Maximum number of bytes available in pTX
//                  pTX_Length      - Output: number of bytes written to pTX
//
//  Returns:        0               - Exception frame successfully built
//                  < 0             - Insufficient buffer space or internal error
//
//  Description:    Builds a complete Modbus RTU exception response frame. The function writes the
//                  device address, the function code with the exception bit set (function | 0x80),
//                  and the Modbus exception code. A CRC16 is then computed over the first three
//                  bytes and appended in little‑endian order. The resulting frame always has a
//                  fixed length of 5 bytes: [Addr][Func|0x80][Exception][CRC_L][CRC_H].
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::BuildException(uint8_t Address, uint8_t Function, uint8_t ExceptionCode, uint8_t* pTX, size_t TX_MaxLength, size_t* pTX_Length)
{
    if(TX_MaxLength < 5)
    {
        return -1;
    }

    pTX[0] = Address;
    pTX[1] = Function | 0x80;
    pTX[2] = ExceptionCode;

    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = (uint16_t)ModbusCRC.CalculateBuffer(pTX, 3);
    pTX[3] = uint8_t(ComputedCRC);
    pTX[4] = uint8_t(ComputedCRC >> 8);

    *pTX_Length = 5;

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindSlaveHandler
//
//  Parameters:     DeviceAddress   - Modbus slave address extracted from the request
//                  Function        - Modbus function code extracted from the request
//
//  Returns:        Pointer to MODBUS_SlaveCommandEntry_t if a matching handler exists
//                  nullptr if no handler is registered or if no application is attached
//
//  Description:    Retrieves the application‑level handler associated with the specified
//                  DeviceAddress and Function. This function delegates the lookup to the
//                  MODBUS_Application instance, which maintains the registered slave command
//                  table. If no application is attached or no matching entry is found, the
//                  function returns nullptr, allowing the caller to generate an appropriate
//                  Modbus exception response.
//
//-------------------------------------------------------------------------------------------------
MODBUS_SlaveCommandEntry_t* MODBUS_Manager::FindSlaveHandler(uint8_t DeviceAddress, uint8_t Function)
{
    if(m_pApplication == nullptr)
    {
        return nullptr;
    }

    return m_pApplication->FindSlaveHandler(DeviceAddress, Function);
}

//-------------------------------------------------------------------------------------------------

#if (MODBUS_USE_ROUTER_PASSTHRU == DEF_ENABLED)
MODBUS_PassThruRule_t 		MODBUS_Router::m_PassThruRules [MODBUS_MAX_PASSTHRU_RULES];
size_t            			MODBUS_Router::m_PassThruCount = 0;
#endif // (MODBUS_USE_ROUTER_PASSTHRU == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClassTaskMODBUS_Wrapper
//
//  Parameters:     pvParameters   - Pointer to the MODBUS_Router instance passed by the RTOS
//
//  Returns:        void
//
//  Description:    Entry point for the Modbus task. This wrapper converts the generic RTOS
//                  task parameter into a MODBUS_Router instance and invokes its Run() method.
//                  The function must remain C‑linkage compatible so it can be used directly
//                  as an RTOS task entry function.
//
//-------------------------------------------------------------------------------------------------
extern "C" void ClassTaskMODBUS_Wrapper(void* pvParameters)
{
    (static_cast<MODBUS_Router*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void
//  Return:         nOS_Error
//
//  Description:    Initializes the router by clearing the backend table. All backend slots are
//                  set to nullptr, indicating that no MODBUS backends are registered yet. Backends
//                  will be attached later through explicit registration calls.
//
//-------------------------------------------------------------------------------------------------
nOS_Error MODBUS_Router::Initialize(void)
{
    nOS_Error Error = NOS_OK;

    for(size_t BackEnd = 0; BackEnd < MODBUS_MAX_BACKENDS; BackEnd++)
    {
        m_BackEnds[BackEnd] = nullptr;
    }

  #if (MODBUS_USE_ROUTER_PASSTHRU == DEF_ENABLED)
	MODBUS_ROUTER_PASSTHRU_TABLE(MODBUS_INIT_PASSTHRU_ENTRY)
  #endif

	m_Manager.SetApplication(&myMODBUS_Application);
    myMODBUS_Application.SetManager(&m_Manager);

    Error = nOS_ThreadCreate(&m_Handle,
                             ClassTaskMODBUS_Wrapper,
                             this,
                             &m_Stack[0],
                             TASK_MODBUS_STACK_SIZE,
                             TASK_MODBUS_PRIO,
                             "Task MODBUS");

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Run
//
//  Parameters:     None
//
//  Returns:        Nothing
//
//  Description:    Iterates through all registered MODBUS backends and invokes their Process()
//                  method. Each backend is responsible for handling its own state machine,
//                  timeouts, and queued Modbus operations. Backends that are not registered
//                  (nullptr entries) are skipped.
//
//-------------------------------------------------------------------------------------------------
void MODBUS_Router::Run(void)
{
    while(1)
    {
        for(size_t BackEnd = 0; BackEnd < MODBUS_MAX_BACKENDS; BackEnd++)
        {
            MODBUS_InterfaceBackEnd* pBackEnd = m_BackEnds[BackEnd];

            if(pBackEnd != nullptr)
            {
                // 1. Faire tourner le backend (MASTER + SLAVE transport)
                pBackEnd->Process();

                // 2. Vérifier si une requête SLAVE est prête
                if(pBackEnd->HasRequest() == true)
                {
                    const uint8_t* pRX;
                    size_t         RX_Length;

                    if(pBackEnd->GetRequest(&pRX, &RX_Length))
                    {
                        size_t   TX_Max    = pBackEnd->GetTXBufferSize();
                        uint8_t* pTX       = (uint8_t*)pMemoryPool->Alloc(TX_Max, MEM_DBG_MB_TX_SER);
                        size_t   TX_Length = 0;

                        // Call the manager to process the request
                        m_Manager.HandleRequest(pRX, RX_Length, pTX, TX_Max, &TX_Length);

                        // Send the response
                        if(TX_Length > 0)
                        {
                            pBackEnd->Send(pTX, TX_Length);
                        }
                    }
                }
            }
        }

        nOS_Sleep(1);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterEndpoint
//
//  Parameters:     pBackEnd    - Pointer to a Modbus backend implementing IModbusBackend
//
//  Returns:        true        - Backend successfully registered in the first available slot
//                  false       - No free slot available; backend table is full
//
//  Description:    Registers a Modbus backend with the router. The backend is stored in the
//                  first empty slot of the internal backend table. Once registered, the backend
//                  will be included in the router's periodic Process() calls.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Router::RegisterEndpoint(MODBUS_InterfaceBackEnd* pBackEnd)
{
    for(size_t BackEnd = 0; BackEnd < MODBUS_MAX_BACKENDS; BackEnd++)
    {
        if(m_BackEnds[BackEnd] == nullptr)
        {
            m_BackEnds[BackEnd] = pBackEnd;
			pBackEnd->SetManager(&m_Manager);
            return true;
        }
    }

    return false; // No more space
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterPassThru
//
//  Parameters:     PassThruRule - Passthrough routing rule to register
//
//  Returns:        true         - Rule successfully added
//                  false        - Table is full; rule not added
//
//  Description:    Adds a new passthrough routing rule to the dynamic passthrough table. Static
//                  rules (if any) are loaded during initialization, and this function appends
//                  additional rules at runtime until the table reaches its maximum capacity.
//
//-------------------------------------------------------------------------------------------------
#if (MODBUS_USE_ROUTER_PASSTHRU == DEF_DISABLED)
bool MODBUS_Router::RegisterPassThru(const MODBUS_PassThruRule_t& PassThruRule)
{
    if(m_PassThruCount >= MODBUS_MAX_PASSTHRU_RULES)
	{
        return false;
	}

    // Copy fields explicitly (safe)
    m_PassThruRules[m_PassThruCount].SrcDeviceAddress = PassThruRule.SrcDeviceAddress;
    m_PassThruRules[m_PassThruCount].DstDeviceAddress = PassThruRule.DstDeviceAddress;
    m_PassThruCount++;
    return true;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           Queue
//
//  Parameters:     Command     - Modbus command to route toward the appropriate backend
//
//  Returns:        true        - Command successfully queued to a backend
//                  false       - No backend available to handle the command or passthru target
//
//  Description:    Routes a Modbus command to the correct backend. The router first checks
//                  passthru rules: if the command's DeviceAddress matches a passthru source, the
//                  DeviceAddress is rewritten to the mapped destination and the command is forwarded
//                  to the backend capable of handling that destination address. If no passthru
//                  rule applies, the router searches for a backend that can handle the original
//                  DeviceAddress. If no backend matches, the command cannot be queued.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Router::Queue(MODBUS_Command_t& Command)
{
  #if (MODBUS_USE_ROUTER_PASSTHRU == DEF_DISABLED)
    // Check passthru rules if it exist
    for(size_t Rules = 0; Rules < MODBUS_MAX_PASSTHRU_RULES; Rules++)
    {
        if(Command.DeviceAddress == m_PassThruRules[Rules].SrcDeviceAddress)
        {
            MODBUS_Command_t NewCommand = Command;
            NewCommand.DeviceAddress = m_PassThruRules[Rules].DstDeviceAddress;

            // Find backend for new DeviceAddress
            for(size_t BackEnd = 0; BackEnd < MODBUS_MAX_BACKENDS; BackEnd++)
            {
                MODBUS_InterfaceBackEnd* pBackEnd = m_BackEnds[BackEnd];

                if((pBackEnd != 0) && (pBackEnd->CanHandle(NewCommand.DeviceAddress) == true))
                {
                    return pBackEnd->Queue(NewCommand);
                }
            }

            return false;
        }
    }
  #endif // (MODBUS_USE_ROUTER_PASSTHRU == DEF_DISABLED)

    for(size_t BackEnd = 0; BackEnd < MODBUS_MAX_BACKENDS; BackEnd++)
    {
        MODBUS_InterfaceBackEnd* pBackEnd = m_BackEnds[BackEnd];

        if((pBackEnd != nullptr) && (pBackEnd->CanHandle(Command.DeviceAddress) == true))
        {
            return pBackEnd->Queue(Command);
        }
    }

    return false; // No backend to handle this address
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsBusy
//
//  Parameters:     None
//
//  Returns:        true        - At least one registered backend reports it is busy
//                  false       - All registered backends are idle or no backends are registered
//
//  Description:    Scans all registered Modbus backends and queries their busy state. This allows
//                  higher-level logic to determine whether any backend is currently processing a
//                  transaction, waiting on a timeout, or handling queued operations. Backends that
//                  are not registered (nullptr entries) are ignored.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Router::IsBusy(void)
{
    for(size_t BackEnd = 0; BackEnd < MODBUS_MAX_BACKENDS; BackEnd++)
    {
        if((m_BackEnds[BackEnd] != nullptr) && (m_BackEnds[BackEnd]->IsBusy() == true))
        {
            return true;
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
