//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F4_io.h
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
// Define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

#define GPIOxx                          ((uint32_t)0x00000000)
#define NUMBER_OF_IO_PORT               ((uint32_t)11)
#define NUMBER_OF_PIN_PER_PORT          ((uint32_t)16)

#define LED_Init(p)                     IO_PinInit(p)
#define LED_Toggle(p)                   IO_TogglePin(p)

//#define IO_NULL                       ((IO_TypeDef *)nullptr)
#define IO_PIN_MASK_0                   ((uint16_t)0x0001)
#define IO_PIN_MASK_1                   ((uint16_t)0x0002)
#define IO_PIN_MASK_2                   ((uint16_t)0x0004)
#define IO_PIN_MASK_3                   ((uint16_t)0x0008)
#define IO_PIN_MASK_4                   ((uint16_t)0x0010)
#define IO_PIN_MASK_5                   ((uint16_t)0x0020)
#define IO_PIN_MASK_6                   ((uint16_t)0x0040)
#define IO_PIN_MASK_7                   ((uint16_t)0x0080)
#define IO_PIN_MASK_8                   ((uint16_t)0x0100)
#define IO_PIN_MASK_9                   ((uint16_t)0x0200)
#define IO_PIN_MASK_10                  ((uint16_t)0x0400)
#define IO_PIN_MASK_11                  ((uint16_t)0x0800)
#define IO_PIN_MASK_12                  ((uint16_t)0x1000)
#define IO_PIN_MASK_13                  ((uint16_t)0x2000)
#define IO_PIN_MASK_14                  ((uint16_t)0x4000)
#define IO_PIN_MASK_15                  ((uint16_t)0x8000)
#define IO_PIN_MASK_All                 ((uint16_t)0xFFFF)

//#define IO_SPEED_PIN_SHIFT            (5)
#define IO_SPEED_PIN_MASK               ((uint32_t)0x00000003)
#define IO_SPEED_FREQ_LOW               ((uint32_t)0x00000000)
#define IO_SPEED_FREQ_MEDIUM            ((uint32_t)0x00000001)
#define IO_SPEED_FREQ_HIGH              ((uint32_t)0x00000002)
#define IO_SPEED_FREQ_VERY_HIGH         ((uint32_t)0x00000003)

#define IO_MODE_PIN_MASK                ((uint32_t)0x00000003)
#define IO_MODE_INPUT                   ((uint32_t)0x00000000)
#define IO_MODE_OUTPUT                  ((uint32_t)0x00000001)
#define IO_MODE_ALTERNATE               ((uint32_t)0x00000002)
#define IO_MODE_ANALOG                  ((uint32_t)0x00000003)

//#define IO_TYPE_SHIFT                 (2)
#define IO_TYPE_PIN_DRIVE_MASK          ((uint32_t)0x00000001)
#define IO_TYPE_PIN_PP                  ((uint32_t)0x00000000)
#define IO_TYPE_PIN_OD                  ((uint32_t)0x00000001)

//#define IO_PULL_SHIFT                 (3)
#define IO_TYPE_PIN_PULL_MASK           ((uint32_t)0x00000006)
#define IO_TYPE_PIN_NO_PULL             ((uint32_t)0x00000000)
#define IO_TYPE_PIN_PULL_UP             ((uint32_t)0x00000002)
#define IO_TYPE_PIN_PULL_DOWN           ((uint32_t)0x00000004)

// TODO need to be validated
// External trigger
#define IO_EXTI_TRIGGER_NONE            ((uint8_t)0) // No Trigger Mode
#define IO_EXTI_TRIGGER_RISING          ((uint8_t)1) // Trigger Rising Mode
#define IO_EXTI_TRIGGER_FALLING         ((uint8_t)2) // Trigger Falling Mode
#define IO_EXTI_TRIGGER_RISING_FALLING  ((uint8_t)3) // Trigger Rising & Falling Mode

//#define IO_AF_SHIFT                     (8)
#define IO_AF_MASK                      ((uint32_t)0x0000000F)

