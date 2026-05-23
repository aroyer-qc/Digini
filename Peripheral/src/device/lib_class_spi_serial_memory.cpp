//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_serial_memory.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#define SPI_MEMORY_GLOBAL
#include "./lib_digini.h"
#undef  SPI_MEMORY_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MEM_SFDP_PAGE_SIZE                	256             // At this time page are always 256 bytes
#define MEM_SFDP_SECTOR_SIZE              	4096            // At this time sector are always 4096 bytes

#define	MEM_WIP_FLAG		              	uint8_t(0x01)   // Write in progress Flag

#define MEM_SST_DEVICES                   	uint32_t(0xBF0000)

#define MEM_WAIT_LOOP_DELAY               	2
#define MEM_WAIT_LOOP_RETRY               	15

#define SFDP_SIGNATURE_0_OFFSET             0
#define SFDP_MAJOR_REVISION_OFFSET          5
#define SFDP_PARAM_ID_LSB_OFFSET            8
#define SFDP_PARAM_ID_MSB_OFFSET            9
#define SFDP_PARAM_MINOR_OFFSET             10
#define SFDP_PARAM_TABLE_PTR_0              12
#define SFDP_PARAM_TABLE_PTR_1              13
#define SFDP_PARAM_TABLE_PTR_2              14
#define SFDP_TABLE_ERASE_SUPPORT_OFFSET     0
#define SFDP_TABLE_ERASE_OPCODE_OFFSET      1
#define SFDP_TABLE_ADDRESS_BYTES_OFFSET     2
#define SFDP_SIGNATURE                      "SFDP"
#define SFDP_SIGNATURE_SIZE                 4
#define SFDP_BYTE_SIZE                      8
#define SFDP_HEADER_SIZE                    16
#define SFDP_BASIC_FLASH_PARAMETER_TABLE    8

#define BFPT_TABLE_SIZE						64
#define BFPT_ADDRESSING_MODE_OFFSET			2
#define BFPT_DENSITY_OFFSET_0               4   // DWORD 1,  byte 0
#define BFPT_DENSITY_OFFSET_1               5
#define BFPT_DENSITY_OFFSET_2               6
#define BFPT_DENSITY_OFFSET_3               7

