//-------------------------------------------------------------------------------------------------
//
//  File : lib_generic_spi_lcd_128X160 - ER_TFTM018_3 - ST7735.cpp
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
//
// Note(s)
//
//      This driver use LCD as foreground layer, and internal ram for construction layer.
//      RGB565 only because of RAM limitation on the STM32F4
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_ST7735_GLOBAL
#include "./lib_digini.h"
#undef  LIB_ST7735_GLOBAL



const ST7735_InitCMD_t GrafxDriver::m_InitCMD[GRAFX_NUMBER_OF_INIT_CMD] =
{
    {ST7735_SWRESET,   nullptr, 0, 50},
    {,             SSD2119_OSCILLATOR_ENABLE},          // Enable oscillator
    {,              SSD2119_POWER_CONTROL_1_VALUE},      // Power step 1
    {,              SSD2119_POWER_CONTROL_2_VALUE},      // Power step 2
    {ST7735_INVCTR, 0x03, 1,              },      // Power step 3
    {,              SSD2119_POWER_CONTROL_4_VALUE},      // Power step 4
    {,              SSD2119_POWER_CONTROL_5_VALUE},      // Power step 5
    {,                   SSD2119_VCOM_OTP_1_VALUE},           // VCOM amplitude
    {,                   SSD2119_VCOM_OTP_2_VALUE},           // VCOM amplitude
    {,                   SSD2119_DISPLAY_EXIT_SLEEP_MODE},    // Exit sleep
    {,                   SSD2119_ENTRY_MODE_VALUE},           // RGB565, normal scan
    {,               SSD2119_OUTPUT_CONTROL_VALUE},       // Panel type + scan direction
    {,              0x0000},                             // Start at gate 0
    {,              SSD2119_FRAME_FREQUENCY_VALUE},      // Frame timing
    {,    SSD2119_FRAME_FREQUENCY_CONTROL_2_VALUE},
    {,          SSD2119_FRAME_CYCLE_CONTROL_VALUE},
    {,         0x0009},
    {,              0x0000},
    {,              0x0106},
    {,              0x0100},
    {,              0x0303},
    {,              0x0003},
    {,              0x0004},
    {,              0x0203},
    {,              0x0303},
    {,              0x0100},
    {,             0x0504},
    {,              SSD2119_DISPLAY_ON_VALUE}             // Display ON
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void*      pArg       Optional initialization argument passed to the driver.
//
//  Return:         None
//
//  Description:    Performs the complete initialization sequence of the graphics driver.
//                  Resets the LCD controller, reads the device identification code, and
//                  sends the full initialization command table to configure the ST7735.
//                  Once the controller is ready, the foreground display layer is cleared
//                  to a known state.
//
//  Note(s):        IO with those name must exist into bsp_io_def.h:
//                  IO_ST7735_CS, IO_ST7735_DC, IO_LCD_RESET, IO_ST7735_BACKLIGHT
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Initialize(const void* pArg)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN;

    m_pSPI = (SPI_Driver*)pArg;
    m_pSPI->Initialize();                                           // CS(NSS) handle by class

    // I may need to provide a pointer to the background image for building element to display on the screen (merge)
    //m_pBackground = (StaticImageInfo_t*)pArg;

    GrafxGenDriver::Initialize(nullptr);
    IO_SetPinHigh(IO_LCD_RESET);
    LIB_Delay_mSec(50);

    SendCommand(ST7735_SWRESET, nullptr, 0, 50);            // Software reset

    // Frame Rate
    const uint8_t Data_FRMCTR[6] = {0x05, 0x3C, 0x3C,0x05, 0x3C, 0x3C};
    SendCommand(ST7735_FRMCTR1, (uint8_t*)&Data_FRMCTR[0], 3, 1);    // Frame control 1
    SendCommand(ST7735_FRMCTR2, (uint8_t*)&Data_FRMCTR[0], 3, 1);    // Frame control 2
    SendCommand(ST7735_FRMCTR3, (uint8_t*)&Data_FRMCTR[0], 6, 1);    // Frame control 2

    SendCommand(ST7735_INVCTR, 0x03, 1);                   // Dot inversion

	// Power Sequence

    const uint8_t Data_PWR_CTRL1[3] = {0x28, 0x08, 0x04};
    const uint8_t Data_PWR_CTRL3[2] = {0x0D, 0x00};
    const uint8_t Data_PWR_CTRL4[2] = {0x8D, 0x2A};
    const uint8_t Data_PWR_CTRL5[2] = {0x8D, 0xEE};
    SendCommand(ST7735_POWER_CTRL_1, (uint8_t*)&Data_PWR_CTRL1[0], 3, 1);
    SendCommand(ST7735_POWER_CTRL_2, 0xC0, 1);
    SendCommand(ST7735_POWER_CTRL_3, (uint8_t*)&Data_PWR_CTRL3[0], 2, 1);
    SendCommand(ST7735_POWER_CTRL_4, (uint8_t*)&Data_PWR_CTRL4[0], 2, 1);
    SendCommand(ST7735_POWER_CTRL_5, (uint8_t*)&Data_PWR_CTRL5[0], 2, 1);

    SendCommand(ST7735_VMCTR1, 0x10, 1);                        // VCOM
    SendCommand(ST7735_MADCTL, 0xC0, 1);                        // MX, MY, RGB mode

	// Gamma Sequence
    const uint8_t Data_GMCTRP1[16] = {0x04, 0x22, 0x07, 0x0A, 0x2E, 0x30, 0x25, 0x2A,
                                      0x28, 0x26, 0x2E, 0x3A, 0x00, 0x01, 0x03, 0x13};
    const uint8_t Data_GMCTRN1[16] = {0x04, 0x16, 0x06, 0x0D, 0x2D, 0x26, 0x23, 0x27,
                                      0x27, 0x25, 0x2D, 0x3B, 0x00, 0x01, 0x04, 0x13};
    SendCommand(ST7735_GMCTRP1, (uint8_t*)&Data_GMCTRP1[0], 16, 1);
    SendCommand(ST7735_GMCTRN1, (uint8_t*)&Data_GMCTRN1[0], 16, 1);

    SendCommand(ST7735_COLMOD, 0x05, 1);                        // 65K mode
    SendCommand(ST7735_SLPOUT, nullptr, 0, 120);                // Exit Sleep
    DisplayOn();
    Clear();
    IO_SetPinHigh(IO_ST7735_BACKLIGHT);

    ClearLayer(FOREGROUND_DISPLAY_LAYER_0);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearLayer
//
//  Parameter(s):   Layer_e    Layer      Specifies the display layer to clear.
//
//  Return:         None
//
//  Description:    Clears the selected display layer by writing a constant color value to the
//                  entire GRAM region associated with this driver. The GRAM write pointer is
//                  positioned at the origin before sequential pixel data is written.
//
//  Note(s):        Only layers managed directly by this driver are cleared. Any additional
//                  layers are handled by the base graphics class.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::ClearLayer(Layer_e Layer)
{
   	if(Layer == FOREGROUND_DISPLAY_LAYER_0)
    {
        ResetWindow();

        for(uint32_t i = 0; i < GRAFX_DRIVER_SIZE; i++)
        {
            WriteData(0x0000);
        }
    }
    else
    {
        GrafxGenDriver::ClearLayer(Layer);
    }
}
































































//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyWidgetToDevice
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::CopyWidgetToDevice(ImageID_e Image, Cartesian_t Position)
{
    StaticImageInfo_t* pImage;

    DB_Central.Get(&pImage, GFX_IMAGE_INFO, uint16_t(Image));
    CopyWidgetToDevice(pImage->Info.Size, Position);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyWidgetToDevice
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::CopyWidgetToDevice(BoxSize_t BoxSize, Cartesian_t Position)
{
    DisplayLayer* pLayer   = &LayerTable[DisplayLayer::GetDrawing()];
    uint32_t      Address = pLayer->GetAddress();
    uint16_t*     pDataPtr;
    uint32_t      SizeX = BoxSize.Width;
    uint32_t      SizeY = BoxSize.Height;
    uint32_t      ImageSize = SizeX * SizeY;

    uint16_t* pImageDestination = (uint16_t*)pMemoryPool->Alloc(ImageSize * sizeof(uint16_t), MEM_DBG_GRAFX_CL1);

    DMA2D->CR      = DMA2D_M2M_PFC;                                 // Memory-to-Memory with Pixel Format Conversion
    DMA2D->FGMAR   = Address;                                       // Source address
    DMA2D->FGOR    = (uint32_t)pLayer->GetSize().X - SizeX;         // Source line offset so none as we are linear
    DMA2D->FGPFCCR = DMA2D_CONVERSION_ARGB8888;                     // Defines the size of pixel. 0 for format PIXEL_FORMAT_ARGB8888
    DMA2D->OMAR    = uint32_t(pImageDestination);                   // Destination address
    DMA2D->OOR     = 0;                                             // Destination line offset none as we are linear
    DMA2D->OPFCCR  = DMA2D_CONVERSION_RGB565;                       // Defines the size of pixel. 0 for format PIXEL_FORMAT_ARGB8888
    DMA2D->NLR     = (SizeX << 16) | SizeY;                         // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                             // Start operation
    while ((DMA2D->ISR & DMA2D_ISR_ALL_FLAG) == 0);                 // Wait for transfer complete
    DMA2D->IFCR = DMA2D_IFCR_ALL_FLAG;                              // Clear flag

    SetWindow(Position.X, Position.Y, &BoxSize);
    pDataPtr = pImageDestination;

    for(uint32_t i = 0; i < ImageSize; i++)
    {
        WriteData(*pDataPtr);
        pDataPtr++;
    }

    pMemoryPool->Free((void**)&pImageDestination);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetRAM_Pointer
//
//  Parameter(s):   uint16_t   PosX       Specifies the horizontal GRAM address.
//                  uint16_t   PosY       Specifies the vertical   GRAM address.
//
//  Return:         None
//
//  Description:    Positions the internal GRAM write pointer of the LCD controller.
//                  The X address register is written first, followed by the RAM data
//                  register selection. The Y address register is then updated to
//                  complete the pointer setup before pixel
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SetRAM_Pointer(uint16_t PosX, uint16_t PosY)
{
	WriteCommand(SSD2119_Y_RAM_ADDRESS_REGISTER, PosY);
    WriteCommand(SSD2119_X_RAM_ADDRESS_REGISTER, PosX);
	SetWriteRAM_Ready();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ReadCommand
//
//  Parameter(s):   Register    8-bit register index to read from the LCD controller.
//
//  Return:         uint16_t    16-bit value returned by the LCD controller.
//
//  Description:    Sends the specified register index to the SSD2119, waits briefly for the bus
//                  to settle, and then reads the corresponding 16-bit data from the LCD RAM
//                  interface. This function performs a direct register read and assumes that the
//                  controller is already configured for indexed register access.
//
//-------------------------------------------------------------------------------------------------
uint16_t GrafxDriver::ReadCommand(uint8_t Register)
{
    LCD_REG = Register;
    LIB_Delay_uSec(3);
    return LCD_RAM;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetWindow
//
//  Parameter(s):
//
//  Return:
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SetWindow(uint16_t PosX, uint16_t PosY, BoxSize_t* pBoxSize)
{
    Box_t Box;
    Box.Pos.X       = PosX;
    Box.Pos.Y       = PosY;
    Box.Size.Width  = pBoxSize->Width;
    Box.Size.Height = pBoxSize->Height;
    SetWindow(&Box);
}

//-------------------------------------------------------------------------------------------------
//  Name:           SetWindow
//
//  Parameter(s):   pBox    Pointer to a Box_t structure defining the drawing region.
//                          - pBox->Pos.X / pBox->Pos.Y   : Top-left coordinate of the window
//                          - pBox->Size.Width / Height   : Dimensions of the window
//
//  Return:         None
//
//  Description:    Configure the SSD2119 GRAM access window to the rectangular region defined
//                  by pBox. This sets the horizontal and vertical RAM boundaries so that all
//                  subsequent pixel writes are clipped to this area. The GRAM cursor is then
//                  positioned at the top-left corner of the window and the controller is placed
//                  in write-ready state.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SetWindow(Box_t* pBox)
{
    uint16_t StartX = pBox->Pos.X;
    uint16_t EndX   = StartX + pBox->Size.Width  - 1;
    uint16_t StartY = pBox->Pos.Y;
    uint16_t EndY   = StartY + pBox->Size.Height - 1;
    WriteCommand(SSD2119_HORIZONTAL_RAM_START_REGISTER, StartX);        // Horizontal window (X) Start
    WriteCommand(SSD2119_HORIZONTAL_RAM_END_REGISTER,   EndX);          // Horizontal window (X) End
    uint16_t Vertical = (EndY << 8) | StartY;
    WriteCommand(SSD2119_VERTICAL_RAM_POSITION_REGISTER, Vertical);     // Vertical window (Y) packed into one register
    SetRAM_Pointer(StartX, StartY);                                     // Set GRAM cursor to top-left of window
}
//-------------------------------------------------------------------------------------------------
//
//   Function name: SetWindow
//
//   Parameter(s):  PosX1         X and ...
//                  PosY1         Y for top left corner of the window
//                  PosX2         X and ...
//                  PosY2         Y for bottom right corner of the window
//   Return value:  None
//
//   Description:   Set drawing window
//
//-------------------------------------------------------------------------------------------------

void GrafxDriver::SetWindow(uint8_t PosX1, uint8_t PosY1, uint8_t PosX2, uint8_t PosY2)
{
    uint8_t Data[4] = {0,0,0,0};

    //uint16_t StartX = pBox->Pos.X;
    //uint16_t EndX   = StartX + pBox->Size.Width  - 1;
    //uint16_t StartY = pBox->Pos.Y;
    //uint16_t EndY   = StartY + pBox->Size.Height - 1;

??

    Data[1] = PosX1;                // X start
    Data[3] = PosX2;                // X end
    SendCommand(ST7735_CASET, &Data[0], 4, 1);
    Data[1] = PosY1;                // Y start
    Data[3] = PosY2;                // Y end
    SendCommand(ST7735_RASET, &Data[0], 4, 1);
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteRLE16
//
//  Parameter(s):   pData           Pointer to the RLE16 source data.
//                  pDestination    Pointer to the destination RGB565 buffer.
//                  Size            Number of RLE entries to decode.
//
//  Return:         None
//
//  Description:    Decode 16-bit RLE (RGB565) image data and write the expanded pixels either to
//                  the LCD interface (when pDestination is nullptr) or into a destination buffer.
//                  Each RLE entry contains a pixel value and a repeat count (Repeat + 1). The
//                  function expands all entries sequentially until the specified number of RLE
//                  items has been processed.
//
//  Note(s):        - When pDestination is nullptr, pixels are streamed directly to the LCD using
//                    WriteData(), which must accept RGB565 format.
//                  - The caller must ensure that the destination buffer is large enough to hold
//                    all expanded pixels.
//                  - No bounds checking is performed on the expanded output; RLE data must be
//                    valid and consistent with the expected image size.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteRLE16(StaticImageRLE_16_t* pData, uint16_t* pDestination, size_t Size)
{
    while(Size != 0)
    {
        uint32_t Repeat = pData->Repeat + 1;
        uint16_t Pixel  = pData->Pixel;
        pData++;

        if(pDestination == nullptr)
        {
            while(Repeat--)
            {
                WriteData(Pixel);
            }
        }
        else
        {
            while(Repeat--)
            {
                *pDestination = Pixel;
                pDestination++;
            }
        }

        Size--;
    }
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           WriteRLE32
//
//  Parameter(s):   pData           Pointer to the RLE32 source data.
//                  pDestination    Pointer to the destination ARGB8888 buffer.
//                  Size            Number of RLE entries to decode.
//
//  Return:         None
//
//  Description:    Decode 32-bit RLE (ARGB8888) image data and write the expanded pixels into
//                  the destination buffer. Each RLE entry contains a pixel value and a repeat
//                  count (Repeat + 1). The function expands all entries sequentially until the
//                  specified number of RLE items has been processed.
//
//  Note(s):        - This LCD does not support ARGB8888, therefore no write to LCD memory.
//                  - The caller must ensure that the destination buffer is large enough to
//                    receive all expanded pixels.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteRLE32(StaticImageRLE_32_t* pData, uint32_t* pDestination, size_t Size)
{
    if(pDestination != nullptr)
    {
        while(Size != 0)
        {
            uint32_t Repeat = pData->Repeat + 1;
            uint32_t Pixel  = pData->Pixel;
            pData++;

            while(Repeat--)
            {
                *pDestination = Pixel;
                pDestination++;
            }

            Size--;
        }
    }
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX





















/*

//-------------------------------------------------------------------------------------------------
//
//  Function name:  Write color pixels
//
//  Parameter(s):   uint16_t Color
//  Return value:   uint16_t Count
//
//  Description:    write a number of pixel with a pecific color
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::PutColor(uint16_t Color, uint16_t Count)
{
    m_pSPI->LockToDevice(IO_ST7735_CS);
    WriteCommand(ST7735_RAM_WRITE);                         // Access to RAM
    IO_SetPinHigh(IO_ST7735_DC);                            // Set memory to data
    m_pSPI->OverrideMemoryIncrement();
    m_pSPI->Write((uint8_t*)&Color, Count);                 // Write color

#if 0
    while(Count--)                                          // Counter
    {
        m_pSPI->Write(Color);                               // Write color
    }
#endif

    m_pSPI->UnlockFromDevice(IO_ST7735_CS);
}

//-------------------------------------------------------------------------------------------------
//
//  Function name:  WriteCommand
//
//  Parameter(s):   uint8_t         Command
//  Return value:   None
//
//  Description:    Send a command to LCD
//
//  Note(s):        Any sub data will be sent using LCD_WriteByte
//                  This function does not handle the CS
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteCommand(uint8_t Command)
{
    IO_SetPinLow(IO_ST7735_DC);
    m_pSPI->Write(Command);
}

//-------------------------------------------------------------------------------------------------
//
//  Function name:   ReadCommand
//
//  Parameter(s):   uint8_t         Command
//  Return value:   uint8_t         Data
//
//  Description:    Send a read command to LCD
//
//-------------------------------------------------------------------------------------------------
uint8_t GrafxDriver::ReadCommand(uint8_t Command)
{
    uint8_t Data[2];

    IO_SetPinLow(IO_ST7735_DC);         // Command (active low)
    m_pSPI->Transfer(&Command, 1, &Data[0], 2, IO_ST7735_CS);
    return Data[1];
}

//-------------------------------------------------------------------------------------------------
//
//  Function name:  ReadData
//
//  Parameter(s):   None
//  Return value:   uint8_t         Data
//
//  Description:    Read one byte from LCD
//
//-------------------------------------------------------------------------------------------------
uint8_t GrafxDriver::ReadData(void)
{
    uint8_t Data;

    IO_SetPinLow(IO_ST7735_DC);         // Data (active high)
    m_pSPI->Transfer(nullptr, 0, &Data, 1, IO_ST7735_CS);
    return Data;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint8_t         Data
//   Return value:  None
//
//   Description:   Write 8 bits data to LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint8_t Data)
{
    //IO_SetPinHigh(IO_ST7735_DC);				// Data (active high)
    m_pSPI->Write(&Data, 1);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint16_t         Data
//   Return value:  None
//
//   Description:   Write 16 bits data to LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint16_t Data)
{
    //IO_SetPinHigh(IO_ST7735_DC);			            // Data (active high)
    m_pSPI->Write((uint8_t*)&Data, 2);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint32_t         Data
//   Return value:  None
//
//   Description:   Write 32 bits data to LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint32_t Data)
{
    //IO_SetPinHigh(IO_ST7735_DC);			            // Data (active high)
    m_pSPI->Write((uint8_t*)&Data, 4);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WriteData
//
//   Parameter(s):  uint8_t*        pData
//                  uint16_t        Size
//   Return value:  None
//
//   Description:   Write a stream of data LCD
//
//-------------------------------------------------------------------------------------------------

void GrafxDriver::WriteData(uint8_t* pData, uint32_t Size)
{
    //IO_SetPinHigh(IO_ST7735_DC);        			    // Data (active high)
    m_pSPI->Write(pData, Size);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SendCommand
//
//   Parameter(s):  uint8_t         Register        Register command
//                  uint8_t         Data            Data to send
//   Return value:  None
//
//   Description:   Send a command and write a uint8_t data LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SendCommand(uint8_t Register, uint8_t Data, uint32_t Delay)
{
    SendCommand(Register, &Data, 1, Delay);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SendCommand
//
//   Parameter(s):  uint8_t         Register        Register command
//                  uint8_t*        pData           Array to send
//                  uint32_t        Size            Size of the array
//   Return value:  None
//
//   Description:   Send a command and write a stream of data LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SendCommand(uint8_t Register, uint8_t* pData, uint32_t Size, uint32_t Delay)
{
    m_pSPI->LockToDevice(IO_ST7735_CS);
    WriteCommand(Register);             // Send register

    if(Size > 0)
    {
        WriteData(pData, Size);
    }

    m_pSPI->UnlockFromDevice(IO_ST7735_CS);

    if(Delay > 0)
    {
        LIB_Delay_mSec(Delay);
    }
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           LayerConfig
//
//  Parameter(s):   DisplayLayer* pLayer
//  Return:         None
//
//  Description:    Configuration for layer
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::LayerConfig(DisplayLayer* pLayer)
{
    VAR_UNUSED(pLayer);         // This is a single layer LCD controller
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BlockCopy
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  Cartesian_t*    pDstPos
//                  PixelFormat_e   SrcPixelFormat
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from 2D memory region to another 2D memory region
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    VAR_UNUSED(BlendMode);
    VAR_UNUSED(SrcPixelFormat);         // ST7735 driver does not use SrcPixelFormat.. it should be 65K color

//    WriteCommand(SSD1779_COPY);
    WriteData(uint8_t(pBox->Pos.X));
    WriteData(uint8_t(pBox->Pos.Y));
    WriteData(uint8_t(pBox->Pos.X + pBox->Size.Width));
    WriteData(uint8_t(pBox->Pos.Y + pBox->Size.Height));
    WriteData(uint8_t(pDstPos->X));
    WriteData(uint8_t(pDstPos->Y));
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawRectangle
//
//   Parameter(s):  Box_t*    pBox
//    Return Value: none
//
//   Description:   Draw a rectangle
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawRectangle(Box_t* pBox)
{
    DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
	uint16_t Color  = (uint16_t(pLayer->GetColor()));
    SetWindow(pBox->Pos.X, pBox->Pos.Y, pBox->Pos.X + (pBox->Size.Width - 1), pBox->Pos.Y + (pBox->Size.Height - 1));
    PutColor(Color, pBox->Size.Width * pBox->Size.Height);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawBox
//
//  Parameter(s):   uint16_t    PosX
//                  uint16_t    PosY
//                  uint16_t    Width
//                  uint16_t    Height
//                  uint16_t    Thickness
//  Return:         None
//
//  Description:    Draw a box in a specific Thickness
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t Thickness)
{
    DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
	uint16_t Color  = (uint16_t(pLayer->GetColor()));

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint16_t X2 = PosX + Width;
        uint16_t Y2 = PosY + Height;

        DrawVLine(PosX,           PosY, Y2, Thickness);
        DrawVLine(X2 - Thickness, PosY, Y2, Thickness);
        DrawHLine(PosY,           PosX, X2, Thickness);
        DrawHLine(Y2 - Thickness, PosX, X2, Thickness);
    }
    else
    {
        // Left Side
        SetWindow(PosX, PosY, PosX + (Thickness - 1), PosY + Height);
        PutColor(Color, Width * Height);

        // Right Side
        SetWindow(PosX + (Width - (Thickness - 1)), PosY, PosX + Width, PosY + Height);
        PutColor(Color, Width * Height);

        // Top
        SetWindow(PosX + Thickness, PosY, PosX + (Width - Thickness), PosY + (Thickness - 1));
        PutColor(Color, Width * Height);

        // Bottom
        SetWindow(PosX + Thickness, PosY + (Height - (Thickness - 1)), PosX + (Width - Thickness), PosY + Height);
        PutColor(Color, Width * Height);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawPixel
//
//  Parameter(s):   uint16_t    PosX
//                  uint16_t    PosY
//  Return:         None
//
//  Description:    Put a pixel on LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawPixel(uint16_t PosX, uint16_t PosY)
{
    DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    uint16_t Color  = (uint16_t(pLayer->GetColor()));

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint16_t* pAddress = (uint16_t*)pLayer->GetAddress() + (((PosY * GRAFX_DRIVER_SIZE_X) + PosX) * 2);
        *pAddress = Color;
    }
    else
    {
        SetWindow(PosX, PosY, PosX, PosY);
        PutColor(Color, 1);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawHLine
//
//  Parameter(s):   uint16_t    Y
//                  uint16_t    X1
//                  uint16_t    X2
//                  uint16_t    Thickness
//  Return:         None
//
//  Description:    Displays a horizontal line of a specific Thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawHLine(uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)
{
    uint16_t Temp;
	uint16_t Length;

    // X1 need to be the lowest
    if(PosX1 > PosX2)
    {
        Temp  = PosX1;
        PosX1 = PosX2;
        PosX2 = Temp;
    }

    Length = (PosX2 - PosX1) + 1;

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        this->DrawLine(PosX1, PosY, Length, Thickness, DRAW_HORIZONTAL);
    }
    else
    {
        DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
        uint16_t Color  = (uint16_t(pLayer->GetColor()));
        SetWindow(PosX1, PosY, PosX2, PosY + (Thickness - 1));
        PutColor(Color, Length * Thickness);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawVLine
//
//  Parameter(s):   uint16_t    wPosX
//                  uint16_t    wPosY1
//                  uint16_t    wPosY2
//                  uint16_t    wThickness
//  Return:         None
//
//  Description:    Displays a vertical line of a specific Thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawVLine(uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)
{
    uint16_t Temp;
	uint16_t Length;

    // Y1 need to be the lowest
    if(PosY1 > PosY2)
    {
        Temp  = PosY1;
        PosY1 = PosY2;
		PosY2 = Temp;
    }

    Length = PosY2 - PosY1;

	if(DisplayLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        this->DrawLine(PosX, PosY1, Length, Thickness, DRAW_VERTICAL);
    }
    else
    {
        DisplayLayer*  pLayer = &LayerTable[DisplayLayer::GetDrawing()];
        uint16_t Color  = (uint16_t(pLayer->GetColor()));
        SetWindow(PosX, PosY1, PosX + (Thickness - 1), PosY2);
        PutColor(Color, Length * Thickness);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintFont
//
//  Parameter(s):   FONT_sDescriptor*   pDescriptor
//                  Cartesian_t*        pPos
//  Return:         none
//
//  Description:    This function will print a font to drawing layer with the drawing color
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::PrintFont(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)
{
    uint8_t            PixelSize;
    uint32_t           Address;
    DisplayLayer*            pLayer;
    uint32_t           PixelFormat;
    struct32_t         AreaConfig;

    pLayer             = &LayerTable[DisplayLayer::GetDrawing()];
    PixelFormat        = m_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize          = pLayer->GetPixelSize();
    Address            = pLayer->GetAddress() + (((pPos->Y * GRAFX_DRIVER_SIZE_X) + pPos->X) * (uint32_t)PixelSize);
    AreaConfig.u_16.u1 = pDescriptor->Size.Width;
    AreaConfig.u_16.u0 = pDescriptor->Size.Height;

    // TODO provide a method without DMA2D
    // use memory to memory normal DMA
    VAR_UNUSED(Address);
    VAR_UNUSED(PixelFormat);
    VAR_UNUSED(AreaConfig);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Clear
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear and home the LCD
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Clear(void)
{
	SetWindow(0, 0, GRAFX_DRIVER_SIZE_X, GRAFX_DRIVER_SIZE_Y);      // Use the whole LCD
	PutColor(RED, GRAFX_RAM_SIZE_X * GRAFX_RAM_SIZE_Y); 	        // Draw individual pixels
}

//-------------------------------------------------------------------------------------------------

#if 0

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void Adafruit_ST77xx::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m % 4; // can't be higher than 3

  switch (rotation) {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 1:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  case 2:
    madctl = ST77XX_MADCTL_RGB;
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 3:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}


/**
 * @desc    Set partial area / window
 *
 * @param   uint8_t x - start row
 * @param   uint8_t x - end row
 *
 * @return  uint8_t
 */
uint8_t ST7735_SetPartialArea(uint8_t sRow, uint8_t eRow)
{
  // check if coordinates is out of range
  if ((sRow > SIZE_Y) ||
      (eRow > SIZE_Y)) {
    // out of range
    return 0;
  }
  // column address set
  ST7735_CommandSend(PTLAR);
  // start start Row
  ST7735_Data8BitsSend(0x00);
  // start start Row
  ST7735_Data8BitsSend(sRow);
  // row end Row
  ST7735_Data8BitsSend(0x00);
  // end end Row
  ST7735_Data8BitsSend(eRow);
  // column address set
  ST7735_CommandSend(PTLON);
  // success
  return 1;
}

/**
 * @desc    Set text position x, y
 *
 * @param   uint8_t x - position
 * @param   uint8_t y - position
 *
 * @return  void
 */
char ST7735_SetPosition(uint8_t x, uint8_t y)
{
  // check if coordinates is out of range
  if ((x > MAX_X) && (y > MAX_Y)) {
    // error
    return ST7735_ERROR;

  } else if ((x > MAX_X) && (y <= MAX_Y)) {
    // set position y
    cacheMemIndexRow = y;
    // set position x
    cacheMemIndexCol = 2;
  } else {
    // set position y
    cacheMemIndexRow = y;
    // set position x
    cacheMemIndexCol = x;
  }
  // success
  return ST7735_SUCCESS;
}

/**
 * @desc    Check text position x, y
 *
 * @param   unsigned char x - position
 * @param   unsigned char y - position
 * @param   unsigned char
 *
 * @return  char
 */
char ST7735_CheckPosition(unsigned char x, unsigned char y, unsigned char max_y, ESizes size)
{
  // check if coordinates is out of range
  if ((x > MAX_X) && (y > max_y)) {
    // out of range
    return ST7735_ERROR;

  }
  // if next line
  if ((x > MAX_X) && (y <= max_y)) {
    // set position y
    cacheMemIndexRow = y;
    // set position x
    cacheMemIndexCol = 2;
  }

  // success
  return ST7735_SUCCESS;
}

#endif


















#if 0 //7735B

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SWRESET);                               // Software reset
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(50);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SLPOUT);                                // Out Of Sleep
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_COLMOD);                                // Set color mode
    WriteData(uint8_t(0x05));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_FRMCTR1);           // Frame rate control
    WriteData(uint8_t(0x00));               //     fastest refresh
    WriteData(uint8_t(0x06));               //     6 lines front porch
    WriteData(uint8_t(0x03));               //     3 lines back porch
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

      // D7  D6  D5  D4  D3  D2  D1  D0
      // MY  MX  MV  ML RGB  MH   -   -
      // ------------------------------
      // ------------------------------
      // MV  MX  MY -> {MV (row / column exchange) MX (column address order), MY (row address order)}
      // ------------------------------
      //  0   0   0 -> begin left-up corner, end right-down corner left-right (normal view)
      //  0   0   1 -> begin left-down corner, end right-up corner left-right (Y-mirror)
      //  0   1   0 -> begin right-up corner, end left-down corner right-left (X-mirror)
      //  0   1   1 -> begin right-down corner, end left-up corner right-left (X-mirror, Y-mirror)
      //  1   0   0 -> begin left-up corner, end right-down corner up-down (X-Y exchange)
      //  1   0   1 -> begin left-down corner, end right-up corner down-up (X-Y exchange, Y-mirror)
      //  1   1   0 -> begin right-up corner, end left-down corner up-down (X-Y exchange, X-mirror)
      //  1   1   1 -> begin right-down corner, end left-up corner
      //               down-up (X-Y exchange, X-mirror, Y-mirror)
      // ------------------------------
      //  ML: vertical refresh order
      //      0 -> refresh top to bottom
      //      1 -> refresh bottom to top
      // ------------------------------
      // RGB: filter panel
      //      0 -> RGB
      //      1 -> BGR
      // ------------------------------
      //  MH: horizontal refresh order
      //      0 -> refresh left to right
      //      1 -> refresh right to left
      // 0xA0 = 1010 0000
IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_MADCTL);                                // Set memory data access control
    WriteData(uint8_t(0x08));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low


IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_DISSET5);
    WriteData(uint8_t(0x15));
    WriteData(uint8_t(0x02));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_INVCTR);
  WriteData(uint8_t(0x00));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_1);
  WriteData(uint8_t(0x02));
  WriteData(uint8_t(0x70));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_2);
  WriteData(uint8_t(0x05));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_3);
  WriteData(uint8_t(0x01));
  WriteData(uint8_t(0x02));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_VMCTR1);
  WriteData(uint8_t(0x10));
  //WriteData(uint8_t(0x3C));
  //WriteData(uint8_t(0x38));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_6);
  WriteData(uint8_t(0x11));
  WriteData(uint8_t(0x15));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRP1);
  WriteData(uint8_t(0x09)); WriteData(uint8_t(0x16)); WriteData(uint8_t(0x09)); WriteData(uint8_t(0x20));
  WriteData(uint8_t(0x21)); WriteData(uint8_t(0x1B)); WriteData(uint8_t(0x13)); WriteData(uint8_t(0x19));
  WriteData(uint8_t(0x17)); WriteData(uint8_t(0x15)); WriteData(uint8_t(0x1E)); WriteData(uint8_t(0x2B));
  WriteData(uint8_t(0x04)); WriteData(uint8_t(0x05)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x0E));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRN1);
  WriteData(uint8_t(0x0B)); WriteData(uint8_t(0x14)); WriteData(uint8_t(0x08)); WriteData(uint8_t(0x1E));
  WriteData(uint8_t(0x22)); WriteData(uint8_t(0x1D)); WriteData(uint8_t(0x18)); WriteData(uint8_t(0x1E));
  WriteData(uint8_t(0x1B)); WriteData(uint8_t(0x1A)); WriteData(uint8_t(0x24)); WriteData(uint8_t(0x2B));
  WriteData(uint8_t(0x06)); WriteData(uint8_t(0x06)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x0F));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_CASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0x81));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_RASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0x81));
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_NORON);
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_DISPON);                                // Main screen turn on
IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

