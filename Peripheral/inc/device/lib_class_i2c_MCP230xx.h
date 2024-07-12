//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_i2c_MCP230xx.h
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
//
//  Note(s)
//
//              Use the IO_PIN_MASK_x from the lib_io.h
//              There is 2 extra define 
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// ---- Global Interrupts definitions ----
//#define IRQ_FLAG_GPIO               0x80
//#define IRQ_FLAG_ADC                0x40
//#define IRQ_FLAG_TEMP               0x20
//#define IRQ_FLAG_FE                 0x10
//#define IRQ_FLAG_FF                 0x08
//#define IRQ_FLAG_FOV                0x04
//#define IRQ_FLAG_FTH                0x02
//#define IRQ_FLAG_TOUCH              0x01
//#define IRQ_FLAG_ALL                0xFF


#define IO_PIN_MASK_ALL_LOW           ((uint16_t)0x00FF)
#define IO_PIN_MASK_ALL_HIGH          ((uint16_t)0xFF00)

//-------------------------------------------------------------------------------------------------
// typedef struct(s)
//-------------------------------------------------------------------------------------------------

enum MCP230xx_Device_e
{
    MCP23008_DEVICE,
    MCP23017_DEVICE,
};

enum MCP230xx_Polarity_e
{
    MCP230xx_POLARITY_ACTIVE_LOW    = 0,
    MCP230xx_POLARITY_ACTIVE_HIGH   = 1,
};

// Valid for both ConfigurePinInt and
enum MCP230xx_DriveMode_e
{
    MCP230xx_IO_PUSH_PULL               = 0,
    MCP230xx_IO_OPEN_DRAIN              = 1,
};

enum MCP230xx_Direction_e
{
    MCP230xx_IO_DIR_INPUT               = 0,
    MCP230xx_IO_DIR_OUTPUT              = 1,
};

enum MCP230xx_Level_e
{
    MCP230xx_IO_LOW                     = 0,
    MCP230xx_IO_HIGH                    = 1,
};

enum MCP230xx_PullUp_e
{
    MCP230xx_INPUT_NO_PULL              = 0,
    MCP230xx_INPUT_PULL_UP              = 1,
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class MCP230xx
{
    public:

        SystemState_e       Initialize              (I2C_Driver* pI2C, uint8_t DeviceAddress, MCP230xx_Device_e Device);

        // Interrupt access method
        void                ConfigurePinIntType     (MCP230xx_DriveMode_e DriveMode, MCP230xx_Polarity_e Polarity);
        void                SetIRQ_Enable           (bool State);
        bool                GetIRQ_Status           (uint8_t IRQ_Flag);
        void                ClearIRQ_Status         (uint8_t IRQ_Flag);

        // Pin control method
        void                PinInitInput            (uint16_t Pin, MCP230xx_PullUp_e PullEnable);                                               // Set pin in input
        void                PinInitOutput           (uint16_t Pin, MCP230xx_DriveMode_e DriveMode, MCP230xx_Level_e Output = MCP230xx_IO_LOW ); // Set pin in output and configure pin drive mode MCP230xx_IO_PUSH_PULL or MCP230xx_IO_OPEN_DRAIN
        void                SetPinLow               (uint16_t Pin);                                                                             // Set pin low
        void                SetPinHigh              (uint16_t Pin);                                                                             // Set pin high
        void                TogglePin               (uint16_t Pin);                                                                             // Toggle pin
        void                SetPin                  (uint16_t Pin, MCP230xx_Level_e Output);                                                    // Set pin to specific level
        MCP230xx_Level_e    ReadPin                 (uint16_t Pin);                                                                             // Read the level on the pin (work in both input and output)  WARNING: A single pin must be selected.

    private:

        void                ConfigurePinType        (uint16_t Pin, MCP230xx_DriveMode_e DriveMode);
        void                ConfigurePinDirection   (uint16_t Pin, MCP230xx_Direction_e Direction);
        void                ConfigurePinOutputLatch (uint16_t Pin, MCP230xx_Level_e Output);
        uint16_t            GetOLAT_State           (uint16_t Pin);
        uint16_t            GetIODIR_State          (uint16_t Pin);

        I2C_Driver*         m_pI2C;
        MCP230xx_Device_e   m_DeviceType;
        uint8_t             m_DeviceAddress;
        uint16_t            m_PinOpenDrainOutput;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_MCP230XX__
#include "device_var.h"
#undef  __CLASS_MCP230XX__

//-------------------------------------------------------------------------------------------------

#else // (USE_I2C_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for I2C must be enable and configure to use this device driver")

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
