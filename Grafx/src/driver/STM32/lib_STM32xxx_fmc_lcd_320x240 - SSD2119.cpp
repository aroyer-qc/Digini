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

const SSD2119_InitCMD_t GrafxDriver::InitCMD[/*GRAFX_NUMBER_OF_INIT_CMD*/11] =
{
    {SSD2119_VCOM_OTP_1_REGISTER,                   0x0006},        // Sets VCOM amplitude from OTP block. This stabilizes the common electrode voltage and reduces flicker.
    {SSD2119_OSCILLATOR_START_REGISTER,             0x0001},        // Starts the internal oscillator. This must be done before any display timing registers are touched.
    {SSD2119_SLEEP_MODE_REGISTER,                   0x0000},
    {SSD2119_DISPLAY_CONTROL_REGISTER,              0x0033},
    {SSD2119_ENTRY_MODE_REGISTER,                   0x6874},        // Color format (16-bit, 18-bit, etc.), Horizontal/vertical increment mode, BGR/RGB order, Addressing mode. :0x6874 is the standard 16-bit 565, left‑to‑right, top-to-bottom mode.
    {SSD2119_LCD_DRIVE_AC_CONTROL_REGISTER,         0x0600},        // Controls: AC drive frequency, Polarity, Line inversion. : 0x0600 is a stable default for most TFT glass.
    {SSD2119_FRAME_CYCLE_CONTROL_REGISTER,          0x5308},        // Controls: Frame frequency, Line period, Porch timing. : 0x5308 is a typical stable timing for 60–70 Hz refresh.
    {SSD2119_POWER_CONTROL_2_REGISTER,              0x0004},        // Booster control step 2.
    {SSD2119_POWER_CONTROL_3_REGISTER,              0x000F},        // Booster control step 3.
    {SSD2119_POWER_CONTROL_4_REGISTER,              0x1B00},        // VCOM voltage setting (coarse).
    {SSD2119_POWER_CONTROL_5_REGISTER,              0x00B5},        // VCOM voltage setting (fine). This pair (0x2E00 + 0x00BE) is what stabilizes the TFT’s common electrode.


    /*
    {SSD2119_DISPLAY_CONTROL_REGISTER,              0x0021},
    {SSD2119_DEVICE_CODE_READ_REGISTER,             0x0001},
    {SSD2119_DISPLAY_CONTROL_REGISTER,              0x0023},
    {SSD2119_SLEEP_MODE_REGISTER,                   0x0000},
    {SSD2119_DISPLAY_CONTROL_REGISTER,              0x0033},

    {SSD2119_OSCILLATOR_START_REGISTER,             0x0001},        // Starts the internal oscillator. This must be done before any display timing registers are touched.
    {SSD2119_SLEEP_MODE_REGISTER,                   0x0000},
    {SSD2119_DISPLAY_CONTROL_REGISTER,              0x0033},
    {SSD2119_ENTRY_MODE_REGISTER,                   0x6874},        // Color format (16-bit, 18-bit, etc.), Horizontal/vertical increment mode, BGR/RGB order, Addressing mode. :0x6874 is the standard 16-bit 565, left‑to‑right, top-to-bottom mode.
    {SSD2119_LCD_DRIVE_AC_CONTROL_REGISTER,         0x0600},        // Controls: AC drive frequency, Polarity, Line inversion. : 0x0600 is a stable default for most TFT glass.
    {SSD2119_POWER_CONTROL_1_REGISTER,              0x4A38},        // Main power control: Booster, Voltage regulator, Reference voltage. This is part of the power‑up ramp.
    {SSD2119_OUTPUT_CONTROL_REGISTER,               0x72EF},        // Controls: Scan direction, Gate driver shift direction, LCD panel type, Display resolution mapping. : 0x72EF is a common value for 320×240 TFT panels.
    {SSD2119_GATE_SCAN_START_REGISTER,              0x0000},        // Start scanning from gate line 0.
    {SSD2119_FRAME_FREQUENCY_REGISTER,              0xA000},        // Primary frame frequency control.
    {SSD2119_VCOM_OTP_1_REGISTER,                   0x0006},        // Sets VCOM amplitude from OTP block. This stabilizes the common electrode voltage and reduces flicker.
    {SSD2119_SLEEP_MODE_CONTROL_REGISTER,           0x0999},        // Extended sleep control: Deep sleep exit, Oscillator gating, Power block stabilization. : 0x08D9 is a known “panel-ready” value.
    {SSD2119_FRAME_FREQUENCY_CONTROL_2_REGISTER,    0x3800},        // Fine-tunes oscillator division for frame timing.
    {SSD2119_FRAME_CYCLE_CONTROL_REGISTER,          0x5308},        // Controls: Frame frequency, Line period, Porch timing. : 0x5308 is a typical stable timing for 60–70 Hz refresh.
    {SSD2119_POWER_CONTROL_2_REGISTER,              0x0004},        // Booster control step 2.
    {SSD2119_POWER_CONTROL_3_REGISTER,              0x000F},        // Booster control step 3.
    {SSD2119_POWER_CONTROL_4_REGISTER,              0x1B00},        // VCOM voltage setting (coarse).
    {SSD2119_POWER_CONTROL_5_REGISTER,              0x00B5},        // VCOM voltage setting (fine). This pair (0x2E00 + 0x00BE) is what stabilizes the TFT’s common electrode.
    {SSD2119_VERTICAL_RAM_POSITION_REGISTER,        0xEF00},
    {SSD2119_HORIZONTAL_RAM_START_REGISTER,         0x0000},
    {SSD2119_HORIZONTAL_RAM_END_REGISTER,           0x013F},
    {SSD2119_GAMMA_CONTROL_1_REGISTER,              0x0000},
    {SSD2119_GAMMA_CONTROL_2_REGISTER,              0x0101},
    {SSD2119_GAMMA_CONTROL_3_REGISTER,              0x0100},
    {SSD2119_GAMMA_CONTROL_4_REGISTER,              0x0305},
    {SSD2119_GAMMA_CONTROL_5_REGISTER,              0x0707},
    {SSD2119_GAMMA_CONTROL_6_REGISTER,              0x0305},
    {SSD2119_GAMMA_CONTROL_7_REGISTER,              0x0707},
    {SSD2119_GAMMA_CONTROL_8_REGISTER,              0x0201},
    {SSD2119_GAMMA_CONTROL_9_REGISTER,              0x1200},
    {SSD2119_GAMMA_CONTROL_10_REGISTER,             0x0900},
    {SSD2119_DISPLAY_CONTROL_REGISTER,              0x0033},        // Final display enable: Turns on the display, Enables scanning, Enables frame output. : 0x0033 = display ON, internal oscillator ON, scanning enabled.
*/
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
static uint16_t ChipID;
void GrafxDriver::Initialize(void* pArg)
{
    GrafxGenDriver::Initialize(pArg);

    IO_SetPinHigh(IO_LCD_RESET);
    LIB_Delay_mSec(5);

    ChipID = ReadCommand(SSD2119_DEVICE_CODE_READ_REGISTER);

    // Send the complete list of initialization command to LCD
    for(int i = 0; i < 11/* GRAFX_NUMBER_OF_INIT_CMD*/; i++)
    {
        WriteCommand(InitCMD[i].Register, InitCMD[i].Parameter);

        if(InitCMD[i].Register == SSD2119_SLEEP_MODE_REGISTER)
        {
            LIB_Delay_mSec(30);
        }
    }

    ClearLayer(FOREGROUND_DISPLAY);

// test
    CLayer::SetColor(BLUE);
    DrawPixel(10, 10);
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
//static uint32_t COUNT = 0;
void GrafxDriver::ClearLayer(Layer_e Layer)
{
   	if(Layer == FOREGROUND_DISPLAY)
    {
        SetRAM_Pointer(0, 0);

        for(uint32_t i = 0; i < GRAFX_DRIVER_SIZE; i++)
        {
            WriteData(0x001F);
  //          COUNT++;
        }
    }
    else
    {
        GrafxGenDriver::ClearLayer(Layer);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: DrawRectangle
//
//   Parameter(s):  Box_t*  pBox
//                  uint8_t Mode
//   Return value:  None
//
//   Description:   Draw a box or a rectangle
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawRectangle(Box_t* pBox, uint8_t Mode)
{
    VAR_UNUSED(pBox);
    VAR_UNUSED(Mode);
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
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::BlockCopy(void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    VAR_UNUSED(pSrc);
    VAR_UNUSED(pBox);
    VAR_UNUSED(pDstPos);
    VAR_UNUSED(SrcPixelFormat);
    VAR_UNUSED(BlendMode);

    // We are not calling the gen driver. because we don't have that functionnality
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
    VAR_UNUSED(Image);
    VAR_UNUSED(Position);
    VAR_UNUSED(BlendMode);
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
    VAR_UNUSED(pBox);
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
    uint16_t Color = 0;

   // m_pLayer = &LayerTable[CLayer::GetDrawing()];

    // if memory calculate offset check if we can call the default function
    // else

    SetRAM_Pointer(PosX, PosY);
    //get the color!!
    SetWriteRAM_Ready();
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

    //m_pLayer = &LayerTable[CLayer::GetDrawing()];

    // if memory calculate offset check if we can call the default function
    // else

    //get the color!!
    SetRAM_Pointer(PosX, PosY);
    // loop for the thickness
    // configure the offset of autoincrement...
    // loop for the number of pixel..
   // Write(Color);
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
    // m_pLayer = &LayerTable[CLayer::GetDrawing()];

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
//                  to stabilize, then reads the corresponding 16‑bit data from the
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

#endif // DIGINI_USE_GRAFX
