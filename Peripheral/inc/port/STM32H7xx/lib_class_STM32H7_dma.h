//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H7_dma.h
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
//
// Note(s)  Does not support at this time : Memory to memory transfer.
//                                          DMA Fifo mode.
//                                          BDMA mode
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s) and macro(s)
//-------------------------------------------------------------------------------------------------

#define DMA_CHECK_FIFO_PARAMS               DEF_ENABLED                             // When FIFO config is good this can be deactivated

// DMA Define
#define DMA_MODE_NORMAL                     0x00000000        						// Normal Mode
#define DMA_MODE_CIRCULAR                   DMA_SxCR_CIRC     						// Circular Mode
#define DMA_MODE_PERIPHERAL_FLOW_CTRL       DMA_SxCR_PFCTRL   						// Peripheral flow control mode

#define DMA_PERIPHERAL_NO_INCREMENT         0x00000000        						// Peripheral increment mode Disable
#define DMA_PERIPHERAL_INCREMENT            DMA_SxCR_PINC     						// Peripheral increment mode Enable

#define DMA_MEMORY_NO_INCREMENT             0x00000000        						// Memory increment mode Disable
#define DMA_MEMORY_INCREMENT                DMA_SxCR_MINC     						// Memory increment mode Enable

#define DMA_PERIPHERAL_SIZE_8_BITS          0x00000000        						// Peripheral data alignment : uint8_t
#define DMA_PERIPHERAL_SIZE_16_BITS         DMA_SxCR_PSIZE_0  						// Peripheral data alignment : uint16_t
#define DMA_PERIPHERAL_SIZE_32_BITS         DMA_SxCR_PSIZE_1  						// Peripheral data alignment : uint32_t

#define DMA_MEMORY_SIZE_8_BITS              0x00000000        						// Memory data alignment : uint8_t
#define DMA_MEMORY_SIZE_16_BITS             DMA_SxCR_MSIZE_0  						// Memory data alignment : uint16_t
#define DMA_MEMORY_SIZE_32_BITS             DMA_SxCR_MSIZE_1  						// Memory data alignment : uint32_t

#define DMA_PERIPHERAL_BURST_SINGLE         0x00000000                              // Peripheral burst single transfer configuration
#define DMA_PERIPHERAL_BURST_INC4           DMA_SxCR_PBURST_0                       // Peripheral burst of 4 beats transfer configuration
#define DMA_PERIPHERAL_BURST_INC8           DMA_SxCR_PBURST_1                       // Peripheral burst of 8 beats transfer configuration
#define DMA_PERIPHERAL_BURST_INC16          (DMA_SxCR_PBURST_0 | DMA_SxCR_PBURST_1) // Peripheral burst of 16 beats transfer configuration

#define DMA_MEMORY_BURST_SINGLE             0x00000000                              // Memory burst single transfer configuration
#define DMA_MEMORY_BURST_INC4               DMA_SxCR_MBURST_0                       // Memory burst of 4 beats transfer configuration
#define DMA_MEMORY_BURST_INC8               DMA_SxCR_MBURST_1                       // Memory burst of 8 beats transfer configuration
#define DMA_MEMORY_BURST_INC16              (DMA_SxCR_MBURST_0 | DMA_SxCR_MBURST_1) // Memory burst of 16 beats transfer configuration

#define DMA_PRIORITY_LEVEL_LOW              0x00000000           					// Priority level : Low
#define DMA_PRIORITY_LEVEL_MEDIUM           DMA_SxCR_PL_0        					// Priority level : Medium
#define DMA_PRIORITY_LEVEL_HIGH             DMA_SxCR_PL_1        					// Priority level : High
#define DMA_PRIORITY_LEVEL_VERY_HIGH        DMA_SxCR_PL          					// Priority level : Very_High

#define DMA_PERIPHERAL_TO_MEMORY            0x00000000           					// Peripheral to memory direction
#define DMA_MEMORY_TO_PERIPHERAL            DMA_SxCR_DIR_0       					// Memory to peripheral direction
#define DMA_MEMORY_TO_MEMORY                DMA_SxCR_DIR_1       					// Memory to memory direction

