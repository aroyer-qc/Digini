//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_font.cpp
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
//  Notes:
//
// FONT_sStaticDescriptor inner working
//
// the high nibble bits of the first uint8_t are for the SIZE X       ___________ _______
// the low nibble bits of the first uint8_t are for the SIZE Y      |           |        Offset Y
//                                                                  |           |_______
// the high nibble bits of the second uint8_t are for the OFFSET X  |   ****    |
// the low nibble bits of the second uint8_t are for the OFFSET Y   |  **  **   |
//                                                                  |  **  **   |        Size Y
// the high nibble bits of the third uint8_t are for the PADDING X  |  **  **   |
// the low nibble bits of the third uint8_t are not used (see note) |   ****    |_______
//                                                                  |___________|
//                                                                  |  |     |  |
//                                                                  |  |     |  |_______ Padding X
//                                                                  |  |_____|__________ Size X
//                                                                  |__|________________ Offset X
//
//  Notes: Number 32 (0x20) or SPACE the third bytes low nibble hold the Font Total size in X (0 = 16)
//  Notes: Number 33 (0x21) or '!' the third bytes low nibble hold the font Height in Y (0 = 16)
//  Notes: Number 34 (0x22) or '"' the third bytes low nibble hold the font Interline
//
//
//  Raw monochrome data description
//
//  Each nibble represent a grouping of pixel
//  higher bit of grouping represent the state of the pixel
//  the lower 3 bits represent (how many pixel + 1) there is in row with this value
//
//  note that this value can be on multi line according to value on the Descriptor
//
//  |   Nibble 1    |   Nibble 2    |
//  |_______________|_______________|
//  |   |   |   |   |   |   |   |   |
//  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//  |___|___|___|___|___|___|___|___|
//  | 0 | Number of | 0 | Number of |
//  |or | Pixel + 1 |or | Pixel + 1 |
//  | 1 |           | 1 |           |
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

CFont FontDefault;

