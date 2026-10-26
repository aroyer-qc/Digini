//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32H5_io.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#define GPIOxx                          NULL
#define NUMBER_OF_IO_PORT               ((uint32_t)11)
#define NUMBER_OF_PIN_PER_PORT          ((uint32_t)16)

#define LED_Init(p)                     IO_PinInit(p)
#define LED_Toggle(p)                   IO_TogglePin(p)

//#define IO_NULL                       ((IO_TypeDef *)nullptr)

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
#define IO_AF_ANALOG                    ((uint8_t)0x00)  // Analog need AF = 0
#define IO_AF_OUTPUT                    ((uint8_t)0x00)  // Output need AF = 0
#define IO_AF0_RTC_50Hz                 ((uint8_t)0x00)  // RTC_50Hz Alternate Function mapping
#define IO_AF0_MCO                      ((uint8_t)0x00)  // MCO (MCO1 and MCO2) Alternate Function mapping
#define IO_AF0_SWJ                      ((uint8_t)0x00)  // SWJ (SWD and JTAG) Alternate Function mapping
#define IO_AF0_TRACE                    ((uint8_t)0x00)  // TRACE Alternate Function mapping
#define IO_AF0_PWR                      ((uint8_t)0x00)  // PWR Alternate Function mapping
//  AF 1 selection
#define IO_AF1_TIM1                     ((uint8_t)0x01)  // TIM1 Alternate Function mapping
#define IO_AF1_TIM2                     ((uint8_t)0x01)  // TIM2 Alternate Function mapping
#define IO_AF1_TIM16                    ((uint8_t)0x01)  // TIM16 Alternate Function mapping
#define IO_AF1_TIM17                    ((uint8_t)0x01)  // TIM17 Alternate Function mapping
#define IO_AF1_LPTIM1                   ((uint8_t)0x01)  // LPTIM1 Alternate Function mapping
//  AF 2 selection
#define IO_AF2_TIM3                     ((uint8_t)0x02)  // TIM3 Alternate Function mapping
#define IO_AF2_TIM4                     ((uint8_t)0x02)  // TIM4 Alternate Function mapping
#define IO_AF2_TIM5                     ((uint8_t)0x02)  // TIM5 Alternate Function mapping
#define IO_AF2_TIM12                    ((uint8_t)0x02)  // TIM12 Alternate Function mapping
#define IO_AF2_TIM15                    ((uint8_t)0x02)  // TIM15 Alternate Function mapping
#define IO_AF2_LPTIM3                   ((uint8_t)0x02)  // LPTIM3 Alternate Function mapping
#define IO_AF2_SAI1                     ((uint8_t)0x02)  // SAI1 Alternate Function mapping
//  AF 3 selection
#define IO_AF3_TIM1                     ((uint8_t)0x03)  // TIM1 Alternate Function mapping
#define IO_AF3_TIM8                     ((uint8_t)0x03)  // TIM8 Alternate Function mapping
#define IO_AF3_LPTIM2                   ((uint8_t)0x03)  // LPTIM2 Alternate Function mapping
#define IO_AF3_LPTIM3                   ((uint8_t)0x03)  // LPTIM3 Alternate Function mapping
#define IO_AF3_I3C1                     ((uint8_t)0x03)  // I3C1 Alternate Function mapping
#define IO_AF3_OCTOSPI                  ((uint8_t)0x03)  // OCTOSPI Alternate Function mapping
#define IO_AF3_LPUART1                  ((uint8_t)0x03)  // LPUART1 Alternate Function mapping
//  AF 4 selection
#define IO_AF4_CEC                      ((uint8_t)0x04)  // CEC Alternate Function mapping
#define IO_AF4_DCM                      ((uint8_t)0x04)  // DCM Alternate Function mapping
#define IO_AF4_I2C1                     ((uint8_t)0x04)  // I2C1 Alternate Function mapping
#define IO_AF4_I2C2                     ((uint8_t)0x04)  // I2C2 Alternate Function mapping
#define IO_AF4_I2C3                     ((uint8_t)0x04)  // I2C3 Alternate Function mapping
#define IO_AF4_I2C4                     ((uint8_t)0x04)  // I2C3 Alternate Function mapping
#define IO_AF4_LPTIM1                   ((uint8_t)0x04)  // LPTIM1 Alternate Function mapping
#define IO_AF4_LPTIM2                   ((uint8_t)0x04)  // LPTIM2 Alternate Function mapping
#define IO_AF4_SPI1                     ((uint8_t)0x04)  // SPI1 Alternate Function mapping
#define IO_AF4_I2S1                     ((uint8_t)0x04)  // I2S1 Alternate Function mapping
#define IO_AF4_TIM15                    ((uint8_t)0x04)  // TIM15 Alternate Function mapping
#define IO_AF4_USART1                   ((uint8_t)0x04)  // USART1 Alternate Function mapping
//  AF 5 selection
#define IO_AF5_CEC                      ((uint8_t)0x05)  // CEC Alternate Function mapping
#define IO_AF5_I3C1                     ((uint8_t)0x05)  // I3C1 Alternate Function mapping
#define IO_AF5_SPI1                     ((uint8_t)0x05)  // SPI1 Alternate Function mapping
#define IO_AF5_SPI2                     ((uint8_t)0x05)  // SPI2/I2S2 Alternate Function mapping
#define IO_AF7_SPI2                     ((uint8_t)0x07)  // SPI2/I2S2 Alternate Function mapping
#define IO_AF5_SPI3                     ((uint8_t)0x05)  // SPI3/I2S3 Alternate Function mapping
#define IO_AF5_SPI4                     ((uint8_t)0x05)  // SPI4 Alternate Function mapping
#define IO_AF5_SPI5                     ((uint8_t)0x05)  // SPI5 Alternate Function mapping
#define IO_AF5_SPI6                     ((uint8_t)0x05)  // SPI6 Alternate Function mapping
#define IO_AF5_LPTIM1                   ((uint8_t)0x05)  // LPTIM1 Alternate Function mapping
#define IO_AF5_I2S1                     ((uint8_t)0x05)  // I2S1 Alternate Function mapping
#define IO_AF5_I2S2                     ((uint8_t)0x05)  // I2S2 Alternate Function mapping
#define IO_AF5_I2S3                     ((uint8_t)0x05)  // I2S3 Alternate Function mapping
//  AF 6 selection
#define IO_AF6_I2C4                     ((uint8_t)0x06)  // I2C4 Alternate Function mapping
#define IO_AF6_OCTOSPI                  ((uint8_t)0x06)  // OCTOSPI Alternate Function mapping
#define IO_AF6_SPI3                     ((uint8_t)0x06)  // SPI3 Alternate Function mapping
#define IO_AF6_SPI4                     ((uint8_t)0x06)  // SPI4 Alternate Function mapping
#define IO_AF6_I2S3                     ((uint8_t)0x06)  // I2S3 Alternate Function mapping
#define IO_AF6_SAI1                     ((uint8_t)0x06)  // SAI1 Alternate Function mapping
#define IO_AF6_UART4                    ((uint8_t)0x06)  // UART4 Alternate Function mapping
#define IO_AF6_UART12                   ((uint8_t)0x06)  // UART12 Alternate Function mapping
#define IO_AF6_USART10                  ((uint8_t)0x06)  // USART10 Alternate Function mapping
#define IO_AF6_USBPD                    ((uint8_t)0x06)  // USBPD Alternate Function mapping
//  AF 7 selection
#define IO_AF7_SDMMC1                   ((uint8_t)0x07)  // SDMMC2 Alternate Function mapping
#define IO_AF7_UART7                    ((uint8_t)0x07)  // UART7 Alternate Function mapping
#define IO_AF7_UART8                    ((uint8_t)0x07)  // UART8 Alternate Function mapping
#define IO_AF7_UART12                   ((uint8_t)0x07)  // UART12 Alternate Function mapping
#define IO_AF7_USART1                   ((uint8_t)0x07)  // USART1 Alternate Function mapping
#define IO_AF7_USART2                   ((uint8_t)0x07)  // USART2 Alternate Function mapping
#define IO_AF7_USART3                   ((uint8_t)0x07)  // USART3 Alternate Function mapping
#define IO_AF7_USART6                   ((uint8_t)0x07)  // USART6 Alternate Function mapping
#define IO_AF7_USART10                  ((uint8_t)0x07)  // USART10 Alternate Function mapping
#define IO_AF7_USART11                  ((uint8_t)0x07)  // USART11 Alternate Function mapping
#define IO_AF7_I2S2                     ((uint8_t)0x07)  // I2S2 Alternate Function mapping
#define IO_AF7_I2S3                     ((uint8_t)0x07)  // I2S3 Alternate Function mapping
#define IO_AF7_SPI2                     ((uint8_t)0x07)  // SPI2 Alternate Function mapping
#define IO_AF7_SPI3                     ((uint8_t)0x07)  // SPI3 Alternate Function mapping
#define IO_AF7_SPI6                     ((uint8_t)0x07)  // SPI6 Alternate Function mapping
//  AF 8 selection
#define IO_AF8_SDMMC1                   ((uint8_t)0x08)  // SDMMC1 Alternate Function mapping
#define IO_AF8_UART4                    ((uint8_t)0x08)  // UART4 Alternate Function mapping
#define IO_AF8_UART5                    ((uint8_t)0x08)  // UART5 Alternate Function mapping
#define IO_AF8_UART8                    ((uint8_t)0x08)  // UART8 Alternate Function mapping
#define IO_AF8_LPUART1                  ((uint8_t)0x08)  // LPUART1 Alternate Function mapping
#define IO_AF8_SAI2                     ((uint8_t)0x08)  // SAI2 Alternate Function mapping
#define IO_AF8_SPI6                     ((uint8_t)0x08)  // SPI6 Alternate Function mapping
//  AF 9 selection
#define IO_AF9_SDMMC2                   ((uint8_t)0x09) // SDMMC2 Alternate Function mapping
#define IO_AF9_FDCAN1                   ((uint8_t)0x09)  // FDCAN1 Alternate Function mapping
#define IO_AF9_FDCAN2                   ((uint8_t)0x09)  // FDCAN2 Alternate Function mapping
#define IO_AF9_FMC                      ((uint8_t)0x09)  // FMC Alternate Function mapping
#define IO_AF9_OCTOSPI                  ((uint8_t)0x09)  // OCTOSPI Alternate Function mapping
#define IO_AF9_TIM13                    ((uint8_t)0x09)  // TIM13 Alternate Function mapping
#define IO_AF9_TIM14                    ((uint8_t)0x09)  // TIM14 Alternate Function mapping
//  AF 10 selection
#define IO_AF10_CRS                     ((uint8_t)0x0A)  // CRS Alternate Function mapping
#define IO_AF10_FMC                     ((uint8_t)0x0A)  // FMC Alternate Function mapping
#define IO_AF10_SDMMC2                  ((uint8_t)0x0A)  // SDMMC2 Alternate Function mapping
#define IO_AF10_OCTOSPI                 ((uint8_t)0x0A)  // OCTOSPI Alternate Function mapping
#define IO_AF10_SAI2                    ((uint8_t)0x0A)  // SAI2 Alternate Function mapping
#define IO_AF10_TIM8                    ((uint8_t)0x0A)  // TIM8 Alternate Function mapping
#define IO_AF10_USB                     ((uint8_t)0x0A)  // USB Alternate Function mapping
//  AF 11 selection
#define IO_AF11_ETH                     ((uint8_t)0x0B)  // ETHERNET Alternate Function mapping
#define IO_AF11_FMC                     ((uint8_t)0x0B)  // FMC Alternate Function mapping
#define IO_AF11_SDMMC2                  ((uint8_t)0x0B)  // SDMMC2 Alternate Function mapping
#define IO_AF11_OCTOSPI                 ((uint8_t)0x0B)  // OCTOSPI Alternate Function mapping
#define IO_AF11_UART7                   ((uint8_t)0x0B)  // UART7 Alternate Function mapping
#define IO_AF11_UART9                   ((uint8_t)0x0B)  // UART9 Alternate Function mapping
#define IO_AF11_USBPD                   ((uint8_t)0x0B)  // USBPD Alternate Function mapping
//  AF 12 selection
#define IO_AF12_FMC                     ((uint8_t)0x0C)  // FMC Alternate Function mapping
#define IO_AF12_SDMMC                   ((uint8_t)0x0C)  // SDMMC Alternate Function mapping
//  AF 13 selection
#define IO_AF13_DCMI                    ((uint8_t)0x0D)  // DCMI Alternate Function mapping
#define IO_AF13_FMC                     ((uint8_t)0x0D)  // FMC Alternate Function mapping
#define IO_AF13_LPTIM5                  ((uint8_t)0x0D)  // LPTIM5 Alternate Function mapping
//  AF 14 selection
#define IO_AF14_LPTIM3                  ((uint8_t)0x0E)  // LPTIM3 Alternate Function mapping
#define IO_AF14_LPTIM4                  ((uint8_t)0x0E)  // LPTIM4 Alternate Function mapping
#define IO_AF14_LPTIM5                  ((uint8_t)0x0E)  // LPTIM5 Alternate Function mapping
#define IO_AF14_LPTIM6                  ((uint8_t)0x0E)  // LPTIM6 Alternate Function mapping
#define IO_AF14_TIM2                    ((uint8_t)0x0E)  // TIM2 Alternate Function mapping
#define IO_AF14_UART5                   ((uint8_t)0x0E)  // UART5 Alternate Function mapping
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
#define EXPAND_X_IO_IRQ_AS_ENUM(ENUM_ID, IO_ID, NUMBER, PRIO, TRIGGER) ENUM_ID,

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef enum
{
    IO_NOT_DEFINED = 0,
    IO_DEF(EXPAND_X_IO_AS_ENUM)
    IO_NUM,
} IO_ID_e;

