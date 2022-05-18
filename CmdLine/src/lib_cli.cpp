//-------------------------------------------------------------------------------------------------
//
//  File : lib_cli.cpp
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
#include "lib_cli.h"
#undef  CLI_GLOBAL

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef SystemState_e (CommandLine::*const CLI_Function_t)(void);

//-------------------------------------------------------------------------------------------------
// Driver Check
//-------------------------------------------------------------------------------------------------

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

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
// Const(s)
//-------------------------------------------------------------------------------------------------

const CLI_Function_t CLI_Function[NUMBER_OF_CLI_CMD] =
{
  #if CLI_USE_VT100_MENU == DEF_ENABLED
    CmdMENU,
  #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_FUNCTION_POINTER)   // Generation of the function pointer array
 };


const char*              CommandLine::m_ErrorLabel                       = "ERROR, %s";
const CLI_CmdInputInfo_t CommandLine::m_CmdInputInfo[NUMBER_OF_CLI_CMD]  =
{
    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_INPUT_INFO)
};

#if CLI_USE_VT100_MENU == DEF_ENABLED
  const char CommandLine::m_StrAT_MENU[SIZE_OF_AT_MENU] = "MENU";
#endif

X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_CONST_STRING)           // Generation of all the string


const char* CommandLine::m_pCmdStr[NUMBER_OF_CLI_CMD] =
{
  #if CLI_USE_VT100_MENU == DEF_ENABLED
    &StrAT_MENU[0],
  #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_CMD_STRING)
};

const int CommandLine::m_CmdStrSize[NUMBER_OF_CLI_CMD] =
{
  #if CLI_USE_VT100_MENU == DEF_ENABLED
    sizeof("MENU") - 1,
  #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_STRING_SIZE)
};

