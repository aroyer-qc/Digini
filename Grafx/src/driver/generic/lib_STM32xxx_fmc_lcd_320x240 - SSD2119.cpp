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
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct SSD2119_InitCMD_t
{
    uint8_t     Register;
    uint16_t    Parameter;
};

struct SSD2119_TFT_LCD_t
{
    volatile uint16_t REG;   // A16 = 0
    volatile uint16_t RAM;   // A16 = 1
} 

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const SSD2119_InitCMD_t GrafxDriver::InitCMD[GRAFX_NUMBER_OF_INIT_CMD] =
{
    {SSD2119_VCOM_OTP_1_REGISTER,                   0x0006},        // Sets VCOM amplitude from OTP block. This stabilizes the common electrode voltage and reduces flicker.
    {SSD2119_OSCILLATOR_START_REGISTER,             0x0001},        // Starts the internal oscillator. This must be done before any display timing registers are touched.
    {SSD2119_SLEEP_MODE_REGISTER,                   0x0000},        // Exit sleep mode. Turns on internal power blocks.
    {SSD2119_OUTPUT_CONTROL_REGISTER,               0x72EF},        // Controls: Scan direction, Gate driver shift direction, LCD panel type, Display resolution mapping. : 0x72EF is a common value for 320×240 TFT panels.
    {SSD2119_LCD_DRIVE_AC_CONTROL_REGISTER,         0x0600},        // Controls: AC drive frequency, Polarity, Line inversion. : 0x0600 is a stable default for most TFT glass.
    {SSD2119_POWER_CONTROL_1_REGISTER,              0x6A38},        // Main power control: Booster, Voltage regulator, Reference voltage. This is part of the power‑up ramp.
    {SSD2119_ENTRY_MODE_REGISTER,                   0x6874},        // Color format (16-bit, 18-bit, etc.), Horizontal/vertical increment mode, BGR/RGB order, Addressing mode. :0x6874 is the standard 16-bit 565, left‑to‑right, top-to-bottom mode.
    {SSD2119_GATE_SCAN_START_REGISTER,              0x0000},        // Start scanning from gate line 0.
    {SSD2119_FRAME_CYCLE_CONTROL_REGISTER,          0x5308},        // Controls: Frame frequency, Line period, Porch timing. : 0x5308 is a typical stable timing for 60–70 Hz refresh.
    {SSD2119_POWER_CONTROL_2_REGISTER,              0x0003},        // Booster control step 2.
    {SSD2119_POWER_CONTROL_3_REGISTER,              0x000A},        // Booster control step 3.
    {SSD2119_POWER_CONTROL_4_REGISTER,              0x2E00},        // VCOM voltage setting (coarse).
    {SSD2119_POWER_CONTROL_5_REGISTER,              0x00BE},        // VCOM voltage setting (fine). This pair (0x2E00 + 0x00BE) is what stabilizes the TFT’s common electrode.
    {SSD2119_FRAME_FREQUENCY_REGISTER,              0x8000},        // Primary frame frequency control.
    {SSD2119_FRAME_FREQUENCY_CONTROL_2_REGISTER,    0x7800},        // Fine-tunes oscillator division for frame timing.
    {SSD2119_VCOM_CONTROL_1_REGISTER,               0x0078},        // Fine VCOM tuning (offset, flicker compensation).
    {SSD2119_X_RAM_ADDRESS_REGISTER,                0x0000},        // Initial X RAM pointer.
    {SSD2119_Y_RAM_ADDRESS_REGISTER,                0x0000},        // Initial Y RAM pointer. 
    {SSD2119_SLEEP_MODE_CONTROL_REGISTER,           0x08D9},        // Extended sleep control: Deep sleep exit, Oscillator gating, Power block stabilization. : 0x08D9 is a known “panel-ready” value.
    // Adjust the Gamma Curve
    // These 10 registers define the full gamma curve: Brightness, Contrast, Color linearity, Mid-tone shaping, Saturation
    {SSD2119_GAMMA_CONTROL_1_REGISTER,              0x0000}, 
    {SSD2119_GAMMA_CONTROL_2_REGISTER,              0x0104}, 
    {SSD2119_GAMMA_CONTROL_3_REGISTER,              0x0100}, 
    {SSD2119_GAMMA_CONTROL_4_REGISTER,              0x0305}, 
    {SSD2119_GAMMA_CONTROL_5_REGISTER,              0x0505}, 
    {SSD2119_GAMMA_CONTROL_6_REGISTER,              0x0305}, 
    {SSD2119_GAMMA_CONTROL_7_REGISTER,              0x0707}, 
    {SSD2119_GAMMA_CONTROL_8_REGISTER,              0x0300}, 
    {SSD2119_GAMMA_CONTROL_9_REGISTER,              0x1200}, 
    {SSD2119_GAMMA_CONTROL_10_REGISTER,             0x0800}, 
    
    {SSD2119_DISPLAY_CONTROL_REGISTER,              0x0033},        // Final display enable: Turns on the display, Enables scanning, Enables frame output. : 0x0033 = display ON, internal oscillator ON, scanning enabled.
};

//-------------------------------------------------------------------------------------------------

