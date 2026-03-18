//-------------------------------------------------------------------------------------------------
//
//  File : taskMQTT.cpp
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
//
//                      ===========================
//                         MQTT SUBSCRIPTION TODO
//                         (Final Architecture: Topic + Queue)
//                      ===========================
//
//
//
//
//
//                      6. Add a message structure for queue delivery
//
//                         struct MQTT_Message
//                         {
//                             const char* Topic;
//                             uint8_t*    Payload;
//                             size_t      Length;
//                         };
//
//
//                      7. Update TaskMQTT (RX loop)
//                         - When a message arrives:
//                             - Extract topic, payload, length
//                             - For each subscription:
//                                   if MatchTopic(sub->Topic, incomingTopic):
//                                       Create MQTT_Message
//                                       Push it into sub->UserQueue
//                                       (DO NOT call user code directly)
//
//
//                      8. Add MQTT_Client connection handling
//                         - When MQTT_Client becomes connected:
//                             - TaskMQTT iterates through m_Subscriptions[]
//                             - Sends SUBSCRIBE for each Topic
//                             - Handles SUBACK
//                         - On reconnect:
//                             - Repeat the SUBSCRIBE procedure automatically
//
//
//                      9. Add protection mechanisms
//                         - Prevent registering the same topic twice
//                         - Prevent overflow if m_SubCount == MAX_SUBS
//                         - Prevent queue overflow (drop or overwrite policy)
//
//
//                      10. (Optional) Add UnregisterAll()
//                          - Free all subscriptions
//                          - Reset m_SubCount to 0
//                          - Optionally flush user queues
//
//                      //
//-------------------------------------------------------------------------------------------------

#define TASK_MQTT_GLOBAL
#include "./lib_digini.h"
#undef  TASK_MQTT_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MQTT_TASK_PERIOD_MS                                 1000
#define MQTT_ETEHERNET_READY_RETRY_MS                       1000
#define MQTT_CONNECT_TO_BROKER_KEEP_ALIVE_SEC               30
#define MQTT_TASK_KEEP_ALIVE_SEC                            60

