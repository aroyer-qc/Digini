//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_serial_memory.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MEM_OPT_READ_ID             (1 << 0)
#define MEM_OPT_SFDP                (1 << 1)
#define MEM_OPT_FAST_READ           (1 << 2)
#define MEM_OPT_PAGE_ERASE          (1 << 3)
#define MEM_OPT_SECTOR_ERASE        (1 << 4)
#define MEM_OPT_4_BYTES_ADDR        (1 << 5)
#define MEM_OPT_EEPROM_STYLE        (1 << 6)   // pas d’effacement


#define EXPAND_X_SERIAL_MEM_AS_ENUM(ENUM_ID, MEMORY, PAGE_SIZE, NB_OF_PAGE, PAGE_ERASE_SIZE, SECTOR_SIZE, SECTOR_ERASE_SIZE, OPTION) \
                                    ENUM_ID,

#define EXPAND_X_SERIAL_MEM_AS_CLASS_CONST(ENUM_ID, MEMORY_ID, PAGE_SIZE, NB_OF_PAGE, PAGE_ERASE_SIZE, SECTOR_SIZE, SECTOR_ERASE_SIZE, OPTION) \
                                                   {MEMORY_ID, PAGE_SIZE, NB_OF_PAGE, PAGE_ERASE_SIZE, SECTOR_SIZE, SECTOR_ERASE_SIZE, OPTION, ((NB_OF_PAGE * PAGE_SIZE) / SECTOR_SIZE)},

#define SERIAL_MEMORY_DEF(X_SERIAL_MEM) \
/*                                                    Memory type,        Memory ID, Page Size,   NB of Page, Page Erase Sz,  Sector Sz, Sector Erase Sz,   Option */ \
    IF_USE( MEM_USE_M25PE16,            X_SERIAL_MEM( FLASH_M25PE16,      0x208015,  256,         8192,       256,            512,        65536,            MEM_OPT_READ_ID | MEM_OPT_FAST_READ | MEM_OPT_PAGE_ERASE | MEM_OPT_SECTOR_ERASE )) \
    IF_USE( MEM_USE_M25PE80,            X_SERIAL_MEM( FLASH_M25PE80,      0x208014,  256,         4096,       256,            512,        65536,            MEM_OPT_READ_ID | MEM_OPT_FAST_READ | MEM_OPT_PAGE_ERASE | MEM_OPT_SECTOR_ERASE )) \
    IF_USE( MEM_USE_M95512,             X_SERIAL_MEM( EEPROM_M95512,      0x000000,  128,         512,        0,              128,        0,                MEM_OPT_EEPROM_STYLE )) \
    IF_USE( MEM_USE_M95P16,             X_SERIAL_MEM( EEPROM_M95P16,      0x20BA15,  256,         8192,       256,            65536,      65536,            MEM_OPT_PAGE_ERASE | MEM_OPT_SECTOR_ERASE )) \
    IF_USE( MEM_USE_M95P32,             X_SERIAL_MEM( EEPROM_M95P32,      0x20BA16,  256,         16384,      256,            65536,      65536,            MEM_OPT_PAGE_ERASE | MEM_OPT_SECTOR_ERASE )) \
    IF_USE( MEM_USE_M95P64,             X_SERIAL_MEM( EEPROM_M95P64,      0x20BA17,  256,         32768,      256,            65536,      65536,            MEM_OPT_PAGE_ERASE | MEM_OPT_SECTOR_ERASE )) \

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum SerialMemoryCmd_e
{
    //MEMORY_CMD_READ_QPI                   = 0x00,     // Fast Read -- QPI             Winbond  QSPI not relevant here
    MEMORY_CMD_WRITE_STATUS_REGISTER        = 0x01,     // Write status register
    MEMORY_CMD_PAGE_PROGRAM                 = 0x02,     // Page program
    MEMORY_CMD_READ                         = 0x03,     // Read data bytes
    MEMORY_CMD_WRITE_DISABLE                = 0x04,     // Write disable
    MEMORY_CMD_READ_STATUS_REGISTER         = 0x05,     // Read status register
    MEMORY_CMD_WRITE_ENABLE                 = 0x06,     // Write enable
    MEMORY_CMD_PAGE_WRITE                   = 0x0A,     // Page write instruction                   ???
    MEMORY_CMD_FAST_READ                    = 0x0B,     // Read data bytes
    MEMORY_CMD_SECTOR_ERASE                 = 0x20,     // Sector erase -- 4K
  //MEMORY_CMD_READ_STATUS_REGISTER_2       = 0x35,     // Read status register         Winbond
    MEMORY_CMD_SECTOR_ERASE_32K             = 0x52,     // Sector erase -- 32K
    MEMORY_CMD_READ_SFPD                    = 0x5A,     // Read SFDP command
    MEMORY_CMD_CHIP_ERASE                   = 0x60,     // Chip Erase
    MEMORY_CMD_ENABLE_RESET                 = 0x66,     // Enable Reset flash           Winbond
    MEMORY_CMD_READ_ID                      = 0x90,     // Read Manufacturer/Device ID
    MEMORY_CMD_UNLOCK                       = 0x98,     // Unlock SST devices           SST
    MEMORY_CMD_RESET                        = 0x99,     // Reset flash                  Winbond
    MEMORY_CMD_READ_JEDEC_ID                = 0x9F,     // Read JEDEC ID
    MEMORY_CMD_READ_ELECTRONIC_SIGNATURE    = 0xAB,     // Read Electronic Signature or Release from deep power down instruction
    MEMORY_CMD_DEEP_POWER_DOWN              = 0xB9,     // Deep power down instruction
    MEMORY_CMD_BULK_ERASE                   = 0xC7,     // Bulk Erase instruction
    MEMORY_CMD_SECTOR_ERASE_64K             = 0xD8,     // Sector erase -- 64K
    MEMORY_CMD_WRITE_LOCK_REGISTER          = 0xE5,     // Write to lock Register instruction       ???
    MEMORY_CMD_READ_LOCK_REGISTER           = 0xE8,     // Read lock Register instruction           ???
    MEMORY_CMD_READ_SPIQ                    = 0xEB,     // Fast Read -- SPIQ            Winbond
    MEMORY_CMD_RESET_QPI                    = 0xFF,     // Reset flash                  SST
    MEMORY_CMD_READ_SECURE_STATUS           = 0x2B,     // Read MXIC secure status
};

