//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_io.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define IO_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  IO_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define NUMBER_OF_IO_PORT               ((uint32_t)11)
#define IO_PORT_MASK_FOR_CLOCK_ENABLE   0x00003C00                  // Keep only offset for each port
#define IO_PORT_SHIFT_FOR_CLOCK_ENABLE  10                          // Need to shift 10 bits to set value from 0 - 7

//-------------------------------------------------------------------------------------------------
//  private variable(s)
//-------------------------------------------------------------------------------------------------

#ifdef IO_IRQ_DEF
IO_PinChangeCallback_t IO_PinChangeCallback[IO_IRQ_NUM] = {nullptr};
#endif

//-------------------------------------------------------------------------------------------------
// private prototype
//-------------------------------------------------------------------------------------------------

#ifdef IO_IRQ_DEF
static void _IO_GetPinInfo (IO_IrqID_e IO_ID, uint32_t* pPinNumber, uint32_t* pPinMask);
#endif

//-------------------------------------------------------------------------------------------------
// private function
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       _IO_GetPinInfo
//
//  Parameter(s):   IO_IRQ_ID        ID of the IO "IRQ" pin definition
//                  pPinNumber       Pointer on variable to return pin number
//                  pPinMask         Pointer on variable to return pin mask
//  Return:         None
//
//  Description:    Get pin number and mask from struct.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
static void _IO_GetPinInfo(IO_IrqID_e IO_IRQ_ID, uint32_t* pPinNumber, uint32_t* pPinMask)
{
    const    IO_IRQ_Properties_t* pIRQ_Properties;
    const    IO_Properties_t*     pIO_Properties;
    uint32_t PinNumber;
    uint32_t PinMask;

    pIRQ_Properties = &IO_IRQ_Properties[IO_IRQ_ID];
    pIO_Properties  = &IO_Properties[pIRQ_Properties->IO_ID];
    PinNumber       = pIO_Properties->PinNumber;
    PinMask         = 1 << PinNumber;

    if(pPinNumber != nullptr) *pPinNumber = PinNumber;
    if(pPinMask   != nullptr) *pPinMask   = PinMask;
}
#endif

