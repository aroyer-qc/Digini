//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_sdram.cpp
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

#if (USE_SDRAM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define BTR_CLEAR_MASK          ((uint32_t)(FMC_BTR_ADDSET | FMC_BTR_ADDHLD | FMC_BTR_DATAST | FMC_BTR_BUSTURN | FMC_BTR_ACCMOD))                                                           // BTR register clear mask
#define BWTR_CLEAR_MASK         ((uint32_t)(FMC_BWTR_ADDSET | FMC_BWTR_ADDHLD | FMC_BWTR_DATAST | FMC_BWTR_BUSTURN | FMC_BWTR_ACCMOD))                                                      // BWTR register clear mask
#define PCR_CLEAR_MASK          ((uint32_t)(FMC_PCR_PWAITEN | FMC_PCR_PBKEN | FMC_PCR_PWID | FMC_PCR_ECCEN | FMC_PCR_TCLR | FMC_PCR_TAR | FMC_PCR_ECCPS))                                   // PCR register clear mask
#define PMEM_CLEAR_MASK         ((uint32_t)(FMC_PMEM_MEMSET | FMC_PMEM_MEMWAIT | FMC_PMEM_MEMHOLD | FMC_PMEM_MEMHIZ))                                                                       // PMEM register clear mask
#define PATT_CLEAR_MASK         ((uint32_t)(FMC_PATT_ATTSET | FMC_PATT_ATTWAIT | FMC_PATT_ATTHOLD | FMC_PATT_ATTHIZ))                                                                       // PATT register clear mask
#define SDCR_CLEAR_MASK         ((uint32_t)(FMC_SDCR_NC | FMC_SDCR_NR | FMC_SDCR_MWID | FMC_SDCR_NB | FMC_SDCR_CAS | FMC_SDCR_WP | FMC_SDCR_SDCLK | FMC_SDCR_RBURST | FMC_SDCR_RPIPE))      // SDCR register clear mask
#define SDTR_CLEAR_MASK         ((uint32_t)(FMC_SDTR_TMRD | FMC_SDTR_TXSR | FMC_SDTR_TRAS | FMC_SDTR_TRC | FMC_SDTR_TWR | FMC_SDTR_TRP | FMC_SDTR_TRCD))                                    // SDTR register clear mask
#define SDTR_TIMING_CLEAR_MASK  ((uint32_t)(FMC_SDTR_TRC | FMC_SDTR_TRP))                                                                                                                   // SDTR register clear mask for timing

// FMC SDRAM Command Mode
#define SDCMR_CMD_NORMAL_MODE                   (0x00000000U)
#define SDCMR_CMD_CLK_ENABLE                    (0x00000001U)
#define SDCMR_CMD_PALL                          (0x00000002U)
#define SDCMR_CMD_AUTO_REFRESH_MODE             (0x00000003U)
#define SDCMR_CMD_LOAD_MODE                     (0x00000004U)
#define SDCMR_CMD_SELF_REFRESH_MODE             (0x00000005U)
#define SDCMR_CMD_POWER_DOWN_MODE               (0x00000006U)

#if (CFG_SDRAM_BANK == FMC_SDRAM_BANK1)
  #define SDCMR_BANK                            FMC_SDCMR_CTB1
#elif (CFG_SDRAM_BANK == FMC_SDRAM_BANK2)
  #define SDCMR_BANK                            FMC_SDCMR_CTB2
#else
  #define SDCMR_BANK                            (FMC_SDCMR_CTB1 | FMC_SDCMR_CTB2)
#endif

#define SDCMR_MRD                               (CFG_SDRAM_MRD_WRITE_BURST_MODE | CFG_SDRAM_MRD_OPERATION_MODE | CFG_SDRAM_MRD_CAS_LATENCY | CFG_SDRAM_MRD_BURST_TYPE | CFG_SDRAM_MRD_BURST_LENGTH)

// SDTR timing macros (STM32F7 naming)
#define SDTR_LOAD_TO_ACTIVITY_DELAY             ((uint32_t)(CFG_SDRAM_LOAD_TO_ACTIVITY_DELAY  - 1U))
#define SDTR_EXIT_SELF_REFRESH_DELAY            ((uint32_t)(CFG_SDRAM_EXIT_SELF_REFRESH_DELAY - 1U) << FMC_SDTR1_TXSR_Pos)
#define SDTR_SELF_REFRESH_TIME                  ((uint32_t)(CFG_SDRAM_SELF_REFRESH_TIME       - 1U) << FMC_SDTR1_TRAS_Pos)
#define SDTR_ROW_CYCLE_DELAY                    ((uint32_t)(CFG_SDRAM_ROW_CYCLE_DELAY         - 1U) << FMC_SDTR1_TRC_Pos)
#define SDTR_WRITE_RECOVERY_TIME                ((uint32_t)(CFG_SDRAM_WRITE_RECOVERY_TIME     - 1U) << FMC_SDTR1_TWR_Pos)
#define SDTR_RP_DELAY                           ((uint32_t)(CFG_SDRAM_RP_DELAY                - 1U) << FMC_SDTR1_TRP_Pos)
#define SDTR_RCD_DELAY                          ((uint32_t)(CFG_SDRAM_RCD_DELAY               - 1U) << FMC_SDTR1_TRCD_Pos)

#define SDCMR_AUTO_REFRESH_CYCLE                ((uint32_t)(CFG_SDRAM_AUTO_REFRESH_CYCLE      - 1U) << FMC_SDCMR_NRFS_Pos)

