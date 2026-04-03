//-------------------------------------------------------------------------------------------------
//
//  File : lib_fmc_lcd_320x240_SSD2119.h
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
//  Note(s):
//
//     Put this into device_cfg.h
//
//      - #define GRAFX_LCD_BASE                 FMC_BANK1_x  (NE1-NE4 = region x = 1 to 4)
//      - #define GRAFX_LCD_REGISTER_SELECT_BIT  16           Ex. A16 (any available Address line)
//
//
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define GRAFX_NUMBER_OF_ACTIVE_LAYER                1

// Display size
#define GRAFX_DRIVER_SIZE_X                         320
#define GRAFX_DRIVER_SIZE_Y                         240
#define GRAFX_DRIVER_SIZE                           (GRAFX_DRIVER_SIZE_X * GRAFX_DRIVER_SIZE_Y)

//#define GRAFX_HSYNC                                 2               // Horizontal synchronization
//#define GRAFX_HBP                                   2               // Horizontal back porch
//#define GRAFX_HFP                                   2               // Horizontal front porch
//#define GRAFX_VSYNC                                 1               // Vertical synchronization
//#define GRAFX_VBP                                   2               // Vertical back porch
//#define GRAFX_VFP                                   2               // Vertical front porch

//#define GRAFX_USE_SOFT_COPY_LINEAR
//#define GRAFX_USE_SOFT_COPY_LAYER_TO_LAYER
//#define GRAFX_USE_SOFT_PIXEL                      // We use this driver DMA for this function
//#define GRAFX_USE_SOFT_BOX                        // We use this driver function
//#define GRAFX_USE_SOFT_VLINE
//#define GRAFX_USE_SOFT_HLINE
#define GRAFX_USE_SOFT_DLINE
#define GRAFX_USE_SOFT_CIRCLE
//#define GRAFX_USE_SOFT_RECTANGLE

//#define GRAFX_USE_SOFT_PRINT_FONT                 // we use this driver to print FONT using DMA2D
//#define GRAFX_USE_SOFT_ALPHA                      // We use this uP + LCD controller has alpha acceleration
//#define GRAFX_USE_SOFT_COPY                       // We use this driver DMA for this function
//#define GRAFX_USE_SOFT_FILL                       // We use this driver DMA for this function

#define GRAFX_NUMBER_OF_INIT_CMD                        30

// SSD2119 Command Set
#define SSD2119_DEVICE_CODE_READ_REGISTER               0x00
#define SSD2119_OSCILLATOR_START_REGISTER               0x00
#define SSD2119_OUTPUT_CONTROL_REGISTER                 0x01
#define SSD2119_LCD_DRIVE_AC_CONTROL_REGISTER           0x02
#define SSD2119_POWER_CONTROL_1_REGISTER                0x03
#define SSD2119_DISPLAY_CONTROL_REGISTER                0x07
#define SSD2119_FRAME_CYCLE_CONTROL_REGISTER            0x0B
#define SSD2119_POWER_CONTROL_2_REGISTER                0x0C
#define SSD2119_POWER_CONTROL_3_REGISTER                0x0D
#define SSD2119_POWER_CONTROL_4_REGISTER                0x0E
#define SSD2119_GATE_SCAN_START_REGISTER                0x0F
#define SSD2119_SLEEP_MODE_REGISTER                     0x10
#define SSD2119_ENTRY_MODE_REGISTER                     0x11
#define SSD2119_SLEEP_MODE_CONTROL_REGISTER             0x12
#define SSD2119_GENERAL_INTERFACE_CONTROL_REGISTER      0x15
#define SSD2119_POWER_CONTROL_5_REGISTER                0x1E
//#define SSD2119_X_RAM_ADDRESS_REGISTER                  0x20
//#define SSD2119_Y_RAM_ADDRESS_REGISTER                  0x21
#define SSD2119_RAM_DATA_REGISTER                       0x22
#define SSD2119_FRAME_FREQUENCY_REGISTER                0x25
#define SSD2119_FRAME_FREQUENCY_CONTROL_2_REGISTER      0x26
#define SSD2119_VCOM_CONTROL_1_REGISTER                 0x27
#define SSD2119_VCOM_OTP_1_REGISTER                     0x28
#define SSD2119_VCOM_OTP_2_REGISTER                     0x29
#define SSD2119_GAMMA_CONTROL_1_REGISTER                0x30
#define SSD2119_GAMMA_CONTROL_2_REGISTER                0x31
#define SSD2119_GAMMA_CONTROL_3_REGISTER                0x32
#define SSD2119_GAMMA_CONTROL_4_REGISTER                0x33
#define SSD2119_GAMMA_CONTROL_5_REGISTER                0x34
#define SSD2119_GAMMA_CONTROL_6_REGISTER                0x35
#define SSD2119_GAMMA_CONTROL_7_REGISTER                0x36
#define SSD2119_GAMMA_CONTROL_8_REGISTER                0x37
#define SSD2119_GAMMA_CONTROL_9_REGISTER                0x3A
#define SSD2119_GAMMA_CONTROL_10_REGISTER               0x3B
#define SSD2119_VERTICAL_RAM_POSITION_REGISTER          0x44
#define SSD2119_HORIZONTAL_RAM_START_REGISTER           0x45
#define SSD2119_HORIZONTAL_RAM_END_REGISTER             0x46
#define SSD2119_X_RAM_ADDRESS_REGISTER                  0x4E
#define SSD2119_Y_RAM_ADDRESS_REGISTER                  0x4F

