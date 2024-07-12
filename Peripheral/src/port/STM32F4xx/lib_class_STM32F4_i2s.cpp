//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_i2s.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define I2S_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  I2S_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_I2S_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CR1_CLEAR_MASK                  ((uint16_t)0xFBF5)          // I2S registers Masks
#define FLAG_MASK                       ((uint32_t)0x00FFFFFF)      // I2S FLAG mask
#define I2S_TIME_OUT                    100                         // Milliseconds
#define I2S_PLL_TIME_OUT                5                           // Milliseconds

// SPI_I2S_DMA_transfer_requests
#define SPI_I2S_DMAReq_Tx               ((uint16_t)0x0002)
#define SPI_I2S_DMAReq_Rx               ((uint16_t)0x0001)
#define IS_SPI_I2S_DMAREQ(DMAREQ) ((((DMAREQ) & (uint16_t)0xFFFC) == 0x00) && ((DMAREQ) != 0x00))

// SPI_I2S_interrupts_definition
#define SPI_I2S_IT_TXE                  ((uint8_t)0x71)
#define SPI_I2S_IT_RXNE                 ((uint8_t)0x60)
#define SPI_I2S_IT_ERR                  ((uint8_t)0x50)
#define I2S_IT_UDR                      ((uint8_t)0x53)
#define SPI_I2S_IT_TIFRFE               ((uint8_t)0x58)

#define IS_SPI_I2S_CONFIG_IT(IT) (((IT) == SPI_I2S_IT_TXE) || \
                                  ((IT) == SPI_I2S_IT_RXNE) || \
                                  ((IT) == SPI_I2S_IT_ERR))

#define SPI_I2S_IT_OVR                  ((uint8_t)0x56)
//#define SPI_IT_MODF                     ((uint8_t)0x55)
//#define SPI_IT_CRCERR                   ((uint8_t)0x54)

#define IS_SPI_I2S_CLEAR_IT(IT) (((IT) == SPI_IT_CRCERR))

#define IS_SPI_I2S_GET_IT(IT) (((IT) == SPI_I2S_IT_RXNE)|| ((IT) == SPI_I2S_IT_TXE) || \
                               ((IT) == SPI_IT_CRCERR)  || ((IT) == SPI_IT_MODF) || \
                               ((IT) == SPI_I2S_IT_OVR) || ((IT) == I2S_IT_UDR) ||\
                               ((IT) == SPI_I2S_IT_TIFRFE))

// SPI_I2S_flags_definition
#define SPI_I2S_FLAG_RXNE               ((uint16_t)0x0001)
#define SPI_I2S_FLAG_TXE                ((uint16_t)0x0002)
#define I2S_FLAG_CHSIDE                 ((uint16_t)0x0004)
#define I2S_FLAG_UDR                    ((uint16_t)0x0008)
#define SPI_FLAG_CRCERR                 ((uint16_t)0x0010)
#define SPI_FLAG_MODF                   ((uint16_t)0x0020)
#define SPI_I2S_FLAG_OVR                ((uint16_t)0x0040)
#define SPI_I2S_FLAG_BSY                ((uint16_t)0x0080)
#define SPI_I2S_FLAG_TIFRFE             ((uint16_t)0x0100)

#define IS_SPI_I2S_CLEAR_FLAG(FLAG) (((FLAG) == SPI_FLAG_CRCERR))
#define IS_SPI_I2S_GET_FLAG(FLAG) (((FLAG) == SPI_I2S_FLAG_BSY) || ((FLAG) == SPI_I2S_FLAG_OVR)  || \
                                   ((FLAG) == SPI_FLAG_MODF)    || ((FLAG) == SPI_FLAG_CRCERR)   || \
                                   ((FLAG) == I2S_FLAG_UDR)     || ((FLAG) == I2S_FLAG_CHSIDE)   || \
                                   ((FLAG) == SPI_I2S_FLAG_TXE) || ((FLAG) == SPI_I2S_FLAG_RXNE) || \
                                   ((FLAG) == SPI_I2S_FLAG_TIFRFE))


#define SPI_DMAReq_Tx                SPI_I2S_DMAReq_Tx
#define SPI_DMAReq_Rx                SPI_I2S_DMAReq_Rx
#define SPI_IT_TXE                   SPI_I2S_IT_TXE
#define SPI_IT_RXNE                  SPI_I2S_IT_RXNE
#define SPI_IT_ERR                   SPI_I2S_IT_ERR
#define SPI_IT_OVR                   SPI_I2S_IT_OVR
#define SPI_FLAG_RXNE                SPI_I2S_FLAG_RXNE
#define SPI_FLAG_TXE                 SPI_I2S_FLAG_TXE
#define SPI_FLAG_OVR                 SPI_I2S_FLAG_OVR
#define SPI_FLAG_BSY                 SPI_I2S_FLAG_BSY
#define SPI_DeInit                   SPI_I2S_DeInit
#define SPI_ITConfig                 SPI_I2S_ITConfig
#define SPI_DMACmd                   SPI_I2S_DMACmd
#define SPI_SendData                 SPI_I2S_SendData
#define SPI_ReceiveData              SPI_I2S_ReceiveData
#define SPI_GetFlagStatus            SPI_I2S_GetFlagStatus
#define SPI_ClearFlag                SPI_I2S_ClearFlag
#define SPI_GetITStatus              SPI_I2S_GetITStatus
#define SPI_ClearITPendingBit        SPI_I2S_ClearITPendingBit


#define RCC_PLLI2SCFGR_PLLI2SN_Pos   6
//#define RCC_PLLI2SCFGR_PLLI2SQ_Pos
#define RCC_PLLI2SCFGR_PLLI2SR_Pos   28

//-------------------------------------------------------------------------------------------------
// variable(s)
//-------------------------------------------------------------------------------------------------

