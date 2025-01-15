//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H7_hsem.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

/* SemID to mask helper Macro.
  * @param  __SEMID__: semaphore ID from 0 to 31
  * @retval Semaphore Mask.
  */
#define HSEM_SEMID_TO_MASK(__SEMID__) (1 << (__SEMID__))

/* Enables the specified HSEM interrupts.
  * @param  __SEM_MASK__: semaphores Mask
  * @retval None.
  */
#if defined(DUAL_CORE)
#define HSEM_ENABLE_IT(__SEM_MASK__) ((((SCB->CPUID & 0x000000F0) >> 4 )== 0x7) ? \
                                            (HSEM->C1IER |= (__SEM_MASK__)) : \
                                            (HSEM->C2IER |= (__SEM_MASK__)))
#else
#define HSEM_ENABLE_IT(__SEM_MASK__) (HSEM->IER |= (__SEM_MASK__))
#endif /* DUAL_CORE */
/* Disables the specified HSEM interrupts.
  * @param  __SEM_MASK__: semaphores Mask
  * @retval None.
  */
#if defined(DUAL_CORE)
#define HSEM_DISABLE_IT(__SEM_MASK__) ((((SCB->CPUID & 0x000000F0) >> 4 )== 0x7) ? \
                                             (HSEM->C1IER &= ~(__SEM_MASK__)) :       \
                                             (HSEM->C2IER &= ~(__SEM_MASK__)))
#else
#define HSEM_DISABLE_IT(__SEM_MASK__) (HSEM->IER &= ~(__SEM_MASK__))
#endif

/* Checks whether interrupt has occurred or not for semaphores specified by a mask.
  * @param  __SEM_MASK__: semaphores Mask
  * @retval semaphores Mask : Semaphores where an interrupt occurred.
  */
#if defined(DUAL_CORE)
#define HSEM_GET_IT(__SEM_MASK__) ((((SCB->CPUID & 0x000000F0) >> 4 )== 0x7) ? \
                                         ((__SEM_MASK__) & HSEM->C1MISR) :        \
                                         ((__SEM_MASK__) & HSEM->C2MISR1))
#else
#define HSEM_GET_IT(__SEM_MASK__) ((__SEM_MASK__) & HSEM->MISR)
#endif

/* Get the semaphores release status flags.
  * @param  __SEM_MASK__: semaphores Mask
  * @retval semaphores Mask : Semaphores where Release flags rise.
  */
#if defined(DUAL_CORE)
#define HSEM_GET_FLAG(__SEM_MASK__) ((((SCB->CPUID & 0x000000F0) >> 4 )== 0x7) ? \
                                           (__SEM_MASK__) & HSEM->C1ISR :           \
                                           (__SEM_MASK__) & HSEM->C2ISR)
#else
#define HSEM_GET_FLAG(__SEM_MASK__) ((__SEM_MASK__) & HSEM->ISR)
#endif

/* Clears the HSEM Interrupt flags.
  * @param  __SEM_MASK__: semaphores Mask
  * @retval None.
  */
#if defined(DUAL_CORE)
#define HSEM_CLEAR_FLAG(__SEM_MASK__) ((((SCB->CPUID & 0x000000F0) >> 4 )== 0x7) ? \
                                             (HSEM->C1ICR |= (__SEM_MASK__)) :        \
                                             (HSEM->C2ICR |= (__SEM_MASK__)))
#else
#define HSEM_CLEAR_FLAG(__SEM_MASK__) (HSEM->ICR |= (__SEM_MASK__))
#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

// HSEM Take and Release functions
SystemState_e       HSEM_Take                   (uint32_t SemID, uint32_t ProcessID);
SystemState_e       HSEM_FastTake               (uint32_t SemID);
bool                HSEM_IsSemTaken             (uint32_t SemID);                           // maybe use SystemState_e
void                HSEM_Release                (uint32_t SemID, uint32_t ProcessID);
void                HSEM_ReleaseAll             (uint32_t Key, uint32_t CoreID);

// HSEM Set and Get Key functions.
void                HSEM_SetClearKey            (uint32_t Key);
uint32_t            HSEM_GetClearKey            (void);

// HSEM Notification functions.
void                HSEM_ActivateNotification   (uint32_t SemMask);
void                HSEM_DeactivateNotification (uint32_t SemMask);
void                HSEM_FreeCallback           (uint32_t SemMask);
void                HSEM_IRQHandler             (void);

//-------------------------------------------------------------------------------------------------

