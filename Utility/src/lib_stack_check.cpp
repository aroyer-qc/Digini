#define STACKCHECK_NUMBER_OF_STACK                  4
#define STACKCHECK_HIGH_WATER_MARK_CODE             0xFF            // This is for nOS. Depend on the value put in stack by OS. FreeRTOS it's 0xA5

class StackCheck
{
    public:
    
        void Initialize(void);
        void RegisterStack(void* pStack, size_t STackSz);
        void Process(void);

    private:
    
        void*   m_pStackTop    [STACKCHECK_NUMBER_OF_STACK];
        void*   m_pStackBottom [STACKCHECK_NUMBER_OF_STACK];
        void*   m_pMaxReach    [STACKCHECK_NUMBER_OF_STACK];
        size_t  m_Size         [STACKCHECK_NUMBER_OF_STACK];
        uint8_t m_Percent      [STACKCHECK_NUMBER_OF_STACK];
        int     m_FreeSlot;
}

void StackCheck::Initialize(void)
{
    for(int = 0; i < STACKCHECK_NUMBER_OF_STACK; i++)
    {
        m_pStackTop[i]    = nullptr;
        m_pStackBottom[i] = nullptr;
        m_pMaxReach[i]    = nullptr;
        m_Size            = 0;
    }
    
    m_FreeSlot = 0;
}

void StackCheck::RegisterStack(void* pStack, size_t STackSz)
{
    if(m_FreeSlot < STACKCHECK_NUMBER_OF_STACK)
    {
        STackSz--;
        m_pStackBottom[m_FreeSlot] = pStack;
        m_pStackTop   [m_FreeSlot] = static_cast<void*>(uint32_t(pStack) + uint32_t(StackSz));
        m_pMaxReach   [m_FreeSlot] = m_pStackTop[m_FreeSlot];
        m_FreeSlot++;
    }
}

void StackCheck::Process(void)
{
    void* pScan;
    void  IsItFound;
    
    for(int i = 0; i < STACKCHECK_NUMBER_OF_STACK; i++)
    {
        if(i < STACKCHECK_NUMBER_OF_STACK)
        {
            IsItFound = false;
            
            for(pScan = m_pStackBottom; ((pScan <= m_pStackTop[i]) && (IsItFound == false)); pScan++)
            {
                if(*pScan != STACKCHECK_HIGH_WATER_MARK_CODE)
                {
                    if(pScan < m_pMaxReach)
                    {
                        IsItFound   = true;
                        m_pMaxReach = pScan;
                        m_Percent = uint8_t((uint32_t((m_pStackTop) - uint32_t(pScan)) * 100) / m_Size[i]);
                    }
                }
            }
        }
        // os yield not needed on idle task
    }
}