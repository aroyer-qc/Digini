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

#define UDP_RX_QUEUE_DEPTH          8
#define RAW_RX_QUEUE_DEPTH          4
#define TCP_RX_QUEUE_DEPTH          8
#define TCP_TX_QUEUE_DEPTH          8

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

// Socket Types
enum SocketType_e
{
    SOCKET_TYPE_NONE,
    SOCKET_TYPE_STREAM,
    SOCKET_TYPE_DATAGRAM,
    SOCKET_TYPE_RAW_IP,
    SOCKET_TYPE_UNKNOWN,
    SOCKET_TYPE_INVALID
};

// Socket States
enum SocketState_e
{
    SOCKET_STATE_CLOSED,
    SOCKET_STATE_LISTENING,
    SOCKET_STATE_CONNECTING,
    SOCKET_STATE_CONNECTED,
    SOCKET_STATE_CLOSING,
    SOCKET_STATE_ERROR
};

enum TCP_State_e
{
    TCP_STATE_CLOSED,
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECEIVED,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_CLOSING,
    TCP_STATE_LAST_ACK,
    TCP_STATE_TIME_WAIT
};


// Socket Options
enum SocketOption_e
{
    SOCKET_OPT_NON_BLOCKING,
    SOCKET_OPT_BROADCAST,
    SOCKET_OPT_REUSE_ADDRESS,
    SOCKET_OPT_TIMEOUT,
    SOCKET_OPT_KEEP_ALIVE,
};

enum SocketFlag_e
{
    SOCKET_FLAG_NONE        = 0x00000000,

    SOCKET_FLAG_BROADCAST   = 0x00000001,   // Allow sending to 255.255.255.255
    SOCKET_FLAG_REUSEADDR   = 0x00000002,   // Allow binding to an address already in use

    // Future flags:
    SOCKET_FLAG_KEEPALIVE   = 0x00000004,   // TCP keepalive
    SOCKET_FLAG_LINGER      = 0x00000008,   // TCP linger
    SOCKET_FLAG_RESERVED1   = 0x00000010,
    SOCKET_FLAG_RESERVED2   = 0x00000020,

};

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

typedef struct
{
    uint8_t*    pData;         // pointer to received bytes (zero-copy)
    uint16_t    Length;        // number of bytes in this segment
} TCP_Message_t;

typedef struct
{
    uint32_t    RemoteIP;      // sender IP
    uint32_t    LocalIP;       // destination IP (interface)
    uint8_t     Protocol;      // IP protocol number (ICMP, IGMP, custom)
    uint8_t*    pData;         // pointer to payload (zero-copy)
    uint16_t    Length;        // payload length
} RAW_Message_t;

// Socket Address Structure
struct SocketInfo_t
{
    IP_Address_t    Address;
    IP_Port_t       Port;
};


struct UDP_Socket_t
{
    IP_Port_t      LocalPort;      // Bound port (0 = unbound)
    IP_Address_t   LocalIP;        // Optional: only if multi-IP system
    nOS_Queue      RX_Queue;       // Queue of IP_PacketMsg_t* (ownership transfers here)
    UDP_Message_t  RX_QueueBuffer[UDP_RX_QUEUE_DEPTH];
    uint16_t       Flags;          // Bitmask: broadcast allowed, reuse-port, etc.
                                   // (optional, but future-proof)
};

struct TCP_Socket_t
{
    // 4-tuple identifying the connection
    IP_Address_t    LocalIP;
    uint16_t        LocalPort;
    IP_Address_t    RemoteIP;
    uint16_t        RemotePort;
    // Sequence and ack numbers
    uint32_t        SndUna;     // First unacknowledged byte
    uint32_t        SndNxt;     // Next byte to send
    uint32_t        SndWnd;     // Send window size
    uint32_t        Iss;        // Initial send sequence
    uint32_t        RcvNxt;     // Next expected byte
    uint32_t        RcvWnd;     // Receive window size
    uint32_t        Irs;        // Initial receive sequence
    TCP_State_e     State;
    // Timers (RTO, keepalive, time-wait, etc.)
    uint32_t        RtoMs;
    uint32_t        RtoTimer;
    uint32_t        KeepAliveTimer;
    uint32_t        TimeWaitTimer;
    // Queues for data
    TCP_Message_t   RX_QueueBuffer[TCP_RX_QUEUE_DEPTH];
    nOS_Queue       RX_Queue;
    TCP_Message_t   TX_QueueBuffer[TCP_TX_QUEUE_DEPTH];
    nOS_Queue       TX_Queue;