// These PLL parameters are valid when the f(VCO clock) = 1Mhz
const uint32_t I2S_Driver::m_PLLN[NB_OF_I2S_FREQUENCY] = {256, 429, 213, 429, 426, 271, 258, 344, 0/*xxx*/};
const uint32_t I2S_Driver::m_PLLR[NB_OF_I2S_FREQUENCY] = {5,   4,   4,   4,   4,   6,   3,   1,   0/*xxx*/};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   I2S_Driver
//
//   Parameter(s):  I2S_ID          ID of the I2S info
//
//   Description:   Initializes the I2Sx peripheral according to the specified Parameters
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
I2S_Driver::I2S_Driver(I2S_ID_e I2S_ID)
{
    //m_IsItInitialize = false;
    m_pInfo = &I2S_Info[I2S_ID];
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   None
//  Return:         none
//
//  Description:    Initializes the I2Sx peripheral according to the specified parameters in
//                  config.
//
//  Note(s):        The function calculates the optimal prescaler needed to obtain the most
//                  accurate audio frequency (depending on the I2S clock source, the PLL values
//                  and the product configuration). But in case the prescaler value is greater
//                  than 511, the default value (0x02) will be configured instead.
//
//-------------------------------------------------------------------------------------------------
void I2S_Driver::Initialize(void)
{
    DMA_Stream_TypeDef* pDMA;

    m_Timeout = 0;

    NVIC_DisableIRQ(m_pInfo->I2S_DMA_IRQn);

    // ---- Module configuration ----
    RCC->APB1ENR  |=  m_pInfo->RCC_APB1xPeriph;          // I2S exist only on APB1
    RCC->APB1RSTR |=  m_pInfo->RCC_APB1xPeriph;          // Reset I2S
    RCC->APB1RSTR &= ~m_pInfo->RCC_APB1xPeriph;          // Release reset signal of I2S

    // ---- GPIO configuration ----
    IO_PinInit(m_pInfo->MCLK);
    IO_PinInit(m_pInfo->CK);
    IO_PinInit(m_pInfo->SD);
    IO_PinInit(m_pInfo->WS);

    m_Status = SYS_IDLE;
    //m_NoMemoryIncrement = false; ??

    // Preinit register that won't change
    pDMA = m_pInfo->pDMA_Stream;
    pDMA->PAR = uint32_t(&m_pInfo->pI2Sx->DR);          // Configure receive data register
    pDMA->CR  = DMA_PERIPH_TO_MEMORY          |
                DMA_MODE_NORMAL               |
                DMA_PERIPH_NO_INCREMENT       |
                DMA_MEMORY_INCREMENT          |
                DMA_P_DATA_ALIGN_BYTE         |  //I2S3_DMAx_PERIPH_DATA_SIZE
                DMA_M_DATA_ALIGN_BYTE         |  //I2S3_DMAx_MEM_DATA_SIZE
                DMA_P_BURST_SINGLE            |
                DMA_M_BURST_SINGLE            |
                DMA_PRIORITY_HIGH             |
                DMA_SxCR_TCIE                 |
        //hdma_i2sTx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
        //hdma_i2sTx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
                m_pInfo->DMA_Channel;


#if 0
/**
  * @note   if an external clock is used as source clock for the I2S, then the define
  *         I2S_EXTERNAL_CLOCK_VAL in file stm32f4xx_conf.h should be enabled and set
  *         to the value of the the source clock frequency (in Hz).
  */
//void I2S_Init(SPI_TypeDef* SPIx, I2S_InitTypeDef* I2S_InitStruct)
    uint16_t tmpreg = 0;
    uint16_t i2sdiv = 2;
    uint16_t i2sodd = 0;
    uint16_t packetlength = 1;
    uint32_t tmp = 0, i2sclk = 0;
  #ifndef I2S_EXTERNAL_CLOCK_VAL
    uint32_t pllm = 0;
    uint32_t plln = 0;
    uint32_t pllr = 0;
  #endif // I2S_EXTERNAL_CLOCK_VAL

    //----------------------- SPIx I2SCFGR & I2SPR Configuration -----------------
    // Clear I2SMOD, I2SE, I2SCFG, PCMSYNC, I2SSTD, CKPOL, DATLEN and CHLEN bits
    SPIx->I2SCFGR &= I2SCFGR_CLEAR_MASK;
    SPIx->I2SPR    = 0x0002;

    // Get the I2SCFGR register value
    tmpreg = SPIx->I2SCFGR;

    // If the default value has to be written, reinitialize i2sdiv and i2sodd
    if(I2S_InitStruct->I2S_AudioFreq == I2S_AudioFreq_Default)
    {
        i2sodd = (uint16_t)0;
        i2sdiv = (uint16_t)2;
    }
    // If the requested audio frequency is not the default, compute the prescaler
    else
    {
        // Check the frame length (For the Prescaler computing) *******************
        if(I2S_InitStruct->I2S_DataFormat == I2S_DataFormat_16b)
        {
            packetlength = 1;       // Packet length is 16 bits
        }
        else
        {
            packetlength = 2;       // Packet length is 32 bits
        }

        // Get I2S source Clock frequency  ****************************************

        // If an external I2S clock has to be used, this define should be set in the project configuration or in the stm32f4xx_conf.h file
      #ifdef I2S_EXTERNAL_CLOCK_VAL
        if((RCC->CFGR & RCC_CFGR_I2SSRC) == 0)                                                                  // Set external clock as I2S clock source
        {
            RCC->CFGR |= (uint32_t)RCC_CFGR_I2SSRC;
        }

        i2sclk = I2S_EXTERNAL_CLOCK_VAL;                                                                        // Set the I2S clock to the external clock  value

      #else // There is no define for External I2S clock source
        if((RCC->CFGR & RCC_CFGR_I2SSRC) != 0)                                                                  // Set PLLI2S as I2S clock source
        {
            RCC->CFGR &= ~(uint32_t)RCC_CFGR_I2SSRC;
        }

        plln = (uint32_t)(((RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SN) >> RCC_PLLI2SCFGR_PLLI2SN_Pos));          // Get the PLLI2SN value
        pllr = (uint32_t)(((RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SR) >> RCC_PLLI2SCFGR_PLLI2SR_Pos));          // Get the PLLI2SR value
        pllm = (uint32_t)(RCC->PLLCFGR & RCC_PLLCFGR_PLLM);                                                     // Get the PLLM value
        i2sclk = (uint32_t)(((HSE_VALUE / pllm) * plln) / pllr);                                                // Get the I2S source clock value
      #endif // I2S_EXTERNAL_CLOCK_VAL

        // Compute the Real divider depending on the MCLK output state, with a floating point
        if(I2S_InitStruct->I2S_MCLKOutput == I2S_MCLKOutput_Enable)
        {
            tmp = (uint16_t)(((((i2sclk >> 8) * 10) / I2S_InitStruct->I2S_AudioFreq)) + 5);                    // MCLK output is enabled
        }
        else
        {
            tmp = (uint16_t)(((((i2sclk / (packetlength << 5)) * 10) / I2S_InitStruct->I2S_AudioFreq)) + 5);    // MCLK output is disabled
        }

        tmp = tmp / 10;                                                                                         // Remove the flatting point
        i2sodd = (uint16_t)(tmp & (uint16_t)0x0001);                                                            // Check the parity of the divider
        i2sdiv = (uint16_t)((tmp - i2sodd) >> 1);                                                                // Compute the i2sdiv prescaler
        i2sodd = (uint16_t) (i2sodd << 8);                                                                      // Get the Mask for the Odd bit (SPI_I2SPR[8]) register
    }

    // Test if the divider is 1 or 0 or greater than 0xFF
    if((i2sdiv < 2) || (i2sdiv > 0xFF))
    {                                                                                                           // Set the default values
        i2sdiv = 2;
        i2sodd = 0;
    }

    // Write to SPIx I2SPR register the computed value
    SPIx->I2SPR = (uint16_t)((uint16_t)i2sdiv | (uint16_t)(i2sodd | (uint16_t)I2S_InitStruct->I2S_MCLKOutput));

    // Configure the I2S with the SPI_InitStruct values
    tmpreg |= (uint16_t)((uint16_t)SPI_I2SCFGR_I2SMOD              |
                         (uint16_t)(I2S_InitStruct->I2S_Mode       |
                         (uint16_t)(I2S_InitStruct->I2S_Standard   |
                         (uint16_t)(I2S_InitStruct->I2S_DataFormat |
                         (uint16_t)I2S_InitStruct->I2S_CPOL))));

    // Write to SPIx I2SCFGR
    SPIx->I2SCFGR = tmpreg;
}
    #endif












    // I2S DMA IRQ Channel configuration
    ISR_Init(m_pInfo->I2S_DMA_IRQn, 5);    
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterCallback
//
//  Parameter(s):   pCallback       Callback pointer
//  Return:         None
//
//  Description:    Register callback for user code in ISR
//
//-------------------------------------------------------------------------------------------------
void I2S_Driver::RegisterCallback(CallbackInterface* pCallback)
{
    m_pCallback = pCallback;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableCallbackType
//
//  Parameter(s):   CallBackType    Type if the ISR callback
//                  pContext        Context for ISR
//  Return:         None
//
//  Description:    Enable the type of interrupt for the callback.
//
//-------------------------------------------------------------------------------------------------
void I2S_Driver::EnableCallbackType(int CallBackType, void* pContext)
{
    switch(CallBackType)
    {
        case I2S_CALLBACK_HALF_COMPLETE:
        {
            m_pContextHalf  = pContext;
            m_CallBackType |= CallBackType;
        }
        break;

        case I2S_CALLBACK_COMPLETE:
        {
            m_pContext      = pContext;
            m_CallBackType |= CallBackType;
        }
        break;

        case I2S_CALLBACK_ERROR:
        {
            m_pContextERROR = pContext;
            m_CallBackType |= CallBackType;
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SetFrequency
//
//   Parameter(s):  Frequency           Audio frequency used to play the audio stream.
//   Return Value:  SystemState_e
//
//   Description:   Update the audio frequency
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2S_Driver::SetFrequency(I2S_Frequency_e Frequency)
{
    nOS_TickCounter TickStart;

    // Disable the PLLI2S
    CLEAR_BIT(RCC->CR, RCC_CR_PLLI2SON);

    // Wait here until PLLI2S is disabled or time out
    TickStart = GetTick();

    while((RCC->CR & RCC_CR_PLLI2SRDY) != 0)
    {
        if(TickHasTimeOut(TickStart, I2S_PLL_TIME_OUT) == true)
        {
            return SYS_TIME_OUT;            // Return in case of timeout detected
        }
    }

    // I2S clock config
    // PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) × (PLLI2SN/PLLM)
    // I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR
    RCC->PLLI2SCFGR = (I2S_Driver::m_PLLN[Frequency] << RCC_PLLI2SCFGR_PLLI2SN_Pos) | (I2S_Driver::m_PLLR[Frequency] << RCC_PLLI2SCFGR_PLLI2SR_Pos);

    // Enable the PLLI2S
    SET_BIT(RCC->CR, RCC_CR_PLLI2SON);

    // Wait till PLLI2S is ready
    TickStart = GetTick();

    while((RCC->CR & RCC_CR_PLLI2SRDY) == 0)
    {
        if(TickHasTimeOut(TickStart, I2S_PLL_TIME_OUT) == true)
        {
            return SYS_TIME_OUT;            // Return in case of timeout detected
        }
    }

  return SYS_READY;

// Update the I2S audio frequency configuration
//??? why I2S3_Init(AudioFreq);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      GetStatus
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Return general status of the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2S_Driver::GetStatus(void)
{
    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Transmit
//
//   Parameter(s):  pBuffer     Pointer to buffer address.
//                  Size        Number of data to be written.
//   Return Value:  None
//
//   Description:   Sends n-Bytes on the I2S interface using DMA.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2S_Driver::Transmit(uint16_t* pBuffer, size_t Size)
{
    uint32_t            Register;
    DMA_Stream_TypeDef* pDMA;

    if((pBuffer == nullptr) || (Size == 0))
    {
        return SYS_ERROR;
    }

    if(m_Status != SYS_READY)
    {
        return SYS_BUSY;
    }

    // Set state and reset error code
    m_Status = SYS_BUSY_TX;
    //hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
    m_pBuffer = pBuffer;

    Register = m_pInfo->pI2Sx->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);

    if((Register == uint32_t(I2S_DATAFORMAT_24B)) || (Register == uint32_t(I2S_DATAFORMAT_32B)))
    {
        m_TX_transfertSize  = (Size << 1);
        m_TX_transfertCount = (Size << 1);
    }
    else
    {
        m_TX_transfertSize  = Size;
        m_TX_transfertCount = Size;
    }

    pDMA  = m_pInfo->pDMA_Stream;

    m_Status = SYS_BUSY_TX;                 // Set flag to busy in TX
    pDMA->M0AR = uint32_t(pBuffer);       // Set DMA source
    pDMA->PAR  = uint32_t(&m_pInfo->pI2Sx->DR);        // Configure DMA Stream destination address
    // TODO size must be of the buffer size used by DMA
    pDMA->NDTR = Size;                      // Set size of the TX
    pDMA->CR &= uint32_t(~DMA_SxCR_DBM);  // Clear DBM bit
    DMA_EnableInterrupt(pDMA, m_pInfo->IT_Flag);                                             // TODO if transfer is less than HT of full buffer lenght.. handle ending
    SET_BIT(pDMA->FCR, DMA_SxFCR_FEIE);      // Enable Common interrupts
    SET_BIT(pDMA->CR, DMA_SxCR_EN);          // Enable the DMA module
    DMA_ClearFlag(pDMA, m_pInfo->IT_Flag);   // Clear IRQ DMA flag

    // Check if the I2S is already enabled
//    if(CheckBit(m_pInfo->pI2Sx->I2SCFGR, SPI_I2SCFGR_I2SE) == 0)   // ?? why ?
    {
        // Enable I2S peripheral
        SET_BIT(m_pInfo->pI2Sx->I2SCFGR, SPI_I2SCFGR_I2SE);
    }

    // Check if the I2S Tx request is already enabled
//    if(CheckBit(m_pInfo->pI2Sx->CR2, SPI_CR2_TXDMAEN) == 0)   ?? why ?
    {
        // Enable Tx DMA Request
        SET_BIT(m_pInfo->pI2Sx->CR2, SPI_CR2_TXDMAEN);
    }

    return SYS_READY;
}

/*
//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):
//
//  Return:
//
//  Description:
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2S_Driver::TX_CompleteCallback(void)
{
    // Call the user function which will manage directly transfer complete
    // TODO call user callback
    return SYS_BUSY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):
//
//  Return:
//
//  Description:
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2S_Driver::TX_HalfCompleteCallback(void)
{
    // Manage the remaining file size and new address offset: This function should be coded by user
    // TODO call user callback
    return SYS_BUSY;
}
*/

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2S_DRIVER == DEF_ENABLED)


#if 0


/*==============================================================================
                                             User NOTES
1. How To use this driver:
--------------------------
   - This driver supports STM32F401xx devices on STM32F401-Discovery Kit:
        a) to play an audio file (all functions names start by BSP_AUDIO_OUT_xxx)
        b) to record an audio file through MP45DT02, ST MEMS (all functions names start by AUDIO_IN_xxx)
a) PLAY A FILE:
==============
   + Call the function BSP_AUDIO_OUT_Init(
                                    OutputDevice: physical output mode (OUTPUT_DEVICE_SPEAKER,
                                                 OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_AUTO or
                                                 OUTPUT_DEVICE_BOTH)
                                    Volume: initial volume to be set (0 is min (mute), 100 is max (100%)
                                    AudioFreq: Audio frequency in Hz (8000, 16000, 22500, 32000 ...)
                                    this parameter is relative to the audio file/stream type.
                                   )
      This function configures all the hardware required for the audio application (codec, I2C, I2S,
      GPIOs, DMA and interrupt if needed). This function returns 0 if configuration is OK.
      If the returned value is different from 0 or the function is stuck then the communication with
      the codec (try to un-plug the power or reset device in this case).
      - OUTPUT_DEVICE_SPEAKER: only speaker will be set as output for the audio stream.
      - OUTPUT_DEVICE_HEADPHONE: only headphones will be set as output for the audio stream.
      - OUTPUT_DEVICE_AUTO: Selection of output device is made through external switch (implemented
         into the audio jack on the discovery board). When the Headphone is connected it is used
         as output. When the headphone is disconnected from the audio jack, the output is
         automatically switched to Speaker.
      - OUTPUT_DEVICE_BOTH: both Speaker and Headphone are used as outputs for the audio stream
         at the same time.
   + Call the function BSP_AUDIO_OUT_Play(
                                  pBuffer: pointer to the audio data file address
                                  Size: size of the buffer to be sent in Bytes
                                 )
      to start playing (for the first time) from the audio file/stream.
   + Call the function BSP_AUDIO_OUT_Pause() to pause playing
   + Call the function BSP_AUDIO_OUT_Resume() to resume playing.
       Note. After calling BSP_AUDIO_OUT_Pause() function for pause, only BSP_AUDIO_OUT_Resume() should be called
          for resume (it is not allowed to call BSP_AUDIO_OUT_Play() in this case).
       Note. This function should be called only when the audio file is played or paused (not stopped).
   + For each mode, you may need to implement the relative callback functions into your code.
      The Callback functions are named BSP_AUDIO_OUT_XXXCallBack() and only their prototypes are declared in
      the stm32f401_discovery_audio.h file. (refer to the example for more details on the callbacks implementations)
   + To Stop playing, to modify the volume level, the frequency or to mute, use the functions
       BSP_AUDIO_OUT_Stop(), BSP_AUDIO_OUT_SetVolume(), AUDIO_OUT_SetFrequency() BSP_AUDIO_OUT_SetOutputMode and BSP_AUDIO_OUT_SetMute().
   + The driver API and the callback functions are at the end of the stm32f401_discovery_audio.h file.

Driver architecture:
--------------------
   + This driver provide the High Audio Layer: consists of the function API exported in the stm32f401_discovery_audio.h file
       (BSP_AUDIO_OUT_Init(), BSP_AUDIO_OUT_Play() ...)
   + This driver provide also the Media Access Layer (MAL): which consists of functions allowing to access the media containing/
       providing the audio file/stream. These functions are also included as local functions into
       the stm32f401_discovery_audio.c file (I2S3_Init()...)
Known Limitations:
-------------------
   1- When using the Speaker, if the audio file quality is not high enough, the speaker output
      may produce high and uncomfortable noise level. To avoid this issue, to use speaker
      output properly, try to increase audio file sampling rate (typically higher than 48KHz).
      This operation will lead to larger file size.
   2- Communication with the audio codec (through I2C) may be corrupted if it is interrupted by some
      user interrupt routines (in this case, interrupts could be disabled just before the start of
      communication then re-enabled when it is over). Note that this communication is only done at
      the configuration phase (BSP_AUDIO_OUT_Init() or BSP_AUDIO_OUT_Stop()) and when Volume control modification is
      performed (BSP_AUDIO_OUT_SetVolume() or BSP_AUDIO_OUT_SetMute()or BSP_AUDIO_OUT_SetOutputMode()).
      When the audio data is played, no communication is required with the audio codec.
   3- Parsing of audio file is not implemented (in order to determine audio file properties: Mono/Stereo, Data size,
      File size, Audio Frequency, Audio Data header size ...). The configuration is fixed for the given audio file.
   4- Supports only Stereo audio streaming. To play mono audio streams, each data should be sent twice
      on the I2S or should be duplicated on the source buffer. Or convert the stream in stereo before playing.
   5- Supports only 16-bits audio data size.
b) RECORD A FILE:
================
   + Call the function BSP_AUDIO_IN_Init(
                                    AudioFreq: Audio frequency in Hz (8000, 16000, 22500, 32000 ...)
                                    )
      This function configures all the hardware required for the audio application (I2S,
      GPIOs, DMA and interrupt if needed). This function returns 0 if configuration is OK.
   + Call the function BSP_AUDIO_IN_Record(
                            pbuf Main buffer pointer for the recorded data storing
                            size Current size of the recorded buffer
                            )
      to start recording from the microphone.
   + User needs to implement user callbacks to retrieve data saved in the record buffer
      (AUDIO_IN_RxHalfCpltCallback/BSP_AUDIO_IN_ReceiveComplete_CallBack)
   + Call the function AUDIO_IN_STOP() to stop recording
==============================================================================*/


/*##### PLAY #####*/
static AUDIO_DrvTypeDef           *pAudioDrv;
I2S_HandleTypeDef                 hAudioOutI2s;

/*### RECORDER ###*/
I2S_HandleTypeDef                 hAudioInI2s;

/* PDM filters params */
PDM_Filter_Handler_t  PDM_FilterHandler[2];
PDM_Filter_Config_t   PDM_FilterConfig[2];

__IO uint16_t AudioInVolume = DEFAULT_AUDIO_IN_VOLUME;

static uint8_t I2S3_Init(uint32_t AudioFreq);
static uint8_t I2S2_Init(uint32_t AudioFreq);
static void PDMDecoder_Init(uint32_t AudioFreq, uint32_t ChnlNbrIn, uint32_t ChnlNbrOut);

/**
  * @brief  Configures the audio peripherals.
  * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @retval SYS_READY if correct communication, else wrong communication
  */
SystemState_e I2S::InitInput(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
    SystemState_e ret = SYS_READY;

    /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
    BSP_AUDIO_OUT_ClockConfig(&hAudioOutI2s, AudioFreq, NULL);

    /* I2S data transfer preparation:
    Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
    hAudioOutI2s.Instance = I2S3;
    if(HAL_I2S_GetState(&hAudioOutI2s) == HAL_I2S_STATE_RESET)
    {
        /* Init the I2S MSP: this __weak function can be redefined by the application*/
        BSP_AUDIO_OUT_MspInit(&hAudioOutI2s, NULL);
    }

    /* I2S data transfer preparation: Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
    /* Configure the I2S peripheral */
    if(I2S3_Init(AudioFreq) != SYS_READY)
    {
        ret = SYS_ERROR;
    }

    if(ret == SYS_READY)
    {
        /* Retieve audio codec identifier */
        if(((cs43l22_drv.ReadID(AUDIO_I2C_ADDRESS)) & CS43L22_ID_MASK) == CS43L22_ID)
        {
            /* Initialize the audio driver structure */
            pAudioDrv = &cs43l22_drv;
        }
        else
        {
            ret = SYS_ERROR;
        }
    }

    if(ret == SYS_READY)
    {
        pAudioDrv->Init(AUDIO_I2C_ADDRESS, OutputDevice, Volume, AudioFreq);
    }

    return ret;
}

/**
  * @brief  Starts playing audio stream from a data buffer for a determined size.
  * @param  pBuffer: Pointer to the buffer
  * @param  Size: Number of audio data BYTES.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
{
  /* Call the audio Codec Play function */
  if(pAudioDrv->Play(AUDIO_I2C_ADDRESS, pBuffer, Size) != 0)
  {
    return SYS_ERROR;
  }
  else
  {
    /* Update the Media layer and enable it for play */
    HAL_I2S_Transmit_DMA(&hAudioOutI2s, pBuffer, DMA_MAX(Size/AUDIODATA_SIZE));

    return SYS_READY;
  }
}

/**
  * @brief   Pauses the audio file stream. In case of using DMA, the DMA Pause
  *          feature is used.
  * WARNING: When calling BSP_AUDIO_OUT_Pause() function for pause, only the
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play()
  *          function for resume could lead to unexpected behavior).
  * @retval  AUDIO_OK if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_OUT_Pause(void)
{
  /* Call the Audio Codec Pause/Resume function */
  if(pAudioDrv->Pause(AUDIO_I2C_ADDRESS) != 0)
  {
    return SYS_ERROR;
  }
  else
  {
    /* Call the Media layer pause function */
    HAL_I2S_DMAPause(&hAudioOutI2s);

    /* Return SYS_READY when all operations are correctly done */
    return SYS_READY;
  }
}

/**
  * @brief   Resumes the audio file streaming.
  * WARNING: When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play()
  *          function for resume could lead to unexpected behavior).
  * @retval  SYS_READY if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_OUT_Resume(void)
{
    /* Call the Audio Codec Pause/Resume function */
    if(pAudioDrv->Resume(AUDIO_I2C_ADDRESS) != 0)
    {
        return SYS_ERROR;
    }
    else
    {
        /* Call the Media layer resume function */
        HAL_I2S_DMAResume(&hAudioOutI2s);

        /* Return SYS_READY when all operations are correctly done */
        return SYS_READY;
    }
}

/**
  * @brief  Stops audio playing and Power down the Audio Codec.
  * @param  Option: could be one of the following parameters
  *           - CODEC_PDWN_HW: completely shut down the codec (physically).
  *                            Then need to reconfigure the Codec after power on.
  * @retval SYS_READY if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_OUT_Stop(uint32_t Option)
{
    /* Call DMA Stop to disable DMA stream before stopping codec */
    HAL_I2S_DMAStop(&hAudioOutI2s);

    /* Call Audio Codec Stop function */
    if(pAudioDrv->Stop(AUDIO_I2C_ADDRESS, Option) != 0)
    {
        return SYS_ERROR;
    }
    else
    {
        if(Option == CODEC_PDWN_HW)
        {
            /* Wait at least 1ms */
            HAL_Delay(1);

            /* Reset the pin */
            HAL_GPIO_WritePin(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, GPIO_PIN_RESET);
        }

        /* Return SYS_READY when all operations are correctly done */
        return SYS_READY;
    }
}

