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

#define MQTT_TASK_PERIOD_MS                                 200
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
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" void TaskMQTT_Wrapper(void* pvParameters)
{
    (static_cast<ClassMQTT*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
// Initialize()
//-------------------------------------------------------------------------------------------------

SystemState_e ClassMQTT::Initialize(NetworkContext* pContext, const char* pClientName, const IP_Address_t ServerIP, uint16_t Port)
{
    // Initialize MQTT library with the network context
    m_pContext = pContext;
    m_Client.Initialize(pContext, this);

    m_pClientName = pClientName;
    m_ServerIP    = ServerIP;
    m_ServerPort  = Port;

    m_SubcriptionsCount = 0;

    for(size_t i = 0; i < MQTT_MAX_SUBCRIPTIONS; i++)
    {
        m_Subscriptions[i] = nullptr;
    }

    nOS_SemCreate(&m_WakeSem, 0, 1);

    // Create task
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
// Run()
//-------------------------------------------------------------------------------------------------
void ClassMQTT::Run(void)
{
    bool AlreadySubscribed = false;

    while(1)
    {
        // Wait for an event or periodic timeout
        nOS_SemTake(&m_WakeSem, MQTT_TASK_PERIOD_MS);

        // If Ethernet is down -> ensure MQTT is disconnected
        if(m_pContext->IsEthernetReady() == false)
        {
            m_Client.Disconnect();
            AlreadySubscribed = false;
            nOS_Sleep(MQTT_ETEHERNET_READY_RETRY_MS);
            continue;
        }

        MQTT_State_e State = m_Client.GetState();

        // If idle -> try to connect
        if(State == MQTT_STATE_IDLE)
        {
            m_Client.Connect(&m_ServerIP, m_ServerPort, m_pClientName, MQTT_CONNECT_TO_BROKER_KEEP_ALIVE_SEC);
            continue;
        }

        // If connected -> ensure all topics are subscribed
        if(State == MQTT_STATE_CONNECTED)
        {
            if(AlreadySubscribed == false)
            {
                for(size_t i = 0; i < m_SubcriptionsCount; i++)
                {
                    m_Client.Subscribe(m_Subscriptions[i]->pTopic, MQTT_QOS_0);
                }

                AlreadySubscribed = true;
            }

            continue;
        }

        // Other states (WAIT_CONNACK, WAIT_SUBACK, etc.)
        // -> nothing to do, Process() handles them
    }
}


//-------------------------------------------------------------------------------------------------
// RegisterTopic()
//                      3. Implement RegisterTopic(topic, userQueue)
//                         - Allocate an MQTT_Subscription via pMemoryPool
//                         - Copy or reference the topic
//                         - Store the userQueue pointer
//                         - Add it to m_Subscriptions[]
//                         - Increment m_SubCount
//                         - (Topics may be registered BEFORE MQTT_Client exists or connects)
//
//-------------------------------------------------------------------------------------------------
SystemState_e ClassMQTT::RegisterTopic(const char* pTopic, nOS_Queue* pUserQueue)
{
    // Protection: no more room
    if(m_SubcriptionsCount >= MQTT_MAX_SUBCRIPTIONS)
    {
        return SYS_ERROR;
    }

    // Protection: prevent duplicate topics
    for(size_t i = 0; i < m_SubcriptionsCount; i++)
    {
        if(strcmp(m_Subscriptions[i]->pTopic, pTopic) == 0)
        {
            return SYS_ALREADY_EXIST;
        }
    }

    // Allocate subscription entry
    MQTT_Subscription_t* pSubscription = (MQTT_Subscription_t*)pMemoryPool->Alloc(sizeof(MQTT_Subscription_t), MEM_DBG_MQTTSUB);

    if(pSubscription == nullptr)
    {
        return SYS_ERROR;
    }

    // Store topic + queue
    pSubscription->pTopic     = pTopic;
    pSubscription->pUserQueue = pUserQueue;

    // Add to list
    m_Subscriptions[m_SubcriptionsCount++] = pSubscription;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
// UnRegisterTopic()
//
//                      4. Implement UnregisterTopic(topic)
//                         - Iterate through m_Subscriptions[]
//                         - Find the entry with Topic == topic (or strcmp)
//                         - Free it via pMemoryPool
//                         - Compact the list (shift left)
//                         - Decrement m_SubCount
//
//-------------------------------------------------------------------------------------------------
SystemState_e ClassMQTT::UnRegisterTopic(const char* pTopic)
{
    // Search for the subscription
    for(size_t i = 0; i < m_SubcriptionsCount; i++)
    {
        MQTT_Subscription_t* pSubscription = m_Subscriptions[i];

        if(strcmp(pSubscription->pTopic, pTopic) == 0)
        {
            // Free the subscription entry
            pMemoryPool->Free((void**)&pSubscription);

            // Shift remaining entries left
            for(size_t j = i; j < m_SubcriptionsCount - 1; j++)
            {
                m_Subscriptions[j] = m_Subscriptions[j + 1];
            }

            // Clear last entry
            m_Subscriptions[m_SubcriptionsCount - 1] = nullptr;

            // Decrement count
            m_SubcriptionsCount--;

            return SYS_READY;
        }
    }

    return SYS_NOT_FOUND;
}

//-------------------------------------------------------------------------------------------------
// MatchTopic()
//
//                      5. Implement MatchTopic(subTopic, incomingTopic)
//                         - Support:
//                             - Exact match
//                             - Wildcard '+'
//                             - Wildcard '#'
//                         - Return true if the topic matches
//-------------------------------------------------------------------------------------------------
SystemState_e ClassMQTT::MatchTopic(const char* pSubscriptionTopic, const char* pIncomingTopic)
{
    const char* pSubscription  = pSubscriptionTopic;
    const char* pIncoming      = pIncomingTopic;

    while((*pSubscription != '\0') && (*pIncoming == '\0'))
    {
        // Wildcard '#': matches everything remaining
        if(*pSubscription == '#')
        {
            // '#' must be last character in subscription
            return (*(pSubscription + 1) == '\0') ? SYS_READY : SYS_ERROR;
        }

        // Wildcard '+': matches exactly one level
        if(*pSubscription == '+')
        {
            // Skip '+' in subscription
            pSubscription++;

            // Skip one level in incoming topic
            while((*pIncoming != '\0') && (*pIncoming != '/'))
            {
                pIncoming++;
            }

            // If both end here, it's a match
            if((*pSubscription == '\0') && (*pIncoming == '\0'))
            {
                return SYS_READY;
            }

            // If subscription expects more but incoming ended -> no match
            if((*pSubscription != '\0') && (*pIncoming == '\0'))
            {
                return SYS_ERROR;
            }

            // Skip '/' if present
            if(*pIncoming == '/')
            {
                pIncoming++;
            }

            continue;
        }

        // Normal character: must match exactly
        if(*pSubscription != *pIncoming)
        {
            return SYS_ERROR;
        }

        pSubscription++;
        pIncoming++;
    }

    // If both ended at the same time -> match
    if((*pSubscription == '\0') && (*pIncoming == '\0'))
    {
        return SYS_READY;
    }

    // Special case: subscription ends with '#' and incoming still has content
    if((*pSubscription == '#') && (*(pSubscription + 1) == '\0'))
    {
        return SYS_READY;
    }

    return SYS_ERROR;
}

//    return m_Client.Subscribe(pTopic, MQTT_QOS_0);
//    return m_Client.Publish(pTopic, (const uint8_t*)pMsg, strlen(pMsg), MQTT_QOS_0);
//    return m_Client.Connect(&ServerIP, Port, m_pClientName, MQTT_CONNECT_TO_BROKER_KEEP_ALIVE_SEC);

//-------------------------------------------------------------------------------------------------

void ClassMQTT::OnEvent(void)
{
    nOS_SemGive(&m_WakeSem);
}

//-------------------------------------------------------------------------------------------------

void ClassMQTT::ReceivedTopic(const char* pTopic, const uint8_t* pPayload, size_t Length)
{
    DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET,
                         "MQTT RX Topic: %s | Payload (%u bytes): %.*s\n",
                         pTopic,
                         (unsigned)Length,
                         (int)Length,
                         (const char*)pPayload);

    // Iterate through all subscriptions
    for(size_t i = 0; i < m_SubcriptionsCount; i++)
    {
        MQTT_Subscription_t* pSubscription = m_Subscriptions[i];

        // Check topic match
        if(MatchTopic(pSubscription->pTopic, pTopic) == SYS_READY)
        {
            MQTT_Message_t msg;

            msg.pTopic   = pTopic;
            msg.pPayload = (uint8_t*)pPayload;   // No copy (zero‑copy dispatch)
            msg.Length   = Length;

            // Try to push message to user queue
            if(nOS_QueueWrite(pSubscription->pUserQueue, &msg, 0) != NOS_OK)
            {
                DEBUG_PrintSerialLog(SYS_DEBUG_LEVEL_ETHERNET, "MQTT RX DROP: queue full for topic '%s'\n", pSubscription->pTopic);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)
