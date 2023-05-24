 //-------------------------------------------------------------------------------------------------
//
//  File : lib_crc.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

#if (DIGINI_USE_CRC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// define(s)
//-------------------------------------------------------------------------------------------------

#define CRC_REVERSAL_BITSIZE                32
#define CRC_REVERSAL_CHAR_BITSIZE_SHIFT     24
#define CRC_CHAR_BITSIZE                    8

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const CRC_Info_t CRC_Calc::m_MethodList[NUMBER_OF_CRC_METHOD] =
{
  #if (DIGINI_USE_CRC_8_TO_32_BITS == DEF_ENABLED)
    CRC_8_TO_32_METHOD_DEF(EXPAND_X_CRC_AS_CLASS_CONST)
  #endif
  #if (DIGINI_USE_CRC_MORE_THAN_32_BITS == DEF_ENABLED)
    CRC_33_AND_MORE_METHOD_DEF(EXPAND_X_CRC_AS_CLASS_CONST)
  #endif
};

//-------------------------------------------------------------------------------------------------
//
//   Class: CRC_Calc
//
//   Description:   Class to handle CRC Calculation
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   CRC_Calc
//
//   Parameter(s):  CrcType_e
//
//   Description:   Initializes the CRC_ID that define the method for calculating the CRC
//
//-------------------------------------------------------------------------------------------------
CRC_Calc::CRC_Calc(CRC_Type_e Type)
{
    m_Type       = Type;
    m_Polynomial = m_MethodList[m_Type].Polynomial;
    m_RefIn      = m_MethodList[m_Type].RefIn;
    m_Width      = m_MethodList[m_Type].Width;
    m_Mask       = (CRC_uint_t(0x1) << m_Width) - 1;
    m_TopBit     = CRC_uint_t(0x1) << (m_Width - 1);
}

//-------------------------------------------------------------------------------------------------
//
//   Destructor:    ~CRC
//
//   Parameter(s):  None
//
//   Description:   In some case there are resource to be freed
//
//-------------------------------------------------------------------------------------------------
CRC_Calc::~CRC_Calc()
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start
//
//  Parameter(s):   CrcType
//  Return:         void
//
//  Description:    This function init the conversion with the CRC type selected.
//
//-------------------------------------------------------------------------------------------------
void CRC_Calc::Start(void)
{
    m_Remainder = m_MethodList[m_Type].Init;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Done
//
//  Parameter(s):   void
//  Return:         CRC_uint_t
//
//  Description:    This function return the CRC value.
//
//-------------------------------------------------------------------------------------------------
CRC_uint_t CRC_Calc::Done(void)
{
    m_Remainder ^= m_MethodList[m_Type].XorOut;

    if(m_MethodList[m_Type].RefOut == true)
    {
      #if(DIGINI_USE_CRC_MORE_THAN_32_BITS == DEF_ENABLED)
        if(m_Width <= CRC_REVERSAL_BITSIZE)
      #endif
        {
            m_Remainder = (LIB_BitReversal(m_Remainder) >> (CRC_REVERSAL_BITSIZE - m_Width));
        }
      #if(DIGINI_USE_CRC_MORE_THAN_32_BITS == DEF_ENABLED)
        else
        {
            uint32_t High  = uint32_t(m_Remainder);
            uint32_t Low   = uint32_t(m_Remainder  >> CRC_REVERSAL_BITSIZE);
            uint32_t Shift = m_Width - CRC_REVERSAL_BITSIZE;
            m_Remainder    = CRC_uint_t(LIB_BitReversal(High)) << Shift;
            m_Remainder   |= CRC_uint_t(LIB_BitReversal(Low)) >> (CRC_REVERSAL_BITSIZE - Shift);
        }
      #endif
    }

    m_Remainder &= m_Mask;

    return m_Remainder;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Calculate
//
//  Parameter(s):   uint8_t Value
//  Return:         void
//
//  Description:    Find the CRC of a byte.
//
//-------------------------------------------------------------------------------------------------
void CRC_Calc::Calculate( uint8_t Value)
{
    if(m_RefIn == true)
    {
        Value = uint8_t(LIB_BitReversal(uint32_t(Value)) >> CRC_REVERSAL_CHAR_BITSIZE_SHIFT);
    }

    m_Remainder ^= (CRC_uint_t(Value) << (m_Width - CRC_CHAR_BITSIZE));


    for(int i = CRC_CHAR_BITSIZE; i > 0; i--)
    {
        if(m_Remainder & m_TopBit)
        {
            m_Remainder <<= 1;
            m_Remainder  ^= m_Polynomial;
        }
        else
        {
            m_Remainder <<= 1;
        }
    }

    m_Remainder &= m_Mask;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CalculateBuffer
//
//  Parameter(s):   pBuffer
//                  Length
//  Return:         void
//
//  Description:   Calculate the CRC from a byte buffer.
//
//-------------------------------------------------------------------------------------------------
void CRC_Calc::CalculateBuffer(const uint8_t *pBuffer, size_t Length)
{
    for(; Length > 0; Length--, pBuffer++)
    {
        Calculate(*pBuffer);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CalculateFullBuffer
//
//  Parameter(s):   pBuffer
//                  Length
//  Return:         CRC_uint_t
//
//  Description:    Start, Calculate the CRC from a byte buffer and return the CRC.
//
//-------------------------------------------------------------------------------------------------
CRC_uint_t CRC_Calc::CalculateFullBuffer(const uint8_t *pBuffer, size_t Length)
{
    Start();
    CalculateBuffer(pBuffer, Length);
    return Done();
}

//-------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------------------------