//-------------------------------------------------------------------------------------------------
// Public Function
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_PinInit
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Basic pin initialization Using ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PinInit(IO_ID_e IO_ID)
{
    const IO_Properties_t* pIO_Properties;
    GPIO_TypeDef* pPort;
    uint32_t       PinNumber;
    uint32_t       PinMode;
    uint32_t       PinType;
    uint32_t       PinSpeed;
    uint32_t       State;

    pIO_Properties = &IO_Properties[IO_ID];
    pPort          = pIO_Properties->pPort;
    PinNumber      = pIO_Properties->PinNumber;
    PinMode        = pIO_Properties->PinMode;
    PinType        = pIO_Properties->PinType;
    PinSpeed       = pIO_Properties->PinSpeed;
    State          = pIO_Properties->State;

    IO_PinInit(pPort, PinNumber, PinMode, PinType, PinSpeed, State);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_PinInit
//
//  Parameter(s):   pPort
//                  PinNumber
//                  PinMode
//                  PinType
//                  PinSpeed
//                  State                   Default state of the pin if output or alternate function
//
//
//  Return:         None
//
//  Description:    Basic pin initialization using provided configuration.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PinInit(GPIO_TypeDef* pPort, uint32_t PinNumber, uint32_t PinMode, uint32_t PinType, uint32_t PinSpeed, uint32_t State)
{
    uint32_t Pin2BitShift;

    if(pPort != GPIOxx)
    {
        IO_EnableClock(pPort);
        Pin2BitShift = PinNumber << 1;

        // Set pin speed
        pPort->OSPEEDR &= ~(uint32_t)(IO_SPEED_PIN_MASK << Pin2BitShift);
        pPort->OSPEEDR |=  (uint32_t)(PinSpeed          << Pin2BitShift);

        switch(PinMode)
        {
            case IO_MODE_OUTPUT:
            {
                // Preset initial state
                if(State == 0) pPort->BSRRH = (1 << PinNumber);
                else           pPort->BSRRL = (1 << PinNumber);
            }
            break;

            case IO_MODE_ALTERNATE:
            {
                if(PinNumber < 8)
                {
                    pPort->AFR[0] &= ~(uint32_t)(IO_AF_MASK << (PinNumber << 2));
                    pPort->AFR[0] |=  (uint32_t)(State      << (PinNumber << 2));
                }
                else
                {
                    pPort->AFR[1] &= ~(uint32_t)(IO_AF_MASK << ((PinNumber - 8) << 2));
                    pPort->AFR[1] |=  (uint32_t)(State      << ((PinNumber - 8) << 2));
                }
            }
            break;

            // case IO_MODE_ANALOG:  // Nothing to do for analog
            // case IO_MODE_INPUT:     // Nothing to do for input
            default:
            {
            }
            break;
        }

        pPort->PUPDR  &= ~(uint32_t)((IO_TYPE_PIN_PULL_MASK >> 1) << Pin2BitShift);                 // Reset bit for Pull Up
        pPort->PUPDR  |=  (uint32_t)(((PinType & IO_TYPE_PIN_PULL_MASK) >> 1) << Pin2BitShift);     // Set new pull setting

        pPort->OTYPER &= ~(uint32_t)(IO_TYPE_PIN_DRIVE_MASK << PinNumber);                          // Reset bit for Drive type PP or OD
        pPort->OTYPER |=  (uint32_t)((PinType & IO_TYPE_PIN_DRIVE_MASK) << PinNumber);              // Set new type

        pPort->MODER  &= ~(uint32_t)(IO_MODE_PIN_MASK << Pin2BitShift);
        pPort->MODER  |=  (uint32_t)(PinMode          << Pin2BitShift);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_PinInit
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    ID of the IO pin.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PinInitInput(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort;
    uint32_t      PinNumber;

    pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        PinNumber = IO_Properties[IO_ID].PinNumber << 1;
        pPort->MODER &= ~(uint32_t)(IO_MODE_PIN_MASK << PinNumber);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_PinInitOutput
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Initialize the IO In output.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PinInitOutput(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort;
    uint32_t      PinNumber;

    pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        PinNumber = IO_Properties[IO_ID].PinNumber << 1;

        pPort->MODER &= ~(uint32_t)(IO_MODE_PIN_MASK << PinNumber);
        pPort->MODER |=  (uint32_t)(IO_MODE_OUTPUT   << PinNumber);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_SetPinLow
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Sets pin(s) low.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_SetPinLow(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t PinNumber = IO_Properties[IO_ID].PinNumber;
        pPort->BSRRH = (1 << PinNumber);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_SetPinHigh
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Sets pin(s) high.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_SetPinHigh(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t PinNumber = IO_Properties[IO_ID].PinNumber;
        pPort->BSRRL = (1 << PinNumber);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_TogglePin
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Toggles pin(s).
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_TogglePin(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t PinNumber = IO_Properties[IO_ID].PinNumber;
        pPort->ODR ^= (1 << PinNumber);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_SetPin
//
//  Parameter(s):   IO_ID           ID of the IO pin definition
//                  Value           Value to put out on the pin        0 or 1
//  Return:         None
//
//  Description:    Sets pin(s) value.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_SetPin(IO_ID_e IO_ID, bool Value)
{
    if(Value == 0)
    {
        IO_SetPinLow(IO_ID);
    }
    else
    {
        IO_SetPinHigh(IO_ID);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_GetInputPinValue
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         uint32_t        Actual bit value for this input pin on the port
//
//  Description:    Gets input data bit value.
//
//-------------------------------------------------------------------------------------------------
uint32_t IO_GetInputPinValue(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;
    uint32_t      PinValue = 0;

    if(pPort != GPIOxx)
    {
        PinValue = pPort->IDR & (1 << IO_Properties[IO_ID].PinNumber);
    }
    
    return PinValue;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_GetInputPin
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         bool            level on input pin is 0 or 1
//
//  Description:    Gets input data bit.
//
//-------------------------------------------------------------------------------------------------
bool IO_GetInputPin(IO_ID_e IO_ID)
{
    return (IO_GetInputPinValue(IO_ID) != 0) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_GetOutputPin
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         bool            level on internal register output 0 or 1
//
//  Description:    Gets output data bit.
//
//  Note(s):        Get the value in register not actual output... use IO_GetInputPin for this
//
//-------------------------------------------------------------------------------------------------
bool IO_GetOutputPin(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t PinNumber = IO_Properties[IO_ID].PinNumber;

        if((pPort->ODR & (1 << PinNumber)) == 0)
        {
            return false;
        }

        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_IsItValid
//
//  Parameter(s):   IO_ID       ID of the IO pin definition in IO_Properties_t structure
//  Return:         bool        true if valid, false if not
//
//  Description:    Return if this pin is a valid one.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool IO_IsItValid(IO_ID_e IO_ID)
{
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_InitIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to initialize
//                  pCallback       Callback to call when condition of IRQ are met
//  Return:         None
//
//  Description:    Pin initialization for pin using IRQ.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
void IO_InitIRQ(IO_IrqID_e IO_IRQ_ID, IO_PinChangeCallback_t pCallback)
{
    const IO_IRQ_Properties_t* pIRQ_Properties;
    const IO_Properties_t*     pIO_Properties;
    uint32_t                    PriorityGroup;
    uint32_t                   PinNumber;
    uint32_t                   PinMask;

    pIRQ_Properties = &IO_IRQ_Properties[IO_IRQ_ID];
    pIO_Properties  = &IO_Properties[pIRQ_Properties->IO_ID];
    PinNumber       = pIO_Properties->PinNumber;
    PinMask         = 1 << PinNumber;

    // Init The IO for this pin
    IO_PinInit(pIRQ_Properties->IO_ID);

    // Enable EXT Line in SYSCFG
    SYSCFG->EXTICR[PinNumber >> 2] &= ~(((uint32_t)0x0F) << ((PinNumber & 3)) << 2);
    SYSCFG->EXTICR[PinNumber >> 2] |= ((((uint32_t)pIO_Properties->pPort >> 10) & 7) << ((PinNumber & 3) << 2));

    // Disable Event on provided Lines
    CLEAR_BIT(EXTI->EMR, PinMask);

    // Then Enable IT on provided Lines
    SET_BIT(EXTI->IMR, PinMask);

    CLEAR_BIT(EXTI->FTSR, PinMask);             // Disable Falling Trigger on provided Lines
    CLEAR_BIT(EXTI->RTSR, PinMask);             // Disable Rising Trigger on provided Lines

    if((pIRQ_Properties->Trigger & IO_EXTI_TRIGGER_RISING) != 0)
    {
        SET_BIT(EXTI->RTSR, PinMask);           // Enable Rising trigger on provided Lines
    }
    else
    {
        CLEAR_BIT(EXTI->RTSR, PinMask);
    }

    if((pIRQ_Properties->Trigger & IO_EXTI_TRIGGER_FALLING) != 0)
    {
        SET_BIT(EXTI->FTSR, PinMask);           // Enable Falling trigger on provided Lines
    }
    else
    {
        CLEAR_BIT(EXTI->FTSR, PinMask);
    }

    IO_PinChangeCallback[IO_IRQ_ID] = pCallback;

    // Configure interrupt priority for IO
    PriorityGroup = NVIC_GetPriorityGrouping();
    NVIC_SetPriority(pIRQ_Properties->IRQ_Channel, NVIC_EncodePriority(PriorityGroup,6, 0));
    NVIC_EnableIRQ(pIRQ_Properties->IRQ_Channel);

}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_EnableIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to enable
//  Return:         None
//
//  Description:    Enable the IRQ for specify ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
void IO_EnableIRQ(IO_IrqID_e IO_IRQ_ID)
{
    uint32_t PinMask;

    _IO_GetPinInfo(IO_IRQ_ID, nullptr, &PinMask);
    EXTI->PR = PinMask;
    SET_BIT(EXTI->IMR, PinMask);                    // Enable IT on provided Lines
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_EnableIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to disable
//  Return:         None
//
//  Description:    Disable the IRQ for specify ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
void IO_DisableIRQ(IO_IrqID_e IO_IRQ_ID)
{
    uint32_t PinMask;

    _IO_GetPinInfo(IO_IRQ_ID, nullptr, &PinMask);
    CLEAR_BIT(EXTI->IMR, PinMask);                  // Disable IT on provided Lines
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_EnableIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to get the state
//  Return:         IO_ID_e         ID of the IO used
//
//  Description:    Get the IO ID for this IO IRQ ID
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
IO_ID_e IO_GetIO_ID(IO_IrqID_e IO_IRQ_ID)
{
    const IO_IRQ_Properties_t* pIRQ_Properties = &IO_IRQ_Properties[IO_IRQ_ID];
    return pIRQ_Properties->IO_ID;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_EnableIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to get the state
//  Return:         bool            level on input pin 0 or 1
//
//  Description:    Get the state of the IRQ for specify ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
bool IO_GetIRQ_State(IO_IrqID_e IO_IRQ_ID)
{
    uint32_t PinMask;

    _IO_GetPinInfo(IO_IRQ_ID, nullptr, &PinMask);

    if((EXTI->IMR & PinMask) != 0)
    {
        return true;
    }

    return false;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_callBack
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to get the state
//  Return:         None
//
//  Description:    Get the state of the IRQ for specify ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
void IO_CallBack(IO_IrqID_e IO_IRQ_ID)
{
    if(IO_PinChangeCallback[IO_IRQ_ID] != nullptr)
    {
        IO_PinChangeCallback[IO_IRQ_ID](nullptr);
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_PinLowLevelAccess
//
//  Parameter(s):   PortIO
//                  PinNumber
//                  Direction
//                  State
//  Return:         uint32_t
//
//  Description:    Get the state of the IRQ for specify ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint32_t IO_PinLowLevelAccess(uint32_t PortIO, uint32_t PinNumber, uint32_t Direction, uint32_t State)
{
    GPIO_TypeDef* pPort = (GPIO_TypeDef*)IO_Port[PortIO];
    uint32_t      Pin1BitMask;
    uint32_t      Pin2BitShift;

    Pin1BitMask      = 1 << PinNumber;
    Pin2BitShift     = PinNumber << 1;

    IO_EnableClock(pPort);

    if(Direction == IO_MODE_OUTPUT)
    {
        // Preset state
        if(State == 0) pPort->BSRRH = Pin1BitMask;
        else           pPort->BSRRL = Pin1BitMask;
    }

    MODIFY_REG(pPort->MODER, (IO_MODE_PIN_MASK << Pin2BitShift), (Direction << Pin2BitShift));

    return (pPort->IDR >> PinNumber) & 0x01;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       _IO_EnableClock
//
//  Parameter(s):   pPort            Address of the port
//  Return:         None
//
//  Description:    Enable port clock.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_EnableClock(GPIO_TypeDef* pPort)
{
    uint32_t ClockEnable;

    // GPIO clock enable
    ClockEnable   = ((uint32_t)(pPort) & IO_PORT_MASK_FOR_CLOCK_ENABLE);          // Mask bit to keep only enableoffset
    ClockEnable >>= IO_PORT_SHIFT_FOR_CLOCK_ENABLE;
    ClockEnable   = 1 << ClockEnable;                                             // Set bit position
    RCC->AHB1ENR |= ClockEnable;
}

//-------------------------------------------------------------------------------------------------
