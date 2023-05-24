//-------------------------------------------------------------------------------------------------
//
//  File : lib_crc.cpp
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

#if (DIGINI_USE_CRC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const CRC_Info_t CRC_Calc::m_MethodList[NUMBER_OF_CRC_METHOD] =
{
    CRC_METHOD_DEF(EXPAND_X_CRC_AS_CLASS_CONST)
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
    m_Mask       = (0x1 << m_Width) - 1;
    m_TopBit     = 0x1 << (m_Width - 1);
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
//  Return:         uint32_t
//
//  Description:    This function return the CRC value.
//
//-------------------------------------------------------------------------------------------------
uint32_t CRC_Calc::Done(void)
{
    m_Remainder ^= m_MethodList[m_Type].XorOut;

    if(m_MethodList[m_Type].RefOut == true)
    {
        m_Remainder = (LIB_BitReversal(m_Remainder) >> (32 - m_Width));
    }

    m_Remainder &= m_Mask;
;

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
        Value = LIB_BitReversal(uint32_t(Value)) >> 24;
    }

    m_Remainder ^= (uint32_t(Value) << (m_Width - 8));

    for(int i = 8; i > 0; i--)
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
//  Return:         uint32_t
//
//  Description:    Start, Calculate the CRC from a byte buffer and return the CRC.
//
//-------------------------------------------------------------------------------------------------
uint32_t CRC_Calc::CalculateFullBuffer(const uint8_t *pBuffer, size_t Length)
{
    Start();
    CalculateBuffer(pBuffer, Length);
    return Done();
}

//-------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------------------------
