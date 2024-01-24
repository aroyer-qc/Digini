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
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

//      typedef uint8_t       SOCKET;  this will be in the API interface

    typedef uint8_t       Socket_t;

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

//use size_t  for any length value
// TODO make interface for C to C++ to keep socket API call standard..

class NetSOCK
{
    public:
        
        bool        Socket         	    (Socket_t SocketNumber, uint8_t Protocol, IP_Port_t SourcePort, uint8_t Flag);                                // Opens a socket(TCP or UDP or IP_RAW mode)

Bind missing ??
        int         Bind                (int sockfd, const struct sockaddr *addr, size_t Length);

        bool        Listen         	    (Socket_t SocketNumber);                                                                                      // Establish TCP connection (Passive connection)

Accept missing ??
        int         Accept              (int sockfd, struct sockaddr *addr, size_t Length);   ???

        bool        Connect        	    (Socket_t SocketNumber, IP_Address_t DstAddress, IP_Port_t DstPort);                                          // Establish TCP connection (Active connection)
        size_t      Send           	    (Socket_t SocketNumber, const uint8_t* pData, size_t Length);                                                 // Send data (TCP)
        size_t      Received       	    (Socket_t SocketNumber, uint8_t* pData, size_t Length);                                                       // Receive data (TCP)
        void        Close          	    (Socket_t SocketNumber);                                                                                      // Close socket

        void        Disconnect     	    (Socket_t SocketNumber);                                                                                      // Disconnect the connection
        size_t      SendTo         	    (Socket_t SocketNumber, const uint8_t* pData, size_t Length, IP_Address_t PeerAddress, IP_Port_t PeerPort);   // Send data (UDP/IP RAW)
        size_t      ReceivedFrom   	    (Socket_t SocketNumber, uint8_t* pData, size_t Length, IP_Address_t* pPeerAddress, IP_Port_t* pPeerPort);     // Receive data (UDP/IP RAW)
        size_t      GetTX_BufferSize	(Socket_t SocketNumber);
        size_t      GetRX_BufferSize  	(Socket_t SocketNumber);

    private:

        void 	    SetSocket	    	(Socket_t SocketNumber, uint8_t Protocol, uint8_t Flag);
        void 	    Open				(Socket_t SocketNumber);
};

//-------------------------------------------------------------------------------------------------


