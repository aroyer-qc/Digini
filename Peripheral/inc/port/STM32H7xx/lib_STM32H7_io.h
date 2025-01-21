//-------------------------------------------------------------------------------------------------
//
//  File :  lib_STM32H7_io.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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

#define IO_PIN_0                 	    ((uint16_t)0x0001)
#define IO_PIN_1                 	    ((uint16_t)0x0002)
#define IO_PIN_2                 	    ((uint16_t)0x0004)
#define IO_PIN_3                 	    ((uint16_t)0x0008)
#define IO_PIN_4                 	    ((uint16_t)0x0010)
#define IO_PIN_5                        ((uint16_t)0x0020)
#define IO_PIN_6                        ((uint16_t)0x0040)
#define IO_PIN_7                        ((uint16_t)0x0080)
#define IO_PIN_8                        ((uint16_t)0x0100)
#define IO_PIN_9                        ((uint16_t)0x0200)
#define IO_PIN_10                       ((uint16_t)0x0400)
#define IO_PIN_11                       ((uint16_t)0x0800)
#define IO_PIN_12                       ((uint16_t)0x1000)
#define IO_PIN_13                       ((uint16_t)0x2000)
#define IO_PIN_14                       ((uint16_t)0x4000)
#define IO_PIN_15                       ((uint16_t)0x8000)

#define GPIOxx                          ((uint32_t)0x00000000)
#define NUMBER_OF_IO_PORT               ((uint32_t)9)      //((uint32_t)11)
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

//#define IO_AF_SHIFT                   (8)
#define IO_AF_MASK                      ((uint32_t)0x0000000F)

// AF 0 selection
#define IO_AF0_RTC_50Hz      			((uint8_t)0x00)  // RTC_50Hz Alternate Function mapping
#define IO_AF0_MCO           			((uint8_t)0x00)  // MCO (MCO1 and MCO2) Alternate Function mapping
#define IO_AF0_SWJ           			((uint8_t)0x00)  // SWJ (SWD and JTAG) Alternate Function mapping
#define IO_AF0_LCDBIAS       			((uint8_t)0x00)  // LCDBIAS Alternate Function mapping
#define IO_AF0_TRACE         			((uint8_t)0x00)  // TRACE Alternate Function mapping
#if defined (PWR_CPUCR_PDDS_D2) // PWR D1 and D2 domains exists
#define IO_AF0_C1DSLEEP      			((uint8_t)0x00)  // Cortex-M7 Deep Sleep Alternate Function mapping : available on STM32H7 Rev.B and above
#define IO_AF0_C1SLEEP       			((uint8_t)0x00)  // Cortex-M7 Sleep Alternate Function mapping : available on STM32H7 Rev.B and above
#define IO_AF0_D1PWREN       			((uint8_t)0x00)  // Domain 1 PWR enable Alternate Function mapping : available on STM32H7 Rev.B and above
#define IO_AF0_D2PWREN       			((uint8_t)0x00)  // Domain 2 PWR enable Alternate Function mapping : available on STM32H7 Rev.B and above
#if defined(DUAL_CORE)
#define IO_AF0_C2DSLEEP      			((uint8_t)0x00)  // Cortex-M4 Deep Sleep Alternate Function mapping : available on STM32H7 Rev.B and above
#define IO_AF0_C2SLEEP       			((uint8_t)0x00)  // Cortex-M4 Sleep Alternate Function mapping : available on STM32H7 Rev.B and above
#endif // DUAL_CORE
#endif // PWR_CPUCR_PDDS_D2

// AF 1 selection
#define IO_AF1_TIM1          			((uint8_t)0x01)  // TIM1 Alternate Function mapping
#define IO_AF1_TIM2          			((uint8_t)0x01)  // TIM2 Alternate Function mapping
#define IO_AF1_TIM16         			((uint8_t)0x01)  // TIM16 Alternate Function mapping
#define IO_AF1_TIM17         			((uint8_t)0x01)  // TIM17 Alternate Function mapping
#define IO_AF1_LPTIM1        			((uint8_t)0x01)  // LPTIM1 Alternate Function mapping
#if defined(HRTIM1)
#define IO_AF1_HRTIM1        			((uint8_t)0x01)  // HRTIM1 Alternate Function mapping
#endif // HRTIM1

