//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_i2c_EEprom.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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
#include "eeprom_cfg.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_EEPROM == DEF_ENABLED)
#if (USE_I2C_DRIVER == DEF_ENABLED)

#define E2_DEF(X_E2)  \
/*                                       ID of E2               E2 Size     Number of page */    \
    IF_USE( DIGINI_USE_E2_M24LC32A, X_E2(E2_M24LC32A_ID,        4096,       32               ) ) \
    IF_USE( DIGINI_USE_E2_2464,     X_E2(E2_2464_ID,            8192,       32               ) ) \
    IF_USE( DIGINI_USE_E2_24128,    X_E2(E2_24128_ID,           16384,      64               ) ) \
    IF_USE( DIGINI_USE_E2_24256,    X_E2(E2_24256_ID,           32768,      64               ) ) \
    IF_USE( DIGINI_USE_E2_24512,    X_E2(E2_24512_ID,           65535,      128              ) ) \
    IF_USE( DIGINI_USE_E2_M24M01,   X_E2(E2_M24M01_ID,          131072,     256              ) ) \

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EEPROM_SIZE_LIMIT_64K                               0xFFFF

#define EXPAND_X_E2_CFG_AS_ENUM(ENUM_ID, SIZE, NB_OF_PAGE)  ENUM_ID,
#define EXPAND_X_E2_CFG_AS_DATA(ENUM_ID, SIZE, NB_OF_PAGE)  {SIZE, NB_OF_PAGE, (SIZE / NB_OF_PAGE), ((SIZE / NB_OF_PAGE) - 1), ((SIZE > EEPROM_SIZE_LIMIT_64K) ? 3 : 2)},

//-------------------------------------------------------------------------------------------------
// typedef(s)
//-------------------------------------------------------------------------------------------------

enum E2_Device_e
{
    E2_DEF(EXPAND_X_E2_CFG_AS_ENUM)

    NUMBER_OF_DEVICE,
};


struct E2_DeviceInfo_t
{
    uint32_t    Size;
    uint32_t    NbOfPage;
    uint32_t    PageSize;
    uint32_t    PageMask;
    uint8_t     AddressingSize;
};

struct E2_Info_t
{
    E2_Device_e      E2_ID;
    I2C_Driver*      pI2C;
    uint8_t          DeviceAddress;
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class E2_Driver : public MemoryDriverInterface
{
    public:

                                    E2_Driver               (const E2_Info_t* pInfo);

        SystemState_e               Read                    (uint32_t Address, void* pDest,      size_t Size = 1);
        SystemState_e               Write                   (uint32_t Address, const void* pSrc, size_t Size = 1);
        //bool                        IsOperational           ();

    private:

        const E2_Info_t*                m_pInfo;
        const E2_DeviceInfo_t*          m_pDevice;
        static const E2_DeviceInfo_t    m_DeviceInfo[NUMBER_OF_DEVICE];
};

//-------------------------------------------------------------------------------------------------
// constant data
//-------------------------------------------------------------------------------------------------

#include "eeprom_var.h"         // Project variable

//-------------------------------------------------------------------------------------------------

#else // (USE_I2C_DRIVER == DEF_ENABLED)

 #pragma message("DIGINI driver for I2C must be enable and configure to use this device driver")

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
#endif // (DIGINI_USE_EEPROM == DEF_ENABLED)


//-------------------------------------------------------------------------------------------------
