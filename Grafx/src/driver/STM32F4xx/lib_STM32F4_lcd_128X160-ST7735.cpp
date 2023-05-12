//-------------------------------------------------------------------------------------------------
//
//  File : lib_lcd_128X160-ST7735.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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
#include "lib_digini.h"
#undef  LIB_ST7735_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Using DMA2D for virtual layer
#define LTDC_PIXEL_FORMAT_ARGB8888      0
#define LTDC_PIXEL_FORMAT_RGB888        1
#define LTDC_PIXEL_FORMAT_RGB565        2
#define LTDC_PIXEL_FORMAT_ARGB1555      3
#define LTDC_PIXEL_FORMAT_ARGB4444      4
#define LTDC_PIXEL_FORMAT_L8            5
#define LTDC_PIXEL_FORMAT_AL44          6
#define LTDC_PIXEL_FORMAT_AL88          7
#define LTDC_PIXEL_FORMAT_L4            8
#define LTDC_PIXEL_FORMAT_A8            9
#define LTDC_PIXEL_FORMAT_A4            10

#define LTDC_BLENDING_FACTOR1_PAxCA     0x00000600              // Blending factor: Cte Alpha x Pixel Alpha
#define LTDC_BLENDING_FACTOR2_PAxCA     0x00000007              // Blending factor: Cte Alpha x Pixel Alpha

#define DMA2D_M2M                       0                       // DMA2D memory to memory transfer mode
#define DMA2D_M2M_PFC                   ((uint32_t)0x00010000)  // DMA2D memory to memory with pixel format conversion transfer mode
#define DMA2D_M2M_BLEND                 ((uint32_t)0x00020000)  // DMA2D memory to memory with blending transfer mode
#define DMA2D_R2M                       DMA2D_CR_MODE           // DMA2D register to memory transfer mode

//---------------------------------------

#define DELAY                           0x80

#define ST7735_NOP                      0x00                    // No operation
#define ST7735_SWRESET                  0x01                    // Software Reset
#define ST7735_RDDID                    0x04                    // Read Display ID
#define ST7735_RDDST                    0x09                    // Read Display Status

#define ST7735_SLPIN                    0x10                    // Sleep In & Booster OFF
#define ST7735_SLPOUT                   0x11                    // Sleep Out & Booster ON
#define ST7735_PTLON                    0x12                    // Partial Mode ON
#define ST7735_NORON                    0x13                    // Partial Mode OFF (Normal)
#define ST7735_INVOFF                   0x20                    // Display Inversion OFF
#define ST7735_INVON                    0x21                    // Display Inversion ON
#define ST7735_DISPOFF                  0x28                    // Display OFF
#define ST7735_DISPON                   0x29                    // Display ON
#define ST7735_CASET                    0x2A                    // Column Address Set
#define ST7735_RAMRD                    0x2E                    // Memory read
#define ST7735_RASET                    0x2B                    // Row Address Set
#define ST7735_RAM_WRITE                0x2C                    // Memory Write

#define ST7735_PTLAR                    0x30                    // Partial Start/End Address Set
#define ST7735_MADCTL                   0x36                    // Memory Data Access Control
#define ST7735_COLMOD                   0x3A                    // Interface Pixel Format

#define ST7735_FRMCTR1                  0xB1                    //
#define ST7735_FRMCTR2                  0xB2                    //
#define ST7735_FRMCTR3                  0xB3                    //
#define ST7735_INVCTR                   0xB4                    //
#define ST7735_DISSET5                  0xB6                    //

#define ST7735_RDID_1                   0xDA                    // Read ID1
#define ST7735_RDID_2                   0xDB                    // Read ID2
#define ST7735_RDID_3                   0xDC                    // Read ID3
#define ST7735_RDID_4                   0xDD                    // Read ID4

#define ST7735_GMCTRP1                  0xE0                    //
#define ST7735_GMCTRN1                  0xE1                    //

#define ST7735_POWER_CTRL_1             0xC0                    //
#define ST7735_POWER_CTRL_2             0xC1                    //
#define ST7735_POWER_CTRL_3             0xC2                    //
#define ST7735_POWER_CTRL_4             0xC3                    //
#define ST7735_POWER_CTRL_5             0xC4                    //
#define ST7735_POWER_CTRL_6             0xFC                    //
#define ST7735_VMCTR1                   0xC5                    //

// Colors
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define BLUE    0x00F8
#define GREEN   0x0707

#define ST7735_SUCCESS 0
#define ST7735_ERROR   1

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

