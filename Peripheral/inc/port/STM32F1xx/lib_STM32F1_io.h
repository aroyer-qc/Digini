//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F1_io.h
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
// Define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

#define GPIOxx                              ((uint32_t)0x00000000)
#define NUMBER_OF_PIN_PER_PORT              ((uint32_t)16)

#define LED_Init(p)                         IO_PinInit(p)
#define LED_Toggle(p)                       IO_TogglePin(p)

//#define IO_NULL                           ((IO_TypeDef *)nullptr)

// PinMode
#define IO_MODE_ANALOG                      ((uint32_t)0x00000000)      // Analog Mode
#define IO_MODE_INPUT_NO_PULL               ((uint32_t)0x00000004)      // Input Floating Mode
#define IO_MODE_INPUT_WITH_PULL             ((uint32_t)0x00000008)      // Input With Pull Up/Down
#define IO_MODE_OUTPUT_PUSH_PULL_2MHz       ((uint32_t)0x00000002)      // Output Push Pull Mode with IO Speed 2 MHz
#define IO_MODE_OUTPUT_PUSH_PULL_10MHz      ((uint32_t)0x00000001)      // Output Push Pull Mode with IO Speed 10 MHz
#define IO_MODE_OUTPUT_PUSH_PULL_50MHz      ((uint32_t)0x00000003)      // Output Push Pull Mode with IO Speed 50 MHz
#define IO_MODE_OUTPUT_OPEN_DRAIN_2MHz      ((uint32_t)0x00000006)      // Output Open Drain Mode with IO Speed 2 MHz
#define IO_MODE_OUTPUT_OPEN_DRAIN_10MHz     ((uint32_t)0x00000005)      // Output Open Drain Mode with IO Speed 10 MHz
#define IO_MODE_OUTPUT_OPEN_DRAIN_50MHz     ((uint32_t)0x00000007)      // Output Open Drain Mode with IO Speed 50 MHz
#define IO_MODE_ALTERNATE_PUSH_PULL_2MHz    ((uint32_t)0x0000000A)      // Alternate Function Push Pull Mode with IO Speed 2 MHz
#define IO_MODE_ALTERNATE_PUSH_PULL_10MHz   ((uint32_t)0x00000009)      // Alternate Function Push Pull Mode with IO Speed 10 MHz
#define IO_MODE_ALTERNATE_PUSH_PULL_50MHz   ((uint32_t)0x0000000B)      // Alternate Function Push Pull Mode with IO Speed 50 MHz
#define IO_MODE_ALTERNATE_OPEN_DRAIN_2MHz   ((uint32_t)0x0000000E)      // Alternate Function Open Drain Mode with IO Speed 2 MHz
#define IO_MODE_ALTERNATE_OPEN_DRAIN_10MHz  ((uint32_t)0x0000000D)      // Alternate Function Open Drain Mode with IO Speed 10 MHz
#define IO_MODE_ALTERNATE_OPEN_DRAIN_50MHz  ((uint32_t)0x0000000F)      // Alternate Function Open Drain Mode with IO Speed 50 MHz

// When pin mode is IO_MODE_OUTPUT_xxx
#define IO_DEFAULT_OUTPUT_LOW               0
#define IO_DEFAULT_OUTPUT_HIGH              1

// When pin mode is IO_MODE_INPUT_WITH_PULL
#define IO_DEFAULT_PULL_DOWN                0
#define IO_DEFAULT_PULL_UP                  1

// When pin mode IO_MODE_ALTERNATE_xxx or IO_MODE_INPUT_NO_PULL
#define IO_DEFAULT_DONT_CARE                0

// TODO need to be validated
// External trigger
#define IO_EXTI_TRIGGER_NONE            ((uint8_t)0) // No Trigger Mode
#define IO_EXTI_TRIGGER_RISING          ((uint8_t)1) // Trigger Rising Mode
#define IO_EXTI_TRIGGER_FALLING         ((uint8_t)2) // Trigger Falling Mode
#define IO_EXTI_TRIGGER_RISING_FALLING  ((uint8_t)3) // Trigger Rising & Falling Mode

//#define IO_AF_SHIFT                     (8)
//#define IO_AF_MASK                      ((uint32_t)0x0000000F)

