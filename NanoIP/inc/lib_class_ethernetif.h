//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ethernetif.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

#define TASK_ETHERNET_IF_STACK_SIZE         256
#define TASK_ETHERNET_IF_PRIO               7
#define TASK_ETHERNET_IF_NODE_SIZE          64

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class ETH_IF_Driver
{
    public:

        SystemState_e       Initialize                  (const IP_ETH_Config_t* pETH_Config);

      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        uint32_t            GetDBG_RX_Count             (void)                          { return m_DBG_RX_Count; }
        uint32_t            GetDBG_TX_Count             (void)                          { return m_DBG_TX_Count; }
        uint32_t            GetDBG_RX_Drop              (void)                          { return m_DBG_RX_Drop;  }
        uint32_t            GetDBG_TX_Drop              (void)                          { return m_DBG_TX_Drop;  }
      #endif

        void                Run                         (void);
        static void         CallbackWrapper             (void* pContext, uint32_t Event) { static_cast<ETH_IF_Driver*>(pContext)->CallBack(Event); }

    private:

        inline MemoryNode*  LowLevelInput               (void);
        SystemState_e       LowLevelOutput              (MemoryNode* pPacket);               // TODO Should use may chainlist buffer allocation
        void                ArpTimer                    (void* pArg);
        void                PollTheNetworkInterface     (void);                                                 // This might be a PHY, MAC, HEC ( hardwired ethernet controller Ex. W5100, ESP32 etc...)
        void                CallBack                    (uint32_t Event);
      #if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
        void                LinkCallBack                (void* pArg);
      #endif

        nOS_Sem                     m_RX_Sem;
        nOS_Mutex                   m_TX_Mutex;
        static nOS_Thread           m_Handle;
        static nOS_Stack            m_Stack[TASK_ETHERNET_IF_STACK_SIZE];



        const IP_ETH_Config_t*      m_pETH_Config;

        //ETH_Driver                  m_Mac;
        //PHY_DRIVER_INTERFACE        m_Phy;
        //PHY_DriverInterface         m_ETH_Phy;
        ETH_LinkState_e             m_Link;                // Ethernet Link State

      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        uint32_t                    m_DBG_RX_Count;
        uint32_t                    m_DBG_TX_Count;
        uint32_t                    m_DBG_RX_Drop;
        uint32_t                    m_DBG_TX_Drop;
      #endif
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void             FreePacket                  (MemoryNode* pPacket);
extern "C" void  ClassEthernetIf_Wrapper     (void* pvParameters);

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