static const int32_t DRV_PixelFormatTable[PIXEL_FORMAT_COUNT] =
{
  #ifdef GRAFX_COLOR_ARGB8888
    LTDC_PIXEL_FORMAT_ARGB8888,
  #endif
  #ifdef GRAFX_COLOR_RGB888
    -1,
  #endif
  #ifdef GRAFX_COLOR_RGB565
    LTDC_PIXEL_FORMAT_RGB565,
  #endif
  #ifdef GRAFX_COLOR_ARGB1555
    -1,
  #endif
  #ifdef GRAFX_COLOR_ARGB4444
    -1,
  #endif
  #ifdef GRAFX_COLOR_L8
    -1,
  #endif
  #ifdef GRAFX_COLOR_AL44
    -1,
  #endif
  #ifdef GRAFX_COLOR_AL88
    -1,
  #endif
  #ifdef GRAFX_COLOR_L4
    -1,
  #endif
  #ifdef GRAFX_COLOR_A8
    LTDC_PIXEL_FORMAT_A8,
  #endif
  #ifdef GRAFX_COLOR_A4
    -1,
  #endif
  #ifdef GRAFX_COLOR_RGB332
    -1,
  #endif
  #ifdef GRAFX_COLOR_RGB444
    -1,
  #endif
    -1
};

//-------------------------------------------------------------------------------------------------
// Private Function(s)
//-------------------------------------------------------------------------------------------------

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
    IO_SetPinLow(IO_ST7735_CS);
    this->WriteCommand(ST7735_RAM_WRITE);                         // Access to RAM
    IO_SetPinHigh(IO_ST7735_DC);                            // Set memory to data
    m_pSPI->OverrideMemoryIncrement();
    m_pSPI->Transfer(&Color, Count, nullptr, 0, this);       // write color

#if 0
    // Counter
    while(Count--)
    {
        // Write color
        m_pSPI->Transfer(&Color, 1, nullptr, 0, this);
    }
