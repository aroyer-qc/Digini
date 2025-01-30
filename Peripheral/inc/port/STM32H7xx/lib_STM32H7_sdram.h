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
#include "./Digini/inc/lib_typedef.h"
#include "./Peripheral/inc/port/lib_class_fmc.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//--------------------
// SDRAM Configuration

// FMC_SDRAM_Bank FMC SDRAM Bank
#define FMC_SDRAM_BANK1                             (0x00000000)
#define FMC_SDRAM_BANK2                             (0x00000001)

// FMC_SDRAM_Column_Bits_number FMC SDRAM Column Bits number
#define FMC_SDRAM_COLUMN_BITS_NUM_8                 (0x00000000)
#define FMC_SDRAM_COLUMN_BITS_NUM_9                 (0x00000001)
#define FMC_SDRAM_COLUMN_BITS_NUM_10                (0x00000002)
#define FMC_SDRAM_COLUMN_BITS_NUM_11                (0x00000003)

// FMC_SDRAM_Row_Bits_number FMC SDRAM Row Bits number
#define FMC_SDRAM_ROW_BITS_NUM_11                   (0x00000000)
#define FMC_SDRAM_ROW_BITS_NUM_12                   (0x00000004)
#define FMC_SDRAM_ROW_BITS_NUM_13                   (0x00000008)

// FMC_SDRAM_Memory_Bus_Width FMC SDRAM Memory Bus Width
#define FMC_SDRAM_MEM_BUS_WIDTH_8                   (0x00000000)
#define FMC_SDRAM_MEM_BUS_WIDTH_16                  (0x00000010)
#define FMC_SDRAM_MEM_BUS_WIDTH_32                  (0x00000020)

// FMC_SDRAM_Internal_Banks_Number FMC SDRAM Internal Banks Number
#define FMC_SDRAM_INTERN_BANKS_NUM_2                (0x00000000)
#define FMC_SDRAM_INTERN_BANKS_NUM_4                (0x00000040)

// FMC_SDRAM_CAS_Latency FMC SDRAM CAS Latency
#define FMC_SDRAM_CAS_LATENCY_1                     (0x00000080)
#define FMC_SDRAM_CAS_LATENCY_2                     (0x00000100)
#define FMC_SDRAM_CAS_LATENCY_3                     (0x00000180)

// FMC_SDRAM_Write_Protection FMC SDRAM Write Protection
#define FMC_SDRAM_WRITE_PROTECTION_DISABLE          (0x00000000)
#define FMC_SDRAM_WRITE_PROTECTION_ENABLE           (0x00000200)

// FMC_SDRAM_Clock_Period FMC SDRAM Clock Period
#define FMC_SDRAM_CLOCK_DISABLE                     (0x00000000)
#define FMC_SDRAM_CLOCK_PERIOD_2                    (0x00000800)
#define FMC_SDRAM_CLOCK_PERIOD_3                    (0x00000C00)

// FMC_SDRAM_Read_Burst FMC SDRAM Read Burst
#define FMC_SDRAM_RBURST_DISABLE                    (0x00000000)
#define FMC_SDRAM_RBURST_ENABLE                     (0x00001000)

// FMC_SDRAM_Read_Pipe_Delay FMC SDRAM Read Pipe Delay
#define FMC_SDRAM_RPIPE_DELAY_0                     (0x00000000)
#define FMC_SDRAM_RPIPE_DELAY_1                     (0x00002000)
#define FMC_SDRAM_RPIPE_DELAY_2                     (0x00004000)

//--------------
// SDRAM Command

// FMC_SDRAM_Command_Mode FMC SDRAM Command Mode
#define FMC_SDRAM_CMD_NORMAL_MODE                   (0x00000000)
#define FMC_SDRAM_CMD_CLK_ENABLE                    (0x00000001)
#define FMC_SDRAM_CMD_PALL                          (0x00000002)
#define FMC_SDRAM_CMD_AUTO_REFRESH_MODE             (0x00000003)
#define FMC_SDRAM_CMD_LOAD_MODE                     (0x00000004)
#define FMC_SDRAM_CMD_SELF_REFRESH_MODE             (0x00000005)
#define FMC_SDRAM_CMD_POWER_DOWN_MODE               (0x00000006)

