//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_virtual_uart.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_VIRTUAL_UART_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define UART_ISR_RX_IDLE_MASK                   0x10
#define UART_ISR_RX_NOT_EMPTY_MASK              0x20
#define UART_ISR_TX_COMPLETED_MASK              0x40
#define UART_ISR_TX_EMPTY_MASK                  0x80

// Callback type in bit position
#define UART_CALLBACK_NONE                      0x00
#define UART_CALLBACK_RX_IDLE                   0x02
#define UART_CALLBACK_RX_NOT_EMPTY              0x04
#define UART_CALLBACK_TX_COMPLETED              0x08
#define UART_CALLBACK_TX_EMPTY                  0x10
#define UART_CALLBACK_RX_DMA                    0x20
#define UART_CALLBACK_TX_DMA                    0x40

#if ((UART_DRIVER_RX_IDLE_CFG           != DEF_ENABLED) && \
     (UART_DRIVER_RX_NOT_EMPTY_CFG      != DEF_ENABLED) && \
     (UART_DRIVER_TX_COMPLETED_CFG      != DEF_ENABLED) && \
     (UART_DRIVER_TX_EMPTY_CFG          != DEF_ENABLED))

    #define UART_DRIVER_USE_CALLBACK_CFG     DEF_DISABLED
#else
    #define UART_DRIVER_USE_CALLBACK_CFG     DEF_ENABLED
#endif

#define UART_WAIT_ON_BUSY                    true
#define UART_DONT_WAIT_ON_BUSY               false

#define UART_BAUD_IS_VIRTUAL                    0


//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

// Four Virtual Uart are available, many can be added if necessary.
enum VUART_ID_e
{
    #if (UART_DRIVER_SUPPORT_VUART1_CFG == DEF_ENABLED)
        VUART_DRIVER_ID_1,
    #endif

    #if (UART_DRIVER_SUPPORT_VUART2_CFG == DEF_ENABLED)
        VUART_DRIVER_ID_2,
    #endif

    #if (UART_DRIVER_SUPPORT_VUART3_CFG == DEF_ENABLED)
        VUART_DRIVER_ID_3,
    #endif

    #if (UART_DRIVER_SUPPORT_VUART4_CFG == DEF_ENABLED)
        VUART_DRIVER_ID_4,
    #endif

     NB_OF_VUART_DRIVER,
};


struct UART_Info_t
{
    IRQn_Type           IRQn_Channel;
    uint8_t             PreempPrio;
    UART_Config_e       Config;
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
// create an interface
class VUART_Driver
{
    public:

                            VUART_Driver                    (VUART_ID_e VuartID);

        void                Initialize                      (void);
        void                SetConfig                       (UART_Config_e Config, UART_Baud_e BaudID);     { VAR_UNUSED(Config); VAR_UNUSED(BaudID);   }
        void                SetBaudRate                     (UART_Baud_e BaudID)                            { VAR_UNUSED(BaudID);                       }
        void                SetCustomBaudRate               (uint32_t Speed)                                { VAR_UNUSED(Speed);                        }
        uint32_t            GetBaudRate                     (void)                                          { return UART_BAUD_IS_VIRTUAL;              }

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

        void                VirtualUartRX_IRQHandler        (void);
        void                VirtualUartTX_IRQHandler        (void);

    private:

        void                ClearFlag                       (void);
        uint32_t            GetPeripheralClock              (void);

      #if ((UART_DRIVER_RX_NOT_EMPTY_CFG      == DEF_ENABLED) || \
           (UART_DRIVER_RX_IDLE_CFG           == DEF_ENABLED))

        void                EnableRX_ISR                    (uint8_t Mask);
        void                DisableRX_ISR                   (uint8_t Mask);
      #endif

      #if ((UART_DRIVER_TX_COMPLETED_CFG == DEF_ENABLED) || \
           (UART_DRIVER_TX_EMPTY_CFG     == DEF_ENABLED))
        void                EnableTX_ISR                    (uint8_t Mask);
        void                DisableTX_ISR                   (uint8_t Mask);
      #endif

        VUART_ID_e                  m_VuartID;
        static const uint32_t       m_BaudRate[NB_OF_BAUD];
        UART_Info_t*                m_pInfo;
        UART_Transfer_t             m_RX_Transfer;
        UART_Transfer_t             m_TX_Transfer;
        uint32_t                    m_CopySR;

        // DMA Config
        DMA_Driver                  m_DMA_RX;
        DMA_Driver                  m_DMA_TX;
        volatile bool               m_DMA_IsItBusyTX;

        bool                        m_VirtualUartBusyRX;
        bool                        m_VirtualUartBusyTX;

      #if (UART_DRIVER_USE_CALLBACK_CFG == DEF_ENABLED)
        CallbackInterface*          m_pCallback;
        //int                         m_CallBackType;  variables is not used at this time..
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "uart_var.h"         // Project variable

extern const VUART_Info_t UART_Info[NB_OF_VUART_DRIVER];

//-------------------------------------------------------------------------------------------------

#endif // (USE_VIRTUAL_UART_DRIVER == DEF_ENABLED)
