//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_uart.h
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "uart_cfg.h"
#include "lib_class_STM32H5_dma.h"

//-------------------------------------------------------------------------------------------------
//  Define(s)
//-------------------------------------------------------------------------------------------------

#define UART_DMA_DISABLED       nullptr
#define USART_EVENT_RX_COMPLETE (1UL << 1)
#define USART_EVENT_RX_ERROR    (1UL << 2)
#define USART_EVENT_RX_OVERFLOW (1UL << 3)
#define USART_EVENT_RX_TIMEOUT  (1UL << 4)
#define USART_EVENT_TX_COMPLETE (1UL << 5)
#define USART_EVENT_CTS         (1UL << 6)

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef enum
{
#if (UART_DRIVER_SUPPORT_UART1_CFG == DEF_ENABLED)
    DRIVER_UART1_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART2_CFG == DEF_ENABLED)
    DRIVER_UART2_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART3_CFG == DEF_ENABLED)
    DRIVER_UART3_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART4_CFG == DEF_ENABLED)
    DRIVER_UART4_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART5_CFG == DEF_ENABLED)
    DRIVER_UART5_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART6_CFG == DEF_ENABLED)
    DRIVER_UART6_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART7_CFG == DEF_ENABLED)
    DRIVER_UART7_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART8_CFG == DEF_ENABLED)
    DRIVER_UART8_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART9_CFG == DEF_ENABLED)
    DRIVER_UART9_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART10_CFG == DEF_ENABLED)
    DRIVER_UART10_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART11_CFG == DEF_ENABLED)
    DRIVER_UART11_ID,
#endif
#if (UART_DRIVER_SUPPORT_UART12_CFG == DEF_ENABLED)
    DRIVER_UART12_ID,
#endif
    NB_OF_UART_DRIVER,
    UART_NONE
} UART_ID_e;

#ifdef __cplusplus

enum UART_Config_e
{
    UART_CFG_ENABLE_RX_TX   = 0x0000000C,
    UART_CFG_ENABLE_RX      = 0x00000004,
    UART_CFG_ENABLE_TX      = 0x00000008,
    UART_CFG_ENABLE_MASK    = 0x0000000C,

    UART_CFG_NO_PARITY      = 0x00000000,
    UART_CFG_EVEN_PARITY    = 0x00000400,
    UART_CFG_ODD_PARITY     = 0x00000600,
    UART_CFG_PARITY_MASK    = 0x00000600,

    UART_CFG_8_LEN_BITS     = 0x00000000,
    UART_CFG_7_LEN_BITS     = 0x10000000,
    UART_CFG_9_LEN_BITS     = 0x00001000,
    UART_CFG_LENGTH_MASK    = 0x10001000,

    UART_CFG_1_STOP_BIT     = 0x00000000,
    UART_CFG_0_5_STOP_BIT   = 0x40000000,
    UART_CFG_1_5_STOP_BIT   = 0xC0000000,
    UART_CFG_2_STOP_BITS    = 0x80000000,
    UART_CFG_STOP_MASK      = 0xC0000000,

    UART_CFG_OVER_16        = 0x00000000,
    UART_CFG_OVER_8         = 0x00008000,
    UART_CFG_OVER_MASK      = 0x00008000,

    UART_CFG_DE_NO_ENABLE   = 0x00000000,
    UART_CFG_DE_ENABLE_LOW  = 0x0C000000,
    UART_CFG_DE_ENABLE_HIGH = 0x04000000,

