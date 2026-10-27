//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H5_io.cpp
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
#include "BSP_board.h"
#undef  IO_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define IO_PORT_MASK_FOR_CLOCK_ENABLE   0x00003C00 // Keep only offset for each port
#define IO_PORT_SHIFT_FOR_CLOCK_ENABLE  10         // Need to shift 10 bits to set value from 0 - 7
#define IO_RESET_BIT_SHIFT              16

#define IO_MAX_IRQ_COUNT                ((EXTI_LINE_15 & EXTI_PIN_MASK) + 1)
#define IO_GET_INDEX(__GPIOx__)         (((uint32_t )(__GPIOx__) & (~GPIOA_BASE)) >> 10)

#ifndef  REMOVE_IO_EXTI_IRQ_HANDLERS
  #ifdef IO_IRQ_DEF
    #define IO_EXTI_IRQ_HANDLERS(X, ...) \
      void EXTI ## X ##_IRQHandler(void) \
      { \
          _IO_HandleExtiInterrupt(X); \
      } \
  //
  #endif
#endif

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_IO_AS_STRUCT_DATA(ENUM_ID,  IO_PORT, IO_PIN, IO_MODE, IO_TYPE, IO_SPEED, IO_EXTRA ) \
                                           { IO_PORT, IO_PIN, IO_MODE, IO_TYPE, IO_SPEED, IO_EXTRA },

#ifdef IO_IRQ_DEF
#define EXPAND_X_IO_IRQ_AS_STRUCT_DATA(ENUM_ID, IO, NUMBER, PRIO, TRIGGER) \
                                              { \
                                                .IO_ID = IO, \
                                                .IRQ_Channel = NUMBER, \
                                                WHEN(USE_FREERTOS) ( .PreemptionPriority = PRIO, ) \
                                                .Trigger = TRIGGER\
                                              },
#define EXPAND_X_IO_IRQ_AS_CALLBACK_INIT(...) NULL,
#endif

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef struct
{
    GPIO_TypeDef*    pPort;
    uint32_t         PinNumber;
    uint32_t         PinMode;
    uint32_t         PinType;
    uint32_t         PinSpeed;
    uint32_t         State;
} IO_Properties_t;

typedef struct
{
    IO_ID_e         IO_ID;
    IRQn_Type       IRQ_Channel;
    uint8_t         PreemptionPriority;
    uint32_t        Trigger;
} IO_IRQ_Properties_t;

//-------------------------------------------------------------------------------------------------
//  private variable(s)
//-------------------------------------------------------------------------------------------------

const GPIO_TypeDef* IO_Port[NUMBER_OF_IO_PORT] =
{
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    GPIOF,
    GPIOG,
    GPIOH,
    GPIOI
};

const IO_Properties_t IO_Properties[IO_NUM] =
{
    { GPIOxx, 0, 0, 0, 0, 0 },               // IO_NOT_DEFINED
    IO_DEF(EXPAND_X_IO_AS_STRUCT_DATA)
};

#ifdef IO_IRQ_DEF

const IO_IRQ_Properties_t IO_IRQ_Properties[IO_IRQ_NUM] =
{
    IO_IRQ_DEF(EXPAND_X_IO_IRQ_AS_STRUCT_DATA)
};

IO_PinChangeCallback_t IO_Callback[IO_IRQ_NUM] =
{
    IO_IRQ_DEF(EXPAND_X_IO_IRQ_AS_CALLBACK_INIT)
};

IO_IrqID_e IO_IRQ_ReverseLookup[IO_MAX_IRQ_COUNT] = { [0 ... IO_MAX_IRQ_COUNT-1] = IO_IRQ_NOT_DEFINED };

#endif

//-------------------------------------------------------------------------------------------------
// private prototype
//-------------------------------------------------------------------------------------------------

static void _IO_EnableClock        (GPIO_TypeDef* pPort);

#ifdef IO_IRQ_DEF
static void _IO_GetPinInfo         (IO_IrqID_e IO_ID, uint32_t* pPinNumber, uint32_t* pPinMask);
static void _IO_HandleExtiInterrupt(uint8_t PinNumber);
#endif

