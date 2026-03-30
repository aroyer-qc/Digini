//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H7_sdram.h
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

#if (USE_SDRAM_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// FMC SDRAM Bank
#define FMC_SDRAM_BANK1                             (0x00000000)
#define FMC_SDRAM_BANK2                             (0x00000001)

// FMC SDRAM Column Bits number
#define FMC_SDRAM_COLUMN_BITS_NUM_8                 (0x00000000)
#define FMC_SDRAM_COLUMN_BITS_NUM_9                 (0x00000001)
#define FMC_SDRAM_COLUMN_BITS_NUM_10                (0x00000002)
#define FMC_SDRAM_COLUMN_BITS_NUM_11                (0x00000003)

// FMC SDRAM Row Bits number
#define FMC_SDRAM_ROW_BITS_NUM_11                   (0x00000000)
#define FMC_SDRAM_ROW_BITS_NUM_12                   (0x00000004)
#define FMC_SDRAM_ROW_BITS_NUM_13                   (0x00000008)

// FMC SDRAM Memory Bus Width
#define FMC_SDRAM_MEM_BUS_WIDTH_8                   (0x00000000)
#define FMC_SDRAM_MEM_BUS_WIDTH_16                  (0x00000010)
#define FMC_SDRAM_MEM_BUS_WIDTH_32                  (0x00000020)

// FMC SDRAM Internal Banks Number
#define FMC_SDRAM_INTERN_BANKS_NUM_2                (0x00000000)
#define FMC_SDRAM_INTERN_BANKS_NUM_4                (0x00000040)

// FMC SDRAM CAS Latency
#define FMC_SDRAM_CAS_LATENCY_1                     (0x00000080)
#define FMC_SDRAM_CAS_LATENCY_2                     (0x00000100)
#define FMC_SDRAM_CAS_LATENCY_3                     (0x00000180)

// FMC SDRAM Write Protection
#define FMC_SDRAM_WRITE_PROTECTION_DISABLE          (0x00000000)
#define FMC_SDRAM_WRITE_PROTECTION_ENABLE           (0x00000200)

// FMC SDRAM Clock Period
#define FMC_SDRAM_CLOCK_DISABLE                     (0x00000000)
#define FMC_SDRAM_CLOCK_PERIOD_2                    (0x00000800)
#define FMC_SDRAM_CLOCK_PERIOD_3                    (0x00000C00)

// FMC SDRAM Read Burst
#define FMC_SDRAM_RBURST_DISABLE                    (0x00000000)
#define FMC_SDRAM_RBURST_ENABLE                     (0x00001000)

// FMC SDRAM Read Pipe Delay
#define FMC_SDRAM_RPIPE_DELAY_0                     (0x00000000)
#define FMC_SDRAM_RPIPE_DELAY_1                     (0x00002000)
#define FMC_SDRAM_RPIPE_DELAY_2                     (0x00004000)

// FMC MRD bit defintions
#define FMC_SDRAM_MRD_WRITE_BURST_MODE_CONTINUOUS   (0 << (9 + FMC_SDCMR_MRD_Pos))  // MRD[9]	    Write Burst Mode	0 (Continuous), 1 (Single)
#define FMC_SDRAM_MRD_WRITE_BURST_MODE_SINGLE       (1 << (9 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_OPERATION_MODE_STANDARD       (0 << (6 + FMC_SDCMR_MRD_Pos))  // MRD[8:7]	    Operating Mode	    00 (Standard), 01 (All Other State)
#define FMC_SDRAM_MRD_OPERATION_MODE_TEST           (1 << (6 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_CAS_LATENCY_0                 (0 << (4 + FMC_SDCMR_MRD_Pos))  // MRD[6:4]	    CAS Latency	        000, 001, 010, 011
#define FMC_SDRAM_MRD_CAS_LATENCY_1                 (1 << (4 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_CAS_LATENCY_2                 (2 << (4 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_CAS_LATENCY_3                 (3 << (4 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_BURST_TYPE_SEQUENTIAL         (0 << (3 + FMC_SDCMR_MRD_Pos))  // MRD[3]	    Burst Type	        0 (Sequential), 1 (Interleaved)
#define FMC_SDRAM_MRD_BURST_TYPE_INTERLEAVED        (1 << (3 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_BURST_LENGTH_1                (0 << (0 + FMC_SDCMR_MRD_Pos))  // MRD[2:0]	    Burst Length	    00 (1), 01 (2), 10 (4), 11 (8)
#define FMC_SDRAM_MRD_BURST_LENGTH_2                (1 << (0 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_BURST_LENGTH_4                (2 << (0 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_BURST_LENGTH_8                (3 << (0 + FMC_SDCMR_MRD_Pos))

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void    SDRAM_Initialize              (void);

//-------------------------------------------------------------------------------------------------

#endif // USE_SDRAM_DRIVER

