//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_dac.cpp
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "BSP_board.h"

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  constructor
//  Parameter(s):   None
//
//  Description:    Initialize the DAC on the stm32H5xx.
//
//  Note(s):        It is initialize at first call.
//
//-------------------------------------------------------------------------------------------------
DAC_Driver::DAC_Driver()
{
	m_IsItInitialize = false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  None
//  Return:         SystemState_e
//
//  Description:    It start the initialize process from configuration in struct
//
//-------------------------------------------------------------------------------------------------
SystemState_e DAC_Driver::Initialize(void)
{
	uint32_t ClkFrequency;
	uint32_t HighFrequencyInterface;

	if(m_IsItInitialize == true)
	{
		return SYS_ERROR;
	}

	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN);

	// Select the DAC clock source
	// Clock Selection done in system_stm32h5xx.c

	IO_PinInit(IO_DAC_OUT_1);
	IO_PinInit(IO_DAC_OUT_2);
	m_IsItInitialize = true;

	DAC1_NS->CR = 0;														// Make sure DAC is reset

  #if (DAC_CFG_SAMPLE_AND_HOLD == DAC_SAMPLE_AND_HOLD_ENABLE)	
	// Channel 1
	while((DAC1_NS->SR & DAC_SR_BWST1) != 0) {}								// is it busy writing sample time ?
	DAC1_NS->SHSR1 = DAC_SAMPLE_TIME;

	// Channel 2
	while((DAC1_NS->SR & DAC_SR_BWST2) != 0) {}								// is it busy writing sample time ?
	DAC1_NS->SHSR2 = DAC_SAMPLE_TIME;
  
	MODIFY_REG(DAC1_NS->SHHR, (DAC_SHHR_THOLD1_Msk    | DAC_SHHR_THOLD2_Msk),    (DAC_HOLD_TIME    | (DAC_HOLD_TIME    << DAC_SHHR_THOLD2_Pos)));
	MODIFY_REG(DAC1_NS->SHRR, (DAC_SHRR_TREFRESH1_Msk | DAC_SHRR_TREFRESH2_Msk), (DAC_REFRESH_TIME | (DAC_REFRESH_TIME << DAC_SHRR_TREFRESH2_Pos)));
  #endif

	ClkFrequency = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_ADCDAC);

	if      (ClkFrequency >= 160000000) HighFrequencyInterface = DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_160MHZ;
	else if (ClkFrequency >= 80000000)  HighFrequencyInterface = DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ;
	else								HighFrequencyInterface = DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE;

  	DAC1_NS->MCR = DAC_CFG_SAMPLE_AND_HOLD | DAC_CFG_OUTPUT_BUFFER | HighFrequencyInterface;
  	DAC1_NS->CR  = DAC_BOTH_CHANNEL_ENABLE;

	return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:		WriteDAC
//
//  Parameter(s):   uint32_t 			Channel
//					uint32_t 			Value
//  Return:         SystemState_e
//
//  Description:	Write Value on the DAC Channel
//
//-------------------------------------------------------------------------------------------------
SystemState_e DAC_Driver::WriteDAC(uint32_t Channel, uint32_t Value)
{
	if(m_IsItInitialize == false)
	{
		return SYS_ERROR;
	}
  
	if(Channel == DAC_CHANNEL_1)
	{
      #if (DAC_CFG_ALIGNMENT == DAC_ALIGN_12B_R)
        DAC_CHANNEL1->DHR12 = Value;
	  #elif (DAC_CFG_ALIGNMENT == DAC_ALIGN_12B_L)
        DAC_CHANNEL1->DHL12 = Value;
	  #else // (DAC_CFG_ALIGNMENT == DAC_ALIGN_8B_R)
		DAC_CHANNEL1->DHR8 = Value & DAC_DHR8R1_DACC1DHR_Msk
	  #endif
	}
  
	else
	{
      #if (DAC_CFG_ALIGNMENT == DAC_ALIGN_12B_R)
        DAC_CHANNEL2->DHR12 = Value;
	  #elif (DAC_CFG_ALIGNMENT == DAC_ALIGN_12B_L)
        DAC_CHANNEL2->DHL12 = Value;
	  #else // (DAC_CFG_ALIGNMENT == DAC_ALIGN_8B_R)
		DAC_CHANNEL2->DHR8 = Value & DAC_DHR8R2_DACC1DHR_Msk
	  #endif
	}
    
	return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

