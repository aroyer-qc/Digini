//-------------------------------------------------------------------------------------------------
//
//  File : lib_cli.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CMD_LINE == DEF_ENABLED)

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "cli_cfg.h"
#include "lib_cli_expanding_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CMD_MENU            "MENU"

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum CLI_CmdName_e
{
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    AT_MENU,
  #endif

	X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_ENUM)
    NUMBER_OF_CLI_CMD,
    CLI_NO_CMD,
    FIRST_CLI_COMMAND = 0,
};

enum CLI_CommandSupport_e
{
    // Do not used the following 2 value for command support (only for internal processing)
    CLI_CMD_S      = 0x01,                                          // Mask for Startup command only in CLI_CommandSupport_e typedef
    CLI_CMD_H      = 0x02,                                          // Mask for Hold command only in CLI_CommandSupport_e typedef

    CLI_CMD_P      = 0x04,                                          // PLAIN only   ( ATV\r\n is a plain command)
    CLI_CMD_R      = 0x08,                                          // READ cmd only
    CLI_CMD_W      = 0x10,                                          // WRITE cmd only
    CLI_CMD_PR     = CLI_CMD_P | CLI_CMD_R,                         // PLAIN and READ cmd
    CLI_CMD_PW     = CLI_CMD_P | CLI_CMD_W,                         // PLAIN and WRITE cmd
    CLI_CMD_RW     = CLI_CMD_R | CLI_CMD_W,                         // READ and WRITE cmd
    CLI_CMD_PRW    = CLI_CMD_P | CLI_CMD_R | CLI_CMD_W,             // PLAIN, READ and WRITE cmd
    CLI_CMD_SP     = CLI_CMD_S | CLI_CMD_P,                         // PLAIN and only at startup  ( ATH\r\n is a plain command)
    CLI_CMD_SR     = CLI_CMD_S | CLI_CMD_R,                         // READ cmd and only at startup
    CLI_CMD_SW     = CLI_CMD_S | CLI_CMD_W,                         // WRITE cmd and only at startup
    CLI_CMD_SPR    = CLI_CMD_S | CLI_CMD_PR,                        // PLAIN and READ cmd and only at startup
    CLI_CMD_SPW    = CLI_CMD_S | CLI_CMD_PW,                        // PLAIN and WRITE cmd and only at startup
    CLI_CMD_SRW    = CLI_CMD_S | CLI_CMD_RW,                        // READ and WRITE cmd and only at startup
    CLI_CMD_SPRW   = CLI_CMD_S | CLI_CMD_PRW,                       // PLAIN, READ and WRITE cmd and only at startup

    // The command describe with the following mode can only be sent when on HOLD mode
    CLI_CMD_HP     = CLI_CMD_H | CLI_CMD_P,                         // PLAIN only
    CLI_CMD_HR     = CLI_CMD_H | CLI_CMD_R,                         // READ cmd only
    CLI_CMD_HW     = CLI_CMD_H | CLI_CMD_W,                         // WRITE cmd only
    CLI_CMD_HPR    = CLI_CMD_H | CLI_CMD_P | CLI_CMD_R,             // PLAIN and READ cmd
    CLI_CMD_HPW    = CLI_CMD_H | CLI_CMD_P | CLI_CMD_W,             // PLAIN and WRITE cmd
    CLI_CMD_HRW    = CLI_CMD_H | CLI_CMD_R | CLI_CMD_W,             // READ and WRITE cmd
    CLI_CMD_HPRW   = CLI_CMD_H | CLI_CMD_P | CLI_CMD_R | CLI_CMD_W, // PLAIN, READ and WRITE cmd
    CLI_CMD_SHP    = CLI_CMD_H | CLI_CMD_S | CLI_CMD_P,             // PLAIN and only at startup  ( ATH\r\n is a plain command)
    CLI_CMD_SHR    = CLI_CMD_H | CLI_CMD_S | CLI_CMD_R,             // READ cmd and only at startup
    CLI_CMD_SHW    = CLI_CMD_H | CLI_CMD_S | CLI_CMD_W,             // WRITE cmd and only at startup
    CLI_CMD_SHPR   = CLI_CMD_H | CLI_CMD_S | CLI_CMD_PR,            // PLAIN and READ cmd and only at startup
    CLI_CMD_SHPW   = CLI_CMD_H | CLI_CMD_S | CLI_CMD_PW,            // PLAIN and WRITE cmd and only at startup
    CLI_CMD_SHRW   = CLI_CMD_H | CLI_CMD_S | CLI_CMD_RW,            // READ and WRITE cmd and only at startup
    CLI_CMD_SHPRW  = CLI_CMD_H | CLI_CMD_S | CLI_CMD_PRW,           // PLAIN, READ and WRITE cmd and only at startup
};