// AF 2 selection
#define IO_AF2_TIM3                     ((uint8_t)0x02)  // TIM3 Alternate Function mapping
#define IO_AF2_TIM4                     ((uint8_t)0x02)  // TIM4 Alternate Function mapping
#define IO_AF2_TIM5                     ((uint8_t)0x02)  // TIM5 Alternate Function mapping
#define IO_AF2_TIM12                    ((uint8_t)0x02)  // TIM12 Alternate Function mapping
#define IO_AF2_SAI1                     ((uint8_t)0x02)  // SAI1 Alternate Function mapping
#if defined(HRTIM1)
#define IO_AF2_HRTIM1                   ((uint8_t)0x02)  // HRTIM1 Alternate Function mapping
#endif // HRTIM1
#define IO_AF2_TIM15                    ((uint8_t)0x02)  // TIM15 Alternate Function mapping : available on STM32H7A3xxx/STM32H7B3xxx/STM32H7B0xxx

// AF 3 selection
#define IO_AF3_TIM8                     ((uint8_t)0x03)  // TIM8 Alternate Function mapping
#define IO_AF3_LPTIM2                   ((uint8_t)0x03)  // LPTIM2 Alternate Function mapping
#define IO_AF3_DFSDM1                   ((uint8_t)0x03)  // DFSDM Alternate Function mapping
#define IO_AF3_LPTIM3                   ((uint8_t)0x03)  // LPTIM3 Alternate Function mapping
#define IO_AF3_LPTIM4                   ((uint8_t)0x03)  // LPTIM4 Alternate Function mapping
#define IO_AF3_LPTIM5                   ((uint8_t)0x03)  // LPTIM5 Alternate Function mapping
#define IO_AF3_LPUART                   ((uint8_t)0x03)  // LPUART Alternate Function mapping
#if defined(OCTOSPIM)
#define IO_AF3_OCTOSPIM_P1              ((uint8_t)0x03)  // OCTOSPI Manager Port 1 Alternate Function mapping
#define IO_AF3_OCTOSPIM_P2              ((uint8_t)0x03)  // OCTOSPI Manager Port 2 Alternate Function mapping
#endif // OCTOSPIM
#if defined(HRTIM1)
#define IO_AF3_HRTIM1                   ((uint8_t)0x03)  // HRTIM1 Alternate Function mapping
#endif // HRTIM1

// AF 4 selection
#define IO_AF4_I2C                      ((uint8_t)0x04)  // I2C Alternate Function mapping
#define IO_AF4_I2C1                     ((uint8_t)0x04)  // I2C1 Alternate Function mapping
#define IO_AF4_I2C2                     ((uint8_t)0x04)  // I2C2 Alternate Function mapping
#define IO_AF4_I2C3                     ((uint8_t)0x04)  // I2C3 Alternate Function mapping
#define IO_AF4_I2C4                     ((uint8_t)0x04)  // I2C4 Alternate Function mapping
#define IO_AF4_TIM15                    ((uint8_t)0x04)  // TIM15 Alternate Function mapping
#define IO_AF4_CEC                      ((uint8_t)0x04)  // CEC Alternate Function mapping
#define IO_AF4_LPTIM2                   ((uint8_t)0x04)  // LPTIM2 Alternate Function mapping
#define IO_AF4_USART1                   ((uint8_t)0x04)  // USART1 Alternate Function mapping
#define IO_AF4_DFSDM1                   ((uint8_t)0x04)  // DFSDM  Alternate Function mapping
#if defined(DFSDM2_BASE)
#define IO_AF4_DFSDM2                   ((uint8_t)0x04)  // DFSDM2 Alternate Function mapping
#endif // DFSDM2_BASE
#if defined(PSSI)
#define IO_AF4_PSSI                     ((uint8_t)0x04)  // PSSI Alternate Function mapping
#endif // PSSI

