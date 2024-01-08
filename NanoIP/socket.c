//-------------------------------------------------------------------------------------------------
//
//  File : socket.c
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2011-2024 Alain Royer.
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


// it was a work in progress.. to be continued


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define SOCKET_GLOBAL
#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Socket
//
//  Parameter(s):   SOCKET  SocketNumber            Socket to open
//                  uint8_t    Protocol              Protocol to use
//                  int16_t    wSourcePort             Source port
//                  uint8_t    Flag                  Flag
//  Return:         bool    bStatus                 true or false
//
//  Description:    This function initialize the socket channel in particular mode,
//                  set the port and wait for W5100 to do it.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool SOCK_Socket(SOCKET SocketNumber, uint8_t Protocol, uint16_t SourcePort, uint8_t Flag)
{
    bool bStatus = false;


    if((byProtocol == SOCK_MODE_TCP) || (byProtocol == SOCK_MODE_UDP))
    {
        SOCK_Close(SocketNumber);

        if(wSourcePort != 0)
        {
          #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
			//SOCK_SetSocket(SocketNumber, Protocol, Flag); 
			//SOCK_Open(SocketNumber);
			
			pSocket = MEM_AddNode(&SOCK_pSocketList, sizeof(SocketInfo_t), &Error);

			if(pSocket != nullptr)
			{
				pSocket->wPort 				= wPort;
				pSocket->byProtocol 		= Protocol;
				pSocket->wBindSocketCount 	= 0;
				pSocket->pFunction        	= pFunction;
			}
	      #else
		  	NIC_SocketMode(SocketNumber, Protocol, Flag);
			NIC_SourcePort(SocketNumber, wSourcePort);
			NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_OPEN)                    // Wait to process the command...
		  #endif
            bStatus = true;
        }
    }

    return(bStatus);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Close
//
//  Parameter(s):   SOCKET  SocketNumber            Socket to close
//  Return:         void
//
//  Description:    This function close the socket
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SOCK_Close(SOCKET SocketNumber)
{
  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	// Clear Struct for the given socket
  #else
	NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_CLOSE)
    //sock_ir_write(SocketNumber, 0xFF);                      // Clear interrupt
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):   SOCKET  SocketNumber            Socket to listen
//  Return:         bool    bStatus                 true or false
//
//  Description:    This function established the connection for the channel in passive
//                  (server) mode. This function waits for the request from the peer.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool SOCK_Listen(SOCKET SocketNumber)
{
    bool bStatus = false;

  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	if(SOCK_Status() == SOCK_INIT)
	{
		// Set socket in listen mode
		bStatus = true;
	}
  #else
    if(sock_cr_read(SocketNumber) == SOCK_INIT)
	{
		NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_LISTEN)
        bStatus = true;
    }
  #endif

    return(bStatus);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Connect
//
//  Parameter(s):   SOCKET  SocketNumber            Socket to connect to
//                  int32_t   dwDstAddr               Destination IP Address
//                  int16_t    wDstPort                Destination Port
//  Return:         bool    bStatus                 true or false
//
//  Description:    This function established the connection for the channel in active
//                  (client) mode. It wait until the connection is established.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool SOCK_Connect(SOCKET SocketNumber, int32_t dwDstAddr, int16_t wDstPort)
{
    bool bStatus    =   true;
    uint8_t Status;

    if((dwDstAddr == 0xFFFFFFFF) || (dwDstAddr == 0x00000000) || (wDstPort == 0x00))
    {
        bStatus = false;
    }
    else
    {
      #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
		//set destination IP
		//set destination Port
		//send stuff wait answers
		
      #else
	
    	NIC_DestinationIP(SocketNumber, DstAddr);						// Set destination IP
		NIC_DestinationPort(SocketNumber, wDstPort);					// Set destination PORT
		NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_CONNECT);  // Wait to process the command...

        // wait for Established or close (Timeout)
        do
        {
            Status  = sock_sr_read(IP_STREAM_SOCKET);
        }
        while((byStatus != SOCK_CLOSED) && (byStatus != SOCK_ESTABLISHED));
      #endif
		
        if(byStatus == SOCK_CLOSED)
        {
            bStatus = false;
        }
    }

    return(bStatus);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Disconnect