//#define IO_EXT_MODE_IT_SHIFT          (12)
#define IO_EXT_MODE_IT_PIN_MASK         ((uint32_t)0x00000003)
#define IO_EXT_MODE_IT_RISING           ((uint32_t)0x00000001)
#define IO_EXT_MODE_IT_FALLING          ((uint32_t)0x00000002)
#define IO_EXT_MODE_IT_BOTH             ((uint32_t)0x00000003)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_IO_AS_ENUM(ENUM_ID, IO_PORT, IO_PIN, IO_MODE, IO_EXTRA) ENUM_ID,
#define EXPAND_X_IO_IRQ_AS_ENUM(ENUM_ID, IO_ID, NUMBER, TRIGGER) ENUM_ID,
#define EXPAND_X_IO_AS_STRUCT_DATA(ENUM_ID,  IO_PORT, IO_PIN, IO_MODE, IO_EXTRA ) \
                                           { IO_PORT, IO_PIN, IO_MODE, IO_EXTRA },
#define EXPAND_X_IO_IRQ_AS_STRUCT_DATA(ENUM_ID, IO_ID, NUMBER, TRIGGER) \
                                              { IO_ID, NUMBER, TRIGGER},

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum IO_ID_e
{
    IO_NOT_DEFINED = -1,
    IO_DEF(EXPAND_X_IO_AS_ENUM)
    IO_NUM,
};

#ifdef IO_IRQ_DEF
enum IO_IrqID_e
{
    IO_IRQ_DEF(EXPAND_X_IO_IRQ_AS_ENUM)
    IO_IRQ_NUM,
};
#endif

struct IO_Properties_t
{
    GPIO_TypeDef*    pPort;
    uint32_t         PinNumber;
    uint32_t         PinMode;
    uint32_t         State;
};

struct IO_IRQ_Properties_t
{
    IO_ID_e         IO_ID;
    IRQn_Type       IRQ_Channel;
    uint32_t        Trigger;
};

typedef void (*IO_PinChangeCallback_t)(void* pArg);

//-------------------------------------------------------------------------------------------------
//   Global const and variables
//-------------------------------------------------------------------------------------------------

#ifdef IO_DRIVER_GLOBAL

const GPIO_TypeDef* IO_Port[NUMBER_OF_IO_PORT] =
{
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
  #if (NUMBER_OF_IO_PORT == 8)
    GPIOF,
    GPIOG,
  #endif
};

const IO_Properties_t IO_Properties[IO_NUM] =
{
    IO_DEF(EXPAND_X_IO_AS_STRUCT_DATA)
};

#ifdef IO_IRQ_DEF
const IO_IRQ_Properties_t IO_IRQ_Properties[IO_IRQ_NUM] =
{
    IO_IRQ_DEF(EXPAND_X_IO_IRQ_AS_STRUCT_DATA)
};
#endif

#else

extern const GPIO_TypeDef*          IO_Port[NUMBER_OF_IO_PORT];
extern const IO_Properties_t        IO_Properties[IO_NUM];

#ifdef IO_IRQ_DEF
extern const IO_IRQ_Properties_t  IO_IRQ_Properties[IO_IRQ_NUM];
#endif

#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void        IO_PinInit                  (IO_ID_e IO_ID);
void        IO_PinInit                  (GPIO_TypeDef* pPort, uint32_t PinNumber, uint32_t PinMode, uint32_t State);
void        IO_PinInitInput             (IO_ID_e IO_ID);
void        IO_PinInitOutput            (IO_ID_e IO_ID);
void        IO_SetPinLow                (IO_ID_e IO_ID);
void        IO_SetPinHigh               (IO_ID_e IO_ID);
void        IO_TogglePin                (IO_ID_e IO_ID);
void        IO_SetPin                   (IO_ID_e IO_ID, bool Value);
bool        IO_GetInputPin              (IO_ID_e IO_ID);
uint32_t    IO_GetInputPinValue         (IO_ID_e IO_ID);
bool        IO_GetOutputPin             (IO_ID_e IO_ID);
void        IO_EnableClock              (GPIO_TypeDef* pPort);
bool        IO_IsItValid                (IO_ID_e IO_ID);
#ifdef IO_IRQ_DEF
void        IO_InitIRQ                  (IO_IrqID_e IO_IRQ_ID, IO_PinChangeCallback_t pCallback);
void        IO_EnableIRQ                (IO_IrqID_e IO_IRQ_ID);
void        IO_DisableIRQ               (IO_IrqID_e IO_IRQ_ID);
IO_ID_e     IO_GetIO_ID                 (IO_IrqID_e IO_IRQ_ID);
bool        IO_GetIRQ_State             (IO_IrqID_e IO_IRQ_ID);
void        IO_CallBack                 (IO_IrqID_e IO_IRQ_ID);
#endif

uint32_t HALIO_PinLowLevelAccess   (uint32_t PortNumber, uint32_t PinNumber, uint32_t Direction, uint32_t State);

//-------------------------------------------------------------------------------------------------