// FMC_SDRAM_Command_Target FMC SDRAM Command Target
#define FMC_SDRAM_CMD_TARGET_BANK2                  FMC_SDCMR_CTB2
#define FMC_SDRAM_CMD_TARGET_BANK1                  FMC_SDCMR_CTB1
#define FMC_SDRAM_CMD_TARGET_BANK1_2                (0x00000018)

// FMC_SDRAM_Mode_Status FMC SDRAM Mode Status
#define FMC_SDRAM_NORMAL_MODE                       (0x00000000)
#define FMC_SDRAM_SELF_REFRESH_MODE                 FMC_SDSR_MODES1_0
#define FMC_SDRAM_POWER_DOWN_MODE                   FMC_SDSR_MODES1_1

// FMC MRD bit defintions
#define FMC_SDRAM_MRD_WRITE_BURST_MODE_CONTINUOUS   (0 << (9 + FMC_SDCMR_MRD_Pos))  // MRD[9]	    Write Burst Mode	0 (Continuous), 1 (Single)
#define FMC_SDRAM_MRD_WRITE_BURST_MODE_SINGLE       (1 << (9 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_OPERATION_MODE_STANDARD       (0 << (6 + FMC_SDCMR_MRD_Pos))  // MRD[8:7]	    Operating Mode	    00 (Standard), 01 (All Other State)
#define FMC_SDRAM_MRD_OPERATION_MODE_TEST           (1 << (6 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_CAS_LATENCY_0                 (0 << (4 + FMC_SDCMR_MRD_Pos))  // MRD[6:4]	    CAS Latency	    000, 001, 010, 011
#define FMC_SDRAM_MRD_CAS_LATENCY_1                 (1 << (4 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_CAS_LATENCY_2                 (2 << (4 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_CAS_LATENCY_3                 (3 << (4 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_BURST_TYPE_SEQUENTIAL         (0 << (3 + FMC_SDCMR_MRD_Pos))  // MRD[3]	    Burst Type	    0 (Sequential), 1 (Interleaved)
#define FMC_SDRAM_MRD_BURST_TYPE_INTERLEAVED        (1 << (3 + FMC_SDCMR_MRD_Pos))

#define FMC_SDRAM_MRD_BURST_LENGTH_1                (0 << (0 + FMC_SDCMR_MRD_Pos))  // MRD[2:0]	    Burst Length	00 (1), 01 (2), 10 (4), 11 (8)
#define FMC_SDRAM_MRD_BURST_LENGTH_2                (1 << (0 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_BURST_LENGTH_4                (2 << (0 + FMC_SDCMR_MRD_Pos))
#define FMC_SDRAM_MRD_BURST_LENGTH_8                (3 << (0 + FMC_SDCMR_MRD_Pos))


/*
0x220 should be:

    0010 0010 0000    
    
    
    00 1 00 010 0 000       0 0000 0000

    FMC_SDRAM_MRD_WRITE_BURST_MODE_SINGLE
    FMC_SDRAM_MRD_OPERATION_MODE_STANDARD
    FMC_SDRAM_MRD_CAS_LATENCY_2
    FMC_SDRAM_MRD_BURST_TYPE_SEQUENTIAL
    FMC_SDRAM_MRD_BURST_LENGTH_1

    00 00 10 00 10 0000  0 0000 0000

*/

//-------------------------------------------------------------------------------------------------
// typedef struct(s) and enum(s)
//-------------------------------------------------------------------------------------------------

