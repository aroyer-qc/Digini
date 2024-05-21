//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_sdio.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_SDIO_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SD_CARD_USE_POWER_CONTROL       DEF_DISABLED                    // No Power pin control
#define SD_CARD_USE_DETECT_SIGNAL       DEF_DISABLED                    // We are using a form of detect signal     TODO TODO TODO   move to config

// ----- MMC/SDC command -----
#define ACMD                            0x80
#define CMD0                            (0)                             // GO_IDLE_STATE
#define CMD1                            (1)                             // SEND_OP_COND (MMC)
#define CMD2                            (2)                             // ALL_SEND_CID
#define CMD3                            (3)                             // SEND_RELATIVE_ADDR
#define CMD6                            (6)                             // SET_BUS_WIDTH (SDC)
#define CMD7                            (7)                             // SELECT_CARD
#define CMD8                            (8)                             // SEND_IF_COND
#define CMD9                            (9)                             // SEND_CSD
#define CMD10                           (10)                            // SEND_CID
#define CMD12                           (12)                            // STOP_TRANSMISSION
#define CMD13                           (13)                            // SEND_STATUS
// #define ACMD13                       (55)(13)                        // SD_STATUS (SDC)
#define CMD16                           (16)                            // SET_BLOCKLEN
#define CMD17                           (17)                            // READ_SINGLE_BLOCK
#define CMD18                           (18)                            // READ_MULTIPLE_BLOCK
#define CMD23                           (23)                            // SET_BLK_COUNT (MMC)
//#define ACMD23                        (55)(23)                        // SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24                           (24)                            // WRITE_BLOCK
#define CMD25                           (25)                            // WRITE_MULTIPLE_BLOCK
#define CMD32                           (32)                            // ERASE_ER_BLK_START
#define CMD33                           (33)                            // ERASE_ER_BLK_END
#define CMD38                           (38)                            // ERASE
#define CMD41                           (41)                            // SEND_OP_COND (SDC)
//#define ACMD41                        (55)(41)                        // SEND_OP_COND (SDC)
#define CMD51                           (51)                            // SEND_SCR
//#define ACMD51                        (55)(51)                        // SEND_SCR
#define CMD55                           (55)                            // APP_CMD


#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRITE        ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)


#define SD_SHORT_RESPONSE               SDMMC_CMD_WAITRESP_0
#define SD_LONG_RESPONSE                SDMMC_CMD_WAITRESP


#define SD_BUS_WIDE_1B                  ((uint32_t)0x00000000)
#define SD_BUS_WIDE_4B                  SDMMC_CLKCR_WIDBUS_0
#define SD_BUS_WIDE_8B                  SDMMC_CLKCR_WIDBUS_1



#define SD_DETECT_GPIO_PORT             GPIOC
#define SD_DETECT_GPIO_CLOCK            RCC_AHB1ENR_GPIOCEN
#define SD_DETECT_PIN                     GPIO_PIN_MASK_13

// SCR register
#define SD_VALUE_AFTER_ERASE            ((uint32_t)0x00800000)
#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00000400)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00000100)

// Misc
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)
#define SD_VOLTAGE_WINDOW               ((uint32_t)0x80100000)
#define SD_RESP_HIGH_CAPACITY           ((uint32_t)0x40000000)
#define SD_RESP_STD_CAPACITY            ((uint32_t)0x00000000)
#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)


#define SD_DATABLOCK_SIZE_8B            (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_1)
#define SD_DATABLOCK_SIZE_64B           (SDMMC_DCTRL_DBLOCKSIZE_1|SDMMC_DCTRL_DBLOCKSIZE_2)
#define SD_DATABLOCK_SIZE_512B          (SDMMC_DCTRL_DBLOCKSIZE_0|SDMMC_DCTRL_DBLOCKSIZE_3)

#define BLOCK_SIZE                      512         // represent 512KB

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

// FATFS size structure
struct FatFS_Size_t
{
    uint32_t Total; // Total size of memory
    uint32_t Free;  // Free size of memory
};

// TODO TODO part of those struct are not use if SDIO_USE_MAXIMUM_INFORMATION == disabled... fix this


