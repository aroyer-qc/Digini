//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_can.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_CAN_DRIVER == DEF_ENABLED)


// not sure i need this
#define IS_CAN_ALL_PERIPH(PERIPH) (((PERIPH) == CAN1)  || \
                                   ((PERIPH) == CAN2))


// CAN_operating_mode 
#define CAN_Mode_Normal             ((uint8_t)0x00)  // normal mode 
#define CAN_Mode_LoopBack           ((uint8_t)0x01)  // loopback mode 
#define CAN_Mode_Silent             ((uint8_t)0x02)  // silent mode 
#define CAN_Mode_Silent_LoopBack    ((uint8_t)0x03)  // loopback combined with silent mode 

#define IS_CAN_MODE(MODE) (((MODE) == CAN_Mode_Normal)   ||  \
                           ((MODE) == CAN_Mode_LoopBack) ||  \
                           ((MODE) == CAN_Mode_Silent)  ||   \
                           ((MODE) == CAN_Mode_Silent_LoopBack))

// CAN_operating_mode 
#define CAN_OperatingMode_Initialization  ((uint8_t)0x00) // Initialization mode 
#define CAN_OperatingMode_Normal          ((uint8_t)0x01) // Normal mode 
#define CAN_OperatingMode_Sleep           ((uint8_t)0x02) // sleep mode 

#define IS_CAN_OPERATING_MODE(MODE) (((MODE) == CAN_OperatingMode_Initialization) ||\
                                    ((MODE) == CAN_OperatingMode_Normal)|| \
																		((MODE) == CAN_OperatingMode_Sleep))

// CAN_operating_mode_status
#define CAN_ModeStatus_Failed    ((uint8_t)0x00)                     // CAN entering the specific mode failed 
#define CAN_ModeStatus_Success   ((uint8_t)!CAN_ModeStatus_Failed)   // CAN entering the specific mode Succeed 

// CAN_synchronisation_jump_width 
   
#define CAN_SJW_1tq                 ((uint8_t)0x00)  // 1 time quantum 
#define CAN_SJW_2tq                 ((uint8_t)0x01)  // 2 time quantum 
#define CAN_SJW_3tq                 ((uint8_t)0x02)  // 3 time quantum 
#define CAN_SJW_4tq                 ((uint8_t)0x03)  // 4 time quantum 

#define IS_CAN_SJW(SJW) (((SJW) == CAN_SJW_1tq) || ((SJW) == CAN_SJW_2tq)|| \
                         ((SJW) == CAN_SJW_3tq) || ((SJW) == CAN_SJW_4tq))

// CAN_time_quantum_in_bit_segment_1 
#define CAN_BS1_1tq                 ((uint8_t)0x00)  // 1 time quantum 
#define CAN_BS1_2tq                 ((uint8_t)0x01)  // 2 time quantum 
#define CAN_BS1_3tq                 ((uint8_t)0x02)  // 3 time quantum 
#define CAN_BS1_4tq                 ((uint8_t)0x03)  // 4 time quantum 
#define CAN_BS1_5tq                 ((uint8_t)0x04)  // 5 time quantum 
#define CAN_BS1_6tq                 ((uint8_t)0x05)  // 6 time quantum 
#define CAN_BS1_7tq                 ((uint8_t)0x06)  // 7 time quantum 
#define CAN_BS1_8tq                 ((uint8_t)0x07)  // 8 time quantum 
#define CAN_BS1_9tq                 ((uint8_t)0x08)  // 9 time quantum 
#define CAN_BS1_10tq                ((uint8_t)0x09)  // 10 time quantum 
#define CAN_BS1_11tq                ((uint8_t)0x0A)  // 11 time quantum 
#define CAN_BS1_12tq                ((uint8_t)0x0B)  // 12 time quantum 
#define CAN_BS1_13tq                ((uint8_t)0x0C)  // 13 time quantum 
#define CAN_BS1_14tq                ((uint8_t)0x0D)  // 14 time quantum 
#define CAN_BS1_15tq                ((uint8_t)0x0E)  // 15 time quantum 
#define CAN_BS1_16tq                ((uint8_t)0x0F)  // 16 time quantum 

