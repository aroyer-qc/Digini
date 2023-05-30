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
#include "lib_digini.h"
#undef  CONSOLE_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CONSOLE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CON_SERIAL_OUT_SIZE                         256

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
            pMemoryPool->Free((void**)&pContext);
        }
        break;

        // RX data from uart then call RX_Callback.
        case UART_CALLBACK_RX:
        {
            // We should copy the data here from the buffer... remember here it is call from an interrupt... no time to process stuff
            // TODO handle the fifo
// ???
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
    m_pUartDriver           = pUartDriver;
    m_ParserRX_Offset       = 0;
    m_IsItOnHold            = false;
    m_IsItOnStartup         = true;
    m_MuteSerialLogging     = true;
    m_DebugLevel            = CON_DEBUG_LEVEL_0;
  #if (CON_CHILD_PROCESS_PUSH_POP_LEVEL > 1)
    for(int i = 0; i < CON_CHILD_PROCESS_PUSH_POP_LEVEL; i++)
    {
        m_ChildProcess[i]   = nullptr;
        m_pCallbackRX[i]    = nullptr;
    }
  #else
    m_ChildProcess          = nullptr;
    m_pCallbackRX           = nullptr;
  #endif

    m_ActiveProcessLevel    = CON_NOT_CONNECTED;

    m_Fifo.Initialize(CON_FIFO_PARSER_RX_SIZE);
  // ??  pUartDriver->SetBaudRate((CallbackInterface*)this);
    pUartDriver->RegisterCallback((CallbackInterface*)this);
    pUartDriver->EnableCallbackType(UART_CALLBACK_EMPTY_TX, m_pRX_Transfert);
    pUartDriver->EnableCallbackType(UART_CALLBACK_EMPTY_TX);
    pUartDriver->EnableCallbackType(UART_CALLBACK_COMPLETED_TX);
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
  #if (CON_CHILD_PROCESS_PUSH_POP_LEVEL > 1)
    if(m_ChildProcess[m_ActiveProcessLevel] != nullptr)
    {
        m_ChildProcess[m_ActiveProcessLevel]();
    }
  #else
    if(m_ChildProcess != nullptr)
    {
        m_ChildProcess();
    }
  #endif

}
//-------------------------------------------------------------------------------------------------
//
//  Name:           GiveControlToChildProcess
//
//  Parameter(s):   pProcess          pointer to a function  void (*pProcess) (uint8_t Data)
//                  pCallbackRX         CallbackInterface*  for RX data
//  Return:         None
//
//  Description:    Give control of the interface to a child process.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void Console::GiveControlToChildProcess(void(*pProcess)(void), CallbackInterface* pCallbackRX)
{
  #if (CON_CHILD_PROCESS_PUSH_POP_LEVEL > 1)
    if((pProcess != nullptr) && (m_ActiveProcessLevel < (CON_CHILD_PROCESS_PUSH_POP_LEVEL - 1)))
    {
        m_ActiveProcessLevel++;
        m_ChildProcess[m_ActiveProcessLevel] = pProcess;
        m_pCallbackRX[m_ActiveProcessLevel]  = pCallbackRX;
    }
  #else
    if(pProcess != nullptr)
    {
        m_ActiveProcessLevel++;
        m_ChildProcess = pProcess;
        m_pCallbackRX  = pCallbackRX;
    }
  #endif
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
  #if (CON_CHILD_PROCESS_PUSH_POP_LEVEL > 1)
    if((m_ChildProcess != nullptr) && (m_ActiveProcessLevel != CON_NOT_CONNECTED))
    {
        m_ChildProcess[m_ActiveProcessLevel] = nullptr;
        m_pCallbackRX[m_ActiveProcessLevel]  = nullptr;
        m_ActiveProcessLevel--;
    }
  #else
    if(m_ChildProcess != nullptr)
    {
        m_ChildProcess       = nullptr;
        m_pCallbackRX        = nullptr;
        m_ActiveProcessLevel = CON_NOT_CONNECTED;
    }
  #endif
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

    if((pBuffer = (char*)pMemoryPool->Alloc(CON_SERIAL_OUT_SIZE)) != nullptr)
    {
        Size = (MaxSize == CON_SIZE_NONE) ? CON_SERIAL_OUT_SIZE : MaxSize;
        va_start(vaArg, pFormat);
        Size = vsnprintf(&pBuffer[0], Size, pFormat, vaArg);
        while(m_pUartDriver->IsItBusy() == true){};
        m_pUartDriver->SendData((const uint8_t*)&pBuffer[0], &Size, pBuffer);
        va_end(vaArg);
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintSerialLog
//
//  Parameter(s):   CON_DebugLevel_e    Level       Level of printf logging.
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
size_t Console::PrintSerialLog(CON_DebugLevel_e Level, const char* pFormat, ...)
{
    va_list          vaArg;
    char*            pBuffer;
    size_t           Size = 0;

    if(m_MuteSerialLogging == false)
    {
        if((m_DebugLevel & Level) != CON_DEBUG_LEVEL_0)
        {
            if((pBuffer = (char*)pMemoryPool->Alloc(CON_SERIAL_OUT_SIZE)) != nullptr)
            {
                va_start(vaArg, pFormat);
                Size = vsnprintf(pBuffer, CON_SERIAL_OUT_SIZE, pFormat, vaArg);
                m_pUartDriver->SendData((const uint8_t*)pBuffer, &Size, nullptr);
                va_end(vaArg);
                pMemoryPool->Free((void**)&pBuffer);
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
//  Note(s):
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

#endif // (DIGINI_USE_CONSOLE == DEF_ENABLED)