#define BFPT_ERASE_TYPE_1_SIZE_EXP_OFFSET   28  // DWORD 7
#define BFPT_ERASE_TYPE_1_OPCODE_OFFSET     29
#define BFPT_ERASE_TYPE_2_SIZE_EXP_OFFSET   30  // DWORD 8
#define BFPT_ERASE_TYPE_2_OPCODE_OFFSET     31
#define BFPT_ERASE_TYPE_3_SIZE_EXP_OFFSET   32  // DWORD 9
#define BFPT_ERASE_TYPE_3_OPCODE_OFFSET     33
#define BFPT_ERASE_TYPE_4_SIZE_EXP_OFFSET   34  // DWORD 10
#define BFPT_ERASE_TYPE_4_OPCODE_OFFSET     35
#define BFPT_CHIP_ERASE_OPCODE_OFFSET       44  // DWORD 11, byte 0
#define BFPT_PAGE_SIZE_OFFSET_LSB           45  // DWORD 11, byte 1
#define BFPT_PAGE_SIZE_OFFSET_MSB           46  // DWORD 11, byte 2

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const MemoryInfo_t SPI_SerialMemoryDriver::m_MemoryInfoList[NUMBER_OF_MEMORY] =
{
    SERIAL_MEMORY_DEF(EXPAND_X_SERIAL_MEM_AS_CLASS_CONST)
};

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  SPI_Driver*   pSPI          Pointer to SPI driver instance
//                  IO_ID_e       ChipSelect    Chip-select line for the memory device
//
//                  (Optional) MemoryList_e Memory
//                      Used when auto-detection is disabled to select a memory entry
//                      from the static memory information table.
//
//   Return Value:  SystemState_e       SYS_READY on success, or SYS_ERROR if the memory device is
//                                      unsupported or initialization fails.
//
//   Description:   Initializes the serial memory driver. The function stores the SPI driver
//                  pointer, configures the chip-select line, initializes the SPI interface,
//                  reads the JEDEC ID, performs device-specific unlock sequences (SST),
//                  and either auto-detects memory parameters using SFDP or loads predefined
//                  memory information from the static table.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::Initialize(SPI_Driver* pSPI, MemoryList_e Memory, IO_ID_e ChipSelect)
{
    uint32_t MemoryID = 0;

    m_pSPI       = pSPI;
    m_ChipSelect = ChipSelect;

    m_pSPI->Initialize();

    // Memory is defined
    if(Memory < NUMBER_OF_MEMORY)
    {
        const MemoryInfo_t* pInfo = &m_MemoryInfoList[Memory];

        // Read the ID only if supported
        if(pInfo->SupportOptions & MEM_OPT_READ_ID)
        {
            MemoryID = ReadID();

            // For SST : unlock sequence is mandatory
            if((MemoryID & MEM_SST_DEVICES) == MEM_SST_DEVICES)
            {
                m_pSPI->LockToDevice(m_ChipSelect, false);
                WriteEnable();
                m_pSPI->SelectChip(m_ChipSelect);
                m_pSPI->Write(MEMORY_CMD_UNLOCK);
                m_pSPI->DeSelectChip(m_ChipSelect);
                m_pSPI->UnlockFromDevice(m_ChipSelect, false);
            }
        }

        // Copier la structure complète
        memcpy(&m_MemoryInfo, pInfo, sizeof(MemoryInfo_t));
        m_MemoryInfo.MemoryID = MemoryID;

        return SYS_READY;
    }

  #if (SERIAL_MEMORY_USE_AUTO_DETECT == DEF_ENABLED)
    // Auto detect mode
    if(Memory == FLASH_AUTO_DETECT)
    {
        // Parse SFDP and fill m_MemoryInfo
        if(ParseSFDP() != SYS_READY)
        {
            return SYS_FAIL;
        }

        m_MemoryInfo.MemoryID = MemoryID;

        return SYS_READY;
    }
  #endif

    // If we reach here, then the memory is invalid
    return SYS_FAIL;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: ReadID
//
//   Parameter(s):  None
//   Return value:  uint32_t       24-bit JEDEC device ID
//
//   Description:   Sends the READ ID command to the serial memory device and retrieves the 3 bytes
//                  manufacturer/device identifier. The command is issued over the SPI interface
//                  and the returned bytes are combined into a single 24-bit value.
//
//-------------------------------------------------------------------------------------------------
uint32_t SPI_SerialMemoryDriver::ReadID(void)
{
	if((m_MemoryInfo.SupportOptions & MEM_OPT_READ_ID) != 0)
	{
		uint8_t     Buffer[3] = {0x00, 0x00, 0x00};
		uint8_t     Command   = MEMORY_CMD_READ_ID;

		// Send "MEMORY_CMD_READ_ID" instruction and read ID
		m_pSPI->Transfer(&Command, 1, Buffer, 3, m_ChipSelect);
		return (uint32_t(Buffer[0]) << 16) | (uint32_t(Buffer[1]) << 8) | uint32_t(Buffer[2]);
	}

	return 0x00000000;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SendCommandAndAddress
//
//   Parameter(s):  Command        Serial memory opcode to send
//                  Address        Target address (24-bit or 32-bit depending on device)
//
//   Return value:  SystemState_e  SPI transaction result
//
//   Description:   Sends a serial-memory command followed by the device address. The opcode is
//                  always transmitted first, then the address bytes in big-endian order. The
//                  number of address bytes depends on the device configuration:
//
//                      - 3 bytes  when MEM_OPT_4_BYTES_ADDR is not set
//                      - 4 bytes  when MEM_OPT_4_BYTES_ADDR is set
//
//                  This function does not perform byte-swapping or bitwise merging. The command
//                  and address are sent exactly as required by the memory protocol.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::SendCommandAndAddress(SerialMemoryCmd_e Command, uint32_t Address)
{
	uint8_t  Buffer[5];
	uint32_t Index = 0;

    Buffer[Index++] = Command;

    if(Command != MEMORY_CMD_READ_SFPD)
    {
        if((m_MemoryInfo.SupportOptions & MEM_OPT_4_BYTES_ADDR) != 0)
        {
            Buffer[Index++] = uint8_t(Address >> 24);
        }
    }

	Buffer[Index++] = uint8_t(Address >> 16);
	Buffer[Index++] = uint8_t(Address >> 8);
	Buffer[Index++] = uint8_t(Address);

    return m_pSPI->Write(Buffer, Index);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Read
//
//   Parameter(s):  void*          pBuffer        Destination buffer
//                  uint32_t       Address        Memory address to read from
//                  size_t         Length         Number of bytes to read
//
//   Return Value:  SystemState_e  SYS_READY on success, or an error code if the device is busy,
//                                 communication fails, or the SPI interface cannot be
//                                 locked/unlocked.
//
//   Description:   Reads a block of data from the serial memory device. The function locks the
//                  SPI interface, waits for any ongoing program/erase operation to complete,
//                  sends the READ or FAST READ command (depending on device capabilities),
//                  transmits the address, inserts the required dummy byte for FAST READ, then
//                  reads the requested number of bytes. The SPI interface is unlocked before
//                  returning. Any error encountered is propagated.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::Read(void* pBuffer, uint32_t Address, size_t Length)
{
    SystemState_e State;

    if(Length == 0)
    {
        return SYS_READY;
    }

    State = m_pSPI->LockToDevice(m_ChipSelect);

	if(State != SYS_READY)
    {
        return State;
    }

    State = WaitForEndWrite();

    if(State == SYS_READY)
    {
        m_pSPI->SelectChip(m_ChipSelect);

        // Choose READ or FAST READ according to options
        SerialMemoryCmd_e Cmd = (m_MemoryInfo.SupportOptions & MEM_OPT_FAST_READ) ? MEMORY_CMD_FAST_READ : MEMORY_CMD_READ;
        State = SendCommandAndAddress(Cmd, Address);

        if(State == SYS_READY)
        {
            // FAST READ -> mandatory dummy byte
            if(m_MemoryInfo.SupportOptions & MEM_OPT_FAST_READ)
            {
                uint8_t Dummy = 0x00;
                State = m_pSPI->Write(&Dummy, 1);
            }

            if(State == SYS_READY)
            {
                State = m_pSPI->Read((uint8_t*)pBuffer, Length);
            }
        }

        m_pSPI->DeSelectChip(m_ChipSelect);
    }

    SystemState_e UnlockState = m_pSPI->UnlockFromDevice(m_ChipSelect);

	if(UnlockState != SYS_READY)
    {
        return UnlockState;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Write
//
//   Parameter(s):  const void*    pBuffer       Source buffer
//                  uint32_t       Address       Memory address to write to
//                  size_t         Length        Number of bytes to write
//
//   Return Value:  SystemState_e  SYS_READY on success, or an error code if the device is busy,
//                                parameters are invalid, or SPI communication fails.
//
//   Description:   Writes a block of data to the serial memory device. The function locks the
//                  SPI interface, aligns the first write to the current page boundary, and then
//                  programs data page-by-page using WritePage(). The write operation continues
//                  until all bytes are written or an error occurs. The SPI interface is unlocked
//                  before returning.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::Write(const void* pBuffer, uint32_t Address, size_t Length)
{
    SystemState_e State;

    if((pBuffer == nullptr) || (Length == 0))
    {
        return SYS_INVALID_PARAMETER;
    }

    State = m_pSPI->LockToDevice(m_ChipSelect, false);

	if(State != SYS_READY)
    {
        return State;
    }

    // EEPROM-style: write in a single operation (no page boundary)
    if(m_MemoryInfo.SupportOptions & MEM_OPT_EEPROM_STYLE)
    {
        State = WriteBuffer((void*)pBuffer, Address, Length);

        SystemState_e UnlockState = m_pSPI->UnlockFromDevice(m_ChipSelect, false);
        return (UnlockState == SYS_READY) ? State : UnlockState;
    }

    // FLASH: page-based programming
    uint8_t* pData = (uint8_t*)pBuffer;
    uint32_t Size  = Address % m_MemoryInfo.PageSize;

    if(Size != 0)
    {
        Size     = m_MemoryInfo.PageSize - Size;
        State    = WriteBuffer(pData, Address, Size);
        Length  -= Size;
        pData   += Size;
        Address += Size;
    }

    while((Length > 0) && (State == SYS_READY))
    {
        Size     = (Length < m_MemoryInfo.PageSize) ? Length : m_MemoryInfo.PageSize;
        State    = WriteBuffer(pData, Address, Size);
        Length  -= Size;
        pData   += Size;
        Address += Size;
    }

    SystemState_e UnlockState = m_pSPI->UnlockFromDevice(m_ChipSelect, false);
    return (UnlockState == SYS_READY) ? State : UnlockState;
}
//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteBuffer
//
//   Parameter(s):  void*          pBuffer        Source buffer
//                  uint32_t       Address        Memory address to program
//                  size_t         Length         Number of bytes to write (within a single page)
//
//   Return Value:  SystemState_e  SYS_READY on success, or an error code if the device is busy or
//                                 communication fails.
//
//   Description:   Issues a Write operation to the serial memory device. The function assumes the
//                  SPI interface is already locked and that the write does not cross a page
//                  boundary. It waits for any ongoing program/erase operation to complete, enables
//                  writing, sends the appropriate write command and address, and writes the
//                  specified number of bytes. Completion polling is handled by the caller.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::WriteBuffer(void* pBuffer, uint32_t Address, size_t Length)
{
    SystemState_e State;

    State = WaitForEndWrite();                                                  // Ensure device is idle

    if(State == SYS_READY)
    {
        State = WriteEnable();                                                  // Enable write

        if(State == SYS_READY)
        {
            m_pSPI->SelectChip(m_ChipSelect);

            State = SendCommandAndAddress(MEMORY_CMD_WRITE, Address);     		// Issue write command

            if(State == SYS_READY)
            {
                State = m_pSPI->Write((uint8_t*)pBuffer, Length);
            }

            m_pSPI->DeSelectChip(m_ChipSelect);
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      BulkErase
//
//   Parameter(s):  None
//
//   Return Value:  SystemState_e
//                  SYS_READY on success, or an error code propagated from the SPI driver or
//                  internal sequencing functions.
//
//   Description:   Performs a full‑device erase operation on memories that support the bulk erase
//                  command. The function first verifies that bulk erase is supported by checking
//                  MEM_OPT_BULK_ERASE in the memory capability flags. If unsupported, the function
//                  returns SYS_FAIL without issuing any command.
//
//                  When supported, the function locks the SPI interface for exclusive access
//                  (without asserting chip select), issues a Write Enable command, asserts chip
//                  select, transmits the bulk erase opcode, and finally releases the SPI lock.
//
//                  All intermediate operations (locking, write‑enable, command transmission,
//                  unlocking) are validated. Any failure in the sequence immediately aborts the
//                  operation and the corresponding error code is returned.
//
//                  Note: This function only issues the erase command. It does not poll the memory
//                  device for completion. The caller is responsible for monitoring the device’s
//                  status register until the erase operation has finished.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::BulkErase(void)
{
	SystemState_e State = SYS_READY;

    // Guard : bulk erase not supported
    if((m_MemoryInfo.SupportOptions & MEM_OPT_BULK_ERASE) == 0)
    {
        // If no bulk erase but sector erase exists → erase sector by sector
        if(m_MemoryInfo.SupportOptions & MEM_OPT_SECTOR_ERASE)
        {
            for(uint32_t i = 0; i < m_MemoryInfo.NumberOfSectors; i++)
            {
                State = EraseSector(i);

                if(State != SYS_READY)
                {
                    return State;
                }
            }
            return SYS_READY;
        }

        // EEPROM case : no erase command -> write 0xFF everywhere
        uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndSet(256, 0xFF);
        uint32_t Address = 0;
        uint32_t Remaining = m_MemoryInfo.NumberOfPages * m_MemoryInfo.PageSize;

        while(Remaining > 0)
        {
            uint32_t Chunk = (Remaining > 256) ? 256 : Remaining;

           State = Write(pBuffer, Address, Chunk);

			if(State != SYS_READY)
            {
                break;
            }

            Address   += Chunk;
            Remaining -= Chunk;
        }

		pMemoryPool->Free((void**)&pBuffer);
        return State;
    }

    State = m_pSPI->LockToDevice(m_ChipSelect, false);             				// Lock SPI access (BulkErase controls CS)

    if(State != SYS_READY)
    {
        return State;
    }

    State = WriteEnable();                                                      // Enable write

    if(State == SYS_READY)
    {
        m_pSPI->SelectChip(m_ChipSelect);
        State = m_pSPI->Write(m_MemoryInfo.ChipEraseOpCode);                    // Send bulk erase command
        m_pSPI->DeSelectChip(m_ChipSelect);
    }

    SystemState_e UnlockState = m_pSPI->UnlockFromDevice(m_ChipSelect, false);  // Unlock SPI access

    if(UnlockState != SYS_READY)
    {
        return UnlockState;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      EraseSector
//
//   Parameter(s):  uint32_t            Address             Address of the sector to erase
//
//   Return Value:  SystemState_e       SYS_READY or SYS_ERROR
//
//   Description:   Sends an erase command to the serial memory device. The function locks the SPI
//                  interface to the target device, asserts chip select, transmits the erase opcode
//                  and address, then releases the device lock. Any communication error is
//                  propagated through the returned state.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::EraseSector(uint32_t Address)
{
    // Guard : sector erase not supported
    if((m_MemoryInfo.SupportOptions & MEM_OPT_SECTOR_ERASE) == 0)
    {
        return SYS_FAIL;
    }

    SystemState_e State;

    State = m_pSPI->LockToDevice(m_ChipSelect);                                 // Lock SPI access to this device

    if(State != SYS_READY)
    {
        return State;
    }

    State = SendCommandAndAddress(m_MemoryInfo.SectorEraseOpCode, Address);
    SystemState_e UnlockState = m_pSPI->UnlockFromDevice(m_ChipSelect);         // Unlock SPI access

    if(UnlockState != SYS_READY)
    {
        return UnlockState;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WaitForEndWrite
//
//   Parameter(s):  None
//
//   Return Value:  SystemState_e       SYS_READY or SYS_TIME_OUT or SYS_ERROR
//
//   Description:   Polls the memory device's status register until the Write-In-Progress (WIP)
//                  bit clears or a retry timeout occurs. The function asserts chip select,
//                  issues the READ STATUS command, reads the status byte, and repeats until
//                  the write cycle completes. Any SPI communication error or timeout is
//                  propagated through the returned state.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::WaitForEndWrite(void)
{
	SystemState_e State;
	uint8_t	      Command;
	uint8_t	      ReadValue;
	int           Retry;

    Retry   = MEM_WAIT_LOOP_RETRY;
    Command = MEMORY_CMD_READ_STATUS_REGISTER;

	do
	{
        nOS_Sleep(MEM_WAIT_LOOP_DELAY); 							            // The write page take 11 ms typic. 25ms maximum.
        Retry--;
		m_pSPI->SelectChip(m_ChipSelect);
		State = m_pSPI->Transfer(&Command, 1, &ReadValue, 1);		            // Send "Read Status" instruction
		m_pSPI->DeSelectChip(m_ChipSelect);
	}
	while(((ReadValue & MEM_WIP_FLAG) == MEM_WIP_FLAG) && (Retry > 0)); 	// Write in progress

	if(Retry == 0)
    {
        State = SYS_TIME_OUT;
    }

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteEnable
//
//   Parameter(s):  None
//
//   Return Value:  SystemState_e       SYS_READY or ....
//
//   Description:   Sends the Write Enable (WREN) command to the memory device. The function
//                  asserts chip select, transmits the WREN opcode, and then releases chip select.
//                  This sets the Write Enable Latch (WEL), allowing subsequent program or erase
//                  operations to be accepted by the device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::WriteEnable(void)
{
    m_pSPI->SelectChip(m_ChipSelect);
	SystemState_e State = m_pSPI->Write(MEMORY_CMD_WRITE_ENABLE);        // Send "Write Enable" instruction
    m_pSPI->DeSelectChip(m_ChipSelect);

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteDisable
//
//   Parameter(s):  None
//
//   Return Value:  SystemState_e       SYS_READY or ....
//
//   Description:   Sends the Write Disable (WRDI) command to the memory device. The function
//                  asserts chip select, transmits the WRDI opcode, and then releases chip select.
//                  This clears the Write Enable Latch (WEL) and prevents any program or erase
//                  operations until WriteEnable() is issued again.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::WriteDisable(void)
{
    m_pSPI->SelectChip(m_ChipSelect);
	SystemState_e State = m_pSPI->Write(MEMORY_CMD_WRITE_DISABLE);       // Send "Write Disable" instruction
    m_pSPI->DeSelectChip(m_ChipSelect);

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadSFDP
//
//   Parameter(s):  uint32_t    NumberOfByteToRead     Number of bytes to read from the SFDP table
//                  uint32_t    Address                Starting SFDP address
//                  uint8_t*    pBuffer                Destination buffer
//
//   Return Value:  SystemState_e                      SYS_READY or SYS_ERROR
//
//   Description:   Reads data from the Serial Memory Discoverable Parameters (SFDP) table. The
//                  function locks the SPI interface, sends the SFDP read command and address,
//                  performs a dummy byte transfer as required by the SFDP protocol, reads the
//                  requested number of bytes, then unlocks the SPI interface.
//
//-------------------------------------------------------------------------------------------------
#if (SERIAL_MEMORY_USE_AUTO_DETECT == DEF_ENABLED)
SystemState_e SPI_SerialMemoryDriver::ReadSFDP(uint32_t NumberOfByteToRead, uint32_t Address, uint8_t *pBuffer)
{
    SystemState_e State;
	uint8_t Dummy = 0;

    State = m_pSPI->LockToDevice(m_ChipSelect);                             // Let it handle the  CS

    if(State != SYS_READY)
    {
        return State;
    }

    State = SendCommandAndAddress(MEMORY_CMD_READ_SFPD, Address);

    if(State == SYS_READY)
    {
        State = m_pSPI->Transfer(&Dummy, 1, pBuffer, NumberOfByteToRead);
    }

    SystemState_e UnlockState = m_pSPI->UnlockFromDevice(m_ChipSelect);    // Let it Release the CS

    if(UnlockState != SYS_READY)
    {
        return UnlockState;
    }

    return State;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function:      ParseSFDP
//
//   Parameter(s):  None
//
//   Return Value:  SystemState_e
//                  SYS_READY on success, SYS_FAIL on invalid SFDP or communication error.
//
//   Description:   Reads the SFDP header and the Basic Flash Parameter Table (BFPT), extracts
//                  density, erase types, page size, and chip erase opcode, and fills the
//                  MemoryInfo_t structure accordingly. Only parameters defined by the SFDP
//                  specification are used.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialMemoryDriver::ParseSFDP(void)
{
    uint8_t  Header						[SFDP_HEADER_SIZE];
    uint8_t  BasicFlashParameterTable	[BFPT_TABLE_SIZE];
    uint32_t DensityBits  = 0;
    uint32_t FlashDensity = 0;

    // Read SFDP header
    if(ReadSFDP(SFDP_HEADER_SIZE, 0x00, Header) != SYS_READY)
    {
        return SYS_FAIL;
    }

    // Validate SFDP signature
    if(memcmp(Header, "SFDP", 4) != 0)
    {
        return SYS_FAIL;
    }

    // Extract BFPT address
    uint32_t BFPT_Address = (Header[SFDP_PARAM_TABLE_PTR_2] << 16)  | (Header[SFDP_PARAM_TABLE_PTR_1] << 8) | Header[SFDP_PARAM_TABLE_PTR_0];

    // Read BFPT (64 bytes is enough for density + erase types + page size)
    if(ReadSFDP(BFPT_TABLE_SIZE, BFPT_Address, BasicFlashParameterTable) != SYS_READY)
    {
        return SYS_FAIL;
    }

    // Density (1)
    DensityBits  = (BasicFlashParameterTable[BFPT_DENSITY_OFFSET_3] << 24) |
                   (BasicFlashParameterTable[BFPT_DENSITY_OFFSET_2] << 16) |
                   (BasicFlashParameterTable[BFPT_DENSITY_OFFSET_1] << 8)  |
                    BasicFlashParameterTable[BFPT_DENSITY_OFFSET_0];

    FlashDensity = (DensityBits + 1) / 8;   // Convert bits to bytes

    // Erase types (7–10)
    // Pick the smallest valid erase size
    uint32_t EraseSize = 0;
    uint8_t  EraseOpCode = 0;

    for(int i = 0; i < 4; i++)
    {
        uint8_t EraseSizeExponent = BasicFlashParameterTable[BFPT_ERASE_TYPE_1_SIZE_EXP_OFFSET + (i * 2)];
        uint8_t OpCode            = BasicFlashParameterTable[BFPT_ERASE_TYPE_1_OPCODE_OFFSET   + (i * 2)];

        if((OpCode != 0x00) && (OpCode != 0xFF))
        {
            uint32_t Size = 1 << EraseSizeExponent;

            if((EraseSize == 0) || (Size < EraseSize))
            {
                EraseSize   = Size;
                EraseOpCode = OpCode;
            }
        }
    }

    if(EraseSize == 0)
    {
        return SYS_FAIL; // No valid erase type found
    }

    // Chip Erase opcode (uint32_t 8)
    uint8_t ChipEraseOpCode = BasicFlashParameterTable[BFPT_CHIP_ERASE_OPCODE_OFFSET];
    bool    HasChipErase    = ((ChipEraseOpCode != 0x00) && (ChipEraseOpCode != 0xFF));

    // Page size (uint32_t 11)
    uint16_t PageSize = (BasicFlashParameterTable[BFPT_PAGE_SIZE_OFFSET_MSB] << 8) |
                         BasicFlashParameterTable[BFPT_PAGE_SIZE_OFFSET_LSB];

    if(PageSize == 0)
    {
        PageSize = 256; // Fallback for older devices
    }

    // Fill MemoryInfo structure
    m_MemoryInfo.PageSize          = PageSize;
    m_MemoryInfo.NumberOfPages     = FlashDensity / PageSize;
    m_MemoryInfo.SectorSize        = EraseSize;
    m_MemoryInfo.SectorEraseSize   = EraseSize;
    m_MemoryInfo.NumberOfSectors   = FlashDensity / EraseSize;
    m_MemoryInfo.PageEraseSize     = 0; // Flash does not support page erase
    m_MemoryInfo.ChipEraseOpCode   = SerialMemoryCmd_e(ChipEraseOpCode);
    m_MemoryInfo.SectorEraseOpCode = SerialMemoryCmd_e(EraseOpCode);
    m_MemoryInfo.SupportOptions = MEM_OPT_READ_ID | MEM_OPT_FAST_READ | MEM_OPT_SFDP | MEM_OPT_SECTOR_ERASE;

    if(HasChipErase == true)
    {
        m_MemoryInfo.SupportOptions |= MEM_OPT_BULK_ERASE;
    }

    // Addressing mode (0)
    uint8_t AddressingMode = (BasicFlashParameterTable[BFPT_ADDRESSING_MODE_OFFSET] >> 1) & 0x03;

    if((AddressingMode == 1) || (AddressingMode == 2))
    {
        m_MemoryInfo.SupportOptions |= MEM_OPT_4_BYTES_ADDR;
    }

    // Read the ID
    m_MemoryInfo.MemoryID = ReadID();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
