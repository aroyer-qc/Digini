//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_crc.cpp
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
//  Note(s):    The module of the STM32F1 is very basic, it accept only 32 bits as input value.
//              there is no REFIN/REFOUT. It is MPEG-2 with granularity of 4 bytes.
//
//              To calculate buffer of size that is not a multiple of 4 (sizeof(uint32_t)), the
//              remainder of size/4 is padded with zero.
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
    CRC_32_HW_METHOD_DEF(EXPAND_X_HW_CRC_AS_CLASS_CONST)
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   CRC_HW_Type_e       ID of the CRC method to use.
//  Return:         void
//
//  Description:    This function Initialize the hardware for CRC calculation.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::Initialize(CRC_HW_Type_e Type)
{
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
    m_Type = Type;
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
    CRC->DR = m_MethodList[m_Type].RevInit;
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

    return CRC_Value;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AddByte
//
//  Parameter(s):   Value               Add this byte to calcuation
//  Return:         void
//
//  Description:    Add byte to calculation of on going CRC sequence.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::AddByte(uint8_t Value)
{
    AddBuffer(&Value, 1);
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
void CRC_Driver::AddBuffer(const uint8_t* pBuffer, size_t Length)
{
    uint32_t i;
    uint32_t Remainder;
    uint32_t Data;

    Remainder  = Length % 4;
    Length    -= Remainder;

    // 4 bytes are entered in a row with a single word write
    for(i = 0; i < uint32_t(Length); )
    {
        Data  = ((uint32_t)pBuffer[i++] << 24);
        Data |= ((uint32_t)pBuffer[i++] << 16);
        Data |= ((uint32_t)pBuffer[i++] << 8);
        Data |=  (uint32_t)pBuffer[i++];
    }

    // last bytes specific handling
    while(Remainder != 0)
    {
        Data = (pBuffer[i++] << 24);
        Remainder--;

        if(Remainder != 0)
        {
            Data >>= 8;
        }
        else
        {
            CRC->DR = Data;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CalculateBuffer
//
//  Parameter(s):   pBuffer
//                  Length
//  Return:         CRC_uint_t
//
//  Description:    Start, Calculate the CRC from a byte buffer and return the CRC.
//
//-------------------------------------------------------------------------------------------------
uint32_t CRC_Driver::CalculateBuffer(const uint8_t* pBuffer, size_t Length)
{
    Start();
    AddBuffer(pBuffer, Length);
    return GetValue();
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_CRC_DRIVER == DEF_ENABLED)
