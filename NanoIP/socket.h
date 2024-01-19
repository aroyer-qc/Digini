//-------------------------------------------------------------------------------------------------
//
//  File :  socket.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2009-2024 Alain Royer.
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------


#define     SOCK_MODE_CLOSE                 0x00                                // < Unused socket
#define    	SOCK_MODE_TCP                   0x01                                // < TCP
#define     SOCK_MODE_UDP                   0x02                                // < UDP

//-------------------------------------------------------------------------------------------------
// macro(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef uint8_t       SOCKET;

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

//use size_t  for any length value

bool        SOCK_Socket         	(SOCKET SocketNumber, uint8_t Protocol, uint16_t SourcePort, uint8_t Flag);                         // Opens a socket(TCP or UDP or IP_RAW mode)
void        SOCK_Close          	(SOCKET SocketNumber);                                                                              // Close socket
bool        SOCK_Listen         	(SOCKET SocketNumber);                                                                              // Establish TCP connection (Passive connection)
bool        SOCK_Connect        	(SOCKET SocketNumber, uint32_t DstAddr, uint16_t DstPort);                                          // Establish TCP connection (Active connection)
void        SOCK_Disconnect     	(SOCKET SocketNumber);                                                                              // Disconnect the connection
uint16_t    SOCK_Send           	(SOCKET SocketNumber, const uint8_t* pData, size_t Length);                                            // Send data (TCP)
uint16_t    SOCK_Received       	(SOCKET SocketNumber, uint8_t* pData, size_t Length);                                                  // Receive data (TCP)
uint16_t    SOCK_SendTo         	(SOCKET SocketNumber, const uint8_t* pData, size_t Length, uint32_t PeerAddr, uint16_t PeerPort);      // Send data (UDP/IP RAW)
uint16_t    SOCK_ReceivedFrom   	(SOCKET SocketNumber, uint8_t* pData, size_t Length, uint32_t* pPeerAddr, uint16_t* pPeerPort);        // Receive data (UDP/IP RAW)
size_t      SOCK_GetTX_BufferSize	(SOCKET SocketNumber);
size_t      SOCK_GetRX_BufferSize  	(SOCKET SocketNumber);

#ifdef SOCKET_PRIVATE
void 	    SOCK_SetSocket	    	(SOCKET	SocketNumber, uint8_t Protocol, uint8_t Flag);
void 	    SOCK_Open				(SOCKET SocketNumber);

#endif

//-------------------------------------------------------------------------------------------------