#define IS_CAN_BS1(BS1) ((BS1) <= CAN_BS1_16tq)

// CAN_time_quantum_in_bit_segment_2
  
#define CAN_BS2_1tq                 ((uint8_t)0x00)  // 1 time quantum 
#define CAN_BS2_2tq                 ((uint8_t)0x01)  // 2 time quantum 
#define CAN_BS2_3tq                 ((uint8_t)0x02)  // 3 time quantum 
#define CAN_BS2_4tq                 ((uint8_t)0x03)  // 4 time quantum 
#define CAN_BS2_5tq                 ((uint8_t)0x04)  // 5 time quantum 
#define CAN_BS2_6tq                 ((uint8_t)0x05)  // 6 time quantum 
#define CAN_BS2_7tq                 ((uint8_t)0x06)  // 7 time quantum 
#define CAN_BS2_8tq                 ((uint8_t)0x07)  // 8 time quantum 

#define IS_CAN_BS2(BS2) ((BS2) <= CAN_BS2_8tq)

// CAN_clock_prescaler 
#define IS_CAN_PRESCALER(PRESCALER) (((PRESCALER) >= 1) && ((PRESCALER) <= 1024))

// CAN_filter_number 
#define IS_CAN_FILTER_NUMBER(NUMBER) ((NUMBER) <= 27)

// CAN_filter_mode 
#define CAN_FilterMode_ID_Mask       ((uint8_t)0x00)  // identifier/mask mode 
#define CAN_FilterMode_ID_List       ((uint8_t)0x01)  // identifier list mode 

#define IS_CAN_FILTER_MODE(MODE) (((MODE) == CAN_FilterMode_ID_Mask) || \
                                  ((MODE) == CAN_FilterMode_ID_List))

// CAN_filter_scale 
#define CAN_FilterScale_16bit       ((uint8_t)0x00) // Two 16-bit filters 
#define CAN_FilterScale_32bit       ((uint8_t)0x01) // One 32-bit filter 

#define IS_CAN_FILTER_SCALE(SCALE) (((SCALE) == CAN_FilterScale_16bit) || \
                                    ((SCALE) == CAN_FilterScale_32bit))

// CAN_filter_FIFO
#define CAN_Filter_FIFO0             ((uint8_t)0x00)  // Filter FIFO 0 assignment for filter x 
#define CAN_Filter_FIFO1             ((uint8_t)0x01)  // Filter FIFO 1 assignment for filter x 
#define IS_CAN_FILTER_FIFO(FIFO) (((FIFO) == CAN_FilterFIFO0) || \
                                  ((FIFO) == CAN_FilterFIFO1))

// Legacy defines 
#define CAN_FilterFIFO0  CAN_Filter_FIFO0
#define CAN_FilterFIFO1  CAN_Filter_FIFO1

// CAN_Start_bank_filter_for_slave_CAN 
#define IS_CAN_BANKNUMBER(BANKNUMBER) (((BANKNUMBER) >= 1) && ((BANKNUMBER) <= 27))

// CAN_Tx 
#define IS_CAN_TRANSMITMAILBOX(TRANSMITMAILBOX) ((TRANSMITMAILBOX) <= ((uint8_t)0x02))
#define IS_CAN_STDID(STDID)   ((STDID) <= ((uint32_t)0x7FF))
#define IS_CAN_EXTID(EXTID)   ((EXTID) <= ((uint32_t)0x1FFFFFFF))
#define IS_CAN_DLC(DLC)       ((DLC) <= ((uint8_t)0x08))

// CAN_identifier_type 
#define CAN_ID_Standard             ((uint32_t)0x00000000)  // Standard ID 
#define CAN_ID_Extended             ((uint32_t)0x00000004)  // Extended ID 
#define IS_CAN_IDTYPE(IDTYPE) (((IDTYPE) == CAN_ID_Standard) || \
                               ((IDTYPE) == CAN_ID_Extended))

// Legacy defines 
#define CAN_ID_STD      CAN_ID_Standard           
#define CAN_ID_EXT      CAN_ID_Extended