// AF 5 selection
#define IO_AF5_SPI                      ((uint8_t)0x05)  // SPI Alternate Function mapping
#define IO_AF5_SPI1                     ((uint8_t)0x05)  // SPI1 Alternate Function mapping
#define IO_AF5_SPI2                     ((uint8_t)0x05)  // SPI2 Alternate Function mapping
#define IO_AF5_SPI3                     ((uint8_t)0x05)  // SPI3 Alternate Function mapping
#define IO_AF5_SPI4                     ((uint8_t)0x05)  // SPI4 Alternate Function mapping
#define IO_AF5_SPI5                     ((uint8_t)0x05)  // SPI5 Alternate Function mapping
#define IO_AF5_SPI6                     ((uint8_t)0x05)  // SPI6 Alternate Function mapping
#define IO_AF5_CEC                      ((uint8_t)0x05)  // CEC  Alternate Function mapping

// AF 6 selection
#define IO_AF6_SPI2                     ((uint8_t)0x06)  // SPI2 Alternate Function mapping
#define IO_AF6_SPI3                     ((uint8_t)0x06)  // SPI3 Alternate Function mapping
#define IO_AF6_SAI1                     ((uint8_t)0x06)  // SAI1 Alternate Function mapping
#define IO_AF6_I2C4                     ((uint8_t)0x06)  // I2C4 Alternate Function mapping
#define IO_AF6_DFSDM1                   ((uint8_t)0x06)  // DFSDM Alternate Function mapping
#define IO_AF6_UART4                    ((uint8_t)0x06)  // UART4 Alternate Function mapping
#if defined(DFSDM2_BASE)
#define IO_AF6_DFSDM2                   ((uint8_t)0x06)  // DFSDM2 Alternate Function mapping
#endif // DFSDM2_BASE
#if defined(SAI3)
#define IO_AF6_SAI3                     ((uint8_t)0x06)  // SAI3 Alternate Function mapping
#endif // SAI3
#if defined(OCTOSPIM)
#define IO_AF6_OCTOSPIM_P1              ((uint8_t)0x06)  // OCTOSPI Manager Port 1 Alternate Function mapping
#endif // OCTOSPIM

// AF 7 selection
#define IO_AF7_SPI2                     ((uint8_t)0x07)  // SPI2 Alternate Function mapping
#define IO_AF7_SPI3                     ((uint8_t)0x07)  // SPI3 Alternate Function mapping
#define IO_AF7_SPI6                     ((uint8_t)0x07)  // SPI6 Alternate Function mapping
#define IO_AF7_USART1                   ((uint8_t)0x07)  // USART1 Alternate Function mapping
#define IO_AF7_USART2                   ((uint8_t)0x07)  // USART2 Alternate Function mapping
#define IO_AF7_USART3                   ((uint8_t)0x07)  // USART3 Alternate Function mapping
#define IO_AF7_USART6                   ((uint8_t)0x07)  // USART6 Alternate Function mapping
#define IO_AF7_UART7                    ((uint8_t)0x07)  // UART7 Alternate Function mapping
#define IO_AF7_DFSDM1                   ((uint8_t)0x07)  // DFSDM Alternate Function mapping
#define IO_AF7_SDMMC1                   ((uint8_t)0x07)  // SDMMC1 Alternate Function mapping

