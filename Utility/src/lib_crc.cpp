//-------------------------------------------------------------------------------------------------
//
//  File : crc.cpp
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


uint32_t BitReversal(uint32_t Value)
{
  uint32_t result;

#if (__CORTEX_M >= 0x03U) || (__CORTEX_SC >= 300U)
   __ASM volatile ("rbit %0, %1" : "=r" (result) : "r" (Value));
#else
  int32_t s = 4 /*sizeof(v)*/ * 8 - 1; /* extra shift needed at end */

  result = Value;                      /* r will be reversed bits of v; first get LSB of v */
  for(Value >>= 1U; Value; Value >>= 1U)
  {
    result <<= 1U;
    result |= Value & 1U;
    s--;
  }
  result <<= s;                        /* shift when v's highest bits are zero */
#endif
  return(result);
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
        m_Remainder ^= (BitReversal(m_Remainder) >> (32 - m_Width));
    }

    m_Remainder &= (0xFFFFFFFF >> (32 - m_Width));

    return m_Remainder;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Byte
//
//  Parameter(s):   uint8_t Byte
//  Return:         void
//
//  Description:    Find the CRC of a byte.
//
//-------------------------------------------------------------------------------------------------

void CRC_Calc::Calculate(const uint8_t Value)
{
    if(m_RefIn == true)
    {
        // m_Remainder ^= BitReversal(Value);            // probably need to shift the data 24 for CRC8, and 16 Bits for CRC16
        m_Remainder ^= (BitReversal(uint32_t(Value)) >> (32 - m_Width));
    }
    else
    {
        m_Remainder ^= (uint32_t(Value) << (m_Width - 8));
    }

    for(uint32_t i = 8; i > 0; i--)
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
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Done
//
//  Parameter(s):   pBuffer
//                  Length
//  Return:         void
//
//  Description:    Find the CRC from a byte buffer.
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

#endif

//-------------------------------------------------------------------------------------------------
