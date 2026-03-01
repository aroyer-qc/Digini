//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ethernet_if.h
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TASK_ETHERNET_IF_STACK_SIZE                     96
#define TASK_ETHERNET_IF_PRIO                           7
#define TASK_ETHERNET_IF_NODE_SIZE                      64

#define TASK_ETHERNET_IF_THREAD_NAME_PREAMBULE          "Task ETH IF Input: "
#define TASK_ETHERNET_IF_THREAD_NAME_PREAMBULE_SIZE     sizeof(TASK_ETHERNET_IF_THREAD_NAME_PREAMBULE)
#define TASK_ETHERNET_IF_THREAD_NAME_EXTRACT_SIZE       4
#define TASK_ETHERNET_IF_THREAD_NAME_SIZE               (TASK_ETHERNET_IF_THREAD_NAME_PREAMBULE_SIZE + TASK_ETHERNET_IF_THREAD_NAME_EXTRACT_SIZE) + 1


//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class ETH_IF_Driver
{
    public:

        SystemState_e           Initialize                  (const IP_ETH_Config_t* pETH_Config, NetworkContext& Context);
        void                    Run                         (void);
        NetworkContext*         GetContext                  (void)                                          { return m_pContext; }
        void                    OnMAC_Event                 (uint32_t Event);
        static SystemState_e    LowLevelOutputWrapper       (void* pContext, IP_PacketMsg_t** ppPacketMsg)  { ETH_IF_Driver* Self = static_cast<ETH_IF_Driver*>(pContext);
                                                                                                              return Self->LowLevelOutput(ppPacketMsg);               }

    private:

        SystemState_e           LowLevelOutput              (IP_PacketMsg_t** ppPacketMsg);
        void                    PollTheNetworkInterface     (void);                                                 // This might be a PHY, MAC, HEC ( hardwired ethernet controller Ex. W5100, ESP32 etc...)
      #if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
        void                    LinkCallBack                (void* pArg);
      #endif

        nOS_Sem                     m_RX_Sem;
        nOS_Mutex                   m_TX_Mutex;

        // In a multi interface system this cannot be static
        nOS_Thread                  m_Handle;
        nOS_Stack                   m_Stack                 [TASK_ETHERNET_IF_STACK_SIZE];
        NetworkContext*             m_pContext;
        const IP_ETH_Config_t*      m_pETH_Config;
        char                        m_ThreadName            [TASK_ETHERNET_IF_THREAD_NAME_SIZE]     = TASK_ETHERNET_IF_THREAD_NAME_PREAMBULE;
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

extern "C" void  ClassEthernetIf_Wrapper     (void* pvParameters);

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
