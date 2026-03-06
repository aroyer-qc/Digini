//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_mqtt.h
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
//  Description:    Lightweight MQTT 3.1.1 client for NanoIP stack.
//                  - Non-blocking, state-machine driven
//                  - Uses existing NetworkContext / Socket abstraction
//                  - No dynamic allocation required by design
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MQTT_BROKER_PORT                1883

#define MQTT_FLAG_CONNECT        0x00
#define MQTT_FLAG_CONNACK        0x00
#define MQTT_FLAG_PUBLISH        0x00
#define MQTT_FLAG_PUBACK         0x00
#define MQTT_FLAG_PUBREC         0x00
#define MQTT_FLAG_PUBREL         0x02
#define MQTT_FLAG_PUBCOMP        0x00
#define MQTT_FLAG_SUBSCRIBE      0x02
#define MQTT_FLAG_SUBACK         0x00
#define MQTT_FLAG_UNSUBSCRIBE    0x02
#define MQTT_FLAG_UNSUBACK       0x00
#define MQTT_FLAG_PINGREQ        0x00
#define MQTT_FLAG_PINGRESP       0x00
#define MQTT_FLAG_DISCONNECT     0x00
#define MQTT_FLAG_AUTH           0x00

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum MQTT_State_e
{
    MQTT_STATE_IDLE = 0,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_WAIT_CONNACK,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_WAIT_SUBACK,
    MQTT_STATE_WAIT_UNSUBACK,
    MQTT_STATE_PUBLISHING,
    MQTT_STATE_RECONNECTING,
    MQTT_STATE_ERROR
};

enum MQTT_QoS_e
{
    MQTT_QOS_0 = 0,
    MQTT_QOS_1 = 1,
    MQTT_QOS_2 = 2
};

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef void (*MQTT_MessageCallback_t)(void* pContext, const char* pTopic, const uint8_t* pPayload, size_t Length);

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class MQTT_Client
{
    public:

        bool                    Initialize                  (NetworkContext* pContext);
        bool                    Connect                     (const IP_Address_t* pServerIP, IP_Port_t Port, const char* pClientID, uint16_t KeepAliveSeconds);
        bool                    Subscribe                   (const char* pTopic, MQTT_QoS_e QoS);

      #if (MQTT_USE_UNSUBSCRIBE == DEF_ENABLED)
        bool                    Unsubscribe                 (const char* pTopic);
      #endif

        bool                    Publish                     (const char* pTopic, const uint8_t* pPayload, size_t Length, MQTT_QoS_e QoS);
        bool                    Disconnect                  (void);
        void                    Process                     (void);
        void                    SetMessageCallback          (MQTT_MessageCallback_t Callback, void* pUserContext);
        MQTT_State_e            GetState                    (void)                                                      { return m_State; }
        bool                    IsConnected                 (void)                                                      { return (m_State == MQTT_STATE_CONNECTED); }
        TCP_Socket*             GetSocket                   (void) const                                                { return m_pSocket; }

private:

        TCP_Socket*             TCP_Connect                 (const IP_Address_t* pServerIP, IP_Port_t Port);
        bool                    SendFrame                   (uint8_t* pBuffer, size_t Length);
        bool                    SendConnectFrame            (const char* pClientID);
        bool                    SendSubscribeFrame          (const char* pTopic, MQTT_QoS_e QoS);
      #if (MQTT_USE_UNSUBSCRIBE == DEF_ENABLED)
        bool                    SendUnsubscribeFrame        (const char* pTopic);
      #endif
        bool                    SendPublishFrame            (const char* pTopic, const uint8_t* pPayload, size_t Length, MQTT_QoS_e QoS);
        bool                    SendDisconnect              (void);
        bool                    SendPingReq                 (void);
        bool                    HandleIncomingData          (void);
        bool                    ParseIncomingPacket         (uint8_t* pBuffer, size_t Length);
        uint16_t                NextPacketID                (void);

        static bool             DecodeRemainingLength       (const uint8_t* pBuffer, size_t Length, size_t* pValue, size_t* pBytesUsed);

      #if (MQTT_USE_UNSUBSCRIBE == DEF_ENABLED)
        static size_t           EncodeFixedHeader           (uint8_t* pOut, uint8_t PacketType, uint8_t Flags, size_t RemainingLength);
      #endif

        NetworkContext*         m_pContext;
        TCP_Socket*             m_pSocket;
        MQTT_State_e            m_State;

        uint16_t                m_KeepAliveSeconds;
        TickCount_t             m_LastActivityTick;
        TickCount_t             m_ConnectStartTick;
        TickCount_t             m_PingSentTick;

        uint16_t                m_NextPacketID;
        bool                    m_WaitingPingResp;

        MQTT_MessageCallback_t  m_MessageCallback;
        void*                   m_pMessageContext;

        // Automatic reconnect
        TickCount_t             m_ReconnectStartTick;
        uint16_t                m_ReconnectDelaySeconds;
        bool                    m_ReconnectEnabled;



        char                    m_ClientID[64];
        IP_Address_t            m_LastServerIP;
        uint16_t                m_LastServerPort;
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)
