//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_i2c_MCP230xx.cpp
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

#define LIB_MCP230XX_GLOBAL
#include "./Digini/lib_digini.h"
#undef  LIB_MCP230XX_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// General Control Registers
#define IOEXP_INITIAL_IOCON         0x0A    // IOCON initial command address when powered-up
#define IOEXP_IOCONA                0x05    // I/O Expander configuration
#define IOEXP_IOCONB                0x15    // Same functionnality as IOEXP_IOCONA

// ---- Interrupt Control register ----

        // MCP23008 and MCP23017
#define IOEXP_GPINTENA              0x02    // Interrupt on Change: If a bit is set the corresponding pin interrupt is enable (Need DEFVAL and INTCON configured)
#define IOEXP_DEFVALA               0x03
#define IOEXP_INTCONA               0x04
#define IOEXP_INTFA                 0x07    // Interrupt flag:      A set bit indicates that the associated pin caused the interrupt.
#define IOEXP_INTCAPA               0x08    // Interrupt Capture:   The INTCAP register captures the GPIO port (all pins) value at the time the interrupt occurred.
        // Only MCP23017
#define IOEXP_GPINTENB              0x12    // Interrupt on Change: If a bit is set the corresponding pin interrupt is enable (Need DEFVAL and INTCON configured)
#define IOEXP_DEFVALB               0x13
#define IOEXP_INTCONB               0x14
#define IOEXP_INTFB                 0x17    // Interrupt flag:      A set bit indicates that the associated pin caused the interrupt.
#define IOEXP_INTCAPB               0x18    // Interrupt Capture:   The INTCAP register captures the GPIO port (all pins) value at the time the interrupt occurred.

// ---- GPIO Registers ----

        // MCP23008 and MCP23017
#define IOEXP_GPIOA                 0x09    // I/O Port:                The GPIO register reflects the value on the port.
#define IOEXP_IODIRA                0x00    // I/O Direction:           If a bit is set, the corresponding pin becomes an input.
#define IOEXP_IOPOLA                0x01    // Input Polarity:          If a bit is set, the corresponding GPIO register bit will reflect the inverted value on the pin.
#define IOEXP_GPPUA                 0x06    // GPIO Pull-up resistor:   If a bit is set and the corresponding pin is cfg as input, the pin is internally pull-up with a 100k resistor
#define IOEXP_OLATA                 0x0A    // Ouput Latch:             A write to this register modifies the output latches that modifies the pins configured as outputs.
        // Only MCP23017
#define IOEXP_GPIOB                 0x19    // I/O Port:                The GPIO register reflects the value on the port.
#define IOEXP_IODIRB                0x10    // I/O Direction:           If a bit is set, the corresponding pin becomes an input.
#define IOEXP_IOPOLB                0x11    // Input Polarity:          If a bit is set, the corresponding GPIO register bit will reflect the inverted value on the pin.
#define IOEXP_GPPUB                 0x16    // GPIO Pull-up resistor:   If a bit is set and the corresponding pin is cfg as input, the pin is internally pull-up with a 100k resistor
#define IOEXP_OLATB                 0x1A    // Ouput Latch:             A write to this register modifies the output latches that modifies the pins configured as outputs.

// ---- Global control bit ----
#define IOEXP_IOCON_BANK            0x80    // Controls how the registers are addressed
#define IOEXP_IOCON_MIRROR          0x40
#define IOEXP_IOCON_SEQOP           0x20
#define IOEXP_IOCON_DISSLW          0x10
#define IOEXP_IOCON_HAEN            0x08
#define IOEXP_IOCON_ODR_OPEN_DRAIN  0x00
#define IOEXP_IOCON_ODR_PUSH_PULL   0x04
#define IOEXP_IOCON_INTPOL_LOW      0x00
#define IOEXP_IOCON_INTPOL_HIGH     0x02

// ---- Bit port ----
#define IOEXP_PORT_A_MASK           0x00FF
#define IOEXP_PORT_B_MASK           0xFF00
#define IOEXP_PORT_B_SHIFT          8

