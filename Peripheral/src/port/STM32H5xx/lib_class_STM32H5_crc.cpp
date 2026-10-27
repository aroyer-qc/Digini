//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_crc.cpp
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "BSP_board.h"

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const CRC_Info_t CRC_Driver::m_MethodList[NUMBER_OF_CRC_METHOD] =
{
    CRC_METHOD_DEF(EXPAND_X_CRC_AS_CLASS_CONST)
};

#ifdef USE_FREERTOS
xSemaphoreHandle CRC_Driver::m_Sem             = nullptr;
bool             CRC_Driver::m_SemIsInitialize = false;
#endif

//-------------------------------------------------------------------------------------------------
//
//  constructor
//  Parameter(s):   None
//
//  Description:    Enable the CRC block on the stm32H5xx.
//
//  Note(s):        It is initialize at first call.
//
//-------------------------------------------------------------------------------------------------
CRC_Driver::CRC_Driver()
{
#ifdef USE_FREERTOS
    taskENTER_CRITICAL();                              // Make no other try to initialize the mutex at the same time

    if(CRC_Driver::m_SemIsInitialize == false)
    {
        CRC_Driver::m_SemIsInitialize = true;
        RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
        vSemaphoreCreateBinary(CRC_Driver::m_Sem);
    }
    taskEXIT_CRITICAL();
   #else
        RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
   #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start
//  Parameter(s):   CRC_Type_e      Type
//  Return:         void
//
//  Description:    This function init the conversion with the CRC type selected.
//
//-------------------------------------------------------------------------------------------------
void CRC_Driver::Start(CRC_Type_e Type)
{
    m_pMethod = &CRC_Driver::m_MethodList[Type];
    
    #if (CRC_USE_CRC32_PARKER_F4 == DEF_ENABLED)
    m_Type = Type;
    #endif

    #ifdef USE_FREERTOS
    while(xSemaphoreTake(CRC_Driver::m_Sem, portMAX_DELAY ) == pdFALSE) {}
    #endif
    CRC->POL  = m_pMethod->Polynomial;
    CRC->INIT = m_pMethod->Init;
    CRC->CR   = m_pMethod->Mode;
    CRC->CR  |= 1;
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

    CRC_Value = CRC->DR ^ m_pMethod->XorOut;
    #ifdef USE_FREERTOS
    xSemaphoreGive(CRC_Driver::m_Sem);
    #endif
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
    uint8_t* pBuffer8 = (uint8_t*)pBuffer;
    uint32_t Data32;

    for(; Length >= 4; Length -= 4)
    {
        Data32 = *((uint32_t*)pBuffer8);
        pBuffer8 += 4;

      #if (CRC_USE_CRC32_PARKER_F4 == DEF_ENABLED)
        if(m_Type == CRC32_PARKER_F4)
        {
            CRC->DR = Data32;
        }
        else
      #endif  
        {
            CRC->DR = SWAP_32(Data32);
        }
    }

    if(Length != 0)
    {
        if(Length >= 2)
        {
          #if (CRC_USE_CRC32_PARKER_F4 == DEF_ENABLED)
            if(m_Type == CRC32_PARKER_F4)
            {
                if (Length == 3)
                    Data32 =  (uint32_t)*(uint32_t*)pBuffer8 & 0x00FFFFFF;
                else
                    Data32 =  (uint32_t)*(uint32_t*)pBuffer8 & 0x0000FFFF;
                CRC->DR = Data32;
                Length = 0;
            }
            else
          #endif
            {
                uint16_t Data16 = *((uint16_t*)pBuffer8);
                volatile uint16_t* pRegister = (volatile uint16_t *)(volatile void *)(&CRC->DR);
                *pRegister = SWAP_16(Data16);
                Length -= 2;
                pBuffer8 += 2;
            }
        }

        if(Length == 1)
        {
          #if (CRC_USE_CRC32_PARKER_F4 == DEF_ENABLED)
            if(m_Type == CRC32_PARKER_F4)
            {
                Data32 =  (uint32_t)*(uint32_t*)pBuffer8 & 0x000000FF; // Same as 722
                CRC->DR = Data32;
            }
            else
          #endif
            {
                *(volatile uint8_t *)(volatile void *)(&CRC->DR) = *pBuffer8;
            }
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
uint32_t CRC_Driver::CalculateBuffer(const uint32_t* pBuffer, size_t Length, CRC_Type_e Type)
{
    Start(Type);
    AddBuffer(pBuffer, Length);
    return GetValue();
}

//-------------------------------------------------------------------------------------------------
