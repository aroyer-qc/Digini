//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_fatfs_usb_key.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#include "./lib_digini.h"
#include "usbh_def.h"
#include "usbh_msc.h"
#include "task_usb_host.h"              // until USB is integrated into Digini!!

//-------------------------------------------------------------------------------------------------

#if (DIGINI_FATFS_USE_USB_KEY == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Class: FatFS_USB_Key
//
//
//   Description:   Class to handle FatFS for flash disk
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function:      Configure
//
//   Parameter(s):  uint8_t*    pBuffer         Data buffer allocated for flash disk
//                  size_t      Size            Size of the buffer
//
//   Return value:  None
//
//-------------------------------------------------------------------------------------------------
//void FatFS_USB_Key::Configure(uint8_t* pBuffer, size_t Size)
//{
   // m_Size           = Size;
   // m_IsItInitialize = true;
//}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Initialize
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Initialize flash disk
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_USB_Key::Initialize(void* pParameter)
{
    VAR_UNUSED(pParameter);
	m_Status = STA_OK;
    return STA_OK;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Status
//
//   Parameter(s):  None
//   Return value:  DSTATUS
//
//   Description:   Get Status from Flash disk device.
//
//-------------------------------------------------------------------------------------------------
DSTATUS FatFS_USB_Key::Status(void)
{
    TaskUSB_Host* pUSB = TaskUSB_Host::GetInstance();

    if((pUSB == nullptr) || (pUSB->Get_MSC_IsConnected() == false))
	{
        return STA_NOINIT;
	}

    return STA_OK;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Read
//
//   Parameter(s):  uint8_t*  pBuffer
//                  uint32_t  Sector
//                  uint16_t   NumberOfSectors
//   Return value:  DRESULT
//
//   Description:   Read From flash disk device.
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_USB_Key::Read(uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    TaskUSB_Host* pUSB = TaskUSB_Host::GetInstance();

	if(Status() != STA_OK)
	{
        return RES_NOTRDY;
	}

    USBH_HandleTypeDef* pHost = pUSB->GetUSBH_Handle();

    if(USBH_MSC_Read(pHost, 0, Sector, pBuffer, NumberOfSectors) == USBH_OK)
	{
        return RES_OK;
	}

    return RES_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Write
//
//   Parameter(s):  const uint8_t*  pBuffer
//                  uint32_t        Sector
//                  uint16_t        NumberOfSectors
//   Return value:  DRESULT
//
//   Description:   Write to the flash disk device
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_USB_Key::Write(const uint8_t* pBuffer, uint32_t Sector, uint16_t NumberOfSectors)
{
    TaskUSB_Host* pUSB = TaskUSB_Host::GetInstance();

	if(Status() != STA_OK)
	{
        return RES_NOTRDY;
	}

    USBH_HandleTypeDef* pHost = pUSB->GetUSBH_Handle();

    if(USBH_MSC_Write(pHost, 0, Sector,  (uint8_t*)pBuffer, NumberOfSectors) == USBH_OK)
	{
        return RES_OK;
	}

    return RES_ERROR;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: IO_Control
//
//   Parameter(s):  uint8_t    Control        Control code
//                  void*      pBuffer        Buffer to send/receive control data
//   Return value:  DRESULT
//
//   Description:   Control FatFs required functions
//
//-------------------------------------------------------------------------------------------------
DRESULT FatFS_USB_Key::IO_Ctrl(uint8_t Control, void *pBuffer)
{
    TaskUSB_Host* pUSB = TaskUSB_Host::GetInstance();

	if(Status() != STA_OK)
	{
        return RES_NOTRDY;
	}

    MSC_LUNTypeDef* pLUN = pUSB->GetLUN_Info();

    switch(Control)
    {
        case CTRL_SYNC:
		{
            return RES_OK;
		}

        case GET_SECTOR_COUNT:
		{
            *(DWORD*)pBuffer = pLUN->capacity.block_nbr;
			return RES_OK;
		}

        case GET_SECTOR_SIZE:
		{
            *(WORD*)pBuffer = pLUN->capacity.block_size;
			return RES_OK;
		}

        case GET_BLOCK_SIZE:
		{
            *(DWORD*)pBuffer = 1;
            return RES_OK;
		}
    }

    return RES_PARERR;
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_FATFS_USE_USB_KEY

//-------------------------------------------------------------------------------------------------