struct CLI_CmdParam_t
{
    uint8_t  Base;
    int32_t  Min;
    int32_t  Max;
};

struct CLI_CmdInputInfo_t
{
    CLI_CommandSupport_e    Support;
    uint8_t                 NumberOfParam;
    CLI_CmdParam_t          Param[CLI_NUMBER_OF_SUPPORTED_PARAM];
};

enum CLI_InputState_e
{
    CLI_CMD_LINE,
    CLI_USER_FUNCTION,
};

enum CLI_Step_e
{
    CLI_STEP_IDLE                 = 0,
    CLI_STEP_WAITING_FOR_A        = CLI_STEP_IDLE,
    CLI_STEP_WAITING_FOR_T        = 1,
    CLI_STEP_GETTING_DATA         = 2,
    CLI_STEP_CMD_VALID            = 3,
    CLI_STEP_CMD_MALFORMED        = 4,
    CLI_STEP_CMD_BUFFER_OVERFLOW  = 5,
};

enum CLI_StrCmdSize_e
{
    #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
      SZ_OF_AT_MENU = sizeof(CMD_MENU) - 1,
    #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_SIZE_OF)                // Create the sizeof() for each string

};

typedef void (*CLI_ChildProcess_t)(uint8_t Data);

//-------------------------------------------------------------------------------------------------
// Function(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

class CommandLine : public ChildProcessInterface
{
    public:

        void            IF_Process                  (void);
        //void            IF_CallbackFunction         (int Type, void* pContext);

        void            Initialize                  (Console* pConsole);

        //void            GiveControlToChildProcess   (void(*pProcess)(uint8_t Data));
        void            ReleaseControl              (void);
        void            LockDisplay                 (bool State);
        void            SendAnswer                  (CLI_CmdName_e CmdName, SystemState_e State, const char* Answer);

        // For child process to send data
        SystemState_e   SendData                    (const uint8_t* p_BufferTX, size_t* pSizeTX, void* pContext = nullptr);


      #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
        SystemState_e   CmdMENU                     (void);
      #endif

    // --------------------------------------------------------------------------------------------
    // Expansion of all user CLI function

        X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_FUNCTION)   // Generation of all prototype

    private:

      #if (CLI_USE_PASSWORD == DEF_ENABLED)
        SystemState_e   CLI_HandleCmdPassword       (void);
      #endif

        bool            ProcessRX                   (void);
        //void            RX_Callback                 (uint8_t Data);
        void            ProcessParams               (CLI_CmdName_e Command);

    // --------------------------------------------------------------------------------------------

        Console*                                m_pConsole;
        CLI_InputState_e                        m_InputState;
        int                                     m_ParserRX_Offset;
        CLI_Step_e                              m_Step;
        ChildProcessInterface*                  m_pChildProcess;
        TickCount_t                             m_CommandTimeOut;
        int16_t                                 m_CommandNameSize;
        bool                                    m_MuteSerialLogging;
        TickCount_t                             m_StartupTick;
        bool                                    m_IsItOnStartup;
        bool                                    m_IsItOnHold;
        bool                                    m_ReadCommand;
        bool                                    m_PlainCommand;
        int32_t                                 m_ParamValue[CLI_NUMBER_OF_SUPPORTED_PARAM];
        char*                                   m_pParamStr[CLI_NUMBER_OF_SUPPORTED_PARAM];

        static const CLI_CmdInputInfo_t         m_CmdInputInfo[NUMBER_OF_CLI_CMD];
        static const char*                      m_ErrorLabel;
        static const char*                      m_pCmdStr[NUMBER_OF_CLI_CMD];
        static const int                        m_CmdStrSize[NUMBER_OF_CLI_CMD];

      #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
        static const char                       m_StrAT_MENU[sizeof(CMD_MENU)];
      #endif

        X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_CLASS_CONST_STRING)           // Generation of all the string

      #if (CLI_USE_PASSWORD == DEF_ENABLED)
        static char                             m_CMD_Password[CLI_PASSWORD_SIZE];
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "cli_var.h"        // Project variable

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_CMD_LINE == DEF_ENABLED)
