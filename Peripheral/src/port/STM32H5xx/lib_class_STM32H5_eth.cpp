//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_eth.cpp
//
//-------------------------------------------------------------------------------------------------
//
// # Limitations
//
// Descriptor and data buffers location:
// - Memory for ETH DMA Descriptors (DMARxDscrTab and DMATxDscrTab) must be configured as:
//   non-cacheable, non-shareable device memory, execute never
// - Memory for ETH data buffers (Rx_Buff and Tx_Buff) must be configured as:
//   non-cacheable, non-shareable normal memory, execute never
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "BSP_board.h"
#include "stm32h563xx.h"
#include "stm32h5xx_hal_eth.h"
#include <cstring>

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Receive/transmit Checksum offload enable
#ifndef EMAC_CHECKSUM_OFFLOAD
#define EMAC_CHECKSUM_OFFLOAD 1
#endif

// IEEE 1588 time stamping enable (PTP)
#ifndef EMAC_TIME_STAMP
#define EMAC_TIME_STAMP 0
#endif

/* Time to block waiting for transmissions to finish */
#define ETH_TX_TIMEOUT_MS (2000U)

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------
static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT];  // Ethernet Rx DMA Descriptors
static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT];  // Ethernet Tx DMA Descriptors
static SemaphoreHandle_t  RxPktSemaphore = nullptr;       // Semaphore to signal incoming packets
static SemaphoreHandle_t  TxPktSemaphore = nullptr;       // Semaphore to signal transmit packet complete

