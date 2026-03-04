#include "./lib_digini.h"

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)


ETH_STM32_Adapter::ETH_STM32_Adapter(ETH_MAC_DriverInterface* pMAC, ETH_PHY_DriverInterface* pPHY, uint8_t PHY_Address)
: m_pMAC(pMAC), m_pPHY(pPHY), m_PHY_Address(PHY_Address), m_pIF_Driver(nullptr)
{
}

//-------------------------------------------------------------------------------------------------

SystemState_e ETH_STM32_Adapter::Initialize(ETH_IF_Driver* pIF_Driver)
{
    m_pIF_Driver = pIF_Driver;

    if((m_pMAC == nullptr) || (m_pPHY == nullptr))
    {
        return SYS_FAIL;
    }

    SystemState_e State;

    //  Init MAC
    State = m_pMAC->Initialize(m_pIF_Driver, m_PHY_Address);

    if(State != SYS_READY)
    {
        return State;
    }

    State = m_pMAC->InitializeInterface();

    if(State != SYS_READY)
    {
        return State;
    }

    // Set MAC address (from IF context)
    IP_MAC_Address_t MAC;
    NetworkContext* pContext = pIF_Driver->GetContext();

    pContext->GetMAC_Address(&MAC);
    m_pMAC->SetMAC_Address(&MAC);

    // Init PHY
    State = m_pPHY->Initialize(m_pMAC, m_PHY_Address);

    if(State != SYS_READY)
    {
        return State;
    }

    //  Start MAC + DMA
    m_pMAC->Start();

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------

SystemState_e ETH_STM32_Adapter::SendFrame(IP_PacketMsg_t** pPacketMessage)
{
    return m_pMAC->SendFrame(pPacketMessage);
}

//-------------------------------------------------------------------------------------------------

SystemState_e ETH_STM32_Adapter::ReceiveFrame(IP_PacketMsg_t** pPacketMessage)
{
    return m_pMAC->ReceiveFrame(pPacketMessage);
}

//-------------------------------------------------------------------------------------------------

bool ETH_STM32_Adapter::LinkIsUp(void)
{
    return (m_pPHY->GetLinkState() == ETH_LINK_UP);
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
