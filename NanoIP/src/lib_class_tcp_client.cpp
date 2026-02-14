//-------------------------------------------------------------------------------------------------
//
//  File:           lib_class_tcp_client.h
//
//  Description:    Minimal deterministic TCP client for NanoIP stack.
//                  - Active open only (client mode)
//                  - Non-blocking, state-machine driven
//                  - Zero dynamic allocation
//                  - Integrates with Socket + NetworkContext
//                  - Provides a byte-stream interface for MQTT/HTTP/etc.
//
//-------------------------------------------------------------------------------------------------
#ifndef LIB_CLASS_TCP_CLIENT_H
#define LIB_CLASS_TCP_CLIENT_H

#include <stdint.h>
#include <stddef.h>

#include "lib_class_context.h"        // NetworkContext
#include "lib_class_socket.h"         // Socket, SocketInfo_t
#include "lib_ethernet_typedef.h"     // IP_Address_t
#include "lib_class_IP_Manager.h"     // For checksum helpers

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

//-------------------------------------------------------------------------------------------------
//  TCP Client Class
//-------------------------------------------------------------------------------------------------
class TCP_Client
{
public:

    //---------------------------------------------------------------------------------------------
    //
    //  Name:           Initialize
    //
    //  Parameter(s):   NetworkContext* pContext
    //
    //  Return:         bool
    //
    //  Description:    Allocates a TCP socket and resets internal state.
    //
    //---------------------------------------------------------------------------------------------
    bool Initialize(NetworkContext* pContext);

    //---------------------------------------------------------------------------------------------
    //
    //  Name:           Connect
    //
    //  Parameter(s):   const IP_Address_t* pServerIP
    //                  uint16_t Port
    //
    //  Return:         bool
    //
    //  Description:    Starts an active TCP open (SYN). Non-blocking.
    //                  Caller must periodically call Process().
    //
    //---------------------------------------------------------------------------------------------
    bool Connect(const IP_Address_t* pServerIP, uint16_t Port);

    //---------------------------------------------------------------------------------------------
    //
    //  Name:           Send
    //
    //  Parameter(s):   const uint8_t* pData
    //                  size_t Length
    //
    //  Return:         size_t
    //                      Number of bytes accepted for sending.
    //
    //  Description:    Queues data for transmission. Actual sending occurs in Process().
    //
    //---------------------------------------------------------------------------------------------
    size_t Send(const uint8_t* pData, size_t Length);

    //---------------------------------------------------------------------------------------------
    //
    //  Name:           Receive
    //
    //  Parameter(s):   uint8_t* pBuffer
    //                  size_t MaxLength
    //
    //  Return:         size_t
    //                      Number of bytes copied into pBuffer.
    //
    //  Description:    Retrieves available stream data from the internal RX buffer.
    //
    //---------------------------------------------------------------------------------------------
    size_t Receive(uint8_t* pBuffer, size_t MaxLength);

    //---------------------------------------------------------------------------------------------
    //
    //  Name:           Close
    //
    //  Description:    Initiates a graceful TCP close (FIN).
    //
    //---------------------------------------------------------------------------------------------
    void Close(void);

    //---------------------------------------------------------------------------------------------
    //
    //  Name:           Process
    //
    //  Description:    Drives the TCP state machine:
    //                      - SYN / SYN-ACK / ACK handshake
    //                      - Retransmissions
    //                      - Incoming segment parsing
    //                      - ACK generation
    //                      - FIN handshake
    //                      - Stream reassembly
    //
    //---------------------------------------------------------------------------------------------
    void Process(void);

    //---------------------------------------------------------------------------------------------
    //
    //  Name:           GetState
    //
    //---------------------------------------------------------------------------------------------
    TCP_ClientState_e GetState(void) const { return m_State; }

    bool IsConnected(void) const { return (m_State == TCP_CLIENT_STATE_ESTABLISHED); }

private:

    //---------------------------------------------------------------------------------------------
    //  Internal helpers
    //---------------------------------------------------------------------------------------------
    bool SendSyn(void);
    bool SendAck(uint32_t AckNumber);
    bool SendFin(void);
    bool SendSegment(const uint8_t* pPayload, size_t Length, bool PushFlag);

    bool HandleIncoming(IP_EthernetPacket_t* pPacket);
    bool ParseTcpHeader(IP_EthernetPacket_t* pPacket);

    void RetransmitIfNeeded(void);
    void UpdateTimers(void);

    //---------------------------------------------------------------------------------------------
    //  Members
    //---------------------------------------------------------------------------------------------
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

    // Simple TX/RX buffers (stream-based)
    uint8_t             m_TxBuffer[512];
    size_t              m_TxLength;

    uint8_t             m_RxBuffer[512];
    size_t              m_RxLength;
};

#endif // LIB_CLASS_TCP_CLIENT_H