//-------------------------------------------------------------------------------------------------
//
//  File : TCP_interface.h
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
// enum(s)
//-------------------------------------------------------------------------------------------------

enum TCP_State_e
{
    TCP_STATE_CLOSED = 0,      // No connection / socket inactive
    TCP_STATE_LISTEN,          // Server waiting for incoming SYN
    TCP_STATE_SYN_SENT,        // Client sent SYN, waiting for SYN+ACK
    TCP_STATE_SYN_RECEIVED,    // SYN received, SYN+ACK sent, waiting for ACK
    TCP_STATE_ESTABLISHED,     // Fully open connection

    TCP_STATE_FIN_WAIT_1,      // FIN sent, waiting for ACK or FIN from peer
    TCP_STATE_FIN_WAIT_2,      // ACK received, waiting for peer's FIN
    TCP_STATE_CLOSING,         // Simultaneous close: FIN sent and FIN received
    TCP_STATE_CLOSE_WAIT,      // FIN received, waiting for local application to close
    TCP_STATE_LAST_ACK,        // FIN sent after CLOSE_WAIT, waiting for final ACK
    TCP_STATE_TIME_WAIT,       // Waiting before final close (2MSL)

    TCP_STATE_ERROR            // Internal error state (RST, abort, corruption)
};

enum SocketEvent_e
{
    SOCKET_EVENT_NONE = 0,     // No event
    SOCKET_EVENT_CONNECTED,    // TCP connection established
    SOCKET_EVENT_RX_READY,     // New payload available in RX buffer
    SOCKET_EVENT_CLOSED,       // Connection closed (FIN or RST)
    SOCKET_EVENT_ERROR,        // NOT USE AT THIS POINT
    SOCKET_EVENT_ACCEPT,       // Connection accepted
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class TCP_Socket;               // forward declaration

class TCP_SocketEventHandler
{
    public:

        virtual void            OnSocketEvent       (TCP_Socket* pSocket, SocketEvent_e Event)      = 0;
};

class TCP_Socket
{
    public:

        virtual                 ~TCP_Socket         (){}

        virtual size_t          Send                (const uint8_t* pBuffer, size_t Length)         = 0;
        virtual size_t          Receive             (uint8_t* pBuffer, size_t MaxLength)            = 0;
        virtual void            Close               (void)                                          = 0;
        virtual bool            IsConnected         (void) const                                    = 0;
        virtual TCP_State_e     GetState            (void) const                                    = 0;
        virtual void            SetEventHandler     (TCP_SocketEventHandler* pHandler)              = 0;

      #if (IP_USE_TCP_SERVER == DEF_ENABLED)
        virtual bool            IsListening         (void) const                                    = 0;
        //virtual TCP_Socket*     GetAcceptedSocket   (void)                                          = 0;
      #endif

    protected:

        TCP_TX_Segment_t        m_TX_Window        [TCP_MAX_TX_SEGMENTS];
        uint32_t                m_NextSequence;
        uint32_t                m_UnAckedSequence;
        uint32_t                m_RetransmitTimeOut;
        bool                    m_Active;
};

class TCP_Manager
{
    public:

        virtual                 ~TCP_Manager    (){}
      #if (IP_USE_TCP_CLIENT == DEF_ENABLED)
        virtual TCP_Socket*     Connect         (const IP_Address_t& ServerIP, uint16_t Port)   = 0;
      #endif

      #if (IP_USE_TCP_SERVER == DEF_ENABLED)
        virtual TCP_Socket*     CreateSocket    (void)                                          = 0;    // Create a new TCP socket (server or client)
        virtual bool            EnterListen     (TCP_Socket* pSocket, uint16_t Backlog)         = 0;    // Put a socket into listening mode
        virtual void            Close           (TCP_Socket* pSocket)                           = 0;    // Close and free a socket
        virtual TCP_Socket*     Accept          (TCP_Socket* pListenSocket)                     = 0;    // Retrieve next accepted client socket (non-blocking)
      #endif

        virtual void            Process         (void)                                          = 0;
        virtual void            ProcessSegment  (IP_PacketMsg_t* pPacket)                       = 0;
        virtual IP_PacketMsg_t* SendSegment     (TCP_Socket* pSocket, const uint8_t* pPayload,
                                                 size_t Length, uint8_t Flags,
                                                 bool Retransmit = false)                       = 0;
};

//-------------------------------------------------------------------------------------------------