#endif

    IO_SetPinHigh(IO_ST7735_CS);
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
//  Note(s):        any sub data will be sent using LCD_WriteByte
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteCommand(uint8_t Command)
{
    IO_SetPinLow(IO_ST7735_DC);
    m_pSPI->Transfer(&Command, 1, nullptr, 0, this);
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
    m_pSPI->Transfer(&Command, 1, &Data[0], 2, this);
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
    m_pSPI->Transfer(nullptr, 0, &Data, 1, this);
    return Data;
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SetWindow(uint8_t PosX1, uint8_t PosY1, uint8_t PosX2, uint8_t PosY2)
{
    uint8_t Data[4] = {0,0,0,0};

    Data[1] = PosX1;                // X start
    Data[3] = PosX2;                // X end
    this->SendCommand(ST7735_CASET, &Data[0], 4, 1);
    Data[1] = PosY1;                // Y start
    Data[3] = PosY2;                // Y end
    this->SendCommand(ST7735_RASET, &Data[0], 4, 1);
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint8_t Data)
{
    IO_SetPinHigh(IO_ST7735_DC);				// Data (active high)
    m_pSPI->Transfer(&Data, 1, nullptr, 0, this);
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint16_t Data)
{
    IO_SetPinHigh(IO_ST7735_DC);			            // Data (active high)
    m_pSPI->Transfer((uint8_t*)&Data, 2, nullptr, 0, this);
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::WriteData(uint32_t Data)
{
    IO_SetPinHigh(IO_ST7735_DC);			            // Data (active high)
    m_pSPI->Transfer((uint8_t*)&Data, 4, nullptr, 0, this);
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------

void GrafxDriver::WriteData(uint8_t* pData, uint32_t Size)
{
    IO_SetPinHigh(IO_ST7735_DC);        			    // Data (active high)
    m_pSPI->Transfer(pData, Size, nullptr, 0, this);
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SendCommand(uint8_t Register, uint8_t Data, uint32_t Delay)
{
    this->SendCommand(Register, &Data, 1, Delay);
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
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::SendCommand(uint8_t Register, uint8_t* pData, uint32_t Size, uint32_t Delay)
{
    IO_SetPinLow(IO_ST7735_CS);
    this->WriteCommand(Register);             // Send register

    if(Size > 0)
    {
        this->WriteData(pData, Size);
    }

    IO_SetPinHigh(IO_ST7735_CS);

    if(Delay > 0)
    {
        LIB_Delay_mSec(Delay);
    }
}

//-------------------------------------------------------------------------------------------------
// Global Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void* Arg           ( to pass any kind data the driver might need
//  Return:         None
//
//  Description:    LCD configuration specific
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------

void GrafxDriver::Initialize(void* pArg)
{
    m_pSPI = (SPI_Driver*)pArg;

    IO_PinInit(IO_ST7735_CS);
    IO_PinInit(IO_ST7735_DC);
    IO_PinInit(IO_ST7735_RESET);
    IO_PinInit(IO_ST7735_BACKLIGHT);

    m_pSPI->Initialize();                                       // CS(NSS) handle by class

    LIB_Delay_mSec(1);                                              // Hold Reset at least 10 uSec
    IO_SetPinHigh(IO_ST7735_RESET);
    LIB_Delay_mSec(50);

    this->SendCommand(ST7735_SWRESET, nullptr, 0, 50);            // Software reset

    //------------------------------------ST7735S Frame Rate-----------------------------------------//

    const uint8_t Data_FRMCTR[6] = {0x05, 0x3C, 0x3C,0x05, 0x3C, 0x3C};
    this->SendCommand(ST7735_FRMCTR1, (uint8_t*)&Data_FRMCTR[0], 3, 1);    // Frame control 1
    this->SendCommand(ST7735_FRMCTR2, (uint8_t*)&Data_FRMCTR[0], 3, 1);    // Frame control 2
    this->SendCommand(ST7735_FRMCTR3, (uint8_t*)&Data_FRMCTR[0], 6, 1);    // Frame control 2

	//------------------------------------End ST7735S Frame Rate-----------------------------------------//

    this->SendCommand(ST7735_INVCTR, 0x03, 1);                   // Dot inversion

	//------------------------------------ST7735S Power Sequence-----------------------------------------//

    const uint8_t Data_PWR_CTRL1[3] = {0x28, 0x08, 0x04};
    const uint8_t Data_PWR_CTRL3[2] = {0x0D, 0x00};
    const uint8_t Data_PWR_CTRL4[2] = {0x8D, 0x2A};
    const uint8_t Data_PWR_CTRL5[2] = {0x8D, 0xEE};

    this->SendCommand(ST7735_POWER_CTRL_1, (uint8_t*)&Data_PWR_CTRL1[0], 3, 1);
    this->SendCommand(ST7735_POWER_CTRL_2, 0xC0, 1);
    this->SendCommand(ST7735_POWER_CTRL_3, (uint8_t*)&Data_PWR_CTRL3[0], 2, 1);
    this->SendCommand(ST7735_POWER_CTRL_4, (uint8_t*)&Data_PWR_CTRL4[0], 2, 1);
    this->SendCommand(ST7735_POWER_CTRL_5, (uint8_t*)&Data_PWR_CTRL5[0], 2, 1);

    //---------------------------------End ST7735S Power Sequence-------------------------------------//

    this->SendCommand(ST7735_VMCTR1, 0x10, 1);                        // VCOM
    this->SendCommand(ST7735_MADCTL, 0xC0, 1);                        // MX, MY, RGB mode

	//------------------------------------ST7735S Gamma Sequence-----------------------------------------//

    const uint8_t Data_GMCTRP1[16] = {0x04, 0x22, 0x07, 0x0A, 0x2E, 0x30, 0x25, 0x2A,
                                      0x28, 0x26, 0x2E, 0x3A, 0x00, 0x01, 0x03, 0x13};
    const uint8_t Data_GMCTRN1[16] = {0x04, 0x16, 0x06, 0x0D, 0x2D, 0x26, 0x23, 0x27,
                                      0x27, 0x25, 0x2D, 0x3B, 0x00, 0x01, 0x04, 0x13};

    this->SendCommand(ST7735_GMCTRP1, (uint8_t*)&Data_GMCTRP1[0], 16, 1);
    this->SendCommand(ST7735_GMCTRN1, (uint8_t*)&Data_GMCTRN1[0], 16, 1);

	//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//

    this->SendCommand(ST7735_COLMOD, 0x05, 1);                        // 65K mode
    this->SendCommand(ST7735_SLPOUT, nullptr, 0, 120);                // Exit Sleep
    this->DisplayOn();
    this->Clear();
    IO_SetPinHigh(IO_ST7735_BACKLIGHT);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LayerConfig
//
//  Parameter(s):   CLayer* pLayer
//  Return:         None
//
//  Description:    Configuration for layer
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::LayerConfig(CLayer* pLayer)
{
    VAR_UNUSED(pLayer); // This is a single layer LCD controller
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DRV_Copy
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  Cartesian_t*    pDstPos
//                  PixelFormat_e   SrcPixelFormat_e
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from square memory region to another square memory
//                  region
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Copy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat_e, BlendMode_e BlendMode)
{
	if(CLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint32_t           PixelFormatSrc;
        uint32_t           PixelFormatDst;
        uint32_t           Address;
        struct32_t         AreaConfig;
        CLayer*            pLayer;
        uint8_t            PixelSize;

        pLayer             = &LayerTable[CLayer::GetDrawing()];
        PixelFormatSrc     = DRV_PixelFormatTable[SrcPixelFormat_e];
        PixelFormatDst     = DRV_PixelFormatTable[pLayer->GetPixelFormat()];
        PixelSize          = pLayer->GetPixelSize();
        Address            = pLayer->GetAddress() + (((pDstPos->Y * GRAFX_SIZE_X) + pDstPos->X) * (uint32_t)PixelSize);

        AreaConfig.u_16.u1 = pBox->Size.Width;
        AreaConfig.u_16.u0 = pBox->Size.Height;


        DMA2D->CR          = (BlendMode == CLEAR_BLEND) ? 0x00000000UL : 0x00020000UL;                                  // Memory to memory and TCIE blending BG + Source
        DMA2D->CR         |= (1 << 9);

        //Source
        DMA2D->FGMAR       = (uint32_t)(pSrc) +(((pBox->Pos.Y * GRAFX_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);    // Source address
        DMA2D->FGOR        = (uint32_t)GRAFX_SIZE_X - (uint32_t)pBox->Size.Width;                                       // Source line offset none as we are linear
        DMA2D->FGPFCCR     = PixelFormatSrc;                                                                            // Defines the size of pixel

        // Source
        DMA2D->BGMAR       = Address;                                                                                   // Source address
        DMA2D->BGOR        = (uint32_t)GRAFX_SIZE_X - (uint32_t)pBox->Size.Width;                                       // Source line offset
        DMA2D->BGPFCCR     = PixelFormatDst;                                                                            // Defines the size of pixel

        //Destination
        DMA2D->OMAR        = Address;                                                                                   // Destination address
        DMA2D->OOR         = (uint32_t)GRAFX_SIZE_X - (uint32_t)pBox->Size.Width;                                       // Destination line offset none as we are linear
        DMA2D->OPFCCR      = PixelFormatDst;                                                                            // Defines the size of pixel

        DMA2D->NLR         = AreaConfig.u_32;                                                                           // Size configuration of area to be transfered
        DMA2D->CR         |= 1;                                                                                         // Start operation

        while(DMA2D->CR & DMA2D_CR_START);                                                                              // Wait until transfer is done
    }
    else
    {

    }
}


void GrafxDriver::CopyLinear(void* pSrc, Box_t* pBox, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    uint32_t           PixelFormatSrc;
    uint32_t           PixelFormatDst;
    uint32_t           Address;
    struct32_t         AreaConfig;
    CLayer*            pLayer;
    uint8_t            PixelSize;

    pLayer             = &LayerTable[CLayer::GetDrawing()];
    PixelFormatSrc     = DRV_PixelFormatTable[SrcPixelFormat];
    PixelFormatDst     = DRV_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize          = pLayer->GetPixelSize();
    Address            = pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);

    AreaConfig.u_16.u1 = pBox->Size.Width;
    AreaConfig.u_16.u0 = pBox->Size.Height;

    DMA2D->CR          = (BlendMode == CLEAR_BLEND) ? 0x00000000UL : 0x00020000UL;         // Memory to memory and TCIE blending BG + Source
    DMA2D->CR         |= (1 << 9);

    // Source
    DMA2D->FGMAR       = (uint32_t)pSrc;                                                   // Source address
    DMA2D->FGOR        = 0;                                                                // Source line offset none as we are linear
    DMA2D->FGPFCCR     = PixelFormatSrc;                                                   // Defines the size of pixel

    // Source
    DMA2D->BGMAR       = Address;                                                          // Source address
    DMA2D->BGOR        = (uint32_t)GRAFX_SIZE_X - (uint32_t)pBox->Size.Width;              // Source line offset
    DMA2D->BGPFCCR     = PixelFormatDst;                                                   // Defines the size of pixel

    // Destination
    DMA2D->OMAR        = Address;                                                          // Destination address
    DMA2D->OOR         = (uint32_t)GRAFX_SIZE_X - (uint32_t)pBox->Size.Width;              // Destination line offset
    DMA2D->OPFCCR      = PixelFormatDst;                                                   // Defines the size of pixel

    DMA2D->NLR         = AreaConfig.u_32;                                                  // Size configuration of area to be transfered
    DMA2D->CR         |= 1;                                                                // Start operation

    while(DMA2D->CR & DMA2D_CR_START);                                                     // Wait until transfer is done
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           BlockCopy
//
//  Parameter(s):   void*           pSrc
//                  uint16_t        X
//                  uint16_t        Y
//                  uint16_t        Width
//                  uint16_t        Height
//                  uint16_t        DstX
//                  uint16_t        DstY
//                  PixelFormat_e   SrcPixelFormat_e
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from square memory region to another square memory
//                  region
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::BlockCopy(void* pSrc, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    Box_t       Box;
    Cartesian_t Pos;

    Box.Pos.X       = X;
    Box.Pos.Y       = Y;
    Box.Size.Width  = Width;
    Box.Size.Height = Height;
    Pos.X           = DstX;
    Pos.Y           = DstY;

    this->BlockCopy(pSrc, &Box, &Pos, SrcPixelFormat, BlendMode);
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
  	//uint16_t Color;

/* TODO problem here
    m_pLayer = &LayerTable[CLayer::GetDrawing()];
	Color    = (uint16_t(m_pLayer->GetColor()));

	if(CLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint32_t           PixelFormat;
        uint32_t           Address;
        struct32_t         AreaConfig;
        uint8_t            PixelSize;

        PixelFormat        = DRV_PixelFormatTable[m_pLayer->GetPixelFormat()];
        PixelSize          = m_pLayer->GetPixelSize();
        Address            = m_pLayer->GetAddress() + (((pBox->Pos.Y * GRAFX_SIZE_X) + pBox->Pos.X) * (uint32_t)PixelSize);
        AreaConfig.u_16.u1 = pBox->Size.Width;
        AreaConfig.u_16.u0 = pBox->Size.Height;
        DMA2D->CR          = 0x00030000UL | (1 << 9);                              // Register to memory and TCIE
        DMA2D->OCOLR       = (uint32_t)Color;                                      // Color to be used
        DMA2D->OMAR        = Address;                                              // Destination address
        DMA2D->OOR         = (uint32_t)GRAFX_SIZE_X - (uint32_t)pBox->Size.Width;  // Destination line offset
        DMA2D->OPFCCR      = PixelFormat;                                          // Defines the number of pixels to be transfered
        DMA2D->NLR         = AreaConfig.u_32;                                      // Size configuration of area to be transfered
        DMA2D->CR         |= 1;                                                    // Start operation
        while(DMA2D->CR & DMA2D_CR_START);                                         // Wait until transfer is done
    }
    else
    {
        SetWindow(pBox->Pos.X, pBox->Pos.Y, pBox->Pos.X + (pBox->Size.Width - 1), pBox->Pos.Y + (pBox->Size.Height - 1));
        PutColor(Color, pBox->Size.Width * pBox->Size.Height);
    }
*/
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
//  Description:    Draw a box in a specific thickness
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, uint16_t Thickness)
{
	uint16_t Color;

    m_pLayer = &LayerTable[CLayer::GetDrawing()];
	Color    = (uint16_t(m_pLayer->GetColor()));

	if(CLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint16_t X2 = PosX + Width;
        uint16_t Y2 = PosY + Height;

        this->DrawVLine(PosX,           PosY, Y2, Thickness);
        this->DrawVLine(X2 - Thickness, PosY, Y2, Thickness);
        this->DrawHLine(PosY,           PosX, X2, Thickness);
        this->DrawHLine(Y2 - Thickness, PosX, X2, Thickness);
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
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawPixel(uint16_t PosX, uint16_t PosY)
{
	uint16_t Color;

    CLayer*   pLayer  = &LayerTable[CLayer::GetDrawing()];;
    Color = (uint16_t(m_pLayer->GetColor()));

	if(CLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        uint16_t* pAddress = (uint16_t*)pLayer->GetAddress() + (((PosY * GRAFX_SIZE_X) + PosX) * 2);
        *pAddress = Color;
    }
    else
    {
        m_pLayer = &LayerTable[CLayer::GetDrawing()];
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
//                  uint16_t    ThickNess
//  Return:         None
//
//  Description:    Displays a horizontal line of a specific thickness.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawHLine(uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)
{
	uint16_t Color;
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

	if(CLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        this->DrawLine(PosX1, PosY, Length, Thickness, DRAW_HORIZONTAL);
    }
    else
    {
        m_pLayer = &LayerTable[CLayer::GetDrawing()];
        Color  = (uint16_t(m_pLayer->GetColor()));
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
//                  uint16_t    wThickNess
//  Return:         None
//
//  Description:    Displays a vertical line of a specific thickness.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawVLine(uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)
{
	uint16_t Color;
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

	if(CLayer::GetDrawing() == CONSTRUCTION_FOREGROUND_LAYER)
    {
        this->DrawLine(PosX, PosY1, Length, Thickness, DRAW_VERTICAL);
    }
    else
    {
        m_pLayer = &LayerTable[CLayer::GetDrawing()];
        Color  = (uint16_t(m_pLayer->GetColor()));
        SetWindow(PosX, PosY1, PosX + (Thickness - 1), PosY2);
        PutColor(Color, Length * Thickness);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawLine
//
//  Parameter(s):   uint16_t   PosX       Specifies the X position, can be a value from 0 to 240.
//                  uint16_t   PosY       Specifies the Y position, can be a value from 0 to 320.
//                  uint16_t   Length     Line length.
//                  uint16_t   Thickness
//                  DrawMode_e Direction  line direction.
//                                        This Parameter can be one of the following values
//                                                 DRAW_HORIZONTAL
//                                                 DRAW_VERTICAL
//  Return:         None
//
//  Description:    Displays a line of a specific thickness.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawLine(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction)
{
    // No supported... need to use bresenham soft version
    //    Not supported
    VAR_UNUSED(PosX);
    VAR_UNUSED(PosY);
    VAR_UNUSED(Length);
    VAR_UNUSED(Thickness);
    VAR_UNUSED(Direction);
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
    CLayer*            pLayer;
    uint32_t           PixelFormat;
    struct32_t         AreaConfig;

    pLayer             = &LayerTable[CLayer::GetDrawing()];
    PixelFormat        = DRV_PixelFormatTable[pLayer->GetPixelFormat()];
    PixelSize          = pLayer->GetPixelSize();
    Address            = pLayer->GetAddress() + (((pPos->Y * GRAFX_SIZE_X) + pPos->X) * (uint32_t)PixelSize);
    AreaConfig.u_16.u1 = pDescriptor->Size.Width;
    AreaConfig.u_16.u0 = pDescriptor->Size.Height;

    DMA2D->CR = DMA2D_M2M_BLEND;                                                // Memory to memory blending BG + Source

    // Font layer in Alpha blending linear (A8)
    DMA2D->FGMAR   = (uint32_t)pDescriptor->pAddress;                           // Source address 1
    DMA2D->FGOR    = 0;                                                         // Font source line offset - none as we are linear
    DMA2D->FGCOLR  = pLayer->GetTextColor();
    DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_A8;                                      // Defines the number of pixels to be transfered

    DMA2D->BGMAR   = Address;                                                   // Source address 2
    DMA2D->BGOR    = (uint32_t)GRAFX_SIZE_X - (uint32_t)AreaConfig.u_16.u1;     // Font source line offset - none as we are linear
    DMA2D->BGPFCCR = PixelFormat;                                               // Defines the number of pixels to be transfered

    // Output Layer
    DMA2D->OMAR    = Address;
    DMA2D->OOR     = (uint32_t)GRAFX_SIZE_X - (uint32_t)AreaConfig.u_16.u1;     // Destination line offset
    DMA2D->OPFCCR  = PixelFormat;

    // Area
    DMA2D->NLR     = AreaConfig.u_32;                                           // Size configuration of area to be transfered

    SET_BIT(DMA2D->CR, DMA2D_CR_START);                                         // Start operation
    while(DMA2D->CR & DMA2D_CR_START){};                                        // Wait until transfer is done
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayOn
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Enables the Display
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DisplayOn(void)
{
    SendCommand(ST7735_DISPON, nullptr, 0, 1);                  // Display on
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayOff
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Disables the Display
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DisplayOff(void)
{
    SendCommand(ST7735_DISPOFF, nullptr, 0, 1);                  // Display on
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
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Clear(void)
{
	SetWindow(0, 0, GRAFX_SIZE_X, GRAFX_SIZE_Y);    // Use the whole LCD
	PutColor(RED, GRAFX_RAM_SIZE_X * GRAFX_RAM_SIZE_Y); 	    // Draw individual pixels
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

