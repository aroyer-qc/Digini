//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_tcp_client.h
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

#if (IP_USE_TCP_CLIENT == DEF_ENABLED) || (IP_USE_TCP_SERVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

// TCP Socket (extends generic Socket)
class TCP_Socket : public Socket
{
    public:

                        TCP_Socket      (NetworkContext& Context, TCP_Manager& TCP);

        TCP_State_e     GetState        (void) const                    { return m_State; }
        bool            IsConnected     (void)                          { return (m_State == TCP_STATE_ESTABLISHED); }

        size_t          Send            (const uint8_t* pData, size_t Length);
        size_t          Receive         (uint8_t* pBuffer, size_t MaxLength);
        void            Close           (void);

    private:

        friend class    TCP_Manager;

        TCP_Manager*    m_pTCP;

        TCP_State_e     m_State;

        uint32_t        m_SeqNumber;        // Our sequence number
        uint32_t        m_AckNumber;        // Expected next byte
        uint16_t        m_RemoteWindow;
        uint16_t        m_LocalWindow;

        TickCount_t     m_LastSendTick;
        TickCount_t     m_LastReceivedTick;
        TickCount_t     m_RetransmitStart;

        bool            m_RetransmitPending;
        uint8_t         m_LastFlags;
        size_t          m_LastPayloadLength;
        // Replace static buffers later with your allocator
        uint8_t         m_TX_Buffer[512];
        size_t          m_TX_Length;

        uint8_t         m_RX_Buffer[512];
        size_t          m_RX_Length;
};

//-------------------------------------------------------------------------------------------------

class TCP_Manager
{
    public:

        bool            Initialize                      (NetworkContext& Context);

      #if (IP_USE_TCP_CLIENT == DEF_ENABLED)
        Socket*         Connect                         (const IP_Address_t& ServerIP, uint16_t Port);
      #endif

      #if (IP_USE_TCP_SERVER == DEF_ENABLED)
        SystemState_e   EnterListen                     (Socket* pSocket, uint16_t Backlog);
        void            Close                           (Socket* pSocket);
      #endif

        void            Process                         (void);                 // Called from main loop
        void            ProcessSegment                  (IP_EthernetPacket_t* pPacket);
        bool            SendSegment                     (TCP_Socket* pSocket, const uint8_t* pPayload, size_t Length, uint8_t Flags, bool Retransmit = false);

    private:

        bool            ParseTCP_Header                 (IP_EthernetPacket_t* pPacket, TCP_Socket*& pSockOut);
        void            ProcessIncomingFlags            (TCP_Socket* pSocket, IP_EthernetPacket_t* pPacket, uint8_t Flags, uint32_t Seq, uint32_t Ack, size_t PayloadLen);

        void            RetransmitIfNeeded              (TCP_Socket* pSocket);
        void            UpdateTimers                    (void);

        SocketManager*  m_pSocketManager;
        NetworkContext* m_pContext;

        // Later: dynamic list of sockets
        Socket*         m_pClientSocket;
        TCP_Socket*     m_pServerSockets[IP_TCP_MAX_LISTEN];
};

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_TCP_CLIENT == DEF_ENABLED)