// CAN_remote_transmission_request 
#define CAN_RTR_Data                ((uint32_t)0x00000000)  // Data frame 
#define CAN_RTR_Remote              ((uint32_t)0x00000002)  // Remote frame 
#define IS_CAN_RTR(RTR) (((RTR) == CAN_RTR_Data) || ((RTR) == CAN_RTR_Remote))

// Legacy defines 
#define CAN_RTR_DATA     CAN_RTR_Data         
#define CAN_RTR_REMOTE   CAN_RTR_Remote

// CAN_transmit_constants 
#define CAN_TxStatus_Failed         ((uint8_t)0x00)// CAN transmission failed 
#define CAN_TxStatus_Ok             ((uint8_t)0x01) // CAN transmission succeeded 
#define CAN_TxStatus_Pending        ((uint8_t)0x02) // CAN transmission pending 
#define CAN_TxStatus_NoMailBox      ((uint8_t)0x04) // CAN cell did not provide an empty mailbox 
// Legacy defines 	
#define CANTXFAILED                  CAN_TxStatus_Failed
#define CANTXOK                      CAN_TxStatus_Ok
#define CANTXPENDING                 CAN_TxStatus_Pending
#define CAN_NO_MB                    CAN_TxStatus_NoMailBox

// CAN_receive_FIFO_number_constants 
#define CAN_FIFO0                 ((uint8_t)0x00) // CAN FIFO 0 used to receive 
#define CAN_FIFO1                 ((uint8_t)0x01) // CAN FIFO 1 used to receive 

#define IS_CAN_FIFO(FIFO) (((FIFO) == CAN_FIFO0) || ((FIFO) == CAN_FIFO1))

// CAN_sleep_constants 
#define CAN_Sleep_Failed     ((uint8_t)0x00) // CAN did not enter the sleep mode 
#define CAN_Sleep_Ok         ((uint8_t)0x01) // CAN entered the sleep mode 

// Legacy defines 	
#define CANSLEEPFAILED   CAN_Sleep_Failed
#define CANSLEEPOK       CAN_Sleep_Ok

// CAN_wake_up_constants 
#define CAN_WakeUp_Failed        ((uint8_t)0x00) // CAN did not leave the sleep mode 
#define CAN_WakeUp_Ok            ((uint8_t)0x01) // CAN leaved the sleep mode 

// Legacy defines 
#define CANWAKEUPFAILED   CAN_WakeUp_Failed        
#define CANWAKEUPOK       CAN_WakeUp_Ok        

// CAN_Error_Code_constants
#define CAN_ErrorCode_NoErr           ((uint8_t)0x00) // No Error  
#define	CAN_ErrorCode_StuffErr        ((uint8_t)0x10) // Stuff Error  
#define	CAN_ErrorCode_FormErr         ((uint8_t)0x20) // Form Error  
#define	CAN_ErrorCode_ACKErr          ((uint8_t)0x30) // Acknowledgment Error  
#define	CAN_ErrorCode_BitRecessiveErr ((uint8_t)0x40) // Bit Recessive Error  
#define	CAN_ErrorCode_BitDominantErr  ((uint8_t)0x50) // Bit Dominant Error  
#define	CAN_ErrorCode_CRCErr          ((uint8_t)0x60) // CRC Error   
#define	CAN_ErrorCode_SoftwareSetErr  ((uint8_t)0x70) // Software Set Error  

// CAN_flags 

/// If the flag is 0x3XXXXXXX, it means that it can be used with CAN_GetFlagStatus() and CAN_ClearFlag() functions. 
/// If the flag is 0x1XXXXXXX, it means that it can only be used with CAN_GetFlagStatus() function.  

// Transmit Flags 
#define CAN_FLAG_RQCP0             ((uint32_t)0x38000001) // Request MailBox0 Flag 
#define CAN_FLAG_RQCP1             ((uint32_t)0x38000100) // Request MailBox1 Flag 
#define CAN_FLAG_RQCP2             ((uint32_t)0x38010000) // Request MailBox2 Flag 

