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

#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:           Socket
//
//  Parameter(s):   Socket_t    SocketNumber            Socket to open
//                  uint8_t     Protocol                Protocol to use
//                  IP_Port_t   SourcePort              Source port
//                  uint8_t     Flag                    Flag
//  Return:         bool        bStatus                 true or false
//
//  Description:    This function initialize the socket channel in particular mode,
//                  set the port and wait for W5100 to do it.
//
//-------------------------------------------------------------------------------------------------
bool NetSOCK::Socket(Socket_t SocketNumber, uint8_t Protocol, IP_Port_t SourcePort, uint8_t Flag)
{
    bool Status = false;


    if((Protocol == SOCK_MODE_TCP) || (Protocol == SOCK_MODE_UDP))
    {
        SOCK_Close(SocketNumber);

        if(SourcePort != 0)
        {
          #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
			//SOCK_SetSocket(SocketNumber, Protocol, Flag); 
			//SOCK_Open(SocketNumber);
			
			pSocket = MEM_AddNode(&SOCK_pSocketList, sizeof(SocketInfo_t), &Error);

			if(pSocket != nullptr)
			{
				pSocket->Port 				= Port;
				pSocket->Protocol 		    = Protocol;
				pSocket->BindSocketCount 	= 0;
				pSocket->pFunction        	= pFunction;
			}
	      #else
		  	NIC_SocketMode(SocketNumber, Protocol, Flag);
			NIC_SourcePort(SocketNumber, SourcePort);
			NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_OPEN)                    // Wait to process the command...
		  #endif
            Status = true;
        }
    }

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Close
//
//  Parameter(s):   Socket_t  SocketNumber            Socket to close
//  Return:         void
//
//  Description:    This function close the socket
//
//-------------------------------------------------------------------------------------------------
void NetSOCK::Close(Socket_t SocketNumber)
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
//  Name:           SOCK_Listen
//
//  Parameter(s):   Socket_t    SocketNumber            Socket to listen
//  Return:         bool        bStatus                 true or false
//
//  Description:    This function established the connection for the channel in passive
//                  (server) mode. This function waits for the request from the peer.
//
//-------------------------------------------------------------------------------------------------
bool NetSOCK::Listen(Socket_t SocketNumber)
{
    bool Status = false;

  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	if(SOCK_Status() == SOCK_INIT)
	{
		// Set socket in listen mode
		Status = true;
	}
  #else
    if(sock_cr_read(SocketNumber) == SOCK_INIT)
	{
		NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_LISTEN)
        Status = true;
    }
  #endif

    return bStatus;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Connect
