//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H7_ospi.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define OSPI_DRIVER_GLOBAL
#include "./lib_digini.h"
#undef  OSPI_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_OSPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Class: OSPI_Driver
//
//
//   Description:   Class to handle OSPI
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private Constants and Macros
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   OSPI_Driver
//
//   Parameter(s):  None
//
//   Description:   Initializes the OSPI peripheral according to the specified Parameters
//
//   Note(s):       OSPI_InitStruct: pointer to a OSPI_InitTypeDef structure that contains
//                  the configuration information for the specified OSPI peripheral.
//
//-------------------------------------------------------------------------------------------------
OSPI_Driver::OSPI_Driver()
{
    m_IsItInitialize  = false;
//    m_KickAutoPolling = false;
//    m_IsItBusy        = false;
//    m_State           = OSPI_STATE_RESET;
//    m_ErrorCode       = SYS_READY;
//    m_DMA_ErrorCode   = SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Initialize the OSPI module
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Initialize(void)
{
    nOS_Error       Error;
    TickCount_t     TickStart;
    SystemState_e   State = SYS_READY;

    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;
        Error = nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
        VAR_UNUSED(Error);
    }

  #if (OSPI_SUPPORT_FLASH_RESET_PIN == DEF_ENABLED)
    IO_PinInit(IO_FLASH_RESET);                                     // Init pin reset for Flash at low level (RESET state)
  #endif

    // Enable clock on OSPI module and DMA
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    RCC->AHB3ENR |= RCC_AHB3ENR_OSPIEN;

    // Reset the QuadSPI memory interface
    RCC->AHB3RSTR |=   RCC_AHB3RSTR_OSPIRST;
    RCC->AHB3RSTR &= ~(RCC_AHB3RSTR_OSPIRST);

    return State;                                                                   // Return function status
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Read
//
//   Parameter(s):  Address     Read start address
//                  pBuffer     Pointer to data to be read
//                  Size        Size of data to read
//   Return Value:  SystemState_e
//
//   Description:   Reads an amount of data from the OSPI memory.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Read(uint32_t Address, uint8_t* pBuffer, uint32_t Size)
{
    SystemState_e State = SYS_BUSY;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Write
//
//   Parameter(s):  Address     Write start address
//                  pBuffer     Pointer to data to be written
//                  Size        Size of data to write
//   Return Value:  SystemState_e
//
//   Description:   Write an amount of data to the OSPI memory.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Write(uint32_t Address, uint8_t* pBuffer, uint32_t Size)
{
    SystemState_e State = SYS_BUSY;
    
    this->Lock();
    this->Unlock();
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  ****  ****  * *   *   *  ***** *****
//  *   * *   * * *   *  * *   *   *
//  ****  ****  * *   * *   *  *   ****
//  *     *  *  *  * *  *****  *   *
//  *     *   * *   *   *   *  *   *****
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       WaitReady
//
//  Parameter(s):   Timeout      Duration of the time out
//  Return Value:
//
//  Description:    Wait for the OSPI module to be ready.
//
//  Note(s):        return  SYS_READY     if available
//                          SYS_TIME_OUT  if busy pass timeout
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::WaitReady(uint32_t TimeOut)
{
    SystemState_e State = SYS_READY;
    TickCount_t TickStart = GetTick();

    while(((QUADSPI->SR & QUADSPI_SR_BUSY) != 0) && (State == SYS_READY))   // Wait until flag busy is cleared or time out
    {
        nOS_Yield();

        if(TickHasTimeOut(TickStart, TimeOut) == true)
        {
            State = SYS_TIME_OUT;
        }
    }

    while((m_State != OSPI_STATE_READY) && (State == SYS_READY))            // Wait until flag is in expected state
    {
        if(TimeOut != OSPI_MAX_DELAY)                                       // Check for the Timeout
        {
            if(TickHasTimeOut(TickStart, TimeOut) == true)
            {
                State = SYS_TIME_OUT;
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       WaitReady
//
//  Parameter(s):   None
//  Return Value:   SYS_READY
//
//  Description:    Wait for the write enable to be ready, stay in if busy.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::WriteEnable(void)
{
    SystemState_e State;

    do
    {
        // Enable write operations
        if((State = this->Command(OSPI_CMD_WRITE_ENABLE_QUAD, 0, 0, OSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
        {
            this->WaitReady(OSPI_TIMEOUT_SHORT_TIMEOUT_VALUE);
        }
    }
    while(State == SYS_BUSY);

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Config
//
//  Parameter(s):   CmdID            The command ID for structure that contains configuration information.
//                  Address          Address to send.
//                  Size             Size of the data
//                  FunctionalMode   Functional mode to configured
//                                      This parameter can be one of the following values:
//                                          OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE:    Indirect write mode
//                                          OSPI_FUNCTIONAL_MODE_AUTO_POLLING:      Automatic polling mode
//  Return Value:   None
//
//  Description:    Configure the communication registers.
//
//-------------------------------------------------------------------------------------------------
void OSPI_Driver::Config(OSPI_CmdList_e CmdID, uint32_t Address, uint32_t Size, uint32_t FunctionalMode)
{
  #if (OSPI_READ_DISCOVERY_PARAMETER == DEF_ENABLED)
    uint8_t DummyCycles;
  #endif

    OSPI_Command_t* pCmd = (OSPI_Command_t*)&m_Cmd[CmdID];

    QUADSPI->FCR = (QUADSPI_FCR_CTEF | QUADSPI_FCR_CTCF);               // Clear interrupt: Transmit error and transmit complete

    if(pCmd->DataMode != OSPI_DATA_NONE)
    {
        QUADSPI->DLR = Size - 1;                                        // Configure OSPI: DLR register with the number of data to read or write
    }

  #if (OSPI_READ_DISCOVERY_PARAMETER == DEF_ENABLED)
    DummyCycles = (CmdID == OSPI_CMD_READ_DISCOVERY_PARAMETER) ? m_DiscoveryDummyCycles : pCmd->DummyCycles;
  #endif

    uint32_t Config = (OSPI_COMMON_CONFIG    |
                       pCmd->DataMode        |
                       pCmd->Instruction     |
                       pCmd->InstructionMode |
                       pCmd->AddressMode     |
                       FunctionalMode        |
                     #if (OSPI_READ_DISCOVERY_PARAMETER == DEF_ENABLED)
                       (DummyCycles << QUADSPI_CCR_DCYC_Pos));
                     #else
                       (pCmd->DummyCycles << QUADSPI_CCR_DCYC_Pos));
                     #endif

    //---- Preset command with common config ----
    if(pCmd->AddressMode != OSPI_ADDRESS_NONE)
    {
        //---- Command with instruction and address ----
        // Configure OSPI: CCR register with all communications parameters
        QUADSPI->CCR = (Config | OSPI_ADDRESS_32_BITS);
        QUADSPI->AR  = Address;                                         // Configure OSPI: AR register with address value
    }
    else
    {
        //---- Command with only instruction ----
        // Configure OSPI: CCR register with all communications parameters
        QUADSPI->CCR = Config;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       ReadCommand
//
//  Parameter(s):   CmdID        The command ID for structure that contains configuration information.
//                  Address      Address in flash for this command
//                  pData        Pointer on buffer to return data
//                  Size         Size of data to read
//                  Timeout      Timeout duration
//  Return Value:   SystemState_e
//
//  Description:    Read data from specific command.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::ReadCommand(OSPI_CmdList_e CmdID, uint32_t Address, uint8_t* pData, uint32_t Size, TickCount_t TimeOut)
{
    SystemState_e State;

    if(pData != nullptr)
    {
        if((State = this->Command(CmdID, Address, Size, TimeOut)) == SYS_READY)
        {
            if((State = this->DMA_Receive(pData, Size)) == SYS_READY)
            {
                // Wait operation to be completed as Status is modified in DMA
                State = this->WaitReady(OSPI_TIMEOUT_READ_COMMAND);
            }
        }
    }
    else
    {
        State = SYS_READY;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       WriteCommand
//
//  Parameter(s):   CmdID        The command ID for structure that contains configuration information.
//                  Address      Address in flash for this command
//                  pData        Pointer on buffer to return data
//                  Size         Size of data to read
//                  Timeout      Timeout duration
//  Return Value:   SystemState_e
//
//  Description:    Write data for specific command.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::WriteCommand(OSPI_CmdList_e CmdID, uint32_t Address, uint8_t* pData, uint32_t Size, TickCount_t TimeOut)
{
    SystemState_e State;
    TickCount_t   TickStart;

    if(pData != nullptr)
    {
        TickStart = GetTick();

        if((State = this->Command(CmdID, Address, Size, TimeOut)) == SYS_READY)
        {
            if((State = this->DMA_Transmit(pData, Size)) == SYS_READY)
            {
                while((this->WaitFlashReady() == SYS_BUSY) && (State != SYS_TIME_OUT))
                {
                    nOS_Yield();

                    if(TickHasTimeOut(TickStart, TimeOut) == true)
                    {
                        State = SYS_TIME_OUT;
                    }
                }

                // Wait operation to be completed as Status is modified in DMA
                State = this->WaitReady(OSPI_TIMEOUT_SHORT_TIMEOUT_VALUE);
            }
        }
    }
    else
    {
        State = SYS_ERROR;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Command
//
//  Parameter(s):   CmdID        The command ID for structure that contains configuration information.
//                  Address      Address in flash for this command
//                  Size         Size of data to read
//                  Timeout      Timeout duration
//  Return Value:   SystemState_e
//
//  Description:    Set the command configuration.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Command(OSPI_CmdList_e CmdID, uint32_t Address, uint32_t Size, TickCount_t Timeout)
{
    SystemState_e State;

    if(m_State == OSPI_STATE_READY)
    {
        m_ErrorCode = SYS_READY;
        m_State     = OSPI_STATE_BUSY;                                          // Update OSPI state
        State = this->WaitFlagStateUntilTimeout(QUADSPI_SR_BUSY, 0, Timeout);   // Wait till BUSY flag reset

        if(State == SYS_READY)
        {
            this->Config(CmdID,
                         Address,
                         Size,
                         OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);                  // Call the configuration function

            if(m_Cmd[CmdID].DataMode == OSPI_DATA_NONE)
            {
                State = this->WaitFlagStateUntilTimeout(QUADSPI_SR_TCF,
                                                        1,
                                                        OSPI_TIMEOUT_DEFAULT_VALUE);

                if(State == SYS_READY)
                {
                    m_State = OSPI_STATE_READY;                                 // Update OSPI state
                }
            }
            else
            {
                m_State = OSPI_STATE_READY;                                     // Update OSPI state
            }
        }
    }
    else
    {
        State = SYS_BUSY;
    }

    return State;                                                               // Return function status
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Command_IT
//
//  Parameter(s):   CmdID        The command ID for structure that contains configuration information.
//                  Address      Address in flash for this command
//                  Size         Size of data to read
//  Return Value:   SystemState_e
//
//  Description:    Set the command configuration in interrupt mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Command_IT(OSPI_CmdList_e CmdID, uint32_t Address, uint32_t Size)
{
    SystemState_e   State;
    OSPI_DataMode_e DataMode  = m_Cmd[CmdID].DataMode;

    if(m_State == OSPI_STATE_READY)
    {
        m_ErrorCode = SYS_READY;
        m_State     = OSPI_STATE_BUSY;                                          // Update OSPI state
        State = this->WaitFlagStateUntilTimeout(QUADSPI_SR_BUSY,
                                                0,
                                                OSPI_TIMEOUT_DEFAULT_VALUE);    // Wait till BUSY flag reset
        if(State == SYS_READY)
        {
            if(DataMode == OSPI_DATA_NONE)
            {
                SET_BIT(QUADSPI->FCR, (QUADSPI_FCR_CTEF | QUADSPI_FCR_CTCF));   // Clear interrupt: Transmit error and transmit complete
            }

            this->Config(CmdID,
                         Address,
                         Size,
                         OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);                  // Call the configuration function

            if(DataMode == OSPI_DATA_NONE)
            {
                // When there is no data phase, the transfer start as soon as the configuration is done so activate TC and TE interrupts
                SET_BIT(QUADSPI->CR, QUADSPI_CR_TEIE | QUADSPI_CR_TCIE);        // Enable the OSPI Transfer Error Interrupt
            }
            else
            {
                m_State = OSPI_STATE_READY;                                     // Update OSPI state
            }
        }
    }
    else
    {
        State = SYS_BUSY;
    }

    return State;                                                               // Return function status
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Abort_IT
//
//  Parameter(s):   None
//  Return Value:   SystemState_e
//
//  Description:    Abort the current transmission (non-blocking function)
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Abort_IT(void)
{
    SystemState_e State = SYS_READY;

    if((m_State & OSPI_STATE_BUSY) != 0)                                // Check if the state is in one of the busy states
    {
        m_State = OSPI_STATE_ABORT;                                     // Update OSPI state
        CLEAR_BIT(QUADSPI->CR, (QUADSPI_CR_TOIE |
                                QUADSPI_CR_SMIE |
                                QUADSPI_CR_FTIE |
                                QUADSPI_CR_TCIE |
                                QUADSPI_CR_TEIE));                      // Disable all interrupts

        if(READ_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN) != 0)
        {
            CLEAR_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN);                   // Disable the DMA transfer by clearing the DMAEN bit in the OSPI CR register

            if(this->DMA_Abort() != SYS_READY)
            {
                m_ErrorCode = SYS_ERROR;                                // Set error code to DMA
                State       = SYS_ERROR;
            }
        }
        else
        {
            QUADSPI->FCR = QUADSPI_FCR_CTCF;                            // Clear interrupt
            SET_BIT(QUADSPI->CR, (QUADSPI_CR_TCIE |                     // Enable the OSPI Transfer Complete Interrupt and
                                  QUADSPI_CR_ABORT));                   // Configure OSPI with Abort request
        }
    }
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):
//   Return Value:
//
//   Description:   Lock the driver
//
//-------------------------------------------------------------------------------------------------
void OSPI_Driver::Lock(void)
{
    while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK) {};
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Unlock
//
//   Parameter(s):
//   Return Value:
//
//   Description:   Unlock the driver
//
//-------------------------------------------------------------------------------------------------
void OSPI_Driver::Unlock(void)
{
    nOS_MutexUnlock(&m_Mutex);
}

/**********************************************************************************************************************************
 * @brief           This function handles QUADSPI interrupt request.
 *
 * @param           None
 *
 * @return          None
 *
 *********************************************************************************************************************************/
extern "C" void QUADSPI_IRQHandler(void)
{
    uint32_t Flag      = QUADSPI->SR;
    uint32_t IT_Source = QUADSPI->CR;

    // OSPI Transfer Complete interrupt occurred -------------------------------
    if(((Flag & QUADSPI_SR_TCF) != 0) && ((IT_Source & QUADSPI_CR_TCIE) != 0))
    {
        QUADSPI->FCR = QUADSPI_FCR_CTCF;                            // Clear interrupt
        CLEAR_BIT(QUADSPI->CR, (QUADSPI_CR_TCIE |
                                QUADSPI_CR_TEIE ));                 // Disable the OSPI Transfer Error and Transfer complete Interrupts

        // Transfer complete callback
        if(OSPI.m_State == OSPI_STATE_BUSY_INDIRECT_TX)
        {
            if(READ_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN) != 0)
            {
                CLEAR_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN);           // Disable the DMA transfer by clearing the DMAEN bit in the OSPI CR register
                DMA2_Stream7->CR &= ~DMA_SxCR_EN;                   // Disable the DMA channel
            }

            OSPI.m_State = OSPI_STATE_READY;                        // Change state of OSPI
            OSPI.m_KickAutoPolling = true;                          // When transfer is completed
        }
        else if(OSPI.m_State == OSPI_STATE_BUSY_INDIRECT_RX)
        {
            if(READ_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN) != 0)
            {
                CLEAR_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN);           // Disable the DMA transfer by clearing the DMAEN bit in the OSPI CR register
                DMA2_Stream7->CR &= ~DMA_SxCR_EN;                   // Disable the DMA channel
            }

            OSPI.m_State = OSPI_STATE_READY;                        // Change state of OSPI
        }
        else if(OSPI.m_State == OSPI_STATE_BUSY)
        {
            OSPI.m_State = OSPI_STATE_READY;                        // Change state of OSPI
        }
        else if(OSPI.m_State == OSPI_STATE_ABORT)
        {
            CLEAR_BIT(QUADSPI->CCR, QUADSPI_CCR_FMODE);             // Reset functional mode configuration to indirect write mode by default
            OSPI.m_State = OSPI_STATE_READY;                        // Change state of OSPI

            if(OSPI.m_ErrorCode == SYS_READY)
            {
                // Abort called by the user
                //OSPI.AbortCpltCallback();                           // Abort Complete callback
            }
            else
            {
                // Abort due to an error (eg :  DMA error)
                //OSPI.ErrorCallback();                              // Error callback

            }
        }
    }

    // OSPI Status Match interrupt occurred ------------------------------------
    else if(((Flag & QUADSPI_SR_SMF) != 0) && ((IT_Source & QUADSPI_CR_SMIE) != 0))
    {
        QUADSPI->FCR = QUADSPI_FCR_CSMF;                            // Clear interrupt

        if(READ_BIT(QUADSPI->CR, QUADSPI_CR_APMS) != 0)             // Check if the automatic poll mode stop is activated
        {
            CLEAR_BIT(QUADSPI->CR, (QUADSPI_CR_SMIE |               // Disable Status Match Interrupts
                                    QUADSPI_CR_TEIE));              //    and Transfer Error
            OSPI.m_State = OSPI_STATE_READY;                        // Change state of OSPI
        }
    }

    // OSPI Transfer Error interrupt occurred ---------------------------------
    else if(((Flag & QUADSPI_SR_TEF) != 0) && ((IT_Source & QUADSPI_CR_TEIE) != 0))
    {
        QUADSPI->FCR = QUADSPI_FCR_CTEF;                            // Clear interrupt
        CLEAR_BIT(QUADSPI->CR, (QUADSPI_CR_SMIE |
                                QUADSPI_CR_TCIE |
                                QUADSPI_CR_TEIE |
                                QUADSPI_CR_FTIE));                  // Disable all the OSPI Interrupts
        OSPI.m_ErrorCode = SYS_TRANSFER_ERROR;                      // Set error code

        if(READ_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN) != 0)
        {
            CLEAR_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN);               // Disable the DMA transfer by clearing the DMAEN bit in the OSPI CR register
            //OSPI.XferAbortCallback = OSPI.DMA_AbortCplt;

            if(OSPI.DMA_Abort_IT() != SYS_READY)
            {
                OSPI.m_ErrorCode = SYS_DMA_ERROR;                   // Set error code to DMA
                OSPI.m_State     = OSPI_STATE_READY;                // Change state of OSPI
              //  OSPI.ErrorCallback();                               // Error callback
            }
        }
        else
        {
            OSPI.m_State = OSPI_STATE_READY;                        // Change state of OSPI
            //OSPI.ErrorCallback();                                  // Error callback
        }
    }
}

//-------------------------------------------------------------------------------------------------
//  DDDDDD  MM     MM  AAAAA
//  DD   DD MMMM MMMM AA   AA
//  DD   DD MM MMM MM AAAAAAA
//  DD   DD MM  M  MM AA   AA
//  DDDDDD  MM     MM AA   AA
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Transmit
//
//  Parameter(s):   pData        Pointer to data buffer.
//                  Size         Size of the DMA TX.
//  Return Value:   SystemState_e
//
//  Description:    Send an amount of data in non-blocking mode with DMA.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::DMA_Transmit(uint8_t* pData, uint32_t Size)
{
    SystemState_e State = SYS_READY;

    if(m_State == OSPI_STATE_READY)
    {
        m_ErrorCode = SYS_READY;                        // Clear the error code
        m_State     = OSPI_STATE_BUSY_INDIRECT_TX;      // Update state
        MODIFY_REG(QUADSPI->CCR,
                   QUADSPI_CCR_FMODE,
                   OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);
        SET_BIT(QUADSPI->CR,
                (QUADSPI_CR_TEIE |                      // Enable the OSPI transfer error
                 QUADSPI_CR_TCIE));                     // Enable TC Interrupts: Workaround for OSPI low kernel clock frequency
        MODIFY_REG(DMA2_Stream7->CR,
                   DMA_SxCR_DIR_Msk,
                   OSPI_DMA_WRITE_DIRECTION);           // DMA Direction : memory to peripheral
        this->DMA_Start(pData, Size + 1);
        SET_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN);
    }
    else
    {
        State = SYS_BUSY;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Receive
//
//  Parameter(s):   pData        Pointer to data buffer.
//                  Size         Size to received.
//  Return Value:   SystemState_e
//
//  Description:    Receive an amount of data in non-blocking mode with DMA.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::DMA_Receive(uint8_t *pData, uint32_t Size)
{
    SystemState_e State;

    if(m_State == OSPI_STATE_READY)
    {
        m_ErrorCode = SYS_READY;                                            // Clear the error code
        m_State     = OSPI_STATE_BUSY_INDIRECT_RX;                          // Update state

        // Note: OSPI need to be configured to indirect mode before starting the DMA to avoid premature triggering for the DMA transfer
        MODIFY_REG(QUADSPI->CCR,                                            // Configure OSPI: CCR register with functional as indirect read
                   QUADSPI_CCR_FMODE,
                   OSPI_FUNCTIONAL_MODE_INDIRECT_READ);
        MODIFY_REG(DMA2_Stream7->CR,                                        // DMA Direction : peripheral to memory
                   DMA_SxCR_DIR_Msk,
                   OSPI_DMA_READ_DIRECTION);

        if((State = this->DMA_Start(pData, Size + 1)) == SYS_READY)
        {
            SET_BIT(QUADSPI->CR, (QUADSPI_CR_TEIE | QUADSPI_CR_TCIE));      // Enable the OSPI transfer error and transmit complete interrupt
            SET_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN);                         // Enable the OSPI DMA
            QUADSPI->AR = QUADSPI->AR;                                      // Start the transfer by re-writing the address in AR register
        }

        return State;
    }

    return SYS_BUSY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Start
//
//  Parameter(s):   pData               Data buffer to transfer
//                  BlockDataLength     The length of a block transfer in bytes.
//  Return Value:   SystemState_e
//
//  Description:    Starts the DMA Transfer with interrupts enabled.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::DMA_Start(uint8_t* pData, uint32_t BlockDataLength)
{
    m_DMA_ErrorCode = SYS_READY;                                // Initialize the error code
    DMA2_Stream7->NDTR = BlockDataLength;                       // Configure the DMA Channel data length
    DMA2->HIFCR = (DMA_HIFCR_CTEIF7 | DMA_HIFCR_CTCIF7);        // Clear interrupt flags
    DMA2_Stream7->M0AR = (uint32_t)pData;                       // Configure DMA Channel Source address
    DMA2_Stream7->CR |= DMA_SxCR_EN;                            // Enable the Peripheral

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Abort
//
//  Parameter(s):   pData               Data buffer to transfer
//                  BlockDataLength     The length of a block transfer in bytes.
//  Return Value:   SystemState_e
//
//  Description:    Aborts the DMA Transfer.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::DMA_Abort(void)
{
    TickCount_t TickStart =  GetTick();

    DMA2_Stream7->CR &= ~(DMA_SxCR_TEIE |                                   // Disable all the transfer interrupts
                          DMA_SxCR_EN);                                     // Disable the channel

    while((DMA2_Stream7->CR & DMA_SxCR_EN) != 0)                            // Check if the DMA Channel is effectively disabled
    {
        if(TickHasTimeOut(TickStart, OSPI_TIMEOUT_DMA_ABORT) == true)
        {
            m_DMA_ErrorCode = SYS_TIME_OUT;                                 // Update error code
            return SYS_ERROR;
        }
    }

    DMA2->HIFCR = DMA_HIFCR_CTEIF7;                                         // Clear all interrupt flags

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Abort_IT
//
//  Parameter(s):   None
//  Return Value:   SystemState_e
//
//  Description:    Aborts the DMA Transfer in Interrupt mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::DMA_Abort_IT(void)
{
    DMA2_Stream7->CR &= ~DMA_SxCR_EN;             // Disable the DMA channel
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA2_Stream7_IRQHandler
//
//  Parameter(s):   None
//  Return Value:   None
//
//  Description:    handle the DMA IRQ for OSPI.
//
//-------------------------------------------------------------------------------------------------
extern "C" NOS_ISR(DMA2_Stream7_IRQHandler)
{
    uint32_t Count = 0;
    uint32_t TimeOut = SYSTEM_CORE_CLOCK / 9600U;
    uint32_t Flag;

    Flag = DMA2->HISR;

    // Transfer Error Interrupt management
    if((Flag & DMA_HISR_TEIF7) != 0)
    {
        if((DMA2_Stream7->CR & QUADSPI_CR_TEIE) != 0)
        {
            DMA2_Stream7->CR &= ~QUADSPI_CR_TEIE;               // Disable the transfer error interrupt
            OSPI.m_DMA_ErrorCode = SYS_DMA_ERROR;               // Update error code : Read Transfer error
            DMA2->HIFCR = DMA_HIFCR_CTEIF7;                     // Clear the transfer error flags
        }
    }

    if(OSPI.m_DMA_ErrorCode != SYS_READY)                       // Manage error case
    {
        DMA2_Stream7->CR &= ~DMA_SxCR_EN;                       // Disable the channel

        do
        {
            if(++Count > TimeOut)
            {
                break;
            }
        }
        while((DMA2_Stream7->CR & DMA_SxCR_EN) != 0);

        OSPI.DMA_Error();
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_Error
//
//  Parameter(s):   None
//  Return Value:   None
//
//  Description:    DMA OSPI communication error callback.
//
//-------------------------------------------------------------------------------------------------
void OSPI_Driver::DMA_Error(void)
{
    m_ErrorCode = SYS_DMA_ERROR;
    CLEAR_BIT(QUADSPI->CR, QUADSPI_CR_DMAEN);       // Disable the DMA transfer by clearing the DMAEN bit in the OSPI CR register
    this->Abort_IT();                               // Abort the OSPI
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       DMA_AbortCplt
//
//  Parameter(s):   None
//  Return Value:   None
//
//  Description:    DMA OSPI abort complete callback.
//
//-------------------------------------------------------------------------------------------------
void OSPI_Driver::DMA_AbortCplt(void)
{
    if(m_State == OSPI_STATE_ABORT)                     // DMA Abort called by OSPI abort
    {
        QUADSPI->FCR = QUADSPI_FCR_CTCF;                // Clear interrupt
        SET_BIT(QUADSPI->CR, QUADSPI_CR_TCEN);          // Enable the OSPI Transfer Complete Interrupt
        SET_BIT(QUADSPI->CR, QUADSPI_CR_ABORT);         // Configure OSPI: CR register with Abort request
    }
    else                                                // DMA Abort called due to a transfer error interrupt
    {
        m_State = OSPI_STATE_READY;                     // Change state of OSPI
        //this->ErrorCallback();                          // Error callback
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_OSPI_DRIVER == DEF_ENABLED)



































#if 0


//-------------------------------------------------------------------------------------------------
//
//   Function:      MemoryMapped
//
//   Parameter(s):  Command    Structure that contains the command configuration information
//                  Config     Structure that contains the memory mapped configuration information.
//   Return Value:  SystemState_e
//
//   Description:   Configure the Memory Mapped mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::MemoryMapped(OSPI_CommandTypeDef* Command, OSPI_MemoryMappedTypeDef* Config)
{
    SystemState_e State;

    State = this->WaitReady(OSPI_FLAG_BUSY, 0, 100);   // TODO found the right timeout

    if(State == SYS_READY)
    {
        // Configure OSPI: CR register with time out counter enable
        QUADSPI->CR &= ~(uint32_t)QuadSPI_CR_TCEN;
        QUADSPI->CR = Config->TimeOutActivation);

        if(Config->TimeOutActivation == OSPI_TIMEOUT_COUNTER_ENABLE)
        {
            // Configure OSPI: LPTR register with the low-power time out value
            QUADSPI->LPTR = Config->TimeOutPeriod;

            // Enable the OSPI TimeOut Interrupt
            QUADSPI->CR |= OSPI_IT_TO;
        }

        // Call the configuration function
        this->Config(Command, OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED);
    }
    else
    {
        State = SYS_BUSY;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Config
//
//   Parameter(s):  Command         Structure that contains the command configuration information
//                  FunctionalMode  Functional mode to configured
//   Return Value:  None
//
//   Description:   This function configures the communication registers
//
//-------------------------------------------------------------------------------------------------
void OSPI_Driver::Config(OSPI_CommandTypeDef *Command, uint32_t FunctionalMode)
{
    if((Command->DataMode != OSPI_DATA_NONE) && (FunctionalMode != OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED))
    {
        // Configure OSPI: DLR register with the number of data to read or write
        QUADSPI->DLR, (Command->NbData - 1);
    }

    if(Command->InstructionMode != OSPI_INSTRUCTION_NONE)
    {
        if(Command->AlternateByteMode != OSPI_ALTERNATE_BYTES_NONE)
        {
            // Configure OSPI: ABR register with alternate bytes value
            QUADSPI->ABR, Command->AlternateBytes);

            if(Command->AddressMode != OSPI_ADDRESS_NONE)
            {
                //---- Command with instruction, address and alternate bytes ----
                // Configure OSPI: CCR register with all communications parameters
                QUADSPI->CCR, (Command->DdrMode           | Command->DdrHoldHalfCycle    | Command->SIOOMode           |
                               Command->DataMode          | (Command->DummyCycles << 18) | Command->AlternateBytesSize |
                               Command->AlternateByteMode | Command->AddressSize         | Command->AddressMode        |
                               Command->InstructionMode   | Command->Instruction         | FunctionalMode));

                if(FunctionalMode != OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
                {
                    // Configure OSPI: AR register with address value
                    QUADSPI->AR, Command->Address;
                }
            }
            else
            {
                //---- Command with instruction and alternate bytes ----
                // Configure OSPI: CCR register with all communications parameters
                QUADSPI->CCR, (Command->DdrMode           | Command->DdrHoldHalfCycle    | Command->SIOOMode           |
                               Command->DataMode          | (Command->DummyCycles << 18) | Command->AlternateBytesSize |
                               Command->AlternateByteMode | Command->AddressMode         | Command->InstructionMode    |
                               Command->Instruction       | FunctionalMode));
            }
        }
        else
        {
            if(Command->AddressMode != OSPI_ADDRESS_NONE)
            {
                //---- Command with instruction and address ----
                // Configure OSPI: CCR register with all communications parameters
                QUADSPI->CCR, (Command->DdrMode     | Command->DdrHoldHalfCycle    | Command->SIOOMode          |
                               Command->DataMode    | (Command->DummyCycles << 18) | Command->AlternateByteMode |
                               Command->AddressSize | Command->AddressMode         | Command->InstructionMode   |
                               Command->Instruction | FunctionalMode));

                if(FunctionalMode != OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
                {
                    // Configure OSPI: AR register with address value
                    QUADSPI->AR, Command->Address;
                }
            }
            else
            {
                //---- Command with only instruction ----
                // Configure OSPI: CCR register with all communications parameters
                QUADSPI->CCR, (Command->DdrMode     | Command->DdrHoldHalfCycle    | Command->SIOOMode          |
                               Command->DataMode    | (Command->DummyCycles << 18) | Command->AlternateByteMode |
                               Command->AddressMode | Command->InstructionMode     | Command->Instruction       |
                               FunctionalMode));
            }
        }
    }
    else
    {
        if(Command->AlternateByteMode != OSPI_ALTERNATE_BYTES_NONE)
        {
            // Configure OSPI: ABR register with alternate bytes value
            QUADSPI->ABR, Command->AlternateBytes;

            if(Command->AddressMode != OSPI_ADDRESS_NONE)
            {
                //---- Command with address and alternate bytes ----
                // Configure OSPI: CCR register with all communications parameters
                QUADSPI->CCR, (Command->DdrMode           | Command->DdrHoldHalfCycle    | Command->SIOOMode           |
                               Command->DataMode          | (Command->DummyCycles << 18) | Command->AlternateBytesSize |
                               Command->AlternateByteMode | Command->AddressSize         | Command->AddressMode        |
                               Command->InstructionMode   | FunctionalMode));

                if(FunctionalMode != OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
                {
                    // Configure OSPI: AR register with address value
                    QUADSPI->AR, Command->Address;
                }
            }
            else
            {
                //---- Command with only alternate bytes ----
                // Configure OSPI: CCR register with all communications parameters
                QUADSPI->CCR, (Command->DdrMode           | Command->DdrHoldHalfCycle    | Command->SIOOMode           |
                               Command->DataMode          | (Command->DummyCycles << 18) | Command->AlternateBytesSize |
                               Command->AlternateByteMode | Command->AddressMode         | Command->InstructionMode    |
                               FunctionalMode));
            }
        }
        else
        {
            if(Command->AddressMode != OSPI_ADDRESS_NONE)
            {
                //---- Command with only address ----
                // Configure OSPI: CCR register with all communications parameters
                QUADSPI->CCR, (Command->DdrMode     | Command->DdrHoldHalfCycle    | Command->SIOOMode          |
                               Command->DataMode    | (Command->DummyCycles << 18) | Command->AlternateByteMode |
                               Command->AddressSize | Command->AddressMode         | Command->InstructionMode   |
                               FunctionalMode));

                if(FunctionalMode != OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED)
                {
                    // Configure OSPI: AR register with address value
                    QUADSPI->AR, Command->Address;
                }
            }
            else
            {
                //---- Command with only data phase ----
                if(Command->DataMode != OSPI_DATA_NONE)
                {
                    // Configure OSPI: CCR register with all communications parameters
                    QUADSPI->CCR, (Command->DdrMode     | Command->DdrHoldHalfCycle    | Command->SIOOMode          |
                                   Command->DataMode    | (Command->DummyCycles << 18) | Command->AlternateByteMode |
                                   Command->AddressMode | Command->InstructionMode     | FunctionalMode));
                }
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Command
//
//   Parameter(s):  Command     Structure that contains the command configuration information
//                  Timeout     Time out duration
//
//   Return Value:  SystemState_e
//
//   Description:   This function is used only in Indirect Read or Write Modes
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Command(OSPI_CommandTypeDef* Command, uint32_t Timeout)
{
    SystemState_e State;

    // Wait till BUSY flag reset
    State = this->WaitReady(OSPI_FLAG_BUSY, 0, Timeout)

    if(State == SYS_READY)
    {
        // Call the configuration function
        this->Config(Command, OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

        if(Command->DataMode == OSPI_DATA_NONE)
        {
            // When there is no data phase, the transfer start as soon as the configuration is done
            // so wait until TC flag is set to go back in idle state
            if(this->WaitReady(OSPI_FLAG_TC, 1, Timeout) != SYS_READY)
            {
                State = SYS_TIME_OUT;
            }
            else
            {
                QUADSPI->FCR = OSPI_FLAG_TC;
            }
        }
    }

  return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Receive
//
//   Parameter(s):  pData       Pointer to data buffer.
//
//   Return Value:  SystemState_e
//
//   Description:   Receives an amount of data in non blocking mode with DMA.
//
//   Note(s):       This function is used only in Indirect Read Mode
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Receive(uint8_t *pData)
{
    SystemState_e State;
    uint32_t *tmp;
    //uint32_t addr_reg = READ_REG(QUADSPI->AR);

    if(pData != nullptr)
    {
        //remove that crap!!
        // Configure counters and size of the handle
        hqspi->RxXferCount = READ_REG(hqspi->Instance->DLR) + 1;
        hqspi->RxXferSize = READ_REG(hqspi->Instance->DLR) + 1;
        hqspi->pRxBuffPtr = pData;
        //hqspi->hdma->XferCpltCallback = OSPI_DMARxCplt;         // Set the OSPI DMA transfer complete callback
        //hqspi->hdma->XferHalfCpltCallback = OSPI_DMARxHalfCplt; // Set the OSPI DMA Half transfer complete callback
        //hqspi->hdma->XferErrorCallback = OSPI_DMAError;         // Set the DMA error callback

        m_pDMA->CR &= ~(uint32_t)DMA_SxCR_DIR;
        m_pDMA->CR |=  DMA_PERIPH_TO_MEMORY;                   // Configure the direction of the DMA

        tmp = (uint32_t*)&pData;        // Enable the DMA Channel
        DMA_Start_IT(hqspi->hdma, (uint32_t)&QUADSPI->DR, *(uint32_t*)tmp, hqspi->RxXferSize);

        QUADSPI->CCR &= ~(uint32_t)QuadSPI_CCR_FMODE;
        QUADSPI->CCR |=  OSPI_FUNCTIONAL_MODE_INDIRECT_READ;    // Configure OSPI: CCR register with functional as indirect read
        // probably it wont work!!
        QUADSPI->AR = QUADSPI->AR;                              // Start the transfer by re-writing the address in AR register
        QUADSPI->CR = QuadSPI_CR_DMAEN;                         // Enable the DMA transfer by setting the DMAEN bit in the OSPI CR register
    }
    else
    {
        State = SYS_ERROR;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Transmit
//
//   Parameter(s):  pData       Pointer to data buffer.
//
//   Return Value:  SystemState_e
//
//   Description:   Sends an amount of data in non blocking mode with DMA.
//
//   Note(s):       This function is used only in Indirect Read Mode
//
//-------------------------------------------------------------------------------------------------
SystemState_e OSPI_Driver::Transmit(uint8_t* pData)
{
  SystemState_e State;
  uint32_t *tmp;

    if(pData != nullptr)
    {
        // Configure counters and size of the handle
        hqspi->TxXferCount = READ_REG(hqspi->Instance->DLR) + 1;
        hqspi->TxXferSize = READ_REG(hqspi->Instance->DLR) + 1;
        hqspi->pTxBuffPtr = pData;

        // Configure OSPI: CCR register with functional mode as indirect write
        QUADSPI->CCR &= ~(uint32_t)QuadSPI_CCR_FMODE;
        QUADSPI->CCR |=  OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE;

        //hqspi->hdma->XferCpltCallback = OSPI_DMATxCplt;           // Set the OSPI DMA transfer complete callback
        //hqspi->hdma->XferHalfCpltCallback = OSPI_DMATxHalfCplt;   // Set the OSPI DMA Half transfer complete callback
        //hqspi->hdma->XferErrorCallback = OSPI_DMAError;           // Set the DMA error callback

        m_pDMA->CR &= ~(uint32_t)DMA_SxCR_DIR;
        m_pDMA->CR |=  DMA_MEMORY_TO_PERIPH;                      // Configure the direction of the DMA

        // Enable the OSPI transmit DMA Channel
        tmp = (uint32_t*)&pData;
        DMA_Start_IT(hqspi->hdma, *(uint32_t*)tmp, (uint32_t)&hqspi->Instance->DR, hqspi->TxXferSize);

        // Enable the DMA transfer by setting the DMAEN bit in the OSPI CR register
        QUADSPI->CR = QuadSPI_CR_DMAEN;
    }
    else
    {
      State = SYS_OK;
    }

    return State;
}

#if 0
//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    IRQHandler
//
//  Description:    This function handles SPIx interrupt request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void OSPI_Driver::IRQHandler(void)
{
    if(m_pPort->CallBackISR != nullptr)
    {
        m_pPort->CallBackISR();
    }
}
#endif

#endif // (USE_OSPI_DRIVER == DEF_ENABLED)




#if 0

  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;

  /* Enable the OctoSPI memory interface clock */
  OSPI2_CLK_ENABLE();

  /* Reset the OctoSPI memory interface */
  __HAL_RCC_OSPI2_FORCE_RESET();
  __HAL_RCC_OSPI2_RELEASE_RESET();

  /* Enable the GPIO clocks */
  OSPI2_CS_GPIO_CLK_ENABLE();
  OSPI2_CLK_P_GPIO_CLK_ENABLE();
  OSPI2_D0_GPIO_CLK_ENABLE();
  OSPI2_D1_GPIO_CLK_ENABLE();
  OSPI2_D2_GPIO_CLK_ENABLE();
  OSPI2_D3_GPIO_CLK_ENABLE();
  OSPI2_D4_GPIO_CLK_ENABLE();
  OSPI2_D5_GPIO_CLK_ENABLE();
  OSPI2_D6_GPIO_CLK_ENABLE();
  OSPI2_D7_GPIO_CLK_ENABLE();
  OSPI2_DQS_GPIO_CLK_ENABLE();

  /* OSPI CS GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_CS_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF3_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_CS_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI CLK GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_CLK_PIN;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_CLK_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI D0 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_D0_PIN;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_D0_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI D1 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_D1_PIN;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_D1_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI2 D2 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_D2_PIN;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_D2_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI2 D3 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_D3_PIN;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_D3_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI2 D4 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_D4_PIN;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_D4_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI2 D5 GPIO pin configuration  */
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  GPIO_InitStruct.Pin       = OSPI2_D5_PIN;
  HAL_GPIO_Init(OSPI2_D5_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI2 D6 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = OSPI2_D6_PIN;
  GPIO_InitStruct.Alternate = GPIO_AF3_OCTOSPIM_P2;
  HAL_GPIO_Init(OSPI2_D6_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI2 D7 GPIO pin configuration  */
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  GPIO_InitStruct.Pin       = OSPI2_D7_PIN;
  HAL_GPIO_Init(OSPI2_D7_GPIO_PORT, &GPIO_InitStruct);

  /* OSPI2 DQS GPIO pin configuration  */
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P2;
  GPIO_InitStruct.Pin       = OSPI2_DQS_PIN;
  HAL_GPIO_Init(OSPI2_DQS_GPIO_PORT, &GPIO_InitStruct);

  /* Enable and set OctoSPI interrupt to the lowest priority */
  HAL_NVIC_SetPriority(OCTOSPI2_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(OCTOSPI2_IRQn);


#include "stm32h7xx_hal.h"

#if defined(OCTOSPI) || defined(OCTOSPI1) || defined(OCTOSPI2)

#ifdef HAL_OSPI_MODULE_ENABLED

/* Private define ------------------------------------------------------------*/
#define OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE ((uint32_t)0x00000000)         /*!< Indirect write mode    */
#define OSPI_FUNCTIONAL_MODE_INDIRECT_READ  ((uint32_t)OCTOSPI_CR_FMODE_0) /*!< Indirect read mode     */
#define OSPI_FUNCTIONAL_MODE_AUTO_POLLING   ((uint32_t)OCTOSPI_CR_FMODE_1) /*!< Automatic polling mode */
#define OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  ((uint32_t)OCTOSPI_CR_FMODE)   /*!< Memory-mapped mode     */

#define OSPI_CFG_STATE_MASK  0x00000004U
#define OSPI_BUSY_STATE_MASK 0x00000008U

#define OSPI_NB_INSTANCE   2U
#define OSPI_IOM_NB_PORTS  2U
#define OSPI_IOM_PORT_MASK 0x1U

/* Private macro -------------------------------------------------------------*/
#define IS_OSPI_FUNCTIONAL_MODE(MODE) (((MODE) == OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE) || \
                                       ((MODE) == OSPI_FUNCTIONAL_MODE_INDIRECT_READ)  || \
                                       ((MODE) == OSPI_FUNCTIONAL_MODE_AUTO_POLLING)   || \
                                       ((MODE) == OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED))

/* Private function prototypes -----------------------------------------------*/
static void              OSPI_DMACplt(MDMA_HandleTypeDef *hmdma);
static void              OSPI_DMAError(MDMA_HandleTypeDef *hmdma);
static void              OSPI_DMAAbortCplt(MDMA_HandleTypeDef *hmdma);
static HAL_StatusTypeDef OSPI_WaitFlagStateUntilTimeout(OSPI_HandleTypeDef *hospi, uint32_t Flag, FlagStatus State,
                                                        uint32_t Tickstart, uint32_t Timeout);
static HAL_StatusTypeDef OSPI_ConfigCmd(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd);
static HAL_StatusTypeDef OSPIM_GetConfig(uint8_t instance_nb, OSPIM_CfgTypeDef *cfg);
static void OSPI_DMAAbortOnError(MDMA_HandleTypeDef *hmdma);

/**
  * @brief  Initialize the OSPI mode according to the specified parameters
  *         in the OSPI_InitTypeDef and initialize the associated handle.
  * @param  hospi : OSPI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Init(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t tickstart = HAL_GetTick();

  /* Check the OSPI handle allocation */
  if (hospi == NULL)
  {
    status = HAL_ERROR;
    /* No error code can be set set as the handler is null */
  }
  else
  {
    /* Initialize error code */
    hospi->ErrorCode = HAL_OSPI_ERROR_NONE;

    /* Check if the state is the reset state */
    if (hospi->State == HAL_OSPI_STATE_RESET)
    {
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
      /* Reset Callback pointers in HAL_OSPI_STATE_RESET only */
      hospi->ErrorCallback         = HAL_OSPI_ErrorCallback;
      hospi->AbortCpltCallback     = HAL_OSPI_AbortCpltCallback;
      hospi->FifoThresholdCallback = HAL_OSPI_FifoThresholdCallback;
      hospi->CmdCpltCallback       = HAL_OSPI_CmdCpltCallback;
      hospi->RxCpltCallback        = HAL_OSPI_RxCpltCallback;
      hospi->TxCpltCallback        = HAL_OSPI_TxCpltCallback;
      hospi->RxHalfCpltCallback    = HAL_OSPI_RxHalfCpltCallback;
      hospi->TxHalfCpltCallback    = HAL_OSPI_TxHalfCpltCallback;
      hospi->StatusMatchCallback   = HAL_OSPI_StatusMatchCallback;
      hospi->TimeOutCallback       = HAL_OSPI_TimeOutCallback;

      if (hospi->MspInitCallback == NULL)
      {
        hospi->MspInitCallback = HAL_OSPI_MspInit;
      }

      /* Init the low level hardware */
      hospi->MspInitCallback(hospi);
#else
      /* Initialization of the low level hardware */
      HAL_OSPI_MspInit(hospi);
#endif /* defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */

      /* Configure the default timeout for the OSPI memory access */
      (void)HAL_OSPI_SetTimeout(hospi, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);

      /* Configure memory type, device size, chip select high time, delay block bypass,
         free running clock, clock mode */
      MODIFY_REG(hospi->Instance->DCR1,
                 (OCTOSPI_DCR1_MTYP | OCTOSPI_DCR1_DEVSIZE | OCTOSPI_DCR1_CSHT | OCTOSPI_DCR1_DLYBYP |
                  OCTOSPI_DCR1_FRCK | OCTOSPI_DCR1_CKMODE),
                 (hospi->Init.MemoryType | ((hospi->Init.DeviceSize - 1U) << OCTOSPI_DCR1_DEVSIZE_Pos) |
                  ((hospi->Init.ChipSelectHighTime - 1U) << OCTOSPI_DCR1_CSHT_Pos) |
                  hospi->Init.DelayBlockBypass | hospi->Init.ClockMode));

      /* Configure wrap size */
      MODIFY_REG(hospi->Instance->DCR2, OCTOSPI_DCR2_WRAPSIZE, hospi->Init.WrapSize);

      /* Configure chip select boundary and maximum transfer */
      hospi->Instance->DCR3 = ((hospi->Init.ChipSelectBoundary << OCTOSPI_DCR3_CSBOUND_Pos) |
                               (hospi->Init.MaxTran << OCTOSPI_DCR3_MAXTRAN_Pos));

      /* Configure refresh */
      hospi->Instance->DCR4 = hospi->Init.Refresh;

      /* Configure FIFO threshold */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FTHRES, ((hospi->Init.FifoThreshold - 1U) << OCTOSPI_CR_FTHRES_Pos));

      /* Wait till busy flag is reset */
      status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

      if (status == HAL_OK)
      {
        /* Configure clock prescaler */
        MODIFY_REG(hospi->Instance->DCR2, OCTOSPI_DCR2_PRESCALER,
                   ((hospi->Init.ClockPrescaler - 1U) << OCTOSPI_DCR2_PRESCALER_Pos));

        /* Configure Dual Quad mode */
        MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_DQM, hospi->Init.DualQuad);

        /* Configure sample shifting and delay hold quarter cycle */
        MODIFY_REG(hospi->Instance->TCR, (OCTOSPI_TCR_SSHIFT | OCTOSPI_TCR_DHQC),
                   (hospi->Init.SampleShifting | hospi->Init.DelayHoldQuarterCycle));

        /* Enable OctoSPI */
        __HAL_OSPI_ENABLE(hospi);

        /* Enable free running clock if needed : must be done after OSPI enable */
        if (hospi->Init.FreeRunningClock == HAL_OSPI_FREERUNCLK_ENABLE)
        {
          SET_BIT(hospi->Instance->DCR1, OCTOSPI_DCR1_FRCK);
        }

        /* Initialize the OSPI state */
        if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
        {
          hospi->State = HAL_OSPI_STATE_HYPERBUS_INIT;
        }
        else
        {
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Initialize the OSPI MSP.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_MspInit(OSPI_HandleTypeDef *hospi)
{
  UNUSED(hospi);
}

/**
  * @brief  De-Initialize the OSPI peripheral.
  * @param  hospi : OSPI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_DeInit(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the OSPI handle allocation */
  if (hospi == NULL)
  {
    status = HAL_ERROR;
    /* No error code can be set set as the handler is null */
  }
  else
  {
    /* Disable OctoSPI */
    __HAL_OSPI_DISABLE(hospi);

    /* Disable free running clock if needed : must be done after OSPI disable */
    CLEAR_BIT(hospi->Instance->DCR1, OCTOSPI_DCR1_FRCK);

#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    if (hospi->MspDeInitCallback == NULL)
    {
      hospi->MspDeInitCallback = HAL_OSPI_MspDeInit;
    }

    /* DeInit the low level hardware */
    hospi->MspDeInitCallback(hospi);
#else
    /* De-initialize the low-level hardware */
    HAL_OSPI_MspDeInit(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */

    /* Reset the driver state */
    hospi->State = HAL_OSPI_STATE_RESET;
  }

  return status;
}


/**
  * @brief  Handle OSPI interrupt request.
  * @param  hospi : OSPI handle
  * @retval None
  */
void HAL_OSPI_IRQHandler(OSPI_HandleTypeDef *hospi)
{
  __IO uint32_t *data_reg = &hospi->Instance->DR;
  uint32_t flag           = hospi->Instance->SR;
  uint32_t itsource       = hospi->Instance->CR;
  uint32_t currentstate   = hospi->State;

  /* OctoSPI fifo threshold interrupt occurred -------------------------------*/
  if (((flag & HAL_OSPI_FLAG_FT) != 0U) && ((itsource & HAL_OSPI_IT_FT) != 0U))
  {
    if (currentstate == HAL_OSPI_STATE_BUSY_TX)
    {
      /* Write a data in the fifo */
      *((__IO uint8_t *)data_reg) = *hospi->pBuffPtr;
      hospi->pBuffPtr++;
      hospi->XferCount--;
    }
    else if (currentstate == HAL_OSPI_STATE_BUSY_RX)
    {
      /* Read a data from the fifo */
      *hospi->pBuffPtr = *((__IO uint8_t *)data_reg);
      hospi->pBuffPtr++;
      hospi->XferCount--;
    }
    else
    {
      /* Nothing to do */
    }

    if (hospi->XferCount == 0U)
    {
      /* All data have been received or transmitted for the transfer */
      /* Disable fifo threshold interrupt */
      __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_FT);
    }

    /* Fifo threshold callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->FifoThresholdCallback(hospi);
#else
    HAL_OSPI_FifoThresholdCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)*/
  }
  /* OctoSPI transfer complete interrupt occurred ----------------------------*/
  else if (((flag & HAL_OSPI_FLAG_TC) != 0U) && ((itsource & HAL_OSPI_IT_TC) != 0U))
  {
    if (currentstate == HAL_OSPI_STATE_BUSY_RX)
    {
      if ((hospi->XferCount > 0U) && ((flag & OCTOSPI_SR_FLEVEL) != 0U))
      {
        /* Read the last data received in the fifo */
        *hospi->pBuffPtr = *((__IO uint8_t *)data_reg);
        hospi->pBuffPtr++;
        hospi->XferCount--;
      }
      else if (hospi->XferCount == 0U)
      {
        /* Clear flag */
        hospi->Instance->FCR = HAL_OSPI_FLAG_TC;

        /* Disable the interrupts */
        __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* RX complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->RxCpltCallback(hospi);
#else
        HAL_OSPI_RxCpltCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
      }
      else
      {
        /* Nothing to do */
      }
    }
    else
    {
      /* Clear flag */
      hospi->Instance->FCR = HAL_OSPI_FLAG_TC;

      /* Disable the interrupts */
      __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;

      if (currentstate == HAL_OSPI_STATE_BUSY_TX)
      {
        /* TX complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->TxCpltCallback(hospi);
#else
        HAL_OSPI_TxCpltCallback(hospi);
#endif /* defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
      }
      else if (currentstate == HAL_OSPI_STATE_BUSY_CMD)
      {
        /* Command complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->CmdCpltCallback(hospi);
#else
        HAL_OSPI_CmdCpltCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
      }
      else if (currentstate == HAL_OSPI_STATE_ABORT)
      {
        if (hospi->ErrorCode == HAL_OSPI_ERROR_NONE)
        {
          /* Abort called by the user */
          /* Abort complete callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
          hospi->AbortCpltCallback(hospi);
#else
          HAL_OSPI_AbortCpltCallback(hospi);
#endif /* defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)*/
        }
        else
        {
          /* Abort due to an error (eg : DMA error) */
          /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
          hospi->ErrorCallback(hospi);
#else
          HAL_OSPI_ErrorCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
        }
      }
      else
      {
        /* Nothing to do */
      }
    }
  }
  /* OctoSPI status match interrupt occurred ---------------------------------*/
  else if (((flag & HAL_OSPI_FLAG_SM) != 0U) && ((itsource & HAL_OSPI_IT_SM) != 0U))
  {
    /* Clear flag */
    hospi->Instance->FCR = HAL_OSPI_FLAG_SM;

    /* Check if automatic poll mode stop is activated */
    if ((hospi->Instance->CR & OCTOSPI_CR_APMS) != 0U)
    {
      /* Disable the interrupts */
      __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_SM | HAL_OSPI_IT_TE);

      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;
    }

    /* Status match callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->StatusMatchCallback(hospi);
#else
    HAL_OSPI_StatusMatchCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
  }
  /* OctoSPI transfer error interrupt occurred -------------------------------*/
  else if (((flag & HAL_OSPI_FLAG_TE) != 0U) && ((itsource & HAL_OSPI_IT_TE) != 0U))
  {
    /* Clear flag */
    hospi->Instance->FCR = HAL_OSPI_FLAG_TE;

    /* Disable all interrupts */
    __HAL_OSPI_DISABLE_IT(hospi, (HAL_OSPI_IT_TO | HAL_OSPI_IT_SM | HAL_OSPI_IT_FT | HAL_OSPI_IT_TC | HAL_OSPI_IT_TE));

    /* Set error code */
    hospi->ErrorCode = HAL_OSPI_ERROR_TRANSFER;

    /* Check if the DMA is enabled */
    if ((hospi->Instance->CR & OCTOSPI_CR_DMAEN) != 0U)
    {
      /* Disable the DMA transfer on the OctoSPI side */
      CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

      /* Disable the DMA transfer on the DMA side */
      hospi->hmdma->XferAbortCallback = OSPI_DMAAbortCplt;
      if (HAL_MDMA_Abort_IT(hospi->hmdma) != HAL_OK)
      {
        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->ErrorCallback(hospi);
#else
        HAL_OSPI_ErrorCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)*/
      }
    }
    else
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;

      /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
      hospi->ErrorCallback(hospi);
#else
      HAL_OSPI_ErrorCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
    }
  }
  /* OctoSPI timeout interrupt occurred --------------------------------------*/
  else if (((flag & HAL_OSPI_FLAG_TO) != 0U) && ((itsource & HAL_OSPI_IT_TO) != 0U))
  {
    /* Clear flag */
    hospi->Instance->FCR = HAL_OSPI_FLAG_TO;

    /* Timeout callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->TimeOutCallback(hospi);
#else
    HAL_OSPI_TimeOutCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
  }
}

/**
  * @brief  Set the command configuration.
  * @param  hospi   : OSPI handle
  * @param  cmd     : structure that contains the command configuration information
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Command(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t state;
  uint32_t tickstart = HAL_GetTick();

  /* Check the state of the driver */
  state = hospi->State;
  if (((state == HAL_OSPI_STATE_READY)         && (hospi->Init.MemoryType != HAL_OSPI_MEMTYPE_HYPERBUS)) ||
      ((state == HAL_OSPI_STATE_READ_CMD_CFG)  && ((cmd->OperationType == HAL_OSPI_OPTYPE_WRITE_CFG)
                                                   || (cmd->OperationType == HAL_OSPI_OPTYPE_WRAP_CFG))) ||
      ((state == HAL_OSPI_STATE_WRITE_CMD_CFG) && ((cmd->OperationType == HAL_OSPI_OPTYPE_READ_CFG)  ||
                                                   (cmd->OperationType == HAL_OSPI_OPTYPE_WRAP_CFG))))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Initialize error code */
      hospi->ErrorCode = HAL_OSPI_ERROR_NONE;

      /* Configure the registers */
      status = OSPI_ConfigCmd(hospi, cmd);

      if (status == HAL_OK)
      {
        if (cmd->DataMode == HAL_OSPI_DATA_NONE)
        {
          /* When there is no data phase, the transfer start as soon as the configuration is done
             so wait until TC flag is set to go back in idle state */
          status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, Timeout);

          __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);
        }
        else
        {
          /* Update the state */
          if (cmd->OperationType == HAL_OSPI_OPTYPE_COMMON_CFG)
          {
            hospi->State = HAL_OSPI_STATE_CMD_CFG;
          }
          else if (cmd->OperationType == HAL_OSPI_OPTYPE_READ_CFG)
          {
            if (hospi->State == HAL_OSPI_STATE_WRITE_CMD_CFG)
            {
              hospi->State = HAL_OSPI_STATE_CMD_CFG;
            }
            else
            {
              hospi->State = HAL_OSPI_STATE_READ_CMD_CFG;
            }
          }
          else if (cmd->OperationType == HAL_OSPI_OPTYPE_WRITE_CFG)
          {
            if (hospi->State == HAL_OSPI_STATE_READ_CMD_CFG)
            {
              hospi->State = HAL_OSPI_STATE_CMD_CFG;
            }
            else
            {
              hospi->State = HAL_OSPI_STATE_WRITE_CMD_CFG;
            }
          }
          else
          {
            /* Wrap configuration, no state change */
          }
        }
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Set the command configuration in interrupt mode.
  * @param  hospi : OSPI handle
  * @param  cmd   : structure that contains the command configuration information
  * @note   This function is used only in Indirect Read or Write Modes
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Command_IT(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the state of the driver */
  if ((hospi->State  == HAL_OSPI_STATE_READY) && (cmd->OperationType     == HAL_OSPI_OPTYPE_COMMON_CFG) &&
      (cmd->DataMode == HAL_OSPI_DATA_NONE)   && (hospi->Init.MemoryType != HAL_OSPI_MEMTYPE_HYPERBUS))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

    if (status == HAL_OK)
    {
      /* Initialize error code */
      hospi->ErrorCode = HAL_OSPI_ERROR_NONE;

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

      /* Configure the registers */
      status = OSPI_ConfigCmd(hospi, cmd);

      if (status == HAL_OK)
      {
        /* Update the state */
        hospi->State = HAL_OSPI_STATE_BUSY_CMD;

        /* Enable the transfer complete and transfer error interrupts */
        __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_TE);
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the Hyperbus parameters.
  * @param  hospi   : OSPI handle
  * @param  cfg     : Structure containing the Hyperbus configuration
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_HyperbusCfg(OSPI_HandleTypeDef *hospi, OSPI_HyperbusCfgTypeDef *cfg, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t state;
  uint32_t tickstart = HAL_GetTick();

  /* Check the state of the driver */
  state = hospi->State;
  if ((state == HAL_OSPI_STATE_HYPERBUS_INIT) || (state == HAL_OSPI_STATE_READY))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Configure Hyperbus configuration Latency register */
      WRITE_REG(hospi->Instance->HLCR, ((cfg->RWRecoveryTime << OCTOSPI_HLCR_TRWR_Pos) |
                                        (cfg->AccessTime << OCTOSPI_HLCR_TACC_Pos)     |
                                        cfg->WriteZeroLatency | cfg->LatencyMode));

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_READY;
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Set the Hyperbus command configuration.
  * @param  hospi   : OSPI handle
  * @param  cmd     : Structure containing the Hyperbus command
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_HyperbusCmd(OSPI_HandleTypeDef *hospi, OSPI_HyperbusCmdTypeDef *cmd, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the state of the driver */
  if ((hospi->State == HAL_OSPI_STATE_READY) && (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Re-initialize the value of the functional mode */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, 0U);

      /* Configure the address space in the DCR1 register */
      MODIFY_REG(hospi->Instance->DCR1, OCTOSPI_DCR1_MTYP_0, cmd->AddressSpace);

      /* Configure the CCR and WCCR registers with the address size and the following configuration :
         - DQS signal enabled (used as RWDS)
         - DTR mode enabled on address and data
         - address and data on 8 lines */
      WRITE_REG(hospi->Instance->CCR, (cmd->DQSMode | OCTOSPI_CCR_DDTR | OCTOSPI_CCR_DMODE_2 |
                                       cmd->AddressSize | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADMODE_2));
      WRITE_REG(hospi->Instance->WCCR, (cmd->DQSMode | OCTOSPI_WCCR_DDTR | OCTOSPI_WCCR_DMODE_2 |
                                        cmd->AddressSize | OCTOSPI_WCCR_ADDTR | OCTOSPI_WCCR_ADMODE_2));

      /* Configure the DLR register with the number of data */
      WRITE_REG(hospi->Instance->DLR, (cmd->NbData - 1U));

      /* Configure the AR register with the address value */
      WRITE_REG(hospi->Instance->AR, cmd->Address);

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_CMD_CFG;
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Transmit an amount of data in blocking mode.
  * @param  hospi   : OSPI handle
  * @param  pData   : pointer to data buffer
  * @param  Timeout : Timeout duration
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Transmit(OSPI_HandleTypeDef *hospi, uint8_t *pData, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  __IO uint32_t *data_reg = &hospi->Instance->DR;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect write */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

      do
      {
        /* Wait till fifo threshold flag is set to send data */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_FT, SET, tickstart, Timeout);

        if (status != HAL_OK)
        {
          break;
        }

        *((__IO uint8_t *)data_reg) = *hospi->pBuffPtr;
        hospi->pBuffPtr++;
        hospi->XferCount--;
      }
      while (hospi->XferCount > 0U);

      if (status == HAL_OK)
      {
        /* Wait till transfer complete flag is set to go back in idle state */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, Timeout);

        if (status == HAL_OK)
        {
          /* Clear transfer complete flag */
          __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

          /* Update state */
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Receive an amount of data in blocking mode.
  * @param  hospi   : OSPI handle
  * @param  pData   : pointer to data buffer
  * @param  Timeout : Timeout duration
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Receive(OSPI_HandleTypeDef *hospi, uint8_t *pData, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  __IO uint32_t *data_reg = &hospi->Instance->DR;
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect read */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_READ);

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }

      do
      {
        /* Wait till fifo threshold or transfer complete flags are set to read received data */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, (HAL_OSPI_FLAG_FT | HAL_OSPI_FLAG_TC), SET, tickstart, Timeout);

        if (status != HAL_OK)
        {
          break;
        }

        *hospi->pBuffPtr = *((__IO uint8_t *)data_reg);
        hospi->pBuffPtr++;
        hospi->XferCount--;
      }
      while (hospi->XferCount > 0U);

      if (status == HAL_OK)
      {
        /* Wait till transfer complete flag is set to go back in idle state */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, Timeout);

        if (status == HAL_OK)
        {
          /* Clear transfer complete flag */
          __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

          /* Update state */
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Send an amount of data in non-blocking mode with interrupt.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Transmit_IT(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect write */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_BUSY_TX;

      /* Enable the transfer complete, fifo threshold and transfer error interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Receive an amount of data in non-blocking mode with interrupt.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer
  * @note   This function is used only in Indirect Read Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Receive_IT(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      /* Configure counters and size */
      hospi->XferCount = READ_REG(hospi->Instance->DLR) + 1U;
      hospi->XferSize  = hospi->XferCount;
      hospi->pBuffPtr  = pData;

      /* Configure CR register with functional mode as indirect read */
      MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_READ);

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

      /* Update the state */
      hospi->State = HAL_OSPI_STATE_BUSY_RX;

      /* Enable the transfer complete, fifo threshold and transfer error interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Send an amount of data in non-blocking mode with DMA.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer
  * @note   This function is used only in Indirect Write Mode
  * @note   If DMA peripheral access is configured as halfword, the number
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Transmit_DMA(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t data_size = hospi->Instance->DLR + 1U;

  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      hospi->XferCount = data_size;

      {
        hospi->XferSize = hospi->XferCount;
        hospi->pBuffPtr = pData;

        /* Configure CR register with functional mode as indirect write */
        MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);

        /* Clear flags related to interrupt */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

        /* Update the state */
        hospi->State = HAL_OSPI_STATE_BUSY_TX;

        /* Set the MDMA transfer complete callback */
        hospi->hmdma->XferCpltCallback = OSPI_DMACplt;

        /* Set the MDMA error callback */
        hospi->hmdma->XferErrorCallback = OSPI_DMAError;

        /* Clear the MDMA abort callback */
        hospi->hmdma->XferAbortCallback = NULL;

        /* In Transmit mode , the MDMA destination is the OSPI DR register : Force the MDMA Destination Increment
           to disable */
        MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_DINC | MDMA_CTCR_DINCOS), MDMA_DEST_INC_DISABLE);

        /* Update MDMA configuration with the correct SourceInc field for Write operation */
        if (hospi->hmdma->Init.SourceDataSize == MDMA_SRC_DATASIZE_BYTE)
        {
          MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_SINC | MDMA_CTCR_SINCOS), MDMA_SRC_INC_BYTE);
        }
        else if (hospi->hmdma->Init.SourceDataSize == MDMA_SRC_DATASIZE_HALFWORD)
        {
          MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_SINC | MDMA_CTCR_SINCOS), MDMA_SRC_INC_HALFWORD);
        }
        else if (hospi->hmdma->Init.SourceDataSize == MDMA_SRC_DATASIZE_WORD)
        {
          MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_SINC | MDMA_CTCR_SINCOS), MDMA_SRC_INC_WORD);
        }
        else
        {
          /* in case of incorrect source data size */
          hospi->ErrorCode |= HAL_OSPI_ERROR_DMA;
          status = HAL_ERROR;
        }

        /* Enable the transmit MDMA Channel */
        if (HAL_MDMA_Start_IT(hospi->hmdma, (uint32_t)pData, (uint32_t)&hospi->Instance->DR, hospi->XferSize, 1) == \
            HAL_OK)
        {
          /* Enable the transfer error interrupt */
          __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TE);

          /* Enable the DMA transfer by setting the DMAEN bit  */
          SET_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);
        }
        else
        {
          status = HAL_ERROR;
          hospi->ErrorCode = HAL_OSPI_ERROR_DMA;
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Receive an amount of data in non-blocking mode with DMA.
  * @param  hospi : OSPI handle
  * @param  pData : pointer to data buffer.
  * @note   This function is used only in Indirect Read Mode
  * @note   If DMA peripheral access is configured as halfword, the number
  *         of data and the fifo threshold should be aligned on halfword
  * @note   If DMA peripheral access is configured as word, the number
  *         of data and the fifo threshold should be aligned on word
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Receive_DMA(OSPI_HandleTypeDef *hospi, uint8_t *pData)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t data_size = hospi->Instance->DLR + 1U;
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;
  /* Check the data pointer allocation */
  if (pData == NULL)
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
  }
  else
  {
    /* Check the state */
    if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
    {
      hospi->XferCount = data_size;

      {
        hospi->XferSize  = hospi->XferCount;
        hospi->pBuffPtr  = pData;

        /* Configure CR register with functional mode as indirect read */
        MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, OSPI_FUNCTIONAL_MODE_INDIRECT_READ);

        /* Clear flags related to interrupt */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_TC);

        /* Update the state */
        hospi->State = HAL_OSPI_STATE_BUSY_RX;

        /* Set the DMA transfer complete callback */
        hospi->hmdma->XferCpltCallback = OSPI_DMACplt;

        /* Set the DMA error callback */
        hospi->hmdma->XferErrorCallback = OSPI_DMAError;

        /* Clear the DMA abort callback */
        hospi->hmdma->XferAbortCallback = NULL;

        /* In Receive mode , the MDMA source is the OSPI DR register : Force the MDMA Source Increment to disable */
        MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_SINC | MDMA_CTCR_SINCOS), MDMA_SRC_INC_DISABLE);

        /* Update MDMA configuration with the correct DestinationInc field for read operation */
        if (hospi->hmdma->Init.DestDataSize == MDMA_DEST_DATASIZE_BYTE)
        {
          MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_DINC | MDMA_CTCR_DINCOS), MDMA_DEST_INC_BYTE);
        }
        else if (hospi->hmdma->Init.DestDataSize == MDMA_DEST_DATASIZE_HALFWORD)
        {
          MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_DINC | MDMA_CTCR_DINCOS), MDMA_DEST_INC_HALFWORD);
        }
        else if (hospi->hmdma->Init.DestDataSize == MDMA_DEST_DATASIZE_WORD)
        {
          MODIFY_REG(hospi->hmdma->Instance->CTCR, (MDMA_CTCR_DINC | MDMA_CTCR_DINCOS), MDMA_DEST_INC_WORD);
        }
        else
        {
          /* in case of incorrect destination data size */
          hospi->ErrorCode |= HAL_OSPI_ERROR_DMA;
          status = HAL_ERROR;
        }

        /* Enable the transmit MDMA Channel */
        if (HAL_MDMA_Start_IT(hospi->hmdma, (uint32_t)&hospi->Instance->DR, (uint32_t)pData, hospi->XferSize, 1) == \
            HAL_OK)
        {
          /* Enable the transfer error interrupt */
          __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TE);

          /* Trig the transfer by re-writing address or instruction register */
          if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
          {
            WRITE_REG(hospi->Instance->AR, addr_reg);
          }
          else
          {
            if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
            {
              WRITE_REG(hospi->Instance->AR, addr_reg);
            }
            else
            {
              WRITE_REG(hospi->Instance->IR, ir_reg);
            }
          }

          /* Enable the DMA transfer by setting the DMAEN bit  */
          SET_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);
        }
        else
        {
          status = HAL_ERROR;
          hospi->ErrorCode = HAL_OSPI_ERROR_DMA;
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the OSPI Automatic Polling Mode in blocking mode.
  * @param  hospi   : OSPI handle
  * @param  cfg     : structure that contains the polling configuration information.
  * @param  Timeout : Timeout duration
  * @note   This function is used only in Automatic Polling Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_AutoPolling(OSPI_HandleTypeDef *hospi, OSPI_AutoPollingTypeDef *cfg, uint32_t Timeout)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;
#ifdef USE_FULL_ASSERT
  uint32_t dlr_reg = hospi->Instance->DLR;
#endif /* USE_FULL_ASSERT */

    /* Check the state */
  if ((hospi->State == HAL_OSPI_STATE_CMD_CFG) && (cfg->AutomaticStop == HAL_OSPI_AUTOMATIC_STOP_ENABLE))
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, Timeout);

    if (status == HAL_OK)
    {
      /* Configure registers */
      WRITE_REG(hospi->Instance->PSMAR, cfg->Match);
      WRITE_REG(hospi->Instance->PSMKR, cfg->Mask);
      WRITE_REG(hospi->Instance->PIR,   cfg->Interval);
      MODIFY_REG(hospi->Instance->CR, (OCTOSPI_CR_PMM | OCTOSPI_CR_APMS | OCTOSPI_CR_FMODE),
                 (cfg->MatchMode | cfg->AutomaticStop | OSPI_FUNCTIONAL_MODE_AUTO_POLLING));

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }

      /* Wait till status match flag is set to go back in idle state */
      status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_SM, SET, tickstart, Timeout);

      if (status == HAL_OK)
      {
        /* Clear status match flag */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_SM);

        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the OSPI Automatic Polling Mode in non-blocking mode.
  * @param  hospi : OSPI handle
  * @param  cfg   : structure that contains the polling configuration information.
  * @note   This function is used only in Automatic Polling Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_AutoPolling_IT(OSPI_HandleTypeDef *hospi, OSPI_AutoPollingTypeDef *cfg)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();
  uint32_t addr_reg = hospi->Instance->AR;
  uint32_t ir_reg = hospi->Instance->IR;
#ifdef USE_FULL_ASSERT
  uint32_t dlr_reg = hospi->Instance->DLR;
#endif /* USE_FULL_ASSERT */

  /* Check the state */
  if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

    if (status == HAL_OK)
    {
      /* Configure registers */
      WRITE_REG(hospi->Instance->PSMAR, cfg->Match);
      WRITE_REG(hospi->Instance->PSMKR, cfg->Mask);
      WRITE_REG(hospi->Instance->PIR,   cfg->Interval);
      MODIFY_REG(hospi->Instance->CR, (OCTOSPI_CR_PMM | OCTOSPI_CR_APMS | OCTOSPI_CR_FMODE),
                 (cfg->MatchMode | cfg->AutomaticStop | OSPI_FUNCTIONAL_MODE_AUTO_POLLING));

      /* Clear flags related to interrupt */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TE | HAL_OSPI_FLAG_SM);

      /* Update state */
      hospi->State = HAL_OSPI_STATE_BUSY_AUTO_POLLING;

      /* Enable the status match and transfer error interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_SM | HAL_OSPI_IT_TE);

      /* Trig the transfer by re-writing address or instruction register */
      if (hospi->Init.MemoryType == HAL_OSPI_MEMTYPE_HYPERBUS)
      {
        WRITE_REG(hospi->Instance->AR, addr_reg);
      }
      else
      {
        if (READ_BIT(hospi->Instance->CCR, OCTOSPI_CCR_ADMODE) != HAL_OSPI_ADDRESS_NONE)
        {
          WRITE_REG(hospi->Instance->AR, addr_reg);
        }
        else
        {
          WRITE_REG(hospi->Instance->IR, ir_reg);
        }
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Configure the Memory Mapped mode.
  * @param  hospi : OSPI handle
  * @param  cfg   : structure that contains the memory mapped configuration information.
  * @note   This function is used only in Memory mapped Mode
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_MemoryMapped(OSPI_HandleTypeDef *hospi, OSPI_MemoryMappedTypeDef *cfg)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  /* Check the state */
  if (hospi->State == HAL_OSPI_STATE_CMD_CFG)
  {
    /* Wait till busy flag is reset */
    status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

    if (status == HAL_OK)
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_BUSY_MEM_MAPPED;

      if (cfg->TimeOutActivation == HAL_OSPI_TIMEOUT_COUNTER_ENABLE)
      {
        assert_param(IS_OSPI_TIMEOUT_PERIOD(cfg->TimeOutPeriod));

        /* Configure register */
        WRITE_REG(hospi->Instance->LPTR, cfg->TimeOutPeriod);

        /* Clear flags related to interrupt */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TO);

        /* Enable the timeout interrupt */
        __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TO);
      }

      /* Configure CR register with functional mode as memory-mapped */
      MODIFY_REG(hospi->Instance->CR, (OCTOSPI_CR_TCEN | OCTOSPI_CR_FMODE),
                 (cfg->TimeOutActivation | OSPI_FUNCTIONAL_MODE_MEMORY_MAPPED));
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Transfer Error callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_ErrorCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_ErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Abort completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_AbortCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_AbortCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  FIFO Threshold callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_FifoThresholdCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_FIFOThresholdCallback could be implemented in the user file
   */
}

/**
  * @brief  Command completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_CmdCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_CmdCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_RxCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_RxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_TxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Rx Half Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_RxHalfCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_RxHalfCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Tx Half Transfer completed callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_TxHalfCpltCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_OSPI_TxHalfCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Status Match callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_StatusMatchCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_StatusMatchCallback could be implemented in the user file
   */
}

/**
  * @brief  Timeout callback.
  * @param  hospi : OSPI handle
  * @retval None
  */
__weak void HAL_OSPI_TimeOutCallback(OSPI_HandleTypeDef *hospi)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hospi);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_OSPI_TimeOutCallback could be implemented in the user file
   */
}

#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
/**
  * @brief  Register a User OSPI Callback
  *         To be used to override the weak predefined callback
  * @param hospi : OSPI handle
  * @param CallbackID : ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OSPI_ERROR_CB_ID          OSPI Error Callback ID
  *          @arg @ref HAL_OSPI_ABORT_CB_ID          OSPI Abort Callback ID
  *          @arg @ref HAL_OSPI_FIFO_THRESHOLD_CB_ID OSPI FIFO Threshold Callback ID
  *          @arg @ref HAL_OSPI_CMD_CPLT_CB_ID       OSPI Command Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_CPLT_CB_ID        OSPI Rx Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_CPLT_CB_ID        OSPI Tx Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_HALF_CPLT_CB_ID   OSPI Rx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_HALF_CPLT_CB_ID   OSPI Tx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_STATUS_MATCH_CB_ID   OSPI Status Match Callback ID
  *          @arg @ref HAL_OSPI_TIMEOUT_CB_ID        OSPI Timeout Callback ID
  *          @arg @ref HAL_OSPI_MSP_INIT_CB_ID       OSPI MspInit callback ID
  *          @arg @ref HAL_OSPI_MSP_DEINIT_CB_ID     OSPI MspDeInit callback ID
  * @param pCallback : pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_OSPI_RegisterCallback(OSPI_HandleTypeDef *hospi, HAL_OSPI_CallbackIDTypeDef CallbackID,
                                            pOSPI_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
    return HAL_ERROR;
  }

  if (hospi->State == HAL_OSPI_STATE_READY)
  {
    switch (CallbackID)
    {
      case  HAL_OSPI_ERROR_CB_ID :
        hospi->ErrorCallback = pCallback;
        break;
      case HAL_OSPI_ABORT_CB_ID :
        hospi->AbortCpltCallback = pCallback;
        break;
      case HAL_OSPI_FIFO_THRESHOLD_CB_ID :
        hospi->FifoThresholdCallback = pCallback;
        break;
      case HAL_OSPI_CMD_CPLT_CB_ID :
        hospi->CmdCpltCallback = pCallback;
        break;
      case HAL_OSPI_RX_CPLT_CB_ID :
        hospi->RxCpltCallback = pCallback;
        break;
      case HAL_OSPI_TX_CPLT_CB_ID :
        hospi->TxCpltCallback = pCallback;
        break;
      case HAL_OSPI_RX_HALF_CPLT_CB_ID :
        hospi->RxHalfCpltCallback = pCallback;
        break;
      case HAL_OSPI_TX_HALF_CPLT_CB_ID :
        hospi->TxHalfCpltCallback = pCallback;
        break;
      case HAL_OSPI_STATUS_MATCH_CB_ID :
        hospi->StatusMatchCallback = pCallback;
        break;
      case HAL_OSPI_TIMEOUT_CB_ID :
        hospi->TimeOutCallback = pCallback;
        break;
      case HAL_OSPI_MSP_INIT_CB_ID :
        hospi->MspInitCallback = pCallback;
        break;
      case HAL_OSPI_MSP_DEINIT_CB_ID :
        hospi->MspDeInitCallback = pCallback;
        break;
      default :
        /* Update the error code */
        hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (hospi->State == HAL_OSPI_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_OSPI_MSP_INIT_CB_ID :
        hospi->MspInitCallback = pCallback;
        break;
      case HAL_OSPI_MSP_DEINIT_CB_ID :
        hospi->MspDeInitCallback = pCallback;
        break;
      default :
        /* Update the error code */
        hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Unregister a User OSPI Callback
  *         OSPI Callback is redirected to the weak predefined callback
  * @param hospi : OSPI handle
  * @param CallbackID : ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OSPI_ERROR_CB_ID          OSPI Error Callback ID
  *          @arg @ref HAL_OSPI_ABORT_CB_ID          OSPI Abort Callback ID
  *          @arg @ref HAL_OSPI_FIFO_THRESHOLD_CB_ID OSPI FIFO Threshold Callback ID
  *          @arg @ref HAL_OSPI_CMD_CPLT_CB_ID       OSPI Command Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_CPLT_CB_ID        OSPI Rx Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_CPLT_CB_ID        OSPI Tx Complete Callback ID
  *          @arg @ref HAL_OSPI_RX_HALF_CPLT_CB_ID   OSPI Rx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_TX_HALF_CPLT_CB_ID   OSPI Tx Half Complete Callback ID
  *          @arg @ref HAL_OSPI_STATUS_MATCH_CB_ID   OSPI Status Match Callback ID
  *          @arg @ref HAL_OSPI_TIMEOUT_CB_ID        OSPI Timeout Callback ID
  *          @arg @ref HAL_OSPI_MSP_INIT_CB_ID       OSPI MspInit callback ID
  *          @arg @ref HAL_OSPI_MSP_DEINIT_CB_ID     OSPI MspDeInit callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_OSPI_UnRegisterCallback(OSPI_HandleTypeDef *hospi, HAL_OSPI_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (hospi->State == HAL_OSPI_STATE_READY)
  {
    switch (CallbackID)
    {
      case  HAL_OSPI_ERROR_CB_ID :
        hospi->ErrorCallback = HAL_OSPI_ErrorCallback;
        break;
      case HAL_OSPI_ABORT_CB_ID :
        hospi->AbortCpltCallback = HAL_OSPI_AbortCpltCallback;
        break;
      case HAL_OSPI_FIFO_THRESHOLD_CB_ID :
        hospi->FifoThresholdCallback = HAL_OSPI_FifoThresholdCallback;
        break;
      case HAL_OSPI_CMD_CPLT_CB_ID :
        hospi->CmdCpltCallback = HAL_OSPI_CmdCpltCallback;
        break;
      case HAL_OSPI_RX_CPLT_CB_ID :
        hospi->RxCpltCallback = HAL_OSPI_RxCpltCallback;
        break;
      case HAL_OSPI_TX_CPLT_CB_ID :
        hospi->TxCpltCallback = HAL_OSPI_TxCpltCallback;
        break;
      case HAL_OSPI_RX_HALF_CPLT_CB_ID :
        hospi->RxHalfCpltCallback = HAL_OSPI_RxHalfCpltCallback;
        break;
      case HAL_OSPI_TX_HALF_CPLT_CB_ID :
        hospi->TxHalfCpltCallback = HAL_OSPI_TxHalfCpltCallback;
        break;
      case HAL_OSPI_STATUS_MATCH_CB_ID :
        hospi->StatusMatchCallback = HAL_OSPI_StatusMatchCallback;
        break;
      case HAL_OSPI_TIMEOUT_CB_ID :
        hospi->TimeOutCallback = HAL_OSPI_TimeOutCallback;
        break;
      case HAL_OSPI_MSP_INIT_CB_ID :
        hospi->MspInitCallback = HAL_OSPI_MspInit;
        break;
      case HAL_OSPI_MSP_DEINIT_CB_ID :
        hospi->MspDeInitCallback = HAL_OSPI_MspDeInit;
        break;
      default :
        /* Update the error code */
        hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (hospi->State == HAL_OSPI_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_OSPI_MSP_INIT_CB_ID :
        hospi->MspInitCallback = HAL_OSPI_MspInit;
        break;
      case HAL_OSPI_MSP_DEINIT_CB_ID :
        hospi->MspDeInitCallback = HAL_OSPI_MspDeInit;
        break;
      default :
        /* Update the error code */
        hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
        /* update return status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hospi->ErrorCode |= HAL_OSPI_ERROR_INVALID_CALLBACK;
    /* update return status */
    status =  HAL_ERROR;
  }

  return status;
}
#endif /* defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */

/**
  * @brief  Abort the current transmission.
  * @param  hospi : OSPI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Abort(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t state;
  uint32_t tickstart = HAL_GetTick();

  /* Check if the state is in one of the busy or configured states */
  state = hospi->State;
  if (((state & OSPI_BUSY_STATE_MASK) != 0U) || ((state & OSPI_CFG_STATE_MASK) != 0U))
  {
    /* Check if the DMA is enabled */
    if ((hospi->Instance->CR & OCTOSPI_CR_DMAEN) != 0U)
    {
      /* Disable the DMA transfer on the OctoSPI side */
      CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

      /* Disable the DMA transfer on the DMA side */
      status = HAL_MDMA_Abort(hospi->hmdma);
      if (status != HAL_OK)
      {
        hospi->ErrorCode = HAL_OSPI_ERROR_DMA;
      }
    }

    if (__HAL_OSPI_GET_FLAG(hospi, HAL_OSPI_FLAG_BUSY) != RESET)
    {
      /* Perform an abort of the OctoSPI */
      SET_BIT(hospi->Instance->CR, OCTOSPI_CR_ABORT);

      /* Wait until the transfer complete flag is set to go back in idle state */
      status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_TC, SET, tickstart, hospi->Timeout);

      if (status == HAL_OK)
      {
        /* Clear transfer complete flag */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

        /* Wait until the busy flag is reset to go back in idle state */
        status = OSPI_WaitFlagStateUntilTimeout(hospi, HAL_OSPI_FLAG_BUSY, RESET, tickstart, hospi->Timeout);

        if (status == HAL_OK)
        {
          /* Update state */
          hospi->State = HAL_OSPI_STATE_READY;
        }
      }
    }
    else
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Abort the current transmission (non-blocking function)
  * @param  hospi : OSPI handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_Abort_IT(OSPI_HandleTypeDef *hospi)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t state;

  /* Check if the state is in one of the busy or configured states */
  state = hospi->State;
  if (((state & OSPI_BUSY_STATE_MASK) != 0U) || ((state & OSPI_CFG_STATE_MASK) != 0U))
  {
    /* Disable all interrupts */
    __HAL_OSPI_DISABLE_IT(hospi, (HAL_OSPI_IT_TO | HAL_OSPI_IT_SM | HAL_OSPI_IT_FT | HAL_OSPI_IT_TC | HAL_OSPI_IT_TE));

    /* Update state */
    hospi->State = HAL_OSPI_STATE_ABORT;

    /* Check if the DMA is enabled */
    if ((hospi->Instance->CR & OCTOSPI_CR_DMAEN) != 0U)
    {
      /* Disable the DMA transfer on the OctoSPI side */
      CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

      /* Disable the DMA transfer on the DMA side */
      hospi->hmdma->XferAbortCallback = OSPI_DMAAbortCplt;
      if (HAL_MDMA_Abort_IT(hospi->hmdma) != HAL_OK)
      {
        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* Abort callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->AbortCpltCallback(hospi);
#else
        HAL_OSPI_AbortCpltCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)*/
      }
    }
    else
    {
      if (__HAL_OSPI_GET_FLAG(hospi, HAL_OSPI_FLAG_BUSY) != RESET)
      {
        /* Clear transfer complete flag */
        __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

        /* Enable the transfer complete interrupts */
        __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC);

        /* Perform an abort of the OctoSPI */
        SET_BIT(hospi->Instance->CR, OCTOSPI_CR_ABORT);
      }
      else
      {
        /* Update state */
        hospi->State = HAL_OSPI_STATE_READY;

        /* Abort callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
        hospi->AbortCpltCallback(hospi);
#else
        HAL_OSPI_AbortCpltCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
      }
    }
  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/** @brief  Set OSPI Fifo threshold.
  * @param  hospi     : OSPI handle.
  * @param  Threshold : Threshold of the Fifo.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPI_SetFifoThreshold(OSPI_HandleTypeDef *hospi, uint32_t Threshold)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the state */
  if ((hospi->State & OSPI_BUSY_STATE_MASK) == 0U)
  {
    /* Synchronize initialization structure with the new fifo threshold value */
    hospi->Init.FifoThreshold = Threshold;

    /* Configure new fifo threshold */
    MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FTHRES, ((hospi->Init.FifoThreshold - 1U) << OCTOSPI_CR_FTHRES_Pos));

  }
  else
  {
    status = HAL_ERROR;
    hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_SEQUENCE;
  }

  /* Return function status */
  return status;
}

/** @brief  Get OSPI Fifo threshold.
  * @param  hospi : OSPI handle.
  * @retval Fifo threshold
  */
uint32_t HAL_OSPI_GetFifoThreshold(const OSPI_HandleTypeDef *hospi)
{
  return ((READ_BIT(hospi->Instance->CR, OCTOSPI_CR_FTHRES) >> OCTOSPI_CR_FTHRES_Pos) + 1U);
}

/** @brief Set OSPI timeout.
  * @param  hospi   : OSPI handle.
  * @param  Timeout : Timeout for the memory access.
  * @retval None
  */
HAL_StatusTypeDef HAL_OSPI_SetTimeout(OSPI_HandleTypeDef *hospi, uint32_t Timeout)
{
  hospi->Timeout = Timeout;
  return HAL_OK;
}

/**
  * @brief  Return the OSPI error code.
  * @param  hospi : OSPI handle
  * @retval OSPI Error Code
  */
uint32_t HAL_OSPI_GetError(const OSPI_HandleTypeDef *hospi)
{
  return hospi->ErrorCode;
}

/**
  * @brief  Return the OSPI handle state.
  * @param  hospi : OSPI handle
  * @retval HAL state
  */
uint32_t HAL_OSPI_GetState(const OSPI_HandleTypeDef *hospi)
{
  /* Return OSPI handle state */
  return hospi->State;
}

/**
  * @brief  Configure the OctoSPI IO manager.
  * @param  hospi   : OSPI handle
  * @param  cfg     : Configuration of the IO Manager for the instance
  * @param  Timeout : Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OSPIM_Config(OSPI_HandleTypeDef *hospi, OSPIM_CfgTypeDef *cfg, uint32_t Timeout)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t instance;
  uint8_t index;
  uint8_t ospi_enabled = 0U;
  uint8_t other_instance;
  OSPIM_CfgTypeDef IOM_cfg[OSPI_NB_INSTANCE];

  /* Prevent unused argument(s) compilation warning */
  UNUSED(Timeout);

  if (hospi->Instance == OCTOSPI1)
  {
    instance = 0U;
    other_instance = 1U;
  }
  else
  {
    instance = 1U;
    other_instance = 0U;
  }

  /**************** Get current configuration of the instances ****************/
  for (index = 0U; index < OSPI_NB_INSTANCE; index++)
  {
    if (OSPIM_GetConfig(index + 1U, &(IOM_cfg[index])) != HAL_OK)
    {
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
    }
  }

  if (status == HAL_OK)
  {
    /********** Disable both OctoSPI to configure OctoSPI IO Manager **********/
    if ((OCTOSPI1->CR & OCTOSPI_CR_EN) != 0U)
    {
      CLEAR_BIT(OCTOSPI1->CR, OCTOSPI_CR_EN);
      ospi_enabled |= 0x1U;
    }
    if ((OCTOSPI2->CR & OCTOSPI_CR_EN) != 0U)
    {
      CLEAR_BIT(OCTOSPI2->CR, OCTOSPI_CR_EN);
      ospi_enabled |= 0x2U;
    }

    /***************** Deactivation of previous configuration *****************/
    CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[instance].NCSPort - 1U)], OCTOSPIM_PCR_NCSEN);
    if ((OCTOSPIM->CR & OCTOSPIM_CR_MUXEN) != 0U)
    {
      /* De-multiplexing should be performed */
      CLEAR_BIT(OCTOSPIM->CR, OCTOSPIM_CR_MUXEN);

      if (other_instance == 1U)
      {
        SET_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].ClkPort - 1U)], OCTOSPIM_PCR_CLKSRC);
        if (IOM_cfg[other_instance].DQSPort != 0U)
        {
          SET_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].DQSPort - 1U)], OCTOSPIM_PCR_DQSSRC);
        }
        if (IOM_cfg[other_instance].IOLowPort != HAL_OSPIM_IOPORT_NONE)
        {
          SET_BIT(OCTOSPIM->PCR[((IOM_cfg[other_instance].IOLowPort - 1U)& OSPI_IOM_PORT_MASK)], \
                  OCTOSPIM_PCR_IOLSRC_1);
        }
        if (IOM_cfg[other_instance].IOHighPort != HAL_OSPIM_IOPORT_NONE)
        {
          SET_BIT(OCTOSPIM->PCR[((IOM_cfg[other_instance].IOHighPort - 1U)& OSPI_IOM_PORT_MASK)], \
                  OCTOSPIM_PCR_IOHSRC_1);
        }
      }
    }
    else
    {
      if (IOM_cfg[instance].ClkPort != 0U)
      {
        CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[instance].ClkPort - 1U)], OCTOSPIM_PCR_CLKEN);
        if (IOM_cfg[instance].DQSPort != 0U)
        {
          CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[instance].DQSPort - 1U)], OCTOSPIM_PCR_DQSEN);
        }
        if (IOM_cfg[instance].IOLowPort != HAL_OSPIM_IOPORT_NONE)
        {
          CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[instance].IOLowPort - 1U)& OSPI_IOM_PORT_MASK)], OCTOSPIM_PCR_IOLEN);
        }
        if (IOM_cfg[instance].IOHighPort != HAL_OSPIM_IOPORT_NONE)
        {
          CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[instance].IOHighPort - 1U)& OSPI_IOM_PORT_MASK)], OCTOSPIM_PCR_IOHEN);
        }
      }
    }

    /********************* Deactivation of other instance *********************/
    if ((cfg->ClkPort == IOM_cfg[other_instance].ClkPort) || (cfg->NCSPort == IOM_cfg[other_instance].NCSPort) ||
        ((cfg->DQSPort == IOM_cfg[other_instance].DQSPort) && (cfg->DQSPort != 0U)) ||
        (cfg->IOLowPort == IOM_cfg[other_instance].IOLowPort) ||
        (cfg->IOHighPort == IOM_cfg[other_instance].IOHighPort))
    {
      if ((cfg->ClkPort   == IOM_cfg[other_instance].ClkPort)   &&
          (cfg->DQSPort    == IOM_cfg[other_instance].DQSPort)  &&
          (cfg->IOLowPort == IOM_cfg[other_instance].IOLowPort) &&
          (cfg->IOHighPort == IOM_cfg[other_instance].IOHighPort))
      {
        /* Multiplexing should be performed */
        SET_BIT(OCTOSPIM->CR, OCTOSPIM_CR_MUXEN);
      }
      else
      {
        CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].ClkPort - 1U)], OCTOSPIM_PCR_CLKEN);
        if (IOM_cfg[other_instance].DQSPort != 0U)
        {
          CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].DQSPort - 1U)], OCTOSPIM_PCR_DQSEN);
        }
        CLEAR_BIT(OCTOSPIM->PCR[(IOM_cfg[other_instance].NCSPort - 1U)], OCTOSPIM_PCR_NCSEN);
        if (IOM_cfg[other_instance].IOLowPort != HAL_OSPIM_IOPORT_NONE)
        {
          CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[other_instance].IOLowPort - 1U)& OSPI_IOM_PORT_MASK)],
                    OCTOSPIM_PCR_IOLEN);
        }
        if (IOM_cfg[other_instance].IOHighPort != HAL_OSPIM_IOPORT_NONE)
        {
          CLEAR_BIT(OCTOSPIM->PCR[((IOM_cfg[other_instance].IOHighPort - 1U)& OSPI_IOM_PORT_MASK)],
                    OCTOSPIM_PCR_IOHEN);
        }
      }
    }

    /******************** Activation of new configuration *********************/
    MODIFY_REG(OCTOSPIM->PCR[(cfg->NCSPort - 1U)], (OCTOSPIM_PCR_NCSEN | OCTOSPIM_PCR_NCSSRC),
               (OCTOSPIM_PCR_NCSEN | (instance << OCTOSPIM_PCR_NCSSRC_Pos)));

    if ((cfg->Req2AckTime - 1U) > ((OCTOSPIM->CR & OCTOSPIM_CR_REQ2ACK_TIME) >> OCTOSPIM_CR_REQ2ACK_TIME_Pos))
    {
      MODIFY_REG(OCTOSPIM->CR, OCTOSPIM_CR_REQ2ACK_TIME, ((cfg->Req2AckTime - 1U) << OCTOSPIM_CR_REQ2ACK_TIME_Pos));
    }

    if ((OCTOSPIM->CR & OCTOSPIM_CR_MUXEN) != 0U)
    {
      MODIFY_REG(OCTOSPIM->PCR[(cfg->ClkPort - 1U)], (OCTOSPIM_PCR_CLKEN | OCTOSPIM_PCR_CLKSRC), OCTOSPIM_PCR_CLKEN);
      if (cfg->DQSPort != 0U)
      {
        MODIFY_REG(OCTOSPIM->PCR[(cfg->DQSPort - 1U)], (OCTOSPIM_PCR_DQSEN | OCTOSPIM_PCR_DQSSRC), OCTOSPIM_PCR_DQSEN);
      }

      if ((cfg->IOLowPort & OCTOSPIM_PCR_IOLEN) != 0U)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOLowPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC), OCTOSPIM_PCR_IOLEN);
      }
      else if (cfg->IOLowPort != HAL_OSPIM_IOPORT_NONE)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOLowPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC), OCTOSPIM_PCR_IOHEN);
      }
      else
      {
        /* Nothing to do */
      }

      if ((cfg->IOHighPort & OCTOSPIM_PCR_IOLEN) != 0U)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOHighPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC), (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC_0));
      }
      else if (cfg->IOHighPort != HAL_OSPIM_IOPORT_NONE)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOHighPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC), (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC_0));
      }
      else
      {
        /* Nothing to do */
      }
    }
    else
    {
      MODIFY_REG(OCTOSPIM->PCR[(cfg->ClkPort - 1U)], (OCTOSPIM_PCR_CLKEN | OCTOSPIM_PCR_CLKSRC),
                 (OCTOSPIM_PCR_CLKEN | (instance << OCTOSPIM_PCR_CLKSRC_Pos)));
      if (cfg->DQSPort != 0U)
      {
        MODIFY_REG(OCTOSPIM->PCR[(cfg->DQSPort - 1U)], (OCTOSPIM_PCR_DQSEN | OCTOSPIM_PCR_DQSSRC),
                   (OCTOSPIM_PCR_DQSEN | (instance << OCTOSPIM_PCR_DQSSRC_Pos)));
      }

      if ((cfg->IOLowPort & OCTOSPIM_PCR_IOLEN) != 0U)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOLowPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC),
                   (OCTOSPIM_PCR_IOLEN | (instance << (OCTOSPIM_PCR_IOLSRC_Pos + 1U))));
      }
      else if (cfg->IOLowPort != HAL_OSPIM_IOPORT_NONE)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOLowPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC),
                   (OCTOSPIM_PCR_IOHEN | (instance << (OCTOSPIM_PCR_IOHSRC_Pos + 1U))));
      }
      else
      {
        /* Nothing to do */
      }

      if ((cfg->IOHighPort & OCTOSPIM_PCR_IOLEN) != 0U)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOHighPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC),
                   (OCTOSPIM_PCR_IOLEN | OCTOSPIM_PCR_IOLSRC_0 | (instance << (OCTOSPIM_PCR_IOLSRC_Pos + 1U))));
      }
      else if (cfg->IOHighPort != HAL_OSPIM_IOPORT_NONE)
      {
        MODIFY_REG(OCTOSPIM->PCR[((cfg->IOHighPort - 1U)& OSPI_IOM_PORT_MASK)],
                   (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC),
                   (OCTOSPIM_PCR_IOHEN | OCTOSPIM_PCR_IOHSRC_0 | (instance << (OCTOSPIM_PCR_IOHSRC_Pos + 1U))));
      }
    }

    /******* Re-enable both OctoSPI after configure OctoSPI IO Manager ********/
    if ((ospi_enabled & 0x1U) != 0U)
    {
      SET_BIT(OCTOSPI1->CR, OCTOSPI_CR_EN);
    }
    if ((ospi_enabled & 0x2U) != 0U)
    {
      SET_BIT(OCTOSPI2->CR, OCTOSPI_CR_EN);
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  DMA OSPI process complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMACplt(MDMA_HandleTypeDef *hmdma)
{
  OSPI_HandleTypeDef *hospi = (OSPI_HandleTypeDef *)(hmdma->Parent);
  hospi->XferCount = 0;

  /* Disable the DMA transfer on the OctoSPI side */
  CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

  /* Disable the DMA channel */
  __HAL_MDMA_DISABLE(hmdma);

  /* Enable the OSPI transfer complete Interrupt */
  __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC);
}

