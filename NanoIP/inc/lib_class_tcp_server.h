class TCP_Server
{
public:
    void Listen(uint16_t Port);
    void Poll(); // process incoming segments
    void SendResponse(ConnectionID, const uint8_t* pData, size_t Length);

private:
    TCP_Connection ConnectionTable[MAX_CONN];
    void HandleSyn(...);
    void HandleEstablished(...);
    void HandleFin(...);
};