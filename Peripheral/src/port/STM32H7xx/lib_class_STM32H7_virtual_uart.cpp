//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H7_virtual_uart.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define VIRTUAL_UART_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  VIRTUAL_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_VIRTUAL_UART_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  private variable(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Class: VUART_Driver
//
//
//   Description:   Class to handle VIRTUAL UART
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   VUART_Driver
//
//   Parameter(s):  VuartID                 ID for the data to use for this class
//
//   Description:   Initializes the VUARTx according to the specified Parameters
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
VUART_Driver::VUART_Driver(VUART_ID_e VuartID)
{
    m_VuartID = VuartID;
    m_pInfo  = (VUART_Info_t*)&VUART_Info[VuartID];
}

//-------------------------------------------------------------------------------------------------
//
//   Name:          Initialize
//
//   Parameter(s):  None
//   Return:        None
//
//   Description:   Initialize this VUART port.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void VUART_Driver::Initialize(void)
{
    m_Enable            = false;
    m_VirtualUartBusyRX = false;
    m_VirtualUartBusyTX = false;

    ISR_Init(m_pInfo->VirtualUartRX_IRQn, m_pInfo->PreempPrio);
    ISR_Init(m_pInfo->VirtualUartTX_IRQn, m_pInfo->PreempPrio);
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
void VUART_Driver::Enable(void)
{
    m_Enable = true;
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
void VUART_Driver::Disable(void)
{
    m_Enable = false;
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
bool VUART_Driver::IsItBusy(void)
{
    if((m_VirtualUartBusyRX == true) || (m_VirtualUartBusyTX == true))
    {
        return true;
    }

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
uint32_t VUART_Driver::GetBaudRate(void)
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
void VUART_Driver::ClearFlag(void)
{
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
void VUART_Driver::VirtualSendData(const uint8_t* pBuffer, uint16_t Size)
{
    ReceivedFromVirtualUart(pBuffer, Size);
    ISR_SetPendingIRQ(VirtualUartTX_IRQn);
    m_VirtualUartBusyTX = true;
}

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
SystemState_e VUART_Driver::SendData(const uint8_t* pBufferTX, size_t* pSizeTX)
{
    SystemState_e State = SYS_READY;

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
    return State;
}

//-------------------------------------------------------------------------------------------------
//  DDDDDD  MM     MM  AAAAA
//  DD   DD MMMM MMMM AA   AA
//  DD   DD MM MMM MM AAAAAAA
//  DD   DD MM  M  MM AA   AA
//  DDDDDD  MM     MM AA   AA
//-------------------------------------------------------------------------------------------------

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
void VUART_Driver::DMA_ConfigRX(uint8_t* pBufferRX, size_t SizeRX)
{
    UART_Transfer_t* pTransferRX = &m_RX_Transfer;

    if(pBufferRX != nullptr)
    {
        pTransferRX->u.Size     = SizeRX;
        pTransferRX->StaticSize = SizeRX;
        pTransferRX->pBuffer    = pBufferRX;
    }
    else
    {
        pTransferRX->u.Size = pTransferRX->StaticSize;
    }
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
void VUART_Driver::DMA_ConfigTX(uint8_t* pBufferTX, size_t SizeTX)
{
     UART_Transfer_t* pTransferTX = &m_TX_Transfer;

    if(pBufferTX != nullptr)
    {
        pTransferTX->pBuffer    = pBufferTX;
        pTransferTX->u.Size     = SizeTX;
        pTransferTX->StaticSize = SizeTX;
    }
    else
    {
        pTransferTX->u.Size = pTransferTX->StaticSize;
    }
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
void VUART_Driver::DMA_EnableRX(void)
{
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
void VUART_Driver::DMA_DisableRX(void)
{
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
void VUART_Driver::DMA_EnableTX(void)
{
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
void VUART_Driver::DMA_DisableTX(void)
{
}

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
size_t VUART_Driver::DMA_GetSizeRX(uint16_t SizeRX)
{
    return m_VirtualVar.SizeRX;
}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  IIII RRRRRR    QQQQQ
//   II  RR   RR  QQ   QQ
//   II  RRRRRR   QQ   QQ
//   II  RR  RR   QQ  QQQ
//  IIII RR   RR   QQQQ QQ
//-------------------------------------------------------------------------------------------------

 #if ((UART_DRIVER_RX_ERROR_CFG     == DEF_ENABLED) || \
      (UART_DRIVER_RX_NOT_EMPTY_CFG == DEF_ENABLED) || \
      (UART_DRIVER_RX_IDLE_CFG      == DEF_ENABLED))

//-------------------------------------------------------------------------------------------------
//
//   Function:      EnableRX_ISR
//
//   Parameter(s):  Mask        RX ISR request mask
//   Return Value:  None
//
//   Description:   Enable specific receive interrupt
//
//-------------------------------------------------------------------------------------------------
void VUART_Driver::EnableRX_ISR(uint8_t Mask)
{
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DisableRX_ISR
//
//   Parameter(s):  Mask        RX ISR request mask
//   Return Value:  None
//
//   Description:   Enable specific receive interrupt
//
//-------------------------------------------------------------------------------------------------
void VUART_Driver::DisableRX_ISR(uint8_t Mask)
{
}

//-------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------------------------

#if ((UART_DRIVER_TX_COMPLETED_CFG == DEF_ENABLED) || \
     (UART_DRIVER_TX_EMPTY_CFG     == DEF_ENABLED))

//-------------------------------------------------------------------------------------------------
//
//   Function:      EnableTX_ISR
//
//   Parameter(s):  Mask        TX ISR request mask
//   Return Value:  None
//
//   Description:   Enable specific transmit interrupt
//
//-------------------------------------------------------------------------------------------------
void VUART_Driver::EnableTX_ISR(uint8_t Mask)
{
}

//-------------------------------------------------------------------------------------------------

#if (UART_DRIVER_USE_CALLBACK_CFG == DEF_ENABLED)

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
void VUART_Driver::RegisterCallback(CallbackInterface* pCallback)
{
    m_pCallback = pCallback;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableCallbackType
//
//  Parameter(s):   CallBackType    Type of the ISR callback
//  Return:         None
//
//  Description:    Enable the type of interrupt for the callback.
//
//-------------------------------------------------------------------------------------------------
void VUART_Driver::EnableCallbackType(int CallBackType)
{
    uint8_t Mask;

  #if (UART_DRIVER_RX_NOT_EMPTY_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_RX_NOT_EMPTY) != 0)
    {
        Mask = UART_ISR_RX_NOT_EMPTY_MASK;
    }
  #endif

  #if (UART_DRIVER_RX_IDLE_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_RX_IDLE) != 0)
    {
        Mask = UART_ISR_RX_IDLE_MASK;
    }
  #endif

#if (UART_DRIVER_RX_ERROR_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_ERROR) != 0)
    {
        Mask = UART_ISR_RX_ERROR_MASK;
    }
  #endif

  #if (UART_DRIVER_TX_EMPTY_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_EMPTY_TX) != 0)
    {
        Mask = UART_ISR_TX_EMPTY_MASK;
    }
  #endif

  #if (UART_DRIVER_TX_COMPLETED_CFG == DEF_ENABLED)
    if((CallBackType & UART_CALLBACK_TX_COMPLETED) != 0)
    {
        Mask = UART_ISR_TX_COMPLETED_MASK;
    }
  #endif

    if(Mask != 0)
    {
        EnableRX_ISR(Mask);
    }
}

//-------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    VirtualUartRX_IRQHandler
//
//  Description:    This function handles virtual UART interrupt.
//
//  TODO move this to a another virtual driver
//
//-------------------------------------------------------------------------------------------------
void VUART_Driver::VirtualUartRX_IRQHandler(void)
{
    /*       fix this!!
	if(m_pContextCompletedTX == nullptr) m_pCallbackCompletedTX(m_pContextTX);
     else
     {
           m_pCallbackCompletedTX(m_pContextCompletedTX);
//         DMA_ConfigRX(nullptr, 0);     // Reset RX packet to avoid override with a new RX packet
     }

     m_VirtualUartBusyRX = false;
*/
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    VirtualUartTX_IRQHandler
//
//  Description:    This function handles virtual UART interrupt.
//
//-------------------------------------------------------------------------------------------------
void VUART_Driver::VirtualUartTX_IRQHandler(void)
{
    if(m_pContextCompletedTX != nullptr)
    {
        m_pCallbackCompletedTX(m_pContextCompletedTX);
    }
    
    m_VirtualUartBusyTX = false;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_VIRTUAL_UART_DRIVER == DEF_ENABLED)
