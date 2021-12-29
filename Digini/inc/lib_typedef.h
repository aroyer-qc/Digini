//-------------------------------------------------------------------------------------------------
//
//  File : lib_typedef.h
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
// Type definition(s), enum(s) and structure(s)
//-------------------------------------------------------------------------------------------------

//  ASCII Table
enum ASCII_e
{
    ASCII_NULL                = 0,        // 0x00
    ASCII_START_OF_HEADING,
    ASCII_START_OF_TEXT,
    ASCII_END_OF_TEXT,
    ASCII_END_OF_TRANSMIT,
    ASCII_ENQUIRY,
    ASCII_ACK,
    ASCII_AUDIBLE_BELL,
    ASCII_BACKSPACE,
    ASCII_HORIZ_TAB,
    ASCII_LINE_FEED,
    ASCII_VERT_TAB,
    ASCII_FORM_FEED,
    ASCII_CARRIAGE_RETURN,
    ASCII_SHIFT_OUT,
    ASCII_SHIFT_IN,
    ASCII_DATA_LINK_ESC,
    ASCII_DEVICE_CTRL1,                    // 0x11            Use for color change in graphic printf for a single caracter
    ASCII_DEVICE_CTRL2,                    // 0x12            Use for color change in graphic printf for following caracter
    ASCII_DEVICE_CTRL3,                    // 0x13            Use for font change in graphic printf for a single caracter
    ASCII_DEVICE_CTRL4,                    // 0x14            Use for font change in graphic printf for following caracter
    ASCII_NACK,
    ASCII_SYNC_IDLE,
    ASCII_END_TX_BLOCK,
    ASCII_CANCEL,
    ASCII_END_OF_MEDIUM,
    ASCII_SUBSTITUTION,
    ASCII_ESCAPE,
    ASCII_FILE_SEPARATOR,
    ASCII_GROUP_SEPARATOR,
    ASCII_RECORD_SEPARATOR,
    ASCII_UNIT_SEPARATOR,
    ASCII_SPACE,
    ASCII_EXCLAM_MARK,
    ASCII_QUOTE_MARK,
    ASCII_NBR,
    ASCII_SINGLE_COLOR_OVERRIDE = ASCII_DEVICE_CTRL1,
    ASCII_COLOR_OVERRIDE        = ASCII_DEVICE_CTRL2,
    ASCII_SINGLE_FONT_OVERRIDE  = ASCII_DEVICE_CTRL3,
    ASCII_FONT_OVERRIDE         = ASCII_DEVICE_CTRL4,
};

enum SystemState_e
{
    SYS_READY                       = 0,                                      // READY is good for everything

    // Fat FS section in bit position
    SYS_STA_OK                      = 0,
    STA_OK                          = 0,

    SYS_STA_NOINIT                  = 1,        // Drive not initialized
    STA_NOINIT                      = 1,        // Drive not initialized
    SYS_STA_NODISK                  = 2,        // No medium in the drive
    STA_NODISK                      = 2,        // No medium in the drive
    SYS_STA_PROTECT                 = 4,        // Write protected
    STA_PROTECT                     = 4,        // Write protected

    // General use status (SD Card, SPI, I2C etc...)

