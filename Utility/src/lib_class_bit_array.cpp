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

#define BA_BIT_PER_BYTE             8
#define BA_BYTE_MASK                0xFF
#define OffsetAfterBoundary         BitsBeforeBoundary
#define OffsetBeforeBoundary        BitsAfterBoundary

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const uint8_t BIT_Array::m_Mask[LIB_BA_BIT_PER_BYTE] =
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
    uint8_t  Mask;
    uint32_t BitsBeforeBoundary;
    uint32_t BitsAfterBoundary;
    uint8_t  Data;

    if((Index + uint32_t(Count)) < uint32_t(m_Size))                        // Validate boundary
    {
        // Preset all fix value
        pBitStream         = GetBytePointer(Index);                         // The pointer on the first byte to be modified
        BitsAfterBoundary  = Index % LIB_BA_BIT_PER_BYTE;                   // Number of bits after uint8_t boundary
        BitsBeforeBoundary = LIB_BA_BIT_PER_BYTE - BitsAfterBoundary;       // Number of bits before the uint_t boundary

        do
        {
            Mask = m_Mask[OffsetBeforeBoundary];                            // Get the mask to protect bits in stream not touch by modification

            // -----------------------------------------------------------
            // Process data before the boundary
            // -----------------------------------------------------------

            Data = *pData >> OffsetBeforeBoundary;

            if(Count < BitsBeforeBoundary)
            {
                Mask |= ~m_Mask[OffsetBeforeBoundary + Count];
                Data &= ~Mask;
                Count =  0;
            }
            else
            {
                Count -= BitsBeforeBoundary;
            }

            *pBitStream &= Mask;                                            // Clear all bit in the mask
            *pBitStream |= Data;                                            // Apply data to destination

            // -----------------------------------------------------------
            // Process data after the boundary
            // -----------------------------------------------------------

            if(Count > 0)                                                   // If still some bit to transfer in boundary part
            {
                pBitStream++;
                Data = *pData << OffsetAfterBoundary;
                Mask = ~m_Mask[OffsetBeforeBoundary];

                if(Count <= BitsAfterBoundary)
                {
                    Mask  = m_Mask[Count];
                    Data &= Mask;
                    Count = 0;
                }
                else
                {
                    Count -= BitsAfterBoundary;
                }

                *pBitStream &= Mask;                                        // Clear all bit in reverse mask
                *pBitStream |= Data;                                        // Apply data to destination
                pData++;
           }

            // -----------------------------------------------------------
        }
        while(Count > 0);
    }
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
