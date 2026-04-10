//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32xxx_fmc_lcd_320x240 - SSD2119.cpp
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

#include "digini_cfg.h"
#ifdef DIGINI_USE_GRAFX
#define LIB_SSD2119_GLOBAL
#include "./lib_digini.h"
#undef  LIB_SSD2119_GLOBAL

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------


//notes: In R01h, bits REV, BGR, RL, CM will override the corresponding hardware pins settings. Setting R28h as 0x0006 is required before setting R25h and R29h registers.

const SSD2119_InitCMD_t GrafxDriver::m_InitCMD[GRAFX_NUMBER_OF_INIT_CMD] =
{
    {SSD2119_DISPLAY_CONTROL_REGISTER,        SSD2119_DISPLAY_OFF_VALUE},          // Display OFF
    {SSD2119_OSCILLATOR_START_REGISTER,       SSD2119_OSCILLATOR_ENABLE},          // Enable oscillator
    {SSD2119_POWER_CONTROL_1_REGISTER,        SSD2119_POWER_CONTROL_1_VALUE},      // Power step 1
    {SSD2119_POWER_CONTROL_2_REGISTER,        SSD2119_POWER_CONTROL_2_VALUE},      // Power step 2
    {SSD2119_POWER_CONTROL_3_REGISTER,        SSD2119_POWER_CONTROL_3_VALUE},      // Power step 3
    {SSD2119_POWER_CONTROL_4_REGISTER,        SSD2119_POWER_CONTROL_4_VALUE},      // Power step 4
    {SSD2119_POWER_CONTROL_5_REGISTER,        SSD2119_POWER_CONTROL_5_VALUE},      // Power step 5
    {SSD2119_VCOM_OTP_1_REGISTER,             SSD2119_VCOM_OTP_1_VALUE},           // VCOM amplitude
    {SSD2119_SLEEP_MODE_REGISTER,             SSD2119_DISPLAY_EXIT_SLEEP_MODE},    // Exit sleep
    {SSD2119_ENTRY_MODE_REGISTER,             SSD2119_ENTRY_MODE_VALUE},           // RGB565, normal scan
    {SSD2119_OUTPUT_CONTROL_REGISTER,         SSD2119_OUTPUT_CONTROL_VALUE},       // Panel type + scan direction
    {SSD2119_GATE_SCAN_START_REGISTER,        0x0000},                             // Start at gate 0
    {SSD2119_FRAME_FREQUENCY_REGISTER,        SSD2119_FRAME_FREQUENCY_VALUE},      // Frame timing
    {SSD2119_FRAME_FREQUENCY_CONTROL_2_REGISTER, SSD2119_FRAME_FREQUENCY_CONTROL_2_VALUE},
    {SSD2119_FRAME_CYCLE_CONTROL_REGISTER,    SSD2119_FRAME_CYCLE_CONTROL_VALUE},
    {SSD2119_GAMMA_CONTROL_1_REGISTER,        0x0000},
    {SSD2119_GAMMA_CONTROL_2_REGISTER,        0x0101},
    {SSD2119_GAMMA_CONTROL_3_REGISTER,        0x0100},
    {SSD2119_GAMMA_CONTROL_4_REGISTER,        0x0305},
    {SSD2119_GAMMA_CONTROL_5_REGISTER,        0x0707},
    {SSD2119_GAMMA_CONTROL_6_REGISTER,        0x0305},
    {SSD2119_GAMMA_CONTROL_7_REGISTER,        0x0707},
    {SSD2119_GAMMA_CONTROL_8_REGISTER,        0x0201},
    {SSD2119_GAMMA_CONTROL_9_REGISTER,        0x1200},
    {SSD2119_GAMMA_CONTROL_10_REGISTER,       0x0900},
    {SSD2119_DISPLAY_CONTROL_REGISTER,        SSD2119_DISPLAY_ON_VALUE}            // Display ON
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
//                  sends the full initialization command table to configure the SSD2119.
//                  Once the controller is ready, the foreground display layer is cleared
//                  to a known state.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Initialize(void* pArg)
{
    // I may need to provide a pointer to the background image for building element to display on the screen (merge)
    m_pBackgroundInfo = (StaticImageInfo_t*)pArg;

    GrafxGenDriver::Initialize(nullptr);

    IO_SetPinHigh(IO_LCD_RESET);
    LIB_Delay_mSec(5);

    // Send the complete list of initialization command to LCD
    for(int i = 0; i < GRAFX_NUMBER_OF_INIT_CMD; i++)
    {
        WriteCommand(m_InitCMD[i].Register, m_InitCMD[i].Parameter);

        if(m_InitCMD[i].Register == SSD2119_SLEEP_MODE_REGISTER)
        {
            LIB_Delay_mSec(30);
        }
    }

    ClearLayer(FOREGROUND_DISPLAY_LAYER_0);
/*
    DisplayLayer::SetColor(BLUE);
    DrawPixel(100, 100);

    Box_t Box;
    Box.Size.Width = 30;
    Box.Size.Height = 30;
    Box.Pos.X = 30;
    Box.Pos.Y = 30;
    DrawRectangle(&Box);


    DisplayLayer::SetColor(RED);
    DrawCircle(120, 130, 100, POLY_SHAPE);


    DisplayLayer::SetColor(MAGENTA);
    DrawBox(20, 20, 280, 200, 4);

    while(1)
    {
        DisplayLayer::SetColor(RNG_GetRandomFromRange(0, 65535));
        Box.Size.Width = RNG_GetRandomFromRange(10, 50);
        Box.Size.Height = RNG_GetRandomFromRange(10, 50);
        Box.Pos.X = RNG_GetRandomFromRange(0, 269);
        Box.Pos.Y = RNG_GetRandomFromRange(0, 189);
        DrawRectangle(&Box);

        DisplayLayer::SetColor(RNG_GetRandomFromRange(0, 65535));
        DrawCircle(RNG_GetRandomFromRange(30, 289),
                   RNG_GetRandomFromRange(30, 209),
                   RNG_GetRandomFromRange(5, 30),
                   POLY_SHAPE);

    }
*/

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

void GrafxDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness)
{
    Box_t Box;

    Box.Pos.X = PosX;
    Box.Pos.Y = PosY;
    Box.Size.Width  = Length;
    Box.Size.Height = Thickness;
    DrawRectangle(&Box);                        // Top
    Box.Pos.Y = PosY + Height - Thickness;
    DrawRectangle(&Box);                        // Bottom
    Box.Pos.Y = PosY + Thickness;
    Box.Size.Width  = Thickness;
    DrawRectangle(&Box);                        // Left
    Box.Pos.X = PosX + Length - Thickness;
    DrawRectangle(&Box);                        // Right
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawRectangle
//
//  Parameter(s):   Box_t*   pBox
//  Return:         None
//
//  Description:    Fill a region in a specific color
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawRectangle(Box_t* pBox)
{
    uint32_t Color;
    uint32_t Size;
    DisplayLayer*  pLayer;

    pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    Color       = pLayer->GetColor();
    SetWindow(pBox);

    Size = pBox->Size.Width * pBox->Size.Height;

    for(uint32_t i = 0; i < Size; i++)
    {
         WriteData(Color);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DRV_Copy
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  Cartesian_t*    pDstPos
//                  PixelFormat_e   SrcPixelFormat
//                  BlendMode_e     BlendMode
//  Return:         None
//
//  Description:    Copy a rectangle region from square memory region to another square memory
//                  region
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::Copy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    VAR_UNUSED(pSrc);
    VAR_UNUSED(pBox);
    VAR_UNUSED(pDstPos);
    VAR_UNUSED(SrcPixelFormat);
    VAR_UNUSED(BlendMode);
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
//  Note(s):        Source is linear, also we work only on a RG565 destination.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    uint16_t* pBuffer;
    size_t    Size = pBox->Size.Width *  pBox->Size.Height * sizeof(uint16_t);

    // What is needed here..

    pMemoryPool->Alloc(Size, MEM_DBG_GRAFX_BC);                                     // Reserve memory for merging image into background
        // -    Use DMA2D to merge
        // merge both image.

    SetWindow(pBox);
    pMemoryPool->Free((void**)&pBuffer);

    VAR_UNUSED(pSrc);
    VAR_UNUSED(pDstPos);
    VAR_UNUSED(SrcPixelFormat);
    VAR_UNUSED(BlendMode);

    // We are not calling the gen driver. because we don't have that functionality
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyLinear
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  PixelFormat_e   SrcPixelFormat)
//                  BlendMode_e     BlendMode
//  Return:         None
//
//   Description:   Copy a rectangle region from linear memory region to square memory area
//
//  Note(s):        Source is linear like an array
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::CopyLinear(void* pSrc, Box_t* pBox, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    VAR_UNUSED(pSrc);
    VAR_UNUSED(pBox);
    VAR_UNUSED(SrcPixelFormat);
    VAR_UNUSED(BlendMode);
}

void GrafxDriver::CopyLinear(void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, PixelFormat_e PixelFormat, BlendMode_e BlendMode)
{
//    Not supported for now

    VAR_UNUSED(pSrc);
    VAR_UNUSED(PosX);
    VAR_UNUSED(PosY);
    VAR_UNUSED(pSrc);
    VAR_UNUSED(Width);
    VAR_UNUSED(Height);
    VAR_UNUSED(PixelFormat);
    VAR_UNUSED(BlendMode);
}

void GrafxDriver::CopyLinear(ImageID_e Image, Cartesian_t Position, BlendMode_e BlendMode)
{

    // this is the next function to develop in this driver
    VAR_UNUSED(Image);
    VAR_UNUSED(Position);
    VAR_UNUSED(BlendMode);
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
    DisplayLayer*  pLayer;
    uint32_t Color;

    pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    Color  = pLayer->GetColor();
    SetRAM_Pointer(PosX, PosY);
    WriteData(Color);
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
//  Description:    Displays a line of a specific Thickness.
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawLine(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction)
{

    //m_pLayer = &LayerTable[DisplayLayer::GetDrawing()];

    // if memory calculate offset check if we can call the default function
    // else

    //get the color!!
    SetRAM_Pointer(PosX, PosY);
    // loop for the thickness
    // configure the offset of autoincrement...
    // loop for the number of pixel..
   // Write(Color);
}

void GrafxDriver::ImageCopy(StaticImageInfo_t* pImageInfo, uint16_t PosX, uint16_t PosY, BlendMode_e BlendMode)
{
    Box_t Box;
    uint32_t  Size  = pImageInfo->RawSize;

    Box.Pos.X       = PosX;
    Box.Pos.Y       = PosY;
    Box.Size.Width  = pImageInfo->ImageInfo.Size.Width;
    Box.Size.Height = pImageInfo->ImageInfo.Size.Height;
    SetWindow(&Box);

    //pLayer = &LayerTable[DisplayLayer::GetDrawing()];
    switch(pImageInfo->Compression)
    {
        case COMPX_COMPRESSION_NONE:
        {
            uint16_t* pData = (uint16_t*)pImageInfo->ImageInfo.pPointer;

            for(uint32_t i = 0; i > Size; i++)
            {
                WriteData(*pData);
                pData++;
            }
        }
        break;

        case COMPX_RLE_16:
        {
            uint8_t* pData = (uint8_t*)pImageInfo->ImageInfo.pPointer;

            for(uint32_t i = 0; i > Size; )
            {
                uint32_t Repeat = uint32_t(*(uint8_t*)pData) + 1;
                pData++;
                uint16_t Pixel = *(uint16_t*)pData;
                pData += sizeof(uint16_t);

                while(Repeat--)
                {
                    WriteData(Pixel);
                    i++;
                }
            }
        }
        break;

        default: break;

/*
        case COMPX_RLE_32:
        {
            uint8_t* pData = (uint16_t*)pImageInfo->ImageInfo.pPointer;

            for(uint32_t i = 0; i > Size; )
            {
                uint32_t Repeat = uint32_t(*(uint8_t*)pData) + 1;
                pData++;
                uint32_t Pixel = *(uint32_t*)pData);
                pData += sizeof(uint32_t);

                while(Repeat--)
                {
                    WriteData(Pixel);
                    i++;
                }
            }
        }
        break;
*/
    }
}


void GrafxDriver::PrintFont(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)
{
    //s32_t         AreaConfig;
    PixelFormat_e PixelFormat;
    uint8_t       PixelSize;


VAR_UNUSED(pDescriptor);
VAR_UNUSED(pPos);
VAR_UNUSED(PixelFormat);
VAR_UNUSED(PixelSize);
    // m_pLayer = &LayerTable[DisplayLayer::GetDrawing()];

//    uint32_t           Address;

 //   AreaConfig.u_16.u1 = pDescriptor->Size.Width;
  //  AreaConfig.u_16.u0 = pDescriptor->Size.Height;
 //   PixelFormat        = m_pLayer->GetPixelFormat();
 //   PixelSize          = m_pLayer->GetPixelSize();

/*
(uint32_t)pDescriptor->pAddress;                           // Source address 1 of the font
0;                                                         // Font source line offset - none as we are linear
pLayer->GetTextColor();

//??    Address            = pLayer->GetAddress() + (((pPos->Y * GRAFX_DRIVER_SIZE_X) + pPos->X) * (uint32_t)PixelSize);

//must be all the info needed
*/
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
//  Parameter(s):   uint8_t    Register    Specifies the register address to read.
//
//  Return:         uint16_t                Register value returned by the LCD controller.
//
//  Description:    Sends a register index to the LCD controller, waits for the bus
//                  to stabilize, then reads the corresponding 16-bit data from the
//                  LCD RAM interface.
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
//  Name:           WriteWindowData
//
//  Parameter(s):
//
//  Return:
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void WriteData(uint16_t* pData, size_t Length){}
//void GrafxDriver::WriteWindowData(Cartesian_t* pDstPos, uint16_t* pData, size_t Length)
//{
    // Set window frame into LCD

    // Set Start Address
    //SetRAM_Pointer      (uint16_t PosX, uint16_t PosY);


//}


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
void GrafxDriver::SetWindow(Box_t* pBox)
{
    uint16_t StartX = pBox->Pos.X;
    uint16_t EndX   = StartX + pBox->Size.Width  - 1;
    uint16_t StartY = pBox->Pos.Y;
    uint16_t EndY   = StartY + pBox->Size.Height - 1;
    WriteCommand(SSD2119_HORIZONTAL_RAM_START_REGISTER, StartX);        // Horizontal window (X) Start
    WriteCommand(SSD2119_HORIZONTAL_RAM_END_REGISTER,   EndX);          // Horizontal window (X) End
    uint16_t Vertical = (EndY << 8) | (StartY & 0x00FF);
    WriteCommand(SSD2119_VERTICAL_RAM_POSITION_REGISTER, Vertical);     // Vertical window (Y) packed into one register
    SetRAM_Pointer(StartX, StartY);                                     // Set GRAM cursor to top-left of window
    SetWriteRAM_Ready();
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
void GrafxDriver::ResetWindow(void)
{
    WriteCommand(SSD2119_VERTICAL_RAM_POSITION_REGISTER, SSD2119_VERTICAL_WINDOWS_FULL_SIZE);
    WriteCommand(SSD2119_HORIZONTAL_RAM_START_REGISTER,  SSD2119_HORIZONTAL_WINDOWS_START_FULL_SIZE);
    WriteCommand(SSD2119_HORIZONTAL_RAM_END_REGISTER,    SSD2119_HORIZONTAL_WINDOWS_END_FULL_SIZE);
    SetRAM_Pointer(0, 0);
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
