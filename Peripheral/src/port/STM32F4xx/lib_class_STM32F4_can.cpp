//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_can.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2022 Alain Royer.
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

#define CAN_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  CAN_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_CAN_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MCR_DBF                 ((uint32_t)0x00010000)      // CAN Master Control Register bits
#define TMIDxR_TXRQ             ((uint32_t)0x00000001)      // CAN Mailbox Transmit Request
#define FMR_FINIT               ((uint32_t)0x00000001)      // CAN Filter Master Register bits
#define INAK_TIMEOUT            ((uint32_t)0x0000FFFF)      // Time out for INAK bit
#define SLAK_TIMEOUT            ((uint32_t)0x0000FFFF)      // Time out for SLAK bit
#define CAN_FLAGS_TSR           ((uint32_t)0x08000000)      // Flags in TSR register
#define CAN_FLAGS_RF1R          ((uint32_t)0x04000000)      // Flags in RF1R register
#define CAN_FLAGS_RF0R          ((uint32_t)0x02000000)      // Flags in RF0R register
#define CAN_FLAGS_MSR           ((uint32_t)0x01000000)      // Flags in MSR register
#define CAN_FLAGS_ESR           ((uint32_t)0x00F00000)      // Flags in ESR register
#define CAN_TXMAILBOX_0         ((uint8_t)0x00)             // Mailboxes definition
#define CAN_TXMAILBOX_1         ((uint8_t)0x01)
#define CAN_TXMAILBOX_2         ((uint8_t)0x02) 
#define CAN_MODE_MASK           ((uint32_t) 0x00000003)