/**
  * @brief  Controls the current audio volume level.
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for
  *         Mute and 100 for Max volume level).
  * @retval SYS_READY if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_OUT_SetVolume(uint8_t Volume)
{
    /* Call the codec volume control function with converted volume value */
    if(pAudioDrv->SetVolume(AUDIO_I2C_ADDRESS, Volume) != 0)
    {
        return SYS_ERROR;
    }
    else
    {
        /* Return SYS_READY when all operations are correctly done */
        return SYS_READY;
    }
}

/**
  * @brief  Enables or disables the MUTE mode by software
  * @param  Cmd: could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to
  *         unmute the codec and restore previous volume level.
  * @retval SYS_READY if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_OUT_SetMute(uint32_t Cmd)
{
    /* Call the Codec Mute function */
    if(pAudioDrv->SetMute(AUDIO_I2C_ADDRESS, Cmd) != 0)
    {
        return SYS_ERROR;
    }
    else
    {
        /* Return SYS_READY when all operations are correctly done */
        return SYS_READY;
    }
}

/**
  * @brief  Switch dynamically (while audio file is played) the output target
  *         (speaker or headphone).
  * @note   This function modifies a global variable of the audio codec driver: OutputDev.
  * @param  Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
  *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO
  * @retval SYS_READY if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_OUT_SetOutputMode(uint8_t Output)
{
    /* Call the Codec output Device function */
    if(pAudioDrv->SetOutputMode(AUDIO_I2C_ADDRESS, Output) != 0)
    {
        return SYS_ERROR;
    }
    else
    {
        /* Return SYS_READY when all operations are correctly done */
        return SYS_READY;
    }
}




