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
#include "lib_digini.h"
#undef  LIB_SERIAL_FLASH_GLOBAL
//#include "lib_class_serial_flash.h"

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