#define DMA_BUFFERABLE_TRANSFER_DISABLE     0x00000000           					// Peripheral to memory direction
#define DMA_BUFFERABLE_TRANSFER_ENABLE      DMA_SxCR_TRBUFF      					// Enable the DMA to handle bufferable transfers ( must be enable for DMA stream manages UART/USART/LPUART transfers.)

#define DMA_FIFOMODE_DISABLE                0x00000000                              // FIFO mode disable
#define DMA_FIFOMODE_ENABLE                 DMA_SxFCR_DMDIS                         // FIFO mode enable

#define DMA_FIFO_THRESHOLD_1_QUARTER_FULL   0x00000000                              // FIFO threshold 1 quart full configuration
#define DMA_FIFO_THRESHOLD_HALF_FULL        DMA_SxFCR_FTH_0                         // FIFO threshold half full configuration
#define DMA_FIFO_THRESHOLD_3_QUARTERS_FULL  DMA_SxFCR_FTH_1                         // FIFO threshold 3 quarts full configuration
#define DMA_FIFO_THRESHOLD_FULL             DMA_SxFCR_FTH                           // FIFO threshold full configuration

// BDMA Define
#define BDMA_MODE_NORMAL                    0x00000000        						// Normal Mode
#define BDMA_MODE_CIRCULAR                  DMA_CCR_CIRC     						// Circular Mode

#define BDMA_PERIPHERAL_NO_INCREMENT        0x00000000        						// Peripheral increment mode Disable
#define BDMA_PERIPHERAL_INCREMENT           DMA_CCR_PINC     						// Peripheral increment mode Enable

#define BDMA_MEMORY_NO_INCREMENT            0x00000000        						// Memory increment mode Disable
#define BDMA_MEMORY_INCREMENT               DMA_CCR_MINC     						// Memory increment mode Enable

#define BDMA_PERIPHERAL_SIZE_8_BITS         0x00000000        						// Peripheral data alignment : uint8_t
#define BDMA_PERIPHERAL_SIZE_16_BITS        DMA_CCR_PSIZE_0  						// Peripheral data alignment : uint16_t
#define BDMA_PERIPHERAL_SIZE_32_BITS        DMA_CCR_PSIZE_1  						// Peripheral data alignment : uint32_t

#define BDMA_MEMORY_SIZE_8_BITS             0x00000000        						// Memory data alignment : uint8_t
#define BDMA_MEMORY_SIZE_16_BITS            DMA_CCR_MSIZE_0  						// Memory data alignment : uint16_t
#define BDMA_MEMORY_SIZE_32_BITS            DMA_CCR_MSIZE_1  						// Memory data alignment : uint32_t

#define BDMA_PRIORITY_LEVEL_LOW             0x00000000           					// Priority level : Low
#define BDMA_PRIORITY_LEVEL_MEDIUM          DMA_CCR_PL_0        					// Priority level : Medium
#define BDMA_PRIORITY_LEVEL_HIGH            DMA_CCR_PL_1        					// Priority level : High
#define BDMA_PRIORITY_LEVEL_VERY_HIGH       DMA_CCR_PL          					// Priority level : Very_High

#define BDMA_PERIPHERAL_TO_MEMORY           0x00000000           					// Peripheral to memory direction
#define BDMA_MEMORY_TO_PERIPHERAL           DMA_CCR_DIR_0       					// Memory to peripheral direction

// DMAMUX1 requests
#define DMA_REQUEST_MEM2MEM                 0   									// Memory to memory transfer

#define DMA_REQUEST_GENERATOR0              1   									// DMAMUX1 request generator 0
#define DMA_REQUEST_GENERATOR1              2   									// DMAMUX1 request generator 1
#define DMA_REQUEST_GENERATOR2       	    3   									// DMAMUX1 request generator 2
#define DMA_REQUEST_GENERATOR3       	    4   									// DMAMUX1 request generator 3
#define DMA_REQUEST_GENERATOR4       	    5   									// DMAMUX1 request generator 4
#define DMA_REQUEST_GENERATOR5       	    6   									// DMAMUX1 request generator 5
#define DMA_REQUEST_GENERATOR6       	    7   									// DMAMUX1 request generator 6
#define DMA_REQUEST_GENERATOR7       	    8   									// DMAMUX1 request generator 7

