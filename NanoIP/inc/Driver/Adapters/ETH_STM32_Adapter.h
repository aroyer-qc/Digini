class ETH_STM32_Adapter : public ETH_LinkDriver
{
    public:
    
                        ETH_STM32_Adapter           (ETH_MAC_DriverInterface* pMAC, ETH_PHY_DriverInterface* pPHY, uint8_t PHY_Address);

        bool            Initialize                  (void* pContext) override;
        bool            SendFrame                   (const uint8_t* pData, size_t Length) override;
        bool            ReceiveFrame                (uint8_t* pBuffer, size_t* pLength) override;
        bool            LinkIsUp                    (void) override;

    private:
    
        ETH_MAC_DriverInterface*    m_pMAC;
        ETH_PHY_DriverInterface*        m_pPHY;
        uint8_t                     m_PHY_Address;
};