#endif


#if 0 //7735R
static uint8_t Test[4];

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SWRESET);                           // Software reset
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(50);


//IO_SetPinLow(IO_ST7735_CS);
//WriteCommand(ST7735_RDID_1);
//Test[0] = ReadData();
//IO_SetPinHigh(IO_ST7735_CS);
/*
IO_SetPinLow(IO_ST7735_CS);
WriteCommand(ST7735_RDID_2);
Test[1] = ReadData();
IO_SetPinHigh(IO_ST7735_CS);

IO_SetPinLow(IO_ST7735_CS);
WriteCommand(ST7735_RDID_3);
Test[2] = ReadData();
IO_SetPinHigh(IO_ST7735_CS);

IO_SetPinLow(IO_ST7735_CS);
WriteCommand(ST7735_RDID_4);
Test[3] = ReadData();
IO_SetPinHigh(IO_ST7735_CS);
*/


//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_SLPOUT);                                // Out Of Sleep
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_COLMOD);                                // Set color mode
    WriteData(uint8_t(0x05));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_FRMCTR1);           // Frame rate control - normal Rate = fosc/(1x2+40) * (LINE+2C+2D)
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    WriteData(uint8_t(0x01));
    WriteData(uint8_t(0x2C));
    WriteData(uint8_t(0x2D));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_FRMCTR2);           // Frame rate control - idle mode = fosc/(1x2+40) * (LINE+2C+2D)
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    WriteData(uint8_t(0x01));
    WriteData(uint8_t(0x2C));
    WriteData(uint8_t(0x2D));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
   WriteCommand(ST7735_FRMCTR3);           // Frame rate control - partial mode = fosc/(1x2+40) * (LINE+2C+2D)
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x05));
    //WriteData(uint8_t(0x3C));
    //WriteData(uint8_t(0x3C));
    WriteData(uint8_t(0x01));
    WriteData(uint8_t(0x2C));
    WriteData(uint8_t(0x2D));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_INVCTR);
  WriteData(uint8_t(0x03));
