//-------------------------------------------------------------------------------------------------
//
//  File : taskMQTT.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Global Macro
//-------------------------------------------------------------------------------------------------

#ifdef TASK_MQTT_GLOBAL
    #define TASK_MQTT_EXTERN
#else
    #define TASK_MQTT_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TASK_MQTT_STACK_SIZE               320
#define TASK_MQTT_PRIO                     4

#define MQTT_MAX_SUBCRIPTIONS              10

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct MQTT_Subscription_t
{
    const char*     pTopic;             // Topic string to match
    nOS_Queue*      pUserQueue;         // Queue where messages will be delivered
    bool            AlreadySubscribed;
};

struct MQTT_Message_t
{
    const char*     pTopic;
    uint8_t*        pPayload;
    size_t          Length;
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------
class ClassMQTT : MQTT_Handler
{
    public:

        SystemState_e               Initialize                  (NetworkContext* pContext, const char* pClientName, const IP_Address_t ServerIP, uint16_t Port);
        void                        Run                         (void);

        SystemState_e               SubscribeTopic              (const char* Topic, nOS_Queue* pUserQueue);
        SystemState_e               UnSubscribeTopic            (const char* Topic);
      //SystemState_e               UnSubscribeAllTopic         (void);                                                 // Optional for later

        static void                 FreeTopicMessage            (MQTT_Message_t* pTopicMessage);

    private:

        SystemState_e               MatchTopic                  (const char* pSubcriptionTopic, const char* pIncomingTopic);
        void                        OnEvent                     (void);
        void                        ReceivedTopic               (const char* pTopic, const uint8_t* pPayload, size_t Length);

        static MQTT_Message_t*      AllocateTopicMessage        (const char* pTopic, const uint8_t* pPayload, size_t Length);

        nOS_Thread                  m_Handle;
        nOS_Stack                   m_Stack                     [TASK_MQTT_STACK_SIZE];
        nOS_Sem                     m_WakeSem;

        const char*                 m_pClientName;
        IP_Address_t                m_ServerIP;
        uint16_t                    m_ServerPort;

        NetworkContext*             m_pContext;
        MQTT_Client                 m_Client;                                               // The MQTT library instance

        MQTT_Subscription_t*        m_pSubscriptions            [MQTT_MAX_SUBCRIPTIONS];    // To replace later by growing link list of Subcriptions
        size_t                      m_SubcriptionsCount;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

TASK_MQTT_EXTERN   class ClassMQTT    TaskMQTT;

#ifdef TASK_MQTT_GLOBAL
                    class ClassMQTT*  pTaskMQTT = &TaskMQTT;
#else
    extern          class ClassMQTT*  pTaskMQTT;
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_MQTT == DEF_ENABLED)
