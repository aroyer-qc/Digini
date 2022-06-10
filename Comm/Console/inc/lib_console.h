//-------------------------------------------------------------------------------------------------
//
//  File : lib_console.h
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

//#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CONSOLE == DEF_ENABLED)

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CON_FIFO_PARSER_RX_SIZE         64
#define CON_SIZE_NONE                   0

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum CON_DebugLevel_e
{
    CON_DEBUG_LEVEL_0    = 0x00,
    CON_DEBUG_LEVEL_1    = 0x01,
    CON_DEBUG_LEVEL_2    = 0x02,
    CON_DEBUG_LEVEL_3    = 0x04,
    CON_DEBUG_LEVEL_4    = 0x08,
    CON_DEBUG_LEVEL_5    = 0x10,
    CON_DEBUG_LEVEL_6    = 0x20,
    CON_DEBUG_LEVEL_7    = 0x40,
    CON_DEBUG_LEVEL_8    = 0x80,
};

typedef void (*CON_ChildProcess_t)(uint8_t Data);

//-------------------------------------------------------------------------------------------------
// Function(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

class Console : public CallbackInterface
{
    public:

        void            Initialize                  (UART_Driver* pUartDriver);
        void            Process                     (void);
        void            GiveControlToChildProcess   (void(*pProcess)(uint8_t Data));
        void            ReleaseControl              (void);
        void            DisplayTimeDateStamp        (Date_t* pDate, Time_t* pTime);
        size_t          Printf                      (int nSize, const char* pFormat, ...);
        void            LockDisplay                 (bool State);
        bool            GetString                   (char* pBuffer, size_t Size);
        bool            GetAtoi                     (int32_t* pValue, int32_t Min, int32_t Max, uint8_t Base);
        bool            IsItA_Comma                 (void);
        bool            IsItAnEOL                   (void);
        size_t          PrintSerialLog              (CLI_DebugLevel_e Level, const char* pFormat, ...);
        void            SetSerialLogging            (bool Mute);
        SystemState_e   SendData                    (const uint8_t* p_BufferTX, size_t* pSizeTX, void* pContext = nullptr);
        void            CallbackFunction            (int Type, void* pContext);
        void            RX_Callback                 (uint8_t Data);
        void            ProcessParams               (CLI_CmdName_e Command);

    // --------------------------------------------------------------------------------------------

        UART_Driver*                            m_pUartDriver;
        int                                     m_ParserRX_Offset;
        CON_ChildProcess_t                      m_ChildProcess;
        TickCount_t                             m_CommandTimeOut;
        int16_t                                 m_CommandNameSize;
        //int16_t                                 m_DataSize;
        bool                                    m_MuteSerialLogging;
        TickCount_t                             m_StartupTick;
        bool                                    m_IsItOnStartup;
        char                                    m_BufferParserRX[CLI_FIFO_PARSER_RX_SIZE];
        FIFO_Buffer*                            m_pFifo;
        bool                                    m_IsItOnHold;
        CON_DebugLevel_e                        m_DebugLevel;
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_CONSOLE == DEF_ENABLED)