struct SD_CSD_t
{
    uint32_t DeviceSize;                // Device Size
    uint8_t  DeviceSizeMul;             // Device size multiplier
  #if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)
    uint8_t  CSDStruct;                 // CSD structure
    uint8_t  SysSpecVersion;            // System specification version
  //uint8_t  TAAC;                      // Data read access time 1
  //uint8_t  NSAC;                      // Data read access time 2 in CLK cycles
    uint8_t  MaxBusClkFrec;             // Max. bus clock frequency
  //uint16_t CardComdClasses;           // Card command classes
    uint8_t  RdBlockLen;                // Max. read data block length
    uint8_t  PartBlockRead;             // Partial blocks for read allowed
    uint8_t  WrBlockMisalign;           // Write block misalignment
    uint8_t  RdBlockMisalign;           // Read block misalignment
    uint8_t  DSRImpl;                   // DSR implemented
  //uint8_t  MaxRdCurrentVDDMin;        // Max. read current @ VDD min
  //uint8_t  MaxRdCurrentVDDMax;        // Max. read current @ VDD max
  //uint8_t  MaxWrCurrentVDDMin;        // Max. write current @ VDD min
  //uint8_t  MaxWrCurrentVDDMax;        // Max. write current @ VDD max
    uint8_t  EraseGrSize;               // Erase group size
    uint8_t  EraseGrMul;                // Erase group size multiplier
    uint8_t  WrProtectGrSize;           // Write protect group size
    uint8_t  WrProtectGrEnable;         // Write protect group enable
    uint8_t  ManDeflECC;                // Manufacturer default ECC
  //uint8_t  WrSpeedFact;               // Write speed factor
    uint8_t  MaxWrBlockLen;             // Max. write data block length
    uint8_t  WriteBlockPaPartial;       // Partial blocks for write allowed
    uint8_t  ContentProtectAppli;       // Content protection application
    uint8_t  FileFormatGroup;           // File format group
    uint8_t  CopyFlag;                  // Copy flag (OTP)
    uint8_t  PermWrProtect;             // Permanent write protection
    uint8_t  TempWrProtect;             // Temporary write protection
    uint8_t  FileFormat;                // File format
    uint8_t  ECC;                       // ECC code
    uint8_t  _CRC;                      // CSD CRC
  #endif
};

#if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)
struct SD_CID_t
{
    uint8_t  ManufacturerID;            // Manufacturer ID
    uint16_t OEM_AppliID;               // OEM/Application ID
    char     ProductName[6];            // Product Name
    uint8_t  ProductRev;                // Product Revision
    uint32_t ProductSN;                 // Product Serial Number
    uint16_t ManufacturingDate;         // Manufacturing Date
    uint8_t  _CRC;                      // CID CRC
};
#endif

struct SD_SCR_t
{
    uint32_t Array[2];
};

enum SD_ResponseType_e
{
    R0_RESPONSE_ARG0,
    R0_RESPONSE,
    R1_RESPONSE,
    R1_RESPONSE_B,
    R2_RESPONSE,
    R3_RESPONSE,
    R4_RESPONSE,
    R5_RESPONSE,
    R6_RESPONSE,
    R7_RESPONSE,
};

 enum SD_StatusFlag_e
{
    SD_FLAG_COMMAND_CRC_FAIL = 0,
    SD_FLAG_DATA_CRC_FAIL,
    SD_FLAG_COMMAND_TIME_OUT,
    SD_FLAG_DATA_TIME_OUT,
    SD_FLAG_TX_UNDERRUN,
    SD_FLAG_RX_OVERRUN,
    SD_FLAG_COMMAND_RESPONSE_END,
    SD_FLAG_COMMAND_SENT,
    SD_FLAG_DATA_END,
    SD_FLAG_START_BIT_ERROR,
    SD_FLAG_DATA_BLOCK_END,
    SD_FLAG_COMMAND_ACTIVE,
};

enum SD_TransferType_e
{
    SD_CARD_TRANSFER_TO_CARD,
    SD_CARD_TRANSFER_FROM_CARD
};

enum SD_CardType_e
{
    SD_STD_CAPACITY_V1_1       = 0,
    SD_STD_CAPACITY_V2_0       = 1,
    SD_HIGH_CAPACITY           = 2,
    SD_MULTIMEDIA              = 3,
    SD_SECURE_DIGITAL_IO       = 4,
    SD_HIGH_SPEED_MULTIMEDIA   = 5,
    SD_SECURE_DIGITAL_IO_COMBO = 6,
    SD_HIGH_CAPACITY_MMC       = 7,
};

enum SD_Operation_e
{
    SD_SINGLE_BLOCK    = 0,       // Single block operation
    SD_MULTIPLE_BLOCK  = 1,       // Multiple blocks operation
};

#if (SD_CARD_USE_POWER_CONTROL == DEF_ENABLED)
struct SD_CardIO_t
{
  #if (SD_CARD_USE_DETECT_SIGNAL == DEF_ENABLED)
    GPIO_TypeDef*         pDetectPort;
    uint16_t              DetectPin;
    uint8_t               DetectPinSource;
    uint32_t              DetectClock;
  #endif
  #if (SD_CARD_USE_POWER_CONTROL == DEF_ENABLED)
    IO_Output_t*           pPower;
  #endif
};
#endif