#define DMA_REQUEST_ADC1             	    9   									// DMAMUX1 ADC1 request
#define DMA_REQUEST_ADC2             	    10  									// DMAMUX1 ADC2 request

#define DMA_REQUEST_TIM1_CH1         	    11   									// DMAMUX1 TIM1 CH1 request
#define DMA_REQUEST_TIM1_CH2         	    12   									// DMAMUX1 TIM1 CH2 request
#define DMA_REQUEST_TIM1_CH3         	    13   									// DMAMUX1 TIM1 CH3 request
#define DMA_REQUEST_TIM1_CH4         	    14   									// DMAMUX1 TIM1 CH4 request
#define DMA_REQUEST_TIM1_UP          	    15   									// DMAMUX1 TIM1 UP request
#define DMA_REQUEST_TIM1_TRIG        	    16   									// DMAMUX1 TIM1 TRIG request
#define DMA_REQUEST_TIM1_COM         	    17   									// DMAMUX1 TIM1 COM request

#define DMA_REQUEST_TIM2_CH1         	    18   									// DMAMUX1 TIM2 CH1 request
#define DMA_REQUEST_TIM2_CH2         	    19   									// DMAMUX1 TIM2 CH2 request
#define DMA_REQUEST_TIM2_CH3         	    20   									// DMAMUX1 TIM2 CH3 request
#define DMA_REQUEST_TIM2_CH4         	    21   									// DMAMUX1 TIM2 CH4 request
#define DMA_REQUEST_TIM2_UP          	    22   									// DMAMUX1 TIM2 UP request

#define DMA_REQUEST_TIM3_CH1         	    23   									// DMAMUX1 TIM3 CH1 request
#define DMA_REQUEST_TIM3_CH2         	    24   									// DMAMUX1 TIM3 CH2 request
#define DMA_REQUEST_TIM3_CH3         	    25   									// DMAMUX1 TIM3 CH3 request
#define DMA_REQUEST_TIM3_CH4         	    26   									// DMAMUX1 TIM3 CH4 request
#define DMA_REQUEST_TIM3_UP          	    27   									// DMAMUX1 TIM3 UP request
#define DMA_REQUEST_TIM3_TRIG        	    28   									// DMAMUX1 TIM3 TRIG request

#define DMA_REQUEST_TIM4_CH1         	    29   									// DMAMUX1 TIM4 CH1 request
#define DMA_REQUEST_TIM4_CH2         	    30   									// DMAMUX1 TIM4 CH2 request
#define DMA_REQUEST_TIM4_CH3         	    31   									// DMAMUX1 TIM4 CH3 request
#define DMA_REQUEST_TIM4_UP          	    32   									// DMAMUX1 TIM4 UP request

#define DMA_REQUEST_I2C1_RX          	    33   									// DMAMUX1 I2C1 RX request
#define DMA_REQUEST_I2C1_TX          	    34   									// DMAMUX1 I2C1 TX request
#define DMA_REQUEST_I2C2_RX          	    35   									// DMAMUX1 I2C2 RX request
#define DMA_REQUEST_I2C2_TX          	    36   									// DMAMUX1 I2C2 TX request

#define DMA_REQUEST_SPI1_RX          	    37   									// DMAMUX1 SPI1 RX request
#define DMA_REQUEST_SPI1_TX          	    38   									// DMAMUX1 SPI1 TX request
#define DMA_REQUEST_SPI2_RX          	    39   									// DMAMUX1 SPI2 RX request
#define DMA_REQUEST_SPI2_TX          	    40   									// DMAMUX1 SPI2 TX request

#define DMA_REQUEST_USART1_RX        	    41   									// DMAMUX1 USART1 RX request
#define DMA_REQUEST_USART1_TX        	    42   									// DMAMUX1 USART1 TX request
#define DMA_REQUEST_USART2_RX        	    43   									// DMAMUX1 USART2 RX request
#define DMA_REQUEST_USART2_TX        	    44   									// DMAMUX1 USART2 TX request
#define DMA_REQUEST_USART3_RX        	    45   									// DMAMUX1 USART3 RX request
#define DMA_REQUEST_USART3_TX        	    46   									// DMAMUX1 USART3 TX request

