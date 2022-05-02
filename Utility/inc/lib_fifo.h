//-------------------------------------------------------------------------------------------------
//
//  File : lib_fifo.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

#include "lib_typedef.h"

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class FIFO_Buffer
{
    public:
                        FIFO_Buffer             ();
                        FIFO_Buffer             (uint16_t Size);
                        ~FIFO_Buffer            ();
        void            Initialize              (uint16_t Size);



        int32_t         At                      (uint32_t Offset);
        uint32_t        Flush                   (uint32_t BytesToFlush);
        uint8_t         Atoi                    (int32_t* Value, uint8_t Base);
        uint8_t         AtoiAt                  (uint32_t Offset, int32_t* Value);
        bool            Memncmp                 (const void* pMemPtr, uint32_t Length);
        int32_t         Memnchr                 (char Character, uint32_t Length);
        void            ToUpper                 (uint32_t Length);
        bool            Move                    (FIFO_Buffer* pFifoDst, uint32_t Length);

//        bool            Pop                     (uint8_t* pData);
        uint32_t        Pop                     (void* pBuffer, uint32_t BytesToRead);
//        bool            Push                    (uint8_t Data);
        uint32_t        Push                    (const void *pBuffer, uint32_t BytesToWrite);

        bool            ReadyRead               (void);
        bool            ReadyWrite              (void);

        uint32_t        CheckFreeSpace          (void);
        uint32_t        CheckUsedSpace          (void);

        void            PopForward              (uint32_t Size);
        void            PushForward             (uint32_t Size);
        void            PushBackward            (uint32_t Size);

    private:

        volatile uint16_t                       m_PushIndex;
        volatile uint16_t                       m_PopIndex;
        volatile uint8_t*                       m_pBuffer;
        volatile uint16_t                       m_Size;


        void            IndexIncrement          (uint16_t *p, uint16_t *msb);

};

//-------------------------------------------------------------------------------------------------
