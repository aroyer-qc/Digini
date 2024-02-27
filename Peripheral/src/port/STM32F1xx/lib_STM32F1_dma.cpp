//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F1_dma.cpp
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

#define DMA_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  DMA_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_ClearFlag
//
//  Parameter(s):   pDMA        DMA channel to modify flag
//                  Flag        to clear
//  Return:         None
//
//  Description:    Clear flag for specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_ClearFlag(DMA_Channel_TypeDef* pDMA, uint32_t Flag)
{
    volatile uint32_t* pRegister = nullptr;

    if((intptr_t(pDMA) & DMA1_BASE) == DMA1_BASE)
    {
        pRegister = reinterpret_cast<volatile uint32_t*>(&DMA1->IFCR);
    }

  #if (DMA2_SUPPORT == DEF_ENABLED)
    else if((intptr_t(pDMA) & DMA2_BASE) == DMA2_BASE)
    {
        pRegister = reinterpret_cast<volatile uint32_t*>(&DMA2->IFCR);
    }
  #endif

    if(pRegister != nullptr)
    {
        *pRegister |= Flag;
//        SET_BIT(*pRegister, Flag);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_CheckFlag
//
//  Parameter(s):   pDMA        DMA channel to check flag
//                  Flag        To check
//  Return:         None
//
//  Description:    Check flag for specific DMA channel.
//-------------------------------------------------------------------------------------------------
uint32_t DMA_CheckFlag(DMA_Channel_TypeDef* pDMA, uint32_t Flag)
{
    uint32_t Register = 0;
    uint32_t Result   = 0;

    if((intptr_t(pDMA) & DMA1_BASE) == DMA1_BASE)
    {
        Register = DMA1->ISR;
    }

  #if (DMA2_SUPPORT == DEF_ENABLED)
    else if((pDMA & DMA2_BASE) == DMA2_BASE)
    {
        Register = DMA2->ISR;
    }
  #endif

    if((Register & Flag) != 0)
    {
        Result = 1;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Enable
//
//  Parameter(s):   pDMA        DMA channel to enable
//  Return:         None
//
//  Description:    Enable a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_Enable(DMA_Channel_TypeDef* pDMA)
{
    SET_BIT(pDMA->CCR, DMA_CCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Disable
//
//  Parameter(s):   pDMA        DMA channel to disable
//  Return:         None
//
//  Description:    Disable a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_Disable(DMA_Channel_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CCR, DMA_CCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableInterrupt
//
//  Parameter(s):   pDMA        DMA channel
//                  Interrupt   Interrupt tn enable
//  Return:         None
//
//  Description:    Enable a group of interrupt for specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_EnableInterrupt(DMA_Channel_TypeDef* pDMA, uint32_t Interrupt)
{
    SET_BIT(pDMA->CCR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableInterrupt
//
//  Parameter(s):   pDMA        DMA channel
//                  Interrupt   Interrupt tn enable
//  Return:         None
//
//  Description:    Disable a group of interrupt for specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_DisableInterrupt(DMA_Channel_TypeDef* pDMA, uint32_t Interrupt)
{
    CLEAR_BIT(pDMA->CCR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableTransmitCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA channel to enable
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_EnableTransmitCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    SET_BIT(pDMA->CCR, DMA_CCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableTransmitCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA channel to enable
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA channel.
//-------------------------------------------------------------------------------------------------
void DMA_DisableTransmitCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CCR, DMA_CCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_EnableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA stream to enable
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_EnableTransmitHalfCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    SET_BIT(pDMA->CCR, DMA_CCR_HTIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_DisableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   pDMA        DMA stream to enable
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA stream.
//-------------------------------------------------------------------------------------------------
void DMA_DisableTransmitHalfCompleteInterrupt(DMA_Channel_TypeDef* pDMA)
{
    CLEAR_BIT(pDMA->CCR, DMA_CCR_HTIE);
}

//---------------------------------------------------------------------------------------------------------------------------------




#if 0
    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_MEDIUM;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);



/**
  * @brief  Initialize the DMA according to the specified
  *         parameters in the DMA_InitTypeDef and initialize the associated handle.
  * @param  hdma: Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA Channel.
  * @retval HAL status
  */
 HAL_DMA_Init(DMA_HandleTypeDef *hdma)
{
  if ((uint32_t)(hdma->Instance) < (uint32_t)(DMA2_Channel1))     /* calculation of the channel index */
  { /* DMA1 */
    hdma->ChannelIndex = (((uint32_t)hdma->Instance - (uint32_t)DMA1_Channel1) / ((uint32_t)DMA1_Channel2 - (uint32_t)DMA1_Channel1)) << 2;
    hdma->DmaBaseAddress = DMA1;
  }

  else
  { /* DMA2 */
    hdma->ChannelIndex = (((uint32_t)hdma->Instance - (uint32_t)DMA2_Channel1) / ((uint32_t)DMA2_Channel2 - (uint32_t)DMA2_Channel1)) << 2;
    hdma->DmaBaseAddress = DMA2;
  }

  tmp = hdma->Instance->CCR;    /* Get the CR register value */
  tmp &= ((uint32_t)~(DMA_CCR_PL    | DMA_CCR_MSIZE  | DMA_CCR_PSIZE  | \
                      DMA_CCR_MINC  | DMA_CCR_PINC   | DMA_CCR_CIRC   | \
                      DMA_CCR_DIR));  /* Clear PL, MSIZE, PSIZE, MINC, PINC, CIRC and DIR bits */
  tmp |=  hdma->Init.Direction        |
          hdma->Init.PeriphInc           | hdma->Init.MemInc           |
          hdma->Init.PeriphDataAlignment | hdma->Init.MemDataAlignment |
          hdma->Init.Mode                | hdma->Init.Priority;  /* Prepare the DMA Channel configuration */
  hdma->Instance->CCR = tmp;  /* Write to DMA Channel CR register */
}

/**
  * @brief  Start the DMA Transfer.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA Channel.
  * @param  SrcAddress: The source memory Buffer address
  * @param  DstAddress: The destination memory Buffer address
  * @param  DataLength: The length of data to be transferred from source to destination
  * @retval HAL status
  */
 HAL_DMA_Start(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
    __HAL_DMA_DISABLE(hdma);    /* Disable the peripheral */
    DMA_SetConfig(hdma, SrcAddress, DstAddress, DataLength);    /* Configure the source, destination address and the data length & clear flags*/
    __HAL_DMA_ENABLE(hdma);         /* Enable the Peripheral */
 }

/**
  * @brief  Handles DMA interrupt request.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA Channel.
  * @retval None
  */
void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
  uint32_t flag_it = hdma->DmaBaseAddress->ISR;
  uint32_t source_it = hdma->Instance->CCR;

  /* Half Transfer Complete Interrupt management ******************************/
  if (((flag_it & (DMA_FLAG_HT1 << hdma->ChannelIndex)) != RESET) && ((source_it & DMA_IT_HT) != RESET))
  {
    /* Disable the half transfer interrupt if the DMA mode is not CIRCULAR */
    if((hdma->Instance->CCR & DMA_CCR_CIRC) == 0U)
    {
      /* Disable the half transfer interrupt */
      __HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT);
    }
    /* Clear the half transfer complete flag */
    __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_HT_FLAG_INDEX(hdma));

    /* DMA peripheral state is not updated in Half Transfer */
    /* but in Transfer Complete case */

    if(hdma->XferHalfCpltCallback != NULL)
    {
      /* Half transfer callback */
      hdma->XferHalfCpltCallback(hdma);
    }
  }

  /* Transfer Complete Interrupt management ***********************************/
  else if (((flag_it & (DMA_FLAG_TC1 << hdma->ChannelIndex)) != RESET) && ((source_it & DMA_IT_TC) != RESET))
  {
    if((hdma->Instance->CCR & DMA_CCR_CIRC) == 0U)
    {
      /* Disable the transfer complete and error interrupt */
      __HAL_DMA_DISABLE_IT(hdma, DMA_IT_TE | DMA_IT_TC);

      /* Change the DMA state */
      hdma->State = HAL_DMA_STATE_READY;
    }
    /* Clear the transfer complete flag */
      __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));

    /* Process Unlocked */
    __HAL_UNLOCK(hdma);

    if(hdma->XferCpltCallback != NULL)
    {
      /* Transfer complete callback */
      hdma->XferCpltCallback(hdma);
    }
  }

  /* Transfer Error Interrupt management **************************************/
  else if (( RESET != (flag_it & (DMA_FLAG_TE1 << hdma->ChannelIndex))) && (RESET != (source_it & DMA_IT_TE)))
  {
    /* When a DMA transfer error occurs */
    /* A hardware clear of its EN bits is performed */
    /* Disable ALL DMA IT */
    __HAL_DMA_DISABLE_IT(hdma, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));

    /* Clear all flags */
    hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << hdma->ChannelIndex);

    /* Update error code */
    hdma->ErrorCode = HAL_DMA_ERROR_TE;

    /* Change the DMA state */
    hdma->State = HAL_DMA_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hdma);

    if (hdma->XferErrorCallback != NULL)
    {
      /* Transfer error callback */
      hdma->XferErrorCallback(hdma);
    }
  }
  return;
}

/**
  * @brief  Sets the DMA Transfer parameter.
  * @param  hdma:       pointer to a DMA_HandleTypeDef structure that contains
  *                     the configuration information for the specified DMA Channel.
  * @param  SrcAddress: The source memory Buffer address
  * @param  DstAddress: The destination memory Buffer address
  * @param  DataLength: The length of data to be transferred from source to destination
  * @retval HAL status
  */
static void DMA_SetConfig(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
  hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << hdma->ChannelIndex);  /* Clear all flags */
  hdma->Instance->CNDTR = DataLength;  /* Configure DMA Channel data length */

  if((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH)  /* Memory to Peripheral */
  {
    hdma->Instance->CPAR = DstAddress;    /* Configure DMA Channel destination address */
    hdma->Instance->CMAR = SrcAddress;    /* Configure DMA Channel source address */
  }
  /* Peripheral to Memory */
  else
  {
    hdma->Instance->CPAR = SrcAddress;      /* Configure DMA Channel source address */
    hdma->Instance->CMAR = DstAddress;      /* Configure DMA Channel destination address */
  }
}


#endif
