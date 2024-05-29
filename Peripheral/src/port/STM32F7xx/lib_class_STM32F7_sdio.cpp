//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_sdio.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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
//
// Notes:
//
//          SD_Detect-Pin
//          SDIO_D0  = SD-Card DAT0
//          SDIO_D1  = SD-Card DAT1
//          SDIO_D2  = SD-Card DAT2
//          SDIO_D3  = SD-Card DAT3/CD
//          SDIO_CK  = SD-Card Clock
//          SDIO_CMD = SD-Card CMD
//
//          DMA : either  DMA2_STREAM3_CHANNEL4
//                   or   DMA2_STREAM6_CHANNEL4
//
//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define SDIO_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  SDIO_DRIVER_GLOBAL
#include "stm32f7xx_ll_sdmmc.h"   // to replace this with my stuff

//-------------------------------------------------------------------------------------------------

#if (USE_SDIO_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SD_DATA_TIMEOUT                 ((uint32_t)0xFFFFFFFF)
#define SD_SOFTWARE_COMMAND_TIMEOUT     ((uint32_t)0x00010000)

// stuff to change
#define SD_CARD_RETRY_TIMER             (1000 / SD_CARD_TICK_PERIOD)
#define SD_CARD_TICK_PERIOD             1


#define ClearAllFlag()                  { SDMMC1->ICR = (SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_CTIMEOUT | \
                                                         SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_RXOVERR  | \
                                                         SDMMC_FLAG_CMDREND  | SDMMC_FLAG_CMDSENT  | SDMMC_FLAG_DATAEND  | \
                                                         SDMMC_FLAG_DBCKEND); }


#define ClearClockRegister()            { SDMMC1->CLKCR = ~((uint32_t)(SDMMC_CLKCR_CLKDIV  | SDMMC_CLKCR_PWRSAV |\
                                                                       SDMMC_CLKCR_BYPASS  | SDMMC_CLKCR_WIDBUS |\
                                                                       SDMMC_CLKCR_NEGEDGE | SDMMC_CLKCR_HWFC_EN)); }

