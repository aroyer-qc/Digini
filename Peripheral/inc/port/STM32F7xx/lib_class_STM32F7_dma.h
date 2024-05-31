//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_dma.h
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

#define DMA_MODE_NORMAL                0x00000000        // Normal Mode
#define DMA_MODE_CIRCULAR              DMA_SxCR_CIRC     // Circular Mode
#define DMA_MODE_PERIPHERAL_FLOW_CTRL  DMA_SxCR_PFCTRL   // Peripheral flow control mode

#define DMA_PERIPHERAL_NO_INCREMENT    0x00000000        // Peripheral increment mode Disable
#define DMA_PERIPHERAL_INCREMENT       DMA_SxCR_PINC     // Peripheral increment mode Enable

#define DMA_MEMORY_NO_INCREMENT        0x00000000        // Memory increment mode Disable
#define DMA_MEMORY_INCREMENT           DMA_SxCR_MINC     // Memory increment mode Enable

#define DMA_PERIPHERAL_SIZE_8_BITS     0x00000000        // Peripheral data alignment : uint8_t
#define DMA_PERIPHERAL_SIZE_16_BITS    DMA_SxCR_PSIZE_0  // Peripheral data alignment : uint16_t
#define DMA_PERIPHERAL_SIZE_32_BITS    DMA_SxCR_PSIZE_1  // Peripheral data alignment : uint32_t

#define DMA_MEMORY_SIZE_8_BITS         0x00000000        // Memory data alignment : uint8_t
#define DMA_MEMORY_SIZE_16_BITS        DMA_SxCR_MSIZE_0  // Memory data alignment : uint16_t
#define DMA_MEMORY_SIZE_32_BITS        DMA_SxCR_MSIZE_1  // Memory data alignment : uint32_t

#define DMA_PERIPHERAL_BURST_SINGLE    0x00000000                              // Peripheral burst single transfer configuration
#define DMA_PERIPHERAL_BURST_INC4      DMA_SxCR_PBURST_0                       // Peripheral burst of 4 beats transfer configuration
#define DMA_PERIPHERAL_BURST_INC8      DMA_SxCR_PBURST_1                       // Peripheral burst of 8 beats transfer configuration
#define DMA_PERIPHERAL_BURST_INC16     (DMA_SxCR_PBURST_0 | DMA_SxCR_PBURST_1) // Peripheral burst of 16 beats transfer configuration

#define DMA_MEMORY_BURST_SINGLE        0x00000000                              // Memory burst single transfer configuration
#define DMA_MEMORY_BURST_INC4          DMA_SxCR_MBURST_0                       // Memory burst of 4 beats transfer configuration
#define DMA_MEMORY_BURST_INC8          DMA_SxCR_MBURST_1                       // Memory burst of 8 beats transfer configuration
#define DMA_MEMORY_BURST_INC16         (DMA_SxCR_MBURST_0 | DMA_SxCR_MBURST_1) // Memory burst of 16 beats transfer configuration

#define DMA_CHANNEL_0                  0x00000000
#define DMA_CHANNEL_1                  DMA_SxCR_CHSEL_0
#define DMA_CHANNEL_2                  DMA_SxCR_CHSEL_1
#define DMA_CHANNEL_3                  (DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1)
#define DMA_CHANNEL_4                  DMA_SxCR_CHSEL_2
#define DMA_CHANNEL_5                  (DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0)
#define DMA_CHANNEL_6                  (DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1)
#define DMA_CHANNEL_7                  (DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0)
#define DMA_CHANNEL_8                  DMA_SxCR_CHSEL_3
#define DMA_CHANNEL_9                  (DMA_SxCR_CHSEL_3 | DMA_SxCR_CHSEL_0)

#define DMA_PRIORITY_LEVEL_LOW         0x00000000           // Priority level : Low
#define DMA_PRIORITY_LEVEL_MEDIUM      DMA_SxCR_PL_0        // Priority level : Medium
#define DMA_PRIORITY_LEVEL_HIGH        DMA_SxCR_PL_1        // Priority level : High
#define DMA_PRIORITY_LEVEL_VERY_HIGH   DMA_SxCR_PL          // Priority level : Very_High

#define DMA_PERIPHERAL_TO_MEMORY       0x00000000           // Peripheral to memory direction
#define DMA_MEMORY_TO_PERIPHERAL       DMA_SxCR_DIR_0       // Memory to peripheral direction
#define DMA_MEMORY_TO_MEMORY           DMA_SxCR_DIR_1       // Memory to memory direction


// Callback type in bit position
#define DMA_CALLBACK_NONE               0x00
#define DMA_CALLBACK_HALF_TRANSFER      0x01
#define DMA_CALLBACK_COMPLETED_TRANSFER 0x02

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct DMA_Info_t
{
    uint32_t            ConfigAndChannel;
    uint32_t            DMA_Flag;
    DMA_Stream_TypeDef* pDMA;
    IRQn_Type           IRQn_Channel;
    uint8_t             PreempPrio;
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
        //void        EnableCallbackType                      (int CallbackType);
        void        EnableIRQ                               (uint8_t PremptionPriority);

        // Inline method
        void        Enable                                  (void)                              { SET_BIT(m_pDMA->CR, DMA_SxCR_EN);     }
        void        Disable                                 (void)                              { CLEAR_BIT(m_pDMA->CR, DMA_SxCR_EN);   }
        void        ClearFlag                               (void)                              { ClearFlag(m_Flag);                    }
        size_t      GetLength                               (void)                              { return size_t(m_pDMA->NDTR);          }
        void        SetLength                               (size_t Length)                     { m_pDMA->NDTR = uint32_t(Length);      }
        void        SetMemoryIncrement                      (void)                              { SET_BIT(m_pDMA->CR, DMA_SxCR_MINC);   }
        void        SetNoMemoryIncrement                    (void)                              { CLEAR_BIT(m_pDMA->CR, DMA_SxCR_MINC); }
        void        SetFifoControl                          (uint32_t Control)                  { m_pDMA->FCR = Control;                }
        void        RegisterCallback                        (CallbackInterface* pCallback)      { m_pCallback = pCallback;              }
        void        EnableInterrupt                         (uint32_t Interrupt)                { SET_BIT(m_pDMA->CR, Interrupt);       }
        void        DisableInterrupt                        (uint32_t Interrupt)                { CLEAR_BIT(m_pDMA->CR, Interrupt);     }
        void        EnableTransmitCompleteInterrupt         (void)                              { SET_BIT(m_pDMA->CR, DMA_SxCR_TCIE);   }
        void        DisableTransmitCompleteInterrupt        (void)                              { CLEAR_BIT(m_pDMA->CR, DMA_SxCR_TCIE); }
        void        EnableTransmitHalfCompleteInterrupt     (void)                              { SET_BIT(m_pDMA->CR, DMA_SxCR_HTIE);   }
        void        DisableTransmitHalfCompleteInterrupt    (void)                              { CLEAR_BIT(m_pDMA->CR, DMA_SxCR_HTIE); }

    private:

        void        EnableClock                             (void);


        DMA_Stream_TypeDef*         m_pDMA;
        uint32_t                    m_Flag;
        IRQn_Type                   m_IRQn_Channel;
        uint32_t                    m_Direction;
        CallbackInterface*          m_pCallback;
        //int                         m_CallBackType;       // variables is not used at this time.
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "dma_var.h"         // Project variable

//-------------------------------------------------------------------------------------------------