#define DMA_REQUEST_TIM8_CH1         	    47   									// DMAMUX1 TIM8 CH1 request
#define DMA_REQUEST_TIM8_CH2         	    48   									// DMAMUX1 TIM8 CH2 request
#define DMA_REQUEST_TIM8_CH3         	    49   									// DMAMUX1 TIM8 CH3 request
#define DMA_REQUEST_TIM8_CH4         	    50   									// DMAMUX1 TIM8 CH4 request
#define DMA_REQUEST_TIM8_UP          	    51   									// DMAMUX1 TIM8 UP request
#define DMA_REQUEST_TIM8_TRIG        	    52   									// DMAMUX1 TIM8 TRIG request
#define DMA_REQUEST_TIM8_COM         	    53   									// DMAMUX1 TIM8 COM request

#define DMA_REQUEST_TIM5_CH1         	    55   									// DMAMUX1 TIM5 CH1 request
#define DMA_REQUEST_TIM5_CH2         	    56   									// DMAMUX1 TIM5 CH2 request
#define DMA_REQUEST_TIM5_CH3         	    57   									// DMAMUX1 TIM5 CH3 request
#define DMA_REQUEST_TIM5_CH4         	    58   									// DMAMUX1 TIM5 CH4 request
#define DMA_REQUEST_TIM5_UP          	    59   									// DMAMUX1 TIM5 UP request
#define DMA_REQUEST_TIM5_TRIG        	    60   									// DMAMUX1 TIM5 TRIG request

#define DMA_REQUEST_SPI3_RX          	    61   									// DMAMUX1 SPI3 RX request
#define DMA_REQUEST_SPI3_TX          	    62   									// DMAMUX1 SPI3 TX request

#define DMA_REQUEST_UART4_RX         	    63   									// DMAMUX1 UART4 RX request
#define DMA_REQUEST_UART4_TX         	    64   									// DMAMUX1 UART4 TX request
#define DMA_REQUEST_UART5_RX         	    65   									// DMAMUX1 UART5 RX request
#define DMA_REQUEST_UART5_TX         	    66   									// DMAMUX1 UART5 TX request

#define DMA_REQUEST_DAC1_CH1         	    67   									// DMAMUX1 DAC1 Channel 1 request
#define DMA_REQUEST_DAC1_CH2         	    68   									// DMAMUX1 DAC1 Channel 2 request

#define DMA_REQUEST_TIM6_UP          	    69   									// DMAMUX1 TIM6 UP request
#define DMA_REQUEST_TIM7_UP          	    70   									// DMAMUX1 TIM7 UP request

#define DMA_REQUEST_USART6_RX        	    71   									// DMAMUX1 USART6 RX request
#define DMA_REQUEST_USART6_TX        	    72   									// DMAMUX1 USART6 TX request

#define DMA_REQUEST_I2C3_RX          	    73   									// DMAMUX1 I2C3 RX request
#define DMA_REQUEST_I2C3_TX          	    74   									// DMAMUX1 I2C3 TX request

#if defined (PSSI)
#define DMA_REQUEST_DCMI_PSSI        	    75   									// DMAMUX1 DCMI/PSSI request
#define DMA_REQUEST_DCMI             	    DMA_REQUEST_DCMI_PSSI 					// Legacy define
#else
#define DMA_REQUEST_DCMI             	    75   									// DMAMUX1 DCMI request
#endif // PSSI

#define DMA_REQUEST_CRYP_IN          	    76   									// DMAMUX1 CRYP IN request
#define DMA_REQUEST_CRYP_OUT         	    77   									// DMAMUX1 CRYP OUT request

#define DMA_REQUEST_HASH_IN          	    78   									// DMAMUX1 HASH IN request

#define DMA_REQUEST_UART7_RX         	    79   									// DMAMUX1 UART7 RX request
#define DMA_REQUEST_UART7_TX         	    80   									// DMAMUX1 UART7 TX request
#define DMA_REQUEST_UART8_RX         	    81   									// DMAMUX1 UART8 RX request
#define DMA_REQUEST_UART8_TX         	    82   									// DMAMUX1 UART8 TX request

#define DMA_REQUEST_SPI4_RX          	    83   									// DMAMUX1 SPI4 RX request
#define DMA_REQUEST_SPI4_TX          	    84   									// DMAMUX1 SPI4 TX request
#define DMA_REQUEST_SPI5_RX          	    85   									// DMAMUX1 SPI5 RX request
#define DMA_REQUEST_SPI5_TX          	    86   									// DMAMUX1 SPI5 TX request

