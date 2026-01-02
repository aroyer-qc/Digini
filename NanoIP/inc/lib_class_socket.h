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
// Enum(s)
//-------------------------------------------------------------------------------------------------

// Socket Types
enum SocketType_e
{
    SOCKET_TYPE_TCP,
    SOCKET_TYPE_UDP,
    SOCKET_TYPE_RAW_IP,
    SOCKET_TYPE_UNKNOWN
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
    SOCKET_OPT_BLOCKING,
    SOCKET_OPT_NON_BLOCKING,
    SOCKET_OPT_REUSE_ADDRESS,
    SOCKET_OPT_KEEP_ALIVE,
    SOCKET_OPT_TIMEOUT
};

/*
union
{
    TCP_Socket_t* tcp;
    UDP_Socket_t* udp;
    RAW_Socket_t* raw;
    void* ptr;
} m_Proto;

*/

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

// Socket Address Structure
struct SocketInfo_t
{
    IP_Address_t    Address;
    IP_Port_t       Port;
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
    MsgQueue_t      RxQueue;    // Filled by TCP input when data arrives
    MsgQueue_t      TxQueue;    // Filled by Socket::Send(), drained by TCP output

    // Flags
    uint16_t        Flags;      // e.g., FIN_SENT, FIN_RECEIVED, etc.
    uint16_t        Mss;        // Maximum Segment Size

    // Link to owning Socket object
    class Socket*   pSocket;
};

struct UDP_Socket_t
{
    IP_Address_t    LocalIP;    // Optional, may be ANY
    uint16_t        LocalPort;

    // Optional connected peer (for connected UDP)
    IP_Address_t    RemoteIP;
    uint16_t        RemotePort;
    bool            IsConnected;

    // Per-socket RX queue: each element is an IP_PacketMsg_t*
    MsgQueue_t      RxQueue;

    // Link to owning Socket
    Socket*         pSocket;

    // Options (broadcast, etc.)
    uint32_t        Flags;
};

struct RAW_Socket_t
{
    uint8_t         Protocol;   // IP protocol number (ICMP, etc.)
    IP_Address_t    LocalIP;    // Optional filter
    MsgQueue_t      RxQueue;    // Packets delivered to this socket

    Socket*         pSocket;
    uint32_t        Flags;
};
//-------------------------------------------------------------------------------------------------

class SocketManager
{
    public:
        static Socket*  AllocSocket(SocketType_e Type);
        static void     FreeSocket(Socket** ppSocket);
        static Socket*  GetByID(uint16_t id);

    private:
        static Socket   s_Sockets[SOCKET_MAX_COUNT];
        static bool     s_InUse[SOCKET_MAX_COUNT];
};

class Socket
{
    public:

        void            Create              (SocketType_e Type);
        SystemState_e   Bind                (SocketInfo_t* pInfo);
        SystemState_e   Listen              (uint16_t Backlog);
        SystemState_e   Accept              (Socket** ppClientSocket, SocketInfo_t* pClientInfo);
        SystemState_e   Connect             (SocketInfo_t* pInfo);

        SystemState_e   Send                (uint8_t* pData, size_t Length, size_t* pBytesSent);
        SystemState_e   Recv                (uint8_t* pBuffer, size_t BufferSize, size_t* pBytesReceived);

        SystemState_e   SendTo              (uint8_t* pData, size_t Length, SocketInfo_t* pDestInfo, size_t* pBytesSent);
        SystemState_e   RecvFrom            (uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived);

        SystemState_e   Close               (void);
        SystemState_e   Shutdown            (void);

        SystemState_e   SetOption           (SocketOption_e Option, void* pValue, size_t ValueSize);
        SystemState_e   GetOption           (SocketOption_e Option, void* pValue, size_t* pValueSize);

        SocketState_e   GetState            (void);
        SocketType_e    GetType             (void);

        bool            IsConnected         (void);
        bool            IsBound             (void);
        bool            IsListening         (void);

        static Socket*  AllocSocket         (SocketType_e Type);
        static void     FreeSocket          (Socket** ppSocket);

    #if (SOCKET_USE_STATISTICS == DEF_ENABLED)
        void            GetStats            (SocketStats_t* pStats);
        void            ResetStats          (void);
    #endif

    private:

        SocketType_e    m_Type;
        SocketState_e   m_State;

        SocketInfo_t    m_LocalInfo;
        SocketInfo_t    m_RemoteInfo;

        uint16_t        m_SocketID;         // Used by dispatcher
        bool            m_IsBlocking;
        bool            m_IsBound;
        bool            m_IsListening;
        uint32_t        m_TimeoutMs;

        // Socket flags (SO_REUSEADDR, SO_BROADCAST, etc.)
        uint32_t        m_Flags;

        // Per-socket RX queue (filled by IP/UDP/TCP dispatcher)
        MsgQueue_t      m_RxQueue;

        // Per-socket TX queue (used by TCP only)
        MsgQueue_t      m_TxQueue;

        // Protocol-specific data
        union
        {
            TCP_Socket_t*   tcp;
            UDP_Socket_t*   udp;
            RAW_Socket_t*   raw;
            void*           ptr;    // fallback
        } m_Proto;

