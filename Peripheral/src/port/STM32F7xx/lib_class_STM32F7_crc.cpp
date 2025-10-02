//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_crc.cpp
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

#if (USE_CRC_DRIVER == DEF_ENABLED)

const CRC_HW_Info_t CRC_Driver::m_MethodList[NUMBER_OF_HW_CRC_METHOD] =
{
    CRC_32_HW_METHOD_DEF(EXPAND_X_HW_CRC_AS_CLASS_CONST)
};

nOS_Mutex CRC_Driver::m_Mutex;
bool      CRC_Driver::m_MutexIsInitialize = false;

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
    nOS_StatusReg sr;

    nOS_EnterCritical(sr);                              // Make no other try to initialize the mutex at the same time

    if(CRC_Driver::m_MutexIsInitialize == false)
    {
        CRC_Driver::m_MutexIsInitialize = true;
        RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
        nOS_MutexCreate(&CRC_Driver::m_Mutex, NOS_MUTEX_NORMAL, NOS_MUTEX_PRIO_INHERIT);
    }

    nOS_LeaveCritical(sr);

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
    while(nOS_MutexLock(&CRC_Driver::m_Mutex, NOS_WAIT_INFINITE) != NOS_OK);
    CRC->POL  = m_MethodList[m_Type].Polynomial;
    CRC->INIT = m_MethodList[m_Type].Init;
    CRC->CR   = m_MethodList[m_Type].Mode;
    CRC->CR  |= CRC_CR_RESET;
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
    while(nOS_MutexUnlock(&CRC_Driver::m_Mutex) != NOS_OK);
    return CRC_Value;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AddByte
//
//  Parameter(s):   Value               Add this byte to calculation
//  Return:         void
//
//  Description:    Add byte to calculation of on going CRC sequence.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::AddByte(uint8_t Value)
{
    CRC->DR = Value;
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
//  Note(s);        Size is always in bytes
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::AddBuffer(const uint32_t* pBuffer, size_t Length)
{
    uint32_t i;
    uint8_t* pBuffer8 = (uint8_t*)pBuffer;

    for(i = 0; i < (Length / 4); i++)
    {
        uint32_t Data32 = *((uint32_t*)&pBuffer8[4 * i]);
        CRC->DR = SWAP_32(Data32);
    }

    if((Length % 4) != 0)
    {
        if((Length % 4) == 1)
        {
            *(volatile uint8_t *)(volatile void *)(&CRC->DR) = pBuffer8[4 * i];
        }

        if((Length % 4) >= 2)
        {
            volatile uint16_t* pRegister = (volatile uint16_t *)(volatile void *)(&CRC->DR);
            uint16_t Data16 = *((uint16_t*)&pBuffer8[4 * i]);
            *pRegister = SWAP_16(Data16);
        }

        if((Length % 4) == 3)
        {
            *(volatile uint8_t *)(volatile void *)(&CRC->DR) = pBuffer8[(4 * i) + 2];
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
uint32_t CRC_Driver::CalculateBuffer(const uint32_t* pBuffer, size_t Length, CRC_HW_Type_e Type)
{
    Initialize(Type);
    Start();
    AddBuffer(pBuffer, Length);
    return GetValue();
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_CRC_DRIVER == DEF_ENABLED)