/**
  * @brief  Manages the DMA FIFO error event.
  */
__weak void BSP_AUDIO_OUT_Error_CallBack(void)
{
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Initializes the Audio Codec audio interface (I2S)
  * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral.
  */
static SystemState_e I2S3_Init(uint32_t AudioFreq)
{
    /* Initialize the hAudioOutI2s Instance parameter */
    hAudioOutI2s.Instance         = I2S3;

    /* Disable I2S block */
    __HAL_I2S_DISABLE(&hAudioOutI2s);

    /* I2S3 peripheral configuration */
    hAudioOutI2s.Init.AudioFreq   = AudioFreq;
    hAudioOutI2s.Init.ClockSource = I2S_CLOCK_PLL;
    hAudioOutI2s.Init.CPOL        = I2S_CPOL_LOW;
    hAudioOutI2s.Init.DataFormat  = I2S_DATAFORMAT_16B;
    hAudioOutI2s.Init.MCLKOutput  = I2S_MCLKOUTPUT_ENABLE;
    hAudioOutI2s.Init.Mode        = I2S_MODE_MASTER_TX;
    hAudioOutI2s.Init.Standard    = I2S_STANDARD;

    /* Initialize the I2S peripheral with the structure above */
    if(HAL_I2S_Init(&hAudioOutI2s) != HAL_OK)
    {
        return SYS_ERROR;
    }
    else
    {
        return SYS_READY;
    }
}

/**
  * @brief  Initializes wave recording.
  * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral.
  * @param  BitRes: Audio frequency to be configured for the I2S peripheral.
  * @param  ChnlNbr: Audio frequency to be configured for the I2S peripheral.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_IN_Init(uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr)
{
    /* Configure PLL clock */
    BSP_AUDIO_IN_ClockConfig(&hAudioInI2s, AudioFreq, NULL);

    /* Configure the PDM library */
    /* On STM32F401-Discovery a single microphone is mounted, samples are duplicated
       to make stereo audio streams */
    PDMDecoder_Init(AudioFreq, ChnlNbr, 2);

    /* Configure the I2S peripheral */
    hAudioInI2s.Instance = I2S2;
    if(HAL_I2S_GetState(&hAudioInI2s) == HAL_I2S_STATE_RESET)
    {
        /* Initialize the I2S Msp: this __weak function can be rewritten by the application */
        BSP_AUDIO_IN_MspInit(&hAudioInI2s, NULL);
    }

    // Configure the I2S2
    I2S2_Init(AudioFreq);

    /* Return AUDIO_OK when all operations are correctly done */
    return SYS_READY;
}

/**
  * @brief  Starts audio recording.
  * @param  pbuf: Main buffer pointer for the recorded data storing
  * @param  size: Current size of the recorded buffer
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_IN_Record(uint16_t* pbuf, uint32_t size)
{
  HAL_I2S_Receive_DMA(&hAudioInI2s, pbuf, size);    // Start the process receive DMA
  return SYS_READY;                                 // Return AUDIO is OK when all operations are correctly done
}

/**
  * @brief  Stops audio recording.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_IN_Stop(void)
{
    HAL_I2S_DMAStop(&hAudioInI2s);      // Call the Media layer pause function
    return SYS_READY;                   // Return AUDIO_OK when all operations are correctly done
}

/**
  * @brief  Pauses the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_IN_Pause(void)
{
    HAL_I2S_DMAPause(&hAudioInI2s);     // Call the Media layer pause function */
    return SYS_READY;                   // Return AUDIO_OK when all operations are correctly done
}

/**
  * @brief  Resumes the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
SystemState_e BSP_AUDIO_IN_Resume(void)
{
    HAL_I2S_DMAResume(&hAudioInI2s);    // Call the Media layer pause/resume function
    return SYS_READY;                   // Return AUDIO_OK when all operations are correctly done
}

/**
  * @brief  Controls the audio in volume level.
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for
  *         Mute and 100 for Max volume level).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_SetVolume(uint8_t Volume)
{
  /* Set the Global variable AudioInVolume */
  AudioInVolume = Volume;

  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}

/**
  * @brief  Converts audio format from PDM to PCM.
  * @param  PDMBuf: Pointer to data PDM buffer
  * @param  PCMBuf: Pointer to data PCM buffer
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_PDMToPCM(uint16_t *PDMBuf, uint16_t *PCMBuf)
{
  uint16_t AppPDM[INTERNAL_BUFF_SIZE/2];
  uint32_t index = 0;

  /* PDM Demux */
  for(index = 0; index<INTERNAL_BUFF_SIZE/2; index++)
  {
    AppPDM[index] = HTONS(PDMBuf[index]);
  }

  for(index = 0; index < DEFAULT_AUDIO_IN_CHANNEL_NBR; index++)
  {
    /* PDM to PCM filter */
    PDM_Filter((uint8_t*)&AppPDM[index], (uint16_t*)&(PCMBuf[index]), &PDM_FilterHandler[index]);
  }

  /* Duplicate samples since a single microphone in mounted on STM32F4-Discovery */
  for(index = 0; index < PCM_OUT_SIZE; index++)
  {
    PCMBuf[(index<<1)+1] = PCMBuf[index<<1];
  }

  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}
/**
  * @brief  Audio In Clock Config.
  * @param  hi2s: I2S handle
  * @param  AudioFreq: Audio frequency used to record the audio stream.
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  * @note   This API is called by BSP_AUDIO_IN_Init()
  *         Being __weak it can be overwritten by the application
  */
__weak void BSP_AUDIO_IN_ClockConfig(I2S_HandleTypeDef *hi2s, uint32_t AudioFreq, void *Params)
{
  RCC_PeriphCLKInitTypeDef rccclkinit;

  /*Enable PLLI2S clock*/
  HAL_RCCEx_GetPeriphCLKConfig(&rccclkinit);
  /* PLLI2S_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  if ((AudioFreq & 0x7) == 0)
  {
    /* Audio frequency multiple of 8 (8/16/32/48/96/192)*/
    /* PLLI2S_VCO Output = PLLI2S_VCO Input * PLLI2SN = 192 Mhz */
    /* I2SCLK = PLLI2S_VCO Output/PLLI2SR = 192/6 = 32 Mhz */
    rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    rccclkinit.PLLI2S.PLLI2SN = 192;
    rccclkinit.PLLI2S.PLLI2SR = 6;
    HAL_RCCEx_PeriphCLKConfig(&rccclkinit);
  }
  else
  {
    /* Other Frequency (11.025/22.500/44.100) */
    /* PLLI2S_VCO Output = PLLI2S_VCO Input * PLLI2SN = 290 Mhz */
    /* I2SCLK = PLLI2S_VCO Output/PLLI2SR = 290/2 = 145 Mhz */
    rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    rccclkinit.PLLI2S.PLLI2SN = 290;
    rccclkinit.PLLI2S.PLLI2SR = 2;
    HAL_RCCEx_PeriphCLKConfig(&rccclkinit);
  }
}

/**
  * @brief  BSP AUDIO IN MSP Init.
  * @param  hi2s: I2S handle
  * @param  Params : pointer on additional configuration parameters, can be NULL.
  */