//-------------------------------------------------------------------------------------------------
// Private(s) Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           HandleCmdPassword
//
//  Parameter(s):   void
//
//  Return:         SystemState_e       SYS_READY               if password is valid
//                                      SYS_INVALID_PASSWORD    if pwd invalid
//
//  Description:    Check if password is valid parsing the FIFO
//
//-------------------------------------------------------------------------------------------------
#if (CLI_USE_PASSWORD == DEF_ENABLED)
SystemState_e CommandLine::HandleCmdPassword(void)
{
    SystemState_e State = SYS_READY;

    // Check if password if configured
    if(m_CMD_Password[0] != '\0')
    {
        // Application need password
        for(int i = 0; i <= sizeof(m_CMD_Password); i++)
        {
            // There must be a comma in the FIFO
            if(m_pTxFifo->At(i) == COMMA)
            {
                // Check if password match with internal password
                if(m_pTxFifo->Memncmp(&m_CMD_Password[0], i) == true)
                {
                    if(m_CMD_Password[i] == '\0')
                    {
                        i++;
                        m_pTxFifo->Flush(i);
                        break;
                    }
                    else
                    {
                        State = SYS_INVALID_PASSWORD;
                        break;
                    }
                }
                else
                {
                    State = SYS_INVALID_PASSWORD;
                    break;
                }
            }
        }
    }

     return State;
}
#endif

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
void CommandLine::CallbackFunction(int Type, void* pContext)
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
void CommandLine::RX_Callback(uint8_t Data)
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

                        m_pTxFifo->HeadBackward(1);
                    }
                    else
                    {
                        // Write data into the Fifo buffer

                        // Check if we can write into the Fifo
                        if(m_pTxFifo->Write((const void*)&Data, 1) != 1)
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
                        m_pTxFifo->Flush(CLI_FIFO_PARSER_RX_SIZE);
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
//
//  Name:           ProcessParams
//
//  Parameter(s):   CLI_CmdName_e    Command
//  Return:         None
//
//  Description:    Process the write parameters for command
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CommandLine::ProcessParams(CLI_CmdName_e Command)
{
    SystemState_e         State;
    const CLI_CmdParam_t* pParam;
    int32_t*              pValue;
    uint8_t               NbParam;

    State   = SYS_READY;
    NbParam = m_CmdInputInfo[Command].NumberOfParam;

    for(int i = 0; (i < NbParam) && (State == SYS_READY); /* no increment here */)
    {
        pParam = &m_CmdInputInfo[Command].Param[i];
        pValue = &m_ParamValue[i];

        if(pParam->Base != DEF_STRING_BASE)
        {
            i++;                    // Must be incremented before comparison

            if(GetAtoi(pValue, pParam->Min, pParam->Max, pParam->Base) == true)
            {
                if(i < NbParam)
                {
                    if(IsItA_Comma() == false)
                    {
                        State = SYS_INVALID_PARAMETER;
                    }
                }
                else if(IsItAnEOL() == false)
                {
                    State = SYS_INVALID_PARAMETER;
                }
            }
            else
            {
                State = SYS_INVALID_PARAMETER;
            }
        }
        else
        {
            m_pParamStr[i] = (char*)pMemory->Alloc(CLI_STRING_SIZE);

            if(m_pParamStr[i] != nullptr)
            {
                if(GetString(m_pParamStr[i], CLI_STRING_SIZE) == false)
                {
                    State = SYS_INVALID_PARAMETER;
                }
            }
            else
            {
                State = SYS_FAIL_MEMORY_ALLOCATION;
            }

            i++;                    // Must be incremented after variable offset use
        }
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
void CommandLine::Initialize(UART_Driver* pUartDriver)
{
    TickCount_t Delay;

    m_pUartDriver           = pUartDriver;
    m_InputState            = CLI_CMD_LINE;       // Initial input type
    m_Step                  = CLI_STEP_IDLE;      // Initial step of the parser
    m_ParserRX_Offset       = 0;
    m_IsItOnHold            = false;
    m_IsItOnStartup         = true;
    m_MuteSerialLogging     = true;
    m_DebugLevel            = CLI_DEBUG_LEVEL_0;
    m_ChildProcess          = nullptr;

    m_pTxFifo = new FIFO_Buffer(CLI_FIFO_PARSER_RX_SIZE);
    pUartDriver->RegisterCallback((CallbackInterface*)this);
    pUartDriver->EnableCallbackType(UART_CALLBACK_EMPTY_TX);
    pUartDriver->EnableCallbackType(UART_CALLBACK_COMPLETED_TX);
    Printf(CLI_SIZE_NONE, CLI_STRING_RESET_TERMINAL);
    Delay = GetTick();
    while(TickHasTimeOut(Delay, CLI_TERMINAL_RESET_DELAY) == false);
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
void CommandLine::Process(void)
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

    // --------------------------------------------------------------------------------------------
    // CLI Parser
    //
    if(m_pTxFifo->ReadyRead() == true)
    {
        switch(int(m_Step))
        {
            case CLI_STEP_CMD_VALID:
            {
                // Preprocessing line of data to trap ERROR or OK or continue for data
                CommandNameSize = m_CommandNameSize;
                m_pTxFifo->ToUpper(CommandNameSize);
                State = SYS_INVALID_COMMAND;

            // Process the valid input by iterating through valid command list
                for(Command = int(FIRST_CLI_COMMAND); Command < int(NUMBER_OF_CLI_CMD); Command++)
                {
                    if(CommandNameSize == m_CmdStrSize[Command])                                          // First size must match
                    {
                        if(m_pTxFifo->Memncmp(m_pCmdStr[Command], CommandNameSize) == true)               // Compare command string
                        {
                            m_pTxFifo->Flush(CommandNameSize);                                              // Flush the command from the FIFO

                            if((m_ReadCommand == false) && (m_PlainCommand == false))
                            {
                               ProcessParams(CLI_CmdName_e(Command));                                                  // if its a write command, get all parameters.
                            }

                            Support = m_CmdInputInfo[Command].Support;

                            if((((Support & CLI_CMD_H) != 0) && (m_IsItOnHold == true))         ||          // Hold specified command only on hold mode
                               (((Support & CLI_CMD_S) != 0) && (m_IsItOnStartup == true))      ||          // Startup specified command only startup phase
                               ((Support & (CLI_CMD_H | CLI_CMD_S)) == 0))                                  // All other command
                            {
                                State = (this->*CLI_Function[Command])();
                                SendAnswer(CLI_CmdName_e(Command), State, nullptr);
                            }
                        }
                    }
                }

                if(State == SYS_INVALID_COMMAND)
                {
                    Printf(CLI_SIZE_NONE, m_ErrorLabel, "Command invalid\r\n");
                }

                m_pTxFifo->Flush(CLI_FIFO_PARSER_RX_SIZE);                                                  // Flush completely the FIFO
                m_Step = CLI_STEP_IDLE;
            }
            break;

            case CLI_STEP_CMD_MALFORMED:
            {
                Printf(CLI_SIZE_NONE, m_ErrorLabel, "Malformed packet\r\n");
                m_Step = CLI_STEP_IDLE;
            }
            break;

            case CLI_STEP_CMD_BUFFER_OVERFLOW:
            {
                Printf(CLI_SIZE_NONE, m_ErrorLabel, "Buffer overflow\r\n");
                m_Step = CLI_STEP_IDLE;
            }
            break;

            default: /* Nothing to do */ break;
        }
    }

    //---------------------------------------------------------------------------------------------

    if(m_Step != CLI_STEP_IDLE)
    {
        if(TickHasTimeOut(m_CommandTimeOut, CLI_CMD_TIME_OUT) == true)
        {
            m_Step = CLI_STEP_IDLE;
            Printf(CLI_SIZE_NONE, m_ErrorLabel, "Command timeout\r\n");
            m_pTxFifo->Flush(CLI_FIFO_PARSER_RX_SIZE);
        }
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
void CommandLine::GiveControlToChildProcess(void(*pProcess)(uint8_t Data))
{
    if(pProcess != nullptr)
    {
        m_ChildProcess = pProcess;
        m_InputState   = CLI_USER_FUNCTION;
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
size_t CommandLine::Printf(int MaxSize, const char* pFormat, ...)
{
    char*   pBuffer;
    va_list vaArg;
    size_t  Size = 0;

    if((pBuffer = (char*)pMemory->Alloc(CLI_SERIAL_OUT_SIZE)) == nullptr)
    {
        return 0;
    }

    Size = (MaxSize == CLI_SIZE_NONE) ? CLI_SERIAL_OUT_SIZE : MaxSize;
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
size_t CommandLine::PrintSerialLog(CLI_DebugLevel_e Level, const char* pFormat, ...)
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
void CommandLine::SetSerialLogging(bool Mute)
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
void CommandLine::LockDisplay(bool State)
{
    VAR_UNUSED(State);
   // CLI_IsDisplayLock = State;
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

void CommandLine::DisplayTimeDateStamp(Date_t* pDate, Time_t* pTime)
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
//  Name:           SendAnswer
//
//  Parameter(s):   CLI_CmdName_e   CommandID           Command identification.
//                  SystemState_e   State               State code.
//                  const char*     Answer              Specific error string if not nullptr.
//
//  Return:         None
//
//  Description:    Send state line to terminal.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CommandLine::SendAnswer(CLI_CmdName_e Cmd, SystemState_e State, const char* Answer)
{
    const char* pMsg1 = "NACK, ";
    const char* pMsg2 = "";

    switch(State)   // Response message to command
    {
        case SYS_READY:                  { pMsg1 = "OK";                break; }
        case SYS_OK_READ:                { pMsg1 = "";
                                           pMsg2 = Answer;
                                           break;
                                         }

      #ifdef CLI_USE_EXTENDED_ERROR
        case SYS_OK_DENIED:              { pMsg2 = "Denied";                   break; }
        case SYS_CMD_NO_READ_SUPPORT:    { pMsg2 = "No Read Support";          break; }
        case SYS_CMD_NO_WRITE_SUPPORT:   { pMsg2 = "No Write Support";         break; }
        case SYS_INVALID_PARAMETER:      { pMsg2 = "Invalid Parameter";        break; }
        case SYS_INVALID_PASSWORD:       { pMsg2 = "Password Invalid";         break; }
        case SYS_FAIL_MEMORY_ALLOCATION: { pMsg2 = "Memory allocation Error";  break; }
      #else
        case SYS_OK_DENIED:
        case SYS_CMD_NO_READ_SUPPORT:
        case SYS_CMD_NO_WRITE_SUPPORT:
        case SYS_INVALID_PARAMETER:
        case SYS_INVALID_PASSWORD:
        case SYS_FAIL_MEMORY_ALLOCATION:
       #endif

        // SYS_OK_SILENT:                // We return nothing.. it is silent
        // SYS_INVALID_COMMAND:          // This case is not supposed to happened
        default:                         { pMsg1 = "";                  break; }
    }

    if(State != SYS_OK_SILENT)
    {
        Printf(CLI_SIZE_NONE, "AT%s=%s%s\r\n", m_pCmdStr[Cmd], pMsg1, pMsg2);
    }
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
bool CommandLine::GetString(char* pBuffer, size_t Size)
{
    char Character;
    bool Result = false;

    if(m_pTxFifo->At(0) == '"')
    {
        m_pTxFifo->Flush(1);

        for(size_t i = 0; ((i <= Size) && (Result == false)); i++)
        {
            if(m_pTxFifo->Read(&Character, 1) == 1)
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
bool CommandLine::IsItA_Comma(void)
{
    bool Result = false;

    if(m_pTxFifo->At(0) == ',')
    {
        m_pTxFifo->Flush(1);
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
bool CommandLine::GetAtoi(int32_t* pValue, int32_t Min, int32_t Max, uint8_t Base)
{
    uint32_t Size;
    bool     Result = false;

    Size = m_pTxFifo->Atoi(pValue, Base);

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
bool CommandLine::IsItAnEOL(void)
{
    bool Result = false;

    if(m_pTxFifo->ReadyRead() == false)
    {
        Result = true;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------

