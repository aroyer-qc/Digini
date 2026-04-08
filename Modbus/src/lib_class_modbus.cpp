enum class ModbusPath
{
    LOCAL,
    TCP,
    RTU,
    AUTO
};

struct ModbusCommand
{
    uint8_t     functionCode;
    uint16_t    address;
    uint16_t    quantity;

    uint8_t*    payload = nullptr;   // fourni par MemoryPool
    uint16_t    payloadLen = 0;

    uint8_t     unitId = 1;
    uint32_t    timeoutMs = 200;

    ModbusTransport transport = ModbusTransport::AUTO;
};

struct ModbusTcpEndpoint
{
    int         socketFd = -1;
    uint32_t    ipAddr = 0;   // IPv4 packed
    uint16_t    port = 502;
};

struct ModbusRtuEndpoint
{
    int         uartFd = -1;
    uint32_t    baudrate = 115200;
    uint8_t     parity = 0;
    uint8_t     stopBits = 1;
};

struct ModbusExecutionContext
{
    ModbusTcpEndpoint* tcp = nullptr;
    ModbusRtuEndpoint* rtu = nullptr;
    uint32_t timestampMs = 0;
    uint8_t  retries = 0;

    void* userData = nullptr;
};

struct ModbusRoute
{
    ModbusRouteType type = ModbusRouteType::AUTO;

    uint8_t     unitId = 1;
    uint32_t    ipAddr = 0;   // IPv4 packed
    uint16_t    port = 502;
};

struct ModbusCommandEntry
{
    const char*     name;   // string literal ou pool
    ModbusCommand   cmd;
};

struct ModbusCommandRegistry {
    ModbusCommand* entries = nullptr;   // tableau statique ou MemoryPool
    uint16_t       count = 0;
};

enum class ModbusCommandId : uint16_t {
    READ_HOLDING,
    READ_INPUT,
    WRITE_SINGLE,
    WRITE_MULTIPLE,
    COUNT
};

class ModbusRegistry {

public:
    static const ModbusCommand* Get(const ModbusCommandRegistry& reg, ModbusCommandId id)   // pas sur que c'est utile
    {
        uint16_t idx = static_cast<uint16_t>(id);
        
        if(idx >= reg.count)
        {
            return nullptr;
        }
        
        return &reg.entries[idx];
    }

    static ModbusCommand* Get(ModbusCommandRegistry& reg, ModbusCommandId id)
    {
        uint16_t idx = static_cast<uint16_t>(id);
        
        if(idx >= reg.count)
        {
            return nullptr;
        }
        
        return &reg.entries[idx];
    }
};

class ModbusRouter
{
    public:
        ModbusExecutionContext* selectContext(const ModbusCommand& cmd, const ModbusRoute& route)
        {
            switch(route.type)
            {
                case ModbusRouteType::LOCAL:
                    return localContext;

                case ModbusRouteType::TCP:
                    return getTcpContext(route.ipAddr, route.port);

                case ModbusRouteType::RTU:
                    return getRtuContext(route.unitId);

                case ModbusRouteType::AUTO:
                    return autoSelect(cmd);
            }
            
            return nullptr;
        }

private:
    ModbusExecutionContext* localContext = nullptr;

    ModbusExecutionContext* getTcpContext(uint32_t ip, uint16_t port);
    ModbusExecutionContext* getRtuContext(uint8_t unitId);
    ModbusExecutionContext* autoSelect(const ModbusCommand&);
};