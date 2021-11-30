//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_sai.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

#include "digini_cfg.h"
#ifdef DIGINI_USE_SAI
#include <stdint.h>
#define STM32F7_SAI_GLOBAL
#include "lib_class_STM32F7_sai.h"
#undef  STM32F7_SAI_GLOBAL
#include "lib_STM32F7_gpio.h"
#include "lib_macro.h"
#include "bsp.h"
#include "assert.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// forward declaration(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Class: SAI
//
//   Description:   Class to handle SAI
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   SAI
//
//   Parameter(s):  SAI_PortInfo_t*  pPort
//
//   Description:   Initialize the SAIx peripheral according to the specified Parameters
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SAI::SAI(SAI_PortInfo_t* pPort)
{
    m_pPort = pPort;
}

//-------------------------------------------------------------------------------------------------
//
//   Destructor:   SAI
//
//  Description:    De-initializes the SAIx peripheral
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SAI::~SAI()
{
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   None
//  Return:         none
//
//  Description:    It start the initialize process from configuration in struct
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SAI::Initialize(void)
{
	nOS_TickCounter    TickStart;
 	uint32_t           TempGCR;
	uint32_t 		   ClockFrequency;
 	SAI_PortConfig_t*  pConfig        = &SAI_PortConfig[m_pPort->ConfigID];
 	uint32_t           AudioFrequency = pConfig->AudioFrequency;
 	SAI_Block_TypeDef* pSAI_Block     = m_pPort->pSAI_Block;

	//---------------------------------------------------------------------------------------------
	// Disable SAI

	//HAL_SAI_DeInit(&haudio_out_sai); don't need!!
 	pSAI_Block->CR1 &= ~SAI_xCR1_SAIEN;

	//------------------------------------ SAI1 configuration --------------------------------------
	if(m_pPort->pSAIx == SAI1)
	{
		// Configure SAI1 Clock source
		if(m_pPort->Clock == SAIx_CLK_SOURCE_PLLI2S)
		{
			MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_SAI1SEL, RCC_SAI1CLKSOURCE_PLLI2S);
		}
		else // SAIx_CLK_SOURCE_PLLSAI
		{
			MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_SAI1SEL, RCC_SAI1CLKSOURCE_PLLSAI);
		}
	}
	//------------------------------------ SAI2 configuration --------------------------------------
	else // if(pSAI == SAI2)
	{
		// Configure SAI2 Clock source
   		if(m_pPort->Clock == SAIx_CLK_SOURCE_PLLI2S)
		{
			MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_SAI2SEL, RCC_SAI2CLKSOURCE_PLLI2S);
		}
		else // SAIx_CLK_SOURCE_PLLSAI
		{
			MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_SAI2SEL, RCC_SAI2CLKSOURCE_PLLSAI);
		}
	}

	//-------------------------------------- PLLI2S Configuration ---------------------------------
	if(m_pPort->Clock == SAIx_CLK_SOURCE_PLLI2S)
	{
		uint32_t PLLI2SN;
		uint32_t PLLI2SQ;
		uint32_t PLLI2SDivQ;
		
		RCC->CR &= ~(RCC_CR_PLLI2SON);  							// Disable the PLLI2S
		TickStart = GET_START_TIME();		    					// Get Start Tick
		while((RCC->CR & RCC_CR_PLLI2SRDY) == RCC_CR_PLLI2SRDY)		// Wait till PLLI2S is disable
		{
			if(HAS_TIMED_OUT(TickStart, PLLI2S_TIMEOUT_VALUE) == true)
			{ 
				return; // HAL_TIMEOUT;	        					// return in case of Timeout detected
			}
		}
		
		//---------------------------------------------------------------------------------------------
		// PLL clock is set depending on the AudioFrequency (44.1khz vs 48khz groups)

		// Set the PLL configuration according to the audio frequency
		if((AudioFrequency == SAI_AUDIO_FREQUENCY_11K) || (AudioFrequency == SAI_AUDIO_FREQUENCY_22K) || (AudioFrequency == SAI_AUDIO_FREQUENCY_44K))
		{
			PLLI2SN        = 429;				// PLLI2S_VCO: VCO_429M
			PLLI2SQ        = 2;					// SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
			PLLI2SDivQ     = 19;				// SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 214.5/19 = 11.289 Mhz
		}
		else // AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K
		{
			PLLI2SN        = 344;				// PLLI2S_VCO: VCO_344M
			PLLI2SQ        = 7;					// SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 344/7 = 49.142 Mhz
			PLLI2SDivQ     = 1;					// SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 49.142/1 = 49.142 Mhz
		}
		
		ClockFrequency = (PLLI2SN * 1000000) / PLLI2SQ;

		// Configure PLLI2S prescalers
        MODIFY_REG(RCC->PLLI2SCFGR, 
		           (RCC_PLLI2SCFGR_PLLI2SN_Msk | RCC_PLLI2SCFGR_PLLI2SQ_Msk),
				   ((PLLI2SN << RCC_PLLI2SCFGR_PLLI2SN_Pos) | (PLLI2SQ << RCC_PLLI2SCFGR_PLLI2SQ_Pos)));

		// SAI_CLK_x = SAI_CLK(first level)/PLLI2SDIVQ
		MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_PLLI2SDIVQ, PLLI2SDivQ - 1);
	
		RCC->CR |= RCC_CR_PLLI2SON;  								// Enable the PLLI2S
		TickStart = GET_START_TIME();		    					// Get Start Tick
		while((RCC->CR & RCC_CR_PLLI2SRDY) != RCC_CR_PLLI2SRDY)		// Wait till PLLI2S is ready
		{
			if(HAS_TIMED_OUT(TickStart, PLLI2S_TIMEOUT_VALUE) == true)
			{
				return; // HAL_TIMEOUT;	        					// return in case of Timeout detected
			}
		}
	} 

	//-------------------------------------- PLLSAI Configuration ---------------------------------
	// PLLSAI is configured when a peripheral will use it as source clock : SAI1, SAI2, LTDC or CK48
	else // if(m_pPort->Clock == SAIx_CLK_SOURCE_PLLSAI)
	{
		uint32_t PLLSAIN;
		uint32_t PLLSAIQ;
		uint32_t PLLSAIDivQ;

		RCC->CR &= ~(RCC_CR_PLLSAION);							// Disable PLLSAI Clock
		TickStart = GET_START_TIME();							// Get Start Tick
		while((RCC->CR & RCC_CR_PLLSAIRDY) == RCC_CR_PLLSAIRDY)	// Wait till PLLSAI is disabled
		{
			if(HAS_TIMED_OUT(TickStart, PLLSAI_TIMEOUT_VALUE) == true)
			{ 
				return; // HAL_TIMEOUT;							// return in case of Timeout detected
			}
		} 
    
		//---------------------------------------------------------------------------------------------
		// PLL clock is set depending on the AudioFrequency (44.1khz vs 48khz groups)

		// Set the PLL configuration according to the audio frequency
		// TODO calculate the pll value for a PLLSAI usage
		if((AudioFrequency == SAI_AUDIO_FREQUENCY_11K) || (AudioFrequency == SAI_AUDIO_FREQUENCY_22K) || (AudioFrequency == SAI_AUDIO_FREQUENCY_44K))
		{
			PLLSAIN       = 1;				// PLLI2S_VCO: VCO_429M
			PLLSAIQ       = 1;				// SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
			PLLSAIDivQ    = 1;				// SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 214.5/19 = 11.289 Mhz
		}
		else // SAI_AUDIO_FREQUENCY_8K, SAI_AUDIO_FREQUENCY_16K, SAI_AUDIO_FREQUENCY_48K, SAI_AUDIO_FREQUENCY_96K
		{
			PLLSAIN       = 1;				// PLLI2S_VCO: VCO_344M
			PLLSAIQ       = 1;				// SAI_CLK(first level) = PLLI2S_VCO/PLLSAIQ = 344/7 = 49.142 Mhz
			PLLSAIDivQ    = 1;				// SAI_CLK_x = SAI_CLK(first level)/PLLI2SDivQ = 49.142/1 = 49.142 Mhz
		}

		ClockFrequency = (PLLSAIN * 100000) / PLLSAIQ;

		// Configure PLLSAI prescalers
	    MODIFY_REG(RCC->PLLSAICFGR,
		           (RCC_PLLSAICFGR_PLLSAIN_Msk | RCC_PLLSAICFGR_PLLSAIQ_Msk),
                   ((PLLSAIN << RCC_PLLSAICFGR_PLLSAIN_Pos) | (PLLSAIQ) << RCC_PLLSAICFGR_PLLSAIQ_Pos));

		// SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ 
		MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_PLLSAIDIVQ, (PLLSAIDivQ - 1) << 8);

		RCC->CR |= RCC_CR_PLLSAION;								// Enable PLLSAI Clock
		TickStart = GET_START_TIME();		    				// Get Start Tick
		while((RCC->CR & RCC_CR_PLLSAIRDY) != RCC_CR_PLLSAIRDY)	// Wait till PLLSAI is ready
		{
			if(HAS_TIMED_OUT(TickStart, PLLSAI_TIMEOUT_VALUE) == true)
			{ 
				return; // HAL_TIMEOUT;	        				// return in case of Timeout detected
			}
		}
	}

	// Init the SAI MSP: this __weak function can be redefined by the application
	//BSP_AUDIO_OUT_MspInit(&haudio_out_sai, NULL);

	IO_PinInit(&m_pPort->MCLK);
	IO_PinInit(&m_pPort->SCK_SD);
	IO_PinInit(&m_pPort->SD);
	IO_PinInit(&m_pPort->FS);

	// Enable peripheral clock
	if(m_pPort->pSAIx == SAI1)
	{
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI1EN);
	}
	else
	{
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SAI2EN);
	}
	
	uint32_t ckstr_bits     = 0;
	uint32_t syncen_bits    = 0;
  
	// SAI Block Synchro Configuration -----------------------------------------
	// This setting must be done with both audio block (A & B) disabled
	switch(pConfig->ExternalSynchro)
	{
		case SAI_EXTERNAL_SYNC_DISABLE:	           TempGCR = 0; 	                break;
		case SAI_EXTERNAL_SYNC_OUT_BLOCK_A_ENABLE: TempGCR = SAI_GCR_SYNCOUT_0;     break;
		case SAI_EXTERNAL_SYNC_OUT_BLOCK_B_ENABLE: TempGCR = SAI_GCR_SYNCOUT_1;     break;
		default:    break;
	}
  
	switch(pConfig->Synchro)
	{
		case SAI_ASYNCHRONOUS:			{ syncen_bits = 0;                 } break;
		case SAI_SYNCHRONOUS:   		{ syncen_bits = SAI_xCR1_SYNCEN_0; } break;
		case SAI_SYNCHRONOUS_EXT_SAI1:  { syncen_bits = SAI_xCR1_SYNCEN_1; } break;
		case SAI_SYNCHRONOUS_EXT_SAI2: 	{ syncen_bits = SAI_xCR1_SYNCEN_1; TempGCR |= SAI_GCR_SYNCIN_0; } break;
    	default:   break;      
	}

    m_pPort->pSAIx->GCR = TempGCR;

	if(AudioFrequency != SAI_AUDIO_FREQUENCY_MCKDIV)
	{
		uint32_t tmpval;


		// Configure Master Clock using the following formula :
		//   MCLK_x = SAI_CK_x / (MCKDIV[3:0] * 2) with MCLK_x = 256 * FS
		//   FS = SAI_CK_x / (MCKDIV[3:0] * 2) * 256
        //   MCKDIV[3:0] = SAI_CK_x / FS * 512
		// (freq x 10) to keep Significant digits
		tmpval = (ClockFrequency * 10) / (AudioFrequency * 2 * 256);
		pConfig->Mckdiv = tmpval / 10;
    
		// Round result to the nearest integer
		if((tmpval % 10) > 8)
		{
			pConfig->Mckdiv += 1;
		}
	}
  
	// Compute CKSTR bits of SAI CR1 according ClockStrobing and AudioMode
	if((pConfig->AudioMode == SAI_MODE_MASTER_TX) || (pConfig->AudioMode == SAI_MODE_SLAVE_TX))
	{ // Transmit
		ckstr_bits = (pConfig->ClockStrobing == SAI_CLOCK_STROBING_RISING_EDGE) ? 0 : SAI_xCR1_CKSTR;
	}
	else
	{ // Receive
		ckstr_bits = (pConfig->ClockStrobing == SAI_CLOCK_STROBING_RISING_EDGE) ? SAI_xCR1_CKSTR : 0;
	}
  
	// SAI Block Configuration -------------------------------------------------
	// SAI CR1 Configuration
	MODIFY_REG(pSAI_Block->CR1,
			   (SAI_xCR1_MODE | SAI_xCR1_PRTCFG |  SAI_xCR1_DS | SAI_xCR1_LSBFIRST | SAI_xCR1_CKSTR | SAI_xCR1_SYNCEN | SAI_xCR1_MONO | SAI_xCR1_OUTDRIV  | SAI_xCR1_DMAEN | SAI_xCR1_NODIV | SAI_xCR1_MCKDIV),
			   (pConfig->AudioMode | pConfig->Protocol | pConfig->DataSize | pConfig->FirstBit | ckstr_bits | syncen_bits | pConfig->MonoStereoMode | pConfig->OutputDrive | pConfig->NoDivider | (pConfig->Mckdiv << 20)));

	// SAI CR2 Configuration
	MODIFY_REG(pSAI_Block->CR2,
			   (SAI_xCR2_FTH | SAI_xCR2_FFLUSH | SAI_xCR2_COMP | SAI_xCR2_CPL),
               (pConfig->FIFOThreshold | pConfig->CompandingMode | pConfig->TriState));
  
	// SAI Frame Configuration -----------------------------------------
	MODIFY_REG(pSAI_Block->FRCR,
			   (SAI_xFRCR_FRL | SAI_xFRCR_FSALL | SAI_xFRCR_FSDEF | SAI_xFRCR_FSPOL | SAI_xFRCR_FSOFF),
               ((pConfig->FrameLength - 1) | pConfig->FSOffset | pConfig->FSDefinition | pConfig->FSPolarity | ((pConfig->ActiveFrameLength - 1) << 8)));
  
	// SAI Block_x SLOT Configuration ------------------------------------------
	// This register has no meaning in AC 97 and SPDIF audio protocol
	MODIFY_REG(pSAI_Block->SLOTR,
			   (SAI_xSLOTR_FBOFF | SAI_xSLOTR_SLOTSZ | SAI_xSLOTR_NBSLOT | SAI_xSLOTR_SLOTEN),
               (pConfig->FirstBitOffset | pConfig->SlotSize | (pConfig->SlotActive << 16) | ((pConfig->SlotNumber - 1) << 8)));
  
	pSAI_Block->CR1 |= SAI_xCR1_SAIEN;	// Enable SAI peripheral to generate MCLK






