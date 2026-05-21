//-------------------------------------------------------------------------------------------------
//
//  File : usbh_conf.c
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
// Include(s)
//-------------------------------------------------------------------------------------------------
#include "usbh_core.h"
#include "stm32f7xx_hal_hcd.h"
#include "lib_delay.h"

//-------------------------------------------------------------------------------------------------
// Private variable(s)
//-------------------------------------------------------------------------------------------------

HCD_HandleTypeDef hhcd_USB_OTG_FS;

//-------------------------------------------------------------------------------------------------
// Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_Get_USB_Status
//
//  Parameter(s):   HAL_Status 		HAL status returned by HAL HCD layer
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Converts a HAL_StatusTypeDef into a USBH_StatusTypeDef.
//                  Used by the USB Host middleware to normalize HAL return codes.
//
//  Note(s):        HAL_TIMEOUT, HAL_ERROR and any undefined status are mapped to USBH_FAIL.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_Get_USB_Status(HAL_StatusTypeDef HAL_Status)
{
    USBH_StatusTypeDef USB_Status = USBH_OK;

    switch(HAL_Status)
    {
        case HAL_OK:      USB_Status = USBH_OK;   break;
        case HAL_BUSY:    USB_Status = USBH_BUSY; break;
        //case HAL_TIMEOUT:
        //case HAL_ERROR:
        // and all other
        default:          USB_Status = USBH_FAIL; break;
    }

    return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_Init
//
//  Parameter(s):   pHost 		USB Host handle
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Initialize the Low Level portion of the USB Host driver.
//                  This function configures the HCD (Host Controller Driver)
//                  and links it to the USB Host stack.
//
//  Note(s):        This implementation is specific to STM32F4 (USB_OTG_FS).
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_Init(USBH_HandleTypeDef *pHost)
{
    // Set the LL Driver parameters
    hhcd_USB_OTG_FS.Instance                 = USB_OTG_FS;
    hhcd_USB_OTG_FS.Init.Host_channels       = 8;
    hhcd_USB_OTG_FS.Init.speed               = HCD_SPEED_FULL;
    hhcd_USB_OTG_FS.Init.dma_enable          = DISABLE;
    hhcd_USB_OTG_FS.Init.phy_itface          = HCD_PHY_EMBEDDED;
    hhcd_USB_OTG_FS.Init.Sof_enable          = ENABLE;
    hhcd_USB_OTG_FS.Init.low_power_enable    = DISABLE;
    hhcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;

    // Link the driver to the stack
    hhcd_USB_OTG_FS.pData = pHost;
    pHost->pData = &hhcd_USB_OTG_FS;

    // Initialize LL Driver
    HAL_StatusTypeDef HAL_Status = HAL_HCD_Init(&hhcd_USB_OTG_FS);

    return USBH_Get_USB_Status(HAL_Status);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_DeInit
//
//  Parameter(s):   pHost 		USB Host handle
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    De-initialize the Low Level portion of the USB Host driver.
//                  This function calls HAL_HCD_DeInit() and converts the HAL
//                  return code into a USBH_StatusTypeDef.
//
//  Note(s):        HAL_HCD_DeInit() is expected to release hardware resources
//                  associated with the USB OTG FS Host controller.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_DeInit(USBH_HandleTypeDef *pHost)
{
    USBH_StatusTypeDef USB_Status = USBH_Get_USB_Status(HAL_HCD_DeInit(pHost->pData));
    return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_Start
//
//  Parameter(s):   pHost 		USB Host handle
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Start the Low Level portion of the USB Host driver.
//                  This function enables the USB OTG FS Host controller
//                  and allows the USB Host stack to begin enumeration.
//
//  Note(s):        HAL_HCD_Start() must be called after USBH_LL_Init().
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_Start(USBH_HandleTypeDef *pHost)
{
    USBH_StatusTypeDef USB_Status = USBH_Get_USB_Status(HAL_HCD_Start(pHost->pData));
    return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_Stop
//
//  Parameter(s):   pHost 		USB Host handle
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Stop the Low Level portion of the USB Host driver.
//                  This function disables the USB OTG FS Host controller
//                  and halts all ongoing USB Host operations.
//
//  Note(s):        HAL_HCD_Stop() should be called before de-initializing
//                  the USB Host stack or when stopping USB Host activity.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_Stop(USBH_HandleTypeDef *pHost)
{
    USBH_StatusTypeDef USB_Status = USBH_Get_USB_Status(HAL_HCD_Stop(pHost->pData));
    return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_GetSpeed
//
//  Parameter(s):   pHost 		USB Host handle
//
//  Return:         USBH_SpeedTypeDef
//
//  Description:    Return the current USB Host Speed as reported by the
//                  Low Level driver. For STM32F4 USB OTG FS, only FULL Speed
//                  is supported.
//
//  Note(s):        USB OTG FS peripheral does not support HIGH Speed.
//
//-------------------------------------------------------------------------------------------------
USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef *pHost)
{
    return USBH_SPEED_FULL;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_ResetPort
//
//  Parameter(s):   pHost 		USB Host handle
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Reset the USB Host port through the Low Level driver.
//                  This operation forces a USB bus reset on the OTG FS port,
//                  allowing device re-enumeration.
//
//  Note(s):        HAL_HCD_ResetPort() performs the actual hardware reset
//                  on the USB OTG FS Host controller.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_ResetPort(USBH_HandleTypeDef *pHost)
{
    USBH_StatusTypeDef USB_Status = USBH_Get_USB_Status(HAL_HCD_ResetPort(pHost->pData));
    return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_GetLastXferSize
//
//  Parameter(s):   pHost 		USB Host handle
//                  Pipe  		Pipe index
//
//  Return:         uint32_t
//
//  Description:    Return the number of bytes transferred during the last
//                  USB Host channel transaction for the specified Pipe.
//
//  Note(s):        HAL_HCD_HC_GetXferCount() reads the transfer count from
//                  the Host Channel (HC) registers of the OTG FS controller.
//
//-------------------------------------------------------------------------------------------------
uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef *pHost, uint8_t Pipe)
{
    return HAL_HCD_HC_GetXferCount(pHost->pData, Pipe);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_OpenPipe
//
//  Parameter(s):   pHost       		USB Host handle
//                  Pipe        		Pipe index
//                  EndPointNumber      Endpoint number
//                  DeviceAddress		USB device address
//                  Speed       		Device Speed
//                  EndpointType     	Endpoint type
//                  MaxPacketSize       Max packet size
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Open a USB Host Pipe through the Low Level driver.
//                  This function configures a Host Channel (HC) of the
//                  USB OTG FS controller for the specified endpoint.
//
//  Note(s):        HAL_HCD_HC_Init() programs the HC registers and prepares
//                  the Pipe for future transfers.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_OpenPipe(USBH_HandleTypeDef *pHost, uint8_t Pipe, uint8_t EndPointNumber, uint8_t DeviceAddress, uint8_t Speed, uint8_t EndpointType, uint16_t MaxPacketSize)
{
    USBH_StatusTypeDef USB_Status = USBH_Get_USB_Status(HAL_HCD_HC_Init(pHost->pData, Pipe, EndPointNumber, DeviceAddress, Speed, EndpointType, MaxPacketSize));
    return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_ClosePipe
//
//  Parameter(s):   pHost 		USB Host handle
//                  Pipe  		Pipe index
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Close a USB Host Pipe through the Low Level driver.
//                  This function halts the specified Host Channel (HC)
//                  associated with the Pipe.
//
//  Note(s):        HAL_HCD_Stop() stops the entire USB Host controller,
//                  which is incorrect here. The correct API is HC_Halt.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_ClosePipe(USBH_HandleTypeDef *pHost, uint8_t Pipe)
{
    USBH_StatusTypeDef USB_Status =USBH_Get_USB_Status(HAL_HCD_HC_Halt(pHost->pData, Pipe));
    return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_ActivatePipe
//
//  Parameter(s):   pHost 		USB Host handle
//                  Pipe  		Pipe index
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Activate a USB Host Pipe through the Low Level driver.
//                  For STM32F4 USB OTG FS, no explicit activation step is
//                  required. The function is kept for API completeness.
//
//  Note(s):        This function is intentionally empty on STM32F4 FS.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_ActivatePipe(USBH_HandleTypeDef *pHost, uint8_t Pipe)
{
  // Prevent unused argument(s) compilation warning
	UNUSED(pHost);
	UNUSED(Pipe);

	return USBH_OK;
}

//-------------------------------------------------------------------------------------------------
//  Name:           USBH_LL_SubmitURB
//
//  Parameter(s):   pHost       : USB Host handle
//                  Pipe        : Pipe index
//                  Direction   : 0 = OUT, 1 = IN
//                  EndpointType: Control / Bulk / Interrupt / Isochronous
//                  Token       : PID (SETUP / DATA0 / DATA1)
//                  pBuffer     : Data buffer
//                  Length      : Buffer length
//                  DoPing      : Ping protocol (HS only)
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Submit a USB transfer request to the Host Channel. On STM32F7,
//                  the D-Cache must be synchronized with memory before OUT transfers
//                  and after IN transfers to ensure correct DMA operation.
//
//  Note(s):        - OUT  : Clean D-Cache (CPU -> RAM -> USB DMA)
//                  - IN   : Invalidate D-Cache (USB DMA -> RAM -> CPU)
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_SubmitURB(USBH_HandleTypeDef *pHost, uint8_t Pipe, uint8_t Direction, uint8_t EndpointType, uint8_t Token, uint8_t *pBuffer, uint16_t Length, uint8_t DoPing)
{
    // OUT transfer -> Clean D-Cache so DMA sees the latest data
    if((Direction == 0) && (pBuffer != NULL) && (Length > 0))
    {
        SCB_CleanDCache_by_Addr((uint32_t *)pBuffer, Length);
    }
	
	USBH_StatusTypeDef USB_Status = USBH_Get_USB_Status(HAL_HCD_HC_SubmitRequest(pHost->pData, Pipe, Direction, EndpointType, Token, pBuffer, Length, DoPing));
	
    // IN transfer -> Invalidate D-Cache so CPU sees fresh data
    if((Direction == 1) && (pBuffer != NULL) && (Length > 0))
    {
        SCB_InvalidateDCache_by_Addr((uint32_t *)pBuffer, Length);
    }	
	
	return USB_Status;
}

//-------------------------------------------------------------------------------------------------
//  Name:           USBH_LL_GetURBState
//
//  Parameter(s):   pHost : USB Host handle
//                  Pipe  : Pipe index
//
//  Return:         USBH_URBStateTypeDef
//
//  Description:    Return the current URB state for the specified pipe. On STM32F7,
//                  the D-Cache must be invalidated after IN transfers to ensure the
//                  CPU reads the data written by the USB DMA.
//
//  Note(s):        - OUT transfers do not require cache maintenance here.
//                  - IN transfers require Invalidate D-Cache.
//-------------------------------------------------------------------------------------------------
USBH_URBStateTypeDef USBH_LL_GetURBState(USBH_HandleTypeDef *pHost, uint8_t Pipe)
{
    HCD_HandleTypeDef *pHCD = (HCD_HandleTypeDef *)pHost->pData;

    USBH_URBStateTypeDef URB_State = HAL_HCD_HC_GetURBState(pHCD, Pipe);

    // If transfer completed AND direction is IN -> Invalidate D-Cache
    if(URB_State == USBH_URB_DONE)
    {
        HCD_HCTypeDef* pHC = &pHCD->hc[Pipe];

        if ((pHC->ep_is_in == 1) && (pHC->xfer_buff != NULL) && (pHC->xfer_len > 0))
        {
            SCB_InvalidateDCache_by_Addr((uint32_t *)pHC->xfer_buff, pHC->xfer_len);
        }
    }

    return URB_State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_DriverVBUS
//
//  Parameter(s):   pHost 		USB Host handle
//                  State 		VBUS State (0 = inactive, 1 = active)
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Drive the USB VBUS line. On STM32F4 USB OTG FS, VBUS
//                  control is typically handled by external hardware and
//                  no software action is required.
//
//  Note(s):        This function is intentionally empty. It is kept for API
//                  compatibility with the USB Host stack.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_DriverVBUS(USBH_HandleTypeDef *pHost, uint8_t State)
{
	// Prevent unused argument(s) compilation warning
	UNUSED(pHost);
	UNUSED(State);

	return USBH_OK;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_SetToggle
//
//  Parameter(s):   pHost  		USB Host handle
//                  Pipe   		Pipe index
//                  Toggle 		Data Toggle value (0 or 1)
//
//  Return:         USBH_StatusTypeDef
//
//  Description:    Set the data Toggle for the specified Pipe. On STM32F4
//                  USB OTG FS, the Toggle is managed automatically by the
//                  hardware, so no software action is required.
//
//  Note(s):        This function is intentionally empty. It is kept for API
//                  compatibility with the USB Host stack.
//
//-------------------------------------------------------------------------------------------------
USBH_StatusTypeDef USBH_LL_SetToggle(USBH_HandleTypeDef *pHost, uint8_t Pipe, uint8_t Toggle)
{
	// Prevent unused argument(s) compilation warning
	UNUSED(pHost);
	UNUSED(Pipe);
	UNUSED(Toggle);

	return USBH_OK;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_LL_GetToggle
//
//  Parameter(s):   pHost 		USB Host handle
//                  Pipe  		Pipe index
//
//  Return:         uint8_t
//
//  Description:    Return the current data Toggle for the specified Pipe.
//                  On STM32F4 USB OTG FS, the Toggle is managed entirely by
//                  the hardware, so this function always returns 0.
//
//  Note(s):        This function is intentionally static. It is kept for API
//                  compatibility with the USB Host stack.
//
//-------------------------------------------------------------------------------------------------
uint8_t USBH_LL_GetToggle(USBH_HandleTypeDef *pHost, uint8_t Pipe)
{
	// Prevent unused argument(s) compilation warning
	UNUSED(pHost);
	UNUSED(Pipe);

	uint8_t Toggle = 0U;
	return Toggle;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           USBH_Delay
//
//  Parameter(s):   Delay 		Delay in milliseconds
//
//  Return:         None
//
//  Description:    Delay routine used by the USB Host Library. This function
//                  provides a blocking delay for the specified duration.
//
//  Note(s):        LIB_Delay_mSec() must provide an accurate millisecond
//                  delay for proper USB timing behavior.
//
//-------------------------------------------------------------------------------------------------
void USBH_Delay(uint32_t Delay)
{
	LIB_Delay_mSec(Delay);
}
//-------------------------------------------------------------------------------------------------