// AF 8 selection
#define IO_AF8_SPI6                     ((uint8_t)0x08)  // SPI6 Alternate Function mapping
#define IO_AF8_SAI2                     ((uint8_t)0x08)  // SAI2 Alternate Function mapping
#define IO_AF8_UART4                    ((uint8_t)0x08)  // UART4 Alternate Function mapping
#define IO_AF8_UART5                    ((uint8_t)0x08)  // UART5 Alternate Function mapping
#define IO_AF8_UART8                    ((uint8_t)0x08)  // UART8 Alternate Function mapping
#define IO_AF8_SPDIF                    ((uint8_t)0x08)  // SPDIF Alternate Function mapping
#define IO_AF8_LPUART                   ((uint8_t)0x08)  // LPUART Alternate Function mapping
#define IO_AF8_SDMMC1                   ((uint8_t)0x08)  // SDMMC1 Alternate Function mapping
#if defined(SAI4)
#define IO_AF8_SAI4                     ((uint8_t)0x08)  // SAI4 Alternate Function mapping
#endif // SAI4

// AF 9 selection
#define IO_AF9_FDCAN1                   ((uint8_t)0x09)  // FDCAN1 Alternate Function mapping
#define IO_AF9_FDCAN2                   ((uint8_t)0x09)  // FDCAN2 Alternate Function mapping
#define IO_AF9_TIM13                    ((uint8_t)0x09)  // TIM13 Alternate Function mapping
#define IO_AF9_TIM14                    ((uint8_t)0x09)  // TIM14 Alternate Function mapping
#define IO_AF9_SDMMC2                   ((uint8_t)0x09)  // SDMMC2 Alternate Function mapping
#define IO_AF9_LTDC                     ((uint8_t)0x09)  // LTDC Alternate Function mapping
#define IO_AF9_SPDIF                    ((uint8_t)0x09)  // SPDIF Alternate Function mapping
#define IO_AF9_FMC                      ((uint8_t)0x09)  // FMC Alternate Function mapping
#if defined(QUADSPI)
#define IO_AF9_QUADSPI                  ((uint8_t)0x09)  // QUADSPI Alternate Function mapping
#endif // QUADSPI
#if defined(SAI4)
#define IO_AF9_SAI4                     ((uint8_t)0x09)  // SAI4 Alternate Function mapping
#endif // SAI4
#if defined(OCTOSPIM)
#define IO_AF9_OCTOSPIM_P1              ((uint8_t)0x09)  // OCTOSPI Manager Port 1 Alternate Function mapping
#define IO_AF9_OCTOSPIM_P2              ((uint8_t)0x09)  // OCTOSPI Manager Port 2 Alternate Function mapping
#endif // OCTOSPIM

// AF 10 selection
#define IO_AF10_SAI2                    ((uint8_t)0x0A)  // SAI2 Alternate Function mapping
#define IO_AF10_SDMMC2                  ((uint8_t)0x0A)  // SDMMC2 Alternate Function mapping
#if defined(USB2_OTG_FS)
#define IO_AF10_OTG2_FS                 ((uint8_t)0x0A)  // OTG2_FS Alternate Function mapping
#endif //USB2_OTG_FS
#define IO_AF10_COMP1                   ((uint8_t)0x0A)  // COMP1 Alternate Function mapping
#define IO_AF10_COMP2                   ((uint8_t)0x0A)  // COMP2 Alternate Function mapping
#if defined(LTDC)
#define IO_AF10_LTDC                    ((uint8_t)0x0A)  // LTDC Alternate Function mapping
#endif //LTDC
#define IO_AF10_CRS_SYNC                ((uint8_t)0x0A)  // CRS Sync Alternate Function mapping : available on STM32H7 Rev.B and above
#if defined(QUADSPI)
#define IO_AF10_QUADSPI                 ((uint8_t)0x0A)  // QUADSPI Alternate Function mapping
#endif // QUADSPI
#if defined(SAI4)
#define IO_AF10_SAI4                    ((uint8_t)0x0A)  // SAI4 Alternate Function mapping
#endif // SAI4
#if !defined(USB2_OTG_FS)
#define IO_AF10_OTG1_FS                 ((uint8_t)0x0A)  // OTG1_FS Alternate Function mapping : available on STM32H7A3xxx/STM32H7B3xxx/STM32H7B0xxx
#endif // !USB2_OTG_FS
#define IO_AF10_OTG1_HS                 ((uint8_t)0x0A)  // OTG1_HS Alternate Function mapping
#if defined(OCTOSPIM)
#define IO_AF10_OCTOSPIM_P1             ((uint8_t)0x0A)  // OCTOSPI Manager Port 1 Alternate Function mapping
#endif // OCTOSPIM
#define IO_AF10_TIM8                    ((uint8_t)0x0A)  // TIM8 Alternate Function mapping