/**
  * @brief  DMA OSPI communication error callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMAError(MDMA_HandleTypeDef *hmdma)
{
  OSPI_HandleTypeDef *hospi = (OSPI_HandleTypeDef *)(hmdma->Parent);
  hospi->XferCount = 0;
  hospi->ErrorCode = HAL_OSPI_ERROR_DMA;

  /* Disable the DMA transfer on the OctoSPI side */
  CLEAR_BIT(hospi->Instance->CR, OCTOSPI_CR_DMAEN);

  /* Disable all interrupts */
  __HAL_OSPI_DISABLE_IT(hospi, HAL_OSPI_IT_TC | HAL_OSPI_IT_FT | HAL_OSPI_IT_TE);

  /* Update state */
  hospi->State = HAL_OSPI_STATE_ABORT;

  /* Disable the DMA transfer on the DMA side */
  hospi->hmdma->XferAbortCallback = OSPI_DMAAbortOnError;
  if (HAL_MDMA_Abort_IT(hospi->hmdma) != HAL_OK)
  {
    /* Update state */
    hospi->State = HAL_OSPI_STATE_READY;

    /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->ErrorCallback(hospi);
#else
    HAL_OSPI_ErrorCallback(hospi);
#endif /*(USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
  }
}

/**
  * @brief  DMA OSPI abort complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMAAbortOnError(MDMA_HandleTypeDef *hmdma)
{
  OSPI_HandleTypeDef *hospi = (OSPI_HandleTypeDef *)(hmdma->Parent);

  /* DMA abort called by OctoSPI abort */
  if (__HAL_OSPI_GET_FLAG(hospi, HAL_OSPI_FLAG_BUSY) != RESET)
  {
    /* Clear transfer complete flag */
    __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

    /* Enable the transfer complete interrupts */
    __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC);

    /* Perform an abort of the OctoSPI */
    SET_BIT(hospi->Instance->CR, OCTOSPI_CR_ABORT);
  }
  else
  {
    /* Update state */
    hospi->State = HAL_OSPI_STATE_READY;

    /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->ErrorCallback(hospi);
#else
    HAL_OSPI_ErrorCallback(hospi);
#endif /*(USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
  }
}

