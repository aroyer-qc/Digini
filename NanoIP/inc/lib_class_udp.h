//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_udp.h
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

#if (IP_USE_UDP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define UDP_PORT_BOOT_P_SERVER				67
#define UDP_PORT_BOOT_P_CLIENT				68

#define UDP_EPHEMERAL_PORT_MIN              49152
#define UDP_EPHEMERAL_PORT_MAX              65535

#define UDP_MAX_BINDS                       8

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct UDP_BoundEntry_t
{
    IP_Port_t Port;
    Socket*   pSocket;
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------
class NetUDP
{
    public:

        void 				    Initialize  		    (NetworkContext* pContext);
        void                    Process				    (IP_PacketMsg_t* pMsg);
        SystemState_e           Send                    (UDP_Socket_t* pUdp, uint8_t* pData, size_t Length, const SocketInfo_t* pDestInfo, size_t* pBytesSent);

        bool                    RegisterSocket          (Socket* pSock, IP_Port_t Port);
        void                    UnregisterSocket        (IP_Port_t Port);
        IP_Port_t               AllocateEphemeralPort   (void);

    private:

        Socket*                 FindSocketByPort        (IP_Port_t Port);

        UDP_BoundEntry_t        m_BoundSockets          [UDP_MAX_BINDS];
        size_t                  m_BoundCount            = 0;
        IP_Port_t               m_NextEphemeralPort     = UDP_EPHEMERAL_PORT_MIN;

        NetworkContext*         m_pContext;
};

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_UDP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


