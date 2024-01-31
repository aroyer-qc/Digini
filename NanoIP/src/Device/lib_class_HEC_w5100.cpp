//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_HEC_W5100.cpp
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
//  Note(s)
//
//              HEC - Hardwired Ethernet Controller
//
//              The W5100 chip is a Hardwired TCP/IP embedded ethernet controller that enables
//              easier Internet connection for embedded systems.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetSocketMemorySize
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    This function initialize the W5100 controller
//
//-------------------------------------------------------------------------------------------------
void Init(void)
{
    // Basic Setting

    mr_reset = 1;
    while(mr_reset != 0);           // wait until chip is reset

    mr     = 0b00000000; // Mode Register
    /*         ||||||||
               ||||||||____ Disable Indirect BUD I/F mode
               |||||||_____ Auto Increment Disable (Full Address Mode)
               ||||||______ Not Used ??
               |||||_______ Disable PPPoE (This unit do not use ADSL directly)
               ||||________ Disable Ping Block (Ping request are accepted)
               |||_________ Reserved
               |___________ Reset
    */

    imr    = 0b11001111; // Interrupt Mask Register
    /*         ||||||||
               ||||||||____ Socket 0 Enable
               |||||||_____ Socket 1 Enable
               ||||||______ Socket 2 Enable
               |||||_______ Socket 3 Enable
               ||||________ Set to 0
               |||_________ PPPoE Disable
               ||__________ Destination Unreachable Enable
               |___________ IP Conflict Enable
    */

    rtr_write(2000);        // Retry Time Value Register    2000 * 100uSec
    rcr = 4;                // Retry Count Register

    // MAC Address
    shar0 = IP_MAC.ba[0];
    shar1 = IP_MAC.ba[1];
    shar2 = IP_MAC.ba[2];
    shar3 = IP_MAC.ba[3];
    shar4 = IP_MAC.ba[4];
    shar5 = IP_MAC.ba[5];

    //Socket TX & RX memory size
    SetSocketMemorySize(IP_TX_SocketMemorySize, IP_RX_SocketMemorySize);

    // Network Setting Information

    if(IP_Flag.b.DHCP_Mode != YES)
    {
        gar(IP_StaticGatewayIP);
        subr(IP_StaticSubnetMask);
        sipr(IP_StaticIP);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetSocketMemorySize
//
//  Parameter(s):   uint8_t    TX_Size       Tx memory size (00 - 1KB, 01- 2KB, 10 - 4KB, 11 - 8KB)
//                  uint8_t    RX_Size       Rx memory size (00 - 1KB, 01- 2KB, 10 - 4KB, 11 - 8KB)
//
//  Return:         void
//
//  Description:    This function set the transmit & receive buffer size as per the channels
//                  is used
//
//  Note(s):        for TMSR and RMSR bits are as follows
//                      bit 1-0 : memory size of channel #0
//                      bit 3-2 : memory size of channel #1
//                      bit 5-4 : memory size of channel #2
//                      bit 7-6 : memory size of channel #3
//
//                  Maximum memory size for Tx, Rx in the W5100 is 8 KB
//
//                  Example:
//
//                  If the 8 KB memory are assigned to a certain channel, the other 3 channels can
//                  not be used because there is no more available memory.
//
//                  If two 4 KB memory are assigned to 2 channels, the other 2 channels can not be
//                  used because there is no more available memory.
//
//                  Ans so on....
//
//-------------------------------------------------------------------------------------------------
void SetSocketMemorySize(uint8_t TX_Size, uint8_t RX_Size)
{
    uint8_t  i;
    uint16_t TX_Sum;
    uint16_t RX_Sum;

  #if (DBG_W5100 == DEF_ENABLED)
    DBG_printf("W5100_SetSocketMemorySize()\r\n");
  #endif

    TX_Sum = 0;
    RX_Sum = 0;

    tmsr = TX_Size;                                           // Set Tx memory size for each channel
    rmsr = RX_Size;                                           // Set Rx memory size for each channel

    m_TX_BaseAddress[0] = W5100_MEMORY_TX_BASE;           // Set base address of TX memory for channel #0
    m_RX_BaseAddress[0] = W5100_MEMORY_RX_BASE;           // Set base address of RX memory for channel #0

    for(i = 0; i < W5100_NUMBER_OF_SOCKET; i++)                 // Set the size, masking and base address of Tx & Rx memory by each channel
    {
        m_TX_Size[i] = 0;
        m_RX_Size[i] = 0;

        if(TX_Sum < 0x2000)
        {
            switch((TX_Size >> (i << 1)) & 0x03)              // Set TX memory size
            {
                case 0:     m_TX_Size[i] = 0x0400;
                            m_TX_Mask[i] = 0x03FF;
                            break;

                case 1:     m_TX_Size[i] = 0x0800;
                            m_TX_Mask[i] = 0x07FF;
                            break;

                case 2:     m_TX_Size[i] = 0x1000;
                            m_TX_Mask[i] = 0x0FFF;
                            break;

                case 3:     m_TX_Size[i] = 0x2000;
                            m_TX_Mask[i] = 0x1FFF;
                            break;
            }
        }

        if(RX_Sum < 0x2000)
        {
            switch((RX_Size >> (i << 1)) & 0x03)                      // Set Rx memory size
            {
                case 0:     m_RX_Size[i] = 0x0400;
                            m_RX_Mask[i] = 0x03FF;
                            break;

                case 1:     m_RX_Size[i] = 0x0800;
                            m_RX_Mask[i] = 0x07FF;
                            break;

                case 2:     m_RX_Size[i] = 0x1000;
                            m_RX_Mask[i] = 0x0FFF;
                            break;

                case 3:     m_RX_Size[i] = 0x2000;
                            m_RX_Mask[i] = 0x1FFF;
                            break;
            }
        }

        TX_Sum += m_TX_Size[i];
        RX_Sum += m_RX_Size[i];

        if(i != 0)                                      // Sets base address of Tx and Rx memory for channel #1,#2,#3
        {
            m_TX_BaseAddress[i] = m_TX_BaseAddress[i - 1] + m_TX_Size[i - 1];
            m_RX_BaseAddress[i] = m_RX_BaseAddress[i - 1] + m_RX_Size[i - 1];
        }
      #if (DBG_W5100 == DEF_ENABLED)
        DBG_UartPrintf("%d : %.4x : %.4x : %.4x : %.4x\r\n", i, (uint16_t)WORD)m_TX_BaseAddress[i], (uint16_t)m_RX_BaseAddress[i], m_TX_Size[i], m_RX_Mask[i]);
      #endif
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ProcessTX_Data
//
//  Parameter(s):   Socket_t        SocketNumber                Number of the socket to check
//                  uint8_t*        pData                       Buffer with data to send
//                  size_t          Length                      Lenght of the Buffer
//  Return:         void
//
//  Description:    This function read the TX write pointer register and after copy the data in
//                  buffer, then update the TX write pointer register.
//
//  Note(s):        This function is being called by W5100_Send() and W5100_SendTo().
//
//-------------------------------------------------------------------------------------------------
void ProcessTX_Data(Socket_t SocketNumber, uint8_t* pData, size_t Length)
{
    uint16_t Ptr;

    Ptr = sock_tx_wpr_read(SocketNumber);                   // Get TX pointer
    WriteData(SocketNumber, pData, wPtr, Length);           // Write data
    Ptr += Length;                                          // Add data lenght to pointer
    sock_tx_wpr_write(SocketNumber, wPtr);                  // Update Pointer
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ProcessRX_Data
//
//  Parameter(s):   Socket_t    SocketNumber                Number of the socket to check
//                  uint8_t*    pData                       Buffer with data to send
//                  size_t      Length                      Lenght of the Buffer
//  Return:         void
//
//  Description:    This function read the RX read pointer register and after copy the data from
//                  buffer, then update the RX read pointer register.
//
//  Note(s):        This function is being called by W5100_Send() and W5100_SendTo().
//
//-------------------------------------------------------------------------------------------------
void ProcessRX_Data(Socket_t SocketNumber, uint8_t* pData, size_t Length)
{
    uint16_t Ptr;

    Ptr = sock_rx_rpr_read(SocketNumber);                // Get RX pointer
    ReadData(SocketNumber, wPtr, pData, Length);      // Read data
    Ptr += Length;                                         // Add data lenght to pointer
    sock_rx_rpr_write(SocketNumber, Ptr);                // Update Pointer
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteData
//
//  Parameter(s):   Socket_t   SocketNumber                    Number of the socket to check
//                  uint8_t*   pData                           Buffer with data to send
//                  size_t     Length                          Lenght of the Buffer
//  Return:         void
//
//  Description:    Transfer data from application to NIC chip buffer.
//                  It calculate the actual physical address where to write the data in TX buffer.
//                  It also take care of the rollover condition at the boundary of the socket.
//
//-------------------------------------------------------------------------------------------------
void WriteData(Socket_t SocketNumber, uint8_t* pData, WORD BufferPtr, size_t Length)
{
    size_t    Size;
    uint16_t  DstMask;
    uint8_t*  pDst;

    DstMask = BufferPtr & m_TX_Mask[SocketNumber];
    pDst     = (far uint8_t *)(m_TX_BaseAddress[SocketNumber] + (uint32_t)DstMask);

    if((DstMask + Length) > m_TX_Size[SocketNumber])
    {
        Size = m_TX_Size[SocketNumber] - DstMask;
        WriteBuffer(pData, pDst, Size);
        pData += Size;
        Size   = Length - wSize;
        pDst   = (far uint8_t *)m_TX_BaseAddress[SocketNumber];
        WriteBuffer(pData, pDst, Size);
    }
    else
    {
        WriteBuffer(pData, pDst, Length);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           W5100_ReadData
//
//  Parameter(s):   Socket_t  SocketNumber                Number of the socket to check
//                  BYTE*     pData                       Buffer with data to send
//                  size_t    Length                      Lenght of the Buffer
//  Return:         void
//
//  Description:    Transfert data from NIC chip buffer to NIC application.
//                  It calculate the actual physical address where to read the data in TX buffer.
//                  It also take care of the rollover condition at the boundary of the socket.
//
//-------------------------------------------------------------------------------------------------
void W5100_ReadData(Socket_t SocketNumber, uint16_t BufferPtr, uint8_t* pData, size_t Length)
{
    size_t         Size;
    uint16_t       SrcMask;
    far uint8_t*   pSrc;

    SrcMask = BufferPtr & m_RX_Mask[SocketNumber];
    pSrc    = (far uint8_t *)(m_RX_BaseAddress[SocketNumber] + (uint32_t)wSrcMask);

    if((SrcMask + Length) > m_RX_Size[SocketNumber])
    {
        Size = m_RX_Size[SocketNumber] - SrcMask;
        W5100_ReadBuffer(pSrc, pData, Size);
        pData += Size;
        Size = Length - Size;
        pSrc = (far uint8_t*)m_RX_BaseAddress[SocketNumber];
        ReadBuffer(pSrc, pData, Size);
    }
    else
    {
        ReadBuffer(pSrc, pData, Length);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteBuffer
//
//  Parameter(s):   uint8_t*       pSrc        Buffer where to get the data
//                  far uint8_t*   pDst        Physical address in the NIC chip
//                  size_t         Length      Lenght to transfert
//  Return:         void
//
//  Description:    This function write into W5100 memory(Buffer)
//
//-------------------------------------------------------------------------------------------------
void WriteBuffer(uint8_t* pSrc, far uint8_t* pDst, size_t Length)
{
    OS_CPU_SR  cpu_sr = 0;

    OS_ENTER_CRITICAL();
    while(Length != 0)
    {
        *pDst = *pSrc;
        pSrc++;
        pDst++;
        Length--;
    }
    OS_EXIT_CRITICAL();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadBuffer
//
//  Parameter(s):   uint8_t*   pSrc        Physical address in the NIC chip
//                  uint8_t*   pDst        Buffer where to transfert date
//                  size_t     Length      Lenght to transfert
//  Return:         void
//
//  Description:    This function read into W5100 memory(Buffer)
//
//-------------------------------------------------------------------------------------------------
void ReadBuffer(far uint8_t* pSrc, uint8_t* pDst, size_t Length)
{
    OS_CPU_SR  cpu_sr = 0;

    OS_ENTER_CRITICAL();
    while(Length != 0)
    {
        *pDst = *pSrc;
        pSrc++;
        pDst++;
        Length--;
    }
    OS_EXIT_CRITICAL();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Int0_ISR
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Socket interrupt routine
//
//-------------------------------------------------------------------------------------------------
void Int0_ISR(void)
{
    // Process all of interupt
    do
    {
        if(isr_conflict == true)
        {
            //DBG_UartPrintf("IP conflict\r\n");
        }

        if(isr_unreacheable == true)
        {
            //DBG_UartPrintf("INT Port Unreachable\r\n");
            //DBG_UartPrintf("ipr : %d.%d.%d.%d\r\n", uipr0, uipr1, uipr2, uipr3);
            //DBG_UartPrintf("uportr : %.4x %.2x\r\n", uport);
        }

        isr = 0xF0;                                             // Clear Interrupt


        if(isr_s0 == true)
        {
            m_SocketIntStatus[0] = sock_ir_read(0);       // Save Interrupt Value
            sock_ir_write(0, m_SocketIntStatus[0]);       // Clear Interrupt
        }

        if(isr_s1 == true)
        {
            m_SocketIntStatus[1] = sock_ir_read(1);       // Save Interrupt Value
            sock_ir_write(1, m_SocketIntStatus[1]);       // Clear Interrupt
        }

        if(isr_s2 == true)
        {
            m_SocketIntStatus[2] = sock_ir_read(2);       // Save Interrupt Value
            sock_ir_write(2, m_SocketIntStatus[2]);       // Clear Interrupt
        }

        if(isr_s3 == true)
        {
            m_SocketIntStatus[3] = sock_ir_read(3);       // Save Interrupt Value
            sock_ir_write(3, m_SocketIntStatus[3]);       // Clear Interrupt
        }
    }
    while(isr != 0);                        // if exist, contiue to process

}



