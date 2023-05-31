//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_fatfs_sdio.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_FATFS_USE_SDIO_SD_CARD == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   FatFS_SDIO
//
//   Parameter(s):  None
//
//   Description:   Initialize SDIO Driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
FatFS_SDIO::FatFS_SDIO(void* pArg)
{
    m_pSDIO_Driver = (SDIO_Driver*)pArg;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Initialize SD Card
//
//   Note(s):       This should the standard sequence to init a SD card
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_SDIO::Initialize(void)
{
    SystemState_e  State;
    SD_CSD_t*      pCSD;
    SD_CID_t*      pCID;

    m_pSDIO_Driver->Initialize();

    m_pSDIO_Driver->PowerOFF();

    // Check if SD card is present
    if(m_pSDIO_Driver->IsDetected() == SYS_DEVICE_NOT_PRESENT)
    {
        return STA_NOINIT;
    }

    if((State = m_pSDIO_Driver->PowerON()) == SYS_READY)                                // Identify card operating voltage
    {
        if((State = m_pSDIO_Driver->InitializeCard()) == SYS_READY)                     // Initialize the present card and put them in idle state
        {
            pCSD = pMemoryPool->Alloc(sizeof(SD_CSD_t));
            pCID = pMemoryPool->Alloc(sizeof(SD_CID_t));

            if((State = m_pSDIO_Driver->GetCardInfo(pCSD, pCID)) == SYS_READY)    // Read CSD/CID MSD registers
            {
                State = m_pSDIO_Driver->SelectTheCard();                                // Select the Card
            }

            pMemoryPool->Free(&pCSD);
            pMemoryPool->Free(&pCID);
        }
    }

    if(State == SYS_READY)                                                              // Configure SD Bus width
    {
        State = m_pSDIO_Driver->SetBusWidth(SD_BUS_WIDE_4B);                            // Enable wide operation
    }

    m_Status = (State == SYS_READY) ? SYS_READY : STA_NOINIT;
    return m_Status;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Status
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Get Status from SD Card
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_SDIO::Status(void)
{
    SystemState_e State;

    // Check SDCARD status
    State = (m_pSDIO_Driver->GetStatus() == SYS_READY) ? SYS_READY : STA_NOINIT;
    return (DSTATUS)State;
}


//-------------------------------------------------------------------------------------------------
//
//   Function:       Read
//
//   Parameter(s):  uint8_t*  pBuffer
//                  uint32_t  Sector
//                  uint8_t   NumberOfSectors
//   Return value:  DRESULT
//
//   Description:   Read From SD Card
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_SDIO::Read(uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    // Prepare the DMA Transfer
    m_pSDIO_Driver->StartBlockTransfert(DMA2_Stream3, (uint32_t *)pBuffer, BLOCK_SIZE, NumberOfSectors);

    // Read block(s) in DMA transfer mode
    if(m_pSDIO_Driver->ReadBlocks((uint64_t)(Sector * BLOCK_SIZE), BLOCK_SIZE, NumberOfSectors) == SYS_READY)
    {
        // Wait until transfer is complete
        if(m_pSDIO_Driver->CheckOperation(SDMMC_STA_RXACT) == SYS_READY)
        {
            return RES_OK;
        }
    }

    return RES_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Write
//
//   Parameter(s):  const uint8_t*  pBuffer
//                  uint32_t        Sector
//                  uint8_t         NumberOfSectors
//   Return value:  DRESULT
//
//   Description:   Write to the SD Card
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_SDIO::Write(const uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    // Prepare the DMA Transfer
    m_pSDIO_Driver->StartBlockTransfert(DMA2_Stream6, (uint32_t *)pBuffer, BLOCK_SIZE, NumberOfSectors);

    // Write block(s) in DMA transfer mode
    if(m_pSDIO_Driver->WriteBlocks((uint64_t)(Sector * BLOCK_SIZE), BLOCK_SIZE, NumberOfSectors) == SYS_READY)
    {
        // Wait until transfer is complete
        if(m_pSDIO_Driver->CheckOperation(SDMMC_STA_TXACT) == SYS_READY)
        {
            return RES_OK;
        }
    }

    return RES_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: IO_Ctrl
//
//   Parameter(s):  uint8_t    Control        Control code
//                    void*    pBuffer        Buffer to send/receive control data
//   Return value:  DRESULT
//
//   Description:   Control
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
#if _USE_IOCTL == 1
DRESULT FatFS_SDIO::IO_Ctrl(uint8_t Control, void *pBuffer)
{
    DRESULT     Result;
//  uint8_t     b;
    //uint8_t*    pPtr = (uint8_t*)pBuffer;
    //uint32_t    Data;
    //uint32_t*   dp;
    //uint32_t    st;
    //uint32_t    ed;


    if(m_Status == STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    Result = RES_ERROR;

    switch(Control)
    {
        // Make sure that all data has been written on the media
        case CTRL_SYNC:
        {
            Result = RES_OK;
            break;
        }

        // Get number of sectors on the disk (uint32_t)
        case GET_SECTOR_COUNT:
        {
            *(DWORD *)pBuffer = m_pSDIO_Driver->GetCardCapacity();
            Result = RES_OK;
            break;
        }

        // Get sectors on the disk (uint16_t)
        case GET_SECTOR_SIZE:
        {
            *(WORD *)pBuffer = BLOCK_SIZE;
            Result = RES_OK;
            break;
        }

        // Get erase block size in unit of sectors (uint32_t)
        case GET_BLOCK_SIZE:
        {
            *(DWORD*)pBuffer = BLOCK_SIZE;
            break;
        }

/*        // Erase a block of sectors
        case CTRL_ERASE_SECTOR:
        {

            if(!(m_CardType & HIGH_CAPACITY_SD_CARD) || (((m_CardCSD[0].u_8.u0 >> 6) == 0) && ((m_CardCSD[2].u_8.u2 & 0x40) == 0)))
            {
                break;                                                              // Check if sector erase can be applied to the card
            }

            dp = (uint32_t*)pBuffer;
            st = dp[0];
            ed = dp[1];

// TODO need to check this
        //    if(!(m_CardType & CT_BLOCK))
        //    {
        //       st *= 512;
        //       ed *= 512;
        //    }

            if((m_pSDIO_Driver->SendCommand((CMD32 | SD_SHORT_RESPONSE), st, R1_RESPONSE, &m_Response[0]) == SYS_READY) &&
               (m_pSDIO_Driver->SendCommand((CMD33 | SD_SHORT_RESPONSE), ed, R1_RESPONSE, &m_Response[0]) == SYS_READY) &&
               (m_pSDIO_Driver->SendCommand((CMD38 | SD_SHORT_RESPONSE), 0,  R1_RESPONSE, &m_Response[0]) == SYS_READY) &&
               (WaitReady(30000) == SYS_READY))
            {
                Result = RES_OK;
            }
            break;
        }

        // Get card type flags (1 byte)
        case MMC_GET_TYPE:
        {
            *pPtr = m_CardType;
            Result = RES_OK;
            break;
        }

        case GET_CSD:                                                               // Get CSD (16 bytes)
        {
            memcpy(pPtr, m_CardCSD, 16);
            Result = RES_OK;
            break;
        }

        case MMC_GET_CID:                                                           // Get CID (16 bytes)
        {
            memcpy(pPtr, m_CardCID, 16);
            Result = RES_OK;
            break;
        }

        case MMC_GET_OCR:                                                           // Get OCR (4 bytes)
        {
            memcpy(pPtr, &m_OCR, 4);
            Result = RES_OK;
            break;
        }

        // Receive SD status as a data block (64 bytes)

        case MMC_GET_SDSTAT:
            if(m_CardType & CT_SDC)                                                 // SDC
            {
                if(WaitReady(500))
                {

                    m_pDriver->ReadyReception(1, 64);                                          // Ready to receive data blocks
                    if((This->SendCommand((ACMD13 | SD_SHORT_RESPONSE), 0, R1_RESPONSE, &m_Response[0]) == true) &&
                       (m_Response[0] & 0xC0580000) == 0)                           // Start to read
                    {
                        while((MCI_dwXferWp == 0) && ((MCI_byXferStat & 0xC) == 0) && (m_Detect == DEVICE_PRESENT));

                        if((MCI_byXferStat & 0xC) == 0)
                        {
                            LIB_memcpy(pBuffer, DmaBuff[0], 64);                   // Copy_al2un(buff, DmaBuff[0], 64);
                            Result = RES_OK;
                        }
                    }

                }
                //StopTransfert();                                                // Close data path
            }
            break;
*/
        default:
            Result = RES_PARERR;
    }

    return Result;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetDriveSize
//
//   Parameter(s):  char*           pDriveName
//   Return value:  FATFS_Size_t*   SizeStruct
//   Return value:  FRESULT
//
//   Description:   Get response from SD device
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
FRESULT FatFS_SDIO::GetDriveSize(char* pDriveName, FatFS_Size_t* SizeStruct)
{
    FATFS*  pFS;
    DWORD   FreeCluster;
    FRESULT Result;

    // Get volume information and free clusters of drive
    if((Result = f_getfree(pDriveName, &FreeCluster, &pFS)) == FR_OK)
    {
        // Get total sectors and free sectors
        SizeStruct->Total = (pFS->n_fatent - 2) * pFS->csize * 0.5;
        SizeStruct->Free = FreeCluster * pFS->csize * 0.5;
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_FATFS_USE_SDIO_SD_CARD == DEF_ENABLED)