//-------------------------------------------------------------------------------------------------
//
//   Class: MCP230xx
//
//
//   Description:   Class to handle IO Expander Microchip MCP23008 and MCP23017 with I2C
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   I2C_Driver* pI2C          Pointer on the driver use by this class
//                  uint8_t                   Device Address of the I2C peripheral
//  Return:         SystemState_e
//
//  Description:    Initialize the MCP230xx IO expander
//
//  Note(s):        I2C and interrupt pin on processor are initialize in BSP
//
//-------------------------------------------------------------------------------------------------
SystemState_e MCP230xx::Initialize(I2C_Driver* pI2C, uint8_t DeviceAddress, MCP230xx_Device_e Device)
{
    uint8_t RegisterValue;

    m_pI2C          = pI2C;
    m_DeviceAddress = DeviceAddress;
    m_DeviceType    = Device;

    // Initialize I2C link
    m_pI2C->Initialize();

    m_pI2C->LockToDevice(DeviceAddress);

    // First set register access mode for MCP230017 if it is one
    if(m_DeviceType == MCP23017_DEVICE)
    {
        m_pI2C->ReadRegister(IOEXP_INITIAL_IOCON, &RegisterValue);
        RegisterValue |= IOEXP_IOCON_BANK;
        m_pI2C->WriteRegister(IOEXP_INITIAL_IOCON, RegisterValue);
    }

    RegisterValue = (IOEXP_IOCON_BANK           |                   // Conserve BANK 1
                     IOEXP_IOCON_SEQOP          |                   // Sequential operation mode is disable. this driver is not using this mode.
                     IOEXP_IOCON_ODR_OPEN_DRAIN |                   // Our default is INT open drain
                     IOEXP_IOCON_INTPOL_LOW);                       // Our default is INT active low

    m_pI2C->WriteRegister(IOEXP_IOCONA, RegisterValue);
    m_pI2C->UnlockFromDevice(DeviceAddress);

    ConfigurePinType(IO_PIN_MASK_All, MCP230xx_IO_OPEN_DRAIN);      // Default: all output are open-drain (class control)

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ConfigurePinType
//
//  Parameter(s):
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::ConfigurePinType(uint16_t Pin, MCP230xx_DriveMode_e DriveMode)
{
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           ConfigurePinDirection
//
//  Parameter(s):   Pin                     The number of the pin.
//                  MCP230xx_Direction_e    MCP230xx_IO_DIR_INPUT or MCP230xx_IO_DIR_OUTPUT
//  Return:         None
//
//  Description:    Private method - Set the direction of the pin.
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::ConfigurePinDirection(uint16_t Pin, MCP230xx_Direction_e Direction)
{
    uint8_t RegisterValue;

    m_pI2C->LockToDevice(m_DeviceAddress);

    if((Pin & IOEXP_PORT_A_MASK) != 0)
    {
        m_pI2C->ReadRegister(IOEXP_IODIRA, &RegisterValue);

        if(Direction == MCP230xx_IO_DIR_OUTPUT) RegisterValue &= ~(uint8_t(Pin));
        else                                    RegisterValue |=   uint8_t(Pin);

        m_pI2C->WriteRegister(IOEXP_IODIRA, RegisterValue);
    }
    else if(m_DeviceType == MCP23017_DEVICE)
    {
        if((Pin & IOEXP_PORT_B_MASK) != 0)
        {
            Pin >>= IOEXP_PORT_B_SHIFT;

            m_pI2C->ReadRegister(IOEXP_IODIRB, &RegisterValue);

            if(Direction == MCP230xx_IO_DIR_OUTPUT) RegisterValue &= ~(uint8_t(Pin));
            else                                    RegisterValue |=   uint8_t(Pin);

            m_pI2C->WriteRegister(IOEXP_IODIRB, RegisterValue);
        }
    }

    m_pI2C->UnlockFromDevice(m_DeviceAddress);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ConfigurePinOutputLatch
//
//  Parameter(s):   Pin                 The number of the pin.
//                  MCP230xx_Level_e    Output level.  MCP230xx_IO_LOW or MCP230xx_IO_HIGH
//  Return:         None
//
//  Description:    Private method - Set the latch Value for the output pin.
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::ConfigurePinOutputLatch(uint16_t Pin, MCP230xx_Level_e Output)
{
    uint8_t RegisterValue;

    m_pI2C->LockToDevice(m_DeviceAddress);

    if((Pin & IOEXP_PORT_A_MASK) != 0)
    {
        m_pI2C->ReadRegister(IOEXP_OLATA, &RegisterValue);

        if(Output == MCP230xx_IO_LOW)   RegisterValue &= ~(uint8_t(Pin));
        else                            RegisterValue |=   uint8_t(Pin);

        m_pI2C->WriteRegister(IOEXP_OLATA, RegisterValue);
    }
    else if(m_DeviceType == MCP23017_DEVICE)
    {
        if((Pin & IOEXP_PORT_B_MASK) != 0)
        {
            Pin >>= IOEXP_PORT_B_SHIFT;

            m_pI2C->ReadRegister(IOEXP_OLATB, &RegisterValue);

            if(Output == MCP230xx_IO_LOW)   RegisterValue &= ~(uint8_t(Pin));
            else                            RegisterValue |=   uint8_t(Pin);

            m_pI2C->WriteRegister(IOEXP_OLATB, RegisterValue);
        }
    }

    m_pI2C->UnlockFromDevice(m_DeviceAddress);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetOLAT_State
//
//  Parameter(s):   Pin       The number of the pin.
//  Return:         uint16_t  State of the output latch.                   = 0, MCP230xx_IO_LOW
//                                                                        != 0, MCP230xx_IO_HIGH
//
//  Description:    Private method - Get the state of the latch for the specified pin.
//
//-------------------------------------------------------------------------------------------------
uint16_t MCP230xx::GetOLAT_State(uint16_t Pin)
{
    uint8_t  RegisterValue = 0;
    uint16_t State;

    if((Pin & IOEXP_PORT_A_MASK) != 0)
    {
        m_pI2C->ReadRegister(IOEXP_OLATA, &RegisterValue, m_DeviceAddress);
        State = RegisterValue;
    }
    else if(m_DeviceType == MCP23017_DEVICE)
    {
        if((Pin & IOEXP_PORT_B_MASK) != 0)
        {
            m_pI2C->ReadRegister(IOEXP_OLATB, &RegisterValue, m_DeviceAddress);
            State = uint16_t(RegisterValue) << IOEXP_PORT_B_SHIFT;
        }
    }

    return State & Pin;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetIODIR_State
//
//  Parameter(s):   Pin       The number of the pin.
//  Return:         uint16_t  State of the direction.                  = 0, MCP230xx_IO_DIR_OUTPUT
//                                                                    != 0,  MCP230xx_IO_DIR_INPUT
//
//  Description:    Private method - Get the state of the direction for the specified pin.
//
//-------------------------------------------------------------------------------------------------
uint16_t MCP230xx::GetIODIR_State(uint16_t Pin)
{
    uint8_t  RegisterValue = 0;
    uint16_t State;

    if((Pin & IOEXP_PORT_A_MASK) != 0)
    {
        m_pI2C->ReadRegister(IOEXP_IODIRA, &RegisterValue, m_DeviceAddress);
        State = RegisterValue;
    }
    else if(m_DeviceType == MCP23017_DEVICE)
    {
        if((Pin & IOEXP_PORT_B_MASK) != 0)
        {
            m_pI2C->ReadRegister(IOEXP_IODIRB, &RegisterValue, m_DeviceAddress);
            State = RegisterValue << IOEXP_PORT_B_SHIFT;
        }
    }

    return State & Pin;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PinInitInput
//
//  Parameter(s):
//  Return:         None
//
//  Description:    Set pin in input and configure pull-up mode
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::PinInitInput(uint16_t Pin, MCP230xx_PullUp_e PullEnable)  // Set pin in input
{
    m_pI2C->LockToDevice(m_DeviceAddress);




    m_pI2C->UnlockFromDevice(m_DeviceAddress);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PinInitOutput
//
//  Parameter(s):   Pin                   The number of the pin.
//                  MCP230xx_DriveMode_e  DriveMode MCP230xx_IO_PUSH_PULL or MCP230xx_IO_OPEN_DRAIN
//                  MCP230xx_Level_e      Output    MCP230xx_IO_LOW       or MCP230xx_IO_HIGH
//  Return:         None
//
//  Description:    Set pin in output and configure pin drive mode
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::PinInitOutput(uint16_t Pin, MCP230xx_DriveMode_e DriveMode, MCP230xx_Level_e Output)
{
    m_pI2C->LockToDevice(m_DeviceAddress);




    m_pI2C->UnlockFromDevice(m_DeviceAddress);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetPinLow
//
//  Parameter(s):   Pin                 The number of the pin.
//  Return:         None
//
//  Description:    Set pin low
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::SetPinLow(uint16_t Pin)
{
    SetPin(Pin, MCP230xx_IO_LOW);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetPinHigh
//
//  Parameter(s):   Pin                 The number of the pin.
//  Return:         None
//
//  Description:    Set pin high
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::SetPinHigh(uint16_t Pin)
{
    SetPin(Pin, MCP230xx_IO_HIGH);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TogglePin
//
//  Parameter(s):   Pin                 The number of the pin.
//  Return:         None
//
//  Description:    Toggle pin
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::TogglePin(uint16_t Pin)
{
    uint16_t         State;
    MCP230xx_Level_e Level;

    if((m_PinOpenDrainOutput & Pin) == Pin)
    {
        State = GetIODIR_State(Pin);                 // Get pin actual pin output from open drain

    }
    else
    {
        State = GetOLAT_State(Pin);                 // Get pin actual pin output from push-pull
    }

    Level = (State == 0) ? MCP230xx_IO_HIGH : MCP230xx_IO_LOW;
    SetPin(Pin, Level);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetPin
//
//  Parameter(s):   Pin                 The number of the pin.
//                  MCP230xx_Level_e    Output level.  MCP230xx_IO_LOW or MCP230xx_IO_HIGH
//  Return:         None
//
//  Description:    Set pin to specific level
//
//  Note(s)         According to DriveMode (MCP230xx_IO_PUSH_PULL or MCP230xx_IO_OPEN_DRAIN)
//                  The method to drive the pin differ in control.
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::SetPin(uint16_t Pin, MCP230xx_Level_e Output)
{
    if((m_PinOpenDrainOutput & Pin) == Pin)          // Drive in open drain
    {
        if(Output == MCP230xx_IO_LOW)   { ConfigurePinDirection(Pin, MCP230xx_IO_DIR_OUTPUT); }
        else                            { ConfigurePinDirection(Pin, MCP230xx_IO_DIR_INPUT);  }
    }
    else                                            // Drive in push-pull
    {
        ConfigurePinOutputLatch(Pin, Output);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadPin
//
//  Parameter(s):   Pin                 The number of the pin.
//
//  Return:         MCP230xx_Level_e    MCP230xx_IO_LOW or MCP230xx_IO_HIGH
//
//  Description:    Read the level on the pin (work in both input and output)
//
// Note(s):         It must be a single pin, otherwise result will be unpredictable.
//
//-------------------------------------------------------------------------------------------------
MCP230xx_Level_e MCP230xx::ReadPin(uint16_t Pin)
{
    MCP230xx_Level_e    Level;
    uint8_t             RegisterValue = 0;
    uint16_t            PinRegisterValue = 0;

    if((Pin & IOEXP_PORT_A_MASK) != 0)
    {
        m_pI2C->ReadRegister(IOEXP_GPIOA, &RegisterValue, m_DeviceAddress);
        PinRegisterValue = RegisterValue & Pin;
    }
    else if(m_DeviceType == MCP23017_DEVICE)
    {
        if((Pin & IOEXP_PORT_B_MASK) != 0)
        {
            m_pI2C->ReadRegister(IOEXP_GPIOB, &RegisterValue, m_DeviceAddress);
            PinRegisterValue = (RegisterValue << IOEXP_PORT_B_SHIFT) & Pin;
        }
    }

    Level = (PinRegisterValue == 0) ? MCP230xx_IO_LOW : MCP230xx_IO_HIGH;

    return Level;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ConfigurePinIntType
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::ConfigurePinIntType(MCP230xx_DriveMode_e DriveMode, MCP230xx_Polarity_e Polarity)
{
}






//-------------------------------------------------------------------------------------------------
//
//  Name:           SetIRQ_Enable
//
//  Parameter(s):   bool        State      State of the interupt
//
//                                              - Could be DEF_ENABLE or DEF_DISABLE.
//
//  Return:         none
//
//  Description:    Enables or disables the interrupt.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::SetIRQ_Enable(bool State)
{
    m_pI2C->LockToDevice(m_DeviceAddress);
//    m_pI2C->Read(INT_CTRL, &Register);                                  // Read the Interrupt Control register

//    if(State != DEF_DISABLE)  Register |=  GIT_EN;                      // Set the global interrupts to be Enabled
    //else                      Register &= ~GIT_EN;                      // Set the global interrupts to be Disabled

    //m_pI2C->Write(INT_CTRL, Register);                                  // Write Back the Interrupt Control register
    m_pI2C->UnlockFromDevice(m_DeviceAddress);
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           GetIRQ_Status
//
//  Parameter(s):   uint8_t  IRQ_Flag       The interrupt source to be checked, could be:
//
//                             - IRQ_FLAG_ADC    : ADC interrupt
//                             - IRQ_FLAG_T_TEMP : Temperature Sensor interrupts
//                             - IRQ_FLAG__FE    : Touch Panel Controller FIFO Error interrupt
//                             - IRQ_FLAG__FF    : Touch Panel Controller FIFO Full interrupt
//                             - IRQ_FLAG__FOV   : Touch Panel Controller FIFO Overrun interrupt
//                             - IRQ_FLAG__FTH   : Touch Panel Controller FIFO Threshold interrupt
//                             - IRQ_FLAG__TOUCH : Touch Panel Controller Touch Detected interrupt
//
//  Return:         bool    Status          Status of the checked flag. Could be DEF_SET/DEF_RESET.
//
//  Description:    Checks the selected Global interrupt source pending bit
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool MCP230xx::GetIRQ_Status(uint8_t IRQ_Flag)
{
//    uint8_t Register;

   // m_pI2C->Read(INT_STA, &Register, m_pDevice);                    // Get the Interrupt status
//    if((Register & (uint8_t)IRQ_Flag) != 0)
//    {
//        return true;
//    }

  return false;
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearIRQ_Status
//
//  Parameter(s):   uint8_t  IRQ_Flag       The Global interrupt to be cleared, could be any
//                                          combination of the following values:
//
//                             - IRQ_FLAG_ADC    : ADC interrupt
//                             - IRQ_FLAG_T_TEMP : Temperature Sensor interrupts
//                             - IRQ_FLAG__FE    : Touch Panel Controller FIFO Error interrupt
//                             - IRQ_FLAG__FF    : Touch Panel Controller FIFO Full interrupt
//                             - IRQ_FLAG__FOV   : Touch Panel Controller FIFO Overrun interrupt
//                             - IRQ_FLAG__FTH   : Touch Panel Controller FIFO Threshold interrupt
//                             - IRQ_FLAG__TOUCH : Touch Panel Controller Touch Detected interrupt
//
//  Return:         none
//
//  Description:    Clears the selected Global interrupt pending bit(s)
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void MCP230xx::ClearIRQ_Status(uint8_t IRQ_Flag)
{
   // m_pI2C->Write(INT_STA, IRQ_Flag, m_pDevice);                    // Write 1 to the bits that have to be cleared
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
