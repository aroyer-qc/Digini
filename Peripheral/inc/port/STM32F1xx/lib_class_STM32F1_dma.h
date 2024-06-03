//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_dma.h
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


// Callback type in bit position
#define DMA_CALLBACK_NONE                   0x00
#define DMA_CALLBACK_HALF_TRANSFER          0x01
#define DMA_CALLBACK_COMPLETED_TRANSFER     0x02

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct DMA_Info_t
{
    uint32_t             Config;
    uint32_t             DMA_Flag;
    DMA_TypeDef*         pDMA;
    DMA_Channel_TypeDef* pDMA_Channel;
    IRQn_Type            IRQn_Channel;
    uint8_t              PreempPrio;
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


        DMA_TypeDef*                m_pDMA;
        DMA_Channel_TypeDef*        m_pDMA_Channel;
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