//-------------------------------------------------------------------------------------------------
// private function
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       _IO_EnableClock
//
//  Parameter(s):   pPort            Address of the port
//  Return:         None
//
//  Description:    Enables an IO's port clock.
//
//-------------------------------------------------------------------------------------------------
static void _IO_EnableClock(GPIO_TypeDef* pPort)
{
    uint32_t ClockEnable;

    // GPIO clock enable
    ClockEnable   = ((uint32_t)(pPort) & IO_PORT_MASK_FOR_CLOCK_ENABLE);          // Mask bit to keep only enableoffset
    ClockEnable >>= IO_PORT_SHIFT_FOR_CLOCK_ENABLE;
    ClockEnable   = 1 << ClockEnable;                                             // Set bit position
    RCC->AHB2ENR |= ClockEnable;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       _IO_GetPinInfo
//
//  Parameter(s):   IO_IRQ_ID        ID of the IO "IRQ" pin definition
//                  pPinNumber       Pointer on variable to return pin number
//                  pPinMask         Pointer on variable to return pin mask
//  Return:         None
//
//  Description:    Gets the pin number and mask from struct.
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

    if(pPinNumber != NULL) *pPinNumber = PinNumber;
    if(pPinMask   != NULL) *pPinMask   = PinMask;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       _IO_HandleExtiInterrupt
//
//  Parameter(s):   PinNumber        Pin number of the IO that caused the interrupt
//  Return:         None
//
//  Description:    Handles the interrupt of a given IO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
static void _IO_HandleExtiInterrupt(uint8_t PinNumber)
{
    bool callbackNeeded = false;
    IO_IrqID_e io_id = IO_IRQ_ReverseLookup[PinNumber];
    uint32_t pinMask = (1 << PinNumber);

    // Check for rising edge
    if(EXTI->RPR1 & pinMask)
    {
        // Clear the rising edge pending bit
        EXTI->RPR1 = pinMask;
        callbackNeeded = true;
    }

    // Check for falling edge
    if(EXTI->FPR1 & pinMask)
    {
        // Clear the falling edge pending bit
        EXTI->FPR1 = pinMask;
        callbackNeeded = true;
    }

    if(callbackNeeded && (io_id != IO_IRQ_NOT_DEFINED))
    {
        if (IO_Callback[io_id] != NULL)
        {
            IO_Callback[io_id]();
        }
    }
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
//  Description:    Basic pin initialization using the ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PinInit(IO_ID_e IO_ID)
{
    const IO_Properties_t* pIO_Properties = &IO_Properties[IO_ID];
    GPIO_TypeDef*          pPort          = pIO_Properties->pPort;

    if(pPort != GPIOxx)
    {
        uint32_t PinNumber = pIO_Properties->PinNumber;
        uint32_t PinMode   = pIO_Properties->PinMode;
        uint32_t PinType   = pIO_Properties->PinType;
        uint32_t PinSpeed  = pIO_Properties->PinSpeed;
        uint32_t State     = pIO_Properties->State;

        _IO_EnableClock(pPort);
        uint32_t Pin2BitShift = PinNumber << 1;

        // Set pin speed
        pPort->OSPEEDR &= ~(uint32_t)(IO_SPEED_PIN_MASK << Pin2BitShift);
        pPort->OSPEEDR |=  (uint32_t)(PinSpeed          << Pin2BitShift);

        if(PinMode == IO_MODE_OUTPUT)
        {
            // Reset alternate function
            IO_SetPin(IO_ID, State);
            State = IO_AF_OUTPUT;
        }

        // Set alternate function
        SET_4BIT_REG(pPort->AFR[PinNumber >> 3], State, (PinNumber & 7));

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
//  Function:       IO_PinInitInput
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Changes only the mode of a pin to input.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PinInitInput(IO_ID_e IO_ID)
{
    ASSERT(IO_ID < IO_NUM);
    GPIO_TypeDef* pPort;

    pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t Pin2BitShift = IO_Properties[IO_ID].PinNumber << 1;
        pPort->MODER  &= ~(uint32_t)(IO_MODE_PIN_MASK << Pin2BitShift);
        pPort->MODER  |=  (uint32_t)(IO_MODE_OUTPUT   << Pin2BitShift);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_PinInitOutput
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Changes only the mode of a pin to output.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_PinInitOutput(IO_ID_e IO_ID)
{
    ASSERT(IO_ID < IO_NUM);
    GPIO_TypeDef* pPort;

    pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t Pin2BitShift = IO_Properties[IO_ID].PinNumber << 1;
        pPort->MODER  &= ~(uint32_t)(IO_MODE_PIN_MASK << Pin2BitShift);
        pPort->MODER  |=  (uint32_t)(IO_MODE_OUTPUT   << Pin2BitShift);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_SetPinLow
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Sets a pin low.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_SetPinLow(IO_ID_e IO_ID)
{
    ASSERT(IO_ID < IO_NUM);
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t PinNumber = IO_Properties[IO_ID].PinNumber;
        pPort->BSRR = (1 << (PinNumber + IO_RESET_BIT_SHIFT));
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_SetPinHigh
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Sets a pin high.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_SetPinHigh(IO_ID_e IO_ID)
{
    ASSERT(IO_ID < IO_NUM);
    GPIO_TypeDef* pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        uint32_t PinNumber = IO_Properties[IO_ID].PinNumber;
        pPort->BSRR = (1 << PinNumber);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_TogglePin
//
//  Parameter(s):   IO_ID           ID of the IO pin definition in IO_Properties_t structure
//  Return:         None
//
//  Description:    Toggles a pin.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void IO_TogglePin(IO_ID_e IO_ID)
{
    ASSERT(IO_ID < IO_NUM);
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
//  Description:    Sets a pin to a given value.
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
    ASSERT(IO_ID < IO_NUM);
    uint32_t      PinValue = 0;
    GPIO_TypeDef* pPort    = IO_Properties[IO_ID].pPort;

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
    ASSERT(IO_ID < IO_NUM);
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
    GPIO_TypeDef* pPort;
    
    if (IO_ID >= IO_NUM) {
        return false;
    }

    pPort = IO_Properties[IO_ID].pPort;

    if(pPort != GPIOxx)
    {
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_PinInitIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to initialize
//                  pCallback       Callback to call when condition of IRQ are met
//                  pArg            Optional parameter to pass information to callback function
//  Return:         None
//
//  Description:    Configures a pin to trigger on edges using IRQs.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
void IO_PinInitIRQ(IO_IrqID_e IO_IRQ_ID, IO_PinChangeCallback_t pCallback)
{
    ASSERT(IO_IRQ_ID < IO_IRQ_NUM);
    const IO_IRQ_Properties_t* pIRQ_Properties;
    const IO_Properties_t*     pIO_Properties;
    uint32_t                   PinNumber;
    uint32_t                   PinMask;
    uint32_t                   ExtiShift;

    pIRQ_Properties = &IO_IRQ_Properties[IO_IRQ_ID];
    pIO_Properties  = &IO_Properties[pIRQ_Properties->IO_ID];
    PinNumber       = pIO_Properties->PinNumber;
    PinMask         = 1 << PinNumber;
    ExtiShift       = (PinNumber & 0x3U) << 3;

    // Init The IO for this pin
    IO_PinInit(pIRQ_Properties->IO_ID);

    // Enable EXTI Line
    EXTI->EXTICR[PinNumber >> 2] &= ~(((uint32_t)0xFF) << ExtiShift);
    EXTI->EXTICR[PinNumber >> 2] |= ((((uint32_t)pIO_Properties->pPort >> 10) & 7) << ExtiShift);

    // Disable Event and IT on provided Lines
    CLEAR_BIT(EXTI->EMR1, PinMask);
    CLEAR_BIT(EXTI->IMR1, PinMask);    

    // Disable the triggers (rising and falling)
    CLEAR_BIT(EXTI->FTSR1, PinMask);
    CLEAR_BIT(EXTI->RTSR1, PinMask);

    // Configure the requested trigger(s)
    if((pIRQ_Properties->Trigger & IO_EXTI_TRIGGER_RISING) != 0)
    {
        SET_BIT(EXTI->RTSR1, PinMask);           // Enable Rising trigger on provided Lines
    }

    if((pIRQ_Properties->Trigger & IO_EXTI_TRIGGER_FALLING) != 0)
    {
        SET_BIT(EXTI->FTSR1, PinMask);           // Enable Falling trigger on provided Lines
    }

    // If we are setting the callback for the same IO, we should check that we are not using a different callback
    // i.e. we are making sure that there is no unexpected behavior if two different modules attempt to configure
    //      the same IO with different callback functions
    ASSERT(pCallback == NULL || IO_Callback[IO_IRQ_ID] == NULL || IO_Callback[IO_IRQ_ID] == pCallback);
    if (pCallback != NULL)
    {
        IO_Callback[IO_IRQ_ID] = pCallback;
    }

    // Assign IO_ID to reverse lookup table (only a single pin can have an interrupt registered)
    ASSERT((IO_IRQ_ReverseLookup[PinNumber] == IO_IRQ_NOT_DEFINED) || (IO_IRQ_Properties[IO_IRQ_ReverseLookup[PinNumber]].IO_ID == pIRQ_Properties->IO_ID));
    IO_IRQ_ReverseLookup[PinNumber] = IO_IRQ_ID;

    // Configure interrupt priority for IO
    ISR_Init(pIRQ_Properties->IRQ_Channel, pIRQ_Properties->PreemptionPriority);
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_EnableIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to enable
//  Return:         None
//
//  Description:    Enables the IRQ for a specific ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
void IO_EnableIRQ(IO_IrqID_e IO_IRQ_ID)
{
    ASSERT(IO_IRQ_ID < IO_IRQ_NUM);
    uint32_t PinMask;

    _IO_GetPinInfo(IO_IRQ_ID, NULL, &PinMask);
    EXTI->RPR1 = PinMask;
    EXTI->FPR1 = PinMask;

    SET_BIT(EXTI->IMR1, PinMask);                    // Enable IT on provided Lines
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_DisableIRQ
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to disable
//  Return:         None
//
//  Description:    Disables the IRQ for specify ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
void IO_DisableIRQ(IO_IrqID_e IO_IRQ_ID)
{
    ASSERT(IO_IRQ_ID < IO_IRQ_NUM);
    uint32_t PinMask;

    _IO_GetPinInfo(IO_IRQ_ID, NULL, &PinMask);
    CLEAR_BIT(EXTI->IMR1, PinMask);                  // Disable IT on provided Lines
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_GetIO_ID
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to get the state
//  Return:         IO_ID_e         ID of the IO used
//
//  Description:    Gets the IO ID for this IO IRQ ID.
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
IO_ID_e IO_GetIO_ID(IO_IrqID_e IO_IRQ_ID)
{
    ASSERT(IO_IRQ_ID < IO_IRQ_NUM);
    const IO_IRQ_Properties_t* pIRQ_Properties = &IO_IRQ_Properties[IO_IRQ_ID];
    return pIRQ_Properties->IO_ID;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       IO_GetIRQ_State
//
//  Parameter(s):   IO_IRQ_ID       ID of the IRQ to get the state
//  Return:         bool            level on input pin 0 or 1
//
//  Description:    Gets the state of the IRQ for a specific ID.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef IO_IRQ_DEF
bool IO_GetIRQ_State(IO_IrqID_e IO_IRQ_ID)
{
    ASSERT(IO_IRQ_ID < IO_IRQ_NUM);
    uint32_t PinMask;

    _IO_GetPinInfo(IO_IRQ_ID, NULL, &PinMask);

    if((EXTI->IMR1 & PinMask) != 0)
    {
        return true;
    }

    return false;
}

// Defines all IRQ Handlers for IOs
#ifndef  REMOVE_IO_EXTI_IRQ_HANDLERS
EVAL(REPEAT(16, IO_EXTI_IRQ_HANDLERS))
#endif

#endif

//-------------------------------------------------------------------------------------------------
