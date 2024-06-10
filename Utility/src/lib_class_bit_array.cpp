//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_bit_array.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#include "./Digini/lib_digini.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define BA_BIT_PER_BYTE         8
#define BA_BYTE_MASK            0xFF

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const uint8_t BIT_Array::m_ByteMask[LIB_BA_BIT_PER_BYTE] =
{
    0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Constructor
//
//  Parameter(s):   None
//
//  Description:    Preinit pointer and size of the bit arrays
//
//-------------------------------------------------------------------------------------------------
BIT_Array::BIT_Array(uint8_t* pBuffer, size_t Size)
{
    m_pBuffer = pBuffer;
    m_Size    = Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Get
//
//  Parameter(s):   uint32_t Index              Index of the entry
//  Return:         bool                        value at index 0 or 1
//
//  Description:    Get a single entry by index
//
//-------------------------------------------------------------------------------------------------
bool BIT_Array::Get(uint32_t Index)
{
    uint8_t* pByte = GetBytePointer(Index);
    uint8_t  Mask  = GetBitMask(Index);

    if(pByte != nullptr)
    {
        if(((*pByte) & Mask) != 0)
        {
            return 1;
        }
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Get
//
//  Parameter(s):   uint32_t Index              Index of the entry
//  Return:         None
//
//  Description:    Set/Reset a single entry by index
//
//-------------------------------------------------------------------------------------------------
void BIT_Array::Set(uint32_t Index, bool Value)
{
    uint8_t* pByte = GetBytePointer(Index);
    uint8_t  Mask  = GetBitMask(Index);

    if(pByte != nullptr)
    {
        if(Value == 0)
        {
            *pByte &= ~Mask;
        }
        else
        {
            *pByte |= Mask;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Set
//
//  Parameter(s):   uint32_t Index              Index of the entry
//                  uint8_t* pData              Data to use for this Set command
//                  size_t   Count              Number of bit to affect by this Set command
//  Return:         None
//
//  Description:    Set/reset 1 to 32 bits Size from the index.
//
//-------------------------------------------------------------------------------------------------
void BIT_Array::Set(uint32_t Index, uint8_t* pData, size_t Count)
{
    uint8_t* pBitStream;
    uint8_t  B1_Mask;
    uint8_t  AppliedMask;
    uint8_t  Offset;

    pBitStream = GetBytePointer(Index);                     // The pointer on the first byte to be modified
    Offset     = Index % LIB_BA_BIT_PER_BYTE;               // Offset to shift the data
    B1_Mask    = m_ByteMask[Offset];                        // Get the mask for all bit in the first byte  (subsequent byte modification use the same mask)

    do
    {
        AppliedMask = B1_Mask;                              // Calculate proper to prevent erasing bit over the count
        AppliedMask =


        *pBitStream &= AppliedMask;                         // Clear all bit in the mask
        *pBitStream |= ((*pData) >> Offset);                // Shift

        if(B1_Mask != 0x00)                                 // If overlap next byte in bitstream
        {
            AppliedMask = B1_Mask;                          // Calculate proper to prevent erasing bit over the count


            pBitStream++;
            *pBitStream &= ~(AppliedMask);                  // Clear all bit in reverse mask
            *pBitStream |= ((*pData) << (8 - Offset));      // Shift
        }

        Count -= LIB_BA_BIT_PER_BYTE;
    }
    while(Count >= 8);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Fill
//
//  Parameter(s):   bool  Value
//  Return:         None
//
//  Description:    Fill the entire arrays with 0 or 1
//
//-------------------------------------------------------------------------------------------------
void BIT_Array::Fill(bool Value)
{
    size_t NumberOfByte = (m_Size / BA_BIT_PER_BYTE) + 1;

    for(size_t i = 0; i < NumberOfByte; i++)
    {
        m_pBuffer[i] = (Value == 0) ? 0x00 : 0xFF;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetBytePointer
//
//  Parameter(s):   uint32_t Index              Index of the entry
//  Return:         None
//
//  Description:   Return the pointer of the uint8_t containing the bit from the index
//
//-------------------------------------------------------------------------------------------------
uint8_t* BIT_Array::GetBytePointer(uint32_t Index)
{
    if(Index < m_Size)
    {
        return &m_pBuffer[Index / BA_BIT_PER_BYTE];
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetBitMask
//
//  Parameter(s):   uint32_t Index      Index of the entry
//  Return:         uint8_t             Return the bit mask for the entry
//
//  Description:    Return a bit mask
//
//-------------------------------------------------------------------------------------------------
uint8_t BIT_Array::GetBitMask(uint32_t Index)
{
    if(Index < m_Size)
    {
        return (uint8_t(1) << (Index % BA_BIT_PER_BYTE));
    }

    return 0;       // No mask
}

//-------------------------------------------------------------------------------------------------
