//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_layer.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_LAYER_AS_COLOR_TABLE(ENUM_ID, WORK_LAYER, PIXEL_FORMAT, SIZE_X, SIZE_Y) DisplayLayer(ENUM_ID, 0, WORK_LAYER, SIZE_X, SIZE_Y, PIXEL_FORMAT),

//-------------------------------------------------------------------------------------------------
//
//   global Clayer array
//
//-------------------------------------------------------------------------------------------------

#ifdef LAYER_DEF
DisplayLayer LayerTable[LAYER_COUNT] =
{
    LAYER_DEF(EXPAND_X_LAYER_AS_COLOR_TABLE)
};
#endif

//-------------------------------------------------------------------------------------------------
//
//   Static Variables
//
//-------------------------------------------------------------------------------------------------

Layer_e             DisplayLayer::m_ActiveDrawingLayer;
      //static DisplayLayer*      m_pActiveDrawingLayer;

DisplayLayer*       DisplayLayer::m_pActiveBG_Layer;
#if (GRAFX_USE_CONSTRUCTION_BACKGROUND_LAYER == DEF_ENABLED)
DisplayLayer*       DisplayLayer::m_pConstructBG_Layer;
#endif
DisplayLayer*       DisplayLayer::m_pActiveFG_Layer;
#if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
DisplayLayer*       DisplayLayer::m_pConstructFG_Layer;
#endif
uint8_t             DisplayLayer::m_LayerStackCounter;
Layer_e             DisplayLayer::m_LayerStack[CLAYER_STACK_LEVEL];

// This table must match the enum PixelFormat_e (lib_grafx_enum.h)
const uint8_t       DisplayLayer::m_LayerPixelSize[PIXEL_FORMAT_COUNT] =
{
  #if (GRAFX_COLOR_ARGB8888 == DEF_ENABLED)
    4,
  #endif
  #if (GRAFX_COLOR_RGB888 == DEF_ENABLED)
    4,
  #endif
  #if (GRAFX_COLOR_RGB565 == DEF_ENABLED)
    2,
  #endif
  #if (GRAFX_COLOR_ARGB1555 == DEF_ENABLED)
    2,
  #endif
  #if (GRAFX_COLOR_ARGB4444 == DEF_ENABLED)
    4,
  #endif
  #if (GRAFX_COLOR_L8 == DEF_ENABLED)
    1,
  #endif
  #if (GRAFX_COLOR_AL44 == DEF_ENABLED)
    1,
  #endif
  #if (GRAFX_COLOR_AL88 == DEF_ENABLED)
    2,
  #endif
  #if (GRAFX_COLOR_L4 == DEF_ENABLED)
    1,
  #endif
  #if (GRAFX_COLOR_A8 == DEF_ENABLED)
    1,
  #endif
  #if (GRAFX_COLOR_A4 == DEF_ENABLED)
    1,
  #endif
  #if (GRAFX_COLOR_RGB332 == DEF_ENABLED)
    1,
  #endif
  #if (GRAFX_COLOR_RGB444 == DEF_ENABLED)
    2,
  #endif
    0,    // Dummy
};

