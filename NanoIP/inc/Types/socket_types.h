//-------------------------------------------------------------------------------------------------
//
//  File : IP_frame_types.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct UDP_Message_t
{
    uint32_t    RemoteIP;       // sender IP
    IP_Port_t   RemotePort;     // sender port
    uint32_t    LocalIP;        // destination IP (interface)
    IP_Port_t   LocalPort;      // destination port
    uint8_t*    pData;          // pointer to payload (zero-copy)
    uint16_t    Length;         // payload length
};

struct TCP_Message_t
{
    uint8_t*    pData;         // pointer to received bytes (zero-copy)
    uint16_t    Length;        // number of bytes in this segment
};

struct RAW_Message_t
{
    uint32_t    RemoteIP;      // sender IP
    uint32_t    LocalIP;       // destination IP (interface)
    uint8_t     Protocol;      // IP protocol number (ICMP, IGMP, custom)
    uint8_t*    pData;         // pointer to payload (zero-copy)
    uint16_t    Length;        // payload length
};

// Socket Address Structure
struct SocketInfo_t
{
    IP_Address_t    Address;
    IP_Port_t       Port;
};

struct UDP_Socket_t
{
    IP_Port_t       LocalPort;     // Bound port (0 = unbound)
    uint16_t        Flags;         // Bitmask: broadcast allowed, reuse-port, etc. (optional, but future-proof)
};

struct RAW_Socket_t
{
    uint8_t         Protocol;           // IP protocol number (ICMP, etc.)
    IP_Address_t    LocalIP;            // Optional filter
    uint8_t         Flags;
};

union SocketProtocol_t
{
/*    TCP_Socket*     pTCP;  */ // Full C++ TCP protocol object.
                            // TCP is stateful and connection‑oriented, requiring:
                            //   - sequence/ack numbers
                            //   - sliding window management
                            //   - retransmission timers
                            //   - handshake/teardown state machine
                            //   - TX/RX buffering
                            //   - integration with TCP_Manager
                            // Because of this complexity, TCP uses a dedicated class
                            // rather than a lightweight struct.

    UDP_Socket_t*   pUDP;   // Lightweight POD (Plain Old Data) struct.
                            // UDP is stateless and connectionless:
                            //   - no sequence numbers
                            //   - no retransmission
                            //   - no timers
                            //   - no handshake
                            // Only stores bound port and simple flags, so a struct
                            // is sufficient and avoids unnecessary overhead.

    RAW_Socket_t*   pRAW;   // Lightweight POD struct.
                            // RAW sockets expose raw IP packets directly:
                            //   - no transport‑layer state
                            //   - no connection tracking
                            //   - no retransmission or timers
                            // Only protocol filters and flags are needed, so a
                            // simple struct is appropriate.

    void*           pPtr;   // Generic fallback pointer for future protocol types.
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
