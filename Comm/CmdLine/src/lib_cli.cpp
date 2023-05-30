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
#include "lib_digini.h"
#undef  CLI_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CMD_LINE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef SystemState_e (CommandLine::*const CLI_Function_t)(void);

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CLI_PASSWORD_SIZE                           16
#define CLI_TERMINAL_RESET_DELAY                    100
#define CLI_AT_STR_SIZE                             2

#define CLI_STRING_CLEAR_SCREEN                     "\033[2J\e[H"
#define CLI_STRING_RESET_TERMINAL                   "\033c\r\n"

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const CLI_Function_t CLI_Function[NUMBER_OF_CLI_CMD] =
{
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    &CommandLine::CmdMENU,
  #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_FUNCTION_POINTER)   // Generation of the function pointer array
};

const char*              CommandLine::m_ErrorLabel                       = "ERROR, %s";
const CLI_CmdInputInfo_t CommandLine::m_CmdInputInfo[NUMBER_OF_CLI_CMD]  =
{
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    {CLI_CMD_R, 0, {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}} },
  #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_INPUT_INFO)
};

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
  const char CommandLine::m_StrAT_MENU[sizeof(CMD_MENU)] = CMD_MENU;
#endif

X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_CONST_STRING)           // Generation of all the string


const char* CommandLine::m_pCmdStr[NUMBER_OF_CLI_CMD] =
{
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    &m_StrAT_MENU[0],
  #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_CMD_STRING)
};

const int CommandLine::m_CmdStrSize[NUMBER_OF_CLI_CMD] =
{
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    sizeof(CMD_MENU) - 1,
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
            if(m_pFifo->At(i) == COMMA)
            {
                // Check if password match with internal password
                if(m_pFifo->Memncmp(&m_CMD_Password[0], i) == true)
                {
                    if(m_CMD_Password[i] == '\0')
                    {
                        i++;
                        m_pFifo->Flush(i);
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
//  Description:
//
//-------------------------------------------------------------------------------------------------
void CommandLine::CallbackFunction(int Type, void* pContext)
{
    switch(Type)
    {
        // TX from UART is completed then release memory.
        case UART_CALLBACK_COMPLETED_TX:
        {
            pMemoryPool->Free((void**)&pContext);
        }
        break;

        // RX data from UART then call RX_Callback.
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

                        m_pConsole->HeadBackward(1);
                    }
                    else
                    {
                        // Write data into the Fifo buffer

                        // Check if we can write into the Fifo
                        if(m_pConsole->Write((const void*)&Data, 1) != 1)
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
                        m_pConsole->Flush(CON_FIFO_PARSER_RX_SIZE);
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

            if(m_pConsole->GetAtoi(pValue, pParam->Min, pParam->Max, pParam->Base) == true)
            {
                if(i < NbParam)
                {
                    if(m_pConsole->IsItA_Comma() == false)
                    {
                        State = SYS_INVALID_PARAMETER;
                    }
                }
                else if(m_pConsole->IsItAnEOL() == false)
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
            m_pParamStr[i] = (char*)pMemoryPool->Alloc(CLI_STRING_SIZE);

            if(m_pParamStr[i] != nullptr)
            {
                if(m_pConsole->GetString(m_pParamStr[i], CLI_STRING_SIZE) == false)
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
//  Parameter(s):  Console*    pConsole       Pointer on the console class object
//  Return:         None
//
//  Description:    Initialize command line interface
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CommandLine::Initialize(Console* pConsole)
{
    TickCount_t Delay;

    m_pConsole          = pConsole;
    m_InputState        = CLI_CMD_LINE;       // Initial input type
    m_Step              = CLI_STEP_IDLE;      // Initial step of the parser
    m_ParserRX_Offset   = 0;
    m_IsItOnHold        = false;
    m_IsItOnStartup     = true;
    m_MuteSerialLogging = true;
    m_ChildProcess      = nullptr;

    pConsole->Printf(CON_SIZE_NONE, CLI_STRING_RESET_TERMINAL);
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
//  Note(s):        (1) This process is called in the Console process. do not call this process
//                      directly.
//                  (2) Preprocessing line of data to trap ERROR or OK or continue for data
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
    if(m_pConsole->ReadyRead() == true)
    {
        switch(int(m_Step))
        {
            case CLI_STEP_CMD_VALID:
            {
                // Preprocessing line of data to trap ERROR or OK or continue for data
                CommandNameSize = m_CommandNameSize;
                m_pConsole->ToUpper(CommandNameSize);
                State = SYS_INVALID_COMMAND;

            // Process the valid input by iterating through valid command list
                for(Command = int(FIRST_CLI_COMMAND); Command < int(NUMBER_OF_CLI_CMD); Command++)
                {
                    if(CommandNameSize == m_CmdStrSize[Command])                                            // First size must match
                    {
                        if(m_pConsole->Memncmp(m_pCmdStr[Command], CommandNameSize) == true)                // Compare command string
                        {
                            m_pConsole->Flush(CommandNameSize);                                             // Flush the command from the FIFO

                            if((m_ReadCommand == false) && (m_PlainCommand == false))
                            {
                               ProcessParams(CLI_CmdName_e(Command));                                       // if its a write command, get all parameters.
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
                    m_pConsole->Printf(CON_SIZE_NONE, m_ErrorLabel, "Command invalid\r\n");
                }

                m_pConsole->Flush(CON_FIFO_PARSER_RX_SIZE);                                                  // Flush completely the FIFO
                m_Step = CLI_STEP_IDLE;
            }
            break;

            case CLI_STEP_CMD_MALFORMED:
            {
                m_pConsole->Printf(CON_SIZE_NONE, m_ErrorLabel, "Malformed packet\r\n");
                m_Step = CLI_STEP_IDLE;
            }
            break;

            case CLI_STEP_CMD_BUFFER_OVERFLOW:
            {
                m_pConsole->Printf(CON_SIZE_NONE, m_ErrorLabel, "Buffer overflow\r\n");
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
            m_pConsole->Printf(CON_SIZE_NONE, m_ErrorLabel, "Command timeout\r\n");
            m_pConsole->Flush(CON_FIFO_PARSER_RX_SIZE);
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
void CommandLine::ReleaseControl(void)
{
    if(m_ChildProcess != nullptr)
    {
        m_ChildProcess = nullptr;
        m_InputState   = CLI_CMD_LINE;
    }
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
SystemState_e CommandLine::SendData(const uint8_t* p_BufferTX, size_t* pSizeTX, void* pContext)
{
    return SYS_READY;
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
//void CommandLine::SetSerialLogging(bool Mute)
//{
    //m_MuteSerialLogging = Mute;
//}

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
    //m_CLI_IsDisplayLock = State;
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
        m_pConsole->Printf(CON_SIZE_NONE, "AT%s=%s%s\r\n", m_pCmdStr[Cmd], pMsg1, pMsg2);
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_CMD_LINE == DEF_ENABLED)
