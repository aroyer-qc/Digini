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
// Information:
//
//
//      In the STM32H7 series, the memory banks are used to interface with different types of
//      external memory. Here's a brief overview of what each bank typically supports:
//
//          Bank 0: Often used for boot memory or internal flash memory.
//          Bank 1: Typically used for SDRAM (Synchronous Dynamic RAM).
//          Bank 2: Can be used for NOR Flash or additional SDRAM.
//          Bank 3: Often used for PSRAM (Pseudo Static RAM) or additional NOR Flash.
//          Bank 4: Can be used for NAND Flash or additional PSRAM.
//          Bank 5 and Bank 6: These are additional banks available in the STM32H7 series to
//                             support more extensive memory configurations, such as larger SDRAM
//                             or additional Flash memory.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// BTR register clear mask
#define BTR_CLEAR_MASK          ((uint32_t)(FMC_BTRx_ADDSET | FMC_BTRx_ADDHLD | FMC_BTRx_DATAST | FMC_BTRx_BUSTURN | FMC_BTRx_CLKDIV | FMC_BTRx_DATLAT | FMC_BTRx_ACCMOD))

// BWTR register clear mask
#define BWTR_CLEAR_MASK         ((uint32_t)(FMC_BWTRx_ADDSET | FMC_BWTRx_ADDHLD | FMC_BWTRx_DATAST | FMC_BWTRx_BUSTURN | FMC_BWTRx_ACCMOD))

// PCR register clear mask
#define PCR_CLEAR_MASK          ((uint32_t)(FMC_PCR_PWAITEN | FMC_PCR_PBKEN | FMC_PCR_PWID | FMC_PCR_ECCEN | FMC_PCR_TCLR | FMC_PCR_TAR | FMC_PCR_ECCPS))

// PMEM register clear mask
#define PMEM_CLEAR_MASK         ((uint32_t)(FMC_PMEM_MEMSET | FMC_PMEM_MEMWAIT | FMC_PMEM_MEMHOLD | FMC_PMEM_MEMHIZ))

// PATT register clear mask
#define PATT_CLEAR_MASK         ((uint32_t)(FMC_PATT_ATTSET | FMC_PATT_ATTWAIT | FMC_PATT_ATTHOLD | FMC_PATT_ATTHIZ))

// SDCR register clear mask
#define SDCR_CLEAR_MASK         ((uint32_t)(FMC_SDCRx_NC | FMC_SDCRx_NR | FMC_SDCRx_MWID | FMC_SDCRx_NB | FMC_SDCRx_CAS | FMC_SDCRx_WP | FMC_SDCRx_SDCLK | FMC_SDCRx_RBURST | FMC_SDCRx_RPIPE))

// SDTR register clear mask
#define SDTR_CLEAR_MASK         ((uint32_t)(FMC_SDTRx_TMRD | FMC_SDTRx_TXSR | FMC_SDTRx_TRAS | FMC_SDTRx_TRC | FMC_SDTRx_TWR | FMC_SDTRx_TRP | FMC_SDTRx_TRCD))

// SDTR register clear mask for timing
#define SDTR_TIMING_CLEAR_MASK  ((uint32_t)(FMC_SDTRx_TRC | FMC_SDTRx_TRP))