#define DMA_REQUEST_SAI1_A           	    87   									// DMAMUX1 SAI1 A request
#define DMA_REQUEST_SAI1_B           	    88   									// DMAMUX1 SAI1 B request

#if defined(SAI2)
#define DMA_REQUEST_SAI2_A           	    89   									// DMAMUX1 SAI2 A request
#define DMA_REQUEST_SAI2_B           	    90   									// DMAMUX1 SAI2 B request
#endif // SAI2

#define DMA_REQUEST_SWPMI_RX         	    91   									// DMAMUX1 SWPMI RX request
#define DMA_REQUEST_SWPMI_TX         	    92   									// DMAMUX1 SWPMI TX request

#define DMA_REQUEST_SPDIF_RX_DT      	    93   									// DMAMUX1 SPDIF RXDT request
#define DMA_REQUEST_SPDIF_RX_CS      	    94   									// DMAMUX1 SPDIF RXCS request

#if defined(HRTIM1)
#define DMA_REQUEST_HRTIM_MASTER     	    95   									// DMAMUX1 HRTIM1 Master request 1
#define DMA_REQUEST_HRTIM_TIMER_A    	    96   									// DMAMUX1 HRTIM1 Timer A request 2
#define DMA_REQUEST_HRTIM_TIMER_B    	    97   									// DMAMUX1 HRTIM1 Timer B request 3
#define DMA_REQUEST_HRTIM_TIMER_C    	    98   									// DMAMUX1 HRTIM1 Timer C request 4
#define DMA_REQUEST_HRTIM_TIMER_D    	    99   									// DMAMUX1 HRTIM1 Timer D request 5
#define DMA_REQUEST_HRTIM_TIMER_E   	    100  								    // DMAMUX1 HRTIM1 Timer E request 6
#endif // HRTIM1

#define DMA_REQUEST_DFSDM1_FLT0     	    101  								    // DMAMUX1 DFSDM Filter0 request
#define DMA_REQUEST_DFSDM1_FLT1     	    102  								    // DMAMUX1 DFSDM Filter1 request
#define DMA_REQUEST_DFSDM1_FLT2     	    103  								    // DMAMUX1 DFSDM Filter2 request
#define DMA_REQUEST_DFSDM1_FLT3     	    104  								    // DMAMUX1 DFSDM Filter3 request

#define DMA_REQUEST_TIM15_CH1       	    105  								    // DMAMUX1 TIM15 CH1 request
#define DMA_REQUEST_TIM15_UP        	    106  								    // DMAMUX1 TIM15 UP request
#define DMA_REQUEST_TIM15_TRIG      	    107  								    // DMAMUX1 TIM15 TRIG request
#define DMA_REQUEST_TIM15_COM       	    108  								    // DMAMUX1 TIM15 COM request

#define DMA_REQUEST_TIM16_CH1       	    109  								    // DMAMUX1 TIM16 CH1 request
#define DMA_REQUEST_TIM16_UP        	    110  								    // DMAMUX1 TIM16 UP request

#define DMA_REQUEST_TIM17_CH1       	    111  								    // DMAMUX1 TIM17 CH1 request
#define DMA_REQUEST_TIM17_UP        	    112  								    // DMAMUX1 TIM17 UP request

#if defined(SAI3)
#define DMA_REQUEST_SAI3_A          	    113  								    // DMAMUX1 SAI3 A request
#define DMA_REQUEST_SAI3_B          	    114  								    // DMAMUX1 SAI3 B request
#endif // SAI3

#if defined(ADC3)
#define DMA_REQUEST_ADC3            	    115   									// DMAMUX1 ADC3 request
#endif // ADC3

#if defined(UART9)
#define DMA_REQUEST_UART9_RX        	    116   									// DMAMUX1 UART9 request
#define DMA_REQUEST_UART9_TX        	    117   									// DMAMUX1 UART9 request
#endif // UART9

#if defined(USART10)
#define DMA_REQUEST_USART10_RX      	    118   									// DMAMUX1 USART10 request
#define DMA_REQUEST_USART10_TX      	    119   									// DMAMUX1 USART10 request
#endif // USART10

