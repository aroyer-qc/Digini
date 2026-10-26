//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_i2c.cpp
//
//  Note(s) : No support for 10 Bit Address
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define I2C_DRIVER_GLOBAL
#include "BSP_board.h"
#undef  I2C_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// I2C Error Code definition
#define I2C_ERROR_NONE                  (0x0000)    // No error
#define I2C_ERROR_BERR                  (0x0001)    // BERR error
#define I2C_ERROR_ARLO                  (0x0002)    // ARLO error
#define I2C_ERROR_AF                    (0x0004)    // ACKF error
#define I2C_ERROR_OVR                   (0x0008)    // OVR error
#define I2C_ERROR_DMA                   (0x0010)    // DMA transfer error
#define I2C_ERROR_TIMEOUT               (0x0020)    // Timeout error
#define I2C_ERROR_SIZE                  (0x0040)    // Size Management error
#define I2C_ERROR_DMA_PARAM             (0x0080)    // DMA Parameter Error
#define I2C_ERROR_INVALID_CALLBACK      (0x0100)    // Invalid Callback error
#define I2C_ERROR_INVALID_PARAM         (0x0200)    // Invalid Parameters error
 

// N/U at this time no support for 10 Bits
//#define I2C_ADDRESS_10_BIT              (0x0400)  // 10-bit address flag
//#define I2C_ADDRESS_GENERAL_CALL        (0x8000)  // General Call flag

// Private define Sequential Transfer Options default/reset value
#define I2C_NO_OPTION_FRAME             (0xFFFF0000)
#define MAX_NBYTE_SIZE                  255
#define DEFAULT_CR1_CLEAR_MASK          (I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_START | I2C_CR2_STOP)

