//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H7_sdram.cpp
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
//
//   Function name: SDRAM_Initialize
//
//   Parameter(s):  Timing Pointer to SDRAM control timing structure
//   Return:        SystemState_e
//
//   Description:   Performs the SDRAM device initialization sequence.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void SDRAM::Initialize(FMC_SDRAM_TimingTypeDef* Timing)
{
    // ---- FMC Reset ----
    RCC->AHB3RSTR |=  RCC_AHB3RSTR_FMCRST;
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_FMCRST;

    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;                      // Enable Clock

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    if(m_State == SYS_RESET)

        SDRAM_BSP_Initialize();                                             // Initialize the low level hardware (MSP)
    }

    m_State = SYS_BUSY;                                                     // Initialize the SDRAM controller state
    something->SDRAM_Init(hsdram->Instance, &(hsdram->Init));                      // Initialize SDRAM control Interface
    something->SDRAM_TimingInit(Timing, hsdram->Init.SDBank);   // Initialize SDRAM timing Interface
    m_State = SYS_READY;                                                  // Update the SDRAM controller state
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: IRQHandler
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   This function handles SDRAM refresh error interrupt request.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void SDRAM::IRQHandler(void)
{
    if(__FMC_SDRAM_GET_FLAG(hsdram->Instance, FMC_SDRAM_FLAG_REFRESH_IT))           // Check SDRAM interrupt Rising edge flag
    {
        SDRAM_RefreshErrorCallback();                                               // SDRAM refresh error interrupt callback
        __FMC_SDRAM_CLEAR_FLAG(hsdram->Instance, FMC_SDRAM_FLAG_REFRESH_ERROR);     // Clear SDRAM refresh error interrupt pending bit
    }
}


//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteProtectionEnable
//
//   Parameter(s):  None
//   Return:        SystemState_e
//
//   Description:   Enables dynamically SDRAM write protection.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
/*
SystemState_e SDRAM::WriteProtectionEnable(void)
{
    if(m_State == SYS_BUSY)                                                     // Check the SDRAM controller state
    {
        return SYS_BUSY;
    }

    m_State = SYS_BUSY;                                                         // Update the SDRAM state
    FMC_SDRAM_WriteProtection_Enable(hsdram->Instance, hsdram->Init.SDBank);    // Enable write protection
    m_State = SYS_PROTECTED;                                                    // Update the SDRAM state

    return SYS_OK;
}
*/
//-------------------------------------------------------------------------------------------------
//
//   Function name:
//
//   Parameter(s):  None
//   Return:        SystemState_e
//
//   Description:   Disables dynamically SDRAM write protection.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
/*
SystemState_e SDRAM::WriteProtectionDisable(void)
{
    if(m_State == SYS_BUSY)                                                     // Check the SDRAM controller state
    {
        return SYS_BUSY;
    }

    m_State = SYS_BUSY;                                                         // Update the SDRAM state
    FMC_SDRAM_WriteProtection_Disable(hsdram->Instance, hsdram->Init.SDBank);   // Disable write protection
    m_State = SYS_READY;                                                        // Update the SDRAM state

    return SYS_OK;
}
*/
//-------------------------------------------------------------------------------------------------
//
//   Function name:
//
//   Parameter(s):  Command SDRAM command structure
//                  Timeout Timeout duration
//   Return:        SystemState_e
//
//   Description:   Sends Command to the SDRAM bank.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDRAM::SendCommand(FMC_SDRAM_CommandTypeDef* Command, uint32_t Timeout)
{
    if(m_State == SYS_BUSY)                                                 // Check the SDRAM controller state
    {
        return SYS_BUSY;
    }

    m_State = SYS_BUSY;                                                     // Update the SDRAM state
    FMC_SDRAM_SendCommand(hsdram->Instance, Command, Timeout);              // Send SDRAM command

    if(Command->CommandMode == FMC_SDRAM_CMD_PALL)                          // Update the SDRAM controller state state
    {
        m_State = SYS_PRECHARGED;
    }
    else
    {
        m_State = SYS_READY;
    }

    return SYS_OK;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name:
//
//   Parameter(s):  None
//   Return:        SystemState_e
//
//   Description:   Programs the SDRAM Memory Refresh rate.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDRAM::ProgramRefreshRate(uint32_t RefreshRate)
{
    if(m_State == SYS_BUSY)                                                     // Check the SDRAM controller state
    {
        return SYS_BUSY;
    }

    m_State = SYS_BUSY;                                                         // Update the SDRAM state
    FMC_SDRAM_ProgramRefreshRate(hsdram->Instance ,RefreshRate);                // Program the refresh rate
    m_State = SYS_READY;                                                        // Update the SDRAM state

    return SYS_OK;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SetAutoRefreshNumber
//
//   Parameter(s):  AutoRefreshNumber The SDRAM auto Refresh number
//   Return:        SystemState_e
//
//   Description:   Sets the Number of consecutive SDRAM Memory auto Refresh commands.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDRAM::SetAutoRefreshNumber(uint32_t AutoRefreshNumber)
{
    if(m_State == SYS_BUSY)                                                     // Check the SDRAM controller state
    {
        return SYS_BUSY;
    }

    m_State = SYS_BUSY;                                                         // Update the SDRAM state
    FMC_SDRAM_SetAutoRefreshNumber(hsdram->Instance ,AutoRefreshNumber);        // Set the Auto-Refresh number
    m_State = SYS_READY;                                                        // Update the SDRAM state

    return SYS_OK;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetModeStatus
//
//   Parameter(s):  None
//   Return:        uint32_t    The SDRAM memory mode.
//
//   Description:   Returns the SDRAM memory current mode.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
uint32_t SDRAM::GetModeStatus(void)
{
    return FMC_SDRAM_GetModeStatus(hsdram->Instance, hsdram->Init.SDBank);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetState
//
//   Parameter(s):  None
//   Return:        SystemState_e
//
//   Description:   Returns the SDRAM state.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDRAM::GetState(void)
{
    return m_State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_RefreshErrorCallback
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   SDRAM Refresh error callback.
//
//   Notes:         This function Should not be modified, when the callback is needed, the
//                  SDRAM_RefreshErrorCallback could be implemented in the user file
//
//-------------------------------------------------------------------------------------------------
__weak void SDRAM_RefreshErrorCallback(void)
{
     __asm("nop");
}

//-------------------------------------------------------------------------------------------------

