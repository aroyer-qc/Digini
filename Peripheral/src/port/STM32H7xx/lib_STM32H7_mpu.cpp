//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7_mpu.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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

#if (USE_MPU_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Expanding Macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_MPU_AS_ENUM(       REGION, BASE_ADDRESS,  MPU_SIZE,  ACCESS_PERMISSION,  IS_BUFFERABLE,  IS_CACHEABLE,  IS_SHAREABLE,  TYPE_EXT_FIELD,  DISABLE_EXEC,            SUB_REGION_DISABLE)   REGION##_ENUM,
#define EXPAND_X_MPU_AS_STRUCT_DATA(REGION, BASE_ADDRESS,  MPU_SIZE,  ACCESS_PERMISSION,  IS_BUFFERABLE,  IS_CACHEABLE,  IS_SHAREABLE,  TYPE_EXT_FIELD,  DISABLE_EXEC,            SUB_REGION_DISABLE)   \
                                  { REGION, BASE_ADDRESS, (MPU_SIZE | ACCESS_PERMISSION | IS_BUFFERABLE | IS_CACHEABLE | IS_SHAREABLE | TYPE_EXT_FIELD | DISABLE_EXEC | (uint32_t(SUB_REGION_DISABLE) << MPU_RASR_SRD_Pos))},

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum MPU_NumberOfConfig_e
{
    MPU_DEF(EXPAND_X_MPU_AS_ENUM)
    MPU_NUM,
};

struct MPU_Region_t
{
  uint8_t  Number;
  uint32_t BaseAddress;
  uint32_t Config;
};

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const MPU_Region_t MPU_Region[MPU_NUM] =
{
    MPU_DEF(EXPAND_X_MPU_AS_STRUCT_DATA)
};

//-------------------------------------------------------------------------------------------------
//
//   Function name:     MPU_Initialize
//
//   Parameter(s):      MPU_Priviledge_e Priviledge
//   Return value:      None
//
//   Description:       Configure all specified region and enable the MPU module and set priviledge
//                      mode.
//
//-------------------------------------------------------------------------------------------------
void MPU_Initialize(MPU_Priviledge_e Priviledge)
{
    const MPU_Region_t* pRegion;

    __DMB();                                                        // Disable the MPU, Make sure outstanding transfers are done
    CLEAR_BIT(SCB->SHCSR, SCB_SHCSR_MEMFAULTENA_Msk);               // Disable fault exceptions
    MPU->CTRL = 0;                                                  // Disable the MPU and clear the control register

    for(int i = 0; i < int(MPU_NUM); i++)
    {
        pRegion = &MPU_Region[i];

        MPU->RNR  = pRegion->Number;                                // Set the Region number
        CLEAR_BIT(MPU->RASR, MPU_REGION_ENABLE);
        MPU->RBAR = pRegion->BaseAddress;
        MPU->RASR = pRegion->Config | MPU_REGION_ENABLE;
    }

    MPU->CTRL = uint32_t(Priviledge) | MPU_CTRL_ENABLE_Msk;         // Enable the MPU
    SET_BIT(SCB->SHCSR, SCB_SHCSR_MEMFAULTENA_Msk);                 // Enable fault exceptions
    __DSB();                                                        // Ensure MPU setting take effects
    __ISB();
}

//-------------------------------------------------------------------------------------------------

#endif
