//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_spi.h
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
//
//  Note(s):
//          - Only support DMA transfer as it is not making sense otherwise.
//
//          - It used the IO_ID_e for CS as device to lock the SPI as this IO ID is unique.
//          - The SPI Driver is not handling the NSS pin, it's the driver using the SPI that know
//            it's IO for the CS
//
//  We used the IO_ID_e as Device to lock the SPI as this IO ID is unique.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SPI_SPEED_FCLK_DIV2         (0)
#define SPI_SPEED_FCLK_DIV4         (SPI_CR1_BR_0)
#define SPI_SPEED_FCLK_DIV8         (SPI_CR1_BR_1)
#define SPI_SPEED_FCLK_DIV16        (SPI_CR1_BR_1 | SPI_CR1_BR_0)
#define SPI_SPEED_FCLK_DIV32        (SPI_CR1_BR_2)
#define SPI_SPEED_FCLK_DIV64        (SPI_CR1_BR_2 | SPI_CR1_BR_0)
#define SPI_SPEED_FCLK_DIV128       (SPI_CR1_BR_2 | SPI_CR1_BR_1)
#define SPI_SPEED_FCLK_DIV256       (SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0)

#define SPI_CFG_CR1_CLEAR_MASK      (SPI_CR1_CPHA    | SPI_CR1_CPOL     | SPI_CR1_MSTR   | \
                                     SPI_CR1_BR      | SPI_CR1_LSBFIRST | SPI_CR1_SSI    | \
                                     SPI_CR1_SSM     | SPI_CR1_RXONLY   | SPI_CR1_DFF    | \
                                     SPI_CR1_CRCNEXT | SPI_CR1_CRCEN    | SPI_CR1_BIDIOE | \
                                     SPI_CR1_BIDIMODE)

#define SPI_SPEED_MASK              (SPI_CR1_BR)

// Operation Mode
#define SPI_MODE_MASTER             (SPI_CR1_MSTR | SPI_CR1_SSI)
#define SPI_MODE_SLAVE              0x00000000U

// Clock Polarity
#define SPI_POLARITY_LOW            0x00000000U                                                 // Clock to 0 when idle
#define SPI_POLARITY_HIGH           (SPI_CR1_CPOL)                                              // Clock to 1 when idle

// Clock Phase
#define SPI_PHASE_1_EDGE            0x00000000U                                                 // First clock transition is the first data capture edge
#define SPI_PHASE_2_EDGE            (SPI_CR1_CPHA)                                              // Second clock transition is the first data capture edge

// Transmission Bit Order
#define SPI_LSB_FIRST               (SPI_CR1_LSBFIRST)                                          // Data is transmitted/received with the LSB first
#define SPI_MSB_FIRST               0x00000000U                                                 // Data is transmitted/received with the MSB first

// Transfer Mode
#define SPI_FULL_DUPLEX             0x00000000U                                                 // Full-Duplex mode. Rx and TX transfer on 2 lines
#define SPI_HALF_DUPLEX             SPI_CR1_BIDIMODE                                            // Half-Duplex mode. Rx and TX transfer on 1 bidirectional line
#define SPI_SIMPLEX_RX              (SPI_CR1_RXONLY)                                            // Simplex Rx mode.  Rx transfer only on 1 line
#define SPI_HALF_DUPLEX_RX          (SPI_CR1_BIDIMODE)                                          // Half-Duplex Rx mode. RX transfer on 1 line
#define SPI_HALF_DUPLEX_TX          (SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE)                         // Half-Duplex TX mode. TX transfer on 1 line
#define SPI_DUPLEX_MASK             SPI_HALF_DUPLEX_TX                                          // Full-Duplex mode. Rx and TX transfer on 2 lines

// Slave Select Pin Mode
#define SPI_NSS_SOFT                (SPI_CR1_SSM)                                               // NSS managed internally. NSS pin not used and free
#define SPI_NSS_HARD_INPUT          0x00000000U                                                 // NSS pin used in Input. Only used in Master mode
#define SPI_NSS_HARD_OUTPUT         (((uint32_t)SPI_CR2_SSOE << 16U))                           // NSS pin used in Output. Only used in Slave mode as chip select