// AF 11 selection
#define IO_AF11_SWP           			((uint8_t)0x0B)  // SWP Alternate Function mapping
#define IO_AF11_MDIOS                   ((uint8_t)0x0B)  // MDIOS Alternate Function mapping
#define IO_AF11_UART7                   ((uint8_t)0x0B)  // UART7 Alternate Function mapping
#define IO_AF11_SDMMC2                  ((uint8_t)0x0B)  // SDMMC2 Alternate Function mapping
#define IO_AF11_DFSDM1                  ((uint8_t)0x0B)  // DFSDM1 Alternate Function mapping
#define IO_AF11_COMP1                   ((uint8_t)0x0B)  // COMP1 Alternate Function mapping
#define IO_AF11_COMP2                   ((uint8_t)0x0B)  // COMP2 Alternate Function mapping
#define IO_AF11_TIM1                    ((uint8_t)0x0B)  // TIM1 Alternate Function mapping
#define IO_AF11_TIM8                    ((uint8_t)0x0B)  // TIM8 Alternate Function mapping
#define IO_AF11_I2C4                    ((uint8_t)0x0B)  // I2C4 Alternate Function mapping
#if defined(DFSDM2_BASE)
#define IO_AF11_DFSDM2                  ((uint8_t)0x0B)  // DFSDM2 Alternate Function mapping
#endif // DFSDM2_BASE
#if defined(USART10)
#define IO_AF11_USART10                 ((uint8_t)0x0B)  // USART10 Alternate Function mapping
#endif // USART10
#if defined(UART9)
#define IO_AF11_UART9                   ((uint8_t)0x0B)  // UART9 Alternate Function mapping
#endif // UART9
#if defined(ETH)
#define IO_AF11_ETH                     ((uint8_t)0x0B)  // ETH Alternate Function mapping
#endif // ETH
#if defined(LTDC)
#define IO_AF11_LTDC                    ((uint8_t)0x0B)  // LTDC Alternate Function mapping : available on STM32H7A3xxx/STM32H7B3xxx/STM32H7B0xxx
#endif //LTDC
#if defined(OCTOSPIM)
#define IO_AF11_OCTOSPIM_P1             ((uint8_t)0x0B)  // OCTOSPI Manager Port 1 Alternate Function mapping
#endif // OCTOSPIM

// AF 12 selection
#define IO_AF12_FMC                     ((uint8_t)0x0C)  // FMC Alternate Function mapping
#define IO_AF12_SDMMC1                  ((uint8_t)0x0C)  // SDMMC1 Alternate Function mapping
#define IO_AF12_MDIOS                   ((uint8_t)0x0C)  // MDIOS Alternate Function mapping
#define IO_AF12_COMP1                   ((uint8_t)0x0C)  // COMP1 Alternate Function mapping
#define IO_AF12_COMP2                   ((uint8_t)0x0C)  // COMP2 Alternate Function mapping
#define IO_AF12_TIM1                    ((uint8_t)0x0C)  // TIM1 Alternate Function mapping
#define IO_AF12_TIM8                    ((uint8_t)0x0C)  // TIM8 Alternate Function mapping
#if defined(LTDC)
#define IO_AF12_LTDC                    ((uint8_t)0x0C)  // LTDC Alternate Function mapping
#endif //LTDC
#if defined(USB2_OTG_FS)
#define IO_AF12_OTG1_FS                 ((uint8_t)0x0C)  // OTG1_FS Alternate Function mapping
#endif // USB2_OTG_FS

