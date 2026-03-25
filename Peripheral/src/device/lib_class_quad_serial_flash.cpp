//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_serial_flash.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

#define LIB_SERIAL_FLASH_GLOBAL
#include "./lib_digini.h"
#undef  LIB_SERIAL_FLASH_GLOBAL


//-------------------------------------------------------------------------------------------------

//#if (USE_QSPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void* pArg          Pointer on the type of driver use by this class
//
//  Return:         SystemState_e
//
//  Description:    Initialize the serial flash memory
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SERIAL_FLASH_Driver::Initialize(void* pArg)
{
    SystemState_e State;

    m_pQuadSPI = (QuadSPI*)pArg;

    // Initialize QSPI
    m_pQuadSPI->Initialize();

    // Configure the QSPI in memory-mapped mode
    QSPI_CommandTypeDef         Command;
    QSPI_MemoryMappedTypeDef    Config;

    // Configure the command for the read instruction
    Command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    Command.Instruction       = QUAD_INOUT_FAST_READ_CMD;
    Command.AddressMode       = QSPI_ADDRESS_4_LINES;
    Command.AddressSize       = QSPI_ADDRESS_24_BITS;
    Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    Command.DataMode          = QSPI_DATA_4_LINES;
    Command.DummyCycles       = N25Q128A_DUMMY_CYCLES_READ_QUAD;
    Command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    Command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    Command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    Config.TimeOutActivation  = QSPI_TIMEOUT_COUNTER_DISABLE;
    Config.TimeOutPeriod      = 1;

    State = m_pQuadSPI->MemoryMapped(&Command, &Config);

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Reset
//
//  Parameter(s):   None
//
//  Return:         SystemState_e
//
//  Description:    Software Reset the Serial Flash.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SERIAL_FLASH_Driver::Reset(void)
{
    QSPI_Command_t Command;

    /* Initialize the reset enable command */
    Command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    Command.Instruction       = RESET_ENABLE_CMD;
    Command.AddressMode       = QSPI_ADDRESS_NONE;
    Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    Command.DataMode          = QSPI_DATA_NONE;
    Command.DummyCycles       = 0;
    Command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    Command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode        = QSPI_SIOO_INST_EVERY_CMD;

    // Send the command
    if(QSPI_Command(&s_command, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
    {
        return SYS_ERROR;
    }

    // Send the reset memory command
    s_command.Instruction = RESET_MEMORY_CMD;
    if(QSPI_Command(&Command, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
    {
        return SYS_ERROR;
    }

    // Configure automatic polling mode to wait the memory is ready
    if(QSPI_AutoPollingMemReady(QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
    {
        return SYS_ERROR;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Read
//
//  Parameter(s):   pData           Pointer to data to be read
//                  ReadAddress     Read start address
//                  Size            Size of data to read
//
//  Return:         SystemState_e
//
//  Description:    Reads an amount of data from the QSPI memory.
//
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SERIAL_FLASH_Driver::Read(uint8_t* pData, uint32_t ReadAddress, uint32_t Size)
{
    QSPI_Command_t Command;

    // Initialize the read command
    Command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    Command.Instruction       = QUAD_INOUT_FAST_READ_CMD;
    Command.AddressMode       = QSPI_ADDRESS_4_LINES;
    Command.AddressSize       = QSPI_ADDRESS_24_BITS;
    Command.Address           = ReadAddress;
    Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    Command.DataMode          = QSPI_DATA_4_LINES;
    Command.DummyCycles       = N25Q128A_DUMMY_CYCLES_READ_QUAD;
    Command.NbData            = Size;
    Command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    Command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    Command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    // Configure the command
    if(m_pQuadSPI->Command(&Command, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
    {
        return SYS_ERROR;
    }

    // Reception of the data
    if(m_pQuadSPI->Receive(pData, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
    {
        return SYS_ERROR;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Write
//
//  Parameter(s):   pData           Pointer to data to be written
//                  WriteAddress    Write start address
//                  Size            Size of data to write
//
//  Return:         SystemState_e
//
//  Description:    Writes an amount of data to the QSPI memory.
//
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SERIAL_FLASH_Driver::Write(uint8_t* pData, uint32_t WriteAddress, uint32_t Size)
{
    QSPI_Command_t Command;
    uint32_t       EndAddress;
    uint32_t       CurrentSize;
    uint32_t       CurrentAddress;

    // Calculation of the size between the write address and the end of the page
    CurrentAddress = 0;

    while(CurrentAddress <= WriteAddress)
    {
        CurrentAddress += N25Q128A_PAGE_SIZE;
    }

    CurrentSize = CurrentAddress - WriteAddress;

    // Check if the size of the data is less than the remaining place in the page
    if(CurrentSize > Size)
    {
        CurrentSize = Size;
    }

    // Initialize the address variables
    CurrentAddress = WriteAddress;
    EndAddress     = WriteAddress + Size;

    // Initialize the program command
    Command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    Command.Instruction       = EXT_QUAD_IN_FAST_PROG_CMD;
    Command.AddressMode       = QSPI_ADDRESS_4_LINES;
    Command.AddressSize       = QSPI_ADDRESS_24_BITS;
    Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    Command.DataMode          = QSPI_DATA_4_LINES;
    Command.DummyCycles       = 0;
    Command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    Command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    Command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    // Perform the write page by page
    do
    {
        Command.Address = CurrentAddress;
        Command.NbData  = CurrentSize;

        // Enable write operations
        if (QSPI_WriteEnable() != SYS_READY)
        {
            return SYS_ERROR;
        }

        // Configure the command
        if (m_pQuadSPI->Command(&Command, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
        {
            return SYS_ERROR;
        }

        // Transmission of the data
        if (m_pQuadSPI->Transmit(pData, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
        {
            return SYS_ERROR;
        }

        // Configure automatic polling mode to wait for end of program
        if(QSPI_AutoPollingMemReady(&QSPIHandle, QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
        {
            return SYS_ERROR;
        }

        // Update the address and size variables for next page programming
        CurrentAddress += CurrentSize;
        pData += CurrentSize;
        CurrentSize = ((CurrentAddress + N25Q128A_PAGE_SIZE) > EndAddress) ? (EndAddress - CurrentAddress) : N25Q128A_PAGE_SIZE;
    }
    while (CurrentAddress < EndAddress);

    return SYS_READY;
}


#if 0

/**
  * @brief  Erases the specified block of the QSPI memory.
  * @param  BlockAddress: Block address to erase
  * @retval QSPI memory status
  */
uint8_t SERIAL_FLASH_Driver::EraseBlock(uint32_t BlockAddress)
{
  QSPI_Command_t Command;

  /* Initialize the erase command */
  Command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  Command.Instruction       = SUBSECTOR_ERASE_CMD;
  Command.AddressMode       = QSPI_ADDRESS_1_LINE;
  Command.AddressSize       = QSPI_ADDRESS_24_BITS;
  Command.Address           = BlockAddress;
  Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  Command.DataMode          = QSPI_DATA_NONE;
  Command.DummyCycles       = 0;
  Command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  Command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  Command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(&QSPIHandle) != SYS_READY)
  {
    return SYS_ERROR;
  }

  /* Send the command */
  if (QSPI_Command(&sCommand, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
  {
    return SYS_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&QSPIHandle, N25Q128A_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return SYS_ERROR;
  }

  return SYS_READY;
}

/**
  * @brief  Erases the entire QSPI memory.
  * @retval QSPI memory status
  */
uint8_t SERIAL_FLASH_Driver::EraseChip(void)
{
  QSPI_Command_t Command;

  /* Initialize the erase command */
  Command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  Command.Instruction       = BULK_ERASE_CMD;
  Command.AddressMode       = QSPI_ADDRESS_NONE;
  Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  Command.DataMode          = QSPI_DATA_NONE;
  Command.DummyCycles       = 0;
  Command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  Command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  Command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(&QSPIHandle) != SYS_READY)
  {
    return SYS_ERROR;
  }

  /* Send the command */
  if (QSPI_Command(&QSPIHandle, &Command, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
  {
    return SYS_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&QSPIHandle, N25Q128A_BULK_ERASE_MAX_TIME) != SYS_READY)
  {
    return SYS_ERROR;
  }

  return SYS_READY;
}

/**
  * @brief  Reads current status of the QSPI memory.
  * @retval QSPI memory status
  */
uint8_tSERIAL_FLASH_Driver::GetStatus(void)
{
  QSPI_Command_t Command;
  uint8_t reg;

  /* Initialize the read flag status register command */
  Command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  Command.Instruction       = READ_FLAG_STATUS_REG_CMD;
  Command.AddressMode       = QSPI_ADDRESS_NONE;
  Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  Command.DataMode          = QSPI_DATA_1_LINE;
  Command.DummyCycles       = 0;
  Command.NbData            = 1;
  Command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  Command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  Command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (QSPI_Command(&Command, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
  {
    return SYS_ERROR;
  }

  /* Reception of the data */
  if (QSPI_Receive(&reg, QPSI_TIMEOUT_DEFAULT_VALUE) != SYS_READY)
  {
    return SYS_ERROR;
  }

  /* Check the value of the register */
  if ((reg & (N25Q128A_FSR_PRERR | N25Q128A_FSR_VPPERR | N25Q128A_FSR_PGERR | N25Q128A_FSR_ERERR)) != 0)
  {
    return SYS_ERROR;
  }
  else if ((reg & (N25Q128A_FSR_PGSUS | N25Q128A_FSR_ERSUS)) != 0)
  {
    return SYS_SUSPENDED;
  }
  else if ((reg & N25Q128A_FSR_READY) != 0)
  {
    return SYS_READY;
  }
  else
  {
    return SYS_BUSY;
  }
}

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo: pointer on the configuration structure
  * @retval QSPI memory status
*/

  /*##-2- Configure peripheral GPIO ##########################################*/
/**
  * @brief  This function reset the QSPI memory.
  * @param  hqspi: QSPI handle
  * @retval None
  */
/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t SERIAL_FLASH_Driver::DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef s_command;
  uint8_t reg;

  /* Initialize the read volatile configuration register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_VOL_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (QSPI_Command(hqspi, &s_command, QPSI_TIMEOUT_DEFAULT_VALUE) != OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (QSPI_Receive(hqspi, &reg, QPSI_TIMEOUT_DEFAULT_VALUE) != OK)
  {
    return QSPI_ERROR;
  }

  /* Enable write operations */
  if (QSPI_WriteEnable(hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Update volatile configuration register (with new dummy cycles) */
  s_command.Instruction = WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, N25Q128A_VCR_NB_DUMMY, (N25Q128A_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(N25Q128A_VCR_NB_DUMMY)));

  /* Configure the write volatile configuration register command */
  if (QSPI_Command(hqspi, &s_command, QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t SERIAL_FLASH_Driver::WriteEnable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = WRITE_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = N25Q128A_SR_WREN;
  s_config.Mask            = N25Q128A_SR_WREN;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction    = READ_STATUS_REG_CMD;
  s_command.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t SERIAL_FLASH_Driver::AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Configure automatic polling mode to wait for memory ready */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match           = 0;
  s_config.Mask            = N25Q128A_SR_WIP;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, Timeout) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

#endif

//-------------------------------------------------------------------------------------------------

//#endif // (USE_QSPI_DRIVER == DEF_ENABLED)

























//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private Define(s)
//-------------------------------------------------------------------------------------------------

//#define DEBUG_TEST_QSPI

#define EXPAND_X_CMD_AS_STRUCT_DATA(CMD_ID,  CMD, ADDR_MODE, DATA_MODE, DUMMY ) \
                                           { CMD, ADDR_MODE, DATA_MODE, DUMMY },

#define MEMORY_READY_MATCH_VALUE    0x00
#define MEMORY_READY_MASK_VALUE     0x01
#define AUTO_POLLING_INTERVAL       0x10

#define XSPI_TIMEOUT_DEFAULT_VALUE  32000

#define QSPI_SR_QUAD_ENABLE_BIT     0x02   

#define MEMORY_DRIVE_MASK           0x60
#define MEMORY_DRIVE_AT_25_PERCENT  0x60
#define MEMORY_DRIVE_AT_50_PERCENT  0x40
#define MEMORY_DRIVE_AT_75_PERCENT  0x20
#define MEMORY_DRIVE_AT_100_PERCENT 0x00

#define XSPI_FUNCTIONAL_MODE_INDIRECT_WRITE ((uint32_t)0x00000000)         // Indirect write mode
#define XSPI_FUNCTIONAL_MODE_INDIRECT_READ  ((uint32_t)XSPI_CR_FMODE_0)    // Indirect read mode
#define XSPI_FUNCTIONAL_MODE_AUTO_POLLING   ((uint32_t)XSPI_CR_FMODE_1)    // Automatic polling mode
#define XSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  ((uint32_t)XSPI_CR_FMODE)      // Memory-mapped mode
#define XSPI_INSTRUCTION_1_LINE             ((uint32_t)XSPI_CCR_IMODE_0)   // Instruction on a single line
#define XSPI_ADDRESS_24_BITS                ((uint32_t)XSPI_CCR_ADSIZE_1)


//-------------------------------------------------------------------------------------------------
//
//   Class: XSPI_Driver
//
//   Description:   Class to handle XSPI W25Q32 memory
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private Constants and Macros
//-------------------------------------------------------------------------------------------------
const W25Q_Command_t W25Q_Driver::m_Cmd[XSPI_NB_OF_CMD] =
{
    XSPI_CMD_DEF(EXPAND_X_CMD_AS_STRUCT_DATA)
};

const DMA_Info_t W25Q_Driver::m_DMA_RX_Info =
{
    BSP_DMA_MODE_NORMAL,
    GPDMA1_REQUEST_OCTOSPI1,
    BSP_DMA_BREQ_SINGLE_BURST,
    BSP_DMA_PERIPHERAL_TO_MEMORY,
    (BSP_DMA_SOURCE_NO_INCREMENT | BSP_DMA_DESTINATION_INCREMENT),
    (BSP_DMA_SOURCE_SIZE_8_BITS  | BSP_DMA_DESTINATION_SIZE_8_BITS),
    BSP_DMA_PRIORITY_LOW_PRIO_WEIGHT_LOW,
    1,
    1,
    BSP_DMA_TC_EVENT_BLOCK_TRANSFER,
    GPDMA1_Channel6,
    DMA_CFCR_TCF,                                   // Transfer complete flag
    GPDMA1_Channel6_IRQn,
    configMAX_SYSCALL_INTERRUPT_PRIORITY,           // PreempPrio
    DMA_LINKED_LIST_DISABLE,
};

const DMA_Info_t W25Q_Driver::m_DMA_TX_Info =
{
    BSP_DMA_MODE_NORMAL,
    GPDMA1_REQUEST_OCTOSPI1,
    BSP_DMA_BREQ_SINGLE_BURST,
    BSP_DMA_MEMORY_TO_PERIPHERAL,
    (BSP_DMA_SOURCE_INCREMENT   | BSP_DMA_DESTINATION_NO_INCREMENT),
    (BSP_DMA_SOURCE_SIZE_8_BITS | BSP_DMA_DESTINATION_SIZE_8_BITS),
    BSP_DMA_PRIORITY_LOW_PRIO_WEIGHT_LOW,
    1,
    1,
    BSP_DMA_TC_EVENT_BLOCK_TRANSFER,
    GPDMA1_Channel7,
    DMA_CFCR_TCF,                                   // Transfer complete flag
    GPDMA1_Channel7_IRQn,
    configMAX_SYSCALL_INTERRUPT_PRIORITY,           // PreempPrio
    DMA_LINKED_LIST_DISABLE,
};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   W25Q_Driver
//
//   Parameter(s):  None
//
//   Description:   Initializes the SPI peripheral according to the specified parameters
//
//-------------------------------------------------------------------------------------------------
W25Q_Driver::W25Q_Driver() : TFlash()
{
  #ifdef USE_FREERTOS
    m_IsItInitialize  = false;
  #else
    m_IsItDMA_Busy    = false;
  #endif  
    m_Status          = SYS_RESET;
    m_ErrorCode       = SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Initialize the SPI module
//
//-------------------------------------------------------------------------------------------------
#ifdef DEBUG_TEST_QSPI
uint8_t buffer[256];
uint8_t Buffer1[5000];
uint8_t Buffer2[5000];
uint8_t Buffer3[256];
uint8_t StatusFlag;
#include <stdio.h>
uint32_t    EraseCounter = 0;
uint32_t    ReadCounter = 0;
uint32_t    WriteErrorCounter = 0;
uint32_t    ReadErrorCounter1 = 0;
uint32_t    ReadErrorCounter2 = 0;
uint32_t    ReadErrorCounter3 = 0;
#endif

SystemState_e W25Q_Driver::Initialize(void)
{
    SystemState_e      State = SYS_READY;
    uint32_t           FlashID;
    uint32_t           Density;

    m_pXSPI = OCTOSPI1;

    XSPI_MSP_Init();                                            // Initialization of the low level hardware and OCTOSPI

    if(m_Status == SYS_READY)
    {
        m_DMA_TX.Initialize((DMA_Info_t*)&m_DMA_TX_Info);
        m_DMA_TX.SetDestination((void*)&m_pXSPI->DR);           // Configure transmit data register
        m_DMA_RX.Initialize((DMA_Info_t*)&m_DMA_RX_Info);
        m_DMA_RX.SetSource((void*)&m_pXSPI->DR);                // Configure receive data register

      #ifdef USE_FREERTOS
        if(m_IsItInitialize == false)
        {
            m_IsItInitialize = true;
            m_Mutex   = xSemaphoreCreateMutex();                // Create a mutex type semaphore.
            m_DMA_Busy = xSemaphoreCreateBinary();              // Create a binary type semaphore.
        }
      #endif

        if((State = ReadID(&FlashID)) == SYS_READY)
        {
            if((State = ReadSFDP_Density(&Density)) == SYS_READY)
            {
                State = QuadEnable();
            }
        }
    }

    if(State == SYS_READY)
    {
        State = SetDriverStrength(MEMORY_DRIVE_AT_50_PERCENT);
    }

    if(State == SYS_READY)
    {
        m_Status = SYS_READY;

        if(Density > 0)
        {
            m_FlashInfo.PageSize        = 256;
            m_FlashInfo.NbPages         = Density / 256;
            m_FlashInfo.SectorSize      = 1024 * 4;
            m_FlashInfo.SectorEraseSize = 1024 * 4;
            m_FlashInfo.NbSectors       = Density / 4096;
            m_FlashInfo.FlashID         = FlashID;
        }
        else
        {
            // Error... Flash not supported.
            m_FlashInfo.PageSize  = 0;
            m_FlashInfo.NbSectors = 0;
            m_FlashInfo.FlashID   = FlashID;
        }
    }
    else
    {
        m_Status = SYS_INITIALIZATION_FAIL;
    }

#ifdef DEBUG_TEST_QSPI
    memset(Buffer1, 0xFF, 5000);
    memset(Buffer2, 0xFF, 256);
    memset(Buffer3, 0xFF, 256);

    memcpy(Buffer1, "The quick brown fox jumps over the lazy dog. Portez ce vieux whisky au juge blond qui fume. 0123456789", 102);
    //memcpy(Buffer1, "poutine qwertyuiopasdfghjklzxcvbnm,./<>?;':|}{+_)(*&^%$#@!`~ POUTINE", 68);
    memcpy(Buffer3, "0123456789876543210123456789876543210abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 89);


    StatusFlag = ReadStatus(XSPI_CMD_READ_STATUS);
    StatusFlag = ReadStatus(XSPI_CMD_READ_STATUS_2);
    StatusFlag = ReadStatus(XSPI_CMD_READ_STATUS_3);
    //GlobalSectorUnlock();

   // BulkErase();
    read(Buffer2, 0,  102);
	write(Buffer1, 0, 102);
    read(Buffer2, 0,  102);
    eraseSector(0);
    read(Buffer3, 0,  102);

/*
	write(Buffer1, 100, 10);
	write(Buffer1, 4000, 5000);
	write(Buffer1, 220, 102);
	write(Buffer2, 1000, 68);
	write(Buffer3, 4080, 89);

    read(buffer, 256,  256);
    read(buffer, 0,    256);
    read(buffer, 220,  256);
    read(buffer, 256,  256);
    read(buffer, 1024,  256);
    read(buffer, 1000,  256);
    read(buffer, 4096,  256);
    read(buffer, 4096 - 256, 256);
    read(buffer, 4096 + 10, 256);
*/

	BulkErase();

    for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
    {
        memset(Buffer1, 0xFF, 256);     // Erase buffer
        snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
        write(Buffer1, i * m_FlashInfo.PageSize, 64);
    }

    for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
    {
        read(Buffer2, i * m_FlashInfo.PageSize, 64);
    }

    write(Buffer2, (5 * m_FlashInfo.PageSize) + 128 , 102);

uint8_t Count = 0;

    while(1)
    {
        Count++;

        if((Count % 10) == 0)
        {
            Count = 0;
            BulkErase();
            EraseCounter++;

            // Check if all page are empty 
            memset(Buffer1, 0xFF, 256);
            for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
            {
                read(buffer, i * m_FlashInfo.PageSize, 256);

                if(memcmp(buffer, Buffer1, 64) != 0)
                {
                   ReadErrorCounter1++;
                }
            }

            // Rewrite test content
            for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
            {
                memset(Buffer1, 0xFF, 256);     // Erase buffer
                snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
                write(Buffer1, i * m_FlashInfo.PageSize, 64);
            }

            for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
            {
                read(buffer, i * m_FlashInfo.PageSize, 64);

                memset(Buffer1, 0xFF, 256);     // Erase buffer
                snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
                if(memcmp(buffer, Buffer1, 64) != 0)
                {
                    WriteErrorCounter++;
                }
            }



            write(Buffer2, (5 * m_FlashInfo.PageSize) + 128 , 102);
        }

        for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
        {
            read(buffer, i * m_FlashInfo.PageSize, 64);

            memset(Buffer1, 0xFF, 256);     // Erase buffer
            snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
            if(memcmp(buffer, Buffer1, 64) != 0)
            {
                ReadErrorCounter2++;
            }
        }
    
        read(buffer, (5 * m_FlashInfo.PageSize) + 128, 128);
        if(memcmp(buffer, Buffer2, 128) != 0)
        {
            ReadErrorCounter3++;
        }

        ReadCounter++;

    }
#endif    

    return State;
}

//---------------------------------------------------------------------------------------------
//
//   Function:      XSPI_MSP_Init
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Low level Init XSPI
//
//---------------------------------------------------------------------------------------------
void W25Q_Driver::XSPI_MSP_Init(void)
{
    if(m_Status == SYS_RESET)                                                           // Check if the state is the reset state
    {
        RCC->AHB4ENR |= RCC_AHB4ENR_OCTOSPI1EN;                                         // Enable clock on XSPI module

        // Reset the XSPI memory interface
        RCC->AHB4RSTR |=   RCC_AHB4RSTR_OCTOSPI1RST;
        RCC->AHB4RSTR &= ~(RCC_AHB4RSTR_OCTOSPI1RST);

        IO_PinInit(IO_XSPI_CLK);
        IO_PinInit(IO_XSPI_CS);
        IO_PinInit(IO_XSPI_IO0);
        IO_PinInit(IO_XSPI_IO1);
        IO_PinInit(IO_XSPI_IO2);
        IO_PinInit(IO_XSPI_IO3);

        // Configure memory type, device size, chip select high time, free running clock, clock mode
        MODIFY_REG(m_pXSPI->DCR1, (XSPI_DCR1_MTYP  | XSPI_DCR1_DEVSIZE | XSPI_DCR1_CSHT         | XSPI_DCR1_FRCK           | XSPI_DCR1_CKMODE),
                                  (W25_MEMORY_TYPE | W25_MEMORY_SIZE   | W25_CS_HIGH_TIME_CYCLE | W25_FREE_RUN_CLK_DISABLE | W25_CLOCK_MODE_0));

        SET_BIT(m_pXSPI->DCR1, OCTOSPI_DCR1_DLYBYP);                                                                        // Delay block is bypassed
        CLEAR_BIT(m_pXSPI->DCR2, XSPI_DCR2_WRAPSIZE);                                                                       // Configure wrap size not supported (0)
        CLEAR_BIT(m_pXSPI->DCR3, XSPI_DCR3_CSBOUND);                                                                        // Configure chip select boundary to none
        m_pXSPI->DCR4 = 0;                                                                                                  // Configure refresh
        CLEAR_BIT(m_pXSPI->CR, XSPI_CR_FTHRES);                                                                             // Configure FIFO threshold to 1 byte (0)
        m_Status = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_BUSY, 0, XSPI_TIMEOUT_DEFAULT_VALUE);                            // Wait till busy flag is reset

        if(m_Status == SYS_READY)
        {
            MODIFY_REG(m_pXSPI->DCR2, XSPI_DCR2_PRESCALER, W25_CLOCK_PRESCALER);                                            // Configure clock prescaler
            CLEAR_BIT(m_pXSPI->CR, XSPI_CR_DMM);                                                                            // Configure Memory mode to single memory mode
            MODIFY_REG(m_pXSPI->TCR, (XSPI_TCR_SSHIFT | XSPI_TCR_DHQC), (W25_SAMPLE_SHIFT_NONE | W25_XSPI_DHQC_DISABLE));   // Configure sample shifting and delay hold quarter cycle
            SET_BIT(m_pXSPI->CR, XSPI_CR_EN);                                                                               // Enable XSPI
        }
    
        m_Status = SYS_READY;
    }
}

//---------------------------------------------------------------------------------------------
//
//   Function:      WaitFlagStateUntilTimeout
//
//   Parameter(s):  Flag            Flag to test
//   uint32_t       State           Expected state of the flag,
//                  TimeOut         Timeout duration
//   Return Value:  SystemState_e   State of the function
//
//   Description:   Wait XSPI module to be ready with the requested flag
//
//---------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WaitFlagStateUntilTimeout(uint32_t Flag, uint32_t State, uint32_t TimeOut)
{
    uint32_t Tickstart = HAL_GetTick(); 

    while((READ_BIT(m_pXSPI->SR, Flag) == State) ? FALSE : TRUE)        // Wait until flag is in expected state
    {
        if(TimeOut != HAL_MAX_DELAY)                                    // Check for the Timeout
        {
            if(((HAL_GetTick() - Tickstart) > TimeOut) || (TimeOut == 0))
            {
                m_Status = SYS_ERROR;
                return SYS_TIME_OUT;
            }
        }
    }

    return SYS_READY;
}

//---------------------------------------------------------------------------------------------
//
//   Function:      XSPI_Command
//
//   Parameter(s):  pCmd            Struct that contains the command configuration information
//                  Timeout         Timeout duration
//   Return Value:  SystemState_e   State of the function
//
//   Description:   Set the command configuration.
//
//---------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::XSPI_Command(W25Q_XSPI_RegularCmd_t* const pCmd, uint32_t Timeout)
{
    SystemState_e State;
    
    State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_BUSY, 0, Timeout);      // Wait till busy flag is reset

    if(State == SYS_READY)
    {
        m_ErrorCode = SYS_READY;                                            // Initialize error code
        State = ConfigCmd(pCmd);                                            // Configure the registers

        if(State == SYS_READY)
        {
            if(pCmd->DataMode == XSPI_DATA_NONE)
            {
                // When there is no data phase, the transfer start as soon as the configuration is done so wait until TC flag is set to go back in idle state
                State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_TC, HAL_XSPI_FLAG_TC, Timeout);
            }
            
            m_pXSPI->FCR = XSPI_FCR_CTCF;
        }
    }
    else
    {
        State = SYS_BUSY;
    }

    return State;
}

//---------------------------------------------------------------------------------------------
//
//   Function:      ConfigCmd
//
//   Parameter(s):  pCmd
//   Return Value:  SystemState_e   State of the function
//
//   Description:   Configure the registers for the regular command mode.
//
//   Note(s):       Custom tailored for the W25
//
//---------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ConfigCmd(W25Q_XSPI_RegularCmd_t* pCmd)
{
    SystemState_e State = SYS_READY;

    MODIFY_REG(m_pXSPI->CR, XSPI_CR_FMODE | XSPI_CR_MSEL, 0);           // Re-initialize the value of the functional mode in CR
    MODIFY_REG(m_pXSPI->TCR, XSPI_TCR_DCYC, pCmd->DummyCycles);         // Configure the TCR register with the number of dummy cycles

    m_pXSPI->CCR = XSPI_INSTRUCTION_1_LINE | pCmd->AddressMode | pCmd->DataMode;

    if(pCmd->DataMode != XSPI_DATA_NONE)
    {
        m_pXSPI->DLR = (pCmd->DataLength - 1);                          // Configure the DLR register with the number of data
    }

    m_pXSPI->IR = pCmd->Instruction;                                    // Configure the IR register with the instruction value

    if(pCmd->AddressMode != XSPI_ADDRESS_NONE)
    {
        SET_BIT(m_pXSPI->CCR, XSPI_ADDRESS_24_BITS);                    // W25Q32 is 24 Bits
        m_pXSPI->AR = pCmd->Address;                                    // Configure the AR register with the address value
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      write
//
//   Parameter(s):  pBuffer                 Buffer of the data to write
//                  Address                 Address where to write the data
//                  Size                    number of bytes to write
//   Return Value:  bool                    true or false
//
//   Description:   Write data to the flash
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::write(void* pBuffer, uint32_t Address, size_t Size)
{
	SystemState_e  State = SYS_READY;
    uint8_t*       pBuf  = (uint8_t*)pBuffer;
    uint32_t       PageRemainSize;

    PageRemainSize = m_FlashInfo.PageSize - (Address % m_FlashInfo.PageSize);
    
    // Check if the size of the data is less than the remaining place in the page
    if(PageRemainSize > Size)
    {
        PageRemainSize = Size;
    }

    do
    {
        State = WritePage(pBuf, Address, PageRemainSize);


if(Address == 266240)
{
    __asm("nop");
}


        // Update the address and size variables for next page programming
        Address += PageRemainSize;
        pBuf    += PageRemainSize;
        Size    -= PageRemainSize;
        PageRemainSize = (Size > m_FlashInfo.PageSize) ? m_FlashInfo.PageSize : Size;
    }
    while((Size != 0) && (State == SYS_READY));

	return (State == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      read
//
//   Parameter(s):  pBuffer                 Buffer of the data to write
//                  Address                 Address where to write the data
//                  Size                    number of bytes to write
//   Return Value:  bool                    true or false
//
//   Description:   Read data from the flash
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::read(void* pBuffer, uint32_t Address, size_t Size)
{
	SystemState_e State = SYS_READY;

	if(Size > 0)
	{
        State = ReadCommand(XSPI_CMD_FAST_READ, Address, (uint8_t*)pBuffer, Size, XSPI_TIMEOUT_DEFAULT_VALUE);

if(Address == 266240)
{
    __asm("nop");
}
	}

    return (State == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WritePage
//
//   Parameter(s):  pBuffer                 Buffer of the data to write
//                  Address                 Address where to write the data
//                  Size                    number of bytes to write
//   Return Value:  SystemState_e           State of the function
//
//   Description:   Write a page data to the flash
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WritePage(void* pBuffer, uint32_t Address, size_t Size)
{
    SystemState_e State;

    State = WriteCommand(XSPI_CMD_PAGE_PROG, Address, (uint8_t*)pBuffer, Size, XSPI_TIMEOUT_DEFAULT_VALUE);

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      readID
//
//   Parameter(s):  None
//   Return Value:  uint32_t            ChipID
//
//   Description:   Get the chip ID
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::readID(void)
{
    return m_FlashInfo.FlashID;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getEraseSectorSize
//
//   Parameter(s):  None
//   Return Value:  uint32_t            Page Size
//
//   Description:   Get the page size
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::getEraseSectorSize(void)
{
    return m_FlashInfo.SectorEraseSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getPageSize
//
//   Parameter(s):  None
//   Return Value:  uint32_t            Page Size
//
//   Description:   Get the page size
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::getPageSize(void)
{
    return m_FlashInfo.PageSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getSectorSize
//
//   Parameter(s):  None
//   Return Value:  uint32_t            Sector Size
//
//   Description:   Get the sector size
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::getSectorSize(void)
{
    return m_FlashInfo.SectorSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      eraseSector
//
//   Parameter(s):  SectorAddress       Address of the sector
//   Return Value:  bool                true or false
//
//   Description:   Erase a specific sector
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::eraseSector(uint32_t SectorAddress)
{
if(SectorAddress == 266240)
{
    __asm("nop");
}

   return (EraseSector(SectorAddress, XSPI_TIMEOUT_DEFAULT_VALUE) == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getSizeInBytes
//
//   Parameter(s):  None
//   Return Value:  uint64_t            Flash Size
//
//   Description:   Get the flash size
//
//-------------------------------------------------------------------------------------------------
uint64_t W25Q_Driver::getSizeInBytes(void)
{
    return m_FlashInfo.NbPages * m_FlashInfo.PageSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      sync
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   wait for quad flash to be ready
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::sync(void)
{
    Lock();       
    WaitReady(XSPI_TIMEOUT_DEFAULT_VALUE);
    Unlock();
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      BulkErase
//
//   Parameter(s):  None
//   Return Value:  return true or false
//
//   Description:   Erase all the flash content.
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::BulkErase(void)
{
    SystemState_e State;
	
    if((State = WriteEnable()) == SYS_READY)
    {
        State = SendCommand(XSPI_CMD_CHIP_ERASE);
    }

	return (State == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      EraseSector
//
//   Parameter(s):  SectorAddress       Address of the sector
//                  
//   Return Value:  SystemState_e       SYS_READY of other ERROR
//
//   Description:  Erase a specific sector
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::EraseSector(uint32_t SectorAddress, TickType_t TimeOut)
{
    SystemState_e State;
	
    State = WriteCommand(XSPI_CMD_SECTOR_ERASE, SectorAddress, nullptr, 0, TimeOut);

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadCommand
//
//   Parameter(s):  Command             Command ID to send
//   Return Value:  SystemState_e       State of the function
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ReadCommand(W25Q_Command_e Cmd, uint32_t Address, uint8_t* pBuffer, size_t Size, TickType_t TimeOut)
{
    SystemState_e          State = SYS_READY;
    W25Q_XSPI_RegularCmd_t Command;

    PreConfigCommand(&Command, Cmd);
    Command.Address    = Address;
    Command.DataLength = uint32_t(Size);
    Lock();

    if(XSPI_Command(&Command, TimeOut) == SYS_READY)
    {
      #ifndef USE_FREERTOS        
        m_IsItDMA_Busy = true;
      #endif
        State = Receive_DMA(pBuffer);

        if(State == SYS_READY)
        {
            State = WaitForDMA();
        }
    }

    Unlock();
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteCommand
//
//   Parameter(s):  Command     Command ID to send
//                  Address     Address in flash memory
//                  pBuffer     Data to write
//                  Size        Size of the data
//                  TimeOut     Time out before failing

//   Return Value:  None
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WriteCommand(W25Q_Command_e Cmd, uint32_t Address, uint8_t* pBuffer, size_t Size, TickType_t TimeOut, bool IsWR_Enable)
{
    SystemState_e          State = SYS_READY;
    W25Q_XSPI_RegularCmd_t Command;

    if(IsWR_Enable == true)
    {
        State = WriteEnable();
    }

    if(State == SYS_READY)
    {
        PreConfigCommand(&Command, Cmd);
        Command.Address    = Address;
        Command.DataLength = uint32_t(Size);

        if((State = XSPI_Command(&Command, TimeOut)) == SYS_READY)
        {
            if(Command.DataMode != XSPI_DATA_NONE)
            {
              #ifndef USE_FREERTOS        
                m_IsItDMA_Busy = true;
              #endif
                if((State = Transmit_DMA(pBuffer)) == SYS_READY)
                {
                    State = WaitForDMA();
                }
            }
        }

        State = WaitReady(XSPI_TIMEOUT_DEFAULT_VALUE);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadID
//
//   Parameter(s):  pChipID                 Pointer on variable to deposit Chip ID
//   Return Value:  SystemState_e
//
//   Description:   Get the Chip ID identification
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ReadID(uint32_t* pChipID)
{
    SystemState_e State;
    uint8_t       Buffer[3];

    State = ReadCommand(XSPI_CMD_READ_ID, 0, Buffer, 3, XSPI_TIMEOUT_DEFAULT_VALUE);
	*pChipID = (uint32_t(Buffer[0]) << 16) | (uint32_t(Buffer[1]) << 8) | uint32_t(Buffer[2]);
	
    //for MODBUS Test ID return
    extern uint16_t QuadFlashID;
	QuadFlashID = *pChipID;
  
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadStatus
//
//   Parameter(s):  StatusCMD                   Which status command ID
//   Return Value:  uint8_t                     Status
//
//   Description:   Read one of the 3 status available
//
//-------------------------------------------------------------------------------------------------
uint8_t W25Q_Driver::ReadStatus(W25Q_Command_e Command)
{
    uint8_t Status;

    ReadCommand(Command, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE);

    return Status;
}

SystemState_e W25Q_Driver::GlobalSectorUnlock(void)
{
    SystemState_e State;

    if((State = WriteEnable()) == SYS_READY)
    {
        State = SendCommand(XSPI_CMD_GLOBAL_SECTOR_UNLOCK);
    }

    return State;
}


//-------------------------------------------------------------------------------------------------
//
//  Function:       QuadEnable
//
//  Parameter(s):   None
//  Return Value:   SystemState_e
//
//  Description:    Set the QUAD Enable bit in status register 2.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::QuadEnable(void)
{
    SystemState_e State = SYS_READY;
    uint8_t       Status;

    if((State = ReadCommand(XSPI_CMD_READ_STATUS_2, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        // Make sure we are not in already enable!
        if((Status & QSPI_SR_QUAD_ENABLE_BIT) == 0)
        {
            Status |= QSPI_SR_QUAD_ENABLE_BIT;
            State = WriteCommand(XSPI_CMD_WRITE_STATUS_2, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE, false);

            if(State == SYS_READY)
            {
                Status = 0;

                if((State = ReadCommand(XSPI_CMD_READ_STATUS_2, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
                {
                    if((Status & QSPI_SR_QUAD_ENABLE_BIT) == 0)
                    {
                        State = SYS_INITIALIZATION_FAIL;
                    }
                }
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SetDriverStrength
//
//   Parameter(s):  uint8_t     Strength         Strength value for the output of the IO0 - IO3   
//   Return Value:  
//
//   Description:   This function set the strength of the driver output.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::SetDriverStrength(uint8_t Strength)
{
    SystemState_e State = SYS_READY;
    uint8_t       Status;

    if((State = ReadCommand(XSPI_CMD_READ_STATUS_3, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        // Make sure we are not already at the right driver setting!
        if((Status & MEMORY_DRIVE_MASK) != Strength)
        {
            Status &= ~uint8_t(MEMORY_DRIVE_MASK);
            Status |= Strength;
            State = WriteCommand(XSPI_CMD_WRITE_STATUS_3, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE, true);

            if(State == SYS_READY)
            {
                Status = 0;

                if((State = ReadCommand(XSPI_CMD_READ_STATUS_3, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
                {
                    if((Status & MEMORY_DRIVE_MASK) != Strength)
                    {
                        State = SYS_INITIALIZATION_FAIL;
                    }
                }
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteEnable
//
//   Parameter(s):  
//   Return Value:  
//
//   Description:   This function send a Write Enable and wait it is effective.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WriteEnable(void)
{
    return SendCommand(XSPI_CMD_WRITE_ENABLE);                // Enable write operations
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      PreConfigCommand
//
//   Parameter(s):  pCommand        Command structure to fill
//   Command        Command         Command ID to configure
//   Return Value:  None
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::PreConfigCommand(W25Q_XSPI_RegularCmd_t* pCommand, W25Q_Command_e Command)
{
    memset(pCommand, 0, sizeof(W25Q_XSPI_RegularCmd_t));

    pCommand->Instruction        = m_Cmd[Command].Instruction;
    pCommand->AddressMode        = m_Cmd[Command].AddressMode;
    pCommand->DataMode           = m_Cmd[Command].DataMode;
    pCommand->DummyCycles        = m_Cmd[Command].DummyCycles;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SendCommand
//
//   Parameter(s):  Cmd                 Command ID to send
//   Return Value:  SystemState_e
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::SendCommand(W25Q_Command_e Cmd)
{
    SystemState_e          State;
    W25Q_XSPI_RegularCmd_t Command;

    PreConfigCommand(&Command, Cmd);
    
    if((State = XSPI_Command(&Command, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        State = WaitReady(XSPI_TIMEOUT_DEFAULT_VALUE);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       WaitReady
//
//  Parameter(s):   TimeOut      Duration of the time out
//  Return Value:
//
//  Description:    Wait for the SPI module to be ready.
//
//  Note(s):        return  SYS_READY     if available
//                          SYS_TIME_OUT  if busy pass timeout
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WaitReady(TickType_t TimeOut)
{
    W25Q_XSPI_RegularCmd_t  Command;
    SystemState_e           State = SYS_READY;

    PreConfigCommand(&Command, XSPI_CMD_READ_STATUS);
    Command.DataLength = 1;
    
    if(XSPI_Command(&Command, XSPI_TIMEOUT_DEFAULT_VALUE) == SYS_READY)
    {
        State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_BUSY, 0, TimeOut);                      // Wait till busy flag is reset

        if(State == SYS_READY)
        {
            // Configure registers
            WRITE_REG(m_pXSPI->PSMAR, MEMORY_READY_MATCH_VALUE);
            WRITE_REG(m_pXSPI->PSMKR, MEMORY_READY_MASK_VALUE);
            WRITE_REG(m_pXSPI->PIR,   AUTO_POLLING_INTERVAL);
            MODIFY_REG(m_pXSPI->CR,   (XSPI_CR_PMM | XSPI_CR_APMS | XSPI_CR_FMODE),
                                      (HAL_XSPI_MATCH_MODE_AND | HAL_XSPI_AUTOMATIC_STOP_ENABLE | XSPI_FUNCTIONAL_MODE_AUTO_POLLING));

            if(READ_BIT(m_pXSPI->CCR, XSPI_CCR_ADMODE) != XSPI_ADDRESS_NONE)                    // Trig the transfer by re-writing address or instruction register
            {
                WRITE_REG(m_pXSPI->AR, Command.Address); //AddressRegister);
            }
            else
            {
                WRITE_REG(m_pXSPI->IR, Command.Instruction);//InstructionRegister);
            }

            State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_SM, HAL_XSPI_FLAG_SM, TimeOut);     // Wait till status match flag is set to go back in idle state

            if(State == SYS_READY)
            {
                m_pXSPI->FCR = XSPI_FCR_CSMF;                                                   // Clear status match flag
                m_Status = SYS_READY;
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Lock the driver
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::Lock(void)
{
  #ifdef USE_FREERTOS
    taskENTER_CRITICAL();
    xSemaphoreTake(m_Mutex, portMAX_DELAY);
  #endif
    m_Status = SYS_LOCK;
  #ifdef USE_FREERTOS
    taskEXIT_CRITICAL();
  #endif    
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Unlock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Unlock the driver
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::Unlock(void)
{
  #ifdef USE_FREERTOS
    taskENTER_CRITICAL();
    xSemaphoreGive(m_Mutex);
  #endif 
    m_Status = SYS_UNLOCK;
  #ifdef USE_FREERTOS
    taskEXIT_CRITICAL();
  #endif    
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadSFDP_Density
//
//   Parameter(s):  uint32_t*       pDensity            Pointer to return density value
//   Return Value:  SystemState_e
//
//   Description:   Get density value of the flash
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ReadSFDP_Density(uint32_t* pDensity)
{
    SystemState_e State = SYS_READY;
    
    *pDensity = 0;

    // Check JEDEC serial flash discoverable parameters for device specific info
    uint8_t Header[16];

    if((State = ReadCommand(XSPI_CMD_READ_DISCOVERY_PARAMETER, 0, Header, 16, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        // Verify SFDP signature for sanity, and check that major/minor version is acceptable
        if((memcmp(&Header[0], "SFDP", 4) != 0) || (Header[5] != 1))
        {
            return State;
        }

        // The SFDP spec indicates the standard table is always at offset 0
        // in the parameter headers, we check just to be safe
        if((Header[8] != 0) || (Header[10] != 1))
        {
            return State;
        }

        // Parameter table pointer, spi commands are BE, SFDP is LE,
        // also sfdp command expects extra read wait byte
        uint8_t Table[8];
        uint32_t TableAddress = (Header[14] << 16) | (Header[13] << 8) | Header[12];
        
        if((State = ReadCommand(XSPI_CMD_READ_DISCOVERY_PARAMETER, TableAddress, Table, 8, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
        {
            // Check erase size, currently only supports 4 kbytes
            if(((Table[0] & 0x3) != 0x1) || (Table[1] != 0x20)) // 4K erase sector
            {
                return State;
            }

            // Check address size, currently only supports 3 bytes addresses
            if(((Table[2] & 0x4) != 0) || ((Table[7] & 0x80) != 0))
            {
                return State;
            }

            // Get device density, stored as size in bits - 1
            *pDensity = ((Table[7] << 24) |  (Table[6] << 16) | (Table[5] << 8 ) | (Table[4] << 0 ));
            *pDensity = (*pDensity + 1) / 8;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Transmit_DMA
//
//   Parameter(s):  uint8_t* pData          Pointer to data buffer
//   Return Value:  SystemState_e
//
//   Description:   Send an amount of data in non-blocking mode with DMA.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::Transmit_DMA(uint8_t* pData)
{
    SystemState_e  State     = SYS_READY;
    uint32_t       DataSize  = m_pXSPI->DLR + 1;

    #ifndef USE_FREERTOS        
    m_IsItDMA_Busy = true;
    #endif  

    MODIFY_REG(m_pXSPI->CR, XSPI_CR_FMODE, XSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);	            // Configure CR register with functional mode as indirect write
    m_pXSPI->FCR = XSPI_FCR_CTEF | XSPI_FCR_CTCF; 				                                // Clear flags related to interrupt
    m_DMA_TX.EnableInterrupt(DMA_IT_TC | DMA_IT_DTE | DMA_IT_ULE | DMA_IT_USE | DMA_IT_TO);
    m_DMA_TX.SetSource(pData);                                                                  // Set DMA source
    m_DMA_TX.SetLength(DataSize);                                                               // Set size of the TX
    m_Status = SYS_BUSY_TX;                                                                     // Update the state
    m_DMA_TX.Enable();                                                                          // Enable the DMA module
    m_DMA_TX.ClearFlag();                                                                       // Clear IRQ DMA flag
    SET_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);                                                        // Enable the DMA transfer by setting the DMAEN bit

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Receive_DMA
//
//   Parameter(s):  uint8_t* pData          Pointer to data buffer
//   Return Value:  SystemState_e
//
//   Description:   Receive an amount of data in non-blocking mode with DMA.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::Receive_DMA(uint8_t* pData)
{
    SystemState_e  State         = SYS_READY;
    uint32_t       DataSize      = m_pXSPI->DLR + 1;
	uint32_t AddressRegister     = m_pXSPI->AR;
	uint32_t InstructionRegister = m_pXSPI->IR;

    #ifndef USE_FREERTOS        
    m_IsItDMA_Busy = true;
    #endif  

    MODIFY_REG(m_pXSPI->CR, XSPI_CR_FMODE, XSPI_FUNCTIONAL_MODE_INDIRECT_READ);	                // Configure CR register with functional mode as indirect read
    m_pXSPI->FCR = XSPI_FCR_CTEF | XSPI_FCR_CTCF; 			                	                // Clear flags related to interrupt
    m_DMA_RX.EnableInterrupt(DMA_IT_TC | DMA_IT_DTE | DMA_IT_ULE | DMA_IT_USE | DMA_IT_TO);
    m_DMA_RX.SetDestination(pData);                                                             // Set DMA source
    m_DMA_RX.SetLength(DataSize);                                                               // Set size of the RX
    m_Status = SYS_BUSY_RX;                                                                     // Update the state
    m_DMA_RX.Enable();                                                                          // Enable the DMA module
    m_DMA_RX.ClearFlag();                                                                       // Clear IRQ DMA flag

    if(READ_BIT(m_pXSPI->CCR, XSPI_CCR_ADMODE) != XSPI_ADDRESS_NONE)
    {
        WRITE_REG(m_pXSPI->AR, AddressRegister);
    }
    else
    {
        WRITE_REG(m_pXSPI->IR, InstructionRegister);
    }

    SET_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);              // Enable the DMA transfer by setting the DMAEN bit

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_RX_IRQ_Handler and DMA_TX_IRQ_Handler
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   DMA IRQ handler for XSPI
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::DMA_RX_IRQ_Handler(void)
{
	m_pXSPI->FCR = XSPI_FCR_CTCF;
    m_Status = SYS_READY;
    CLEAR_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);    // Disable the DMA transfer on the XSPI side
    m_DMA_RX.Disable();
    m_DMA_RX.ClearFlag();                               // Clear TC and HT transfer flags
    ReleaseFromISR();
}

void W25Q_Driver::DMA_TX_IRQ_Handler(void)
{
	m_pXSPI->FCR = XSPI_FCR_CTCF;
    m_Status = SYS_READY;
    CLEAR_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);    // Disable the DMA transfer on the XSPI side
    m_DMA_TX.Disable();
    m_DMA_TX.ClearFlag();                               // Clear TC and HT transfer flags
    ReleaseFromISR();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReleaseFromISR
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Common code for DMA RX and TX
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::ReleaseFromISR(void)
{
  #ifdef USE_FREERTOS
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(m_DMA_Busy, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  #else
    m_IsItDMA_Busy = false;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WaitForDMA
//
//  Parameter(s):   None
//  Return:         SystemState_e               SYS_READY or SYS_TIMEOUT
//
//  Description:    Wait for DMA to release flag or FreeRTOS semaphore.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WaitForDMA(void)
{
    SystemState_e State;
  #ifndef USE_FREERTOS
    uint32_t    TickStart  = GetTick();
    uint32_t    ElapseTick;
  #endif

  #ifdef USE_FREERTOS
    State = (xSemaphoreTake(m_DMA_Busy, XSPI_TIMEOUT_DEFAULT_VALUE) == pdFALSE) ? SYS_TIME_OUT : SYS_READY;
  #else
    // Don't do a busy wait if
    //           1) We're in FreeRTOS context
    //           2) and not in an interrupt
    //           3) and the semaphore raised within 100ms
    do
    {
        State = TickHasTimeOut(TickStart, XSPI_TIMEOUT_DEFAULT_VALUE) ? SYS_TIME_OUT : SYS_READY;
    }
    while((m_IsItDMA_Busy == true) && (State == SYS_READY));

    #endif

    return State;
}

//-------------------------------------------------------------------------------------------------

extern "C"
{
    //---------------------------------------------------------------------------------------------
    //
    //   Function:      GPDMA1_Channel6_IRQHandler
    //
    //   Parameter(s):  None
    //   Return Value:  None
    //
    //   Description:   This function handles GPDMA1 Channel 6 global interrupt for OCTOSPI.
    //
    //---------------------------------------------------------------------------------------------
    void GPDMA1_Channel6_IRQHandler(void)
    {
        W25Q32.DMA_RX_IRQ_Handler();
    }

    //---------------------------------------------------------------------------------------------
    //
    //   Function:      GPDMA1_Channel7_IRQHandler
    //
    //   Parameter(s):  None
    //   Return Value:  None
    //
    //   Description:   This function handles GPDMA1 Channel 7 global interrupt for OCTOSPI
    //
    //---------------------------------------------------------------------------------------------
    void GPDMA1_Channel7_IRQHandler(void)
    {
        W25Q32.DMA_TX_IRQ_Handler();
    }

    //---------------------------------------------------------------------------------------------
}

//-------------------------------------------------------------------------------------------------