#define IFCR_CLEAR_MASK_STREAM3         (DMA_LIFCR_CTCIF3 | DMA_LIFCR_CHTIF3 | DMA_LIFCR_CTEIF3 | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CFEIF3)
#define IFCR_CLEAR_MASK_STREAM6         (DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6 | DMA_HIFCR_CDMEIF6 | DMA_HIFCR_CFEIF6)

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum SD_CardState_t
{
    SD_CARD_READY                  = ((uint32_t)0x00000001),  // Card state is ready
    SD_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),  // Card is in identification state
    SD_CARD_STANDBY                = ((uint32_t)0x00000003),  // Card is in standby state
    SD_CARD_TRANSFER               = ((uint32_t)0x00000004),  // Card is in transfer state
    SD_CARD_SENDING                = ((uint32_t)0x00000005),  // Card is sending an operation
    SD_CARD_RECEIVING              = ((uint32_t)0x00000006),  // Card is receiving operation information
    SD_CARD_PROGRAMMING            = ((uint32_t)0x00000007),  // Card is in programming state
    SD_CARD_DISCONNECTED           = ((uint32_t)0x00000008),  // Card is disconnected
    SD_CARD_ERROR                  = ((uint32_t)0x000000FF)   // Card is in error state
};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   SDIO_Driver
//
//   Parameter(s):  DMA_Info_t* pDMA_RX
//                  DMA_Info_t* pDMA_TX
//
//   Description:   Initializes the IO and the peripheral
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SDIO_Driver::SDIO_Driver(SDIO_Info_t* pInfo)
{
    m_pInfo = pInfo;
    m_IsItInitialize = false;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//
//   Description:   Initializes the IO and the peripheral
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::Initialize(void)
{
    uint32_t PriorityGroup;

    if(m_IsItInitialize == false)
    {
        nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);

        // Reset SDIO Module
        RCC->APB2RSTR |=  RCC_APB2RSTR_SDMMC1RST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_SDMMC1RST;

        // Enable SDIO clock
        RCC->APB2ENR |= RCC_APB2ENR_SDMMC1EN;

        /// ---- GPIOs Configuration ----
        for(uint32_t IO_Id = uint32_t(IO_SD_D0); IO_Id <= uint32_t(IO_SD_CMD); IO_Id++)
        {
            IO_PinInit(IO_ID_e(IO_Id));
        }

      #if SD_CARD_USE_DETECT_SIGNAL == 1
        IO_PinInit(IO_DETECT_SD_CARD);
      #endif

      #if SD_CARD_USE_POWER_CONTROL == 1
        IO_OutputInit(IO_POWER_SD_CARD);
      #endif

        // Initialize DMA2 channel 3 for RX from SD CARD
        m_DMA_RX.Initialize(&m_pInfo->DMA_RX); // Write config that will never change
        m_DMA_RX.SetSource((void*)&SDMMC1->FIFO);
        m_DMA_RX.ClearFlag(IFCR_CLEAR_MASK_STREAM3);
//??        DMA2_Stream3->FCR  = (DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);                 // Configuration FIFO control register


        // Initialize DMA2 channel 6 for TX to SD CARD
        m_DMA_RX.Initialize(&m_pInfo->DMA_TX); // Write config that will never change
        m_DMA_RX.SetDestination((void*)&SDMMC1->FIFO);
        m_DMA_RX.ClearFlag(IFCR_CLEAR_MASK_STREAM6);
//??        DMA2_Stream6->FCR  = (DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);                 // Configuration FIFO control register

        m_CardStatus       = STA_NOINIT;
        m_TransferError    = SYS_READY;
 //       m_DMA_XferComplete = false;
        m_TransferComplete = false;

        //m_SendStopTransfer = false;
        // m_TickPeriod   = SD_CARD_TICK_PERIOD;

        PriorityGroup = NVIC_GetPriorityGrouping();

        // NVIC configuration for SDIO interrupts
        NVIC_SetPriority(SDMMC1_IRQn, NVIC_EncodePriority(PriorityGroup, 5, 0));
        NVIC_EnableIRQ(SDMMC1_IRQn);

        // NVIC configuration for DMA transfer complete interrupt
        NVIC_SetPriority(DMA2_Stream3_IRQn, NVIC_EncodePriority(PriorityGroup, 6, 0));
        NVIC_EnableIRQ(DMA2_Stream3_IRQn);

        NVIC_SetPriority(DMA2_Stream6_IRQn, NVIC_EncodePriority(PriorityGroup, 6, 0));
        NVIC_EnableIRQ(DMA2_Stream6_IRQn);

            // Initialize SDMMC peripheral interface with default configuration for SD card initialization
        ClearClockRegister();
        SDMMC1->CLKCR |=  (uint32_t)SDMMC_INIT_CLK_DIV;

        m_IsItInitialize = true;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetStatus
//
//   Parameter(s):  uint32_t*       pResponse
//   Return value:  SystemState_e
//
//   Description:   Get response from SD device
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::GetStatus(void)
{
    SystemState_e  State;
    uint32_t       Response1;
    SD_CardState_t CardState;

    if((State = this->IsDetected()) == SYS_READY)
    {
        if((State = this->TransmitCommand((CMD13 | SD_SHORT_RESPONSE), m_RCA, 1)) == SYS_READY)
        {
            Response1 = SDMMC1->RESP1;
            CardState = (SD_CardState_t)((Response1 >> 9) & 0x0F);

            // Find SD status according to card state
            if     (CardState == SD_CARD_TRANSFER)  State = SYS_READY;
            else if(CardState == SD_CARD_ERROR)     State = SYS_DEVICE_ERROR;
            else                                    State = SYS_DEVICE_BUSY;
        }
        else
        {
            State = SYS_DEVICE_ERROR;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetResponse
//
//   Parameter(s):  uint32_t*       pResponse
//   Return value:  SystemState_e
//
//   Description:   Get response from SD device
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::GetResponse(uint32_t* pResponse)
{
    pResponse[0] = SDMMC1->RESP1;
    pResponse[1] = SDMMC1->RESP2;
    pResponse[2] = SDMMC1->RESP3;
    pResponse[3] = SDMMC1->RESP4;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetBusWidth
//
//  Parameter(s):   uint32_t        WideMode
//  Return:         SystemState_e   State
//
//  Description:    Set the bus width of the SDMMC module
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::SetBusWidth(uint32_t WideMode)
{
    SystemState_e State;
    uint32_t      Temp;

    // MMC Card does not support this feature
    if(m_CardType == SD_MULTIMEDIA)
    {
        State = SYS_UNSUPPORTED_FEATURE;
    }
    else if((m_CardType == SD_STD_CAPACITY_V1_1) || (m_CardType == SD_STD_CAPACITY_V2_0) ||\
            (m_CardType == SD_HIGH_CAPACITY))
    {
        if(WideMode == SD_BUS_WIDE_8B)
        {
            State = SYS_UNSUPPORTED_FEATURE;
        }
        else if((WideMode == SD_BUS_WIDE_4B) ||
                (WideMode == SD_BUS_WIDE_1B))
        {
            if((SDMMC1->RESP1 & SD_CARD_LOCKED) != SD_CARD_LOCKED)
            {
                // Get SCR Register
                if((State = this->FindSCR()) == SYS_READY)
                {
                    Temp = (WideMode == SD_BUS_WIDE_4B) ? SD_WIDE_BUS_SUPPORT : SD_SINGLE_BUS_SUPPORT;

                    // If requested card supports wide bus operation
                    if((m_SCR.Array[0] & Temp) != 0)
                    {
                        // Send CMD55 APP_CMD with argument as card's RCA.
                        if((State = this->TransmitCommand((CMD55 | SD_SHORT_RESPONSE), m_RCA, 1)) == SYS_READY)
                        {
                            Temp = (WideMode == SD_BUS_WIDE_4B) ? 2 : 0;

                            // Send ACMD6 APP_CMD with argument as 2 for wide bus mode
                            State =  this->TransmitCommand((CMD6 | SD_SHORT_RESPONSE), Temp, 1);
                        }
                    }
                    else
                    {
                        State = SYS_REQUEST_NOT_APPLICABLE;
                    }
                }
            }
            else
            {
                State = SYS_LOCK_UNLOCK_FAILED;
            }
        }
        else
        {
            State = SYS_INVALID_PARAMETER;  // WideMode is not a valid argument
        }

        if(State == SYS_READY)
        {
            // Configure the SDMMC peripheral
            ClearClockRegister();
            SDMMC1->CLKCR |=  (uint32_t)WideMode;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: TransmitCommand
//
//   Parameter(s):  uint8_t         Command
//                  uint32_t        Argument
//                  int32_t         ResponseType
//   Return value:  SystemState_e
//
//   Description:   Send command to the SD Card
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::TransmitCommand(uint8_t Command, uint32_t Argument, int32_t ResponseType)
{
    SystemState_e   State;

    ClearAllFlag();                                                     // Clear the Command Flags
    SDMMC1->ARG = (uint32_t)Argument;                                   // Set the SDMMC Argument value
    SDMMC1->CMD = (uint32_t)(Command | SDMMC_CMD_CPSMEN);               // Set SDMMC command parameters
    m_LastCommand = Command;

    if((Argument == 0) && (ResponseType == 0))
    {
        ResponseType = -1;       // Go idle command
    }

    State = this->CmdResponse(Command & SDMMC_CMD_CMDINDEX, ResponseType);
    ClearAllFlag();                                                     // Clear the Command Flags

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: CheckOCR_Response
//
//   Parameter(s):  uint32_t        Response_R1      OCR Response code
//   Return value:  SystemState_e
//
//   Description:   Analyze the OCR response and return the appropriate error code
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::CheckOCR_Response(uint32_t Response_R1)
{
    if((Response_R1 & SD_OCR_ERRORBITS)             == 0)                           return SYS_READY;
    if((Response_R1 & SD_OCR_ADDR_OUT_OF_RANGE)     == SD_OCR_ADDR_OUT_OF_RANGE)    return SYS_OUT_OF_RANGE;
    if((Response_R1 & SD_OCR_ADDR_MISALIGNED)       == SD_OCR_ADDR_MISALIGNED)      return SYS_ADDRESS_MISALIGNED;
    if((Response_R1 & SD_OCR_BLOCK_LEN_ERR)         == SD_OCR_BLOCK_LEN_ERR)        return SYS_BLOCK_LENGTH_ERROR;
    if((Response_R1 & SD_OCR_ERASE_SEQ_ERR)         == SD_OCR_ERASE_SEQ_ERR)        return SD_ERASE_SEQ_ERR;
    if((Response_R1 & SD_OCR_BAD_ERASE_PARAM)       == SD_OCR_BAD_ERASE_PARAM)      return SD_BAD_ERASE_PARAM;
    if((Response_R1 & SD_OCR_WRITE_PROT_VIOLATION)  == SD_OCR_WRITE_PROT_VIOLATION) return SD_WRITE_PROT_VIOLATION;
    if((Response_R1 & SD_OCR_LOCK_UNLOCK_FAILED)    == SD_OCR_LOCK_UNLOCK_FAILED)   return SYS_LOCK_UNLOCK_FAILED;
    if((Response_R1 & SD_OCR_COM_CRC_FAILED)        == SD_OCR_COM_CRC_FAILED)       return SYS_CRC_FAIL;
    if((Response_R1 & SD_OCR_ILLEGAL_CMD)           == SD_OCR_ILLEGAL_CMD)          return SYS_INVALID_COMMAND;
    if((Response_R1 & SD_OCR_CARD_ECC_FAILED)       == SD_OCR_CARD_ECC_FAILED)      return SD_CARD_ECC_FAILED;
    if((Response_R1 & SD_OCR_CC_ERROR)              == SD_OCR_CC_ERROR)             return SD_CC_ERROR;
    if((Response_R1 & SD_OCR_GENERAL_UNKNOWN_ERROR) == SD_OCR_GENERAL_UNKNOWN_ERROR)return SYS_GENERAL_UNKNOWN_ERROR;
    if((Response_R1 & SD_OCR_STREAM_READ_UNDERRUN)  == SD_OCR_STREAM_READ_UNDERRUN) return SYS_UNDERRUN;
    if((Response_R1 & SD_OCR_STREAM_WRITE_OVERRUN)  == SD_OCR_STREAM_WRITE_OVERRUN) return SYS_OVERRUN;
    if((Response_R1 & SD_OCR_CID_CSD_OVERWRITE)     == SD_OCR_CID_CSD_OVERWRITE)    return SD_CID_CSD_OVERWRITE;
    if((Response_R1 & SD_OCR_WP_ERASE_SKIP)         == SD_OCR_WP_ERASE_SKIP)        return SD_WP_ERASE_SKIP;
    if((Response_R1 & SD_OCR_CARD_ECC_DISABLED)     == SD_OCR_CARD_ECC_DISABLED)    return SD_CARD_ECC_DISABLED;
    if((Response_R1 & SD_OCR_ERASE_RESET)           == SD_OCR_ERASE_RESET)          return SD_ERASE_RESET;
    if((Response_R1 & SD_OCR_AKE_SEQ_ERROR)         == SD_OCR_AKE_SEQ_ERROR)        return SD_AKE_SEQ_ERROR;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: CmdResponse
//
//   Parameter(s):  uint8_t             Command
//                  uint32_t            Argument
//                  SD_ResponseType_e   ResponseType
//   Return value:  SystemState_e
//
//   Description:   Checks for error conditions for any response.
//                              - R0, R1, R6, R7
//                              - R2 (CID or CSD) response.
//                              - R3 (OCR) response.
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::CmdResponse(uint8_t Command, int32_t ResponseType)
{
    uint32_t      Response_R1;
    uint32_t      TimeOut;
    uint32_t      Flag;

    if(ResponseType == -1) Flag = SDMMC_STA_CMDSENT;
    else                        Flag = SDMMC_STA_CCRCFAIL | SDMMC_STA_CMDREND | SDMMC_STA_CTIMEOUT;

    TimeOut = SD_SOFTWARE_COMMAND_TIMEOUT;
    do
    {
        TimeOut--;
    }
    while(((SDMMC1->STA & Flag) == 0) && (TimeOut > 0));

    if(ResponseType <= 0)
    {
        if(TimeOut == 0)                            return SYS_COMMAND_TIME_OUT;
        else                                        return SYS_READY;
    }

    if((SDMMC1->STA & SDMMC_STA_CTIMEOUT) != 0)     return SYS_COMMAND_TIME_OUT;

    if(ResponseType == 3)
    {
        if(TimeOut == 0)                            return SYS_COMMAND_TIME_OUT;    // Card is not V2.0 compliant or card does not support the set voltage range
        else                                        return SYS_READY;               // Card is SD V2.0 compliant
    }

    if((SDMMC1->STA & SDMMC_STA_CCRCFAIL) != 0)     return SYS_CRC_FAIL;
    if(ResponseType == 2)                           return SYS_READY;
    if((uint8_t)SDMMC1->RESPCMD != Command)         return SYS_INVALID_COMMAND;     // Check if response is of desired command

    Response_R1 = SDMMC1->RESP1;                                                    // We have received response, retrieve it for analysis

    if(ResponseType == 1)
    {
        return this->CheckOCR_Response(Response_R1);
    }
    else if(ResponseType == 6)
    {
        if((Response_R1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)) == 0)
        {
            m_RCA = Response_R1;
        }

        if((Response_R1 & SD_R6_GENERAL_UNKNOWN_ERROR) == SD_R6_GENERAL_UNKNOWN_ERROR)      return SYS_GENERAL_UNKNOWN_ERROR;
        if((Response_R1 & SD_R6_ILLEGAL_CMD)           == SD_R6_ILLEGAL_CMD)                return SYS_INVALID_COMMAND;
        if((Response_R1 & SD_R6_COM_CRC_FAILED)        == SD_R6_COM_CRC_FAILED)             return SYS_CRC_FAIL;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      IsDetected
//
//   Parameter(s):  None
//   Return Value:  SystemState_e       Return
//
//   Description:   Test if card is present
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::IsDetected(void)
{
  #if SD_CARD_USE_DETECT_SIGNAL == 1
    return (IO_Input(DIGINI_SD_CARD_DETECT_IO) == IO_RESET) ? SYS_READY : SYS_DEVICE_NOT_PRESENT;
  #else
    return SYS_READY;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SD_PowerON
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Inquires cards about their operating voltage and configures clock
//                  controls and stores SD information that will be needed in future.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::PowerON(void)
{
    SystemState_e   State;
    uint32_t        Response;
    uint32_t        Count        = 0;
    bool            ValidVoltage = false;
    uint32_t        SD_Type      = SD_RESP_STD_CAPACITY;
    nOS_TickCounter TickStart;

    // Power ON Sequence -------------------------------------------------------
    SDMMC1->CLKCR &= ~SDMMC_CLKCR_CLKEN;        // Disable SDMMC Clock
    SDMMC1->POWER  = SDMMC_POWER_PWRCTRL;       // Set Power State to ON

    // 1ms: required power up waiting time before starting the SD initialization sequence
    TickStart = nOS_GetTickCount();
    while((nOS_GetTickCount() - TickStart) < 1){}

    SDMMC1->CLKCR |= SDMMC_CLKCR_CLKEN;           // Enable SDMMC Clock

    // CMD0: GO_IDLE_STATE -----------------------------------------------------
    // No CMD Response required
    if((State = this->TransmitCommand(CMD0, 0, 0)) != SYS_READY)
    {
        // CMD Response Timeout (wait for CMDSENT flag)
        return State;
    }

    // CMD8: SEND_IF_COND ------------------------------------------------------
    // Send CMD8 to verify SD card interface operating condition
    // Argument: - [31:12]: Reserved (shall be set to '0')
    //- [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
    //- [7:0]: Check Pattern (recommended 0xAA)
    // CMD Response: R7
    if((State = this->TransmitCommand((CMD8 | SD_SHORT_RESPONSE), SD_CHECK_PATTERN, 7)) == SYS_READY)
    {
        // SD Card 2.0
        m_CardType = SD_STD_CAPACITY_V2_0;
        SD_Type    = SD_RESP_HIGH_CAPACITY;
    }

    // Send CMD55
    // If State is Command Timeout, it is a MMC card
    // If State is SYS_READY it is a SD card: SD card 2.0 (voltage range mismatch) or SD card 1.x
    if((State = this->TransmitCommand((CMD55 | SD_SHORT_RESPONSE), 0, 1)) == SYS_READY)
    {
        // SD CARD
        // Send ACMD41 SD_APP_OP_COND with Argument 0x80100000
        while((ValidVoltage == false) && (Count < SD_MAX_VOLT_TRIAL))
        {

            // SEND CMD55 APP_CMD with RCA as 0
            if((State = this->TransmitCommand((CMD55 | SD_SHORT_RESPONSE), 0, 1)) != SYS_READY)
            {
                return State;
            }

            // Send CMD41
            if((State = this->TransmitCommand((CMD41 | SD_SHORT_RESPONSE), SD_VOLTAGE_WINDOW | SD_Type, 3)) != SYS_READY)
            {
                return State;
            }

            Response = SDMMC1->RESP1;                                   // Get command response
            ValidVoltage = (((Response >> 31) == 1) ? true : false);    // Get operating voltage
            Count++;
        }

        if(Count >= SD_MAX_VOLT_TRIAL)
        {
            return SYS_INVALID_VOLTAGE_RANGE;
        }

        if((Response & SD_RESP_HIGH_CAPACITY) == SD_RESP_HIGH_CAPACITY)
        {
            m_CardType = SD_HIGH_CAPACITY;
        }
    } // else MMC Card

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SD_PowerON
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Put interface in power OFF
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::PowerOFF(void)
{
    // Set Power State to OFF
    SDMMC1->POWER = (uint32_t)0;

    // Reset SDIO Module
    RCC->APB2RSTR |=  RCC_APB2RSTR_SDMMC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_SDMMC1RST;
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      SD_InitializeCard
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Initializes all cards or single card as the case may be Card(s) come into standby state.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::InitializeCard(void)
{
    SystemState_e State;

    if((SDMMC1->POWER & SDMMC_POWER_PWRCTRL) != 0) // Power off
    {
        if(m_CardType != SD_SECURE_DIGITAL_IO)
        {
            // Send CMD2 ALL_SEND_CID
            if((State = this->TransmitCommand((CMD2 | SD_LONG_RESPONSE), 0, 2)) != SYS_READY)
            {
                return State;
            }

            // Get Card identification number data
            this->GetResponse(m_CardCID);
        }

        if((m_CardType == SD_STD_CAPACITY_V1_1)       || (m_CardType == SD_STD_CAPACITY_V2_0) ||
           (m_CardType == SD_SECURE_DIGITAL_IO_COMBO) || (m_CardType == SD_HIGH_CAPACITY))
        {
            // Send CMD3 SET_REL_ADDR with argument 0
            // SD Card publishes its RCA.
            if((State = this->TransmitCommand((CMD3 | SD_SHORT_RESPONSE), 0, 6)) != SYS_READY)
            {
                return State;
            }
        }

        if(m_CardType != SD_SECURE_DIGITAL_IO)
        {
            // Send CMD9 SEND_CSD with argument as card's RCA
            if((State = this->TransmitCommand((CMD9 | SD_LONG_RESPONSE), m_RCA, 2)) == SYS_READY)
            {
                // Get Card Specific Data
                this->GetResponse(m_CardCSD);
            }
        }
    }
    else
    {
        State = SYS_REQUEST_NOT_APPLICABLE;
    }

    // Card are initialized
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: DataInit
//
//   Parameter(s):  uint32_t       Size
//                  uint32_t       DataBlockSize
//                  bool           IsItReadFromCard
//   Return value:  None
//
//   Description:   Configuration for SDIO Data using SDIO_DataInitTypeDef
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::DataInit(uint32_t Size, uint32_t DataBlockSize, bool IsItReadFromCard)
{
    uint32_t Direction;

    SDMMC1->DTIMER = SD_DATA_TIMEOUT;       // Set the SDMMC Data TimeOut value
    SDMMC1->DLEN   = Size;                  // Set the SDMMC DataLength value
    Direction      = (IsItReadFromCard == true) ? SDMMC_DCTRL_DTDIR : 0;
    SDMMC1->DCTRL |=  (uint32_t)(DataBlockSize | Direction | SDMMC_DCTRL_DTEN);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      FindSCR
//
//   Parameter(s):  pSCR: pointer to the buffer that will contain the SCR value
//   Return Value:
//
//   Description:   Finds the SD card SCR register value.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::FindSCR(void)
{
    SystemState_e State;
    int           Index;

    // Set Block Size To 8 Bytes
    // Send CMD16 Set Block length
    if((State = this->TransmitCommand((CMD16 | SD_SHORT_RESPONSE), 8, 1)) == SYS_READY)
    {
        // Send CMD55 APP_CMD with argument as card's RCA
        if((State = this->TransmitCommand((CMD55 | SD_SHORT_RESPONSE), m_RCA, 1)) == SYS_READY)
        {
            this->DataInit(8, SD_DATABLOCK_SIZE_8B, true);

            // Send ACMD51 SD_APP_SEND_SCR with argument as 0
            if((State = this->TransmitCommand((CMD51 | SD_SHORT_RESPONSE), 0, 1)) == SYS_READY)
            {
                Index = 0;
                while(((SDMMC1->STA & (SDMMC_STA_RXOVERR | SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT | SDMMC_STA_DBCKEND)) == 0) && (Index < 2))
                {
                    if((SDMMC1->STA & SDMMC_STA_RXDAVL) != 0)
                    {
                        m_SCR.Array[Index] = SDMMC1->FIFO;
                        Index++;
                    }
                }

                if((SDMMC1->STA & SDMMC_STA_DTIMEOUT) != 0)
                {
                    State = SYS_DATA_TIME_OUT;
                }
                else if((SDMMC1->STA & SDMMC_STA_DCRCFAIL) != 0)
                {
                    State = SYS_CRC_FAIL;
                }
                else if((SDMMC1->STA & SDMMC_STA_RXOVERR)  != 0)
                {
                    State = SYS_OVERRUN;
                }
                else if((SDMMC1->STA & SDMMC_STA_RXDAVL)   != 0)
                {
                    State = SYS_OUT_OF_BOUND;
                }
            }
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetStatus
//
//   Parameter(s):  None
//   Return value:  SystemState_e
//
//   Description:   Returns information about specific card.
//                  Contains all SD card information
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::GetCardInfo(void)
{
    uint8_t       Temp;
    SystemState_e State;

    State = SYS_READY;

  #if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)
    // Byte 0
    Temp = uint8_t(m_CardCSD[0] >> 24);
    m_CSD.CSDStruct      = Temp >> 6;
    m_CSD.SysSpecVersion = (Temp & 0x3C) >> 2;                                      // Only for MMC

    // Byte 1
    //m_CSD.TAAC = uint8_t(m_CardCSD[0] >> 16);                                     // Information not to be used as per standard

    // Byte 2
    //m_CSD.NSAC = uint8_t(m_CardCSD[0] >> 8);                                      // Information not to be used as per standard

    // Byte 3
    m_CSD.MaxBusClkFrec = uint8_t(m_CardCSD[0]);

    // Byte 4
    //m_CSD.CardComdClasses = uint16_t(uint8_t(m_CardCSD[1] >> 24)) << 4;           // Not useful information

    // Byte 5
    Temp = uint8_t(m_CardCSD[1] >> 16);
    //m_CSD.CardComdClasses |= uint16_t(Temp >> 4);         // not useful information
    m_CSD.RdBlockLen       = Temp & 0x0F;
  #endif

    // Byte 6
    Temp = uint8_t(m_CardCSD[1] >> 8);
  #if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)
    m_CSD.PartBlockRead   = Temp >> 7;
    m_CSD.WrBlockMisalign = (Temp & 0x40) >> 6;
    m_CSD.RdBlockMisalign = (Temp & 0x20) >> 5;
    m_CSD.DSRImpl         = (Temp & 0x10) >> 4;
  #endif

    if((m_CardType == SD_STD_CAPACITY_V1_1) || (m_CardType == SD_STD_CAPACITY_V2_0))
    {
        m_CSD.DeviceSize = uint32_t(uint8_t(Temp & 0x03)) << 10;

        // Byte 7
        m_CSD.DeviceSize |= uint32_t(uint8_t(m_CardCSD[1])) << 2;

        // Byte 8
        Temp = uint8_t(m_CardCSD[2] >> 24);
        m_CSD.DeviceSize |= uint32_t(Temp >> 6);

      #if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)
        //m_CSD.MaxRdCurrentVDDMin = (Temp & 0x38) >> 3;        // Not useful information
        //m_CSD.MaxRdCurrentVDDMax = Temp & 0x07;               // Not useful information
      #endif
        // Byte 9
        Temp = uint8_t(m_CardCSD[2] >> 16);
      #if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)
        //m_CSD.MaxWrCurrentVDDMin = Temp >> 5;                 // Not useful information
        //m_CSD.MaxWrCurrentVDDMax = (Temp & 0x1C) >> 2;        // Not useful information
      #endif
        m_CSD.DeviceSizeMul = (Temp & 0x03) << 1;

        // Byte 10
        m_CSD.DeviceSizeMul |= uint8_t(m_CardCSD[2] >> 8) >> 7;

        m_CardCapacity  = m_CSD.DeviceSize + 1;
        m_CardCapacity *= (1 << (m_CSD.DeviceSizeMul + 2));
        m_CardBlockSize = 1 << (m_CSD.RdBlockLen);
    }
    else if(m_CardType == SD_HIGH_CAPACITY)
    {
        // Byte 7
        m_CSD.DeviceSize = (uint32_t(uint8_t(m_CardCSD[1]) & 0x3F) << 16);

        // Byte 8
        m_CSD.DeviceSize |= (uint32_t(uint8_t(m_CardCSD[2] >> 24)) << 8);

        // Byte 9
        m_CSD.DeviceSize |= uint32_t(uint8_t(m_CardCSD[2] >> 16));

        m_CardCapacity  = (m_CSD.DeviceSize + 1) * BLOCK_SIZE;
        m_CardBlockSize = BLOCK_SIZE;
    }
    else
    {
        // Not supported card type
        State = SYS_ERROR;
    }

  #if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)

    // Byte 10
    Temp = uint8_t(m_CardCSD[2] >> 8);
    m_CSD.EraseGrSize = (Temp & 0x40) >> 6;
    m_CSD.EraseGrMul  = (Temp & 0x3F) << 1;

    // Byte 11
    Temp = uint8_t(m_CardCSD[2]);
    m_CSD.EraseGrMul     |= Temp >> 7;
    m_CSD.WrProtectGrSize = (Temp & 0x7F);

    // Byte 12
    Temp = uint8_t(m_CardCSD[3] >> 24);
    m_CSD.WrProtectGrEnable =  Temp >> 7;
    m_CSD.ManDeflECC        = (Temp & 0x60) >> 5;
    //m_CSD.WrSpeedFact       = (Temp & 0x1C) >> 2;         // Information not to be used as per standard
    m_CSD.MaxWrBlockLen     = (Temp & 0x03) << 2;

    // Byte 13
    Temp = uint8_t(m_CardCSD[3] >> 16);
    m_CSD.MaxWrBlockLen      |= (Temp >> 6);
    m_CSD.WriteBlockPaPartial = (Temp & 0x20) >> 5;
    m_CSD.ContentProtectAppli = (Temp & 0x01);

    // Byte 14
    Temp = uint8_t(m_CardCSD[3] >> 8);
    m_CSD.FileFormatGroup = Temp >> 7;
    m_CSD.CopyFlag        = (Temp & 0x40) >> 6;
    m_CSD.PermWrProtect   = (Temp & 0x20) >> 5;
    m_CSD.TempWrProtect   = (Temp & 0x10) >> 4;
    m_CSD.FileFormat      = (Temp & 0x0C) >> 2;
    m_CSD.ECC             = (Temp & 0x03);

    // Byte 15
    m_CSD._CRC = uint8_t(m_CardCSD[3]) >> 1;

    // CID Section

    m_CID.ManufacturerID      =          uint8_t(m_CardCID[0]   >> 24);                 // Byte 0
    m_CID.OEM_AppliID         = uint16_t(uint8_t(m_CardCID[0]   >> 16))        << 8;    // Byte 1
    m_CID.OEM_AppliID        |= uint16_t(uint8_t(m_CardCID[0]   >> 8));                 // Byte 2

    m_CID.ProductName[0]      = char(m_CardCID[0]);                                     // Byte 3
    m_CID.ProductName[1]      = char(m_CardCID[1] >> 24);                               // Byte 4
    m_CID.ProductName[2]      = char(m_CardCID[1] >> 16);                               // Byte 4
    m_CID.ProductName[3]      = char(m_CardCID[1] >> 8);                                // Byte 4
    m_CID.ProductName[4]      = char(m_CardCID[1]);                                     // Byte 4
    m_CID.ProductName[5]      = '\0';

    m_CID.ProductRev          =          uint8_t(m_CardCID[2]   >> 24);                  // Byte 8
    m_CID.ProductSN           = uint32_t(uint8_t(m_CardCID[2]   >> 16))        << 24;    // Byte 9
    m_CID.ProductSN          |= uint32_t(uint8_t(m_CardCID[2]   >> 8))         << 16;    // Byte 10
    m_CID.ProductSN          |= uint32_t(uint8_t(m_CardCID[2]))                << 8;     // Byte 11
    m_CID.ProductSN          |= uint32_t(uint8_t(m_CardCID[3]   >> 24));                 // Byte 12
    m_CID.ManufacturingDate   = uint16_t(uint8_t(m_CardCID[3]   >> 16) & 0x0F) << 8;     // Byte 13
    m_CID.ManufacturingDate  |= uint16_t(uint8_t(m_CardCID[3]   >> 8));                  // Byte 14
    m_CID._CRC                =          uint8_t(m_CardCID[3])  >> 1;                    // Byte 15
  #endif

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
//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Lock the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::Lock(void)
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::Unlock(void)
{
    nOS_MutexUnlock(&m_Mutex);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: StartBlockTransfert
//
//   Parameter(s):  DMA_Stream_TypeDef* pDMA            DMA Stream to use for the DMA operation
//                  uint64_t            pBuffer         Pointer to/for the buffer the data
//                  uint32_t            BlockSize       The SD card Data block size
//                  uint32_t            NumberOfBlocks  Number of blocks to access
//   Return value:  SystemState_e
//
//   Description:   Prepare the DMA transfer
//
//   note(s):       This should be preceding the functions WriteBlocks() or ReadBlocks()
//                  completion of the write process (by SD current status polling).
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::StartBlockTransfert(DMA_Stream_TypeDef* pDMA, uint32_t* pBuffer, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
    SDMMC1->DCTRL      = 0;                                                                 // Initialize data control register
    m_TransferComplete = false;                                                                 // Initialize handle flags
//    m_DMA_XferComplete = false;
    m_TransferError    = SYS_READY;
    m_Operation        = (NumberOfBlocks > 1) ? SD_MULTIPLE_BLOCK : SD_SINGLE_BLOCK;        // Initialize SD Read operation
    SDMMC1->MASK      |= (SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE |                   // Enable transfer interrupts
                          SDMMC_MASK_DATAENDIE  | SDMMC_MASK_RXOVERRIE);
    SDMMC1->DCTRL     |= SDMMC_DCTRL_DMAEN;                                                 // Enable SDMMC DMA transfer
    pDMA->CR          &= ~DMA_SxCR_EN;                                                      // Disable the Peripheral
    pDMA->NDTR         = (uint32_t)(BlockSize * NumberOfBlocks) / 4;                        // Configure DMA Stream data length
    pDMA->M0AR         = (uint32_t)pBuffer;                                                 // Configure DMA Stream memory address
    pDMA->CR          |= DMA_SxCR_TCIE | DMA_SxCR_HTIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE;    // Enable all interrupts
    pDMA->FCR         |= DMA_SxFCR_FEIE;
    pDMA->CR          |= DMA_SxCR_EN;                                                       // Enable the Peripheral
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: ReadBlocks
//
//   Parameter(s):  uint64_t ReadAddress
//                  uint32_t BlockSize
//                  uint32_t NumberOfBlocks
//   Return value:  SystemState_e
//
//   Description:   Reads block(s) from a specified address in a card. The Data transfer is managed
//                  by DMA mode.
//
//   note(s):       This API should be followed by the function CheckOperation() to check the
//                  completion of the write process (by SD current status polling).
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::ReadBlocks(uint64_t ReadAddress, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
    SystemState_e State;
    uint32_t      CmdIndex;

    if(m_CardType == SD_HIGH_CAPACITY)
    {
        BlockSize    = 512;
        ReadAddress /= 512;
    }

    // Set Block Size for Card
    if((State = this->TransmitCommand((CMD16 | SD_SHORT_RESPONSE), BlockSize, 1)) == SYS_READY)
    {
        // Configure the SD DPSM (Data Path State Machine)
        this->DataInit(BlockSize * NumberOfBlocks, SD_DATABLOCK_SIZE_512B, true);

    }

    // Send CMD18 READ_MULT_BLOCK with argument data address
    // or send CMD17 READ_SINGLE_BLOCK depending on number of block
    CmdIndex = (NumberOfBlocks > 1) ? CMD18 : CMD17;
    State    = this->TransmitCommand((CmdIndex | SD_SHORT_RESPONSE), (uint32_t)ReadAddress, 1);

    // Update the SD transfer error in SD handle
    m_TransferError = State;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteBlocks
//
//   Parameter(s):  uint64_t WriteAddress
//                  uint32_t BlockSize
//                  uint32_t NumberOfBlocks
//   Return value:  SystemState_e
//
//   Description:   Writes block(s) to a specified address in a card. The Data transfer is managed
//                  by DMA mode.
//
//   note(s):       This API should be followed by the function CheckOperation() to check the
//                  completion of the write process (by SD current status polling).
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::WriteBlocks(uint64_t WriteAddress, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
    SystemState_e State;
    uint32_t      CmdIndex;

    if(m_CardType == SD_HIGH_CAPACITY)
    {
        BlockSize    = 512;
        WriteAddress /= 512;
    }

    // Check number of blocks command
    // Send CMD24 WRITE_SINGLE_BLOCK
    // Send CMD25 WRITE_MULT_BLOCK with argument data address
    CmdIndex = (NumberOfBlocks > 1) ? CMD25 : CMD24;

    // Set Block Size for Card
    if((State = this->TransmitCommand((CmdIndex | SD_SHORT_RESPONSE), (uint32_t)WriteAddress, 1)) != SYS_READY)
    {
        return State;
    }

    // Configure the SD DPSM (Data Path State Machine)
    this->DataInit(BlockSize * NumberOfBlocks, SD_DATABLOCK_SIZE_512B, false);

    m_TransferError = State;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: CheckOperation
//
//   Parameter(s):  uint32_t       Flag
//   Return value:  SystemState_e
//
//   Description:   This function waits until the SD DMA data wirte or read transfer is finished.
//                  This should be called after WriteBlocks_DMA or SD_ReadBlocks_DMA() function
//                  to insure that all data sent is already transferred by the DMA controller.
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::CheckOperation(uint32_t Flag)
{
    SystemState_e  State;
    uint32_t       TimeOut;

    // Wait for DMA/SD transfer end or SD error variables
    TimeOut = SD_DATA_TIMEOUT;
    while(/*(m_DMA_XferComplete == false)     &&*/
          (m_TransferComplete == false)     &&
          (m_TransferError    == SYS_READY) &&
          (TimeOut > 0))
    {
        TimeOut--;
    }

    // Wait until the Rx transfer is no longer active
    TimeOut = SD_DATA_TIMEOUT;
    while(((SDMMC1->STA & Flag) != 0) && (TimeOut > 0))
    {
        TimeOut--;
    }

    // Send stop command in multi block read
    if(m_Operation == SD_MULTIPLE_BLOCK)
    {
        State = this->TransmitCommand((CMD12 | SD_SHORT_RESPONSE), 0, 1);
    }

    if((TimeOut == 0) && (State == SYS_READY))
    {
        State = SYS_DATA_TIME_OUT;
    }

    // Return error state
    if(m_TransferError != SYS_READY)
    {
        return m_TransferError;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SelectTheCard
//
//   Parameter(s):  None
//   Return value:  SystemState_e
//
//   Description:   Select the card
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::SelectTheCard(void)
{
    SystemState_e State;

    State = TransmitCommand((CMD7 | SD_SHORT_RESPONSE), m_RCA, 1);      // Select the Card
    ClearClockRegister();

    return State;
}


/*
//-------------------------------------------------------------------------------------------------
//
//   Function name: WaitReady
//
//   Parameter(s):  uint32_t        Timer            Timeout in unit of 1 msec
//   Return value:  SystemState_e
//
//   Description:   Wait card ready
//
//-------------------------------------------------------------------------------------------------
SystemState_e SDIO_Driver::WaitReady(uint32_t Timer)
{
    uint32_t Response;

    m_TimeOut = Timer;

    do
    {
        this->TransmitCommand(CMD13, m_RCA, 1);
        SDMMC1->RESP1);

        if((Response & 0x00001E00) == 0x00000800)
        {
            return SYS_READY;
        }

        if(this->IsDetected() == SYS_DEVICE_NOT_PRESENT)                   // if card is Remove while waiting
        {
            return SYS_DEVICE_NOT_PRESENT;
        }
    }
    while(m_TimeOut != 0);

    return SYS_TIME_OUT;
}
*/

//-------------------------------------------------------------------------------------------------
//
//   Function name: SD DMA transfer complete RX and TX callback.
//
//   Parameter(s):  pDMA_Stream
//   Return value:  None
//
//   Description:   DMA IRQ
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::DMA_Complete(DMA_Stream_TypeDef* pDMA_Stream)
{
    //m_DMA_XferComplete = true;              // DMA transfer is complete          m_TransferComplete is probably enough
    while(m_TransferComplete == false){}    // Wait until SD transfer is complete
    //Disable
    pDMA_Stream->CR &= ~DMA_SxCR_EN;        // Disable the stream
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SDIO_IRQHandler
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   SDIO IRQ
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::SDMMC1_IRQHandler(void)
{
    // Check for SDMMC interrupt flags
    if((SDMMC1->STA & SDMMC_STA_DATAEND) != 0)
    {
        m_TransferError    = SYS_READY;
        m_TransferComplete = true;
    }
    else if((SDMMC1->STA & SDMMC_STA_DCRCFAIL) != 0)    m_TransferError = SYS_CRC_FAIL;
    else if((SDMMC1->STA & SDMMC_STA_DTIMEOUT) != 0)    m_TransferError = SYS_DATA_TIME_OUT;
    else if((SDMMC1->STA & SDMMC_STA_RXOVERR) != 0)     m_TransferError = SYS_OVERRUN;
    else if((SDMMC1->STA & SDMMC_STA_TXUNDERR) != 0)    m_TransferError = SYS_UNDERRUN;

    ClearAllFlag();

    // Disable all SDMMC peripheral interrupt sources
    SDMMC1->MASK &= ~(SDMMC_MASK_DCRCFAILIE | SDMMC_MASK_DTIMEOUTIE | SDMMC_MASK_DATAENDIE  |
                      SDMMC_MASK_TXFIFOHEIE | SDMMC_MASK_RXFIFOHFIE | SDMMC_MASK_TXUNDERRIE |
                      SDMMC_MASK_RXOVERRIE);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: RX_IRQHandler
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   DMA IRQ
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::RX_IRQHandler(void)
{
    // Transfer Error Interrupt management
    if((DMA2->LISR & DMA_LISR_TEIF3) != 0)
    {
        if((DMA2_Stream3->CR & DMA_SxCR_TEIE) != 0)
        {
            DMA2_Stream3->CR   &= ~DMA_SxCR_TEIE;       // Disable the transfer error interrupt
            DMA2->LIFCR = DMA_LIFCR_CTEIF3;             // Clear the transfer error flag
        }
    }

    // FIFO Error Interrupt management
    if((DMA2->LISR & DMA_LISR_FEIF3) != 0)
    {
        if((DMA2_Stream3->FCR & DMA_SxFCR_FEIE) != 0)
        {
            DMA2_Stream3->FCR   &= ~DMA_SxFCR_FEIE;     // Disable the FIFO Error interrupt
            DMA2->LIFCR = DMA_LIFCR_CFEIF3;             // Clear the FIFO error flag
        }
    }

    // Direct Mode Error Interrupt management
    if((DMA2->LISR & DMA_LISR_DMEIF3) != 0)
    {
        if((DMA2_Stream3->CR & DMA_SxCR_DMEIE) != 0)
        {
            DMA2_Stream3->CR   &= ~DMA_SxCR_DMEIE;       // Disable the direct mode Error interrupt
            DMA2->LIFCR = DMA_LIFCR_CDMEIF3;             // Clear the FIFO error flag
        }
    }

    // Half Transfer Complete Interrupt management
    if((DMA2->LISR & DMA_LISR_HTIF3) != 0)
    {
        if((DMA2_Stream3->CR & DMA_SxCR_HTIE) != 0)
        {
            if(((DMA2_Stream3->CR) & (uint32_t)(DMA_SxCR_DBM)) != 0)    // Multi_Buffering mode enabled
            {
                DMA2->LIFCR = DMA_LIFCR_CHTIF3;                         // Clear the half transfer complete flag
            }
            else
            {
                if((DMA2_Stream3->CR & DMA_SxCR_CIRC) == 0)             // Disable the half transfer interrupt if the DMA mode is not CIRCULAR
                {
                    DMA2_Stream3->CR   &= ~DMA_SxCR_HTIE;               // Disable the half transfer interrupt
                }

                DMA2->LIFCR = DMA_LIFCR_CHTIF3;                         // Clear the half transfer complete flag
            }
        }
    }

    // Transfer Complete Interrupt management
    if((DMA2->LISR & DMA_LISR_TCIF3) != 0)
    {
        if((DMA2_Stream3->CR & DMA_SxCR_TCIE) != 0)
        {
            if((DMA2_Stream3->CR & (uint32_t)(DMA_SxCR_DBM)) != 0)
            {
                DMA2->LIFCR = DMA_LIFCR_CTCIF3;                         // Clear the transfer complete flag
            }
            else //Disable the transfer complete interrupt if the DMA mode is not CIRCULAR
            {
                if((DMA2_Stream3->CR & DMA_SxCR_CIRC) == 0)
                {
                    DMA2_Stream3->CR &= ~DMA_SxCR_TCIE;                 // Disable the transfer complete interrupt
                }

                DMA2->LIFCR = DMA_LIFCR_CTCIF3;                         // Clear the transfer complete flag
                DMA_Complete(DMA2_Stream3);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: TX_IRQHandler
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   DMA IRQ
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void SDIO_Driver::TX_IRQHandler(void)
{
    // Transfer Error Interrupt management
    if((DMA2->HISR & DMA_HISR_TEIF6) != 0)
    {
        if((DMA2_Stream6->CR & DMA_SxCR_TEIE) != 0)
        {
            DMA2_Stream6->CR   &= ~DMA_SxCR_TEIE;       // Disable the transfer error interrupt
            DMA2->HIFCR = DMA_HIFCR_CTEIF6;             // Clear the transfer error flag
        }
    }

    // FIFO Error Interrupt management
    if((DMA2->HISR & DMA_HISR_FEIF6) != 0)
    {
        if((DMA2_Stream6->FCR & DMA_SxFCR_FEIE) != 0)
        {
            DMA2_Stream6->FCR   &= ~DMA_SxFCR_FEIE;     // Disable the FIFO Error interrupt
            DMA2->HIFCR = DMA_HIFCR_CFEIF6;             // Clear the FIFO error flag
        }
    }

    // Direct Mode Error Interrupt management
    if((DMA2->HISR & DMA_HISR_DMEIF6) != 0)
    {
        if((DMA2_Stream6->CR & DMA_SxCR_DMEIE) != 0)
        {
            DMA2_Stream6->CR   &= ~DMA_SxCR_DMEIE;       // Disable the direct mode Error interrupt
            DMA2->HIFCR = DMA_HIFCR_CDMEIF6;             // Clear the FIFO error flag
        }
    }

    // Half Transfer Complete Interrupt management
    if((DMA2->HISR & DMA_HISR_HTIF6) != 0)
    {
        if((DMA2_Stream6->CR & DMA_SxCR_HTIE) != 0)
        {
            if(((DMA2_Stream6->CR) & (uint32_t)(DMA_SxCR_DBM)) != 0)    // Multi_Buffering mode enabled
            {
                DMA2->HIFCR = DMA_HIFCR_CHTIF6;                         // Clear the half transfer complete flag
            }
            else
            {
                if((DMA2_Stream6->CR & DMA_SxCR_CIRC) == 0)             // Disable the half transfer interrupt if the DMA mode is not CIRCULAR
                {
                    DMA2_Stream6->CR &= ~DMA_SxCR_HTIE;                 // Disable the half transfer interrupt
                }

                DMA2->HIFCR = DMA_HIFCR_CHTIF6;                         // Clear the half transfer complete flag
            }
        }
    }

    // Transfer Complete Interrupt management
    if((DMA2->HISR & DMA_HISR_TCIF6) != 0)
    {
        if((DMA2_Stream6->CR & DMA_SxCR_TCIE) != 0)
        {
            if((DMA2_Stream6->CR & (uint32_t)(DMA_SxCR_DBM)) != 0)
            {
                DMA2->HIFCR = DMA_HIFCR_CTCIF6;                         // Clear the transfer complete flag
            }
            else //Disable the transfer complete interrupt if the DMA mode is not CIRCULAR
            {
                if((DMA2_Stream6->CR & DMA_SxCR_CIRC) == 0)
                {
                    DMA2_Stream6->CR &= ~DMA_SxCR_TCIE;               // Disable the transfer complete interrupt
                }

                DMA2->HIFCR = DMA_HIFCR_CTCIF6;                         // Clear the transfer complete flag
                DMA_Complete(DMA2_Stream6);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: CardDetectIRQ_Handler
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   IRQ Handler for the card detect
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (SD_CARD_USE_DETECT_SIGNAL == 1)
void SDIO_Driver::CardDetectIRQ_Handler(void)
{
    m_Detect = SYS_DEVICE_NOT_PRESENT;

    if(IO_Input(DETECT_SD_CARD) == IO_RESET)
    {
        m_Detect = SYS_READY;
    }
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (USE_SDIO_DRIVER == DEF_ENABLED)