__weak void BSP_AUDIO_IN_MspInit(I2S_HandleTypeDef *hi2s, void *Params)
{
  static DMA_HandleTypeDef hdma_i2sRx;
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable the I2S2 peripheral clock */
  I2S2_CLK_ENABLE();

  /* Enable I2S GPIO clocks */
  I2S2_SCK_GPIO_CLK_ENABLE();
  I2S2_MOSI_GPIO_CLK_ENABLE();

  /* I2S2 pins configuration: SCK and MOSI pins ------------------------------*/
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;

  GPIO_InitStruct.Pin       = I2S2_SCK_PIN;
  GPIO_InitStruct.Alternate = I2S2_SCK_AF;
  HAL_GPIO_Init(I2S2_SCK_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = I2S2_MOSI_PIN ;
  GPIO_InitStruct.Alternate = I2S2_MOSI_AF;
  HAL_GPIO_Init(I2S2_MOSI_GPIO_PORT, &GPIO_InitStruct);

  /* Enable the DMA clock */
  I2S2_DMAx_CLK_ENABLE();

  if(hi2s->Instance == I2S2)
  {
    /* Configure the hdma_i2sRx handle parameters */
    hdma_i2sRx.Init.Channel             = I2S2_DMAx_CHANNEL;
    hdma_i2sRx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_i2sRx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_i2sRx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_i2sRx.Init.PeriphDataAlignment = I2S2_DMAx_PERIPH_DATA_SIZE;
    hdma_i2sRx.Init.MemDataAlignment    = I2S2_DMAx_MEM_DATA_SIZE;
    hdma_i2sRx.Init.Mode                = DMA_CIRCULAR;
    hdma_i2sRx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_i2sRx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_i2sRx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_i2sRx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_i2sRx.Init.PeriphBurst         = DMA_MBURST_SINGLE;

    hdma_i2sRx.Instance = I2S2_DMAx_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(hi2s, hdmarx, hdma_i2sRx);

    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hdma_i2sRx);

    /* Configure the DMA Stream */
    HAL_DMA_Init(&hdma_i2sRx);
  }

  /* I2S DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(I2S2_DMAx_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(I2S2_DMAx_IRQ);
}


/**
  * @brief  User callback when record buffer is filled.
  */
__weak void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  /* This function should be implemented by the user application.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  */
__weak void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
  /* This function should be implemented by the user application.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
}

/**
  * @brief  Audio IN Error callback function.
  */
__weak void BSP_AUDIO_IN_Error_Callback(void)
{
  /* This function is called when an Interrupt due to transfer error on or peripheral
     error occurs. */
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Initializes the PDM library.
  * @param  AudioFreq: Audio sampling frequency
  * @param  ChnlNbrIn: Number of input audio channels in the PDM buffer
  * @param  ChnlNbrOut: Number of desired output audio channels in the  resulting PCM buffer
  *         Number of audio channels (1: mono; 2: stereo)
  */
static void PDMDecoder_Init(uint32_t AudioFreq, uint32_t ChnlNbrIn, uint32_t ChnlNbrOut)
{
  uint32_t index = 0;

  /* Enable CRC peripheral to unlock the PDM library */
  __HAL_RCC_CRC_CLK_ENABLE();

  for(index = 0; index < ChnlNbrIn; index++)
  {
    /* Init PDM filters */
    PDM_FilterHandler[index].bit_order  = PDM_FILTER_BIT_ORDER_LSB;
    PDM_FilterHandler[index].endianness = PDM_FILTER_ENDIANNESS_LE;
    PDM_FilterHandler[index].high_pass_tap = 2122358088;
    PDM_FilterHandler[index].out_ptr_channels = ChnlNbrOut;
    PDM_FilterHandler[index].in_ptr_channels  = ChnlNbrIn;
    PDM_Filter_Init((PDM_Filter_Handler_t *)(&PDM_FilterHandler[index]));

    /* PDM lib config phase */
    PDM_FilterConfig[index].output_samples_number = AudioFreq/1000;
    PDM_FilterConfig[index].mic_gain = 24;
    PDM_FilterConfig[index].decimation_factor = PDM_FILTER_DEC_FACTOR_64;
    PDM_Filter_setConfig((PDM_Filter_Handler_t *)&PDM_FilterHandler[index], &PDM_FilterConfig[index]);
  }
}

/**
  * @brief  Initializes the Audio Codec audio interface (I2S)
  * @note   This function assumes that the I2S input clock (through PLL_R in
  *         Devices RevA/Z and through dedicated PLLI2S_R in Devices RevB/Y)
  *         is already configured and ready to be used.
  * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral.
  */
static uint8_t I2S2_Init(uint32_t AudioFreq)
{
  /* Initialize the hAudioInI2s Instance parameter */
  hAudioInI2s.Instance          = I2S2;

  /* Disable I2S block */
  __HAL_I2S_DISABLE(&hAudioInI2s);

  /* I2S2 peripheral configuration */
  hAudioInI2s.Init.AudioFreq    = 2 * AudioFreq;
  hAudioInI2s.Init.ClockSource  = I2S_CLOCK_PLL;
  hAudioInI2s.Init.CPOL         = I2S_CPOL_HIGH;
  hAudioInI2s.Init.DataFormat   = I2S_DATAFORMAT_16B;
  hAudioInI2s.Init.MCLKOutput   = I2S_MCLKOUTPUT_DISABLE;
  hAudioInI2s.Init.Mode         = I2S_MODE_MASTER_RX;
  hAudioInI2s.Init.Standard     = I2S_STANDARD_LSB;

  /* Initialize the I2S peripheral with the structure above */
  if(HAL_I2S_Init(&hAudioInI2s) != HAL_OK)
  {
    return AUDIO_ERROR;
  }
  else
  {
    return AUDIO_OK;
  }
}

//AUDIO IN OUT Private Functions

/**
  * @brief  I2S error callbacks.
  * @param  hi2s: I2S handle
  */
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
  /* Manage the error generated on DMA FIFO: This function
     should be coded by user (its prototype is already declared in stm32f401_discovery_audio.h) */
  if(hi2s->Instance == I2S3)
  {
    BSP_AUDIO_OUT_Error_CallBack();
  }
  if(hi2s->Instance == I2S2)
  {
    BSP_AUDIO_IN_Error_Callback();
  }
}


#endif






































#if 0

/**

 ===================================================================
                  ##### How to use this driver #####
 ===================================================================
 [..]
   (#) Enable peripheral clock using the following functions
       RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE) for SPI1
       RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE) for SPI2
       RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE) for SPI3
       RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE) for SPI4
       RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE) for SPI5
       RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE) for SPI6.

   (#) Enable SCK, MOSI, MISO and NSS GPIO clocks using RCC_AHB1PeriphClockCmd()
       function. In I2S mode, if an external clock source is used then the I2S
       CKIN pin GPIO clock should also be enabled.

   (#) Peripherals alternate function:
       (++) Connect the pin to the desired peripherals' Alternate Function (AF)
            using GPIO_PinAFConfig() function
       (++) Configure the desired pin in alternate function by:
            GPIO_InitStruct->GPIO_Mode = GPIO_Mode_AF
       (++) Select the type, pull-up/pull-down and output speed via GPIO_PuPd,
            GPIO_OType and GPIO_Speed members
       (++) Call GPIO_Init() function In I2S mode, if an external clock source is
            used then the I2S CKIN pin should be also configured in Alternate
            function Push-pull pull-up mode.

   (#) Program the Polarity, Phase, First Data, Baud Rate Prescaler, Slave
       Management, Peripheral Mode and CRC Polynomial values using the SPI_Init()
       function.
       In I2S mode, program the Mode, Standard, Data Format, MCLK Output, Audio
       frequency and Polarity using I2S_Init() function. For I2S mode, make sure
       that either:
       (++) I2S PLL is configured using the functions
            RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S), RCC_PLLI2SCmd(ENABLE) and
            RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY); or
       (++) External clock source is configured using the function
            RCC_I2SCLKConfig(RCC_I2S2CLKSource_Ext) and after setting correctly
            the define constant I2S_EXTERNAL_CLOCK_VAL in the stm32f4xx_conf.h file.

   (#) Enable the NVIC and the corresponding interrupt using the function
       SPI_ITConfig() if you need to use interrupt mode.

   (#) When using the DMA mode
       (++) Configure the DMA using DMA_Init() function
       (++) Active the needed channel Request using SPI_I2S_DMACmd() function

   (#) Enable the SPI using the SPI_Cmd() function or enable the I2S using
       I2S_Cmd().

   (#) Enable the DMA using the DMA_Cmd() function when using DMA mode.

   (#) Optionally, you can enable/configure the following parameters without
       re-initialization (i.e there is no need to call again SPI_Init() function):
       (++) When bidirectional mode (SPI_Direction_1Line_Rx or SPI_Direction_1Line_Tx)
            is programmed as Data direction parameter using the SPI_Init() function
            it can be possible to switch between SPI_Direction_Tx or SPI_Direction_Rx
            using the SPI_BiDirectionalLineConfig() function.
       (++) When SPI_NSS_Soft is selected as Slave Select Management parameter
            using the SPI_Init() function it can be possible to manage the
            NSS internal signal using the SPI_NSSInternalSoftwareConfig() function.
       (++) Reconfigure the data size using the SPI_DataSizeConfig() function
       (++) Enable or disable the SS output using the SPI_SSOutputCmd() function

    (#) To use the CRC Hardware calculation feature refer to the Peripheral
        CRC hardware Calculation subsection.


 [..] It is possible to use SPI in I2S full duplex mode, in this case, each SPI
      peripheral is able to manage sending and receiving data simultaneously
      using two data lines. Each SPI peripheral has an extended block called I2Sxext
      (ie. I2S2ext for SPI2 and I2S3ext for SPI3).
      The extension block is not a full SPI IP, it is used only as I2S slave to
      implement full duplex mode. The extension block uses the same clock sources
      as its master.
      To configure I2S full duplex you have to:

      (#) Configure SPIx in I2S mode (I2S_Init() function) as described above.

      (#) Call the I2S_FullDuplexConfig() function using the same strucutre passed to
          I2S_Init() function.

      (#) Call I2S_Cmd() for SPIx then for its extended block.

      (#) To configure interrupts or DMA requests and to get/clear flag status,
          use I2Sxext instance for the extension block.

 [..] Functions that can be called with I2Sxext instances are: I2S_Cmd(),
      I2S_FullDuplexConfig(), SPI_I2S_ReceiveData(), SPI_I2S_SendData(),
      SPI_I2S_DMACmd(), SPI_I2S_ITConfig(), SPI_I2S_GetFlagStatus(),
      SPI_I2S_ClearFlag(), SPI_I2S_GetITStatus() and SPI_I2S_ClearITPendingBit().

      Example: To use SPI3 in Full duplex mode (SPI3 is Master Tx, I2S3ext is Slave Rx):

      RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
      I2S_StructInit(&I2SInitStruct);
      I2SInitStruct.Mode = I2S_Mode_MasterTx;
      I2S_Init(SPI3, &I2SInitStruct);
      I2S_FullDuplexConfig(SPI3ext, &I2SInitStruct)
      I2S_Cmd(SPI3, ENABLE);
      I2S_Cmd(SPI3ext, ENABLE);
      ...
      while (SPI_I2S_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET)
      {}
      SPI_I2S_SendData(SPI3, txdata[i]);
      ...
      while (SPI_I2S_GetFlagStatus(I2S3ext, SPI_FLAG_RXNE) == RESET)
      {}
      rxdata[i] = SPI_I2S_ReceiveData(I2S3ext);
      ...

   (@) In I2S mode: if an external clock is used as source clock for the I2S,
       then the define I2S_EXTERNAL_CLOCK_VAL in file stm32f4xx_conf.h should
       be enabled and set to the value of the source clock frequency (in Hz).

   (@) In SPI mode: To use the SPI TI mode, call the function SPI_TIModeCmd()
       just after calling the function SPI_Init().

  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_spi.h"
#include "stm32f4xx_rcc.h"

/** @defgroup SPI
  * @brief SPI driver modules
  */

/* SPI registers Masks */
#define CR1_CLEAR_MASK            ((uint16_t)0x3040)
#define I2SCFGR_CLEAR_MASK        ((uint16_t)0xF040)

/* RCC PLLs masks */
#define PLLCFGR_PPLR_MASK         ((uint32_t)0x70000000)
#define PLLCFGR_PPLN_MASK         ((uint32_t)0x00007FC0)

#define SPI_CR2_FRF               ((uint16_t)0x0010)
#define SPI_SR_TIFRFE             ((uint16_t)0x0100)

///** @defgroup SPI_Group1 Initialization and Configuration functions
// *  @brief   Initialization and Configuration functions
// *
// ===============================================================================
//             ##### Initialization and Configuration functions #####
// ===============================================================================
// [..] This section provides a set of functions allowing to initialize the SPI
//      Direction, SPI Mode, SPI Data Size, SPI Polarity, SPI Phase, SPI NSS
//      Management, SPI Baud Rate Prescaler, SPI First Bit and SPI CRC Polynomial.
//
// [..] The SPI_Init() function follows the SPI configuration procedures for Master
//      mode and Slave mode (details for these procedures are available in reference
//      manual (RM0090)).
//
///**
//  * @brief  Initializes the SPIx peripheral according to the specified
//  *         parameters in the I2S_InitStruct.
//  * @param  SPIx: where x can be  2 or 3 to select the SPI peripheral (configured in I2S mode).
//  * @param  I2S_InitStruct: pointer to an I2S_InitTypeDef structure that
//  *         contains the configuration information for the specified SPI peripheral
//  *         configured in I2S mode.
//  *
//  * @note   The function calculates the optimal prescaler needed to obtain the most
//  *         accurate audio frequency (depending on the I2S clock source, the PLL values
//  *         and the product configuration). But in case the prescaler value is greater
//  *         than 511, the default value (0x02) will be configured instead.
//  *
//  * @note   if an external clock is used as source clock for the I2S, then the define
//  *         I2S_EXTERNAL_CLOCK_VAL in file stm32f4xx_conf.h should be enabled and set
//  *         to the value of the the source clock frequency (in Hz).
//  *
//  * @retval None
//  */
void I2S_Init(SPI_TypeDef* SPIx, I2S_InitTypeDef* I2S_InitStruct)
{
  uint16_t tmpreg = 0, i2sdiv = 2, i2sodd = 0, packetlength = 1;
  uint32_t tmp = 0, i2sclk = 0;
#ifndef I2S_EXTERNAL_CLOCK_VAL
  uint32_t pllm = 0, plln = 0, pllr = 0;
#endif /* I2S_EXTERNAL_CLOCK_VAL */

/*----------------------- SPIx I2SCFGR & I2SPR Configuration -----------------*/
  /* Clear I2SMOD, I2SE, I2SCFG, PCMSYNC, I2SSTD, CKPOL, DATLEN and CHLEN bits */
  SPIx->I2SCFGR &= I2SCFGR_CLEAR_MASK;
  SPIx->I2SPR = 0x0002;

  /* Get the I2SCFGR register value */
  tmpreg = SPIx->I2SCFGR;

  /* If the default value has to be written, reinitialize i2sdiv and i2sodd*/
  if(I2S_InitStruct->I2S_AudioFreq == I2S_AudioFreq_Default)
  {
    i2sodd = (uint16_t)0;
    i2sdiv = (uint16_t)2;
  }
  /* If the requested audio frequency is not the default, compute the prescaler */
  else
  {
    /* Check the frame length (For the Prescaler computing) *******************/
    if(I2S_InitStruct->I2S_DataFormat == I2S_DataFormat_16b)
    {
      /* Packet length is 16 bits */
      packetlength = 1;
    }
    else
    {
      /* Packet length is 32 bits */
      packetlength = 2;
    }

    /* Get I2S source Clock frequency  ****************************************/

    /* If an external I2S clock has to be used, this define should be set
       in the project configuration or in the stm32f4xx_conf.h file */
  #ifdef I2S_EXTERNAL_CLOCK_VAL
    /* Set external clock as I2S clock source */
    if ((RCC->CFGR & RCC_CFGR_I2SSRC) == 0)
    {
      RCC->CFGR |= (uint32_t)RCC_CFGR_I2SSRC;
    }

    /* Set the I2S clock to the external clock  value */
    i2sclk = I2S_EXTERNAL_CLOCK_VAL;

  #else /* There is no define for External I2S clock source */
    /* Set PLLI2S as I2S clock source */
    if ((RCC->CFGR & RCC_CFGR_I2SSRC) != 0)
    {
      RCC->CFGR &= ~(uint32_t)RCC_CFGR_I2SSRC;
    }

    /* Get the PLLI2SN value */
    plln = (uint32_t)(((RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SN) >> 6) & \
                      (RCC_PLLI2SCFGR_PLLI2SN >> 6));

    /* Get the PLLI2SR value */
    pllr = (uint32_t)(((RCC->PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SR) >> 28) & \
                      (RCC_PLLI2SCFGR_PLLI2SR >> 28));

    /* Get the PLLM value */
    pllm = (uint32_t)(RCC->PLLCFGR & RCC_PLLCFGR_PLLM);

    /* Get the I2S source clock value */
    i2sclk = (uint32_t)(((HSE_VALUE / pllm) * plln) / pllr);
  #endif /* I2S_EXTERNAL_CLOCK_VAL */

    /* Compute the Real divider depending on the MCLK output state, with a floating point */
    if(I2S_InitStruct->I2S_MCLKOutput == I2S_MCLKOutput_Enable)
    {
      /* MCLK output is enabled */
      tmp = (uint16_t)(((((i2sclk / 256) * 10) / I2S_InitStruct->I2S_AudioFreq)) + 5);
    }
    else
    {
      /* MCLK output is disabled */
      tmp = (uint16_t)(((((i2sclk / (32 * packetlength)) *10 ) / I2S_InitStruct->I2S_AudioFreq)) + 5);
    }

    /* Remove the flatting point */
    tmp = tmp / 10;

    /* Check the parity of the divider */
    i2sodd = (uint16_t)(tmp & (uint16_t)0x0001);

    /* Compute the i2sdiv prescaler */
    i2sdiv = (uint16_t)((tmp - i2sodd) / 2);

    /* Get the Mask for the Odd bit (SPI_I2SPR[8]) register */
    i2sodd = (uint16_t) (i2sodd << 8);
  }

  /* Test if the divider is 1 or 0 or greater than 0xFF */
  if ((i2sdiv < 2) || (i2sdiv > 0xFF))
  {
    /* Set the default values */
    i2sdiv = 2;
    i2sodd = 0;
  }

  /* Write to SPIx I2SPR register the computed value */
  SPIx->I2SPR = (uint16_t)((uint16_t)i2sdiv | (uint16_t)(i2sodd | (uint16_t)I2S_InitStruct->I2S_MCLKOutput));

  /* Configure the I2S with the SPI_InitStruct values */
  tmpreg |= (uint16_t)((uint16_t)SPI_I2SCFGR_I2SMOD | (uint16_t)(I2S_InitStruct->I2S_Mode | \
                  (uint16_t)(I2S_InitStruct->I2S_Standard | (uint16_t)(I2S_InitStruct->I2S_DataFormat | \
                  (uint16_t)I2S_InitStruct->I2S_CPOL))));

  /* Write to SPIx I2SCFGR */
  SPIx->I2SCFGR = tmpreg;
}

/**
  * @brief  Enables or disables the specified SPI peripheral (in I2S mode).
  * @param  SPIx: where x can be 2 or 3 to select the SPI peripheral (or I2Sxext
  *         for full duplex mode).
  * @param  NewState: new state of the SPIx peripheral.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void I2S_Cmd(SPI_TypeDef* SPIx, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    /* Enable the selected SPI peripheral (in I2S mode) */
    SPIx->I2SCFGR |= SPI_I2SCFGR_I2SE;
  }
  else
  {
    /* Disable the selected SPI peripheral in I2S mode */
    SPIx->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SE);
  }
}

