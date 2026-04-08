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

#define MODBUS_RTU_SILENT_INTERVAL_MSEC   5   // ou selon ta vitesse


void ModbusRTU::IF_Process(void)
{
    switch(m_State)
    {
        case MODBUS_IDLE:
        {
            if(m_HasPending == false)
            {
                return;
            }

            m_State = MODBUS_BUILD_FRAME;
        }
        break;
        
        case MODBUS_BUILD_FRAME:
        {
            // we will use pMemoryPool
            // Demande au central de construire la trame RTU
            int FrameLen = m_pCentral->BuildFrame(m_Cmd, m_TxBuf, sizeof(m_TxBuf));

            if(FrameLen <= 0)
            {
                // Erreur de construction (commande invalide, buffer trop petit, etc.)
                m_State = MODBUS_State_e::MODBUS_ERROR;
                return;
            }

            m_TxLen = static_cast<size_t>(FrameLen);

            // Prochaine étape : envoyer la trame
            m_State = MODBUS_State_e::MODBUS_SEND_FRAME;
        }
        break;
        
        case MODBUS_SEND_FRAME:
        {
            int Sent = Send(m_TxBuf, m_TxLen);

            if(Sent < 0)
            {
                m_State = MODBUS_State_e::MODBUS_ERROR;
                return;
            }

            // Démarre le silent interval
            m_SilentTick = GetTick();

            m_State = MODBUS_State_e::MODBUS_WAIT_SILENT;
        }
        break;
        
        case MODBUS_WAIT_SILENT:
        {
             // Le silent interval est écoulé ?
            if(TickHasTimeOut(m_SilentTick, MODBUS_RTU_SILENT_INTERVAL_MSEC))
            {
                // Prépare la réception
                m_RxLen     = 0;
                m_StartTick = GetTick();

                m_State = MODBUS_State_e::MODBUS_WAIT_RESPONSE;
            }
        }
        break;
        
        case MODBUS_WAIT_RESPONSE:
        {
            uint8_t  Byte;
            int      Result;

            // Lecture non bloquante : timeout = 0
            Result = Received(&Byte, 1, 0);

            if(Result < 0)
            {
                m_State      = MODBUS_State_e::MODBUS_ERROR;
                return;
            }

            if(Result > 0)
            {
                if(m_RxLen < sizeof(m_RxBuf))
                {
                    m_RxBuf[m_RxLen++] = Byte;
                }
                else
                {
                    // Overflow du buffer RX
                    m_State = MODBUS_State_e::MODBUS_ERROR;
                    return;
                }

                // Assez d’octets pour considérer la trame complète ?
                if(IsEndOfRtuFrame(m_RxBuf, m_RxLen) == true)
                {
                    m_State = MODBUS_State_e::MODBUS_PARSE_RESPONSE;
                    return;
                }
            }

            // Timeout global de réponse
            if(TickHasTimeOut(m_StartTick, m_Cmd.TimeoutMsec) == true)
            {
                m_State = MODBUS_State_e::MODBUS_ERROR;
                return;
            }
    }
        break;
        
        case MODBUS_PARSE_RESPONSE:
        {
            int Status = m_pCentral->ParseResponse(m_Cmd,
                                                   m_RxBuf,
                                                   m_RxLen);

            if(Status < 0)
            {
                m_State = MODBUS_ERROR;
                return;
            }

            m_State = MODBUS_DONE
        }
        break;
        
        case MODBUS_DONE:
        {
            m_HasPending = false;
            m_State      = MODBUS_IDLE;
        }
        break;

        case MODBUS_ERROR:
        {
            m_HasPending = false;
            m_State      = MODBUS_IDLE;
            // Optionnel : notifier le central d’une erreur
    
        }
        break;

        default:
            break;
    }
}

void ModbusRTU::Initialize(ModbusCentral* pCentral, Console* pConsole)
{
    m_pCentral = pCentral;
    m_pConsole = pConsole;
}

int ModbusRTU::Send(const uint8_t* pData, size_t Length)
{
    if(m_pConsole == nullptr)
    {
        return -1;
    }

    return m_pConsole->Write(pData, Length);
}

int ModbusRTU::Received(uint8_t* pBuffer, size_t MaxLength, TickCount_t TimeOutMsec)
{
    if(m_pConsole == nullptr)
        return -1;

    return m_pConsole->Read(pBuffer, MaxLength, TimeOutMsec);    
}

bool ModbusRTU::Queue(const MODBUS_Command& Command)
{
    // Déjà occupé ?
    if(m_HasPending == true)
    {
        return false;
    }

    // Accepte la commande
    m_Command    = Command;
    m_HasPending = true;

    // Démarre la machine à états
    m_State = MODBUS_BUILD_FRAME;

    return true;
}

bool ModbusRTU::IsEndOfRTU_Frame(const uint8_t* pBuf, size_t Len)
{
    if(Len < 4)
        return false;   // adresse + fonction + CRC(2)

    uint8_t function = pBuf[1];

    switch(function)
    {
        // ------------------------------
        // Fonctions avec champ ByteCount
        // ------------------------------
        case 0x01: // Read Coils
        case 0x02: // Read Discrete Inputs
        case 0x03: // Read Holding Registers
        case 0x04: // Read Input Registers
        {
            if(Len < 3)
                return false;

            uint8_t byteCount = pBuf[2];
            size_t expected = 3 + byteCount + 2; // addr + func + bytecount + data + CRC

            return (Len >= expected);
        }

        // ------------------------------
        // Fonctions Write Single
        // ------------------------------
        case 0x05: // Write Single Coil
        case 0x06: // Write Single Register
            return (Len >= 8); // addr + func + addr_hi + addr_lo + val_hi + val_lo + CRC(2)

        // ------------------------------
        // Fonctions Write Multiple
        // ------------------------------
        case 0x0F: // Write Multiple Coils
        case 0x10: // Write Multiple Registers
            return (Len >= 8); // addr + func + addr_hi + addr_lo + qty_hi + qty_lo + CRC(2)

        // ------------------------------
        // Exception responses
        // ------------------------------
        default:
        {
            if(function & 0x80)
            {
                return (Len >= 5); // addr + func + exception_code + CRC(2)
            }
        }
        break;
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
