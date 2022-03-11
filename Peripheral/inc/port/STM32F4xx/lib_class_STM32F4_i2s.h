//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_i2s.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2022 Alain Royer.
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_I2S_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// typedef(s)
//-------------------------------------------------------------------------------------------------

enum I2S_ID_e
{
    #if (I2S_DRIVER_SUPPORT_I2S1_CFG == DEF_ENABLED)
        DRIVER_I2S1_ID,
    #endif

    #if (I2S_DRIVER_SUPPORT_I2S2_CFG == DEF_ENABLED)
        DRIVER_I2S2_ID,
    #endif

    #if (I2S_DRIVER_SUPPORT_I2S3_CFG == DEF_ENABLED)
        DRIVER_I2S3_ID,
    #endif

    NB_OF_I2S_DRIVER,
};


// To found how many port there is
enum I2S_Port_e
{
  #define X_I2S(I2S_NUMBER, CLOCK, SPEED, PREEMP_PRIO, EVENT_IRQ_NB, ERROR_IRQ_NB, GPIO_SCL, GPIO_PIN_MASK_SCL, GPIO_SCL_CLOCK, GPIO_SCL_AF, \
                                                                                   GPIO_SDA, GPIO_PIN_MASK_SDA, GPIO_SDA_CLOCK, GPIO_SDA_AF) I2C##I2C_NUMBER##_PORT,
    I2S_DEF
  #undef X_I2S
    I2S_NB_PORT_CONST
};



typedef struct
{
  uint16_t I2S_Mode;         // Specifies the I2S operating mode.                            This Parameter can be a value of @ref I2S_Mode
  uint16_t I2S_Standard;     // Specifies the standard used for the I2S communication.       This Parameter can be a value of @ref I2S_Standard
  uint16_t I2S_DataFormat;   // Specifies the data format for the I2S communication.         This Parameter can be a value of @ref I2S_Data_Format
  uint16_t I2S_MCLKOutput;   // Specifies whether the I2S MCLK output is enabled or not.     This Parameter can be a value of @ref I2S_MCLK_Output
  uint32_t I2S_AudioFreq;    // Specifies the frequency selected for the I2S communication.  This Parameter can be a value of @ref I2S_Audio_Frequency
  uint16_t I2S_CPOL;         // Specifies the idle state of the I2S clock.                   This Parameter can be a value of @ref I2S_Clock_Polarity
}I2S_InitTypeDef;


struct I2S_PortInfo_t
{
    I2S_TypeDef*        pI2Sx;
//    uint32_t            Clock;
//    uint32_t            Timing;
//    uint8_t             PreempPrio;
//    IRQn_Type           EV_IRQn;
//    IRQn_Type           ER_IRQn;
//    IO_Config_t         SCL;
//    IO_Config_t         SDA;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class I2S
{
    public:

                        I2S                 (I2S_PortInfo_t* pPort);

        //SystemState_e   LockToDevice           (uint8_t Device);       // Set I2S to this device and lock
        //SystemState_e   UnlockFromDevice     (uint8_t Device);       // Unlock I2S from device
        SystemState_e   GetStatus           (void);

        SystemState_e   ReadRegister        (uint8_t Register, void* pRxBuffer, size_t RxSize);
        SystemState_e   ReadRegister        (uint8_t Register, void* pRxBuffer, size_t RxSize, uint8_t Device);
        SystemState_e   Transfer            (void* pTxBuffer, size_t TxSize, void* pRxBuffer, size_t RxSize);
        SystemState_e   Transfer            (void* pTxBuffer, size_t TxSize, void* pRxBuffer, size_t RxSize, uint8_t Device);

        void            Initialize          (void);

    private:

        void            Lock                (void);
        void            Unlock              (void);

        I2S_PortInfo_t*                 m_pPort;
        nOS_Mutex                       m_Mutex;
        int16_t                         m_Device;

        volatile size_t                 m_TxSize;
        volatile size_t                 m_RxSize;
        volatile uint8_t*               m_pTxBuffer;
        volatile uint8_t*               m_pRxBuffer;

        volatile uint8_t*               m_pAddressInDevice;
        volatile uint8_t*               m_pDataAddress;
        volatile size_t                 m_Size;
        volatile size_t                 m_AddressLengthCount;

        volatile SystemState_e          m_Status;
        volatile uint8_t                m_Timeout;
};



/*  Function used to set the SPI configuration to the default reset state *****/ 
void SPI_I2S_DeInit(SPI_TypeDef* SPIx);
/* Initialization and Configuration functions *********************************/
void I2S_Init(SPI_TypeDef* SPIx, I2S_InitTypeDef* I2S_InitStruct);
void I2S_StructInit(I2S_InitTypeDef* I2S_InitStruct);
void I2S_Cmd(SPI_TypeDef* SPIx, FunctionalState NewState);
void I2S_FullDuplexConfig(SPI_TypeDef* I2Sxext, I2S_InitTypeDef* I2S_InitStruct);
/* Data transfers functions ***************************************************/ 
void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data);
uint16_t SPI_I2S_ReceiveData(SPI_TypeDef* SPIx);

