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
//         - Keep‑alive timer is not reset when receiving PUBLISH packets.
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

#if (IP_USE_MQTT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MQTT_CONNECT_TIMEOUT_MS             10000     // 10 seconds
#define MQTT_PINGRESP_TIMEOUT_MS            5000      // 5 seconds
#define MQTT_DISCONNECTED_PERIOD_MS         500


#define MQTT_REMAINING_LEN_MASK             0x7F
#define MQTT_REMAINING_LEN_CONTINUATION     0x80
#define MQTT_REMAINING_LEN_BASE_128         128
#define MQTT_REMAINING_LEN_MAX_MULTIPLIER   (MQTT_REMAINING_LEN_BASE_128 * MQTT_REMAINING_LEN_BASE_128 * MQTT_REMAINING_LEN_BASE_128)

//-------------------------------------------------------------------------------------------------
// Define(s)
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
    MQTT_PACKET_TYPE_PINGREQ       = 12,
    MQTT_PACKET_TYPE_PINGRESP      = 13,
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
//  Parameter(s):   NetworkContext*        pContext       Pointer to the NanoIP network context
//
//  Return:         bool - true  : Initialization successful.
//                       - false : Initialization failed (reserved for future error handling).
//
//  Description:    Initializes the internal state of the MQTT client. This function stores
//                  the network context, registers the application event callback, resets all
//                  MQTT state variables, clears runtime counters, and prepares the client for
//                  operation. No TCP socket is created at this stage; the socket will be
//                  allocated later when Connect() is called.
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
    m_WaitingPingResp         = false;

    m_ClientID[0]             = '\0';
    m_LastServerIP            = IP_ADDRESS(0,0,0,0);
    m_LastServerPort          = 0;
    m_pRX_Packet              = nullptr;

    nOS_SemCreate(&m_RX_ReadySem, 0, 1);

    // Create task
    /*Error =*/ nOS_ThreadCreate(&m_Handle,
                             TaskMQTT_ClientWrapper,
                             this,
                             &m_Stack[0],
                             TASK_MQTT_CLIENT_STACK_SIZE,
                             TASK_MQTT_CLIENT_PRIO,
                             "MQTT Test");

    return true;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Run
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:
//
//---------------------------------------------------------------------------------------------
void MQTT_Client::Run(void)
{
    while(1)
    {
        TickCount_t Now = GetTick();

        if(nOS_SemTake(&m_RX_ReadySem, NOS_NO_WAIT) == NOS_OK)
        {
            // Process incoming MQTT packets
            if(HandleIncomingData() == true)
            {
                OnEvent();   // wake application
            }
        }

        switch(m_State)
        {
            case MQTT_STATE_CONNECTING:
            {
                // Wait for TCP connection to be established
                if((m_pSocket != nullptr) && (m_pSocket->GetState() == TCP_STATE_ESTABLISHED))
                {
                    SendConnectFrame(m_ClientID);
                    m_State            = MQTT_STATE_WAIT_CONNACK;
                    m_LastActivityTick = Now;
                }
                else if((Now - m_ConnectStartTick) > MQTT_CONNECT_TIMEOUT_MS)
                {
                    // TCP connection timeout → return to IDLE
                    m_State = MQTT_STATE_IDLE;
                    OnEvent();   // wake application so it can retry
                }
            }
            break;

            case MQTT_STATE_CONNECTED:
            {
                // Check for PINGRESP timeout
                if(m_WaitingPingResp && ((Now - m_PingSentTick) > MQTT_PINGRESP_TIMEOUT_MS))
                {
                    SendDisconnect();
                    m_State = MQTT_STATE_IDLE;
                    OnEvent();
                }

                // Keep-alive timer
                if((Now - m_LastActivityTick) >
                   (m_KeepAliveSeconds * TICKS_PER_SECOND))
                {
                    if(SendPingReq())
                    {
                        m_WaitingPingResp = true;
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

        // Always sleep to avoid CPU starvation
        nOS_Sleep(50);
    }
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Connect
//
//  Parameter(s):   const IP_Address_t* pServerIP   MQTT broker IP address (network order).
//                  uint16_t Port                   MQTT broker TCP port (typically 1883).
//                  const char* pClientId           Null-terminated client identifier string.
//                  uint16_t KeepAliveSeconds       MQTT keep-alive interval in seconds.
//
//  Return:         bool            - true  : Connection sequence started.
//                                  - false : Socket not ready or internal error.
//
//  Description:    Starts a non-blocking MQTT CONNECT sequence. The caller must periodically
//                  call Process() until the state reaches MQTT_STATE_CONNECTED or ERROR.
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
            // Socket mort -> on le détruit
            m_pSocket->Close();
            m_pSocket = nullptr;
        }
        else
        {
            // Socket encore vivant → on ne reconnecte pas
            return true;
        }
    }

    // Save it for reconnect
    m_LastServerIP   = *pServerIP;
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
//  Parameter(s):   const char* pTopic      Null-terminated topic filter string.
//                  MQTT_QoS_e QoS          Requested QoS level.
//
//  Return:         bool                    - true  : SUBSCRIBE frame queued/Sent.
//                                          - false : Client not connected or internal error.
//
//  Description:    Starts a non-blocking SUBSCRIBE sequence. The caller must periodically
//                  call Process() until the state returns to MQTT_STATE_CONNECTED or ERROR.
//
//  Note(s):        Fixed Header:
//                    - byte 1: 0x82  (SUBSCRIBE + flags)
//                    - byte 2+: Remaining Length (varint)
//
//                  Variable Header:
//                    - Packet Identifier MSB
//                    - Packet Identifier LSB
//
//                  Payload:
//                    - Topic Length MSB
//                    - Topic Length LSB
//                    - Topic bytes...
//                    - QoS (1 byte)
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
//  Parameter(s):   const char* pTopic          Null-terminated topic string.
//                  const uint8_t* pPayload     Pointer to payload buffer.
//                  size_t Length               Payload length in bytes.
//                  MQTT_QoS_e QoS              QoS level (0 or 1 typically).
//
//  Return:         bool                    - true  : PUBLISH frame queued/Sent.
//                                          - false : Client not connected or internal error.
//
//  Description:    Sends a PUBLISH message in a non-blocking manner. For QoS 0, completion
//                  is immediate. For QoS 1, the client will wait for PUBACK in Process().
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

bool MQTT_Client::Disconnect(void)
{
    if(m_State == MQTT_STATE_CONNECTED)
    {
        m_UserRequestedDisconnect = true;
        SendDisconnect();       // TODO we don't check the true or false
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

    size_t clientID_Len = strlen(pClientID);

    size_t RemainingLength = 10 + 2 + clientID_Len;

    size_t HeaderLen = EncodeFixedHeader(pBuffer,
                                         MQTT_PACKET_TYPE_CONNECT,
                                         0x00,
                                         RemainingLength);

    Index = HeaderLen;

if((Index + RemainingLength) > MQTT_RX_BUFFER_SIZE)
{
    pMemoryPool->Free((void**)&pBuffer);
    return false;
}

    memcpy(&pBuffer[Index], (void*)"\0\x04MQTT\x04\x02", 8);
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

    size_t RemainingLength = 2 + 2 + TopicLen + 1;

    size_t HeaderLen = EncodeFixedHeader(pBuffer,
                                         MQTT_PACKET_TYPE_SUBSCRIBE,
                                         MQTT_FLAG_SUBSCRIBE,
                                         RemainingLength);

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

    size_t RemainingLength = 2 + 2 + TopicLen;

    size_t HeaderLen = EncodeFixedHeader(pBuffer,
                                         MQTT_PACKET_TYPE_UNSUBSCRIBE,
                                         MQTT_FLAG_UNSUBSCRIBE,
                                         RemainingLength);

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

/*
Fixed Header:
  byte 1: 0x30 | (QoS << 1)
  byte 2+: Remaining Length (varint)

Variable Header:
  Topic length MSB
  Topic length LSB
  Topic bytes...
  [Packet ID MSB]   (only for QoS 1 or 2)
  [Packet ID LSB]

Payload:
  Raw payload bytes
*/

bool MQTT_Client::SendPublishFrame(const char* pTopic,
                                   const uint8_t* pPayload,
                                   size_t Length,
                                   MQTT_QoS_e QoS)
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

    size_t RemainingLength = 2 + TopicLen + Length;

    if(QoS > MQTT_QOS_0)
    {
        RemainingLength += 2;
    }

    uint8_t Flags = (QoS << 1);

    size_t HeaderLength = EncodeFixedHeader(pBuffer,
                                         MQTT_PACKET_TYPE_PUBLISH,
                                         Flags,
                                         RemainingLength);

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

bool MQTT_Client::SendPingReq(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t Buffer[4];
    size_t HeaderLength = EncodeFixedHeader(Buffer, MQTT_PACKET_TYPE_PINGREQ, 0x00, 0);
    size_t Sent = m_pSocket->Send(Buffer, HeaderLength);

    if(Sent != HeaderLength)
    {
        return false;
    }

    m_LastActivityTick = GetTick();
    return true;
}

//---------------------------------------------------------------------------------------------

bool MQTT_Client::HandleIncomingData(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    // Allocate RX buffer if needed
    if(m_pRX_Packet == nullptr)
    {
        m_pRX_Packet = (uint8_t*)pMemoryPool->AllocAndClear(MQTT_RX_BUFFER_SIZE, MEM_DBG_MQTT5);

        if(m_pRX_Packet == nullptr)
        {
            return false;
        }

        m_RX_Index          = 0;
        m_BytesNeeded       = -1;   // -1 = decoding Remaining Length varint
        m_RemainingLength   = 0;
        m_RemainingLenBytes = 0;
        m_Multiplier        = 1;
    }

    // ----------------------------------------------------
    // Read as much as possible from TCP
    // ----------------------------------------------------
    size_t Received = m_pSocket->Receive(&m_pRX_Packet[m_RX_Index],
                                         MQTT_RX_BUFFER_SIZE - m_RX_Index);

    if(Received == 0)
    {
        return false;   // No data available
    }

    m_RX_Index += Received;

    // ----------------------------------------------------
    // 1) Need at least 1 byte for fixed header
    // ----------------------------------------------------
    if(m_RX_Index < 1)
    {
        return true;
    }

    // ----------------------------------------------------
    // 2) Decode Remaining Length (MQTT varint)
    // ----------------------------------------------------
    if(m_BytesNeeded == -1)
    {
        size_t i = 1;

        while(i < m_RX_Index)
        {
            uint8_t Byte = m_pRX_Packet[i];
            m_RemainingLength += (Byte & 0x7F) * m_Multiplier;
            m_Multiplier *= 128;
            m_RemainingLenBytes++;

            if((Byte & 0x80) == 0)
            {
                m_BytesNeeded = m_RemainingLength;
                break;
            }

            i++;
        }

        if(m_BytesNeeded == -1)
        {
            return true; // Need more bytes
        }
    }

    // ----------------------------------------------------
    // 3) Check if full packet received
    // ----------------------------------------------------
    size_t FixedHeaderSize = 1 + m_RemainingLenBytes;
    size_t TotalNeeded     = FixedHeaderSize + m_RemainingLength;

    if(m_RX_Index < TotalNeeded)
    {
        return true; // Need more data
    }

    // ----------------------------------------------------
    // 4) Parse packet
    // ----------------------------------------------------
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

        case MQTT_PACKET_TYPE_PINGRESP:
        {
            if(RemainingLength != 0)
            {
                return false;
            }

            m_WaitingPingResp = false;
            m_LastActivityTick = GetTick();
            return true;
        }

        default:
            return false;
    }
}

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

bool MQTT_Client::SendFrame(uint8_t* pBuffer, size_t Length)
{
    size_t Sent = m_pSocket->Send(pBuffer, Length);
    pMemoryPool->Free((void**)&pBuffer);
    return (Sent == Length);
}

//-------------------------------------------------------------------------------------------------
//  Name:           DecodeRemainingLength
//
//  Parameter(s):   const uint8_t* pBuffer
//                      Pointer to the start of the MQTT Remaining Length field.
//
//                  size_t Length
//                      Number of bytes available in the buffer.
//
//                  size_t* pValue
//                      Output pointer. On success, receives the decoded Remaining Length value.
//
//                  size_t* pBytesUsed
//                      Output pointer. On success, receives the number of bytes consumed by the
//                      variable-length encoding (1 to 4 bytes).
//
//  Return:         bool
//                      true  - Remaining Length successfully decoded.
//                      false - Invalid encoding, insufficient bytes, or multiplier overflow.
//
//  Description:    Decodes the MQTT "Remaining Length" field, which uses a variable-length
//                  base-128 encoding. Each byte contributes 7 bits of payload and one continuation
//                  bit. The function accumulates the decoded value, detects malformed encodings,
//                  and reports how many bytes were consumed.
//
//                  This helper is required for parsing all MQTT control packets, as the Remaining
//                  Length determines the boundary of the variable header and payload.
//
//  Notes:          - Valid encodings use 1 to 4 bytes.
//                  - The function validates multiplier overflow as required by MQTT 3.1.1.
//                  - The caller must ensure pBuffer points to the first Remaining Length byte.
//
//-------------------------------------------------------------------------------------------------
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

void MQTT_Client::OnEvent()
{
    if(m_pHandler != nullptr)
    {
        m_pHandler->OnEvent();
    }
}

//---------------------------------------------------------------------------------------------

#endif // (IP_USE_MQTT == DEF_ENABLED)
