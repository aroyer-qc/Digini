//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_fmc.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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
//
//  Note(s)
//
//  This file provides firmware functions to manage the following
//  functionalities of the Flexible Memory Controller (FMC) peripheral memories:
//   - Initialization functions
//   - Peripheral Control functions
//   - Peripheral State functions
//
//=================================================================================================
//
//  The Flexible memory controller (FMC) includes three memory controllers:
//   - The NOR/PSRAM memory controller
//   - The NAND memory controller
//   - The Synchronous DRAM (SDRAM) controller
//
//  The FMC functional block makes the interface with synchronous and asynchronous static memories,
//  SDRAM memories, and 16-bit PC memory cards. Its main purposes are:
//   - To translate AHB transactions into the appropriate external device protocol
//   - To meet the access time requirements of the external memory devices
//
//  All external memories share the addresses, data and control signals with the controller.
//  Each external device is accessed by means of a unique Chip Select. The FMC performs only one
//  access at a time to an external device.
//       The main features of the FMC controller are the following:
//        - Interface with static-memory mapped devices including:
//           - Static random access memory (SRAM)
//           - Read-only memory (ROM)
//           - NOR Flash memory/OneNAND Flash memory
//           - PSRAM (4 memory banks)
//           - 16-bit PC Card compatible devices
//           - Two banks of NAND Flash memory with ECC hardware to check up to 8 Kbytes of data
//        - Interface with synchronous DRAM (SDRAM) memories
//        - Independent Chip Select control for each memory bank
//        - Independent configuration for each memory bank
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
#include "./Digini/Peripheral/inc/port/lib_class_fmc.h"

//-------------------------------------------------------------------------------------------------

#if defined (DIGINI_USE_SRAM_MODULE) || defined(DIGINI_USE_NOR_MODULE) || defined(DIGINI_USE_NAND_MODULE) || defined(DIGINI_USE_SDRAM_MODULE)

//-------------------------------------------------------------------------------------------------
//
//   Section name:  NORSRAM device driver
//
//    Notes:        ##### How to use NORSRAM device driver #####
//
//   This driver contains a set of APIs to interface with the FMC NORSRAM banks in order to run the
//   NORSRAM external devices.
//
//    - Bank control configuration using the function NORSRAM_Init()
//    - Bank timing configuration using the function NORSRAM_TimingInit()
//    - Bank extended timing configuration using the function NORSRAM_ExtendedTimingInit()
//    - Bank enable/disable write operation using the functions NORSRAM_WriteOperationEnable() /
//      NORSRAM_WriteOperationDisable()
//
//-------------------------------------------------------------------------------------------------
#if DIGINI_USE_NOR_MODULE

