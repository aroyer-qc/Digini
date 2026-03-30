//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_serial_flash.cpp
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

#define SPI_FLASH_GLOBAL
#include "./lib_digini.h"
#undef  SPI_FLASH_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define FLASH_SFDP_PAGE_SIZE                256             // At this time page are always 256 bytes
#define FLASH_SFDP_SECTOR_SIZE              4096            // At this time sector are always 4096 bytes

#define	FLASH_WIP_FLAG		                uint8_t(0x01)   // Write in progress Flag

#define FLASH_SST_DEVICES                   uint32_t(0xBF0000)

#define FLASH_WAIT_LOOP_DELAY               2
#define FLASH_WAIT_LOOP_RETRY               15

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
#define SFDP_TABLE_DENSITY_0_OFFSET         4   // Density bits [7:0]
#define SFDP_TABLE_DENSITY_1_OFFSET         5
#define SFDP_TABLE_DENSITY_2_OFFSET         6
#define SFDP_TABLE_DENSITY_3_OFFSET         7   // Density bits [31:24]
#define SFDP_SIGNATURE                      "SFDP"
#define SFDP_SIGNATURE_SIZE                 4
#define SFDP_BYTE_SIZE                      8
#define SFDP_HEADER_SIZE                    16
#define SFDP_BASIC_FLASH_PARAMETER_TABLE    8

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

