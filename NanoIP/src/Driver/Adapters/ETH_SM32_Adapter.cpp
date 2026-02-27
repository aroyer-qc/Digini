#include "./lib_digini.h"

ETH_STM32_Adapter::ETH_STM32_Adapter(ETH_MAC_DriverInterface* pMAC, ETH_PHY_DriverInterface* pPHY, uint8_t PHY_Address) : m_pMAC(pMAC), m_pPHY(pPHY), m_PHY_Address(PHY_Address)
{
}

bool ETH_STM32_Adapter::Initialize(void* pContext)
{
    if(m_pMAC->Initialize(pContext, m_PHY_Address) != SYS_READY)
    {
        return false;
    }

    if(m_pMAC->InitializeInterface() != SYS_READY)
    {
        return false;
    }

    return true;
}

bool ETH_STM32_Adapter::SendFrame(const uint8_t* pData, size_t Length)
{
    IP_PacketMsg_t* pMsg = (IP_PacketMsg_t*)pData;
    return m_pMAC->SendTX_Packet(&pMsg) == SYS_READY;
}

bool ETH_STM32_Adapter::ReceiveFrame(uint8_t* pBuffer, size_t* pLength)
{
    IP_PacketMsg_t* pMsg = nullptr;

    if(m_pMAC->GetRX_Packet(&pMsg) != SYS_READY)
    {
        return false;
    }

    uint32_t size = m_pMAC->GetRX_FrameSize();
    memcpy(pBuffer, pMsg->pPacket, size);
    *pLength = size;

    return true;
}

bool ETH_STM32_Adapter::LinkIsUp(void)
{
    uint16_t reg = 0;

    if(m_pPHY->Read(m_PHY_Address, PHY_BSR, &reg) != SYS_READY)
    {
        return false;
    }

    return (reg & PHY_LINKED_STATUS) != 0;
}