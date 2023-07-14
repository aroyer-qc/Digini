//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_spi.h
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
//
//  Only support DMA transfer as it is not making sense otherwise.
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

    #if (SPI_DRIVER_SUPPORT_SPI4_CFG == DEF_ENABLED)
		DRIVER_SPI4_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI5_CFG == DEF_ENABLED)
		DRIVER_SPI5_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI6_CFG == DEF_ENABLED)
		DRIVER_SPI6_ID,
	#endif

    NB_OF_SPI_DRIVER,
};

struct SPI_Info_t
{
    SPI_ID_e            SPI_ID;
    SPI_TypeDef*        pSPIx;
    IO_ID_e             PinCLK;
    IO_ID_e             PinMOSI;
    IO_ID_e             PinMISO;
    IO_ID_e             PinNSS;
    uint32_t            Clock;
    uint16_t            portConfig;
    uint8_t             PreempPrio;
    IRQn_Type           IRQn_Channel;
    uint16_t            IRQ_Source;
    void                (*CallBackISR)();

  #if (SPI_DRIVER_SUPPORT_DMA == DEF_ENABLED)
    DMA_TypeDef*        pDMA;
    uint32_t            DMA_ChannelRX;
    uint32_t            DMA_Flag_TC_RX;
    DMA_Stream_TypeDef* pDMA_RX;

    uint32_t            DMA_ChannelTX;
    uint32_t            DMA_Flag_TC_TX;
    DMA_Stream_TypeDef* pDMA_TX;
  #endif // (SPI_DRIVER_SUPPORT_DMA == DEF_ENABLED)
};

struct SPI_DeviceInfo_t
{
    uint32_t            SlowSpeed;
    uint32_t            FastSpeed;
    uint8_t             TimeOut;
    uint32_t            ChipSelectClock;
    GPIO_TypeDef*       pChipSelect;
    uint16_t            ChipSelectPin;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SPI_Driver : public DriverInterface
{
    public:
                        SPI_Driver              (SPI_ID_e SPI_ID);

        void            Initialize              (void);
        SystemState_e   GetStatus               (void);
        SystemState_e   LockToDevice            (SPI_DeviceInfo_t* pDevice);                                             // Set SPI to this device and lock
        SystemState_e   UnlockFromDevice        (SPI_DeviceInfo_t* pDevice);                                             // Unlock SPI from device

        // Read function (overloaded)
        SystemState_e   Read                    (uint8_t* pBuffer, size_t Size);
        SystemState_e   Read                    (uint8_t*  Data);
        SystemState_e   Read                    (uint16_t* Data);
        SystemState_e   Read                    (uint32_t* Data);
        SystemState_e   Read                    (uint8_t* pBuffer, size_t Size, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Read                    (uint8_t*  pData, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Read                    (uint16_t* pData, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Read                    (uint32_t* pData, SPI_DeviceInfo_t* pDevice);

        // Write function (overloaded)
        SystemState_e   Write                   (const uint8_t* pBuffer, size_t Size);
        SystemState_e   Write                   (uint8_t  Data);
        SystemState_e   Write                   (uint16_t Data);
        SystemState_e   Write                   (uint32_t Data);
        SystemState_e   Write                   (const uint8_t* pBuffer, size_t Size, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Write                   (uint8_t  Data, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Write                   (uint16_t Data, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Write                   (uint32_t Data, SPI_DeviceInfo_t* pDevice);

        void            IRQHandler              (void);
        SystemState_e   Request                 (AccessRequest_e Request, uint8_t *pBuffer, size_t Size);
        void            Config                  (uint32_t Speed);
        //void            TickHook                (void);
        // void            RegisterDriverTick      (void (*CallBackTick)());
        SystemState_e   WaitReady               (void);

    private:

//        uint16_t              GetPrescalerFromSpeed   (uint32_t speed);
        void                    Lock                    (void);
        void                    Unlock                  (void);

        SPI_Info_t*                             m_pInfo;
        SPI_DeviceInfo_t*                       m_pDevice;
        nOS_Mutex                               m_Mutex;
        AccessRequest_e                         m_Request;
        uint32_t                                m_SlowSpeed;
        uint32_t                                m_FastSpeed;
        volatile size_t                         m_Size;
        volatile SystemState_e		            m_State;
        volatile uint8_t                        m_Timeout;
        void                                    (*m_pCallBackTick)();

};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "spi_var.h"

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