//-------------------------------------------------------------------------------------------------
//
//   class: DisplayLayer
//
//
//   Description:   Class to handle layer properties and function
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   DisplayLayer
//
//   Parameter(s):  Layer_e      VirtualLayer
//                  uint32_t     LayerAddress
//                  LayerType_e  ActiveOnLayer
//                  uint16_t     SizeX
//                  uint16_t     SizeY
//                  PixelFormat_e PixelFormat
//   Return Value:  none
//
//-------------------------------------------------------------------------------------------------
DisplayLayer::DisplayLayer(Layer_e          VirtualLayer,
                           uint32_t         LayerAddress,
                           LayerType_e      ActiveOnLayer,
                           uint16_t         SizeX,
                           uint16_t         SizeY,
                           PixelFormat_e    PixelFormat)
{
    m_VirtualLayer   = VirtualLayer;
    m_LayerAddress   = LayerAddress;
    m_ActiveOnLayer  = ActiveOnLayer;
    m_Size.X         = SizeX;
    m_Size.Y         = SizeY;
    m_PixelSize      = DisplayLayer::GetPixelSize(PixelFormat);
    m_TotalSize      = (uint32_t)SizeX * (uint32_t)SizeY * (uint32_t)m_PixelSize;
    m_PixelFormat    = PixelFormat;
    m_Alpha          = 255;
    m_Color          = GetFormatColor(PixelFormat, BLACK);
    m_TextColor      = GetFormatColor(PixelFormat, WHITE);

    DisplayLayer::m_LayerStackCounter = CLAYER_STACK_LEVEL;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: Clear
//
//   Parameter(s):  none
//   Return Value:  none
//
//   Description:   Clear the layer
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::Clear(void)
{
    uint32_t DrawingColor = m_Color;
    DisplayLayer::PushDrawing();
    SetDrawing(m_VirtualLayer);         // why m_VirtualLayer ????
    m_Color = 0;
    //myGrafx->DrawRectangle(0, 0, m_Size.X, m_Size.Y); //TODO commented while working on FMC8080
    m_Color = DrawingColor;
    DisplayLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetAddress
//
//   Parameter(s):  uint32_t           Address
//   Return Value:  none
//
//   Description:   Set layer base address
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetAddress(uint32_t Address)
{
    m_LayerAddress = Address;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetAddress
//
//   Parameter(s):  none
//   Return Value:  uint32_t           Address
//
//   Description:   Get layer base address
//
//-------------------------------------------------------------------------------------------------
uint32_t DisplayLayer::GetAddress(void)
{
    return m_LayerAddress;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetTotalSize
//
//   Parameter(s):  none
//   Return Value:  uint32_t           Address
//
//   Description:   Get the total size of the layer
//
//-------------------------------------------------------------------------------------------------
uint32_t DisplayLayer::GetTotalSize(void)
{
    return m_TotalSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetSize
//
//   Parameter(s):  none
//   Return Value:  Cartesian_t  Size
//
//   Description:   Get the size in X and Y of the layer
//
//-------------------------------------------------------------------------------------------------
Cartesian_t DisplayLayer::GetSize(void)
{
    return m_Size;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetPixelFormat
//
//   Parameter(s):  none
//   Return Value:  PixelFormat_e    PixelFormat
//
//   Description:   Return the pixel format value
//
//-------------------------------------------------------------------------------------------------
PixelFormat_e DisplayLayer::GetPixelFormat(void)
{
    return m_PixelFormat;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetPixelSize
//
//   Parameter(s):  none
//   Return Value:  uint8_t    PixelSize
//
//   Description:   Return the pixel size
//
//-------------------------------------------------------------------------------------------------
uint8_t DisplayLayer::GetPixelSize(void)
{
    return m_PixelSize;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetPixelSize
//
//   Parameter(s):  PixelFormat_e Pixel format to get the pixel size
//   Return Value:  uint8_t        PixelSize
//
//   Description:   Return the pixel size
//
//-------------------------------------------------------------------------------------------------
uint8_t DisplayLayer::GetPixelSize(PixelFormat_e PixelFormat)
{
    return m_LayerPixelSize[PixelFormat];
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetActive
//
//   Parameter(s):  Layer_e          ActiveLayer
//   Return Value:  none
//
//   Description:   Set the virtual layer to be use as active layer
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetActive(LayerType_e ActiveLayer)
{
    m_ActiveOnLayer = ActiveLayer;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetActive
//
//   Parameter(s):  none
//   Return Value:  LayerType_e      ActiveLayer
//
//   Description:   Get setting for active layer
//
//-------------------------------------------------------------------------------------------------
LayerType_e DisplayLayer::GetActive(void)
{
    return m_ActiveOnLayer;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetVitual
//
//   Parameter(s):  none
//   Return Value:  Layer_e          ActiveLayer
//
//   Description:   Get setting for virtual layer
//
//-------------------------------------------------------------------------------------------------
Layer_e DisplayLayer::GetVirtual(void)
{
    return m_VirtualLayer;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetAlpha
//
//   Parameter(s):  uint8_t       Alpha
//   Return Value:  none
//
//   Description:   Set the Alpha constant for the layer.
//
//   note(s):       Update the physical layer if active
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetAlpha(uint8_t Alpha)
{
    m_Alpha = Alpha;

    if(m_ActiveOnLayer != LAYER_VIRTUAL)
    {
        // Force new value on global alpha blending
        SetActiveLayer(m_ActiveOnLayer, m_VirtualLayer);
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetAlpha
//
//   Parameter(s):  none
//   Return Value:  uint8_t       Alpha
//
//   Description:   Get the Alpha constant for the layer
//
//-------------------------------------------------------------------------------------------------
uint8_t DisplayLayer::GetAlpha(void)
{
    return m_Alpha;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetColor
//
//   2 Overloaded functions
//
//   Parameter(s):  ColorTable_e    Index
//   Return Value:  none
//
//   Parameter(s):  uint32_t        Color
//   Return Value:  none
//
//   Description:   Set the drawing color
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetColor(ColorTable_e Index)
{
     SetColor(GetFormatColor(LayerTable[DisplayLayer::m_ActiveDrawingLayer].m_PixelFormat, Index));
}

void DisplayLayer::SetColor(uint32_t Color)
{
     LayerTable[DisplayLayer::m_ActiveDrawingLayer].m_Color = Color;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetTextColor
//
//   2 Overloaded functions
//
//   Parameter(s):  ColorTable_e    Index
//   Return Value:  none
//
//   Parameter(s):  uint32_t        Color
//   Return Value:  none
//
//   Description:   Set the text drawing color
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetTextColor(ColorTable_e Index)
{
     SetTextColor(GetFormatColor(LayerTable[DisplayLayer::m_ActiveDrawingLayer].m_PixelFormat, Index));
}

void DisplayLayer::SetTextColor(uint32_t Color)
{
     LayerTable[DisplayLayer::m_ActiveDrawingLayer].m_TextColor = Color;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetColor
//
//   Parameter(s):  none
//   Return Value:  uint32_t   Value
//
//   Description:   Return the color value according to pixel format
//
//-------------------------------------------------------------------------------------------------
uint32_t DisplayLayer::GetColor(void)
{
    return LayerTable[DisplayLayer::m_ActiveDrawingLayer].m_Color;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetTextColor
//
//   Parameter(s):  none
//   Return Value:  uint32_t   Value
//
//   Description:   Return the text color value according to pixel format
//
//-------------------------------------------------------------------------------------------------
uint32_t DisplayLayer::GetTextColor(void)
{
    return LayerTable[DisplayLayer::m_ActiveDrawingLayer].m_TextColor;
}
//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetActiveLayer
//
//   Parameter(s):  LayerType_e LayerType               Physical layer affected
//                  DisplayLayer*     pLayer                  Virtual layer to used
//   Return Value:  none
//
//   Description:   Set the active layer according to Parameter
//
//   Note(s)        For practical purpose, the previous layer will be assign to construct duty
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetActiveLayer(LayerType_e LayerType, DisplayLayer* pLayer)
{
    switch(LayerType)
    {
        case LAYER_BACKGROUND:
        {
            // Deactivate physical access from previous owner of the active layer
            if(DisplayLayer::m_pActiveBG_Layer != nullptr)
            {
                DisplayLayer::m_pActiveBG_Layer->SetActive(LAYER_VIRTUAL);                    // Set the old layer to virtual status
              #if (GRAFX_USE_CONSTRUCTION_BACKGROUND_LAYER == DEF_ENABLED)
                DisplayLayer::m_pConstructBG_Layer = DisplayLayer::m_pActiveBG_Layer;               // And also assign this layer to construct duty
              #endif
            }

            // Activate new owner of the active layer
            DisplayLayer::m_pActiveBG_Layer = pLayer;
            DisplayLayer::m_pActiveBG_Layer->SetActive(LayerType);
            myGrafx->LayerConfig(DisplayLayer::m_pActiveBG_Layer);
        }
        break;

        case LAYER_FOREGROUND:
        {
            // Deactivate physical access from previous owner of the active layer
            if(DisplayLayer::m_pActiveFG_Layer != nullptr)
            {
                DisplayLayer::m_pActiveFG_Layer->SetActive(LAYER_VIRTUAL);                    // Set the old layer to virtual status
              #if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
                DisplayLayer::m_pConstructFG_Layer = DisplayLayer::m_pActiveFG_Layer;               // And also assign this layer to construct duty
              #endif
            }

            // Activate new owner of the active layer
            DisplayLayer::m_pActiveFG_Layer = pLayer;
            DisplayLayer::m_pActiveFG_Layer->SetActive(LayerType);
            myGrafx->LayerConfig(DisplayLayer::m_pActiveFG_Layer);
        }
        break;

        case LAYER_VIRTUAL:
        {
            // TODO (Alain#1#): Handle the virtual layer if any

        }
        break;

        case LAYER_DUMMY:
        {
            // TODO (Alain#1#): Handle the virtual layer if any

        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetActiveLayer
//
//   Parameter(s):  LayerType_e LayerType               Physical layer affected
//                  Layer_e     Layer                   Virtual layer to used
//   Return Value:  none
//
//   Description:   Set the active layer according to Parameter
//
//   Note(s)        For practical purpose, the previous layer will be assign to construct duty
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetActiveLayer(LayerType_e LayerType, Layer_e Layer)
{
    SetActiveLayer(LayerType, &LayerTable[Layer]);
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetDrawing
//
//   Parameter(s):  none
//   Return Value:  Layer_e     Layer
//
//   Description:   Get the active drawing layer.
//
//-------------------------------------------------------------------------------------------------
Layer_e DisplayLayer::GetDrawing(void)
{
    return DisplayLayer::m_ActiveDrawingLayer;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetDrawingLayer
//
//   Parameter(s):  Layer_e     Layer
//   Return Value:  none
//
//   Description:   Set the default layer for drawing (it can be virtual or physical)
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::SetDrawing(Layer_e Layer)
{
    DisplayLayer::m_ActiveDrawingLayer = Layer;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: PushDrawingLayer
//
//   Parameter(s):  none
//   Return Value:  none
//
//   Description:   Push the actual drawing layer on a stack
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::PushDrawing(void)
{
    if(m_LayerStackCounter != 0)
    {
        m_LayerStackCounter--;
        m_LayerStack[m_LayerStackCounter] = m_ActiveDrawingLayer;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: PopDrawingLayer
//
//   Parameter(s):  none
//   Return Value:  none
//
//   Description:   Get a drawing layer from stack
//
//-------------------------------------------------------------------------------------------------
void DisplayLayer::PopDrawing(void)
{
    if(DisplayLayer::m_LayerStackCounter < CLAYER_STACK_LEVEL)
    {
        DisplayLayer::m_ActiveDrawingLayer = DisplayLayer::m_LayerStack[m_LayerStackCounter];
        DisplayLayer::m_LayerStackCounter++;
    }
}

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function Name: ClearLayer
//
//   Parameter(s):  none
//   Return Value:  none
//
//   Description:   Clear the layer
//
//-------------------------------------------------------------------------------------------------
//void ClearLayer()
//{
//    ActiveDrawingLayer->Clear();
//}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: ToggleLayer
//
//   Parameter(s):  none
//   Return Value:  none
//
//   Description:   Toggle between active and construct layer
//
//-------------------------------------------------------------------------------------------------
//void ToggleLayer()
//{
//    SetActiveLayer(LAYER_BACKGROUND, pConstructBG_Layer);
//    SetActiveLayer(LAYER_FOREGROUND, pConstructFG_Layer);
//}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetConstructLayer
//
//   Parameter(s):  LayerType_e  LayerType              Physical layer affected
//                  Layer_e     Layer                   Virtual layer to used
//   Return Value:  none
//
//   Description:   Set the construct layer
//
//-------------------------------------------------------------------------------------------------
/*
void SetConstructLayer(LayerType_e LayerType, Layer_e Layer)
{
    switch(LayerType)
    {
        case LAYER_BACKGROUND:
        {
            pConstructBG_Layer = &LayerTable[Layer];            // And also assign this layer to construct duty
            break;
        }

        case LAYER_FOREGROUND:
        {
            pConstructFG_Layer = &LayerTable[Layer];            // And also assign this layer to construct duty
            break;
        }
    }
    pConstructFG_Layer->SetActiveLayer(LAYER_VIRTUAL);              // Make sure it is to virtual status
}
*/

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetDrawingOnConstructLayer
//
//   Parameter(s):  LayerType_e  LayerType
//   Return Value:  none
//
//   Description:   Set drawing layer on one of hte construct layer
//
//-------------------------------------------------------------------------------------------------
/*
void SetDrawingOnConstructLayer(LayerType_e LayerType)
{
    if(LayerType == LAYER_BACKGROUND) pActiveDrawingLayer = pConstructBG_Layer;
    else                              pActiveDrawingLayer = pConstructFG_Layer;
}
*/

//-------------------------------------------------------------------------------------------------
//
//   Function Name: SetAlphaLayer
//
//   Parameter(s):  uint8_t    Alpha
//   Return Value:  none
//
//   Description:   Set the alpha value globally for the drawing layer
//
//-------------------------------------------------------------------------------------------------
/*
void SetAlphaLayer(uint8_t Alpha)
{
    pActiveDrawingLayer->SetAlpha(Alpha);
}
*/
//-------------------------------------------------------------------------------------------------
#endif // DIGINI_USE_GRAFX




