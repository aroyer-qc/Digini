//-------------------------------------------------------------------------------------------------
//
//  File :  tcp.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TCP_SYN_ACK_PACKET_SIZE         24
#define TCP_SYN_ACK_IP_PACKET_SIZE      44

#define TCP_ACK_PACKET_SIZE             24
#define TCP_ACK_IP_PACKET_SIZE          44


#define TCP_WINDOW_SIZE                 1396

#define TCP_FLAG_FIN                    0x01                // FIN Flag
#define TCP_FLAG_SYN                    0x02                // SYN Flag
#define TCP_FLAG_RST                    0x04                // Reset Flag
#define TCP_FLAG_PSH                    0x08                // Push Flag
#define TCP_FLAG_ACK                    0x10                // Acknowledge
#define TCP_FLAG_SYN_ACK                0x12                // SYN Flag
#define TCP_FLAG_URG                    0x20                // Urgent Flag

#define TCP_FLAG_SYN_FIN                0x03                // Illegal flag combination
#define TCP_FLAG_nullptr                0x00


#define TCP_HTTP_PORT                   htons(80)

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void                TCP_Init                        (void);
IP_PacketMsg_t*     TCP_Process                     (IP_PacketMsg_t* pRX);

//-------------------------------------------------------------------------------------------------