// FMC SDRAM Configuration Structure definition
struct FMC_SDRAM_Timing_t
{
    uint32_t SDBank;                    // Specifies SDRAM memory device that will be used.                                                                             This parameter, value of FMC_SDRAM_Bank.
    uint32_t ColumnBitsNumber;          // Defines number of bits of column address.                                                                                    This parameter, value of FMC_SDRAM_Column_Bits_number.
    uint32_t RowBitsNumber;             // Defines number of bits of column address.                                                                                    This parameter, value of FMC_SDRAM_Row_Bits_number.
    uint32_t MemoryDataWidth;           // Defines memory device width.                                                                                                 This parameter, value of FMC_SDRAM_Memory_Bus_Width.
    uint32_t InternalBankNumber;        // Defines number of the device's internal banks.                                                                               This parameter, value of FMC_SDRAM_Internal_Banks_Number.
    uint32_t CASLatency;                // Defines SDRAM CAS latency in number of memory clock cycles.                                                                  This parameter, value of FMC_SDRAM_CAS_Latency.
    uint32_t WriteProtection;           // Enables SDRAM device to be accessed in write mode.                                                                           This parameter, value of FMC_SDRAM_Write_Protection.
    uint32_t SDClockPeriod;             // Define  SDRAM Clock Period for both SDRAM devices and they allow to disable the clock before changing frequency.             This parameter, value of FMC_SDRAM_Clock_Period.
    uint32_t ReadBurst;                 // This bit enable the SDRAM ctrl to anticipate the next read commands during the CAS latency and stores data in the Read FIFO. This parameter, value of FMC_SDRAM_Read_Burst.
    uint32_t ReadPipeDelay;             // Define delay in system clock cycles on read data path.                                                                       This parameter, value of FMC_SDRAM_Read_Pipe_Delay.
}; 

// FMC SDRAM Timing parameters structure definition
struct FMC_SDRAM_Timing_t
{
    uint32_t LoadToActiveDelay;         // Defines delay between a Load Mode Register command and an active or Refresh command in number of memory clock cycles.        This parameter, value between Min_Data = 1 and Max_Data = 16
    uint32_t ExitSelfRefreshDelay;      // Defines delay from releasing the self refresh command to issuing the Activate command in number of memory clock cycles.      This parameter, value between Min_Data = 1 and Max_Data = 16
    uint32_t SelfRefreshTime;           // Defines minimum Self Refresh period in number of memory clock cycles.                                                        This parameter, value between Min_Data = 1 and Max_Data = 16
    uint32_t RowCycleDelay;             // Defines delay between the Refresh command and the Activate command and the delay between two consecutive Refresh.
                                        // commands in number of memory clock cycles.                                                                                   This parameter, value between Min_Data = 1 and Max_Data = 16
    uint32_t WriteRecoveryTime;         // Defines Write recovery Time in number of memory clock cycles.                                                                This parameter, value between Min_Data = 1 and Max_Data = 16
    uint32_t RPDelay;                   // Defines delay between a Precharge Command and an other command in number of memory clock cycles.                             This parameter, value between Min_Data = 1 and Max_Data = 16
    uint32_t RCDDelay;                  // Defines delay between the Activate Command and a Read/Write command in number of memory clock cycles.                        This parameter, value between Min_Data = 1 and Max_Data = 16
};

// SDRAM command parameters structure definition
struct FMC_SDRAM_Command_t
{
    uint32_t CommandMode;               // Defines command issued to the SDRAM device.                                                                                  This parameter, Value of FMC_SDRAM_Command_Mode.
    uint32_t CommandTarget;             // Defines which device (1 or 2) the command will be issued to.                                                                 This parameter, value of FMC_SDRAM_Command_Target.
    uint32_t AutoRefreshNumber;         // Defines number of consecutive auto refresh command issued in auto refresh mode.                                              This parameter, value between Min_Data = 1 and Max_Data = 15
    uint32_t ModeRegisterDefinition;    // Defines SDRAM Mode register content.
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SDRAM_Driver
{
    public:

        void            Initialize              (void);

//        SystemState_e   WriteProtectionEnable   (void);
        //SystemState_e   WriteProtectionDisable  (void);

        SystemState_e   ProgramRefreshRate      (uint32_t RefreshRate);
        SystemState_e   SetAutoRefreshNumber    (uint32_t AutoRefreshNumber);
        uint32_t        GetModeStatus           (void);
        SystemState_e   GetState                (void);

        void            IRQHandler              (void);

    private:

        SystemState_e   SendCommand             (FMC_SDRAM_Command_t* Command, uint32_t Timeout);

        SystemState_e   m_State;
};

void SDRAM_RefreshErrorCallback     (void);
void SDRAM_DMA_XferCpltCallback     (void);
void SDRAM_DMA_XferErrorCallback    (void);

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_SDRAM_MODULE

