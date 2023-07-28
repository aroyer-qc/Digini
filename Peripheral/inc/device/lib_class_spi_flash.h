//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_flash.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

//#include "lib_digini.h"
#include "lib_class_memory_driver_interface.h"
#include "flash_cfg.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_FLASH == DEF_ENABLED)
#if (USE_SPI_DRIVER == DEF_ENABLED) || (USE_QSPI_DRIVER == DEF_ENABLED)

#define FLASH_DEF(X_FLASH)  \
/*                                      ID of FLASH,                Device ID,          FLASH Size  Number of page    sector Size/Erase      Block Size/Erase,  PageSize*/    \
/*                                                                                      in KBytes    redondant!!!                                                       */    \
    IF_USE( DIGINI_USE_FLASH_W25Q80,    X_FLASH(E2_W25Q80_ID,       0x13,  0x4014,      1024,       4096                  4096                   32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q16,    X_FLASH(E2_W25Q16_ID,       0x14,  0x4015,      2048,       8192                  4096                   32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q32,    X_FLASH(E2_W25Q32_ID,       0x15,  0x4016,      4096,       16384                 4096                   32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q64,    X_FLASH(E2_W25Q64_ID,       0x16,  0x4017, (QSPI = 0x6017), 8192,  32768          4096                   32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q128,   X_FLASH(E2_W25Q128_ID,      0x17,  0x4018, (QSPI = 0x6018), 16384, 65536          4096                   32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q256,   X_FLASH(E2_W25Q256_ID,      0x18,  0x4019, (QSPI = 0x6019 or 7019), 32768, 131072         4096                   32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q512,   X_FLASH(E2_W25Q512_ID,      0x19,  0x4020,      65536,      262144,               4096                   32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q01,    X_FLASH(E2_W25Q01_ID,       0x20,  0x4021,      131072,     1048576,              4096,                  32768/65536       256 ) ) \
    IF_USE( DIGINI_USE_FLASH_W25Q02,    X_FLASH(E2_W25Q02_ID,       0x21,  0x7022,      262144,     256              ) ) \

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_FLASH_CFG_AS_ENUM(ENUM_ID, SIZE, NB_OF_PAGE)  ENUM_ID,
#define EXPAND_X_FLASH_CFG_AS_DATA(ENUM_ID, SIZE, NB_OF_PAGE)  {SIZE, NB_OF_PAGE, (SIZE / NB_OF_PAGE), ((SIZE / NB_OF_PAGE) - 1), ((SIZE > EEPROM_SIZE_LIMIT_64K) ? 3 : 2)},

//-------------------------------------------------------------------------------------------------
// typedef(s)
//-------------------------------------------------------------------------------------------------

enum FLASH_Device_e
{
    FLASH_DEF(EXPAND_X_FLASH_CFG_AS_ENUM)

    NUMBER_OF_FLASH_DEVICE,
};


struct FLASH_DeviceInfo_t
{
    uint32_t    ChipID;
    uint32_t    Size;
    uint32_t    NbOfPage;
    uint32_t    PageSize;
    uint32_t    PageMask;
    uint8_t     AddressingSize;
};

struct FLASH_Info_t
{
    FLASH_Device_e   Flash_ID;
    SPI_Driver*      pSPI;  // or QSPI Driver!!
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class FLASH_Driver : public MemoryDriverInterface
{
    public:

                                    FLASH_Driver               (const FLASH_Info_t* pInfo);

        SystemState_e               Read                    (uint32_t Address, void* pDest,      size_t Size = 1);
        SystemState_e               Write                   (uint32_t Address, const void* pSrc, size_t Size = 1);
        //bool                        IsOperational           ();

    private:

        const FLASH_Info_t*                m_pInfo;
        const FLASH_DeviceInfo_t*          m_pDevice;
        static const FLASH_DeviceInfo_t    m_DeviceInfo[NUMBER_OF_FLASH_DEVICE];
};

//-------------------------------------------------------------------------------------------------
// constant data
//-------------------------------------------------------------------------------------------------

#include "flash_var.h"         // Project variable

//-------------------------------------------------------------------------------------------------

#else // (USE_SPI_DRIVER == DEF_ENABLED) || (USE_QSPI_DRIVER == DEF_ENABLED)

 #pragma message("DIGINI driver for SPI and/or QSPI must be enable and configure to use this device driver")

#endif // (USE_SPI_DRIVER == DEF_ENABLED) || (USE_QSPI_DRIVER == DEF_ENABLED)
#endif // (DIGINI_USE_FLASH == DEF_ENABLED)


//-------------------------------------------------------------------------------------------------