// Receive Flags 
#define CAN_FLAG_FMP0              ((uint32_t)0x12000003) // FIFO 0 Message Pending Flag 
#define CAN_FLAG_FF0               ((uint32_t)0x32000008) // FIFO 0 Full Flag            
#define CAN_FLAG_FOV0              ((uint32_t)0x32000010) // FIFO 0 Overrun Flag         
#define CAN_FLAG_FMP1              ((uint32_t)0x14000003) // FIFO 1 Message Pending Flag 
#define CAN_FLAG_FF1               ((uint32_t)0x34000008) // FIFO 1 Full Flag            
#define CAN_FLAG_FOV1              ((uint32_t)0x34000010) // FIFO 1 Overrun Flag         

// Operating Mode Flags 
#define CAN_FLAG_WKU               ((uint32_t)0x31000008) // Wake up Flag 
#define CAN_FLAG_SLAK              ((uint32_t)0x31000012) // Sleep acknowledge Flag 
//note When SLAK interrupt is disabled (SLKIE=0), no polling on SLAKI is possible. In this case the SLAK bit can be polled.

// Error Flags 
#define CAN_FLAG_EWG               ((uint32_t)0x10F00001) // Error Warning Flag   
#define CAN_FLAG_EPV               ((uint32_t)0x10F00002) // Error Passive Flag   
#define CAN_FLAG_BOF               ((uint32_t)0x10F00004) // Bus-Off Flag         
#define CAN_FLAG_LEC               ((uint32_t)0x30F00070) // Last error code Flag 

#define IS_CAN_GET_FLAG(FLAG) (((FLAG) == CAN_FLAG_LEC)  || ((FLAG) == CAN_FLAG_BOF)   || \
                               ((FLAG) == CAN_FLAG_EPV)  || ((FLAG) == CAN_FLAG_EWG)   || \
                               ((FLAG) == CAN_FLAG_WKU)  || ((FLAG) == CAN_FLAG_FOV0)  || \
                               ((FLAG) == CAN_FLAG_FF0)  || ((FLAG) == CAN_FLAG_FMP0)  || \
                               ((FLAG) == CAN_FLAG_FOV1) || ((FLAG) == CAN_FLAG_FF1)   || \
                               ((FLAG) == CAN_FLAG_FMP1) || ((FLAG) == CAN_FLAG_RQCP2) || \
                               ((FLAG) == CAN_FLAG_RQCP1)|| ((FLAG) == CAN_FLAG_RQCP0) || \
                               ((FLAG) == CAN_FLAG_SLAK ))

#define IS_CAN_CLEAR_FLAG(FLAG)(((FLAG) == CAN_FLAG_LEC) || ((FLAG) == CAN_FLAG_RQCP2) || \
                                ((FLAG) == CAN_FLAG_RQCP1)  || ((FLAG) == CAN_FLAG_RQCP0) || \
                                ((FLAG) == CAN_FLAG_FF0)  || ((FLAG) == CAN_FLAG_FOV0) ||\
                                ((FLAG) == CAN_FLAG_FF1) || ((FLAG) == CAN_FLAG_FOV1) || \
                                ((FLAG) == CAN_FLAG_WKU) || ((FLAG) == CAN_FLAG_SLAK))
  
// CAN_interrupts 
#define CAN_IT_TME                  ((uint32_t)0x00000001) // Transmit mailbox empty Interrupt

// Receive Interrupts 
#define CAN_IT_FMP0                 ((uint32_t)0x00000002) // FIFO 0 message pending Interrupt
#define CAN_IT_FF0                  ((uint32_t)0x00000004) // FIFO 0 full Interrupt
#define CAN_IT_FOV0                 ((uint32_t)0x00000008) // FIFO 0 overrun Interrupt
#define CAN_IT_FMP1                 ((uint32_t)0x00000010) // FIFO 1 message pending Interrupt
#define CAN_IT_FF1                  ((uint32_t)0x00000020) // FIFO 1 full Interrupt
#define CAN_IT_FOV1                 ((uint32_t)0x00000040) // FIFO 1 overrun Interrupt

