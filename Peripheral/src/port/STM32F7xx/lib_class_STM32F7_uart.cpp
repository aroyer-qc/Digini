//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_uart.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define UART_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  UART_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_UART_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define UART_BACK_OFFSET_RESET_REGISTER     0x20

#define UART_CR1_PARITY_NONE                0                                   // Parity control disabled
#define UART_CR1_PARITY_EVEN                USART_CR1_PCE                       // Parity control enabled and Even Parity is selected
#define UART_CR1_PARITY_ODD                 (USART_CR1_PCE | USART_CR1_PS)      // Parity control enabled and Odd Parity is selected

#define UART_CR1_DATA_WIDTH_8B              0                                   // 8 bits word length
#define UART_CR1_DATA_WIDTH_9B              USART_CR1_M0                        // 9 bits word length
#define UART_CR1_DATA_WIDTH_7B              USART_CR1_M1                        // 9 bits word length

#define UART_CR1_OVERSAMPLING_16            0                                   // Oversampling by 16
#define UART_CR1_OVERSAMPLING_8             USART_CR1_OVER8                     // Oversampling by 8

#define UART_CR1_OVER_16                    0
#define UART_CR1_OVER_8                     USART_CR1_OVER8

#define UART_CR1_RX                         USART_CR1_RE
#define UART_CR1_TX                         USART_CR1_TE
#define UART_CR1_RX_TX                      USART_CR1_RE | USART_CR1_TE

#define UART_CR2_STOP_1B                    0                                   // 1   stop bit
#define UART_CR2_STOP_0_5B                  USART_CR2_STOP_0                    // 0.5 stop bit
#define UART_CR2_STOP_2_B                   USART_CR2_STOP_1                    // 2   stop bits
#define UART_CR2_STOP_1_5B                  USART_CR2_STOP_0 | USART_CR2_STOP_1 // 1.5 stop bits

#define UART_CR2_LSB_FIRST                  0
#define UART_CR2_MSB_FIRST                  USART_CR2_MSBFIRST

#define UART_CR3_FLOW_CTS                   USART_CR3_CTSE
#define UART_CR3_FLOW_CTS_ISR               USART_CR3_CTSIE
#define UART_CR3_FLOW_RTS                   USART_CR3_RTSE

//-------------------------------------------------------------------------------------------------
//  private variable(s)
//-------------------------------------------------------------------------------------------------

const uint32_t UART_Driver::m_BaudRate[NB_OF_BAUD] =
{
    9600,
    19200,
    38400,
    57600,
    115200,
    230400,
    460800,
    921600,
    1843200,
};