// Data width
#define SPI_DATA_WIDTH_8_BIT        0x00000000U
#define SPI_DATA_WIDTH_16_BIT       (SPI_CR1_DFF)

#define SPI_DMA_DISABLED            (0xFFFFFFFFU)                                               // Config for spi_var, to disable DMA if not needed

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

enum SPI_ID_e
{
    #if (SPI_DRIVER_SUPPORT_SPI1_CFG == DEF_ENABLED)
		DRIVER_SPI1_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI2_CFG == DEF_ENABLED)
		DRIVER_SPI2_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI3_CFG == DEF_ENABLED)
		DRIVER_SPI3_ID,
	#endif

    NB_OF_SPI_DRIVER
};

struct SPI_Info_t
{
    SPI_TypeDef*        pSPIx;
    uint32_t            Config;
    uint32_t            Speed;
    IO_ID_e             PinCLK;
    IO_ID_e             PinMOSI;
    IO_ID_e             PinMISO;
    IO_ID_e             PinNSS;
    IRQn_Type           IRQn;
  #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
    DMA_Info_t          DMA_RX;
    DMA_Info_t          DMA_TX;
  #endif
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SPI_Driver
{
    public:
                        SPI_Driver              (SPI_ID_e SPI_ID);

        void            Initialize              (void);

        SystemState_e   GetStatus               (void)                        { return m_Status;     }

        SystemState_e   LockToDevice            (IO_ID_e Device);                                                // Set SPI to this device and lock
        SystemState_e   UnlockFromDevice        (IO_ID_e Device);                                                // Unlock SPI from device

        // Read function (overloaded)
        SystemState_e   Read                    (uint8_t* pBuffer, size_t Size);
        SystemState_e   Read                    (uint8_t* pBuffer, size_t Size, IO_ID_e Device);
        SystemState_e   Read                    (uint8_t* pData);
        SystemState_e   Read                    (uint8_t* pData, IO_ID_e Device);

        // Write function (overloaded)
        SystemState_e   Write                   (uint8_t* pBuffer, size_t Size);
        SystemState_e   Write                   (uint8_t* pBuffer, size_t Size, IO_ID_e Device);
        SystemState_e   Write                   (uint8_t  Data);
        SystemState_e   Write                   (uint8_t  Data, IO_ID_e Device);

        SystemState_e   Transfer                (uint8_t* pTX_Data, uint32_t TX_Size, uint8_t* pRX_Data, uint32_t RX_Size);
        SystemState_e   Transfer                (uint8_t* pTX_Data, uint32_t TX_Size, uint8_t* pRX_Data, uint32_t RX_Size, IO_ID_e Device);

        void            IRQHandler              (void);
        SystemState_e   WaitReady               (void);

      #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
        SystemState_e   GetDMA_Status           (void)                        {	return m_DMA_Status; }
        void            OverrideMemoryIncrement (void);
        static void     DMA_RX_IRQ_Handler      (SPI_ID_e SPI_ID);
        static void     DMA_TX_IRQ_Handler      (SPI_ID_e SPI_ID);
      #endif

    private:

        void            SetPrescalerFromSpeed   (uint32_t Speed, uint32_t PCLK_Frequency);

      #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
        SystemState_e   WaitDMA                 (void);
      #endif

        nOS_Mutex               m_Mutex;
        SPI_Info_t*             m_pInfo;
        SPI_ID_e                m_SPI_ID;
        IO_ID_e                 m_Device;
        bool                    m_NoMemoryIncrement;
        volatile SystemState_e  m_Status;
        volatile uint8_t        m_Timeout;
        static SPI_Driver*      m_pDriver[NB_OF_SPI_DRIVER];

      #if (SPI_DRIVER_SUPPORT_DMA_CFG == DEF_ENABLED)
        DMA_Driver              m_DMA_RX;
        DMA_Driver              m_DMA_TX;
        volatile SystemState_e  m_DMA_Status;
        nOS_Sem                 m_DMA_ReleaseSem;

        bool                    m_IsItUsingDMA_TX;
        bool                    m_IsItUsingDMA_RX;
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "spi_var.h"

extern SPI_Info_t SPI_Info[NB_OF_SPI_DRIVER];

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