#if 0			// this part is in the audio driver.... not in this drive
	  /* wm8994 codec initialization */
	  deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);

	  if((deviceid) == WM8994_ID)
	  {
	    /* Reset the Codec Registers */
	    wm8994_drv.Reset(AUDIO_I2C_ADDRESS);
	    /* Initialize the audio driver structure */
	    audio_drv = &wm8994_drv;
	    ret = AUDIO_OK;
	  }
	  else
	  {
	    ret = AUDIO_ERROR;
	  }

	  if(ret == AUDIO_OK)
	  {
	    /* Initialize the codec internal registers */
	    audio_drv->Init(AUDIO_I2C_ADDRESS, OutputDevice, Volume, AudioFreq);
	  }
#endif































#if 0
	uint32_t tmpregisterGCR = 0;
	uint32_t ckstr_bits = 0;
	uint32_t syncen_bits = 0;

	HAL_SAI_MspInit(hsai);

	SAI_Disable(hsai);										// Disable the selected SAI peripheral

	// SAI Block Synchro Configuration -----------------------------------------

	// This setting must be done with both audio block (A & B) disabled
	switch(hsai->Init.SynchroExt)
	{
		case SAI_SYNCEXT_DISABLE:		   { tmpregisterGCR = 0;                 }	break;
		case SAI_SYNCEXT_OUTBLOCKA_ENABLE: { tmpregisterGCR = SAI_GCR_SYNCOUT_0; }	break;
		case SAI_SYNCEXT_OUTBLOCKB_ENABLE: { tmpregisterGCR = SAI_GCR_SYNCOUT_1; }	break;
		default: break;
	}

	switch(hsai->Init.Synchro)
	{
		case SAI_ASYNCHRONOUS:	       { syncen_bits = 0;                                                     }	break;
		case SAI_SYNCHRONOUS:	       { syncen_bits = SAI_xCR1_SYNCEN_0;                                     }	break;
		case SAI_SYNCHRONOUS_EXT_SAI1: { syncen_bits = SAI_xCR1_SYNCEN_1;                                     }	break;
		case SAI_SYNCHRONOUS_EXT_SAI2: { syncen_bits = SAI_xCR1_SYNCEN_1; tmpregisterGCR |= SAI_GCR_SYNCIN_0; }	break;
		default: break;
	}


	pSAI->GCR = tmpregisterGCR;

	if(hsai->Init.AudioFrequency != SAI_AUDIO_FREQUENCY_MCKDIV)
	{
		uint32_t freq = 0;
		uint32_t tmpval;

		if((pSAI == SAI1_Block_A) || (pSAI == SAI1_Block_B))
		{
			freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI1);
		}
		else
		{
			freq = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI2);
		}

		// Configure Master Clock using the following formula :
	   	//   MCLK_x = SAI_CK_x / (MCKDIV[3:0] * 2) with MCLK_x = 256 * FS
	   	//   FS = SAI_CK_x / (MCKDIV[3:0] * 2) * 256
	   	//   MCKDIV[3:0] = SAI_CK_x / FS * 512
		// (freq x 10) to keep Significant digits
		tmpval = (freq * 10) / (hsai->Init.AudioFrequency * 2 * 256);
		hsai->Init.Mckdiv = tmpval / 10;

		// Round result to the nearest integer
		if((tmpval % 10) > 8)
		{
			hsai->Init.Mckdiv+= 1;
		}
	}

	// Compute CKSTR bits of SAI CR1 according ClockStrobing and AudioMode
	if((hsai->Init.AudioMode == SAI_MODEMASTER_TX) || (hsai->Init.AudioMode == SAI_MODESLAVE_TX))
	{ // Transmit
		ckstr_bits = (hsai->Init.ClockStrobing == SAI_CLOCKSTROBING_RISINGEDGE) ? 0 : SAI_xCR1_CKSTR;
	}
	else
	{ // Receive
	ckstr_bits = (hsai->Init.ClockStrobing == SAI_CLOCKSTROBING_RISINGEDGE) ? SAI_xCR1_CKSTR : 0;
	}

	// SAI Block Configuration -------------------------------------------------
	// SAI CR1 Configuration
	pSAI->CR1 &= ~(SAI_xCR1_MODE | SAI_xCR1_PRTCFG |  SAI_xCR1_DS | SAI_xCR1_LSBFIRST | SAI_xCR1_CKSTR | SAI_xCR1_SYNCEN |
				   SAI_xCR1_MONO | SAI_xCR1_OUTDRIV  | SAI_xCR1_DMAEN | SAI_xCR1_NODIV | SAI_xCR1_MCKDIV);

	pSAI->CR1 |= (hsai->Init.AudioMode | hsai->Init.Protocol | hsai->Init.DataSize | hsai->Init.FirstBit |
				  ckstr_bits | syncen_bits | hsai->Init.MonoStereoMode | hsai->Init.OutputDrive |
				  hsai->Init.NoDivider | (hsai->Init.Mckdiv << 20));

	// SAI CR2 Configuration
	pSAI->CR2 &= ~(SAI_xCR2_FTH | SAI_xCR2_FFLUSH | SAI_xCR2_COMP | SAI_xCR2_CPL);
	pSAI->CR2 |=  (hsai->Init.FIFOThreshold | hsai->Init.CompandingMode | hsai->Init.TriState);

	// SAI Frame Configuration -----------------------------------------
	pSAI->FRCR &=(~(SAI_xFRCR_FRL | SAI_xFRCR_FSALL | SAI_xFRCR_FSDEF | SAI_xFRCR_FSPOL | SAI_xFRCR_FSOFF));
	pSAI->FRCR |=((hsai->FrameInit.FrameLength - 1) | hsai->FrameInit.FSOffset | hsai->FrameInit.FSDefinition |
						  hsai->FrameInit.FSPolarity | ((hsai->FrameInit.ActiveFrameLength - 1) << 8));

	// SAI Block_x SLOT Configuration ------------------------------------------
	// This register has no meaning in AC 97 and SPDIF audio protocol
	pSAI->SLOTR &= (~(SAI_xSLOTR_FBOFF | SAI_xSLOTR_SLOTSZ | SAI_xSLOTR_NBSLOT | SAI_xSLOTR_SLOTEN));

	pSAI->SLOTR|=  hsai->SlotInit.FirstBitOffset | hsai->SlotInit.SlotSize | (hsai->SlotInit.SlotActive << 16) | ((hsai->SlotInit.SlotNumber - 1) << 8);



