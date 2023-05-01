//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_i2c.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
// Email: aroyer.qc@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define I2C_ADDRESSING_MODE_7_BIT                   ((uint32_t)0x00000001)
#define I2C_ADDRESSING_MODE_10_BIT                  ((uint32_t)0x00000002)

#define I2C_ADDRESS_7_BITS                          ((uint32_t)0x00000001)
#define I2C_ADDRESS_10_BITS                         ((uint32_t)0x00000002)

#define I2C_GENERAL_CALL_DISABLE                    ((uint32_t)0x00000000)
#define I2C_GENERAL_CALL_ENABLE                     I2C_CR1_GCEN

#define I2C_NO_STRETCH_DISABLE                      ((uint32_t)0x00000000)
#define I2C_NO_STRETCH_ENABLE                       I2C_CR1_NOSTRETCH

#define I2C_DUAL_ADDRESS_DISABLE                    ((uint32_t)0x00000000)
#define I2C_DUAL_ADDRESS_ENABLE                     I2C_OAR2_OA2EN

#define I2C1_SELECT_PLCK1                           ((uint32_t)0x00000000)
#define I2C1_SELECT_SYS_CLK                         ((uint32_t)RCC_DCKCFGR2_I2C1SEL_0)
#define I2C1_SELECT_HSI                             ((uint32_t)RCC_DCKCFGR2_I2C1SEL_1)

#define I2C2_SELECT_PLCK1                           ((uint32_t)0x00000000)
#define I2C2_SELECT_SYS_CLK                         ((uint32_t)RCC_DCKCFGR2_I2C2EL_0)
#define I2C2_SELECT_HSI                             ((uint32_t)RCC_DCKCFGR2_I2C2SEL_1)

#define I2C3_SELECT_PLCK1                           ((uint32_t)0x00000000)
#define I2C3_SELECT_SYS_CLK                         ((uint32_t)RCC_DCKCFGR2_I2C3SEL_0)
#define I2C3_SELECT_HSI                             ((uint32_t)RCC_DCKCFGR2_I2C3SEL_1)

#define I2C4_SELECT_PLCK1                           ((uint32_t)0x00000000)
#define I2C4_SELECT_SYS_CLK                         ((uint32_t)RCC_DCKCFGR2_I2C4SEL_0)
#define I2C4_SELECT_HSI                             ((uint32_t)RCC_DCKCFGR2_I2C4SEL_1)



//toDO !!
// Callback type in bit position
#define I2C_CALLBACK_NONE                  0x000
#define I2C_CALLBACK_MASTER_TX_COMPLETED   0x001
#define I2C_CALLBACK_MASTER_RX_COMPLETED   0x002
#define I2C_CALLBACK_SLAVE_TX_COMPLETED    0x004
#define I2C_CALLBACK_SLAVE_RX_COMPLETED    0x008
#define I2C_CALLBACK_ADDRESS               0x010
#define I2C_CALLBACK_LISTEN_COMPLETED      0x020
#define I2C_CALLBACK_ERROR                 0x100
#define I2C_CALLBACK_ABORT                 0x200

#if (I2C_ISR_MASTER_TX_CFG == DEF_ENABLED) || (I2C_ISR_MASTER_RX_CFG == DEF_ENABLED)  || \
    (I2C_ISR_SLAVE_TX_CFG == DEF_ENABLED)  || (I2C_ISR_SLAVE_RX_CFG == DEF_ENABLED)   || \
   (I2C_ISR_ADRRESS_CFG == DEF_ENABLED)   || (I2C_ISR_TX_LISTEN_CFG == DEF_ENABLED)  || \
    (I2C_ISR_ERROR_CFG == DEF_ENABLED)     || (I2C_ISR_ABORT_CFG == DEF_ENABLED)
    #define I2C_ISR_CFG                     DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum I2C_ID_e
{
    #if (I2C_DRIVER_SUPPORT_I2C1_CFG == DEF_ENABLED)
        DRIVER_I2C1_ID,
    #endif

    #if (I2C_DRIVER_SUPPORT_I2C2_CFG == DEF_ENABLED)
        DRIVER_I2C2_ID,
    #endif

    #if (I2C_DRIVER_SUPPORT_I2C3_CFG == DEF_ENABLED)
        DRIVER_I2C3_ID,
    #endif

    #if (I2C_DRIVER_SUPPORT_I2C4_CFG == DEF_ENABLED)
        DRIVER_I2C4_ID,
    #endif

    NB_OF_I2C_DRIVER,
};