    uint16_t        Flags;              // Flags e.g., FIN_SENT, FIN_RECEIVED, etc.
    uint16_t        Mss;                // Maximum Segment Size
    class Socket*   pSocket;            // Link to owning Socket object
};

struct RAW_Socket_t
{
    uint8_t         Protocol;           // IP protocol number (ICMP, etc.)
    IP_Address_t    LocalIP;            // Optional filter
    Socket*         RemoteIP;
    // RX message queue (socket-layer view of incoming RAW packets)
    RAW_Message_t   RX_QueueBuffer[RAW_RX_QUEUE_DEPTH];
    nOS_Queue       RX_Queue;
    // Optional flags for future behavior (promiscuous, drop-on-full, etc.)
    uint8_t         Flags;
};


union SocketProtocol_t
{
  #if (IP_USE_TCP == DEF_ENABLED)
    TCP_Socket_t*   pTCP;
  #endif
  #if (IP_USE_UDP == DEF_ENABLED)
    UDP_Socket_t*   pUDP;
  #endif
  #if (IP_USE_RAW == DEF_ENABLED)
    RAW_Socket_t*   pRAW;
  #endif
    void*           pPtr;    // fallback
};

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
      #endif


        SystemState_e       Send                (uint8_t* pData, size_t Length, size_t* pBytesSent);
        SystemState_e       Recv                (uint8_t* pBuffer, size_t BufferSize, size_t* pBytesReceived);

        SystemState_e       SendTo              (uint8_t* pData, size_t Length, SocketInfo_t* pDestInfo, size_t* pBytesSent);
        SystemState_e       RecvFrom            (uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived);

        void                Close               (void);

        SystemState_e       SetOption           (SocketOption_e Option, void* pValue, size_t ValueSize);
        SystemState_e       GetOption           (SocketOption_e Option, void* pValue, size_t* pValueSize);

        SocketState_e       GetState            (void);
        SocketType_e        GetType             (void);

        void                SetActive           (bool Active)           {m_Active = Active;            }
        bool                GetActive           (void)                  {return m_Active;              }

      #if (IP_USE_UDP == DEF_ENABLED)
        bool                IsBound             (void);
        bool                IsListening         (void);

        UDP_Socket_t*       GetUDP              (void)                  { return m_Protocol.pUDP;      }
      #else
        UDP_Socket_t*       GetUDP              (void)                  { return nullptr;              }
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        TCP_Socket_t*       GetTCP              (void)                  { return m_Protocol.pTCP;      }
      #else
        TCP_Socket_t*       GetTCP              (void)                  { return nullptr;              }
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        RAW_Socket_t*       GetRAW              (void)                  { return m_Protocol.pRAW;      }
      #else
        RAW_Socket_t*       GetRAW              (void)                  { return nullptr;              }
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

      #if (IP_USE_UDP == DEF_ENABLED)
        UDP_Socket_t            m_UDP_Storage;
      #endif

      #if (IP_USE_TCP == DEF_ENABLED)
        TCP_Socket_t            m_TCP_Storage;
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        RAW_Socket_t            m_RAW_Storage;
      #endif

        // Socket flags (SO_REUSEADDR, SO_BROADCAST, etc.)
        uint32_t                m_Flags;

        // Per-socket RX queue (filled by IP/UDP/TCP dispatcher)
        nOS_Queue               m_RxQueue;

        // Per-socket TX queue (used by TCP only)
        nOS_Queue               m_TxQueue;


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

        Socket*             FindUDP_SocketByPort    (IP_Port_t port);
        Socket*             FindRAW_ByProtocol      (uint8_t protocol);
        Socket*             FindTCP_Connection      (uint32_t localIP, IP_Port_t localPort, uint32_t remoteIP, IP_Port_t remotePort);
        Socket*             FindTCP_Listener        (IP_Port_t localPort);

    private:

        NetworkContext*     m_pContext                          = nullptr;
        Socket*             m_ActiveSockets[SOCKET_MAX_COUNT]   = { nullptr };
        uint8_t             m_ActiveCount                       = 0;
};

//-------------------------------------------------------------------------------------------------