extern "C"
{
    // Global ethernet interrupt handler
    static void (*pIRQHandlerImpl)();
    void ETH_IRQHandler(void)
    {
        // Calls a registered interrupt handler when it is provided
        if(pIRQHandlerImpl)
        {
            pIRQHandlerImpl();
        }
    }

    // HAL Init / Deinit Callback: These functions are provided by this driver
    static void (*pMspInitImpl)(void);
    void HAL_ETH_MspInit(ETH_HandleTypeDef* pETH_Handle)
    {
        UNUSED(pETH_Handle);
        // This is called by HAL when HAL_ETH_Init() is called.
        if(pMspInitImpl)
        {
            pMspInitImpl();
        }
    }
    static void (*pMspDeInitImpl)(void);
    void HAL_ETH_MspDeInit(ETH_HandleTypeDef* pETH_Handle)
    {
        UNUSED(pETH_Handle);
        // This is called by HAL when HAL_ETH_DeInit() is called.
        if(pMspDeInitImpl)
        {
            pMspDeInitImpl();
        }
    }

    // TX/RX Callback: These functions are provided by this driver
    static void (*pRxCpltCallbackImpl)(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef* pETH_Handle)
    {
        // Called when an Ethernet packet is received
        if(pRxCpltCallbackImpl)
        {
            pRxCpltCallbackImpl(pETH_Handle);
        }
    }
    static void (*pTxCpltCallbackImpl)(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef* pETH_Handle)
    {
        // Called when end of transfer occur
        if(pTxCpltCallbackImpl)
        {
            pTxCpltCallbackImpl(pETH_Handle);
        }
    }
    static void (*pErrorCallbackImpl)(ETH_HandleTypeDef* pETH_Handle);
    void HAL_ETH_ErrorCallback(ETH_HandleTypeDef* pETH_Handle)
    {
        // Called when a DMA or Ethernet error occur
        if(pErrorCallbackImpl)
        {
            pErrorCallbackImpl(pETH_Handle);
        }
    }

    // Buffer Management Callback: These functions are provided by the IP stack adaptator
    static void (*pRxBufferAllocateCallbackImpl)(uint8_t** buff);
    void HAL_ETH_RxAllocateCallback(uint8_t** buff)
    {
        // Called by ETH_UpdateDescriptor() to make reception ready for next packet
        if(pRxBufferAllocateCallbackImpl)
        {
            pRxBufferAllocateCallbackImpl(buff);
        }
    }
    static void (*pRxBufferLinkCallbackImpl)(void** pStart, void** pEnd, uint8_t* buff, uint16_t Length);
    void HAL_ETH_RxLinkCallback(void** pStart, void** pEnd, uint8_t* buff, uint16_t Length)
    {
        // Called by HAL_ETH_ReadData() to allow chainning multiple received buffers.
        if(pRxBufferLinkCallbackImpl)
        {
            pRxBufferLinkCallbackImpl(pStart, pEnd, buff, Length);
        }
    }
    static void (*pTxBufferFreeCallbackImpl)(uint32_t* buff);
    void HAL_ETH_TxFreeCallback(uint32_t* buff)
    {
        // Called by HAL_ETH_ReleaseTxPacket()
        if(pTxBufferFreeCallbackImpl)
        {
            pTxBufferFreeCallbackImpl(buff);
        }
    }

    // This function is called by IP stack Adaptator to signal that a free RX buffer has become available
    // and that reception can resume. This must be called only when there were no available buffers before and
    // now there are available buffers.
    static void (*pOnRxBufferAvailableImpl)(void);
    void OnRxBufferAvailable()
    {
        // Called by HAL_ETH_ReleaseTxPacket()
        if(pOnRxBufferAvailableImpl)
        {
            pOnRxBufferAvailableImpl();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          SystemState_e Initialize (void)
//  \brief       Initialize Ethernet MAC Device.
//  \param[in]   phyIfMode  The type of interface with the PHY (PHY_STATUS_RMII or PHY_STATUS_MII)
//  \return      \ref execution_status
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::Initialize(const ETH_MediaInterfaceTypeDef phyIfMode, PHY_DriverInterface* pPhy)
{
    uint8_t MAC_def[6];

    pIRQHandlerImpl          = &ETH_Driver::IRQHandlerImpl;
    pMspInitImpl             = &ETH_Driver::MspInit;
    pMspDeInitImpl           = &ETH_Driver::MspDeInit;

    pRxCpltCallbackImpl      = &ETH_Driver::RxCpltCallback;
    pTxCpltCallbackImpl      = &ETH_Driver::TxCpltCallback;
    pErrorCallbackImpl       = &ETH_Driver::ErrorCallback;

    pOnRxBufferAvailableImpl = ETH_Driver::SignalRxBufferAvailable;

    GetMAC_From_GUID(MAC_def);

    m_ETH_Handle.Instance            = ETH;
    m_ETH_Handle.Init.TxDesc         = DMATxDscrTab;
    m_ETH_Handle.Init.RxDesc         = DMARxDscrTab;
    m_ETH_Handle.Init.RxBuffLen      = ETH_MAX_PACKET_SIZE;
    m_ETH_Handle.Init.MACAddr        = (uint8_t*)MAC_def;
    m_ETH_Handle.Init.MediaInterface = phyIfMode;
    m_pPhy                           = pPhy;

    // create a binary semaphore used to sync ethernetif with frame reception
    RxPktSemaphore                   = xSemaphoreCreateBinary();

    // create a binary semaphore used to sync ethernetif with end of frame transmission
    TxPktSemaphore                   = xSemaphoreCreateBinary();

    if(HAL_ETH_Init(&m_ETH_Handle) != HAL_OK)
    {
        return SYS_ERROR;
    }

    // Configure the microcontroller's Ethernet MAC
    m_MAC_Config = {
        .SourceAddrControl               = ETH_SOURCEADDRESS_REPLACE_ADDR0, /*!< Selects the Source Address Insertion or Replacement Control. This parameter can be a value of @ref ETH_Source_Addr_Control */
        .ChecksumOffload                 = ENABLE,                          /*!< Enables or Disable the checksum checking for received packet payloads TCP, UDP or ICMP headers */
        .InterPacketGapVal               = ETH_INTERPACKETGAP_96BIT,        // Controls the minimum interframe gap between frames during transmission
        .GiantPacketSizeLimitControl     = DISABLE,                         /*!< Enables or disables the Giant Packet Size Limit Control. */
        .Support2KPacket                 = DISABLE,                         /*!< Enables or disables the IEEE 802.3as Support for 2K length Packets */
        .CRCStripTypePacket              = ENABLE,                          /*!< Enables or disables the CRC stripping for Type packets.*/
        .AutomaticPadCRCStrip            = DISABLE,                         /*!< Enables or disables  the Automatic MAC Pad/CRC Stripping.*/
        .Watchdog                        = ENABLE,                          // If enabled, limits frames to 2048 bytes maximum in receive FIFO (don't support jumbo frames)
        .Jabber                          = ENABLE,                          // If enabled, limits frames to 2048 bytes maximum in transmit FIFO (don't support jumbo frames)
        .JumboPacket                     = DISABLE,                         /*!< Enables or disables receiving Jumbo Packet When enabled, the MAC allows jumbo packets of 9,018 bytes without reporting a giant packet error */
        .Speed                           = ETH_SPEED_100M,                  /*!< Sets the Ethernet speed: 10/100 Mbps. This parameter can be a value of @ref ETH_Speed */
        .DuplexMode                      = ETH_FULLDUPLEX_MODE,             /*!< Selects the MAC duplex mode: Half-Duplex or Full-Duplex mode. This parameter can be a value of @ref ETH_Duplex_Mode */
        .LoopbackMode                    = DISABLE,                         /*!< Enables or disables the loopback mode */
        .CarrierSenseBeforeTransmit      = DISABLE,                         /*!< Enables or disables the Carrier Sense Before Transmission in Full Duplex Mode. */
        .ReceiveOwn                      = DISABLE,                         /*!< Enables or disables the Receive Own in Half Duplex mode. */
        .CarrierSenseDuringTransmit      = DISABLE,                         /*!< Enables or disables the Carrier Sense During Transmission in the Half Duplex mode */
        .RetryTransmission               = ENABLE,                          /*!< Enables or disables the MAC retry transmission, when a collision occurs in Half Duplex mode.*/
        .BackOffLimit                    = ETH_BACKOFFLIMIT_10,             /*!< Selects the BackOff limit value. This parameter can be a value of @ref ETH_Back_Off_Limit */
        .DeferralCheck                   = DISABLE,                         /*!< Enables or disables the deferral check function in Half Duplex mode. */
        .PreambleLength                  = ETH_PREAMBLELENGTH_7,            /*!< Selects or not the Preamble Length for Transmit packets (Full Duplex mode). This parameter can be a value of @ref ETH_Preamble_Length */
        .UnicastSlowProtocolPacketDetect = DISABLE,                         /*!< Enable or disables the Detection of Slow Protocol Packets with unicast address. */
        .SlowProtocolDetect              = DISABLE,                         /*!< Enable or disables the Slow Protocol Detection. */
        .CRCCheckingRxPackets            = DISABLE,                         /*!< Enable or disables the CRC Checking for Received Packets. */
        .GiantPacketSizeLimit            = 0x618U,                          /*!< Specifies the packet size that the MAC will declare it as Giant, If it's size is greater than the value programmed in this field in units of bytes This parameter must be a number between Min_Data = 0x618 (1518 byte) and Max_Data = 0x3FFF (32 Kbyte). */
        .ExtendedInterPacketGap          = DISABLE,                         /*!< Enable or disables the extended inter packet gap. */
        .ExtendedInterPacketGapVal       = 0x0U,                            /*!< Sets the Extended IPG between Packet during transmission. This parameter can be a value from 0x0 to 0xFF */
        .ProgrammableWatchdog            = DISABLE,                         /*!< Enable or disables the Programmable Watchdog.*/
        .WatchdogTimeout                 = ETH_MACWTR_WTO_2KB,              /*!< This field is used as watchdog timeout for a received packet. This parameter can be a value of @ref ETH_Watchdog_Timeout */
        .PauseTime                       = 0x0U,                            /*!< This field holds the value to be used in the Pause Time field in the transmit control packet. This parameter must be a number between Min_Data = 0x0 and Max_Data = 0xFFFF.*/
        .ZeroQuantaPause                 = ENABLE,                          /*!< Enable or disables the automatic generation of Zero Quanta Pause Control packets.*/
        .PauseLowThreshold               = ETH_PAUSELOWTHRESHOLD_MINUS_4,   /*!< This field configures the threshold of the PAUSE to be checked for automatic retransmission of PAUSE Packet. This parameter can be a value of @ref ETH_Pause_Low_Threshold */
        .TransmitFlowControl             = DISABLE,                         /*!< Enables or disables the MAC to transmit Pause packets in Full Duplex mode or the MAC back pressure operation in Half Duplex mode */
        .UnicastPausePacketDetect        = ENABLE,                          /*!< Enables or disables the MAC to detect Pause packets with unicast address of the station */
        .ReceiveFlowControl              = DISABLE,                         /*!< Enables or disables the MAC to decodes the received Pause packet and disables its transmitter for a specified (Pause) time */
        .TransmitQueueMode               = ETH_TRANSMITSTOREFORWARD,        /*!< Specifies the Transmit Queue operating mode. This parameter can be a value of @ref ETH_Transmit_Mode */
        .ReceiveQueueMode                = ETH_RECEIVESTOREFORWARD,         /*!< Specifies the Receive Queue operating mode. This parameter can be a value of @ref ETH_Receive_Mode */
        .DropTCPIPChecksumErrorPacket    = ENABLE,                          /*!< Enables or disables Dropping of TCPIP Checksum Error Packets. */
        .ForwardRxErrorPacket            = DISABLE,                         /*!< Enables or disables  forwarding Error Packets. */
        .ForwardRxUndersizedGoodPacket   = DISABLE                          /*!< Enables or disables  forwarding Undersized Good Packets.*/
    };

    // clang-format off
    m_MAC_Filter = {
        .PromiscuousMode                 = DISABLE, 
        .ReceiveAllMode                  = DISABLE, 
        .HachOrPerfectFilter             = ENABLE, 
        .HashUnicast                     = DISABLE, 
        .HashMulticast                   = DISABLE, 
        .PassAllMulticast                = ENABLE, 
        .SrcAddrFiltering                = DISABLE, 
        .SrcAddrInverseFiltering         = DISABLE, 
        .DestAddrInverseFiltering        = DISABLE, 
        .BroadcastFilter                 = DISABLE, 
        .ControlPacketsFilter            = ETH_CTRLPACKETS_BLOCK_ALL
    };
    // clang-format on

    m_Flags |= ETH_EMAC_FLAG_POWER;

    HAL_ETH_SetMACConfig(&m_ETH_Handle, &m_MAC_Config);
    HAL_ETH_SetMACFilterConfig(&m_ETH_Handle, &m_MAC_Filter);

    m_IsInitialized = true;
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          SystemState_e Deinitialize (void)
//  \brief       De-Initialize Ethernet MAC Device.
//  \param[in]   None
//  \return      \ref execution_status
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::Deinitialize(void)
{
    if(m_ETH_Handle.Instance != nullptr)
    {
        HAL_ETH_DeInit(&m_ETH_Handle);
    }

    m_Flags &= ~ETH_EMAC_FLAG_POWER;
    m_IsInitialized = false;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          RegisterStackCallback
//  \brief       Some functions must be provided by the IP Stack which uses the ethernet driver.
//               These functions are dealing with buffer allocation, chaining and freeing.
//  \param[in]   rxBuffAllocator    Function to allocate a data buffer for reception
//  \param[in]   rxBuffLink         Function to chain multiple receive buffers 
//  \param[in]   txBuffFree         Function to free allocate a data buffer for reception
//  \return      \ref execution_status
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::RegisterStackCallback(bufferAllocatorFct rxBuffAllocator, bufferLinkerFct rxBuffLink, bufferFreeFct txBuffFree)
{
    pRxBufferAllocateCallbackImpl = rxBuffAllocator;
    pRxBufferLinkCallbackImpl     = rxBuffLink;
    pTxBufferFreeCallbackImpl     = txBuffFree;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          int32_t GetMacAddress (ETH_MAC_Address_t* pAddress)
//  \brief       Get Ethernet MAC Address.
//  \param[in]   pAddress  Pointer to address
//  \return      \ref execution_status
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::GetMacAddress(ETH_MAC_Address_t* pAddress) const
{
    uint32_t Value;

    if(nullptr == pAddress)
    {
        return SYS_PARAMETER_ERROR;
    }
    else if((m_Flags & ETH_EMAC_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
    else if(false == m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }

    Value            = m_ETH_Handle.Instance->MACA0HR;
    pAddress->MAC[5] = (uint8_t)(Value >> 8);
    pAddress->MAC[4] = (uint8_t)(Value);
    Value            = m_ETH_Handle.Instance->MACA0LR;
    pAddress->MAC[3] = (uint8_t)(Value >> 24);
    pAddress->MAC[2] = (uint8_t)(Value >> 16);
    pAddress->MAC[1] = (uint8_t)(Value >> 8);
    pAddress->MAC[0] = (uint8_t)(Value);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          SystemState_e SetMacAddress (const ETH_MAC_Address_t* pAddress)
//  \brief       Set Ethernet MAC Address.
//  \param[in]   pAddress  Pointer to address
//  \return      \ref execution_status
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::SetMacAddress(const ETH_MAC_Address_t* pAddress)
{
    if(nullptr == pAddress)
    {
        return SYS_PARAMETER_ERROR;
    }
    else if((m_Flags & ETH_EMAC_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
    else if(false == m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }

    // Set Ethernet MAC Address registers
    m_ETH_Handle.Instance->MACA0HR = ((uint32_t)pAddress->MAC[5] << 8) | (uint32_t)pAddress->MAC[4];
    m_ETH_Handle.Instance->MACA0LR = ((uint32_t)pAddress->MAC[3] << 24) | ((uint32_t)pAddress->MAC[2] << 16) | ((uint32_t)pAddress->MAC[1] << 8) | (uint32_t)pAddress->MAC[0];

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          SystemState_e SetAddressFilter (const ETH_MAC_Address_t* pAddress,
//                                               uint32_t                 NumberOfAddress)
//  \brief       Configure Address Filter.
//  \param[in]   pAddress         Pointer to addresses
//  \param[in]   NumberOfAddress  Number of addresses to configure
//  \return      \ref execution_status
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::SetAddressFilter(const ETH_MAC_Address_t* pAddress, uint32_t NumberOfAddress)
{
    uint32_t   crc;
    CRC_Driver CRC_Block;

    if((pAddress == nullptr) && (NumberOfAddress != 0))
    {
        return SYS_PARAMETER_ERROR;
    }
    else if((m_Flags & ETH_EMAC_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
    else if(false == m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }

    // Use unicast address filtering for first 3 MAC addresses
    m_ETH_Handle.Instance->MACPFR &= ~(ETH_MACPFR_HPF | ETH_MACPFR_HMC);
    m_ETH_Handle.Instance->MACHT0R = 0;
    m_ETH_Handle.Instance->MACHT1R = 0;

    if(NumberOfAddress == 0)
    {
        m_ETH_Handle.Instance->MACA1HR = 0;
        m_ETH_Handle.Instance->MACA1LR = 0;
        m_ETH_Handle.Instance->MACA2HR = 0;
        m_ETH_Handle.Instance->MACA2LR = 0;
        m_ETH_Handle.Instance->MACA3HR = 0;
        m_ETH_Handle.Instance->MACA3LR = 0;
        return SYS_READY;
    }

    m_ETH_Handle.Instance->MACA1HR = ((uint32_t)pAddress->MAC[5] << 8) | (uint32_t)pAddress->MAC[4] | ETH_MACAHR_AE;
    m_ETH_Handle.Instance->MACA1LR = ((uint32_t)pAddress->MAC[3] << 24) | ((uint32_t)pAddress->MAC[2] << 16) | ((uint32_t)pAddress->MAC[1] << 8) | (uint32_t)pAddress->MAC[0];
    NumberOfAddress--;

    if(NumberOfAddress == 0)
    {
        m_ETH_Handle.Instance->MACA2HR = 0;
        m_ETH_Handle.Instance->MACA2LR = 0;
        m_ETH_Handle.Instance->MACA3HR = 0;
        m_ETH_Handle.Instance->MACA3LR = 0;
        return SYS_READY;
    }

    pAddress++;

    m_ETH_Handle.Instance->MACA2HR = ((uint32_t)pAddress->MAC[5] << 8) | (uint32_t)pAddress->MAC[4] | ETH_MACAHR_AE;
    m_ETH_Handle.Instance->MACA2LR = ((uint32_t)pAddress->MAC[3] << 24) | ((uint32_t)pAddress->MAC[2] << 16) | ((uint32_t)pAddress->MAC[1] << 8) | (uint32_t)pAddress->MAC[0];
    NumberOfAddress--;

    if(NumberOfAddress == 0)
    {
        m_ETH_Handle.Instance->MACA3HR = 0;
        m_ETH_Handle.Instance->MACA3LR = 0;
        return SYS_READY;
    }

    pAddress++;

    m_ETH_Handle.Instance->MACA3HR = ((uint32_t)pAddress->MAC[5] << 8) | (uint32_t)pAddress->MAC[4] | ETH_MACAHR_AE;
    m_ETH_Handle.Instance->MACA3LR = ((uint32_t)pAddress->MAC[3] << 24) | ((uint32_t)pAddress->MAC[2] << 16) | ((uint32_t)pAddress->MAC[1] << 8) | (uint32_t)pAddress->MAC[0];
    NumberOfAddress--;

    if(NumberOfAddress == 0)
    {
        return SYS_READY;
    }

    pAddress++;

    // Calculate 64-bit Hash table for remaining MAC addresses
    for(; NumberOfAddress; pAddress++, NumberOfAddress--)
    {
        crc = CRC_Block.CalculateBuffer((uint32_t*)&pAddress->MAC[0], 6U, CRC32_ETHERNET) >> 26;

        if(crc & 0x20U)
        {
            m_ETH_Handle.Instance->MACHT1R |= (1U << (crc & 0x1FU));
        }
        else
        {
            m_ETH_Handle.Instance->MACHT0R |= (1U << crc);
        }
    }

    // Enable both, unicast and hash address filtering
    m_ETH_Handle.Instance->MACPFR |= ETH_MACPFR_HPF | ETH_MACPFR_HMC;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          SystemState_e SendFrame(const uint8_t* frame, size_t Length, uint32_t Flags)
//  \brief       Send Ethernet frame.
//  \param[in]   frame   Pointer to frame buffer with data to send
//  \param[in]   Length  Frame buffer length in bytes
//  \param[in]   Flags   Frame transmit Flags (see ARM_ETH_MAC_TX_FRAME_...)
//  \return      \ref execution_status
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::SendFrame(ETH_BufferTypeDef* txBuffers, uint32_t len, void* appPtr)
{
    SystemState_e      errVal;
    ETH_TxPacketConfig txConfig;

    /* Set Tx packet config common parameters */
    memset(&txConfig, 0, sizeof(ETH_TxPacketConfig));
    txConfig.Attributes   = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    txConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    txConfig.CRCPadCtrl   = ETH_CRC_PAD_INSERT;

    txConfig.Length       = len;
    txConfig.TxBuffer     = txBuffers;
    txConfig.pData        = appPtr;

    do
    {
        if(HAL_ETH_Transmit_IT(&m_ETH_Handle, &txConfig) == HAL_OK)
        {
            errVal = SYS_ERROR_NONE;
        }
        else
        {
            if(HAL_ETH_GetError(&m_ETH_Handle) & HAL_ETH_ERROR_BUSY)
            {
                // Wait for descriptors to become available (2 seconds timeout)
                xSemaphoreTake(TxPktSemaphore, ETH_TX_TIMEOUT_MS);
                HAL_ETH_ReleaseTxPacket(&m_ETH_Handle);
                errVal = SYS_BUSY_TX;
            }
            else
            {
                /* Other error */
                errVal = SYS_UNRECOVERED_ERROR;
            }
        }
    } while(errVal == SYS_BUSY_TX);
    return errVal;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          SystemState_e ReadFrame (uint8_t *frame, uint32_t Length)
//  \brief       Read data of received Ethernet frame.
//  \param[in]   ppBuf   Address of a pointer to the buffer
//  \return      Receive status
//                 - SYS_READY indicates a frame is ready to be consummed
//                 - SYS_ERROR a timeout while waiting or other errors
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::ReadFrame(void** ppBuf)
{
    if(nullptr == ppBuf)
    {
        return SYS_PARAMETER_ERROR;
    }
    else if((m_Flags & ETH_EMAC_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
    else if(false == m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }

    if(HAL_OK == HAL_ETH_ReadData(&m_ETH_Handle, (void**)ppBuf))
    {
        return SYS_READY;
    }

    *ppBuf = nullptr;
    return SYS_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          WaitRxFrame
//  \brief       Wait for frame to be received.
//  \return      Receive status
//                 - SYS_ERROR when the ethernet driver is powered down
//                 - SYS_NOT_INITIALIZE when driver initialization is not
//                 - SYS_READY indicates a frame is ready to be consummed
//                 - SYS_TIME_OUT a timeout while waiting or other errors
//
//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::WaitRxFrame(uint32_t timeoutMs)
{
    if((m_Flags & ETH_EMAC_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
    else if(false == m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }

    if(xSemaphoreTake(RxPktSemaphore, timeoutMs) == pdTRUE)
    {
        return SYS_READY;
    }

    return SYS_TIME_OUT;
}

//-------------------------------------------------------------------------------------------------
SystemState_e ETH_Driver::ConfigurePhyIfMode(const PHY_Status_e Mode)
{
    if((Mode & PHY_STATUS_1_GBITS) != 0)
    {
        return SYS_UNSUPPORTED_FEATURE;
    }
    else if((m_Flags & ETH_EMAC_FLAG_POWER) == 0)
    {
        return SYS_ERROR;
    }
    else if(false == m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }

    // Read current configuration from device
    HAL_ETH_GetMACConfig(&m_ETH_Handle, &m_MAC_Config);
    if(((Mode & PHY_STATUS_SPEED_MASK) == 0) || ((Mode & PHY_STATUS_DUPLEX_MASK) == 0))
    {
        return SYS_ERROR;
    }

    // Update config "cache" and push new config to device
    m_MAC_Config.Speed      = ((Mode & PHY_STATUS_100_MBITS) == PHY_STATUS_100_MBITS) ? ETH_SPEED_100M : ETH_SPEED_10M;
    m_MAC_Config.DuplexMode = ((Mode & PHY_STATUS_FULL_DUPLEX) == PHY_STATUS_FULL_DUPLEX) ? ETH_FULLDUPLEX_MODE : ETH_HALFDUPLEX_MODE;
    HAL_ETH_SetMACConfig(&m_ETH_Handle, &m_MAC_Config);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          void GetMAC_From_GUID(uint8_t * pMAC)
//  \brief       Build MAC address from GUID.
//  \param[in]   uint8_t * pMAC
//  \return      \ref none
//
//  \note        1. 96-bit unique ID
//                  This 96-bit UID is unique for every STM32 ever made. These 96-bit unique IDs are
//                  created as a combination of unique fabrication parameters:
//
//                  UID[31:0]: X and Y coordinates on the wafer expressed in BCD format
//                  UID[63:40]: LOT_NUM[23:0] Lot number (ASCII encoded)
//                  UID[39:32]: WAF_NUM[7:0] Wafer number (8-bit unsigned number)
//                  UID[95:64]: LOT_NUM[55:24] Lot number (ASCII encoded)
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::GetMAC_From_GUID(uint8_t* pMAC)
{
    uint32_t uid[2];

    uid[0]  = HAL_GetUIDw0();
    uid[1]  = HAL_GetUIDw1();

    pMAC[0] = 0;
    pMAC[1] = uint8_t(uid[0]);
    pMAC[2] = uint8_t(uid[0] >> 8);
    pMAC[3] = uint8_t(uid[0] >> 16);
    pMAC[4] = uint8_t(uid[0] >> 24);
    pMAC[5] = uint8_t(uid[1]);
}

//-------------------------------------------------------------------------------------------------

SystemState_e ETH_Driver::MAC_Receiver(const FunctionalState state)
{
    if(false == m_IsInitialized)
    {
        return SYS_NOT_INITIALIZE;
    }

    if(ENABLE == state)
    {
        HAL_ETH_Start_IT(&m_ETH_Handle);  // Enable MAC receiver
    }
    else
    {
        HAL_ETH_Stop_IT(&m_ETH_Handle);  // Disable MAC receiver
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  \fn          void IRQHandlerImpl()
//  \brief       Handler for the global ethernet interrupt
//  \param[in]   ETH_HandleTypeDef *    Handle to the ethernet management structure
//  \return      \ref none
//
//  \note        Gets indirectly called by system-provided ETH_IRQHandler()
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::IRQHandlerImpl()
{
    HAL_ETH_IRQHandler(&(GetInstance().m_ETH_Handle));
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      MspInit
//
//   Return Value:  None
//
//   Description:   Initialize IO for the ETHERNET
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::MspInit(void)
{
    // Enable ETH, ETHRX and ETHTX clocks
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_ETHRXEN | RCC_AHB1ENR_ETHTXEN | RCC_AHB1ENR_ETHEN);

    IO_PinInit(IO_ETH_REF_CLK);
    IO_PinInit(IO_ETH_TXEN);
    IO_PinInit(IO_ETH_TXD0);
    IO_PinInit(IO_ETH_TXD1);
    IO_PinInit(IO_ETH_RXD0);
    IO_PinInit(IO_ETH_RXD1);
    IO_PinInit(IO_ETH_CRS_DV);
    IO_PinInit(IO_ETH_TXER);

    ISR_Init(ETH_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      MspDeInit
//
//   Return Value:  None
//
//   Description:   DeInitialize IO for the ETHERNET
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::MspDeInit(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      RxCpltCallback
//
//   Return Value:  None
//
//   Description:   Packet has been received and is ready to be processed by host application
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::RxCpltCallback(ETH_HandleTypeDef* pETH_Handle)
{
    UNUSED(pETH_Handle);
    xSemaphoreGiveFromISR(RxPktSemaphore, nullptr);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      RxCpltCallback
//
//   Return Value:  None
//
//   Description:   Packet has been sent and host application can perform whatever post processing
//                  such as release the tx buffers, etc...
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::TxCpltCallback(ETH_HandleTypeDef* pETH_Handle)
{
    UNUSED(pETH_Handle);
    xSemaphoreGiveFromISR(TxPktSemaphore, nullptr);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ErrorCallback
//
//   Return Value:  None
//
//   Description:   Ethernet DMA transfer error callback
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::ErrorCallback(ETH_HandleTypeDef* pETH_Handle)
{
    if((HAL_ETH_GetDMAError(pETH_Handle) & ETH_DMACSR_RBU) == ETH_DMACSR_RBU)
    {
        xSemaphoreGiveFromISR(RxPktSemaphore, nullptr);
    }

    if((HAL_ETH_GetDMAError(pETH_Handle) & ETH_DMACSR_TBU) == ETH_DMACSR_TBU)
    {
        xSemaphoreGiveFromISR(TxPktSemaphore, nullptr);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SignalRxBufferAvailable
//
//   Return Value:  None
//
//   Description:   Method called from the IP Stack when receive buffer were not available before and 
//                  becomes available.  This should wake up reception and rebuild the RX Descriptors.
//
//-------------------------------------------------------------------------------------------------
void ETH_Driver::SignalRxBufferAvailable(void)
{
    xSemaphoreGive(RxPktSemaphore);
}