/**
  * @brief  Configures the data size for the selected SPI.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @param  SPI_DataSize: specifies the SPI data size.
  *          This parameter can be one of the following values:
  *            @arg SPI_DataSize_16b: Set data frame format to 16bit
  *            @arg SPI_DataSize_8b: Set data frame format to 8bit
  * @retval None
  */
void SPI_DataSizeConfig(SPI_TypeDef* SPIx, uint16_t SPI_DataSize)
{
  /* Clear DFF bit */
  SPIx->CR1 &= (uint16_t)~SPI_DataSize_16b;
  /* Set new DFF bit value */
  SPIx->CR1 |= SPI_DataSize;
}

/**
  * @brief  Selects the data transfer direction in bidirectional mode for the specified SPI.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @param  SPI_Direction: specifies the data transfer direction in bidirectional mode.
  *          This parameter can be one of the following values:
  *            @arg SPI_Direction_Tx: Selects Tx transmission direction
  *            @arg SPI_Direction_Rx: Selects Rx receive direction
  * @retval None
  */
void SPI_BiDirectionalLineConfig(SPI_TypeDef* SPIx, uint16_t SPI_Direction)
{
  if (SPI_Direction == SPI_Direction_Tx)
  {
    /* Set the Tx only mode */
    SPIx->CR1 |= SPI_Direction_Tx;
  }
  else
  {
    /* Set the Rx only mode */
    SPIx->CR1 &= SPI_Direction_Rx;
  }
}

/**
  * @brief  Configures internally by software the NSS pin for the selected SPI.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @param  SPI_NSSInternalSoft: specifies the SPI NSS internal state.
  *          This parameter can be one of the following values:
  *            @arg SPI_NSSInternalSoft_Set: Set NSS pin internally
  *            @arg SPI_NSSInternalSoft_Reset: Reset NSS pin internally
  * @retval None
  */
void SPI_NSSInternalSoftwareConfig(SPI_TypeDef* SPIx, uint16_t SPI_NSSInternalSoft)
{
  if (SPI_NSSInternalSoft != SPI_NSSInternalSoft_Reset)
  {
    /* Set NSS pin internally by software */
    SPIx->CR1 |= SPI_NSSInternalSoft_Set;
  }
  else
  {
    /* Reset NSS pin internally by software */
    SPIx->CR1 &= SPI_NSSInternalSoft_Reset;
  }
}

/**
  * @brief  Enables or disables the SS output for the selected SPI.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @param  NewState: new state of the SPIx SS output.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI_SSOutputCmd(SPI_TypeDef* SPIx, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    /* Enable the selected SPI SS output */
    SPIx->CR2 |= (uint16_t)SPI_CR2_SSOE;
  }
  else
  {
    /* Disable the selected SPI SS output */
    SPIx->CR2 &= (uint16_t)~((uint16_t)SPI_CR2_SSOE);
  }
}