#if defined(FMAC)
#define DMA_REQUEST_FMAC_READ       	    120   									// DMAMUX1 FMAC Read request
#define DMA_REQUEST_FMAC_WRITE      	    121   									// DMAMUX1 FMAC Write request
#endif // FMAC

#if defined(CORDIC)
#define DMA_REQUEST_CORDIC_READ     	    122   									// DMAMUX1 CORDIC Read request
#define DMA_REQUEST_CORDIC_WRITE    	    123   									// DMAMUX1 CORDIC Write request
#endif // CORDIC

#if defined(I2C5)
#define DMA_REQUEST_I2C5_RX         	    124   									// DMAMUX1 I2C5 RX request
#define DMA_REQUEST_I2C5_TX         	    125   									// DMAMUX1 I2C5 TX request
#endif // I2C5

#if defined(TIM23)
#define DMA_REQUEST_TIM23_CH1        	    126   									// DMAMUX1 TIM23 CH1 request
#define DMA_REQUEST_TIM23_CH2        	    127   									// DMAMUX1 TIM23 CH2 request
#define DMA_REQUEST_TIM23_CH3        	    128   									// DMAMUX1 TIM23 CH3 request
#define DMA_REQUEST_TIM23_CH4        	    129   									// DMAMUX1 TIM23 CH4 request
#define DMA_REQUEST_TIM23_UP         	    130   									// DMAMUX1 TIM23 UP request
#define DMA_REQUEST_TIM23_TRIG       	    131   									// DMAMUX1 TIM23 TRIG request
#endif // TIM23

#if defined(TIM24)
#define DMA_REQUEST_TIM24_CH1        	    132   									// DMAMUX1 TIM24 CH1 request
#define DMA_REQUEST_TIM24_CH2        	    133   									// DMAMUX1 TIM24 CH2 request
#define DMA_REQUEST_TIM24_CH3        	    134   									// DMAMUX1 TIM24 CH3 request
#define DMA_REQUEST_TIM24_CH4        	    135   									// DMAMUX1 TIM24 CH4 request
#define DMA_REQUEST_TIM24_UP         	    136   									// DMAMUX1 TIM24 UP request
#define DMA_REQUEST_TIM24_TRIG       	    137   									// DMAMUX1 TIM24 TRIG request
#endif // TIM24

// DMAMUX2 requests
#define BDMA_REQUEST_MEM2MEM          	    0   									// memory to memory transfer
#define BDMA_REQUEST_GENERATOR0       	    1   									// DMAMUX2 request generator 0
#define BDMA_REQUEST_GENERATOR1       	    2   									// DMAMUX2 request generator 1
#define BDMA_REQUEST_GENERATOR2       	    3   									// DMAMUX2 request generator 2
#define BDMA_REQUEST_GENERATOR3       	    4   									// DMAMUX2 request generator 3
#define BDMA_REQUEST_GENERATOR4       	    5   									// DMAMUX2 request generator 4
#define BDMA_REQUEST_GENERATOR5       	    6   									// DMAMUX2 request generator 5
#define BDMA_REQUEST_GENERATOR6       	    7   									// DMAMUX2 request generator 6
#define BDMA_REQUEST_GENERATOR7       	    8   									// DMAMUX2 request generator 7
#define BDMA_REQUEST_LPUART1_RX       	    9   									// DMAMUX2 LP_UART1_RX request
#define BDMA_REQUEST_LPUART1_TX      	    10   									// DMAMUX2 LP_UART1_TX request
#define BDMA_REQUEST_SPI6_RX         	    11   									// DMAMUX2 SPI6 RX request
#define BDMA_REQUEST_SPI6_TX         	    12   									// DMAMUX2 SPI6 TX request
#define BDMA_REQUEST_I2C4_RX         	    13   									// DMAMUX2 I2C4 RX request
#define BDMA_REQUEST_I2C4_TX         	    14   									// DMAMUX2 I2C4 TX request
#if defined(SAI4)
#define BDMA_REQUEST_SAI4_A          	    15   									// DMAMUX2 SAI4 A request
#define BDMA_REQUEST_SAI4_B          	    16   									// DMAMUX2 SAI4 B request
#endif // SAI4
#if defined(ADC3)
#define BDMA_REQUEST_ADC3            	    17   									// DMAMUX2 ADC3 request
#endif // ADC3
#if defined(DAC2)
#define BDMA_REQUEST_DAC2_CH1        	    17   									// DMAMUX2 DAC2 CH1 request
#endif // DAC2
#if defined(DFSDM2_Channel0)
#define BDMA_REQUEST_DFSDM2_FLT0     	    18   									// DMAMUX2 DFSDM2 request
#endif // DFSDM1_Channel0

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum DMA_Type_e
{
    DMA_TYPE,
    BDMA_TYPE,
};