    UART_CFG_N_7_1          = (UART_CFG_NO_PARITY | UART_CFG_7_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_N_8_1          = (UART_CFG_NO_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_N_9_1          = (UART_CFG_NO_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_1_STOP_BIT),

    UART_CFG_E_7_1          = (UART_CFG_EVEN_PARITY | UART_CFG_7_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_E_8_1          = (UART_CFG_EVEN_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_E_9_1          = (UART_CFG_EVEN_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_1_STOP_BIT),

    UART_CFG_O_7_1          = (UART_CFG_ODD_PARITY | UART_CFG_7_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_O_8_1          = (UART_CFG_ODD_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_1_STOP_BIT),
    UART_CFG_O_9_1          = (UART_CFG_ODD_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_1_STOP_BIT),

    UART_CFG_N_7_2          = (UART_CFG_NO_PARITY | UART_CFG_7_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_N_8_2          = (UART_CFG_NO_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_N_9_2          = (UART_CFG_NO_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_2_STOP_BITS),

    UART_CFG_E_7_2          = (UART_CFG_EVEN_PARITY | UART_CFG_7_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_E_8_2          = (UART_CFG_EVEN_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_E_9_2          = (UART_CFG_EVEN_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_2_STOP_BITS),

    UART_CFG_O_7_2          = (UART_CFG_ODD_PARITY | UART_CFG_7_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_O_8_2          = (UART_CFG_ODD_PARITY | UART_CFG_8_LEN_BITS | UART_CFG_2_STOP_BITS),
    UART_CFG_O_9_2          = (UART_CFG_ODD_PARITY | UART_CFG_9_LEN_BITS | UART_CFG_2_STOP_BITS),
};

struct UART_Info_t
{
    USART_TypeDef* pUARTx;
    IO_ID_e        PinRX;
    IO_ID_e        PinTX;
    IO_ID_e        PinDE;
    uint32_t       Config;
    uint32_t       BaudRate;
    size_t         BufferSize;
    IRQn_Type      IRQn;
    uint8_t        PreempPrio;
    DMA_Info_t     DMA_RX;
    DMA_Info_t     DMA_TX;
};

enum
{
	UART_ERR_NONE = 0,
	UART_ERR_FE   = 1,
	UART_ERR_NE   = 2,
	UART_ERR_ORE  = 3,
	UART_ERR_PE   = 4
} UART_ErrorCode_t;

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class UART_Driver
{
	public:

							UART_Driver					(UART_ID_e UART_ID);
				           ~UART_Driver					();

    void          			Initialize					(void);

    void          			EnableCTS_RT				(IO_ID_e IO_ForCTS, IO_ID_e IO_ForRTS);

    void          			SetBaudRate					(uint32_t BaudRate);
    void          			SetStopBits					(uint8_t NumberOfStopBits);
    void          			SetParity					(bool ParityEnable, bool OddParity);
    void          			SetEightDataBitsAndParity	(bool ParityEnable, bool OddParity);
    void          			SetFlowControl				(bool EnableHWFlowControl);

    bool          			IsReceiving					(void);

    void          			SetConfig					(uint32_t Config);

    SystemState_e 			GetStatus					(void) const				{ return m_Status; }
    
	static UART_Driver* 	GetDriverPointer			(UART_ID_e UART_ID);

    size_t              	GetRX_Count					(void) const				{ return m_CountRX; }

    SystemState_e 			Send						(uint8_t* pBuffer, size_t Size, portTickType xBlockTime = portMAX_DELAY);
    SystemState_e 			Send						(uint8_t Data, portTickType xBlockTime = portMAX_DELAY);

    void          			WaitReady					(portTickType xBlockTime);
    void         			RegisterCallback			(CallBackInterface* pCallBack);
    
	void          			IRQ_Handler					(void);
    static void   			IRQ_Handler					(UART_ID_e UART_ID);

  private:
    SystemState_e          	Transfer					(uint8_t* pTX_Data, uint32_t TX_Size, uint8_t* pRX_Data, uint32_t RX_Size);  // (kept for legacy, if used elsewhere)
    void                   	Config						(UART_ID_e UART_ID);
    void                   	Enable						(void);
    void                   	Disable						(void);
    void                   	ClearAllFlag				(void);
    void                   	DMA_EnableTX				(void);
    void                   	DMA_DisableTX				(void);
    void                   	DMA_EnableRX				(void);
    void                   	DMA_DisableRX				(void);

    uint32_t               	m_CLK_Frequency;
    const UART_Info_t*     	m_pInfo;
    USART_TypeDef*         	m_pUARTx;
    UART_ID_e              	m_UART_ID;
    volatile SystemState_e 	m_Status;
    static UART_Driver*    	m_pDriver[NB_OF_UART_DRIVER];
    CallBackInterface*     	m_pCallBack;

    DMA_Driver*            	m_pDMA_RX;
    DMA_Driver             	m_DMA_TX;

    uint8_t*               	m_pBufferRX;
    uint8_t*               	m_pBufferLowRX;
    uint8_t*               	m_pBufferHiRX;
    size_t                 	m_SizeRX;
    size_t                 	m_LastPos;
    volatile size_t        	m_CountRX;
    uint32_t               	m_ISR_Mask;
    volatile size_t        	m_LastNDTR;
    bool                   	m_RxCircular;
};

#include "uart_var.h"

extern UART_Info_t UART_Info[NB_OF_UART_DRIVER];

#endif  // __cplusplus