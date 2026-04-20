//-------------------------------------------------------------------------------------------------
//
//  File : lib_Grafx_GenDriver.cpp
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

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

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
//-------------------------------------------------------------------------------------------------
 void GrafxGenDriver::BlockCopy(void* pSrc, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t DstX, uint16_t DstY, PixelFormat_e SrcPixelFormat, BlendMode_e BlendMode)
{
    Box_t Box;

    Box.Pos.X = X;
    Box.Pos.Y = Y;
    Box.Size.Width  = Width;
    Box.Size.Height = Height;

    BlockCopy(pSrc, &Box, &Box.Pos, SrcPixelFormat, BlendMode);
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawRectangle
//
//   Parameter(s):  uint16_t    X
//                  uint16_t    Y
//                  uint16_t    Width
//                  uint16_t    Height
//    Return Value: none
//
//   Description:   Draw a rectangle
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawRectangle(uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height)
{
    Box_t Box;

    Box.Pos.X       = PosX;
    Box.Pos.Y       = PosY;
    Box.Size.Width  = Width;
    Box.Size.Height = Height;
    DrawRectangle(&Box);
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
void GrafxGenDriver::DrawBox(uint16_t PosX, uint16_t PosY, uint16_t Length, uint16_t Height, uint16_t Thickness)
{
    uint16_t X2 = PosX + Length;
    uint16_t Y2 = PosY + Height;

    DrawVLine(PosX,           PosY, Y2, Thickness);
    DrawVLine(X2 - Thickness, PosY, Y2, Thickness);
    DrawHLine(PosY,           PosX, X2, Thickness);
    DrawHLine(Y2 - Thickness, PosX, X2, Thickness);
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
void GrafxGenDriver::DrawHLine(uint16_t PosY, uint16_t PosX1, uint16_t PosX2, uint16_t Thickness)
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
void GrafxGenDriver::DrawVLine(uint16_t PosX, uint16_t PosY1, uint16_t PosY2, uint16_t Thickness)
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
//  Name:           LayerConfig
//
//  Parameter(s):   DisplayLayer* pLayer
//  Return:         None
//
//  Description:    Configuration for layer
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::LayerConfig(Layer_e Layer)
{
    DisplayLayer* pLayer = &LayerTable[Layer];
    LayerConfig(pLayer);
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: BlendFromImage
//
//   Parameter(s):  ImageID_e            ImageID
//                  Cartesian_t          Position
//                  BlendMode_e          BlendMode
//   Return Value:  none
//
//   Description:   Copy an image from memory region to window memory area
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::BlendFromImage(ImageID_e ImageID, Cartesian_t Position, BlendMode_e BlendMode)
{
    ImageInfo_t Image;
    Box_t       Box;

    if(ImageID != INVALID_IMAGE)
    {
        DB_Central.Get(&Image, GFX_IMAGE_INFO, uint16_t(ImageID), 0);
        Box.Pos.X       = Position.X;
        Box.Size.Width  = Image.Size.Width;
        Box.Pos.Y       = Position.Y;
        Box.Size.Height = Image.Size.Height;
        myGrafx->CopyLinear(Image.pPointer, &Box, Image.PixelFormat, BlendMode);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyLinear
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  PixelFormat_e   PixelFormat)
//                  BlendMode_e     BlendMode
//  Return:         None
//
//   Description:   Copy a rectangle region from linear memory region to square memory area
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
/*
void GrafxGenDriver::CopyLinear(void* pSrc, Box_t* pBox, PixelFormat_e PixelFormat, BlendMode_e BlendMode)
{
//    Not supported for now

    VAR_UNUSED(pSrc);
    VAR_UNUSED(pBox);
    VAR_UNUSED(PixelFormat);
    VAR_UNUSED(BlendMode);
}
*/

//-------------------------------------------------------------------------------------------------
//
//  Name:           CopyLinear
//
//  Parameter(s):   void*           pSrc
//                  Box_t*          pBox
//                  PixelFormat_e   PixelFormat)
//                  BlendMode_e     BlendMode
//  Return:         None
//
//   Description:   Copy a rectangle region from linear memory region to square memory area
//
//  Note(s):        Source is linear
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::CopyLinear(void* pSrc, uint16_t PosX, uint16_t PosY, uint16_t Width, uint16_t Height, PixelFormat_e PixelFormat, BlendMode_e BlendMode)
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

//-------------------------------------------------------------------------------------------------
//
//   Function Name: CopyLayerToLayer
//
//   Parameter(s):  Layer_e              SrcLayer
//                  Layer_e              DstLayer
//                  Box_t*               pBox
//   Return Value:  none
//
//   Description:   Copy a rectangle region from a layer to another layer
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::CopyLayerToLayer(Layer_e SrcLayer, Layer_e DstLayer, Box_t* pBox)
{
    DisplayLayer* pLayer;

    DisplayLayer::PushDrawing();
    DisplayLayer::SetDrawing(DstLayer);
    pLayer = &LayerTable[SrcLayer];
    BlockCopy((void*)pLayer->GetAddress(), pBox, &pBox->Pos, pLayer->GetPixelFormat(), CLEAR_BLEND);

    DisplayLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: CopyLayerToLayer
//
//   Parameter(s):  Layer_e             SrcLayer
//                  Layer_e             DstLayer
//                  uint16_t            X
//                  uint16_t            Y
//                  uint16_t            Width
//                  uint16_t            Height
//   Return Value:  none
//
//   Description:   Copy a rectangle region from a layer to another layer
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::CopyLayerToLayer(Layer_e SrcLayer, Layer_e DstLayer, uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height)
{
    Box_t        Box;

    Box.Pos.X       = X;
    Box.Size.Width  = Width;
    Box.Pos.Y       = Y;
    Box.Size.Height = Height;

    CopyLayerToLayer(SrcLayer, DstLayer, &Box);
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: CopyBlockLayerToLayer
//
//   Parameter(s):  Layer_e             SrcLayer
//                  Layer_e             DstLayer
//                  uint16_t            X
//                  uint16_t            Y
//                  uint16_t            Width
//                  uint16_t            Height
//   Return Value:  none
//
//   Description:   Copy a rectangle region from a layer to another layer
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::CopyLayerToLayer(Layer_e SrcLayer, Layer_e DstLayer, uint16_t SrcX, uint16_t SrcY, uint16_t DstX, uint16_t DstY, uint16_t Width, uint16_t Height)
{
    DisplayLayer* pLayer;
    Box_t         Box;
    Cartesian_t   Pos;

    Box.Pos.X       = SrcX;
    Box.Size.Width  = Width;
    Box.Pos.Y       = SrcY;
    Box.Size.Height = Height;

    Pos.X = DstX;
    Pos.Y = DstY;

    DisplayLayer::PushDrawing();
    DisplayLayer::SetDrawing(DstLayer);
    pLayer = &LayerTable[SrcLayer];
    BlockCopy((void*)pLayer->GetAddress(), &Box, &Pos, pLayer->GetPixelFormat(), CLEAR_BLEND);
    DisplayLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawDLine
//
//   Parameter(s):  float    X1
//                  float    Y1
//                  float    X2
//                  float    Y2
//   Return Value:  none
//
//   Description:   Draw a diagonal line of desire Thickness using the Xiaolin Wu Algorithm
//                  This algorithm allow line with anti-aliasing ( improvement on Bresenham)
//
//-------------------------------------------------------------------------------------------------
#if 0
void GrafxGenDriver::DrawDLine(float X1, float Y1, float X2, float Y2)
{
    int16_t DeltaX;
    int16_t DeltaY;
    float   Gradient;
    float   xEnd;
    float   yEnd;
    float   xGap;
    int16_t xPxl1;
    int16_t yPxl1;
    int16_t xPxl2;
    int16_t yPxl2;
    float   Intery;

    DeltaX = X2 - X1;
    DeltaY = Y2 - Y1;

    if(abs(DeltaX) > abs(DeltaY))
    {
        // Handle horizontal line
        if(X2 < X1)
        {
            float TP;
            TP = X1; X1 = X2; X2 = TP;
            TP = Y1; Y1 = Y2; Y2 = TP;
        }

        Gradient = DeltaY / DeltaX;

        // Handle first end of line point
        xEnd = static_cast<float>(round(X1));
        yEnd = Y1 + Gradient * (xEnd - X1);
        xGap = rfpart(X1 + 0.5);

        xPxl1 = static_cast<int>(xEnd);
        yPxl1 = ipart(yEnd);

        // Add the first endpoint
        plot(xPxl1, yPxl1, rfpart(yEnd) * xGap);
        plot(xPxl1, yPxl1 + 1, fpart(yEnd) * xGap);

        Intery = yEnd + Gradient;

        // Handle second end of line point
        xEnd = static_cast<float>(round(X2));
        yEnd = Y2 + Gradient * (xEnd - X2);
        xGap = fpart(X2 + 0.5);

        xPxl2 = static_cast<int>(xEnd);
        yPxl2 = ipart(yEnd);

        plot(xPxl2, yPxl2, rfpart(yEnd) * xGap);
        plot(xPxl2, yPxl2 + 1, fpart(yEnd) * xGap);

        // Main loop
        for(int16_t x = xPxl1 + 1; x < xPxl2; x++)
        {
            plot(x, ipart(Intery), rfpart(Intery));
            plot(x, ipart(Intery) + 1, fpart(Intery));
            Intery += Gradient;
        }
    }
    else
    {
         // TODO Handle horizontal lines by swapping X and Y
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawCircle
//
//   Parameter(s):  Circle_t*        pCircle
//                  PolygonMode_e    PolygonMode
//   Return Value:  none
//
//   Description:   Draw a circle on screen base on Bresenham algo
//
//   notes:         Option are:         FILL_POLY  -> for a completely fill circle
//                                      SHAPE_POLY -> for the contour shape only
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawCircle(Circle_t* pCircle, PolygonMode_e PolygonMode)
{
    DrawCircle(pCircle->Pos.X, pCircle->Pos.Y, pCircle->R, PolygonMode);
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: DrawCircle
//
//   Parameter(s):  uint16_t           X
//                  uint16_t           Y
//                  uint16_t           Radius
//                  GRAFX_PolygonMode_e   PolygonMode
//   Return Value:  none
//
//   Description:   Draw a circle on screen base on Bresenham algo
//
//   notes:         Option are:         POLY_FILL  -> for a completely fill circle
//                                      POLY_SHAPE -> for the contour shape only
//
//-------------------------------------------------------------------------------------------------
void GrafxGenDriver::DrawCircle(uint16_t PosX, uint16_t PosY, uint16_t Radius, PolygonMode_e PolygonMode)
{
    int16_t     X;
    int16_t     Y;
    int16_t     Decision;
    uint16_t    q1;
    uint16_t    q2;
    uint16_t    q3;
    uint16_t    q4;
    uint16_t    q5;
    uint16_t    q6;
    uint16_t    q7;
    uint16_t    q8;
    uint8_t     Skip;

    X = 0;
    Y = Radius;
    Decision = 3 - ((int16_t)Radius << 1);

    while(X <= Y)
    {
        q1 = PosX - (uint16_t)X;
        q2 = PosY - (uint16_t)Y;
        q3 = PosX - (uint16_t)Y;
        q4 = PosY - (uint16_t)X;
        q5 = PosX + (uint16_t)X;
        q6 = PosY + (uint16_t)Y;
        q7 = PosX + (uint16_t)Y;
        q8 = PosY + (uint16_t)X;

        if(PolygonMode == POLY_FILL)
        {
            q1 = (X > (int16_t)PosX) ? 0 : q1;
            q2 = (Y > (int16_t)PosY) ? 0 : q2;
            q3 = (Y > (int16_t)PosX) ? 0 : q3;
            q4 = (X > (int16_t)PosY) ? 0 : q4;

            DrawHLine(q2, q1, q5, 1);
            DrawHLine(q6, q1, q5, 1);
            DrawHLine(q8, q3, q7, 1);
            DrawHLine(q4, q3, q7, 1);
        }
        else
        {
            Cartesian_t Size;

            Size.X = 240; Size.Y = 320;
           // Size = DisplayLayer::GetMaxSize();
// TODO (Alain#1#): update this when it is time


            Skip = 0;
            if(X  >  (int16_t)PosX)   Skip |= 0x01;
            if(Y  >  (int16_t)PosY)   Skip |= 0x02;
            if(Y  >  (int16_t)PosX)   Skip |= 0x04;
            if(X  >  (int16_t)PosY)   Skip |= 0x08;
            if(q5 >= (int16_t)Size.X) Skip |= 0x10;
            if(q6 >= (int16_t)Size.Y) Skip |= 0x20;
            if(q7 >= (int16_t)Size.X) Skip |= 0x40;
            if(q8 >= (int16_t)Size.Y) Skip |= 0x80;

            if((Skip & 0x03) == 0) DrawPixel(q1, q2);
            if((Skip & 0x12) == 0) DrawPixel(q5, q2);
            if((Skip & 0x21) == 0) DrawPixel(q1, q6);
            if((Skip & 0x30) == 0) DrawPixel(q5, q6);
            if((Skip & 0x0C) == 0) DrawPixel(q3, q4);
            if((Skip & 0x48) == 0) DrawPixel(q7, q4);
            if((Skip & 0x84) == 0) DrawPixel(q3, q8);
            if((Skip & 0xC0) == 0) DrawPixel(q7, q8);
        }

        if(Decision < 0)
        {
            Decision += ((int16_t)X << 2) + 6;
        }
        else
        {
            Decision += ((((int16_t)X - (int16_t)Y) << 2) + 10);
            Y -= 1;
        }
        X++;
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_GRAFX == DEF_ENABLED)