// Operating Mode Interrupts 
#define CAN_IT_WKU                  ((uint32_t)0x00010000) // Wake-up Interrupt
#define CAN_IT_SLK                  ((uint32_t)0x00020000) // Sleep acknowledge Interrupt

// Error Interrupts 
#define CAN_IT_EWG                  ((uint32_t)0x00000100) // Error warning Interrupt
#define CAN_IT_EPV                  ((uint32_t)0x00000200) // Error passive Interrupt
#define CAN_IT_BOF                  ((uint32_t)0x00000400) // Bus-off Interrupt
#define CAN_IT_LEC                  ((uint32_t)0x00000800) // Last error code Interrupt
#define CAN_IT_ERR                  ((uint32_t)0x00008000) // Error Interrupt

// Flags named as Interrupts : kept only for FW compatibility 
#define CAN_IT_RQCP0   CAN_IT_TME
#define CAN_IT_RQCP1   CAN_IT_TME
#define CAN_IT_RQCP2   CAN_IT_TME


#define IS_CAN_IT(IT)        (((IT) == CAN_IT_TME) || ((IT) == CAN_IT_FMP0)  ||\
                             ((IT) == CAN_IT_FF0)  || ((IT) == CAN_IT_FOV0)  ||\
                             ((IT) == CAN_IT_FMP1) || ((IT) == CAN_IT_FF1)   ||\
                             ((IT) == CAN_IT_FOV1) || ((IT) == CAN_IT_EWG)   ||\
                             ((IT) == CAN_IT_EPV)  || ((IT) == CAN_IT_BOF)   ||\
                             ((IT) == CAN_IT_LEC)  || ((IT) == CAN_IT_ERR)   ||\
                             ((IT) == CAN_IT_WKU)  || ((IT) == CAN_IT_SLK))

#define IS_CAN_CLEAR_IT(IT) (((IT) == CAN_IT_TME) || ((IT) == CAN_IT_FF0)    ||\
                             ((IT) == CAN_IT_FOV0)|| ((IT) == CAN_IT_FF1)    ||\
                             ((IT) == CAN_IT_FOV1)|| ((IT) == CAN_IT_EWG)    ||\
                             ((IT) == CAN_IT_EPV) || ((IT) == CAN_IT_BOF)    ||\
                             ((IT) == CAN_IT_LEC) || ((IT) == CAN_IT_ERR)    ||\
                             ((IT) == CAN_IT_WKU) || ((IT) == CAN_IT_SLK))

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum CAN_ID_e
{
    #if (CAN_DRIVER_SUPPORT_CAN1_CFG == DEF_ENABLED)
        DRIVER_CAN1_ID,
    #endif

    #if (CAN_DRIVER_SUPPORT_CAN2_CFG == DEF_ENABLED)
        DRIVER_CAN2_ID,
    #endif

    #if (CAN_DRIVER_SUPPORT_CAN3_CFG == DEF_ENABLED)
        DRIVER_CAN3_ID,
    #endif

    #if (CAN_DRIVER_SUPPORT_CAN4_CFG == DEF_ENABLED)
        DRIVER_CAN4_ID,
    #endif

    NB_OF_CAN_DRIVER,
};

struct CAN_Info_t
{
    I2C_ID_e            CAN_ID;
    I2C_TypeDef*        pCANx;
    IO_ID_e             CANL;
    IO_ID_e             CANH;
    uint32_t            RCC_APB1_En;
    //uint32_t            Speed;
    //uint8_t             PreempPrio;
    IRQn_Type           CANx_TX_IRQn;      
    IRQn_Type           CANx_RX0_IRQn;     
    IRQn_Type           CANx_RX1_IRQn;     
    IRQn_Type           CANx_SCE_IRQn;
};

