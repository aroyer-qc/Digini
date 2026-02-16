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

#if (IP_USE_TCP_CLIENT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum TCP_ClientState_e
{
    TCP_CLIENT_STATE_CLOSED         = 0,
    TCP_CLIENT_STATE_SYN_SENT,
    TCP_CLIENT_STATE_ESTABLISHED,
    TCP_CLIENT_STATE_FIN_WAIT_1,
    TCP_CLIENT_STATE_FIN_WAIT_2,
    TCP_CLIENT_STATE_TIME_WAIT,
    TCP_CLIENT_STATE_ERROR
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class TCP_Client
{
    public:

        bool                Initialize              (NetworkContext* pContext);
        bool                Connect                 (const IP_Address_t* pServerIP, uint16_t Port);
        size_t              Send                    (const uint8_t* pData, size_t Length);
        size_t              Receive                 (uint8_t* pBuffer, size_t MaxLength);
        void                Close                   (void);
        void                Process                 (void);
        TCP_ClientState_e   GetState                (void)                              { return m_State; }
        bool                IsConnected             (void)                              { return (m_State == TCP_CLIENT_STATE_ESTABLISHED); }

    private:

        bool                SendSYN                 (void);
        bool                SendACK                 (uint32_t AckNumber);
        bool                SendFIN                 (void);
        bool                SendSegment             (const uint8_t* pPayload, size_t Length, bool PushFlag);

        bool                HandleIncoming          (IP_EthernetPacket_t* pPacket);
        bool                ParseTCP_Header         (IP_EthernetPacket_t* pPacket);
        void                ProcessIncomingFlags    (void);
        bool                ValidateSequence        (uint32_t Seq, uint32_t Len);

        void                RetransmitIfNeeded      (void);
        void                UpdateTimers            (void);

        NetworkContext*     m_pContext;
        Socket*             m_pSocket;

        TCP_ClientState_e   m_State;

        IP_Address_t        m_ServerIP;
        uint16_t            m_ServerPort;

        uint32_t            m_SeqNumber;            // Our sequence number
        uint32_t            m_AckNumber;            // Expected next byte from server
        uint16_t            m_RemoteWindow;
        uint16_t            m_LocalWindow;

        TickCount_t         m_LastSendTick;
        TickCount_t         m_LastReceivedTick;
        TickCount_t         m_RetransmitStart;

        bool                m_RetransmitPending;
        uint8_t             m_LastFlags;            // SYN, ACK, FIN, PSH
        size_t              m_LastPayloadLength;

        TickCount_t         m_ConnectionStart;

        uint8_t*            m_pLastSegment;
        size_t              m_LastSegmentLength;

        // TODO replace with my stuff... no static buffer
        uint8_t             m_TxBuffer[512];
        size_t              m_TxLength;

        uint8_t             m_RxBuffer[512];
        size_t              m_RX_Length;
};

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_TCP_CLIENT == DEF_ENABLED)
