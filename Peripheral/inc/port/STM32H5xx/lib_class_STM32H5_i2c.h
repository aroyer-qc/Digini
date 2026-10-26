//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_i2c.h
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "I2C_cfg.h"

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define I2C_ANALOG_FILTER_ENABLE        0x00000000
#define I2C_ANALOG_FILTER_DISABLE       I2C_CR1_ANFOFF

// I2C Event
#define I2C_EVENT_TRANSFER_DONE         (uint32_t(1) << 0)  // Master/Slave Transmit/Receive finished
#define I2C_EVENT_TRANSFER_INCOMPLETE   (uint32_t(1) << 1)  // Master/Slave Transmit/Receive incomplete transfer
#define I2C_EVENT_ADDRESS_NACK          (uint32_t(1) << 2)  // Address not acknowledged from Slave
#define I2C_EVENT_ARBITRATION_LOST      (uint32_t(1) << 3)  // Master lost arbitration
#define I2C_EVENT_BUS_ERROR             (uint32_t(1) << 4)  // Bus error detected (START/STOP at illegal position)

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

enum I2C_Mode_t
{
    I2C_MODE_NONE               = 0x00,     // No I2C communication on going
    I2C_MODE_MASTER             = 0x10,     // I2C communication is in Master Mode
    I2C_MODE_SLAVE              = 0x20,     // I2C communication is in Slave Mode
    I2C_MODE_MEM                = 0x40      // I2C communication is in Memory Mode
};

struct I2C_Info_t
{
    I2C_ID_e            I2C_ID;
    I2C_TypeDef*        pI2Cx;
    IO_ID_e             SCL;
    IO_ID_e             SDA;
    uint32_t            Timing;
    uint32_t            AnalogFilter;
    uint32_t            DigitalFilter;
    uint8_t             PreempPrio;
    IRQn_Type           EV_IRQn;
    IRQn_Type           ER_IRQn;
};

typedef void (*I2C_EventCallBack_t)(uint32_t Event);

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class I2C_Driver
{
    public:

                        I2C_Driver              (I2C_ID_e I2C_ID);

        SystemState_e   GetStatus               (void);

        void            AbortTransfer           (void);

        SystemState_e   Transmit                (uint32_t Address, const uint8_t* pBuffer, size_t TxSize, bool IsItXfertPending);
        SystemState_e   Receive                 (uint32_t Address, uint8_t* pBuffer, size_t TxSize, bool IsItXfertPending);

        void            Initialize              (I2C_EventCallBack_t pEventCallBack);
        void            ER_IRQHandler           (void);
        void            EV_IRQHandler           (void);

    private:

        void            TransferConfig          (uint32_t Mode, uint32_t Request);
        void            ConvertOtherXferOptions (void);
        void            MasterCompleted         (uint32_t ITFlags);
        void            MasterSequenceCompleted (void);
        void            Flush_TXDR              (void);
        void            MasterSequenceError     (uint32_t ErrorCode);
        void            ErrorCallback           (void);

        I2C_ID_e                            m_I2C_ID;
        I2C_TypeDef*                        m_pI2Cx;
        const I2C_Info_t*                   m_pInfo;
        I2C_EventCallBack_t                 m_pEventCallBack;

        volatile uint32_t                   m_Address;
        volatile size_t                     m_XferCount;
        volatile size_t                     m_XferSize;
        volatile uint8_t*                   m_pXferBuffer;
        
        volatile uint32_t                   m_XferOptions;
        I2C_Mode_t                          m_Mode;

        volatile bool                       m_StatusAbort;
        volatile bool                       m_StatusBusy;
        volatile bool                       m_StatusXferNoStop;

        volatile SystemState_e              m_State;
        volatile SystemState_e              m_PreviousState;
        volatile uint16_t                   m_ErrorCodeFlags;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "i2c_var.h"

extern const I2C_Info_t I2C_Info[NB_OF_I2C_DRIVER];

//-------------------------------------------------------------------------------------------------

#endif // __cplusplus