// CAN init structure definition
struct CAN_Info
{
    uint16_t        CAN_Prescaler;   // Specifies the length of a time quantum.               It ranges from 1 to 1024. 
    uint8_t         CAN_Mode;         // Specifies the CAN operating mode.                                 This Parameter can be a value of @ref CAN_operating_mode 
    uint8_t         CAN_SJW;          // Specifies the maximum number of time quanta                                  the CAN hardware is allowed to lengthen or                                  shorten a bit to perform resynchronization.                                 This Parameter can be a value of @ref CAN_synchronisation_jump_width 
    uint8_t         CAN_BS1;          // Specifies the number of time quanta in Bit                                  Segment 1. This Parameter can be a value of                                  @ref CAN_time_quantum_in_bit_segment_1 
    uint8_t         CAN_BS2;          // Specifies the number of time quanta in Bit Segment 2.                                 This Parameter can be a value of @ref CAN_time_quantum_in_bit_segment_2 
    FunctionalState CAN_TTCM; // Enable or disable the time triggered communication mode.                                This Parameter can be set either to ENABLE or DISABLE. 
    FunctionalState CAN_ABOM;  // Enable or disable the automatic bus-off management.                                  This Parameter can be set either to ENABLE or DISABLE. 
    FunctionalState CAN_AWUM;  // Enable or disable the automatic wake-up mode.                                   This Parameter can be set either to ENABLE or DISABLE. 
    FunctionalState CAN_NART;  // Enable or disable the non-automatic retransmission mode.                                  This Parameter can be set either to ENABLE or DISABLE. 
    FunctionalState CAN_RFLM;  // Enable or disable the Receive FIFO Locked mode.                                  This Parameter can be set either to ENABLE or DISABLE. 
    FunctionalState CAN_TXFP;  // Enable or disable the transmit FIFO priority.                                  This Parameter can be set either to ENABLE or DISABLE. 
};

// CAN filter init structure definition
struct CAN_FilterInfo_t
{
  uint16_t CAN_FilterIDHigh;         // Specifies the filter identification number (MSBs for a 32-bit                                              configuration, first one for a 16-bit configuration).                                              This Parameter can be a value between 0x0000 and 0xFFFF   uint16_t CAN_FilterIDLow;          // Specifies the filter identification number (LSBs for a 32-bit                                              configuration, second one for a 16-bit configuration).                                              This Parameter can be a value between 0x0000 and 0xFFFF 
  uint16_t CAN_FilterMask_ID_High;     // Specifies the filter mask number or identification number,                                              according to the mode (MSBs for a 32-bit configuration,                                              first one for a 16-bit configuration).                                              This Parameter can be a value between 0x0000 and 0xFFFF 
  uint16_t CAN_FilterMask_ID_Low;      // Specifies the filter mask number or identification number,                                              according to the mode (LSBs for a 32-bit configuration,                                              second one for a 16-bit configuration).                                              This Parameter can be a value between 0x0000 and 0xFFFF 
  uint16_t CAN_FilterFIFOAssignment; // Specifies the FIFO (0 or 1) which will be assigned to the filter.                                              This Parameter can be a value of @ref CAN_filter_FIFO 
  uint8_t CAN_FilterNumber;          // Specifies the filter which will be initialized. It ranges from 0 to 13. 
  uint8_t CAN_FilterMode;            // Specifies the filter mode to be initialized.                                              This Parameter can be a value of @ref CAN_filter_mode 
  uint8_t CAN_FilterScale;           // Specifies the filter scale.                                              This Parameter can be a value of @ref CAN_filter_scale 
  FunctionalState CAN_FilterActivation; // Enable or disable the filter.                                              This Parameter can be set either to ENABLE or DISABLE. 
};



// CAN Tx message structure definition  
struct CAN_TX_Message_t
{
  uint32_t StdID;  // Specifies the standard identifier.                        This Parameter can be a value between 0 to 0x7FF. 
  uint32_t ExtID;  // Specifies the extended identifier.                        This Parameter can be a value between 0 to 0x1FFFFFFF. 
  uint8_t  IDE;     // Specifies the type of identifier for the message that                         will be transmitted. This Parameter can be a value                         of @ref CAN_identifier_type 
  uint8_t  RTR;     // Specifies the type of frame for the message that will                        be transmitted. This Parameter can be a value of                         @ref CAN_remote_transmission_request 
  uint8_t  DLC;     // Specifies the length of the frame that will be                         transmitted. This Parameter can be a value between                         0 to 8 
  uint8_t  Data[8]; // Contains the data to be transmitted. It ranges from 0                         to 0xFF. 
};