/**
  * @brief  Enables or disables the SPIx/I2Sx DMA interface.
  *
  * @note   This function can be called only after the SPI_Init() function has
  *         been called.
  * @note   When TI mode is selected, the control bits SSM, SSI, CPOL and CPHA
  *         are not taken into consideration and are configured by hardware
  *         respectively to the TI mode requirements.
  *
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6
  * @param  NewState: new state of the selected SPI TI communication mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI_TIModeCmd(SPI_TypeDef* SPIx, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    /* Enable the TI mode for the selected SPI peripheral */
    SPIx->CR2 |= SPI_CR2_FRF;
  }
  else
  {
    /* Disable the TI mode for the selected SPI peripheral */
    SPIx->CR2 &= (uint16_t)~SPI_CR2_FRF;
  }
}

/**
  * @brief  Configures the full duplex mode for the I2Sx peripheral using its
  *         extension I2Sxext according to the specified parameters in the
  *         I2S_InitStruct.
  * @param  I2Sxext: where x can be  2 or 3 to select the I2S peripheral extension block.
  * @param  I2S_InitStruct: pointer to an I2S_InitTypeDef structure that
  *         contains the configuration information for the specified I2S peripheral
  *         extension.
  *
  * @note   The structure pointed by I2S_InitStruct parameter should be the same
  *         used for the master I2S peripheral. In this case, if the master is
  *         configured as transmitter, the slave will be receiver and vice versa.
  *         Or you can force a different mode by modifying the field I2S_Mode to the
  *         value I2S_SlaveRx or I2S_SlaveTx indepedently of the master configuration.
  *
  * @note   The I2S full duplex extension can be configured in slave mode only.
  *
  * @retval None
  */
void I2S_FullDuplexConfig(SPI_TypeDef* I2Sxext, I2S_InitTypeDef* I2S_InitStruct)
{
  uint16_t tmpreg = 0, tmp = 0;

/*----------------------- SPIx I2SCFGR & I2SPR Configuration -----------------*/
  /* Clear I2SMOD, I2SE, I2SCFG, PCMSYNC, I2SSTD, CKPOL, DATLEN and CHLEN bits */
  I2Sxext->I2SCFGR &= I2SCFGR_CLEAR_MASK;
  I2Sxext->I2SPR = 0x0002;

  /* Get the I2SCFGR register value */
  tmpreg = I2Sxext->I2SCFGR;

  /* Get the mode to be configured for the extended I2S */
  if ((I2S_InitStruct->I2S_Mode == I2S_Mode_MasterTx) || (I2S_InitStruct->I2S_Mode == I2S_Mode_SlaveTx))
  {
    tmp = I2S_Mode_SlaveRx;
  }
  else
  {
    if ((I2S_InitStruct->I2S_Mode == I2S_Mode_MasterRx) || (I2S_InitStruct->I2S_Mode == I2S_Mode_SlaveRx))
    {
      tmp = I2S_Mode_SlaveTx;
    }
  }


  /* Configure the I2S with the SPI_InitStruct values */
  tmpreg |= (uint16_t)((uint16_t)SPI_I2SCFGR_I2SMOD | (uint16_t)(tmp | \
                  (uint16_t)(I2S_InitStruct->I2S_Standard | (uint16_t)(I2S_InitStruct->I2S_DataFormat | \
                  (uint16_t)I2S_InitStruct->I2S_CPOL))));

  /* Write to SPIx I2SCFGR */
  I2Sxext->I2SCFGR = tmpreg;
}

/** @defgroup SPI_Group2 Data transfers functions
 *  @brief   Data transfers functions
 *
 ===============================================================================
                      ##### Data transfers functions #####
 ===============================================================================

 [..] This section provides a set of functions allowing to manage the SPI data
      transfers. In reception, data are received and then stored into an internal
      Rx buffer while. In transmission, data are first stored into an internal Tx
      buffer before being transmitted.

 [..] The read access of the SPI_DR register can be done using the SPI_I2S_ReceiveData()
      function and returns the Rx buffered value. Whereas a write access to the SPI_DR
      can be done using SPI_I2S_SendData() function and stores the written data into
      Tx buffer.
  */

/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @retval The value of the received data.
  */
uint16_t SPI_I2S_ReceiveData(SPI_TypeDef* SPIx)
{
  /* Return the data in the DR register */
  return SPIx->DR;
}

/**
  * @brief  Transmits a Data through the SPIx/I2Sx peripheral.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  Data: Data to be transmitted.
  * @retval None
  */
void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data)
{
  /* Write in the DR register the data to be sent */
  SPIx->DR = Data;
}

/**
  * @}
  */

/** @defgroup SPI_Group3 Hardware CRC Calculation functions
 *  @brief   Hardware CRC Calculation functions
 *
 ===============================================================================
                 ##### Hardware CRC Calculation functions #####
 ===============================================================================

 [..] This section provides a set of functions allowing to manage the SPI CRC hardware
      calculation

 [..] SPI communication using CRC is possible through the following procedure:
   (#) Program the Data direction, Polarity, Phase, First Data, Baud Rate Prescaler,
       Slave Management, Peripheral Mode and CRC Polynomial values using the SPI_Init()
       function.
   (#) Enable the CRC calculation using the SPI_CalculateCRC() function.
   (#) Enable the SPI using the SPI_Cmd() function
   (#) Before writing the last data to the TX buffer, set the CRCNext bit using the
       SPI_TransmitCRC() function to indicate that after transmission of the last
       data, the CRC should be transmitted.
   (#) After transmitting the last data, the SPI transmits the CRC. The SPI_CR1_CRCNEXT
        bit is reset. The CRC is also received and compared against the SPI_RXCRCR
        value.
        If the value does not match, the SPI_FLAG_CRCERR flag is set and an interrupt
        can be generated when the SPI_I2S_IT_ERR interrupt is enabled.

 [..]
   (@) It is advised not to read the calculated CRC values during the communication.

   (@) When the SPI is in slave mode, be careful to enable CRC calculation only
       when the clock is stable, that is, when the clock is in the steady state.
       If not, a wrong CRC calculation may be done. In fact, the CRC is sensitive
       to the SCK slave input clock as soon as CRCEN is set, and this, whatever
       the value of the SPE bit.

   (@) With high bitrate frequencies, be careful when transmitting the CRC.
       As the number of used CPU cycles has to be as low as possible in the CRC
       transfer phase, it is forbidden to call software functions in the CRC
       transmission sequence to avoid errors in the last data and CRC reception.
       In fact, CRCNEXT bit has to be written before the end of the transmission/reception
       of the last data.

   (@) For high bit rate frequencies, it is advised to use the DMA mode to avoid the
       degradation of the SPI speed performance due to CPU accesses impacting the
       SPI bandwidth.

   (@) When the STM32F4xx is configured as slave and the NSS hardware mode is
       used, the NSS pin needs to be kept low between the data phase and the CRC
       phase.

   (@) When the SPI is configured in slave mode with the CRC feature enabled, CRC
       calculation takes place even if a high level is applied on the NSS pin.
       This may happen for example in case of a multi-slave environment where the
       communication master addresses slaves alternately.

   (@) Between a slave de-selection (high level on NSS) and a new slave selection
       (low level on NSS), the CRC value should be cleared on both master and slave
       sides in order to resynchronize the master and slave for their respective
       CRC calculation.

   (@) To clear the CRC, follow the procedure below:
       (#@) Disable SPI using the SPI_Cmd() function
       (#@) Disable the CRC calculation using the SPI_CalculateCRC() function.
       (#@) Enable the CRC calculation using the SPI_CalculateCRC() function.
       (#@) Enable SPI using the SPI_Cmd() function.
  */

/**
  * @brief  Enables or disables the CRC value calculation of the transferred bytes.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @param  NewState: new state of the SPIx CRC value calculation.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI_CalculateCRC(SPI_TypeDef* SPIx, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    /* Enable the selected SPI CRC calculation */
    SPIx->CR1 |= SPI_CR1_CRCEN;
  }
  else
  {
    /* Disable the selected SPI CRC calculation */
    SPIx->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_CRCEN);
  }
}

/**
  * @brief  Transmit the SPIx CRC value.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @retval None
  */
void SPI_TransmitCRC(SPI_TypeDef* SPIx)
{
  /* Enable the selected SPI CRC transmission */
  SPIx->CR1 |= SPI_CR1_CRCNEXT;
}

/**
  * @brief  Returns the transmit or the receive CRC register value for the specified SPI.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @param  SPI_CRC: specifies the CRC register to be read.
  *          This parameter can be one of the following values:
  *            @arg SPI_CRC_Tx: Selects Tx CRC register
  *            @arg SPI_CRC_Rx: Selects Rx CRC register
  * @retval The selected CRC register value..
  */
uint16_t SPI_GetCRC(SPI_TypeDef* SPIx, uint8_t SPI_CRC)
{
  uint16_t crcreg = 0;

  if (SPI_CRC != SPI_CRC_Rx)
  {
    /* Get the Tx CRC register */
    crcreg = SPIx->TXCRCR;
  }
  else
  {
    /* Get the Rx CRC register */
    crcreg = SPIx->RXCRCR;
  }
  /* Return the selected CRC register */
  return crcreg;
}

/**
  * @brief  Returns the CRC Polynomial register value for the specified SPI.
  * @param  SPIx: where x can be 1, 2, 3, 4, 5 or 6 to select the SPI peripheral.
  * @retval The CRC Polynomial register value.
  */
uint16_t SPI_GetCRCPolynomial(SPI_TypeDef* SPIx)
{
  return SPIx->CRCPR;               // Return the CRC polynomial register
}

/** @defgroup SPI_Group4 DMA transfers management functions
 *  @brief   DMA transfers management functions
  *
 ===============================================================================
                   ##### DMA transfers management functions #####
 ===============================================================================
  */

