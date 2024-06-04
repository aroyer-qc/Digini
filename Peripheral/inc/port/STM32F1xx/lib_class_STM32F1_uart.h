//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_uart.h
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

#if (USE_UART_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#if (UART_DRIVER_SUPPORT_UART1_DMA_CFG  == DEF_ENABLED) || \
    (UART_DRIVER_SUPPORT_UART2_DMA_CFG  == DEF_ENABLED) || \
    (UART_DRIVER_SUPPORT_UART3_DMA_CFG  == DEF_ENABLED) || \
    (UART_DRIVER_SUPPORT_UART4_DMA_CFG  == DEF_ENABLED) || \
    (UART_DRIVER_SUPPORT_UART5_DMA_CFG  == DEF_ENABLED)
  #define UART_DRIVER_DMA_CFG                 DEF_ENABLED
#else
  #define UART_DRIVER_DMA_CFG                 DEF_DISABLED
#endif

#if (UART_DRIVER_DMA_CFG                  == DEF_ENABLED) || \
    (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
  #define UART_DRIVER_ANY_DMA_OR_VIRTUAL_CFG  DEF_ENABLED
#else
  #define UART_DRIVER_ANY_DMA_OR_VIRTUAL_CFG  DEF_DISABLED
#endif

#define UART_ISR_RX_ERROR                   0x01
#define UART_ISR_RX_BYTE                    0x02
#define UART_ISR_RX_IDLE                    0x04
#define UART_ISR_TX_EMPTY                   0x01
#define UART_ISR_TX_COMPLETED               0x02

// Callback type in bit position
#define UART_CALLBACK_NONE                  0x00
#define UART_CALLBACK_RX                    0x01
#define UART_CALLBACK_IDLE                  0x02
#define UART_CALLBACK_ERROR                 0x04
#define UART_CALLBACK_CTS                   0x08
#define UART_CALLBACK_EMPTY_TX              0x10
#define UART_CALLBACK_COMPLETED_TX          0x20
#define UART_CALLBACK_DMA_RX                0x40

#if (UART_ISR_RX_CFG == DEF_ENABLED)  || (UART_ISR_RX_IDLE_CFG == DEF_ENABLED)  || (UART_ISR_RX_ERROR_CFG == DEF_ENABLED) || \
   (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED) || (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
    #define UART_ISR_CFG                    DEF_ENABLED
#endif

#define UART_WAIT_ON_BUSY                    true
#define UART_DONT_WAIT_ON_BUSY               false

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

enum UART_ID_e
{
    #if (UART_DRIVER_SUPPORT_UART1_CFG == DEF_ENABLED)
        UART_DRIVER_ID_1,
    #endif

    #if (UART_DRIVER_SUPPORT_UART2_CFG == DEF_ENABLED)
        UART_DRIVER_ID_2,
    #endif

    #if (UART_DRIVER_SUPPORT_UART3_CFG == DEF_ENABLED)
        UART_DRIVER_ID_3,
    #endif

    #if (UART_DRIVER_SUPPORT_UART4_CFG == DEF_ENABLED)
        UART_DRIVER_ID_4,
    #endif

    #if (UART_DRIVER_SUPPORT_UART5_CFG == DEF_ENABLED)
        UART_DRIVER_ID_5,
    #endif

    NB_OF_REAL_UART_DRIVER,
    INTERNAL_RESYNC_OFFSET = NB_OF_REAL_UART_DRIVER - 1,

    #if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
        UART_DRIVER_VIRTUAL,
    #endif

     NB_OF_UART_DRIVER,
};

enum UART_DMA_ID_e
{
    #if (UART_DRIVER_SUPPORT_UART1_DMA_CFG == DEF_ENABLED)
        UART_DMA_DRIVER_ID_1,
    #endif

    #if (UART_DRIVER_SUPPORT_UART2_DMA_CFG == DEF_ENABLED)
        UART_DMA_DRIVER_ID_2,
    #endif

    #if (UART_DRIVER_SUPPORT_UART3_DMA_CFG == DEF_ENABLED)
        UART_DMA_DRIVER_ID_3,
    #endif

    #if (UART_DRIVER_SUPPORT_UART4_DMA_CFG == DEF_ENABLED)
        UART_DMA_DRIVER_ID_4,
    #endif

    #if (UART_DRIVER_SUPPORT_UART5_DMA_CFG == DEF_ENABLED)
        UART_DMA_DRIVER_ID_5,
    #endif

     NB_OF_UART_DMA_DRIVER,
};

enum UART_Baud_e
{
    UART_BAUD_9600,
    UART_BAUD_19200,
    UART_BAUD_38400,
    UART_BAUD_57600,
    UART_BAUD_115200,
    UART_BAUD_230400,
    UART_BAUD_460800,
    UART_BAUD_921600,
    UART_BAUD_1843200,
    NB_OF_BAUD,
    UART_BAUD_IS_VIRTUAL,
};

//to do
enum UART_Config_e
{
    UART_ENABLE_RX_TX       =   0x0000000C,
    UART_ENABLE_RX          =   0x00000004,
    UART_ENABLE_TX          =   0x00000008,
    UART_ENABLE_RX_TX_MASK  =   0x0000000C,

    UART_NO_PARITY          =   0x00000000,
    UART_EVEN_PARITY        =   0x00000400,
    UART_ODD_PARITY         =   0x00000600,
    UART_PARITY_MASK        =   0x00000600,

    UART_8_LEN_BITS         =   0x00000000,
    UART_9_LEN_BITS         =   0x00001000,
    UART_LENGTH_MASK        =   0x00001000,

    UART_1_STOP_BIT         =   0x00000000,
    UART_0_5_STOP_BIT       =   0x10000000,
    UART_1_5_STOP_BIT       =   0x30000000,
    UART_2_STOP_BITS        =   0x20000000,
    UART_STOP_MASK          =   0x30000000,

    // Some more common config
    UART_CONFIG_N_8_1    =   (UART_NO_PARITY   | UART_8_LEN_BITS | UART_1_STOP_BIT),
    UART_CONFIG_N_9_1    =   (UART_NO_PARITY   | UART_9_LEN_BITS | UART_1_STOP_BIT),

    UART_CONFIG_E_8_1    =   (UART_EVEN_PARITY | UART_8_LEN_BITS | UART_1_STOP_BIT),
    UART_CONFIG_E_9_1    =   (UART_EVEN_PARITY | UART_9_LEN_BITS | UART_1_STOP_BIT),

    UART_CONFIG_O_8_1    =   (UART_ODD_PARITY  | UART_8_LEN_BITS | UART_1_STOP_BIT),
    UART_CONFIG_O_9_1    =   (UART_ODD_PARITY  | UART_9_LEN_BITS | UART_1_STOP_BIT),

    UART_CONFIG_N_8_2    =   (UART_NO_PARITY   | UART_8_LEN_BITS | UART_2_STOP_BITS),
    UART_CONFIG_N_9_2    =   (UART_NO_PARITY   | UART_9_LEN_BITS | UART_2_STOP_BITS),

    UART_CONFIG_E_8_2    =   (UART_EVEN_PARITY | UART_8_LEN_BITS | UART_2_STOP_BITS),
    UART_CONFIG_E_9_2    =   (UART_EVEN_PARITY | UART_9_LEN_BITS | UART_2_STOP_BITS),

    UART_CONFIG_O_8_2    =   (UART_ODD_PARITY  | UART_8_LEN_BITS | UART_2_STOP_BITS),
    UART_CONFIG_O_9_2    =   (UART_ODD_PARITY  | UART_9_LEN_BITS | UART_2_STOP_BITS),

    UART_CR1_CONFIG_MASK = 0x0000160C,        // TX RX Enable, Length (8 or 9 Bits), Parity (DISABLE, ODD, EVEN)
    UART_CR2_CONFIG_MASK = 0x00003000,        // STOP Bits,
};

struct UART_Info_t
{
    USART_TypeDef*      pUARTx;
    IO_ID_e             PinRX;
    IO_ID_e             PinTX;
    uint32_t            RCC_APBxPeriph;
    volatile uint32_t*  RCC_APBxEN_Register;
    IRQn_Type           IRQn_Channel;
    uint8_t             PreempPrio;
    UART_Config_e       Config;
    UART_Baud_e         BaudID;
    bool                IsItBlockingOnBusy;  // todo check if used
};

#if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
struct UART_DMA_Info_t
{
    UART_ID_e           UartID;
    DMA_Info_t          DMA_RX;
    DMA_Info_t          DMA_TX;
};

/*#if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
struct UART_DMA_Info_t
{
    UART_ID_e            UartID;
    uint32_t             FlagRX;
    DMA_Channel_TypeDef* DMA_ChannelRX;
    uint32_t             FlagTX;
    DMA_Channel_TypeDef* DMA_ChannelTX;
    IRQn_Type            Tx_IRQn;
    uint32_t             RCC_AHBxPeriph;
};
*/

struct UART_Transfer_t
{
    uint8_t*            pBuffer;
    size_t  Size;
    size_t              StaticSize;
};

#endif

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class UART_Driver
{
    public:

                            UART_Driver                     (UART_ID_e UartID);

        void                SetConfig                       (UART_Config_e Config, UART_Baud_e BaudID);
        void                SetBaudRate                     (UART_Baud_e BaudID);
        uint32_t            GetBaudRate                     (void);

        SystemState_e       SendData                        (const uint8_t* p_BufferTX, size_t* pSizeTX);

        bool                IsItBusy                        (void);

      #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
        void                DMA_ConfigRX                    (uint8_t* pBufferRX, size_t SizeRX);
        void                DMA_ConfigTX                    (uint8_t* pBufferTX, size_t SizeTX);
        void                DMA_EnableRX                    (void);
        void                DMA_DisableRX                   (void);
        void                DMA_EnableTX                    (void);
        void                DMA_DisableTX                   (void);
        size_t              DMA_GetSizeRX                   (uint16_t SizeRX);
      #endif

      #if (UART_ISR_CFG == DEF_ENABLED)
        void                RegisterCallback                (CallbackInterface* pCallback);
        void                EnableCallbackType              (int CallbackType);
      #endif

        void                Enable                          (void);
        void                Disable                         (void);

        void                IRQ_Handler                     (void);

       #if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
        void                VirtualUartRX_IRQHandler        (void);
        void                VirtualUartTX_IRQHandler        (void);
      #endif

    private:

        void                ClearFlag                       (void);

      #if (UART_ISR_RX_IDLE_CFG  == DEF_ENABLED) || (UART_ISR_RX_ERROR_CFG == DEF_ENABLED) || (UART_ISR_RX_CFG == DEF_ENABLED)
        void                EnableRX_ISR                    (uint8_t Mask);
        void                DisableRX_ISR                   (uint8_t Mask);
      #endif

      #if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED) || (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
        void                EnableTX_ISR                    (uint8_t Mask);
        void                DisableTX_ISR                   (uint8_t Mask);
      #endif

        UART_ID_e                   m_UartID;
        static const uint32_t       m_BaudRate[NB_OF_BAUD];
        UART_Info_t*                m_pInfo;
        USART_TypeDef*              m_pUart;
        UART_Transfer_t             m_RX_Transfer;
        UART_Transfer_t             m_TX_Transfer;

        uint32_t                    m_CopySR;

//from old stuff
//UART_Variables_t            m_Variables;
//void*                       m_pContextRX;       // This is the global context if there is no individual context set
//void*                       m_pContextTX;       // This is the global context if there is no individual context set

        // DMA Config
      //#if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
        //UART_DMA_Info_t*            m_pDMA_Info;
        //bool                        m_DMA_IsItBusyTX;
//      #endif

        // DMA Config
      #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
        DMA_Driver                  m_DMA_RX;
        DMA_Driver                  m_DMA_TX;
        UART_DMA_Info_t*            m_pDMA_Info;
        bool                        m_DMA_IsItBusyTX;
      #endif



      #if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
        bool                        m_VirtualUartBusyRX;
        bool                        m_VirtualUartBusyTX;
      #endif

      #if (UART_ISR_CFG == DEF_ENABLED)
        CallbackInterface*          m_pCallback;
        int                         m_CallBackType;
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "uart_var.h"         // Project variable

extern const UART_Info_t UART_Info[NB_OF_REAL_UART_DRIVER];

#if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
extern const UART_DMA_Info_t UART_DMA_Info[NB_OF_UART_DMA_DRIVER];
#endif

//-------------------------------------------------------------------------------------------------

#endif // (USE_UART_DRIVER == DEF_ENABLED)