//
//  Parameter(s):   Socket_t      SocketNumber          Socket to connect to
//                  IP_Address_t  DstAddress            Destination IP Address
//                  IP_Port_t     DstPort               Destination Port
//  Return:         bool          Status                true or false
//
//  Description:    This function established the connection for the channel in active
//                  (client) mode. It wait until the connection is established.
//
//-------------------------------------------------------------------------------------------------
bool NetSOCK::Connect(Socket_t SocketNumber, IP_Address_t DstAddress, IP_Port_t DstPort)
{
    bool    bStatus = true;
    uint8_t Status;

    if((DstAddress == IP_ADDRESS(255,255,255,255) || (DstAddress == IP_ADDRESS(0,0,0,0) || (DstPort == 0))
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
	
    	NIC_DestinationIP(SocketNumber, DstAddress);					// Set destination IP
		NIC_DestinationPort(SocketNumber, wDstPort);					// Set destination PORT
		NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_CONNECT);    // Wait to process the command...

        // wait for Established or close (Timeout)
        do
        {
            Status = sock_sr_read(IP_STREAM_SOCKET);
        }
        while((byStatus != SOCK_CLOSED) && (byStatus != SOCK_ESTABLISHED));
        
      #endif
		
        if(byStatus == SOCK_CLOSED)
        {
            bStatus = false;
        }
    }

    return bStatus;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Disconnect
//
//  Parameter(s):   Socket_t  SocketNumber            Socket to disconnect from
//  Return:         void
//
//  Description:    This function is used to disconnect the socket
//
//-------------------------------------------------------------------------------------------------
void NetSOCK::Disconnect(Socket_t SocketNumber)
{
  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	// Disconnect 
  #else
    NIC_ProcessCommandAndWait(SocketNumber, NIC_SOCKET_DISCONNECT); 
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameter(s):   Socket_t        SocketNumber            Socket to send data to
//                  const uint8_t*  pData                   Buffer containing the data
//                  size_t          Length                  Size of the data
//  Return:         size_t
//
//  Description:    This function is an application I/F function to send the data in TCP mode
//
//-------------------------------------------------------------------------------------------------
size_t NetSOCK::Send(Socket_t SocketNumber, const uint8_t* pData, size_t Length)
{
    uint8_t    Status    = 0;
    size_t     Return    = 0;
    size_t     FreeSize  = 0;

  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
  #else
    if(Length > W5100_TX_Size[SocketNumber])               // Check size not to exceed MAX size.
    {
        Return = W5100_TX_Size[SocketNumber];
    }
    else
    {
        Return = Length;
    }

    // Start if freebuffer is available
    do
    {
        FreeSize = SOCK_GetTX_FSR(SocketNumber);
        Status  = sock_sr_read(SocketNumber);

        if((byStatus != SOCK_ESTABLISHED) && (byStatus != SOCK_CLOSE_WAIT))
        {
            Return = 0;
            break;
        }
    }
    while(FreeSize < Return);

    // Copy data
    W5100_ProcessTX_Data(SocketNumber, (uint8_t *)pData, Return);
    W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_SEND);     // Wait to process the command...

    while((sock_ir_read(SocketNumber) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
    {
        if(sock_sr_read(SocketNumber) == SOCK_CLOSED)
        {
            SOCK_Close(SocketNumber);
            return 0;
        }
    }

    sock_ir_write(SocketNumber, Sn_IR_SEND_OK);
  #endif

    return Return;
}
//
//-------------------------------------------------------------------------------------------------
//
//  Name:           received
//
//  Parameter(s):   Socket_t        SocketNumber            Socket to send data to
//                  const uint8_t*  pData                   Buffer to put the data in
//                  size_t          Length
//  Return:         size_t
//
//  Description:    This function is an application I/F function which is used to receive the data
//                  in TCP mode. It continues to wait for data as needed  the application
//
//-------------------------------------------------------------------------------------------------
size_t NetSOCK::Received(Socket_t SocketNumber, uint8_t *pData, size_t Length)
{
    size_t Return = 0;

    if(Length > 0)
    {
        W5100_ProcessRX_Data(SocketNumber, pData, Length);
        W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_RECV);     // Wait to process the command...
        Return = Length;
    }
    
    return Return;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SendTo
//
//  Parameter(s):   Socket_t        SocketNumber            Socket to send data to
//                  const uint8_t*  pData                   Buffer containing the data
//                  size_t          Length                  lenght of the packet to send
//                  IP_Address_t    DstAddress              Destination IP Address
//                  IP_Port_t       DstPort                 Destination Port
//  Return:         size_t
//
//  Description:    This function is an application I/F function which is used to send the data for
//                  other protcol then TCP mode. Unlike TCP transmission, The peer's destination
//                  address and the port is needed.
//
//-------------------------------------------------------------------------------------------------
size_t NetSOCK::SendTo(Socket_t SocketNumber, const uint8_t* pData, size_t Length, IP_Address_t PeerAddressess, IP_Port_t PeerPort)
{
    size_t Return = 0;


    if(Length > W5100_TX_Size[SocketNumber])               // Check size not to exceed MAX size.
    {
        Return = W5100_TX_Size[SocketNumber];
    }
    else
    {
        Return = Length;
    }

    if((PeerAddressess == IP_ADDRESS(0,0,0,0)) || (PeerPort == 0) || (Return == 0))
    {
        Return = 0;
    }
    else
    {
        sock_dipr_write(SocketNumber, PeerAddressess);
        sock_dportr_write(SocketNumber, PeerPort);

        // copy data
        W5100_ProcessTX_Data(SocketNumber, (uint8_t*)pData, Return);
        W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_SEND);     // Wait to process the command...

        while((sock_ir_read(SocketNumber) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK)
        {
            if((sock_ir_read(SocketNumber) & Sn_IR_TIMEOUT) != 0)
            {
                sock_ir_write(SocketNumber, (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); // clear SEND_OK & TIMEOUT
                return 0;
            }
        }
        sock_ir_write(SocketNumber, Sn_IR_SEND_OK);
    }

    return Return;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_ReceivedFrom
//
//  Parameter(s):   Socket_t        SocketNumber
//                  uint8_t*        pData
//                  size_t          Length
//                  IP_Address_t*   pPeerAddress
//                  IP_Port_t*      pPeerPort
//  Return:         size_t          Length
//
//  Description:    This function is an application I/F function which is used to receive the data
//                  in other protcol then TCP mode. This function is used to receive UDP, IP_RAW
//                  and MAC_RAW mode, it handle the header as well.
//
//-------------------------------------------------------------------------------------------------
size_t SOCK_ReceivedFrom(Socket_t SocketNumber, uint8_t* pData, size_t Length, IP_Address_t* pPeerAddress, IP_Port_t* pPeerPort)
{
    uint8_t    Head[8];
    size_t     DataLength = 0;
    size_t     Ptr        = 0;

    if(Length > 0)
    {
        wPtr = sock_rx_rpr_read(SocketNumber);

        switch(sock_mr_read(SocketNumber) & 0x07)
        {
            case Sn_MR_UDP:
            {
                W5100_ReadData(SocketNumber, wPtr, Head, 8);
                wPtr += 8;

                // Read peer's IP address, port number, and get data lenght
                *pPeerAddress = ntohl(*((int32_t*)&Head[0]));
                *pPeerPort    = ntohs(*((int16_t*)&Head[4]));
                DataLength    = ntohs(*((int16_t*)&Head[6]));

                W5100_ReadData(SocketNumber, Ptr, pData, DataLength);         // data copy.
                Ptr += DataLength;
                sock_rx_rpr_write(SocketNumber, Ptr);
            }
            break;

            case Sn_MR_IPRAW:
            {
                W5100_ReadData(SocketNumber, Ptr, Head, 6);
                Ptr += 6;

                // Read peer's IP address, port number, and get data lenght
                *pPeerAddress = *((int32_t*)&Head[0]);
                DataLength    = *((int16_t*)&Head[4]);

                W5100_ReadData(SocketNumber, Ptr, pData, DataLength);         // data copy.
                Ptr += DataLength;
                sock_rx_rpr_write(SocketNumber, Ptr);
            }
            break;

            case Sn_MR_MACRAW:
            {
                W5100_ReadData(SocketNumber, Ptr, Head, 2);
                Ptr += 2;
                DataLength = *((int16_t*)&Head[0]);

                W5100_ReadData(SocketNumber, Ptr, pData, DataLength);
                Ptr += DataLength;
                sock_rx_rpr_write(SocketNumber, Ptr);
            }
            break;

            default:    
            {
            }
            break;
        }

        W5100_ProcessCmdAndWait(SocketNumber, Sn_CR_RECV);     // Wait to process the command...
    }

    return DataLength;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_GetTX_FSR
//
//  Parameter(s):   Socket_t  SocketNumber
//  Return:         size_t    Free Size
//
//  Description:    Get the free size in the nic TX Buffer
//
/-------------------------------------------------------------------------------------------------
size_t SOCK_GetTX_BufferSize(Socket_t SocketNumber)
{
  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	//Get Buffer size for TX
  #else
	return SOCK_GetTX_BufferSize(SocketNumber);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_GetRX_RSR
//
//  Parameter(s):   Socket_t      SocketNumber
//  Return:         size_t                      Buffer size.
//
//  Description:    Get the size of the received data size
//
//-------------------------------------------------------------------------------------------------
size_t SOCK_GetRX_BufferSize(Socket_t SocketNumber)
{
  #if (IP_HARDWARE_SOCKET == DEF_DISABLED)
	//Get Buffer size for TX
  #else
	return SOCK_GetTX_BufferSize(SocketNumber);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_SetSocket
//
//  Parameter(s):   Socket_t  SocketNumber
//					uint8_t	byProtocol
//					uint8_t	byFlag
//
//  Return:         void
//
//  Description:    Set the Socket mode
//
//-------------------------------------------------------------------------------------------------
#if (IP_HARDWARE_SOCKET == DEF_DISABLED)
void SOCK_SetSocket(Socket_t SocketNumber, uint8_t Protocol, uint8_t Flag)
{
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           SOCK_Open
//
//  Parameter(s):   Socket_t  SocketNumber
//
//  Return:         void
//
//  Description:    Open Socket with the data in structure
//
//-------------------------------------------------------------------------------------------------
#if (IP_HARDWARE_SOCKET == DEF_DISABLED)
void SOCK_Open(SOCKETSocketNumber)
{
}
#endif

//-------------------------------------------------------------------------------------------------