#ifdef IO_IRQ_DEF
typedef enum
{
    IO_IRQ_DEF(EXPAND_X_IO_IRQ_AS_ENUM)
    IO_IRQ_NUM,
    IO_IRQ_NOT_DEFINED
} IO_IrqID_e;
#endif

typedef void (*IO_PinChangeCallback_t)(void);

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void        IO_PinInit                  (IO_ID_e IO_ID);
void        IO_PinInitInput             (IO_ID_e IO_ID);
void        IO_PinInitOutput            (IO_ID_e IO_ID);
void        IO_SetPinLow                (IO_ID_e IO_ID);
void        IO_SetPinHigh               (IO_ID_e IO_ID);
void        IO_TogglePin                (IO_ID_e IO_ID);
void        IO_SetPin                   (IO_ID_e IO_ID, bool Value);
bool        IO_GetInputPin              (IO_ID_e IO_ID);
uint32_t    IO_GetInputPinValue         (IO_ID_e IO_ID);
bool        IO_GetOutputPin             (IO_ID_e IO_ID);
bool        IO_IsItValid                (IO_ID_e IO_ID);
#ifdef IO_IRQ_DEF
void        IO_PinInitIRQ               (IO_IrqID_e IO_IRQ_ID, IO_PinChangeCallback_t pCallback);
void        IO_EnableIRQ                (IO_IrqID_e IO_IRQ_ID);
void        IO_DisableIRQ               (IO_IrqID_e IO_IRQ_ID);
IO_ID_e     IO_GetIO_ID                 (IO_IrqID_e IO_IRQ_ID);
bool        IO_GetIRQ_State             (IO_IrqID_e IO_IRQ_ID);
#endif

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//-------------------------------------------------------------------------------------------------

#include "bsp_io_var.h"

//-------------------------------------------------------------------------------------------------