#define SDCMR_MRD               (CFG_FMC_SDRAM_MRD_WRITE_BURST_MODE | \
                                 CFG_FMC_SDRAM_MRD_OPERATION_MODE   | \
                                 CFG_FMC_SDRAM_MRD_CAS_LATENCY      | \
                                 CFG_FMC_SDRAM_MRD_BURST_TYPE       | \
                                 CFG_FMC_SDRAM_MRD_BURST_LENGTH)

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
void SDRAM::Initialize(FMC_SDRAM_Timing_t* Timing)
{
    // ---- FMC Reset ----
    RCC->AHB3RSTR |=  RCC_AHB3RSTR_FMCRST;
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_FMCRST;

    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;                      // Enable Clock

    // Set SDRAM bank configuration parameters
    if(Init->SDBank == FMC_SDRAM_BANK1)
    {
        // Set SDRAM bank configuration parameters
        MODIFY_REG(Device->SDCR[FMC_SDRAM_BANK1], SDCR_CLEAR_MASK,
               (Init->ColumnBitsNumber   |
                Init->RowBitsNumber      |
                Init->MemoryDataWidth    |
                Init->InternalBankNumber |
                Init->CASLatency         |
                Init->WriteProtection    |
                Init->SDClockPeriod      |
                Init->ReadBurst          |
                Init->ReadPipeDelay));
    
        // Set SDRAM device timing parameters
        MODIFY_REG(Device->SDTR[FMC_SDRAM_BANK1], SDTR_CLEAR_MASK,
                   (((Timing->LoadToActiveDelay) - 1)                                      |
                   (((Timing->ExitSelfRefreshDelay) - 1) << FMC_SDTRx_TXSR_Pos) |
                   (((Timing->SelfRefreshTime) - 1)      << FMC_SDTRx_TRAS_Pos) |
                   (((Timing->RowCycleDelay) - 1)        << FMC_SDTRx_TRC_Pos)  |
                   (((Timing->WriteRecoveryTime) - 1)    << FMC_SDTRx_TWR_Pos)  |
                   (((Timing->RPDelay) - 1)              << FMC_SDTRx_TRP_Pos)  |
                   (((Timing->RCDDelay) - 1)             << FMC_SDTRx_TRCD_Pos)));
    
    }
    else /* FMC_Bank2_SDRAM */
    {
        // Set SDRAM bank configuration parameters
        MODIFY_REG(Device->SDCR[FMC_SDRAM_BANK1],  FMC_SDCRx_SDCLK | FMC_SDCRx_RBURST | FMC_SDCRx_RPIPE, (Init->SDClockPeriod | Init->ReadBurst | Init->ReadPipeDelay));
        MODIFY_REG(Device->SDCR[FMC_SDRAM_BANK2],  SDCR_CLEAR_MASK, (Init->ColumnBitsNumber | Init->RowBitsNumber | Init->MemoryDataWidth | Init->InternalBankNumber | Init->CASLatency | Init->WriteProtection));


        // Set SDRAM device timing parameters
        MODIFY_REG(Device->SDTR[FMC_SDRAM_BANK1], SDTR_TIMING_CLEAR_MASK,
                   (((Timing->RowCycleDelay) - 1)         << FMC_SDTRx_TRC_Pos)  |
                   (((Timing->RPDelay) - 1)               << FMC_SDTRx_TRP_Pos));

        MODIFY_REG(Device->SDTR[FMC_SDRAM_BANK2],
                   SDTR_CLEAR_MASK,
                   (((Timing->LoadToActiveDelay) - 1)                                      |
                    (((Timing->ExitSelfRefreshDelay) - 1) << FMC_SDTRx_TXSR_Pos) |
                    (((Timing->SelfRefreshTime) - 1)      << FMC_SDTRx_TRAS_Pos) |
                    (((Timing->WriteRecoveryTime) - 1)    << FMC_SDTRx_TWR_Pos)  |
                    (((Timing->RCDDelay) - 1)             << FMC_SDTRx_TRCD_Pos)));

    }
    
     
    
    
    
    SDRAM_Init(hsdram->Instance, &(hsdram->Init));                      // Initialize SDRAM control Interface
    SDRAM_TimingInit(Timing, hsdram->Init.SDBank);   // Initialize SDRAM timing Interface
    
    
    
    
    // SDRAM initialization sequence
    FMC_Bank5_6_R->SDCMR = (FMC_SDRAM_CMD_CLK_ENABLE | bank0 or 1 or both);                 // Clock enable command
    for(index = 0; index<5000; index++);                                                    // Delay
    FMC_Bank5_6_R->SDCMR = (FMC_SDRAM_CMD_PALL | bank0 or 1 or both);                       // PALL command
    FMC_Bank5_6_R->SDCMR = (FMC_SDRAM_CMD_AUTO_REFRESH_MODE | bank0 or 1 or both | ((CFG_SDRAM_AUTO_REFRESH_CYCLE - 1) << FMC_SDCMR_NRFS_Pos)); // Auto refresh mode
    FMC_Bank5_6_R->SDCMR = (FMC_SDRAM_CMD_LOAD_MODE | bank0 or 1 or both | SDCMR_MRD);      // Load mode
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
/*
void SDRAM::IRQHandler(void)
{
    if(__FMC_SDRAM_GET_FLAG(hsdram->Instance, FMC_SDRAM_FLAG_REFRESH_IT))           // Check SDRAM interrupt Rising edge flag
    {
        SDRAM_RefreshErrorCallback();                                               // SDRAM refresh error interrupt callback
        __FMC_SDRAM_CLEAR_FLAG(hsdram->Instance, FMC_SDRAM_FLAG_REFRESH_ERROR);     // Clear SDRAM refresh error interrupt pending bit
    }
}
*/

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