    SYS_ADDRESS_MISALIGNED,
    SYS_ALREADY_BLANK,
    SYS_ARBITRATION_LOST,
    SYS_AUDIO_CODEC_ERROR,
    SYS_BLOCK_LENGTH_ERROR,
    SYS_BUS_ERROR,
    SYS_BUSY,
    SYS_BUSY_TX,
    SYS_BUSY_RX,
    SYS_BUSY_B4_RELEASE,
    SYS_COMMAND_TIME_OUT,
    SYS_CANNOT_SET,
    SYS_CRC_FAIL,
    SYS_CRC_OK,
    SYS_DATA_TIME_OUT,
    SYS_DEVICE_BUSY,
    SYS_DEVICE_DISABLE,
    SYS_DEVICE_ERROR,
    SYS_DEVICE_FAIL,
    SYS_DEVICE_NOT_PRESENT,
    SYS_DEVICE_NOT_READY,
    SYS_DMA_ERROR,
    SYS_FAIL,
    SYS_ERROR,
    SYS_RESET,
    SYS_FAIL_MEMORY_ALLOCATION,
    SYS_GENERAL_UNKNOWN_ERROR,
    SYS_HUNG,
    SYS_IDLE,
    SYS_ILLEGAL_COMMAND,
    SYS_INITIALIZATION_FAIL,
    SYS_INVALID_PARAMETER,
    SYS_INVALID_VALUE,
    SYS_INVALID_VOLTAGE_RANGE,
    SYS_IO_ERROR,
    SYS_LOCK_UNLOCK_FAILED,
    SYS_NO_DATA,
    SYS_NO_DRIVER,
    SYS_NOT_LOCK_TO_DEVICE,
    SYS_NOT_SUPPORTED,
    SYS_NULLPTR,
    SYS_OUT_OF_BOUND,
    SYS_OUT_OF_RANGE,
    SYS_OVERRUN,
    SYS_POOL_EMPTY,
    SYS_POOL_FULL,
    SYS_POOL_NOT_ALLOCATED_ERROR,
    SYS_POOL_USED,
    SYS_READ_ONLY,
    SYS_RESPONSE_RECEIVED           = 7,
    SYS_REQUEST_NOT_APPLICABLE,
    SYS_SDRAM_ERROR,
    SYS_SECTOR_LOCK_FOUND,                          // SPi Flash
    SYS_SOURCE_ERROR,
    SYS_START_BIT_ERROR,
    SYS_TIME_OUT,
    SYS_TRANSFER_ERROR,
    SYS_UNDERRUN,
    SYS_UNRECOVERED_ERROR,
    SYS_UNSUPPORTED_FEATURE,
    SYS_WRONG_DEVICE,
    SYS_WRONG_SIZE,
    SYS_WRONG_VALUE,

    SD_CC_ERROR,
    SD_ERASE_SEQ_ERR,
    SD_CARD_ECC_FAILED,
    SD_BAD_ERASE_PARAM,
    SD_WRITE_PROT_VIOLATION,
    SD_CID_CSD_OVERWRITE,
    SD_WP_ERASE_SKIP,
    SD_CARD_ECC_DISABLED,
    SD_ERASE_RESET,
    SD_AKE_SEQ_ERROR,
    SD_ADDRESS_OUT_OF_RANGE,
};

enum ChipSelect_e
{
    CS_DISABLE,
    CS_ENABLE,
    CS_PULSE_HIGH,
    CS_PULSE_LOW,
};

/*
typedef enum __ChipReset_e
{
    CHIP_RESET,
    CHIP_RELEASE,
} ChipReset_e;
*/

enum Power_e
{
    POWER_OFF,
    POWER_UP,
    POWER_ON,
};

enum AccessRequest_e
{
    ACCESS_READ,
    ACCESS_WRITE,
};

/*
typedef enum __WritePermission_e
{
    WRITE_PROTECTED,
    WRITE_ALLOWED,
} WritePermission_e;
*/

enum Range_e
{
    RANGE_MIN,
    RANGE_MAX,
    RANGE_AVERAGE,
    RANGE_MID_POINT,
};

// --------- General typedef ---------

union s8_t
{
    uint8_t         u_8;
    int8_t          n_8;
    char            c;

    struct
    {
        uint8_t     n0 :4;
        uint8_t     n1 :4;
    }nibble;

    struct
    {
        uint8_t     p0 :2;
        uint8_t     p1 :2;
        uint8_t     p2 :2;
        uint8_t     p3 :2;
    }pair;