//-------------------------------------------------------------------------------------------------
//
//   Class: I2C_Driver
//
//   Description:   Class to handle I2C_Driver
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   I2C_Driver
//
//   Parameter(s):  I2C_ID_e
//
//   Description:   Initializes the I2C_ID peripheral according to the specified Parameters
//
//-------------------------------------------------------------------------------------------------
I2C_Driver::I2C_Driver(I2C_ID_e I2C_ID)
{
    m_I2C_ID = I2C_ID;
    m_pInfo  = &I2C_Info[I2C_ID];
    m_StatusBusy             = false;
    m_StatusAbort            = true;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   I2C_EventCallBack_t     pEventCallBack
//  Return:         none
//
//  Description:    It start the initialize process from configuration in struct
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::Initialize(I2C_EventCallBack_t pEventCallBack)
{
    m_pI2Cx          = m_pInfo->pI2Cx;
    m_pEventCallBack = pEventCallBack;

    NVIC_DisableIRQ(m_pInfo->EV_IRQn);
    NVIC_DisableIRQ(m_pInfo->ER_IRQn);

    // ---- GPIO configuration ----
    IO_PinInit(m_pInfo->SCL);
    IO_PinInit(m_pInfo->SDA);

    switch(uint32_t(m_I2C_ID))
    {
      #if (I2C_DRIVER_SUPPORT_I2C1_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C1_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1LRSTR |=  RCC_APB1LRSTR_I2C1RST;           // Enable I2C1 reset state
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_I2C1RST;           // Release I2C1 from reset state
            RCC->APB1LENR  |=  RCC_APB1LENR_I2C1EN;             // Enable I2C_PORT clock
        }
        break;
      #endif

      #if (I2C_DRIVER_SUPPORT_I2C2_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C2_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB1LRSTR |=  RCC_APB1LRSTR_I2C2RST;           // Enable I2C2 reset state
            RCC->APB1LRSTR &= ~RCC_APB1LRSTR_I2C2RST;           // Release I2C2 from reset state
            RCC->APB1LENR  |=  RCC_APB1LENR_I2C2EN;             // Enable I2C2_PORT clock
        }
        break;
      #endif

      #if (I2C_DRIVER_SUPPORT_I2C3_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C3_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB3RSTR |=  RCC_APB3RSTR_I2C3RST;             // Enable I2C3 reset state
            RCC->APB3RSTR &= ~RCC_APB3RSTR_I2C3RST;             // Release I2C3 from reset state
            RCC->APB3ENR  |=  RCC_APB3ENR_I2C3EN;               // Enable I2C3_PORT clock
        }
        break;
      #endif

      #if (I2C_DRIVER_SUPPORT_I2C4_CFG == DEF_ENABLED)
        case uint32_t(DRIVER_I2C4_ID):
        {
            // ---- Reset peripheral and set clock ----
            RCC->APB3RSTR |=  RCC_APB3RSTR_I2C4RST;             // Enable I2C4 reset state
            RCC->APB3RSTR &= ~RCC_APB3RSTR_I2C4RST;             // Release I2C4 from reset state
            RCC->APB3ENR  |=  RCC_APB3ENR_I2C4EN;               // Enable I2C4_PORT clock
        }
        break;
      #endif
    }

    m_pI2Cx->TIMINGR = m_pInfo->Timing;                                                             // Calculated using STM32CubeIDE

    // Set analog filter bit and digital filter coefficient
    MODIFY_REG(m_pI2Cx->CR1, (I2C_CR1_ANFOFF_Msk | I2C_CR1_DNF_Msk), (m_pInfo->AnalogFilter | (m_pInfo->DigitalFilter << I2C_CR1_DNF_Pos)));

    // ---- I2Cx CR1 Configuration ----
    m_pI2Cx->CR1 |= I2C_CR1_PE;                                                                     // Enable the selected I2C peripheral

    // ---- Enable I2C event interrupt ----
    ISR_Init(m_pInfo->EV_IRQn, m_pInfo->PreempPrio);                                                // NVIC Setup for interrupt request
    ISR_Init(m_pInfo->ER_IRQn, m_pInfo->PreempPrio);                                                // NVIC Setup for interrupt request
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      GetStatus
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Return general status of the driver
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::GetStatus(void)
{
    return m_State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Transmit
//
//   Parameter(s):  uint32_t    Address             Address of the device
//                  uint8_t*    pBuffer             Pointer on buffer to read data to transmit
//                  size_t      Size                Size of the data to send
//                  bool        IsItXfertPending    don't stop transfer at end of function
//   Return Value:  SystemState_e
//
//   Description:   Transmit data to the selected device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Transmit(uint32_t Address, const uint8_t* pBuffer, size_t Size, bool IsItXfertPending)
{
    SystemState_e Status;
    uint32_t      Option;
    uint32_t      XferRequest;
    uint32_t      XferMode;

    if((pBuffer == nullptr) || (Size == 0))
    {
        return SYS_PARAMETER_ERROR;
    }

    // Remove support N/U
    //if((Address & ~((uint32_t)I2C_ADDRESS_10_BIT | (uint32_t)I2C_ADDRESS_GENERAL_CALL)) > 0x3FF)
    //{
    //    return SYS_PARAMETER_ERROR;
    //}

    if(Size > UINT16_MAX)
    {
        return SYS_ERROR;                               // Does not handle 32-bit count in transfer
    }

    if(m_StatusBusy == true)
    {
        return SYS_BUSY_ERROR;
    }

    m_XferSize   = uint16_t(Size);
    m_Address    = Address << 1;
    m_StatusBusy = true;

    if(IsItXfertPending == true)
    {                                                   // If transfer should not generate STOP at the end
        if(m_StatusXferNoStop == false)
        {                                               // First transfer without STOP generation
            m_StatusXferNoStop = true;
            Option = I2C_FIRST_FRAME;
        }
        else
        {                                               // Any further transfer without STOP generation
            Option = I2C_OTHER_FRAME;
        }
    }
    else
    {                              
        if(m_StatusXferNoStop == true)                  // If transfer should generate STOP at the end
        {
            //If previous request did not generate STOP after transfer, terminate sequence
            m_StatusXferNoStop = false;
            Option = I2C_OTHER_AND_LAST_FRAME;
        }
        else
        {
            Option = I2C_LAST_FRAME;
        }
    }

    XferRequest = I2C_GENERATE_START_WRITE;

    if(m_State == SYS_READY)
    {
        m_State          = SYS_BUSY_TX;
        m_Mode           = I2C_MODE_MASTER;
        m_ErrorCodeFlags = SYS_ERROR_NONE;

        // Prepare transfer parameters
        m_pXferBuffer = (volatile uint8_t*)pBuffer;
        m_XferCount   = (uint16_t)Size;
        m_XferOptions = Option;

        // If hi2c->XferCount > MAX_NBYTE_SIZE, use reload mode
        if(m_XferCount > MAX_NBYTE_SIZE)
        {
            m_XferSize = MAX_NBYTE_SIZE;
            XferMode   = I2C_RELOAD_MODE;
        }
        else
        {
            m_XferSize = m_XferCount;
            XferMode   = m_XferOptions;
        }

        // If transfer direction not change and there is no request to start another frame, do not generate Restart Condition
        // Mean Previous state is same as current state
        if((m_PreviousState == SYS_BUSY_TX) && (IS_I2C_TRANSFER_OTHER_OPTIONS_REQUEST(Option) == 0))
        {
            XferRequest = I2C_NO_STARTSTOP;
        }
        else
        {
            // Convert OTHER_xxx XferOptions if any
            ConvertOtherXferOptions();

            // Update XferMode accordingly if no reload is necessary
            if(m_XferCount <= MAX_NBYTE_SIZE)
            {
                XferMode = m_XferOptions;
            }
        }

        TransferConfig(XferMode, XferRequest);                // Send slave address and set NBYTES to write
        SET_BIT(m_pI2Cx->CR1, I2C_CR1_TCIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_TXIE);
        Status = SYS_READY;
    }
    else
    {
        Status = SYS_ERROR;
        m_StatusBusy = false;
    }

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Receive
//
//   Parameter(s):  uint32_t    Address             Address of the device
//                  uint8_t*    pBuffer             Pointer on buffer to deposit receive data
//                  size_t      Size                Size of the data to read
//                  bool        IsItXfertPending    don't stop transfer at end of function
//   Return Value:  SystemState_e
//
//   Description:   Receive data from the selected device
//
//-------------------------------------------------------------------------------------------------
SystemState_e I2C_Driver::Receive(uint32_t Address, uint8_t* pBuffer, size_t Size, bool IsItXfertPending)
{
    SystemState_e Status;
    uint32_t      Option;
    uint32_t      XferRequest;
    uint32_t      XferMode;

    if((pBuffer == nullptr) || (Size == 0))
    {
        return SYS_PARAMETER_ERROR;
    }

    // Remove support N/U
    //if((Address & ~((uint32_t)I2C_ADDRESS_10_BIT | (uint32_t)I2C_ADDRESS_GENERAL_CALL)) > 0x3FF)
    //{
    //    return SYS_PARAMETER_ERROR;
    //}

    if(Size > UINT16_MAX)
    {
        return SYS_ERROR;                               // HAL does not handle 32-bit count in transfer
    }

    if(m_StatusBusy == true)
    {
        //return SYS_BUSY_ERROR;
    }

    while(m_StatusBusy == true){};


    m_XferSize   = uint16_t(Size);
    m_Address    = Address << 1;
    m_StatusBusy = true;

    if(IsItXfertPending == true)                    // If transfer should not generate STOP at the end
    {
        if(m_StatusXferNoStop == false)
        {  
            m_StatusXferNoStop = true;              // First transfer without STOP generation
            Option = I2C_FIRST_FRAME;
        }
        else
        {                                            // Any further transfer without STOP generation
            Option = I2C_OTHER_FRAME;
        }
    } 
    else
    {                                                // If transfer should generate STOP at the end
        if(m_StatusXferNoStop == true)
        {
            // If previous request did not generate STOP after transfer, terminate sequence
            m_StatusXferNoStop = false;
            Option = I2C_OTHER_AND_LAST_FRAME;
        }
        else
        {
            Option = I2C_LAST_FRAME;
        }
    }

    XferRequest = I2C_GENERATE_START_READ;

    if(m_State == SYS_READY)
    {
        m_State          = SYS_BUSY_RX;
        m_Mode           = I2C_MODE_MASTER;
        m_ErrorCodeFlags = I2C_ERROR_NONE;

        // Prepare transfer parameters
        m_pXferBuffer = pBuffer;
        m_XferCount   = Size;
        m_XferOptions = Option;

        // If hi2c->XferCount > MAX_NBYTE_SIZE, use reload mode
        if(m_XferCount > MAX_NBYTE_SIZE)
        {
            m_XferSize = MAX_NBYTE_SIZE;
            XferMode   = I2C_RELOAD_MODE;
        }
        else
        {
            m_XferSize = m_XferCount;
            XferMode   = m_XferOptions;
        }

        // If transfer direction not change and there is no request to start another frame, do not generate Restart Condition
        // Mean Previous state is same as current state
        if((m_PreviousState == SYS_BUSY_RX) && (IS_I2C_TRANSFER_OTHER_OPTIONS_REQUEST(Option) == 0))
        {
            XferRequest = I2C_NO_STARTSTOP;
        }
        else
        {
            // Convert OTHER_xxx XferOptions if any
            ConvertOtherXferOptions();

            // Update XferMode accordingly if no reload is necessary
            if(m_XferCount <= MAX_NBYTE_SIZE)
            {
                XferMode = m_XferOptions;
            }
        }

        TransferConfig(XferMode, XferRequest);                   // Send Slave Address and set NBYTES to read
        SET_BIT(m_pI2Cx->CR1, I2C_CR1_ERRIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_RXIE);
        Status = SYS_READY;
    }
    else
    {
        Status       = SYS_ERROR;
        m_StatusBusy = false;
    }

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      TransferConfig
//
//   Parameter(s):  Size    Specifies the number of bytes to be programmed.
//                              - This parameter must be a value between 0 and 255.
//                  Mode    New state of the I2C START condition generation.
//                              - This parameter can be one of the following values:
//                                 - I2C_RELOAD_MODE Enable Reload mode
//                                 - I2C_AUTOEND_MODE Enable Automatic end mode
//                                 - I2C_SOFTEND_MODE Enable Software end mode
//                  Request New state of the I2C START condition generation.
//                             - This parameter can be one of the following values:
//                                 - I2C_NO_STARTSTOP Don't Generate stop and start condition.
//                                 - I2C_GENERATE_STOP Generate stop (Size should be set to 0).
//                                 - I2C_GENERATE_START_READ Generate Restart for read request.
//                                 - I2C_GENERATE_START_WRITE Generate Restart for write request.
//   Return Value:  None
//
//   Description:   Handles I2Cx comm when starting transfer or during transfer
//                  (TC or TCR flag are set).
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::TransferConfig(uint32_t Mode, uint32_t Request)
{
    // Declaration of value to prevent undefined behavior of volatile usage
    uint32_t Value = ((m_Address & I2C_CR2_SADD_Msk)                            | 
                      ((m_XferSize << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES_Msk) |
                      uint32_t(Mode)                                            |
                      uint32_t(Request));

    // Update CR2 register
    MODIFY_REG(m_pI2Cx->CR2, DEFAULT_CR1_CLEAR_MASK | (I2C_CR2_RD_WRN & uint32_t(Request >> (31 - I2C_CR2_RD_WRN_Pos))), Value);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ConvertOtherXferOptions
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   If user set XferOptions to I2C_OTHER_FRAME
//                      - It request implicitly to generate a restart condition
//                      - Set XferOptions to I2C_FIRST_FRAME
//
//                  If user set XferOptions to I2C_OTHER_AND_LAST_FRAME
//                      - It request implicitly to generate a restart condition
//                      - Then generate a stop condition at the end of transfer
//                      - Set XferOptions to I2C_FIRST_AND_LAST_FRAME
//
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::ConvertOtherXferOptions(void)
{
    if(m_XferOptions == I2C_OTHER_FRAME)
    {
        m_XferOptions = I2C_FIRST_FRAME;
    }
    else if(m_XferOptions == I2C_OTHER_AND_LAST_FRAME)
    {
        m_XferOptions = I2C_FIRST_AND_LAST_FRAME;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      AbortTransfer
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Abort the transfer
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::AbortTransfer(void)
{
    /*  We don't use Slave mode.. kept for reference
    if(HAL_I2C_GetMode(i2c->h) == HAL_I2C_MODE_SLAVE)
    {
        SET_BIT(m_pI2Cx->CR2, I2C_CR2_NACK);              // Generate NACK when in slave mode
    }
    else if(HAL_I2C_GetMode (i2c->h) == HAL_I2C_MODE_MASTER)
    */
    {
        m_StatusAbort = false;

        // Disable Interrupts and store previous state
        if(m_State == SYS_BUSY_TX)
        {
            CLEAR_BIT(m_pI2Cx->CR1, (I2C_CR1_ERRIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_TXIE));
            m_PreviousState = m_State;
        }
        else if(m_State == SYS_BUSY_RX)
        {
            CLEAR_BIT(m_pI2Cx->CR1, (I2C_CR1_ERRIE | I2C_CR1_TCIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_RXIE));
            m_PreviousState = m_State;
        }

        m_State = SYS_ABORT;

        // Set NBYTES to 1 to generate a dummy read on I2C peripheral
        // Set AUTOEND mode, this will generate a NACK then STOP condition to abort the current transfer
        m_XferSize = 1;

        TransferConfig(I2C_AUTOEND_MODE, I2C_GENERATE_STOP);

        // Note : The I2C interrupts must be enabled to avoid the risk of I2C interrupt handle execution
        SET_BIT(m_pI2Cx->CR1, I2C_CR1_STOPIE);

        // Wait until abort operation complete
        while(m_StatusAbort == false){};
    }

    m_StatusBusy = false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Flush_TXDR
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   I2C Tx data register flush process.
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::Flush_TXDR(void)
{
    if((m_pI2Cx->ISR & I2C_ISR_TXIS) != 0)                  // (Transmit empty) If a pending TXIS flag is set, Write a dummy data in TXDR to clear it
    {
        m_pI2Cx->TXDR = 0;
    }

    if((m_pI2Cx->ISR & I2C_ISR_TXE) == 0)                   // This will flush the TXDR register
    {
       SET_BIT(m_pI2Cx->ISR, I2C_ISR_TXE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      MasterSequenceCompleted
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   I2C Master sequential complete process
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::MasterSequenceCompleted(void)
{
    // No Generate Stop, to permit restart mode
    // The stop will be done at the end of transfer, when I2C_AUTOEND_MODE enable
    if(m_State == SYS_BUSY_TX)
    {
        m_PreviousState = SYS_BUSY_TX;
        CLEAR_BIT(m_pI2Cx->CR1, (I2C_IT_TCI | I2C_IT_TXI));     // Disable TC and TXI interrupts
    }
    else    // SYS_BUSY_RX
    {
        m_PreviousState = SYS_BUSY_RX;
        CLEAR_BIT(m_pI2Cx->CR1, (I2C_IT_TCI | I2C_IT_RXI));     // Disable TC and RXI interrupts
    }

    m_StatusBusy = false;
    m_Mode  = I2C_MODE_NONE;                                     // Reset I2C handle mode
    m_State = SYS_READY;

    if(m_pEventCallBack != nullptr)
    {
        m_pEventCallBack(I2C_EVENT_TRANSFER_DONE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      MasterCompleted
//
//   Parameter(s):  uint32_t    ITFlags                 
//   Return Value:  None
//
//   Description:   Process when transaction is completed.
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::MasterCompleted(uint32_t ITFlags)
{
    uint32_t DummyRead;

    m_pI2Cx->ICR = I2C_ICR_STOPCF;                              // Clear STOP Flag

    if(m_State == SYS_BUSY_TX)                                  // Disable Interrupts and store previous state
    {
        CLEAR_BIT(m_pI2Cx->CR1, I2C_CR1_TCIE | I2C_CR1_TXIE);   // Disable TC and TXI interrupts
        m_PreviousState = m_State;
    }
    else if(m_State == SYS_BUSY_RX)
    {
        CLEAR_BIT(m_pI2Cx->CR1, I2C_CR1_TCIE | I2C_CR1_RXIE);   // Disable TC and TXI interrupts
        m_PreviousState = m_State;
    }

    CLEAR_BIT(m_pI2Cx->CR2, (I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN));     // Clear Configuration Register 2
    m_XferOptions = I2C_NO_OPTION_FRAME;

    if((ITFlags & I2C_ISR_NACKF) != 0)
    {
        m_pI2Cx->ICR = I2C_ICR_NACKCF;                          // Clear NACK Flag
        SET_BIT(m_ErrorCodeFlags, I2C_ERROR_AF);                // Set acknowledge error code
    }

    // Fetch Last receive data if any
    if((m_State == SYS_ABORT) && ((ITFlags & I2C_ISR_RXNE) != 0))
    {
        DummyRead = (uint8_t)m_pI2Cx->RXDR;                     // Read data from RXDR
        UNUSED(DummyRead);
    }

    Flush_TXDR();                                               // Flush TX register

      // Call the callback to inform upper layer of End of Transfer
    if((m_State == SYS_ABORT) || (m_ErrorCodeFlags != I2C_ERROR_NONE))
    {
        // Call the corresponding callback to inform upper layer of End of Transfer
        MasterSequenceError(m_ErrorCodeFlags);
    }
    else if((m_State == SYS_BUSY_TX) ||(m_State == SYS_BUSY_RX))
    {
        m_State         = SYS_READY;
        m_PreviousState = SYS_NONE;
        m_Mode          = I2C_MODE_NONE;
        m_StatusBusy    = false;

        if(m_pEventCallBack != nullptr)
        {
            m_pEventCallBack(I2C_EVENT_TRANSFER_DONE);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    MasterSequenceError
//
//  Parameter(s):   None
//  Return Value:   None
//
//  Description:    This function I2C interrupts error process..
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::MasterSequenceError(uint32_t ErrorCode)
{
    //HAL_I2C_StateTypeDef tmpstate = hi2c->State;

    // Reset handle parameters
    m_Mode        = I2C_MODE_NONE;
    m_XferOptions = I2C_NO_OPTION_FRAME;
    m_XferCount   = 0;
    SET_BIT(m_ErrorCodeFlags, ErrorCode);                                   // Set new error code
    CLEAR_BIT(m_pI2Cx->CR1, I2C_CR1_TCIE | I2C_CR1_TXIE | I2C_CR1_RXIE);    // Disable all interrupts
    Flush_TXDR();                                                           // Flush TX register

    // If state is an abort treatment on going, don't change state
    // This change will be do later
    if(m_State != SYS_ABORT)
    {
        m_State = SYS_READY;

        /* Check if a STOPF is detected */
        if((m_pI2Cx->ISR & I2C_ISR_STOPF) != 0)
        {
            if((m_pI2Cx->ISR & I2C_ISR_NACKF) != 0)
            {
                m_pI2Cx->ICR = I2C_ISR_NACKF;                           // Clear NACK Flag
                SET_BIT(m_ErrorCodeFlags, I2C_ERROR_AF);
            }

            m_pI2Cx->ICR = I2C_ISR_STOPF;                               // Clear STOP Flag
        }
    }
    
    m_PreviousState = SYS_NONE;

    if(m_State == SYS_ABORT)
    {
        m_State       = SYS_READY;
        m_StatusAbort = true;
    }
    else
    {
        ErrorCallback();
    }
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    ErrorCallback
//
//  Parameter(s):   None
//  Return Value:   None
//
//  Description:    This function Transfer error 
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::ErrorCallback(void)
{
    uint32_t Event;

    Event = I2C_EVENT_TRANSFER_DONE | I2C_EVENT_TRANSFER_INCOMPLETE;

    if((m_ErrorCodeFlags & I2C_ERROR_BERR) != 0)
    {
        SET_BIT(Event, I2C_EVENT_BUS_ERROR);                // Bus error
    }

    if((m_ErrorCodeFlags & I2C_ERROR_ARLO) != 0)
    {
        SET_BIT(Event, I2C_EVENT_ARBITRATION_LOST);         // Arbitration lost
    }

    if((m_ErrorCodeFlags & I2C_ERROR_AF) != 0)
    {
        if((m_XferCount == 0) && (m_XferSize > 0))          // Acknowledge not received
        {
            SET_BIT(Event, I2C_EVENT_ADDRESS_NACK);         // Slave address was not acknowledged
        }
    }

    m_StatusBusy = false;

    if(m_pEventCallBack != nullptr)
    {
        m_pEventCallBack(Event);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    EV_IRQHandler
//
//  Description:    This function handles I2Cx Event interrupt request.
//
//  Note(s):        Interrupt Sub-Routine which handle the interrupt flags master mode with
//                  interrupt.
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::EV_IRQHandler()
{
    uint32_t     InterruptFlags;
    uint32_t     InterruptSources;

    InterruptFlags   = m_pI2Cx->ISR;
    InterruptSources = m_pI2Cx->CR1;

    if(((InterruptFlags & I2C_ISR_NACKF) != 0) && ((InterruptSources & I2C_CR1_NACKIE) != 0))
    {
        m_pI2Cx->ICR = I2C_ICR_NACKCF;                          // Clear NACK Flag

        // Set corresponding Error Code
        // No need to generate STOP, it is automatically done
        // Error callback will be send during stop flag treatment
        SET_BIT(m_ErrorCodeFlags, I2C_ERROR_AF);
        Flush_TXDR();
    }
    else if(((InterruptFlags & I2C_ISR_RXNE) != 0) && ((InterruptSources & I2C_CR1_RXIE) != 0))
    {
        CLEAR_BIT(InterruptFlags, I2C_ISR_RXNE);                // Remove RXNE flag on temporary variable as read done

        *m_pXferBuffer = uint8_t(m_pI2Cx->RXDR);                // Read data from RXDR
        m_pXferBuffer++;                                        // Increment Buffer pointer
        m_XferSize--;           
        m_XferCount--;
    }
    else if(((InterruptFlags & I2C_ISR_TXIS) != 0) && ((InterruptSources & I2C_CR1_TXIE) != 0))
    {
        m_pI2Cx->TXDR = *m_pXferBuffer;                         // Write data to TXDR
        m_pXferBuffer++;                                        // Increment Buffer pointer
        m_XferSize--;
        m_XferCount--;
    }
    else if(((InterruptFlags & I2C_ISR_TCR) != 0) && ((InterruptSources & I2C_CR1_TCIE) != 0))
    {
        if((m_XferCount != 0) && (m_XferSize == 0))
        {
            if(m_XferCount > MAX_NBYTE_SIZE)
            {
                m_XferSize = MAX_NBYTE_SIZE;
                TransferConfig(I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
            }
            else
            {
                m_XferSize = m_XferCount;
                TransferConfig(((m_XferOptions != I2C_NO_OPTION_FRAME) ? m_XferOptions : I2C_AUTOEND_MODE), I2C_NO_STARTSTOP);
            }
        }
        else
        {
            if((m_pI2Cx->CR2 & I2C_CR2_AUTOEND) == 0)           // If no stop mode is set
            {
                MasterSequenceCompleted();                      // Call I2C Master Sequential complete process
            }
            else
            {
                // Wrong size Status regarding TCR flag event
                // Call the corresponding callback to inform upper layer of End of Transfer
                MasterSequenceError(I2C_ERROR_SIZE);
            }
        }
    }
    else if(((InterruptFlags & I2C_ISR_TC) != 0) && ((InterruptSources & I2C_CR1_TCIE) != 0))
    {
        if(m_XferCount == 0)
        {
            if((m_pI2Cx->CR2 & I2C_CR2_AUTOEND) == 0)
            {
                if(m_XferOptions == I2C_NO_OPTION_FRAME)        // Generate a stop condition in case of no transfer option
                {
                    m_pI2Cx->CR2 |= I2C_CR2_STOP;               // Generate Stop
                }
                else
                {
                    MasterSequenceCompleted();                  // Call I2C Master Sequential complete process
                }
            }
        }
        else
        {
            // Wrong size Status regarding TC flag event
            // Call the corresponding callback to inform upper layer of End of Transfer
            MasterSequenceError(HAL_I2C_ERROR_SIZE);
        }
    }

    if(((InterruptFlags & I2C_ISR_STOPF) != 0) && ((InterruptSources & I2C_CR1_STOPIE) != 0))              // I2C Master complete process
    {
        MasterCompleted(InterruptFlags);                        // Call I2C Master complete process
    }
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    ER_IRQHandler
//
//  Description:    This function is called in I2C_ER_IRQHandler
//
//  Note(s):        It handle transfer error
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::ER_IRQHandler()
{
    uint32_t Event;

    Event = I2C_EVENT_TRANSFER_DONE | I2C_EVENT_TRANSFER_INCOMPLETE;
  
    if((m_ErrorCodeFlags & I2C_ERROR_BERR) != 0)
    {
        SET_BIT(Event, I2C_EVENT_BUS_ERROR);                // Bus error
    }

    if((m_ErrorCodeFlags & I2C_ERROR_ARLO) != 0)
    {
        SET_BIT(Event, I2C_EVENT_ARBITRATION_LOST);         // Arbitration lost
    }

    if((m_ErrorCodeFlags & I2C_ERROR_AF) != 0)
    {
        if((m_XferCount == 0) && (m_XferSize > 0))          // Acknowledge not received
        {
            SET_BIT(Event, I2C_EVENT_ADDRESS_NACK);         // Slave address was not acknowledged
        }
    }

    m_StatusBusy = false;

    if(m_pEventCallBack != nullptr)
    {
        m_pEventCallBack(Event);
    }
}

//-------------------------------------------------------------------------------------------------

extern "C"
{
    void I2C1_EV_IRQHandler()
    {
        XBUS_I2C.EV_IRQHandler();
    }

    void I2C1_ER_IRQHandler()
    {
        XBUS_I2C.ER_IRQHandler();
    }
}

//-------------------------------------------------------------------------------------------------
