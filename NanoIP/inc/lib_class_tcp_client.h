class TCP_Client
{
public:
    bool Connect(IP_Address_t RemoteIP, uint16_t RemotePort);
    bool Send(const uint8_t* pData, size_t Length);
    bool Receive(uint8_t* pBuffer, size_t* pLength);
    void Close();

private:
    TCP_ControlBlock TCB;
    void HandleSynAck(...);
    void HandleData(...);
    void HandleFin(...);
};