//-------------------------------------------------------------------------------------------------
//
//   Class: CAN_Driver
//
//   Description:   Class to handle I2C_Driver
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   CAN_Driver
//
//   Parameter(s):  CAN_ID_e
//
//   Description:   Initializes the CAN_ID peripheral according to the specified parameters
//
//-------------------------------------------------------------------------------------------------
CAN_Driver::CAN_Driver(CAN_ID_e CAN_ID)
{
    //m_IsItInitialize = false;
    //m_pInfo          = &CAN_Info[CAN_ID];
    //m_Device         = -1;
    //m_State          = SYS_DEVICE_NOT_PRESENT;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   None
//  Return:         none
//
//  Description:    Initializes the CAN peripheral according to the specified parameters
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
/**
  * @param  CANx: where x can be 1 or 2 to select the CAN peripheral.
  * @param  CAN_InitStruct: pointer to a CAN_InitTypeDef structure that contains
  *         the configuration information for the CAN peripheral.
  * @retval Constant indicates initialization succeed which will be 
  *         CAN_InitStatus_Failed or CAN_InitStatus_Success.
  */
//uint8_t CAN_Init(CAN_InitTypeDef* CAN_InitStruct)
SystemState_e CAN_Driver::Initialize(void)
{
    CAN_TypeDef*    pCANx;
    SystemState_e   State   = SYS_FAIL;
    uint32_t        WaitAck = 0;
    ISR_Prio_t      ISR_Prio;
    uint32_t        PriorityGroup;
    

    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;
        Error = nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
        VAR_UNUSED(Error);
    }

    pCANx     = m_pInfo->pCANx;
    //m_Timeout = 0;

//    NVIC_DisableIRQ(m_pInfo->EV_IRQn);
//    NVIC_DisableIRQ(m_pInfo->ER_IRQn);

    // ---- Module configuration ----
//    RCC->APB1RSTR |=  m_pInfo->RCC_APB1_En;             // Reset I2C
//    RCC->APB1RSTR &= ~m_pInfo->RCC_APB1_En;             // Release reset signal of I2C
//    RCC->APB1ENR  |=  m_pInfo->RCC_APB1_En;

    // ---- GPIO configuration ----
    IO_PinInit(m_pInfo->CANL);
    IO_PinInit(m_pInfo->CANH);

    pCANx->MCR &= (~(uint32_t)CAN_MCR_SLEEP);       // Exit from sleep mode
    pCANx->MCR |= CAN_MCR_INRQ;                     // Request initialisation

    while(((pCANx->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) && (WaitAck != INAK_TIMEOUT))  // Wait the acknowledge
    {
        WaitAck++;
    }

    if((pCANx->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) // Check acknowledge
    {
        if(m_pInfo->CAN_TTCM == ENABLE)             // Set the time triggered communication mode
        {
            pCANx->MCR |= CAN_MCR_TTCM;
        }
        else
        {
            pCANx->MCR &= ~(uint32_t)CAN_MCR_TTCM;
        }

        if(m_pInfo->CAN_ABOM == ENABLE)             // Set the automatic bus-off management
        {
            pCANx->MCR |= CAN_MCR_ABOM;
        }
        else
        {
            pCANx->MCR &= ~(uint32_t)CAN_MCR_ABOM;
        }

        if(m_pInfo->CAN_AWUM == ENABLE)             // Set the automatic bus-off management
        {
            pCANx->MCR |= CAN_MCR_AWUM;
        }
        else
        {
            pCANx->MCR &= ~(uint32_t)CAN_MCR_AWUM;
        }

        if(m_pInfo->CAN_NART == ENABLE)             // Set the no automatic retransmission
        {
            pCANx->MCR |= CAN_MCR_NART;
        }
        else
        {
            pCANx->MCR &= ~(uint32_t)CAN_MCR_NART;
        }

        if(m_pInfo->CAN_RFLM == ENABLE)
        {
            pCANx->MCR |= CAN_MCR_RFLM;             // Set the receive FIFO locked mode
        }
        else
        {
            pCANx->MCR &= ~(uint32_t)CAN_MCR_RFLM;
        }

        if(m_pInfo->CAN_TXFP == ENABLE)
        {
            pCANx->MCR |= CAN_MCR_TXFP;             // Set the transmit FIFO priority
        }
        else
        {
            pCANx->MCR &= ~(uint32_t)CAN_MCR_TXFP;
        }

        // Set the bit timing register
        pCANx->BTR = (uint32_t)((uint32_t)m_pInfo->CAN_Mode << 30) |
                               ((uint32_t)m_pInfo->CAN_SJW << 24)  |
                               ((uint32_t)m_pInfo->CAN_BS1 << 16)  |
                               ((uint32_t)m_pInfo->CAN_BS2 << 20)  |
                               ((uint32_t)m_pInfo->CAN_Prescaler - 1);

        pCANx->MCR &= ~(uint32_t)CAN_MCR_INRQ;       // Request leave initialisation

        WaitAck = 0;                                // Wait the acknowledge

        while(((pCANx->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) && (WaitAck != INAK_TIMEOUT))
        {
            WaitAck++;
        }
        // ...and check acknowledged
        if((pCANx->MSR & CAN_MSR_INAK) != CAN_MSR_INAK)
        {
            State = SYS_OK;

            // ---- Enable I2C event interrupt ----
            PriorityGroup = NVIC_GetPriorityGrouping();
            ISR_Prio.PriorityGroup     = PriorityGroup;
            ISR_Prio.SubPriority       = 0;
            ISR_Prio.PremptionPriority = 5;

            // NVIC Setup for TX DMA channels interrupt request
            ISR_Init(m_pInfo->xx_IRQn, &ISR_Prio);

            // NVIC Setup for RX DMA channels interrupt request
            ISR_Init(m_pInfo->yy_IRQn, &ISR_Prio);
        }
    }

    return State;                              // Return the status of initialization
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FilterInitialize
//
//  Parameter(s):   CAN_FilterInitStruct: pointer to a CAN_FilterInitTypeDef structure that
//                  contains the configuration information.
//  Return:         
//
//  Description:    Configures the CAN reception filter according to the specified parameters.
//                  
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::FilterInitialize(CAN_FilterInitTypeDef* CAN_FilterInitStruct)
{
    uint32_t filter_number_bit_pos = 0;

    filter_number_bit_pos = ((uint32_t)1) << CAN_FilterInitStruct->CAN_FilterNumber;

    /* Initialisation mode for the filter */
    CAN1->FMR |= FMR_FINIT;

    /* Filter Deactivation */
    CAN1->FA1R &= ~(uint32_t)filter_number_bit_pos;

    /* Filter Scale */
    if(CAN_FilterInitStruct->CAN_FilterScale == CAN_FilterScale_16bit)
    {
        /* 16-bit scale for the filter */
        CAN1->FS1R &= ~(uint32_t)filter_number_bit_pos;

        /* First 16-bit identifier and First 16-bit mask */
        /* Or First 16-bit identifier and Second 16-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR1 = 
           ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdLow) << 16) |
            (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdLow);

        /* Second 16-bit identifier and Second 16-bit mask */
        /* Or Third 16-bit identifier and Fourth 16-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR2 = 
           ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdHigh) << 16) |
            (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdHigh);
    }

    if(CAN_FilterInitStruct->CAN_FilterScale == CAN_FilterScale_32bit)
    {
        /* 32-bit scale for the filter */
        CAN1->FS1R |= filter_number_bit_pos;
        /* 32-bit identifier or First 32-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR1 = 
            ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdHigh) << 16) |
             (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterIdLow);
        /* 32-bit mask or Second 32-bit identifier */
        CAN1->sFilterRegister[CAN_FilterInitStruct->CAN_FilterNumber].FR2 = 
            ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdHigh) << 16) |
             (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->CAN_FilterMaskIdLow);
    }

    /* Filter Mode */
    if(CAN_FilterInitStruct->CAN_FilterMode == CAN_FilterMode_IdMask)
    {
        /*Id/Mask mode for the filter*/
        CAN1->FM1R &= ~(uint32_t)filter_number_bit_pos;
    }
    else /* CAN_FilterInitStruct->CAN_FilterMode == CAN_FilterMode_IdList */
    {
        /*Identifier list mode for the filter*/
        CAN1->FM1R |= (uint32_t)filter_number_bit_pos;
    }

    /* Filter FIFO assignment */
    if(CAN_FilterInitStruct->CAN_FilterFIFOAssignment == CAN_Filter_FIFO0)
    {
        /* FIFO 0 assignation for the filter */
        CAN1->FFA1R &= ~(uint32_t)filter_number_bit_pos;
    }

    if(CAN_FilterInitStruct->CAN_FilterFIFOAssignment == CAN_Filter_FIFO1)
    {
        /* FIFO 1 assignation for the filter */
        CAN1->FFA1R |= (uint32_t)filter_number_bit_pos;
    }
  
    /* Filter activation */
    if(CAN_FilterInitStruct->CAN_FilterActivation == ENABLE)
    {
        CAN1->FA1R |= filter_number_bit_pos;
    }

    /* Leave the initialisation mode for the filter */
    CAN1->FMR &= ~FMR_FINIT;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SlaveStartBank
//
//  Parameter(s):   CAN_BankNumber: Select the start slave bank filter from 1..27.
//  Return:         None
//
//  Description:    Select the start bank filter for slave CAN.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::SlaveStartBank(uint8_t CAN_BankNumber) 
{
    CAN1->FMR |= FMR_FINIT;                         // Enter Initialisation mode for the filter
    CAN1->FMR &= (uint32_t)0xFFFFC0F1;              // Select the start slave bank
    CAN1->FMR |= (uint32_t)(CAN_BankNumber)<<8;
    CAN1->FMR &= ~FMR_FINIT;                        // Leave Initialisation mode for the filter
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DBGFreeze
//
//  Parameter(s):   CANx: where x can be 1 or 2 to to select the CAN peripheral.
//                  NewState: new state of the CAN peripheral. 
//                      DEF_ENABLE:  CAN reception/transmission is frozen during debug.
//                                   Reception FIFOs can still be accessed/controlled normally.
//                      DEF_DISABLE: CAN is working during debug.
//  Return:         
//
//  Description:    Enables or disables the DBG Freeze for CAN.
//
//  Note(s):        
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::DBGFreeze(FunctionalState NewState)
{
    if(NewState == DEF_ENABLE)
    {
        m_pInfo->pCANx->MCR |= MCR_DBF;   // Enable Debug Freeze
    }
    else
    {
        m_pInfo->pCANx->MCR &= ~MCR_DBF;  // Disable Debug Freeze
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ITConfig
//
//  Parameter(s):   CAN_IT: specifies the CAN interrupt sources to be enabled or disabled.
//                      This parameter can be: 
//                          CAN_IT_TME: Transmit mailbox empty Interrupt 
//                          CAN_IT_FMP0: FIFO 0 message pending Interrupt 
//                          CAN_IT_FF0: FIFO 0 full Interrupt
//                          CAN_IT_FOV0: FIFO 0 overrun Interrupt
//                          CAN_IT_FMP1: FIFO 1 message pending Interrupt 
//                          CAN_IT_FF1: FIFO 1 full Interrupt
//                          CAN_IT_FOV1: FIFO 1 overrun Interrupt
//                          CAN_IT_WKU: Wake-up Interrupt
//                          CAN_IT_SLK: Sleep acknowledge Interrupt  
//                          CAN_IT_EWG: Error warning Interrupt
//                          CAN_IT_EPV: Error passive Interrupt
//                          CAN_IT_BOF: Bus-off Interrupt  
//                          CAN_IT_LEC: Last error code Interrupt
//                          CAN_IT_ERR: Error Interrupt
//                  NewState: new state of the CAN interrupts.
//                      This parameter can be: DEF_ENABLE or DEF_DISABLE.
//                  
//  Return:         
//
//  Description:    Enables or disables the specified CANx interrupts.
//
//  Note(s):        
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::ITConfig(uint32_t CAN_IT, FunctionalState NewState)
{
    if(NewState != DEF_DISABLE)
    {
        m_pInfo->pCANx->IER |= CAN_IT;      // Enable the selected CANx interrupt
    }
    else
    {
        m_pInfo->pCANx->IER &= ~CAN_IT;     // Disable the selected CANx interrupt
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TTComModeCmd
//
//  Parameter(s):   NewState: Mode new state. This parameter can be: ENABLE or DISABLE.
//                            When enabled, Time stamp (TIME[15:0]) value is  sent in the last two
//                            data bytes of the 8-byte message: TIME[7:0] in data byte 6 and
//                            TIME[15:8] in data byte 7.
//                  
//  Return:         None
//
//  Description:    Enables or disables the CAN Time TriggerOperation communication mode.
//
//  Note(s):        DLC must be programmed as 8 in order Time Stamp (2 bytes) to be 
//                  sent over the CAN bus.
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::TTComModeCmd(FunctionalState NewState)
{
    CAN_TypeDef* pCANx;
    
    pCANx = m_pInfo->pCANx;

    if(NewState != DEF_DISABLE)
    {
        pCANx->MCR |= CAN_MCR_TTCM;      // Enable the TTCM mode

        // Set TGT bits
        pCANx->sTxMailBox[0].TDTR |= ((uint32_t)CAN_TDT0R_TGT);
        pCANx->sTxMailBox[1].TDTR |= ((uint32_t)CAN_TDT1R_TGT);
        pCANx->sTxMailBox[2].TDTR |= ((uint32_t)CAN_TDT2R_TGT);
    }
    else
    {
        // Disable the TTCM mode
        pCANx->MCR &= (uint32_t)(~(uint32_t)CAN_MCR_TTCM);

        // Reset TGT bits
        pCANx->sTxMailBox[0].TDTR &= ((uint32_t)~CAN_TDT0R_TGT);
        pCANx->sTxMailBox[1].TDTR &= ((uint32_t)~CAN_TDT1R_TGT);
        pCANx->sTxMailBox[2].TDTR &= ((uint32_t)~CAN_TDT2R_TGT);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Transmit
//
//  Parameter(s):   TxMessage: pointer to a structure which contains CAN Id, CAN DLC and CAN data.
//                  
//  Return:         The number of the mailbox that is used for transmission or
//                  CAN_TxStatus_NoMailBox if there is no empty mailbox.
//
//  Description:    Initiates and transmits a CAN frame message.
//
//  Note(s):        DLC must be programmed as 8 in order Time Stamp (2 bytes) to be 
//                  sent over the CAN bus.
//
//-------------------------------------------------------------------------------------------------
uint8_t CAN_Driver::Transmit(CanTxMsg* TxMessage)
{
    CAN_TypeDef* pCANx;
    uint8_t      TX_Mailbox = 0;
    
    pCANx = m_pInfo->pCANx;

    // Select one empty transmit mailbox
    if((pCANx->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)
    {
        TX_Mailbox = 0;
    }
    else if((pCANx->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)
    {
        TX_Mailbox = 1;
    }
    else if((pCANx->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)
    {
        TX_Mailbox = 2;
    }
    else
    {
        TX_Mailbox = CAN_TxStatus_NoMailBox;
    }

    if(TX_Mailbox != CAN_TxStatus_NoMailBox)
    {
        // Set up the Id
        pCANx->sTxMailBox[TX_Mailbox].TIR &= TMIDxR_TXRQ;
        
        if(TxMessage->IDE == CAN_Id_Standard)
        {
            pCANx->sTxMailBox[TX_Mailbox].TIR |= ((TxMessage->StdId << 21) | TxMessage->RTR);
        }
        else
        {
            pCANx->sTxMailBox[TX_Mailbox].TIR |= ((TxMessage->ExtId << 3) | TxMessage->IDE | TxMessage->RTR);
        }
    
        // Set up the DLC
        TxMessage->DLC &= (uint8_t)0x0000000F;
        pCANx->sTxMailBox[TX_Mailbox].TDTR &= (uint32_t)0xFFFFFFF0;
        pCANx->sTxMailBox[TX_Mailbox].TDTR |= TxMessage->DLC;

        // Set up the data field
        pCANx->sTxMailBox[TX_Mailbox].TDLR = (((uint32_t)TxMessage->Data[3] << 24) | 
                                              ((uint32_t)TxMessage->Data[2] << 16) |
                                              ((uint32_t)TxMessage->Data[1] << 8)  | 
                                              ((uint32_t)TxMessage->Data[0]));
        pCANx->sTxMailBox[TX_Mailbox].TDHR = (((uint32_t)TxMessage->Data[7] << 24) | 
                                              ((uint32_t)TxMessage->Data[6] << 16) |
                                              ((uint32_t)TxMessage->Data[5] << 8)  |
                                              ((uint32_t)TxMessage->Data[4]));
        // Request transmission
        pCANx->sTxMailBox[TX_Mailbox].TIR |= TMIDxR_TXRQ;
    }
  
    return TX_Mailbox;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TransmitStatus
//
//  Parameter(s):   TransmitMailbox: the number of the mailbox that is used for transmission.
//                  
//  Return:         SYS_READY if the CAN driver transmits the message, 
//                  SYS_FAIL  in an other case.
//
//  Description:    Checks the transmission status of a CAN Frame.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CAN_Driver::TransmitStatus(uint8_t TransmitMailbox)
{
    SystemState_e State          = SYS_UNKNOWN;
    uint32_t      MailboxState   = m_pInfo->pCANx->TSR;
    
    switch(TransmitMailbox)
    {
        case CAN_TXMAILBOX_0:   MailboxState &= (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0);  break;
        case CAN_TXMAILBOX_1:   MailboxState &= (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1);  break;
        case CAN_TXMAILBOX_2:   MailboxState &= (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2);  break;
        default:                State = SYS_FAIL; break;
    }
  
    if(State != SYS_FAIL)
    {
        switch(MailboxState)
        {
            case 0x0:                                               // Transmit pending
            {
                State = CAN_TxStatus_Pending;
            }
            break;

            case (CAN_TSR_RQCP0 | CAN_TSR_TME0):                    // Transmit failed
            case (CAN_TSR_RQCP1 | CAN_TSR_TME1):
            case (CAN_TSR_RQCP2 | CAN_TSR_TME2):
            {
                State = SYS_FAIL;
            }
            break;

            case (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0):    // Transmit succeeded
            case (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1):
            case (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2):
            {
                State = SYS_READY;
            }
            break;

            default:
            {
                State = SYS_FAIL;
            }
            break;
        }
    }
    
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CancelTransmit
//
//  Parameter(s):   Mailbox: Mailbox number.
//
//  Return:         None
//
//  Description:    Cancels a transmit request.    
//
//  Note(s):        
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::CancelTransmit(uint8_t Mailbox)
{
    switch(Mailbox)       // Abort transmission
    {
        case CAN_TXMAILBOX_0:       m_pInfo->pCANx->TSR |= CAN_TSR_ABRQ0;      break;
        case CAN_TXMAILBOX_1:       m_pInfo->pCANx->TSR |= CAN_TSR_ABRQ1;      break;
        case CAN_TXMAILBOX_2:       m_pInfo->pCANx->TSR |= CAN_TSR_ABRQ2;      break;
        default:                                                               break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           
//
//  Parameter(s):   FIFONumber: Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
//                  RxMessage: pointer to a structure receive frame which contains CAN Id,
//                  CAN DLC, CAN data and FMI number.
//
//  Return:         None
//
//  Description:    Receives a correct CAN frame.
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::Receive(uint8_t FIFONumber, CAN_RX_Message_t* RxMessage)
{
    CAN_TypeDef* pCANx;
    uint8_t      TX_Mailbox = 0;
    
    pCANx = m_pInfo->pCANx;

    // Get the Id
    RxMessage->IDE = (uint8_t)0x04 & pCANx->sFIFOMailBox[FIFONumber].RIR;
  
    if(RxMessage->IDE == CAN_Id_Standard)
    {
        RxMessage->StdId = (uint32_t)0x000007FF & (pCANx->sFIFOMailBox[FIFONumber].RIR >> 21);
    }
    else
    {
        RxMessage->ExtId = (uint32_t)0x1FFFFFFF & (pCANx->sFIFOMailBox[FIFONumber].RIR >> 3);
    }
  
    RxMessage->RTR     = (uint8_t)0x02 & ( pCANx->sFIFOMailBox[FIFONumber].RIR);
    RxMessage->DLC     = (uint8_t)0x0F & ( pCANx->sFIFOMailBox[FIFONumber].RDTR);           // Get the DLC
    RxMessage->FMI     = (uint8_t)0xFF & ((pCANx->sFIFOMailBox[FIFONumber].RDTR >> 8));     // Get the FMI
    RxMessage->Data[0] = (uint8_t)0xFF & ( pCANx->sFIFOMailBox[FIFONumber].RDLR);           // Get the data field
    RxMessage->Data[1] = (uint8_t)0xFF & ((pCANx->sFIFOMailBox[FIFONumber].RDLR >> 8));
    RxMessage->Data[2] = (uint8_t)0xFF & ((pCANx->sFIFOMailBox[FIFONumber].RDLR >> 16));
    RxMessage->Data[3] = (uint8_t)0xFF & ((pCANx->sFIFOMailBox[FIFONumber].RDLR >> 24));
    RxMessage->Data[4] = (uint8_t)0xFF & ( pCANx->sFIFOMailBox[FIFONumber].RDHR);                        // & is useless
    RxMessage->Data[5] = (uint8_t)0xFF & ((pCANx->sFIFOMailBox[FIFONumber].RDHR >> 8));
    RxMessage->Data[6] = (uint8_t)0xFF & ((pCANx->sFIFOMailBox[FIFONumber].RDHR >> 16));
    RxMessage->Data[7] = (uint8_t)0xFF & ((pCANx->sFIFOMailBox[FIFONumber].RDHR >> 24));
    
    // Release the FIFO
    if(FIFONumber == CAN_FIFO0)
    {
        pCANx->RF0R |= CAN_RF0R_RFOM0;      // Release FIFO0
    }
    else // if(FIFONumber == CAN_FIFO1)
    {
        pCANx->RF1R |= CAN_RF1R_RFOM1;      // Release FIFO1
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           
//
//  Parameter(s):   
//                  
//  Return:         
//                  
//
//  Description:    
//
//  Note(s):        
//                  
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockToDevice
//
//  Parameter(s):   uint8_t        Device
//  Return:         SystemState_e  State
//
//  Description:    This routine will configure the I2C port to work with a specific device and
//                  lock it, so any other access to the port will be block until unlock
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//
//-------------------------------------------------------------------------------------------------
SystemState_e CAN_Driver::LockToDevice(uint8_t Device)
{
    SystemState_e  State = SYS_NOT_LOCK_TO_DEVICE;
    nOS_StatusReg  sr;

    nOS_EnterCritical(sr);

    if(m_Device == -1)
    {
        m_Device = Device;
        this->Lock();
        State = SYS_READY;
    }

    nOS_LeaveCritical(sr);
    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           UnlockFromDevice
//
//  Parameter(s):   uint8_t         Device
//
//  Return:         SystemState_e   State
//
//  Description:    This routine will unlock I2C port from a specific device
//
//  Note(s):        If a write without lock is executed then it will be done on the locked device
//                  if lock and no write at all if not lock to a device
//
//-------------------------------------------------------------------------------------------------
SystemState_e CAN_Driver::UnlockFromDevice(uint8_t Device)
{
    SystemState_e State = SYS_WRONG_DEVICE;
    nOS_StatusReg sr;

    nOS_EnterCritical(sr);

    if(Device == m_Device)
    {
        this->Unlock();
        m_Device = -1;
        State = SYS_READY;
    }
    else if(m_Device == -1)
    {
        State = SYS_NOT_LOCK_TO_DEVICE;
    }

    nOS_LeaveCritical(sr);
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::Lock(void)
{
    while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK){};
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void I2C_Driver::Unlock(void)
{
    nOS_MutexUnlock(&m_Mutex);
}

//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    xx_IRQHandler
//
//  Description:    This function handles xx interrupt request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
/*
void CAN_Driver::xx_IRQHandler()
{
}
*/
//-------------------------------------------------------------------------------------------------
//
//  IRQ Handler:    yy_IRQHandler
//
//  Description:    This function handles xx interrupt request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
/*
void CAN_Driver::yy_IRQHandler()
{
}
*/
//-------------------------------------------------------------------------------------------------

#endif // (USE_CAN_DRIVER == DEF_ENABLED)

/**
  *          This file provides firmware functions to manage the following 
  *          functionalities of the Controller area network (CAN) peripheral:           
  *           + Initialization and Configuration 
  *           + CAN Frames Transmission 
  *           + CAN Frames Reception    
  *           + Operation modes switch  
  *           + Error management          
  *           + Interrupts and flags        
  *         
                        ##### How to use this driver #####
    [..]            
      (#) Enable the CAN controller interface clock using 
          RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE); for CAN1 
          and RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE); for CAN2
      -@- In case you are using CAN2 only, you have to enable the CAN1 clock.
       
      (#) CAN pins configuration
        (++) Enable the clock for the CAN GPIOs using the following function:
             RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);   
        (++) Connect the involved CAN pins to AF9 using the following function 
             GPIO_PinAFConfig(GPIOx, GPIO_PinSourcex, GPIO_AF_CANx); 
        (++) Configure these CAN pins in alternate function mode by calling
             the function  GPIO_Init();
      
      (#) Initialise and configure the CAN using CAN_Init() and 
          CAN_FilterInit() functions.   
                 
      (#) Transmit the desired CAN frame using CAN_Transmit() function.
           
      (#) Check the transmission of a CAN frame using CAN_TransmitStatus()
          function.
                 
      (#) Cancel the transmission of a CAN frame using CAN_CancelTransmit()
          function.  
              
      (#) Receive a CAN frame using CAN_Recieve() function.
           
      (#) Release the receive FIFOs using CAN_FIFORelease() function.
                 
      (#) Return the number of pending received frames using 
          CAN_MessagePending() function.            
                     
      (#) To control CAN events you can use one of the following two methods:
        (++) Check on CAN flags using the CAN_GetFlagStatus() function.  
        (++) Use CAN interrupts through the function CAN_ITConfig() at 
             initialization phase and CAN_GetITStatus() function into 
             interrupt routines to check if the event has occurred or not.
             After checking on a flag you should clear it using CAN_ClearFlag()
             function. And after checking on an interrupt event you should 
             clear it using CAN_ClearITPendingBit() function.            
  */

/** @defgroup CAN_Group1 Initialization and Configuration functions
 *  @brief    Initialization and Configuration functions 
 *
 ===============================================================================
              ##### Initialization and Configuration functions #####
 ===============================================================================  
    [..] This section provides functions allowing to 
      (+) Initialize the CAN peripherals : Prescaler, operating mode, the maximum 
          number of time quanta to perform resynchronization, the number of time 
          quanta in Bit Segment 1 and 2 and many other modes. 
          Refer to  @ref CAN_InitTypeDef  for more details.
      (+) Configures the CAN reception filter.                                      
      (+) Select the start bank filter for slave CAN.
      (+) Enables or disables the Debug Freeze mode for CAN
      (+)Enables or disables the CAN Time Trigger Operation communication mode
   
  */
  


/**
  * @brief  Fills each CAN_InitStruct member with its default value.
  * @param  CAN_InitStruct: pointer to a CAN_InitTypeDef structure which ill be initialized.
  * @retval None
  */
void CAN_StructInit(CAN_Info* CAN_InitStruct)
{
    /* Reset CAN init structure parameters values */
  
    /* Initialize the time triggered communication mode */
    CAN_InitStruct->CAN_TTCM = DISABLE;
  
    /* Initialize the automatic bus-off management */
    CAN_InitStruct->CAN_ABOM = DISABLE;
  
    /* Initialize the automatic wake-up mode */
    CAN_InitStruct->CAN_AWUM = DISABLE;
  
    /* Initialize the no automatic retransmission */
    CAN_InitStruct->CAN_NART = DISABLE;
  
    /* Initialize the receive FIFO locked mode */
    CAN_InitStruct->CAN_RFLM = DISABLE;
  
    /* Initialize the transmit FIFO priority */
    CAN_InitStruct->CAN_TXFP = DISABLE;
  
    /* Initialize the CAN_Mode member */
    CAN_InitStruct->CAN_Mode = CAN_Mode_Normal;
  
    /* Initialize the CAN_SJW member */
    CAN_InitStruct->CAN_SJW = CAN_SJW_1tq;
  
    /* Initialize the CAN_BS1 member */
    CAN_InitStruct->CAN_BS1 = CAN_BS1_4tq;
  
    /* Initialize the CAN_BS2 member */
    CAN_InitStruct->CAN_BS2 = CAN_BS2_3tq;
  
    /* Initialize the CAN_Prescaler member */
    CAN_InitStruct->CAN_Prescaler = 1;
}


/** @defgroup CAN_Group3 CAN Frames Reception functions
 *  @brief    CAN Frames Reception functions 
 *
 ===============================================================================
                ##### CAN Frames Reception functions #####
 ===============================================================================  
    [..] This section provides functions allowing to 
      (+) Receive a correct CAN frame
      (+) Release a specified receive FIFO (2 FIFOs are available)
      (+) Return the number of the pending received CAN frames
   
  */

/**
  * @brief  Releases the specified receive FIFO.
  * @param  
  * @param  FIFONumber: FIFO to release, CAN_FIFO0 or CAN_FIFO1.
  * @retval None
  */
void CAN_FIFORelease(uint8_t FIFONumber)
{
    /* Release FIFO0 */
    if(FIFONumber == CAN_FIFO0)
    {
        CANx->RF0R |= CAN_RF0R_RFOM0;
    }
    /* Release FIFO1 */
    else /* FIFONumber == CAN_FIFO1 */
    {
        CANx->RF1R |= CAN_RF1R_RFOM1;
    }
}

/**
  * @brief  Returns the number of pending received messages.
  * @param 
  * @param  FIFONumber: Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
  * @retval NbMessage : which is the number of pending message.
  */
uint8_t CAN_MessagePending(uint8_t FIFONumber)
{
    uint8_t message_pending = 0;
    
    if(FIFONumber == CAN_FIFO0)
    {
        message_pending = (uint8_t)(CANx->RF0R&(uint32_t)0x03);
    }
    else if(FIFONumber == CAN_FIFO1)
    {
        message_pending = (uint8_t)(CANx->RF1R&(uint32_t)0x03);
    }
    else
    {
        message_pending = 0;
    }
    
    return message_pending;
}

/** @defgroup CAN_Group4 CAN Operation modes functions
 *  @brief    CAN Operation modes functions 
 *
@verbatim    
 ===============================================================================
                    ##### CAN Operation modes functions #####
 ===============================================================================  
    [..] This section provides functions allowing to select the CAN Operation modes
      (+) sleep mode
      (+) normal mode 
      (+) initialization mode
   
  */
  
  
/**
  * @brief  Selects the CAN Operation mode.
  * @param  CAN_OperatingMode: CAN Operating Mode.
  *         This parameter can be one of @ref CAN_OperatingMode_TypeDef enumeration.
  * @retval status of the requested mode which can be 
  *         - CAN_ModeStatus_Failed:  CAN failed entering the specific mode 
  *         - CAN_ModeStatus_Success: CAN Succeed entering the specific mode 
  */
uint8_t CAN_OperatingModeRequest(uint8_t CAN_OperatingMode)
{
    uint8_t status = CAN_ModeStatus_Failed;
  
    /* Timeout for INAK or also for SLAK bits*/
    uint32_t timeout = INAK_TIMEOUT; 

    if(CAN_OperatingMode == CAN_OperatingMode_Initialization)
    {
        /* Request initialisation */
        CANx->MCR = (uint32_t)((CANx->MCR & (uint32_t)(~(uint32_t)CAN_MCR_SLEEP)) | CAN_MCR_INRQ);

        /* Wait the acknowledge */
        while(((CANx->MSR & CAN_MODE_MASK) != CAN_MSR_INAK) && (timeout != 0))
        {
            timeout--;
        }
        
        if((CANx->MSR & CAN_MODE_MASK) != CAN_MSR_INAK)
        {
            status = CAN_ModeStatus_Failed;
        }
        else
        {
            status = CAN_ModeStatus_Success;
        }
    }
    else if(CAN_OperatingMode == CAN_OperatingMode_Normal)
    {
        /* Request leave initialisation and sleep mode  and enter Normal mode */
        CANx->MCR &= (uint32_t)(~(CAN_MCR_SLEEP|CAN_MCR_INRQ));

        /* Wait the acknowledge */
        while(((CANx->MSR & CAN_MODE_MASK) != 0) && (timeout!=0))
        {
            timeout--;
        }
        
        if((CANx->MSR & CAN_MODE_MASK) != 0)
        {
            status = CAN_ModeStatus_Failed;
        }
        else
        {
            status = CAN_ModeStatus_Success;
        }
    }
    else if(CAN_OperatingMode == CAN_OperatingMode_Sleep)
    {
        /* Request Sleep mode */
        CANx->MCR = (uint32_t)((CANx->MCR & (uint32_t)(~(uint32_t)CAN_MCR_INRQ)) | CAN_MCR_SLEEP);

        /* Wait the acknowledge */
        while(((CANx->MSR & CAN_MODE_MASK) != CAN_MSR_SLAK) && (timeout!=0))
        {
            timeout--;
        }
    
        if((CANx->MSR & CAN_MODE_MASK) != CAN_MSR_SLAK)
        {
            status = CAN_ModeStatus_Failed;
        }
        else
        {
            status = CAN_ModeStatus_Success;
        }
    }
    else
    {
        status = CAN_ModeStatus_Failed;
    }

    return uint8_t(status);
}

/**
  * @brief  Enters the Sleep (low power) mode.
  * @param  
  * @retval CAN_Sleep_Ok if sleep entered, CAN_Sleep_Failed otherwise.
  */
uint8_t CAN_Sleep(void)
{
    uint8_t sleepstatus = CAN_Sleep_Failed;
  
    /* Request Sleep mode */
    CANx->MCR = (((CANx->MCR) & (uint32_t)(~(uint32_t)CAN_MCR_INRQ)) | CAN_MCR_SLEEP);
   
    /* Sleep mode status */
    if((CANx->MSR & (CAN_MSR_SLAK|CAN_MSR_INAK)) == CAN_MSR_SLAK)
    {
        /* Sleep mode not entered */
        sleepstatus =  CAN_Sleep_Ok;
    }
  
    /* return sleep mode status */
    return (uint8_t)sleepstatus;
}

/**
  * @brief  Wakes up the CAN peripheral from sleep mode .
  * @param  CANx: where x can be 1 or 2 to select the CAN peripheral.
  * @retval CAN_WakeUp_Ok if sleep mode left, CAN_WakeUp_Failed otherwise.
  */
uint8_t CAN_WakeUp(void)
{
    uint32_t wait_slak = SLAK_TIMEOUT;
    uint8_t wakeupstatus = CAN_WakeUp_Failed;
    
    /* Wake up request */
    CANx->MCR &= ~(uint32_t)CAN_MCR_SLEEP;
    
    /* Sleep mode status */
    while(((CANx->MSR & CAN_MSR_SLAK) == CAN_MSR_SLAK)&&(wait_slak!=0x00))
    {
        wait_slak--;
    }
    if((CANx->MSR & CAN_MSR_SLAK) != CAN_MSR_SLAK)
    {
        /* wake up done : Sleep mode exited */
        wakeupstatus = CAN_WakeUp_Ok;
    }
    
    /* return wakeup status */
    return (uint8_t)wakeupstatus;
}


/** @defgroup CAN_Group5 CAN Bus Error management functions
 *  @brief    CAN Bus Error management functions 
 *
  
 ===============================================================================
                ##### CAN Bus Error management functions #####
 ===============================================================================  
    [..] This section provides functions allowing to 
      (+) Return the CANx's last error code (LEC)
      (+) Return the CANx Receive Error Counter (REC)
      (+) Return the LSB of the 9-bit CANx Transmit Error Counter(TEC).
   
      -@- If TEC is greater than 255, The CAN is in bus-off state.
      -@- if REC or TEC are greater than 96, an Error warning flag occurs.
      -@- if REC or TEC are greater than 127, an Error Passive Flag occurs.
                        
  */
  
/**
  * @brief  Returns the CANx's last error code (LEC).
  * @param  
  * @retval Error code: 
  *          - CAN_ERRORCODE_NoErr: No Error  
  *          - CAN_ERRORCODE_StuffErr: Stuff Error
  *          - CAN_ERRORCODE_FormErr: Form Error
  *          - CAN_ERRORCODE_ACKErr : Acknowledgment Error
  *          - CAN_ERRORCODE_BitRecessiveErr: Bit Recessive Error
  *          - CAN_ERRORCODE_BitDominantErr: Bit Dominant Error
  *          - CAN_ERRORCODE_CRCErr: CRC Error
  *          - CAN_ERRORCODE_SoftwareSetErr: Software Set Error  
  */
uint8_t CAN_GetLastErrorCode(void)
{
    uint8_t errorcode = 0;
  
    /* Get the error code*/
    errorcode = (((uint8_t)CANx->ESR) & (uint8_t)CAN_ESR_LEC);
  
    /* Return the error code*/
    return errorcode;
}

/**
  * @brief  Returns the CANx Receive Error Counter (REC).
  * @note   In case of an error during reception, this counter is incremented 
  *         by 1 or by 8 depending on the error condition as defined by the CAN 
  *         standard. After every successful reception, the counter is 
  *         decremented by 1 or reset to 120 if its value was higher than 128. 
  *         When the counter value exceeds 127, the CAN controller enters the 
  *         error passive state.  
  * @param    
  * @retval CAN Receive Error Counter. 
  */
uint8_t CAN_GetReceiveErrorCounter(void)
{
    uint8_t counter = 0;
  
    /* Get the Receive Error Counter*/
    counter = (uint8_t)((CANx->ESR & CAN_ESR_REC)>> 24);
  
    /* Return the Receive Error Counter*/
    return counter;
}


/**
  * @brief  Returns the LSB of the 9-bit CANx Transmit Error Counter(TEC).
  * @param  
  * @retval LSB of the 9-bit CAN Transmit Error Counter. 
  */
uint8_t CAN_GetLSBTransmitErrorCounter(void)
{
    uint8_t counter = 0;
  
    /* Get the LSB of the 9-bit CANx Transmit Error Counter(TEC) */
    counter = (uint8_t)((CANx->ESR & CAN_ESR_TEC)>> 16);
  
    /* Return the LSB of the 9-bit CANx Transmit Error Counter(TEC) */
    return counter;
}

/**  *  @brief   Interrupts and flags management functions
 *
 ===============================================================================
              ##### Interrupts and flags management functions #####
 ===============================================================================  

     [..] This section provides functions allowing to configure the CAN Interrupts 
          and to get the status and clear flags and Interrupts pending bits.
  
          The CAN provides 14 Interrupts sources and 15 Flags:

   
  *** Flags ***
  =============
    [..] The 15 flags can be divided on 4 groups: 

      (+) Transmit Flags
        (++) CAN_FLAG_RQCP0, 
        (++) CAN_FLAG_RQCP1, 
        (++) CAN_FLAG_RQCP2  : Request completed MailBoxes 0, 1 and 2  Flags
                               Set when when the last request (transmit or abort)
                               has been performed. 

      (+) Receive Flags


        (++) CAN_FLAG_FMP0,
        (++) CAN_FLAG_FMP1   : FIFO 0 and 1 Message Pending Flags 
                               set to signal that messages are pending in the receive 
                               FIFO.
                               These Flags are cleared only by hardware. 

        (++) CAN_FLAG_FF0,
        (++) CAN_FLAG_FF1    : FIFO 0 and 1 Full Flags
                               set when three messages are stored in the selected 
                               FIFO.                        

        (++) CAN_FLAG_FOV0              
        (++) CAN_FLAG_FOV1   : FIFO 0 and 1 Overrun Flags
                               set when a new message has been received and passed 
                               the filter while the FIFO was full.         

      (+) Operating Mode Flags

        (++) CAN_FLAG_WKU    : Wake up Flag
                               set to signal that a SOF bit has been detected while 
                               the CAN hardware was in Sleep mode. 
        
        (++) CAN_FLAG_SLAK   : Sleep acknowledge Flag
                               Set to signal that the CAN has entered Sleep Mode. 
    
      (+) Error Flags

        (++) CAN_FLAG_EWG    : Error Warning Flag
                               Set when the warning limit has been reached (Receive 
                               Error Counter or Transmit Error Counter greater than 96). 
                               This Flag is cleared only by hardware.
                            
        (++) CAN_FLAG_EPV    : Error Passive Flag
                               Set when the Error Passive limit has been reached 
                               (Receive Error Counter or Transmit Error Counter 
                               greater than 127).
                               This Flag is cleared only by hardware.
                             
        (++) CAN_FLAG_BOF    : Bus-Off Flag
                               set when CAN enters the bus-off state. The bus-off 
                               state is entered on TEC overflow, greater than 255.
                               This Flag is cleared only by hardware.
                                   
        (++) CAN_FLAG_LEC    : Last error code Flag
                               set If a message has been transferred (reception or
                               transmission) with error, and the error code is hold.              
                           
  *** Interrupts ***
  ==================
    [..] The 14 interrupts can be divided on 4 groups: 
  
      (+) Transmit interrupt
  
        (++) CAN_IT_TME   :  Transmit mailbox empty Interrupt
                             if enabled, this interrupt source is pending when 
                             no transmit request are pending for Tx mailboxes.      

      (+) Receive Interrupts
         
        (++) CAN_IT_FMP0,
        (++) CAN_IT_FMP1    :  FIFO 0 and FIFO1 message pending Interrupts
                               if enabled, these interrupt sources are pending 
                               when messages are pending in the receive FIFO.
                               The corresponding interrupt pending bits are cleared 
                               only by hardware.
                
        (++) CAN_IT_FF0,              
        (++) CAN_IT_FF1     :  FIFO 0 and FIFO1 full Interrupts
                               if enabled, these interrupt sources are pending 
                               when three messages are stored in the selected FIFO.
        
        (++) CAN_IT_FOV0,        
        (++) CAN_IT_FOV1    :  FIFO 0 and FIFO1 overrun Interrupts        
                               if enabled, these interrupt sources are pending 
                               when a new message has been received and passed 
                               the filter while the FIFO was full.

      (+) Operating Mode Interrupts
         
        (++) CAN_IT_WKU     :  Wake-up Interrupt
                               if enabled, this interrupt source is pending when 
                               a SOF bit has been detected while the CAN hardware 
                               was in Sleep mode.
                                  
        (++) CAN_IT_SLK     :  Sleep acknowledge Interrupt
                               if enabled, this interrupt source is pending when 
                               the CAN has entered Sleep Mode.       

      (+) Error Interrupts 
        
        (++) CAN_IT_EWG     :  Error warning Interrupt 
                               if enabled, this interrupt source is pending when
                               the warning limit has been reached (Receive Error 
                               Counter or Transmit Error Counter=96). 
                               
        (++) CAN_IT_EPV     :  Error passive Interrupt        
                               if enabled, this interrupt source is pending when
                               the Error Passive limit has been reached (Receive 
                               Error Counter or Transmit Error Counter>127).
                          
        (++) CAN_IT_BOF     :  Bus-off Interrupt
                               if enabled, this interrupt source is pending when
                               CAN enters the bus-off state. The bus-off state is 
                               entered on TEC overflow, greater than 255.
                               This Flag is cleared only by hardware.
                                  
        (++) CAN_IT_LEC     :  Last error code Interrupt        
                               if enabled, this interrupt source is pending  when
                               a message has been transferred (reception or
                               transmission) with error, and the error code is hold.
                          
        (++) CAN_IT_ERR     :  Error Interrupt
                               if enabled, this interrupt source is pending when 
                               an error condition is pending.      
                      
    [..] Managing the CAN controller events :
 
         The user should identify which mode will be used in his application to 
         manage the CAN controller events: Polling mode or Interrupt mode.
  
      (#) In the Polling Mode it is advised to use the following functions:
        (++) CAN_GetFlagStatus() : to check if flags events occur. 
        (++) CAN_ClearFlag()     : to clear the flags events.
  

  
      (#) In the Interrupt Mode it is advised to use the following functions:
        (++) CAN_ITConfig()       : to enable or disable the interrupt source.
        (++) CAN_GetITStatus()    : to check if Interrupt occurs.
        (++) CAN_ClearITPendingBit() : to clear the Interrupt pending Bit 
            (corresponding Flag).
        -@@-  This function has no impact on CAN_IT_FMP0 and CAN_IT_FMP1 Interrupts 
             pending bits since there are cleared only by hardware. 
  */ 
/**
  * @brief  Checks whether the specified CAN flag is set or not.
  * @param  CAN_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg CAN_FLAG_RQCP0: Request MailBox0 Flag
  *            @arg CAN_FLAG_RQCP1: Request MailBox1 Flag
  *            @arg CAN_FLAG_RQCP2: Request MailBox2 Flag
  *            @arg CAN_FLAG_FMP0: FIFO 0 Message Pending Flag   
  *            @arg CAN_FLAG_FF0: FIFO 0 Full Flag       
  *            @arg CAN_FLAG_FOV0: FIFO 0 Overrun Flag 
  *            @arg CAN_FLAG_FMP1: FIFO 1 Message Pending Flag   
  *            @arg CAN_FLAG_FF1: FIFO 1 Full Flag        
  *            @arg CAN_FLAG_FOV1: FIFO 1 Overrun Flag     
  *            @arg CAN_FLAG_WKU: Wake up Flag
  *            @arg CAN_FLAG_SLAK: Sleep acknowledge Flag 
  *            @arg CAN_FLAG_EWG: Error Warning Flag
  *            @arg CAN_FLAG_EPV: Error Passive Flag  
  *            @arg CAN_FLAG_BOF: Bus-Off Flag    
  *            @arg CAN_FLAG_LEC: Last error code Flag      
  * @retval The new state of CAN_FLAG (SET or RESET).
  */
FlagStatus CAN_GetFlagStatus(uint32_t CAN_FLAG)
{
    FlagStatus bitstatus = RESET;
 
    if((CAN_FLAG & CAN_FLAGS_ESR) != (uint32_t)RESET)
    { 
        /* Check the status of the specified CAN flag */
        if((CANx->ESR & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)RESET)
        {
            /* CAN_FLAG is set */
            bitstatus = SET;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = RESET;
        }
    }
    else if((CAN_FLAG & CAN_FLAGS_MSR) != (uint32_t)RESET)
    { 
        /* Check the status of the specified CAN flag */
        if((CANx->MSR & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)RESET)
        { 
            /* CAN_FLAG is set */
            bitstatus = SET;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = RESET;
        }
    }
    else if((CAN_FLAG & CAN_FLAGS_TSR) != (uint32_t)RESET)
    {
        /* Check the status of the specified CAN flag */
        if((CANx->TSR & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)RESET)
        { 
            /* CAN_FLAG is set */
            bitstatus = SET;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = RESET;
        }
    }
    else if((CAN_FLAG & CAN_FLAGS_RF0R) != (uint32_t)RESET)
    { 
        /* Check the status of the specified CAN flag */
        if((CANx->RF0R & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)RESET)
        { 
            /* CAN_FLAG is set */
            bitstatus = SET;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = RESET;
        }
    }
    else /* If(CAN_FLAG & CAN_FLAGS_RF1R != (uint32_t)RESET) */
    { 
        /* Check the status of the specified CAN flag */
        if((uint32_t)(CANx->RF1R & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)RESET)
        { 
            /* CAN_FLAG is set */
            bitstatus = SET;
        }
        else
        {            
            /* CAN_FLAG is reset */
            bitstatus = RESET;
        }
    }
    
    /* Return the CAN_FLAG status */
    return  bitstatus;
}

/**
  * @brief  Clears the CAN's pending flags.
  * @param  CAN_FLAG: specifies the flag to clear.
  *          This parameter can be one of the following values:
  *            @arg CAN_FLAG_RQCP0: Request MailBox0 Flag
  *            @arg CAN_FLAG_RQCP1: Request MailBox1 Flag
  *            @arg CAN_FLAG_RQCP2: Request MailBox2 Flag 
  *            @arg CAN_FLAG_FF0: FIFO 0 Full Flag       
  *            @arg CAN_FLAG_FOV0: FIFO 0 Overrun Flag  
  *            @arg CAN_FLAG_FF1: FIFO 1 Full Flag        
  *            @arg CAN_FLAG_FOV1: FIFO 1 Overrun Flag     
  *            @arg CAN_FLAG_WKU: Wake up Flag
  *            @arg CAN_FLAG_SLAK: Sleep acknowledge Flag    
  *            @arg CAN_FLAG_LEC: Last error code Flag        
  * @retval None
  */
void CAN_ClearFlag(uint32_t CAN_FLAG)
{
    uint32_t flagtmp=0;

    if(CAN_FLAG == CAN_FLAG_LEC) // ESR register
    {
        CANx->ESR = (uint32_t)RESET;                            // Clear the selected CAN flags
    }
    else // MSR or TSR or RF0R or RF1R
    {
        flagtmp = CAN_FLAG & 0x000FFFFF;

        if((CAN_FLAG & CAN_FLAGS_RF0R)!=(uint32_t)RESET)
        {
            CANx->RF0R = (uint32_t)(flagtmp);                   // Receive Flags
        }
        else if((CAN_FLAG & CAN_FLAGS_RF1R)!=(uint32_t)RESET)
        {
            CANx->RF1R = (uint32_t)(flagtmp);                   // Receive Flags
        }
        else if((CAN_FLAG & CAN_FLAGS_TSR)!=(uint32_t)RESET)
        {
            CANx->TSR = (uint32_t)(flagtmp);                    // Transmit Flags
        }
        else // If((CAN_FLAG & CAN_FLAGS_MSR)!=(uint32_t)RESET)
        {
            CANx->MSR = (uint32_t)(flagtmp);                    // Operating mode Flags
        }
    }
}

/**
  * @brief  Checks whether the specified CANx interrupt has occurred or not.
  * @param  CAN_IT: specifies the CAN interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg CAN_IT_TME: Transmit mailbox empty Interrupt 
  *            @arg CAN_IT_FMP0: FIFO 0 message pending Interrupt 
  *            @arg CAN_IT_FF0: FIFO 0 full Interrupt
  *            @arg CAN_IT_FOV0: FIFO 0 overrun Interrupt
  *            @arg CAN_IT_FMP1: FIFO 1 message pending Interrupt 
  *            @arg CAN_IT_FF1: FIFO 1 full Interrupt
  *            @arg CAN_IT_FOV1: FIFO 1 overrun Interrupt
  *            @arg CAN_IT_WKU: Wake-up Interrupt
  *            @arg CAN_IT_SLK: Sleep acknowledge Interrupt  
  *            @arg CAN_IT_EWG: Error warning Interrupt
  *            @arg CAN_IT_EPV: Error passive Interrupt
  *            @arg CAN_IT_BOF: Bus-off Interrupt  
  *            @arg CAN_IT_LEC: Last error code Interrupt
  *            @arg CAN_IT_ERR: Error Interrupt
  * @retval The current state of CAN_IT (SET or RESET).
  */
ITStatus CAN_GetITStatus(uint32_t CAN_IT)
{
    ITStatus itstatus = RESET;
  
    if((CANx->IER & CAN_IT) != RESET)                                                           // Check the interrupt enable bit
    {
        switch(CAN_IT)                                                                          // In case the Interrupt is enabled, ....
        {
            case CAN_IT_TME:
                itstatus = CheckITStatus(CANx->TSR, CAN_TSR_RQCP0|CAN_TSR_RQCP1|CAN_TSR_RQCP2); // Check CAN_TSR_RQCPx bits
            break;
            
            case CAN_IT_FMP0:
                itstatus = CheckITStatus(CANx->RF0R, CAN_RF0R_FMP0);                            // Check CAN_RF0R_FMP0 bit
            break;
            
            case CAN_IT_FF0:
                itstatus = CheckITStatus(CANx->RF0R, CAN_RF0R_FULL0);                           // Check CAN_RF0R_FULL0 bit
            break;
            
            case CAN_IT_FOV0:
                itstatus = CheckITStatus(CANx->RF0R, CAN_RF0R_FOVR0);                           // Check CAN_RF0R_FOVR0 bit
            break;
            
            case CAN_IT_FMP1:
                itstatus = CheckITStatus(CANx->RF1R, CAN_RF1R_FMP1);                            // Check CAN_RF1R_FMP1 bit
            break;
            
            case CAN_IT_FF1:
                itstatus = CheckITStatus(CANx->RF1R, CAN_RF1R_FULL1);                           // Check CAN_RF1R_FULL1 bit
            break;
            
            case CAN_IT_FOV1:
                itstatus = CheckITStatus(CANx->RF1R, CAN_RF1R_FOVR1);                           // Check CAN_RF1R_FOVR1 bit
            break;
            
            case CAN_IT_WKU:
                itstatus = CheckITStatus(CANx->MSR, CAN_MSR_WKUI);                              // Check CAN_MSR_WKUI bit
            break;
            
            case CAN_IT_SLK:
                itstatus = CheckITStatus(CANx->MSR, CAN_MSR_SLAKI);                             // Check CAN_MSR_SLAKI bit
            break;
            
            case CAN_IT_EWG:
                itstatus = CheckITStatus(CANx->ESR, CAN_ESR_EWGF);                              // Check CAN_ESR_EWGF bit
            break;
            
            case CAN_IT_EPV:
                itstatus = CheckITStatus(CANx->ESR, CAN_ESR_EPVF);                              // Check CAN_ESR_EPVF bit
            break;
            
            case CAN_IT_BOF:
                itstatus = CheckITStatus(CANx->ESR, CAN_ESR_BOFF);                              // Check CAN_ESR_BOFF bit
            break;
            
            case CAN_IT_LEC:
                itstatus = CheckITStatus(CANx->ESR, CAN_ESR_LEC);                               // Check CAN_ESR_LEC bit
            break;
            
            case CAN_IT_ERR:
                itstatus = CheckITStatus(CANx->MSR, CAN_MSR_ERRI);                              // Check CAN_MSR_ERRI bit 
            break;
            
            default:
                itstatus = RESET;                                                               // In case of error, return RESET
            break;
        }
    }
    else
    {
        itstatus  = RESET;          // In case the Interrupt is not enabled, return RESET
    }
  
    // Return the CAN_IT status
    return itstatus;
}

/**
  * @brief  Clears the CANx's interrupt pending bits.
  * @param  CAN_IT: specifies the interrupt pending bit to clear.
  *          This parameter can be one of the following values:
  *            @arg CAN_IT_TME: Transmit mailbox empty Interrupt
  *            @arg CAN_IT_FF0: FIFO 0 full Interrupt
  *            @arg CAN_IT_FOV0: FIFO 0 overrun Interrupt
  *            @arg CAN_IT_FF1: FIFO 1 full Interrupt
  *            @arg CAN_IT_FOV1: FIFO 1 overrun Interrupt
  *            @arg CAN_IT_WKU: Wake-up Interrupt
  *            @arg CAN_IT_SLK: Sleep acknowledge Interrupt  
  *            @arg CAN_IT_EWG: Error warning Interrupt
  *            @arg CAN_IT_EPV: Error passive Interrupt
  *            @arg CAN_IT_BOF: Bus-off Interrupt  
  *            @arg CAN_IT_LEC: Last error code Interrupt
  *            @arg CAN_IT_ERR: Error Interrupt 
  * @retval None
  */
void CAN_ClearITPendingBit(uint32_t CAN_IT)
{
    switch(CAN_IT)
    {
        case CAN_IT_TME:
        {
            CANx->TSR = CAN_TSR_RQCP0|CAN_TSR_RQCP1|CAN_TSR_RQCP2;          // Clear CAN_TSR_RQCPx (rc_w1)
        }
        break;
        
        case CAN_IT_FF0:
        {
            CANx->RF0R = CAN_RF0R_FULL0;                                    // Clear CAN_RF0R_FULL0 (rc_w1)
        }
        break;
        
        case CAN_IT_FOV0:
        {
            CANx->RF0R = CAN_RF0R_FOVR0;                                    // Clear CAN_RF0R_FOVR0 (rc_w1)
        }
        break;
        
        case CAN_IT_FF1:
        {
            CANx->RF1R = CAN_RF1R_FULL1;                                    // Clear CAN_RF1R_FULL1 (rc_w1)
        }
        break;
        
        case CAN_IT_FOV1:
        {
            CANx->RF1R = CAN_RF1R_FOVR1;                                    // Clear CAN_RF1R_FOVR1 (rc_w1)
        }
        break;
        
        case CAN_IT_WKU:
        {
            CANx->MSR = CAN_MSR_WKUI;                                       // Clear CAN_MSR_WKUI (rc_w1)
        }
        break;
        
        case CAN_IT_SLK:
        {
            CANx->MSR = CAN_MSR_SLAKI;                                      // Clear CAN_MSR_SLAKI (rc_w1)
        }
        break;
        
        case CAN_IT_EWG:
        {
            // Note: the corresponding Flag is cleared by hardware depending on the CAN Bus status
            CANx->MSR = CAN_MSR_ERRI;                                       // Clear CAN_MSR_ERRI (rc_w1)
        }
        break;
        
        case CAN_IT_EPV:
        {
            // Note the corresponding Flag is cleared by hardware depending on the CAN Bus status
            CANx->MSR = CAN_MSR_ERRI;                                       // Clear CAN_MSR_ERRI (rc_w1)
        }
        break;
        
        case CAN_IT_BOF:
        {
            // Note the corresponding Flag is cleared by hardware depending on the CAN Bus status
            CANx->MSR = CAN_MSR_ERRI;                                       // Clear CAN_MSR_ERRI (rc_w1)
        }
        break;
        
        case CAN_IT_LEC:
        {
            CANx->ESR = RESET;                                              // Clear LEC bits
            CANx->MSR = CAN_MSR_ERRI;                                       // Clear CAN_MSR_ERRI (rc_w1)
        }
        break;
        
        case CAN_IT_ERR:
        {
            CANx->ESR = RESET;                                              // Clear LEC bits
            // Note BOFF, EPVF and EWGF Flags are cleared by hardware depending on the CAN Bus status
            CANx->MSR = CAN_MSR_ERRI;                                       // Clear CAN_MSR_ERRI (rc_w1)
        }
        break;
        
        default:
        break;
   }
}

/**
  * @brief  Checks whether the CAN interrupt has occurred or not.
  * @param  CAN_Reg: specifies the CAN interrupt register to check.
  * @param  It_Bit: specifies the interrupt source bit to check.
  * @retval The new state of the CAN Interrupt (SET or RESET).
  */
static ITStatus CheckITStatus(uint32_t CAN_Reg, uint32_t It_Bit)
{
    ITStatus pendingbitstatus = RESET;
  
    if((CAN_Reg & It_Bit) != (uint32_t)RESET)
    {
        pendingbitstatus = SET;             // CAN_IT is set
    }
    else
    {
        pendingbitstatus = RESET;           // CAN_IT is reset
    }
    
    return pendingbitstatus;
}
