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

#include "./Digini/Digini/inc/lib_typedef.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define FIFO_INDEX_OUT_OF_BOUNDS            -1
#define FIFO_CHAR_NOT_FOUND                 -2

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class FIFO_Buffer
{
    public:
                        FIFO_Buffer             ();
                       ~FIFO_Buffer             ();

        void            Initialize              (size_t Size);
        int32_t         At                      (size_t Offset);
        size_t          Flush                   (size_t BytesToFlush);
        size_t          Atoi                    (int32_t* Value, uint8_t Base);
        size_t          AtoiAt                  (size_t Offset, int32_t* Value);
        bool            Memncmp                 (const void* pMemPtr, size_t Length);
        size_t          Memnchr                 (char Character, size_t Length);
        void            ToUpper                 (size_t Length);
        bool            Move                    (FIFO_Buffer* pFifoDst, size_t Length);

        //bool            Read                    (uint8_t* pData);
        size_t          Read                    (void* pBuffer, size_t BytesToRead);
        //bool            Write                   (uint8_t Data);
        size_t          Write                   (const void *pBuffer, size_t BytesToWrite);

        bool            ReadyRead               (void);
        bool            ReadyWrite              (void);

        size_t          CheckFreeSpace          (void);
        size_t          CheckUsedSpace          (void);

        void            TailForward             (size_t Size);
        void            HeadForward             (size_t Size);
        void            HeadBackward            (size_t Size);

    private:

        volatile size_t                         m_PushIndex;
        volatile size_t                         m_PopIndex;
        volatile uint8_t*                       m_pBuffer;
        volatile size_t                         m_Size;
};

//-------------------------------------------------------------------------------------------------