// CAN Rx message structure definition  
  
struct CAN_RX_Message_t
{
  uint32_t StdID;   // Specifies the standard identifier.                        This Parameter can be a value between 0 to 0x7FF. 
  uint32_t ExtID;   // Specifies the extended identifier.                        This Parameter can be a value between 0 to 0x1FFFFFFF. 
  uint8_t  IDE;     // Specifies the type of identifier for the message that                         will be received. This Parameter can be a value of                         @ref CAN_identifier_type 
  uint8_t  RTR;     // Specifies the type of frame for the received message.                        This Parameter can be a value of                         @ref CAN_remote_transmission_request 
  uint8_t  DLC;     // Specifies the length of the frame that will be received.                        This Parameter can be a value between 0 to 8 
  uint8_t  Data[8]; // Contains the data to be received. It ranges from 0 to                         0xFF. 
  uint8_t  FMI;     // Specifies the index of the filter the message stored in                         the mailbox passes through. This Parameter can be a                         value between 0 to 0xFF 
};


//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class CAN_Driver
{
    public:

                        CAN_Driver                  (CAN_ID_e CAN_ID);

        SystemState_e   LockToDevice                (uint8_t Device);       // Set I2C to this device and lock
        SystemState_e   UnlockFromDevice            (uint8_t Device);       // Unlock I2C from device
        SystemState_e   GetStatus                   (void);

        uint8_t         Initialize                  (CAN_Info* CAN_InitStruct);
//        void            Initialize                (void);
        void            xx_IRQHandler               (void);
        void            yy_IRQHandler               (void);


        // Initialization and Configuration functions ******************************** 
        void            FilterInitInitialize        (CAN_FilterInfo_t* CAN_FilterInitStruct);
        void            StructInit                  (CAN_Info* CAN_InitStruct);
        void            SlaveStartBank              (uint8_t CAN_BankNumber); 
        void            DBGFreeze                   (FunctionalState NewState);
        void            TTComModeCmd                (FunctionalState NewState);

        // CAN Frames Transmission functions *****************************************
        uint8_t         Transmit                    (CAN_TX_Message_t* TxMessage);
        uint8_t         TransmitStatus              (uint8_t TransmitMailbox);
        void            CancelTransmit              (uint8_t Mailbox);

        // CAN Frames Reception functions ********************************************
        void            Receive                     (uint8_t FIFONumber, CAN_RX_Message_t* RxMessage);
        void            FIFORelease                 (uint8_t FIFONumber);
        uint8_t         MessagePending              (uint8_t FIFONumber);

        // Operation modes functions *************************************************
        uint8_t         OperatingModeRequest        (uint8_t CAN_OperatingMode);
        uint8_t         Sleep                       (void);
        uint8_t         WakeUp                      (void);

        // CAN Bus Error management functions ****************************************
        uint8_t         GetLastErrorCode            (void);
        uint8_t         GetReceiveErrorCounter      (void);
        uint8_t         GetLSBTransmitErrorCounter  (void);

        // Interrupts and flags management functions *********************************
        void            ITConfig                    (uint32_t CAN_IT, FunctionalState NewState);
        FlagStatus      GetFlagStatus               (uint32_t CAN_FLAG);
        void            ClearFlag                   (uint32_t CAN_FLAG);
        ITStatus        GetITStatus                 (uint32_t CAN_IT);
        void            ClearITPendingBit           (uint32_t CAN_IT);













    private:

        void            Lock                (void);
        void            Unlock              (void);

        CAN_Info_t*                         m_pInfo;
        nOS_Mutex                           m_Mutex;

        volatile bool                       m_IsItInitialize;

        volatile SystemState_e              m_State;
        volatile uint8_t                    m_Timeout;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "can_var.h"

//-------------------------------------------------------------------------------------------------

#endif // (USE_CAN_DRIVER == DEF_ENABLED)
