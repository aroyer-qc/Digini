//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_dac.cpp
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

//-------------------------------------------------------------------------------------------------

#if (USE_DAC_DRIVER == DEF_ENABLED)

#include "dac_var.h"

//-------------------------------------------------------------------------------------------------
//
//  Function name:  Initialize
//
//  Parameter(s):   none.
//
//  Return:         none.
//
//  Description:
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::Initialize(void)
{
#if 0
  /* Note: Hardware constraint (refer to description of this function)        */
  /*       DAC instance must be disabled.                                     */
    if (DAC_InitStruct->WaveAutoGeneration != LL_DAC_WAVE_AUTO_GENERATION_NONE)
    {
      MODIFY_REG(DAC->CR,
                 (DAC_CR_TSEL1
                  | DAC_CR_WAVE1
                  | DAC_CR_MAMP1
                  | DAC_CR_BOFF1
                 ) << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                 ,
                 (DAC_InitStruct->TriggerSource
                  | DAC_InitStruct->WaveAutoGeneration
                  | DAC_InitStruct->WaveAutoGenerationConfig
                  | DAC_InitStruct->OutputBuffer
                 ) << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                );
    }
    else
    {
      MODIFY_REG(DACx->CR,
                 (DAC_CR_TSEL1
                  | DAC_CR_WAVE1
                  | DAC_CR_BOFF1
                 ) << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                 ,
                 (DAC_InitStruct->TriggerSource
                  | LL_DAC_WAVE_AUTO_GENERATION_NONE
                  | DAC_InitStruct->OutputBuffer
                 ) << (DAC_Channel & DAC_CR_CHX_BITOFFSET_MASK)
                );
    }
  }
 #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Start
