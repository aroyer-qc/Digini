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
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext* 	pContext		Pointer on the context
//  Return:         void
//
//  Description:    Initialize the SNTP Client
//
//-------------------------------------------------------------------------------------------------
bool SNTP_Client::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;

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

    m_pSocket = pSockMgr->AllocSocket(SOCKET_TYPE_DATAGRAM);

    if(m_pSocket == nullptr)
    {
        return false;
    }

    bool NonBlocking = true;
    m_pSocket->SetOption(SOCKET_OPT_NON_BLOCKING, &NonBlocking, sizeof(bool));

    // Bind to ephemeral port (0 = auto-assign)
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
//  Name:           Request
//
//  Parameter(s):       Socket_t     SocketNumber
//                      uint8_t*     pDomainName1    Domain Name of the NTP Server 1
//                      uint8_t*     pDomainName2    Domain Name of the NTP_Server 2
//                      uint8_t*     pError          Pointer to return an error code
//  Return:             true or false
//
//  Description:    Send the SNTP request
//
//-------------------------------------------------------------------------------------------------
bool SNTP_Client::SendRequest(const IP_Address_t* pServerIP)
{
    if((m_pSocket == nullptr) || (pServerIP == nullptr))
    {
        return false;
    }

    uint8_t Packet[48];   //this will be on the pool
    memset(Packet, 0, sizeof(Packet));

    // LI = 0, Version = 4, Mode = 3 (client)
    Packet[0] = SNTP_LI_VN_MODE;

    // Transmit Timestamp (seconds since 1900-01-01)
    uint32_t Seconds1900 = GetSystemTime_Seconds_1900();

    Packet[40] = (uint8_t)((Seconds1900 >> 24) & 0xFF);
    Packet[41] = (uint8_t)((Seconds1900 >> 16) & 0xFF);
    Packet[42] = (uint8_t)((Seconds1900 >>  8) & 0xFF);
    Packet[43] = (uint8_t)((Seconds1900 >>  0) & 0xFF);

    // Fractional part (optional, set to 0)
    Packet[44] = 0;
    Packet[45] = 0;
    Packet[46] = 0;
    Packet[47] = 0;

    SocketInfo_t Dest;
    memset(&Dest, 0, sizeof(Dest));
    Dest.Address = *pServerIP;
    Dest.Port    = 123;             // SNTP server port

    size_t BytesSent = 0;
    SystemState_e State = m_pSocket->SendTo(Packet, sizeof(Packet), &Dest, &BytesSent);

    if((State != SYS_READY) || (BytesSent != sizeof(Packet)))
    {
        return false;
    }

    m_State = SNTP_STATE_WAIT_RESPONSE;
    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReceiveResponse
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
bool SNTP_Client::ReceiveResponse(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t      Buffer[128];
    size_t       BytesReceived = 0;
    SocketInfo_t Src;

    SystemState_e State = m_pSocket->RecvFrom(Buffer, sizeof(Buffer), &Src, &BytesReceived);

    if(State != SYS_READY)
    {
        return false;
    }

    if(BytesReceived < 48)
    {
        return false;
    }

    return ParseResponse(Buffer, BytesReceived);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseResponse
//
//  Parameter(s):   uint8_t*        pPacket
//                  size_t          Length
//
//  Return:         void
//
//  Description:
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
    extern uint32_t System_GetUnixTime(void);
    uint32_t UnixNow = System_GetUnixTime();

    return UnixNow + DIFF_1900_1970;
}

uint32_t SNTP_Client::Convert1900ToUnix(uint32_t Seconds1900)
{
    const uint32_t DIFF_1900_1970 = 2208988800UL;
    return Seconds1900 - DIFF_1900_1970;
}
//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_SNTP == DEF_ENABLED)
