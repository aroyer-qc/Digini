//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_mqtt.cpp
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
//      NON-COMPLIANT BEHAVIOR IN THE CLIENT
//      -----------------------------------------------
//
//      1. Incomplete QoS 1 / QoS 2 handling
//         - The PUBLISH Packet Identifier is not stored.
//         - PUBACK is not validated (PacketID ignored).
//         - DUP flag is not processed.
//         - RETAIN flag is not processed.
//         - Full QoS 2 flow (PUBREC / PUBREL / PUBCOMP) is missing.
//
//      2. Simplified SUBACK / UNSUBACK processing
//         - SUBACK return codes are not checked.
//         - SUBACK Packet Identifier is not validated.
//         - UNSUBACK does not restore MQTT_STATE_CONNECTED.
//
//      3. Partial Keep-Alive compliance
//         - Keep-alive timer is not reset when receiving PUBLISH packets.
//         - A PINGREQ is sent even if recent MQTT traffic was received.
//
//      4. Minimal MQTT header validation
//         - No validation of header flags (DUP/QoS/RETAIN).
//         - No validation of packet-specific flags.
//         - Unsupported packet types (AUTH, PUBREL, PUBREC, PUBCOMP, etc.) are not handled.
//
//      5. Incomplete MQTT error handling
//         - CONNACK return codes other than 0x00 are not interpreted.
//         - MQTT_STATE_ERROR is never fully processed or recovered from.
//
//      6. Simplified MQTT state machine
//         - No dedicated state for QoS1 awaiting PUBACK (only PUBLISHING).
//         - No QoS2 states at all.
//         - No explicit transition after UNSUBACK.
//
//      7. Limited MQTT size validation
//         - Remaining Length is validated, but internal lengths
//           (e.g., Topic Length vs Remaining Length) are not fully checked.
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MQTT_CONNECT_TIMEOUT_MS                 10000           // 10 seconds
#define MQTT_PINGRESP_TIMEOUT_MS                5000            // 5 seconds
#define MQTT_DISCONNECTED_PERIOD_MS             500

// MQTT Remaining Length encoding (variable-length integer)
#define MQTT_REMAINING_LEN_MASK                 0x7F
#define MQTT_REMAINING_LEN_CONTINUATION         0x80
#define MQTT_REMAINING_LEN_BASE_128             128
#define MQTT_REMAINING_LEN_MAX_MULTIPLIER       (MQTT_REMAINING_LEN_BASE_128 * MQTT_REMAINING_LEN_BASE_128 * MQTT_REMAINING_LEN_BASE_128)
#define MQTT_CONNECT_REMAINING_SIZE             (10 + 2)        // Size of CONNECT variable header (10 bytes) + 2-byte ClientID length prefix
#define MQTT_PUBLISH_REMAINING_BASE_SIZE        (2)             // Size of PUBLISH topic length field (2 bytes)
#define MQTT_PUBLISH_REMAINING_QOS1_SIZE        (2)             // Size of PUBLISH Packet ID field for QoS > 0 (2 bytes)
#define MQTT_SUBSCRIBE_REMAINING_BASE_SIZE      (2 + 2 + 1)     // Size of SUBSCRIBE fixed fields: PacketID (2) + TopicLen (2) + QoS (1)
#define MQTT_UNSUBSCRIBE_REMAINING_BASE_SIZE    (2 + 2)         // Size of UNSUBSCRIBE fixed fields: PacketID (2) + TopicLen (2)