/**
  * @brief  DMA OSPI abort complete callback.
  * @param  hdma : DMA handle
  * @retval None
  */
static void OSPI_DMAAbortCplt(MDMA_HandleTypeDef *hmdma)
{
  OSPI_HandleTypeDef *hospi = (OSPI_HandleTypeDef *)(hmdma->Parent);
  hospi->XferCount = 0;

  /* Check the state */
  if (hospi->State == HAL_OSPI_STATE_ABORT)
  {
    /* DMA abort called by OctoSPI abort */
    if (__HAL_OSPI_GET_FLAG(hospi, HAL_OSPI_FLAG_BUSY) != RESET)
    {
      /* Clear transfer complete flag */
      __HAL_OSPI_CLEAR_FLAG(hospi, HAL_OSPI_FLAG_TC);

      /* Enable the transfer complete interrupts */
      __HAL_OSPI_ENABLE_IT(hospi, HAL_OSPI_IT_TC);

      /* Perform an abort of the OctoSPI */
      SET_BIT(hospi->Instance->CR, OCTOSPI_CR_ABORT);
    }
    else
    {
      /* Update state */
      hospi->State = HAL_OSPI_STATE_READY;

      /* Abort callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
      hospi->AbortCpltCallback(hospi);
#else
      HAL_OSPI_AbortCpltCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U) */
    }
  }
  else
  {
    /* DMA abort called due to a transfer error interrupt */
    /* Update state */
    hospi->State = HAL_OSPI_STATE_READY;

    /* Error callback */
#if defined (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)
    hospi->ErrorCallback(hospi);
#else
    HAL_OSPI_ErrorCallback(hospi);
#endif /* (USE_HAL_OSPI_REGISTER_CALLBACKS) && (USE_HAL_OSPI_REGISTER_CALLBACKS == 1U)*/
  }
}