// AF 13 selection
#define IO_AF13_DCMI                    ((uint8_t)0x0D)   // DCMI Alternate Function mapping
#define IO_AF13_COMP1                   ((uint8_t)0x0D)   // COMP1 Alternate Function mapping
#define IO_AF13_COMP2                   ((uint8_t)0x0D)   // COMP2 Alternate Function mapping
#if defined(LTDC)
#define IO_AF13_LTDC                    ((uint8_t)0x0D)   // LTDC Alternate Function mapping
#endif //LTDC
#if defined(DSI)
#define IO_AF13_DSI                     ((uint8_t)0x0D)   // DSI Alternate Function mapping
#endif // DSI
#if defined(PSSI)
#define IO_AF13_PSSI                    ((uint8_t)0x0D)   // PSSI Alternate Function mapping
#endif // PSSI
#define IO_AF13_TIM1                    ((uint8_t)0x0D)    // TIM1 Alternate Function mapping

// AF 14 selection
#define IO_AF14_LTDC                    ((uint8_t)0x0E)   // LTDC Alternate Function mapping
#define IO_AF14_UART5                   ((uint8_t)0x0E)   // UART5 Alternate Function mapping

// AF 15 selection
#define IO_AF15_EVENTOUT                ((uint8_t)0x0F)  // EVENTOUT Alternate Function mapping

//#define IO_EXT_MODE_IT_SHIFT          (12)
#define IO_EXT_MODE_IT_PIN_MASK         ((uint32_t)0x00000003)
#define IO_EXT_MODE_IT_RISING           ((uint32_t)0x00000001)
#define IO_EXT_MODE_IT_FALLING          ((uint32_t)0x00000002)
#define IO_EXT_MODE_IT_BOTH             ((uint32_t)0x00000003)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_IO_CFG_AS_ENUM(ENUM_ID, IO_MODE, IO_TYPE, IO_SPEED, IO_EXTRA) ENUM_ID,
#define EXPAND_X_IO_AS_ENUM(ENUM_ID, IO_PORT, IO_PIN, IO_CONFIG) ENUM_ID,
#define EXPAND_X_IO_GROUP_AS_ENUM(ENUM_ID, IO_PORT, IO_GROUP, IO_CONFIG) ENUM_ID,
#define EXPAND_X_IO_IRQ_AS_ENUM(ENUM_ID, IO_ID, NUMBER, PRIO, TRIGGER) ENUM_ID,

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum IO_ConfigID_e
{
    IO_CFG_DEF(EXPAND_X_IO_CFG_AS_ENUM)
    IO_CFG_NUM,
};

enum IO_ID_e
{
    IO_NOT_DEFINED = -1,
    IO_DEF(EXPAND_X_IO_AS_ENUM)
    IO_NUM,
};

#ifdef IO_GROUP_DEF
enum IO_GroupID_e
{
    IO_GROUP_DEF(EXPAND_X_IO_GROUP_AS_ENUM)
    IO_GROUP_NUM,
};


#ifdef IO_IRQ_DEF
enum IO_IrqID_e
{
    IO_IRQ_DEF(EXPAND_X_IO_IRQ_AS_ENUM)
    IO_IRQ_NUM,
};
#endif

typedef void (*IO_PinChangeCallback_t)(void* pArg);

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void        IO_PinInit                  (IO_ID_e IO_ID);
void        IO_PinInitInput             (IO_ID_e IO_ID);
void        IO_PinInitOutput            (IO_ID_e IO_ID);
void        IO_GroupPinInit             (IO_Group_ID_e IO_GroupID);
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
void        IO_CallBack                 (IO_IrqID_e IO_IRQ_ID);
#endif

uint32_t HALIO_PinLowLevelAccess   (uint32_t PortNumber, uint32_t PinNumber, uint32_t Direction, uint32_t State);

//-------------------------------------------------------------------------------------------------
