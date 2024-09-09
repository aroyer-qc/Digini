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

#define CONSOLE_GLOBAL
#include "./lib_digini.h"
#undef  CONSOLE_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CONSOLE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CON_SERIAL_OUT_SIZE                         256

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
    uint8_t* pBuffer;

    m_pUartDriver           = pUartDriver;
    //m_IsItOnHold            = false;
    //m_IsItOnStartup         = true;
    m_MuteSerialLogging     = true;
  #if (DIGINI_USE_DEBUG_IN_CONSOLE == DEF_ENABLED)
    m_DebugLevel            = SYS_DEBUG_NONE;
  #endif
    m_ActiveProcessLevel    = CON_NOT_CONNECTED;

    for(int i = 0; i < CON_CHILD_PROCESS_PUSH_POP_LEVEL; i++)
    {
        m_pChildProcess[i]  = nullptr;
    }

    m_Fifo.Initialize(CON_FIFO_PARSER_RX_SIZE);
    pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(CON_FIFO_PARSER_RX_SIZE, MEM_DBG_CONSOLE_1);        // Reserve memory for UART internal DMA operation.

  #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)                                          // not sure it can work without DMA
    pUartDriver->DMA_ConfigRX(pBuffer, CON_FIFO_PARSER_RX_SIZE);
  #endif

  #if (UART_DRIVER_USE_CALLBACK_CFG == DEF_ENABLED)                                 // not sure it can work without DMA
    pUartDriver->RegisterCallback((CallbackInterface*)this);
   #if (UART_DRIVER_RX_NOT_EMPTY_CFG == DEF_ENABLED)
    pUartDriver->EnableCallbackType(UART_CALLBACK_RX_NOT_EMPTY | UART_CALLBACK_TX_COMPLETED | UART_CALLBACK_RX_ERROR);
   #endif
   #if (UART_DRIVER_RX_IDLE_CFG == DEF_ENABLED)
    pUartDriver->EnableCallbackType(UART_CALLBACK_RX_IDLE | UART_CALLBACK_TX_COMPLETED | UART_CALLBACK_RX_ERROR);
   #endif
  #endif

  #if (CON_TRAP_INCOMING_COMMENT_LINE == DEF_ENABLED)
    m_InTrapForCommentLine = false;
    m_IsItIdle             = true;
  #endif
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
//  Note(s):        If there here active child process than give control to the child process.
//                  Otherwise received data is lost because no handle to use it.
//
//-------------------------------------------------------------------------------------------------
void Console::Process(void)
{
  #if (CON_TRAP_INCOMING_COMMENT_LINE == DEF_ENABLED)
    if(ReadyRead() == true)
    {
        if((m_Fifo.At(0) == CON_TRAP_COMMENT_CHARACTER) && (m_IsItIdle == true))        // Need to be idle and first char is comment.
        {
            m_Fifo.Flush(1);                                                            // Flush it
            m_InTrapForCommentLine = true;                                              // This is a comment line
            m_TrapTimeOut = GetTick();                                                  // Get tick for time out of line
        }
        else
        {
            if(m_InTrapForCommentLine == true)                                          // Need to trap all character code in the line
            {
                if(m_Fifo.At(0) == CON_END_OF_LINE_MARKER)                              // Until end of line marker is detected
                {
                    m_InTrapForCommentLine = false;                                     // No longer in comment line
                    m_IsItIdle = true;                                                  // Idle
                }

                m_Fifo.Flush(1);                                                        // Flush the comment line character from fifo.
            }
        }

        m_IsItIdle = false;                                                             // no longer on idle... prevent detecting CON_TRAP_COMMENT_CHARACTER in middle of reception
    }
    else
    {
        if(TickHasTimeOut(m_TrapTimeOut, CON_TRAP_COMMENT_TIME_OUT) == true)            // Check if not idle for too much time without receiving CON_END_OF_LINE_MARKER
        {
            m_InTrapForCommentLine = false;                                             // Exit Trap
            m_IsItIdle = true;                                                          // Idle
        }
    }
  #endif

    if((m_pChildProcess[m_ActiveProcessLevel - 1] != nullptr) && (m_ActiveProcessLevel != CON_NOT_CONNECTED))
    {
        m_pChildProcess[m_ActiveProcessLevel - 1]->IF_Process();
    }
}
//-------------------------------------------------------------------------------------------------
//
//  Name:           GiveControlToChildProcess
//
//  Parameter(s):   pChildProcess           pointer ChildProcessInterface
//  Return:         None
//
//  Description:    Give control of the interface to a child process.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void Console::GiveControlToChildProcess(ChildProcessInterface* pChildProcess)
{
    if((pChildProcess != nullptr) && (m_ActiveProcessLevel < CON_CHILD_PROCESS_PUSH_POP_LEVEL))
    {
        m_ActiveProcessLevel++;
        m_pChildProcess[m_ActiveProcessLevel - 1] = pChildProcess;
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
    if((m_pChildProcess != nullptr) && (m_ActiveProcessLevel != CON_NOT_CONNECTED))
    {
        m_pChildProcess[m_ActiveProcessLevel - 1] = nullptr;
        m_ActiveProcessLevel--;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Printf
//
//  Parameter(s):   Label_e     Label       ID of the label to with optional formatting.
//                  ...                             Parameter if any.
//
//  Return:         None
//
//  Description:    Common function to format string and send it to console.
//
//-------------------------------------------------------------------------------------------------
size_t Console::Printf(Label_e Label, ...)
{
    va_list     vaArg;
    const char* pFormat;

    pFormat = myLabel.GetPointer(Label);
    va_start(vaArg, Label);
    return Printf(pFormat, &vaArg);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Printf
//
//  Parameter(s):   const char*         pFormat     Formatted string.
//                  ...                             Parameter if any.
//
//  Return:         size_t              Number of character printed.
//
//  Description:    Send formatted string.
//
//  Note(s):        Memory block will be freed by DMA TC
//
//-------------------------------------------------------------------------------------------------
size_t Console::Printf(const char* pFormat, ...)
{
    va_list vaArg;

    va_start(vaArg, pFormat);
    return Printf(pFormat, &vaArg);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Printf
//
//  Parameter(s):   const char* pFormat     Formatted string.
//                  va_list*    p_vaArg     Parameter from va_list.
//
//  Return:         size_t
//
//  Description:    Common function to format string and send it.
//
//  Note(s)         Memory are freed in the callback.
//
//-------------------------------------------------------------------------------------------------
size_t Console::Printf(const char* pFormat, va_list* p_vaArg)
{
    char*  pBuffer;
    size_t Size = 0;

    pBuffer = (char*)pMemoryPool->Alloc(CON_SERIAL_OUT_SIZE, MEM_DBG_CONSOLE_2);

    if(pBuffer != nullptr)
    {
        Size = LIB_vsnprintf(pBuffer, CON_SERIAL_OUT_SIZE, pFormat, *p_vaArg);

        if(Size != 0)
        {
            SendData((const uint8_t*)&pBuffer[0], &Size);
        }
        else
        {
            pMemoryPool->Free((void**)&pBuffer);
        }
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintSerialLog
//
//  Parameter(s):   SystemDebugLevel_e  Level       Level of printf logging.
//                  const char*         pFormat     Formatted string.
//                  ... or va_list                  Parameter if any.
//
//  Return:         size_t              Number of character printed.
//
//  Description:    Send formatted string to console if logging is not muted.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_DEBUG_IN_CONSOLE == DEF_ENABLED)
size_t Console::PrintSerialLog(SystemDebugLevel_e Level, const char* pFormat, ...)
{
    size_t  Size;
    va_list vaArg;

    va_start(vaArg, pFormat);
    Size = PrintSerialLog(Level, pFormat, vaArg);
    va_end(vaArg);

    return Size;
}

size_t Console::PrintSerialLog(SystemDebugLevel_e Level, const char* pFormat, va_list vaArg)
{
    char*            pBuffer;
    size_t           Size = 0;

    if(m_MuteSerialLogging == false)
    {
        //if((m_DebugLevel & Level) != CON_DEBUG_NONE)      TODO fix this finish support for it
        {
            if((pBuffer = (char*)pMemoryPool->Alloc(CON_SERIAL_OUT_SIZE, MEM_DBG_CONSOLE_3)) != nullptr)
            {
                Size = vsnprintf(pBuffer, CON_SERIAL_OUT_SIZE, pFormat, vaArg);
                m_pUartDriver->SendData((const uint8_t*)pBuffer, &Size);
            }
        }
    }

    return Size;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendData
//
//  p_BufferTX      p_BufferTX  Pointer on buffer with data to send.
//                              if = nullptr, internal TX Buffer remains the one set previously
//                  pSizeTX     Number of bytes to send, and on return, number of bytes sent
//   Return Value:  SystemState_e
//
//  Description:    This allow child process to send data to the UART.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e Console::SendData(const uint8_t* p_BufferTX, size_t* pSizeTX)
{
    m_pUartDriver->SendData((const uint8_t*)p_BufferTX, pSizeTX);
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
void Console::DisplayTimeDateStamp(Date_t* pDate, Time_t* pTime)
{
    Printf(CON_SIZE_NONE, CON_TIME_DATE_STAMP, pDate->Year,
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

    if(m_Fifo.At(0) == '"')
    {
        m_Fifo.Flush(1);

        for(size_t i = 0; ((i <= Size) && (Result == false)); i++)
        {
            if(m_Fifo.Read(&Character, 1) == 1)
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
// bool Console::IsItA_Comma(give pointer to fifo or value)
bool Console::IsItA_Comma(void)
{
    bool Result = false;

    if(m_Fifo.At(0) == ',')
    {
        m_Fifo.Flush(1);
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
// bool Console::GetAtoi(,,,, give pointer to fifo or value)
bool Console::GetAtoi(int32_t* pValue, int32_t Min, int32_t Max, uint8_t Base)
{
    uint32_t Size;
    bool     Result = false;

    Size = m_Fifo.Atoi(pValue, Base);

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
//  Note(s):        TODO is it not to simplistic????
//
//-------------------------------------------------------------------------------------------------
bool Console::IsItAnEOL(void)
{
    bool Result = false;

    if(m_Fifo.ReadyRead() == false)
    {
        Result = true;
    }

    return Result;
}

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
        // When DMA transfert is complete.
      #if (UART_DRIVER_DMA_TX_COMPLETED_CFG == DEF_ENABLED)
        case UART_CALLBACK_TX_DMA:
        {
            pMemoryPool->Free((void**)&pContext);
        }
        break;
      #endif

        // TX from uart is completed then release memory.
      #if (UART_DRIVER_TX_COMPLETED_CFG == DEF_ENABLED)
        case UART_CALLBACK_TX_COMPLETED:
        {
            pMemoryPool->Free((void**)&pContext);
        }
        break;
      #endif

      #if (UART_DRIVER_RX_NOT_EMPTY_CFG == DEF_ENABLED)
        case UART_CALLBACK_RX_NOT_EMPTY:
        {
            uint8_t* pData = (uint8_t*)pContext;
            m_Fifo.Write(pData, 1);
        }
        break;
      #endif

      #if (UART_DRIVER_RX_IDLE_CFG == DEF_ENABLED)
        case UART_CALLBACK_RX_IDLE:
        {
            UART_Transfer_t* pTransfer = (UART_Transfer_t*)pContext;
            m_Fifo.Write(pTransfer->pBuffer, pTransfer->Size);
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

#endif // (DIGINI_USE_CONSOLE == DEF_ENABLED)
