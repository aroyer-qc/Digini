//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_dma.cpp
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
//  Function:       Initialize
//
//  Parameter(s):   DMA_ID_e        DMA_ID
//  Return:         None
//
//  Description:    Initialize the DMA pointer 
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Initialize(DMA_ID_e DMA_ID)
{
    // link to physical device;
    //SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN)
    //SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA2EN)
    
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetChannelRX
//
//  Parameter(s):   Source
//                  Destination
//                  Length
//  Return:         None
//
//  Description:    Setup receive data from peripheral for specific DMA channel.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetChannelRX(void* pSource, void* pDestination, size_t Length)
{
    m_pDMA->M0AR = uint32_t(pDestination);
    m_pDMA->PAR  = uint32_t(pSource);
    m_pDMA->NDTR = uint32_t(Length);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetChannelTX
//
//  Parameter(s):   Source
//                  Destination
//                  Length
//  Return:         None
//
//  Description:    Setup transmit data to peripheral for specific DMA channel.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::SetChannelTX(void* pSource, void* pDestination, size_t Length)
{
    m_pDMA->M0AR = uint32_t(pSource);
    m_pDMA->PAR  = uint32_t(pDestination);
    m_pDMA->NDTR = uint32_t(Length);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ClearFlag
//
//  Parameter(s):   Flag        to clear
//  Return:         None
//
//  Description:    Clear flag for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::ClearFlag(uint32_t Flag)
{
    volatile uint32_t* pRegister;

    switch(intptr_t(pDMA))
    {
        case (uint32_t)DMA1_Channel1_BASE:
        case (uint32_t)DMA1_Channel2_BASE:
        case (uint32_t)DMA1_Channel3_BASE:
        case (uint32_t)DMA1_Channel4_BASE: pRegister = &DMA1->LIFCR; break;

        case (uint32_t)DMA1_Channel5_BASE:
        case (uint32_t)DMA1_Channel6_BASE:
        case (uint32_t)DMA1_Channel7_BASE: pRegister = &DMA1->HIFCR; break;

        case (uint32_t)DMA2_Channel1_BASE:
        case (uint32_t)DMA2_Channel2_BASE:
        case (uint32_t)DMA2_Channel3_BASE:
        case (uint32_t)DMA2_Channel4_BASE: pRegister = &DMA2->LIFCR; break;

        case (uint32_t)DMA2_Channel5_BASE: pRegister = &DMA2->HIFCR; break;
    }

    SET_BIT(*pRegister, Flag);}

//-------------------------------------------------------------------------------------------------
//
//  Function:       CheckFlag
//
//  Parameter(s):   Flag        To check
//  Return:         None
//
//  Description:    Check flag for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
uint32_t DMA_Driver::CheckFlag(uint32_t Flag)
{
    volatile uint32_t Register;
    uint32_t          Result     = 0;

    switch(intptr_t(m_pDMA))
    {
        case (uint32_t)DMA1_Stream0_BASE:
        case (uint32_t)DMA1_Stream1_BASE:
        case (uint32_t)DMA1_Stream2_BASE:
        case (uint32_t)DMA1_Stream3_BASE: Register = DMA1->LISR; break;

        case (uint32_t)DMA1_Stream4_BASE:
        case (uint32_t)DMA1_Stream5_BASE:
        case (uint32_t)DMA1_Stream6_BASE:
        case (uint32_t)DMA1_Stream7_BASE: Register = DMA1->HISR; break;

        case (uint32_t)DMA2_Stream0_BASE:
        case (uint32_t)DMA2_Stream1_BASE:
        case (uint32_t)DMA2_Stream2_BASE:
        case (uint32_t)DMA2_Stream3_BASE: Register = DMA2->LISR; break;

        case (uint32_t)DMA2_Stream4_BASE:
        case (uint32_t)DMA2_Stream5_BASE:
        case (uint32_t)DMA2_Stream6_BASE:
        case (uint32_t)DMA2_Stream7_BASE: Register = DMA2->HISR; break;
    }

    if((Register & Flag) != 0)
    {
        Result = 1;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Enable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Enable(void)
{
    SET_BIT(m_pDMA->CR, DMA_SxCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Disable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::Disable(void)
{
    CLEAR_BIT(pDMA->CR, DMA_SxCR_EN);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableInterrupt
//
//  Parameter(s):   Interrupt   Interrupt to enable
//  Return:         None
//
//  Description:    Enable a group of interrupt for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableInterrupt(uint32_t Interrupt)
{
    SET_BIT(m_pDMA->CR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisableInterrupt
//
//  Parameter(s):   Interrupt   Interrupt to enable
//  Return:         None
//
//  Description:    Disable a group of interrupt for specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::DisableInterrupt(uint32_t Interrupt)
{
    CLEAR_BIT(m_pDMA->CR, Interrupt);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableTransmitCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableTransmitCompleteInterrupt((void)
{
    SET_BIT(m_pDMA->CR, DMA_SxCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisableTransmitCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::DisableTransmitCompleteInterrupt(void)
{
    CLEAR_BIT(m_pDMA->CR, DMA_SxCR_TCIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       EnableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::EnableTransmitHalfCompleteInterrupt(void)
{
    SET_BIT(m_pDMA->CR, DMA_SxCR_HTIE);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DisableTransmitHalfCompleteInterrupt
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable the transmit complete interrupt for a specific DMA stream.
//
//-------------------------------------------------------------------------------------------------
void DMA_Driver::DisableTransmitHalfCompleteInterrupt(void)
{
    CLEAR_BIT(m_pDMA->CR, DMA_SxCR_HTIE);
}

//-------------------------------------------------------------------------------------------------










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
  * @brief  Start the DMA Transfer with interrupt enabled.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA Channel.
  * @param  SrcAddress: The source memory Buffer address
  * @param  DstAddress: The destination memory Buffer address
  * @param  DataLength: The length of data to be transferred from source to destination
  * @retval HAL status
  */
HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
    __HAL_DMA_DISABLE(hdma);        /* Disable the peripheral */
    DMA_SetConfig(hdma, SrcAddress, DstAddress, DataLength);        /* Configure the source, destination address and the data length & clear flags*/
    
    /* Enable the transfer complete interrupt */
    /* Enable the transfer Error interrupt */
    if(NULL != hdma->XferHalfCpltCallback)
    {
      __HAL_DMA_ENABLE_IT(hdma, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));         /* Enable the Half transfer complete interrupt as well */
    }
    else
    {
      __HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT);
      __HAL_DMA_ENABLE_IT(hdma, (DMA_IT_TC | DMA_IT_TE));
    }
    __HAL_DMA_ENABLE(hdma);     /* Enable the Peripheral */
  }
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
