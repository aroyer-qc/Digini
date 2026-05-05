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

//------ Note(s) ----------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           BuildFrame
//
//  Parameter(s):   
//                  
//
//  Return:         
//                  
//                  
//  Description:
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::BuildFrame(const ModbusCommand& Command, uint8_t* pOut, size_t MaxLength)
{
    size_t Index = 0;
    
    pOut[Index++] = Command.DeviceAddress;
    pOut[Index++] = Command.Function;
    int PayloadLength = BuildPayload(Command, &pOut[Index], MaxLength - Index); // Payload according to function

    if(PayloadLength < 0)
    {
        return 0;
    }

    Index += PayloadLength;

    if((Index + 2) > MaxLength)                                                 // CRC
    {
        return 0;
    }
    
    CRC_Driver ModbusCRC;
    ModbusCRC.Initialize(CRC_16_MODBUS);
    uint16_t CRC_Result = uint16_t(ModbusCRC.CalculateBuffer(pOut, Index));
    pOut[Index++] = (uint8_t)(CRC_Result & 0xFF);                               // CRC Low
    pOut[Index++] = (uint8_t)((CRC_Result >> 8) & 0xFF);                        // CRC High

    return (int)Index;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           MasterParseResponse
//
//  Parameter(s):   
//                  
//
//  Return:         
//                  
//                  
//  Description:
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::MasterParseResponse(const ModbusCommand& Command, const uint8_t* pIn, size_t Length)
{
    if(Length < 4)                                              // Minimal length
    {
        return -1;
    }

    size_t  Index     = 0;
    uint8_t DeviceAddress = pIn[Index++];
    uint8_t Function  = pIn[Index++];

    if(DeviceAddress != Command.DeviceAddress)                   // Verify DeviceAddress
    {
        return -1;
    }

    if(ValidateCRC(pIn, Length) != SYS_READY)                   // Verify CRC
    {
        return -1;
    }

    if(Function & MODBUS_EXCEPTION_RESPONSE)                    // MODBUS Exception ?
    {
        uint8_t ExceptionCode = pIn[Index];
        return -ExceptionCode;                                  // Standard : negative error
    }

    if(Function != Command.Function)                            // Verify function
    {
        return -1;
    }

    int Status = ParsePayload(Command, Function, &pIn[Index], Length - Index);      // Extract payload according to function

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ValidateCRC
//
//  Parameter(s):   
//                  
//
//  Return:         
//                  
//                  
//  Description:
//
//-------------------------------------------------------------------------------------------------
bool MODBUS_Manager::ValidateCRC(const uint8_t* pData, size_t Length)
{
    if(Length < 3)
    {
        return false;
    }

    uint16_t ReceivedCRC = (uint16_t)pData[Length - 2] | ((uint16_t)pData[Length - 1] << 8);
    CRC_Driver ModbusCRC;
    ModbusCRC.Initialize(CRC_16_MODBUS);
    uint16_t ComputedCRC = (uint16_t)ModbusCRC.CalculateBuffer(pData, Length - 2);

    return (ReceivedCRC == ComputedCRC);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParsePayload
//
//  Parameter(s):   
//                  
//
//  Return:         
//                  
//                  
//  Description:
//
//-------------------------------------------------------------------------------------------------
int MODBUS_Manager::ParsePayload(const ModbusCommand& Command, uint8_t Function, const uint8_t* pIn, size_t Length)
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

            // Copy data in Command.ResultBuffer
            memcpy(Command.ResultBuffer, &pIn[1], ByteCount);
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

            // Register per register extraction
            for(size_t i = 0; i < (ByteCount / 2); i++)
            {
                uint16_t Value = ((uint16_t)pIn[1 + (i * 2)] << 8) | (uint16_t)pIn[2 + (i * 2)];
                Command.Data[i] = Value;
            }

            Command.ResultLength = ByteCount / 2;
            return 0;
        }

        // Write Single Coil / Register
        case MODBUS_WRITE_SINGLE_COIL:
        case MODBUS_WRITE_SINGLE_REGISTER:
        {
            if(Length < 4)
            {
                return -1;
            }

            // Echo command -> nothing to extract
            return 0;
        }

        case MODBUS_WRITE_MULTIPLE_REGISTERS:
        {
            if(Length < 4)
            {
                return -1;
            }

            // Echo address + quantity -> OK
            return 0;
        }
    }

    return -1;
}

//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------


MODBUS_Router::MODBUS_Router()
{
    for(size_t i = 0; i < (size_t)MODBUS_Backend_e::COUNT; i++)
    {
        m_BackEnds[i] = nullptr;
    }
}

void MODBUS_Router::Process(void)
{
    for(size_t i = 0; i < MODBUS_BACKEND_COUNT; i++)
    {
        if(m_BackEnds[i] != nullptr)
        {
            m_BackEnds[i]->Process();
        }
    }
}

bool MODBUS_Router::RegisterEndpoint(IModbusBackend* pBackEnd)
{
    for(size_t i = 0; i < MODBUS_BACKEND_COUNT; i++)
    {
        if(m_BackEnds[i] == nullptr)
        {
            m_BackEnds[i] = pBackEnd;
            return true;
        }
    }
    
    return false; // No more space
}

bool MODBUS_Router::Queue(ModbusCommand& Command)
{
    for(size_t i = 0; i < MODBUS_BACKEND_COUNT; i++)
    {
        MODBUS_InterfaceBackEnd* pBackEnd = m_BackEnds[i];

        if((pBackEnd != nullptr) && (pBackEnd->CanHandle(Command.SlaveID) == true))
        {
            return pBackEnd->Queue(Command);
        }
    }

    return false; // aucun backend ne peut gérer cette adresse
}

bool MODBUS_Router::IsBusy(void)
{
    for(size_t i = 0; i < MODBUS_BACKEND_COUNT; i++)
    {
        if((m_BackEnds[i] != nullptr) && (m_BackEnds[i]->IsBusy() == true))
        {
            return true;
        }
    }
    
    return false;
}

bool MODBUS_Router::Queue(ModbusCommand& Command)
{
    for(size_t i = 0; i < MODBUS_BACKEND_COUNT; i++)
    {
        IModbusBackEnd* pBackEnd = m_BackEnds[i];

        if((pBackEnd != nullptr) && (pBackEnd->CanHandle(Command.SlaveID) == true))
        {
            return pBackEnd->Queue(Command);
        }
    }

    return false;   // No backend to handle this address
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_MODBUS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------