#define MQTT_PROTOCOL_HEADER_SIZE               8               // "\0\x04MQTT\x04\x02"

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum MQTT_ControlPacketType_e
{
    MQTT_PACKET_TYPE_RESERVED      = 0,
    MQTT_PACKET_TYPE_CONNECT       = 1,
    MQTT_PACKET_TYPE_CONNACK       = 2,
    MQTT_PACKET_TYPE_PUBLISH       = 3,
    MQTT_PACKET_TYPE_PUBACK        = 4,
    MQTT_PACKET_TYPE_PUBREC        = 5,
    MQTT_PACKET_TYPE_PUBREL        = 6,
    MQTT_PACKET_TYPE_PUBCOMP       = 7,
    MQTT_PACKET_TYPE_SUBSCRIBE     = 8,
    MQTT_PACKET_TYPE_SUBACK        = 9,
    MQTT_PACKET_TYPE_UNSUBSCRIBE   = 10,
    MQTT_PACKET_TYPE_UNSUBACK      = 11,
    MQTT_PACKET_TYPE_PING_REQUEST  = 12,
    MQTT_PACKET_TYPE_PING_RESPONSE = 13,
    MQTT_PACKET_TYPE_DISCONNECT    = 14,
    MQTT_PACKET_TYPE_AUTH          = 15
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           TaskMQTT_ClientWrapper
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    main() for the MQTT_Client
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" void TaskMQTT_ClientWrapper(void* pvParameters)
{
    (static_cast<MQTT_Client*>(pvParameters))->Run();
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext*  pContext   Pointer to the NanoIP network context
//                  MQTT_Handler*    pHandler   Application-level event callback handler
//
//  Return:         true  - Initialization successful
//                  false - Could not create task
//
//  Description:    Initializes the MQTT client internal state. This function stores the
//                  network context and application callback, resets all MQTT runtime
//                  variables, clears counters, and prepares the client for operation.
//
//                  No TCP socket is created here. The socket will be allocated later when
//                  Connect() is invoked by the application or by the reconnect logic.
//
//                  The MQTT client task (TaskMQTT_ClientWrapper) is created here and will
//                  handle all protocol-level operations (CONNECT, SUBACK, PING, RX parsing,
//                  timeouts, and reconnection logic).
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::Initialize(NetworkContext* pContext, MQTT_Handler* pHandler)
{
    m_pContext                = pContext;
    m_pHandler                = pHandler;
    m_pSocket                 = nullptr;
    m_SocketValid             = false;
    m_State                   = MQTT_STATE_IDLE;

    m_KeepAliveSeconds        = 0;
    m_LastActivityTick        = 0;
    m_ConnectStartTick        = 0;
    m_PingSentTick            = 0;

    m_ReconnectStartTick      = 0;
    m_ReconnectDelaySeconds   = 5;
    m_ReconnectEnabled        = true;
    m_UserRequestedDisconnect = false;

    m_NextPacketID            = 1;
    m_WaitingPingResponse     = false;

    m_ClientID[0]             = '\0';
    m_LastServerIP            = IP_ADDRESS(0,0,0,0);
    m_LastServerPort          = 0;
    m_pRX_Packet              = nullptr;

    nOS_SemCreate(&m_RX_ReadySem, 0, 1);

    bool State = nOS_ThreadCreate(&m_Handle,
                                  TaskMQTT_ClientWrapper,
                                  this,
                                  &m_Stack[0],
                                  TASK_MQTT_CLIENT_STACK_SIZE,
                                  TASK_MQTT_CLIENT_PRIO,
                                  "Task MQTT Client") == NOS_OK ? true : false;

    return State;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Run
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Main MQTT protocol engine. This function runs in its own dedicated task
//                  and drives the entire MQTT state machine. It performs the following:
//
//                  - Processes incoming TCP data when the RX semaphore is signaled.
//                    Incoming bytes are parsed into MQTT packets, and application-level
//                    events are triggered when a complete PUBLISH or control packet is
//                    received.
//
//                  - Manages all MQTT protocol states:
//                        - CONNECTING: waits for TCP establishment, then sends CONNECT.
//                        - WAIT_CONNACK: handled inside HandleIncomingData().
//                        - CONNECTED: handles keep-alive, PINGREQ/PINGRESP, and timeouts.
//                        - IDLE: waiting for the application to initiate a connection.
//
//                  - Handles protocol timeouts such as:
//                        - TCP connection timeout
//                        - PINGRESP timeout
//                        - Keep-alive expiration
//
//                  - Notifies the application task via OnEvent() whenever a state change
//                    or incoming MQTT message requires application-level processing.
//
//                  - Sleeps periodically to avoid CPU starvation and allow other tasks
//                    to run.
//
//                  This function never blocks indefinitely and never performs any
//                  application logic. It is strictly responsible for MQTT protocol
//                  management and socket-level operations.
//
//---------------------------------------------------------------------------------------------
void MQTT_Client::Run(void)
{
    while(1)
    {
        TickCount_t Now = GetTick();

        if(nOS_SemTake(&m_RX_ReadySem, NOS_NO_WAIT) == NOS_OK)
        {
            if(HandleIncomingData() == true)                                                        // Process incoming MQTT packets
            {
                OnEvent();                                                                          // Wake application
            }
        }

        switch(m_State)
        {
            case MQTT_STATE_CONNECTING:
            {
                if((m_pSocket != nullptr) && (m_pSocket->GetState() == TCP_STATE_ESTABLISHED))      // Wait for TCP connection to be established
                {
                    SendConnectFrame(m_ClientID);
                    m_State            = MQTT_STATE_WAIT_CONNACK;
                    m_LastActivityTick = Now;
                }
                else if((Now - m_ConnectStartTick) > MQTT_CONNECT_TIMEOUT_MS)
                {
                    m_State = MQTT_STATE_IDLE;                                                      // TCP connection timeout -> return to IDLE
                    OnEvent();                                                                      // Wake application so it can retry
                }
            }
            break;

            case MQTT_STATE_CONNECTED:
            {
                if(m_WaitingPingResponse && ((Now - m_PingSentTick) > MQTT_PINGRESP_TIMEOUT_MS))    // Check for PINGRESP timeout
                {
                    SendDisconnect();
                    m_State = MQTT_STATE_IDLE;
                    OnEvent();
                }

                if((Now - m_LastActivityTick) > (m_KeepAliveSeconds * TICKS_PER_SECOND))            // Keep-alive timer
                {
                    if(SendPingRequest() == true)
                    {
                        m_WaitingPingResponse = true;
                        m_PingSentTick    = Now;
                    }
                }
            }
            break;

            //case MQTT_STATE_IDLE:
            //case MQTT_STATE_WAIT_CONNACK:
            //case MQTT_STATE_WAIT_SUBACK:
            //case MQTT_STATE_WAIT_UNSUBACK:
            //case MQTT_STATE_PUBLISHING:
            default:
                break;
        }

        nOS_Sleep(50);
    }
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Connect
//
//  Parameter(s):   const IP_Address_t* pServerIP         MQTT broker IP address.
//                  uint16_t            Port              MQTT broker TCP port.
//                  const char*         pClientID         Null-terminated client identifier.
//                  uint16_t            KeepAliveSeconds  MQTT keep-alive interval in seconds.
//
//  Return:         true  - Connection attempt started successfully.
//                  false - Invalid parameters or socket allocation failure.
//
//  Description:    Initiates a non-blocking MQTT connection sequence. If an existing socket
//                  object is present but no longer connected, it is closed and released.
//                  Otherwise, a new TCP socket is created and a connection attempt is started.
//
//                  This function does *not* wait for the TCP connection or the CONNACK.
//                  All protocol-level progression (CONNECT -> CONNACK -> CONNECTED) is handled
//                  asynchronously inside the MQTT client task (Run()).
//
//                  On success, the internal state transitions to MQTT_STATE_CONNECTING and
//                  the MQTT client task will automatically continue the handshake.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::Connect(const IP_Address_t* pServerIP, uint16_t Port, const char* pClientID, uint16_t KeepAliveSeconds)
{
    if((pServerIP == nullptr) || (pClientID == nullptr))
    {
        return false;
    }

    if(m_pSocket != nullptr)
    {
        if(m_pSocket->IsConnected() == false)
        {
            m_pSocket->Close();                         // Dead Socket -> Close
            m_pSocket = nullptr;                        // and release
        }
        else
        {
            return true;                                // Live socket -> we don't reconnect
        }
    }

    m_LastServerIP   = *pServerIP;                      // Save it for reconnect
    m_LastServerPort = Port;
    strncpy(m_ClientID, pClientID, sizeof(m_ClientID) - 1);
    m_ClientID[sizeof(m_ClientID) - 1] = '\0';
    m_KeepAliveSeconds = KeepAliveSeconds;
    m_pSocket = TCP_Connect(pServerIP, Port);

    if(m_pSocket == nullptr)
    {
        m_State       = MQTT_STATE_IDLE;
        m_pSocket     = nullptr;
        m_SocketValid = false;

      #if (IP_DBG_MQTT == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Connect failed");
      #endif

        return false;
    }

    m_pSocket->SetEventHandler(this);
    m_SocketValid      = true;
    m_State            = MQTT_STATE_CONNECTING;
    m_ConnectStartTick = GetTick();
    m_LastActivityTick = m_ConnectStartTick;

  #if (IP_DBG_MQTT == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Connected");
  #endif

    return true;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Subscribe
//
//  Parameter(s):   const char*   pTopic   Null-terminated topic filter string.
//                  MQTT_QoS_e    QoS      Requested QoS level.
//
//  Return:         true  - SUBSCRIBE frame sent successfully.
//                  false - Client not in CONNECTED state or internal error.
//
//  Description:    Sends a non-blocking MQTT SUBSCRIBE request. This function does not wait
//                  for the SUBACK. All protocol-level progression (WAIT_SUBACK -> CONNECTED)
//                  is handled asynchronously inside the MQTT client task (Run()).
//
//                  On success, the internal state transitions to MQTT_STATE_WAIT_SUBACK.
//                  When the SUBACK is received, HandleIncomingData() will restore the state
//                  to MQTT_STATE_CONNECTED and notify the application via OnEvent().
//
//                  Wildcards ('+' and '#') are fully supported. Topic matching is performed
//                  by the MQTT broker; the client receives only the messages that match the
//                  subscribed filter.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::Subscribe(const char* pTopic, MQTT_QoS_e QoS)
{
    if(m_State != MQTT_STATE_CONNECTED)
    {
        return false;
    }

    if(SendSubscribeFrame(pTopic, QoS) == false)
    {
      #if (IP_DBG_MQTT == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Subscribing failed");
      #endif
        return false;
    }

    m_State = MQTT_STATE_WAIT_SUBACK;
    m_LastActivityTick = GetTick();

  #if (IP_DBG_MQTT == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Subscribe");
  #endif

    return true;
}

//---------------------------------------------------------------------------------------------
//  Name:           Unsubscribe
//
//  Parameter(s):   const char* pTopic   Null-terminated topic filter string to unsubscribe.
//
//  Return:         true  - UNSUBSCRIBE frame sent successfully.
//                  false - Client not in CONNECTED state or internal error.
//
//  Description:    Sends a non-blocking MQTT UNSUBSCRIBE request. This function does not wait
//                  for the UNSUBACK. All protocol-level progression (WAIT_UNSUBACK -> CONNECTED)
//                  is handled asynchronously inside the MQTT client task (Run()).
//
//                  On success, the internal state transitions to MQTT_STATE_WAIT_UNSUBACK.
//                  When the UNSUBACK is received, HandleIncomingData() will restore the state
//                  to MQTT_STATE_CONNECTED and notify the application via OnEvent().
//
//                  Wildcards ('+' and '#') are supported exactly as in SUBSCRIBE. Topic
//                  matching and subscription management are performed entirely by the MQTT
//                  broker; the client simply sends the request and processes the response.
//---------------------------------------------------------------------------------------------
#if (MQTT_USE_UNSUBSCRIBE == DEF_ENABLED)
bool MQTT_Client::Unsubscribe(const char* pTopic)
{
    if(m_State != MQTT_STATE_CONNECTED)
    {
        return false;
    }

    if(SendUnsubscribeFrame(pTopic) == false)
    {
      #if (IP_DBG_MQTT == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Unsubscribing failed");
      #endif

        return false;
    }

    m_State = MQTT_STATE_WAIT_UNSUBACK;
    m_LastActivityTick = GetTick();

  #if (IP_DBG_MQTT == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Unsubscribe");
  #endif

    return true;
}
#endif

//---------------------------------------------------------------------------------------------
//
//  Name:           Publish
//
//  Parameter(s):   const char*    pTopic       Null-terminated topic string.
//                  const uint8_t* pPayload     Pointer to the payload buffer.
//                  size_t         Length       Payload length in bytes.
//                  MQTT_QoS_e     QoS          Requested QoS level (typically 0 or 1).
//
//  Return:         true  - PUBLISH frame sent successfully.
//                  false - Client not in CONNECTED state or internal error.
//
//  Description:    Sends a non-blocking MQTT PUBLISH request. For QoS 0, the publish is
//                  considered complete immediately after the frame is sent. For QoS 1, the
//                  client transitions to MQTT_STATE_PUBLISHING and waits asynchronously for
//                  the PUBACK packet.
//
//                  All protocol-level progression (WAIT_PUBACK -> CONNECTED) is handled
//                  inside the MQTT client task (Run()). When the PUBACK is received,
//                  HandleIncomingData() restores the state to MQTT_STATE_CONNECTED and
//                  notifies the application via OnEvent().
//
//                  This function performs no blocking operations and does not wait for any
//                  server response. It simply queues/sends the PUBLISH frame and updates the
//                  internal state machine accordingly.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::Publish(const char* pTopic, const uint8_t* pPayload, size_t Length, MQTT_QoS_e QoS)
{
    if(m_State != MQTT_STATE_CONNECTED)
    {
        return false;
    }

    if(SendPublishFrame(pTopic, pPayload, Length, QoS) == false)
    {
      #if (IP_DBG_MQTT == DEF_ENABLED)
        DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Publishing failed");
      #endif
        return false;
    }

    if(QoS == MQTT_QOS_0)
    {
        m_LastActivityTick = GetTick();
        return true;
    }

    m_State            = MQTT_STATE_PUBLISHING;
    m_LastActivityTick = GetTick();

  #if (IP_DBG_MQTT == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Published");
  #endif

    return true;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Disconnect
//
//  Parameter(s):   None
//
//  Return:         true  - Disconnect sequence initiated and socket closed.
//                  false - (Reserved for future error handling)
//
//  Description:    Performs an application-requested MQTT disconnect. If the client is in the
//                  CONNECTED state, a DISCONNECT control packet is sent to the broker before
//                  tearing down the TCP connection.
//
//                  Regardless of the current protocol state, the underlying TCP socket is
//                  closed and released. The internal state machine is reset to
//                  MQTT_STATE_IDLE, and the application is notified via OnEvent() so it can
//                  react (e.g., attempt a reconnect).
//
//                  This function is non-blocking and does not wait for any broker response.
//                  All cleanup is performed locally, and the MQTT client task will remain
//                  idle until a new Connect() request is issued.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::Disconnect(void)
{
    if(m_State == MQTT_STATE_CONNECTED)
    {
        m_UserRequestedDisconnect = true;

        if(SendDisconnect() == false)
        {
            return false;
        };
    }

    if(m_pSocket != nullptr)
    {
        m_pSocket->Close();
        m_pSocket = nullptr;
        m_SocketValid = false;
    }

  #if (IP_DBG_MQTT == DEF_ENABLED)
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT: Disconnected");
  #endif

    m_State = MQTT_STATE_IDLE;
    OnEvent();
    return true;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           TCP_Connect
//
//  Parameter(s):   const IP_Address_t* pServerIP       Pointer to the broker IP address.
//                  IP_Port_t           Port            TCP port to connect to (typically 1883).
//
//  Return:         TCP_Socket*         Pointer to the allocated TCP socket on success.
//                                      nullptr if the connection attempt could not be started.
//
//  Description:    Internal helper used by the MQTT client to initiate a TCP connection to
//                  the MQTT broker. This function performs no protocol-level work; it simply
//                  delegates the connection request to the NanoIP TCP manager.
//
//                  If the server IP or network context is invalid, the MQTT state machine is
//                  reset to MQTT_STATE_IDLE and no connection is attempted.
//
//                  On success, a TCP socket object is returned and stored internally. The
//                  MQTT client task (Run()) will monitor the socket state and continue the
//                  MQTT CONNECT handshake once the TCP connection reaches ESTABLISHED.
//
//---------------------------------------------------------------------------------------------
TCP_Socket* MQTT_Client::TCP_Connect(const IP_Address_t* pServerIP, IP_Port_t Port)
{
    if((pServerIP == nullptr) || (m_pContext == nullptr))
    {
        m_State = MQTT_STATE_IDLE;
        return nullptr;
    }

    TCP_Manager* pTCP = m_pContext->GetTCP();

    m_pSocket = pTCP->Connect(*pServerIP, Port);

    if(m_pSocket == nullptr)
    {
        m_State = MQTT_STATE_IDLE;
    }
    else
    {
        m_SocketValid = (m_pSocket != nullptr);
    }

    return m_pSocket;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SendConnectFrame
//
//  Parameter(s):   const char* pClientID   Null-terminated MQTT client identifier.
//
//  Return:         true        - CONNECT frame encoded and sent successfully.
//                  false       - Invalid parameters, buffer allocation failure, or TX error.
//
//  Description:    Builds and sends an MQTT CONNECT control packet. This function allocates
//                  a temporary buffer, encodes the fixed header, variable header, and payload
//                  according to the MQTT 3.1.1 specification, and transmits the resulting
//                  frame over the active TCP socket.
//
//                  The CONNECT packet includes:
//                      - Protocol name and version ("MQTT", level 4)
//                      - Connect flags (clean session enabled)
//                      - Keep-alive interval
//                      - Client identifier (length-prefixed)
//
//                  No blocking operations are performed. After transmission, the MQTT client
//                  task (Run()) transitions to MQTT_STATE_WAIT_CONNACK and waits
//                  asynchronously for the broker's CONNACK response.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::SendConnectFrame(const char* pClientID)
{
    if((m_pSocket == nullptr) || (pClientID == nullptr))
    {
        return false;
    }

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(MQTT_RX_BUFFER_SIZE, MEM_DBG_MQTT1);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t Index = 0;
    size_t clientID_Len    = strlen(pClientID);
    size_t RemainingLength = MQTT_CONNECT_REMAINING_SIZE + clientID_Len;
    size_t HeaderLen = EncodeFixedHeader(pBuffer, MQTT_PACKET_TYPE_CONNECT, 0x00, RemainingLength);
    Index = HeaderLen;

    if((Index + RemainingLength) > MQTT_RX_BUFFER_SIZE)
    {
        pMemoryPool->Free((void**)&pBuffer);
        return false;
    }

    memcpy(&pBuffer[Index], (void*)"\0\x04MQTT\x04\x02", MQTT_PROTOCOL_HEADER_SIZE);
    Index += 8;
    pBuffer[Index++] = (m_KeepAliveSeconds >> 8) & 0xFF;
    pBuffer[Index++] = (m_KeepAliveSeconds     ) & 0xFF;
    pBuffer[Index++] = (clientID_Len >> 8) & 0xFF;
    pBuffer[Index++] = (clientID_Len     ) & 0xFF;
    memcpy(&pBuffer[Index], pClientID, clientID_Len);
    Index += clientID_Len;
    bool Result = SendFrame(pBuffer, Index);

    if(Result == false)
    {
        return false;
    }

    m_LastActivityTick = GetTick();
    return true;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SendSubscribeFrame
//
//  Parameter(s):   const char*   pTopic            Null-terminated topic filter string.
//                  MQTT_QoS_e    QoS               Requested QoS level for the subscription.
//
//  Return:         true        - SUBSCRIBE frame encoded and sent successfully.
//                  false       - Invalid socket, buffer allocation failure, or TX error.
//
//  Description:    Builds and transmits an MQTT SUBSCRIBE control packet. This function
//                  allocates a temporary buffer, encodes the fixed header, variable header
//                  (packet identifier), and payload (topic filter + QoS), then sends the
//                  resulting frame over the active TCP socket.
//
//                  Wildcards ('+' and '#') are allowed. Topic matching is performed entirely
//                  by the MQTT broker; the client simply sends the subscription request.
//
//                  This function performs no blocking operations. After transmission, the
//                  MQTT client task (Run()) transitions to MQTT_STATE_WAIT_SUBACK and waits
//                  asynchronously for the broker's SUBACK response.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::SendSubscribeFrame(const char* pTopic, MQTT_QoS_e QoS)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(MQTT_RX_BUFFER_SIZE, MEM_DBG_MQTT2);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t TopicLen = strlen(pTopic);
    size_t RemainingLength = MQTT_SUBSCRIBE_REMAINING_BASE_SIZE + TopicLen;
    size_t HeaderLen = EncodeFixedHeader(pBuffer, MQTT_PACKET_TYPE_SUBSCRIBE, MQTT_FLAG_SUBSCRIBE, RemainingLength);
    size_t Index = HeaderLen;

    if(Index + RemainingLength > MQTT_RX_BUFFER_SIZE)
    {
        pMemoryPool->Free((void**)&pBuffer);
        return false;
    }

    uint16_t PacketID = NextPacketID();
    pBuffer[Index++] = (PacketID >> 8) & 0xFF;
    pBuffer[Index++] = (PacketID     ) & 0xFF;
    pBuffer[Index++] = (TopicLen >> 8) & 0xFF;
    pBuffer[Index++] = (TopicLen     ) & 0xFF;
    memcpy(&pBuffer[Index], pTopic, TopicLen);
    Index += TopicLen;
    pBuffer[Index++] = (uint8_t)QoS;
    return SendFrame(pBuffer, Index);
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SendUnsubscribeFrame
//
//  Parameter(s):   const char* pTopic      Null-terminated topic filter string to unsubscribe.
//
//  Return:         true        - UNSUBSCRIBE frame encoded and sent successfully.
//                  false       - Invalid socket, buffer allocation failure, or TX error.
//
//  Description:    Builds and transmits an MQTT UNSUBSCRIBE control packet. This function
//                  allocates a temporary buffer, encodes the fixed header, variable header
//                  (packet identifier), and payload (topic filter), then sends the resulting
//                  frame over the active TCP socket.
//
//                  Wildcards ('+' and '#') are supported exactly as in SUBSCRIBE. Topic
//                  removal and matching logic are handled entirely by the MQTT broker; the
//                  client simply issues the request.
//
//                  This function performs no blocking operations. After transmission, the
//                  MQTT client task (Run()) transitions to MQTT_STATE_WAIT_UNSUBACK and waits
//                  asynchronously for the broker's UNSUBACK response.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::SendUnsubscribeFrame(const char* pTopic)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(MQTT_RX_BUFFER_SIZE, MEM_DBG_MQTT3);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t TopicLen = strlen(pTopic);
    size_t RemainingLength = MQTT_UNSUBSCRIBE_REMAINING_BASE_SIZE + TopicLen;
    size_t HeaderLen = EncodeFixedHeader(pBuffer, MQTT_PACKET_TYPE_UNSUBSCRIBE,  MQTT_FLAG_UNSUBSCRIBE, RemainingLength);
    size_t Index = HeaderLen;

    if(Index + RemainingLength > MQTT_RX_BUFFER_SIZE)
    {
        pMemoryPool->Free((void**)&pBuffer);
        return false;
    }

    uint16_t PacketID = NextPacketID();
    pBuffer[Index++] = (PacketID >> 8) & 0xFF;
    pBuffer[Index++] = (PacketID     ) & 0xFF;
    pBuffer[Index++] = (TopicLen >> 8) & 0xFF;
    pBuffer[Index++] = (TopicLen     ) & 0xFF;
    memcpy(&pBuffer[Index], pTopic, TopicLen);
    Index += TopicLen;
    return SendFrame(pBuffer, Index);
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SendPublishFrame
//
//  Parameter(s):   const char*    pTopic       Null-terminated topic string.
//                  const uint8_t* pPayload     Pointer to the payload buffer.
//                  size_t         Length       Payload length in bytes.
//                  MQTT_QoS_e     QoS          Requested QoS level (0 or 1).
//
//  Return:         true  - PUBLISH frame encoded and sent successfully.
//                  false - Invalid socket, buffer allocation failure, or TX error.
//
//  Description:    Builds and transmits an MQTT PUBLISH control packet. This function
//                  allocates a temporary buffer, encodes the fixed header, topic name,
//                  optional packet identifier (for QoS > 0), and payload, then sends the
//                  resulting frame over the active TCP socket.
//
//                  For QoS 0, the packet contains only the topic and payload. For QoS 1,
//                  a packet identifier is included and the MQTT client task (Run()) will
//                  wait asynchronously for the corresponding PUBACK.
//
//                  This function performs no blocking operations and does not modify the
//                  MQTT state machine directly. State transitions (e.g., to
//                  MQTT_STATE_PUBLISHING) are handled by the higher-level Publish() API.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::SendPublishFrame(const char* pTopic, const uint8_t* pPayload, size_t Length, MQTT_QoS_e QoS)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(MQTT_RX_BUFFER_SIZE, MEM_DBG_MQTT4);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t TopicLen = strlen(pTopic);
    size_t RemainingLength = MQTT_PUBLISH_REMAINING_BASE_SIZE + TopicLen + Length;

    if(QoS > MQTT_QOS_0)
    {
        RemainingLength += MQTT_PUBLISH_REMAINING_QOS1_SIZE;
    }

    uint8_t Flags = (QoS << 1);
    size_t HeaderLength = EncodeFixedHeader(pBuffer, MQTT_PACKET_TYPE_PUBLISH, Flags, RemainingLength);

    if(HeaderLength + RemainingLength > MQTT_RX_BUFFER_SIZE)
    {
        pMemoryPool->Free((void**)&pBuffer);
        return false;
    }

    size_t Index = HeaderLength;
    pBuffer[Index++] = (TopicLen >> 8) & 0xFF;
    pBuffer[Index++] = (TopicLen     ) & 0xFF;
    memcpy(&pBuffer[Index], pTopic, TopicLen);
    Index += TopicLen;

    if(QoS > MQTT_QOS_0)
    {
        uint16_t PacketID = NextPacketID();
        pBuffer[Index++] = (PacketID >> 8) & 0xFF;
        pBuffer[Index++] = (PacketID     ) & 0xFF;
    }

    memcpy(&pBuffer[Index], pPayload, Length);
    Index += Length;
    return SendFrame(pBuffer, Index);
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SendDisconnect
//
//  Parameter(s):   None
//
//  Return:         true        - DISCONNECT frame sent successfully.
//                  false       - Invalid socket or TX error.
//
//  Description:    Encodes and transmits an MQTT DISCONNECT control packet. The DISCONNECT
//                  frame contains only a fixed header with a zero remaining length, as
//                  defined by the MQTT 3.1.1 specification.
//
//                  This function performs no blocking operations and does not modify the
//                  MQTT state machine. Higher-level logic (Disconnect()) is responsible for
//                  closing the TCP socket and transitioning the client to MQTT_STATE_IDLE.
//
//                  The broker does not send any acknowledgment for DISCONNECT; the packet is
//                  simply transmitted and the connection is closed locally.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::SendDisconnect(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t Buffer[4];
    size_t HeaderLength = EncodeFixedHeader(Buffer, MQTT_PACKET_TYPE_DISCONNECT, 0x00, 0);
    size_t Sent = m_pSocket->Send(Buffer, HeaderLength);

    if(Sent != HeaderLength)
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SendPingRequest
//
//  Parameter(s):   None
//
//  Return:         true        - PINGREQ frame sent successfully.
//                  false       - Invalid socket or TX error.
//
//  Description:    Encodes and transmits an MQTT PINGREQ control packet. The PINGREQ frame
//                  contains only a fixed header with a zero remaining length, as defined by
//                  the MQTT 3.1.1 specification.
//
//                  This function performs no blocking operations. After transmission, the
//                  MQTT client updates its activity timestamp so the keep-alive timer remains
//                  valid. The MQTT client task (Run()) will then wait asynchronously for the
//                  corresponding PINGRESP packet.
//
//                  If the broker does not respond within the configured timeout, the client
//                  will treat the connection as dead and transition back to MQTT_STATE_IDLE.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::SendPingRequest(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t Buffer[4];
    size_t HeaderLength = EncodeFixedHeader(Buffer, MQTT_PACKET_TYPE_PING_REQUEST, 0x00, 0);
    size_t Sent = m_pSocket->Send(Buffer, HeaderLength);

    if(Sent != HeaderLength)
    {
        return false;
    }

    m_LastActivityTick = GetTick();
    return true;
}

//---------------------------------------------------------------------------------------------
//  Name:           HandleIncomingData
//
//  Parameter(s):   None
//
//  Return:         true        - A complete MQTT packet was received and parsed successfully.
//                  false       - No data available, socket invalid, or parsing error.
//
//  Description:    Reads incoming TCP data and incrementally reconstructs MQTT packets.
//                  This function implements the MQTT 3.1.1 packet framing rules, including:
//
//                      - Accumulating bytes into an internal RX buffer.
//                      - Decoding the MQTT Remaining Length field (variable-length integer).
//                      - Determining when a full MQTT packet has been received.
//                      - Passing the completed packet to ParseIncomingPacket().
//
//                  The function may return multiple times while waiting for enough bytes
//                  to complete a packet. Once a full packet is assembled, it is parsed,
//                  the temporary buffer is released, and the internal RX state is reset
//                  to prepare for the next packet.
//
//                  This function performs no blocking operations and does not modify the
//                  MQTT state machine directly. All protocol-level transitions occur inside
//                  ParseIncomingPacket() and the MQTT client task (Run()).
//---------------------------------------------------------------------------------------------
bool MQTT_Client::HandleIncomingData(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    if(m_pRX_Packet == nullptr)             // Allocate RX buffer if needed
    {
        m_pRX_Packet = (uint8_t*)pMemoryPool->AllocAndClear(MQTT_RX_BUFFER_SIZE, MEM_DBG_MQTT5);

        if(m_pRX_Packet == nullptr)
        {
            return false;
        }

        m_RX_Index          = 0;
        m_BytesNeeded       = -1;           // -1 = decoding Remaining Length varint
        m_RemainingLength   = 0;
        m_RemainingLenBytes = 0;
        m_Multiplier        = 1;
    }

    // Read as much as possible from TCP
    size_t Received = m_pSocket->Receive(&m_pRX_Packet[m_RX_Index],  MQTT_RX_BUFFER_SIZE - m_RX_Index);

    if(Received == 0)
    {
        return false;                       // No data available
    }

    m_RX_Index += Received;

    if(m_RX_Index < 1)                      // Need at least 1 byte for fixed header
    {
        return true;
    }

    if(m_BytesNeeded == -1)                 // Decode remaining length (MQTT varint)
    {
        size_t i = 1;

        while(i < m_RX_Index)
        {
            uint8_t Byte = m_pRX_Packet[i];
            m_RemainingLength += (Byte & MQTT_REMAINING_LEN_MASK) * m_Multiplier;
            m_Multiplier *= MQTT_REMAINING_LEN_BASE_128;
            m_RemainingLenBytes++;

            if((Byte & MQTT_REMAINING_LEN_CONTINUATION) == 0)
            {
                m_BytesNeeded = m_RemainingLength;
                break;
            }

            i++;
        }

        if(m_BytesNeeded == -1)
        {
            return true;                    // Need more bytes
        }
    }

    // Check if full packet received
    size_t FixedHeaderSize = 1 + m_RemainingLenBytes;
    size_t TotalNeeded     = FixedHeaderSize + m_RemainingLength;

    if(m_RX_Index < TotalNeeded)
    {
        return true; // Need more data
    }

    // Parse packet
    bool ok = ParseIncomingPacket(m_pRX_Packet, TotalNeeded);

    // Reset for next packet
    pMemoryPool->Free((void**)&m_pRX_Packet);
    m_RX_Index          = 0;
    m_RemainingLenBytes = 0;
    m_BytesNeeded       = -1;
    m_RemainingLength   = 0;
    m_Multiplier        = 1;

    return ok;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           ParseIncomingPacket
//
//  Parameter(s):   uint8_t* pBuffer        Pointer to a fully assembled MQTT packet.
//                  size_t   Length         Total number of bytes in the packet.
//
//  Return:         true  - Packet parsed and processed successfully.
//                  false - Invalid packet, malformed fields, or unsupported type.
//
//  Description:    Decodes and processes a complete MQTT control packet. The function
//                  validates the fixed header, decodes the Remaining Length field, and
//                  dispatches handling based on the MQTT packet type:
//
//                      - CONNACK     -> Completes the CONNECT handshake.
//                      - PUBLISH     -> Extracts topic and payload, forwards to handler.
//                      - PUBACK      -> Completes QoS 1 publish sequence.
//                      - SUBACK      -> Completes subscription sequence.
//                      - UNSUBACK    -> Completes unsubscription sequence.
//                      - PINGRESP    -> Completes keep-alive exchange.
//
//                  Each handler updates the MQTT state machine as required and notifies the
//                  application via OnEvent() when a state transition or incoming message
//                  requires application-level processing.
//
//                  This function performs no blocking operations. It assumes the packet is
//                  already fully assembled by HandleIncomingData(), and it does not retain
//                  any pointers into the provided buffer after returning.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::ParseIncomingPacket(uint8_t* pBuffer, size_t Length)
{
    if((pBuffer == nullptr) || (Length < 2))
    {
        return false;
    }

    uint8_t                  Header     = pBuffer[0];
    MQTT_ControlPacketType_e PacketType = (MQTT_ControlPacketType_e)(pBuffer[0] >> 4);
    size_t RemainingLength      = 0;
    size_t RemainingLengthBytes = 0;

    if(MQTT_Client::DecodeRemainingLength(&pBuffer[1], Length - 1, &RemainingLength, &RemainingLengthBytes) == false)
    {
        return false;
    }

    size_t FixedHeaderSize = 1 + RemainingLengthBytes;

    if((FixedHeaderSize + RemainingLength) > Length)
    {
        return false;
    }

    size_t Index = FixedHeaderSize;

    switch(PacketType)
    {
        case MQTT_PACKET_TYPE_CONNACK:
        {
            if(RemainingLength < 2)
            {
                return false;
            }

            if((Index + 2) > Length)
            {
                return false;
            }

            uint8_t SessionPreSent = pBuffer[Index];
            VAR_UNUSED(SessionPreSent);
            uint8_t ReturnCode = pBuffer[Index + 1];

            if(ReturnCode == 0x00)
            {
                m_State            = MQTT_STATE_CONNECTED;
                m_LastActivityTick = GetTick();
                OnEvent();
                return true;
            }

            m_State = MQTT_STATE_IDLE;
            return false;
        }

        case MQTT_PACKET_TYPE_PUBLISH:
        {
            if(RemainingLength < 2)
            {
                return false;
            }

            if((Index + 2) > Length)
            {
                return false;
            }

            uint16_t TopicLength = (static_cast<uint16_t>(pBuffer[Index]) << 8) | static_cast<uint16_t>(pBuffer[Index + 1]);
            Index += 2;

            if(TopicLength == 0)
            {
                return false;
            }

            if((Index + TopicLength) > (FixedHeaderSize + RemainingLength))
            {
                return false;
            }

            char* pTopic = (char*)pMemoryPool->AllocAndClear(TopicLength + 1, MEM_DBG_MQTT6);

            if(pTopic == nullptr)
            {
                return false;
            }

            memcpy(pTopic, &pBuffer[Index], TopicLength);
            pTopic[TopicLength] = '\0';
            Index += TopicLength;

            uint8_t QoS = (Header >> 1) & 0x03;

            if(QoS > 0)
            {
                if((Index + 2) > (FixedHeaderSize + RemainingLength))
                {
                    pMemoryPool->Free((void**)&pTopic);
                    return false;
                }

                Index += 2;
            }

            if(Index > (FixedHeaderSize + RemainingLength))
            {
                pMemoryPool->Free((void**)&pTopic);
                return false;
            }

            size_t PayloadLen      = (FixedHeaderSize + RemainingLength) - Index;
            const uint8_t* pPayload = &pBuffer[Index];

            m_pHandler->ReceivedTopic(pTopic, pPayload, PayloadLen);
            OnEvent();
            pMemoryPool->Free((void**)&pTopic);
            return true;
        }

        case MQTT_PACKET_TYPE_PUBACK:
        {
            if(RemainingLength < 2)
            {
                return false;
            }

            if((Index + 2) > Length)
            {
                return false;
            }

            uint16_t packetID = (static_cast<uint16_t>(pBuffer[Index]) << 8) | static_cast<uint16_t>(pBuffer[Index + 1]);
            VAR_UNUSED(packetID);

            m_State            = MQTT_STATE_CONNECTED;
            m_LastActivityTick = GetTick();
            OnEvent();
            return true;
        }

        case MQTT_PACKET_TYPE_SUBACK:
        {
            if(RemainingLength < 3)
            {
                return false;
            }

            if((Index + 3) > Length)
            {
                return false;
            }

            uint16_t packetID = (static_cast<uint16_t>(pBuffer[Index]) << 8) | static_cast<uint16_t>(pBuffer[Index + 1]);
            VAR_UNUSED(packetID);

            m_State            = MQTT_STATE_CONNECTED;
            m_LastActivityTick = GetTick();
            OnEvent();
            return true;
        }

        case MQTT_PACKET_TYPE_UNSUBACK:
        {
            return true;
        }

        case MQTT_PACKET_TYPE_PING_RESPONSE:
        {
            if(RemainingLength != 0)
            {
                return false;
            }

            m_WaitingPingResponse = false;
            m_LastActivityTick = GetTick();
            return true;
        }

        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------
//
//  Name:           NextPacketID
//
//  Parameter(s):   None
//
//  Return:         uint16_t        Next non-zero MQTT packet identifier.
//
//  Description:    Generates the next MQTT packet identifier used for QoS 1 messages and
//                  subscription-related packets. The counter increments monotonically and
//                  wraps from 0xFFFF back to 1, skipping the value 0 since MQTT reserves
//                  packet ID 0 as invalid.
//
//                  This function performs no blocking operations and maintains a simple
//                  rolling sequence as required by the MQTT 3.1.1 specification.
//
//---------------------------------------------------------------------------------------------
uint16_t MQTT_Client::NextPacketID(void)
{
    m_NextPacketID++;

    if(m_NextPacketID == 0)
    {
        m_NextPacketID = 1;
    }

    return m_NextPacketID;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SendFrame
//
//  Parameter(s):   uint8_t* pBuffer    Pointer to an encoded MQTT frame allocated from
//                                      the memory pool.
//                  size_t   Length     Number of bytes to transmit.
//
//  Return:         true  - Entire frame sent successfully.
//                  false - TX error or partial send.
//
//  Description:    Sends a fully encoded MQTT frame over the active TCP socket. After the
//                  transmission attempt, the function releases the temporary buffer back to
//                  the memory pool regardless of success or failure.
//
//                  This function performs no blocking operations and does not modify the
//                  MQTT state machine. Higher-level logic is responsible for interpreting
//                  the result and updating protocol state accordingly.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::SendFrame(uint8_t* pBuffer, size_t Length)
{
    size_t Sent = m_pSocket->Send(pBuffer, Length);
    pMemoryPool->Free((void**)&pBuffer);
    return (Sent == Length);
}

//---------------------------------------------------------------------------------------------
//
//  Name:           DecodeRemainingLength
//
//  Parameter(s):   const uint8_t* pBuffer      Pointer to the first byte of the MQTT Remaining
//                                              Length field.
//                  size_t         Length       Number of bytes available starting at pBuffer.
//                  size_t*        pValue       Output: decoded Remaining Length value.
//                  size_t*        pBytesUsed   Output: number of bytes consumed by the
//                                              variable-length encoding.
//
//  Return:         true  - Remaining Length successfully decoded.
//                  false - Invalid encoding, insufficient bytes, or multiplier overflow.
//
//  Description:    Decodes the MQTT Remaining Length field, which uses a variable-length
//                  base-128 encoding. Each byte contributes 7 bits of value and one
//                  continuation bit. The function accumulates the decoded value, detects
//                  malformed encodings, enforces the 4-bytes maximum, and reports how many
//                  bytes were consumed.
//
//                  This helper is used by the MQTT packet parser to determine the boundary
//                  between the fixed header, variable header, and payload. The caller must
//                  ensure that pBuffer points to the first Remaining Length byte.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::DecodeRemainingLength(const uint8_t* pBuffer, size_t Length, size_t* pValue, size_t* pBytesUsed)
{
    if((pBuffer == nullptr) || (pValue == nullptr) || (pBytesUsed == nullptr))
    {
        return false;
    }

    uint32_t Value      = 0;
    uint32_t Multiplier = 1;
    size_t   Index      = 0;

    while(Index < Length)
    {
        uint8_t Encoded = pBuffer[Index];
        Value += (Encoded & MQTT_REMAINING_LEN_MASK) * Multiplier;
        Index++;

        if((Encoded & MQTT_REMAINING_LEN_CONTINUATION) == 0)
        {
            *pValue     = static_cast<size_t>(Value);
            *pBytesUsed = Index;
            return true;
        }

        Multiplier *= MQTT_REMAINING_LEN_BASE_128;

        if(Multiplier > MQTT_REMAINING_LEN_MAX_MULTIPLIER)
        {
            return false;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           EncodeFixedHeader
//
//  Parameter(s):   uint8_t* pOut               Output buffer where the fixed header will be
//                                              written.
//                  uint8_t  PacketType         MQTT control packet type
//                                              (upper 4 bits of byte 1).
//                  uint8_t  Flags              MQTT flags (lower 4 bits of byte 1).
//                  size_t   RemainingLength    Remaining Length field to encode
//                                              (variable-length integer).
//
//  Return:         size_t   Total number of bytes written to pOut for the fixed header
//                           (1 byte for type/flags + 1-4 bytes for Remaining Length).
//
//  Description:    Encodes the MQTT fixed header for any control packet. The first byte
//                  contains the packet type and flags. The Remaining Length field is encoded
//                  using MQTT's variable-length base-128 scheme, where each byte contributes
//                  7 bits of value and one continuation bit.
//
//                  The function writes the encoded header into pOut and returns the number
//                  of bytes produced. It performs no validation of RemainingLength beyond
//                  the natural limits of the encoding loop.
//
//---------------------------------------------------------------------------------------------
size_t MQTT_Client::EncodeFixedHeader(uint8_t* pOut, uint8_t PacketType, uint8_t Flags, size_t RemainingLength)
{
    pOut[0] = (PacketType << 4) | Flags;

    size_t Count = 0;

    do
    {
        uint8_t Byte = RemainingLength % MQTT_REMAINING_LEN_BASE_128;
        RemainingLength = RemainingLength / MQTT_REMAINING_LEN_BASE_128;

        if(RemainingLength > 0)
        {
            Byte = Byte | MQTT_REMAINING_LEN_CONTINUATION;
        }

        pOut[1 + Count] = Byte;
        Count++;
    }
    while(RemainingLength > 0);

    return Count + 1;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           OnSocketEvent
//
//  Parameter(s):   TCP_Socket*   pSocket Pointer to the socket that generated the event.
//                  SocketEvent_e Event   Type of socket event (RX ready, closed, error, etc.).
//
//  Return:         None
//
//  Description:    Callback invoked by the TCP layer whenever the underlying socket generates
//                  an event. The MQTT client reacts as follows:
//
//                      - SOCKET_EVENT_RX_READY
//                          Signals the MQTT task that new data is available by releasing
//                          the RX semaphore. The task will call HandleIncomingData().
//
//                      - SOCKET_EVENT_ERROR / SOCKET_EVENT_CLOSED
//                          Marks the socket as invalid and notifies the application via
//                          OnEvent(). If the disconnect was not user-initiated, the MQTT
//                          state machine is forced back to MQTT_STATE_IDLE so the
//                          application may choose to reconnect. For user-requested
//                          disconnects, no reconnection is attempted.
//
//                  This function performs no blocking operations and does not directly
//                  process MQTT packets. It only updates internal state and signals the
//                  MQTT client task.
//
//---------------------------------------------------------------------------------------------
void MQTT_Client::OnSocketEvent(TCP_Socket* pSocket, SocketEvent_e Event)
{
    switch(Event)
    {
        case SOCKET_EVENT_RX_READY:
        {
            nOS_SemGive(&m_RX_ReadySem);
        }
        break;

        case SOCKET_EVENT_ERROR:
        case SOCKET_EVENT_CLOSED:
        {
            m_SocketValid = false;                              // Mark the socket as invalid

            // If the disconnect was not requested by the user,
            // force the MQTT state machine into reconnecting mode.
            if(m_UserRequestedDisconnect  == false)
            {
                m_State = MQTT_STATE_IDLE;
            }
            else
            {
                // User-initiated disconnect: no reconnection
                m_UserRequestedDisconnect = false;
            }

            OnEvent();
        }
        break;

        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------
//
//  Name:           OnEvent
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Notifies the application-level MQTT handler that an internal state change
//                  or incoming message requires processing. This function simply forwards the
//                  event to the user-provided handler, if one is registered.
//
//                  All protocol logic and state transitions are handled elsewhere; this
//                  callback provides a clean separation between the MQTT client library and
//                  the application layer.
//
//---------------------------------------------------------------------------------------------
void MQTT_Client::OnEvent()
{
    if(m_pHandler != nullptr)
    {
        m_pHandler->OnEvent();
    }
}

//---------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)
