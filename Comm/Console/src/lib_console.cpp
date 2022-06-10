//-------------------------------------------------------------------------------------------------
//
//  File : lib_console.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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

#define CLI_GLOBAL
#include "lib_digini.h"
#undef  CLI_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CONSOLE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CLI_SERIAL_OUT_SIZE                         256
#define CLI_PASSWORD_SIZE                           16
#define CLI_TERMINAL_RESET_DELAY                    100
#define CLI_AT_STR_SIZE                             2

#define CLI_STRING_CLEAR_SCREEN                     "\033[2J\e[H"
#define CLI_STRING_RESET_TERMINAL                   "\033c\r\n"
#define CLI_TIME_DATE_STAMP                         "%04u-%02u-%02u %2u:%02u:%02u: "

//-------------------------------------------------------------------------------------------------
// Private(s) Function(s)
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
void Console::CallbackFunction(int Type, void* pContext)
{
    switch(Type)
    {
        // TX from uart is completed then release memory.
        case UART_CALLBACK_COMPLETED_TX:
        {
            pMemory->Free((void**)&pContext);
        }
        break;

        // RX data from uart then call RX_Callback.
        case UART_CALLBACK_RX:
        {
            RX_Callback(*((uint8_t*)pContext));
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RX_Callback
//
//  Parameter(s):   uint8_t     Data        First byte received
//
//  Return:         None
//
//  Description:    Here we received the character from the terminal.
//
//  Note(s):        This is a state machine to handle incoming character, according to state on menu
//                  or AT Sequence or input (string or decimal/hexadecimal).
//
//-------------------------------------------------------------------------------------------------
void Console::RX_Callback(uint8_t Data)
{
    switch(int(m_InputState))
    {
        case CLI_CMD_LINE:
        {
            //-------------------------------------------------------------------------------------
            // Parse FIFO
            //
            if(m_Step != CLI_STEP_CMD_VALID)
            {
                //---------------------------------------------------------------------------------
                // Waiting to process incoming command
                //
                if(m_Step != CLI_STEP_GETTING_DATA)
                {
                    // Receive more data than AT header so, this accelerate process in IRQ
                    if((((char)Data == 'a') || ((char)Data == 'A')) && (m_Step == CLI_STEP_WAITING_FOR_A))
                    {
                        m_Step = CLI_STEP_WAITING_FOR_T;
                    }
                    else if((((char)Data == 't') || ((char)Data == 'T')) && (m_Step == CLI_STEP_WAITING_FOR_T))
                    {
                        m_Step            = CLI_STEP_GETTING_DATA;
                        m_ReadCommand     = false;
                        m_PlainCommand    = true;
                        m_CommandNameSize = 0;
                    }
                    else
                    {
                        m_Step = CLI_STEP_IDLE;
                    }

                    m_ParserRX_Offset = 0;
                }

                //---------------------------------------------------------------------------------
                // Receiving an AT command. Basic parsing of the command
                else
                {
                    if((char)Data == '=')
                    {
                        m_PlainCommand = false;

                        if(m_ParserRX_Offset != 0)
                        {
                            m_CommandNameSize = m_ParserRX_Offset;
                        }
                        else
                        {
                            m_Step = CLI_STEP_CMD_MALFORMED;
                        }
                    }
                    else if((char)Data == '?')
                    {
                        // '?' must follow '='
                        if(m_CommandNameSize == m_ParserRX_Offset)
                        {
                            m_ReadCommand = true;
                        }
                        else // Malformed packet
                        {
                            m_Step = CLI_STEP_CMD_MALFORMED;
                        }
                    }
                    else if((char)Data == '\r')
                    {
                        if(m_CommandNameSize == 0)    // if not "=" or "=?"
                        {
                            m_CommandNameSize = m_ParserRX_Offset;
                        }

                        m_Step     = CLI_STEP_CMD_VALID;
//                        m_DataSize = m_ParserRX_Offset;   //  TODO found usage because not used !!!!!!!
                    }
                    else if((char)Data == '\b')
                    {
                        m_ParserRX_Offset--;

                        if(m_CommandNameSize == m_ParserRX_Offset)
                        {
                            m_CommandNameSize = 0;
                        }

                        m_pFifo->HeadBackward(1);
                    }
                    else
                    {
                        // Write data into the Fifo buffer

                        // Check if we can write into the Fifo
                        if(m_pFifo->Write((const void*)&Data, 1) != 1)
                        {
                            // We have overrun the Fifo buffer
                            m_Step = CLI_STEP_CMD_BUFFER_OVERFLOW;
                        }
                        else
                        {
                            m_ParserRX_Offset++;
                        }
                    }

                    // Flush the FIFO an Error has occurred.
                    if((m_Step != CLI_STEP_GETTING_DATA) && (m_Step != CLI_STEP_CMD_VALID))
                    {
                        m_pFifo->Flush(CLI_FIFO_PARSER_RX_SIZE);
                        // Parser send Error and reset the state machine
                    }
                }
            }

            if(m_Step != CLI_STEP_IDLE)
            {
                m_CommandTimeOut = GetTick();
            }
        }
        break;

        case CLI_USER_FUNCTION:
        {
            if(m_ChildProcess != nullptr)
            {
                m_ChildProcess(Data);
            }
            else
            {
                // Return to command line since no child process exist.
                m_Step = CLI_STEP_IDLE;
            }
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
// Public(s) Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   UART_Driver*    UartDriver       Pointer on the UART driver to use.
//  Return:         None
//
//  Description:    Initialize command line interface
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void Console::Initialize(UART_Driver* pUartDriver)
{
    TickCount_t Delay;

    m_pUartDriver           = pUartDriver;
    m_ParserRX_Offset       = 0;
    m_IsItOnHold            = false;
    m_IsItOnStartup         = true;
    m_MuteSerialLogging     = true;
    m_DebugLevel            = CON_DEBUG_LEVEL_0;
    m_ChildProcess          = nullptr;

    m_pFifo = new FIFO_Buffer(CLI_FIFO_PARSER_RX_SIZE);
    pUartDriver->RegisterCallback((CallbackInterface*)this);
    pUartDriver->EnableCallbackType(UART_CALLBACK_EMPTY_TX);
    pUartDriver->EnableCallbackType(UART_CALLBACK_COMPLETED_TX);
    Printf(CON_SIZE_NONE, CON_STRING_RESET_TERMINAL);
    Delay = GetTick();
    while(TickHasTimeOut(Delay, CON_TERMINAL_RESET_DELAY) == false);
    m_StartupTick = GetTick();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Command line interface process
//
//  Note(s):        Preprocessing line of data to trap ERROR or OK or continue for data
//
//-------------------------------------------------------------------------------------------------
void Console::Process(void)
{
    SystemState_e        State;
    int16_t              CommandNameSize;
    int                  Command;
    CLI_CommandSupport_e Support;

    if(m_IsItOnStartup == true)
    {
        if(TickHasTimeOut(m_StartupTick, CLI_NUMBER_OF_SECOND_FOR_STARTUP) == true)
        {
            m_IsItOnStartup = false;
        }
    }
    
    // TODO process active child
    if(m_ChildProcess != nullptr)
    {
    }
}
//-------------------------------------------------------------------------------------------------
//
//  Name:           GiveControlToChildProcess
//
//  Parameter(s):   pProcess          pointer to a function  void (*pProcess) (uint8_t Data)
//  Return:         None
//
//  Description:    Give control of the interface to a child process.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void Console::GiveControlToChildProcess(void(*pProcess)(uint8_t Data))
{
    if(pProcess != nullptr)
    {
        m_ChildProcess = pProcess;
        m_InputState   = CLI_USER_FUNCTION;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReleaseControl
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Release control back to the command line
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void Console::ReleaseControl(void)
{
    if(m_ChildProcess != nullptr)
    {
        m_ChildProcess = nullptr;
        m_InputState   = CLI_CMD_LINE;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Printf
//
//  Parameter(s):   size_t              MaxSize     Number of byte max to print
//                  const char*         pFormat     Formatted string.
//                  ...                             Parameter if any.
//
//  Return:         size_t              Number of character printed.
//
//  Description:    Send formatted string.
//
//  Note(s):        Memory block will be freed by DMA TC
//
//-------------------------------------------------------------------------------------------------
size_t Console::Printf(int MaxSize, const char* pFormat, ...)
{
    char*   pBuffer;
    va_list vaArg;
    size_t  Size = 0;

    if((pBuffer = (char*)pMemory->Alloc(CON_SERIAL_OUT_SIZE)) == nullptr)
    {
        return 0;
    }

    Size = (MaxSize == CON_SIZE_NONE) ? CON_SERIAL_OUT_SIZE : MaxSize;
    va_start(vaArg, (const char*)pFormat);
    Size = vsnprintf(&pBuffer[0], Size, pFormat, vaArg);
    while(m_pUartDriver->IsItBusy() == true){};
    m_pUartDriver->SendData((const uint8_t*)&pBuffer[0], &Size, pBuffer);
    va_end(vaArg);

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintSerialLog
//
//  Parameter(s):   CLI_DebugLevel_e    Level       Level of printf logging.
//                  const char*         pFormat     Formatted string.
//                  ...                             Parameter if any.
//
//  Return:         size_t              Number of character printed.
//
//  Description:    Send formatted string to console if menu system is active.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t Console::PrintSerialLog(CLI_DebugLevel_e Level, const char* pFormat, ...)
{
    va_list          vaArg;
    char*            pBuffer;
    size_t           Size = 0;

    if(m_MuteSerialLogging == false)
    {
        if((m_DebugLevel & Level) != CLI_DEBUG_LEVEL_0)
        {
            if((pBuffer = (char*)pMemory->Alloc(CLI_SERIAL_OUT_SIZE)) != nullptr)
            {
                va_start(vaArg, (const char*)pFormat);
                Size = vsnprintf(pBuffer, CLI_SERIAL_OUT_SIZE, pFormat, vaArg);
                m_pUartDriver->SendData((const uint8_t*)pBuffer, &Size, nullptr);
                va_end(vaArg);
                pMemory->Free((void**)&pBuffer);
            }
        }
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendData
//
//  p_BufferTX  Ptr on buffer with data to send.
//                              if = nullptr, internal TX Buffer remains the one set previously
//                  pSizeTX     Number of bytes to send, and on return, number of bytes sent
//   Return Value:  SystemState_e
//
//  Description:    This allow child process to send data to the UART.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e Console::SendData(const uint8_t* p_BufferTX, size_t* pSizeTX, void* pContext)
{

}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetSerialLogging
//
//  Parameter(s):   bool        Mute        - if true, serial logging is muted
//                                          - if false, serial logging unmuted
//
//  Return:         None
//
//  Description:    Set the mute flag for the serial logging.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void Console::SetSerialLogging(bool Mute)
{
    m_MuteSerialLogging = Mute;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockDisplay
//
//  Parameter(s):   bool       State        if true, Display is loccked.
//                                          if false, Display active.
//
//  Return:         None
//
//  Description:    Prevent CLI from updating display.
//
//  Note(s):        It is the responsibility the user to release the lock, when exiting a cli
//                  callback. Useful for logging to file, and prevent garbage to be visible in the
//                  file. also send the print stop label before exiting callback that use this
//                  feature.
//
//-------------------------------------------------------------------------------------------------
void Console::LockDisplay(bool State)
{
    VAR_UNUSED(State);
    //m_CLI_IsDisplayLock = State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayTimeDateStamp
//
//  Parameter(s):   Date_t*       Pointer on the Date Structure
//                  Time_t*       Pointer on the Time Structure
//
//  Return:         None
//
//  Description:    Display time and date stamp on terminal.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
// TODO move in comm_utility.h and .c this is a support function, it does not need to be in CLI
void Console::DisplayTimeDateStamp(Date_t* pDate, Time_t* pTime)
{
    Printf(CLI_SIZE_NONE, CLI_TIME_DATE_STAMP, pDate->Year,
                                               pDate->Month,
                                               pDate->Day,
                                               pTime->Hour,
                                               pTime->Minute,
                                               pTime->Second);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetString
//
//  Parameter(s):   char*   pBuffer     Pointer on the buffer to put the string
//                  size_t  Size        MAX size of the buffer
//
//  Return:         bool
//
//  Description:    Retrieve the string in the FIFO buffer. Must be located in quote, and inside
//                  the provided range
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool Console::GetString(char* pBuffer, size_t Size)
{
    char Character;
    bool Result = false;

    if(m_pFifo->At(0) == '"')
    {
        m_pFifo->Flush(1);

        for(size_t i = 0; ((i <= Size) && (Result == false)); i++)
        {
            if(m_pFifo->Read(&Character, 1) == 1)
            {
                if(Character == '"')
                {
                    Result = true;

                    if(i < Size)
                    {
                        *pBuffer++ = '\0';
                    }
                }
                else
                {
                    *pBuffer++ = Character;
                }
            }
        }
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsItA_Comma
//
//  Parameter(s):   None
//
//  Return:         true or false
//
//  Description:    Check if next character is a comma, and extract it if true, also adjust the
//                  size in command pool structure.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
// TODO move in comm_utility.h and .c this is a support function, it does not need to be in CLI
// bool Console::IsItA_Comma(give pointer to fifo or value)

bool Console::IsItA_Comma(void)
{
    bool Result = false;

    if(m_pFifo->At(0) == ',')
    {
        m_pFifo->Flush(1);
        Result = true;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetAtoi
//
//  Parameter(s):   int32_t*    pValue          Value will be returned here.
//                  int32_t     Min             Minimum accepted value.
//                  int32_t     Max             Maximum accepted value.
//                  int8_t      Base            Base of the extraction.
//                                                  - DECIMAL_BASE
//                                                  - HEXADECIMAL_BASE
//
//  Return:         true or false
//
//  Description:    Get the integer value from the FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
// TODO move in comm_utility.h and .c this is a support function, it does not need to be in CLI
// bool Console::GetAtoi(,,,, give pointer to fifo or value)

bool Console::GetAtoi(int32_t* pValue, int32_t Min, int32_t Max, uint8_t Base)
{
    uint32_t Size;
    bool     Result = false;

    Size = m_pFifo->Atoi(pValue, Base);

    if(Size != 0)
    {
        if((*pValue >= Min) && (*pValue <= Max))
        {
            Result = true;
        }
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsItAnEOL
//
//  Parameter(s):   None
//
//  Return:         true or false
//
//  Description:    Check to be sure there no more parameter on the command line.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool Console::IsItAnEOL(void)
{
    bool Result = false;

    if(m_pFifo->ReadyRead() == false)
    {
        Result = true;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_CONSOLE == DEF_ENABLED)
