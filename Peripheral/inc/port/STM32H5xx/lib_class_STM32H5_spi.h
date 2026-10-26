//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_spi.h
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <cstdint>
#include <cstddef>
#include "lib_define.h"
#include "spi_cfg.h"
#include "stm32h563xx.h"
#include "lib_class_STM32H5_dma.h"
#include "lib_STM32H5_io.h"
#include <atomic>

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SPI_SPEED_FCLK_DIV2         (0x00000000)
#define SPI_SPEED_FCLK_DIV4         (SPI_CFG1_MBR_0)
#define SPI_SPEED_FCLK_DIV8         (SPI_CFG1_MBR_1)
#define SPI_SPEED_FCLK_DIV16        (SPI_CFG1_MBR_1 | SPI_CFG1_MBR_0)
#define SPI_SPEED_FCLK_DIV32        (SPI_CFG1_MBR_2)
#define SPI_SPEED_FCLK_DIV64        (SPI_CFG1_MBR_2 | SPI_CFG1_MBR_0)
#define SPI_SPEED_FCLK_DIV128       (SPI_CFG1_MBR_2 | SPI_CFG1_MBR_1)
#define SPI_SPEED_FCLK_DIV256       (SPI_CFG1_MBR_2 | SPI_CFG1_MBR_1 | SPI_CFG1_MBR_0)

// Clock Phase
#define SPI_PHASE_1_EDGE            0x00000000                                                  // First clock transition is the first data capture edge
#define SPI_PHASE_2_EDGE            (SPI_CFG2_CPHA)                                             // Second clock transition is the first data capture edge

// SPI_Clock_Polarity
#define SPI_POLARITY_LOW            0x00000000
#define SPI_POLARITY_HIGH           SPI_CFG2_CPOL

// Transmission Bit Order
#define SPI_MSB_FIRST               0x00000000                                                  // Data is transmitted/received with the MSB first
#define SPI_LSB_FIRST               (SPI_CFG2_LSBFRST)                                          // Data is transmitted/received with the LSB first

// Communication Mode
#define SPI_FULL_DUPLEX             0x00000000                                                  // Full-Duplex mode. Rx and TX transfer on 2 lines
#define SPI_SIMPLEX_TX              (SPI_CFG2_COMM_0)                                           // Simplex Rx mode.  Rx transfer only on 1 line
#define SPI_SIMPLEX_RX              (SPI_CFG2_COMM_1)                                           // Simplex Rx mode.  Rx transfer only on 1 line
#define SPI_HALF_DUPLEX             (SPI_CFG2_COMM_0 | SPI_CFG2_COMM_1)                         // Half-Duplex mode. Rx and TX transfer on 1 bidirectional line
#define SPI_COMM_MASK               (SPI_CFG2_COMM)                                             // Mask

// Data width
#define SPI_DATA_WIDTH_4_BIT        (SPI_CFG1_DSIZE_1 | SPI_CFG1_DSIZE_0)
#define SPI_DATA_WIDTH_5_BIT        (SPI_CFG1_DSIZE_2)
#define SPI_DATA_WIDTH_6_BIT        (SPI_CFG1_DSIZE_2 | SPI_CFG1_DSIZE_0)
#define SPI_DATA_WIDTH_7_BIT        (SPI_CFG1_DSIZE_2 | SPI_CFG1_DSIZE_1)
#define SPI_DATA_WIDTH_8_BIT        (SPI_CFG1_DSIZE_2 | SPI_CFG1_DSIZE_1 | SPI_CFG1_DSIZE_0)
#define SPI_DATA_WIDTH_9_BIT        (SPI_CFG1_DSIZE_3)
#define SPI_DATA_WIDTH_10_BIT       (SPI_CFG1_DSIZE_3 | SPI_CFG1_DSIZE_0)
#define SPI_DATA_WIDTH_11_BIT       (SPI_CFG1_DSIZE_3 | SPI_CFG1_DSIZE_1)
#define SPI_DATA_WIDTH_12_BIT       (SPI_CFG1_DSIZE_3 | SPI_CFG1_DSIZE_1 | SPI_CFG1_DSIZE_0)
#define SPI_DATA_WIDTH_13_BIT       (SPI_CFG1_DSIZE_3 | SPI_CFG1_DSIZE_2)
#define SPI_DATA_WIDTH_14_BIT       (SPI_CFG1_DSIZE_3 | SPI_CFG1_DSIZE_2 | SPI_CFG1_DSIZE_0)
#define SPI_DATA_WIDTH_15_BIT       (SPI_CFG1_DSIZE_3 | SPI_CFG1_DSIZE_2 | SPI_CFG1_DSIZE_1)
#define SPI_DATA_WIDTH_16_BIT       (SPI_CFG1_DSIZE_3 | SPI_CFG1_DSIZE_2 | SPI_CFG1_DSIZE_1 | SPI_CFG1_DSIZE_0)
// ... up to 32 bits.. add missing one if necessary