union DMA_Pointer_u
{
    DMA_Stream_TypeDef*     pDMA;
    BDMA_Channel_TypeDef*   pBDMA;
    uint32_t*               pPtr;
};

struct DMA_Info_t
{
    uint32_t            Config;
    uint32_t            FIFO_Config;
    uint32_t            MUX_Request;
    uint32_t            Flag;
    uint32_t*           pHandle;
    IRQn_Type           IRQn_Channel;
    uint8_t             PreempPrio;
};

//-------------------------------------------------------------------------------------------------
// class(s)
//-------------------------------------------------------------------------------------------------

class DMA_Driver
{
    public:

        void            Initialize                              (DMA_Info_t* pInfo);
        void            Enable                                  (void);
        void            Disable                                 (void);
        void            SetTransfer                             (void* pSource, void* pDestination, size_t Length);
        void            SetSource                               (void* pSource);
        void            SetDestination                          (void* pDestination);
        size_t          GetLength                               (void);
        void            SetLength                               (size_t Length);
        void            SetMemoryIncrement                      (void);
        void            SetNoMemoryIncrement                    (void);
        void            SetFifoControl                          (uint32_t Control);
        void            ClearFlag                               (uint32_t Flag);
        bool            CheckFlag                               (uint32_t Flag);
        void            EnableIRQ                               (uint8_t PremptionPriority);
        void            EnableInterrupt                         (uint32_t Interrupt);
        void            DisableInterrupt                        (uint32_t Interrupt);
        void            EnableTransmitCompleteInterrupt         (void);
        void            DisableTransmitCompleteInterrupt        (void);
        void            EnableTransmitHalfCompleteInterrupt     (void);
        void            DisableTransmitHalfCompleteInterrupt    (void);

        // Inline method
        void            ClearFlag                               (void)                              { ClearFlag(m_Flag);                    }
        void            RegisterCallback                        (CallbackInterface* pCallback)      { m_pCallback = pCallback;              }

    private:

        void            EnableClock                             (void);
        void            CalcBaseAddress                         (void);
        void            CalcDMAMUX_ChannelBaseAndMask           (void);
        void            CalcDMAMUX_RequestGenBaseAndMask        (uint32_t Request);

      #if (DMA_CHECK_FIFO_PARAMS == DEF_ENABLED)
        SystemState_e   CheckFifoParam                          (DMA_Info_t* pInfo);
      #endif

        DMA_Type_e                          m_DMA_Type;
        DMA_Pointer_u                       m_Handle;

        uint32_t                            m_StreamNumber;                 // This is the real Stream number
        uint32_t                            m_StreamIndex;                  // This is the stream index for 'Low' and
        uint32_t                            m_StreamBaseAddress;            // For flag clearing

        DMAMUX_Channel_TypeDef*             m_pDMAMUX_Channel;
        DMAMUX_ChannelStatus_TypeDef*       m_pDMAMUX_ChannelStatus;
        uint32_t                            m_DMAMUX_ChannelStatusMask;

        DMAMUX_RequestGen_TypeDef*          m_pDMAMUX_RequestGen;           // DMAMUX request generator Base Address
        DMAMUX_RequestGenStatus_TypeDef*    m_pDMAMUX_RequestGenStatus;     // DMAMUX request generator Status Address
        uint32_t                            m_DMAMUX_RequestGenStatusMask;

        uint32_t                            m_Flag;
        IRQn_Type                           m_IRQn_Channel;
        uint32_t                            m_Direction;
        CallbackInterface*                  m_pCallback;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "dma_var.h"         // Project variable

//-------------------------------------------------------------------------------------------------

