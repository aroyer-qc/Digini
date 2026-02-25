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

#define MQTT_CONNECT_TIMEOUT_MS   10000     // 10 seconds
#define MQTT_PINGRESP_TIMEOUT_MS  5000      // 5 seconds


//---------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext* pContext        Pointer to NanoIP network context.
//
//  Return:         bool        - true  : Initialization successful.
//                              - false : Socket allocation or configuration failed.
//
//  Description:    Initializes the MQTT client and allocates an underlying TCP socket.
//
//---------------------------------------------------------------------------------------------
bool MQTT_Client::Initialize(NetworkContext& Context)
{
    m_pContext              = &Context;
    m_pSocket               = nullptr;
    m_State                 = MQTT_STATE_IDLE;

    m_KeepAliveSeconds      = 0;
    m_LastActivityTick      = 0;
    m_ConnectStartTick      = 0;
    m_PingSentTick          = 0;

    m_ReconnectStartTick    = 0;
    m_ReconnectDelaySeconds = 5;
    m_ReconnectEnabled      = true;

    m_NextPacketID          = 1;
    m_WaitingPingResp       = false;

    m_MessageCallback       = nullptr;
    m_pMessageContext       = nullptr;

    m_ClientID[0]           = '\0';
    m_LastServerIP          = IP_ADDRESS(0,0,0,0);
    m_LastServerPort        = 0;

    return true;

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

    // save it for reconnect
    m_LastServerIP   = *pServerIP;
    m_LastServerPort = Port;

    strncpy(m_ClientID, pClientID, sizeof(m_ClientID) - 1);
    m_ClientID[sizeof(m_ClientID) - 1] = '\0';

    m_KeepAliveSeconds = KeepAliveSeconds;

    if(TCP_Connect(pServerIP, Port) == false)
    {
        m_State = MQTT_STATE_ERROR;
        return false;
    }

    m_State            = MQTT_STATE_CONNECTING;
    m_ConnectStartTick = GetTick();
    m_LastActivityTick = m_ConnectStartTick;
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
//  Note(s):        Fixed header:
//                    - byte 1: 0x82  (SUBSCRIBE + flags)
//                    - byte 2+: Remaining Length (varint)
//
//                  Variable header:
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
        return false;
    }

    m_State = MQTT_STATE_WAIT_SUBACK;
    m_LastActivityTick = GetTick();
    return true;
}

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

    if(!SendPublishFrame(pTopic, pPayload, Length, QoS))
    {
        return false;
    }

    // For QoS 0, we stay connected
    if(QoS == MQTT_QOS_0)
    {
        m_LastActivityTick = GetTick();
        return true;
    }

    // For QoS 1 or 2, wait for PUBACK or PUBREC
    m_State = MQTT_STATE_PUBLISHING;
    m_LastActivityTick = GetTick();
    return true;
}

//---------------------------------------------------------------------------------------------

bool MQTT_Client::Disconnect(void)
{
    if(m_State == MQTT_STATE_CONNECTED)
    {
        SendDisconnect();
    }

    if(m_pSocket)
    {
        m_pSocket->Close();
        m_pSocket = nullptr;
    }

    m_State = MQTT_STATE_IDLE;
    return true;
}

