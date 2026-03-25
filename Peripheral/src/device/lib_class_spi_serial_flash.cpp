//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_SerialFlash.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#define FLASH_SFDP_PAGE_SIZE        256             // At this time page are always 256 bytes
#define FLASH_SFDP_SECTOR_SIZE      4096            // At this time sector are always 4096 bytes

#define	FLASH_WIP_FLAG		        uint8_t(0x01)   // Write in progress Flag

#define FLASH_SST_DEVICES           uint32_t(0xBF0000)

#define FLASH_WAIT_END_WRITE_DELAY  10

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

#if (FLASH_USE_AUTO_DETECT_FLASH != DEF_ENABLED)        
const FlashInfo_t SPI_SerialFLash_Driver::m_FlashInfoList[NUMBER_OF_FLASH] =
{
    X_FLASH(EXPAND_X_SERIAL_FLASH_AS_CLASS_CONST)
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   Initialize
//
//   Parameter(s):  SPI_Driver* pSPI
//                  IO_ID_e     ChipSelectIO
//                  IO_ID_e     Reset
//                  IO_ID_e     WriteProtect
//
//   Description:   Get the pointer for the init structure
//
//-------------------------------------------------------------------------------------------------
#if (FLASH_USE_AUTO_DETECT_FLASH == DEF_ENABLED)
SystemState_e SPI_SerialFLash_Driver::Initialize(SPI_Driver* pSPI, IO_ID_e ChipSelect)
#else
SystemState_e SPI_SerialFLash_Driver::Initialize(SPI_Driver* pSPI, FlashList_e Flash, IO_ID_e ChipSelect);
#endif
{
    uint32_t FlashID;
    uint32_t FlashDensity;
    
    m_pSPI             = pSPI;
    m_ChipSelect       = ChipSelect;

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
    FlashDensity = ReadSFDP_Density();

    if(FlashDensity > 0)
    {
        m_FlashInfo.PageSize        = FLASH_PAGE_SIZE;
        m_FlashInfo.NumberOfPages   = FlashDensity / FLASH_SFDP_PAGE_SIZE;
        m_FlashInfo.PageEraseSize   = 0;
        m_FlashInfo.SectorSize      = FLASH_SFDP_SECTOR_SIZE;
        m_FlashInfo.SectorEraseSize = FLASH_SFDP_SECTOR_SIZE;
        m_FlashInfo.NbSectors       = FlashDensity / FLASH_SFDP_SECTOR_SIZE;
        m_FlashInfo.FlashID         = flashID;
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
uint32_t SPI_SerialFLash_Driver::ReadID(void)
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
//   Return value:  None
//
//   Description:   Combines the command byte and the address into a single 32-bit
//                  big-endian sequence, then transmits the 4 bytes over SPI.  
//                  The address is byte-swapped to match the flash device’s MSB-first
//                  transmission order, and the command is inserted into the LSB.
//
//-------------------------------------------------------------------------------------------------
void SPI_SerialFLash_Driver::SendCommandAndAddress(uint8_t Command, uint32_t Address)
{

    LIB_uint32_t_Swap(&Address);				// 0x00123456 => 0x56341200
   	CmdAndAddress  = Address;
    CmdAndAddress |= Command;
    m_pSPI->Write(((uint8_t*)&CmdAndAddress), 4);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Read
//
//   Parameter(s):  uint8_t*        pBuffer
//                  uint32_t        Sector
//                  uint8_t         Count
//   Return value:  DRESULT
//
//   Description:   Read block of data to Flash memory
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SPI_SerialFLash_Driver::Read(void* pBuffer, uint32_t Address, size_t Length)
{
    SystemState_e State;

    State = m_pSPI->LockToDevice(m_ChipSelect);
    
    if(State != SYS_READY)
    {
        return State;
    }
    
	WaitForEndWrite();
	
	if(Count > 0)
	{
		m_pSPI->SelectChip(m_ChipSelect);
		State = SendCommandAndAddress(CMD_READ, Address);
        
        if(State == SYS_READY)
        {
            State = m_pSPI->Read((uint8_t*)Buffer, Length);
        }
        
		m_pSPI->DeSelectChip(m_ChipSelect);
	}

    State = m_pSPI->UnlockFromDevice(m_ChipSelect);

	return State;

    
    
    
    
    
    
    uint8_t Buffer[6];
    DRESULT Result;

    Result                 = RES_ERROR;
    Buffer[0]              = SPI_FLASH_AT45DB641_STREAM_READ_HS;

    *(uint32_t*)&Buffer[1] = ((Sector * 2) * SPI_FLASH_PAGE_SIZE) << 8;
    LIB_uint32_t_Swap((uint32_t*)&Buffer[1]);
    Buffer[5]              = 0;

    if(m_pSPI->LockToDevice(m_ChipSelect) != READY)                         return RES_ERROR;
    ChipSelect(CS_ENABLE);
    if(m_pSPI->Write(&Buffer[0], 6) != READY)                               goto ExitRead;
    if(m_pSPI->Read(pBuffer, (Count * 2) * SPI_FLASH_PAGE_SIZE) != READY)   goto ExitRead;

    Result  = RES_OK;

  ExitRead:

    ChipSelect(CS_DISABLE);
    if(m_pSPI->UnlockFromDevice(m_ChipSelect) != READY)                        return RES_ERROR;
    return Result;
}


//-------------------------------------------------------------------------------------------------
//
//   Function name: Write
//
//   Parameter(s):  const uint8_t*  pBuffer
//                  uint32_t        Sector
//                  uint8_t         Count
//   Return value:  DRESULT
//
//   Description:   Write block of data to Flash memory
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT SPI_SerialFLash_Driver::Write(const uint8_t* pBuffer, uint32_t Sector, uint8_t Count)
{
    uint32_t Command;
    uint32_t Page;
    uint32_t TotalSize;
    DRESULT  Result;

    Result    = RES_ERROR;
    TotalSize = uint32_t(Count) * 512;
    Page      = Sector * 2;                                                                                 // Adjust for real page offset

    if(m_pSPI->LockToDevice(m_ChipSelect) != READY)                                        return RES_ERROR;
    WriteProtect(WRITE_ALLOWED);

    for(uint32_t i = 0; i < TotalSize; i += 512)
    {
        ChipSelect(CS_ENABLE);
        Command = SPI_FLASH_AT45DB641_WRITE_1_TO_MEMORY | (Page << 8);
        if(m_pSPI->Write(Command) != READY)                                             goto ExitWrite;     // Start at 0 in buffer also erase and program memory
        if(m_pSPI->Write(&pBuffer[0], size_t(256)) != READY)                            goto ExitWrite;
        if(WaitReadyAndChipSelect(CS_ENABLE) != READY)                                  goto ExitWrite;
        if(GetLastChipStatus() & SPI_FLASH_AT45DB641_ERROR_ERASE_PROGRAM_MASK)          goto ExitWrite;     // Exit on write error

        Page++;                                                                                             // Next page

        ChipSelect(CS_ENABLE);
        Command = SPI_FLASH_AT45DB641_WRITE_2_TO_MEMORY | (Page << 8);
        if(m_pSPI->Write(Command) != READY)                                             goto ExitWrite;     // Start at 0 in buffer also erase and program memory
        if(m_pSPI->Write(&pBuffer[256], size_t(256)) != READY)                          goto ExitWrite;
        if(WaitReadyAndChipSelect(CS_ENABLE) != READY)                                  goto ExitWrite;
        if(GetLastChipStatus() & SPI_FLASH_AT45DB641_ERROR_ERASE_PROGRAM_MASK)          goto ExitWrite;     // Exit on write error

        Page++;                                                                                             // Next page
    }

    Result = RES_OK;

  ExitWrite:

    ChipSelect(CS_DISABLE);
    WriteProtect(WRITE_PROTECTED);
    if(m_pSPI->UnlockFromDevice(m_ChipSelect) != READY)                                    return RES_ERROR;
    return Result;
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
SystemState_e SPI_SerialFLash_Driver::EraseSector(uint32_t Address)
{
    State = m_pSPI->LockToDevice(m_ChipSelect);                                 // Lock SPI access to this device

    if(State != SYS_READY)
    {
        return State;
    }
    
    SystemState_e PriorityState = SendCommandAndAddress(FLASH_CMD_CHIP_ERASE, Address);

    State = m_pSPI->UnlockFromDevice(m_ChipSelect);                             // Unlock SPI access

    if(State != SYS_READY)
    {
        return State;
    }
    
    // If Write() failed, propagate that error
    return (PriorityState != SYS_READY) ? PriorityState : SYS_READY;
}

//-------------------------------------------------------------------------------------------------

SystemState_e SPI_SerialFLash_Driver::WaitForEndWrite(void)
{
	uint8_t	Command = FLASH_CMD_READ_STATUS_REGISTER;
	uint8_t	ReadValue;
	bool    noError = false;
	int     timeout;
  
    timeout = 150; 		// 25 * 2ms = 50ms timeout.

	do
	{
        nOS_Sleep(FLASH_WAIT_END_WRITE_DELAY); 							// The write page take 11 ms typic. 25ms maximum.

		timeout--;

		m_pSPI->SelectChip(m_ChipSelect);
		m_pSPI->Transfer(&Command, 1, &ReadValue, 1);		        	// Send "Read Status" instruction
		m_pSPI->DeSelectChip(m_ChipSelect);
	} 
	while(((ReadValue & WIP_FLAG) == WIP_FLAG) && (timeout > 0)); 		// Write in progress
	
	noError = ((timeout == 0) ? false : true);

	return noError;
}


//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
