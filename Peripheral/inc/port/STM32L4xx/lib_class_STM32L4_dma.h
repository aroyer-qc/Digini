//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_dma.h
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

#define DMA_PRIORITY_LEVEL_LOW              0x0000           // Priority low
#define DMA_PRIORITY_LEVEL_MEDIUM           0x1000           // Priority medium
#define DMA_PRIORITY_LEVEL_HIGH             0x2000           // Priority high
#define DMA_PRIORITY_LEVEL_VERY_HIGH        0x3000           // Priority very high

#define DMA_MEMORY_SIZE_8_BITS              0x0000           // 8 bits
#define DMA_MEMORY_SIZE_16_BITS             0x0400           // 16 bits
#define DMA_MEMORY_SIZE_32_BITS             0x0800           // 32 Bits

#define DMA_PERIPHERAL_SIZE_8_BITS          0x0000           // 8 bits
#define DMA_PERIPHERAL_SIZE_16_BITS         0x0100           // 16 bits
#define DMA_PERIPHERAL_SIZE_32_BITS         0x0200           // 32 Bits

#define DMA_MEMORY_NO_INCREMENT             0x0000           // Memory increment mode Disable
#define DMA_MEMORY_INCREMENT                0x0080           // Memory increment mode Enable

#define DMA_PERIPHERAL_NO_INCREMENT         0x0000           // Peripheral increment mode Disable
#define DMA_PERIPHERAL_INCREMENT            0x0040           // Peripheral increment mode Enable

#define DMA_MODE_NORMAL                     0x0000           // Normal mode
#define DMA_MODE_CIRCULAR                   0x0020           // Circular mode

#define DMA_PERIPHERAL_TO_MEMORY            0x0000           // Peripheral to memory direction
#define DMA_MEMORY_TO_PERIPHERAL            0x0010           // Memory to peripheral or memory direction
#define DMA_MEMORY_TO_MEMORY                0x4000           // Memory to memory mode


#define DMA_HALF_TRANSFERT_IRQ              0x0004           // Half transfer interrupt enable
#define DMA_TRANSFER_COMPLETE_IRQ           0x0002           // Transfer complete enable

#define DMA_START_TRANSFERT                 0x0001

#define DMA_DISABLED                        (0x8000U)       // To disable DMA if not needed Reserved Bits in the register
#define DMA_MASK_FOR_DISABLED               ~(0x8000U)

#if defined(DMAMUX1)
#define DMA_REQUEST_MEM2MEM                 0U
#define DMA_REQUEST_GENERATOR0              1U
#define DMA_REQUEST_GENERATOR1              2U
#define DMA_REQUEST_GENERATOR2              3U
#define DMA_REQUEST_GENERATOR3              4U
#define DMA_REQUEST_ADC1                    5U

#if defined(STM32L4P5xx) || defined(STM32L4Q5xx)
#define DMA_REQUEST_ADC2                    6U
#define DMA_L4_REQUEST_ADC2_SHIFT           1U
#else
#define DMA_L4_REQUEST_ADC2_SHIFT           0U
#endif

#define DMA_REQUEST_DAC1_CH1                (6U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_DAC1_CH2                (7U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM6_UP                 (8U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM7_UP                 (9U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SPI1_RX                 (10U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SPI1_TX                 (11U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SPI2_RX                 (12U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SPI2_TX                 (13U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SPI3_RX                 (14U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SPI3_TX                 (15U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C1_RX                 (16U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C1_TX                 (17U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C2_RX                 (18U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C2_TX                 (19U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C3_RX                 (20U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C3_TX                 (21U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C4_RX                 (22U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_I2C4_TX                 (23U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_USART1_RX               (24U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_USART1_TX               (25U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_USART2_RX               (26U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_USART2_TX               (27U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_USART3_RX               (28U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_USART3_TX               (29U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_UART4_RX                (30U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_UART4_TX                (31U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_UART5_RX                (32U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_UART5_TX                (33U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_LPUART1_RX              (34U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_LPUART1_TX              (35U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SAI1_A                  (36U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SAI1_B                  (37U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SAI2_A                  (38U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_SAI2_B                  (39U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_OCTOSPI1                (40U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_OCTOSPI2                (41U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM1_CH1                (42U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM1_CH2                (43U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM1_CH3                (44U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM1_CH4                (45U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM1_UP                 (46U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM1_TRIG               (47U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM1_COM                (48U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM8_CH1                (49U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM8_CH2                (50U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM8_CH3                (51U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM8_CH4                (52U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM8_UP                 (53U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM8_TRIG               (54U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM8_COM                (55U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM2_CH1                (56U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM2_CH2                (57U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM2_CH3                (58U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM2_CH4                (59U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM2_UP                 (60U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM3_CH1                (61U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM3_CH2                (62U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM3_CH3                (63U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM3_CH4                (64U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM3_UP                 (65U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM3_TRIG               (66U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM4_CH1                (67U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM4_CH2                (68U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM4_CH3                (69U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM4_CH4                (70U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM4_UP                 (71U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM5_CH1                (72U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM5_CH2                (73U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM5_CH3                (74U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM5_CH4                (75U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM5_UP                 (76U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM5_TRIG               (77U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM15_CH1               (78U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM15_UP                (79U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM15_TRIG              (80U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM15_COM               (81U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM16_CH1               (82U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM16_UP                (83U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM17_CH1               (84U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_TIM17_UP                (85U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_DFSDM1_FLT0             (86U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_DFSDM1_FLT1             (87U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_DFSDM1_FLT2             (88U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_DFSDM1_FLT3             (89U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_DCMI                    (90U + DMA_L4_REQUEST_ADC2_SHIFT)
#if defined(PSSI)
#define DMA_REQUEST_DCMI_PSSI               DMA_REQUEST_DCMI
#endif
#define DMA_REQUEST_AES_IN                  (91U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_AES_OUT                 (92U + DMA_L4_REQUEST_ADC2_SHIFT)
#define DMA_REQUEST_HASH_IN                 (93U + DMA_L4_REQUEST_ADC2_SHIFT)
#else
#define DMA_REQUEST_0                       0U
#define DMA_REQUEST_1                       1U
#define DMA_REQUEST_2                       2U
#define DMA_REQUEST_3                       3U
#define DMA_REQUEST_4                       4U
#define DMA_REQUEST_5                       5U
#define DMA_REQUEST_6                       6U
#define DMA_REQUEST_7                       7U
#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct DMA_Info_t
{
    uint32_t             Config;
    uint32_t             Flag;
    DMA_TypeDef*         pDMA;
    DMA_Channel_TypeDef* pDMA_Channel;
    IRQn_Type            IRQn_Channel;
    uint8_t              PreempPrio;
    uint32_t             MUX_Request;
};