// new WIP
struct I2C_Info_t
{
    I2C_ID_e            I2C_ID;
    I2C_TypeDef*        pI2Cx;
    IO_ID_e             SCL;
    IO_ID_e             SDA;
    uint32_t            ClockSelection;
    uint32_t            RCC_APB1_En;
    uint32_t            Timing;              // I2C_TIMINGR_register value. use STM32Cube to calculate this value or refer to I2C initialization section in Reference manual.
  #if (I2C_DRIVER_SUPPORT_ADVANCED_MODE_CFG == DEF_ENABLED)
    uint32_t            AddressingMode;      // Select the address mode. This parameter can be I2C_ADDRESSING_MODE_7_BIT or I2C_ADDRESSING_MODE_10_BIT.
    uint32_t            DualAddressMode;     // dual addressing mode selection. This parameter can be I2C_DUAL_ADDRESS_ENABLE or I2C_DUAL_ADDRESS_DISABLE.
    uint32_t            OwnAddress_1;        // First device own address and it can be a 7-bit or 10-bit address.
    uint32_t            OwnAddress_2;        // Second device own address. Dual addressing mode must be selected. It can 7-bit address.
    uint32_t            OwnAddress2Masks;    // Acknowledge mask address second device own address if dual addressing mode is selected, this parameter can be a value of @ref I2C_OWN_ADDRESS2_MASKS
    uint32_t            GeneralCallMode;     // General call mode selection. Can be I2C_GENERAL_CALL_ENABLE or I2C_GENERAL_CALL_DISABLE
    uint32_t            NoStretchMode;       // no stretch mode selection. Can be I2C_NO_STRETCH_ENABLE or I2C_NO_STRETCH_DISABLE
  #endif
    uint8_t             PreempPrio;
    IRQn_Type           EV_IRQn;
    IRQn_Type           ER_IRQn;
};

#if 0
// transfert info... should be in class

   I2C_InitTypeDef            Init;           /*!< I2C communication parameters              */
    uint8_t                    *pBuffPtr;      /*!< Pointer to I2C transfer buffer            */
    uint16_t                   XferSize;       /*!< I2C transfer size                         */
    __IO uint16_t              XferCount;      /*!< I2C transfer counter                      */
    __IO uint32_t              XferOptions;    /*!< I2C sequantial transfer options, this parameter can
                                                  be a value of @ref I2C_XFEROPTIONS */

    __IO uint32_t              PreviousState;  /*!< I2C communication Previous state          */
    HAL_StatusTypeDef(*XferISR)(struct __I2C_HandleTypeDef *hi2c, uint32_t ITFlags, uint32_t ITSources);  /*!< I2C transfer IRQ handler function pointer */
    DMA_HandleTypeDef          *hdmatx;        /*!< I2C Tx DMA handle parameters              */
    DMA_HandleTypeDef          *hdmarx;        /*!< I2C Rx DMA handle parameters              */
    HAL_LockTypeDef            Lock;           /*!< I2C locking object                        */
    __IO HAL_I2C_StateTypeDef  State;          /*!< I2C communication state                   */
    __IO HAL_I2C_ModeTypeDef   Mode;           /*!< I2C communication mode                    */
    __IO uint32_t              ErrorCode;      /*!< I2C Error code                            */
    __IO uint32_t              AddrEventCount; /*!< I2C Address Event counter                 */
};
#endif

typedef struct
{

} I2C_InitTypeDef;


