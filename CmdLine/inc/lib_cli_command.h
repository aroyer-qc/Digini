//-------------------------------------------------------------------------------------------------
//
//  File : lib_cli_command.h
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_cli.h"
#include "lib_cli_expanding_macro.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#ifdef CLI_GLOBAL
    #define CLI_EXTERN
#else
    #define CLI_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// typedef(s)
//-------------------------------------------------------------------------------------------------

enum CLI_StrCmdSize_e
{
    #if CLI_USE_VT100_MENU == DEF_ENABLED
      SIZE_OF_AT_MENU = sizeof("MENU") - 1,
    #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_SIZE_OF)                // Create the sizeof() for each string

};

SystemState_e (*CLI_Function_t)(void);


//-------------------------------------------------------------------------------------------------
// Function(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_FUNCTION)               // Generation of all prototype

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

#ifdef CLI_GLOBAL

  #if CLI_USE_VT100_MENU == DEF_ENABLED
    const char StrAT_MENU[SIZE_OF_AT_MENU] = "MENU";
  #endif

    X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_CONST_STRING)           // Generation of all the string


class CommandLineInterface
{
    const CLI_Function_t m_Function[NUMBER_OF_CLI_CMD] =
    {
      #if CLI_USE_VT100_MENU == DEF_ENABLED
        CLI_CmdMENU,
      #endif

        X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_FUNCTION_POINTER)   // Generation of the function pointer array
    };

    const char* m_pCmdStr[NUMBER_OF_CLI_CMD] =
    {
      #if CLI_USE_VT100_MENU == DEF_ENABLED
        &StrAT_MENU[0],
      #endif

        X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_CMD_STRING)
    };

    const int m_CmdStrSize[NUMBER_OF_CLI_CMD] =
    {
      #if CLI_USE_VT100_MENU == DEF_ENABLED
        sizeof("MENU") - 1,
      #endif

        X_CLI_CMD_DEF(EXPAND_CLI_CMD_AS_STRING_SIZE)
    };
};

#else

#endif

//-------------------------------------------------------------------------------------------------

