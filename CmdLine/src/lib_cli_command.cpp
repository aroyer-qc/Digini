//-------------------------------------------------------------------------------------------------
//
//  File : lib_cli_command.cpp
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
//
//  Note: User CLI Command Function: This is the user space for the user CLI command set.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define _CLI_COMMAND_GLOBAL
#include "lib_cli_command.h"
#undef  _CLI_COMMAND_GLOBAL
#include "lib_macro.h"
//#include "generic.h"
#include <string.h>
#include "lib_memory.h"
#include "project_def.h"

//-------------------------------------------------------------------------------------------------
// Prototype(s)
//-------------------------------------------------------------------------------------------------

SystemState_e   CLI_CmdHOLD             (void);
SystemState_e   CLI_CmdRELEASE          (void);
SystemState_e   CLI_CmdRESET            (void);
SystemState_e   CLI_CmdSTATUS           (void);
SystemState_e   CLI_CmdINFO             (void);
SystemState_e   CLI_CmdVERSION          (void);
SystemState_e   CLI_CmdTEST1            (void);
SystemState_e   CLI_CmdTEST2            (void);
SystemState_e   CLI_CmdTEST3            (void);

//-------------------------------------------------------------------------------------------------
// Private(s) function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdHOLD
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    AT Command to put system on hold
//
//  Note(s):        This command for EXAMPLE can be use to put system on hold at boot up.
//                  allowing more command to be perform. command that are not avalaible at run
//                  time.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdHOLD(void)
{
    SystemState_e Error;

    if(m_PlainCommand == true)
    {
        m_IsItOnHold = true;
        Error        = SYS_READY;
    }
    else
    {
        Error = SYS_INVALID_PARAMETER;       // No parameter write or read on this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdRELEASE
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    AT Command to release hold on system
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdRELEASE(void)
{
    SystemState_e Error;

    if(m_IsItOnHold == true)
    {
        if(m_PlainCommand == true)
        {
            m_IsItOnHold = false;
            Error        =  SYS_READY;
        }
        else
        {
            Error =  SYS_INVALID_PARAMETER;       // No parameter write or read on this command
        }
    }
    else
    {
        Error =  SYS_OK_DENIED;                   // Access to this command is forbidden in run mode
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdRESET
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:  	Command to reset and rebbot the system
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdRESET(void)
{
    SystemState_e Error;

    if(m_PlainCommand == true)
    {
        if(m_ReadCommand != true)
        {
            //NVIC_SystemReset();
        }
    }
    else
    {
        Error =  SYS_INVALID_PARAMETER;       // No parameter on this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdSTATUS
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Return system status
//
//  Note(s):        For this command examples.
//
//                    - This command is enable at all time (no check of flag CLI_IsItOnHold).
//                    - Read return a specific user information status.
//                    - Write is not supported in this example.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdSTATUS(void)
{
    SystemState_e Error;
    char          Response[20];
    int           Status;

    // Accept plain or read command
    if((m_ReadCommand == true) || (m_PlainCommand == true))
    {
         // In this example we only return the state of the CLI
        if(m_IsItOnHold == true) Status = 0;
        else                     Status = 1;
        snprintf(Response, 20, "%d", Status);
        SendAnswer(AT_STATUS, SYS_OK_READ, Response);
        Error =  SYS_OK_SILENT;
    }
    else
    {
        Error =  SYS_CMD_NO_WRITE_SUPPORT;     // No write support for this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdTEST1
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Example of a command
//
//  Note(s):        For this command examples.
//
//                    - This command is enable at all time (no check of flag CLI_IsItOnHold).
//                    - Read return a specific user information.
//                    - Write is not supported in this example.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdTEST1(void)
{
    SystemState_e   Error;
    char            Response[11];
    static uint64_t Count = 1234;

    if(m_ReadCommand == true)
    {
        Count++;
        snprintf(Response, 11, "0x%08llX", Count);
        SendAnswer(AT_TEST1,  SYS_OK_READ, Response);
        Error =  SYS_OK_SILENT;
    }
    else
    {
        Error = SYS_CMD_NO_WRITE_SUPPORT;       // No write on this command
    }

    return Error;
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdTEST2
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Example of a command
//
//  Note(s):        For this command examples, you can write 3 news parameters, on read the
//                  parameter. It show how to check for parameter validity.
//
//                    - This command are enable only when on hold (check of flag CLI_IsItOnHold).
//                    - Read return application data or application information.
//                    - Write new value for application data or function.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdTEST2(void)
{
    SystemState_e       Error;
    char                Response[20];
    static uint8_t      State_1;                                // Here value are static because for the example, you can read them back
    static uint8_t      Value_1;
    static int16_t      Value_2;

    if(m_IsItOnHold == true)                                  // This command will be allowed only if system is on hold
    {
        if((m_ReadCommand == true) || (m_PlainCommand == true)) // Process also a plain command has a read
        {
            snprintf(Response, 20, "%d,%d,%d", State_1, Value_1, Value_2);

            SendAnswer(AT_TEST2, SYS_OK_READ, Response);
            Error =  SYS_OK_SILENT;
        }
        else
        {
            State_1 = m_ParamValue[0];
            Value_1 = m_ParamValue[1];
            Value_2 = m_ParamValue[2];
            Error   = SYS_READY;
        }
    }
    else
    {
        Error = SYS_OK_DENIED;               // Access to this command is forbidden in run mode
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdTEST3
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Example of a command
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdTEST3(void)
{
    SystemState_e            Error;
    char                     Response[64];
    static char              Test[CLI_STRING_SIZE];
    static int16_t           Test1;
    static uint16_t          Test2;
    static int32_t           Test3;

    if(m_IsItOnHold == true)
    {
        if(m_ReadCommand == true)
        {
            snprintf(Response, 64, "%d,%d,0x%04lX,\"%s\"", Test1, Test2, Test3, Test);
            SendAnswer(AT_TEST3,  SYS_OK_READ, Response);
            Error =  SYS_OK_SILENT;
        }
        else
        {
            Test1 = m_ParamValue[0];
            Test2 = m_ParamValue[1];
            Test3 = m_ParamValue[2];
            memcpy(&Test[0], m_pParamStr[3], CLI_STRING_SIZE);
            pMemory->Free(&m_pParamStr[3]);
            Error = SYS_READY;
        }
    }
    else
    {
        Error =  SYS_OK_DENIED;               // Access to this command is forbidden in run mode
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdINFO
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Return information
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdINFO(void)
{
    SystemState_e Error;
    char          Response[100];

    if(m_PlainCommand == true)
    {
        snprintf(Response, 100, "%s\r\n    %s\r\n    S/N: %s\r\n    Build: %s", OUR_FIRMWARE_NAME,
                                                                                OUR_MODEL_NAME,
                                                                                OUR_SERIAL_NUMBER,
                                                                                OUR_BUILD_DATE);
        SendAnswer(AT_INFO, SYS_OK_READ, Response);
        Error =  SYS_OK_SILENT;
    }
    else
    {
        Error = SYS_INVALID_PARAMETER;       // No parameter write or read on this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdVERSION
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Return the version
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdVERSION(void)
{
    SystemState_e Error;

    if(m_PlainCommand == true)
    {
        SendAnswer(AT_VERSION, SYS_OK_READ, OUR_FIRMWARE_VERSION);
        Error =  SYS_OK_SILENT;
    }
    else
    {
        Error = SYS_INVALID_PARAMETER;       // No parameter write or read on this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdDBG_LEVEL
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Set individual debug level (bit position)
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLineInterface::CmdDBG_LEVEL(void)
{
    SystemState_e       Error;
    char                Response[64];
    uint8_t             DebugLevel = (uint8_t)CLI_DebugLevel;

    if((m_ReadCommand == true) || (m_PlainCommand == true))     // Process also a plain command has a read
    {
        snprintf(&Response[0], 64, " 0x%02X ", DebugLevel);

        for(int i = 6; i < 14; i++)
        {
            if((DebugLevel & 0x80) == 0) Response[i] = '.';
            else                         Response[i] = '*';
            DebugLevel <<= 1;
        }

        snprintf(&Response[14], 64, "\r\n          01234567\r\n");
        SendAnswer(AT_DEBUG, SYS_OK_READ, Response);
        Error =  SYS_OK_SILENT;
    }
    else
    {
        m_DebugLevel = ParamValue[0];
        Error        = SYS_READY;
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------