struct SDIO_Info_t
{
    IO_ID_e             Pin_D0;                 // SDIO Data 0 to 3
    IO_ID_e             Pin_D1;
    IO_ID_e             Pin_D2;
    IO_ID_e             Pin_D3;
    IO_ID_e             Pin_CLK;                // SDIO CLK
    IO_ID_e             Pin_CMD;				// Command line to SDIO
    IO_ID_e             Pin_Detect;             // Card detect line
    DMA_Info_t          DMA_RX;
    DMA_Info_t          DMA_TX;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SDIO_Driver
{
    public:
                            SDIO_Driver             (SDIO_Info_t* pInfo);

        void                Initialize              (void);

        SystemState_e       CheckOperation          (uint32_t Flag);
        SystemState_e       InitializeCard          (void);
        SystemState_e       IsDetected              (void);
        void                PowerOFF                (void);
        SystemState_e       PowerON                 (void);
        SystemState_e       SelectTheCard           (void);
        SystemState_e       SetBusWidth             (uint32_t BusSize);
        void                StartBlockTransfert     (DMA_Stream_TypeDef* pDMA, uint32_t* pBuffer, uint32_t BlockSize, uint32_t NumberOfBlocks);
        SystemState_e       TransmitCommand         (uint8_t Command, uint32_t Argument, int32_t Response);

        SystemState_e       ReadBlocks              (uint64_t ReadAddress, uint32_t BlockSize, uint32_t NumberOfBlocks);
        SystemState_e       WriteBlocks             (uint64_t WriteAddress, uint32_t BlockSize, uint32_t NumberOfBlocks);

        // Getter
        SystemState_e       GetCardInfo             (void);
        SystemState_e       GetStatus               (void);
        uint32_t            GetCardCapacity         (void)              { return m_CardCapacity; }
        SD_CardType_e       GetCardType             (void)              { return m_CardType;     }
        uint32_t            GetCard_OCR             (void)              { return m_OCR;          }
        const SD_SCR_t*     GetCard_SCR             (void)              { return &m_SCR;         }
        const SD_CSD_t*     GetCard_CSD             (void)              { return &m_CSD;         }
      #if (SDIO_USE_MAXIMUM_INFORMATION == DEF_ENABLED)
        const SD_CID_t*     GetCard_CID             (void)              { return &m_CID;         }
      #endif



        // IRQ Handler
        void                SDMMC1_IRQHandler       (void);
        void                DMA_Stream3IRQHandler   (void);
        void                DMA_Stream6IRQHandler   (void);
       #if (SD_CARD_USE_DETECT_SIGNAL == DEF_ENABLED)
        void                CardDetectIRQ_Handler   (void);
       #endif


    private:

        SystemState_e       FindSCR                 (void);


        void                DataInit                (uint32_t Size, uint32_t DataBlockSize, bool IsItReadFromCard);
        SystemState_e       CmdResponse             (uint8_t Command, int32_t ResponseType);
        SystemState_e       GetResponse             (uint32_t* pResponse);
        SystemState_e       CheckOCR_Response       (uint32_t Response_R1);


        void                DMA_Complete            (DMA_Stream_TypeDef* pDMA_Stream);

       #if (SD_CARD_USE_POWER_CONTROL == DEF_ENABLED)
        void                Power                   (Power_e Pwr)           { IO_Output(pPower, (Pwr == POWER_ON) ? IO_SET : IO_RESET); }
       #else
        void                Power                   (Power_e Pwr)           { VAR_UNUSED(Pwr); }
       #endif

        void                Lock                    (void);
        void                Unlock                  (void);

        bool                    m_IsItInitialize;

        nOS_Mutex               m_Mutex;
        uint8_t                 m_LastCommand;
        volatile SystemState_e  m_TransferError;
        volatile int            m_TransferComplete;
        volatile int            m_DMA_XferComplete;
        volatile SD_Operation_e m_Operation;            // SD transfer operation (read/write)

        SDIO_Info_t*            m_pInfo;
        DMA_Driver              m_DMA_RX;
        DMA_Driver              m_DMA_TX;


       #if (SD_CARD_USE_DETECT_SIGNAL == DEF_ENABLED) || (SD_CARD_USE_POWER_CONTROL == DEF_ENABLED)
        sSD_CardIO*             m_pSD_CardIO;
       #endif

        uint8_t                 m_TickPeriod;
        uint32_t                m_OCR;
        SD_SCR_t                m_SCR;
        uint32_t                m_CardCID[4];
        SD_CID_t                m_CID;
        uint32_t                m_CardCSD[4];
        SD_CSD_t                m_CSD;
        uint32_t                m_RCA;
        uint8_t                 m_Status[16];
        SD_CardType_e           m_CardType;
        uint32_t                m_CardCapacity;             // this capacity is in 1K granularity
        uint32_t                m_CardBlockSize;
        SystemState_e           m_CardStatus;
       #if (SD_CARD_USE_DETECT_SIGNAL == DEF_ENABLED)
        volatile SystemState_e  m_Detect;
       #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "sdio_var.h"

//-------------------------------------------------------------------------------------------------

#endif // (USE_SDIO_DRIVER == DEF_ENABLED)