    #if (SOCKET_USE_STATISTICS == DEF_ENABLED)
        SocketStats_t   m_Stats;
    #endif

        // Internal helpers
        SystemState_e   ValidateSocket      (void);
        SystemState_e   AllocProtocolData   (void);
        void            FreeProtocolData    (void);
};

#if 0
class Socket
{
    public:

        void            Create              (SocketType_e Type);
        SystemState_e   Bind                (SocketInfo_t* pInfo);
        SystemState_e   Listen              (uint16_t Backlog);
        SystemState_e   Accept              (Socket** ppClientSocket, SocketInfo_t* pClientInfo);
        SystemState_e   Connect             (SocketInfo_t* pInfo);
        SystemState_e   Send                (uint8_t* pData, size_t Length, size_t* pBytesSent);
        SystemState_e   Recv                (uint8_t* pBuffer, size_t BufferSize, size_t* pBytesReceived);
        SystemState_e   SendTo              (uint8_t* pData, size_t Length, SocketInfo_t* pDestInfo, size_t* pBytesSent);
        SystemState_e   RecvFrom            (uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived);
        SystemState_e   Close               (void);
        SystemState_e   Shutdown            (void);
        SystemState_e   SetOption           (SocketOption_e Option, void* pValue, size_t ValueSize);
        SystemState_e   GetOption           (SocketOption_e Option, void* pValue, size_t* pValueSize);
        SocketState_e   GetState            (void);
        SocketType_e    GetType             (void);
        bool            IsConnected         (void);
        bool            IsBound             (void);
        bool            IsListening         (void);

        // Static allocation helper
        static Socket*  AllocSocket         (SocketType_e Type);
        static void     FreeSocket          (Socket** ppSocket);

      #if (SOCKET_USE_STATISTICS == DEF_ENABLED)
        void            GetStats            (SocketStats_t* pStats);
        void            ResetStats          (void);
      #endif

    private:

        SocketType_e    m_Type;
        SocketState_e   m_State;
        SocketInfo_t    m_LocalInfo;
        SocketInfo_t    m_RemoteInfo;
        uint16_t        m_SocketID;
        bool            m_IsBlocking;
        bool            m_IsBound;
        uint32_t        m_TimeoutMs;

        // Protocol-specific data
        void*           m_pProtocolData;    // Points to TCP_Socket_t, UDP_Socket_t, or RAW_Socket_t

      #if (SOCKET_USE_STATISTICS == DEF_ENABLED)
        SocketStats_t   m_Stats;
      #endif

        // Internal helpers
        SystemState_e   ValidateSocket      (void);
        SystemState_e   AllocProtocolData   (void);
        void            FreeProtocolData    (void);
};
#endif

//-------------------------------------------------------------------------------------------------


















#if 0
//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define     SOCK_MODE_CLOSE                 0x00                                // < Unused socket
#define    	SOCK_MODE_TCP                   0x01                                // < TCP
#define     SOCK_MODE_UDP                   0x02                                // < UDP

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

//      typedef uint8_t       SOCKET;  this will be in the API interface

    typedef uint8_t       Socket_t;

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

//use size_t  for any length value
// TODO make interface for C to C++ to keep socket API call standard..

class NetSOCK
{
    public:

        bool        Socket         	    (Socket_t SocketNumber, uint8_t Protocol, IP_Port_t SourcePort, uint8_t Flag);                                // Opens a socket(TCP or UDP or IP_RAW mode)

//Bind missing ??
        int         Bind                (int sockfd, const struct sockaddr *addr, size_t Length);

        bool        Listen         	    (Socket_t SocketNumber);                                                                                      // Establish TCP connection (Passive connection)

//Accept missing ??
        int         Accept              (int sockfd, struct sockaddr *addr, size_t Length);   ???

        bool        Connect        	    (Socket_t SocketNumber, IP_Address_t DstAddress, IP_Port_t DstPort);                                          // Establish TCP connection (Active connection)
        size_t      Send           	    (Socket_t SocketNumber, const uint8_t* pData, size_t Length);                                                 // Send data (TCP)
        size_t      Received       	    (Socket_t SocketNumber, uint8_t* pData, size_t Length);                                                       // Receive data (TCP)
        void        Close          	    (Socket_t SocketNumber);                                                                                      // Close socket

        void        Disconnect     	    (Socket_t SocketNumber);                                                                                      // Disconnect the connection
        size_t      SendTo         	    (Socket_t SocketNumber, const uint8_t* pData, size_t Length, IP_Address_t PeerAddress, IP_Port_t PeerPort);   // Send data (UDP/IP RAW)
        size_t      ReceivedFrom   	    (Socket_t SocketNumber, uint8_t* pData, size_t Length, IP_Address_t* pPeerAddress, IP_Port_t* pPeerPort);     // Receive data (UDP/IP RAW)
        size_t      GetTX_BufferSize	(Socket_t SocketNumber);
        size_t      GetRX_BufferSize  	(Socket_t SocketNumber);

    private:

        void 	    SetSocket	    	(Socket_t SocketNumber, uint8_t Protocol, uint8_t Flag);
        void 	    Open				(Socket_t SocketNumber);
};


#endif
//-------------------------------------------------------------------------------------------------