void GrafxDriver::Initialize(void* pArg)
{
    GrafxGenDriver::Initialize(pArg);
    
    // Send the complete list of initialization command to LCD
    for(int i = 0; i < GRAFX_NUMBER_OF_INIT_CMD; i++)
    {
        WriteCommand(InitCMD.Register, InitCMD.Parameter);
    }
}

//-------------------------------------------------------------------------------------------------

void GrafxDriver::SetRAM_Pointer(uint16_t PosX, uint16_t PosY)
{
    WriteCommand(SSD2119_X_RAM_ADDRESS_REGISTER, PosX);
	WriteCommand(SSD2119_Y_RAM_ADDRESS_REGISTER, PosY);
	SetWriteRAM_Ready();
}

//-------------------------------------------------------------------------------------------------

void GrafxDriver::ClearLayer(Layer_e Layer)
{
   	SetWriteRAM_Ready();

	for(uint32_t i = 0; i < GRAFX_DRIVER_SIZE; i++ )
	{
		WriteData(0x0000);
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
//                  uint16_t        X
//                  uint16_t        Y
//                  uint16_t        Width
//                  uint16_t        Height
//                  uint16_t        DstX
//                  uint16_t        DstY
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

    BlockCopy(pSrc, &Box, &Pos, SrcPixelFormat, BlendMode);
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
//  Name:           DrawBox
//
//  Parameter(s):   uint16_t    PosX
//                  uint16_t    PosY
//                  uint16_t    Length
//                  uint16_t    Height
//                  uint16_t    Thickness
//  Return:         None
//
//  Description:    Draw a box in a specific Thickness
//
//-------------------------------------------------------------------------------------------------
void GrafxDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness)
{
    Box_t Box;

    Box.Pos.X = PosX;
    Box.Pos.Y = PosY;
    Box.Size.Width  = Length;
    Box.Size.Height = Height;

    while(Thickness)
    {
        DrawRectangle(&Box);
        Box.Pos.X++;
        Box.Pos.Y++;
        Box.Size.Width  -= 2;
        Box.Size.Height -= 2;
        Thickness--;
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
    uint16_t Color;

    m_pLayer = &LayerTable[CLayer::GetDrawing()];

    // if memory calculate offset check if we can call the default function
    // else
        
    SetRAM_Pointer(PosX, PosY);
    //get the color!!
    SetWriteRAM_Ready();
    Write(Color);
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
    uint16_t Length;

    // X1 need to be the lowest (STM32 need this)
    if(PosX1 > PosX2)
    {
        Length = PosX1 - PosX2;
        PosX1  = PosX2;
    }
    else
    {
        Length = PosX2 - PosX1;
    }

    DrawLine(PosX1, PosY, Length, Thickness, DRAW_HORIZONTAL);
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
    uint16_t Length;

    // Y1 need to be the lowest (STM32 need this)
    if(PosY1 > PosY2)
    {
        Length = PosY1 - PosY2;
        PosY1  = PosY2;
    }
    else
    {
        Length = PosY2 - PosY1;
    }

    DrawLine(PosX, PosY1, Length, Thickness, DRAW_VERTICAL);
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
    
    m_pLayer = &LayerTable[CLayer::GetDrawing()];
    
    // if memory calculate offset check if we can call the default function
    // else
        
    //get the color!!
    SetRAM_Pointer(PosX, PosY);
    // loop for the thickness
    // configure the offset of autoincrement...
    // loop for the number of pixel.. 
    Write(Color);
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
    //s32_t         AreaConfig;
    PixelFormat_e PixelFormat;
    uint8_t       PixelSize;

    m_pLayer = &LayerTable[CLayer::GetDrawing()];

//    uint32_t           Address;

    AreaConfig.u_16.u1 = pDescriptor->Size.Width;
    AreaConfig.u_16.u0 = pDescriptor->Size.Height;
    PixelFormat        = m_pLayer->GetPixelFormat();
    PixelSize          = m_pLayer->GetPixelSize();

/*
(uint32_t)pDescriptor->pAddress;                           // Source address 1 of the font
0;                                                         // Font source line offset - none as we are linear
pLayer->GetTextColor();

//??    Address            = pLayer->GetAddress() + (((pPos->Y * GRAFX_DRIVER_SIZE_X) + pPos->X) * (uint32_t)PixelSize);

//must be all the info needed
*/
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX

#if 0
 ??????
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	//LCD_CS PD7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
	GPIO_ResetBits(GPIOD , GPIO_Pin_7);		 //CS=0;

	LCD_Configuration();
	/* Set MN(multipliers) of PLL, VCO = crystal freq * (N+1) */
	/* PLL freq = VCO/M with 250MHz < VCO < 800MHz */
	/* The max PLL freq is around 120MHz. To obtain 120MHz as the PLL freq */

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB convert to BBBBBGGGGGGRRRRR
* Input          : RGB color
* Output         : None
* Return         : RGB color
* Attention		 :
*******************************************************************************/
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : Get color of the point
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;
	
	LCD_SetCursor(Xpos,Ypos);

	LCD_WriteIndex(0x0022);  
	
    default:	/* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919 */
      dummy = LCD_ReadData();
      dummy = LCD_ReadData(); 
      return  LCD_BGR2RGB( dummy );
	}
}
#endif