//-------------------------------------------------------------------------------------------------
//
//  Name:           TaskMQTT_Wrapper
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    main() for the ClassMQTT
//
//-------------------------------------------------------------------------------------------------
extern "C" void TaskMQTT_Wrapper(void* pvParameters)
{
    (static_cast<ClassMQTT*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext* pContext    Reference to the global network context.
//                  const char* pClientName     Name of the MQTT client.
//                                                  - Also used as the thread identifier.
//                  IP_Address_t ServerIP       IP address of the MQTT broker.
//                  uint16_t Port               TCP port of the MQTT broker.
//
//  Return:         SystemState_e               SYS_READY if initialization completed successfully.
//
//  Description:    Initializes the MQTT subsystem. Stores the network context, configures the
//                  internal MQTT client, and saves the broker connection parameters. Clears the
//                  subscription table and resets the subscription counter.
//
//                  Creates the wake-up semaphore and launches the MQTT thread. The connection to
//                  the broker is not established here; it will be initiated by the internal state
//                  machine once the thread starts running.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ClassMQTT::Initialize(NetworkContext* pContext, const char* pClientName, const IP_Address_t ServerIP, uint16_t Port)
{
    m_pContext = pContext;
    m_Client.Initialize(pContext, this);

    m_pClientName = pClientName;
    m_ServerIP    = ServerIP;
    m_ServerPort  = Port;

    m_SubcriptionsCount = 0;

    for(size_t i = 0; i < MQTT_MAX_SUBCRIPTIONS; i++)
    {
        m_pSubscriptions[i] = nullptr;
    }

    nOS_SemCreate(&m_WakeSem, 0, 1);

    /*Error =*/ nOS_ThreadCreate(&m_Handle,
                                 TaskMQTT_Wrapper,
                                 this,
                                 &m_Stack[0],
                                 TASK_MQTT_STACK_SIZE,
                                 TASK_MQTT_PRIO,
                                 pClientName);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Run
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Main MQTT task loop. Waits on the wake-up semaphore or a periodic timeout,
//                  then evaluates the network and MQTT client state.
//
//                  If Ethernet is not ready, the client is forced into a disconnected state and
//                  all subscription flags are cleared. The task then waits before retrying.
//
//                  When idle, the client attempts to establish a connection to the MQTT broker.
//                  Once connected, the task ensures that all registered topics are subscribed.
//                  All protocol-level transitions (CONNACK, SUBACK, PINGRESP, etc.) are handled
//                  internally by the MQTT client state machine.
//
//-------------------------------------------------------------------------------------------------
void ClassMQTT::Run(void)
{
    while(1)
    {
        nOS_SemTake(&m_WakeSem, MQTT_TASK_PERIOD_MS);               // Wait for an event or periodic timeout

        if(m_pContext->IsEthernetReady() == false)                  // If Ethernet is down -> ensure MQTT is disconnected
        {
            m_Client.Disconnect();

            for(size_t i = 0; i < m_SubcriptionsCount; i++)
            {
                if(m_pSubscriptions[i] != nullptr)
                {
                    m_pSubscriptions[i]->AlreadySubscribed = false;
                }
            }

            nOS_Sleep(MQTT_ETEHERNET_READY_RETRY_MS);
            continue;
        }

        MQTT_State_e State = m_Client.GetState();

        if(State == MQTT_STATE_IDLE)                                // If idle -> try to connect
        {
            m_Client.Connect(&m_ServerIP, m_ServerPort, m_pClientName, MQTT_CONNECT_TO_BROKER_KEEP_ALIVE_SEC);
            continue;
        }

        if(State == MQTT_STATE_CONNECTED)                           // If connected -> ensure all topics are subscribed
        {
            for(size_t i = 0; i < m_SubcriptionsCount; i++)
            {
                if((m_pSubscriptions[i] != nullptr) && (m_pSubscriptions[i]->AlreadySubscribed == false))
                {
                    m_Client.Subscribe(m_pSubscriptions[i]->pTopic, MQTT_QOS_0);
                    m_pSubscriptions[i]->AlreadySubscribed = true;
                    break;
                }
            }
        }

        // Other states (WAIT_CONNACK, WAIT_SUBACK, etc.)
        // -> nothing to do, Process() handles them
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SubscribeTopic
//
//  Parameter(s):   const char* pTopic      Topic string to subscribe to. The pointer is stored
//                                          as-is; the caller must ensure the lifetime of the
//                                          string.
//                  nOS_Queue* pUserQueue   Queue where received MQTT messages for this topic will
//                                          be delivered.
//
//  Return:         SystemState_e           SYS_READY           on success.
//                                          SYS_ALREADY_EXIST   if the topic is already registered.
//                                          SYS_ERROR           if no subscription slots are
//                                                              available or allocation fails.
//
//  Description:    Registers a new MQTT subscription. Allocates a subscription entry from the
//                  memory pool, stores the topic pointer and associated user queue, and appends
//                  the entry to the subscription table.
//
//                  Topics may be registered before the MQTT client connects. The MQTT task will
//                  automatically subscribe to all registered topics once the connection is
//                  established.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ClassMQTT::SubscribeTopic(const char* pTopic, nOS_Queue* pUserQueue)
{
    if(m_SubcriptionsCount >= MQTT_MAX_SUBCRIPTIONS)                // Protection: no more room
    {
        return SYS_ERROR;
    }

    for(size_t i = 0; i < m_SubcriptionsCount; i++)                 // Protection: prevent duplicate topics
    {
        if(strcmp(m_pSubscriptions[i]->pTopic, pTopic) == 0)
        {
            return SYS_ALREADY_EXIST;
        }
    }

    MQTT_Subscription_t* pSubscription = (MQTT_Subscription_t*)pMemoryPool->Alloc(sizeof(MQTT_Subscription_t), MEM_DBG_MQTTSUB);            // Allocate subscription entry

    if(pSubscription == nullptr)
    {
        return SYS_ERROR;
    }

    pSubscription->pTopic            = pTopic;                      // Store topic + queue
    pSubscription->pUserQueue        = pUserQueue;
    pSubscription->AlreadySubscribed = true;

    m_pSubscriptions[m_SubcriptionsCount++] = pSubscription;        // Add to list

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UnSubscribeTopic
//
//  Parameter(s):   const char* pTopic      Topic string to remove from the subscription table.
//
//  Return:         SystemState_e       SYS_READY       if the subscription was found and removed.
//                                      SYS_NOT_FOUND   if no matching topic exists.
//
//  Description:    Removes a previously registered MQTT subscription. Searches the subscription
//                  table for the specified topic, frees the associated subscription entry from the
//                  memory pool, compacts the table by shifting remaining entries left, and updates
//                  the subscription count.
//
//                  If the topic is not found, no action is taken and SYS_NOT_FOUND is returned.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ClassMQTT::UnSubscribeTopic(const char* pTopic)
{
    for(size_t i = 0; i < m_SubcriptionsCount; i++)                     // Search for the subscription
    {
        MQTT_Subscription_t* pSubscription = m_pSubscriptions[i];

        if(strcmp(pSubscription->pTopic, pTopic) == 0)
        {
            pMemoryPool->Free((void**)&pSubscription);                  // Free the subscription entry

            for(size_t j = i; j < m_SubcriptionsCount - 1; j++)         // Shift remaining entries left
            {
                m_pSubscriptions[j] = m_pSubscriptions[j + 1];
            }

            m_pSubscriptions[m_SubcriptionsCount - 1] = nullptr;        // Clear last entry
            m_SubcriptionsCount--;                                      // Decrement count

            return SYS_READY;
        }
    }

    return SYS_NOT_FOUND;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           MatchTopic
//
//  Parameter(s):   const char* pSubscriptionTopic    Topic pattern registered by the user.
//                                                    May contain MQTT wildcards
//                                                    ('+' for single-level, '#' for multi-level).
//                  const char* pIncomingTopic        Topic string received from the broker.
//
//  Return:         SystemState_e       SYS_READY   if the incoming topic matches the subscription
//                                                  pattern.
//                                      SYS_ERROR   if the topics do not match.
//
//  Description:    Compares an incoming MQTT topic against a subscription pattern. Supports
//                  exact matches as well as MQTT wildcards:
//
//                      '+'  Matches exactly one topic level.
//                      '#'  Matches all remaining levels (must be the last character).
//
//                  Returns SYS_READY when the incoming topic satisfies the subscription rule.
//
//-------------------------------------------------------------------------------------------------
SystemState_e ClassMQTT::MatchTopic(const char* pSubscriptionTopic, const char* pIncomingTopic)
{
    const char* pSubscription  = pSubscriptionTopic;
    const char* pIncoming      = pIncomingTopic;

    while((*pSubscription != '\0') && (*pIncoming != '\0'))
    {
        if(*pSubscription == '#')                                                       // Wildcard '#': matches everything remaining
        {
            return (*(pSubscription + 1) == '\0') ? SYS_READY : SYS_ERROR;              // '#' must be last character in subscription
        }

        if(*pSubscription == '+')                                                       // Wildcard '+': matches exactly one level
        {
            pSubscription++;                                                            // Skip '+' in subscription

            while((*pIncoming != '\0') && (*pIncoming != '/'))                          // Skip one level in incoming topic
            {
                pIncoming++;
            }

            if((*pSubscription == '\0') && (*pIncoming == '\0'))                        // If both end here, it's a match
            {
                return SYS_READY;
            }

            if((*pSubscription != '\0') && (*pIncoming == '\0'))                        // If subscription expects more but incoming ended -> no match
            {
                return SYS_ERROR;
            }

            if(*pIncoming == '/')                                                       // Skip '/' if present
            {
                pIncoming++;
            }

            continue;
        }

        if(*pSubscription != *pIncoming)                                                // Normal character: must match exactly
        {
            return SYS_ERROR;
        }

        pSubscription++;
        pIncoming++;
    }

    if((*pSubscription == '\0') && (*pIncoming == '\0'))                                // If both ended at the same time -> match
    {
        return SYS_READY;
    }

    if((*pSubscription == '#') && (*(pSubscription + 1) == '\0'))                       // Special case: subscription ends with '#' and incoming still has content
    {
        return SYS_READY;
    }

    return SYS_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           OnEvent
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Signals the MQTT task that an event has occurred. This function releases the
//                  wake-up semaphore, allowing the MQTT thread to resume execution immediately
//                  instead of waiting for the periodic timeout.
//
//-------------------------------------------------------------------------------------------------
void ClassMQTT::OnEvent(void)
{
    nOS_SemGive(&m_WakeSem);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReceivedTopic
//
//  Parameter(s):   const char*    pTopic                   Topic string received from the broker.
//                  const uint8_t* pPayload                 Pointer to the raw payload buffer.
//
//                  size_t Length                           Number of bytes in the payload.
//
//  Return:         None
//
//  Description:    Called by the MQTT client when a PUBLISH packet is received. Logs the incoming
//                  topic and payload, then iterates through all registered subscriptions. For each
//                  subscription whose pattern matches the incoming topic, a message structure is
//                  allocated and pushed into the subscriber’s queue.
//
//                  If the target queue is full, the message is dropped and the allocated memory
//                  is released. Topic matching supports exact matches and MQTT wildcards.
//
//-------------------------------------------------------------------------------------------------
void ClassMQTT::ReceivedTopic(const char* pTopic, const uint8_t* pPayload, size_t Length)
{
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT RX Topic: %s | Payload (%u bytes): %.*s\n", pTopic, (unsigned)Length, (int)Length, (const char*)pPayload);

    for(size_t i = 0; i < m_SubcriptionsCount; i++)                                                                                     // Iterate through all subscriptions
    {
        MQTT_Subscription_t* pSubscription = m_pSubscriptions[i];

        if(MatchTopic(pSubscription->pTopic, pTopic) == SYS_READY)                                                                      // Check topic match
        {
            MQTT_Message_t* pTopicMessage = AllocateTopicMessage(pTopic, pPayload, Length);                                             // Allocate message

            if(pTopicMessage != nullptr)
            {
                if(nOS_QueueWrite(pSubscription->pUserQueue, &pTopicMessage, 0) != NOS_OK)                                              // Try to push message to user queue
                {
                    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT RX DROP: queue full for topic '%s'\n", pSubscription->pTopic);
                    FreeTopicMessage(pTopicMessage);
                }
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           AllocateTopicMessage
//
//  Parameter(s):   const char*    pTopic       Topic string associated with the incoming MQTT msg.
//                  const uint8_t* pPayload     Pointer to the raw payload buffer.
//                  size_t         Length       Number of bytes in the payload.
//
//  Return:         MQTT_Message_t*             Pointer to a newly allocated message structure.
//                                              Returns nullptr if any allocation fails.
//
//  Description:    Static helper used to construct an MQTT message object. Allocates the message
//                  structure, duplicates the topic string, and copies the payload into a dedicated
//                  buffer. All memory is obtained from the shared memory pool.
//
//                  On failure, all partially allocated components are released before returning
//                  nullptr. The caller is responsible for freeing the returned message via
//                  FreeTopicMessage().
//
//-------------------------------------------------------------------------------------------------
MQTT_Message_t* ClassMQTT::AllocateTopicMessage(const char* pTopic, const uint8_t* pPayload, size_t Length)
{
    MQTT_Message_t* pTopicMessage = (MQTT_Message_t*)pMemoryPool->Alloc(sizeof(MQTT_Message_t), MEM_DBG_MQTTMSG);

    if(pTopicMessage == nullptr)
    {
        return nullptr;
    }

    // Allocate and copy TOPIC
    size_t TopicLength = strlen(pTopic) + 1;   // Include null terminator
    char*  pTopicCopy  = (char*)pMemoryPool->Alloc(TopicLength, MEM_DBG_MQTTTOPIC);

    if(pTopicCopy == nullptr)
    {
        pMemoryPool->Free((void**)&pTopicMessage);
        return nullptr;
    }

    memcpy(pTopicCopy, pTopic, TopicLength);

    // Allocate and copy PAYLOAD
    uint8_t* pPayloadCopy = (uint8_t*)pMemoryPool->Alloc(Length, MEM_DBG_MQTTLOAD);

    if(pPayloadCopy == nullptr)
    {
        pMemoryPool->Free((void**)&pTopicCopy);
        pMemoryPool->Free((void**)&pTopicMessage);
        return nullptr;
    }

    memcpy(pPayloadCopy, pPayload, Length);

    // Fill message
    pTopicMessage->pTopic   = pTopicCopy;
    pTopicMessage->pPayload = pPayloadCopy;
    pTopicMessage->Length   = Length;

    return pTopicMessage;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FreeTopicMessage
//
//  Parameter(s):   MQTT_Message_t* pMessage    Pointer to a message previously allocated by
//                                              AllocateTopicMessage().
//
//  Return:         None
//
//  Description:    Static helper used to releases all memory associated with an MQTT message
//                  object. Frees the duplicated topic string, the payload buffer, and finally the
//                  message structure itself. All memory is returned to the shared memory pool.
//
//                  Safe to call with a nullptr pointer. The caller must ensure that the message
//                  is no longer referenced elsewhere before freeing it.
//
//-------------------------------------------------------------------------------------------------
void ClassMQTT::FreeTopicMessage(MQTT_Message_t* pMessage)
{
    if(pMessage == nullptr)
    {
        return;
    }

    if(pMessage->pTopic != nullptr)                         // Free Topic
    {
        pMemoryPool->Free((void**)pMessage->pTopic);
    }

    if(pMessage->pPayload != nullptr)                       // Free Payload
    {
        pMemoryPool->Free((void**)pMessage->pPayload);
    }

    pMemoryPool->Free((void**)pMessage);                    // Free the message structure itself
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)