//  AF 0 selection
#define IO_AF0_RTC_50Hz                 ((uint8_t)0x00)  // RTC_50Hz Alternate Function mapping
#define IO_AF0_MCO                      ((uint8_t)0x00)  // MCO (MCO1 and MCO2) Alternate Function mapping
#define IO_AF0_SWJ                      ((uint8_t)0x00)  // SWJ (SWD and JTAG) Alternate Function mapping
#define IO_AF0_TRACE                    ((uint8_t)0x00)  // TRACE Alternate Function mapping
//  AF 1 selection
#define IO_AF1_TIM1                     ((uint8_t)0x01)  // TIM1 Alternate Function mapping
#define IO_AF1_TIM2                     ((uint8_t)0x01)  // TIM2 Alternate Function mapping
//  AF 2 selection
#define IO_AF2_TIM3                     ((uint8_t)0x02)  // TIM3 Alternate Function mapping
#define IO_AF2_TIM4                     ((uint8_t)0x02)  // TIM4 Alternate Function mapping
#define IO_AF2_TIM5                     ((uint8_t)0x02)  // TIM5 Alternate Function mapping
//  AF 3 selection
#define IO_AF3_TIM8                     ((uint8_t)0x03)  // TIM8 Alternate Function mapping
#define IO_AF3_TIM9                     ((uint8_t)0x03)  // TIM9 Alternate Function mapping
#define IO_AF3_TIM10                    ((uint8_t)0x03)  // TIM10 Alternate Function mapping
#define IO_AF3_TIM11                    ((uint8_t)0x03)  // TIM11 Alternate Function mapping
//  AF 4 selection
#define IO_AF4_I2C1                     ((uint8_t)0x04)  // I2C1 Alternate Function mapping
#define IO_AF4_I2C2                     ((uint8_t)0x04)  // I2C2 Alternate Function mapping
#define IO_AF4_I2C3                     ((uint8_t)0x04)  // I2C3 Alternate Function mapping
//  AF 5 selection
#define IO_AF5_SPI1                     ((uint8_t)0x05)  // SPI1 Alternate Function mapping
#define IO_AF5_SPI2                     ((uint8_t)0x05)  // SPI2/I2S2 Alternate Function mapping
#define IO_AF5_SPI4                     ((uint8_t)0x05)  // SPI4 Alternate Function mapping
#define IO_AF5_SPI5                     ((uint8_t)0x05)  // SPI5 Alternate Function mapping
#define IO_AF5_SPI6                     ((uint8_t)0x05)  // SPI6 Alternate Function mapping
//  AF 6 selection
#define IO_AF6_SPI3                     ((uint8_t)0x06)  // SPI3 Alternate Function mapping
#define IO_AF6_I2S3                     ((uint8_t)0x06)  // I2S3 Alternate Function mapping
#define IO_AF6_SAI1                     ((uint8_t)0x06)  // SAI1 Alternate Function mapping
//  AF 7 selection
#define IO_AF7_USART1                   ((uint8_t)0x07)  // USART1 Alternate Function mapping
#define IO_AF7_USART2                   ((uint8_t)0x07)  // USART2 Alternate Function mapping
#define IO_AF7_USART3                   ((uint8_t)0x07)  // USART3 Alternate Function mapping
#define IO_AF7_I2S3ext                  ((uint8_t)0x07)  // I2S3ext Alternate Function mapping
//  AF 8 selection
#define IO_AF8_UART4                    ((uint8_t)0x08)  // UART4 Alternate Function mapping
#define IO_AF8_UART5                    ((uint8_t)0x08)  // UART5 Alternate Function mapping
#define IO_AF8_USART6                   ((uint8_t)0x08)  // USART6 Alternate Function mapping
#define IO_AF8_UART7                    ((uint8_t)0x08)  // UART7 Alternate Function mapping
#define IO_AF8_UART8                    ((uint8_t)0x08)  // UART8 Alternate Function mapping
//  AF 9 selection
#define IO_AF9_CAN1                     ((uint8_t)0x09)  // CAN1 Alternate Function mapping
#define IO_AF9_CAN2                     ((uint8_t)0x09)  // CAN2 Alternate Function mapping
#define IO_AF9_TIM12                    ((uint8_t)0x09)  // TIM12 Alternate Function mapping
#define IO_AF9_TIM13                    ((uint8_t)0x09)  // TIM13 Alternate Function mapping
#define IO_AF9_TIM14                    ((uint8_t)0x09)  // TIM14 Alternate Function mapping
#define IO_AF9_I2C2                     ((uint8_t)0x09)  // I2C2 Alternate Function mapping (Only for STM32F401xx Devices)
#define IO_AF9_I2C3                     ((uint8_t)0x09)  // I2C3 Alternate Function mapping (Only for STM32F401xx Devices)
//  AF 10 selection
#define IO_AF10_OTG_FS                  ((uint8_t)0x0A)  // OTG_FS Alternate Function mapping
#define IO_AF10_OTG_HS                  ((uint8_t)0x0A)  // OTG_HS Alternate Function mapping
//  AF 11 selection
#define IO_AF11_ETH                     ((uint8_t)0x0B)  // ETHERNET Alternate Function mapping
//  AF 12 selection
#if defined (STM32F40_41xxx)
  #define IO_AF12_FSMC                  ((uint8_t)0x0C)  // FMC Alternate Function mapping
