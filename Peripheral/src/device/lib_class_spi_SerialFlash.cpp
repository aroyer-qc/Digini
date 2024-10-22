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
//
//   Class: CSPI_FLash
//
//
//   Description:   Class to handle SPI Flash
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   SPI_SerialFLash_Driver
//
//   Parameter(s):  SPI_Driver* pSPI
//                  IO_ID_e     ChipSelectIO
//                  IO_ID_e     Reset
//                  IO_ID_e     WriteProtect
//
//   Description:   Get the pointer for the init structure
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SPI_SerialFLash_Driver::SPI_SerialFLash_Driver(SPI_Driver* pSPI, IO_ID_e ChipSelect, IO_ID_e Reset, IO_ID_e WriteProtect)
{
    m_pSPI             = pSPI;
    m_pDevice          = &SPI_DeviceInfo[SPI_DEVICE_FLASH_AT45DB64];

    m_ChipSelect       = ChipSelect;
    m_ChipReset        = Reset;
    m_ChipWriteProtect = WriteProtect;
    m_Status           = STA_NOINIT;
    m_ChipStatus       = 0;

    IO_PinInit(m_ChipSelect);
    IO_PinInit(m_ChipReset);
    IO_PinInit(m_ChipWriteProtect);

    if(m_pSPI->LockToDevice(m_ChipSelect) != READY)                                 return;
    WriteProtect(WRITE_ALLOWED);
    ChipSelect(CS_ENABLE);

    if(m_pSPI->Write(SPI_FLASH_AT45DB641_SOFTWARE_RESET) != READY)                  goto ExitSPI_Constructor;
    if(WaitReadyAndChipSelect(CS_ENABLE) != READY)                                  goto ExitSPI_Constructor;

    // Change page size to 256 bytes per page
    if(m_pSPI->Write(SPI_FLASH_AT45DB641_CHANGE_PAGE_SIZE_TO_256) != READY)         goto ExitSPI_Constructor;
    if(WaitReadyAndChipSelect(CS_ENABLE) != READY)                                  goto ExitSPI_Constructor;

    // Disable Sector protection if enable
    if(GetLastChipStatus() & SPI_FLASH_AT45DB641_SECTOR_PROTECTION_MASK)
    {
        if(m_pSPI->Write(SPI_FLASH_AT45DB641_DISABLE_SECTOR_PROTECTION) != READY)   goto ExitSPI_Constructor;
        if(WaitReadyAndChipSelect(CS_ENABLE) != READY)                              goto ExitSPI_Constructor;
    }

    if(GetLastChipStatus() & SPI_FLASH_AT45DB641_SECTOR_LOCKDOWN_MASK)
    {
        if(m_pSPI->Write(SPI_FLASH_AT45DB641_DISABLE_SECTOR_LOCKDOWN) != READY)     goto ExitSPI_Constructor;
        if(WaitReadyAndChipSelect(CS_ENABLE) != READY)                              goto ExitSPI_Constructor;
    }

    ExitSPI_Constructor:

    ChipSelect(CS_DISABLE);
    WriteProtect(WRITE_PROTECTED);
    m_pSPI->UnlockFromDevice(m_ChipSelect);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Initialize SD Card
//
//   Note(s):       Check if status of the SPI FLash is ready
//
//-------------------------------------------------------------------------------------------------
DSTATUS SPI_SerialFLash_Driver::Initialize()
{
    DSTATUS  Result            = STA_NOINIT;
    uint16_t StatusRegister;

    if(m_pSPI->LockToDevice(m_ChipSelect) != READY)                                             return STA_NOINIT;
    if(m_pSPI->Write(SPI_FLASH_AT45DB641_STATUS) != READY)                                      goto ExitInitialize;

    m_TimeOut = SPI_FLASH_RETRY_TIMER;
    do
    {
        if(m_pSPI->Read(&StatusRegister) != READY)                                              goto ExitInitialize;
    }
    while((m_TimeOut != 0) && ((StatusRegister & SPI_FLASH_AT45DB641_BUSY_MASK) == 0));

    m_ChipStatus = StatusRegister;

//    if((StatusRegister & SPI_FLASH_AT45DB641_INIT_MASK) != SPI_FLASH_AT45DB641_INIT_READY_MASK) goto ExitInitialize;

    m_Status = STA_OK;
    Result   = STA_OK;

    ExitInitialize:

    ChipSelect(CS_DISABLE);
    WriteProtect(WRITE_PROTECTED);
    if(m_pSPI->UnlockFromDevice(m_ChipSelect) != READY)                                            return STA_NOINIT;

    return Result;
}


//-------------------------------------------------------------------------------------------------
//
//   Function name: Status
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Return status
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DSTATUS SPI_SerialFLash_Driver::Status()
{
    return STA_OK;
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
DRESULT SPI_SerialFLash_Driver::Read(uint8_t* pBuffer, uint32_t Sector, uint8_t Count)
{
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
#if _USE_WRITE == 1
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
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name: IO_Control
//
//   Parameter(s):  uint8_t    Control        Control code
//                  void*      pBuffer        Buffer to send/receive control data
//   Return value:  DRESULT
//
//   Description:   Control
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
#if _USE_IOCTL == 1
DRESULT SPI_SerialFLash_Driver::IO_Control(uint8_t Control, void *pBuffer)
{
    DRESULT     Result;
    uint8_t*    pPtr = (uint8_t*)pBuffer;

    if(m_Status & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    Result = RES_ERROR;

    switch(Control)
    {
        case CTRL_SYNC:                                                             // Make sure that all data has been written on the media
            WaitReadyAndChipSelect(CS_DISABLE);
            Result = RES_OK;
            break;

        case GET_SECTOR_COUNT:                                                      // Get number of sectors on the disk (uint32_t)
            *(uint32_t*)pPtr = SPI_FLASH_PAGE_COUNT / 2;                            // Page size are 256 bytes. FatFs need minimum 512 bytes: so SPI_FLASH_PAGE_COUNT / 2
            Result = RES_OK;
            break;

        case GET_SECTOR_SIZE:                                                       // Get sectors on the disk (uint16_t)
            *(uint16_t*)pPtr = SPI_FLASH_PAGE_SIZE * 2;                             // Page size are 256 bytes. FatFs need minimum 512 bytes
            Result = RES_OK;
            break;

        case GET_BLOCK_SIZE:                                                        // Get erase block size in unit of sectors (uint32_t)
            *(uint32_t*)pPtr = 1;
            Result = RES_OK;
            break;

        case CTRL_ERASE_SECTOR:                                                     // Erase a block of sectors
            uint32_t StartSector;
            uint32_t EndSector;
            uint32_t Count;
            uint32_t Command;

            StartSector = ((uint32_t*)pPtr)[0] * 2;
            EndSector   = ((uint32_t*)pPtr)[1] * 2;

            for(Count = StartSector; Count < (EndSector + 1);)
            {
                if(((EndSector - StartSector) + 1) == SPI_FLASH_PAGE_COUNT)         // Do we erase the entire chip
                {
                    Command = SPI_FLASH_AT45DB641_CHIP_ERASE;
                    Count = (EndSector + 1);
                }
                else if(Count < ((EndSector + 1) - SPI_FLASH_PAGE_IN_BLOCK))        // Do we erase bigger than a block size
                {
                    Command = SPI_FLASH_AT45DB641_BLOCK_ERASE + (Count << 11);
                    Count  += SPI_FLASH_PAGE_IN_BLOCK;
                }
                else                                                                // Erase by page size
                {
                    Command = SPI_FLASH_AT45DB641_PAGE_ERASE + (Count << 8);
                    Count++;
                }

                if(Erase(Command) != READY)         return RES_ERROR;
            }

            break;

        default:
            Result = RES_PARERR;
    }

    return Result;
}
#endif


//-------------------------------------------------------------------------------------------------
//
//   Function name: ChipSelect
//
//   Parameter(s):  eChipSelect Select
//   Return value:  None
//
//   Description:   Perform requested action on chip select
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SPI_SerialFLash_Driver::ChipSelect(eChipSelect Select)
{
   #if SPI_FLASH_USE_SOFTWARE_CHIP_SELECT_CONTROL == 1

    if((Select == CS_DISABLE) || (Select == CS_PULSE_HIGH))
    {
        IO_Output(m_ChipSelect, IO_RESET);
    }
    if((Select == CS_ENABLE) || (Select == CS_PULSE_HIGH))
    {
        IO_Output(m_ChipSelect, IO_SET);
    }
   #else
    VAR_UNUSED(Select);
   #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WaitReady
//
//   Parameter(s):  eChipSelect Select
//   Return Value:  eSystemState
//
//   Description:   Check by ready RDY/BUSY bit in status register of the SPI device
//
//   Note(s):       1. Always use after a command
//                  2. Select or deselect the chip after WaitReady
//
//-------------------------------------------------------------------------------------------------
eSystemState SPI_SerialFLash_Driver::WaitReadyAndChipSelect(eChipSelect Select)
{
    eSystemState State;
    uint16_t     StatusRegister;

    m_TimeOut = 0;

    ChipSelect(CS_PULSE_HIGH);

    if((State = m_pSPI->Write(SPI_FLASH_AT45DB641_STATUS)) != READY)    goto WaitExit;
    m_TimeOut = SPI_FLASH_RETRY_TIMER;
    do
    {
        if((State = m_pSPI->Read(&StatusRegister)) != READY)            goto WaitExit;
    }
    while((m_TimeOut != 0) && ((StatusRegister & SPI_FLASH_AT45DB641_BUSY_MASK) == 0));

    m_ChipStatus = StatusRegister;

  WaitExit:

    ChipSelect(CS_DISABLE);                                                             // end of Command
    if(m_TimeOut == 0) State = TIME_OUT;                                                // Is this a TimeOut
    if((State == READY) && (Select == CS_ENABLE)) ChipSelect(CS_ENABLE);                // Reenable chip select if ask to and we are not in error

    return State;
}


//-------------------------------------------------------------------------------------------------
//
//   Function:      Erase
//
//   Parameter(s):  uint32_t Command
//   Return Value:  None
//
//   Description:   Send a commad to erase part of chip or entire chip
//
//-------------------------------------------------------------------------------------------------
eSystemState SPI_SerialFLash_Driver::Erase(uint32_t Command)
{
    eSystemState State;
    eSystemState PriorityState;

    if((State = m_pSPI->LockToDevice(m_ChipSelect)) != READY)                   return State;
    ChipSelect(CS_ENABLE);
    if((PriorityState = m_pSPI->Write(Command)) != READY)                       goto EraseExit;
    if((PriorityState = m_pSPI->WaitReady()) != READY)                          goto EraseExit;

  EraseExit:

    ChipSelect(CS_DISABLE);
    if((State = m_pSPI->UnlockFromDevice(m_ChipSelect)) != READY)               return State;
    if(PriorityState != READY) State = PriorityState;

    return State;
}


//-------------------------------------------------------------------------------------------------
//
//   Function:      TickHook
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   1mS tick function for the SD SPI
//
//   Note(s):       This function adjust itself for a period of SD_SPI_TICK_PERIOD
//
//-------------------------------------------------------------------------------------------------
void SPI_SerialFLash_Driver::TickHook()
{
    m_TickPeriod--;
    if(m_TickPeriod == 0)
    {
        m_TickPeriod = SPI_FLASH_TICK_PERIOD;

        if(m_TimeOut > 0)
        {
            m_TimeOut--;
        }
    }
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           IsOperational
//
//  Parameter(s):   none
//  Return:         bool            State
//
//  Description:    Checks if the SPI Flash is correctly configured and communicates correctly.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool SPI_SerialFLash_Driver::IsOperational()
{
    uint32_t ID;
    uint8_t  Command;

// WIP!!!!!

    Command = FLASH_CMD_READ_JEDEC_ID;

    if(m_pSPI->Transfer(&Command, 1, &ID, sizeof(uint32_t), m_ChipSelect) != SYS_READY)
    {
        return false;
    }

    if(ID != (uint32_t)/* TODO use config to check ID*/ ) // Return Error if the ID is not correct
    {
        return false;
    }
    
    Command = FLASH_CMD_READ_STATUS_REGISTER;

    if(m_pSPI->Transfer(&Command, 1, &m_ChipStatus, sizeof(uint16_t), m_ChipSelect) != SYS_READY)
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
