//-------------------------------------------------------------------------------------------------
//
//  File : tcp.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2010-2024 Alain Royer.
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

//------ Note(s) ----------------------------------------------------------------------------------
//
//  TCP - Transport Control Protocol
//
//      Client                                  Server
//                            Flag's
//          
//          >---------------- SYN ----------------->    Request connection
//          <-------------- SYN,ACK ---------------<
//          >---------------- ACK ----------------->    note: request possible in ack for connection
//         
//          >------------ ACK + data -------------->    Request (if more than one segment)
//          >---------- PSH,ACK + data ------------>    Request (last data segment of request)
//          <---------------- ACK -----------------<
// 
//          <------------ ACK + data --------------<    Answers (if more than one segment)
//          <---------- PSH,ACK + data ------------<    Request (last data segment of Answers)
//          >---------------- ACK ----------------->
//
//          <---------------- FIN -----------------<
//          >------------- ACK (FIN) -------------->
//
//          Notes ... Packet are not necessarily in sequence
//
//*************************************************************************************************


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <ip.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
// 
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Initialize data structure for TCP and tiner for SOCKET
//  
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void NetTCP::Initialize(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process 
// 
//  Parameter(s):   IP_PacketMsg_t* pRX 
//  Return:         IP_PacketMsg_t* 
//
//  Description:    Process any incoming packet. First we check if the port is open, If it is,
//                  Basic negotiation is done here. request is then passed to the application
//                  using the listen() function with the right port pass as argument.
//  
//  Note(s):
//
//  client  >------- SYN ------->  Server
//          <-- SYN (ACK SYN) --<
//          >---- ACK (SYN) ---->
//          
//          >----- REQUEST ----->
//          >------- FIN ------->
//          <- ACK (Req + FIN) -<
//          
//          <----- ANSWER ------<
//          <------- FIN -------<
//          >---- ACK (FIN) ---->
//
//
//      Normal TCP Flag combination
// 
//          Except for the initial SYN packet, every packet in a connection must have the ACK bit set. 
//          
//              handshake which establishes a TCP connection.
//                  SYN
//                  SYN-ACK
//                  ACK
//
//              Graceful teardown of an existing connection. 
//                  FIN ACK
//                  ACK
//
//              Used to immediately terminate an existing connection. 
//                  RST ACK 
//
//              Packets during the "conversation" portion of the connection (after the three-way
//              handshake but before the teardown or termination) contain just an ACK  default. 
//              Optionally, they may also contain PSH and/or URG.
// 
// 
//      Abnormal TCP Flag combination
//              
//              SYN FIN                 Packets with SYN FIN are malicious. in all forms 
//              SYN FIN PSH
//              SYN FIN RST
//              SYN FIN RST PSH
//              etc...
//     
//              FIN                     Packets should never contain just a FIN flag
//              nullptr                     It is illegal to have a packet with no flags set. 
//
//      Some other strange condition are trap  the code
//              SYN RST                 SYN must be alone for connection process to start
//              FIN RST                 RST is done in priority, so no packet is sent.
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* NetTCP::Process(IP_PacketMsg_t* pRX)
{
    IP_TCP_Header_t*        pTCP_RX;
    SocketInfo_t*           pSocket;
    PortInfo_t*             pPort;
    uint8_t                 Error;
    uint8_t                 Flag    = TCP_FLAG_nullptr;

    pTCP_RX = &pRX->Packet.u.TCP_Frame.Header;
    
    if((pPort = SOCK_ValidPort(pTCP_RX->DstPort, IP_PROTOCOL_TCP)) != nullptr)  // Check first if the port is in our allowed port list
    {
        if((pTCP_RX->Flags == TCP_FLAG_SYN) ||                                  // Is it a SYN packet
           ((pTCP_RX->Flags & TCP_FLAG_ACK) == TCP_FLAG_ACK))                   // Packet other than SYN should alway have ACK bit set
        {
            pTCP_RX->byFlags &= ~(TCP_FLAG_ACK | TCP_FLAG_URG);                 // Remove ACK bit & URG bit ( don't care )

            if(pTCP_RX->byFlags == TCP_FLAG_SYN)                                // Initiate a connection and create the socket
            {
                pSocket = SOCK_OpenSocket(pPort, pRX, &Error);
                Flag = TCP_FLAG_SYN;
            }
            else                                                                // Or get the already open socket
            {
                pSocket = SOCK_LookupSocket(pRX, &Error);
            }

            if(pSocket != nullptr)
            {
                pSocket->Timer = SOCK_TIME_OUT;                                 // We received a packet than reset timeout

                switch(pTCP_RX->Flags)
                {
                    case TCP_FLAG_SYN:                                          // Close a connection
                        pSocket->Send.Next++;
                        break;

                    case TCP_FLAG_RST:                                          // Force close of a connection
                        // TO DO verify the sequence number for real Reset
                        Error = SOCK_CloseSocket(pSocket);
                        return nullptr;
                        break;

                    case TCP_FLAG_PSH:                                          // Send Data to app
                        // TO DO verify the sequence number
                        Push();
                        break;

                    case TCP_FLAG_FIN:                                          // Close a connection
                        pSocket->Send.Next++;
                    //  pSocket->Receive.Next   =  ntohl(pRX->Packet.u.TCP_Frame.Header.AcknowledgeNumber - pSocket->AckNumber); //??
                        break;


                    default:                                                    // Here will end up any malicious bit combination
                        return nullptr;
                        break;
                }

                // We reach this point we need to ACK!
                //  switch(pSocket->byConnectionState)
                //  {
                //      case TCP_SOCKET_SYN_RECEIVE:                            // Client has ack connection
                //          pSocket->byConnectionState  = TCP_SOCKET_LISTENING;
                //          break;
                //
                //      case TCP_SOCKET_LISTENING:  break;
                //      case TCP_SOCKET_CLOSE_WAIT: break;
                //  }
                //
                // test ?? 
                {
                    IP_PacketMsg_t* pTX;
                    if(pTCP_RX->Flags != TCP_FLAG_PSH)
                    {
                        return Ack(Flag, 0);
                    }
                    else
                    {
                        pTX = TCP_Ack(pSocket, Flag, 0);
                        
                        CS8900_Send(pTX);
                        pMemory->Free((void**)&pTX);
                        return Send(nullptr, 0);
                    }
                }


            }
        }
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PutHeader   
// 
//  Parameter(s):   Socket_t*           pSocket
//                  IP_PacketMsg_t*     pTX
//                  size_t              PacketSize
//  Return:         void 
//
//  Description:    Put in header everything static
//  
//  Requirement:    All other data must be already in the header
// 
//-------------------------------------------------------------------------------------------------
void NetTCP::PutHeader(SocketInfo_t* pSocket, IP_PacketMsg_t* pTX, size_t PacketSize)
{
    IP_TCP_Header_t*    pTCP_TX;
    IP_PseudoHeader_t*  pPseudo_TX;

    pTX->Packet.u.IP_Frame.Header.DstIP_Address = pSocket->ClientIP;
    pTX->Packet.u.IP_Frame.Header.SrcIP_Address = IP_HostAddr;

    pTCP_TX                     = &pTX->Packet.u.TCP_Frame.Header;
    pTCP_TX->SrcPort            = pSocket->pPortInfo->Number;
    pTCP_TX->DstPort            = pSocket->ClientPort;
    pTCP_TX->SequenceNumber     = htonl(pSocket->SequenceNumber + (int32_t)pSocket->Receive.Next);
    pTCP_TX->AcknowledgeNumber  = htonl(pSocket->AckNumber + (int32_t)pSocket->Send.Next);
    pTCP_TX->Flags             |= TCP_FLAG_ACK;
    pTCP_TX->Offset             = 0x60;                             // to do process this criss
    pTCP_TX->Window             = htons(TCP_WINDOW_SIZE);
    pTCP_TX->OptionData.by.by0  = 2;
    pTCP_TX->OptionData.by.by1  = 4;
    pTCP_TX->OptionData.by.by2  = 4;
    pTCP_TX->OptionData.by.by3  = 0xB0;

    // Setup pseudo header for checksum calculation
    pPseudo_TX                  = &pTX->Packet.u.TCP_PseudoFrame.Header;
    pPseudo_TX->Protocol        = IP_PROTOCOL_TCP;
    pPseudo_TX->Lengthght          = htons(PacketSize);

    pTCP_TX->Checksum           = 0;  //??? next is = also
    pTCP_TX->Checksum           = IP_CalculateChecksum(pPseudo_TX, PacketSize + (int16_t)sizeof(IP_PseudoHeader_t));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
// 
//  Parameter(s):   void
//  Return:         IP_PacketMsg_t* pTX
//
//  Description:    
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* NetTCP::Send(SocketInfo_t* pSocket, uint8_t* pBuffer, size_t Size)
{
    IP_PacketMsg_t*     pTX         = nullptr;
    IP_TCP_Header_t*    pTCP_TX;

// use memory allocation
    uint8_t Temp[100];
    uint8_t Buffer[1000];

    LIB_sprintf(Temp, "HTTP/1.1 404 Not Found\r\n");
    LIB_strcat(Buffer, Temp);
    LIB_sprintf(Temp, "Content-type: text/html\r\n");
    LIB_strcat(Buffer, Temp);
    LIB_sprintf(Temp, "Content-length: 114\r\n");
    LIB_strcat(Buffer, Temp);
    LIB_sprintf(Temp, "\r\n");
    LIB_strcat(Buffer, Temp);
    LIB_sprintf(Temp, "<html><head><title>Not Found</title></head><body>\r\n");
    LIB_strcat(Buffer, Temp);
    LIB_sprintf(Temp, "Sorry, the object you requested was not found.\r\n");
    LIB_strcat(Buffer, Temp);
    LIB_sprintf(Temp, "</body><html>\r\n");
    LIB_strcat(Buffer, Temp);


    pTX = (IP_PacketMsg_t*)pMemory->AllocAndClear(Size + sizeof(IP_EthernetHeader_t) + TCP_ACK_IP_PACKET_SIZE);					    // Get memory for TX packet
    
    if(pTX != nullptr)
    {
        Size = strlen(Buffer);
        memcpy(((uint8_t*)&pTX->Packet.u.TCP_Frame.Header.OptionData + 4), Buffer, Size);
        
        pTX->PacketSize = (Size + sizeof(IP_EthernetHeader_t) + TCP_ACK_IP_PACKET_SIZE);
        pTCP_TX         = &pTX->Packet.u.TCP_Frame.Header;
        pTCP_TX->Flags  = TCP_FLAG_PSH;
        PutHeader(pTX, TCP_ACK_PACKET_SIZE + Size);

        // Setup MAC & IP header
        memcpy(&pTX->Packet.u.ETH_Header.Dst.Addr, &pSocket->MAC[0], 6);
        pTX->Packet.u.IP_Frame.Header.Lengthght     = htons(Size + TCP_ACK_IP_PACKET_SIZE);
        pTX->Packet.u.IP_Frame.Header.Protocol = IP_PROTOCOL_TCP;
        IP_PutHeader(pTX);
    }

    return pTX;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Ack
// 
//  Parameter(s):   SocketInfo_t*       pSocket
//                  uint8_t             Flag
//                  size_t              Size
//  Return:         IP_PacketMsg_t*     pTX
//
//  Description:    
//
//-------------------------------------------------------------------------------------------------
IP_PacketMsg_t* NetTCP::Ack(SocketInfo_t* pSocket, uint8_t Flag, size_t Size)
{
    IP_PacketMsg_t*     pTX         = nullptr;
    IP_TCP_Header_t*    pTCP_TX;
    size_t              PacketSize;

    
    PacketSize = Size + sizeof(IP_EthernetHeader_t) + TCP_ACK_IP_PACKET_SIZE;

    pTX = (IP_PacketMsg_t*)pMemory->AllocAndClear(PacketSize);					    // Get memory for TX packet
    
    if(pTX != nullptr)
    {
        pTX->PacketSize = PacketSize;
        pTCP_TX         = &pTX->Packet.u.TCP_Frame.Header;
        pTCP_TX->Flags  = Flag;
        PutHeader(pTX, TCP_ACK_PACKET_SIZE);

        // Setup MAC & IP header
        memcpy(&pTX->Packet.u.ETH_Header.Dst.Addr, &pSocket->MAC[0], 6);
        pTX->Packet.u.IP_Frame.Header.Lengthght   = htons(Size + TCP_ACK_IP_PACKET_SIZE);
        pTX->Packet.u.IP_Frame.Header.Protocol = IP_PROTOCOL_TCP;
        IP_PutHeader(pTX);
    }

    return pTX;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Push
// 
//  Parameter(s):   SocketInfo_t*       pSocket
//                  IP_PacketMsg_t*     pRX
//                  
//  Return:         void
//
//  Description:    
//  
//-------------------------------------------------------------------------------------------------
void NetTCP::Push(SocketInfo_t* pSocket, IP_PacketMsg_t* pRX)
{
    uint8_t DataOffset;
    size_t  Size;

    DataOffset = (uint8_t)((pRX->Packet.u.TCP_Frame.Header.Offset & 0xF0) >> 2) + (uint8_t)sizeof(IP_IP_Frame_t);
    Size       = pRX->PacketSize - size_t(DataOffset);

    // Pass pointer of data to service function to get data
    if(pSocket->pPortInfo->pFunction != nullptr)  // temporary
    {
        pSocket->pPortInfo->pFunction(pSocket, &pRX->Packet.u.RawData[DataOffset], Size); 
    }

    pSocket->ConnectionState  = TCP_SOCKET_LISTENING;
    pSocket->Send.Next       += Size;
    pSocket->Receive.Next     = (uint16_t)(ntohl(pRX->Packet.u.TCP_Frame.Header.AcknowledgeNumber) - pSocket->SequenceNumber);
}

//-------------------------------------------------------------------------------------------------




















/*
                                DBG_UartPrintf("Socket Src:%d.%d.%d.%d:%d on port %d listening state\n", uint8_t(pSocket->ClientIP >> 24),
                                                                                                         uint8_t(pSocket->ClientIP >> 16),
                                                                                                         uint8_t(pSocket->ClientIP >> 8),
                                                                                                         uint8_t(pSocket->ClientIP),
                                                                                                         ntohs(pSocket->ClientPort),
                                                                                                         ntohs(pPort->Number));

        DBG_UartPrintf("Socket Src:%d.%d.%d.%d:%d on port %d Negotiating state\n", uint8_t(pSocket->ClientIP >> 24),
                                                                                   uint8_t(pSocket->ClientIP >> 16),
                                                                                   uint8_t(pSocket->ClientIP >> 8),
                                                                                   uint8_t(pSocket->ClientIP),
                                                                                   ntohs(pSocket->ClientPort),,
                                                                                   ntohs(pPort->Number));

//                  DBG_UartPrintf("Socket Src:%d.%d.%d.%d:%d on port %d Receiving request state\n", uint8_t(pSocket->ClientIP >> 24),
//                                                                                                   uint8_t(pSocket->ClientIP >> 16),
//                                                                                                   uint8_t(pSocket->ClientIP >> 8),
//                                                                                                   uint8_t(pSocket->ClientIP),
//                                                                                                   ntohs(pSocket->wClientPort),
//                                                                                                   ntohs(pPort->wNumber));
*/
