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

#include "console_cfg.h"

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
#define CON_SERIAL_OUT_SIZE             256
#define CON_TIME_DATE_STAMP             "%04u-%02u-%02u %2u:%02u:%02u: "
#define CON_NOT_CONNECTED               0

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

typedef void (*CON_ChildProcess_t)(void);

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class Console : public CallbackInterface
{
    public:

        void             Initialize                 (UART_Driver* pUartDriver);
        void             Process                    (void);
        void             GiveControlToChildProcess  (void(*pProcess)(void), CallbackInterface* pCallbackRX);
        void             ReleaseControl             (void);
        void             DisplayTimeDateStamp       (Date_t* pDate, Time_t* pTime);
        size_t           Printf                     (int nSize, const char* pFormat, ...);
        void             LockDisplay                (bool State);
        bool             GetString                  (char* pBuffer, size_t Size);
        bool             GetAtoi                    (int32_t* pValue, int32_t Min, int32_t Max, uint8_t Base);
        bool             IsItA_Comma                (void);
        bool             IsItAnEOL                  (void);
        size_t           PrintSerialLog             (CON_DebugLevel_e Level, const char* pFormat, ...);
        void             SetSerialLogging           (bool Mute);
        SystemState_e    SendData                   (const uint8_t* p_BufferTX, size_t* pSizeTX, void* pContext = nullptr);
        void             CallbackFunction           (int Type, void* pContext);
       // void           ProcessParams              (CLI_CmdName_e Command);

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
        CON_DebugLevel_e GetDebugLevel              (void)                                          { return m_DebugLevel;         }
        void             SetDebugLevel              (CON_DebugLevel_e DebugLevel)                   { m_DebugLevel = DebugLevel;   }

    private:


    // --------------------------------------------------------------------------------------------

        UART_Driver*                            m_pUartDriver;
        UART_Transfert_t*                       m_pRX_Transfert;
        int                                     m_ParserRX_Offset;
        TickCount_t                             m_CommandTimeOut;
        int16_t                                 m_CommandNameSize;
        //int16_t                                 m_DataSize;
        bool                                    m_MuteSerialLogging;
        TickCount_t                             m_StartupTick;
        bool                                    m_IsItOnStartup;
        char                                    m_BufferParserRX[CON_FIFO_PARSER_RX_SIZE];
        FIFO_Buffer                             m_Fifo;
        bool                                    m_IsItOnHold;
        CON_DebugLevel_e                        m_DebugLevel;
        uint8_t                                 m_ActiveProcessLevel;

      #if (CON_CHILD_PROCESS_PUSH_POP_LEVEL > 1)
        CON_ChildProcess_t                      m_ChildProcess[CON_CHILD_PROCESS_PUSH_POP_LEVEL];
        CallbackInterface*                      m_pCallbackRX[CON_CHILD_PROCESS_PUSH_POP_LEVEL];
      #else
        CON_ChildProcess_t                      m_ChildProcess;
        CallbackInterface*                      m_pCallbackRX;
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "console_var.h"        // Project variable

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_CONSOLE == DEF_ENABLED)
