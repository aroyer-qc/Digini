//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_socket.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SOCKET_RX_QUEUE_DEPTH       8

//-------------------------------------------------------------------------------------------------

class Socket
{
    friend class SocketManager;

    public:

                            Socket              (NetworkContext& Context, IP_Manager& Manager);

        void                Create              (SocketType_e Type);
        SystemState_e       Bind                (IP_Port_t Port);

      #if (IP_USE_TCP == DEF_ENABLED)
        SystemState_e       Listen              (uint16_t Backlog);
        SystemState_e       Accept              (Socket** ppClientSocket, SocketInfo_t* pClientInfo);
        SystemState_e       Connect             (SocketInfo_t* pInfo);
        SystemState_e       Shutdown            (void);
        bool                IsConnected         (void);

        SystemState_e       Send                (uint8_t* pData, size_t Length, size_t* pBytesSent);
        SystemState_e       Recv                (uint8_t* pBuffer, size_t BufferSize, size_t* pBytesReceived);
        SystemState_e       Recv                (IP_PacketMsg_t** ppMessage);
      #endif

        SystemState_e       SendTo              (uint8_t* pData, size_t Length, SocketInfo_t* pDestInfo, size_t* pBytesSent);
        SystemState_e       RecvFrom            (uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived);
        SystemState_e       RecvFrom            (IP_PacketMsg_t** ppMessage);

        void                Close               (void);

        SystemState_e       SetOption           (SocketOption_e Option, void* pValue, size_t ValueSize);
        SystemState_e       GetOption           (SocketOption_e Option, void* pValue, size_t* pValueSize);

        SocketState_e       GetState            (void);
        SocketType_e        GetType             (void);

        void                SetActive           (bool Active)           { m_Active = Active;                                       }
        bool                GetActive           (void)                  { return m_Active;                                         }

        bool                EnqueueMessage      (IP_PacketMsg_t* pMsg)  { return (nOS_QueueWrite(&m_RX_Queue, &pMsg, 0) == NOS_OK);}

      #if (IP_USE_UDP == DEF_ENABLED)
        bool                IsBound             (void);
        bool                IsListening         (void);

        UDP_Socket_t*       GetUDP              (void)                  { return m_Protocol.pUDP;                                   }
      #else
        UDP_Socket_t*       GetUDP              (void)                  { return nullptr;                                           }
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        TCP_Socket_t*       GetTCP              (void)                  { return m_Protocol.pTCP;                                   }
      #else
        TCP_Socket_t*       GetTCP              (void)                  { return nullptr;                                           }
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        RAW_Socket_t*       GetRAW              (void)                  { return m_Protocol.pRAW;                                   }
      #else
        RAW_Socket_t*       GetRAW              (void)                  { return nullptr;                                           }
      #endif

    #if (SOCKET_USE_STATISTICS == DEF_ENABLED)
        void                GetStats            (SocketStats_t* pStats);
        void                ResetStats          (void);
    #endif

        bool                HasData             (void);
        void                GetLocalInfo        (SocketInfo_t* pInfo);
        void                GetRemoteInfo       (SocketInfo_t* pInfo);
        IP_Port_t           GetLocalPort        (void)                  { return m_LocalInfo.Port;     }

    private:

        // Internal helpers
        SystemState_e       ValidateSocket      (void);
        //SystemState_e       AllocProtocolData   (void);
        void                FreeProtocolData    (void);
        void                FreeAllMessages     (nOS_Queue* pQueue);

        NetworkContext&         m_Context;
        IP_Manager&             m_Manager;

        SocketType_e            m_Type;
        SocketState_e           m_State;
        SocketInfo_t            m_LocalInfo;
        SocketInfo_t            m_RemoteInfo;
        SocketProtocol_t        m_Protocol;
        uint16_t                m_Backlog;
        bool                    m_IsListening;
        bool                    m_Active;

        uint16_t                m_SocketID;         // Used by dispatcher
        bool                    m_IsBlocking;
        bool                    m_IsBound;
        uint32_t                m_TimeoutMs;

        nOS_Queue               m_RX_Queue;                  // Per-socket RX queue (filled by IP/UDP/TCP dispatcher)
        IP_PacketMsg_t*         m_RX_QueueBuffer[SOCKET_RX_QUEUE_DEPTH];

        // Socket flags (SO_REUSEADDR, SO_BROADCAST, etc.)
        uint32_t                m_Flags;

    #if (SOCKET_USE_STATISTICS == DEF_ENABLED)
        SocketStats_t           m_Stats;
    #endif
};

//-------------------------------------------------------------------------------------------------

class SocketManager
{
    public:

        void                Initialize              (NetworkContext* pContext);

        Socket*             AllocSocket             (SocketType_e Type);
        void                FreeSocket              (Socket** ppSocket);

      #if (IP_USE_UDP == DEF_ENABLED)
        Socket*             FindUDP_SocketByPort    (IP_Port_t port);
        void                UDP_UnregisterSocket    (IP_Port_t Port);
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        Socket*             FindRAW_ByProtocol      (uint8_t protocol);
      #endif
      #if (IP_USE_TCP == DEF_ENABLED)
        Socket*             FindTCP_Connection      (uint32_t localIP, IP_Port_t localPort, uint32_t remoteIP, IP_Port_t remotePort);
        Socket*             FindTCP_Listener        (IP_Port_t localPort);
      #endif

    private:

        NetworkContext*     m_pContext                          = nullptr;
        Socket*             m_ActiveSockets[SOCKET_MAX_COUNT]   = { nullptr };
        uint8_t             m_ActiveCount                       = 0;
};

//-------------------------------------------------------------------------------------------------
