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
// Variable(s)
//-------------------------------------------------------------------------------------------------

MODBUS_MasterRuntime_t MODBUS_Manager::m_ModbusMasterRuntimeTable	[MODBUS_MAX_MASTER_REQUEST_ENTRY];

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildFrameSlave
//
//  Parameters:     Command     - Parsed MODBUS request received from the master
//                  Response    - Application-generated response (normal or exception)
//                  pOut        - Output buffer where the complete MODBUS frame is written
//                  MaxLength   - Maximum number of bytes available in pOut
//
//  Returns:        > 0         - Total frame length written to pOut
//                  0           - Frame could not be built (invalid payload or insufficient space)
//
//  Description:    Builds a complete MODBUS slave response frame. The function writes the
//                  DeviceAddress, the response Function code (normal or exception), the response
//                  payload, and finally appends the MODBUS CRC16. If the payload does not fit in
//                  the output buffer, the function returns 0 to indicate failure.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::BuildFrameSlave(const MODBUS_Command_t& Command, const MODBUS_SlaveResponse_t& Response, uint8_t* pOut, size_t* pLength)
{
    size_t Index = 0;

    // Address + Function
    pOut[Index++] = Command.DeviceAddress;
    pOut[Index++] = Response.IsException ? (Command.Function | 0x80) : Command.Function;

    // Copy payload
    if(Index + Response.PayloadLength + 2 > *pLength)
	{
        return SYS_WRONG_SIZE;
	}

    memcpy(&pOut[Index], Response.pPayload, Response.PayloadLength);
    Index += Response.PayloadLength;

    // CRC
    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = ModbusCRC.CalculateBuffer(pOut, Index);

    pOut[Index++] = (uint8_t)(ComputedCRC & 0xFF);
    pOut[Index++] = (uint8_t)(ComputedCRC >> 8);

    *pLength = Index;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

SystemState_e MODBUS_Manager::MasterRequest(uint32_t RequestID, uint16_t Quantity)
{
    if(RequestID >=  m_pApplication->GetMasterCount())
        return SYS_NULLPTR;

    MODBUS_MasterEntry_t* pEntry = m_pApplication->FindMasterRequest(RequestID);
    if(pEntry == nullptr)
        return SYS_NULLPTR;

    MODBUS_MasterRuntime_t& RunTime = m_ModbusMasterRuntimeTable[RequestID];

    if(RunTime.IsPending)
        return SYS_BUSY;

    if(Quantity > pEntry->MaxRequestQuantity)
        return SYS_INVALID_PARAMETER;

    // Build command (runtime)
    RunTime.Command.DeviceAddress = pEntry->DeviceAddress;
    RunTime.Command.Function      = pEntry->Function;
    RunTime.Command.Address       = 0;
    RunTime.Command.Quantity      = Quantity;
    RunTime.Command.Value         = 0;
    RunTime.Command.pPayload      = nullptr;
    RunTime.Command.PayloadLength = 0;
    RunTime.Command.TimeoutMsec   = pEntry->TimeoutMsec;

    // Send command through router
    if(m_pRouter->Queue(RunTime.Command) == false)
        return SYS_FAIL;

    RunTime.IsPending      = true;
    RunTime.TimestampStart = GetTick();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildFrameMaster
//
//  Parameters:     Command     - MODBUS request to serialize (read/write operation)
//                  pOut        - Output buffer where the complete MODBUS frame is written
//                  MaxLength   - Maximum number of bytes available in pOut
//
//  Returns:        > 0         - Total frame length written to pOut
//                  0           - Frame could not be built (invalid payload or insufficient space)
//
//  Description:    Builds a complete MODBUS master request frame. The function writes the
//                  DeviceAddress, Function code, and the request payload generated by
//                  BuildPayload().
//                  It then appends the MODBUS CRC16. If the payload cannot be generated or the
//                  output buffer is too small, the function returns 0 to indicate failure.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::BuildFrameMaster(MODBUS_Command_t& Command, uint8_t* pOut, size_t* pLength)
{
    size_t Index = 0;
    size_t MaxLength = *pLength;

    // Address + Function
    pOut[Index++] = Command.DeviceAddress;
    pOut[Index++] = Command.Function;

    // Request payload (address, quantity, value…)
    size_t Length = MaxLength - Index;
	SystemState_e State = BuildPayload(Command, &pOut[Index], pLength);

	if(State != SYS_READY)
    {
        return State;
    }

    Index += Length;

    // CRC
    if((Index + 2) > MaxLength)
    {
        return SYS_WRONG_SIZE;
    }

    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = ModbusCRC.CalculateBuffer(pOut, Index);

    pOut[Index++] = uint8_t(ComputedCRC);           // CRC Low
    pOut[Index++] = uint8_t(ComputedCRC >> 8);      // CRC High

    *pLength = Index;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HandleResponse
//
//  Parameters:     pRX         - Pointer to the received MODBUS frame
//                  RX_Length   - Total number of bytes in the received frame
//
//  Returns:        true        - The response frame was valid, successfully parsed, and the
//                                corresponding master request callback was invoked.
//                  false       - Invalid frame (CRC failure, malformed payload, insufficient
//                                length), unknown RequestID, or no matching pending request.
//
//  Description:    Processes a complete MODBUS response received from the Router/backend.
//                  The function delegates frame decoding to ParseResponse(), which validates the
//                  CRC, extracts the function code, identifies MODBUS exceptions, retrieves the
//                  RequestID, and copies the payload into a MODBUS_Response_t structure.
//
//                  Once parsed, the RequestID is used to locate the corresponding master request
//                  entry via MODBUS_Application::FindMasterRequest(). If a matching entry is
//                  found, the request is marked as completed (IsPending cleared), and the
//                  application-provided callback is invoked with the decoded response.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::HandleResponse(const uint8_t* pRX, size_t RX_Length)
{
    MODBUS_MasterResponse_t Response;

    // 1) Parse frame
    SystemState_e State = ParseResponse(pRX, &RX_Length, Response);

    if(State != SYS_READY)
    {
        return State;
    }

    uint32_t RequestID = Response.RequestID;

    // 2) Validate ID
    MODBUS_MasterEntry_t* pCfg = m_pApplication->FindMasterRequest(RequestID);

    if(pCfg == nullptr)
    {
        return SYS_NULLPTR;
    }

    // Clear pending state
    m_ModbusMasterRuntimeTable[RequestID].IsPending = false;

    // Invoke callback
    if(pCfg->pCallback != nullptr)
    {
        pCfg->pCallback(RequestID, Response);
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseResponse
//
//  Parameters:     pIn       - Pointer to the received MODBUS frame
//                  Length    - Total number of bytes in the received frame
//                  Response  - Structure filled with decoded response information
//
//  Returns:        >= 0      - Payload length (excluding RequestID and CRC) successfully parsed
//                  < 0       - MODBUS exception code (returned as a negative value)
//                  -1        - Invalid frame (CRC failure, malformed payload, insufficient length)
//
//  Description:    Validates and interprets a received MODBUS response for master operation.
//                  The function checks minimal frame length, validates the CRC, and decodes the
//                  function code. If the function code indicates a MODBUS exception
//                  (function | 0x80), the exception code is extracted and returned as a negative
//                  value, and the Response structure is marked as an exception. For normal
//                  responses, the first two bytes of the payload are interpreted as the RequestID,
//                  and the remaining bytes are copied into Response.pPayload. The decoded fields
//                  (DeviceAddress, Function, RequestID, exception status, and payload) are stored
//                  in the MODBUS_Response_t structure.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::ParseResponse(const uint8_t* pIn, size_t* pLength, MODBUS_MasterResponse_t& Response)
{
    if(*pLength < 4)
    {
        return SYS_WRONG_SIZE;
    }

    size_t Index = 0;

    Response.DeviceAddress      = pIn[Index++];
    MODBUS_Function_e Function  = MODBUS_Function_e(pIn[Index++]);

    if(ValidateCRC(pIn, *pLength) == false)
    {
        return SYS_CRC_FAIL;
    }

    // Exception response ?
    if(Function & MODBUS_EXCEPTION_RESPONSE)
    {
        Response.Function      = MODBUS_Function_e(Function & ~MODBUS_EXCEPTION_RESPONSE);
        Response.IsException   = true;

        if(*pLength < 5) // Addr + Func + ExCode + CRC(2)
        {
            return SYS_WRONG_SIZE;
        }

        Response.ExceptionCode = pIn[Index];
        Response.PayloadLength = 0;

        return SYS_FAIL; // -(int)Response.ExceptionCode;  // todo create error
    }

    Response.Function    = Function;
    Response.IsException = false;

    // Payload length excluding CRC
    size_t PayloadLength = *pLength - Index - 2;

    if(PayloadLength < 2) // Need at least RequestID (2 bytes)
    {
        return SYS_WRONG_SIZE;
    }

    // First 2 bytes of payload = RequestID
    Response.RequestID = (uint16_t)((pIn[Index] << 8) | pIn[Index + 1]);
    Index         += 2;
    PayloadLength -= 2;

    if(PayloadLength > Response.MaxPayloadLength)
    {
        return SYS_WRONG_SIZE;
    }

    if(PayloadLength > 0 && Response.pPayload != nullptr)
    {
        memcpy(Response.pPayload, &pIn[Index], PayloadLength);
    }

    Response.PayloadLength = (uint16_t)PayloadLength;
    *pLength = PayloadLength;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildPayload
//
//  Parameters:     Command     - MODBUS master request describing the operation to encode
//                  pOut        - Output buffer where the PDU payload will be written
//                  Length      - Maximum number of bytes available in pOut and also the return value of the length
//
//  Returns:
//
//  Description:    Encodes the MODBUS PDU payload for master requests. This function serializes
//                  the address, quantity, value, and optional write‑multiple data depending on the
//                  function code. Only the PDU payload is generated here; the caller is responsible
//                  for writing the device address, function code, and CRC. For unsupported functions
//                  or when the output buffer is too small, a negative value is returned.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::BuildPayload(MODBUS_Command_t& Command, uint8_t* pOut, size_t* pLength)
{
    size_t Index = 0;

    switch(Command.Function)
    {
        // 0x03 - Read Holding Registers
        case MODBUS_READ_HOLDING_REGISTERS:
        {
            if(*pLength < 4)
            {
                return SYS_WRONG_SIZE;
            }

            pOut[Index++] = uint8_t(Command.Address >> 8);
            pOut[Index++] = uint8_t(Command.Address);

            pOut[Index++] = uint8_t(Command.Quantity >> 8);
            pOut[Index++] = uint8_t(Command.Quantity);

            *pLength = Index;
            return SYS_READY;
        }

        // 0x06 - Write Single Register
        case MODBUS_WRITE_SINGLE_REGISTER:
        {
            if(*pLength < 4)
            {
                return SYS_WRONG_SIZE;
            }

            pOut[Index++] = uint8_t(Command.Address >> 8);
            pOut[Index++] = uint8_t(Command.Address);

            pOut[Index++] = uint8_t(Command.Value >> 8);
            pOut[Index++] = uint8_t(Command.Value);

            *pLength = Index;
            return SYS_READY;
        }

        // 0x10 - Write Multiple Registers
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            size_t ByteCount = Command.Quantity * 2;

            if(*pLength < (5 + ByteCount))
            {
                return SYS_WRONG_SIZE;
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

            *pLength = Index;
            return SYS_READY;
        }

        // Unsupported function
        default:
        {
            return SYS_UNSUPPORTED_FEATURE;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ValidateCRC
//
//  Parameters:     pData       - Pointer to the received MODBUS frame
//                  Length      - Total number of bytes in the frame, including CRC
//
//  Returns:        true        - CRC is valid and matches the computed MODBUS CRC16
//                  false       - Frame too short or CRC mismatch
//
//  Description:    Extracts the CRC16 value appended to the end of a MODBUS frame and
//                  recomputes the CRC over all preceding bytes. The function compares the
//                  received CRC with the computed one and returns true only if they match.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::ValidateCRC(const uint8_t* pData, size_t Length)
{
    if(Length < 3)
    {
        return SYS_WRONG_SIZE;
    }

    uint16_t ReceivedCRC = (uint16_t)pData[Length - 2] | ((uint16_t)pData[Length - 1] << 8);
    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = (uint16_t)ModbusCRC.CalculateBuffer(pData, Length - 2);
    return (ReceivedCRC == ComputedCRC) ? SYS_READY : SYS_CRC_FAIL;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParsePayload
//
//  Parameters:     Command     - Original MODBUS command containing buffers for parsed results
//                  Function    - Function code extracted from the response frame
//                  pIn         - Pointer to the payload of the response (after DeviceAddress/Function)
//                  Length      - Number of bytes available in the payload buffer
//
//  Returns:        0           - Payload successfully parsed
//                  -1          - Invalid or incomplete payload format
//                  < 0         - Reserved for exception codes (handled earlier in ParseResponse)
//
//  Description:    Interprets the function-specific payload of a MODBUS response. The function
//                  validates the payload length according to the expected format of the specified
//                  function code and extracts coil states or register values into the buffers
//                  provided in the Command structure. For write-type functions, the response is an
//                  echo of the request and requires no data extraction.
//
//-------------------------------------------------------------------------------------------------
/*
SystemState_e MODBUS_Manager::ParsePayload(MODBUS_Command_t& Command, uint8_t Function, const uint8_t* pIn, size_t Length)
{
    switch(Function)
    {
        case MODBUS_READ_COILS:
        case MODBUS_READ_DISCRETE_INPUTS:
        {
            if(Length < 1)
            {
                return SYS_WRONG_SIZE;
            }

            uint8_t ByteCount = pIn[0];

            if(Length < (size_t)(1 + ByteCount))
            {
                return SYS_WRONG_SIZE;
            }

            // Copy raw bytes into pPayload
            memcpy(Command.pPayload, &pIn[1], ByteCount);
            Command.ResultLength = ByteCount;
            return SYS_READY;
        }

        case MODBUS_READ_HOLDING_REGISTERS:
        case MODBUS_READ_INPUT_REGISTERS:
        {
            if(Length < 1)
            {
                return SYS_WRONG_SIZE;
            }

            uint8_t ByteCount = pIn[0];

            if(Length < (size_t)(1 + ByteCount))
            {
                return SYS_WRONG_SIZE;
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
            return SYS_READY;
        }

        case MODBUS_WRITE_SINGLE_COIL:
        case MODBUS_WRITE_SINGLE_REGISTER:
        {
            if(Length < 4)
            {
                return SYS_WRONG_SIZE;
            }

            // Echo only — nothing to extract
            Command.ResultLength = 0;
            return SYS_READY;
        }

        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            if(Length < 4)
            {
                return SYS_WRONG_SIZE;
            }

            // Echo address + quantity — nothing else to extract
            Command.ResultLength = 0;
            return SYS_READY;
        }
    }

    return SYS_WRONG_SIZE;
}
*/
//-------------------------------------------------------------------------------------------------
//
//  Name:           HandleRequest
//
//  Parameters:     pRX         - Pointer to the received MODBUS request frame
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
SystemState_e MODBUS_Manager::HandleRequest(const uint8_t* pRX, size_t RX_Length, uint8_t* pTX, size_t TX_Max, size_t* pTX_Length)
{
    *pTX_Length = 0;

    MODBUS_Command_t Command;
    SystemState_e State = ParseRequest(pRX, RX_Length, Command);

    if(State != SYS_READY)
    {
        return BuildException(pRX[0], pRX[1], MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, pTX, TX_Max, pTX_Length);
    }

    MODBUS_SlaveCommandEntry_t* pHandler = FindSlaveHandler(Command.DeviceAddress, Command.Function);

	if(pHandler == nullptr)
    {
        return BuildException(Command.DeviceAddress, Command.Function, MODBUS_EXCEPTION_ILLEGAL_FUNCTION, pTX, TX_Max, pTX_Length);
    }

    // Validate quantity limit for this handler (if configured)
    if(Command.Quantity > pHandler->MaxQuantity)
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
	*pTX_Length = TX_Max;
     return BuildFrameSlave(Command, Response, pTX, pTX_Length);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseRequest
//
//  Parameters:     pRX         - Pointer to the received MODBUS request frame
//                  RX_Length   - Total number of bytes in the received frame
//                  Command     - Output structure populated with parsed request fields
//
//  Returns:        0           - Request successfully parsed
//                  < 0         - Invalid frame (null pointer, insufficient length, CRC failure)
//
//  Description:    Parses a MODBUS request on the SLAVE side. The function validates minimal
//                  frame length, checks CRC integrity, and extracts the DeviceAddress and Function
//                  code. The remaining bytes (excluding the CRC) are exposed as the raw PDU payload
//                  through Command.pPayload and Command.PayloadLength. No semantic validation of
//                  the payload is performed here; higher‑level handlers are responsible for
//                  interpreting the function‑specific fields.
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::ParseRequest(const uint8_t* pRX, size_t RX_Length, MODBUS_Command_t& Command)
{
    if(pRX == nullptr)
        return SYS_NULLPTR;

    // Minimum RTU frame: Addr + Func + CRC(2)
    if(RX_Length < 4)
        return SYS_WRONG_SIZE;

    // CRC validation
    if(ValidateCRC(pRX, RX_Length) != SYS_READY)
        return SYS_CRC_FAIL;

    // Extract address and function
    Command.DeviceAddress = pRX[0];
    Command.Function      = MODBUS_Function_e(pRX[1]);

    // Extract payload (without CRC)
    Command.pPayload      = (uint8_t*)&pRX[2];
    Command.PayloadLength = RX_Length - 4;

    // Default values
    Command.Address       = 0;
    Command.Quantity      = 0;
    Command.Value         = 0;

    switch(Command.Function)
    {
        case MODBUS_READ_COILS:                 // 0x01
        case MODBUS_READ_DISCRETE_INPUTS:       // 0x02
        case MODBUS_READ_HOLDING_REGISTERS:     // 0x03
        case MODBUS_READ_INPUT_REGISTERS:       // 0x04
        {
            // PDU: [HiAddr][LoAddr][HiQty][LoQty]
            if(Command.PayloadLength < 4)
                return SYS_WRONG_SIZE;

            Command.Address  = (Command.pPayload[0] << 8) | Command.pPayload[1];
            Command.Quantity = (Command.pPayload[2] << 8) | Command.pPayload[3];
            break;
        }

        case MODBUS_WRITE_SINGLE_COIL:          // 0x05
        case MODBUS_WRITE_SINGLE_REGISTER:      // 0x06
        {
            // PDU: [HiAddr][LoAddr][HiValue][LoValue]
            if(Command.PayloadLength < 4)
                return SYS_WRONG_SIZE;

            Command.Address = (Command.pPayload[0] << 8) | Command.pPayload[1];
            Command.Value   = (Command.pPayload[2] << 8) | Command.pPayload[3];
            Command.Quantity = 1;
            break;
        }

        case MODBUS_WRITE_MULTIPLE_COILS:       // 0x0F
        case MODBUS_WRITE_MULTIPLE_REGISTERS:   // 0x10
        {
            // PDU: [HiAddr][LoAddr][HiQty][LoQty][ByteCount]...
            if(Command.PayloadLength < 5)
                return SYS_WRONG_SIZE;

            Command.Address  = (Command.pPayload[0] << 8) | Command.pPayload[1];
            Command.Quantity = (Command.pPayload[2] << 8) | Command.pPayload[3];
            break;
        }

        default:
            // Unknown function → leave fields at default
            break;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildException
//
//  Parameters:     Address         - Device address to include in the exception frame
//                  Function        - Original MODBUS function code (before exception bit is added)
//                  ExceptionCode   - MODBUS exception code to return (0x01–0x0B)
//                  pTX             - Output buffer where the exception frame will be written
//                  TX_MaxLength    - Maximum number of bytes available in pTX
//                  pTX_Length      - Output: number of bytes written to pTX
//
//  Returns:        0               - Exception frame successfully built
//                  < 0             - Insufficient buffer space or internal error
//
//  Description:    Builds a complete MODBUS exception response frame. The function writes the
//                  device address, the function code with the exception bit set (function | 0x80),
//                  and the MODBUS exception code. A CRC16 is then computed over the first three
//                  bytes and appended in little‑endian order. The resulting frame always has a
//                  fixed length of 5 bytes: [Addr][Func|0x80][Exception][CRC_L][CRC_H].
//
//-------------------------------------------------------------------------------------------------
SystemState_e MODBUS_Manager::BuildException(uint8_t Address, uint8_t Function, uint8_t ExceptionCode, uint8_t* pTX, size_t TX_MaxLength, size_t* pTX_Length)
{
    if(TX_MaxLength < 5)
    {
        return SYS_WRONG_SIZE;
    }

    pTX[0] = Address;
    pTX[1] = Function | 0x80;
    pTX[2] = ExceptionCode;

    CRC_Calc ModbusCRC(CRC_16_MODBUS);
    uint16_t ComputedCRC = (uint16_t)ModbusCRC.CalculateBuffer(pTX, 3);
    pTX[3] = uint8_t(ComputedCRC);
    pTX[4] = uint8_t(ComputedCRC >> 8);

    *pTX_Length = 5;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FindSlaveHandler
//
//  Parameters:     DeviceAddress   - MODBUS slave address extracted from the request
//                  Function        - MODBUS function code extracted from the request
//
//  Returns:        Pointer to MODBUS_SlaveCommandEntry_t if a matching handler exists
//                  nullptr if no handler is registered or if no application is attached
//
//  Description:    Retrieves the application‑level handler associated with the specified
//                  DeviceAddress and Function. This function delegates the lookup to the
//                  MODBUS_Application instance, which maintains the registered slave command
//                  table. If no application is attached or no matching entry is found, the
//                  function returns nullptr, allowing the caller to generate an appropriate
//                  MODBUS exception response.
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
//  Description:    Entry point for the MODBUS task. This wrapper converts the generic RTOS
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
	m_Manager.SetRouter(this);
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
//                  timeouts, and queued MODBUS operations. Backends that are not registered
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
                        size_t   TX_Max    = pBackEnd->GetTX_BufferSize();
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
//  Parameters:     pBackEnd    - Pointer to a MODBUS backend implementing MODBUS_InterfaceBackEnd
//
//  Returns:        true        - Backend successfully registered in the first available slot
//                  false       - No free slot available; backend table is full
//
//  Description:    Registers a MODBUS backend with the router. The backend is stored in the
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
//  Parameters:     Command     - MODBUS command to route toward the appropriate backend
//
//  Returns:        true        - Command successfully queued to a backend
//                  false       - No backend available to handle the command or passthru target
//
//  Description:    Routes a MODBUS command to the correct backend. The router first checks
//                  passthru rules: if the command's DeviceAddress matches a passthru source, the
//                  DeviceAddress is rewritten to the mapped destination and the command is forwarded
//                  to the backend capable of handling that destination address. If no passthru
//                  rule applies, the router searches for a backend that can handle the original
//                  DeviceAddress. If no backend matches, the command cannot be queued.
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Router::Queue(MODBUS_Command_t& Command)
{
  #if (MODBUS_USE_ROUTER_PASSTHRU == DEF_ENABLED)
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
//  Description:    Scans all registered MODBUS backends and queries their busy state. This allows
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
