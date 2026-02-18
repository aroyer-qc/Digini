//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_sntp.cpp
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

//------ Note(s) ----------------------------------------------------------------------------------
//
//  SNTP - Simple Network Time Protocol
//
//       <Message Format>
//
//                                 1  1  1  1  1  1  1  1  1  1  2  2  2  2  2  2  2  2  2  2  3  3
//   0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  LI |   VN   |  MODE  |         STRATUM       |         POLL          |      PRECISION        |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                           ROOT DELAY                                          |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                        ROOT DISPERSION                                        |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                REFERENCE TIMESTAMP SECOND (32)                                |
// |                               REFERENCE TIMESTAMP FRACTION (32)                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                ORIGINATE TIMESTAMP SECOND (32)                                |
// |                               ORIGINATE TIMESTAMP FRACTION (32)                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 RECEIVE TIMESTAMP SECOND (32)                                 |
// |                                RECEIVE TIMESTAMP FRACTION (32)                                |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 TRANSMIT TIMESTAMP SECOND (32)                                |
// |                                TRANSMIT TIMESTAMP FRACTION (32)                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 KEY IDENTIFIER (Optional) (32)                                |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                                 MESSAGE DIGEST (Optional) (128)                               |
// |                                                                                               |
// |                                                                                               |
// |                                                                                               |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (IP_USE_SNTP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SNTP_RESPONSE_TIMEOUT_MS        5000
#define SNTP_PORT                       123
#define SNTP_LI_VN_MODE                 0x23                        // Leap Indicator - 2 bits: 00 (No warning, current value), Version - 4 bits: 100, Mode Client - 3 bits: 011,

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext& Context    Reference to the active network context.
//
//  Return:         bool                    - true  : Initialization successful.
//                                          - false : Failed to allocate or bind the UDP socket.
//
//  Description:    Initializes the SNTP client by allocating a non-blocking UDP socket bound to
//                  an ephemeral port. The client remains idle until SendRequest() is invoked.
//
//-------------------------------------------------------------------------------------------------
bool SNTP_Client::Initialize(NetworkContext& Context)
{
    m_pContext = &Context;

    IP_Manager* pIP = m_pContext->GetIP_Manager();
    if(pIP == nullptr)
    {
        return false;
    }

    SocketManager* pSockMgr = pIP->GetSocketManager();

    if(pSockMgr == nullptr)
    {
        return false;
    }

    // Allocate a UDP socket for SNTP communication
    m_pSocket = pSockMgr->AllocSocket(SOCKET_TYPE_DATAGRAM);

    if(m_pSocket == nullptr)
    {
        return false;
    }

    // Enable non-blocking mode
    bool NonBlocking = true;
    m_pSocket->SetOption(SOCKET_OPT_NON_BLOCKING, &NonBlocking, sizeof(bool));

    // Bind to an ephemeral port (0 = auto-assign)
    SystemState_e State = m_pSocket->Bind(0);

    if(State != SYS_READY)
    {
        pSockMgr->FreeSocket(&m_pSocket);
        m_pSocket = nullptr;
        return false;
    }

    m_State = SNTP_STATE_INITIAL;
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start
//
//  Parameter(s):   const IP_Address_t  ServerIP         IPv4 address of the NTP server.
//
//  Return:         bool        - true  : SNTP request started successfully.
//                              - false : Socket not ready or send failed.
//
//  Description:    Starts a complete SNTP transaction by building and sending the request
//                  packet to the specified server, then arming the internal timeout and
//                  transitioning to WAIT_RESPONSE. The caller must periodically invoke
//                  Process() to complete the exchange.
//
//-------------------------------------------------------------------------------------------------
bool SNTP_Client::Start(const IP_Address_t ServerIP)
{
    if(m_pSocket == nullptr)
    {
        m_State = SNTP_STATE_ERROR;
        return false;
    }

    uint8_t* pPacket = (uint8_t*)pMemoryPool->AllocAndSet(sizeof(SNTP_Header_t), 0, MEM_DBG_SNTPTX);

    // LI = 0, Version = 4, Mode = 3 (client)
    pPacket[0] = SNTP_LI_VN_MODE;

    // Transmit Timestamp (seconds since 1900-01-01)
   // uint32_t Seconds1900 = GetSystemTime_Seconds_1900();
   // pPacket[24] = 0xE8;//(uint8_t)(Seconds1900 >> 24);
   // pPacket[25] = (uint8_t)(Seconds1900 >> 16);
   // pPacket[26] = (uint8_t)(Seconds1900 >>  8);
   // pPacket[27] = (uint8_t)(Seconds1900 >>  0);
   // pPacket[40] = 0xE8;//(uint8_t)(Seconds1900 >> 24);
   // pPacket[41] = (uint8_t)(Seconds1900 >> 16);
   // pPacket[42] = (uint8_t)(Seconds1900 >>  8);
   // pPacket[43] = (uint8_t)(Seconds1900 >>  0);

    // Fractional part left at zero
    SocketInfo_t Destination;
    memset(&Destination, 0, sizeof(SocketInfo_t));
    Destination.Address = ServerIP;
    Destination.Port    = SNTP_PORT;                                   // SNTP server port

    size_t BytesSent = 0;
    SystemState_e State = m_pSocket->SendTo(pPacket, sizeof(SNTP_Header_t), &Destination, &BytesSent);

    // Free TX buffer (It was copied into the packet)
    pMemoryPool->Free((void**)&pPacket);

    if((State != SYS_READY) || (BytesSent != sizeof(SNTP_Header_t)))
    {
        m_State = SNTP_STATE_ERROR;
        return false;
    }

    // Arm timeout and transition to WAIT_RESPONSE
    m_WaitStart = GetTick();
    m_State     = SNTP_STATE_WAIT_RESPONSE;

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Executes the internal SNTP state machine. This function must be called
//                  periodically from the TaskNetwork loop. It handles waiting for the server
//                  response, detecting timeouts, and transitioning to the appropriate state.
//
//                  Expected flow:
//                      - INITIAL:        Idle until SendRequest() is invoked.
//                      - WAIT_RESPONSE:  Poll socket for response; timeout if no reply.
//                      - DONE:           SNTP completed successfully.
//                      - ERROR:          SNTP failed; caller decides when to retry.
//
//-------------------------------------------------------------------------------------------------
void SNTP_Client::Process(void)
{
    switch(m_State)
    {
        // Idle state - waiting for TaskNetwork to call SendRequest()
        case SNTP_STATE_INITIAL:
            break;

        // Request sent - waiting for server response (non-blocking)
        case SNTP_STATE_WAIT_RESPONSE:
        {
            if(ReceiveResponse())                                           // Try to receive a response (non-blocking)
            {
                // ParseResponse() already set state to DONE
                break;
            }

            if(TickHasTimeOut(m_WaitStart, SNTP_RESPONSE_TIMEOUT_MS))       // Timeout check
            {
                m_State = SNTP_STATE_ERROR;
            }
            break;
        }

        // SNTP completed successfully — caller decides when to refresh time
        case SNTP_STATE_DONE:
            break;

        // Error state — caller decides when to retry or redo DNS
        case SNTP_STATE_ERROR:
            break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReceiveResponse
//
//  Parameter(s):   None
//
//  Return:         bool        - true  : A valid SNTP response was received and parsed
//                              - false : No response available, invalid packet, or parse failure
//
//  Description:    Attempts a non-blocking read on the SNTP socket. If a UDP packet is available,
//                  the function allocates a temporary buffer from the memory pool, receives up to
//                  sizeof(SNTP_Header_t) bytes, validates the packet length, and forwards the
//                  payload to ParseResponse() for protocol-level validation and timestamp
//                  extraction.
//
//                  The receive buffer is ALWAYS freed before returning, regardless of success or
//                  failure. This function does not block and does not retry; it simply checks for
//                  one pending SNTP response and processes it if present.
//
//  Note(s):        Expected packet size for SNTP is exactly 48 bytes (RFC 4330). Any packet
//                  smaller than this is discarded silently.
//
//-------------------------------------------------------------------------------------------------
bool SNTP_Client::ReceiveResponse(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    bool         Response = false;
    size_t       BytesReceived = 0;
    SocketInfo_t Src;

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->Alloc(sizeof(SNTP_Header_t), MEM_DBG_SNTPRX);

    if(pBuffer != nullptr)
    {
        SystemState_e State = m_pSocket->RecvFrom(pBuffer, sizeof(SNTP_Header_t), &Src, &BytesReceived);

        if(State != SYS_READY)
        {
            goto exit;
        }

        if(BytesReceived < sizeof(SNTP_Header_t))
        {
            goto exit;
        }

        Response = ParseResponse(pBuffer, BytesReceived);
    }

exit:

    pMemoryPool->Free((void**)&pBuffer);                        // Always free the buffer
    return Response;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseResponse
//
//  Parameter(s):   uint8_t* pPacket    Pointer to the received SNTP packet (expected 48 bytes)
//
//                  size_t Length       Length of the received payload. SNTP requires exactly 48
//                                      bytes, but the caller is responsible for enforcing this
//                                      constraint.
//
//  Return:         bool    - true  : Packet is a valid SNTP server response and the transmit
//                                    timestamp was successfully extracted and converted.
//                          - false : Invalid mode, malformed packet, or timestamp extraction
//                                    failed.
//
//  Description:    Performs protocol-level validation of an SNTP response. The function verifies
//                  that the received packet originates from an SNTP server (Mode = 4) or broadcast
//                  source (Mode = 5). If valid, the server's Transmit Timestamp (T3) is extracted
//                  from bytes 40-43, converted from NTP epoch (1900) to Unix epoch (1970), and
//                  stored internally.
//
//  Note(s):        This function does not modify system time directly; it only updates the
//                  client's internal Unix time field and advances the SNTP state machine to
//                  SNTP_STATE_DONE. System-level time synchronization is handled by the caller or
//                  higher-level logic.
//
//-------------------------------------------------------------------------------------------------
bool SNTP_Client::ParseResponse(uint8_t* pPacket, size_t Length)
{
    VAR_UNUSED(Length);

    // Basic sanity: Mode should be server (4) or broadcast (5)
    uint8_t LI_VN_Mode = pPacket[0];
    uint8_t Mode       = LI_VN_Mode & 0x07;

    if((Mode != 4) && (Mode != 5))
    {
        m_State = SNTP_STATE_ERROR;
        return false;
    }

    // Transmit Timestamp (server time)
    uint32_t Seconds1900 =
        ((uint32_t)pPacket[40] << 24) |
        ((uint32_t)pPacket[41] << 16) |
        ((uint32_t)pPacket[42] <<  8) |
        ((uint32_t)pPacket[43] <<  0);

    m_UnixTime = Convert1900ToUnix(Seconds1900);
    m_State    = SNTP_STATE_DONE;

    return true;
}


//that need big fix

uint32_t SNTP_Client::GetSystemTime_Seconds_1900(void)
{
    // Difference between 1900-01-01 and 1970-01-01 in seconds
    const uint32_t DIFF_1900_1970 = 2208988800UL;

    // Replace this with your real Unix time source
    //extern uint32_t System_GetUnixTime(void);
    uint32_t UnixNow = 0;//System_GetUnixTime();

    return UnixNow + DIFF_1900_1970;
}

uint32_t SNTP_Client::Convert1900ToUnix(uint32_t Seconds1900)
{
    const uint32_t DIFF_1900_1970 = 2208988800UL;
    return Seconds1900 - DIFF_1900_1970;
}
//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_SNTP == DEF_ENABLED)