//-------------------------------------------------------------------------------------------------
// class(s)
//-------------------------------------------------------------------------------------------------

class DMA_Driver
{
    public:

        void        Initialize                              (DMA_Info_t* pInfo);
        void        SetTransfer                             (void* pSource, void* pDestination, size_t Length);
        void        SetSource                               (void* pSource);
        void        SetDestination                          (void* pDestination);

        void        ClearFlag                               (uint32_t Flag);
        bool        CheckFlag                               (uint32_t Flag);
        void        EnableIRQ                               (void);

        // Inline method
        void        Enable                                  (void)                              { SET_BIT(m_pDMA_Channel->CCR, DMA_CCR_EN);      }
        void        Disable                                 (void)                              { CLEAR_BIT(m_pDMA_Channel->CCR, DMA_CCR_EN);    }
        void        ClearFlag                               (void)                              { ClearFlag(m_Flag);                             }
        size_t      GetLength                               (void)                              { return size_t(m_pDMA_Channel->CNDTR);          }
        void        SetLength                               (size_t Length)                     { m_pDMA_Channel->CNDTR = uint32_t(Length);      }
        void        SetMemoryIncrement                      (void)                              { SET_BIT(m_pDMA_Channel->CCR, DMA_CCR_MINC);    }
        void        SetNoMemoryIncrement                    (void)                              { CLEAR_BIT(m_pDMA_Channel->CCR, DMA_CCR_MINC);  }
        void        SetFifoControl                          (uint32_t Control)                  { m_pDMA->IFCR = Control;                        }
        void        RegisterCallback                        (CallbackInterface* pCallback)      { m_pCallback = pCallback;                       }
        void        EnableInterrupt                         (uint32_t Interrupt)                { SET_BIT(m_pDMA_Channel->CCR, Interrupt);       }
        void        DisableInterrupt                        (uint32_t Interrupt)                { CLEAR_BIT(m_pDMA_Channel->CCR, Interrupt);     }
        void        EnableTransmitCompleteInterrupt         (void)                              { SET_BIT(m_pDMA_Channel->CCR, DMA_CCR_TCIE);    }
        void        DisableTransmitCompleteInterrupt        (void)                              { CLEAR_BIT(m_pDMA_Channel->CCR, DMA_CCR_TCIE);  }
        void        EnableTransmitHalfCompleteInterrupt     (void)                              { SET_BIT(m_pDMA_Channel->CCR, DMA_CCR_HTIE);    }
        void        DisableTransmitHalfCompleteInterrupt    (void)                              { CLEAR_BIT(m_pDMA_Channel->CCR, DMA_CCR_HTIE);  }

    private:

        void        EnableClock                             (void);

      #if defined(DMAMUX1)
        void        CalcDMAMUX_ChannelBaseAndMask           (void);
        void        CalcDMAMUX_RequestGenBaseAndMask        (uint32_t Request);

        DMAMUX_Channel_TypeDef*                             m_pDMAMUX_Channel;
        DMAMUX_ChannelStatus_TypeDef*                       m_pDMAMUX_ChannelStatus;
        uint32_t                                            m_DMAMUX_ChannelStatusMask;
        DMAMUX_RequestGen_TypeDef*                          m_pDMAMUX_RequestGen;
        DMAMUX_RequestGenStatus_TypeDef*                    m_pDMAMUX_RequestGenStatus;
        uint32_t                                            m_DMAMUX_RequestGenStatusMask;
      #endif

        DMA_TypeDef*                m_pDMA;
        DMA_Channel_TypeDef*        m_pDMA_Channel;
        uint32_t                    m_ChannelNumber;
        uint32_t                    m_Flag;
        IRQn_Type                   m_IRQn_Channel;
        uint8_t                     m_PreempPrio;
        uint32_t                    m_Direction;
        CallbackInterface*          m_pCallback;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "dma_var.h"         // Project variable

//-------------------------------------------------------------------------------------------------