/**
  * @brief  Wait for a flag state until timeout.
  * @param  hospi     : OSPI handle
  * @param  Flag      : Flag checked
  * @param  State     : Value of the flag expected
  * @param  Timeout   : Duration of the timeout
  * @param  Tickstart : Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef OSPI_WaitFlagStateUntilTimeout(OSPI_HandleTypeDef *hospi, uint32_t Flag,
                                                        FlagStatus State, uint32_t Tickstart, uint32_t Timeout)
{
  /* Wait until flag is in expected state */
  while ((__HAL_OSPI_GET_FLAG(hospi, Flag)) != State)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hospi->State     = HAL_OSPI_STATE_ERROR;
        hospi->ErrorCode |= HAL_OSPI_ERROR_TIMEOUT;

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/**
  * @brief  Configure the registers for the regular command mode.
  * @param  hospi : OSPI handle
  * @param  cmd   : structure that contains the command configuration information
  * @retval HAL status
  */
static HAL_StatusTypeDef OSPI_ConfigCmd(OSPI_HandleTypeDef *hospi, OSPI_RegularCmdTypeDef *cmd)
{
  HAL_StatusTypeDef status = HAL_OK;
  __IO uint32_t *ccr_reg;
  __IO uint32_t *tcr_reg;
  __IO uint32_t *ir_reg;
  __IO uint32_t *abr_reg;

  /* Re-initialize the value of the functional mode */
  MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FMODE, 0U);

  /* Configure the flash ID */
  if (hospi->Init.DualQuad == HAL_OSPI_DUALQUAD_DISABLE)
  {
    MODIFY_REG(hospi->Instance->CR, OCTOSPI_CR_FSEL, cmd->FlashId);
  }

  if (cmd->OperationType == HAL_OSPI_OPTYPE_WRITE_CFG)
  {
    ccr_reg = &(hospi->Instance->WCCR);
    tcr_reg = &(hospi->Instance->WTCR);
    ir_reg  = &(hospi->Instance->WIR);
    abr_reg = &(hospi->Instance->WABR);
  }
  else if (cmd->OperationType == HAL_OSPI_OPTYPE_WRAP_CFG)
  {
    ccr_reg = &(hospi->Instance->WPCCR);
    tcr_reg = &(hospi->Instance->WPTCR);
    ir_reg  = &(hospi->Instance->WPIR);
    abr_reg = &(hospi->Instance->WPABR);
  }
  else
  {
    ccr_reg = &(hospi->Instance->CCR);
    tcr_reg = &(hospi->Instance->TCR);
    ir_reg  = &(hospi->Instance->IR);
    abr_reg = &(hospi->Instance->ABR);
  }

  /* Configure the CCR register with DQS and SIOO modes */
  *ccr_reg = (cmd->DQSMode | cmd->SIOOMode);

  if (cmd->AlternateBytesMode != HAL_OSPI_ALTERNATE_BYTES_NONE)
  {
    /* Configure the ABR register with alternate bytes value */
    *abr_reg = cmd->AlternateBytes;

    /* Configure the CCR register with alternate bytes communication parameters */
    MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_ABMODE | OCTOSPI_CCR_ABDTR | OCTOSPI_CCR_ABSIZE),
               (cmd->AlternateBytesMode | cmd->AlternateBytesDtrMode | cmd->AlternateBytesSize));
  }

  /* Configure the TCR register with the number of dummy cycles */
  MODIFY_REG((*tcr_reg), OCTOSPI_TCR_DCYC, cmd->DummyCycles);

  if (cmd->DataMode != HAL_OSPI_DATA_NONE)
  {
    if (cmd->OperationType == HAL_OSPI_OPTYPE_COMMON_CFG)
    {
      /* Configure the DLR register with the number of data */
      hospi->Instance->DLR = (cmd->NbData - 1U);
    }
  }

  if (cmd->InstructionMode != HAL_OSPI_INSTRUCTION_NONE)
  {
    if (cmd->AddressMode != HAL_OSPI_ADDRESS_NONE)
    {
      if (cmd->DataMode != HAL_OSPI_DATA_NONE)
      {
        /* ---- Command with instruction, address and data ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE  | OCTOSPI_CCR_IDTR  | OCTOSPI_CCR_ISIZE  |
                                OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE |
                                OCTOSPI_CCR_DMODE  | OCTOSPI_CCR_DDTR),
                   (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize |
                    cmd->AddressMode     | cmd->AddressDtrMode     | cmd->AddressSize     |
                    cmd->DataMode        | cmd->DataDtrMode));
      }
      else
      {
        /* ---- Command with instruction and address ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE  | OCTOSPI_CCR_IDTR  | OCTOSPI_CCR_ISIZE  |
                                OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE),
                   (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize |
                    cmd->AddressMode     | cmd->AddressDtrMode     | cmd->AddressSize));

        /* The DHQC bit is linked with DDTR bit which should be activated */
        if ((hospi->Init.DelayHoldQuarterCycle == HAL_OSPI_DHQC_ENABLE) &&
            (cmd->InstructionDtrMode == HAL_OSPI_INSTRUCTION_DTR_ENABLE))
        {
          MODIFY_REG((*ccr_reg), OCTOSPI_CCR_DDTR, HAL_OSPI_DATA_DTR_ENABLE);
        }
      }

      /* Configure the IR register with the instruction value */
      *ir_reg = cmd->Instruction;

      /* Configure the AR register with the address value */
      hospi->Instance->AR = cmd->Address;
    }
    else
    {
      if (cmd->DataMode != HAL_OSPI_DATA_NONE)
      {
        /* ---- Command with instruction and data ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE | OCTOSPI_CCR_IDTR | OCTOSPI_CCR_ISIZE |
                                OCTOSPI_CCR_DMODE | OCTOSPI_CCR_DDTR),
                   (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize |
                    cmd->DataMode        | cmd->DataDtrMode));
      }
      else
      {
        /* ---- Command with only instruction ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_IMODE | OCTOSPI_CCR_IDTR | OCTOSPI_CCR_ISIZE),
                   (cmd->InstructionMode | cmd->InstructionDtrMode | cmd->InstructionSize));

        /* The DHQC bit is linked with DDTR bit which should be activated */
        if ((hospi->Init.DelayHoldQuarterCycle == HAL_OSPI_DHQC_ENABLE) &&
            (cmd->InstructionDtrMode == HAL_OSPI_INSTRUCTION_DTR_ENABLE))
        {
          MODIFY_REG((*ccr_reg), OCTOSPI_CCR_DDTR, HAL_OSPI_DATA_DTR_ENABLE);
        }
      }

      /* Configure the IR register with the instruction value */
      *ir_reg = cmd->Instruction;

    }
  }
  else
  {
    if (cmd->AddressMode != HAL_OSPI_ADDRESS_NONE)
    {
      if (cmd->DataMode != HAL_OSPI_DATA_NONE)
      {
        /* ---- Command with address and data ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE |
                                OCTOSPI_CCR_DMODE  | OCTOSPI_CCR_DDTR),
                   (cmd->AddressMode | cmd->AddressDtrMode | cmd->AddressSize | cmd->DataMode |
                    cmd->DataDtrMode));
      }
      else
      {
        /* ---- Command with only address ---- */

        /* Configure the CCR register with all communication parameters */
        MODIFY_REG((*ccr_reg), (OCTOSPI_CCR_ADMODE | OCTOSPI_CCR_ADDTR | OCTOSPI_CCR_ADSIZE),
                   (cmd->AddressMode | cmd->AddressDtrMode | cmd->AddressSize));
      }

      /* Configure the AR register with the instruction value */
      hospi->Instance->AR = cmd->Address;
    }
    else
    {
      /* ---- Invalid command configuration (no instruction, no address) ---- */
      status = HAL_ERROR;
      hospi->ErrorCode = HAL_OSPI_ERROR_INVALID_PARAM;
    }
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Get the current IOM configuration for an OctoSPI instance.
  * @param  instance_nb : number of the instance
  * @param  cfg         : configuration of the IO Manager for the instance
  * @retval HAL status
  */
static HAL_StatusTypeDef OSPIM_GetConfig(uint8_t instance_nb, OSPIM_CfgTypeDef *cfg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t reg;
  uint32_t value = 0U;
  uint32_t index;

  if ((instance_nb == 0U) || (instance_nb > OSPI_NB_INSTANCE) || (cfg == NULL))
  {
    /* Invalid parameter -> error returned */
    status = HAL_ERROR;
  }
  else
  {
    /* Initialize the structure */
    cfg->ClkPort    = 0U;
    cfg->DQSPort    = 0U;
    cfg->NCSPort    = 0U;
    cfg->IOLowPort  = 0U;
    cfg->IOHighPort = 0U;

    if (instance_nb == 2U)
    {
      if ((OCTOSPIM->CR & OCTOSPIM_CR_MUXEN) == 0U)
      {
        value = (OCTOSPIM_PCR_CLKSRC | OCTOSPIM_PCR_DQSSRC | OCTOSPIM_PCR_NCSSRC
                 | OCTOSPIM_PCR_IOLSRC_1 | OCTOSPIM_PCR_IOHSRC_1);
      }
      else
      {
        value = OCTOSPIM_PCR_NCSSRC;
      }
    }

    /* Get the information about the instance */
    for (index = 0U; index < OSPI_IOM_NB_PORTS; index ++)
    {
      reg = OCTOSPIM->PCR[index];

      if ((reg & OCTOSPIM_PCR_CLKEN) != 0U)
      {
        /* The clock is enabled on this port */
        if ((reg & OCTOSPIM_PCR_CLKSRC) == (value & OCTOSPIM_PCR_CLKSRC))
        {
          /* The clock correspond to the instance passed as parameter */
          cfg->ClkPort = index + 1U;
        }
      }

      if ((reg & OCTOSPIM_PCR_DQSEN) != 0U)
      {
        /* The DQS is enabled on this port */
        if ((reg & OCTOSPIM_PCR_DQSSRC) == (value & OCTOSPIM_PCR_DQSSRC))
        {
          /* The DQS correspond to the instance passed as parameter */
          cfg->DQSPort = index + 1U;
        }
      }

      if ((reg & OCTOSPIM_PCR_NCSEN) != 0U)
      {
        /* The nCS is enabled on this port */
        if ((reg & OCTOSPIM_PCR_NCSSRC) == (value & OCTOSPIM_PCR_NCSSRC))
        {
          /* The nCS correspond to the instance passed as parameter */
          cfg->NCSPort = index + 1U;
        }
      }

      if ((reg & OCTOSPIM_PCR_IOLEN) != 0U)
      {
        /* The IO Low is enabled on this port */
        if ((reg & OCTOSPIM_PCR_IOLSRC_1) == (value & OCTOSPIM_PCR_IOLSRC_1))
        {
          /* The IO Low correspond to the instance passed as parameter */
          if ((reg & OCTOSPIM_PCR_IOLSRC_0) == 0U)
          {
            cfg->IOLowPort = (OCTOSPIM_PCR_IOLEN | (index + 1U));
          }
          else
          {
            cfg->IOLowPort = (OCTOSPIM_PCR_IOHEN | (index + 1U));
          }
        }
      }

      if ((reg & OCTOSPIM_PCR_IOHEN) != 0U)
      {
        /* The IO High is enabled on this port */
        if ((reg & OCTOSPIM_PCR_IOHSRC_1) == (value & OCTOSPIM_PCR_IOHSRC_1))
        {
          /* The IO High correspond to the instance passed as parameter */
          if ((reg & OCTOSPIM_PCR_IOHSRC_0) == 0U)
          {
            cfg->IOHighPort = (OCTOSPIM_PCR_IOLEN | (index + 1U));
          }
          else
          {
            cfg->IOHighPort = (OCTOSPIM_PCR_IOHEN | (index + 1U));
          }
        }
      }
    }
  }

  /* Return function status */
  return status;
}

#endif /* HAL_OSPI_MODULE_ENABLED */
#endif /* OCTOSPI || OCTOSPI1 || OCTOSPI2 */
#endif // if 0


#if 0 // fron H5 seem almost identical
//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32H5_W25Qxx_xspi.cpp
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define XSPI_DRIVER_GLOBAL
#include "BSP_board.h"
#undef  XSPI_DRIVER_GLOBAL
#include <string.h>

//-------------------------------------------------------------------------------------------------

using namespace INFRA;

//-------------------------------------------------------------------------------------------------
// Private Define(s)
//-------------------------------------------------------------------------------------------------

//#define DEBUG_TEST_QSPI

#define EXPAND_X_CMD_AS_STRUCT_DATA(CMD_ID,  CMD, CMD_MODE, ADDR_MODE, ADDR_SIZE, DATA_MODE, DUMMY ) \
                                           { CMD, CMD_MODE, ADDR_MODE, ADDR_SIZE, DATA_MODE, DUMMY },

#define MEMORY_READY_MATCH_VALUE    0x00
#define MEMORY_READY_MASK_VALUE     0x01
#define AUTO_POLLING_INTERVAL       0x10

#define XSPI_TIMEOUT_DEFAULT_VALUE  5000

#define QSPI_SR_QUAD_ENABLE_BIT     0x02   

#define MEMORY_DRIVE_MASK           0x60
#define MEMORY_DRIVE_AT_25_PERCENT  0x60
#define MEMORY_DRIVE_AT_50_PERCENT  0x40
#define MEMORY_DRIVE_AT_75_PERCENT  0x20
#define MEMORY_DRIVE_AT_100_PERCENT 0x00

#define XSPI_FUNCTIONAL_MODE_INDIRECT_WRITE ((uint32_t)0x00000000)         // Indirect write mode
#define XSPI_FUNCTIONAL_MODE_INDIRECT_READ  ((uint32_t)XSPI_CR_FMODE_0)    // Indirect read mode
#define XSPI_FUNCTIONAL_MODE_AUTO_POLLING   ((uint32_t)XSPI_CR_FMODE_1)    // Automatic polling mode
#define XSPI_FUNCTIONAL_MODE_MEMORY_MAPPED  ((uint32_t)XSPI_CR_FMODE)      // Memory-mapped mode

//-------------------------------------------------------------------------------------------------
//
//   Class: XSPI_Driver
//
//   Description:   Class to handle XSPI W25Q32 memory
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private Constants and Macros
//-------------------------------------------------------------------------------------------------
const W25Q_Command_t W25Q_Driver::m_Cmd[XSPI_NB_OF_CMD] =
{
    XSPI_CMD_DEF(EXPAND_X_CMD_AS_STRUCT_DATA)
};

const DMA_Info_t W25Q_Driver::m_DMA_RX_Info =
{
    BSP_DMA_MODE_NORMAL,
    GPDMA1_REQUEST_OCTOSPI1,
    BSP_DMA_BREQ_SINGLE_BURST,
    BSP_DMA_PERIPHERAL_TO_MEMORY,
    (BSP_DMA_SOURCE_NO_INCREMENT | BSP_DMA_DESTINATION_INCREMENT),
    (BSP_DMA_SOURCE_SIZE_8_BITS  | BSP_DMA_DESTINATION_SIZE_8_BITS),
    BSP_DMA_PRIORITY_LOW_PRIO_WEIGHT_LOW,
    1,
    1,
    BSP_DMA_TC_EVENT_BLOCK_TRANSFER,
    GPDMA1_Channel4,
    DMA_CFCR_TCF,                                   // Transfer complete flag
    GPDMA1_Channel4_IRQn,
    configMAX_SYSCALL_INTERRUPT_PRIORITY,           // PreempPrio
    DMA_LINKED_LIST_DISABLE,
};

const DMA_Info_t W25Q_Driver::m_DMA_TX_Info =
{
    BSP_DMA_MODE_NORMAL,
    GPDMA1_REQUEST_OCTOSPI1,
    BSP_DMA_BREQ_SINGLE_BURST,
    BSP_DMA_MEMORY_TO_PERIPHERAL,
    (BSP_DMA_SOURCE_INCREMENT   | BSP_DMA_DESTINATION_NO_INCREMENT),
    (BSP_DMA_SOURCE_SIZE_8_BITS | BSP_DMA_DESTINATION_SIZE_8_BITS),
    BSP_DMA_PRIORITY_LOW_PRIO_WEIGHT_LOW,
    1,
    1,
    BSP_DMA_TC_EVENT_BLOCK_TRANSFER,
    GPDMA1_Channel5,
    DMA_CFCR_TCF,                                   // Transfer complete flag
    GPDMA1_Channel5_IRQn,
    configMAX_SYSCALL_INTERRUPT_PRIORITY,           // PreempPrio
    DMA_LINKED_LIST_DISABLE,
};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   W25Q_Driver
//
//   Parameter(s):  None
//
//   Description:   Initializes the SPI peripheral according to the specified parameters
//
//-------------------------------------------------------------------------------------------------
W25Q_Driver::W25Q_Driver() : TFlash()
{
  #ifdef USE_FREERTOS
    m_IsItInitialize  = false;
  #else
    m_IsItDMA_Busy    = false;
  #endif  
    m_Status          = SYS_RESET;
    m_ErrorCode       = SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Initialize
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Initialize the SPI module
//
//-------------------------------------------------------------------------------------------------
#ifdef DEBUG_TEST_QSPI
uint8_t buffer[256];
uint8_t Buffer1[5000];
uint8_t Buffer2[256];
uint8_t Buffer3[256];
#include <stdio.h>
uint32_t    EraseCounter = 0;
uint32_t    ReadCounter = 0;
uint32_t    WriteErrorCounter = 0;
uint32_t    ReadErrorCounter1 = 0;
uint32_t    ReadErrorCounter2 = 0;
uint32_t    ReadErrorCounter3 = 0;
#endif

SystemState_e W25Q_Driver::Initialize(void)
{
    SystemState_e      State = SYS_READY;
    uint32_t           FlashID;
    uint32_t           Density;

    m_pXSPI = OCTOSPI1;

    XSPI_MSP_Init();              // Initialization of the low level hardware and OCTOSPI

    if(m_Status == SYS_READY)
    {
        m_DMA_TX.Initialize((DMA_Info_t*)&m_DMA_TX_Info);
        m_DMA_TX.SetDestination((void*)&m_pXSPI->DR);          // Configure transmit data register
        m_DMA_RX.Initialize((DMA_Info_t*)&m_DMA_RX_Info);
        m_DMA_RX.SetSource((void*)&m_pXSPI->DR);               // Configure receive data register

      #ifdef USE_FREERTOS
        if(m_IsItInitialize == false)
        {
            m_IsItInitialize = true;
            m_Mutex   = xSemaphoreCreateMutex();                // Create a mutex type semaphore.
            m_DMA_Busy = xSemaphoreCreateBinary();              // Create a binary type semaphore.
        }
      #endif

        if((State = ReadID(&FlashID)) == SYS_READY)
        {
            if((State = ReadSFDP_Density(&Density)) == SYS_READY)
            {
                State = QuadEnable();
            }
        }
    }

    if(State == SYS_READY)
    {
        State = SetDriverStrength(MEMORY_DRIVE_AT_50_PERCENT);
    }

    if(State == SYS_READY)
    {
        m_Status = SYS_READY;

        if(Density > 0)
        {
            m_FlashInfo.PageSize        = 256;
            m_FlashInfo.NbPages         = Density / 256;
            m_FlashInfo.SectorSize      = 1024 * 4;
            m_FlashInfo.SectorEraseSize = 1024 * 4;
            m_FlashInfo.NbSectors       = Density / 4096;
            m_FlashInfo.FlashID         = FlashID;
        }
        else
        {
            // Error... Flash not supported.
            m_FlashInfo.PageSize  = 0;
            m_FlashInfo.NbSectors = 0;
            m_FlashInfo.FlashID   = FlashID;
        }
    }
    else
    {
        m_Status = SYS_INITIALIZATION_FAIL;
    }

#ifdef DEBUG_TEST_QSPI
    memset(Buffer1, 0xFF, 5000);
    memset(Buffer2, 0xFF, 256);
    memset(Buffer3, 0xFF, 256);

    memcpy(Buffer2, "The quick brown fox jumps over the lazy dog. Portez ce vieux whisky au juge blond qui fume. 0123456789", 102);
    //memcpy(Buffer1, "poutine qwertyuiopasdfghjklzxcvbnm,./<>?;':|}{+_)(*&^%$#@!`~ POUTINE", 68);
    memcpy(Buffer3, "0123456789876543210123456789876543210abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 89);

/*
	write(Buffer1, 100, 10);
	write(Buffer1, 4000, 5000);
	write(Buffer1, 220, 102);
	write(Buffer2, 1000, 68);
	write(Buffer3, 4080, 89);

    read(buffer, 256,  256);
    read(buffer, 0,    256);
    read(buffer, 220,  256);
    read(buffer, 256,  256);
    read(buffer, 1024,  256);
    read(buffer, 1000,  256);
    read(buffer, 4096,  256);
    read(buffer, 4096 - 256, 256);
    read(buffer, 4096 + 10, 256);
*/

	BulkErase();

    for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
    {
        memset(Buffer1, 0xFF, 256);     // Erase buffer
        snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
        write(Buffer1, i * m_FlashInfo.PageSize, 64);
    }

    write(Buffer2, (5 * m_FlashInfo.PageSize) + 128 , 102);

uint8_t Count = 0;

    while(1)
    {
        Count++;

        if((Count % 10) == 0)
        {
            Count = 0;
            BulkErase();
            EraseCounter++;

            // Check if all page are empty 
            memset(Buffer1, 0xFF, 256);
            for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
            {
                read(buffer, i * m_FlashInfo.PageSize, 256);

                if(memcmp(buffer, Buffer1, 64) != 0)
                {
                   ReadErrorCounter1++;
                }
            }

            // Rewrite test content
            for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
            {
                memset(Buffer1, 0xFF, 256);     // Erase buffer
                snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
                write(Buffer1, i * m_FlashInfo.PageSize, 64);
            }

            for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
            {
                read(buffer, i * m_FlashInfo.PageSize, 64);

                memset(Buffer1, 0xFF, 256);     // Erase buffer
                snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
                if(memcmp(buffer, Buffer1, 64) != 0)
                {
                    WriteErrorCounter++;
                }
            }



            write(Buffer2, (5 * m_FlashInfo.PageSize) + 128 , 102);
        }

        for(uint32_t i = 0; i < m_FlashInfo.NbPages; i++)
        {
            read(buffer, i * m_FlashInfo.PageSize, 64);

            memset(Buffer1, 0xFF, 256);     // Erase buffer
            snprintf((char*)Buffer1, 64, "This is the page number %ld and have fun testing this", i);
            if(memcmp(buffer, Buffer1, 64) != 0)
            {
                ReadErrorCounter2++;
            }
        }
    
        read(buffer, (5 * m_FlashInfo.PageSize) + 128, 128);
        if(memcmp(buffer, Buffer2, 128) != 0)
        {
            ReadErrorCounter3++;
        }

        ReadCounter++;

    }
#endif    

    return State;
}

//---------------------------------------------------------------------------------------------
//
//   Function:      XSPI_MSP_Init
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Low level Init XSPI
//
//---------------------------------------------------------------------------------------------
void W25Q_Driver::XSPI_MSP_Init(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    if(m_Status == SYS_RESET)                                                           // Check if the state is the reset state
    {
        memset(&PeriphClkInitStruct, 0, sizeof(RCC_PeriphCLKInitTypeDef));

        // Initializes the peripherals clock
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_OSPI;
        PeriphClkInitStruct.OspiClockSelection   = RCC_OSPICLKSOURCE_HCLK;
        HAL_RCCEx_GetPeriphCLKConfig(&PeriphClkInitStruct);

        RCC->AHB4ENR |= RCC_AHB4ENR_OCTOSPI1EN;                                         // Enable clock on XSPI module

        // Reset the XSPI memory interface
        RCC->AHB4RSTR |=   RCC_AHB4RSTR_OCTOSPI1RST;
        RCC->AHB4RSTR &= ~(RCC_AHB4RSTR_OCTOSPI1RST);

        IO_PinInit(IO_XSPI_CLK);
        IO_PinInit(IO_XSPI_CS);
        IO_PinInit(IO_XSPI_IO0);
        IO_PinInit(IO_XSPI_IO1);
        IO_PinInit(IO_XSPI_IO2);
        IO_PinInit(IO_XSPI_IO3);

        // Configure memory type, device size, chip select high time, free running clock, clock mode
        MODIFY_REG(m_pXSPI->DCR1, (XSPI_DCR1_MTYP  | XSPI_DCR1_DEVSIZE | XSPI_DCR1_CSHT         | XSPI_DCR1_FRCK           | XSPI_DCR1_CKMODE),
                                  (W25_MEMORY_TYPE | W25_MEMORY_SIZE   | W25_CS_HIGH_TIME_CYCLE | W25_FREE_RUN_CLK_DISABLE | W25_CLOCK_MODE_0));

        SET_BIT(m_pXSPI->DCR1, OCTOSPI_DCR1_DLYBYP);                                                                        // Delay block is bypassed
        CLEAR_BIT(m_pXSPI->DCR2, XSPI_DCR2_WRAPSIZE);                                                                       // Configure wrap size not supported (0)
        CLEAR_BIT(m_pXSPI->DCR3, XSPI_DCR3_CSBOUND);                                                                        // Configure chip select boundary to none
        m_pXSPI->DCR4 = 0;                                                                                                  // Configure refresh
        CLEAR_BIT(m_pXSPI->CR, XSPI_CR_FTHRES);                                                                             // Configure FIFO threshold to 1 byte (0)
        m_Status = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_BUSY, 0, XSPI_TIMEOUT_DEFAULT_VALUE);                            // Wait till busy flag is reset

        if(m_Status == SYS_READY)
        {
            MODIFY_REG(m_pXSPI->DCR2, XSPI_DCR2_PRESCALER, W25_CLOCK_PRESCALER);                                            // Configure clock prescaler
            CLEAR_BIT(m_pXSPI->CR, XSPI_CR_DMM);                                                                            // Configure Memory mode to single memory mode
            MODIFY_REG(m_pXSPI->TCR, (XSPI_TCR_SSHIFT | XSPI_TCR_DHQC), (W25_SAMPLE_SHIFT_NONE | W25_XSPI_DHQC_DISABLE));   // Configure sample shifting and delay hold quarter cycle
            SET_BIT(m_pXSPI->CR, XSPI_CR_EN);                                                                               // Enable XSPI
        }
    
        m_Status = SYS_READY;
    }
}

//---------------------------------------------------------------------------------------------
//
//   Function:      WaitFlagStateUntilTimeout
//
//   Parameter(s):  Flag            Flag to test
//   uint32_t       State           Expected state of the flag,
//                  TimeOut         Timeout duration
//   Return Value:  SystemState_e   State of the function
//
//   Description:   Wait XSPI module to be ready with the requested flag
//
//---------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WaitFlagStateUntilTimeout(uint32_t Flag, uint32_t State, uint32_t TimeOut)
{
    uint32_t Tickstart = HAL_GetTick(); 

    while((READ_BIT(m_pXSPI->SR, Flag) == State) ? FALSE : TRUE)        // Wait until flag is in expected state
    {
        if(TimeOut != HAL_MAX_DELAY)                                    // Check for the Timeout
        {
            if(((HAL_GetTick() - Tickstart) > TimeOut) || (TimeOut == 0))
            {
                m_Status = SYS_ERROR;
                return SYS_TIME_OUT;
            }
        }
    }

    return SYS_READY;
}

//---------------------------------------------------------------------------------------------
//
//   Function:      XSPI_Command
//
//   Parameter(s):  pCmd            Struct that contains the command configuration information
//                  Timeout         Timeout duration
//   Return Value:  SystemState_e   State of the function
//
//   Description:   Set the command configuration.
//
//---------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::XSPI_Command(W25Q_XSPI_RegularCmd_t* const pCmd, uint32_t Timeout)
{
    SystemState_e State;
    
    State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_BUSY, 0, Timeout);      // Wait till busy flag is reset

    if(State == SYS_READY)
    {
        m_ErrorCode = SYS_READY;                                            // Initialize error code
        State = ConfigCmd(pCmd);                                            // Configure the registers

        if(State == SYS_READY)
        {
            if(pCmd->DataMode == HAL_XSPI_DATA_NONE)
            {
                // When there is no data phase, the transfer start as soon as the configuration is done so wait until TC flag is set to go back in idle state
                State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_TC, HAL_XSPI_FLAG_TC, Timeout);
                m_pXSPI->FCR = HAL_XSPI_FLAG_TC;
            }
        }
    }
    else
    {
        State = SYS_BUSY;
    }

    return State;
}

