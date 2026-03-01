class ETH_STM32_Adapter : public ETH_LinkDriver
{
    public:

                        ETH_STM32_Adapter           (ETH_MAC_DriverInterface* pMAC, ETH_PHY_DriverInterface* pPHY, uint8_t PHY_Address);

        SystemState_e   Initialize                  (ETH_IF_Driver* pIF_Driver) override;
        SystemState_e   SendFrame                   (IP_PacketMsg_t** pPacketMessage) override;
        SystemState_e   ReceiveFrame                (IP_PacketMsg_t** pPacketMessage) override;
        bool            LinkIsUp                    (void) override;

    private:

        ETH_MAC_DriverInterface*    m_pMAC;
        ETH_PHY_DriverInterface*    m_pPHY;
        uint8_t                     m_PHY_Address;
        ETH_IF_Driver*              m_pIF_Driver;
};
