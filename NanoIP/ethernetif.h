//-------------------------------------------------------------------------------------------------
//
//  File : ethernetif.h
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

#include "lib_digini.h"

#if (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

void FreePacket(MemoryNode* pPacket);

//-------------------------------------------------------------------------------------------------

class ETH_IF_Driver
{
    public:

        SystemState_e    Initialize                     (EthernetIF_t* pNetIf);



      #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
        uint32_t    m_DBG_RX_Count;
        uint32_t    m_DBG_TX_Count;
        uint32_t    m_DBG_RX_Drop;
        uint32_t    m_DBG_TX_Drop;
      #endif


    private:

        void             Input                          (void* pParam);


        inline MemoryNode*   LowLevelInput              (void);
        SystemState_e        LowLevelOutput             (MemoryNode* pPacket);               // TODO Should use may chainlist buffer allocation
        void                 ArpTimer                   (void* pArg);
        void                 Callback                   (uint32_t Event);
        void                 PollTheNetworkInterface    (void);                                                 // This might be a PHY, MAC, HEC ( hardwired ethernet controller Ex. W5100, ESP32 etc...)
      #if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
        void                 LinkCallBack               (void* pArg);
      #endif





        nOS_Sem                     m_RX_Sem;
        nOS_Mutex                   m_TX_Mutex;
        nOS_Thread                  m_TaskHandle;
        nOS_Stack                   m_Stack[TASK_ETHERNET_IF_STACK_SIZE];

        EthernetIF_t*               m_pNefIf;

        ETH_Driver                  m_Mac;
        PHY_DRIVER_INTERFACE        m_Phy;
        PHY_DriverInterface         m_ETH_Phy;
        ETH_LinkState_e             m_Link;                // Ethernet Link State
};

//-------------------------------------------------------------------------------------------------

void FreePacket        (MemoryNode* pPacket);


#endif