    struct
    {
        uint8_t     b0 :1;
        uint8_t     b1 :1;
        uint8_t     b2 :1;
        uint8_t     b3 :1;
        uint8_t     b4 :1;
        uint8_t     b5 :1;
        uint8_t     b6 :1;
        uint8_t     b7 :1;
    }bit;
};

union s16_t
{
    uint16_t        u_16;
    int16_t         n_16;
   // wchar_t         wc;
    union s8_t      s8_Array[2];

    uint8_t         u8_Array[2];
    int8_t          n8_Array[2];
    char            cArray[2];
    signed char     nArray[2];

    struct
    {
        uint8_t     u0;
        uint8_t     u1;
    }u_8;

    struct
    {
        uint16_t    n0 :4;
        uint16_t    n1 :4;
        uint16_t    n2 :4;
        uint16_t    n3 :4;
    } nibble;

    struct
    {
        uint16_t    n0 :2;
        uint16_t    n1 :2;
        uint16_t    n2 :2;
        uint16_t    n3 :2;
        uint16_t    n4 :2;
        uint16_t    n5 :2;
        uint16_t    n6 :2;
        uint16_t    n7 :2;
    } pair;

    struct
    {
        uint16_t    b0 :1;
        uint16_t    b1 :1;
        uint16_t    b2 :1;
        uint16_t    b3 :1;
        uint16_t    b4 :1;
        uint16_t    b5 :1;
        uint16_t    b6 :1;
        uint16_t    b7 :1;
        uint16_t    b8 :1;
        uint16_t    b9 :1;
        uint16_t    bA :1;
        uint16_t    bB :1;
        uint16_t    bC :1;
        uint16_t    bD :1;
        uint16_t    bE :1;
        uint16_t    bF :1;
    }b;

};

union s32_t
{
    uint32_t u_32;
    int32_t  n_32;

    uint16_t    u16_Array[2];
    int16_t     n16_Array[2];
    union s16_t s16_Array[2];
   // wchar_t     wcArray[2];


    uint8_t     u8_Array[4];
    int8_t      n8_Array[4];
    union s8_t  s8_Array[4];
    char        cArray[4];
    signed char nArray[4];


    struct
    {
        uint8_t u0;
        uint8_t u1;
        uint8_t u2;
        uint8_t u3;
    } u_8;
    struct
    {
        uint16_t u0;
        uint16_t u1;
    } u_16;
};

union s64_t
{
    uint64_t    u64;
    int64_t     n64;

    uint32_t    u32_Array[2];
    int32_t     n32_Array[2];
    union s32_t s32_Array[2];

    uint16_t    u16_Array[4];
    int16_t     n16_Array[4];
    union s16_t s16_Array[8];

    uint8_t     u8_Array[8];
    int8_t      n8_Array[8];
    union s8_t  s8_Array[8];

    struct
    {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
        uint8_t b4;
        uint8_t b5;
        uint8_t b6;
        uint8_t b7;
    } bit;

    struct
    {
        uint16_t u0;
        uint16_t u1;
        uint16_t u2;
        uint16_t u3;
    } u_16;
};

// --------- Time and Date ---------

#ifdef __cplusplus   // need to provide wrapper

struct Time_t
{
    uint8_t     Second;
    uint8_t     Minute;
    uint8_t     Hour;
};

struct Date_t
{
    uint8_t     Day;
    uint8_t     Month;
    uint8_t     Year;
};

struct Clock_t
{
    Time_t      Time;
    Date_t      Date;
    uint8_t     DayOfWeek;
    uint16_t    DayOfYear;
    uint8_t     WeekOfYear;
    uint8_t     Century;
    uint16_t    MinuteOfDay;
    uint32_t    SecondStamp;
};

enum TimeFormat_e
{
    TIME_FORMAT_24_HOUR = 0,
    TIME_FORMAT_12_HOUR,
};

#endif

// --------- Miscelleaneous ---------

struct LinearEquation_t
{
    int16_t Slope;
    int16_t Y_Intercept;
};