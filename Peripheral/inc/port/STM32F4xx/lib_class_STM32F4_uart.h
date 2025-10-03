//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_uart.h
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

#define UART_SR_RX_PARITY_ERROR_MASK            0x01
#define UART_SR_RX_FRAMING_ERROR_MASK           0x02
#define UART_SR_RX_NOISE_DETECTED_MASK          0x04
#define UART_SR_RX_OVERRUN_ERROR_MASK           0x08
#define UART_SR_RX_ERROR_MASK                   (UART_SR_RX_PARITY_ERROR_MASK | UART_SR_RX_FRAMING_ERROR_MASK | UART_SR_RX_NOISE_DETECTED_MASK | UART_SR_RX_OVERRUN_ERROR_MASK)
#define UART_SR_RX_IDLE_MASK                    0x10
#define UART_SR_RX_NOT_EMPTY_MASK               0x20
#define UART_SR_TX_COMPLETED_MASK               0x40
#define UART_SR_TX_EMPTY_MASK                   0x80

// Callback type in bit position
#define UART_CALLBACK_NONE                      0x00
#define UART_CALLBACK_RX_ERROR                  0x01
#define UART_CALLBACK_RX_IDLE                   0x02
#define UART_CALLBACK_RX_NOT_EMPTY              0x04
#define UART_CALLBACK_TX_COMPLETED              0x08
#define UART_CALLBACK_TX_EMPTY                  0x10
#define UART_CALLBACK_RX_DMA                    0x20
#define UART_CALLBACK_TX_DMA                    0x40

#if ((UART_DRIVER_RX_PARITY_ERROR_CFG   == DEF_ENABLED) || \
     (UART_DRIVER_RX_FRAMING_ERROR_CFG  == DEF_ENABLED) || \
     (UART_DRIVER_RX_NOISE_DETECTED_CFG == DEF_ENABLED) || \
     (UART_DRIVER_RX_OVERRUN_ERROR_CFG  == DEF_ENABLED))

    #define UART_DRIVER_RX_ERROR_CFG            DEF_ENABLED
#endif

#if ((UART_DRIVER_RX_PARITY_ERROR_CFG   != DEF_ENABLED) && \
     (UART_DRIVER_RX_FRAMING_ERROR_CFG  != DEF_ENABLED) && \
     (UART_DRIVER_RX_NOISE_DETECTED_CFG != DEF_ENABLED) && \
     (UART_DRIVER_RX_OVERRUN_ERROR_CFG  != DEF_ENABLED) && \
     (UART_DRIVER_RX_IDLE_CFG           != DEF_ENABLED) && \
     (UART_DRIVER_RX_NOT_EMPTY_CFG      != DEF_ENABLED) && \
     (UART_DRIVER_TX_COMPLETED_CFG      != DEF_ENABLED) && \
     (UART_DRIVER_TX_EMPTY_CFG          != DEF_ENABLED))

    #define UART_DRIVER_USE_CALLBACK_CFG     DEF_DISABLED
#else
    #define UART_DRIVER_USE_CALLBACK_CFG     DEF_ENABLED
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

    #if (UART_DRIVER_SUPPORT_UART6_CFG == DEF_ENABLED)
        UART_DRIVER_ID_6,
    #endif

    #if (UART_DRIVER_SUPPORT_UART7_CFG == DEF_ENABLED)
        UART_DRIVER_ID_7,
    #endif

    #if (UART_DRIVER_SUPPORT_UART8_CFG == DEF_ENABLED)
        UART_DRIVER_ID_8,
    #endif

     NB_OF_UART_DRIVER,
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

enum UART_Config_e
{
    UART_CFG_NO_PARITY          =   0x00000000,
    UART_CFG_EVEN_PARITY        =   0x00000400,
    UART_CFG_ODD_PARITY         =   0x00000600,
    UART_CFG_PARITY_MASK        =   0x00000600,

    UART_CFG_8_LEN_BITS         =   0x00000000,
    UART_CFG_9_LEN_BITS         =   0x00001000,
    UART_CFG_LENGTH_MASK        =   0x00001000,

    UART_CFG_1_STOP_BIT         =   0x00000000,
    UART_CFG_0_5_STOP_BIT       =   0x10000000,
    UART_CFG_1_5_STOP_BIT       =   0x30000000,
    UART_CFG_2_STOP_BITS        =   0x20000000,
    UART_CFG_STOP_MASK          =   0x30000000,

    UART_CFG_OVER_16            =   0x00000000,
    UART_CFG_OVER_8             =   0x00008000,
    UART_CFG_OVER_MASK          =   0x00008000,

    UART_CFG_ENABLE_RX_TX       =   0x0000000C,
    UART_CFG_ENABLE_RX          =   0x00000004,
    UART_CFG_ENABLE_TX          =   0x00000008,
    UART_CFG_ENABLE_MASK        =   0x0000000C,

