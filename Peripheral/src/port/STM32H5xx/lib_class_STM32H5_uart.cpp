//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_uart.cpp
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define UART_DRIVER_GLOBAL
#include "BSP_board.h"
#include "lib_class_STM32H5_uart.h"
#undef UART_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define UART_DMA_TRANSFER_TIMEOUT  500

// As per PDF RM0481
#define UART_CR1_DEFAULT           0x00000000
#define UART_CR2_DEFAULT           0x00000000
#define UART_CR3_DEFAULT           0x00000000
#define UART_CFG2_DEFAULT          0x00000000
#define UART_CFG1_BR_OFFSET        0x00000000
#define UART_CFG_DEFAULT_DEAT_DEDT 0x03FF0000

#define UART_REG_CR1_PARITY_MASK   (USART_CR1_PCE | USART_CR1_PS)
#define UART_REG_PARITY_NONE       0
#define UART_REG_PARITY_EVEN       USART_CR1_PCE
#define UART_REG_PARITY_ODD        (USART_CR1_PCE | USART_CR1_PS)

#define UART_REG_DATA_WIDTH_8B     0
#define UART_REG_DATA_WIDTH_9B     USART_CR1_M0
#define UART_REG_DATA_WIDTH_7B     USART_CR1_M1

#define UART_REG_OVERSAMPLING_16   0
#define UART_REG_OVERSAMPLING_8    USART_CR1_OVER8
#define UART_REG_OVERSAMPLING_MASK UART_REG_OVERSAMPLING_8

#define UART_REG_RX_ENABLE         USART_CR1_RE
#define UART_REG_TX_ENABLE         USART_CR1_TE
#define UART_REG_RX_TX_ENABLE      (USART_CR1_RE | USART_CR1_TE)

#define UART_REG_STOP_1B           0
#define UART_REG_STOP_0_5B         USART_CR2_STOP_0
#define UART_REG_STOP_2_B          USART_CR2_STOP_1
#define UART_REG_STOP_1_5B         (USART_CR2_STOP_0 | USART_CR2_STOP_1)

#define UART_REG_CR3_FLOW_MASK     (USART_CR3_RTSE | USART_CR3_CTSE)
#define UART_REG_FLOW_CTS          USART_CR3_CTSE
#define UART_REG_FLOW_CTS_ISR      USART_CR3_CTSIE
#define UART_REG_FLOW_RTS          USART_CR3_RTSE

#define UART_REG_CR2_CONFIG_OFFSET 18
#define UART_REG_CR3_CONFIG_OFFSET 12
#define UART_REG_CR1_MASK          0x13FF160C
#define UART_REG_CR2_MASK          0x00003000
#define UART_REG_CR2_PRE_OFF7_MASK (UART_REG_CR2_MASK << UART_REG_CR2_CONFIG_OFFSET)
#define UART_REG_CR3_MASK          0x0000C000
#define UART_REG_CR3_PRE_OFF7_MASK (UART_REG_CR3_MASK << UART_REG_CR3_CONFIG_OFFSET)

#define UART_CTS_STATE             (bool((m_pUARTx->ISR & USART_ISR_CTS_Msk) >> USART_ISR_CTS_Pos))

//-------------------------------------------------------------------------------------------------
//
//   Class: UART_Driver
//
//
//   Description:   Class to handle UART
//
//-------------------------------------------------------------------------------------------------

UART_Driver* UART_Driver::m_pDriver[NB_OF_UART_DRIVER] = {nullptr};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   UART_Driver
//
//   Parameter(s):  UART_ID          ID of the UART info
//
//   Description:   Initializes the UARTx peripheral according to the specified Parameters
//
//-------------------------------------------------------------------------------------------------
UART_Driver::UART_Driver(UART_ID_e UART_ID)
{
    Config(UART_ID);
}