#endif // STM32F40_41xxx
#if defined (STM32F427_437xx) || defined (STM32F429_439xx)
  #define IO_AF12_FMC                   ((uint8_t)0x0C)  // FMC Alternate Function mapping
#endif // STM32F427_437xx || STM32F429_439xx
#define IO_AF12_OTG_HS_FS               ((uint8_t)0x0C)  // OTG HS configured in FS, Alternate Function mapping
#define IO_AF12_SDIO                    ((uint8_t)0x0C)  // SDIO Alternate Function mapping
//  AF 13 selection
#define IO_AF13_DCMI                    ((uint8_t)0x0D)  // DCMI Alternate Function mapping
//  AF 14 selection
#define IO_AF14_LTDC                    ((uint8_t)0x0E)  // LCD-TFT Alternate Function mapping
//  AF 15 selection
#define IO_AF15_EVENTOUT                ((uint8_t)0x0F)  // EVENTOUT Alternate Function mapping

//#define IO_EXT_MODE_IT_SHIFT          (12)
#define IO_EXT_MODE_IT_PIN_MASK         ((uint32_t)0x00000003)
#define IO_EXT_MODE_IT_RISING           ((uint32_t)0x00000001)
#define IO_EXT_MODE_IT_FALLING          ((uint32_t)0x00000002)
#define IO_EXT_MODE_IT_BOTH             ((uint32_t)0x00000003)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_IO_AS_ENUM(ENUM_ID, IO_PORT, IO_PIN, IO_MODE, IO_TYPE, IO_SPEED, IO_EXTRA) ENUM_ID,
#define EXPAND_X_IO_IRQ_AS_ENUM(ENUM_ID, IO_ID, NUMBER, TRIGGER) ENUM_ID,
#define EXPAND_X_IO_AS_STRUCT_DATA(ENUM_ID,  IO_PORT, IO_PIN, IO_MODE, IO_TYPE, IO_SPEED, IO_EXTRA ) \
                                           { IO_PORT, IO_PIN, IO_MODE, IO_TYPE, IO_SPEED, IO_EXTRA },
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
    uint32_t         PinType;
    uint32_t         PinSpeed;
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
    GPIOF,
    GPIOG,
    GPIOH,
    GPIOI,
    GPIOJ,
    GPIOK,
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
void        IO_PinInit                  (GPIO_TypeDef* pPort, uint32_t PinNumber, uint32_t PinMode, uint32_t PinType, uint32_t PinSpeed, uint32_t State); // is it necessary?
void        IO_PinInitInput             (IO_ID_e IO_ID);
void        IO_PinInitOutput            (IO_ID_e IO_ID);
void        IO_SetPinLow                (IO_ID_e IO_ID);
void        IO_SetPinHigh               (IO_ID_e IO_ID);
void        IO_TogglePin                (IO_ID_e IO_ID);
void        IO_SetPin                   (IO_ID_e IO_ID, uint32_t Value);
bool        IO_GetInputPin              (IO_ID_e IO_ID);
bool        IO_GetOutputPin             (IO_ID_e IO_ID);
void        IO_EnableClock              (GPIO_TypeDef* pPort);// is it necessary?
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