//  WriteData(uint8_t(0x07));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_1);
  WriteData(uint8_t(0x28));
  WriteData(uint8_t(0x08));
  WriteData(uint8_t(0x04));
//  WriteData(uint8_t(0xA2));
//  WriteData(uint8_t(0x02));
//  WriteData(uint8_t(0x84));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_2);
  WriteData(uint8_t(0xC0));
//  WriteData(uint8_t(0xC5));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_3);
//  WriteData(uint16_t(0x0D00));
  WriteData(uint16_t(0x0A00));
//  WriteData(uint8_t(0x0A));
//  WriteData(uint8_t(0x00));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_4);
//  WriteData(uint16_t(0x2A8D));
  WriteData(uint16_t(0x2A8A));
//  WriteData(uint8_t(0x8A));
//  WriteData(uint8_t(0x2A));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_POWER_CTRL_5);
  //WriteData(uint16_t(0xEE8D));
  WriteData(uint16_t(0xEE8A));
//  WriteData(uint8_t(0x8A));
//  WriteData(uint8_t(0xEE));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_VMCTR1);
  WriteData(uint8_t(0x0E));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_INVOFF);
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

      // D7  D6  D5  D4  D3  D2  D1  D0
      // MY  MX  MV  ML RGB  MH   -   -
      // ------------------------------
      // ------------------------------
      // MV  MX  MY -> {MV (row / column exchange) MX (column address order), MY (row address order)}
      // ------------------------------
      //  0   0   0 -> begin left-up corner, end right-down corner left-right (normal view)
      //  0   0   1 -> begin left-down corner, end right-up corner left-right (Y-mirror)
      //  0   1   0 -> begin right-up corner, end left-down corner right-left (X-mirror)
      //  0   1   1 -> begin right-down corner, end left-up corner right-left (X-mirror, Y-mirror)
      //  1   0   0 -> begin left-up corner, end right-down corner up-down (X-Y exchange)
      //  1   0   1 -> begin left-down corner, end right-up corner down-up (X-Y exchange, Y-mirror)
      //  1   1   0 -> begin right-up corner, end left-down corner up-down (X-Y exchange, X-mirror)
      //  1   1   1 -> begin right-down corner, end left-up corner
      //               down-up (X-Y exchange, X-mirror, Y-mirror)
      // ------------------------------
      //  ML: vertical refresh order
      //      0 -> refresh top to bottom
      //      1 -> refresh bottom to top
      // ------------------------------
      // RGB: filter panel
      //      0 -> RGB
      //      1 -> BGR
      // ------------------------------
      //  MH: horizontal refresh order
      //      0 -> refresh left to right
      //      1 -> refresh right to left
      // 0xA0 = 1010 0000