//-------------------------------------------------------------------------------------------------
//
//  Name:           FONT_Initialize
//  Parameter(s):   void
//  Return:         void
//
//  Description:    This function will transfer basic font set for error and loading screen
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (GRAFX_USE_FONT_SIZE_8 == DEF_ENABLED) || (GRAFX_USE_FONT_SIZE_12 == DEF_ENABLED) || (GRAFX_USE_FONT_SIZE_16 == DEF_ENABLED)
void FONT_Initialize(void)
{
    uint16_t                        Counter;
    uint16_t                        SizeCount;
    uint8_t                         Font;
    uint16_t                        Character;
    uint8_t                         Value;
    uint8_t                         BitCount;
    uint8_t                         BitValue;
    uint8_t*                        pMemory;
    const StaticFontDescriptor_t*   pDscFont;
    FontDescriptor_t                FontDescriptor;
    FontInfo_t                 		FontInfo;

    // load in RAM default 8x8 and/or 12x12 and/or 16x16 font
    for(Font = 0; Font < NB_SYSTEM_FONTS; Font++)
    {
        // Clear unused descriptor
        FontDescriptor.pAddress  = nullptr;
        FontDescriptor.TotalSize = 0;
        for(Character = 0;   Character < 32;  Character++)  DB_Central.Set(&FontDescriptor, GFX_FONT_DESC_INFO, Font, Character);
        for(Character = 128; Character < 256; Character++)  DB_Central.Set(&FontDescriptor, GFX_FONT_DESC_INFO, Font, Character);

        FontInfo.Width     = StaticFontDescriptor[Font][0].Padding & 0x0F;
        FontInfo.Width     = (FontInfo.Width == 0) ? 16 : FontInfo.Width;   // Save the Width
        FontInfo.Height    = StaticFontDescriptor[Font][1].Padding & 0x0F;
        FontInfo.Height    = (FontInfo.Height == 0) ? 16 : FontInfo.Height;  // Save the height of the font into database
        FontInfo.Interline = StaticFontDescriptor[Font][2].Padding & 0x0F;   // Get Interline
        DB_Central.Set(&FontInfo, GFX_FONT_INFO, Font, 0);
        Counter = 0;

        for(Character = 32; Character < 128; Character++)
        {
            DB_Central.Get(&pMemory, GFX_FREE_RAM_POINTER, 0, 0);
            pDscFont                        = &StaticFontDescriptor[Font][Character - 32];
            FontDescriptor.pAddress         = pMemory;                                                          // Set address in memory
            FontDescriptor.WidthPixel       = pDscFont->Size >> 4;
            FontDescriptor.HeightPixel      = pDscFont->Size & 0x0F;
            FontDescriptor.LeftBearing      = pDscFont->Offset >> 4;
            FontDescriptor.RightBearing     = pDscFont->Padding >> 4;
            FontDescriptor.OffsetY          = pDscFont->Offset & 0x0F;

          #if (GRAFX_USE_FONT_SIZE_16 == DEF_ENABLED)
            if(Font == SYS_FT_16)    // Only for 16x16
            {
                if(Character > 34)         // 32,33,34 hold special value code
                {
                    if((FontDescriptor.WidthPixel == 0)  && ((pDscFont->Padding & 0x01) != 0)) FontDescriptor.WidthPixel  = 16;
                    if((FontDescriptor.HeightPixel == 0) && ((pDscFont->Padding & 0x02) != 0)) FontDescriptor.HeightPixel = 16;
                }
            }
          #endif

            FontDescriptor.HorizontalAdvance = FontDescriptor.WidthPixel + FontDescriptor.LeftBearing + FontDescriptor.RightBearing;
            FontDescriptor.TotalSize         = FontDescriptor.WidthPixel * FontDescriptor.HeightPixel;                      // Calculate size

          #ifdef LCD_INVERT_SCREEN
            if(Font.Size > 0)
            {
                pMemory += (Font.Size - 1);
            }
          #endif

            if(FontDescriptor.TotalSize == 0)
            {
                FontDescriptor.pAddress = nullptr;
            }
            else
            {
                SizeCount = 0;

                while(SizeCount < FontDescriptor.TotalSize)
                {
                    Value = FontRaw[Font][Counter++];

                    // First Nibble
                    if(SizeCount < FontDescriptor.TotalSize)
                    {
                        BitCount = (uint8_t)(((Value >> 4) & 0x07) + 1);
                        BitValue = Value >> 7;

                        while(BitCount--)
                        {
                            *pMemory = (BitValue != 0) ? 0xFF : 0x00;
                            SizeCount++;
                          #ifdef LCD_INVERT_SCREEN
                            pMemory--;
                          #else
                            pMemory++;
                          #endif
                        }
                    }

                    // Second Nibble
                    if(SizeCount < FontDescriptor.TotalSize)
                    {
                        BitCount = (uint8_t)((Value & 0x07) + 1);
                        BitValue = (uint8_t)((Value & 0x08) >> 3);

                        while(BitCount--)
                        {
                            *pMemory = (BitValue != 0) ? 0xFF : 0x00;
                            SizeCount++;
                            #ifdef LCD_INVERT_SCREEN
                                pMemory--;
                            #else
                                pMemory++;
                            #endif
                        }
                    }
                }
            }

            // Save the descriptor into database
            DB_Central.Set(&FontDescriptor, GFX_FONT_DESC_INFO, Font, Character);

            // Save Free SDRAM Pointer
            if(FontDescriptor.TotalSize != 0)
            {
                FontDescriptor.pAddress += FontDescriptor.TotalSize;
                DB_Central.Set(&FontDescriptor.pAddress, GFX_FREE_RAM_POINTER, 0, 0);
            }
        }
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintFont
//
//  Parameter(s):   FontDescriptor_t*   pDescriptor
//                  Cartesian_t*        pPos
//  Return:         none
//
//  Description:    This function will print a font to drawing layer with the drawing color
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void PrintFont(FontDescriptor_t* pDescriptor, Cartesian_t* pPos)
{
    myGrafx->PrintFont(pDescriptor, pPos);
}




#if 0

struct RotationLookUp_t
{
    int16_t CosQ;
    int16_t SinQ;
};


// table every 9°
const RotationLookUp_t RotationTable[40] =
{
    {  256,    0 }, {  253,   40 }, {  244,   79 }, {  228,  117 }, {  207,  154 }, {  181,  181 }, {  154,  207 }, {  117,  228 },
    {   79,  244 }, {   40,  253 }, {    0,  256 }, {  -40,  253 }, {  -79,  244 }, { -117,  228 }, { -154,  207 }, { -181,  181 },
    { -207,  154 }, { -228,  117 }, { -244,   79 }, { -253,   40 }, { -256,    0 }, { -253,  -40 }, { -244,  -79 }, { -228, -117 },
    { -207, -154 }, { -181, -181 }, { -154, -207 }, { -117, -228 }, {  -79, -244 }, {  -40, -253 }, {    0, -256 }, {   40, -253 },
    {   79, -244 }, {  117, -228 }, {  154, -207 }, {  181, -181 }, {  207, -154 }, {  228, -117 }, {  244,  -79 }, {  253,  -40 }
};

//every 5°
const RotationLookUp_t RotationTable[72] =
{   {  256,    0 }, {  255,   22 }, {  252,   44 }, {  247,   66 }, {  239,   88 }, {  228,  109 }, {  216,  128 }, {  200,  147 },
    {  182,  165 }, {  162,  181 }, {  140,  195 }, {  116,  208 }, {   90,  218 }, {   63,  226 }, {   34,  232 }, {    4,  235 },
    {  -26,  236 }, {  -56,  234 }, {  -85,  230 }, { -113,  223 }, { -140,  214 }, { -165,  202 }, { -188,  188 }, { -209,  171 },
    { -228,  152 }, { -244,  131 }, { -258,  108 }, { -269,   84 }, { -277,   58 }, { -282,   31 }, { -284,    4 }, { -283,  -23 },
    { -279,  -50 }, { -272,  -76 }, { -262, -101 }, { -249, -125 }, { -233, -147 }, { -214, -168 }, { -193, -187 }, { -169, -204 },
    { -143, -219 }, { -115, -231 }, {  -85, -241 }, {  -54, -248 }, {  -22, -253 }, {   11, -255 }, {   44, -255 }, {   76, -252 },
    {  108, -247 }, {  138, -239 }, {  167, -228 }, {  194, -215 }, {  219, -200 }, {  242, -182 }, {  263, -162 }, {  281, -140 },
    {  297, -116 }, {  309,  -90 }, {  319,  -63 }, {  326,  -34 }, {  330,   -4 }, {  331,   26 }, {  329,   56 }, {  324,   85 },
    {  316,  113 }, {  305,  140 }, {  291,  165 }, {  274,  188 }, {  255,  209 }, {  233,  228 }, {  209,  244 }, {  183,  258 }
};


// every 3° 
const RotationLookUp_t RotationTable[120] =
{
    {  256,    0 }, {  256,   13 }, {  255,   27 }, {  253,   40 }, {  250,   53 }, {  246,   66 }, {  241,   79 }, {  235,   92 }, 
    {  228,  104 }, {  220,  116 }, {  212,  128 }, {  202,  140 }, {  192,  151 }, {  181,  162 }, {  169,  172 }, {  156,  181 }, 
    {  143,  190 }, {  129,  199 }, {  114,  207 }, {   99,  214 }, {   83,  220 }, {   67,  226 }, {   50,  231 }, {   33,  235 }, 
    {   16,  239 }, {   -1,  241 }, {  -18,  243 }, {  -35,  244 }, {  -52,  244 }, {  -69,  244 }, {  -85,  243 }, { -101,  241 }, 
    { -117,  238 }, { -132,  234 }, { -147,  230 }, { -161,  224 }, { -175,  218 }, { -188,  211 }, { -200,  203 }, { -212,  194 }, 
    { -223,  185 }, { -233,  174 }, { -243,  163 }, { -251,  151 }, { -259,  138 }, { -266,  125 }, { -272,  111 }, { -277,   96 }, 
    { -281,   81 }, { -284,   66 }, { -286,   50 }, { -287,   34 }, { -287,   17 }, { -286,    0 }, { -284,  -17 }, { -281,  -34 },
    { -277,  -50 }, { -272,  -66 }, { -266,  -81 }, { -259,  -96 }, { -251, -111 }, { -243, -125 }, { -233, -138 }, { -223, -151 }, 
    { -212, -163 }, { -200, -174 }, { -188, -185 }, { -175, -194 }, { -161, -203 }, { -147, -211 }, { -132, -218 }, { -117, -224 }, 
    { -101, -230 }, {  -85, -234 }, {  -69, -238 }, {  -52, -241 }, {  -35, -243 }, {  -18, -244 }, {   -1, -244 }, {   16, -243 }, 
    {   33, -241 }, {   50, -239 }, {   67, -235 }, {   83, -231 }, {   99, -226 }, {  114, -220 }, {  129, -214 }, {  143, -207 }, 
    {  156, -199 }, {  169, -190 }, {  181, -181 }, {  192, -172 }, {  202, -162 }, {  212, -151 }, {  220, -140 }, {  228, -128 }, 
    {  235, -116 }, {  241, -104 }, {  246,  -92 }, {  250,  -79 }, {  253,  -66 }, {  255,  -53 }, {  256,  -40 }, {  256,  -27 }, 
    {  255,  -13 }, {  256,    0 }, {  256,   13 }, {  255,   27 }, {  253,   40 }, {  250,   53 }, {  246,   66 }, {  241,   79 }, 
    {  235,   92 }, {  228,  104 }, {  220,  116 }, {  212,  128 }, {  202,  140 }, {  192,  151 }, {  181,  162 }, {  169,  172 }  
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           RotateA8_Q8
//
//  Parameter(s):   const uint8_t*     pSrc
//                  uint8_t*           pDst
//                  int                Width
//                  int                Height
//                  int                AngleIndex
//
//  Return:         none
//
//  Description:    Rotate an 8-bit alpha (A8) bitmap using a Q8.8 fixed-point lookup table.
//                  Rotation is performed around the center of the box. The inner loop is fully
//                  incremental (no multiplications), making it extremely fast on Cortex-M.
//
//  Note(s):        - AngleIndex = angleDeg / step   (step = 3°, 5°, 9°, etc.)
//                  - RotTable[] must contain Q8.8 cos/sin values
//                  - Out-of-bounds samples are written as 0
//                  - Designed for MCU: no float, no FPU required
//                  - IMPORTANT: The destination buffer SHOULD BE SQUARE (Width == Height) to
//                    avoid clipping of rotated corners.
//
//-------------------------------------------------------------------------------------------------
void ImageRotationA8_Q8(const uint8_t* pSrc, uint8_t* pDst, int Width, int Height, int AngleIndex)
{
    int CenterX = Width  / 2;
    int CenterY = Height / 2;

    int16_t CosQ = RotTable[AngleIndex].CosQ;
    int16_t SinQ = RotTable[AngleIndex].SinQ;

    for(int y = 0; y < Height; y++)
    {
        int DeltaY = y - CenterY;

        // Starting point (x = 0) in Q8.8
        int RotatedX_Q = (((-CenterX) * CosQ) + (DeltaY * SinQ)) + (CenterX << 8);
        int RotatedY_Q = ((CenterX * SinQ) + (DeltaY * CosQ)) + (CenterY << 8);

        uint8_t* pDstPtr = pDst + y * Width;

        for(int x = 0; x < Width; x++)
        {
            int RotatedX = RotatedX_Q >> 8;
            int RotatedY = RotatedY_Q >> 8;

            if((unsigned)RotatedX < (unsigned)Width && (unsigned)RotatedY < (unsigned)Height)
            {
                pDstPtr[x] = pSrc[RotatedY * Width + RotatedX];
            }
            else
            {
                pDstPtr[x] = 0;
            }

            RotatedX_Q += CosQ;
            RotatedY_Q -= SinQ;
        }
    }
}


// usage
int angleIndex = angleDeg / 9;   // 0..39
RotateA8_Q8(srcA8, dstA8, w, h, angleIndex);




// Placement sur un bitmap
//-------------------------------------------------------------------------------------------------
//
//  Name:           ComputeCircularPlacement
//
//  Parameter(s):   int                CenterX
//                  int                CenterY
//                  int                Radius
//                  int                AngleIndex
//                  int                BitmapWidth
//                  int                BitmapHeight
//                  int*               pOutX
//                  int*               pOutY
//
//  Return:         none
//
//  Description:    Compute the top-left drawing position for a rotated bitmap placed on the
//                  circumference of a virtual circle. The angle uses the same Q8.8 LUT as the
//                  rotation function.
//
//  Note(s):        - AngleIndex = angleDeg / step   (step = 3°, 5°, 9°, etc.)
//                  - RotTable[] must contain Q8.8 cos/sin values
//                  - (pOutX, pOutY) receive the top-left corner where the bitmap must be drawn
//                  - The bitmap is centered on the circle point
//
//-------------------------------------------------------------------------------------------------
void ComputeCircularPlacement(int CenterX, int CenterY, int Radius, int AngleIndex, int BitmapWidth, int BitmapHeight, int* pOutX,  int* pOutY)
{
    int16_t CosQ = RotTable[AngleIndex].CosQ;
    int16_t SinQ = RotTable[AngleIndex].SinQ;

    // Bitmap position of the bitmap on the circle
    int PosX = CenterX + ((Radius * CosQ) >> 8);
    int PosY = CenterY + ((Radius * SinQ) >> 8);

    // Top-Left corner adjustment
    *pOutX = PosX - (BitmapWidth  / 2);
    *pOutY = PosY - (BitmapHeight / 2);
}

#endif


//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
