//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H7_fmc_sdram.cpp
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

#define FMC_BTR_CLEAR_MASK                  ((uint32_t)(FMC_BTRx_ADDSET | FMC_BTRx_ADDHLD | FMC_BTRx_DATAST | FMC_BTRx_BUSTURN | FMC_BTRx_CLKDIV | FMC_BTRx_DATLAT | FMC_BTRx_ACCMOD))                          // BTR register clear mask
#define FMC_BWTR_CLEAR_MASK                 ((uint32_t)(FMC_BWTRx_ADDSET | FMC_BWTRx_ADDHLD | FMC_BWTRx_DATAST | FMC_BWTRx_BUSTURN | FMC_BWTRx_ACCMOD))                                                         // BWTR register clear mask
#define FMC_PCR_CLEAR_MASK                  ((uint32_t)(FMC_PCR_PWAITEN | FMC_PCR_PBKEN | FMC_PCR_PWID | FMC_PCR_ECCEN | FMC_PCR_TCLR | FMC_PCR_TAR | FMC_PCR_ECCPS))                                           // PCR register clear mask
#define FMC_PMEM_CLEAR_MASK                 ((uint32_t)(FMC_PMEM_MEMSET | FMC_PMEM_MEMWAIT | FMC_PMEM_MEMHOLD | FMC_PMEM_MEMHIZ))                                                                               // PMEM register clear mask
#define FMC_PATT_CLEAR_MASK                 ((uint32_t)(FMC_PATT_ATTSET | FMC_PATT_ATTWAIT | FMC_PATT_ATTHOLD | FMC_PATT_ATTHIZ))                                                                               // PATT register clear mask
#define FMC_SDCR_CLEAR_MASK                 ((uint32_t)(FMC_SDCRx_NC | FMC_SDCRx_NR | FMC_SDCRx_MWID | FMC_SDCRx_NB | FMC_SDCRx_CAS | FMC_SDCRx_WP | FMC_SDCRx_SDCLK | FMC_SDCRx_RBURST | FMC_SDCRx_RPIPE))     // SDCR register clear mask
#define FMC_SDTR_CLEAR_MASK                 ((uint32_t)(FMC_SDTRx_TMRD | FMC_SDTRx_TXSR | FMC_SDTRx_TRAS | FMC_SDTRx_TRC | FMC_SDTRx_TWR | FMC_SDTRx_TRP | FMC_SDTRx_TRCD))                                     // SDTR register clear mask
#define FMC_SDTR_TIMING_CLEAR_MASK          ((uint32_t)(FMC_SDTRx_TRC | FMC_SDTRx_TRP))                                                                                                                         // SDTR register clear mask for timing

// FMC SDRAM Command Mode
#define FMC_SDCMR_CMD_NORMAL_MODE           (0x00000000)
#define FMC_SDCMR_CMD_CLK_ENABLE            (0x00000001)
#define FMC_SDCMR_CMD_PALL                  (0x00000002)
#define FMC_SDCMR_CMD_AUTO_REFRESH_MODE     (0x00000003)
#define FMC_SDCMR_CMD_LOAD_MODE             (0x00000004)
#define FMC_SDCMR_CMD_SELF_REFRESH_MODE     (0x00000005)
#define FMC_SDCMR_CMD_POWER_DOWN_MODE       (0x00000006)

#if (CFG_SDRAM_BANK == FMC_SDRAM_BANK1)
  #define FMC_SDCMR_BANK                    FMC_SDCMR_CTB1
#elif (CFG_SDRAM_BANK == FMC_SDRAM_BANK2)
  #define FMC_SDCMR_BANK                    FMC_SDCMR_CTB2
#else
  #define FMC_SDCMR_BANK                    (FMC_SDCMR_CTB1 | FMC_SDCMR_CTB2)
#endif

#define FMC_SDCMR_MODE_REGISTER             (CFG_SDRAM_MRD_WRITE_BURST_MODE | \
                                             CFG_SDRAM_MRD_OPERATION_MODE   | \
                                             CFG_SDRAM_MRD_CAS_LATENCY      | \
                                             CFG_SDRAM_MRD_BURST_TYPE       | \
                                             CFG_SDRAM_MRD_BURST_LENGTH)