//-------------------------------------------------------------------------------------------------
//
//   Class: UART_Driver
//
//
//   Description:   Class to handle UART
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   UART_Driver
//
//   Parameter(s):  UartID                  ID for the data to use for this class
//
//   Description:   Initializes the UART_Driver class
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
UART_Driver::UART_Driver(UART_ID_e UartID)
{
    DMA_Stream_TypeDef* pDMA;

  #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
    m_pDMA_Info      = nullptr;
    m_DMA_IsItBusyTX = false;
  #endif

    if(UartID < NB_OF_REAL_UART_DRIVER)
    {
        m_UartID = UartID;
        m_pInfo  = (UART_Info_t*)&UART_Info[UartID];
        m_pUart  = m_pInfo->pUARTx;

        *(uint32_t*)((uint32_t)m_pInfo->RCC_APBxEN_Register - UART_BACK_OFFSET_RESET_REGISTER) |=  m_pInfo->RCC_APBxPeriph;
        *(uint32_t*)((uint32_t)m_pInfo->RCC_APBxEN_Register - UART_BACK_OFFSET_RESET_REGISTER) &= ~m_pInfo->RCC_APBxPeriph;
        *(m_pInfo->RCC_APBxEN_Register) |= m_pInfo->RCC_APBxPeriph;

        SetConfig(m_pInfo->Config, m_pInfo->BaudID);

        IO_PinInit(m_pInfo->PinRX);
        IO_PinInit(m_pInfo->PinTX);

      #if (UART_ISR_RX_CFG == DEF_ENABLED)  || (UART_ISR_RX_IDLE_CFG == DEF_ENABLED)  || (UART_ISR_RX_ERROR_CFG == DEF_ENABLED) || \
          (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED) || (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
        m_pCallback    = nullptr;
        m_CallBackType = UART_CALLBACK_NONE;
      #endif

        ISR_Init(m_pInfo->IRQn_Channel, 0, m_pInfo->PreempPrio);
        ClearFlag();

      #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
        for(uint32_t i = 0; i < (uint32_t)NB_OF_UART_DMA_DRIVER; i++)
        {
            if(UART_DMA_Info[i].UartID == UartID)
            {
                m_pDMA_Info = (UART_DMA_Info_t*)&UART_DMA_Info[i];
                i = NB_OF_UART_DMA_DRIVER;
            }
        }

        if(m_pDMA_Info != nullptr)
        {
            SET_BIT(RCC->AHB1ENR, m_pDMA_Info->RCC_AHBxPeriph);                 // Initialize DMA clock

            pDMA = m_pDMA_Info->DMA_StreamRX;                                   // Write config that will never change
            pDMA->CR = DMA_MODE_NORMAL                    |
                       DMA_PERIPH_TO_MEMORY               |
                       DMA_PERIPH_NO_INCREMENT            |
                       DMA_MEMORY_INCREMENT               |
                       DMA_P_DATA_ALIGN_BYTE              |
                       DMA_M_DATA_ALIGN_BYTE              |
                       DMA_P_BURST_SINGLE                 |
                       DMA_M_BURST_SINGLE                 |
                       DMA_PRIORITY_HIGH;
            SET_BIT(pDMA->CR, m_pDMA_Info->DMA_ChannelRX);
            pDMA->PAR = uint32_t(&m_pUart->RDR);
            pDMA->NDTR = UART_DRIVER_INTERNAL_RX_BUFFER_SIZE;

            pDMA = m_pDMA_Info->DMA_StreamTX;                                   // Write config that will never change
            pDMA->CR = DMA_MODE_NORMAL                    |
                       DMA_MEMORY_TO_PERIPH               |
                       DMA_PERIPH_NO_INCREMENT            |
                       DMA_MEMORY_INCREMENT               |
                       DMA_P_DATA_ALIGN_BYTE              |
                       DMA_M_DATA_ALIGN_BYTE              |
                       DMA_P_BURST_SINGLE                 |
                       DMA_M_BURST_SINGLE                 |
                       DMA_PRIORITY_HIGH;
            SET_BIT(pDMA->CR, m_pDMA_Info->DMA_ChannelTX);
            pDMA->PAR = uint32_t(&m_pUart->TDR);
        }

        m_DMA_IsItBusyTX = false;
        memset(&m_RX_Transfer, 0x00, sizeof(UART_Transfer_t));
      #endif

        memset(&m_TX_Transfer, 0x00, sizeof(UART_Transfer_t));
    }
  #if (SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
    else if(m_UartID == UART_DRIVER_VIRTUAL)
    {
        m_VirtualUartBusyRX = false;
        m_VirtualUartBusyTX = false;

        ISR_Init(VirtualUartRX_IRQn, 0, m_pInfo->PreempPrio);
        ISR_Init(VirtualUartTX_IRQn, 0, m_pInfo->PreempPrio);
    }
  #endif
    else
    {
        m_pInfo = nullptr;
        m_pUart = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Enable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable uart
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::Enable(void)
{
    if(m_pUart != nullptr)
    {
        SET_BIT(m_pUart->CR1, USART_CR1_UE);              // Enable the USART
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Disable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable uart RX/TX, isr and uart
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::Disable(void)
{
    if(m_pUart != nullptr)
    {
      #if (UART_ISR_RX_ERROR_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pUart->CR3, USART_CR3_EIE);
      #endif

      #if (UART_ISR_RX_BYTE_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pUart->CR1, USART_CR1_RXNEIE);
      #endif

      #if (UART_ISR_RX_IDLE_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pUart->CR1, USART_CR1_IDLEIE);
      #endif

      #if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pUart->CR1, USART_CR1_TXEIE);
      #endif

      #if (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
        CLEAR_BIT(m_pUart->CR1, USART_CR1_TCIE);
      #endif

      #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
        DMA_DisableRX();
        DMA_DisableTX();
      #endif

        CLEAR_BIT(m_pUart->CR1, USART_CR1_UE);    // Disable the UART
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetBaudRate
//
//  Parameter(s):   BaudRate             BaudRate ID
//                  ReEnable             if true the UART will be re enable
//  Return:         None
//
//  Description:    Set UART Baudrate
//
//  Note(s):        - Disable uart
//                  - Set baud rate and uart properties
//                  - Enable uart if ReEnable is true
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetBaudRate(UART_Baud_e BaudRate)
{
    uint32_t PeriphClk;

    if(m_pUart != nullptr)
    {
        CLEAR_BIT(m_pUart->CR1, USART_CR1_UE);                      // Disable the UART

        // Retrieve Clock frequency used for USART Peripheral
      #if (UART_DRIVER_SUPPORT_UART1_CFG == DEF_ENABLED)
        if(m_pUart == USART1)
        {
          PeriphClk = SYS_APB2_CLOCK_FREQUENCY;
        }
      #endif

      #if (UART_DRIVER_SUPPORT_UART2_CFG == DEF_ENABLED)
        if(m_pUart == USART2)
        {
          PeriphClk = SYS_APB1_CLOCK_FREQUENCY;
        }
      #endif

      #if (UART_DRIVER_SUPPORT_UART3_CFG == DEF_ENABLED)
        if(m_pUart == USART3)
        {
          PeriphClk = SYS_APB1_CLOCK_FREQUENCY;
        }
      #endif

      #if (UART_DRIVER_SUPPORT_UART4_CFG == DEF_ENABLED)
        if(m_pUart == UART4)
        {
          PeriphClk = SYS_APB1_CLOCK_FREQUENCY;
        }
      #endif

      #if (UART_DRIVER_SUPPORT_UART5_CFG == DEF_ENABLED)
        if(m_pUart == UART5)
        {
          PeriphClk = SYS_APB1_CLOCK_FREQUENCY;
        }
      #endif

      #if (UART_DRIVER_SUPPORT_UART6_CFG == DEF_ENABLED)
        if(m_pUART == USART6)
        {
          PeriphClk = SYS_APB2_CLOCK_FREQUENCY;
        }
      #endif

      #if (UART_DRIVER_SUPPORT_UART7_CFG == DEF_ENABLED)
        if(m_pUart == UART7)
        {
          PeriphClk = SYS_APB1_CLOCK_FREQUENCY;
        }
      #endif

      #if (UART_DRIVER_SUPPORT_UART8_CFG == DEF_ENABLED)
        if(m_pUart == UART8)
        {
          PeriphClk = SYS_APB1_CLOCK_FREQUENCY;
        }
      #endif

        if((m_pUart->CR1 & USART_CR1_OVER8_Msk) == UART_CR1_OVER_8)
        {
            PeriphClk <<= 1;
        }

        m_pUart->BRR = uint16_t(PeriphClk / m_BaudRate[BaudRate]);
        SET_BIT(m_pUart->CR1, USART_CR1_UE);                                // Enable the UART
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetConfig
//
//  Parameter(s):   Config               Config for UART
//                  BaudID               Baud ID
//  Return:         None
//
//  Description:    Set UART Config
//
//  Note(s):        - Disable uart
//                  - Set baud rate and uart properties
//                  - Enable uart
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetConfig(UART_Config_e Config, UART_Baud_e BaudID)
{
    if(m_pUart != nullptr)
    {
        uint32_t       CR1_Register = 0;
        uint32_t       CR2_Register = 0;
        uint32_t       CR3_Register = 0;
        UART_Config_e  MaskedConfig;

        CLEAR_BIT(m_pUart->CR1, USART_CR1_UE);                      // Disable the UART

        // Parity
        MaskedConfig = UART_Config_e(uint32_t(Config) & UART_PARITY_MASK);

        if(MaskedConfig == UART_EVEN_PARITY)
        {
            CR1_Register |= UART_CR1_PARITY_EVEN;
        }
        else if(MaskedConfig == UART_ODD_PARITY)
        {
            CR1_Register |= UART_CR1_PARITY_ODD;
        }

        // Length
        MaskedConfig = UART_Config_e(uint32_t(Config) & UART_LENGTH_MASK);

        if(MaskedConfig == UART_7_LEN_BITS)
        {
            CR1_Register |= UART_CR1_DATA_WIDTH_7B;
        }
        else if(MaskedConfig == UART_9_LEN_BITS)
        {
            CR1_Register |= UART_CR1_DATA_WIDTH_9B;
        }

        // Stop Bits
        MaskedConfig = UART_Config_e(uint32_t(Config) & UART_STOP_MASK);

        switch(MaskedConfig)
        {
            case UART_0_5_STOP_BIT: CR2_Register |= UART_CR2_STOP_0_5B; break;
            case UART_1_5_STOP_BIT: CR2_Register |= UART_CR2_STOP_1_5B; break;
            case UART_2_STOP_BITS:  CR2_Register |= UART_CR2_STOP_2_B;  break;
            default: break;
        }

        // LSB or MSB
        if((Config & UART_DATA_ORDER_MASK) == UART_DATA_ORDER_MSB)
        {
            CR2_Register |= UART_CR2_MSB_FIRST;
        }

        // OverSampling 8 or 16
        if((Config & UART_OVER_MASK) == UART_OVER_8)
        {
            CR1_Register |= UART_CR1_OVER_8;
        }

        // RX and TX enable
        MaskedConfig = UART_Config_e(uint32_t(Config) & UART_ENABLE_MASK);

        switch(MaskedConfig)
        {
            case UART_ENABLE_RX:    CR1_Register |= UART_CR1_RX;    break;
            case UART_ENABLE_TX:    CR1_Register |= UART_CR1_TX;    break;
            case UART_ENABLE_RX_TX: CR1_Register |= UART_CR1_RX_TX; break;
            default: break;
        }

        // Register modification
        MODIFY_REG(m_pUart->CR1, (USART_CR1_M_Msk | USART_CR1_PCE_Msk | USART_CR1_PS_Msk | USART_CR1_OVER8_Msk), CR1_Register);
        MODIFY_REG(m_pUart->CR2, (USART_CR2_STOP_Msk | USART_CR2_MSBFIRST_Msk),                                  CR2_Register);
        MODIFY_REG(m_pUart->CR3, (USART_CR3_CTSE_Msk | USART_CR3_CTSIE_Msk | USART_CR3_RTSE_Msk),                CR3_Register);
        SetBaudRate(BaudID);        // Will re-enable the UART
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsItBusy
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Check if UART is busy
//
//-------------------------------------------------------------------------------------------------
bool UART_Driver::IsItBusy(void)
{
    if(m_pUart != nullptr)
    {
        return m_DMA_IsItBusyTX;
    }
  #if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
    else if(m_UartID == UART_DRIVER_VIRTUAL)
    {
        if((m_VirtualUartBusyRX == true) || (m_VirtualUartBusyTX == true))
        {
            return true;
        }
    }
  #endif

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetBaudRate
//
//  Parameter(s):   None
//  Return:         uint32_t    Baud rate
//
//  Description:    Return the real value of the baud rate
//
//-------------------------------------------------------------------------------------------------
uint32_t UART_Driver::GetBaudRate(void)
{
    return m_BaudRate[m_pInfo->BaudID];
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearFlag
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear flag CMCF, CTSCF, TCCF, IDLECF, ORECF, FECF, PECF
//
//  Note(s):        This method is use to clear any of those flag. It also do a read on the data
//                  register.
//                  No check for valid UART as this is called inside class function
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::ClearFlag(void)
{
    volatile uint32_t tmpreg;

    m_pUart->ICR = (USART_ICR_CMCF | USART_ICR_CTSCF | USART_ICR_TCCF | USART_ICR_IDLECF | USART_ICR_ORECF| USART_ICR_FECF | USART_ICR_PECF);
    tmpreg = m_pUart->RDR;
    VAR_UNUSED(tmpreg);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           VirtualSendData
//
//  Parameter(s):   pBuffer         Pointer on the buffer containing the data to send
//                  Size            Status
//  Return:         None
//
//  Description:    Send data using virtual comm driver with software ISR
//
//-------------------------------------------------------------------------------------------------
#if (UART_SUPPORT_VIRTUAL_CFG == DEF_ENABLED)
void UART_Driver::VirtualSendData(const uint8_t* pBuffer, uint16_t Size)
{
    ReceivedFromVirtualUart(pBuffer, Size);
    ISR_SetPendingIRQ(VirtualUartTX_IRQn);
    m_VirtualUartBusyTX = true;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function:      SendData
//
//   Parameter(s):  p_BufferTX  Ptr on buffer with data to send.
//                              if = nullptr, internal TX Buffer remains the one set previously
//                  pSizeTX     Number of bytes to send, and on return, number of bytes sent
//   Return Value:  SystemState_e
//
//   Description:   Send a data packet to the UART in DMA
//
//-------------------------------------------------------------------------------------------------
SystemState_e UART_Driver::SendData(const uint8_t* pBufferTX, size_t* pSizeTX)
{
    SystemState_e       State = SYS_READY;
    uint32_t            Flag;

    if(m_pUart != nullptr)
    {
        if(*pSizeTX != 0)
        {
          #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
            DMA_Stream_TypeDef* pDMA;

            if(m_pInfo->IsItBlockingOnBusy == true)
            {
                while(m_DMA_IsItBusyTX ==true)
                {
                    nOS_Yield();
                }
            }

            if(m_DMA_IsItBusyTX == false)
            {
                m_DMA_IsItBusyTX = true;
                pDMA             = m_pDMA_Info->DMA_StreamTX;
                Flag             = m_pDMA_Info->FlagTX;

                DMA_Disable(pDMA);
                DMA_ClearFlag(pDMA, Flag);

                if(pBufferTX != nullptr)
                {
                    pDMA->M0AR = (uint32_t)pBufferTX;
                    pDMA->NDTR = *pSizeTX;
                    m_TX_Transfer.Size    = *pSizeTX;
                    m_TX_Transfer.pBuffer = (uint8_t*)pBufferTX;
                }
                else
                {
                    pDMA->M0AR = (uint32_t)m_TX_Transfer.pBuffer;
                    pDMA->NDTR = m_TX_Transfer.Size;
                }

                ClearFlag();
                DMA_Enable(pDMA);                    // Transmission starts as soon as TXE is detected
                DMA_EnableTX();
            }
            else
            {
                State = SYS_BUSY;
            }
          #elif
                // Setup IRQ Transfer TODO
          #endif
        }
        else
        {
            State = SYS_READY;
        }
    }
  #if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
    else if(m_UartID == UART_DRIVER_VIRTUAL)
    {
        nOS_EnterCritical();

        if(m_VirtualUartBusyRX == false)
        {
            m_VirtualUartBusyRX = true;
            nOS_LeaveCritical();

            memcpy(m_pDMA_BufferRX, pBuffer, Size);
            m_SizeRX = Size;
            NVIC_SetPendingIRQ(VirtualUartRX_IRQn);
            state = SYS_READY;
        }
        else
        {
            state =  SYS_BUSY;
        }

        nOS_LeaveCritical();

  #endif
    else
    {
        State = SYS_WRONG_VALUE;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//  DDDDDD  MM     MM  AAAAA
//  DD   DD MMMM MMMM AA   AA
//  DD   DD MM MMM MM AAAAAAA
//  DD   DD MM  M  MM AA   AA
//  DDDDDD  MM     MM AA   AA
//-------------------------------------------------------------------------------------------------

#if (UART_DRIVER_ANY_DMA_OR_VIRTUAL_CFG == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_ConfigRX
//
//   Parameter(s):  pBufferRX   Ptr on buffer to store data received.
//                              if pBufferRX = nullptr, pBufferRX remains the one set previously
//                  SizeRX      Number of data max to received
//   Return Value:  None
//
//   Description:   Config the RX DMA to receive data
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_ConfigRX(uint8_t* pBufferRX, size_t SizeRX)
{
    UART_Transfer_t* pTransferRX = &m_RX_Transfer;

    if(m_pUart != nullptr)
    {
        DMA_Stream_TypeDef* pDMA;

        DMA_DisableRX();
        pDMA = m_pDMA_Info->DMA_StreamRX;

        if(pBufferRX != nullptr)
        {
            pDMA->M0AR = (uint32_t)pBufferRX;
            pDMA->NDTR = SizeRX;
            pTransferRX->pBuffer    = pBufferRX;
            pTransferRX->Size       = SizeRX;
            pTransferRX->StaticSize = SizeRX;
        }
        else
        {
            pDMA->M0AR = uint32_t(pTransferRX->pBuffer);
            pDMA->NDTR = pTransferRX->StaticSize;
            pTransferRX->Size = pTransferRX->StaticSize;
        }

        DMA_EnableRX();
    }
  #if (SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
    else if(m_UartID == UART_DRIVER_VIRTUAL)
    {
        if(pBufferRX != nullptr)
        {
            pTransferRX->Size       = SizeRX;
            pTransferRX->StaticSize = SizeRX;
            pTransferRX->pBuffer    = pBufferRX;
        }
        else
        {
            pTransferRX->Size = pTransferRX->StaticSize;
        }
    }
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_ConfigTX
//
//   Parameter(s):  pBufferRX       Pointer on the RX buffer
//                  SizeRX          Size of RX buffer
//                  pBufferTX       Pointer on the TX buffer
//                  SizeTX          Size of TX buffer
//
//   Return Value:  None
//
//   Description:   Initialization of UART DMA
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_ConfigTX(uint8_t* pBufferTX, size_t SizeTX)
{
     UART_Transfer_t* pTransferTX = &m_TX_Transfer;

    if(m_pUart != nullptr)
    {
        DMA_Stream_TypeDef* pDMA;

        DMA_DisableTX();
        pDMA = m_pDMA_Info->DMA_StreamTX;

        if(pBufferTX != nullptr)
        {
            pDMA->M0AR = uint32_t(pBufferTX);
            pDMA->NDTR = SizeTX;
            pTransferTX->pBuffer    = pBufferTX;
            pTransferTX->Size       = SizeTX;
            pTransferTX->StaticSize = SizeTX;
        }
        else
        {
            pDMA->M0AR = (uint32_t)pTransferTX->pBuffer;
            pDMA->NDTR = pTransferTX->StaticSize;
            pTransferTX->Size = pTransferTX->StaticSize;
        }

        DMA_EnableTX();
    }

  #if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
    else if(m_UartID == UART_DRIVER_VIRTUAL)
    {
        if(pBufferTX != nullptr)
        {
            pTransferTX->pBuffer    = pBufferTX;
            pTransferTX->Size       = SizeTX;
            pTransferTX->StaticSize = SizeTX;
        }
        else
        {
            pTransferTX->Size = pTransferTX->StaticSize;
        }
    }
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_EnableRX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Enable DMA receive
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_EnableRX(void)
{
    if(m_pUart != nullptr)
    {
        if(m_pDMA_Info != nullptr)
        {
            DMA_Stream_TypeDef* pDMA;

            pDMA = m_pDMA_Info->DMA_StreamRX;
            m_pUart->CR3 |= USART_CR3_DMAR;         // Enable the DMA transfer
            (void)m_pUart->RDR;
            DMA_ClearFlag(pDMA, m_pDMA_Info->FlagRX);
            DMA_Enable(pDMA);
            EnableRX_ISR(UART_ISR_RX_ERROR | UART_ISR_RX_IDLE);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_DisableRX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Disable DMA receive
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_DisableRX(void)
{
    if(m_pUart != nullptr)
    {
        if(m_pDMA_Info != nullptr)
        {
            DMA_Stream_TypeDef* pDMA;

            pDMA = m_pDMA_Info->DMA_StreamRX;
            DMA_Disable(pDMA);
            CLEAR_BIT(m_pUart->CR3, USART_CR3_DMAR);
            DisableRX_ISR(UART_ISR_RX_ERROR | UART_ISR_RX_IDLE);
            DMA_ClearFlag(pDMA, m_pDMA_Info->FlagRX);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_EnableTX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Enable DMA transmit
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_EnableTX(void)
{
    if(m_pUart != nullptr)
    {
        if(m_pDMA_Info != nullptr)
        {
            EnableTX_ISR(UART_ISR_TX_COMPLETED);
            m_pUart->CR3 |= USART_CR3_DMAT;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_DisableTX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Disable DMA transmit
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_DisableTX(void)
{
    if(m_pUart != nullptr)
    {
        if(m_pDMA_Info != nullptr)
        {
            DMA_Stream_TypeDef* pDMA;

            pDMA = m_pDMA_Info->DMA_StreamTX;
            DMA_Disable(pDMA);
            DMA_ClearFlag(pDMA, m_pDMA_Info->FlagTX);
            CLEAR_BIT(m_pUart->CR3, USART_CR3_DMAT);
        }
    }
}
/*
//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_GetInfoRX
//
//   Parameter(s):  None
//   Return Value:  UART_Transfer_t*      Pointer on the DMA info
//
//   Description:   return the actual data struct for DMA transfer
//
//-------------------------------------------------------------------------------------------------
UART_Transfer_t* UART_Driver::GetInfoRX(void)
{
    if(m_UartID < NB_OF_UART_DRIVER)
    {
        return &m_RX_Transfer;
    }
    return nullptr;
}
*/
//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_GetSizeRX
//
//   Parameter(s):  None
//   Return Value: size_t       Number of bytes
//
//   Description:   Get number of bytes received
//
//-------------------------------------------------------------------------------------------------
size_t UART_Driver::DMA_GetSizeRX(uint16_t SizeRX)
{
    size_t SizeDataRX = 0;

    if(m_pUart != nullptr)
    {
        DMA_Stream_TypeDef* pDMA;

        pDMA = m_pDMA_Info->DMA_StreamRX;

        DMA_Disable(pDMA);
        DMA_ClearFlag(pDMA, m_pDMA_Info->FlagRX);
        SizeDataRX = pDMA->NDTR;                     // Number of byte available in p_RxBuf

        if(SizeDataRX <= SizeRX)
        {
            SizeDataRX = (uint16_t)(SizeRX - SizeDataRX);
        }
        else
        {
            SizeDataRX = SizeRX;
        }

        DMA_Enable(pDMA);
    }
  #if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
    else if(m_UartID == UART_DRIVER_VIRTUAL)
    {
        SizeDataRX = m_VirtualVar.SizeRX;
    }
  #endif

    return SizeDataRX;
}


// TODO or check : Do i need DMA IRQ or IDLE will manage everything here...



//-------------------------------------------------------------------------------------------------

#endif // UART_DRIVER_ANY_DMA_OR_VIRTUAL_CFG == DEF_ENABLED

//-------------------------------------------------------------------------------------------------
//  IIII RRRRRR    QQQQQ
//   II  RR   RR  QQ   QQ
//   II  RRRRRR   QQ   QQ
//   II  RR  RR   QQ  QQQ
//  IIII RR   RR   QQQQ QQ
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function:      EnableRX_ISR
//
//   Parameter(s):  Mask        RX ISR request
//                                  UART_ISR_RX_ERROR_MASK
//                                  UART_ISR_RX_BYTE_MASK
//                                  UART_ISR_RX_TIMEOUT_MASK
//   Return Value:  None
//
//   Description:   Enable specific receive interrupt
//
//-------------------------------------------------------------------------------------------------
#if (UART_ISR_RX_IDLE_CFG  == DEF_ENABLED) || (UART_ISR_RX_ERROR_CFG == DEF_ENABLED) || (UART_ISR_RX_CFG == DEF_ENABLED)
void UART_Driver::EnableRX_ISR(uint8_t Mask)
{
    if(m_pUart != nullptr)
    {
        volatile uint32_t Register;

        // Idle, Error flag (Overrun, Framing error, Parity error)
        m_pUart->ICR = (USART_ICR_IDLECF | USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_PECF);
        Register = m_pUart->RDR;
        (void)Register;

      #if (UART_ISR_RX_IDLE_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_RX_IDLE) != 0)
        {
            m_pUart->CR1 |= USART_CR1_IDLEIE;
        }
      #endif

      #if (UART_ISR_RX_ERROR_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_RX_ERROR) != 0)
        {
            m_pUart->CR3 |= USART_CR3_EIE;
        }
      #endif

      #if (UART_ISR_RX_BYTE_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_RX_BYTE) != 0)
        {
            m_pUart->CR1 |= USART_CR1_RXNEIE;
        }
      #endif
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function:      DisableRX_ISR
//
//   Parameter(s):  Mask        TX ISR request
//                                  UART_ISR_RX_BYTE_MASK
//                                  UART_ISR_RX_TIMEOUT_MASK
//   Return Value:  None
//
//   Description:   Enable specific receive interrupt
//
//-------------------------------------------------------------------------------------------------
#if (UART_ISR_RX_IDLE_CFG == DEF_ENABLED) || (UART_ISR_RX_ERROR_CFG == DEF_ENABLED) || (UART_ISR_RX_CFG == DEF_ENABLED)
void UART_Driver::DisableRX_ISR(uint8_t Mask)
{
    if(m_pUart != nullptr)
    {
        volatile uint32_t Register;

        Register = m_pUart->RDR;
        (void)Register;

      #if (UART_ISR_RX_IDLE_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_RX_IDLE) != 0)
        {
            m_pUart->ICR = USART_ICR_IDLECF;
            CLEAR_BIT(m_pUart->CR1, USART_CR1_IDLEIE);
        }
      #endif

      #if (UART_ISR_RX_ERROR_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_RX_ERROR) != 0)
        {
            m_pUart->ICR = (USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_PECF);
            CLEAR_BIT(m_pUart->CR3, USART_CR3_EIE);
        }
      #endif

      #if (UART_ISR_RX_BYTE_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_RX_BYTE) != 0)
        {
            m_pUart->ICR = USART_ICR_RXNE;
            CLEAR_BIT(m_pUart->CR1, USART_CR1_RXNEIE);
        }
      #endif
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function:      EnableTX_ISR
//
//   Parameter(s):  Mask        TX ISR request
//                                  UART_ISR_TX_EMPTY_MASK
//                                  UART_ISR_TX_COMPLETE_MASK
//   Return Value:  None
//
//   Description:   Enable specific transmit interrupt
//
//-------------------------------------------------------------------------------------------------
#if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED) || (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
void UART_Driver::EnableTX_ISR(uint8_t Mask)
{
    if(m_pUart != nullptr)
    {
        m_pUart->ICR = USART_ICR_TCCF;

      #if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_TX_EMPTY) != 0)
        {
           m_pUart->CR1 |= USART_CR1_TXEIE;
        }
      #endif

      #if (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_TX_COMPLETED) != 0)
        {
            m_pUart->CR1 |= USART_CR1_TCIE;
        }
      #endif
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function:      DisableTX_ISR
//
//   Parameter(s):  Mask        TX ISR request
//                                  UART_ISR_TX_EMPTY_MASK
//                                  UART_ISR_TX_COMPLETE_MASK
//   Return Value:  None
//
//   Description:   Disable specific transmit interrupt
//
//-------------------------------------------------------------------------------------------------
#if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED) || (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
void UART_Driver::DisableTX_ISR(uint8_t Mask)
{
    if(m_pUart != nullptr)
    {
        m_pUart->ICR = USART_ICR_TCCF;

      #if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_TX_EMPTY) != 0)
        {
            CLEAR_BIT(m_pUart->CR1, USART_CR1_TXEIE);
        }
      #endif

      #if (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
        if((Mask & UART_ISR_TX_COMPLETED) != 0)
        {
            CLEAR_BIT(m_pUart->CR1, USART_CR1_TCIE);
        }
      #endif
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterCallback
//
//  Parameter(s):   pCallback       Callback pointer
//  Return:         None
//
//  Description:    Register callback for user code in ISR
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::RegisterCallback(CallbackInterface* pCallback)
{
    m_pCallback = pCallback;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableCallbackType
//
//  Parameter(s):   CallBackType    Type if the ISR callback
//  Return:         None
//
//  Description:    Enable the type of interrupt for the callback.
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::EnableCallbackType(int CallBackType)
{
  #if (UART_ISR_RX_BYTE_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_RX) != 0)
    {
        m_CallBackType |= CallBackType;
        EnableRX_ISR(UART_ISR_RX_BYTE);
    }
  #endif

  #if (UART_ISR_RX_IDLE_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_IDLE) != 0)
    {
        m_CallBackType |= CallBackType;
        EnableRX_ISR(UART_ISR_RX_IDLE);
    }
  #endif

#if (UART_ISR_RX_ERROR_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_ERROR) != 0)
    {
        m_CallBackType |= CallBackType;
        EnableRX_ISR(UART_ISR_RX_ERROR);
    }
  #endif

  #if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_EMPTY_TX) != 0)
    {
        m_CallBackType |= CallBackType;
        //EnableRX_ISR(UART_ISR_TX_EMPTY);      // don't... only on send data
    }
  #endif

  #if (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_COMPLETED_TX) != 0)
    {
        m_CallBackType       |= CallBackType;
        EnableRX_ISR(UART_ISR_TX_COMPLETED);
    }
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    IRQ_Handler
//
//  Description:    This function handle UARTx/USARTx interrupt.
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::IRQ_Handler(void)
{
    if(m_pUart != nullptr)
    {
        uint32_t Status;

        Status = m_pUart->ISR;

      #if (UART_ISR_RX_ERROR_CFG == DEF_ENABLED)
        if((Status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) != 0)
        {
            ClearFlag();

            if(m_pCallback != nullptr)
            {
                m_pCallback->CallbackFunction(UART_CALLBACK_ERROR, &m_RX_Transfer);
            }

            m_DMA_IsItBusyTX = false;
            return;
        }
      #endif

//#define UART_CALLBACK_DMA_RX                0x40




//        received byte per byte need this method if needed
      #if (UART_ISR_RX_BYTE_CFG == DEF_ENABLED)
        if((Status & USART_ISR_RXNE) != 0)
        {
            uint8_t* pData = uint8_t(m_pUart->RDR);

            if(m_pCallback != nullptr)
            {
                m_pCallback->CallbackFunction(UART_CALLBACK_RX, (void*)pData);
            }
            return;
        }
      #endif

      #if (UART_ISR_RX_IDLE_CFG == DEF_ENABLED)
        if((Status & USART_ISR_IDLE) != 0)
        {
            m_pUart->ICR = USART_ICR_IDLECF;

           #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
            m_RX_Transfer.Size -= m_pDMA_Info->DMA_StreamRX->NDTR; // Give actual position in the DMA Buffer
           #endif

            if(m_pCallback != nullptr)
            {
                m_pCallback->CallbackFunction(UART_CALLBACK_IDLE, (void*)&m_RX_Transfer);
            }
          #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
            DMA_ConfigRX(nullptr, 0); // Reset RX packet to avoid override with a new RX packet
          #endif
            return;
        }
      #endif

      #if (UART_ISR_TX_COMPLETED_CFG == DEF_ENABLED)
        if((Status & USART_ISR_TC) != 0)
        {
            m_pUart->ICR = USART_ICR_TCCF;

            if(m_pCallback != nullptr)
            {
                m_pCallback->CallbackFunction(UART_CALLBACK_COMPLETED_TX, (void*)m_TX_Transfer.pBuffer);
            }

            DMA_DisableTX();
            m_DMA_IsItBusyTX = false;
            return;
        }
      #endif

      #if (UART_ISR_TX_EMPTY_CFG == DEF_ENABLED)
        if((Status & USART_ISR_TXE) != 0)
        {
            if(m_TX_Transfer.Size < m_TX_Transfer.StaticSize)
            {
                m_pUart->TDR = m_TX_Transfer.pBuffer[m_TX_Transfer.Size++];
            }
            else
            {
                m_pCallback->CallbackFunction(UART_CALLBACK_COMPLETED_TX, (void*)&m_TX_Transfer.pBuffer);
                CLEAR_BIT(m_pUart->CR1, USART_CR1_TXEIE);
            }

            return;
        }
      #endif
    }
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    VirtualUartRX_IRQHandler
//
//  Description:    This function handles virtual UART interrupt.
//
//-------------------------------------------------------------------------------------------------
#if (UART_DRIVER_SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
void UART_Driver::VirtualUartRX_IRQHandler(void)
{
    /*       fix this!!
	if(m_pContextCompletedTX == nullptr) m_pCallbackCompletedTX(m_pContextOverrideTX);
     else
     {
           m_pCallbackCompletedTX(m_pContextCompletedTX);
//         DMA_ConfigRX(nullptr, 0);     // Reset RX packet to avoid override with a new RX packet
     }

     m_VirtualUartBusyRX = false;
*/
#endif

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    VirtualUartTX_IRQHandler
//
//  Description:    This function handles virtual UART interrupt.
//
//-------------------------------------------------------------------------------------------------
#if (SUPPORT_VIRTUAL_UART_CFG == DEF_ENABLED)
void UART_Driver::VirtualUartTX_IRQHandler(void)
{
    if(m_pContextCompletedTX != nullptr) m_pCallbackCompletedTX(m_pContextCompletedTX);
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (USE_UART_DRIVER == DEF_ENABLED)
