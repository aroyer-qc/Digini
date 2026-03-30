//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_context.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const IP_Config_t NetworkContext::m_Config[IP_NUMBER_OF_INTERFACE] =
{
    IF_ETH_DEF(EXPAND_X_IF_AS_STRUCT_DATA)
};

//-------------------------------------------------------------------------------------------------

SystemState_e NetworkContext::InitializeMsgQ(void)
{
    nOS_QueueCreate(&m_Q_Msg, m_ArrayPacketPtr, sizeof(IP_PacketMsg_t*), IP_PACKET_Q_SIZE);
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

SystemState_e NetworkContext::SendPacket(IP_PacketMsg_t* pMsg)
{
    if(m_SendCallback == nullptr)
    {
        IP_Manager::FreeMessage(pMsg);
        return SYS_INVALID_STATE;
    }

    return m_SendCallback(m_SendContext, &pMsg);
}

//-------------------------------------------------------------------------------------------------

void NetworkContext::Initialize(IF_ID_e IF_ID)
{
    m_IF_ID = IF_ID;

  #if (IP_USE_DHCP == DEF_ENABLED)
    m_DHCP.SetEnabled(true);
  #endif

    // Initialize Variables
    //m_DNS_IP_Found = false;  not used so far
    InitializeMsgQ();                                                         // this need to handle error
    SetMAC_Address(&m_Config[IF_ID].IP_ETH_Config.MAC_Address);
    SetHostName(m_Config[IF_ID].pHostName);
    SetMTU(IP_NET_IF_MTU);                                                    // Set netif maximum transfer unit
    SetStaticIP(m_Config[IF_ID].DefaultStatic_IP);
    SetStaticGatewayIP(m_Config[IF_ID].DefaultGateway);
    SetStaticSubnetMask(m_Config[IF_ID].DefaultSubnetMask);
    SetStaticDNS_IP(m_Config[IF_ID].DefaultStaticDNS);
    SetIP_Valid((m_Config[IF_ID].DefaultStatic_IP == IP_ADDRESS(255,255,255,255)) ? false : true);
    m_IF_Driver.Initialize(&m_Config[IF_ID].IP_ETH_Config, this);
    RegisterSendCallback(&m_IF_Driver.LowLevelOutputWrapper, &m_IF_Driver);

    // Now initialize managers
    m_IP_Manager.Initialize(this);
    m_SocketManager.Initialize(this);
    m_ARP.Initialize(this);

  #if (IP_USE_DHCP == DEF_ENABLED)
    m_DHCP.Initialize(this);
  #endif
  #if (IP_USE_ICMP == DEF_ENABLED)
    m_ICMP.Initialize(this);
  #endif
    m_UDP.Initialize(this);
  #if (IP_USE_DNS == DEF_ENABLED)
    m_DNS.Initialize(this);
  #endif
  #if (IP_USE_NTP == DEF_ENABLED)
    m_NTP.Initialize(this);
  #endif
  #if (IP_USE_SNTP == DEF_ENABLED)
    m_SNTP.Initialize(this);
  #endif
  #if (IP_USE_RAW == DEF_ENABLED)
    m_RAW.Initialize(this);
  #endif

  #if (IP_USE_TCP_SERVER == DEF_ENABLED) || (IP_USE_TCP_CLIENT == DEF_ENABLED)
    m_TCP.Initialize(this);
    SetTCP_Manager(&m_TCP);
  #endif
}




bool NetworkContext::IsEthernetReady(void)
{
    bool EthernetReady = (GetLinkState() == ETH_LINK_UP)                    &&
                         (IsIP_Valid() == true)                             &&
                         //(GetActiveIP() != IP_ADDRESS(0.0.0.0))    &&
                         (GetActiveSubnetMask() != IP_ADDRESS(0, 0, 0, 0))  &&
                         (GetActiveGatewayIP() != IP_ADDRESS(0, 0, 0, 0));

    return EthernetReady;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


