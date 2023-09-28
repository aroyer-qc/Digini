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

#if (DIGINI_USE_CONSOLE == DEF_ENABLED)

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "console_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CON_FIFO_PARSER_RX_SIZE         64
#define CON_SIZE_NONE                   0
#define CON_SERIAL_OUT_SIZE             256
#define CON_TIME_DATE_STAMP             "%04u-%02u-%02u %2u:%02u:%02u: "
#define CON_NOT_CONNECTED               -1
#define CON_NUMBER_OF_DEBUG_LEVEL       16

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum CON_DebugLevel_e
{
    CON_DEBUG_NONE          = 0x0000,
    CON_DEBUG_LEVEL_1       = 0x0001,
    CON_DEBUG_LEVEL_2       = 0x0002,
    CON_DEBUG_LEVEL_3       = 0x0004,
    CON_DEBUG_LEVEL_4       = 0x0008,
    CON_DEBUG_LEVEL_5       = 0x0010,
    CON_DEBUG_LEVEL_6       = 0x0020,
    CON_DEBUG_LEVEL_7       = 0x0040,
    CON_DEBUG_LEVEL_8       = 0x0080,
    CON_DEBUG_LEVEL_9       = 0x0100,
    CON_DEBUG_LEVEL_10        = 0x0200,
    CON_DEBUG_LEVEL_11        = 0x0400,
    CON_DEBUG_LEVEL_12        = 0x0800,
    CON_DEBUG_LEVEL_13        = 0x1000,
    CON_DEBUG_LEVEL_14        = 0x2000,
    CON_DEBUG_LEVEL_15        = 0x4000,
    CON_DEBUG_LEVEL_16        = 0x8000,
};

//typedef void (*CON_ChildProcess_t)(void);

class ChildProcessInterface
{
    public:

        virtual void IF_Process             (void)                     = 0;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class Console : public CallbackInterface
{
    public:

        void             Initialize                 (UART_Driver* pUartDriver);
        void             Process                    (void);
        void             GiveControlToChildProcess  (ChildProcessInterface* pChildProcess);
        void             ReleaseControl             (void);
        void             DisplayTimeDateStamp       (Date_t* pDate, Time_t* pTime);
        size_t           Printf                     (int nSize, const char* pFormat, ...);
        void             LockDisplay                (bool State);
        bool             GetString                  (char* pBuffer, size_t Size);
        bool             GetAtoi                    (int32_t* pValue, int32_t Min, int32_t Max, uint8_t Base);
        bool             IsItA_Comma                (void);
        bool             IsItAnEOL                  (void);
        size_t           PrintSerialLog             (CON_DebugLevel_e Level, const char* pFormat, ...);
        size_t           PrintSerialLog             (CON_DebugLevel_e Level, const char* pFormat, va_list va);
        void             SetSerialLogging           (bool Mute);
        SystemState_e    SendData                   (const uint8_t* p_BufferTX, size_t* pSizeTX);
        void             CallbackFunction           (int Type, void* pContext);

        // Passthru FIFO
        inline void      TailForward                (size_t Size)                                   { m_Fifo.TailForward(Size);                   }
        inline void      HeadForward                (size_t Size)                                   { m_Fifo.HeadForward(Size);                   }
        inline void      HeadBackward               (size_t Size)                                   { m_Fifo.HeadBackward(Size);                  }
        inline size_t    Read                       (void* pBuffer, size_t BytesToRead)             { return m_Fifo.Read(pBuffer, BytesToRead);   }
        inline size_t    Write                      (const void *pBuffer, size_t BytesToWrite)      { return m_Fifo.Write(pBuffer, BytesToWrite); }
        inline int32_t   At                         (size_t Offset)                                 { return m_Fifo.At(Offset);                   }
        inline size_t    Flush                      (size_t BytesToFlush)                           { return m_Fifo.Flush(BytesToFlush);          }
        inline uint8_t   Atoi                       (int32_t* Value, uint8_t Base)                  { return m_Fifo.Atoi(Value, Base);            }
        inline uint8_t   AtoiAt                     (size_t Offset, int32_t* Value)                 { return m_Fifo.AtoiAt(Offset, Value);        }
        inline bool      Memncmp                    (const void* pMemPtr, size_t Length)            { return m_Fifo.Memncmp(pMemPtr, Length);     }
        inline size_t    Memnchr                    (char Character, size_t Length)                 { return m_Fifo.Memnchr(Character, Length);   }
        inline void      ToUpper                    (size_t Length)                                 { return m_Fifo.ToUpper(Length);              }
        inline bool      Move                       (FIFO_Buffer* pFifoDst, size_t Length)          { return m_Fifo.Move(pFifoDst, Length);       }
        inline bool      ReadyRead                  (void)                                          { return m_Fifo.ReadyRead();                  }
        inline bool      ReadyWrite                 (void)                                          { return m_Fifo.ReadyWrite();                 }
        inline size_t    CheckFreeSpace             (void)                                          { return m_Fifo.CheckFreeSpace();             }
        inline size_t    CheckUsedSpace             (void)                                          { return m_Fifo.CheckUsedSpace();             }

        // Getter/ Setter
        CON_DebugLevel_e GetDebugLevel              (void)                                          { return m_DebugLevel;       }
        void             SetDebugLevel              (CON_DebugLevel_e DebugLevel)                   { m_DebugLevel = DebugLevel; }

    private:


    // --------------------------------------------------------------------------------------------

    // need to add echo on or off for child process.. Console need it, but menu don't need it!
        UART_Driver*                            m_pUartDriver;
        //UART_Transfer_t**                       m_pRX_Transfer;
        TickCount_t                             m_CommandTimeOut;
        int16_t                                 m_CommandNameSize;
        bool                                    m_MuteSerialLogging;
        TickCount_t                             m_StartupTick;
        bool                                    m_IsItOnStartup;
        char                                    m_BufferParserRX[CON_FIFO_PARSER_RX_SIZE];
        FIFO_Buffer                             m_Fifo;
        bool                                    m_IsItOnHold;
        CON_DebugLevel_e                        m_DebugLevel;
        int32_t                                 m_ActiveProcessLevel;
        ChildProcessInterface*                  m_pChildProcess[CON_CHILD_PROCESS_PUSH_POP_LEVEL];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef CONSOLE_GLOBAL

class Console myConsole;

#else

extern class Console myConsole;

#endif // CONSOLE_GLOBAL

#define DEBUG_PrintSerialLog           myConsole.PrintSerialLog

//-------------------------------------------------------------------------------------------------

#else // (DIGINI_USE_CONSOLE == DEF_ENABLED)

#define DEBUG_PrintSerialLog(...)       // Prevent wrapping all log call with preprocessor

#endif // (DIGINI_USE_CONSOLE == DEF_ENABLED)