// SDTR timing macros (STM32H7 naming)
#define FMC_SDTR_LOAD_TO_ACTIVITY_DELAY     (uint32_t(CFG_SDRAM_LOAD_TO_ACTIVITY_DELAY)   - 1)
#define FMC_SDTR_EXIT_SELF_REFRESH_DELAY    ((uint32_t(CFG_SDRAM_EXIT_SELF_REFRESH_DELAY) - 1) << FMC_SDTRx_TXSR_Pos)
#define FMC_SDTR_SELF_REFRESH_TIME          ((uint32_t(CFG_SDRAM_SELF_REFRESH_TIME)       - 1) << FMC_SDTRx_TRAS_Pos)
#define FMC_SDTR_ROW_CYCLE_DELAY            ((uint32_t(CFG_SDRAM_ROW_CYCLE_DELAY)         - 1) << FMC_SDTRx_TRC_Pos)
#define FMC_SDTR_WRITE_RECOVERY_TIME        ((uint32_t(CFG_SDRAM_WRITE_RECOVERY_TIME)     - 1) << FMC_SDTRx_TWR_Pos)
#define FMC_SDTR_RP_DELAY                   ((uint32_t(CFG_SDRAM_RP_DELAY)                - 1) << FMC_SDTRx_TRP_Pos)
#define FMC_SDTR_RCD_DELAY                  ((uint32_t(CFG_SDRAM_RCD_DELAY)               - 1) << FMC_SDTRx_TRCD_Pos)
#define FMC_SDCMR_AUTO_REFRESH_CYCLE        ((uint32_t(CFG_SDRAM_AUTO_REFRESH_CYCLE)      - 1) << FMC_SDCMR_NRFS_Pos)

#define SDRTR_REFRESH_COUNT                 ((uint32_t)(CFG_SDRAM_REFRESH_COUNT) << FMC_SDRTR_COUNT_Pos)

#define FMC_MRD_CONFIG						(CFG_SDRAM_MRD_BURST_LENGTH   | \
                                             CFG_SDRAM_MRD_BURST_TYPE     | \
                                             CFG_SDRAM_MRD_CAS_LATENCY    | \
											 CFG_SDRAM_MRD_OPERATION_MODE | \
											 (CFG_SDRAM_MRD_WRITE_BURST_MODE <<  FMC_SDCMR_MRD_Pos))

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDRAM_Initialize
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   Performs the SDRAM device initialization sequence (STM32H7 FMC).
//
//-------------------------------------------------------------------------------------------------
void SDRAM_Initialize(void)
{
    // ---- FMC Reset ----
    RCC->AHB3RSTR |=  RCC_AHB3RSTR_FMCRST;
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_FMCRST;
    RCC->AHB3ENR  |=  RCC_AHB3ENR_FMCEN;                    // Enable Clock

  #if (CFG_SDRAM_BANK == FMC_SDRAM_BANK1)

    // Set SDRAM bank configuration parameters
    FMC_Bank5_6_R->SDCR[FMC_SDRAM_BANK1] = (CFG_SDRAM_COLUMN_BITS_NUMBER   |
                                            CFG_SDRAM_ROW_BITS_NUMBER      |
                                            CFG_SDRAM_MEMORY_DATA_WIDTH    |
                                            CFG_SDRAM_INTERNAL_BANK_NUMBER |
                                            CFG_SDRAM_CAS_LATENCY          |
                                            CFG_SDRAM_WRITE_PROTECTION     |
                                            CFG_SDRAM_SD_CLOCK_PERIOD      |
                                            CFG_SDRAM_READ_BURST           |
                                            CFG_SDRAM_PIPE_DELAY);

    // Set SDRAM device timing parameters
    FMC_Bank5_6_R->SDTR[FMC_SDRAM_BANK1] = (FMC_SDTR_LOAD_TO_ACTIVITY_DELAY  |
                                            FMC_SDTR_EXIT_SELF_REFRESH_DELAY |
                                            FMC_SDTR_SELF_REFRESH_TIME       |
                                            FMC_SDTR_ROW_CYCLE_DELAY         |
                                            FMC_SDTR_WRITE_RECOVERY_TIME     |
                                            FMC_SDTR_RP_DELAY                |
                                            FMC_SDTR_RCD_DELAY);

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
    FMC_Bank5_6_R->SDTR[FMC_SDRAM_BANK1] = (FMC_SDTR_ROW_CYCLE_DELAY | FMC_SDTR_RP_DELAY);
    FMC_Bank5_6_R->SDTR[FMC_SDRAM_BANK2] = (FMC_SDTR_LOAD_TO_ACTIVITY_DELAY  |
                                            FMC_SDTR_EXIT_SELF_REFRESH_DELAY |
                                            FMC_SDTR_SELF_REFRESH_TIME       |
                                            FMC_SDTR_WRITE_RECOVERY_TIME     |
                                            FMC_SDTR_RCD_DELAY);

  #endif

    // SDRAM initialization sequence
    FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_CMD_CLK_ENABLE | FMC_SDCMR_BANK);                                     // Clock enable command
    LIB_Delay_uSec(1000);                                                                                   // Delay
    FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_CMD_PALL | FMC_SDCMR_BANK);                                           // PALL command
    FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_CMD_AUTO_REFRESH_MODE | FMC_SDCMR_BANK | SDCMR_AUTO_REFRESH_CYCLE);   // Auto refresh mode
    FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_CMD_LOAD_MODE | FMC_SDCMR_BANK | FMC_MRD_CONFIG);                     // Load mode
    FMC_Bank5_6_R->SDRTR = (CFG_SDRAM_REFRESH_COUNT << 1);                                                  // Set refresh count
//    SET_BIT(FMC_Bank1_R->BTCR[0], FMC_BCR1_FMCEN);                                                        // FMC controller Enable
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SDRAM_DRIVER == DEF_ENABLED)
