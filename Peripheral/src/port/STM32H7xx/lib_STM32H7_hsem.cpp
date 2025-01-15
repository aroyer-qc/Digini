//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7_hsem.cpp
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
// define(s)
//-------------------------------------------------------------------------------------------------

#if defined(DUAL_CORE)
#ifndef HSEM_R_MASTERID
#define HSEM_R_MASTERID HSEM_R_COREID
#endif

#ifndef HSEM_RLR_MASTERID
#define HSEM_RLR_MASTERID HSEM_RLR_COREID
#endif

#ifndef HSEM_CR_MASTERID
#define HSEM_CR_MASTERID HSEM_CR_COREID
#endif
#endif

#define IS_HSEM_SEMID(__SEMID__) ((__SEMID__) <= HSEM_SEMID_MAX )
#define IS_HSEM_PROCESSID(__PROCESSID__) ((__PROCESSID__) <= HSEM_PROCESSID_MAX )
#define IS_HSEM_KEY(__KEY__) ((__KEY__) <= HSEM_CLEAR_KEY_MAX )
#if defined(DUAL_CORE)
#define IS_HSEM_COREID(__COREID__) (((__COREID__) == HSEM_CPU1_COREID) || \
                                    ((__COREID__) == HSEM_CPU2_COREID))
#else
#define IS_HSEM_COREID(__COREID__) ((__COREID__) == HSEM_CPU1_COREID)
#endif