#endif








































#if 0
    nOS_Error               Error;
    uint32_t                PriorityGroup;

    // TODO put an initialization flag
    Error = nOS_MutexCreate(&this->m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
    VAR_UNUSED(Error);

    m_Timeout = 0;

    NVIC_DisableIRQ(m_pPort->EV_IRQn);
    NVIC_DisableIRQ(m_pPort->ER_IRQn);

    // Configure SAI module Frequency
    m_pPort->pSAIx->TIMINGR = m_pPort->Timing;

    PriorityGroup = NVIC_GetPriorityGrouping();
    NVIC_SetPriority(m_pPort->EV_IRQn, NVIC_EncodePriority(PriorityGroup, 5, 0));
    NVIC_EnableIRQ(m_pPort->EV_IRQn);
    NVIC_SetPriority(m_pPort->ER_IRQn, NVIC_EncodePriority(PriorityGroup, 5, 0));
    NVIC_EnableIRQ(m_pPort->ER_IRQn);




    /* wm8994 codec initialization */
    deviceid = wm8994_drv.ReadID(AUDIO_SAI_ADDRESS);

    if((deviceid) == WM8994_ID)
    {
      /* Reset the Codec Registers */
      wm8994_drv.Reset(AUDIO_SAI_ADDRESS);
      /* Initialize the audio driver structure */
      audio_drv = &wm8994_drv;
      ret = AUDIO_OK;
    }
    else
    {
      ret = AUDIO_ERROR;
    }

    if(ret == AUDIO_OK)
    {
      /* Initialize the codec internal registers */
      audio_drv->Init(AUDIO_SAI_ADDRESS, OutputDevice, Volume, AudioFreq);
    }

    return ret;
#endif

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
SystemState_e SAI::GetStatus(void)
{
    return m_Status;
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_SAI




#if 0

from branch antique radio F4
//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_sai.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

#include "digini_cfg.h"
#include <stdint.h>
#define STM32F7_SAI_GLOBAL
#include "lib_class_sai.h"
#undef  STM32F7_SAI_GLOBAL
#include "lib_class_gpio.h"
#include "lib_macro.h"
#include "bsp.h"
#include "assert.h"

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   SAI
//
//   Parameter(s):  SAI_PortInfo_t*  pPort
//
//   Description:   Initialize the SAIx peripheral according to the specified Parameters
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SAI_Driver::SAI_Driver(const SAI_PortInfo_t* pPort)
{
    m_pPort = const_cast<SAI_PortInfo_t*>(pPort);
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
SystemState_e SAI_Driver::GetStatus(void)
{
    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      GetFrequency
//
//   Parameter(s):  None
//   Return Value:  uint32_t
//
//   Description:   Get
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t SAI_Driver::GetFrequency(void)
{
     return m_AudioFrequency;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SetFrequency
//
//   Parameter(s):  AudioFrequency
//   Return Value:  None
//
//   Description:   Set
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SAI_Driver::SetFrequency(uint32_t AudioFrequency)
{
    m_AudioFrequency = AudioFrequency;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SetAudioFrameSlot
//
//   Parameter(s):  None
//   Return Value:  uint32_t
//
//   Description:   Return general status of the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SAI_Driver::SetAudioFrameSlot(uint32_t AudioFrameSlot)
{
    // Disable SAI peripheral to allow access to SAI internal registers
    m_pPort->pSAI_BlockOut->CR1 &= ~SAI_xCR1_SAIEN;

    // Update the SAI audio frame slot configuration
    MODIFY_REG(m_pPort->pSAI_BlockOut->SLOTR, SAI_xSLOTR_SLOTEN_Pos, AudioFrameSlot);

    // Enable SAI peripheral to generate MCLK
    m_pPort->pSAI_BlockOut->CR1 |= SAI_xCR1_SAIEN;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SetAudioFrameSlot
//
//   Parameter(s):  pData    Pointer to data buffer
//                  Size    Amount of data to be sent
//   Return Value:  SystemState_e
//
//   Description:   Transmit an amount of data in non-blocking mode with DMA.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SAI_Driver::Transmit_DMA(uint8_t* pData, uint16_t Size)
{
    if((pData == nullptr) || (Size == 0))
    {
        return SYS_ERROR;
    }

    m_pPort->pDMA->NDTR = Size;                                                    // Configure DMA Stream data length
    m_pPort->pDMA->PAR  = uint32_t(&m_pPort->pSAI_BlockOut->DR);                // Configure DMA Stream source address
    m_pPort->pDMA->M0AR = uint32_t(pData);                                        // Configure DMA Stream destination address

    CLEAR_BIT(m_pPort->pDMA->CR, DMA_SxCR_DBM);                                    // Clear DBM bit
    SET_BIT(m_pPort->pDMA->CR, DMA_SxCR_TCIE |
                               DMA_SxCR_HTIE |
                               DMA_SxCR_TEIE |
                               DMA_SxCR_DMEIE);                                    // Enable Common interrupts
    SET_BIT(m_pPort->pDMA->FCR, DMA_IT_FE);

    SET_BIT(m_pPort->pDMA->CR, DMA_SxCR_EN);
//    SET_BIT(m_pPort->pSAI_BlockOut->CR1, SAI_xCR1_SAIEN);

    // Enable the interrupts for error handling
    SET_BIT(m_pPort->pSAI_BlockOut->IMR, SAI_xIMR_OVRUDRIE | SAI_xIMR_WCKCFGIE);


    // Enable SAI TX DMA Request
    SET_BIT(m_pPort->pSAI_BlockOut->CR1, SAI_xCR1_DMAEN);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

void SAI_Driver::StopDMA(void)
{
    CLEAR_BIT(m_pPort->pDMA->CR, DMA_SxCR_EN);
    CLEAR_BIT(m_pPort->pSAI_BlockOut->CR1, SAI_xCR1_DMAEN);
}

#endif