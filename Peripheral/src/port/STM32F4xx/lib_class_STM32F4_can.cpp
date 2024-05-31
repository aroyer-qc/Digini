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
#include "./Digini/lib_digini.h"
#undef  CAN_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_CAN_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MCR_DBF                 (uint32_t(0x00010000))      // CAN Master Control Register bits
#define TMIDxR_TXRQ             (uint32_t(0x00000001))      // CAN Mailbox Transmit Request
#define FMR_FINIT               (uint32_t(0x00000001))      // CAN Filter Master Register bits
#define INAK_TIMEOUT            (uint32_t(0x0000FFFF))      // Time out for INAK bit
#define SLAK_TIMEOUT            (uint32_t(0x0000FFFF))      // Time out for SLAK bit
#define CAN_FLAGS_TSR           (uint32_t(0x08000000))      // Flags in TSR register
#define CAN_FLAGS_RF1R          (uint32_t(0x04000000))      // Flags in RF1R register
#define CAN_FLAGS_RF0R          (uint32_t(0x02000000))      // Flags in RF0R register
#define CAN_FLAGS_MSR           (uint32_t(0x01000000))      // Flags in MSR register
#define CAN_FLAGS_ESR           (uint32_t(0x00F00000))      // Flags in ESR register
#define CAN_TXMAILBOX_0         (uint8_t(0x00))             // Mailboxes definition
#define CAN_TXMAILBOX_1         (uint8_t(0x01))
#define CAN_TXMAILBOX_2         (uint8_t(0x02)) 
#define CAN_MODE_MASK           (uint32_t(0x00000003))

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

    pCANx = m_pInfo->pCANx;
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

    pCANx->MCR &= ~uint32_t(CAN_MCR_SLEEP);         // Exit from sleep mode
    pCANx->MCR |= CAN_MCR_INRQ;                     // Request initialisation

    while(((pCANx->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) && (WaitAck != INAK_TIMEOUT))  // Wait the acknowledge
    {
        WaitAck++;
    }

    if((pCANx->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) // Check acknowledge
    {
        if(m_pInfo->TTCM == DEF_ENABLE)             // Set the time triggered communication mode
        {
            pCANx->MCR |= CAN_MCR_TTCM;
        }
        else
        {
            pCANx->MCR &= ~uint32_t(CAN_MCR_TTCM);
        }

        if(m_pInfo->ABOM == DEF_ENABLE)             // Set the automatic bus-off management
        {
            pCANx->MCR |= CAN_MCR_ABOM;
        }
        else
        {
            pCANx->MCR &= ~uint32_t(CAN_MCR_ABOM);
        }

        if(m_pInfo->AWUM == DEF_ENABLE)             // Set the automatic bus-off management
        {
            pCANx->MCR |= CAN_MCR_AWUM;
        }
        else
        {
            pCANx->MCR &= ~uint32_t(CAN_MCR_AWUM);
        }

        if(m_pInfo->NART == DEF_ENABLE)             // Set the no automatic retransmission
        {
            pCANx->MCR |= CAN_MCR_NART;
        }
        else
        {
            pCANx->MCR &= ~uint32_t(CAN_MCR_NART);
        }

        if(m_pInfo->RFLM == DEF_ENABLE)
        {
            pCANx->MCR |= CAN_MCR_RFLM;             // Set the receive FIFO locked mode
        }
        else
        {
            pCANx->MCR &= ~uint32_t(CAN_MCR_RFLM);
        }

        if(m_pInfo->TXFP == DEF_ENABLE)
        {
            pCANx->MCR |= CAN_MCR_TXFP;             // Set the transmit FIFO priority
        }
        else
        {
            pCANx->MCR &= ~uint32_t(CAN_MCR_TXFP);
        }

        // Set the bit timing register
        pCANx->BTR = uint32_t(m_pInfo->Mode << 30) |
                     uint32_t(m_pInfo->SJW << 24)  |
                     uint32_t(m_pInfo->BS1 << 16)  |
                     uint32_t(m_pInfo->BS2 << 20)  |
                     uint32_t(m_pInfo->Prescaler) - 1);

        pCANx->MCR &= ~uint32_t(CAN_MCR_INRQ);      // Request leave initialisation

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

    return State;                                   // Return the status of initialization
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FilterInitialize
//
//  Parameter(s):   FilterInitStruct        Pointer to a CAN_FilterInfo_t structure that
//                                          contains the configuration information.
//  Return:         
//
//  Description:    Configures the CAN reception filter according to the specified parameters.
//                  
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::FilterInitialize(CAN_FilterInfo_t* FilterInitStruct)
{
    uint32_t FilterNumberBitPos = 0;

    FilterNumberBitPos = uint32_t(1) << FilterInitStruct->Number;

    CAN1->FMR |= FMR_FINIT;                                             // Initialisation mode for the filter
    CAN1->FA1R &= ~uint32_t(FilterNumberBitPos);                        // Filter Deactivation

    if(FilterInitStruct->Scale == CAN_FilterScale_16bit)                // Filter Scale
    {
        CAN1->FS1R &= ~uint32_t(FilterNumberBitPos);                    // 16-bit scale for the filter
    
        // First 16-bit identifier and First 16-bit mask Or First 16-bit identifier and Second 16-bit identifier
        CAN1->sFilterRegister[FilterInitStruct->Number].FR1 = (uint32_t(FilterInitStruct->Mask_ID_Low) << 16)  | uint32_t(FilterInitStruct->ID_Low);

        // Second 16-bit identifier and Second 16-bit mask Or Third 16-bit identifier and Fourth 16-bit identifier
        CAN1->sFilterRegister[FilterInitStruct->Number].FR2 = (uint32_t(FilterInitStruct->Mask_ID_High) << 16) | uint32_t(FilterInitStruct->ID_High);
    }

    if(FilterInitStruct->Scale == CAN_FilterScale_32bit)
    {
        CAN1->FS1R |= FilterNumberBitPos;                               // 32-bit scale for the filter
        CAN1->sFilterRegister[FilterInitStruct->Number].FR1 = (uint32_t(FilterInitStruct->ID_High) << 16)      | uint32_t(FilterInitStruct->ID_Low);      // 32-bit identifier or First 32-bit identifier
        CAN1->sFilterRegister[FilterInitStruct->Number].FR2 = (uint32_t(FilterInitStruct->Mask_ID_High) << 16) | uint32_t(FilterInitStruct->Mask_ID_Low); // 32-bit mask or Second 32-bit identifier
    }
        
    if(FilterInitStruct->Mode == CAN_FilterMode_ID_Mask)                // Filter Mode
    {
        CAN1->FM1R &= ~uint32_t(FilterNumberBitPos);                    // Id/Mask mode for the filter
    }
    else // FilterInitStruct->Mode == CAN_FilterMode_ID_List
    {
        CAN1->FM1R |= uint32_t(FilterNumberBitPos);                     // Identifier list mode for the filter
    }

    if(FilterInitStruct->FIFO_Assignment == CAN_Filter_FIFO0)           // Filter FIFO assignment
    {
        CAN1->FFA1R &= ~uint32_t(FilterNumberBitPos);                   // FIFO 0 assignation for the filter
    }

    if(FilterInitStruct->FIFO_Assignment == CAN_Filter_FIFO1)
    {
        CAN1->FFA1R |= uint32_t(FilterNumberBitPos);                    // FIFO 1 assignation for the filter
    }
  
    if(FilterInitStruct->Activation == DEF_ENABLE)                      // Filter activation
    {
        CAN1->FA1R |= FilterNumberBitPos;
    }

    CAN1->FMR &= ~FMR_FINIT;                                            // Leave the initialisation mode for the filter
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
    CAN1->FMR &= uint32_t(0xFFFFC0F1);              // Select the start slave bank
    CAN1->FMR |= uint32_t(CAN_BankNumber) << 8;
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
//  Parameter(s):   NewState    Mode new state. This parameter can be: ENABLE or DISABLE.
//                              When enabled, Time stamp (TIME[15:0]) value is  sent in the last
//                              two data bytes of the 8-byte message: TIME[7:0] in data byte 6 and
//                              TIME[15:8] in data byte 7.
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
//  Parameter(s):   TxMessage   Pointer to a structure which contains CAN Id, CAN DLC and CAN data.
//                  
//  Return:         uint8_t     The number of the mailbox that is used for transmission or
//                              CAN_TxStatus_NoMailBox if there is no empty mailbox.
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
    if((pCANx->TSR & CAN_TSR_TME0) == CAN_TSR_TME0)
    {
        TX_Mailbox = 0;
    }
    else if((pCANx->TSR & CAN_TSR_TME1) == CAN_TSR_TME1)
    {
        TX_Mailbox = 1;
    }
    else if((pCANx->TSR & CAN_TSR_TME2) == CAN_TSR_TME2)
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
        TxMessage->DLC &= uint8_t(0x0F);
        pCANx->sTxMailBox[TX_Mailbox].TDTR &= uint32_t(0xFFFFFFF0);
        pCANx->sTxMailBox[TX_Mailbox].TDTR |= TxMessage->DLC;

        // Set up the data field
        pCANx->sTxMailBox[TX_Mailbox].TDLR = ((uint32_t(TxMessage->Data[3]) << 24) | 
                                              (uint32_t(TxMessage->Data[2]) << 16) |
                                              (uint32_t(TxMessage->Data[1]) << 8)  | 
                                              (uint32_t(TxMessage->Data[0])));
        pCANx->sTxMailBox[TX_Mailbox].TDHR = ((uint32_t(TxMessage->Data[7]) << 24) | 
                                              (uint32_t(TxMessage->Data[6]) << 16) |
                                              (uint32_t(TxMessage->Data[5]) << 8)  |
                                              (uint32_t(TxMessage->Data[4])));
        // Request transmission
        pCANx->sTxMailBox[TX_Mailbox].TIR |= TMIDxR_TXRQ;
    }
  
    return TX_Mailbox;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           TransmitStatus
//
//  Parameter(s):   TransmitMailbox     The number of the mailbox that is used for transmission.
//                  
//  Return:         SystemState_e       SYS_READY if the CAN driver transmits the message, 
//                                      SYS_FAIL  in an other case.
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
//  Parameter(s):   Mailbox         Mailbox number.
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
//  Name:           Receive
//
//  Parameter(s):   FIFONumber      Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
//                  RxMessage       Pointer to a structure receive frame which contains CAN Id,
//                                  CAN DLC, CAN data and FMI number.
//
//  Return:         None
//
//  Description:    Receives a correct CAN frame.
//
//-------------------------------------------------------------------------------------------------

// fix the cast


void CAN_Driver::Receive(uint8_t FIFONumber, CAN_RX_Message_t* RxMessage)
{
    CAN_TypeDef* pCANx      = m_pInfo->pCANx;
    uint8_t      TX_Mailbox = 0;

    // Get the Id
    RxMessage->IDE = uint8_t(0x04 & pCANx->sFIFOMailBox[FIFONumber].RIR);
  
    if(RxMessage->IDE == CAN_Id_Standard)
    {
        RxMessage->StdId = uint32_t(pCANx->sFIFOMailBox[FIFONumber].RIR >> 21) & 0x000007FF;
    }
    else
    {
        RxMessage->ExtId = (uint32_t(pCANx->sFIFOMailBox[FIFONumber].RIR >> 3) & 0x1FFFFFFF;
    }
  
    RxMessage->RTR     = uint8_t(0x02 & pCANx->sFIFOMailBox[FIFONumber].RIR);
    RxMessage->DLC     = uint8_t(0x0F & pCANx->sFIFOMailBox[FIFONumber].RDTR);              // Get the DLC
    RxMessage->FMI     = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDTR >> 8);                // Get the FMI
    RxMessage->Data[0] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDLR);                     // Get the data field
    RxMessage->Data[1] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDLR >> 8);
    RxMessage->Data[2] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDLR >> 16);
    RxMessage->Data[3] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDLR >> 24);
    RxMessage->Data[4] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDHR);                     // & is useless
    RxMessage->Data[5] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDHR >> 8);
    RxMessage->Data[6] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDHR >> 16);
    RxMessage->Data[7] = uint8_t(pCANx->sFIFOMailBox[FIFONumber].RDHR >> 24);
    
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
//  Name:           FIFO_Release
//
//  Parameter(s):   FIFO_Number     FIFO to release, CAN_FIFO0 or CAN_FIFO1.
//                  
//  Return:         None
//                  
//
//  Description:    Releases the specified receive FIFO.
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::FIFO_Release(uint8_t FIFO_Number)
{
    if(FIFO_Number == CAN_FIFO0)                // Release FIFO0
    {
        m_pInfo->pCANx->RF0R |= CAN_RF0R_RFOM0;
    }
    else // if (FIFO_Number == CAN_FIFO1)          Release FIFO1
    {
        m_pInfo->pCANx->RF1R |= CAN_RF1R_RFOM1;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           MessagePending
//
//  Parameter(s):   FIFO_Number     Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
//                  
//  Return:         NbMessage       Which is the number of pending message.
//                  
//
//  Description:    Returns the number of pending received messages.
//
//-------------------------------------------------------------------------------------------------
uint8_t CAN_Driver::MessagePending(uint8_t FIFO_Number)
{
    uint8_t MsgPending = 0;
    
    if(FIFO_Number == CAN_FIFO0)
    {
        MsgPending = uint8_t(m_pInfo->pCANx->RF0R);
    }
    else if(FIFO_Number == CAN_FIFO1)
    {
        MsgPending = uint8_t(m_pInfo->pCANx->RF1R);
    }
    
    MsgPending &= 0x03;
    
    return MsgPending;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           OperatingModeRequest
//
//  Parameter(s):   OperatingMode   This parameter can be one of CAN_OperationMode_e enumeration.
//                  
//  Return:         SystemState_e   State of the requested mode which can be 
//                                      - SYS_FAIL:  CAN failed entering the specific mode 
//                                      - SYS_OK:    CAN Succeed entering the specific mode
//                  
//
//  Description:    Selects the CAN Operation mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CAN_Driver::OperatingModeRequest(CAN_OperationMode_e OperatingMode)
{
    CAN_TypeDef*  pCANx = m_pInfo->pCANx;
    SystemState_e State = SYS_FAIL;
  
    uint32_t TimeOut = INAK_TIMEOUT;                                                                    // Timeout for INAK or also for SLAK bits

    if(CAN_OperatingMode == CAN_OperatingMode_Initialization)
    {
        pCANx->MCR = (uint32_t)((pCANx->MCR & (uint32_t)(~(uint32_t)CAN_MCR_SLEEP)) | CAN_MCR_INRQ);    // Request initialisation

        while(((pCANx->MSR & CAN_MODE_MASK) != CAN_MSR_INAK) && (TimeOut != 0))                         // Wait the acknowledge
        {
            TimeOut--;
        }
        
        if((pCANx->MSR & CAN_MODE_MASK) == CAN_MSR_INAK)
        {
            State = SYS_OK;
        }
    }
    else if(CAN_OperatingMode == CAN_OperatingMode_Normal)
    {
        pCANx->MCR &= (uint32_t)(~(CAN_MCR_SLEEP | CAN_MCR_INRQ));                                      // Request leave initialisation and sleep mode  and enter Normal mode

        while(((pCANx->MSR & CAN_MODE_MASK) != 0) && (TimeOut != 0))                                    // Wait the acknowledge
        {
            TimeOut--;
        }
        
        if((pCANx->MSR & CAN_MODE_MASK) == 0)
        {
            State = SYS_OK;
        }
    }
    else if(CAN_OperatingMode == CAN_OperatingMode_Sleep)
    {
        pCANx->MCR = (uint32_t)((pCANx->MCR & (uint32_t)(~(uint32_t)CAN_MCR_INRQ)) | CAN_MCR_SLEEP);    // Request Sleep mode

        while(((pCANx->MSR & CAN_MODE_MASK) != CAN_MSR_SLAK) && (TimeOut != 0))                         // Wait the acknowledge
        {
            TimeOut--;
        }
    
        if((pCANx->MSR & CAN_MODE_MASK) == CAN_MSR_SLAK)
        {
            State = SYS_OK;
        }
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Sleep
//
//  Parameter(s):   None.
//                  
//  Return:         SystemState_e   State of the sleep which can be 
//                                      - SYS_FAIL:  if sleep failed
//                                      - SYS_OK:    if sleep entered
//                  
//
//  Description:    Enters the Sleep (low power) mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CAN_Driver::Sleep(void)
{
    CAN_TypeDef* pCANx  = m_pInfo->pCANx;
    SystemState_e State = SYS_FAIL;
  
    pCANx->MCR = (((pCANx->MCR) & (uint32_t)(~(uint32_t)CAN_MCR_INRQ)) | CAN_MCR_SLEEP);    // Request Sleep mode
   
    if((pCANx->MSR & (CAN_MSR_SLAK|CAN_MSR_INAK)) == CAN_MSR_SLAK)                          // Sleep mode status
    {
        State = SYS_OK;                                                                     // Sleep mode not entered
    }
  
    return State;                                                                           // Return sleep mode status
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           WakeUp
//
//  Parameter(s):   None
//                  
//  Return:         SystemState_e   State of the wake-up which can be 
//                                      - SYS_FAIL:  if wake-up failed
//                                      - SYS_OK:    if wake-up succeded
//                  
//
//  Description:    Wakes up the CAN peripheral from sleep mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e CAN_Driver::WakeUp(void)
{
    CAN_TypeDef* pCANx  = m_pInfo->pCANx;
    uint32_t WaitSlak   = SLAK_TIMEOUT;
    SystemState_e State = SYS_FAIL;
    
    pCANx->MCR &= ~(uint32_t)CAN_MCR_SLEEP;                                 // Wake up request
    
    while(((pCANx->MSR & CAN_MSR_SLAK) == CAN_MSR_SLAK) && (WaitSlak != 0)) // Sleep mode status
    {
        WaitSlak--;
    }
    
    if((pCANx->MSR & CAN_MSR_SLAK) != CAN_MSR_SLAK)
    {
        State = SYS_OK;                                                     // Wake up done, Sleep mode exited
    }
    
    return State;                                                           // Return wakeup state
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetLastErrorCode
//
//  Parameter(s):   None
//                  
//  Return:         Error code: 
//                      - CAN_ERRORCODE_NoErr:              No Error  
//                      - CAN_ERRORCODE_StuffErr:           Stuff Error
//                      - CAN_ERRORCODE_FormErr:            Form Error
//                      - CAN_ERRORCODE_ACKErr :            Acknowledgment Error
//                      - CAN_ERRORCODE_BitRecessiveErr:    Bit Recessive Error
//                      - CAN_ERRORCODE_BitDominantErr:     Bit Dominant Error
//                      - CAN_ERRORCODE_CRCErr:             CRC Error
//                      - CAN_ERRORCODE_SoftwareSetErr:     Software Set Error
//
//  Description:    Returns the CANx's last error code (LEC).
//
//-------------------------------------------------------------------------------------------------
uint8_t CAN_Driver::GetLastErrorCode(void)
{
    return uint8_t(CANx->ESR) & uint8_t(CAN_ESR_LEC);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetReceiveErrorCounter
//
//  Parameter(s):   None
//                  
//  Return:         CAN Receive Error Counter.
//                  
//  Note(s):        In case of an error during reception, this counter is incremented by 1 or by 8
//                  depending on the error condition as defined by the CAN standard. After every
//                  successful reception, the counter is decremented by 1 or reset to 120 if its
//                  value was higher than 128. 
//                  When the counter value exceeds 127, the CAN controller enters the error passive
//                  state.                
//
//  Description:    eturns the CANx Receive Error Counter (REC).
//
//-------------------------------------------------------------------------------------------------
uint8_t CAN_Driver::GetReceiveErrorCounter(void)
{
    return uint8_t((CANx->ESR & CAN_ESR_REC) >> 24);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetLSB_TransmitErrorCounter
//
//  Parameter(s):   None
//                  
//  Return:         LSB of the 9-bit CAN Transmit Error Counter.
//                  
//  Description:    Returns the LSB of the 9-bit CANx Transmit Error Counter(TEC).
//
//-------------------------------------------------------------------------------------------------
uint8_t CAN_Driver::GetLSB_TransmitErrorCounter(void)
{
    return uint8_t((m_pInfo->CANx->ESR & CAN_ESR_TEC) >> 16);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetFlagStatus
//
//  Parameter(s):   Flag        Specifies the flag to check.
//                                  CAN_FLAG_RQCP0: Request MailBox0 Flag
//                                  CAN_FLAG_RQCP1: Request MailBox1 Flag
//                                  CAN_FLAG_RQCP2: Request MailBox2 Flag
//                                  CAN_FLAG_FMP0:  FIFO 0 Message Pending Flag   
//                                  CAN_FLAG_FF0:   FIFO 0 Full Flag       
//                                  CAN_FLAG_FOV0:  FIFO 0 Overrun Flag 
//                                  CAN_FLAG_FMP1:  FIFO 1 Message Pending Flag   
//                                  CAN_FLAG_FF1:   FIFO 1 Full Flag        
//                                  CAN_FLAG_FOV1:  FIFO 1 Overrun Flag     
//                                  CAN_FLAG_WKU:   Wake up Flag
//                                  CAN_FLAG_SLAK:  Sleep acknowledge Flag 
//                                  CAN_FLAG_EWG:   Error Warning Flag
//                                  CAN_FLAG_EPV:   Error Passive Flag  
//                                  CAN_FLAG_BOF:   Bus-Off Flag    
//                                  CAN_FLAG_LEC:   Last error code Flag      
//                  
//  Return:         The new state of Flag (0 or 1).
//                  
//  Description:    Checks whether the specified CAN flag is set or not.
//
//-------------------------------------------------------------------------------------------------
bool CAN_Driver::GetFlagStatus(uint32_t Flag)
{
    bool BitState = 0;
 
    if((Flag & CAN_FLAGS_ESR) != 0)
    { 
        BitState = ((CANx->ESR & (Flag & 0x000FFFFF)) != 0) ? 1 : 0;
    }
    else if((Flag & CAN_FLAGS_MSR) != 0)
    { 
        BitState = ((CANx->MSR & (Flag & 0x000FFFFF)) != 0) ? 1 : 0;
    }
    else if((Flag & CAN_FLAGS_TSR) != 0)
    {
        BitState = ((CANx->TSR & (Flag & 0x000FFFFF)) != 0) ? 1 : 0;
    }
    else if((Flag & CAN_FLAGS_RF0R) != 0)
    { 
        BitState = ((CANx->RF0R & (Flag & 0x000FFFFF)) != 0) ? 1 : 0;
    }
    else if(Flag & CAN_FLAGS_RF1R != 0)
    { 
        BitState = (uint32_t(CANx->RF1R & (Flag & 0x000FFFFF)) != 0) ? 1 : 0;
    }
    
    return  BitState;       // Return the Flag state
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearFlag
//
//  Parameter(s):   Flag        Specifies the flag to clear.
//                                  CAN_FLAG_RQCP0: Request MailBox0 Flag
//                                  CAN_FLAG_RQCP1: Request MailBox1 Flag
//                                  CAN_FLAG_RQCP2: Request MailBox2 Flag
//                                  CAN_FLAG_FF0:   FIFO 0 Full Flag       
//                                  CAN_FLAG_FOV0:  FIFO 0 Overrun Flag 
//                                  CAN_FLAG_FMP1:  FIFO 1 Message Pending Flag   
//                                  CAN_FLAG_FF1:   FIFO 1 Full Flag        
//                                  CAN_FLAG_FOV1:  FIFO 1 Overrun Flag     
//                                  CAN_FLAG_WKU:   Wake up Flag
//                                  CAN_FLAG_SLAK:  Sleep acknowledge Flag 
//                                  CAN_FLAG_LEC:   Last error code Flag      
//                  
//  Return:         None
//                  
//  Description:    Clears the CAN's pending flags.
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::ClearFlag(uint32_t Flag)
{
    uint32_t FlagToClear = 0;

    if(Flag == CAN_FLAG_LEC)                        // ESR register
    {
        m_pInfo->pCANx->ESR = 0;                    // Clear the selected CAN flags
    }
    else // MSR or TSR or RF0R or RF1R
    {
        FlagToClear = Flag & 0x000FFFFF;

        if((Flag & CAN_FLAGS_RF0R) != 0)
        {
            m_pInfo->pCANx->RF0R = FlagToClear;     // Receive Flags
        }
        else if((Flag & CAN_FLAGS_RF1R) != 0)
        {
            m_pInfo->pCANx->RF1R = FlagToClear;     // Receive Flags
        }
        else if((Flag & CAN_FLAGS_TSR) != 0)
        {
            m_pInfo->pCANx->TSR = FlagToClear;      // Transmit Flags
        }
        else // if((Flag & CAN_FLAGS_MSR) != 0)
        {
            m_pInfo->pCANx->MSR = FlagToClear;      // Operating mode Flags
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetITStatus
//
//  Parameter(s):   CAN_IT: Specifies the CAN interrupt source to check.
//                          This parameter can be one of the following values:
//                              CAN_IT_TME: Transmit mailbox empty Interrupt 
//                              CAN_IT_FMP0: FIFO 0 message pending Interrupt 
//                              CAN_IT_FF0: FIFO 0 full Interrupt
//                              CAN_IT_FOV0: FIFO 0 overrun Interrupt
//                              CAN_IT_FMP1: FIFO 1 message pending Interrupt 
//                              CAN_IT_FF1: FIFO 1 full Interrupt
//                              CAN_IT_FOV1: FIFO 1 overrun Interrupt
//                              CAN_IT_WKU: Wake-up Interrupt
//                              CAN_IT_SLK: Sleep acknowledge Interrupt  
//                              CAN_IT_EWG: Error warning Interrupt
//                              CAN_IT_EPV: Error passive Interrupt
//                              CAN_IT_BOF: Bus-off Interrupt  
//                              CAN_IT_LEC: Last error code Interrupt
//                              CAN_IT_ERR: Error Interrupt      
//                  
//  Return:         The current state of CAN_IT (1 or 0).
//                  
//  Description:    Checks whether the specified CANx interrupt has occurred or not.
//
//-------------------------------------------------------------------------------------------------
bool CAN_Driver::GetITStatus(uint32_t CAN_IT)
{
    bool IT_Status = 0;
  
    if((CANx->IER & CAN_IT) != 0)                                                                   // Check the interrupt enable bit
    {
        switch(CAN_IT)                                                                              // In case the Interrupt is enabled, ....
        {
            case CAN_IT_TME:
                IT_Status = CheckITStatus(CANx->TSR, CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2);// Check CAN_TSR_RQCPx bits
            break;
            
            case CAN_IT_FMP0:
                IT_Status = CheckITStatus(CANx->RF0R, CAN_RF0R_FMP0);                               // Check CAN_RF0R_FMP0 bit
            break;
            
            case CAN_IT_FF0:
                IT_Status = CheckITStatus(CANx->RF0R, CAN_RF0R_FULL0);                              // Check CAN_RF0R_FULL0 bit
            break;
            
            case CAN_IT_FOV0:
                IT_Status = CheckITStatus(CANx->RF0R, CAN_RF0R_FOVR0);                              // Check CAN_RF0R_FOVR0 bit
            break;
            
            case CAN_IT_FMP1:
                IT_Status = CheckITStatus(CANx->RF1R, CAN_RF1R_FMP1);                               // Check CAN_RF1R_FMP1 bit
            break;
            
            case CAN_IT_FF1:
                IT_Status = CheckITStatus(CANx->RF1R, CAN_RF1R_FULL1);                              // Check CAN_RF1R_FULL1 bit
            break;
            
            case CAN_IT_FOV1:
                IT_Status = CheckITStatus(CANx->RF1R, CAN_RF1R_FOVR1);                              // Check CAN_RF1R_FOVR1 bit
            break;
            
            case CAN_IT_WKU:
                IT_Status = CheckITStatus(CANx->MSR, CAN_MSR_WKUI);                                 // Check CAN_MSR_WKUI bit
            break;
            
            case CAN_IT_SLK:
                IT_Status = CheckITStatus(CANx->MSR, CAN_MSR_SLAKI);                                // Check CAN_MSR_SLAKI bit
            break;
            
            case CAN_IT_EWG:
                IT_Status = CheckITStatus(CANx->ESR, CAN_ESR_EWGF);                                 // Check CAN_ESR_EWGF bit
            break;
            
            case CAN_IT_EPV:
                IT_Status = CheckITStatus(CANx->ESR, CAN_ESR_EPVF);                                 // Check CAN_ESR_EPVF bit
            break;
            
            case CAN_IT_BOF:
                IT_Status = CheckITStatus(CANx->ESR, CAN_ESR_BOFF);                                 // Check CAN_ESR_BOFF bit
            break;
            
            case CAN_IT_LEC:
                IT_Status = CheckITStatus(CANx->ESR, CAN_ESR_LEC);                                  // Check CAN_ESR_LEC bit
            break;
            
            case CAN_IT_ERR:
                IT_Status = CheckITStatus(CANx->MSR, CAN_MSR_ERRI);                                 // Check CAN_MSR_ERRI bit 
            break;
            
            default:
                IT_Status = 0;                                                                      // In case of error, return RESET
            break;
        }
    }
  
    // Return the CAN_IT status
    return IT_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearITPendingBit
//
//  Parameter(s):   CAN_IT: Specifies the interrupt pending bit to clear.
//                          This parameter can be one of the following values:
//                              CAN_IT_TME: Transmit mailbox empty Interrupt
//                              CAN_IT_FF0: FIFO 0 full Interrupt
//                              CAN_IT_FOV0: FIFO 0 overrun Interrupt
//                              CAN_IT_FF1: FIFO 1 full Interrupt
//                              CAN_IT_FOV1: FIFO 1 overrun Interrupt
//                              CAN_IT_WKU: Wake-up Interrupt
//                              CAN_IT_SLK: Sleep acknowledge Interrupt  
//                              CAN_IT_EWG: Error warning Interrupt
//                              CAN_IT_EPV: Error passive Interrupt
//                              CAN_IT_BOF: Bus-off Interrupt  
//                              CAN_IT_LEC: Last error code Interrupt
//                              CAN_IT_ERR: Error Interrupt      
//                  
//  Return:         None
//                  
//  Description:    Clears the CANx's interrupt pending bits.
//
//-------------------------------------------------------------------------------------------------
void CAN_Driver::ClearITPendingBit(uint32_t CAN_IT)
{
    switch(CAN_IT)
    {
        case CAN_IT_TME:
        {
            CANx->TSR = CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2;      // Clear CAN_TSR_RQCPx (rc_w1)
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

//-------------------------------------------------------------------------------------------------
//
//  Name:           CheckITStatus
//
//  Parameter(s):   CAN_Reg         Specifies the CAN interrupt register to check.
//                  It_Bit          Specifies the interrupt source bit to check.
//  Return:         The new state of the CAN Interrupt (SET or RESET).
//
//  Description:    Checks whether the CAN interrupt has occurred or not.
//
//-------------------------------------------------------------------------------------------------
/**
  * @param  CAN_Reg: specifies the CAN interrupt register to check.
  * @param  It_Bit: specifies the interrupt source bit to check.
  * @retval The new state of the CAN Interrupt (1 or 0).
  */
bool CAN_Driver::CheckITStatus(uint32_t CAN_Reg, uint32_t It_Bit)
{
    bool PendingBitStatus = 0;
  
    if((CAN_Reg & It_Bit) != 0)
    {
        PendingBitStatus = 1;             // CAN_IT is set
    }
    
    return PendingBitStatus;
}

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