/*==============================================================================
                     ##### How to use this driver #####
  ==============================================================================

      Take a semaphore In 2-Step mode Using function HSEM_Take. This function takes as parameters :
           -  the semaphore ID from 0 to 31
           -  the process ID from 0 to 255
       Fast Take semaphore In 1-Step mode Using function HSEM_FastTake. This function takes as parameter :
           -  the semaphore ID from 0_ID to 31. Note that the process ID value is implicitly assumed as zero
       Check if a semaphore is Taken using function HSEM_IsSemTaken. This function takes as parameter :
          -  the semaphore ID from 0_ID to 31
          -  It returns 1 if the given semaphore is taken otherwise (Free) zero
      Release a semaphore using function with HSEM_Release. This function takes as parameters :
           -  the semaphore ID from 0 to 31
           -  the process ID from 0 to 255:
           -  Note: If ProcessID and MasterID match, semaphore is freed, and an interrupt
         may be generated when enabled (notification activated). If ProcessID or MasterID does not match,
         semaphore remains taken (locked)

      Release all semaphores at once taken by a given Master using function HAL_HSEM_Release_All
          This function takes as parameters :
           -  the Release Key (value from 0 to 0xFFFF) can be Set or Get respectively by
              HSEM_SetClearKey() or HSEM_GetClearKey functions
           -  the Master ID:
           -  Note: If the Key and MasterID match, all semaphores taken by the given CPU that corresponds
           to MasterID  will be freed, and an interrupt may be generated when enabled (notification activated). If the
           Key or the MasterID doesn't match, semaphores remains taken (locked)

      Semaphores Release all key functions:
         -   HSEM_SetClearKey() to set semaphore release all Key
         -   HSEM_GetClearKey() to get release all Key
      Semaphores notification functions :
         -   HSEM_ActivateNotification to activate a notification callback on
               a given semaphores Mask (bitfield). When one or more semaphores defined by the mask are released
               the callback HSEM_FreeCallback will be asserted giving as parameters a mask of the released
               semaphores (bitfield).

         -   HSEM_DeactivateNotification to deactivate the notification of a given semaphores Mask (bitfield).
         -  See the description of the macro HSEM_SEMID_TO_MASK to check how to calculate a semaphore mask
                Used by the notification functions
     
     *** HSEM HAL driver macros list ***
     =============================================
     Below the list of most used macros in HSEM HAL driver.

       HSEM_SEMID_TO_MASK: Helper macro to convert a Semaphore ID to a Mask.
       Example of use :
       mask = HSEM_SEMID_TO_MASK(8)  |  HSEM_SEMID_TO_MASK(21) | HSEM_SEMID_TO_MASK(25).
       All next macros take as parameter a semaphore Mask (bitfiled) that can be constructed using  HSEM_SEMID_TO_MASK as the above example.
       HSEM_ENABLE_IT: Enable the specified semaphores Mask interrupts.
       HSEM_DISABLE_IT: Disable the specified semaphores Mask interrupts.
       HSEM_GET_IT: Checks whether the specified semaphore interrupt has occurred or not.
       HSEM_GET_FLAG: Get the semaphores status release flags.
       HSEM_CLEAR_FLAG: Clear the semaphores status release flags.
  */

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_Take
//
//  Parameter(s):   SemID           Semaphore ID from 0 to 31
//                  ProcessID       Process ID from 0 to 255
//                  
//  Return:         SystemState_e
//
//  Description:    Take a semaphore in 2 step mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e HSEM_Take(uint32_t SemID, uint32_t ProcessID)
{
    // Check the parameters 
    //assert_param(IS_HSEM_SEMID(SemID));
    //assert_param(IS_HSEM_PROCESSID(ProcessID));

  #if USE_MULTI_CORE_SHARED_CODE != 0

    // First step  write R register with MasterID, processID and take bit = 1
    HSEM->R[SemID] = ((ProcessID & HSEM_R_PROCID) | ((HAL_GetCurrentCPUID() << POSITION_VAL(HSEM_R_MASTERID)) & HSEM_R_MASTERID) | HSEM_R_LOCK);

    // Second step : read the R register . Take achieved if MasterID and processID match and take bit set to 1
    if(HSEM->R[SemID] == ((ProcessID & HSEM_R_PROCID) | ((HAL_GetCurrentCPUID() << POSITION_VAL(HSEM_R_MASTERID)) & HSEM_R_MASTERID) | HSEM_R_LOCK))
    {
        return SYS_READY;                   // Take success when MasterID and ProcessID match and take bit set
    }
  
  #else

    // First step  write R register with MasterID, processID and take bit = 1
    HSEM->R[SemID] = (ProcessID | HSEM_CR_COREID_CURRENT | HSEM_R_LOCK);

    // Second step : read the R register . Take achieved if MasterID and processID match and take bit set to 1
    if(HSEM->R[SemID] == (ProcessID | HSEM_CR_COREID_CURRENT | HSEM_R_LOCK))
    {
        return SYS_READY;                   // Take success when MasterID and ProcessID match and take bit set
    }

  #endif

    return SYS_ERROR;                       // Semaphore take fails
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_FastTake
//
//  Parameter(s):   SemID       Semaphore ID from 0 to 31
//                  
//  Return:         SystemState_e
//
//  Description:    Fast Take a semaphore with 1 step mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e HSEM_FastTake(uint32_t SemID)
{
    // Check the parameters
    //assert_param(IS_HSEM_SEMID(SemID));

  #if USE_MULTI_CORE_SHARED_CODE != 0
    
    // Read the RLR register to take the semaphore
    if(HSEM->RLR[SemID] == (((HAL_GetCurrentCPUID() << POSITION_VAL(HSEM_R_MASTERID)) & HSEM_RLR_MASTERID) | HSEM_RLR_LOCK))
    {
        return SYS_READY;           // Take success when MasterID match and take bit set
    }
  
  #else
  
    // Read the RLR register to take the semaphore
    if(HSEM->RLR[SemID] == (HSEM_CR_COREID_CURRENT | HSEM_RLR_LOCK))    
    {
        return SYS_READY;           // Take success when MasterID match and take bit set
    }

  #endif

    return SYS_ERROR;                // Semaphore take fails
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_IsSemTaken
//
//  Parameter(s):   SemID       Semaphore ID
//                  
//  Return:         bool        true or false
//
//  Description:    Check semaphore state Taken or not.
//
// maybe use SystemState_e
//
//-------------------------------------------------------------------------------------------------
bool HSEM_IsSemTaken(uint32_t SemID)
{
    return (((HSEM->R[SemID] & HSEM_R_LOCK) != 0) ? true : false);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_Release
//
//  Parameter(s):   SemID           Semaphore ID from 0 to 31
//                  ProcessID       Process ID from 0 to 255
//                  
//  Return:         None
//
//  Description:    Release a semaphore.
//
//-------------------------------------------------------------------------------------------------
void HSEM_Release(uint32_t SemID, uint32_t ProcessID)
{
    // Check the parameters
    //assert_param(IS_HSEM_SEMID(SemID));
    //assert_param(IS_HSEM_PROCESSID(ProcessID));

    // Clear the semaphore by writing to the R register : the MasterID , the processID and take bit = 0
  #if  USE_MULTI_CORE_SHARED_CODE != 0U
    HSEM->R[SemID] = (ProcessID | ((HAL_GetCurrentCPUID() << POSITION_VAL(HSEM_R_MASTERID)) & HSEM_R_MASTERID));
  #else
    HSEM->R[SemID] = (ProcessID | HSEM_CR_COREID_CURRENT);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_ReleaseAll
//
//  Parameter(s):   Key         Semaphore key, value from 0 to 0xFFFF
//                  CoreID     CoreID of the CPU that is using semaphores to be release.
//
//  Return:         None
//
//  Description:    Release all semaphore used by a given master.
//
//-------------------------------------------------------------------------------------------------
void HSEM_ReleaseAll(uint32_t Key, uint32_t CoreID)
{
    //assert_param(IS_HSEM_KEY(Key));
    //assert_param(IS_HSEM_COREID(CoreID));
    HSEM->CR = ((Key << HSEM_CR_KEY_Pos) | (CoreID << HSEM_CR_COREID_Pos));
}

/** @defgroup HSEM_Exported_Functions_Group2 HSEM Set and Get Key functions
  *  @brief    HSEM Set and Get Key functions.
  ==============================================================================
              ##### HSEM Set and Get Key functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Set semaphore Key
      (+) Get semaphore Key  */

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_SetClearKey
//
//  Parameter(s):   Key         Semaphore key, value from 0 to 0xFFFF
//                  
//  Return:         None
//
//  Description:    Set semaphore key.
//
//-------------------------------------------------------------------------------------------------
void  HSEM_SetClearKey(uint32_t Key)
{
//  assert_param(IS_HSEM_KEY(Key));
    MODIFY_REG(HSEM->KEYR, HSEM_KEYR_KEY, (Key << HSEM_KEYR_KEY_Pos));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_GetClearKey
//
//  Parameter(s):   None
//                  
//  Return:         Semaphore key, value from 0 to 0xFFFF
//
//  Description:    Get semaphore key.
//
//-------------------------------------------------------------------------------------------------
uint32_t HSEM_GetClearKey(void)
{
    return (HSEM->KEYR >> HSEM_KEYR_KEY_Pos);
}

/** @defgroup HSEM_Exported_Functions_Group3 HSEM IRQ handler management
  *  @brief    HSEM Notification functions.
  *
  ==============================================================================
      ##### HSEM IRQ handler management and Notification functions #####
  ==============================================================================
[..]  This section provides HSEM IRQ handler and Notification function.
  */

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_ActivateNotification
//
//  Parameter(s):   SemMask         Mask of released semaphores.
//                  
//  Return:         None
//
//  Description:    Activate semaphore release notification for a given semaphores mask.
//
//-------------------------------------------------------------------------------------------------
void HSEM_ActivateNotification(uint32_t SemMask)
{
#if  USE_MULTI_CORE_SHARED_CODE != 0U
  /*enable the semaphore mask interrupts */
  if (HAL_GetCurrentCPUID() == HSEM_CPU1_COREID)
  {
    /*Use interrupt line 0 for CPU1 Master */
    HSEM->C1IER |= SemMask;
  }
  else /* HSEM_CPU2_COREID */
  {
    /*Use interrupt line 1 for CPU2 Master*/
    HSEM->C2IER |= SemMask;
  }
#else
  HSEM_COMMON->IER |= SemMask;
#endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_DeactivateNotification
//
//  Parameter(s):   SemMask         Mask of released semaphores.
//                  
//  Return:         None
//
//  Description:    Deactivate semaphore release notification for a given semaphores mMask.
//
//-------------------------------------------------------------------------------------------------
void HSEM_DeactivateNotification(uint32_t SemMask)
{
  #if USE_MULTI_CORE_SHARED_CODE != 0U
    if(HAL_GetCurrentCPUID() == HSEM_CPU1_COREID)       // Enable the semaphore mask interrupts
    {
        HSEM->C1IER &= ~SemMask;                        // Use interrupt line 0 for CPU1 Master
    }
    else // HSEM_CPU2_COREID
    {
        HSEM->C2IER &= ~SemMask;                        // Use interrupt line 1 for CPU2 Master
    }
  #else
    HSEM_COMMON->IER &= ~SemMask;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_IRQHandler
//
//  Parameter(s):   None
//                  
//  Return:         None
//
//  Description:    Handles HSEM interrupt request
//
//-------------------------------------------------------------------------------------------------
void HSEM_IRQHandler(void)
{
    uint32_t StatusRegister;

    #if USE_MULTI_CORE_SHARED_CODE != 0
    
    if(HAL_GetCurrentCPUID() == HSEM_CPU1_COREID)       // Need to get this function converted
    {
        // Get the list of masked freed semaphores
        StatusRegister = HSEM->C1MISR;                  // Use interrupt line 0 for CPU1 Master
        HSEM->C1IER &= ~((uint32_t)StatusRegister);     // Disable Interrupts
        HSEM->C1ICR = ((uint32_t)StatusRegister);       // Clear Flags
  }
  else // HSEM_CPU2_COREID
  {
       // Get the list of masked freed semaphores
        StatusRegister = HSEM->C2MISR;                  // Use interrupt line 1 for CPU2 Master
        HSEM->C2IER &= ~((uint32_t)StatusRegister);     // Disable Interrupts
        HSEM->C2ICR = ((uint32_t)StatusRegister);       // Clear Flags 
    }
  
  #else
    
    // Get the list of masked freed semaphores  
    StatusRegister = HSEM_COMMON->MISR;
    HSEM_COMMON->IER &= ~((uint32_t)StatusRegister);    // Disable Interrupts
    HSEM_COMMON->ICR = ((uint32_t)StatusRegister);      // Clear Flags

  #endif

    HSEM_FreeCallback(StatusRegister);                  // Call FreeCallback
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           HSEM_FreeCallback
//
//  Parameter(s):   SemMask         Mask of Released semaphores
//                  
//  Return:         None
//
//  Description:    Semaphore Released Callback.
//
//  Note(s):        This function should not be modified, when the callback is needed,
//                  the HSEM_FreeCallback can be implemented in the user file
//
//-------------------------------------------------------------------------------------------------
__weak void HSEM_FreeCallback(uint32_t SemMask)
{
    VAR_UNUSED(SemMask);          // Prevent unused argument(s) compilation warning
}

//-------------------------------------------------------------------------------------------------
