//-------------------------------------------------------------------------------------------------
//
//  File : lib_cli_expand_macro.h
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
// Macro(s)
//-------------------------------------------------------------------------------------------------

#if (CLI_NUMBER_OF_SUPPORTED_PARAM == 1)
#define EXPAND_CLI_CMD_AS_ENUM(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)                  NAME,
#define EXPAND_CLI_CMD_AS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)          const char CommandLine::m_Str##NAME[] = STRING;
#define EXPAND_CLI_CMD_AS_CLASS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)    static const char m_Str##NAME[];
#define EXPAND_CLI_CMD_AS_CMD_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)            &m_Str##NAME[0],
#define EXPAND_CLI_CMD_AS_STRING_SIZE(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)           sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_FUNCTION(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)              SystemState_e FUNCTION(void*);
#define EXPAND_CLI_CMD_AS_FUNCTION_POINTER(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)      &CommandLine::FUNCTION,
#define EXPAND_CLI_CMD_AS_SIZE_OF(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX)               SZ_OF_##NAME  = sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_INPUT_INFO(NAME, STRING, FUNCTION, SUPPORT, NUMBER,   P1_TYPE, P1_MIN, P1_MAX) \
                                                            {SUPPORT, NUMBER, {{P1_TYPE, P1_MIN, P1_MAX}} },
#endif

#if (CLI_NUMBER_OF_SUPPORTED_PARAM == 2)
#define EXPAND_CLI_CMD_AS_ENUM(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)                  NAME,
#define EXPAND_CLI_CMD_AS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)          const char CommandLine::m_Str##NAME[] = STRING;
#define EXPAND_CLI_CMD_AS_CLASS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)    static const char m_Str##NAME[];
#define EXPAND_CLI_CMD_AS_CMD_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)            &m_Str##NAME[0],
#define EXPAND_CLI_CMD_AS_STRING_SIZE(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)           sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_FUNCTION(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)              SystemState_e FUNCTION(void*);
#define EXPAND_CLI_CMD_AS_FUNCTION_POINTER(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)      &CommandLine::FUNCTION,
#define EXPAND_CLI_CMD_AS_SIZE_OF(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX)               SZ_OF_##NAME  = sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_INPUT_INFO(NAME, STRING, FUNCTION, SUPPORT, NUMBER,   P1_TYPE, P1_MIN, P1_MAX,   P2_TYPE, P2_MIN, P2_MAX) \
                                                            {SUPPORT, NUMBER, {{P1_TYPE, P1_MIN, P1_MAX}, {P2_TYPE, P2_MIN, P2_MAX}} },
#endif

#if (CLI_NUMBER_OF_SUPPORTED_PARAM == 3)
#define EXPAND_CLI_CMD_AS_ENUM(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)                  NAME,
#define EXPAND_CLI_CMD_AS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)          const char CommandLine::m_Str##NAME[] = STRING;
#define EXPAND_CLI_CMD_AS_CLASS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)    static const char m_Str##NAME[];
#define EXPAND_CLI_CMD_AS_CMD_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)            &m_Str##NAME[0],
#define EXPAND_CLI_CMD_AS_STRING_SIZE(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)           sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_FUNCTION(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)              SystemState_e FUNCTION(void*);
#define EXPAND_CLI_CMD_AS_FUNCTION_POINTER(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)      &CommandLine::FUNCTION,
#define EXPAND_CLI_CMD_AS_SIZE_OF(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX)               SZ_OF_##NAME  = sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_INPUT_INFO(NAME, STRING, FUNCTION, SUPPORT, NUMBER,   P1_TYPE, P1_MIN, P1_MAX,   P2_TYPE, P2_MIN, P2_MAX,   P3_TYPE, P3_MIN, P3_MAX) \
                                                            {SUPPORT, NUMBER, {{P1_TYPE, P1_MIN, P1_MAX}, {P2_TYPE, P2_MIN, P2_MAX}, {P3_TYPE, P3_MIN, P3_MAX}} },
#endif

#if (CLI_NUMBER_OF_SUPPORTED_PARAM == 4)
#define EXPAND_CLI_CMD_AS_ENUM(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)                  NAME,
#define EXPAND_CLI_CMD_AS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)          const char CommandLine::m_Str##NAME[] = STRING;
#define EXPAND_CLI_CMD_AS_CLASS_CONST_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)    static const char m_Str##NAME[];
#define EXPAND_CLI_CMD_AS_CMD_STRING(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)            &m_Str##NAME[0],
#define EXPAND_CLI_CMD_AS_STRING_SIZE(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)           sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_FUNCTION(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)              SystemState_e FUNCTION(void*);
#define EXPAND_CLI_CMD_AS_FUNCTION_POINTER(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)      &CommandLine::FUNCTION,
#define EXPAND_CLI_CMD_AS_SIZE_OF(NAME, STRING, FUNCTION, SUPPORT, NUMBER, P1_TYPE, P1_MIN, P1_MAX, P2_TYPE, P2_MIN, P2_MAX, P3_TYPE, P3_MIN, P3_MAX, P4_TYPE, P4_MIN, P4_MAX)               SZ_OF_##NAME  = sizeof(STRING) - 1,
#define EXPAND_CLI_CMD_AS_INPUT_INFO(NAME, STRING, FUNCTION, SUPPORT, NUMBER,  P1_TYPE, P1_MIN, P1_MAX,   P2_TYPE, P2_MIN, P2_MAX,   P3_TYPE, P3_MIN, P3_MAX,   P4_TYPE, P4_MIN, P4_MAX) \
                                                            {SUPPORT, NUMBER, {{P1_TYPE, P1_MIN, P1_MAX}, {P2_TYPE, P2_MIN, P2_MAX}, {P3_TYPE, P3_MIN, P3_MAX}, {P4_TYPE, P4_MIN, P4_MAX}} },
#endif



//-------------------------------------------------------------------------------------------------
