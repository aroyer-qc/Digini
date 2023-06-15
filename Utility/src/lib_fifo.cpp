//-------------------------------------------------------------------------------------------------
//
//  File : lib_fifo.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
//
//   Class: FIFO_Buffer
//
//   Description:   Class to handle FIFO_Buffer
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   FIFO_Buffer
//
//   Parameter(s):  None
//
//   Description:   Initializes the fifo
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
FIFO_Buffer::FIFO_Buffer()
{
     m_Size      = 0;
     m_pBuffer   = nullptr;
     m_PushIndex = 0;
     m_PopIndex  = 0;
}

//-------------------------------------------------------------------------------------------------
//
//   Destructor:   ~FIFO_Buffer
//
//   Parameter(s):  None
//
//   Description:   Initializes the fifo
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
FIFO_Buffer::~FIFO_Buffer()
{
    if(m_pBuffer != nullptr)
    {
        pMemoryPool->Free((void**)&m_pBuffer);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   Size            Size of the buffer to get form memory pool
//
//  Return:         None
//
//  Description:    Initialize the FIFO
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void FIFO_Buffer::Initialize(size_t Size)
{
    m_Size      = Size;
    m_pBuffer   = (uint8_t*)pMemoryPool->AllocAndClear(Size);
    m_PushIndex = 0;
    m_PopIndex  = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Read
//
//  Parameter(s):   pBuffer         Buffer where to copy data from FIFO.
//                  BytesToRead     How many bytes to read.
//
//  Return:         size_t          Number of bytes read from the FIFO.
//
//  Description:    Reads a number of bytes from the FIFO
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::Read(void* pBuffer, size_t BytesToRead)
{
    size_t      i;
    uint8_t*    pBuf;

    i = 0;
    pBuf = (uint8_t*)pBuffer;

    if(m_pBuffer != nullptr)
    {
        for(; i < BytesToRead; i++)
        {
            if(ReadyRead() == false)                    // See if any data is available
            {
                return i;                               // Number of bytes read
            }
            else
            {
                *pBuf++ = m_pBuffer[m_PopIndex];        // Grab a byte from the buffer
                m_PopIndex++;                           // Increment the OUT pointer

                if(m_PopIndex == m_Size)
                {
                    m_PopIndex = 0;                     // Check for wrap-around
                }
            }
        }
    }

    return i;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   pBuffer         Buffer where to get the data for the FIFO.
//                  BytesToWrite    How many bytes to write.
//
//  Return:         size_t          Number of bytes written to the FIFO.
//
//  Description:    Number of bytes written to the FIFO
//
//  Note(s):        If the head runs in to the tail, not all bytes are written
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::Write(const void* pBuffer, size_t BytesToWrite)
{
    size_t      i;
    uint8_t*    pBuf;

    i = 0;
    pBuf = (uint8_t*)pBuffer;

    if(m_pBuffer != nullptr)
    {
        for(; i < BytesToWrite; i++)
        {
            // First check to see if there is space in the buffer
            if(ReadyWrite() == false)
            {
                return i;                                           // No more room
            }
            else
            {
                m_pBuffer[m_PushIndex] = *pBuf;
                pBuf++;
                m_PushIndex++;                                      // Increment the IN pointer

                if(m_PushIndex == m_Size)
                {
                    m_PushIndex = 0;                                // Check for wrap-around
                }
            }
        }
    }

    return i;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadyRead
//
//  Parameter(s):   None
//
//  Return:         bool            true  - Data is available to read.
//                                  false - No Data.
//
//  Description:    Is there data to read from the FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::ReadyRead(void)
{
    return (m_PopIndex != m_PushIndex);      // See if any data is available
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadyWrite
//
//  Parameter(s):   None
//
//  Return:         bool            true  - FIFO has room for at least 1 byte.
//                                  false - No more room.
//
//  Description:    Check if FIFO is ready to receive data.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::ReadyWrite(void)
{
    if(((m_PushIndex + 1) == m_PopIndex) || (((m_PushIndex + 1) == m_Size) && (m_PopIndex == 0)))
    {
        return false;  // No more room
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CheckFreeSpace
//
//  Parameter(s):   None
//
//  Return:         size_t        Remaining space in the FIFO.
//
//  Description:    Return the free space available in the FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::CheckFreeSpace(void)
{
    size_t FreeSize;

    if(m_PopIndex <= m_PushIndex)
    {
        FreeSize = ((m_PopIndex + m_Size) - m_PushIndex);
    }
    else
    {
        FreeSize = m_PopIndex - m_PushIndex;
    }

    return FreeSize;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CheckUsedSpace
//
//  Parameter(s):   None
//
//  Return:         size_t        Used space in the FIFO.
//
//  Description:    Return the used space in the FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::CheckUsedSpace(void)
{
    size_t Size;

    if(m_PopIndex <= m_PushIndex)
    {
        Size = m_PushIndex - m_PopIndex;
    }
    else
    {
        Size = ((m_Size - m_PopIndex) + m_PushIndex);
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Move
//
//  Parameter(s):   pFifoDst    Pointer on the destination FIFO_Buffer class.
//                  BytesToCopy Number of data to transfer.
//
//  Return:         If FIFO is ready to written, it return true, otherwise false.
//
//  Description:    Move a number of bytes from one FIFO to another.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::Move(FIFO_Buffer* pFifoDst, size_t BytesToCopy)
{
    uint8_t Data;

    if(pFifoDst->CheckFreeSpace() < BytesToCopy)
    {
        return false;
    }

    // Do we check for readiness of source FIFO... not sure

    while(BytesToCopy)
    {
        if(Read(&Data, 1) != 1)
        {
            return false;
        }

        if(pFifoDst->Write(&Data, 1) != 1)
        {
            return false;
        }

        BytesToCopy--;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           At
//
//  Parameter(s):   Offset  Offset in the FIFO where to read data.
//
//  Return:         int32_t FIFO_INDEX_OUT_OF_BOUNDS if the offset is beyond 'head' or the byte at
//                          position 'Offset'.
//
//  Description:    Retrieve a byte from FIFO without removing the element.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int32_t FIFO_Buffer::At(size_t Offset)
{
    size_t UsedSize;

    UsedSize = m_Size - CheckFreeSpace();

    if(Offset >= UsedSize)
    {
        return FIFO_INDEX_OUT_OF_BOUNDS;
    }

    Offset += m_PopIndex;       // Add Tail offset to offset we want to read

    if(Offset >= m_Size)        // If our offset is farther than the size, we have a wrap-around
    {
        Offset -= m_Size;
    }

    return m_pBuffer[Offset];
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Flush
//
//  Parameter(s):   BytesToFlush    Number of bytes to flush.
//
//  Return:         size_t          Number of bytes actually flush from buffer.
//
//  Description:    Flush a number of bytes from the FIFO or less if not enough bytes to flush.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::Flush(size_t BytesToFlush)
{
    size_t FlushSize;
    size_t TmpHead;

    // For this check make sure Head is actually ahead of tail
    TmpHead = m_PushIndex;

    if(TmpHead < m_PopIndex)
    {
        TmpHead += m_Size;
    }

    if((m_PopIndex + BytesToFlush) <= TmpHead)     // Check if enough byte in buffer to flush
    {
        m_PopIndex += BytesToFlush;

        if(m_PopIndex >= m_Size)
        {
            m_PopIndex -= m_Size;
        }

        FlushSize = BytesToFlush;
    }
    else                                            // If not, only flush what we can
    {
        FlushSize  = TmpHead - m_PopIndex;
        m_PopIndex = m_PushIndex;
    }

    return FlushSize;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Atoi
//
//  Parameter(s):   pResult     Pointer on the int to put the value.
//                  Base        Extract data according to base, support decimal or hexadecimal.
//
//  Return:         size_t      Number of char extracted.
//
//  Description:    Perform atoi() functionality directly on FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::Atoi(int32_t* pResult, uint8_t Base)
{
    int32_t nData;
    uint8_t Data;
    size_t  Count    = 0;
    bool    Negative = false;

    *pResult = 0;                           // Initialize result

    // Iterate through all characters of input string and update result
    for(;;)
    {
        nData = At(0);

        if(nData == FIFO_INDEX_OUT_OF_BOUNDS)
        {
            break;
        }

        Data = (uint8_t)nData;

        if(Base == DEF_DECIMAL_BASE)
        {
            if(((Data < '0') || (Data > '9')) && ((Count != 0) || (Data != '-')))
            {
                break;
            }

            Read(&Data, 1);

            if(Data == '-')
            {
                Negative = true;
            }
            else
            {
                *pResult = (*pResult * DEF_DECIMAL_BASE) + (int)(Data - '0');
            }
        }
        else // HEXADECIMAL_BASE
        {
            if(((Data < '0') || (Data > '9')) &&
               ((Data < 'a') || (Data > 'f')) &&
               ((Data < 'A') || (Data > 'F')) &&
                (Count != 0))
            {
                break;
            }

            Read(&Data, 1);
            *pResult  = (*pResult * DEF_HEXADECIMAL_BASE);

            if     (Data <= '9')   Data =  Data - '0';
            else if(Data <= 'F')   Data = (Data - 'A') + 10;
            else /*(Data <= 'f')*/ Data = (Data - 'a') + 10;

            *pResult += Data;
        }

        Count++;
    }

    if(Negative == true)
    {
        *pResult = 0 - *pResult;
    }


    return Count;                       // Return result.
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AtoiAt
//
//  Parameter(s):   Offset      Offset in the FIFO where to read data.
//                  pResult     Pointer on the int to put the value.
//
//  Return:         size_t      Number of char extracted.
//
//  Description:    Perform atoi() functionality directly on FIFO but with added offset
//                  functionality without extracting characters are not extracted from FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::AtoiAt(size_t Offset, int32_t* pResult)
{
    int32_t nData;
    uint8_t Data;
    size_t  Count    = 0;
    bool    Negative = false;

    *pResult = 0;                       // Initialize result

    // Iterate through all characters of input string and update result
    for(;;)
    {
        nData = At(Offset++);

        if(nData == FIFO_INDEX_OUT_OF_BOUNDS)
        {
            break;
        }

        Data = (uint8_t)nData;

        if((Data > '9' || Data < '0') && (Count != 0 || Data != '-'))
        {
            break;
        }

        if(Data == '-')
        {
            Negative = true;
        }
        else
        {
            *pResult = (*pResult * 10) + (int)(Data - '0');
        }
        Count++;
    }

    if(Negative == true)
    {
        *pResult = 0 - *pResult;
    }

    return Count;                       // Return result.
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Memncmp
//
//  Parameter(s):   pMemPtr     Pointer on the memory buffer.
//                  Length      How many byte to compare.
//
//  Return:         bool        true or false.
//
//  Description:    Compare a number of bytes from the FIFO with buffer.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::Memncmp(const void* pMemPtr, size_t Length)
{
    size_t Count;
    uint8_t* pPtr;

    pPtr = (uint8_t*)pMemPtr;

    for(Count = 0; Count < Length; Count++)
    {
        if(At(Count) != (int16_t)*pPtr)
        {
            return false;
        }

         pPtr++;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Memnchr
//
//  Parameter(s):   Character   Character we want to check in the FIFO.
//                  Length      Up to how many byte to compare.
//
//  Return:         int32_t    -1 if character is not found, or the offset to the character.
//
//  Description:    Search for a character up to N bytes from the FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t FIFO_Buffer::Memnchr(char Character, size_t Length)
{
    size_t Count;

    for(Count = 0; Count < Length; Count++)
    {
        if(At(Count) == Character)
        {
            return Count;
        }
    }
    return FIFO_CHAR_NOT_FOUND;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ToUpper
//
//  Parameter(s):   Length      How many byte to convert.
//
//  Return:         bool        true or false.
//
//  Description:    Convert to upper case any ascii found in the range 0 to Length.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void FIFO_Buffer::ToUpper(size_t Length)
{
    size_t UsedSize;
    size_t Offset;

    UsedSize = m_Size - CheckFreeSpace();

    if(Length > UsedSize)
    {
        Length = UsedSize;
    }

    Offset = Length + m_PopIndex;       // Add Tail offset to offset we want to read

    do
    {
        Length--;
        Offset--;

        if(Offset >= m_Size)            // If our offset is farther than the size, we have a wrap-around
        {
            Offset -= m_Size;
        }

        if((m_pBuffer[Offset] >= 'a') && (m_pBuffer[Offset] <= 'z'))
        {
            m_pBuffer[Offset] -= 32;
        }
    }
    while(Length != 0);
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           TailForward
//
//  Parameter(s):   Size        Size to forward.
//
//  Return:         None
//
//  Description:    Forward tail buffer index.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void FIFO_Buffer::TailForward(size_t Size)
{
    size_t UsedSpace;

    if((UsedSpace = CheckUsedSpace()) < Size)
    {
       Size = UsedSpace;
    }

    m_PopIndex += Size;

    if (m_PopIndex >= m_Size)
    {
        m_PopIndex -= m_Size;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HeadForward
//
//  Parameter(s):   Size        Size to forward.
//
//  Return:         None
//
//  Description:    Forward head buffer index.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void FIFO_Buffer::HeadForward(size_t Size)
{
    size_t FreeSpace;

    if((FreeSpace = CheckFreeSpace()) < Size)
    {
       Size = FreeSpace;
    }

    m_PushIndex += Size;

    if (m_PushIndex >= m_Size)
    {
        m_PushIndex -= m_Size;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HeadBackward
//
//  Parameter(s):   Size        Size to backward.
//
//  Return:         None
//
//  Description:    Backward head buffer index.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void FIFO_Buffer::HeadBackward(size_t Size)
{
    size_t UsedSpace;

    if((UsedSpace = CheckUsedSpace()) < Size)
    {
       Size = UsedSpace;
    }

    if(Size > m_PushIndex)
    {
        Size        -= m_PushIndex;
        m_PushIndex  = m_Size - Size;
    }
    else
    {
        m_PushIndex -= Size;
    }
}

//-------------------------------------------------------------------------------------------------





#if 0


//-------------------------------------------------------------------------------------------------
//
//   Name:   IsEmpty
//
//   Parameter(s):
//   Return Value:
//
//   Description:
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::IsEmpty(void)
{
    return (m_Count <= 0);
}

//-------------------------------------------------------------------------------------------------
//
//   Name:   IsFull
//
//   Parameter(s):
//   Return Value:
//
//   Description:
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::IsFull(void)
{
    return (m_Count >= m_Size);
}

//-------------------------------------------------------------------------------------------------
//
//   Name:   Pop
//
//   Parameter(s):
//   Return Value:
//
//   Description:
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::Pop(uint8_t* pData)
{
    // The FIFO is empty
    if (IsEmpty())
        return false;

    // Pop the data
    *pData = m_pBuffer[m_PopIndex];

    // Calculate the next pop index and update
    m_PopIndex = (m_PopIndex + 1) % m_Size;

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//   Name:   Push
//
//   Parameter(s):
//   Return Value:
//
//   Description:
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
bool FIFO_Buffer::Push(uint8_t Data)
{
    // The FIFO is full
    if(IsFull())
        return false;

    //Push the data
    m_pBuffer[m_PushIndex] = Data;

    // Update the push index
    m_PushIndex = (m_PushIndex + 1) % m_Size;

    return true;
}

#endif