    // Some more common config (all LSB with oversampling at 16, with RX and TX)
    UART_CFG_N_8_1    =   (UART_CFG_NO_PARITY   | UART_CFG_8_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_N_9_1    =   (UART_CFG_NO_PARITY   | UART_CFG_9_LEN_BITS | UART_CFG_1_STOP_BIT),

    UART_CFG_E_8_1    =   (UART_CFG_EVEN_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_E_9_1    =   (UART_CFG_EVEN_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_1_STOP_BIT),

    UART_CFG_O_8_1    =   (UART_CFG_ODD_PARITY  | UART_CFG_8_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_O_9_1    =   (UART_CFG_ODD_PARITY  | UART_CFG_9_LEN_BITS | UART_CFG_1_STOP_BIT),

    UART_CFG_N_8_2    =   (UART_CFG_NO_PARITY   | UART_CFG_8_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_N_9_2    =   (UART_CFG_NO_PARITY   | UART_CFG_9_LEN_BITS | UART_CFG_2_STOP_BITS),

    UART_CFG_E_8_2    =   (UART_CFG_EVEN_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_E_9_2    =   (UART_CFG_EVEN_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_2_STOP_BITS),

    UART_CFG_O_8_2    =   (UART_CFG_ODD_PARITY  | UART_CFG_8_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_O_9_2    =   (UART_CFG_ODD_PARITY  | UART_CFG_9_LEN_BITS | UART_CFG_2_STOP_BITS),

    UART_CFG_CR1_MASK = 0x0000160C,        // TX RX Enable, Length (8 or 9 Bits), Parity (DISABLE, ODD, EVEN)
    UART_CFG_CR2_MASK = 0x00003000,        // STOP Bits,
};

struct UART_Info_t
{
    USART_TypeDef*      pUARTx;
    IRQn_Type           IRQn_Channel;
    uint8_t             PreempPrio;
    UART_Config_e       Config;
    UART_Baud_e         BaudID;
    bool                IsItBlockingOnBusy;  // todo check if used
    DMA_Info_t          DMA_RX;
    DMA_Info_t          DMA_TX;
};

struct UART_Transfer_t
{
    uint8_t*            pBuffer;
    union
    {
        size_t          Size;
        size_t          Head;               // if used in circular buffer
    } u;
    size_t              StaticSize;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class UART_Driver
{
    public:

                            UART_Driver                     (UART_ID_e UartID);

        void                Initialize                      (void);
        void                SetConfig                       (UART_Config_e Config, UART_Baud_e BaudID);
        void                SetBaudRate                     (UART_Baud_e BaudID);
        void                SetCustomBaudRate               (uint32_t Speed);
        uint32_t            GetBaudRate                     (void);

        SystemState_e       SendData                        (const uint8_t* p_BufferTX, size_t* pSizeTX);

        bool                IsItBusy                        (void);

        void                DMA_ConfigRX                    (uint8_t* pBufferRX, size_t SizeRX);
        void                DMA_ConfigTX                    (uint8_t* pBufferTX, size_t SizeTX);
        void                DMA_EnableRX                    (void);
        void                DMA_DisableRX                   (void);
        void                DMA_EnableTX                    (void);
        void                DMA_DisableTX                   (void);
        size_t              DMA_GetSizeRX                   (uint16_t SizeRX);

      #if (UART_DRIVER_USE_CALLBACK_CFG == DEF_ENABLED)
        void                RegisterCallback                (CallbackInterface* pCallback);
        void                EnableCallbackType              (int CallbackType);
      #endif

        void                Enable                          (void);
        void                Disable                         (void);

        void                IRQ_Handler                     (void);
        void                DMA_TX_IRQ_Handler              (void);

    private:

        void                ClearFlag                       (void);
        uint32_t            GetPeripheralClock              (void);

      #if ((UART_DRIVER_RX_ERROR_CFG          == DEF_ENABLED) || \
           (UART_DRIVER_RX_NOT_EMPTY_CFG      == DEF_ENABLED) || \
           (UART_DRIVER_RX_IDLE_CFG           == DEF_ENABLED))

        void                EnableRX_ISR                    (uint8_t Mask);
        void                DisableRX_ISR                   (uint8_t Mask);
      #endif

      #if ((UART_DRIVER_TX_COMPLETED_CFG == DEF_ENABLED) || \
           (UART_DRIVER_TX_EMPTY_CFG     == DEF_ENABLED))
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
        uint32_t                    m_ClockFrequency;

        // DMA Config
        DMA_Driver                  m_DMA_RX;
        DMA_Driver                  m_DMA_TX;
        volatile bool               m_DMA_IsItBusyTX;

      #if (UART_DRIVER_USE_CALLBACK_CFG == DEF_ENABLED)
        CallbackInterface*          m_pCallback;
        //int                         m_CallBackType;  variables is not used at this time..
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "uart_var.h"         // Project variable

extern const UART_Info_t UART_Info[NB_OF_UART_DRIVER];

//-------------------------------------------------------------------------------------------------

#endif // (USE_UART_DRIVER == DEF_ENABLED)
