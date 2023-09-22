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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CMD_LINE == DEF_ENABLED)

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdMUTE
//
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:    Command to put serial logging in mute mode
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdMUTE(void* pArg)
{
    SystemState_e Error;

    VAR_UNUSED(pArg);

    if(m_PlainCommand == true)
    {
        myConsole.SetSerialLogging(true);
        Error = SYS_READY;
    }
    else
    {
        Error = SYS_INVALID_PARAMETER;       // No parameter write or read on this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdUNMUTE
//
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:    Command to put serial logging in mute mode
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdUNMUTE(void* pArg)
{
    SystemState_e Error;

    VAR_UNUSED(pArg);

    if(m_PlainCommand == true)
    {
        myConsole.SetSerialLogging(false);
        Error = SYS_READY;
    }
    else
    {
        Error = SYS_INVALID_PARAMETER;       // No parameter write or read on this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdHOLD
//
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:    Command to put system on hold
//
//  Note(s):        This command for EXAMPLE can be use to put system on hold at boot up.
//                  allowing more command to be perform. command that are not avalaible at run
//                  time.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdHOLD(void* pArg)
{
    SystemState_e Error;

    VAR_UNUSED(pArg);

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
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:    Command to release hold on system
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdRELEASE(void* pArg)
{
    SystemState_e Error;

    VAR_UNUSED(pArg);

    if(m_IsItOnHold == true)
    {
        if(m_PlainCommand == true)
        {
            m_IsItOnHold = false;
            Error        = SYS_READY;
        }
        else
        {
            Error = SYS_INVALID_PARAMETER;       // No parameter write or read on this command
        }
    }
    else
    {
        Error = SYS_OK_DENIED;                   // Access to this command is forbidden in run mode
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdRESET
//
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:  	Command to reset and reboot the system
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdRESET(void* pArg)
{
    SystemState_e Error;

    VAR_UNUSED(pArg);

    if(m_PlainCommand == true)
    {
        if(m_ReadCommand != true)
        {
            //NVIC_SystemReset();
        }
    }
    else
    {
        Error = SYS_INVALID_PARAMETER;       // No parameter on this command
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdSTATUS
//
//  Parameter(s):   pArg                Not used
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
SystemState_e CommandLine::CmdSTATUS(void* pArg)
{
    SystemState_e Error;
    char          Response[20];
    int           Status;

    VAR_UNUSED(pArg);

    // Accept plain or read command
    if((m_ReadCommand == true) || (m_PlainCommand == true))
    {
         // In this example we only return the state of the CLI
        Status = (m_IsItOnHold == true) ? 0 : 1;
        snprintf(Response, 20, "%d", Status);
        SendAnswer(CLI_STATUS, SYS_OK_READ, Response);
        Error = SYS_OK_SILENT;
    }
    else
    {
        Error = SYS_CMD_NO_WRITE_SUPPORT;     // No write support for this command
    }

    return Error;
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdMENU
//
//  Parameter(s):   pArg                Call a child process. This is the pointer to interface.
//  Return:         SystemState_e
//
//  Description:    Access the VT100 Menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
SystemState_e CommandLine::CmdMENU(void* pArg)
{
    SystemState_e Error;

    if(m_PlainCommand == true)
    {
        m_pConsole->GiveControlToChildProcess((ChildProcessInterface*)pArg);
        m_IsItInitialize = false;                                               // when we come back.. it will reinitialize the CLI
        Error = SYS_OK_SILENT;
    }
    else
    {
        Error = SYS_CMD_PLAIN_ONLY;       // No parameter on this command
    }

    return Error;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CmdINFO
//
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:    Return information
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdINFO(void* pArg)
{
    SystemState_e Error;
    char          Response[100];

    VAR_UNUSED(pArg);

    if(m_PlainCommand == true)
    {
        snprintf(Response, 100, "%s\r    %s\r    S/N: %s\r    Build: %s", OUR_FIRMWARE_NAME,
                                                                          OEM_MODEL_NAME,
                                                                          OEM_SERIAL_NUMBER,
                                                                          OUR_BUILD_DATE);
        SendAnswer(CLI_INFO, SYS_OK_READ, Response);
        Error = SYS_OK_SILENT;
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
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:    Return the version
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdVERSION(void* pArg)
{
    SystemState_e Error;

    VAR_UNUSED(pArg);

    if(m_PlainCommand == true)
    {
        SendAnswer(CLI_VERSION, SYS_OK_READ, OUR_FIRMWARE_VERSION);
        Error = SYS_OK_SILENT;
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
//  Parameter(s):   pArg                Not used
//  Return:         SystemState_e
//
//  Description:    Set individual debug level (bit position)
//
//-------------------------------------------------------------------------------------------------
SystemState_e CommandLine::CmdDBG_LEVEL(void* pArg)
{
    SystemState_e Error;
    char          Response[64];
    uint16_t      DebugLevel = (uint16_t)m_pConsole->GetDebugLevel();

    VAR_UNUSED(pArg);

    if((m_ReadCommand == true) || (m_PlainCommand == true))     // Process also a plain command has a read
    {
        snprintf(&Response[0], 64, "0x%04X ", DebugLevel);

        for(int i = 8; i < 23; i++)
        {
            if((DebugLevel & 0x8000) == 0) Response[i] = '.';
            else                           Response[i] = '*';
            DebugLevel <<= 1;
        }

        snprintf(&Response[24], 64, "\n");
        SendAnswer(CLI_DEBUG, SYS_OK_READ, Response);
        Error = SYS_OK_SILENT;
    }
    else
    {
        m_pConsole->SetDebugLevel((CON_DebugLevel_e)m_ParamValue[0]);
        Error = SYS_READY;
    }

    return Error;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_CMD_LINE == DEF_ENABLED)
