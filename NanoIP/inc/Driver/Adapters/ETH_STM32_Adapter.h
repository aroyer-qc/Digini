class ETH_STM32_Adapter : public ETH_LinkDriver
{
    public:

                        ETH_STM32_Adapter           (ETH_MAC_DriverInterface* pMAC, ETH_PHY_DriverInterface* pPHY, uint8_t PHY_Address);

        SystemState_e   Initialize                  (void* pContext) override;
        SystemState_e   SendFrame                   (IP_PacketMsg_t** pPacketMessage) override;
        SystemState_e   ReceiveFrame                (IP_PacketMsg_t** pPacketMessage) override;
        bool            LinkIsUp                    (void) override;
        
        // ISR event dispatcher (called by ETH_Driver::ISR_CallBack)
        void            OnMacEvent                  (uint32_t Event);


    private:

        ETH_MAC_DriverInterface*    m_pMAC;
        ETH_PHY_DriverInterface*    m_pPHY;
        uint8_t                     m_PHY_Address;
        void*                       m_pIF_Context;     // ETH_IF_Driver*
};