//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class I2C_Driver
{
    public:

                        I2C_Driver          (I2C_ID_e I2C_ID);

        SystemState_e   LockToDevice        (uint8_t Device);       // Set I2C to this device and lock
        SystemState_e   UnlockFromDevice    (uint8_t Device);       // Unlock I2C from device
        SystemState_e   GetStatus           (void);

        // Support master IT and DMA
        SystemState_e   Transfer            (uint32_t Address, uint32_t AddressSize, const void* pTxBuffer, size_t TxSize, const void* pRxBuffer, size_t RxSize);
        SystemState_e   Transfer            (uint32_t Address, uint32_t AddressSize, const void* pTxBuffer, size_t TxSize, const void* pRxBuffer, size_t RxSize, uint8_t Device);

        // Support slave IT and DMA
        SystemState_e   SlaveTransfer       (uint32_t Address, uint32_t AddressSize, const void* pTxBuffer, size_t TxSize, const void* pRxBuffer, size_t RxSize);
        SystemState_e   SlaveTransfer       (uint32_t Address, uint32_t AddressSize, const void* pTxBuffer, size_t TxSize, const void* pRxBuffer, size_t RxSize, uint8_t Device);

        // Support memory IT and DMA
        SystemState_e   Write               (const void* pBuffer, size_t Size, uint8_t Device);
        SystemState_e   Write               (const void* pBuffer, size_t Size);
        SystemState_e   Read                (const void* pBuffer, size_t Size);
        SystemState_e   Read                (const void* pBuffer, size_t Size, uint8_t Device);
        SystemState_e   ReadRegister        (uint8_t Register, uint8_t* pValue, uint8_t Device);
        SystemState_e   ReadRegister        (uint8_t Register, uint8_t* pValue);
        SystemState_e   WriteRegister       (uint8_t Register, uint8_t Value, uint8_t Device);
        SystemState_e   WriteRegister       (uint8_t Register, uint8_t Value);

 void InterruptMasterComplete(uint32_t State);

      #if (UART_DRIVER_DMA_CFG == DEF_ENABLED)
// reference
//        void                DMA_ConfigRX                    (uint8_t* pBufferRX, size_t SizeRX);
//        void                DMA_ConfigTX                    (uint8_t* pBufferTX, size_t SizeTX);
//        void                DMA_EnableRX                    (void);
//        void                DMA_DisableRX                   (void);
//        void                DMA_EnableTX                    (void);
//        void                DMA_DisableTX                   (void);
//        size_t              DMA_GetSizeRX                   (uint16_t SizeRX);
      #endif

      #if (I2C_ISR_CFG == DEF_ENABLED)
        void                RegisterCallback                (CallbackInterface* pCallback);
        void                EnableCallbackType              (int CallbackType, void* pContext = nullptr);
      #endif

        void            Initialize          (void);

        // SystemState_e GetMode            (void);
        // SystemState_e GetError           (void);

        void            ER_IRQHandler       (void);
        void            EV_IRQHandler       (void);

    private:

        void            Lock                (void);
        void            Unlock              (void);

        I2C_Info_t*                         m_pInfo;
        I2C_TypeDef*                        m_pI2Cx;
        nOS_Mutex                           m_Mutex;
        int16_t                             m_Device;

        volatile bool                       m_IsItInitialize;
        volatile uint32_t                   m_Address;
        volatile size_t                     m_AddressSize;
        volatile size_t                     m_TxSize;
        volatile size_t                     m_RxSize;
        volatile uint8_t*                   m_pTxBuffer;
        volatile uint8_t*                   m_pRxBuffer;

        volatile uint8_t*                   m_pAddressInDevice;
        volatile uint8_t*                   m_pDataAddress;
        volatile size_t                     m_Size;
        volatile size_t                     m_AddressLengthCount;

        volatile SystemState_e              m_State;
        volatile uint8_t                    m_Timeout;

      #if (I2C_ISR_CFG == DEF_ENABLED)
       #if (I2C_ISR_MASTER_TX_CFG == DEF_ENABLED)
        void*                               m_pContextMasterTX;
       #endif

       #if (I2C_ISR_MASTER_RX_CFG == DEF_ENABLED)
        void*                               m_pContextMasterRX;
       #endif

       #if (I2C_ISR_SLAVE_TX_CFG == DEF_ENABLED)
        void*                               m_pContextSlaveTX;
       #endif

       #if (I2C_ISR_SLAVE_RX_CFG == DEF_ENABLED)
        void*                               m_pContextSlaveRX;
       #endif

       #if (I2C_ISR_ADRRESS_CFG == DEF_ENABLED)
        void*                               m_pContextAddress;
       #endif

       #if (I2C_ISR_TX_LISTEN_CFG == DEF_ENABLED)
        void*                               m_pContextListen;
       #endif

        #if (I2C_ISR_ERROR_CFG == DEF_ENABLED)
        void*                               m_pContextError;
       #endif

       #if (I2C_ISR_ABORT_CFG == DEF_ENABLED)
        void*                               m_pContextAbort;
       #endif
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "i2c_var.h"

//-------------------------------------------------------------------------------------------------

#endif // (USE_I2C_DRIVER == DEF_ENABLED)
