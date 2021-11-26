//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_sdram.h
//
//*************************************************************************************************

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "digini_cfg.h"
#ifdef DIGINI_USE_SDRAM_MODULE
#include <stdint.h>
#include "lib_typedef.h"
#include "lib_class_STM32F7_fmc.h"

//-------------------------------------------------------------------------------------------------
// define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// typedef struct(s) and enum(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SDRAM_Driver
{
    public:

        void            Initialize              (FMC_SDRAM_TimingTypeDef* Timing);
        SystemState_e   Read                    (uint32_t* pAddress, uint8_t*  pDstBuffer, uint32_t BufferSize);
        SystemState_e   Read                    (uint32_t* pAddress, uint16_t* pDstBuffer, uint32_t BufferSize);
        SystemState_e   Read                    (uint32_t* pAddress, uint32_t* pDstBuffer, uint32_t BufferSize);
        SystemState_e   Read_DMA                (uint32_t* pAddress, uint32_t* pDstBuffer, uint32_t BufferSize);
        SystemState_e   Write                   (uint32_t* pAddress, uint8_t*  pSrcBuffer, uint32_t BufferSize);
        SystemState_e   Write                   (uint32_t* pAddress, uint16_t* pSrcBuffer, uint32_t BufferSize);
        SystemState_e   Write                   (uint32_t* pAddress, uint32_t* pSrcBuffer, uint32_t BufferSize);
        SystemState_e   Write_DMA               (uint32_t* pAddress, uint32_t* pSrcBuffer, uint32_t BufferSize);

        SystemState_e   WriteProtectionEnable   (void);
        SystemState_e   WriteProtectionDisable  (void);

        SystemState_e   ProgramRefreshRate      (uint32_t RefreshRate);
        SystemState_e   SetAutoRefreshNumber    (uint32_t AutoRefreshNumber);
        uint32_t        GetModeStatus           (void);
        SystemState_e   GetState                (void);

        void            IRQHandler              (void);

    private:

        SystemState_e   SendCommand             (FMC_SDRAM_CommandTypeDef* Command, uint32_t Timeout);

        SystemState_e   m_State;
};

void SDRAM_BSP_Initialize           (void);
void SDRAM_RefreshErrorCallback     (void);
void SDRAM_DMA_XferCpltCallback     (void);
void SDRAM_DMA_XferErrorCallback    (void);

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_SDRAM_MODULE