#define SPI_DMA_DISABLED            nullptr                                                     // Config for spi_var, to disable DMA if not needed

#define SPI_MODE_MASTER             SPI_CFG2_MASTER
#define SPI_NSS_SOFT                SPI_CFG2_SSM

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

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


    NB_OF_SPI_DRIVER
};

struct SPI_Info_t
{
    SPI_TypeDef*        pSPIx;

    uint32_t            Mode;           // Master or Slave
    uint32_t            DataWidth;      // Data frame size
    uint32_t            CLK_Polarity;
    uint32_t            CLK_Phase;
    uint32_t            NSS_Handling;
    uint32_t            FirstBit;
    uint32_t            Duplex;

    uint32_t            Speed;
    IO_ID_e             PinCLK;
    IO_ID_e             PinMOSI;
    IO_ID_e             PinMISO;
    IO_ID_e             PinNSS;
    IRQn_Type           IRQn;
    uint8_t             PreempPrio;
    DMA_Info_t          DMA_RX;
    DMA_Info_t          DMA_TX;
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
        SystemState_e   GetDMA_Status           (void)                        { return m_DMA_Status; }

        SystemState_e   LockToDevice            (IO_ID_e Device, bool HandleCS = true);                         // Set SPI to this device and lock
        SystemState_e   UnlockFromDevice        (IO_ID_e Device, bool HandleCS = true);                         // Unlock SPI from device

        SystemState_e   SelectChip              (IO_ID_e Device);
        SystemState_e   DeSelectChip            (IO_ID_e Device);

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

        SystemState_e   WaitReady               (void);

        void            OverrideMemoryIncrement (void);
        static void     IRQ_Handler             (SPI_ID_e SPI_ID);

    private:

        void            SetPrescalerFromSpeed   (uint32_t Speed, uint32_t PCLK_Frequency);
        void            ClearAllFlag            (void);

        SystemState_e   WaitEndOfTransfer       (void);
        void            ReleaseFromISR          (void);

      #ifdef USE_FREERTOS
        volatile bool           m_IsItInitialize;
        SemaphoreHandle_t       m_Mutex;
        SemaphoreHandle_t       m_SignalISRToThread;
      #else
        volatile bool           m_IsItInUse;
      #endif    
        SPI_Info_t*             m_pInfo;
        SPI_ID_e                m_SPI_ID;
        bool                    m_NoMemoryIncrement;
        std::atomic<IO_ID_e>    m_Device;
        volatile SystemState_e  m_Status;
        volatile uint8_t        m_Timeout;
        static SPI_Driver*      m_pDriver[NB_OF_SPI_DRIVER];

        DMA_Driver              m_DMA_RX;
        DMA_Driver              m_DMA_TX;
        volatile SystemState_e  m_DMA_Status;
        
        bool                    m_IsItUsingDMA_RX;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "spi_var.h"

extern SPI_Info_t SPI_Info[NB_OF_SPI_DRIVER];

//-------------------------------------------------------------------------------------------------

#endif // __cplusplus