//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_MADCTL);                                // Set memory data access control
    //WriteData(uint8_t(0xC0));
    WriteData(uint8_t(0xC8));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_COLMOD);
    WriteData(uint8_t(0x05));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_CASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x02)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0x81));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_RASET);
  WriteData(uint8_t(0x00)); WriteData(uint8_t(0x01)); WriteData(uint8_t(0x00)); WriteData(uint8_t(0xA0));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRP1);
  //WriteData(uint32_t(0x0A072204));
  //WriteData(uint32_t(0x2A25302E));
  //WriteData(uint32_t(0x3A2E2628));
  //WriteData(uint32_t(0x13030100));
  WriteData(uint32_t(0x12071C02));
  WriteData(uint32_t(0x2D293237));
  WriteData(uint32_t(0x392B2529));
  WriteData(uint32_t(0x10030100));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
  WriteCommand(ST7735_GMCTRN1);
  //WriteData(uint32_t(0x0D061604));
  //WriteData(uint32_t(0x2723262D));
  //WriteData(uint32_t(0x3B2D2527));
  //WriteData(uint32_t(0x13040100));
  WriteData(uint32_t(0x06071D03));
  WriteData(uint32_t(0x2D292C2E));
  WriteData(uint32_t(0x3F372E2E));
  WriteData(uint32_t(0x10020000));
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
  LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_NORON);
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(10);

//IO_SetPinLow(IO_ST7735_CS);				    // Chip enable - active low
    WriteCommand(ST7735_DISPON);                                // Main screen turn on
//IO_SetPinHigh(IO_ST7735_CS);				// Chip enable - active low
    LIB_Delay_mSec(500);

#endif

*/