#define SDRTR_REFRESH_COUNT                     ((uint32_t)(CFG_SDRAM_REFRESH_COUNT) << FMC_SDRTR_COUNT_Pos)

//-------------------------------------------------------------------------------------------------
//
//   Function name: FMC_SDRAM_Initialize
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   Performs the SDRAM device initialization sequence (STM32F7 FMC).
//
//-------------------------------------------------------------------------------------------------
void FMC_SDRAM_Initialize(void)
{
    // ---- FMC Reset ----
    RCC->AHB3RSTR |=  RCC_AHB3RSTR_FMCRST;
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_FMCRST;
    RCC->AHB3ENR  |=  RCC_AHB3ENR_FMCEN;                    // Enable Clock

  #if (CFG_SDRAM_BANK == FMC_SDRAM_BANK1)

    // Set SDRAM bank configuration parameters
    FMC_Bank5_6->SDCR[FMC_SDRAM_BANK1] = (CFG_SDRAM_COLUMN_BITS_NUMBER   |
                                          CFG_SDRAM_ROW_BITS_NUMBER      |
                                          CFG_SDRAM_MEMORY_DATA_WIDTH    |
                                          CFG_SDRAM_INTERNAL_BANK_NUMBER |
                                          CFG_SDRAM_CAS_LATENCY          |
                                          CFG_SDRAM_WRITE_PROTECTION     |
                                          CFG_SDRAM_SD_CLOCK_PERIOD      |
                                          CFG_SDRAM_READ_BURST           |
                                          CFG_SDRAM_PIPE_DELAY);

    // Set SDRAM device timing parameters
    FMC_Bank5_6->SDTR[FMC_SDRAM_BANK1] = (SDTR_LOAD_TO_ACTIVITY_DELAY    |
                                          SDTR_EXIT_SELF_REFRESH_DELAY   |
                                          SDTR_SELF_REFRESH_TIME         |
                                          SDTR_ROW_CYCLE_DELAY           |
                                          SDTR_WRITE_RECOVERY_TIME       |
                                          SDTR_RP_DELAY                  |
                                          SDTR_RCD_DELAY);

  #else // (CFG_SDRAM_BANK == FMC_SDRAM_BANK2)

    // Set SDRAM bank configuration parameters
    FMC_Bank5_6_R->SDCR[FMC_SDRAM_BANK1] = (CFG_SDRAM_SD_CLOCK_PERIOD |
                                            CFG_SDRAM_READ_BURST      |
                                            CFG_SDRAM_PIPE_DELAY);

    FMC_Bank5_6_R->SDCR[FMC_SDRAM_BANK2] = (CFG_SDRAM_COLUMN_BITS_NUMBER   |
                                            CFG_SDRAM_ROW_BITS_NUMBER      |
                                            CFG_SDRAM_MEMORY_DATA_WIDTH    |
                                            CFG_SDRAM_INTERNAL_BANK_NUMBER |
                                            CFG_SDRAM_CAS_LATENCY          |
                                            CFG_SDRAM_WRITE_PROTECTION);

    // Set SDRAM device timing parameters
    FMC_Bank5_6_R->SDTR[FMC_SDRAM_BANK1] = (SDTR_ROW_CYCLE_DELAY | SDTR_RP_DELAY);

    FMC_Bank5_6_R->SDTR[FMC_SDRAM_BANK2] = (SDTR_LOAD_TO_ACTIVITY_DELAY    |
                                            SDTR_EXIT_SELF_REFRESH_DELAY   |
                                            SDTR_SELF_REFRESH_TIME         |
                                            SDTR_WRITE_RECOVERY_TIME       |
                                            SDTR_RCD_DELAY);

  #endif

    // SDRAM initialization sequence
    FMC_Bank5_6->SDCMR = (FMC_SDRAM_CMD_CLK_ENABLE | FMC_SDRAM_CMD_TARGET_BANK1);                                           // Clock enable command
    
    for (int index = 0; index < 5000; index++);                                                                             // Delay (simple loop)

    FMC_Bank5_6->SDCMR = (FMC_SDRAM_CMD_PALL | FMC_SDRAM_CMD_TARGET_BANK1);                                                 // PALL command
   
    uint32_t AutoRefresh = (8 << FMC_SDCMR_NRFS_Pos);
    FMC_Bank5_6->SDCMR = (FMC_SDRAM_CMD_AUTOREFRESH_MODE | SDCMR_BANK | AutoRefresh);                                       // Auto refresh mode

    uint32_t ModeRegisterDefinition = (SDRAM_MODEREG_BURST_LENGTH_1          |
                                       SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                                       SDRAM_MODEREG_CAS_LATENCY_2           |
                                       SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                                       SDRAM_MODEREG_WRITEBURST_MODE_SINGLE) << FMC_SDCMR_MRD_Pos;
    
    FMC_Bank5_6->SDCMR = (FMC_SDRAM_CMD_LOAD_MODE | FMC_SDRAM_CMD_TARGET_BANK1 | AutoRefresh | ModeRegisterDefinition);     // Load mode
    FMC_Bank5_6->SDRTR = (CFG_SDRAM_REFRESH_COUNT << 1);                                                                    // Set refresh count
  //SET_BIT(FMC_Bank1_R->BTCR[0], FMC_BCR1_FMCEN);                                                                          // FMC controller Enable
}

//-------------------------------------------------------------------------------------------------

#endif // USE_SDRAM_DRIVER