/* DMA transfers management functions *****************************************/
void SPI_I2S_DMACmd(SPI_TypeDef* SPIx, uint16_t SPI_I2S_DMAReq, FunctionalState NewState);
/* Interrupts and flags management functions **********************************/
void SPI_I2S_ITConfig(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT, FunctionalState NewState);
FlagStatus SPI_I2S_GetFlagStatus(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG);
void SPI_I2S_ClearFlag(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG);
ITStatus SPI_I2S_GetITStatus(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT);
void SPI_I2S_ClearITPendingBit(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT);




//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef STM32F4_I2S_GLOBAL

  I2S_PortInfo_t I2S_PortInfo[I2S_NB_PORT_CONST] =
  {
    #define X_I2S(I2S_NUMBER, CLOCK, SPEED, PREEMP_PRIO,            EVENT_IRQ_NB,            ERROR_IRQ_NB,  GPIO_SCL, GPIO_PIN_MASK_SCL, GPIO_SCL_CLOCK,                                                                                      GPIO_SCL_AF, \
                                                                                                            GPIO_SDA, GPIO_PIN_MASK_SDA, GPIO_SDA_CLOCK,                                                                                      GPIO_SDA_AF) \
            {I2S##I2S_NUMBER, CLOCK, SPEED, PREEMP_PRIO, (IRQn_Type)EVENT_IRQ_NB, (IRQn_Type)ERROR_IRQ_NB, {GPIO_SCL, GPIO_PIN_MASK_SCL, GPIO_SCL_CLOCK, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_OD, GPIO_PULL_PIN_NO_PULL, GPIO_SPEED_FREQ_HIGH, GPIO_SCL_AF},\
                                                                                                           {GPIO_SDA, GPIO_PIN_MASK_SDA, GPIO_SDA_CLOCK, GPIO_MODER_ALTERNATE, GPIO_TYPE_PIN_OD, GPIO_PULL_PIN_NO_PULL, GPIO_SPEED_FREQ_HIGH, GPIO_SDA_AF},},
      I2S_DEF
    #undef  X_I2S
  };
    GPIO_TypeDef*       pPort;
    uint32_t            Pin;
    uint32_t            Clock;
    uint32_t            Mode;
    uint32_t            Type;
    uint32_t            Pull;
    uint32_t            Speed;
    uint32_t            AF;

  // Name of the I2S Object will be the name of the port + _Port -> Ex. I2S1 -> I2S1_Port
  // Ex. I2S I2S_Port1(&I2S_PortInfo[I2S1_PORT]);
  #define X_I2S(I2S_NUMBER, CLOCK, SPEED, PREEMP_PRIO, EVENT_IRQ_NB, ERROR_IRQ_NB,  GPIO_SCL, GPIO_PIN_MASK_SCL, GPIO_SCL_CLOCK, GPIO_SCL_AF, \
                                                                                    GPIO_SDA, GPIO_PIN_MASK_SDA, GPIO_SDA_CLOCK, GPIO_SDA_AF) \
    class I2S I2S_Port##I2C_NUMBER(&I2S_PortInfo[I2C##I2C_NUMBER##_PORT]);
    I2S_DEF
  #undef  X_I2S

#else //STM32F7_I2S_GLOBAL

  extern I2S_PortInfo_t     I2S_PortInfo[I2S_NB_PORT_CONST];

  #define X_I2S(I2S_NUMBER, CLOCK, SPEED, PREEMP_PRIO, EVENT_IRQ_NB, ERROR_IRQ_NB,  GPIO_SCL, GPIO_PIN_MASK_SCL, GPIO_SCL_CLOCK, GPIO_SCL_AF, \
                                                                                    GPIO_SDA, GPIO_PIN_MASK_SDA, GPIO_SDA_CLOCK, GPIO_SDA_AF) \
    extern class I2S I2S_Port##I2S_NUMBER;
    I2S_DEF
  #undef X_I2S

#endif // STM32F4_I2S_GLOBAL

//-------------------------------------------------------------------------------------------------




