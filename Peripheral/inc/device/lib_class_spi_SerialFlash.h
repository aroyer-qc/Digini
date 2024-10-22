//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_SerialFlash.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_SERIAL_FLASH_AS_ENUM(ENUM_ID,              )          ENUM_ID,
#define EXPAND_X_SERIAL_FLASH_AS_CLASS_CONST(ENUM_ID,       )          {       },

#define SERIAL_FLASH_DEF(X_SERIAL_FLASH) \
/*                                                   Flash type,                Chip ID,   Page Size,   NB of Page, Page Erase Sz,  Sector Sz, Sector Erase Sz,        */ \
    IF_USE( DIGINI_USE_SF_AT25SF321,          X_CRC( SERIAL_FLASH_AT25SF321,    0x1F8701,  256,         16384,      0,              4096,       4096,       )) /*                     */ \
    IF_USE( DIGINI_USE_SF_AT25SF641,          X_CRC( SERIAL_FLASH_AT25SF641,    0x1F1632,  256,         32768,      0,              4096,       4096,       )) /*                     */ \
    IF_USE( DIGINI_USE_SF_M25PE16,            X_CRC( SERIAL_FLASH_M25PE16,      0x208015,  256,         8192,       256,            512,        65536,      )) /*                     */ \
    IF_USE( DIGINI_USE_SF_M25PE80,            X_CRC( SERIAL_FLASH_M25PE80,      0x208014,  256,         4096,       256,            512,        65536,      )) /*                     */ \
    IF_USE( DIGINI_USE_SF_SST26VF032B,        X_CRC( SERIAL_FLASH_SST26VF032B,  0xBF2642,  256,         16384,      0,              4096,       4096,       )) /*                     */ \
    IF_USE( DIGINI_USE_SF_SST26VF064B,        X_CRC( SERIAL_FLASH_SST26VF064B,  0xBF2643,  256,         32768,      0,              4096,       4096,       )) /*                     */ \
    IF_USE( DIGINI_USE_SF_W25Q16JV,           X_CRC( SERIAL_FLASH_W25Q16JV,     0xEF4015,  256,         8192,       0,              4096,       4096,       )) /*                     */ \
    IF_USE( DIGINI_USE_SF_W25Q32JV,           X_CRC( SERIAL_FLASH_W25Q32JV,     0xEF4016,  256,         16384,      0,              4096,       4096,       )) /*                     */ \
    IF_USE( DIGINI_USE_SF_W25Q64JV,           X_CRC( SERIAL_FLASH_W25Q64JV,     0xEF4017,  256,         32768,      0,              4096,       4096,       )) /*                     */ \

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum SerialFlashCmd_e
{
    FLASH_CMD_WRITE_STATUS_REGISTER     = 0x01,     // Write status register
    FLASH_CMD_PAGE_PROGRAM	            = 0x02,     // Page program
    FLASH_CMD_READ			            = 0x03,     // Read data bytes
    FLASH_CMD_WRITE_DISABLE             = 0x04,     // Write disable
    FLASH_CMD_READ_STATUS_REGISTER      = 0x05,     // Read status register
    FLASH_CMD_WRITE_ENABLE	            = 0x06,     // Write enable
    FLASH_CMD_FAST_READ		            = 0x0B,     // Read data bytes
    FLASH_CMD_PAGE_ERASE			    = 0x20,     // Page erase -- 4K
  //FLASH_CMD_READ_STATUS_REGISTER_2	= 0x35,     // Read status register         Winbond
    FLASH_CMD_CHIP_ERASE			    = 0x60,  	// Chip Erase
    FLASH_CMD_ENABLE_RESET		        = 0x66,     // Enable Reset flash           Winbond
    FLASH_CMD_READ_ID		            = 0x90,     // Read Manufacturer/Device ID
    FLASH_CMD_RESET			            = 0x99,     // Reset flash 			        Winbond
    FLASH_CMD_READ_JEDEC_ID		        = 0x9F,     // Read JEDEC ID
    FLASH_CMD_READ_ELECTRONIC_SIGNATURE = 0xAB,     // Read Electronic Signature
    FLASH_CMD_SECTOR_ERASE              = 0xD8,     // Sector erase -- 64K
    FLASH_CMD_SECTOR_ERASE_32K          = 0x52,     // Sector erase -- 32K
    FLASH_CMD_READ_SPIQ		            = 0xEB,     // Fast Read -- SPIQ            Winbond
    FLASH_CMD_READ_QPI			        = 0x00,     // Fast Read -- QPI             Winbond
    FLASH_CMD_RESET_QPI		            = 0xFF,	    // Reset flash                  SST
    FLASH_CMD_READ_SECURE_STATUS        = 0x2B,     // Read MXIC secure status
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SPI_SerialFLash_Driver // : DiskIO_DeviceInterface
{
    public:

                                    SPI_SerialFLash_Driver  (SPI_Driver* pSPI, IO_ID_e ChipSelect, IO_ID_e Reset, IO_ID_e WriteProtect);

        // Set of function for FatFS
        DSTATUS                     Initialize              (void);
        DSTATUS                     Status                  (void);
        DRESULT                     Read                    (uint8_t* pBuffer, uint32_t Sector, uint8_t Count);
        DRESULT                     Write                   (const uint8_t* pBuffer, uint32_t Sector, uint8_t Count);
        DRESULT                     IO_Control              (uint8_t Control, void *pBuffer);
        bool                        IsOperational           (void);
        void                        TickHook                (void);

    private:

        void                        ChipSelect              (eChipSelect Select);
        eSystemState                WaitReadyAndChipSelect  (eChipSelect Select);
        eSystemState                Erase                   (uint32_t Command);
        uint16_t                    GetLastChipStatus       ()                              { return m_ChipStatus; }

       #if SPI_FLASH_USE_RESET == 1
        void                        ChipReset               (eChipReset Reset)              { IO_Output(m_ChipReset, (Reset == CHIP_RESET) ? IO_SET : IO_RESET); }
       #else
        void                        ChipReset               (eChipReset Reset)              { VAR_UNUSED(Reset); }
       #endif

       #if SPI_FLASH_USE_WRITE_PROTECT == 1
        void                        WriteProtect            (eWritePermission Permission)  { IO_Output(m_ChipWriteProtect, (Permission == WRITE_ALLOWED) ? IO_RESET : IO_SET); }
       #else
        void                        WriteProtect            (eWritePermission Permission)  { VAR_UNUSED(Permission); }
       #endif

        bool                        m_isItInitialized;
        SPI_Driver*                 m_pSPI;
        IO_ID_e                     m_ChipSelectIO;
        uint16_t                    m_ChipStatus;


        
        SPI_sDeviceInfo*            m_pDevice;


        IO_ID_e                  m_ChipSelect;
        IO_ID_e                  m_Reset;
        IO_ID_e                  m_WriteProtect;
        uint8_t                     m_TickPeriod;
        eSystemState                m_Status;
        volatile uint32_t           m_TimeOut;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_SPI_SERIAL__
#include "device_var.h"
#undef  __CLASS_SPI_SERIAL__

---------------------------------------------------

#else // (USE_SPI_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for SPI must be enable and configure to use this device driver")

#endif // (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------