#define SSD2119_DISPLAY_ON_VALUE                        0x0033
#define SSD2119_DISPLAY_OFF_VALUE                       0x0000

#define FMC_BANK1_1                                     0x00000000
#define FMC_BANK1_2                                     0x00000002
#define FMC_BANK1_3                                     0x00000004
#define FMC_BANK1_4                                     0x00000006

// GRAFX_LCD_BASE and GRAFX_LCD_REGISTER_SELECT_BIT must be configure into device_cfg.h
#define LCD_REG                                         (*((volatile uint16_t*)(GRAFX_LCD_BASE)))
#define LCD_RAM                                         (*((volatile uint16_t*)(GRAFX_LCD_BASE | (1 << GRAFX_LCD_REGISTER_SELECT_BIT))))

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct SSD2119_InitCMD_t
{
    uint8_t     Register;
    uint16_t    Parameter;
};

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class GrafxDriver : public GrafxGenDriver
{
    public:

        void            Initialize          (void* pArg)           override;

        void            ClearLayer          (Layer_e Layer)        override;
        void            DisplayOn           (void)                 override         { LCD_REG = SSD2119_DISPLAY_CONTROL_REGISTER; LCD_RAM = SSD2119_DISPLAY_ON_VALUE; }
        void            DisplayOff          (void)                 override         { LCD_REG = SSD2119_DISPLAY_CONTROL_REGISTER; LCD_RAM = SSD2119_DISPLAY_OFF_VALUE; }

        void            BlockCopy           (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
        void            Copy                (void* pSrc, Box_t* pBox, Cartesian_t* pDstPos, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
        void            CopyLinear          (void* pSrc, Box_t* pBox, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode);
void CopyLinear      (void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, PixelFormat_e PixelFormat, BlendMode_e BlendMode);
void CopyLinear      (Skin_e Image, Cartesian_t Position, BlendMode_e BlendMode);

        void            DrawLine            (uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Thickness, DrawMode_e Direction);
        void            DrawPixel           (uint16_t PosX, uint16_t PosY);
        void            DrawRectangle       (Box_t* pBox, uint8_t Mode);
        void            DrawRectangle       (Box_t* pBox);
        void            PrintFont           (FontDescriptor_t* pDescriptor, Cartesian_t* pPos);

    private:

        void            SetRAM_Pointer      (uint16_t PosX, uint16_t PosY);

        uint16_t        ReadCommand         (uint8_t Register)                      { LCD_REG = Register; return LCD_RAM; }
        uint16_t        ReadData            (void)                                  { return LCD_RAM; }
        void            WriteCommand        (uint8_t Register, uint16_t Data)       { LCD_REG = Register; LCD_RAM = Data; }
        void            WriteRegister       (uint8_t Register)                      { LCD_REG = Register; }
        void            WriteData           (uint16_t Data)                         { LCD_RAM = Data; }
        void            SetWriteRAM_Ready   (void)                                  { LCD_REG = SSD2119_RAM_DATA_REGISTER; }

        static const    SSD2119_InitCMD_t InitCMD[GRAFX_NUMBER_OF_INIT_CMD];
};

//-------------------------------------------------------------------------------------------------

#ifdef LIB_SSD2119_GLOBAL
GrafxDriver         Grafx;
GrafxDriver*        myGrafx = &Grafx;
#else
extern GrafxDriver* myGrafx;
#endif

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
