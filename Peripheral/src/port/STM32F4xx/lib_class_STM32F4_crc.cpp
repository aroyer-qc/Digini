//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_crc.cpp
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
//
//  Note(s):    The module of the STM32F4 is very basic, it accept only 32 bits as input value.
//              there is no REFIN/REFOUT or XOROUT. It is MPEG-2 with granularity of 4 bytes.
//
//              To calculate buffer of size that is not a multiple of 4 (sizeof(uint32_t)), the
//              reminder of size/4 is padded with zero.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

#if (USE_CRC_DRIVER == DEF_ENABLED)

const CRC_HW_Info_t CRC_Driver::m_MethodList[NUMBER_OF_HW_CRC_METHOD] =
{
    CRC_32_HW_METHOD_DEF(EXPAND_X_CRC_AS_CLASS_CONST)
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   CRC_Type_e          ID of the CRC method to use.
//  Return:         void
//
//  Description:    This function Initialize the hardware for CRC calculation.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::Initialize(CRC_Type_e Type)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
    m_Type  = Type;
    Reset();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start
//  Parameter(s):   None
//  Return:         void
//
//  Description:    This function init the conversion with the CRC type selected.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::Start(void)
{
    CRC->CR = 1;
    CRC_DR  = m_MethodList[m_Type].RevInit;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetValue
//
//  Parameter(s):   void
//  Return:         uint32_t
//
//  Description:    This function return the 32 bits CRC value.
//
//-------------------------------------------------------------------------------------------------
uint32_t CRC_Driver::GetValue(void)
{
    uint32_t CRC_Value;
    
    CRC_Value = CRC->DR ^ m_MethodList[m_Type].XorOut;
    
    return CRC->DR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AddBuffer
//
//  Parameter(s):   pBuffer             Calculate CRC on this buffer
//                  Length              Length of the buffer
//  Return:         void
//
//  Description:    Add buffer to calculation of on going CRC sequence.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::AddBuffer(const void* pBuffer, size_t Length)
{
    //switch(m_Type)// the type should be determine from size of the buffer and used method to faster calculate it
    {
       // case CRC_INPUT_DATA_FORMAT_WORDS:
        {
      //      for(size_t i = 0; i < Length; i++)
            {
       //         CRC->DR = ((uint32_t *)pBuffer)[i];
            }
        }
       // break;

       // case CRC_INPUT_DATA_FORMAT_HALF_WORDS:
        {
      //      HandleHalfWords((uint16_t *)pBuffer, Length);
        }
      //  break;

      //  case CRC_INPUT_DATA_FORMAT_BYTES:
        {
            HandleBytes((uint8_t *)pBuffer, Length);
        }
''        break;
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
CRC_uint_t CRC_Calc::CalculateFullBuffer(const uint8_t* pBuffer, size_t Length)
{
    Start();
    AddBuffer(pBuffer, Length);
    return GetValue();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HandleBytes
//
//  Parameter(s):   pBuffer             Calculate CRC on this buffer
//                  Length              Length of the buffer
//  Return:         void
//
//  Description:    Enter uint8_t buffer data to the CRC calculator.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::HandleBytes(uint8_t* pBuffer, size_t Length)
{
    size_t   i;
    uint16_t Data;
    volatile uint16_t* pRegister;

    // 4 bytes are entered in a row with a single word write,
    for(i = 0; i < (Length / 4); i++)
    {
        CRC->DR = ((uint32_t)pBuffer[4 * i] << 24)       |
                  ((uint32_t)pBuffer[(4 * i) + 1] << 16) |
                  ((uint32_t)pBuffer[(4 * i) + 2] << 8)  |
                   (uint32_t)pBuffer[(4 * i) + 3];
    }

    // last bytes specific handling
    if((Length % 4) != 0)
    {
        if((Length % 4) == 1)
        {
            *(volatile uint8_t *)(volatile void *)(&CRC->DR) = pBuffer[4 * i];
        }

        if((Length % 4) == 2)
        {
            Data = ((uint16_t)(pBuffer[4 * i]) << 8) | (uint16_t)pBuffer[(4 * i) + 1];
            pRegister = (volatile uint16_t *)(volatile void *)(&CRC->DR);
            *pRegister = Data;
        }

        if((Length % 4) == 3)
        {
            Data = ((uint16_t)(pBuffer[4 * i]) << 8) | (uint16_t)pBuffer[(4 * i) + 1];
            pRegister = (volatile uint16_t *)(volatile void *)(&CRC->DR);
            *pRegister = Data;

            *(volatile uint8_t *)(volatile void *)(&CRC->DR) = pBuffer[(4 * i) + 2];
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HandleHalfWords
//
//  Parameter(s):   pBuffer             Calculate CRC on this buffer
//                  Length              Length of the buffer
//  Return:         void
//
//  Description:    Enter uint16_t buffer data to the CRC calculator.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::HandleHalfWords(uint16_t* pBuffer, size_t Length)
{
    size_t i;
    volatile uint16_t* pRegister;

    // 2 half words are entered in a row with a single word write
    for(i = 0; i < (Length / 2); i++)
    {
        CRC->DR = ((uint32_t)pBuffer[2 * i] << 16) | (uint32_t)pBuffer[(2 * i) + 1];
    }

    // Last half word must be carefully fed to the CRC calculator
    if((Length % 2) != 0)
    {
        pRegister  = (volatile uint16_t *)(volatile void *)(&CRC->DR);
        *pRegister = pBuffer[1 * i];
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_CRC_DRIVER == DEF_ENABLED)