//
//   Parameter(s):  uint8_t     Channel
//   Return value:  none
//
//   Description:   Will start the DAC specified channel.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::Start(uint8_t Channel)
{
    DAC->CR |= DAC_CR_EN1 << Channel;

  if(Channel == DAC_CHANNEL_1)
  {
        if((DAC->CR & (DAC_CR_TEN1 | DAC_CR_TSEL1)) == DAC_TRIGGER_SOFTWARE)                            // Check if software trigger enabled
        {
            SET_BIT(DAC->SWTRIGR, DAC_SWTRIGR_SWTRIG1);                                                 // Enable the selected DAC software conversion
        }
    }
    else // (Channel == DAC_CHANNEL_2)
    {
        if((DAC->CR & (DAC_CR_TEN2 | DAC_CR_TSEL2)) == (DAC_TRIGGER_SOFTWARE << (Channel & 0x10UL)))    // Check if software trigger enabled
        {
            DAC->SWTRIGR != DAC_SWTRIGR_SWTRIG2;                                                        // Enable the selected DAC software conversion
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Stop
//
//   Parameter(s):  uint8_t     Channel
//
//   Return value:  none
//
//   Description:   Will stop the DAC specified channel.
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::Stop(uint8_t Channel)
{
    DAC->CR &=  ~(DAC_CR_EN1 << Channel);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Stop
//
//   Parameter(s):  uint8_t     Channel         (DAC_CHANNEL_1 or DAC_CHANNEL_2)
//                  uint16_t    Data            Data to be loaded in the channel.
//                  uint32_t    Alignment
//                                  Value are:
//                                      DAC_ALIGN_8B_RIGHT    8 bits right data alignment
//                                      DAC_ALIGN_12B_LEFT    12 bits left data alignment
//                                      DAC_ALIGN_12B_RIGHT   12 bits right data alignment
//
//   Return value:  none
//
//   Description:    Set the specified value for DAC channel
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::SetValue(uint8_t Channel, uint16_t Data, uint32_t Alignment)
{
    volatile uint32_t DAC_Register;


    if(Channel == DAC_CHANNEL_1)
    {
        DAC_Register = (&DAC->DHR12R1) + Alignment;
    }
    else // (Channel == DAC_CHANNEL_2)
    {
        DAC_Register = (&DAC->DHR12R2) + Alignment;
    }

    *(uint32_t*)DAC_Register = Data;        // Set the DAC channel data
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Start_DMA
//
//   Parameter(s):  uint8_t     Channel
//
//   Return value:  none
//
//   Description:
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::Start_DMA(uint8_t Channel, uint16_t *pData, size_t Length, uint32_t Alignment)
{
    uint32_t tmpreg = 0U;

    if(Channel == DAC_CHANNEL_1)
    {
        //hdac->DMA_Handle1->XferCpltCallback = DAC_DMAConvCpltCh1;               /* Set the DMA transfer complete callback for channel1 */
        //hdac->DMA_Handle1->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh1;       /* Set the DMA half transfer complete callback for channel1 */
        //hdac->DMA_Handle1->XferErrorCallback = DAC_DMAErrorCh1;                 /* Set the DMA error callback for channel1 */

        SET_BIT(DAC->CR, DAC_CR_DMAEN1);                             /* Enable the selected DAC channel1 DMA request */

        //Case of use of channel 1
        switch(Alignment)
        {
            case DAC_ALIGN_12B_RIGHT:
                // Get DHR12R1 address
                tmpreg = (uint32_t)&DAC->DHR12R1;
            break;

            case DAC_ALIGN_12B_LEFT:
                // Get DHR12L1 address
                tmpreg = (uint32_t)&DAC->DHR12L1;
                break;

            case DAC_ALIGN_8B_RIGHT:
                // Get DHR8R1 address
                tmpreg = (uint32_t)&DAC->DHR8R1;
            break;

            default:
            break;
        }
    }
    else
    {
       // hdac->DMA_Handle2->XferCpltCallback = DAC_DMAConvCpltCh2;                   // Set the DMA transfer complete callback for channel2
       // hdac->DMA_Handle2->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh2;    // Set the DMA half transfer complete callback for channel2
       // hdac->DMA_Handle2->XferErrorCallback = DAC_DMAErrorCh2;    // Set the DMA error callback for channel2 */
        SET_BIT(DAC->CR, DAC_CR_DMAEN2);    // Enable the selected DAC channel2 DMA request

        // Case of use of channel 2
        switch (Alignment)
        {
            case DAC_ALIGN_12B_RIGHT:
                /* Get DHR12R2 address */
                tmpreg = (uint32_t)&DAC->DHR12R2;
            break;

            case DAC_ALIGN_12B_LEFT:
                /* Get DHR12L2 address */
                tmpreg = (uint32_t)&DAC->DHR12L2;
            break;

            case DAC_ALIGN_8B_RIGHT:
                /* Get DHR8R2 address */
                tmpreg = (uint32_t)&DAC->DHR8R2;
            break;

            default:
            break;
        }
    }

    /* Enable the DMA Stream */
    if(Channel == DAC_CHANNEL_1)
    {
        /* Enable the DAC DMA underrun interrupt */
        //__HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR1);

        /* Enable the DMA Stream */
        //status = HAL_DMA_Start_IT(hdac->DMA_Handle1, (uint32_t)pData, tmpreg, Length);
    }
    else
    {
        /* Enable the DAC DMA underrun interrupt */
        //__HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR2);

        /* Enable the DMA Stream */
        //status = HAL_DMA_Start_IT(hdac->DMA_Handle2, (uint32_t)pData, tmpreg, Length);
    }

   // if(status == HAL_OK)
    {
        /* Enable the Peripheral */
        //__HAL_DAC_ENABLE(hdac, Channel);
    }
  //  else
    {
        //hdac->ErrorCode |= HAL_DAC_ERROR_DMA;
    }

    /* Return function status */
    //return status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Stop_DMA
//
//   Parameter(s):  uint8_t     Channel
//
//   Return value:  none
//
//   Description:   Disables DAC and stop conversion of channel.
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::Stop_DMA(uint8_t Channel)
{
    DMA_Stream_TypeDef* pDMAx;

    DAC->CR &= ~((DAC_CR_DMAEN1 | DAC_CR_EN1 | DAC_CR_DMAUDRIE1) << Channel);      // Disable the selected DAC channel DMA request, DAC DMA underrun interrupt and peripheral
   // TODO  pDMAx = (Channel == DAC_CHANNEL_1) ? 0 : 1;
    DMA_DisableInterrupt(pDMAx, DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE);
    DMA_Disable(pDMAx);
    //DMA_ClearFlag(pDMAx, DMA_ISR_GIF1);
}

//-------------------------------------------------------------------------------------------------
//
//  Function name:
//
//  Parameter(s):   none.
//
//  Return:         none.
//
//  Description:    Handle DAC streaming.
//
//  Note(s):        1. This is a ISR.
//
//-------------------------------------------------------------------------------------------------

/**
  * @brief  Handles DAC interrupt request
  *         This function uses the interruption of DMA
  *         underrun.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
  #if 0
void HAL_DAC_IRQHandler(DAC_HandleTypeDef *hdac)
{
  if (__HAL_DAC_GET_IT_SOURCE(hdac, DAC_IT_DMAUDR1))
  {
    /* Check underrun flag of DAC channel 1 */
    if (__HAL_DAC_GET_FLAG(hdac, DAC_FLAG_DMAUDR1))
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;

      /* Set DAC error code to channel1 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH1);

      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac, DAC_FLAG_DMAUDR1);

      /* Disable the selected DAC channel1 DMA request */
      CLEAR_BIT(hdac->Instance->CR, DAC_CR_DMAEN1);

      /* Error callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
      hdac->DMAUnderrunCallbackCh1(hdac);
#else
      HAL_DAC_DMAUnderrunCallbackCh1(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
    }
  }

#if defined(DAC_CHANNEL2_SUPPORT)
  if (__HAL_DAC_GET_IT_SOURCE(hdac, DAC_IT_DMAUDR2))
  {
    /* Check underrun flag of DAC channel 2 */
    if (__HAL_DAC_GET_FLAG(hdac, DAC_FLAG_DMAUDR2))
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;

      /* Set DAC error code to channel2 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH2);

      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac, DAC_FLAG_DMAUDR2);

      /* Disable the selected DAC channel2 DMA request */
      CLEAR_BIT(hdac->Instance->CR, DAC_CR_DMAEN2);

      /* Error callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
      hdac->DMAUnderrunCallbackCh2(hdac);
#else
      HAL_DACEx_DMAUnderrunCallbackCh2(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
    }
  }
#endif /* DAC_CHANNEL2_SUPPORT */
}
#endif

void DAC_ISR_Handler(void)
{
}



















/**
  * @brief  Configures the selected DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  sConfig DAC configuration structure.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
  #if 0
HAL_StatusTypeDef HAL_DAC_ConfigChannel(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel)
{
  uint32_t tmpreg1;
  uint32_t tmpreg2;

  /* Check the DAC parameters */
  assert_param(IS_DAC_TRIGGER(sConfig->DAC_Trigger));
  assert_param(IS_DAC_OUTPUT_BUFFER_STATE(sConfig->DAC_OutputBuffer));
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  /* Get the DAC CR value */
  tmpreg1 = hdac->Instance->CR;
  /* Clear BOFFx, TENx, TSELx, WAVEx and MAMPx bits */
  tmpreg1 &= ~(((uint32_t)(DAC_CR_MAMP1 | DAC_CR_WAVE1 | DAC_CR_TSEL1 | DAC_CR_TEN1 | DAC_CR_BOFF1)) << (Channel & 0x10UL));
  /* Configure for the selected DAC channel: buffer output, trigger */
  /* Set TSELx and TENx bits according to DAC_Trigger value */
  /* Set BOFFx bit according to DAC_OutputBuffer value */
  tmpreg2 = (sConfig->DAC_Trigger | sConfig->DAC_OutputBuffer);
  /* Calculate CR register value depending on DAC_Channel */
  tmpreg1 |= tmpreg2 << (Channel & 0x10UL);
  /* Write to DAC CR */
  hdac->Instance->CR = tmpreg1;
  /* Disable wave generation */
  CLEAR_BIT(hdac->Instance->CR, (DAC_CR_WAVE1 << (Channel & 0x10UL)));

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}





HAL_StatusTypeDef HAL_DAC_RegisterCallback(DAC_HandleTypeDef *hdac, HAL_DAC_CallbackIDTypeDef CallbackID,
                                           pDAC_CallbackTypeDef pCallback)

#endif


//-------------------------------------------------------------------------------------------------

#endif // (USE_DAC_DRIVER == DEF_ENABLED)