//-------------------------------------------------------------------------------------------------
//
//   Function name: NORSRAM_Init
//
//   Parameter(s):  Device      Pointer to NORSRAM device instance
//                  Init        Pointer to NORSRAM Initialization structure
//   Return:        None
//
//   Description:   nitialize the FMC_NORSRAM device according to the specified control parameters
//                  in the FMC_NORSRAM_Init_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NORSRAM_Init(FMC_NORSRAM_t* pDevice, FMC_NORSRAM_InitTypeDef* Init)
{
    uint32_t tmpr = 0;

    m_pNORSRAM_Device = pDevice;

    // Get the BTCR register value
    tmpr = pDevice->BTCR[Init->NSBank];

    // Clear MBKEN, MUXEN, MTYP, MWID, FACCEN, BURSTEN, WAITPOL, WAITCFG, WREN, WAITEN, EXTMOD, ASYNCWAIT, CBURSTRW and CCLKEN bits
    tmpr &= ((uint32_t)~(FMC_BCR1_MBKEN     | FMC_BCR1_MUXEN    | FMC_BCR1_MTYP     |
                         FMC_BCR1_MWID      | FMC_BCR1_FACCEN   | FMC_BCR1_BURSTEN  |
                         FMC_BCR1_WAITPOL   | FMC_BCR1_CPSIZE   | FMC_BCR1_WAITCFG  |
                         FMC_BCR1_WREN      | FMC_BCR1_WAITEN   | FMC_BCR1_EXTMOD   |
                         FMC_BCR1_ASYNCWAIT | FMC_BCR1_CBURSTRW | FMC_BCR1_CCLKEN   | FMC_BCR1_WFDIS));

    // Set NORSRAM device control parameters
    tmpr |= (uint32_t)(Init->DataAddressMux       |
                       Init->MemoryType           |
                       Init->MemoryDataWidth      |
                       Init->BurstAccessMode      |
                       Init->WaitSignalPolarity   |
                       Init->WaitSignalActive     |
                       Init->WriteOperation       |
                       Init->WaitSignal           |
                       Init->ExtendedMode         |
                       Init->AsynchronousWait     |
                       Init->WriteBurst           |
                       Init->ContinuousClock      |
                       Init->PageSize             |
                       Init->WriteFifo);

    if(Init->MemoryType == FMC_MEMORY_TYPE_NOR)
    {
        tmpr |= (uint32_t)FMC_NORSRAM_FLASH_ACCESS_ENABLE;
    }

    pDevice->BTCR[Init->NSBank] = tmpr;

    // Configure synchronous mode when Continuous clock is enabled for bank2..4
    if((Init->ContinuousClock == FMC_CONTINUOUS_CLOCK_SYNC_ASYNC) && (Init->NSBank != FMC_NORSRAM_BANK1))
    {
        pDevice->BTCR[FMC_NORSRAM_BANK1] |= (uint32_t)(Init->ContinuousClock);
    }

    if(Init->NSBank != FMC_NORSRAM_BANK1)
    {
        pDevice->BTCR[FMC_NORSRAM_BANK1] |= (uint32_t)(Init->WriteFifo);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NORSRAM_TimingInit
//
//   Parameter(s):  Timing      Pointer to NORSRAM Timing structure
//                  Bank        NORSRAM bank number
//   Return:        None
//
//   Description:   Initialize the FMC_NORSRAM Timing according to the specified parameters in the
//                  FMC_NORSRAM_Timing_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NORSRAM_TimingInit(FMC_NORSRAM_Timing_t* Timing, uint32_t Bank)
{
    uint32_t tmpr = 0;

    // Get the BTCR register value
    tmpr = m_pNORSRAM_Device->BTCR[Bank + 1];

    // Clear ADDSET, ADDHLD, DATAST, BUSTURN, CLKDIV, DATLAT and ACCMOD bits
    tmpr &= ((uint32_t)~(FMC_BTR1_ADDSET  | FMC_BTR1_ADDHLD | FMC_BTR1_DATAST |
                         FMC_BTR1_BUSTURN | FMC_BTR1_CLKDIV | FMC_BTR1_DATLAT |
                         FMC_BTR1_ACCMOD));

    // Set FMC_NORSRAM device timing parameters
    tmpr |= (uint32_t)(Timing->AddressSetupTime                  |
                       ((Timing->AddressHoldTime) << 4)          |
                       ((Timing->DataSetupTime) << 8)            |
                       ((Timing->BusTurnAroundDuration) << 16)   |
                       (((Timing->CLKDivision)-1) << 20)         |
                       (((Timing->DataLatency)-2) << 24)         |
                       (Timing->AccessMode));

    m_pNORSRAM_Device->BTCR[Bank + 1] = tmpr;

    // Configure Clock division value (in NORSRAM bank 1) when continuous clock is enabled
    if(HAL_IS_BIT_SET(m_pNORSRAM_Device->BTCR[FMC_NORSRAM_BANK1], FMC_BCR1_CCLKEN))
    {
        tmpr = (uint32_t)(m_pNORSRAM_Device->BTCR[FMC_NORSRAM_BANK1 + 1] & ~(((uint32_t)0x0F) << 20));
        tmpr |= (uint32_t)(((Timing->CLKDivision)-1) << 20);
        m_pNORSRAM_Device->BTCR[FMC_NORSRAM_BANK1 + 1] = tmpr;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: FMC_NORSRAM_ExtendedTimingInit
//
//   Parameter(s):  Timing      Pointer to NORSRAM Timing structure
//                  Bank        NORSRAM bank number
//   Return:        None
//
//   Description:   Initialize the FMC_NORSRAM Extended mode Timing according to the specified
//                  parameters in the FMC_NORSRAM_Timing_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC_NORSRAM_ExtendedTimingInit(FMC_NORSRAM_Timing_t* Timing, uint32_t Bank, uint32_t ExtendedMode)
{
    uint32_t tmpr = 0;

    // Set NORSRAM device timing register for write configuration, if extended mode is used
    if(ExtendedMode == FMC_EXTENDED_MODE_ENABLE)
    {
        // Get the BWTR register value
        tmpr = m_pNORSRAM_Device->BWTR[Bank];

        // Clear ADDSET, ADDHLD, DATAST, BUSTURN, CLKDIV, DATLAT and ACCMOD bits
        tmpr &= ((uint32_t)~(FMC_BWTR1_ADDSET  | FMC_BWTR1_ADDHLD | FMC_BWTR1_DATAST | FMC_BWTR1_BUSTURN | FMC_BWTR1_ACCMOD));

        tmpr |= (uint32_t)(Timing->AddressSetupTime                  |
                           ((Timing->AddressHoldTime) << 4)          |
                           ((Timing->DataSetupTime) << 8)            |
                           ((Timing->BusTurnAroundDuration) << 16)   |
                           (Timing->AccessMode));

        m_pNORSRAM_Device->BWTR[Bank] = tmpr;
    }
    else
    {
        m_pNORSRAM_Device->BWTR[Bank] = 0x0FFFFFFF;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NORSRAM_WriteOperationEnable
//
//   Parameter(s):  Bank        NORSRAM bank number
//   Return:        None
//
//   Description:   Enables dynamically FMC_NORSRAM write operation.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NORSRAM_WriteOperationEnable(uint32_t Bank)
{
    // Enable write operation
    m_pNORSRAM_Device->BTCR[Bank] |= FMC_WRITE_OPERATION_ENABLE;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NORSRAM_WriteOperationDisable
//
//   Parameter(s):  Bank        NORSRAM bank number
//   Return:        None
//
//   Description:   Disables dynamically FMC_NORSRAM write operation.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NORSRAM_WriteOperationDisable(uint32_t Bank)
{
    // Disable write operation
    m_pNORSRAM_Device->BTCR[Bank] &= ~FMC_WRITE_OPERATION_ENABLE;
}

#endif

//-------------------------------------------------------------------------------------------------
//
//   Section name:  NAND device driver
//
//    Notes:        ##### How to use NAND device driver #####
//
//   This driver contains a set of APIs to interface with the FMC NAND banks in order to run the
//   NAND external devices.
//
//    - Bank control configuration using the function NAND_Init()
//    - Bank common space timing configuration using the function NAND_CommonSpaceTimingInit()
//    - Bank attribute space timing configuration using the function
//      NAND_AttributeSpaceTimingInit()
//    - Bank enable/disable ECC correction feature using the functions NAND_ECC_Enable()/
//      NAND_ECC_Disable()
//    - Bank get ECC correction code using the function NAND_GetECC()
//
//-------------------------------------------------------------------------------------------------
#if DIGINI_USE_NAND_MODULE

//-------------------------------------------------------------------------------------------------
//
//   Function name: NAND_Init
//
//   Parameter(s):  Device      Pointer to NAND device instance
//                  Init        Pointer to NAND Initialization structure
//   Return:        None
//
//   Description:   Initializes the FMC_NAND device according to the specified control parameters
//                  in the FMC_NAND_Init_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NAND_Init(FMC_NAND_t* pDevice, FMC_NAND_Init_t* Init)
{
    uint32_t tmpr  = 0;

    m_pNAND_Device = pDevice;

    // Get the NAND bank 3 register value
    tmpr = pDevice->PCR;

    // Clear PWAITEN, PBKEN, PTYP, PWID, ECCEN, TCLR, TAR and ECCPS bits
    tmpr &= ((uint32_t)~(FMC_PCR_PWAITEN  | FMC_PCR_PBKEN | FMC_PCR_PTYP |
                         FMC_PCR_PWID | FMC_PCR_ECCEN | FMC_PCR_TCLR |
                         FMC_PCR_TAR | FMC_PCR_ECCPS));
    // Set NAND device control parameters
    tmpr |= (uint32_t)(Init->Waitfeature                |
                       FMC_PCR_MEMORY_TYPE_NAND         |
                       Init->MemoryDataWidth            |
                       Init->EccComputation             |
                       Init->ECCPageSize                |
                       ((Init->TCLRSetupTime) << 9)     |
                       ((Init->TARSetupTime) << 13));

    // NAND bank 3 registers configuration
    pDevice->PCR  = tmpr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NAND_CommonSpaceTimingInit
//
//   Parameter(s):  Timing      Pointer to NAND timing structure
//                  Bank        NAND bank number
//   Return:        None
//
//   Description:   Initializes the FMC_NAND Common space Timing according to the specified
//                  parameters in the FMC_NAND_PCC_Timing_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NAND_CommonSpaceTimingInit(FMC_NAND_PCC_Timing_t* Timing, uint32_t Bank)
{
    uint32_t tmpr = 0;

    // Get the NAND bank 3 register value
    tmpr = m_pNAND_Device->PMEM;

    // Clear MEMSETx, MEMWAITx, MEMHOLDx and MEMHIZx bits
    tmpr &= ((uint32_t)~(FMC_PMEM_MEMSET3  | FMC_PMEM_MEMWAIT3 | FMC_PMEM_MEMHOLD3 | FMC_PMEM_MEMHIZ3));
    // Set FMC_NAND device timing parameters
    tmpr |= (uint32_t)(Timing->SetupTime                  |
                       ((Timing->WaitSetupTime) << 8)     |
                       ((Timing->HoldSetupTime) << 16)    |
                       ((Timing->HiZSetupTime) << 24));

    // NAND bank 3 registers configuration
    m_pNAND_Device->PMEM = tmpr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NAND_AttributeSpaceTimingInit
//
//   Parameter(s):  Device      Pointer to NAND device instance
//                  Bank        NAND bank number
//   Return:        None
//
//   Description:   Initializes the FMC_NAND Attribute space Timing according to the specified
//                  parameters in the FMC_NAND_PCC_Timing_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NAND_AttributeSpaceTimingInit(FMC_NAND_PCC_Timing_t* Timing, uint32_t Bank)
{
    uint32_t tmpr = 0;

    // Get the NAND bank 3 register value
    tmpr = m_pNAND_Device->PATT;

    // Clear ATTSETx, ATTWAITx, ATTHOLDx and ATTHIZx bits
    tmpr &= ((uint32_t)~(FMC_PATT_ATTSET3  | FMC_PATT_ATTWAIT3 | FMC_PATT_ATTHOLD3 | FMC_PATT_ATTHIZ3));

    // Set FMC_NAND device timing parameters
    tmpr |= (uint32_t)(Timing->SetupTime                  |
                       ((Timing->WaitSetupTime) << 8)     |
                       ((Timing->HoldSetupTime) << 16)    |
                       ((Timing->HiZSetupTime) << 24));

    // NAND bank 3 registers configuration
    m_pNAND_Device->PATT = tmpr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NAND_ECC_Enable
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   Enables dynamically FMC_NAND ECC feature.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NAND_ECC_Enable(void)
{
    m_pNAND_Device->PCR |= FMC_PCR_ECCEN;           // Enable ECC feature
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NAND_ECC_Disable
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   Disables dynamically FMC_NAND ECC feature.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::NAND_ECC_Disable(void)
{
    m_pNAND_Device->PCR &= ~FMC_PCR_ECCEN;          // Disable ECC feature
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: NAND_GetECC
//
//   Parameter(s):  ECCval      Pointer to ECC value
//                  Bank        NAND bank number
//                  Timeout     Timeout wait value
//   Return:        None
//
//   Description:   Disables dynamically FMC_NAND ECC feature.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
SystemState_e FMC::NAND_GetECC(uint32_t* ECCval, uint32_t Bank, uint32_t Timeout)
{
    uint32_t tickstart = 0;

    // Get tick
    tickstart = HAL_GetTick();

    // Wait until FIFO is empty
    while(__FMC_NAND_GET_FLAG(m_pNAND_Device, Bank, FMC_FLAG_FEMPT) == RESET)
    {
        // Check for the Timeout
        if(Timeout != HAL_MAX_DELAY)
        {
            if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
            {
                return SYS_TIMEOUT;
            }
        }
    }

    // Get the ECCR register value
    *ECCval = (uint32_t)m_pNAND_Device->ECCR;

    return SYS_OK;
}

#endif // DIGINI_USE_NAND_MODULE

//-------------------------------------------------------------------------------------------------
//
//   Section name:  SDRAM device driver
//
//    Notes:        ##### How to use SDRAM device driver #####
//
//   This driver contains a set of APIs to interface with the FMC SDRAM banks in order to run the
//   SDRAM external devices.
//
//
//    - Bank control configuration using the function SDRAM_Init()
//    - Bank timing configuration using the function SDRAM_TimingInit()
//    - Bank enable/disable write operation using the functionsSDRAM_WriteOperation_Enable() /
//      SDRAM_WriteOperationDisable()
//    - Bank send command using the function SDRAM_SendCommand()
//
//-------------------------------------------------------------------------------------------------
#if DIGINI_USE_SDRAM_MODULE

//-------------------------------------------------------------------------------------------------
//
//   Function name:
//
//   Parameter(s):  Device      Pointer to SDRAM device instance
//                  Init        Pointer to SDRAM Initialization structure
//   Return:        None
//
//   Description:   Initializes the FMC_SDRAM device according to the specified control parameters
//                  in the FMC_SDRAM_Init_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::SDRAM_Init(FMC_SDRAM_t* pDevice, FMC_SDRAM_Init_t* Init)
{
    uint32_t tmpr1 = 0;
    uint32_t tmpr2 = 0;

    m_pSDRAM_Device = pDevice;

    // Set SDRAM bank configuration parameters
    if(Init->SDBank != FMC_SDRAM_BANK2)
    {
        tmpr1 = pDevice->SDCR[FMC_SDRAM_BANK1];

        // Clear NC, NR, MWID, NB, CAS, WP, SDCLK, RBURST, and RPIPE bits
        tmpr1 &= ((uint32_t)~(FMC_SDCR1_NC   | FMC_SDCR1_NR      | FMC_SDCR1_MWID |
                              FMC_SDCR1_NB   | FMC_SDCR1_CAS     | FMC_SDCR1_WP   |
                              FMC_SDCR1_SDCLK | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE));

        tmpr1 |= (uint32_t)(Init->ColumnBitsNumber   |
                            Init->RowBitsNumber      |
                            Init->MemoryDataWidth    |
                            Init->InternalBankNumber |
                            Init->CASLatency         |
                            Init->WriteProtection    |
                            Init->SDClockPeriod      |
                            Init->ReadBurst          |
                            Init->ReadPipeDelay);
        pDevice->SDCR[FMC_SDRAM_BANK1] = tmpr1;
    }
    else // FMC_Bank2_SDRAM
    {
        tmpr1 = pDevice->SDCR[FMC_SDRAM_BANK1];

        // Clear SDCLK, RBURST, and RPIPE bits
        tmpr1 &= ((uint32_t)~(FMC_SDCR1_SDCLK | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE));

        tmpr1 |= (uint32_t)(Init->SDClockPeriod      |
                            Init->ReadBurst          |
                            Init->ReadPipeDelay);

        tmpr2 = pDevice->SDCR[FMC_SDRAM_BANK2];

        // Clear NC, NR, MWID, NB, CAS, WP, SDCLK, RBURST, and RPIPE bits
        tmpr2 &= ((uint32_t)~(FMC_SDCR1_NC    | FMC_SDCR1_NR     | FMC_SDCR1_MWID |
                              FMC_SDCR1_NB    | FMC_SDCR1_CAS    | FMC_SDCR1_WP   |
                              FMC_SDCR1_SDCLK | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE));

        tmpr2 |= (uint32_t)(Init->ColumnBitsNumber    |
                            Init->RowBitsNumber       |
                            Init->MemoryDataWidth     |
                            Init->InternalBankNumber  |
                            Init->CASLatency          |
                            Init->WriteProtection);

        pDevice->SDCR[FMC_SDRAM_BANK1] = tmpr1;
        pDevice->SDCR[FMC_SDRAM_BANK2] = tmpr2;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_TimingInit
//
//   Parameter(s):  Timing      Pointer to SDRAM Timing structure
//                  Bank        SDRAM bank number
//   Return:        None
//
//   Description:   Initializes the FMC_SDRAM device timing according to the specified parameters
//                  in the FMC_SDRAM_Timing_t
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::SDRAM_TimingInit(FMC_SDRAM_Timing_t* Timing, uint32_t Bank)
{
    uint32_t tmpr1 = 0;
    uint32_t tmpr2 = 0;

    // Set SDRAM m_pSDRAM_Device timing parameters
    if(Bank != FMC_SDRAM_BANK2)
    {
        tmpr1 = m_pSDRAM_Device->SDTR[FMC_SDRAM_BANK1];

        // Clear TMRD, TXSR, TRAS, TRC, TWR, TRP and TRCD bits
        tmpr1 &= ((uint32_t)~(FMC_SDTR1_TMRD | FMC_SDTR1_TXSR | FMC_SDTR1_TRAS |
                              FMC_SDTR1_TRC  | FMC_SDTR1_TWR  | FMC_SDTR1_TRP  |
                              FMC_SDTR1_TRCD));

        tmpr1 |= (uint32_t)(((Timing->LoadToActiveDelay)-1)           |
                            (((Timing->ExitSelfRefreshDelay)-1) << 4) |
                            (((Timing->SelfRefreshTime)-1) << 8)      |
                            (((Timing->RowCycleDelay)-1) << 12)       |
                            (((Timing->WriteRecoveryTime)-1) <<16)    |
                            (((Timing->RPDelay)-1) << 20)             |
                            (((Timing->RCDDelay)-1) << 24));
        m_pSDRAM_Device->SDTR[FMC_SDRAM_BANK1] = tmpr1;
    }
    else // FMC_Bank2_SDRAM
    {
        tmpr1 = m_pSDRAM_Device->SDTR[FMC_SDRAM_BANK1];

        // Clear TRC and TRP bits
        tmpr1 &= ((uint32_t)~(FMC_SDTR1_TRC | FMC_SDTR1_TRP));

        tmpr1 |= (uint32_t)((((Timing->RowCycleDelay)-1) << 12)       |
                            (((Timing->RPDelay)-1) << 20));

        tmpr2 = m_pSDRAM_Device->SDTR[FMC_SDRAM_BANK2];

        // Clear TMRD, TXSR, TRAS, TRC, TWR, TRP and TRCD bits
        tmpr2 &= ((uint32_t)~(FMC_SDTR1_TMRD | FMC_SDTR1_TXSR | FMC_SDTR1_TRAS |
                              FMC_SDTR1_TRC  | FMC_SDTR1_TWR  | FMC_SDTR1_TRP  |
                              FMC_SDTR1_TRCD));

        tmpr2 |= (uint32_t)(((Timing->LoadToActiveDelay)-1)            |
                            (((Timing->ExitSelfRefreshDelay)-1) << 4)  |
                            (((Timing->SelfRefreshTime)-1) << 8)       |
                            (((Timing->WriteRecoveryTime)-1) <<16)     |
                            (((Timing->RCDDelay)-1) << 24));

        m_pSDRAM_Device->SDTR[FMC_SDRAM_BANK1] = tmpr1;
        m_pSDRAM_Device->SDTR[FMC_SDRAM_BANK2] = tmpr2;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_WriteProtectionEnable
//
//   Parameter(s):  Bank        SDRAM bank number
//   Return:        None
//
//   Description:   Enables dynamically FMC_SDRAM write protection.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::SDRAM_WriteProtectionEnable(uint32_t Bank)
{
    m_pSDRAM_Device->SDCR[Bank] |= FMC_SDRAM_WRITE_PROTECTION_ENABLE;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_WriteProtectionDisable
//
//   Parameter(s):  Bank        SDRAM bank number
//   Return:        None
//
//   Description:   Disables dynamically FMC_SDRAM write protection.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::SDRAM_WriteProtectionDisable(uint32_t Bank)
{
    m_pSDRAM_Device->SDCR[Bank] &= ~FMC_SDRAM_WRITE_PROTECTION_ENABLE;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_SendCommand
//
//   Parameter(s):  Command         Pointer to SDRAM command structure
//                  Timing          Pointer to SDRAM Timing structure
//                  Timeout         Timeout wait value
//   Return:        None
//
//   Description:   Send Command to the FMC SDRAM bank
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::SDRAM_SendCommand(FMC_SDRAM_Command_t* Command, uint32_t Timeout)
{
    volatile uint32_t tmpr = 0;

    // Set command register
    tmpr = (uint32_t)((Command->CommandMode)                  |
                      (Command->CommandTarget)                |
                      (((Command->AutoRefreshNumber)-1) << 5) |
                      ((Command->ModeRegisterDefinition) << 9));

    m_pSDRAM_Device->SDCMR = tmpr;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_ProgramRefreshRate
//
//   Parameter(s):  RefreshRate     The SDRAM refresh rate value.
//   Return:        None
//
//   Description:   Program the SDRAM Memory Refresh rate.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::SDRAM_ProgramRefreshRate(uint32_t RefreshRate)
{
    m_pSDRAM_Device->SDRTR |= (RefreshRate << 1);                               // Set the refresh rate in command register
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_SetAutoRefreshNumber
//
//   Parameter(s):  AutoRefreshNumber       Specifies the auto Refresh number.
//   Return:        None
//
//   Description:   Set the Number of consecutive SDRAM Memory auto Refresh commands.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
void FMC::SDRAM_SetAutoRefreshNumber(uint32_t AutoRefreshNumber)
{
    m_pSDRAM_Device->SDCMR |= (AutoRefreshNumber << 5);                         // Set the Auto-refresh number in command register
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_GetModeStatus
//
//   Parameter(s):  Bank        SDRAM bank number
//   Return:        The FMC SDRAM bank mode status, could be on of the following values:
//                          - FMC_SDRAM_NORMAL_MODE, FMC_SDRAM_SELF_REFRESH_MODE or
//                          - FMC_SDRAM_POWER_DOWN_MODE.
//
//   Description:   Returns the indicated FMC SDRAM bank mode status.
//
//   Notes:
//
//-------------------------------------------------------------------------------------------------
uint32_t FMC::SDRAM_GetModeStatus(uint32_t Bank)
{
    uint32_t tmpreg = 0;

    if(Bank == FMC_SDRAM_BANK1)                                                 // Get the corresponding bank mode
    {
        tmpreg = (uint32_t)(m_pSDRAM_Device->SDSR & FMC_SDSR_MODES1);
    }
    else
    {
        tmpreg = ((uint32_t)(m_pSDRAM_Device->SDSR & FMC_SDSR_MODES2) >> 2);
    }

     return tmpreg;                                                             // Return the mode status
}


#endif // DIGINI_USE_SDRAM_MODULE

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_SRAM_MODULE) || (DIGINI_USE_NOR_MODULE) || (DIGINI_USE_NAND_MODULE) || (DIGINI_USE_SDRAM_MODULE)