#if (FLASH_USE_AUTO_DETECT_FLASH != DEF_ENABLED)
const FlashInfo_t SPI_SerialFLashDriver::m_FlashInfoList[NUMBER_OF_FLASH] =
{
    SERIAL_FLASH_DEF(EXPAND_X_SERIAL_FLASH_AS_CLASS_CONST)
};
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  SPI_Driver*   pSPI          Pointer to SPI driver instance
//                  IO_ID_e       ChipSelect    Chip-select line for the flash device
//
//                  (Optional) FlashList_e Flash
//                      Used when auto-detection is disabled to select a flash entry
//                      from the static flash information table.
//
//   Return Value:  SystemState_e       SYS_READY on success, or SYS_ERROR if the flash device is
//                                      unsupported or initialization fails.
//
//   Description:   Initializes the serial flash driver. The function stores the SPI driver
//                  pointer, configures the chip-select line, initializes the SPI interface,
//                  reads the JEDEC ID, performs device-specific unlock sequences (SST),
//                  and either auto-detects flash parameters using SFDP or loads predefined
//                  flash information from the static table.
//
//-------------------------------------------------------------------------------------------------
#if (FLASH_USE_AUTO_DETECT_FLASH == DEF_ENABLED)
SystemState_e SPI_SerialFLashDriver::Initialize(SPI_Driver* pSPI, IO_ID_e ChipSelect)
#else
SystemState_e SPI_SerialFLashDriver::Initialize(SPI_Driver* pSPI, FlashList_e Flash, IO_ID_e ChipSelect)
#endif
{
    uint32_t FlashID;

    m_pSPI       = pSPI;
    m_ChipSelect = ChipSelect;

    m_pSPI->Initialize();

    FlashID = ReadID();

    if((FlashID & FLASH_SST_DEVICES) == FLASH_SST_DEVICES)          // For SST devices unlock is needed
    {
        m_pSPI->LockToDevice(m_ChipSelect, false);
        WriteEnable();
        m_pSPI->SelectChip(m_ChipSelect);
        m_pSPI->Write(FLASH_CMD_UNLOCK);
        m_pSPI->DeSelectChip(m_ChipSelect);
        m_pSPI->UnlockFromDevice(m_ChipSelect, false);
    }

  #if (FLASH_USE_AUTO_DETECT_FLASH == DEF_ENABLED)
    uint32_t FlashDensity = ReadSFDP_Density();

    if(FlashDensity > 0)
    {
        m_FlashInfo.PageSize        = FLASH_PAGE_SIZE;
        m_FlashInfo.NumberOfPages   = FlashDensity / FLASH_SFDP_PAGE_SIZE;
        m_FlashInfo.PageEraseSize   = 0;
        m_FlashInfo.SectorSize      = FLASH_SFDP_SECTOR_SIZE;
        m_FlashInfo.SectorEraseSize = FLASH_SFDP_SECTOR_SIZE;
        m_FlashInfo.NbSectors       = FlashDensity / FLASH_SFDP_SECTOR_SIZE;
        m_FlashInfo.FlashID         = FlashID;
    }
    else
    {
        // Error... Flash not supported.
        m_FlashInfo.PageSize  = 0;
        m_FlashInfo.NbSectors = 0;                  // Unsupported Flash
        m_FlashInfo.FlashID   = FlashID;
        return SYS_ERROR;
    }
  #else
    memcpy(&m_FlashInfo, &m_FlashInfoList[Flash], sizeof(FlashInfo_t));
  #endif

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: ReadID
//
//   Parameter(s):  None
//   Return value:  uint32_t       24-bit JEDEC device ID
//
//   Description:   Sends the READ ID command to the serial flash device and retrieves the 3 bytes
//                  manufacturer/device identifier. The command is issued over the SPI interface
//                  and the returned bytes are combined into a single 24-bit value.
//
//-------------------------------------------------------------------------------------------------
uint32_t SPI_SerialFLashDriver::ReadID(void)
{
    uint8_t     Buffer[3] = {0x00, 0x00, 0x00};
    uint8_t     Command   = FLASH_CMD_READ_ID;

    // Send "FLASH_CMD_READ_ID" instruction and read ID
    m_pSPI->Transfer(&Command, 1, Buffer, 3, m_ChipSelect);
	return (uint32_t(Buffer[0]) << 16) | (uint32_t(Buffer[1]) << 8) | uint32_t(Buffer[2]);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SendCommandAndAddress
//
//   Parameter(s):  Command     - Flash command opcode
//                  Address     - 24-bit or 32-bit flash address to append
//
//   Return value:  SystemState_e
//
//   Description:   Combines the command byte and the address into a single 32-bit
//                  big-endian sequence, then transmits the 4 bytes over SPI.
//                  The address is byte-swapped to match the flash device’s MSB-first
//                  transmission order, and the command is inserted into the LSB.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::SendCommandAndAddress(SerialFlashCmd_e Command, uint32_t Address)
{
    uint32_t CmdAndAddress;

    LIB_uint32_t_Swap(&Address);				// 0x00123456 => 0x56341200
   	CmdAndAddress  = Address;
    CmdAndAddress |= Command;
    return m_pSPI->Write(((uint8_t*)&CmdAndAddress), 4);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Read
//
//   Parameter(s):  void*          pBuffer        Destination buffer
//                  uint32_t       Address        Flash address to read from
//                  size_t         Length         Number of bytes to read
//
//   Return Value:  SystemState_e  SYS_READY on success, or an error code if the device is busy,
//                                 communication fails, or the SPI interface cannot be
//                                 locked/unlocked.
//
//   Description:   Reads a block of data from the serial flash device. The function locks the
//                  SPI interface, waits for any ongoing program/erase operation to complete,
//                  sends the FAST READ command and address, reads the requested number of bytes,
//                  and then unlocks the SPI interface. Any error encountered is propagated.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::Read(void* pBuffer, uint32_t Address, size_t Length)
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
        State = SendCommandAndAddress(FLASH_CMD_FAST_READ, Address);

        if(State == SYS_READY)
        {
            State = m_pSPI->Read((uint8_t*)pBuffer, Length);
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
//                  uint32_t       Address       Flash address to write to
//                  size_t         Length        Number of bytes to write
//
//   Return Value:  SystemState_e  SYS_READY on success, or an error code if the device is busy,
//                                parameters are invalid, or SPI communication fails.
//
//   Description:   Writes a block of data to the serial flash device. The function locks the
//                  SPI interface, aligns the first write to the current page boundary, and then
//                  programs data page-by-page using WritePage(). The write operation continues
//                  until all bytes are written or an error occurs. The SPI interface is unlocked
//                  before returning.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::Write(const void* pBuffer, uint32_t Address, size_t Length)
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

    uint8_t* pData = (uint8_t*)pBuffer;
    uint32_t Size  = Address % m_FlashInfo.PageSize;   

    if(Size != 0)
    {
        Size     = m_FlashInfo.PageSize - Size;                                         // Calculate the size for this page data
        State    = WritePage(pData, Address, Size);                                     // Write data in page
        Length  -= Size;						                                        // Calculate the remaining bytes to write.
        pData   += Size;						                                        // Adjust the pointer to the next block of data to write.
        Address += Size;						                                        // Adjust the next address to write to.
    }

    while((Length > 0) && (State == SYS_READY))
    {
        Size     = (Length < m_FlashInfo.PageSize) ? Length : m_FlashInfo.PageSize;     // Calculate the size for this page data
        State    = WritePage(pData, Address, Size);                                     // Write data in page
        Length  -= Size;						                                        // Calculate the remaining bytes to write.
        pData   += Size;						                                        // Adjust the pointer to the next block of data to write.
        Address += Size;						                                        // Adjust the next address to write to.
    }

    SystemState_e UnlockState = m_pSPI->UnlockFromDevice(m_ChipSelect, false);

    if(UnlockState != SYS_READY)
    {
        return UnlockState;
    }

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WritePage
//
//   Parameter(s):  void*          pBuffer        Source buffer
//                  uint32_t       Address        Flash address to program
//                  size_t         Length         Number of bytes to write (within a single page)
//
//   Return Value:  SystemState_e  SYS_READY on success, or an error code if the device is busy or
//                                 communication fails.
//
//   Description:   Issues a PAGE PROGRAM operation to the serial flash device. The function
//                  assumes the SPI interface is already locked and that the write does not
//                  cross a page boundary. It waits for any ongoing program/erase operation
//                  to complete, enables writing, sends the PAGE PROGRAM command and address,
//                  and writes the specified number of bytes. Completion polling after the
//                  program operation is handled by the caller.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::WritePage(void* pBuffer, uint32_t Address, size_t Length)
{
    SystemState_e State;

    State = WaitForEndWrite();                                                  // Ensure device is idle

    if(State == SYS_READY)
    {
        State = WriteEnable();                                                  // Enable write

        if(State == SYS_READY)
        {
            m_pSPI->SelectChip(m_ChipSelect);

            State = SendCommandAndAddress(FLASH_CMD_PAGE_PROGRAM, Address);     // Issue PAGE PROGRAM command

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
//   Return Value:  SystemState_e       SYS_READY or SYS_ERROR
//
//   Description:   Issues a bulk erase command to the flash device. The function locks the SPI
//                  interface (without controlling chip select), enables writing, asserts chip
//                  select, transmits the bulk erase opcode, then releases the device lock. Any
//                  communication or sequencing error is propagated through the returned state.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::BulkErase(void)
{
    SystemState_e State;

    State = m_pSPI->LockToDevice(m_ChipSelect, false);                          // Lock SPI access (BulkErase controls CS)

    if(State != SYS_READY)
    {
        return State;
    }

    State = WriteEnable();                                                      // Enable write

    if(State == SYS_READY)
    {
        m_pSPI->SelectChip(m_ChipSelect);
        State = m_pSPI->Write(uint8_t(FLASH_CMD_BULK_ERASE));                   // Send bulk erase command
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
//   Description:   Sends an erase command to the serial flash device. The function locks the SPI
//                  interface to the target device, asserts chip select, transmits the erase opcode
//                  and address, then releases the device lock. Any communication error is
//                  propagated through the returned state.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::EraseSector(uint32_t Address)
{
    SystemState_e State;

    State = m_pSPI->LockToDevice(m_ChipSelect);                                 // Lock SPI access to this device

    if(State != SYS_READY)
    {
        return State;
    }

    State = SendCommandAndAddress(FLASH_CMD_CHIP_ERASE, Address);
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
//   Description:   Polls the flash device's status register until the Write-In-Progress (WIP)
//                  bit clears or a retry timeout occurs. The function asserts chip select,
//                  issues the READ STATUS command, reads the status byte, and repeats until
//                  the write cycle completes. Any SPI communication error or timeout is
//                  propagated through the returned state.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::WaitForEndWrite(void)
{
	SystemState_e State;
	uint8_t	      Command;
	uint8_t	      ReadValue;
	int           Retry;

    Retry   = FLASH_WAIT_LOOP_RETRY;
    Command = FLASH_CMD_READ_STATUS_REGISTER;

	do
	{
        nOS_Sleep(FLASH_WAIT_LOOP_DELAY); 							            // The write page take 11 ms typic. 25ms maximum.
        Retry--;
		m_pSPI->SelectChip(m_ChipSelect);
		State = m_pSPI->Transfer(&Command, 1, &ReadValue, 1);		            // Send "Read Status" instruction
		m_pSPI->DeSelectChip(m_ChipSelect);
	}
	while(((ReadValue & FLASH_WIP_FLAG) == FLASH_WIP_FLAG) && (Retry > 0)); 	// Write in progress

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
//   Description:   Sends the Write Enable (WREN) command to the flash device. The function
//                  asserts chip select, transmits the WREN opcode, and then releases chip select.
//                  This sets the Write Enable Latch (WEL), allowing subsequent program or erase
//                  operations to be accepted by the device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::WriteEnable(void)
{
    m_pSPI->SelectChip(m_ChipSelect);
	SystemState_e State = m_pSPI->Write(FLASH_CMD_WRITE_ENABLE);        // Send "Write Enable" instruction
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
//   Description:   Sends the Write Disable (WRDI) command to the flash device. The function
//                  asserts chip select, transmits the WRDI opcode, and then releases chip select.
//                  This clears the Write Enable Latch (WEL) and prevents any program or erase
//                  operations until WriteEnable() is issued again.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLashDriver::WriteDisable(void)
{
    m_pSPI->SelectChip(m_ChipSelect);
	SystemState_e State = m_pSPI->Write(FLASH_CMD_WRITE_DISABLE);       // Send "Write Disable" instruction
    m_pSPI->DeSelectChip(m_ChipSelect);

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SFDP_Read
//
//   Parameter(s):  uint32_t    NumberOfByteToRead     Number of bytes to read from the SFDP table
//                  uint32_t    Address                Starting SFDP address
//                  uint8_t*    pBuffer                Destination buffer
//
//   Return Value:  SystemState_e                      SYS_READY or SYS_ERROR
//
//   Description:   Reads data from the Serial Flash Discoverable Parameters (SFDP) table. The
//                  function locks the SPI interface, sends the SFDP read command and address,
//                  performs a dummy byte transfer as required by the SFDP protocol, reads the
//                  requested number of bytes, then unlocks the SPI interface.
//
//-------------------------------------------------------------------------------------------------
#if (FLASH_USE_AUTO_DETECT_FLASH == DEF_ENABLED)
SystemState_e SPI_SerialFLashDriver::SFDP_Read(uint32_t NumberOfByteToRead, uint32_t Address, uint8_t *pBuffer)
{
    SystemState_e State;
	uint8_t Dummy = 0;

    State = m_pSPI->LockToDevice(m_ChipSelect);                             // Let it handle the  CS

    if(State != SYS_READY)
    {
        return State;
    }

    State = SendCommandAndAddress(FLASH_CMD_READ_SFPD, Address);

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
//   Function:      ReadSFDP_Density
//
//   Parameter(s):  None
//
//   Return Value:  uint32_t    Device density in bytes, as reported by the SFDP Basic Flash
//                              Parameter Table. Returns 0 if the SFDP header, parameter header,
//                              erase type, address size, or density fields are invalid.
//
//   Description:   Reads the SFDP (Serial Flash Discoverable Parameters) header and the first
//                  DWORDs of the Basic Flash Parameter Table to extract the device density. The
//                  SFDP specification encodes density as (number_of_bits - 1), so the function
//                  reconstructs the true size and converts it to bytes. Only devices supporting
//                  4‑KB erase and 3‑byte addressing are accepted.
//
//-------------------------------------------------------------------------------------------------
#if (FLASH_USE_AUTO_DETECT_FLASH == DEF_ENABLED)
uint32_t TSPI_SerialFLashDriver::ReadSFDP_Density(void)
{
    // Check JEDEC serial flash discoverable parameters for device specific info
    uint8_t Header[SFDP_HEADER_SIZE];

    SFDPRead(SFDP_HEADER_SIZE, 0x0, Header);

    // Verify SFDP signature for sanity
    // Also check that major/minor version is acceptable
    if((memcmp(&Header[0], SFDP_SIGNATURE, SFDP_SIGNATURE_SIZE) != 0) && (Header[SFDP_MAJOR_REVISION_OFFSET] != 1))
    {
        return 0;
    }

    // The SFDP spec indicates the standard table is always at offset 0
    // in the parameter headers, we check just to be safe
    if((Header[SFDP_PARAM_ID_LSB_OFFSET] != 0x00) ||
       (Header[SFDP_PARAM_ID_MSB_OFFSET] != 0xFF) ||
       (Header[SFDP_PARAM_MAJOR_OFFSET]  != 1))
    {
        return 0;
    }

    // Parameter table pointer, spi commands are BE, SFDP is LE,
    // also sfdp command expects extra read wait byte
    uint8_t Table[SFDP_BASIC_FLASH_PARAMETER_TABLE];
    uint32_t TableAddress = (Header[SFDP_PARAM_TABLE_PTR_2] << 16) |
                            (Header[SFDP_PARAM_TABLE_PTR_1] << 8)  |
                             Header[SFDP_PARAM_TABLE_PTR_0];

    SFDP_Read(SFDP_BASIC_FLASH_PARAMETER_TABLE, TableAddress, Table);

    // Check erase size, currently only supports 4 KB
    if(((Table[SFDP_TABLE_ERASE_SUPPORT_OFFSET] & 0x03) != 0x01) ||
        (Table[SFDP_TABLE_ERASE_OPCODE_OFFSET] != 0x20))
    {
        return 0;
    }

    // Check address size, currently only supports 3 bytes addresses
    if (((Table[SFDP_TABLE_ADDRESS_BYTES_OFFSET] & 0x04) != 0) ||
        ((Table[SFDP_TABLE_DENSITY_3_OFFSET] & 0x80) != 0))
	{
        return 0;
    }

    // Get device density, stored as size in bits - 1
    uint32_t Density = ((Table[SFDP_TABLE_DENSITY_3_OFFSET] << 24) |
                        (Table[SFDP_TABLE_DENSITY_2_OFFSET] << 16) |
                        (Table[SFDP_TABLE_DENSITY_1_OFFSET] << 8 ) |
                        (Table[SFDP_TABLE_DENSITY_0_OFFSET] << 0 ));

    return (Density + 1) / SFDP_BYTE_SIZE;
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
