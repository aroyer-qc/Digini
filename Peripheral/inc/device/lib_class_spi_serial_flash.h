//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_serial_flash.h
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

#define EXPAND_X_SERIAL_FLASH_AS_ENUM(ENUM_ID, FLASH_ID, PAGE_SIZE, NB_OF_PAGE, PAGE_ERASE_SIZE, SECTOR_SIZE, SECTOR_ERASE_SIZE) \
                                      ENUM_ID,

#define EXPAND_X_SERIAL_FLASH_AS_CLASS_CONST(ENUM_ID, FLASH_ID, PAGE_SIZE, NB_OF_PAGE, PAGE_ERASE_SIZE, SECTOR_SIZE, SECTOR_ERASE_SIZE) \
                                                     {FLASH_ID, PAGE_SIZE, NB_OF_PAGE, PAGE_ERASE_SIZE, SECTOR_SIZE, SECTOR_ERASE_SIZE, ((NB_OF_PAGE * PAGE_SIZE) / SECTOR_SIZE)},

#define SERIAL_FLASH_DEF(X_SERIAL_FLASH) \
/*                                                        Flash type,         Flash ID,  Page Size,   NB of Page, Page Erase Sz,  Sector Sz, Sector Erase Sz  */ \
    IF_USE( FLASH_USE_AT25SF321,          X_SERIAL_FLASH( FLASH_AT25SF321,    0x1F8701,  256,         16384,      0,              4096,       4096   ))          \
    IF_USE( FLASH_USE_AT25SF641,          X_SERIAL_FLASH( FLASH_AT25SF641,    0x1F1632,  256,         32768,      0,              4096,       4096   ))          \
    IF_USE( FLASH_USE_M25PE16,            X_SERIAL_FLASH( FLASH_M25PE16,      0x208015,  256,         8192,       256,            512,        65536  ))          \
    IF_USE( FLASH_USE_M25PE80,            X_SERIAL_FLASH( FLASH_M25PE80,      0x208014,  256,         4096,       256,            512,        65536  ))          \
    IF_USE( FLASH_USE_SST26VF032B,        X_SERIAL_FLASH( FLASH_SST26VF032B,  0xBF2642,  256,         16384,      0,              4096,       4096   ))          \
    IF_USE( FLASH_USE_SST26VF064B,        X_SERIAL_FLASH( FLASH_SST26VF064B,  0xBF2643,  256,         32768,      0,              4096,       4096   ))          \
    IF_USE( FLASH_USE_W25Q16JV,           X_SERIAL_FLASH( FLASH_W25Q16JV,     0xEF4015,  256,         8192,       0,              4096,       4096   ))          \
    IF_USE( FLASH_USE_W25Q32JV,           X_SERIAL_FLASH( FLASH_W25Q32JV,     0xEF4016,  256,         16384,      0,              4096,       4096   ))          \
    IF_USE( FLASH_USE_W25Q64JV,           X_SERIAL_FLASH( FLASH_W25Q64JV,     0xEF4017,  256,         32768,      0,              4096,       4096   ))          \
    IF_USE( FLASH_USE_W25Q128JV,          X_SERIAL_FLASH( FLASH_W25Q128JV,    0xEF4018,  256,         65536,      0,              4096,       4096   ))          \

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum SerialFlashCmd_e
{
    //FLASH_CMD_READ_QPI			    = 0x00,     // Fast Read -- QPI             Winbond  QSPI not relevant here
    FLASH_CMD_WRITE_STATUS_REGISTER     = 0x01,     // Write status register
    FLASH_CMD_PAGE_PROGRAM	            = 0x02,     // Page program
    FLASH_CMD_READ			            = 0x03,     // Read data bytes
    FLASH_CMD_WRITE_DISABLE             = 0x04,     // Write disable
    FLASH_CMD_READ_STATUS_REGISTER      = 0x05,     // Read status register
    FLASH_CMD_WRITE_ENABLE	            = 0x06,     // Write enable
    FLASH_CMD_PAGE_WRITE                = 0x0A,     // Page write instruction                   ???
    FLASH_CMD_FAST_READ		            = 0x0B,     // Read data bytes
    FLASH_CMD_SECTOR_ERASE			    = 0x20,     // Sector erase -- 4K
  //FLASH_CMD_READ_STATUS_REGISTER_2	= 0x35,     // Read status register         Winbond
    FLASH_CMD_SECTOR_ERASE_32K          = 0x52,     // Sector erase -- 32K
    FLASH_CMD_READ_SFPD	     		    = 0x5A,  	// Read SFDP command
    FLASH_CMD_CHIP_ERASE			    = 0x60,  	// Chip Erase
    FLASH_CMD_ENABLE_RESET		        = 0x66,     // Enable Reset flash           Winbond
    FLASH_CMD_READ_ID		            = 0x90,     // Read Manufacturer/Device ID
    FLASH_CMD_UNLOCK                    = 0x98,     // Unlock SST devices           SST
    FLASH_CMD_RESET			            = 0x99,     // Reset flash 			        Winbond
    FLASH_CMD_READ_JEDEC_ID		        = 0x9F,     // Read JEDEC ID
    FLASH_CMD_READ_ELECTRONIC_SIGNATURE = 0xAB,     // Read Electronic Signature or Release from deep power down instruction
    FLASH_CMD_DEEP_POWER_DOWN           = 0xB9,     // Deep power down instruction
    FLASH_CMD_BULK_ERASE                = 0xC7,     // Bulk Erase instruction
    FLASH_CMD_SECTOR_ERASE_64K          = 0xD8,     // Sector erase -- 64K
    FLASH_CMD_WRITE_LOCK_REGISTER		= 0xE5,     // Write to lock Register instruction       ???
    FLASH_CMD_READ_LOCK_REGISTER        = 0xE8,     // Read lock Register instruction           ???
    FLASH_CMD_READ_SPIQ		            = 0xEB,     // Fast Read -- SPIQ            Winbond
    FLASH_CMD_RESET_QPI		            = 0xFF,	    // Reset flash                  SST
    FLASH_CMD_READ_SECURE_STATUS        = 0x2B,     // Read MXIC secure status
};

