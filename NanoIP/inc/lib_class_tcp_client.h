//
//  Description:    Minimal deterministic TCP client for NanoIP stack.
//                  - Active open only (client mode)
//                  - Non-blocking, state-machine driven
//                  - Zero dynamic allocation
//                  - Integrates with Socket + NetworkContext
//                  - Provides a byte-stream interface for MQTT/HTTP/etc.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  TCP Client States
//-------------------------------------------------------------------------------------------------
typedef enum
{
    TCP_CLIENT_STATE_CLOSED = 0,
    TCP_CLIENT_STATE_SYN_SENT,
    TCP_CLIENT_STATE_ESTABLISHED,
    TCP_CLIENT_STATE_FIN_WAIT_1,
    TCP_CLIENT_STATE_FIN_WAIT_2,
    TCP_CLIENT_STATE_TIME_WAIT,
    TCP_CLIENT_STATE_ERROR
} TCP_ClientState_e;

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

        bool                SendSyn                 (void);
        bool                SendAck                 (uint32_t AckNumber);
        bool                SendFin                 (void);
        bool                SendSegment             (const uint8_t* pPayload, size_t Length, bool PushFlag);

        bool                HandleIncoming          (IP_EthernetPacket_t* pPacket);
        bool                ParseTcpHeader          (IP_EthernetPacket_t* pPacket);

        void                RetransmitIfNeeded      (void);
        void                UpdateTimers            (void);

        NetworkContext*     m_pContext;
        Socket*             m_pSocket;

        TCP_ClientState_e   m_State;

        IP_Address_t        m_ServerIP;
        uint16_t            m_ServerPort;

        uint32_t            m_SeqNumber;         // Our sequence number
        uint32_t            m_AckNumber;         // Expected next byte from server

        TickCount_t         m_LastSendTick;
        TickCount_t         m_LastRecvTick;
        TickCount_t         m_RetransmitStart;

        // TODO replace with my stuff... no static buffer
        uint8_t             m_TxBuffer[512];
        size_t              m_TxLength;

        uint8_t             m_RxBuffer[512];
        size_t              m_RxLength;
};

#endif // LIB_CLASS_TCP_CLIENT_H


class TCP_Client
{
public:
    bool Connect(IP_Address_t RemoteIP, uint16_t RemotePort);
    bool Send(const uint8_t* pData, size_t Length);
    bool Receive(uint8_t* pBuffer, size_t* pLength);
    void Close();

private:
    TCP_ControlBlock TCB;
    void HandleSynAck(...);
    void HandleData(...);
    void HandleFin(...);
};