//-------------------------------------------------------------------------------------------------
//
//   Destructor:    ~UART_Driver
//   Parameter(s):  none
//
//   Description:   Free resource
//
//-------------------------------------------------------------------------------------------------
UART_Driver::~UART_Driver()
{
    if(m_pBufferRX != nullptr)
    {
        delete[] m_pBufferRX;
        m_pBufferRX = nullptr;
    }

    if(m_pDMA_RX != nullptr)
    {
        delete m_pDMA_RX;
        m_pDMA_RX = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:   UART_Driver
//
//   Parameter(s):  UART_ID          ID of the UART info
//   Return Value:  None
//
//   Description:   Initializes the UARTx peripheral according to the specified Parameters
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::Config(UART_ID_e UART_ID)
{
    m_UART_ID   = UART_ID;
    m_pUARTx    = UART_Info[UART_ID].pUARTx;
    m_pInfo     = &UART_Info[UART_ID];
    m_Status    = SYS_UNKNOWN;

    // RX 
    m_pDMA_RX          = nullptr;
    m_pBufferRX        = nullptr;
    m_SizeRX           = 0;
    m_pBufferLowRX     = nullptr;
    m_pBufferHiRX      = nullptr;
    m_LastPos          = 0;
    m_CountRX          = 0;
    m_ISR_Mask         = 0;
    m_LastNDTR         = 0;
    m_pCallBack        = nullptr;
    m_CLK_Frequency    = 0;
    m_RxCircular       = false;
    m_pDriver[UART_ID] = this;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Initialize this UART port
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::Initialize(void)
{
    // Init defined IO's
    IO_PinInit(m_pInfo->PinTX);
    IO_PinInit(m_pInfo->PinRX);
    IO_PinInit(m_pInfo->PinDE);

    switch(uint32_t(m_UART_ID))
    {
#if (UART_DRIVER_SUPPORT_UART1_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART1_ID):
            RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
            RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK2Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART2_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART2_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_USART2RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_USART2RST;
            RCC->APB1LENR |= RCC_APB1LENR_USART2EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART3_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART3_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_USART3RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_USART3RST;
            RCC->APB1LENR |= RCC_APB1LENR_USART3EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART4_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART4_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_UART4RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART4RST;
            RCC->APB1LENR |= RCC_APB1LENR_UART4EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART5_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART5_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_UART5RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART5RST;
            RCC->APB1LENR |= RCC_APB1LENR_UART5EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART6_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART6_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_USART6RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_USART6RST;
            RCC->APB1LENR |= RCC_APB1LENR_USART6EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART7_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART7_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_UART7RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART7RST;
            RCC->APB1LENR |= RCC_APB1LENR_UART7EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART8_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART8_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_UART8RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_UART8RST;
            RCC->APB1LENR |= RCC_APB1LENR_UART8EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART9_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART9_ID):
            RCC->APB1HRSTR |= RCC_APB1HRSTR_UART9RST;
            RCC->APB1HRSTR &= ~RCC_APB1HRSTR_UART9RST;
            RCC->APB1HENR |= RCC_APB1HENR_UART9EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART10_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART10_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_USART10RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_USART10RST;
            RCC->APB1LENR |= RCC_APB1LENR_USART10EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART11_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART11_ID):
            RCC->APB1LRSTR |= RCC_APB1LRSTR_USART11RST;
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_USART11RST;
            RCC->APB1LENR |= RCC_APB1LENR_USART11EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
#if (UART_DRIVER_SUPPORT_UART12_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_UART12_ID):
            RCC->APB1HRSTR |= RCC_APB1HRSTR_UART12RST;
            RCC->APB1HRSTR &= ~RCC_APB1HRSTR_UART12RST;
            RCC->APB1HENR |= RCC_APB1HENR_UART12EN;
            m_CLK_Frequency = HAL_RCC_GetPCLK1Freq();
            break;
#endif
        default:
            break;
    }

    m_RxCircular  = (m_pInfo->DMA_RX.Mode == BSP_DMA_MODE_CIRCULAR);

    m_pUARTx->CR1 = UART_CR1_DEFAULT;
    m_pUARTx->CR2 = UART_CR2_DEFAULT;
    m_pUARTx->CR3 = UART_CR3_DEFAULT;

    SetConfig(m_pInfo->Config);
    SetBaudRate(m_pInfo->BaudRate);

    m_SizeRX    = m_pInfo->BufferSize / 2U;
    
    // Allocate RX buffer
    if(m_pBufferRX == nullptr)
    {
        m_pBufferRX    = new uint8_t[m_pInfo->BufferSize];
        m_pBufferLowRX = m_pBufferRX;
        m_pBufferHiRX  = m_pBufferRX + (m_pInfo->BufferSize / 2U);
    }

    // TX DMA
    m_DMA_TX.Initialize(&m_pInfo->DMA_TX);
    m_DMA_TX.SetDestination((void*)&m_pInfo->pUARTx->TDR);

    // RX DMA
    if(m_pInfo->DMA_RX.pChannel != UART_DMA_DISABLED)
    {
        if(m_pDMA_RX == nullptr)
        {
            m_pDMA_RX = new DMA_Driver;
        }

        if(m_pDMA_RX != nullptr)
        {
            m_pDMA_RX->Initialize(&m_pInfo->DMA_RX);
            SET_BIT(m_pUARTx->CR3, USART_CR3_DMAR);

            if(m_RxCircular)
            {
                // TRUE CIRCULAR: full buffer, no ping‑pong
                m_LastPos = 0;
                m_pDMA_RX->SetTransfer((void*)&m_pInfo->pUARTx->RDR, m_pBufferRX, m_pInfo->BufferSize);
                m_pDMA_RX->Enable();  // circular node runs forever
                m_LastNDTR = m_pDMA_RX->GetRemainder();
                // No RXNE interrupt
            }
            else
            {
                // NORMAL PING‑PONG: half‑buffer
                m_SizeRX    = m_pInfo->BufferSize / 2U;
                m_pBufferRX = m_pBufferLowRX;  // start on low half

                m_pDMA_RX->SetTransfer((void*)&m_pInfo->pUARTx->RDR, m_pBufferRX, m_SizeRX);
                m_pDMA_RX->Enable();
                m_LastNDTR = m_pDMA_RX->GetRemainder();
            }
        }
    }
    else
    {
        // Pure interrupt RX (no DMA)
        SET_BIT(m_pUARTx->CR1, USART_CR1_RXNEIE);
        SET_BIT(m_ISR_Mask, USART_ISR_RXNE);
    }

    // TX complete interrupt is used by Send()
    SET_BIT(m_ISR_Mask, USART_ISR_TC);

    SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
    ClearAllFlag();
    ISR_Init(m_pInfo->IRQn, m_pInfo->PreempPrio);
    m_Status = SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EnableCTS_RTS
//
//  Parameter(s):   IO_ForCTS
//                  IO_ForRTS
//  Return:         None
//
//  Description:    Enable the CTS/RTS mode and init the IO
//
//  Note(s);        It also enable the interrupt for the CTS
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::EnableCTS_RTS(IO_ID_e IO_ForCTS, IO_ID_e IO_ForRTS)
{
    IO_PinInit(IO_ForCTS);
    IO_PinInit(IO_ForRTS);
    CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);
    SET_BIT(m_pUARTx->CR3, (UART_REG_FLOW_CTS | UART_REG_FLOW_RTS | UART_REG_FLOW_CTS_ISR));
    SET_BIT(m_ISR_Mask, USART_ISR_CTSIF);
    SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Enable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enable uart
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::Enable(void)
{
    if(m_pUARTx != nullptr)
    {
        SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Disable
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disable uart RX/TX, isr and uart
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::Disable(void)
{
    if(m_pUARTx != nullptr)
    {
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);
        CLEAR_BIT(m_pUARTx->CR3, USART_CR3_EIE);
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_RXNEIE | USART_CR1_IDLEIE | USART_CR1_TXEIE | USART_CR1_TCIE);
        DMA_DisableRX();
        DMA_DisableTX();
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetConfig
//
//  Parameter(s):   Config               Config for UART
//  Return:         None
//
//  Description:    Set UART Config
//
//  Note(s):        - Disable uart
//                  - Set uart properties
//                  - Enable uart
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetConfig(uint32_t Config)
{
    if(m_pUARTx != nullptr)
    {
        uint32_t CRx_Register = 0;

        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);

        if((Config & UART_REG_CR3_PRE_OFF7_MASK) != 0)
        {
            CRx_Register = UART_CFG_DEFAULT_DEAT_DEDT;
            CRx_Register <<= (((Config & UART_CFG_OVER_MASK) == UART_CFG_OVER_16) ? 1 : 0);
        }

        CRx_Register |= ((Config & UART_REG_CR1_MASK) | USART_CR1_IDLEIE);
        MODIFY_REG(m_pUARTx->CR1, UART_REG_CR1_MASK, CRx_Register);

        CRx_Register = (Config & UART_REG_CR2_PRE_OFF7_MASK) >> UART_REG_CR2_CONFIG_OFFSET;
        MODIFY_REG(m_pUARTx->CR2, UART_REG_CR2_MASK, CRx_Register);

        CRx_Register = (Config & UART_REG_CR3_PRE_OFF7_MASK) >> UART_REG_CR3_CONFIG_OFFSET;
        SET_BIT(CRx_Register, USART_CR3_DMAT);
        MODIFY_REG(m_pUARTx->CR3, UART_REG_CR3_MASK, CRx_Register);

        // Make sure IDLE is actually seen in IRQ_Handler
        SET_BIT(m_ISR_Mask, USART_ISR_IDLE);
    }
}

//-------------------------------------------------------------------------------------------------
// 
//  Name:           SetStopBits
//
//  Parameter(s):   NumberOfStopBits     1 or 2 stop bits
//  Return:         None
//
//  Description:    Configure UART stop bits. STOP[1:0] resides in CR2 and requires UE=0 before
//                  modification. This function temporarily disables the UART if necessary,
//                  updates only the STOP bitfield, and restores UE afterwards.
// 
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetStopBits(uint8_t NumberOfStopBits)
{
    if(m_pUARTx == nullptr)
    {
        return;
    }

    const bool wasEnabled = (m_pUARTx->CR1 & USART_CR1_UE) != 0U;
    if(wasEnabled)
    {
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }

    const uint32_t stopBits = (NumberOfStopBits == 2U) ? UART_REG_STOP_2_B : UART_REG_STOP_1B;
    MODIFY_REG(m_pUARTx->CR2, UART_REG_CR2_MASK, stopBits);

    if(wasEnabled)
    {
        SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }
}

//-------------------------------------------------------------------------------------------------
// 
//  Name:           SetParity
//
//  Parameter(s):   ParityEnable         Enable or disable parity
//                  OddParity            true = odd parity, false = even parity
//  Return:         None
//
//  Description:    Configure UART parity. Parity bits (PCE, PS) reside in CR1 and require UE=0
//                  before modification. This function temporarily disables the UART if necessary,
//                  updates only the parity bitfield, and restores UE afterwards.
// 
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetParity(bool ParityEnable, bool OddParity)
{
    if(m_pUARTx == nullptr)
    {
        return;
    }

    const bool wasEnabled = (m_pUARTx->CR1 & USART_CR1_UE) != 0U;
    if(wasEnabled)
    {
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }

    uint32_t newParity = UART_REG_PARITY_NONE;

    if(ParityEnable)
    {
        newParity = OddParity ? UART_REG_PARITY_ODD : UART_REG_PARITY_EVEN;
    }

    MODIFY_REG(m_pUARTx->CR1, UART_REG_CR1_PARITY_MASK, newParity);

    if(wasEnabled)
    {
        SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetEightDataBitsAndParity
//
//  Parameter(s):   ParityEnable         Enable or disable parity
//                  OddParity            true = odd parity, false = even parity
//  Return:         None
//
//  Description:    Configure UART for exactly 8 data bits with optional parity. STM32 UARTs
//                  reduce the number of usable data bits when parity is enabled. To maintain
//                  8 data bits:
//
//                      - No parity:  M1=0, M0=0  (8 data bits)
//                      - Parity on:  M1=0, M0=1  (9-bit mode → 8 data bits + parity)
//
//                  This function temporarily disables UE if necessary, updates only the
//                  word-length and parity bitfields, and restores UE afterwards.
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetEightDataBitsAndParity(bool ParityEnable, bool OddParity)
{
    if(m_pUARTx == nullptr)
    {
        return;
    }

    const bool wasEnabled = (m_pUARTx->CR1 & USART_CR1_UE) != 0U;
    if(wasEnabled)
    {
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }

    uint32_t wordLength = 0U;

    if(ParityEnable)
    {
        wordLength = USART_CR1_M0;
    }

    MODIFY_REG(m_pUARTx->CR1, (USART_CR1_M0 | USART_CR1_M1), wordLength);

    uint32_t newParity = UART_REG_PARITY_NONE;
    if(ParityEnable)
    {
        newParity = OddParity ? UART_REG_PARITY_ODD : UART_REG_PARITY_EVEN;
    }

    MODIFY_REG(m_pUARTx->CR1, UART_REG_CR1_PARITY_MASK, newParity);

    if(wasEnabled)
    {
        SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }
}

//-------------------------------------------------------------------------------------------------
// 
//  Name:           SetFlowControl
//
//  Parameter(s):   EnableHWFlowControl  true = enable RTS/CTS, false = disable
//  Return:         None
//
//  Description:    Configure UART hardware flow control. RTSE and CTSE reside in CR3. While not
//                  strictly required by hardware, ST recommends modifying flow-control settings
//                  with UE=0 to avoid handshake glitches. This function temporarily disables the
//                  UART if necessary, updates only the flow-control bitfield, and restores UE.
// 
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetFlowControl(bool EnableHWFlowControl)
{
    if(m_pUARTx == nullptr)
    {
        return;
    }

    const bool wasEnabled = (m_pUARTx->CR1 & USART_CR1_UE) != 0U;
    if(wasEnabled)
    {
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }

    const uint32_t flowBits = EnableHWFlowControl ? (UART_REG_FLOW_CTS | UART_REG_FLOW_RTS) : 0U;
    MODIFY_REG(m_pUARTx->CR3, UART_REG_CR3_FLOW_MASK, flowBits);

    if(EnableHWFlowControl)
    {
        SET_BIT(m_ISR_Mask, USART_ISR_CTSIF);
    }
    else
    {
        CLEAR_BIT(m_ISR_Mask, USART_ISR_CTSIF);
    }

    if(wasEnabled)
    {
        SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsReceiving
//
//  Parameter(s):   None
//  Return:         bool                True if the UART is actively receiving data,
//                                      false if the line is currently silent.
//
//  Description:    Determine whether the UART is actively receiving bytes. This function
//                  provides a unified activity check that works in both standard RXNE-driven
//                  mode and DMA-driven mode.
//
//                  In non-DMA mode, RXNE indicates that a byte has arrived and the UART is
//                  therefore active.
//
//                  In DMA mode, RXNE cannot be used as an activity indicator because the
//                  DMA engine drains RDR automatically, leaving RXNE asserted for extended
//                  periods. Instead, activity is detected by monitoring the DMA RX transfer
//                  counter (NDTR). If NDTR changes between calls, new bytes have been
//                  received and the UART is considered active.
//
//                  This function allows higher-level protocols (such as BACnet MS/TP) to
//                  correctly reset silence timers even when bytes are delivered to software
//                  in DMA-sized bursts rather than per-byte interrupts.
//
//-------------------------------------------------------------------------------------------------
bool UART_Driver::IsReceiving()
{
    if(m_pDMA_RX != nullptr)
    {
        size_t ndtr   = m_pDMA_RX->GetRemainder();
        bool   active = (ndtr != m_LastNDTR);
        m_LastNDTR    = ndtr;
        return active;
    }

    return (m_pUARTx->ISR & USART_ISR_RXNE) != 0U;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetBaudRate
//
//  Parameter(s):   BaudRate            Desired baud rate
//  Return:         None
//
//  Description:    Configure UART baud rate. BRR must be written with UE=0. This function
//                  temporarily disables the UART if necessary, computes the correct BRR value
//                  for both oversampling modes (OVER8 and OVER16), updates the BRR register,
//                  and restores UE afterwards.
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::SetBaudRate(uint32_t BaudRate)
{
    if((m_pUARTx == nullptr) || (BaudRate == 0U))
    {
        return;
    }

    const bool wasEnabled = (m_pUARTx->CR1 & USART_CR1_UE) != 0U;
    if(wasEnabled)
    {
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }

    const uint32_t periph_clk = m_CLK_Frequency;
    uint32_t       brr        = 0;

    if((m_pUARTx->CR1 & UART_REG_OVERSAMPLING_8) != 0U)
    {
        const uint32_t usartdiv_times8 = (periph_clk + (BaudRate / 2U)) / BaudRate;
        const uint32_t mantissa        = usartdiv_times8 / 8U;
        const uint32_t fraction        = usartdiv_times8 - (mantissa * 8U);
        brr                            = (mantissa << 4) | (fraction & 0x7U);
    }
    else
    {
        brr = (periph_clk + (BaudRate / 2U)) / BaudRate;
    }

    m_pUARTx->BRR       = brr;

    // ---------------------------------------------------------------------
    // Configure RS-485 DE timing (DEAT/DEDT) based on MS/TP requirements.
    // ---------------------------------------------------------------------
    const uint32_t DEAT = 1U;  // 1 bit-time assertion
    const uint32_t DEDT = 1U;  // 1 bit-time deassertion

    m_pUARTx->CR1 &= ~(USART_CR1_DEAT | USART_CR1_DEDT);
    m_pUARTx->CR1 |= ((DEAT << USART_CR1_DEAT_Pos) | (DEDT << USART_CR1_DEDT_Pos));

    if(wasEnabled)
    {
        SET_BIT(m_pUARTx->CR1, USART_CR1_UE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearAllFlag
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear all flag event if not used
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::ClearAllFlag(void)
{
    m_pUARTx->ICR = USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_ORECF | USART_ICR_IDLECF | USART_ICR_TXFECF | USART_ICR_TCCF | USART_ICR_TCBGTCF | USART_ICR_LBDCF | USART_ICR_CTSCF | USART_ICR_RTOCF | USART_ICR_EOBCF | USART_ICR_UDRCF | USART_ICR_CMCF | USART_ICR_WUCF;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_EnableTX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Enable DMA transmit
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_EnableTX(void)
{
    m_DMA_TX.Enable();
    m_DMA_TX.ClearAllFlag();
    SET_BIT(m_pUARTx->CR1, USART_CR1_TE);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_DisableTX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Disable DMA transmit
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_DisableTX(void)
{
    m_DMA_TX.Disable();
    m_DMA_TX.ClearAllFlag();
    SET_BIT(m_pUARTx->CR1, USART_CR1_TE);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_EnableRX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Enable DMA receive
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_EnableRX(void)
{
    if(m_pDMA_RX == nullptr)
    {
        return;
    }

    SET_BIT(m_pUARTx->CR3, USART_CR3_DMAR);
    (void)m_pUARTx->RDR;
    m_pDMA_RX->ClearFlag();
    m_pDMA_RX->Enable();
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_DisableRX
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Disable DMA receive
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::DMA_DisableRX(void)
{
    if(m_pDMA_RX == nullptr)
    {
        return;
    }

    m_pDMA_RX->Disable();
    m_pDMA_RX->ClearFlag();
    CLEAR_BIT(m_pUARTx->CR3, USART_CR3_DMAR);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameter(s):   Differ according to overloaded function
//
//                      - SystemState_e Write   (uint8_t* pBuffer, size_t Size)
//                      - SystemState_e Write   (uint8_t  Data);
//
//  Return:         None
//
//  Description:    Write single data of data buffer to UART
//
//-------------------------------------------------------------------------------------------------
SystemState_e UART_Driver::Send(uint8_t* pBuffer, size_t Size, portTickType xBlockTime)
{
    SystemState_e State = SYS_READY;

    if(m_pUARTx == nullptr)
    {
        return SYS_NOT_INITIALIZE;
    }

    if((pBuffer == nullptr) || (Size == 0U))
    {
        return SYS_WRONG_SIZE;
    }

    WaitReady(xBlockTime);

    if(m_Status == SYS_READY)
    {
        m_Status = SYS_BUSY_TX;

        m_DMA_TX.SetSource((void*)pBuffer);
        m_DMA_TX.SetLength(Size);

        ClearAllFlag();
        SET_BIT(m_pUARTx->CR1, USART_CR1_TCIE);
        // TC is already in m_ISR_Mask from Initialize()

        DMA_EnableTX();

        WaitReady(xBlockTime);
    }
    else
    {
        State = SYS_WRONG_SIZE;
    }

    return State;
}

SystemState_e UART_Driver::Send(uint8_t Data, portTickType xBlockTime)
{
    return Send(&Data, 1U, xBlockTime);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WaitReady
//
//  Parameter(s):   Block time
//
//  Return:         None 
//
//  Description:    Wait for the driver to be ready
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::WaitReady(portTickType xBlockTime)
{
    while((m_Status != SYS_READY) && (xBlockTime > 0))
    {
#ifdef USE_FREERTOS
        if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING && !portIS_CRITICAL())
        {
            vTaskDelay(1);
        }
#endif
        if(xBlockTime < portMAX_DELAY)
        {
            xBlockTime--;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetUartDriver
//
//  Parameter(s):   return the UART driver pointer
//
//  Return:         UART_Driver* 
//
//  Description:    get the UART driver class pointer
//
//-------------------------------------------------------------------------------------------------
UART_Driver* UART_Driver::GetDriverPointer(UART_ID_e UART_ID)
{
    if(UART_ID < NB_OF_UART_DRIVER)
    {
        return m_pDriver[UART_ID];
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RegisterCallback
//
//  Parameter(s):   CallBackInterface* pCallback
//  Return:         void
//
//  Description:    Register the callback for the application layer
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::RegisterCallback(CallBackInterface* pCallBack)
{
    m_pCallBack = pCallBack;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IRQ_Handler
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Signal application -> the transfer is completed
//
//-------------------------------------------------------------------------------------------------
void UART_Driver::IRQ_Handler(void)
{
    if(m_pUARTx == nullptr)
    {
        return;
    }

    // Read full ISR once
    uint32_t isr    = m_pUARTx->ISR;
    uint32_t Status = isr & m_ISR_Mask;

    // --- UART errors: FE, NE, ORE, PE ---
    const uint32_t errMask = USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE;
    if((isr & errMask) != 0U)
    {
        // Clear only error flags
        m_pUARTx->ICR = USART_ICR_FECF | USART_ICR_NECF | USART_ICR_ORECF | USART_ICR_PECF;

        #if (UART_DRIVER_RX_ERROR_CFG == DEF_ENABLED)
            if(m_pCallBack != nullptr)
            {
                m_pCallBack->CallBackFunction(USART_EVENT_RX_ERROR, (void*)err);
            }
        #endif

        // Do NOT return; IDLE/TC/etc. may still need servicing
    }

    // IDLE: frame boundary
    if((Status & USART_ISR_IDLE) != 0U)
    {
        m_pUARTx->ICR = USART_ICR_IDLECF;

        if(m_pDMA_RX != nullptr)
        {
            if(m_RxCircular)
            {
                // ---- TRUE CIRCULAR MODE ----
                size_t ndtr    = m_pDMA_RX->GetRemainder();
                size_t bufSize = m_pInfo->BufferSize;

                // Position in circular buffer: index = bufSize - ndtr
                size_t pos     = bufSize - ndtr;

                if(pos == m_LastPos)
                {
                    // No new data
                    return;
                }

                if(pos > m_LastPos)
                {
                    // No wrap: [m_LastPos .. pos)
                    size_t count = pos - m_LastPos;
                    m_CountRX    = count;

                    if((m_pCallBack != nullptr) && (count != 0U))
                    {
                        uint8_t* pStart = m_pBufferRX + m_LastPos;
                        m_pCallBack->CallBackFunction(USART_EVENT_RX_COMPLETE, pStart);
                    }

                    m_CountRX = 0;
                }
                else
                {
                    // Wrap:
                    // tail: [m_LastPos .. bufSize)
                    // head: [0 .. pos)
                    size_t tailCount = bufSize - m_LastPos;
                    size_t headCount = pos;

                    if((m_pCallBack != nullptr) && (tailCount != 0U))
                    {
                        m_CountRX      = tailCount;
                        uint8_t* pTail = m_pBufferRX + m_LastPos;
                        m_pCallBack->CallBackFunction(USART_EVENT_RX_COMPLETE, pTail);
                    }

                    if((m_pCallBack != nullptr) && (headCount != 0U))
                    {
                        m_CountRX      = headCount;
                        uint8_t* pHead = m_pBufferRX;  // start of buffer
                        m_pCallBack->CallBackFunction(USART_EVENT_RX_COMPLETE, pHead);
                    }

                    m_CountRX = 0;
                }

                m_LastPos = pos;
            }
            else
            {
                // ---- PING‑PONG MODE (existing behavior) ----
                m_CountRX        = m_SizeRX - m_pDMA_RX->GetRemainder();
                uint8_t* pBuffer = ((m_pBufferRX == m_pBufferHiRX) ? m_pBufferLowRX : m_pBufferHiRX);
                m_pDMA_RX->SetFastDestinationAndLength((void*)pBuffer, m_SizeRX);

                if(m_CountRX != 0U)
                {
                    if(m_pCallBack != nullptr)
                    {
                        m_pCallBack->CallBackFunction(USART_EVENT_RX_COMPLETE, m_pBufferRX);
                    }

                    m_CountRX   = 0;
                    m_pBufferRX = pBuffer;
                }
            }
        }
        else
        {
            // No DMA: RXNE path accumulates into m_pBufferRX
            if(m_CountRX != 0U)
            {
                if(m_pCallBack != nullptr)
                {
                    m_pCallBack->CallBackFunction(USART_EVENT_RX_COMPLETE, m_pBufferRX);
                }
                m_CountRX = 0;
            }
        }
        return;
    }

    // RXNE (no DMA case only)
    if((Status & USART_ISR_RXNE) != 0U)
    {
        if(m_CountRX < m_pInfo->BufferSize)
        {
            m_pBufferRX[m_CountRX] = uint8_t(m_pUARTx->RDR);
            m_CountRX++;
        }
        m_pUARTx->ICR = USART_ICR_NECF;
        return;
    }

    // TX complete
    if((Status & USART_ISR_TC) != 0U)
    {
        m_pUARTx->ICR = USART_ICR_TCCF;

        if(m_pCallBack != nullptr)
        {
            m_pCallBack->CallBackFunction(USART_EVENT_TX_COMPLETE, nullptr);
        }

        DMA_DisableTX();
        m_Status = SYS_READY;
        CLEAR_BIT(m_pUARTx->CR1, USART_CR1_TCIE);
        // TC stays in m_ISR_Mask; flag is cleared here
    }

    // CTS event
    if((Status & USART_ISR_CTSIF) != 0U)
    {
        m_pUARTx->ICR = USART_ICR_CTSCF;

        if(m_pCallBack != nullptr)
        {
            m_pCallBack->CallBackFunction(USART_EVENT_CTS, (void*)UART_CTS_STATE);
        }
    }
}

void UART_Driver::IRQ_Handler(UART_ID_e UART_ID)
{
    UART_Driver::m_pDriver[UART_ID]->IRQ_Handler();
}

extern "C"
{
#if (UART_DRIVER_SUPPORT_UART1_CFG == DEF_ENABLED)
    void USART1_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART1_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART2_CFG == DEF_ENABLED)
    void USART2_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART2_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART3_CFG == DEF_ENABLED)
    void USART3_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART3_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART4_CFG == DEF_ENABLED)
    void UART4_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART4_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART5_CFG == DEF_ENABLED)
    void UART5_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART5_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART6_CFG == DEF_ENABLED)
    void USART6_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART6_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART7_CFG == DEF_ENABLED)
    void UART7_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART7_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART8_CFG == DEF_ENABLED)
    void UART8_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART8_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART9_CFG == DEF_ENABLED)
    void UART9_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART9_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART10_CFG == DEF_ENABLED)
    void USART10_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART10_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART11_CFG == DEF_ENABLED)
    void USART11_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART11_ID);
    }
#endif

#if (UART_DRIVER_SUPPORT_UART12_CFG == DEF_ENABLED)
    void UART12_IRQHandler(void)
    {
        UART_Driver::IRQ_Handler(DRIVER_UART12_ID);
    }
#endif
}