struct FlashInfo_t
{
    uint32_t   FlashID;
    uint32_t   PageSize;
    uint32_t   NumberOfPages;
    uint32_t   PageEraseSize;
    uint32_t   SectorSize;
    uint32_t   SectorEraseSize;
    uint32_t   NumberOfSectors;
};

#if (FLASH_USE_AUTO_DETECT_FLASH != DEF_ENABLED)
enum FlashList_e
{
    SERIAL_FLASH_DEF(EXPAND_X_SERIAL_FLASH_AS_ENUM)
    NUMBER_OF_FLASH,
};
#endif

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SPI_SerialFLashDriver
{
    public:

      #if (FLASH_USE_AUTO_DETECT_FLASH == DEF_ENABLED)
        SystemState_e               Initialize              (SPI_Driver* pSPI, IO_ID_e ChipSelect);
      #else
        SystemState_e               Initialize              (SPI_Driver* pSPI, FlashList_e Flash, IO_ID_e ChipSelect);
      #endif

        SystemState_e               EraseSector             (uint32_t SectorAddress);
        SystemState_e               BulkErase               (void);
        SystemState_e               Read                    (void* pBuffer, uint32_t Address, size_t Length);
        SystemState_e               Write                   (const void* pBuffer, uint32_t Address, size_t Length);

        uint32_t                    GetPageSize             (void)          { return m_FlashInfo.PageSize; }
        uint64_t                    GetFlashSize            (void)          { return m_FlashInfo.NumberOfPages * m_FlashInfo.PageSize; }
        uint32_t                    GetSectorEraseSize      (void)          { return m_FlashInfo.SectorEraseSize; }
        uint32_t                    GetSectorSize           (void)          { return m_FlashInfo.SectorSize; }
        uint32_t                    GetFlashID              (void)          { return m_FlashInfo.FlashID; }

    private:

        SystemState_e               SendCommandAndAddress   (SerialFlashCmd_e Command, uint32_t Address);
        uint32_t                    ReadID                  (void);
        SystemState_e               WaitForEndWrite         (void);
        SystemState_e               WriteEnable             (void);
        SystemState_e               WriteDisable            (void);
        SystemState_e               WritePage               (void* pBuffer, uint32_t Address, size_t Length);

      #if (FLASH_USE_AUTO_DETECT_FLASH == DEF_ENABLED)
        SystemState_e               ReadSFDP                (uint32_t NumberOfByteToRead, uint32_t Address, uint8_t* pBuffer);
        uint32_t                    ReadSFDP_Density        (void);
      #endif

        IO_ID_e                     m_ChipSelect;
        IO_ID_e                     m_Reset;
        IO_ID_e                     m_WriteProtect;
        SPI_Driver*                 m_pSPI;
        FlashInfo_t                 m_FlashInfo;

      #if (FLASH_USE_AUTO_DETECT_FLASH != DEF_ENABLED)
        static const FlashInfo_t    m_FlashInfoList         [NUMBER_OF_FLASH];
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define SPI_FLASH_GLOBAL
#include "device_var.h"
#undef  SPI_FLASH_GLOBAL

//-------------------------------------------------------------------------------------------------

#else // (USE_SPI_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for SPI must be enable and configure to use this device driver")

#endif // (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------