/**
  * @brief  Enables or disables the SPIx/I2Sx DMA interface.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_DMAReq: specifies the SPI DMA transfer request to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg SPI_I2S_DMAReq_Tx: Tx buffer DMA transfer request
  *            @arg SPI_I2S_DMAReq_Rx: Rx buffer DMA transfer request
  * @param  NewState: new state of the selected SPI DMA transfer request.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI_I2S_DMACmd(SPI_TypeDef* SPIx, uint16_t SPI_I2S_DMAReq, FunctionalState NewState)
{
    if(NewState != DISABLE)
    {
        SPIx->CR2 |= SPI_I2S_DMAReq;            // Enable the selected SPI DMA requests
    }
    else
    {
        SPIx->CR2 &= (uint16_t)~SPI_I2S_DMAReq; // Disable the selected SPI DMA requests
    }
}

/** @defgroup SPI_Group5 Interrupts and flags management functions
 *  @brief   Interrupts and flags management functions
  *
 ===============================================================================
            ##### Interrupts and flags management functions #####
 ===============================================================================

 [..] This section provides a set of functions allowing to configure the SPI Interrupts
      sources and check or clear the flags or pending bits status.
      The user should identify which mode will be used in his application to manage
      the communication: Polling mode, Interrupt mode or DMA mode.

 *** Polling Mode ***
 ====================
[..] In Polling Mode, the SPI/I2S communication can be managed by 9 flags:
  (#) SPI_I2S_FLAG_TXE : to indicate the status of the transmit buffer register
  (#) SPI_I2S_FLAG_RXNE : to indicate the status of the receive buffer register
  (#) SPI_I2S_FLAG_BSY : to indicate the state of the communication layer of the SPI.
  (#) SPI_FLAG_CRCERR : to indicate if a CRC Calculation error occur
  (#) SPI_FLAG_MODF : to indicate if a Mode Fault error occur
  (#) SPI_I2S_FLAG_OVR : to indicate if an Overrun error occur
  (#) I2S_FLAG_TIFRFE: to indicate a Frame Format error occurs.
  (#) I2S_FLAG_UDR: to indicate an Underrun error occurs.
  (#) I2S_FLAG_CHSIDE: to indicate Channel Side.

  (@) Do not use the BSY flag to handle each data transmission or reception. It is
      better to use the TXE and RXNE flags instead.

 [..] In this Mode it is advised to use the following functions:
   (+) FlagStatus SPI_I2S_GetFlagStatus(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG);
   (+) void SPI_I2S_ClearFlag(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG);

 *** Interrupt Mode ***
 ======================
 [..] In Interrupt Mode, the SPI communication can be managed by 3 interrupt sources
      and 7 pending bits:
   (+) Pending Bits:
       (##) SPI_I2S_IT_TXE : to indicate the status of the transmit buffer register
       (##) SPI_I2S_IT_RXNE : to indicate the status of the receive buffer register
       (##) SPI_IT_CRCERR : to indicate if a CRC Calculation error occur (available in SPI mode only)
       (##) SPI_IT_MODF : to indicate if a Mode Fault error occur (available in SPI mode only)
       (##) SPI_I2S_IT_OVR : to indicate if an Overrun error occur
       (##) I2S_IT_UDR : to indicate an Underrun Error occurs (available in I2S mode only).
       (##) I2S_FLAG_TIFRFE : to indicate a Frame Format error occurs (available in TI mode only).

   (+) Interrupt Source:
       (##) SPI_I2S_IT_TXE: specifies the interrupt source for the Tx buffer empty
            interrupt.
       (##) SPI_I2S_IT_RXNE : specifies the interrupt source for the Rx buffer not
            empty interrupt.
       (##) SPI_I2S_IT_ERR : specifies the interrupt source for the errors interrupt.

 [..] In this Mode it is advised to use the following functions:
   (+) void SPI_I2S_ITConfig(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT, FunctionalState NewState);
   (+) ITStatus SPI_I2S_GetITStatus(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT);
   (+) void SPI_I2S_ClearITPendingBit(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT);

 *** DMA Mode ***
 ================
 [..] In DMA Mode, the SPI communication can be managed by 2 DMA Channel requests:
   (#) SPI_I2S_DMAReq_Tx: specifies the Tx buffer DMA transfer request
   (#) SPI_I2S_DMAReq_Rx: specifies the Rx buffer DMA transfer request

 [..] In this Mode it is advised to use the following function:
   (+) void SPI_I2S_DMACmd(SPI_TypeDef* SPIx, uint16_t SPI_I2S_DMAReq, FunctionalState
       NewState);
  */

/**
  * @brief  Enables or disables the specified SPI/I2S interrupts.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_IT: specifies the SPI interrupt source to be enabled or disabled.
  *          This parameter can be one of the following values:
  *            @arg SPI_I2S_IT_TXE: Tx buffer empty interrupt mask
  *            @arg SPI_I2S_IT_RXNE: Rx buffer not empty interrupt mask
  *            @arg SPI_I2S_IT_ERR: Error interrupt mask
  * @param  NewState: new state of the specified SPI interrupt.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI_I2S_ITConfig(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT, FunctionalState NewState)
{
  uint16_t itpos = 0, itmask = 0 ;

  /* Get the SPI IT index */
  itpos = SPI_I2S_IT >> 4;

  /* Set the IT mask */
  itmask = (uint16_t)1 << (uint16_t)itpos;

  if (NewState != DISABLE)
  {
    /* Enable the selected SPI interrupt */
    SPIx->CR2 |= itmask;
  }
  else
  {
    /* Disable the selected SPI interrupt */
    SPIx->CR2 &= (uint16_t)~itmask;
  }
}

/**
  * @brief  Checks whether the specified SPIx/I2Sx flag is set or not.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_FLAG: specifies the SPI flag to check.
  *          This parameter can be one of the following values:
  *            @arg SPI_I2S_FLAG_TXE: Transmit buffer empty flag.
  *            @arg SPI_I2S_FLAG_RXNE: Receive buffer not empty flag.
  *            @arg SPI_I2S_FLAG_BSY: Busy flag.
  *            @arg SPI_I2S_FLAG_OVR: Overrun flag.
  *            @arg SPI_FLAG_MODF: Mode Fault flag.
  *            @arg SPI_FLAG_CRCERR: CRC Error flag.
  *            @arg SPI_I2S_FLAG_TIFRFE: Format Error.
  *            @arg I2S_FLAG_UDR: Underrun Error flag.
  *            @arg I2S_FLAG_CHSIDE: Channel Side flag.
  * @retval The new state of SPI_I2S_FLAG (SET or RESET).
  */
FlagStatus SPI_I2S_GetFlagStatus(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG)
{
  FlagStatus bitstatus = RESET;

  /* Check the status of the specified SPI flag */
  if ((SPIx->SR & SPI_I2S_FLAG) != (uint16_t)RESET)
  {
    /* SPI_I2S_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* SPI_I2S_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the SPI_I2S_FLAG status */
  return  bitstatus;
}

/**
  * @brief  Clears the SPIx CRC Error (CRCERR) flag.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_FLAG: specifies the SPI flag to clear.
  *          This function clears only CRCERR flag.
  *            @arg SPI_FLAG_CRCERR: CRC Error flag.
  *
  * @note   OVR (OverRun error) flag is cleared by software sequence: a read
  *          operation to SPI_DR register (SPI_I2S_ReceiveData()) followed by a read
  *          operation to SPI_SR register (SPI_I2S_GetFlagStatus()).
  * @note   UDR (UnderRun error) flag is cleared by a read operation to
  *          SPI_SR register (SPI_I2S_GetFlagStatus()).
  * @note   MODF (Mode Fault) flag is cleared by software sequence: a read/write
  *          operation to SPI_SR register (SPI_I2S_GetFlagStatus()) followed by a
  *          write operation to SPI_CR1 register (SPI_Cmd() to enable the SPI).
  *
  * @retval None
  */
void SPI_I2S_ClearFlag(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG)
{
  /* Clear the selected SPI CRC Error (CRCERR) flag */
  SPIx->SR = (uint16_t)~SPI_I2S_FLAG;
}

/**
  * @brief  Checks whether the specified SPIx/I2Sx interrupt has occurred or not.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_IT: specifies the SPI interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg SPI_I2S_IT_TXE: Transmit buffer empty interrupt.
  *            @arg SPI_I2S_IT_RXNE: Receive buffer not empty interrupt.
  *            @arg SPI_I2S_IT_OVR: Overrun interrupt.
  *            @arg SPI_IT_MODF: Mode Fault interrupt.
  *            @arg SPI_IT_CRCERR: CRC Error interrupt.
  *            @arg I2S_IT_UDR: Underrun interrupt.
  *            @arg SPI_I2S_IT_TIFRFE: Format Error interrupt.
  * @retval The new state of SPI_I2S_IT (SET or RESET).
  */
ITStatus SPI_I2S_GetITStatus(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT)
{
  ITStatus bitstatus = RESET;
  uint16_t itpos = 0, itmask = 0, enablestatus = 0;

  /* Get the SPI_I2S_IT index */
  itpos = 0x01 << (SPI_I2S_IT & 0x0F);

  /* Get the SPI_I2S_IT IT mask */
  itmask = SPI_I2S_IT >> 4;

  /* Set the IT mask */
  itmask = 0x01 << itmask;

  /* Get the SPI_I2S_IT enable bit status */
  enablestatus = (SPIx->CR2 & itmask) ;

  /* Check the status of the specified SPI interrupt */
  if (((SPIx->SR & itpos) != (uint16_t)RESET) && enablestatus)
  {
    /* SPI_I2S_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* SPI_I2S_IT is reset */
    bitstatus = RESET;
  }
  /* Return the SPI_I2S_IT status */
  return bitstatus;
}

/**
  * @brief  Clears the SPIx CRC Error (CRCERR) interrupt pending bit.
  * @param  SPIx: To select the SPIx/I2Sx peripheral, where x can be: 1, 2, 3, 4, 5 or 6
  *         in SPI mode or 2 or 3 in I2S mode or I2Sxext for I2S full duplex mode.
  * @param  SPI_I2S_IT: specifies the SPI interrupt pending bit to clear.
  *         This function clears only CRCERR interrupt pending bit.
  *            @arg SPI_IT_CRCERR: CRC Error interrupt.
  *
  * @note   OVR (OverRun Error) interrupt pending bit is cleared by software
  *          sequence: a read operation to SPI_DR register (SPI_I2S_ReceiveData())
  *          followed by a read operation to SPI_SR register (SPI_I2S_GetITStatus()).
  * @note   UDR (UnderRun Error) interrupt pending bit is cleared by a read
  *          operation to SPI_SR register (SPI_I2S_GetITStatus()).
  * @note   MODF (Mode Fault) interrupt pending bit is cleared by software sequence:
  *          a read/write operation to SPI_SR register (SPI_I2S_GetITStatus())
  *          followed by a write operation to SPI_CR1 register (SPI_Cmd() to enable
  *          the SPI).
  * @retval None
  */
void SPI_I2S_ClearITPendingBit(SPI_TypeDef* SPIx, uint8_t SPI_I2S_IT)
{
  uint16_t itpos = 0;

  /* Get the SPI_I2S IT index */
  itpos = 0x01 << (SPI_I2S_IT & 0x0F);

  /* Clear the selected SPI CRC Error (CRCERR) interrupt pending bit */
  SPIx->SR = (uint16_t)~itpos;
}


#endif


