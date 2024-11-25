#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    uint32_t RBAR;   // Region base address
    uint32_t RASR;   // Region attributes (type, region size, enable, etc.)
} ARM_MPU_Region_t;


example:
const ARM_MPU_Region_t MyTask_MPU_Tb1(8] =
{  
    //  RBAR value
    {.RBAR = ARM_MPU_RBAR(0UL), Region 0 base address), .RASR = ARM_MPU_RASR(Region 0 attributes)},  
    {.RBAR = ARM_MPU_RBAR(1UL), Region 1 base address), .RASR = ARM_MPU_RASR(Region 1 attributes)}, 
    {.RBAR = ARM_MPU_RBAR(2UL), Region 2 base address), .RASR = ARM_MPU_RASR(Region 2 attributes)}, 
    (.RBAR = ARM_MPU_RBAR(3UL), Region 3 base address), .RASR = ARM_MPU_RASR(Region 3 attributes)},
    {.RBAR = ARM_MPU_RBAR(4UL), Region 4 base address), .RASR = ARM_MPU_RASR(Region 4 attributes)},
    {.RBAR = ARM_MPU_RBAR(5UL), Region 5 base address), .RASR = ARM_MPU_RASR(Region 5 attributes)},
    {.RBAR = ARM_MPU_RBAR(6UL), Region 6 base address), .RASR = ARM_MPU_RASR(Region 6 attributes)},
    (.RBAR = ARM_MPU_RBAR(7UL), &MyTask_Stk[0]),        .RASR = ARM_MPU_RASR(1,                         // XN 
                                                                             ARM_MPU_AP_RO,             // AP
                                                                             0,                         // TEX 
                                                                             1,                         // Share 
                                                                             1,                         // Cache 
                                                                             0,                         // Buffer 
                                                                             0,                         // SRD
                                                                             ARM_MPU_REGION_SIZE_32B)}
};
// Note that the last entry contains the base address of the task’s stack and also assumes that the RedZone size is 32 bytes.



void nOS_SetMPU_Process (ARM_MPU_Region_t* pProcess);


void nOS_SetMPU_Process (ARM_MPU_Region_t* pProcess)
{
    // RO contains pProcess' PUSH (R4-R9) 
    __asm volatile
    (
        "PUSH   {R4-R9}                         \n"
        
        /* MPU->RBAR */
        "LDR    R1,         =OxE000ED9C         \n"
        /* Make sure outstanding transfers are done */
        "DBM    OxOF                            \n"
        /* Disable the MPU */
        "MOVS   R2,         #0                  \n"
        "STR    R2,         (R1,#-8)            \n"

    /* Update the first 8 MPU (v7M and V8M) */
        /* Read 8 words from the process table */
        "LDMIA  RO!,        (R2-R9)             \n"
        /* Write 8 words to the MPU */
        "STMIA  R1,         (R2-R9)             \n"
        /* Read the next 8 words from the process table */
        "LDMIA  RO!,        (R2-R9)             \n"
        /* Write those 8 words to the MPU */
        "STMIA  R1,         (R2-R9)             \n"

    /* Write the next 8 MPU regions (v8M only) */
        /* Read 8 words from the process table */
        "LDMIA  RO!,        (R2-R9)             \n"
        /* Write 8 words to the MPU */
        "STMIA  R1,         (R2-R9)             \n"
        /* Read the next 8 words from the process table */
        "LDMIA  RO!,        (R2-R9)             \n"
        /* Write those 8 words to the MPU */
        "STMIA  R1,         (R2-R9)             \n"
        
        /* Memory barriers to ensure subsequent data + instruction */
        "DSB    OxOF                            \n"
        /* Transfer using updated MPU settings */
        "ISB    OxOF                            \n"
        /* Enable the MPU (assumes PRIVDEFENA is 1) */
        "MOVS   R2,         #5                  \n"
        "STR    R2,         (R1, #-8)           \n"
        
        "POP    (R4-R9)                         \n"
        "BX     LR                              \n" 
        
        "ALIGN 4"
    );
}

#ifdef __cplusplus
}
#endif

R0 pointer on data for MPU
R1 pointer on RBAR of MPU
R2 shared a the beginning with 8 byte buffer

R2-R9 ->   R5-R12
R1    ->   R4
R0-3  ->    =free