//
//  Parameter(s):   SOCKET  SocketNumber            Socket to disconnect from
//  Return:         void
//
//  Description:    This function is used to disconnect the socket
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void SOCK_Disconnect(SOCKET SocketNumber)
{
  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	// Disconnect 
  #else
    NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_DISCONNECT); 
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Send
//
//  Parameter(s):   SOCKET      SocketNumber            Socket to send data to
//                  const uint8_t* pData                   Buffer containing the data
//                  int16_t        wLen                    Size of the data
//  Return:         int16_t
//
//  Description:    This function is an application I/F function to send the data in TCP mode
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int16_t SOCK_Send(SOCKET SocketNumber, const uint8_t* pData, int16_t wLen)
{
    uint8_t    Status    = 0;
    int16_t    wReturn     = 0;
    int16_t    wFreeSize   = 0;

  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
  #else
    if(wLen > W5100_wTX_Size[SocketNumber])               // Check size not to exceed MAX size.
    {
        wReturn = W5100_wTX_Size[SocketNumber];
    }
    else
    {
        wReturn = wLen;
    }

    // Start if freebuffer is available
    do
    {
        wFreeSize = SOCK_GetTX_FSR(SocketNumber);
        Status  = sock_sr_read(SocketNumber);

        if((byStatus != SOCK_ESTABLISHED) && (byStatus != SOCK_CLOSE_WAIT))
        {
            wReturn = 0;
            break;
        }
    }
    while(wFreeSize < wReturn);

    // Copy data
    W5100_ProcessTX_Data(SocketNumber, (uint8_t *)pData, wReturn);

    OSTimeDly(200);

    W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_SEND);     // Wait to process the command...

    while((sock_ir_read(SocketNumber) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
    {
        if(sock_sr_read(SocketNumber) == SOCK_CLOSED)
        {
            SOCK_Close(SocketNumber);
            return(0);
        }
    }

    sock_ir_write(SocketNumber, Sn_IR_SEND_OK);
  #endif

    return(wReturn);
}
//
//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_received
//
//  Parameter(s):   SOCKET      SocketNumber            Socket to send data to
//                  const uint8_t* pData                   Buffer to put the data in
//  Return:         void
//
//  Description:    This function is an application I/F function which is used to receive the data
//                  in TCP mode. It continues to wait for data as needed  the application
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int16_t SOCK_Received(SOCKET SocketNumber, uint8_t *pData, int16_t wLen)
{
    int16_t wReturn = 0;

    if(wLen > 0)
    {
        W5100_ProcessRX_Data(SocketNumber, pData, wLen);
        W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_RECV);     // Wait to process the command...
        wReturn = wLen;
    }
    return(wReturn);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_SendTo
//
//  Parameter(s):   SOCKET      SocketNumber            Socket to send data to
//                  const uint8_t* pData                   Buffer containing the data
//                  int16_t        wLen                    lenght of the packet to send
//                  int32_t       dwDstAddr               Destination IP Address
//                  int16_t        wDstPort                Destination Port
//  Return:         void
//
//  Description:    This function is an application I/F function which is used to send the data for
//                  other protcol then TCP mode. Unlike TCP transmission, The peer's destination
//                  address and the port is needed.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int16_t SOCK_SendTo(SOCKET SocketNumber, const uint8_t* pData, int16_t wLen, int32_t dwPeerAddr, int16_t wPeerPort)
{
    int16_t wReturn = 0;


    if(wLen > W5100_wTX_Size[SocketNumber])               // Check size not to exceed MAX size.
    {
        wReturn = W5100_wTX_Size[SocketNumber];
    }
    else
    {
        wReturn = wLen;
    }

    if((dwPeerAddr == 0x00000000) || (wPeerPort == 0x00) || (wReturn == 0))
    {
        wReturn = 0;
    }
    else
    {
        sock_dipr_write(SocketNumber, dwPeerAddr);
        sock_dportr_write(SocketNumber, wPeerPort);

OSTimeDly(200);     // why i need so much delay tabarnak???

        // copy data
        W5100_ProcessTX_Data(SocketNumber, (uint8_t*)pData, wReturn);
        W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_SEND);     // Wait to process the command...

        while((sock_ir_read(SocketNumber) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
        {
            if((sock_ir_read(SocketNumber) & Sn_IR_TIMEOUT) != 0)
            {
                sock_ir_write(SocketNumber, (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); // clear SEND_OK & TIMEOUT
                return(0);
            }
        }
        sock_ir_write(SocketNumber, Sn_IR_SEND_OK);
    }

    return(wReturn);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:
//
//  Parameter(s):   
//  Return:         
//
//  Description:    This function is an application I/F function which is used to receive the data
//                  in other protcol then TCP mode. This function is used to receive UDP, IP_RAW
//                  and MAC_RAW mode, it handle the header as well.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint16_t SOCK_ReceivedFrom(SOCKET SocketNumber, uint8_t* pData, uint16_t Len, uint32_t* pPeerAddr, uint16_t* pPeerPort)
{
    uint8_t    baHead[8];
    uint16_t   DataLenght     = 0;
    uint16_t   Ptr            = 0;

    if(wLen > 0)
    {
        wPtr = sock_rx_rpr_read(SocketNumber);

        switch(sock_mr_read(SocketNumber) & 0x07)
        {
            case Sn_MR_UDP:
            {
                W5100_ReadData(SocketNumber, wPtr, baHead, 8);
                wPtr += 8;

                // Read peer's IP address, port number, and get data lenght
                *pPeerAddr  = ntohl(*((int32_t*)&baHead[0]));
                *pPeerPort  = ntohs(*((int16_t*)&baHead[4]));
                wDataLenght = ntohs(*((int16_t*)&baHead[6]));

                W5100_ReadData(SocketNumber, wPtr, pData, wDataLenght);         // data copy.
                wPtr += wDataLenght;
                sock_rx_rpr_write(SocketNumber, wPtr);
                break;
            }

            case Sn_MR_IPRAW:
            {
                W5100_ReadData(SocketNumber, wPtr, baHead, 6);
                wPtr += 6;

                // Read peer's IP address, port number, and get data lenght
                *pPeerAddr  = *((int32_t*)&baHead[0]);
                wDataLenght = *((int16_t*)&baHead[4]);

                W5100_ReadData(SocketNumber, wPtr, pData, wDataLenght);         // data copy.
                wPtr += wDataLenght;
                sock_rx_rpr_write(SocketNumber, wPtr);
                break;
            }

            case Sn_MR_MACRAW:
            {
                W5100_ReadData(SocketNumber, wPtr, baHead, 2);
                wPtr += 2;
                wDataLenght = *((int16_t*)&baHead[0]);

                W5100_ReadData(SocketNumber, wPtr, pData, wDataLenght);
                wPtr += wDataLenght;
                sock_rx_rpr_write(SocketNumber, wPtr);
                break;
            }

            default:
            {
                break;
            }
        }

        W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_RECV);     // Wait to process the command...
    }

    return(wDataLenght);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_GetTX_FSR
//
//  Parameter(s):   SOCKET  SocketNumber
//  Return:         int16_t    Free Size
//
//  Description:    Get the free size in the nic TX Buffer
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint16_t SOCK_GetTX_BufferSize(SOCKET SocketNumber)
{
  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	//Get Buffer size for TX
  #else
	return(SOCK_GetTX_BufferSize(SocketNumber));
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_GetRX_RSR
//
//  Parameter(s):   SOCKET  SocketNumber
//  Return:         int16_t
//
//  Description:    Get the size of the received data size
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint16_t SOCK_GetRX_BufferSize(SOCKET SocketNumber)
{
  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	//Get Buffer size for TX
  #else
	return(SOCK_GetTX_BufferSize(SocketNumber));
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_SetSocket
//
//  Parameter(s):   SOCKET  SocketNumber
//					uint8_t	byProtocol
//					uint8_t	byFlag
//
//  Return:         void
//
//  Description:    Set the Socket mode
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (IP_HARDWARE_SOCKET == DEF_DISABLED)
void SOCK_SetSocket(SOCKET	SocketNumber, uint8_t Protocol, uint8_t Flag)
{
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Open
//
//  Parameter(s):   SOCKET  SocketNumber
//
//  Return:         void
//
//  Description:    Open Socket with the data in structure
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (IP_HARDWARE_SOCKET == DEF_DISABLED)
void SOCK_Open(SOCKETSocketNumber)
{
}
#endif

//-------------------------------------------------------------------------------------------------