struct MemoryInfo_t
{
    uint32_t   MemoryID;
    uint32_t   PageSize;
    uint32_t   NumberOfPages;
    uint32_t   PageEraseSize;
    uint32_t   SectorSize;
    uint32_t   SectorEraseSize;
    uint32_t   SupportOptions;
    uint32_t   NumberOfSectors;
};

enum MemoryList_e
{
    SERIAL_MEMORY_DEF(EXPAND_X_SERIAL_MEM_AS_ENUM)
    NUMBER_OF_MEMORY,
  #if (SERIAL_MEMORY_USE_AUTO_DETECT == DEF_ENABLED)
    FLASH_AUTO_DETECT,
  #endif
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SPI_SerialMemoryDriver
{
    public:

        SystemState_e               Initialize              (SPI_Driver* pSPI, MemoryList_e Memory, IO_ID_e ChipSelect);

        SystemState_e               EraseSector             (uint32_t SectorAddress);
        SystemState_e               BulkErase               (void);
        SystemState_e               Read                    (void* pBuffer, uint32_t Address, size_t Length);
        SystemState_e               Write                   (const void* pBuffer, uint32_t Address, size_t Length);

        uint32_t                    GetPageSize             (void)          { return m_MemoryInfo.PageSize; }
        uint64_t                    GetMemorySize           (void)          { return m_MemoryInfo.NumberOfPages * m_MemoryInfo.PageSize; }
        uint32_t                    GetSectorEraseSize      (void)          { return m_MemoryInfo.SectorEraseSize; }
        uint32_t                    GetSectorSize           (void)          { return m_MemoryInfo.SectorSize; }
        uint32_t                    GetMemoryID             (void)          { return m_MemoryInfo.MemoryID; }

    private:

        SystemState_e               SendCommandAndAddress   (SerialMemoryCmd_e Command, uint32_t Address);
        uint32_t                    ReadID                  (void);
        SystemState_e               WaitForEndWrite         (void);
        SystemState_e               WriteEnable             (void);
        SystemState_e               WriteDisable            (void);
        SystemState_e               WritePage               (void* pBuffer, uint32_t Address, size_t Length);

      #if (SERIAL_MEMORY_USE_AUTO_DETECT == DEF_ENABLED)
        SystemState_e               ReadSFDP                (uint32_t NumberOfByteToRead, uint32_t Address, uint8_t* pBuffer);
        uint32_t                    ReadSFDP_Density        (void);
      #endif

        IO_ID_e                     m_ChipSelect;
        IO_ID_e                     m_Reset;
        IO_ID_e                     m_WriteProtect;
        SPI_Driver*                 m_pSPI;
        MemoryInfo_t                m_MemoryInfo;
        static const MemoryInfo_t   m_MemoryInfoList        [NUMBER_OF_MEMORY];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define SPI_MEMORY_GLOBAL
#include "device_var.h"
#undef  SPI_MEMORY_GLOBAL

//-------------------------------------------------------------------------------------------------

#else // (USE_SPI_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for SPI must be enable and configure to use this device driver")

#endif // (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------