//---------------------------------------------------------------------------------------------
//
//   Function:      XSPI_Command
//
//   Parameter(s):  pCmd
//   Return Value:  SystemState_e   State of the function
//
//   Description:   Configure the registers for the regular command mode.
//
//   Note(s):       Custom tailored for the W25
//
//---------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ConfigCmd(W25Q_XSPI_RegularCmd_t* pCmd)
{
    SystemState_e State = SYS_READY;

    MODIFY_REG(m_pXSPI->CR, XSPI_CR_FMODE | XSPI_CR_MSEL, 0);           // Re-initialize the value of the functional mode in CR
    m_pXSPI->CCR = 0;                                                   // Re-initialize the value with DQS and SIOO modes in CCR
    MODIFY_REG(m_pXSPI->TCR, XSPI_TCR_DCYC, pCmd->DummyCycles);         // Configure the TCR register with the number of dummy cycles

    if(pCmd->DataMode != HAL_XSPI_DATA_NONE)
    {
        m_pXSPI->DLR = (pCmd->DataLength - 1);                          // Configure the DLR register with the number of data
    }

    if(pCmd->InstructionMode != HAL_XSPI_INSTRUCTION_NONE)
    {
        if(pCmd->AddressMode != HAL_XSPI_ADDRESS_NONE)
        {
            if(pCmd->DataMode != HAL_XSPI_DATA_NONE)                    // ---- Command with instruction, address and data ----
            {
                // Configure the CCR register with all communication parameters
                MODIFY_REG(m_pXSPI->CCR, (XSPI_CCR_IMODE | XSPI_CCR_ADMODE | XSPI_CCR_ADSIZE | XSPI_CCR_DMODE),
                                         (pCmd->InstructionMode | pCmd->AddressMode | pCmd->AddressWidth | pCmd->DataMode));
            }
            else                                                        // ---- Command with instruction and address ----
            {
                // Configure the CCR register with all communication parameters
                MODIFY_REG(m_pXSPI->CCR, (XSPI_CCR_IMODE | XSPI_CCR_ADMODE), (pCmd->InstructionMode | pCmd->AddressMode));
            }

            m_pXSPI->IR = pCmd->Instruction;                            // Configure the IR register with the instruction value
            m_pXSPI->AR = pCmd->Address;                                // Configure the AR register with the address value
        }
        else
        {
            if(pCmd->DataMode != HAL_XSPI_DATA_NONE)                    // ---- Command with instruction and data ----
            {

                // Configure the CCR register with all communication parameters
                MODIFY_REG(m_pXSPI->CCR, (XSPI_CCR_IMODE | XSPI_CCR_DMODE),  (pCmd->InstructionMode | pCmd->DataMode));
            }
            else                                                        // ---- Command with only instruction ----
            {
                // Configure the CCR register with all communication parameters
                MODIFY_REG(m_pXSPI->CCR, XSPI_CCR_IMODE, pCmd->InstructionMode);
            }

            m_pXSPI->IR = pCmd->Instruction;                            // Configure the IR register with the instruction value
        }
    }
    else
    {
        if(pCmd->AddressMode != HAL_XSPI_ADDRESS_NONE)
        {
            if(pCmd->DataMode != HAL_XSPI_DATA_NONE)                    // ---- Command with address and data ----
            {
                // Configure the CCR register with all communication parameters
                MODIFY_REG(m_pXSPI->CCR, (XSPI_CCR_ADMODE | XSPI_CCR_DMODE), (pCmd->AddressMode | pCmd->DataMode));
            }
            else                                                        // ---- Command with only address ----
            {
                // Configure the CCR register with all communication parameters
                MODIFY_REG(m_pXSPI->CCR, (XSPI_CCR_ADMODE | XSPI_CCR_ADSIZE), (pCmd->AddressMode | pCmd->AddressWidth));
            }

            m_pXSPI->AR = pCmd->Address;                                // Configure the AR register with the instruction value
        }
        else
        {
            // ---- Invalid command configuration (no instruction, no address) ----
            State       = SYS_ERROR;
            m_ErrorCode = SYS_INVALID_PARAMETER;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      write
//
//   Parameter(s):  pBuffer                 Buffer of the data to write
//                  Address                 Address where to write the data
//                  Size                    number of bytes to write
//   Return Value:  bool                    true or false
//
//   Description:   Write data to the flash
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::write(void* pBuffer, uint32_t Address, size_t Size)
{
	SystemState_e  State = SYS_READY;
    uint8_t*       pBuf  = (uint8_t*)pBuffer;
    uint32_t       PageRemainSize;

    PageRemainSize = m_FlashInfo.PageSize - (Address % m_FlashInfo.PageSize);
    
    // Check if the size of the data is less than the remaining place in the page
    if(PageRemainSize > Size)
    {
        PageRemainSize = Size;
    }

    do
    {
        State = WritePage(pBuf, Address, PageRemainSize);

        // Update the address and size variables for next page programming
        Address += PageRemainSize;
        pBuf    += PageRemainSize;
        Size    -= PageRemainSize;
        PageRemainSize = (Size > m_FlashInfo.PageSize) ? m_FlashInfo.PageSize : Size;
    }
    while((Size != 0) && (State == SYS_READY));

	return (State == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      read
//
//   Parameter(s):  pBuffer                 Buffer of the data to write
//                  Address                 Address where to write the data
//                  Size                    number of bytes to write
//   Return Value:  bool                    true or false
//
//   Description:   Read data from the flash
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::read(void* pBuffer, uint32_t Address, size_t Size)
{
	SystemState_e State = SYS_READY;

	if(Size > 0)
	{
        State = ReadCommand(XSPI_CMD_FAST_READ, Address, (uint8_t*)pBuffer, Size, XSPI_TIMEOUT_DEFAULT_VALUE);
	}

    return (State == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WritePage
//
//   Parameter(s):  pBuffer                 Buffer of the data to write
//                  Address                 Address where to write the data
//                  Size                    number of bytes to write
//   Return Value:  SystemState_e           State of the function
//
//   Description:   Write a page data to the flash
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WritePage(void* pBuffer, uint32_t Address, size_t Size)
{
    SystemState_e State;

    State = WriteCommand(XSPI_CMD_PAGE_PROG, Address, (uint8_t*)pBuffer, Size, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      readID
//
//   Parameter(s):  None
//   Return Value:  uint32_t            ChipID
//
//   Description:   Get the chip ID
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::readID(void)
{
    return m_FlashInfo.FlashID;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getEraseSectorSize
//
//   Parameter(s):  None
//   Return Value:  uint32_t            Page Size
//
//   Description:   Get the page size
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::getEraseSectorSize(void)
{
    return m_FlashInfo.SectorEraseSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getPageSize
//
//   Parameter(s):  None
//   Return Value:  uint32_t            Page Size
//
//   Description:   Get the page size
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::getPageSize(void)
{
    return m_FlashInfo.PageSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getSectorSize
//
//   Parameter(s):  None
//   Return Value:  uint32_t            Sector Size
//
//   Description:   Get the sector size
//
//-------------------------------------------------------------------------------------------------
uint32_t W25Q_Driver::getSectorSize(void)
{
    return m_FlashInfo.SectorSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      eraseSector
//
//   Parameter(s):  SectorAddress       Address of the sector
//   Return Value:  bool                true or false
//
//   Description:   Erase a specific sector
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::eraseSector(uint32_t SectorAddress)
{
    return (EraseSector(SectorAddress, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      getSizeInBytes
//
//   Parameter(s):  None
//   Return Value:  uint64_t            Flash Size
//
//   Description:   Get the flash size
//
//-------------------------------------------------------------------------------------------------
uint64_t W25Q_Driver::getSizeInBytes(void)
{
    return m_FlashInfo.NbPages * m_FlashInfo.PageSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      sync
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   wait for quad flash to be ready
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::sync(void)
{
    Lock();       
    WaitReady(HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    Unlock();
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      BulkErase
//
//   Parameter(s):  None
//   Return Value:  return true or false
//
//   Description:   Erase all the flash content.
//
//-------------------------------------------------------------------------------------------------
bool W25Q_Driver::BulkErase(void)
{
    SystemState_e State;
	
    if((State = WriteEnable()) == SYS_READY)
    {
        State = SendCommand(XSPI_CMD_CHIP_ERASE);
    }

	return (State == SYS_READY) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      EraseSector
//
//   Parameter(s):  SectorAddress       Address of the sector
//                  
//   Return Value:  SystemState_e       SYS_READY of other ERROR
//
//   Description:  Erase a specific sector
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::EraseSector(uint32_t SectorAddress, TickType_t TimeOut)
{
    SystemState_e State;
	
    State = WriteCommand(XSPI_CMD_SECTOR_ERASE, SectorAddress, nullptr, 0, TimeOut);

	return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadCommand
//
//   Parameter(s):  Command             Command ID to send
//   Return Value:  SystemState_e       State of the function
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ReadCommand(W25Q_Command_e Cmd, uint32_t Address, uint8_t* pBuffer, size_t Size, TickType_t TimeOut)
{
    SystemState_e          State = SYS_READY;
    W25Q_XSPI_RegularCmd_t Command;

    PreConfigCommand(&Command, Cmd);
    Command.Address    = Address;
    Command.DataLength = uint32_t(Size);
    Lock();

    if(XSPI_Command(&Command, TimeOut) == SYS_READY)
    {
      #ifndef USE_FREERTOS        
        m_IsItDMA_Busy = true;
      #endif
        State = Receive_DMA(pBuffer);

        if(State == SYS_READY)
        {
            State = WaitForDMA();
        }
    }

    Unlock();
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteCommand
//
//   Parameter(s):  Command     Command ID to send
//                  Address     Address in flash memory
//                  pBuffer     Data to write
//                  Size        Size of the data
//                  TimeOut     Time out before failing

//   Return Value:  None
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WriteCommand(W25Q_Command_e Cmd, uint32_t Address, uint8_t* pBuffer, size_t Size, TickType_t TimeOut, bool IsWR_Enable)
{
    SystemState_e          State = SYS_READY;
    W25Q_XSPI_RegularCmd_t Command;

    if(IsWR_Enable == true)
    {
        State = WriteEnable();
    }

    if(State == SYS_READY)
    {
        PreConfigCommand(&Command, Cmd);
        Command.Address    = Address;
        Command.DataLength = uint32_t(Size);


        if((State = XSPI_Command(&Command, TimeOut)) == SYS_READY)
        {
          #ifndef USE_FREERTOS        
            m_IsItDMA_Busy = true;
          #endif
            State = Transmit_DMA(pBuffer);
        }

        if(State == SYS_READY)
        {
            if((State = WaitForDMA()) == SYS_READY)
            {
                State = WaitReady(HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadID
//
//   Parameter(s):  pChipID                 Pointer on variable to deposit Chip ID
//   Return Value:  SystemState_e
//
//   Description:   Get the Chip ID identification
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ReadID(uint32_t* pChipID)
{
    SystemState_e State;
    uint8_t       Buffer[3];

    State = ReadCommand(XSPI_CMD_READ_ID, 0, Buffer, 3, XSPI_TIMEOUT_DEFAULT_VALUE);
	*pChipID = (uint32_t(Buffer[0]) << 16) | (uint32_t(Buffer[1]) << 8) | uint32_t(Buffer[2]);
	
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       QuadEnable
//
//  Parameter(s):   None
//  Return Value:   SystemState_e
//
//  Description:    Set the QUAD Enable bit in status register 2.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::QuadEnable(void)
{
    SystemState_e State = SYS_READY;
    uint8_t       Status;

    if((State = ReadCommand(XSPI_CMD_READ_STATUS_2, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        // Make sure we are not in already enable!
        if((Status & QSPI_SR_QUAD_ENABLE_BIT) == 0)
        {
            Status |= QSPI_SR_QUAD_ENABLE_BIT;
            State = WriteCommand(XSPI_CMD_WRITE_STATUS_2, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE, false);

            if(State == SYS_READY)
            {
                Status = 0;

                if((State = ReadCommand(XSPI_CMD_READ_STATUS_2, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
                {
                    if((Status & QSPI_SR_QUAD_ENABLE_BIT) == 0)
                    {
                        State = SYS_INITIALIZATION_FAIL;
                    }
                }
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SetDriverStrength
//
//   Parameter(s):  uint8_t     Strength         Strength value for the output of the IO0 - IO3   
//   Return Value:  
//
//   Description:   This function set the strength of the driver output.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::SetDriverStrength(uint8_t Strength)
{
    SystemState_e State = SYS_READY;
    uint8_t       Status;

    if((State = ReadCommand(XSPI_CMD_READ_STATUS_3, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        // Make sure we are not already at the right driver setting!
        if((Status & MEMORY_DRIVE_MASK) != Strength)
        {
            Status &= ~uint8_t(MEMORY_DRIVE_MASK);
            Status |= Strength;
            State = WriteCommand(XSPI_CMD_WRITE_STATUS_3, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE, true);

            if(State == SYS_READY)
            {
                Status = 0;

                if((State = ReadCommand(XSPI_CMD_READ_STATUS_3, 0, &Status, 1, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
                {
                    if((Status & MEMORY_DRIVE_MASK) != Strength)
                    {
                        State = SYS_INITIALIZATION_FAIL;
                    }
                }
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      WriteEnable
//
//   Parameter(s):  
//   Return Value:  
//
//   Description:   This function send a Write Enable and wait it is effective.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WriteEnable(void)
{
    return SendCommand(XSPI_CMD_WRITE_ENABLE);                // Enable write operations
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      PreConfigCommand
//
//   Parameter(s):  pCommand        Command structure to fill
//   Command        Command         Command ID to configure
//   Return Value:  None
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::PreConfigCommand(W25Q_XSPI_RegularCmd_t* pCommand, W25Q_Command_e Command)
{
    memset(pCommand, 0, sizeof(W25Q_XSPI_RegularCmd_t));

    pCommand->Instruction        = m_Cmd[Command].Instruction;
    pCommand->InstructionMode    = m_Cmd[Command].InstructionMode;
    pCommand->AddressMode        = m_Cmd[Command].AddressMode;
    pCommand->AddressWidth       = m_Cmd[Command].AddressSize;
    pCommand->DataMode           = m_Cmd[Command].DataMode;
    pCommand->DummyCycles        = m_Cmd[Command].DummyCycles;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SendCommand
//
//   Parameter(s):  Cmd                 Command ID to send
//   Return Value:  SystemState_e
//
//   Description:   Send a specific command
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::SendCommand(W25Q_Command_e Cmd)
{
    SystemState_e          State;
    W25Q_XSPI_RegularCmd_t Command;

    PreConfigCommand(&Command, Cmd);
    
    if((State = XSPI_Command(&Command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        State = WaitReady(HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       WaitReady
//
//  Parameter(s):   TimeOut      Duration of the time out
//  Return Value:
//
//  Description:    Wait for the SPI module to be ready.
//
//  Note(s):        return  SYS_READY     if available
//                          SYS_TIME_OUT  if busy pass timeout
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WaitReady(TickType_t TimeOut)
{
    W25Q_XSPI_RegularCmd_t  Command;
    SystemState_e           State               = SYS_READY;
//    uint32_t                AddressRegister     = m_pXSPI->AR;
//    uint32_t                InstructionRegister = m_pXSPI->IR;

    PreConfigCommand(&Command, XSPI_CMD_READ_STATUS);
    Command.DataLength = 1;
    
    if(XSPI_Command(&Command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == SYS_READY)
    {
        State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_BUSY, 0, TimeOut);                 // Wait till busy flag is reset

        if(State == SYS_READY)
        {
            // Configure registers
            WRITE_REG(m_pXSPI->PSMAR, MEMORY_READY_MATCH_VALUE);
            WRITE_REG(m_pXSPI->PSMKR, MEMORY_READY_MASK_VALUE);
            WRITE_REG(m_pXSPI->PIR,   AUTO_POLLING_INTERVAL);
            MODIFY_REG(m_pXSPI->CR,   (XSPI_CR_PMM | XSPI_CR_APMS | XSPI_CR_FMODE),
                                      (HAL_XSPI_MATCH_MODE_AND | HAL_XSPI_AUTOMATIC_STOP_ENABLE | XSPI_FUNCTIONAL_MODE_AUTO_POLLING));

            if(READ_BIT(m_pXSPI->CCR, XSPI_CCR_ADMODE) != HAL_XSPI_ADDRESS_NONE)                // Trig the transfer by re-writing address or instruction register
            {
                WRITE_REG(m_pXSPI->AR, Command.Address); //AddressRegister);
            }
            else
            {
                WRITE_REG(m_pXSPI->IR, Command.Instruction);//InstructionRegister);
            }

            State = WaitFlagStateUntilTimeout(HAL_XSPI_FLAG_SM, HAL_XSPI_FLAG_SM, TimeOut);     // Wait till status match flag is set to go back in idle state

            if(State == SYS_READY)
            {
                WRITE_REG(m_pXSPI->FCR, HAL_XSPI_FLAG_SM);                                      // Clear status match flag
                m_Status = SYS_READY;
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Lock the driver
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::Lock(void)
{
  #ifdef USE_FREERTOS
    taskENTER_CRITICAL();
    xSemaphoreTake(m_Mutex, portMAX_DELAY);
  #endif
    m_Status = SYS_LOCK;
  #ifdef USE_FREERTOS
    taskEXIT_CRITICAL();
  #endif    
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Unlock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Unlock the driver
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::Unlock(void)
{
  #ifdef USE_FREERTOS
    taskENTER_CRITICAL();
    xSemaphoreGive(m_Mutex);
  #endif 
    m_Status = SYS_UNLOCK;
  #ifdef USE_FREERTOS
    taskEXIT_CRITICAL();
  #endif    
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      ReadSFDP_Density
//
//   Parameter(s):  uint32_t*       pDensity            Pointer to return density value
//   Return Value:  SystemState_e
//
//   Description:   Get density value of the flash
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::ReadSFDP_Density(uint32_t* pDensity)
{
    SystemState_e State = SYS_READY;
    
    *pDensity = 0;

    // Check JEDEC serial flash discoverable parameters for device specific info
    uint8_t Header[16];

    if((State = ReadCommand(XSPI_CMD_READ_DISCOVERY_PARAMETER, 0, Header, 16, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
    {
        // Verify SFDP signature for sanity, and check that major/minor version is acceptable
        if((memcmp(&Header[0], "SFDP", 4) != 0) || (Header[5] != 1))
        {
            return State;
        }

        // The SFDP spec indicates the standard table is always at offset 0
        // in the parameter headers, we check just to be safe
        if((Header[8] != 0) || (Header[10] != 1))
        {
            return State;
        }

        // Parameter table pointer, spi commands are BE, SFDP is LE,
        // also sfdp command expects extra read wait byte
        uint8_t Table[8];
        uint32_t TableAddress = (Header[14] << 16) | (Header[13] << 8) | Header[12];
        
        if((State = ReadCommand(XSPI_CMD_READ_DISCOVERY_PARAMETER, TableAddress, Table, 8, XSPI_TIMEOUT_DEFAULT_VALUE)) == SYS_READY)
        {
            // Check erase size, currently only supports 4 kbytes
            if(((Table[0] & 0x3) != 0x1) || (Table[1] != 0x20)) // 4K erase sector
            {
                return State;
            }

            // Check address size, currently only supports 3 bytes addresses
            if(((Table[2] & 0x4) != 0) || ((Table[7] & 0x80) != 0))
            {
                return State;
            }

            // Get device density, stored as size in bits - 1
            *pDensity = ((Table[7] << 24) |  (Table[6] << 16) | (Table[5] << 8 ) | (Table[4] << 0 ));
            *pDensity = (*pDensity + 1) / 8;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Transmit_DMA
//
//   Parameter(s):  uint8_t* pData          Pointer to data buffer
//   Return Value:  SystemState_e
//
//   Description:   Send an amount of data in non-blocking mode with DMA.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::Transmit_DMA(uint8_t* pData)
{
    SystemState_e  State     = SYS_READY;
    uint32_t       DataSize  = m_pXSPI->DLR + 1;

    #ifndef USE_FREERTOS        
    m_IsItDMA_Busy = true;
    #endif  

    MODIFY_REG(m_pXSPI->CR, XSPI_CR_FMODE, XSPI_FUNCTIONAL_MODE_INDIRECT_WRITE);	            // Configure CR register with functional mode as indirect write
    WRITE_REG(m_pXSPI->FCR, HAL_XSPI_FLAG_TE | HAL_XSPI_FLAG_TC); 				                // Clear flags related to interrupt
    m_DMA_TX.EnableInterrupt(DMA_IT_TC | DMA_IT_DTE | DMA_IT_ULE | DMA_IT_USE | DMA_IT_TO);
    m_DMA_TX.SetSource(pData);                                                                  // Set DMA source
    m_DMA_TX.SetLength(DataSize);                                                               // Set size of the TX
    m_DMA_TX.WriteTransferRegisters();
    m_Status = SYS_BUSY_TX;                                                                     // Update the state
    m_DMA_TX.Enable();                                                                          // Enable the DMA module
    m_DMA_TX.ClearFlag();                                                                       // Clear IRQ DMA flag
    SET_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);                                                        // Enable the DMA transfer by setting the DMAEN bit

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Receive_DMA
//
//   Parameter(s):  uint8_t* pData          Pointer to data buffer
//   Return Value:  SystemState_e
//
//   Description:   Receive an amount of data in non-blocking mode with DMA.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::Receive_DMA(uint8_t* pData)
{
    SystemState_e  State         = SYS_READY;
    uint32_t       DataSize      = m_pXSPI->DLR + 1;
	uint32_t AddressRegister     = m_pXSPI->AR;
	uint32_t InstructionRegister = m_pXSPI->IR;

    #ifndef USE_FREERTOS        
    m_IsItDMA_Busy = true;
    #endif  

    MODIFY_REG(m_pXSPI->CR, XSPI_CR_FMODE, XSPI_FUNCTIONAL_MODE_INDIRECT_READ);	                // Configure CR register with functional mode as indirect read
    WRITE_REG(m_pXSPI->FCR, HAL_XSPI_FLAG_TE | HAL_XSPI_FLAG_TC); 				                // Clear flags related to interrupt
    m_DMA_RX.EnableInterrupt(DMA_IT_TC | DMA_IT_DTE | DMA_IT_ULE | DMA_IT_USE | DMA_IT_TO);
    m_DMA_RX.SetDestination(pData);                                                             // Set DMA source
    m_DMA_RX.SetLength(DataSize);                                                               // Set size of the RX
    m_DMA_RX.WriteTransferRegisters();
    m_Status = SYS_BUSY_RX;                                                                     // Update the state
    m_DMA_RX.Enable();                                                                          // Enable the DMA module
    m_DMA_RX.ClearFlag();                                                                       // Clear IRQ DMA flag

    if(READ_BIT(m_pXSPI->CCR, XSPI_CCR_ADMODE) != HAL_XSPI_ADDRESS_NONE)
    {
        WRITE_REG(m_pXSPI->AR, AddressRegister);
    }
    else
    {
        WRITE_REG(m_pXSPI->IR, InstructionRegister);
    }

    SET_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);              // Enable the DMA transfer by setting the DMAEN bit

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      DMA_RX_IRQ_Handler and DMA_TX_IRQ_Handler
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   DMA IRQ handler for XSPI
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::DMA_RX_IRQ_Handler(void)
{
	m_pXSPI->FCR = HAL_XSPI_FLAG_TC;
    m_Status = SYS_READY;
    CLEAR_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);    // Disable the DMA transfer on the XSPI side
    m_DMA_RX.Disable();
    m_DMA_RX.ClearFlag();                               // Clear TC and HT transfer flags
    ReleaseFromISR();
}

void W25Q_Driver::DMA_TX_IRQ_Handler(void)
{
	m_pXSPI->FCR = HAL_XSPI_FLAG_TC;
    m_Status = SYS_READY;
    CLEAR_BIT(m_pXSPI->CR, XSPI_CR_DMAEN);    // Disable the DMA transfer on the XSPI side
    m_DMA_TX.Disable();
    m_DMA_TX.ClearFlag();                               // Clear TC and HT transfer flags
    ReleaseFromISR();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReleaseFromISR
//
//  Parameter(s):   None
//  Return:         void
//
//  Description:    Common code for DMA RX and TX
//
//-------------------------------------------------------------------------------------------------
void W25Q_Driver::ReleaseFromISR(void)
{
  #ifdef USE_FREERTOS
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(m_DMA_Busy, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  #else
    m_IsItDMA_Busy = false;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WaitForDMA
//
//  Parameter(s):   None
//  Return:         SystemState_e               SYS_READY or SYS_TIMEOUT
//
//  Description:    Wait for DMA to release flag or FreeRTOS semaphore.
//
//-------------------------------------------------------------------------------------------------
SystemState_e W25Q_Driver::WaitForDMA(void)
{
    SystemState_e State;
  #ifndef USE_FREERTOS
    uint32_t    TickStart  = GetTick();
    uint32_t    ElapseTick;
  #endif

  #ifdef USE_FREERTOS
    State = (xSemaphoreTake(m_DMA_Busy, XSPI_TIMEOUT_DEFAULT_VALUE) == pdFALSE) ? SYS_TIME_OUT : SYS_READY;
  #else
    // Don't do a busy wait if
    //           1) We're in FreeRTOS context
    //           2) and not in an interrupt
    //           3) and the semaphore raised within 100ms
    do
    {
        State = TickHasTimeOut(TickStart, XSPI_TIMEOUT_DEFAULT_VALUE) ? SYS_TIME_OUT : SYS_READY;
    }
    while((m_IsItDMA_Busy == true) && (State == SYS_READY));

    #endif

    return State;
}

//-------------------------------------------------------------------------------------------------

extern "C"
{
    //---------------------------------------------------------------------------------------------
    //
    //   Function:      GPDMA1_Channel4_IRQHandler
    //
    //   Parameter(s):  None
    //   Return Value:  None
    //
    //   Description:   This function handles GPDMA1 Channel 4 global interrupt for OCTOSPI.
    //
    //---------------------------------------------------------------------------------------------
    void GPDMA1_Channel4_IRQHandler(void)
    {
        W25Q32.DMA_RX_IRQ_Handler();
    }

    //---------------------------------------------------------------------------------------------
    //
    //   Function:      GPDMA1_Channel5_IRQHandler
    //
    //   Parameter(s):  None
    //   Return Value:  None
    //
    //   Description:   This function handles GPDMA1 Channel 5 global interrupt for OCTOSPI
    //
    //---------------------------------------------------------------------------------------------
    void GPDMA1_Channel5_IRQHandler(void)
    {
        W25Q32.DMA_TX_IRQ_Handler();
    }

    //---------------------------------------------------------------------------------------------
}

//-------------------------------------------------------------------------------------------------


#endif




#if 0 


  OSPI_HyperbusCfgTypeDef sHyperbusCfg;
  OSPI_HyperbusCmdTypeDef sCommand;
  OSPI_MemoryMappedTypeDef sMemMappedCfg;

  uint32_t address = 0;
  uint16_t index;
  __IO uint32_t *mem_addr;

  /* Initialize OctoSPI ----------------------------------------------------- */
  OSPIHandle.Instance = OCTOSPI2;
  HAL_OSPI_DeInit(&OSPIHandle);

  OSPIHandle.Init.FifoThreshold         = 4;
  OSPIHandle.Init.DualQuad              = HAL_OSPI_DUALQUAD_DISABLE;
  OSPIHandle.Init.MemoryType            = HAL_OSPI_MEMTYPE_HYPERBUS;
  OSPIHandle.Init.DeviceSize            = OSPI_HYPERRAM_SIZE;
  OSPIHandle.Init.ChipSelectHighTime    = 8;
  OSPIHandle.Init.FreeRunningClock      = HAL_OSPI_FREERUNCLK_DISABLE;
  OSPIHandle.Init.ClockMode             = HAL_OSPI_CLOCK_MODE_0;
  OSPIHandle.Init.WrapSize              = HAL_OSPI_WRAP_NOT_SUPPORTED;
  OSPIHandle.Init.ClockPrescaler        = 4;
  OSPIHandle.Init.SampleShifting        = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  OSPIHandle.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;
  OSPIHandle.Init.DelayBlockBypass      = HAL_OSPI_DELAY_BLOCK_USED;
  OSPIHandle.Init.ChipSelectBoundary    = 23;
  OSPIHandle.Init.Refresh               = 250; /* The chip select should be released every 4�s */

  if (HAL_OSPI_Init(&OSPIHandle) != HAL_OK)
  {
    Error_Handler() ;
  }

  /* Configure the Hyperbus to access memory space -------------------------- */
  sHyperbusCfg.RWRecoveryTime   = OSPI_HYPERRAM_RW_REC_TIME;
  sHyperbusCfg.AccessTime       = OSPI_HYPERRAM_LATENCY;
  sHyperbusCfg.WriteZeroLatency = HAL_OSPI_LATENCY_ON_WRITE;
  sHyperbusCfg.LatencyMode      = HAL_OSPI_FIXED_LATENCY;

  if (HAL_OSPI_HyperbusCfg(&OSPIHandle, &sHyperbusCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  /* Memory-mapped mode configuration --------------------------------------- */
  sCommand.AddressSpace = HAL_OSPI_MEMORY_ADDRESS_SPACE;
  sCommand.AddressSize  = HAL_OSPI_ADDRESS_32_BITS;
  sCommand.DQSMode      = HAL_OSPI_DQS_ENABLE;
  sCommand.Address      = 0;
  sCommand.NbData       = 1;

  if (HAL_OSPI_HyperbusCmd(&OSPIHandle, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }

  sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

  if (HAL_OSPI_MemoryMapped(&OSPIHandle, &sMemMappedCfg) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable and set OctoSPI interrupt to the lowest priority */
  HAL_NVIC_SetPriority(OCTOSPI2_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(OCTOSPI2_IRQn);


/**
  * @brief  Rx Transfer completed callbacks.
  * @param  hospi: OSPI handle
  * @retval None
  */
void HAL_OSPI_RxCpltCallback(OSPI_HandleTypeDef *hospi)
{
  RxCplt++;
}

/**
  * @brief  Command completed callbacks.
  * @param  hospi: OSPI handle
  * @retval None
  */
void HAL_OSPI_CmdCpltCallback(OSPI_HandleTypeDef *hospi)
{
  CmdCplt++;
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hospi: OSPI handle
  * @retval None
  */
 void HAL_OSPI_TxCpltCallback(OSPI_HandleTypeDef *hospi)
{
  TxCplt++;
}

/**
  * @brief  Status Match callbacks
  * @param  hospi: OSPI handle
  * @retval None
  */
void HAL_OSPI_StatusMatchCallback(OSPI_HandleTypeDef *hospi)
{
  StatusMatch++;
}

/**
  * @brief  Transfer Error callback.
  * @param  hospi: OSPI handle
  * @retval None
  */
void HAL_OSPI_ErrorCallback(OSPI_HandleTypeDef *hospi)
{
  Error_Handler();
}


/* S70KL1281 memory */
/* Size of the HyperRAM */
#define OSPI_HYPERRAM_SIZE          24
#define OSPI_HYPERRAM_INCR_SIZE     256

/* Timing of the HyperRAM */
#define OSPI_HYPERRAM_RW_REC_TIME   3
#define OSPI_HYPERRAM_LATENCY       6

/* End address of the OSPI memory */
#define OSPI_HYPERRAM_END_ADDR      (1 << OSPI_HYPERRAM_SIZE)

/* Size of buffers */
#define BUFFERSIZE                  (COUNTOF(aTxBuffer) - 1)
/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)         (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

#endif