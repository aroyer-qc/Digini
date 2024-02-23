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

#define DMA_MEMORY_TO_MEMORY           0x4000           // Memory to memory mode

#define DMA_PRIORITY_LEVEL_LOW         0x0000           // Priority low
#define DMA_PRIORITY_LEVEL_MEDIUM      0x1000           // Priority medium
#define DMA_PRIORITY_LEVEL_HIGH        0x2000           // Priority high
#define DMA_PRIORITY_LEVEL_VERY_HIGH   0x3000           // Priority very high

#define DMA_MEMORY_SIZE_8_BITS         0x0000           // 8 bits
#define DMA_MEMORY_SIZE_16_BITS        0x0400           // 16 bits
#define DMA_MEMORY_SIZE_32_BITS        0x0800           // 32 Bits

#define DMA_PERIPHERAL_8_BITS          0x0000           // 8 bits
#define DMA_PERIPHERAL_16_BITS         0x0100           // 16 bits
#define DMA_PERIPHERAL_32_BITS         0x0200           // 32 Bits

#define DMA_MEMORY_NO_INCREMENT        0x0000           // Memory increment mode Disable
#define DMA_MEMORY_INCREMENT           0x0080           // Memory increment mode Enable

#define DMA_PERIPH_NO_INCREMENT        0x0000           // Peripheral increment mode Disable
#define DMA_PERIPH_INCREMENT           0x0040           // Peripheral increment mode Enable

#define DMA_NORMAL_MODE                0x0000           // Normal mode
#define DMA_CIRCULAR_MODE              0x0020           // Circular mode

#define DMA_READ_FROM_PERIPH           0x0000           // Peripheral to memory direction
#define DMA_READ_FROM_MEMORY           0x0010           // Memory to peripheral or memory direction

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class DMA_Driver
{
    public:
        
        void        initialize                              (DMA_ID_e DMA_ID);
        
        void        SetChannelRX                            (void* pSource, void* pDestination, size_t Length);
        void        SetChannelTX                            (void* pSource, void* pDestination, size_t Length);
        void        ClearFlag                               (uint32_t Flag);
        uint32_t    CheckFlag                               (uint32_t Flag);
        void        EnableInterrupt                         (uint32_t Interrupt);
        void        DisableInterrupt                        (uint32_t Interrupt);
        void        EnableTransmitCompleteInterrupt         (void);
        void        DisableTransmitCompleteInterrupt        (void);
        void        EnableTransmitHalfCompleteInterrupt     (void);
        void        DisableTransmitHalfCompleteInterrupt    (void);
        void        Enable                                  (void);
        void        Disable                                 (void);
        
    private:    
        
        DMA_ID_e                m_DMA_ID;       // maybe useless.
        
        
        DMA_Channel_TypeDef*     m_pDMA;
};

//-------------------------------------------------------------------------------------------------