//---------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   None
//
//  Return:         void
//
//  Description:    Drives the internal MQTT state machine. This function must be called
//                  periodically from the main loop or a scheduler task. It handles:
//                      - TCP connect progress
//                      - CONNECT / CONNACK handshake
//                      - SUBSCRIBE / SUBACK handshake
//                      - PUBLISH acknowledgments (QoS 1)
//                      - Incoming PUBLISH messages
//                      - Keep-alive PINGREQ / PINGRESP
//
//---------------------------------------------------------------------------------------------
void MQTT_Client::Process(void)
{
    TickCount_t Now = GetTick();

    switch(m_State)
    {
        case MQTT_STATE_CONNECTING:
        {
            if((m_pSocket != nullptr) && (m_pSocket->GetState() == TCP_STATE_ESTABLISHED))
            {
                SendConnectFrame(m_ClientID);
                m_State           = MQTT_STATE_WAIT_CONNACK;
                m_LastActivityTick= Now;
            }
            else if((Now - m_ConnectStartTick) > MQTT_CONNECT_TIMEOUT_MS)               //  use macro HasTimeOut
            {
                m_State              = MQTT_STATE_RECONNECTING;
                m_ReconnectStartTick = Now;
            }
        }
        break;

        case MQTT_STATE_CONNECTED:
        {
            HandleIncomingData();

            if((m_WaitingPingResp == true) && ((Now - m_PingSentTick) > MQTT_PINGRESP_TIMEOUT_MS))
            {
                // We WERE connected: send clean DISCONNECT
                SendDisconnect();
                m_State = MQTT_STATE_RECONNECTING;
                m_ReconnectStartTick = Now;
            }

            if((Now - m_LastActivityTick) > (m_KeepAliveSeconds * TICKS_PER_SECOND))
            {
                if(SendPingReq() == true)
                {
                    m_WaitingPingResp = true;
                    m_PingSentTick    = Now;
                }
            }
        }
        break;

        case MQTT_STATE_RECONNECTING:
        {
            // Wait for delay to expire
            if((Now - m_ReconnectStartTick) >= (m_ReconnectDelaySeconds * TICKS_PER_SECOND))
            {
                // Try TCP connect again
                if(TCP_Connect(&m_LastServerIP, m_LastServerPort))
                {
                    // Reset backoff
                    m_ReconnectDelaySeconds = 5;
                }
                else
                {
                    // Increase delay (simple backoff)
                    if(m_ReconnectDelaySeconds < 60)
                    {
                        m_ReconnectDelaySeconds *= 2;
                    }

                    // Restart timer
                    m_ReconnectStartTick = Now;
                }
            }
        }
        break;

        case MQTT_STATE_WAIT_CONNACK:
        case MQTT_STATE_WAIT_SUBACK:
        case MQTT_STATE_PUBLISHING:
        {
            HandleIncomingData();
        }
        break;

        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------
//
//  Name:           SetMessageCallback
//
//  Parameter(s):   MQTT_MessageCallback_t  Callback Function called on incoming PUBLISH
//                                          messages.
//                  void* pUserContext      User context pointer passed back to the callback.
//
//  Return:         void
//
//  Description:    Registers a callback invoked when a valid PUBLISH message is received
//                  on any subscribed topic.
//
//---------------------------------------------------------------------------------------------
void MQTT_Client::SetMessageCallback(MQTT_MessageCallback_t Callback, void* pUserContext)
{
    m_MessageCallback = Callback;
    m_pMessageContext = pUserContext;
}

//---------------------------------------------------------------------------------------------

bool MQTT_Client::TCP_Connect(const IP_Address_t* pServerIP, uint16_t Port)
{
    if((pServerIP == nullptr) || (m_pContext == nullptr))
    {
        m_State = MQTT_STATE_ERROR;
        return false;
    }

    // Access TCP manager from your NetworkContext
    TCP_Manager* pTCP = &m_pContext->GetTCP();

    // Initiate TCP connection (non-blocking)
    m_pSocket = pTCP->Connect(*pServerIP, Port);

    if(m_pSocket == nullptr)
    {
        m_State = MQTT_STATE_ERROR;
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------------------------

bool MQTT_Client::SendConnectFrame(const char* pClientID)  //this is bad... it is not using pMemory
{
    if((m_pSocket == nullptr) || (pClientID == nullptr))
    {
        return false;
    }

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(256, MEM_DBG_MQTT);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t Index = 0;

    // ---------------------------------------------------------
    // FIXED HEADER
    // ---------------------------------------------------------
    pBuffer[Index++] = 0x10;   // CONNECT packet type (0001) + flags (0000)

    // Remaining Length placeholder (we fill it later)
    size_t RemainingIndex = Index++;

    // ---------------------------------------------------------
    // VARIABLE HEADER
    // ---------------------------------------------------------

    // Protocol Name: "MQTT"
    pBuffer[Index++] = 0x00;
    pBuffer[Index++] = 0x04;
    pBuffer[Index++] = 'M';
    pBuffer[Index++] = 'Q';
    pBuffer[Index++] = 'T';
    pBuffer[Index++] = 'T';
    pBuffer[Index++] = 0x04;     // Protocol Level: 4 (MQTT 3.1.1)
    pBuffer[Index++] = 0x02;     // Connect Flags (Clean Session)
    pBuffer[Index++] = (m_KeepAliveSeconds >> 8) & 0xFF;         // Keep Alive (seconds)                 used htons()
    pBuffer[Index++] = (m_KeepAliveSeconds     ) & 0xFF;

    // ---------------------------------------------------------
    // PAYLOAD: Client ID
    // ---------------------------------------------------------
    size_t clientID_Len = strlen(pClientID);
    pBuffer[Index++] = (clientID_Len >> 8) & 0xFF;                         // used htons()
    pBuffer[Index++] = (clientID_Len     ) & 0xFF;
    memcpy(&pBuffer[Index], pClientID, clientID_Len);
    Index += clientID_Len;

    // ---------------------------------------------------------
    // REMAINING LENGTH
    // ---------------------------------------------------------
    size_t RemainingLength = Index - 2; // everything after fixed header
    pBuffer[RemainingIndex] = (uint8_t)RemainingLength;

    // ---------------------------------------------------------
    // SEND OVER TCP
    // ---------------------------------------------------------
    size_t Sent = m_pSocket->Send(pBuffer, Index);           // Send packet
    pMemoryPool->Free((void**)&pBuffer);

    if(Sent != Index)
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

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(256, MEM_DBG_MQTT);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t Index = 0;

    // ---------------------------------------------------------
    // Fixed header
    // ---------------------------------------------------------
    pBuffer[Index++] = 0x82;   // SUBSCRIBE + required flags

    // Remaining length will be filled later
    size_t RemainingLengthIndex = Index++;

    // ---------------------------------------------------------
    // Variable header
    // ---------------------------------------------------------
    uint16_t PacketID = NextPacketID();
    pBuffer[Index++] = (PacketID >> 8) & 0xFF;              // htons()
    pBuffer[Index++] = (PacketID     ) & 0xFF;

    // ---------------------------------------------------------
    // Payload: topic + QoS
    // ---------------------------------------------------------
    size_t TopicLen = strlen(pTopic);

    pBuffer[Index++] = (TopicLen >> 8) & 0xFF;              // htons()
    pBuffer[Index++] = (TopicLen     ) & 0xFF;

    memcpy(&pBuffer[Index], pTopic, TopicLen);
    Index += TopicLen;

    pBuffer[Index++] = (uint8_t)QoS;

    // ---------------------------------------------------------
    // Fill Remaining Length
    // ---------------------------------------------------------
    size_t RemainingLength = Index - 2; // everything after fixed header
    pBuffer[RemainingLengthIndex] = (uint8_t)RemainingLength;

    // ---------------------------------------------------------
    // Send over TCP
    // ---------------------------------------------------------
    size_t Sent = m_pSocket->Send(pBuffer, Index);
    pMemoryPool->Free((void**)&pBuffer);
    return (Sent == Index);
}

//---------------------------------------------------------------------------------------------

/*
Fixed header:
  byte 1: 0x30 | (QoS << 1)
  byte 2+: Remaining Length (varint)

Variable header:
  Topic length MSB
  Topic length LSB
  Topic bytes...
  [Packet ID MSB]   (only for QoS 1 or 2)
  [Packet ID LSB]

Payload:
  Raw payload bytes
*/

bool MQTT_Client::SendPublishFrame(const char* pTopic, const uint8_t* pPayload, size_t Length, MQTT_QoS_e QoS)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(512, MEM_DBG_MQTT);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t Index = 0;

    // ---------------------------------------------------------
    // Fixed header
    // ---------------------------------------------------------
    uint8_t header = 0x30;  // PUBLISH
    header |= (QoS << 1);   // QoS bits
    pBuffer[Index++] = header;

    // Remaining length placeholder
    size_t RemainingIndex = Index++;

    // ---------------------------------------------------------
    // Variable header: Topic
    // ---------------------------------------------------------
    size_t TopicLen = strlen(pTopic);

    pBuffer[Index++] = (TopicLen >> 8) & 0xFF;               // use htons()
    pBuffer[Index++] = (TopicLen     ) & 0xFF;

    memcpy(&pBuffer[Index], pTopic, TopicLen);
    Index += TopicLen;

    // ---------------------------------------------------------
    // Packet ID (QoS 1 or 2 only)
    // ---------------------------------------------------------
    uint16_t packetID = 0;

    if(QoS > MQTT_QOS_0)
    {
        packetID = NextPacketID();
        pBuffer[Index++] = (packetID >> 8) & 0xFF;             // use htons()
        pBuffer[Index++] = (packetID     ) & 0xFF;
    }

    // ---------------------------------------------------------
    // Payload
    // ---------------------------------------------------------
    memcpy(&pBuffer[Index], pPayload, Length);
    Index += Length;

    // ---------------------------------------------------------
    // Remaining Length
    // ---------------------------------------------------------
    size_t RemainingLength = Index - 2; // everything after fixed header
    pBuffer[RemainingIndex] = (uint8_t)RemainingLength;

    // ---------------------------------------------------------
    // Send over TCP
    // ---------------------------------------------------------
    size_t Sent = m_pSocket->Send(pBuffer, Index);
    pMemoryPool->Free((void**)&pBuffer);
    return (Sent == Index);
}

//---------------------------------------------------------------------------------------------

bool MQTT_Client::SendDisconnect(void)
{
    if(m_pSocket == nullptr)
    {
        return false;
    }

    uint8_t Buffer[2];
    Buffer[0] = 0xE0;   // DISCONNECT
    Buffer[1] = 0x00;   // Remaining length = 0

    size_t Sent = m_pSocket->Send(Buffer, 2);

    if(Sent != 2)
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

    uint8_t Buffer[2];
    Buffer[0] = 0xC0;   // PINGREQ
    Buffer[1] = 0x00;   // Remaining length = 0

    size_t Sent = m_pSocket->Send(Buffer, 2);

    if(Sent != 2)
    {
        return false;
    }

    // Update activity timer
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

    uint8_t* pBuffer = (uint8_t*)pMemoryPool->AllocAndClear(256, MEM_DBG_MQTT);

    if(pBuffer == nullptr)
    {
        return false;
    }

    size_t Len = m_pSocket->Receive(pBuffer, sizeof(pBuffer));

    if(Len == 0)
    {
        return false;
    }

    int Result = ParseIncomingPacket(pBuffer, Len);
    pMemoryPool->Free((void**)&pBuffer);

    return Result;
}

//---------------------------------------------------------------------------------------------

bool MQTT_Client::ParseIncomingPacket(uint8_t* pBuffer, size_t Length)
{
    uint8_t PacketType = pBuffer[0] >> 4;

    switch(PacketType)
    {
        case 2: // CONNACK
        {
            // Must be at least 4 bytes: header + 2 bytes payload
            if(Length < 4)
            {
                return false;
            }

            uint8_t SessionPreSent = pBuffer[2];
            VAR_UNUSED(SessionPreSent);                 // Keep the line for future use
            uint8_t ReturnCode     = pBuffer[3];

            if(ReturnCode == 0x00)
            {
                // Connection accepted
                m_State = MQTT_STATE_CONNECTED;
                m_LastActivityTick = GetTick();
                return true;
            }
            else
            {
                // Connection refused
                m_State = MQTT_STATE_ERROR;
                return false;
            }
        }

        case 3: // PUBLISH
        {
            // Decode topic length
            size_t Index = 2;
            uint16_t TopicLen = (pBuffer[Index] << 8) | pBuffer[Index + 1];
            Index += 2;

            // Extract topic
            char topic[128];                            // use pMemoryPool
            memcpy(topic, &pBuffer[Index], TopicLen);
            topic[TopicLen] = '\0';
            Index += TopicLen;

            // QoS > 0 includes packet ID
            uint8_t QoS = (pBuffer[0] >> 1) & 0x03;

            if(QoS > 0)
            {
                Index += 2; // skip packet ID
            }

            // Payload
            size_t PayloadLen = Length - Index;
            const uint8_t* Payload = &pBuffer[Index];

            // Deliver to user callback
            if(m_MessageCallback)
            {
                m_MessageCallback(m_pMessageContext, topic, Payload, PayloadLen);
            }

            return true;
        }

        case 4: // PUBACK
        {
            // Must contain at least: header + remaining length + packet ID
            if(Length < 4)
            {
                return false;
            }

            // Extract Packet ID (optional to validate)
            uint16_t packetID = (pBuffer[2] << 8) | pBuffer[3];
            VAR_UNUSED(packetID);

            // Publish completed
            m_State = MQTT_STATE_CONNECTED;
            m_LastActivityTick = GetTick();
            return true;
        }

        case 9: // SUBACK
        {
            // Minimal validation: packet must contain at least 3 bytes
            if(Length < 3)
            {
                return false;
            }

            // Packet ID is bytes 2 and 3
            uint16_t packetID = (pBuffer[2] << 8) | pBuffer[3];
            VAR_UNUSED(packetID);

            // You may check return code at pBuffer[4] (0x00 = success)
            m_State = MQTT_STATE_CONNECTED;
            return true;
        }

        case 13: // PINGRESP
        {
            // PINGRESP is always exactly 2 bytes after header
            if(Length < 2)
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

#endif // (IP_USE_MQTT == DEF_ENABLED)
