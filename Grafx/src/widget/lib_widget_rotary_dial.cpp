//-------------------------------------------------------------------------------------------------
//
//  File : lib_widget_rotary_dial.cpp
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
//  Requirement(s):
//
//                 270°
//                   │
//           180° ───┼─── 0°
//                   │
//                  90°
//
//   Standard
//      - Need widget ID
//      - Need Service
//      - optional SubService(At user discretion)
//   Widget specific
//      - Widget Position and Size:                 PosX, PosY, SizeX, SizeY    ( Box_t )
//      - Rotary Center Position and Radius:        PosX, PosY, CircleRadius    ( Circle_t )
//      - Arc Shape (Sector):                       Start Angle, End Angle      ( int16_t )
//      - Range (This is the number to print):      Minimum, Maximum            ( int16_t )
//      - Rotation Type:                            Bitmap Rotation Type        ( RotationID_e )
//      - Step Angle:                               Animation Step Angle        ( uint16_t )
//      - Text:                                     FontID, Text Color          ( Text_t )
//      - Options:                                  Option                      ( uint16_t )
//
//              Example:    Start angle = 270° and End Angle = 90°
//                              Then:   We have a vertical Dial
//                          Range Minimum is 1 and Range Max is 10.
//                              Then:   Around the 360° there is a number at every 36°
//                          For Rotation Type USE_ROTATION_TABLE_6 is chosen because 6° it is a
//                          divisor of 36°. It give 6 positions for animation.
//                          Difference (270°, 90°) = 180°
//                          Step Angle: 6°
//
//
//                            ┌──────────────┐
//                            │ *            │
//                            │       *      │
//                            │          *   │
//                            │            * │
//                            │             *│
//         Center of circle ->│ ○           *│ <- Focus Angle (can be in any positn inside the box)
//                            │             *│
//                            │            * │
//                            │          *   │
//                            │       *      │
//                            │ *            │
//                            └──────────────┘
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_USE_GRAFX
#ifdef ROTARY_DIAL_DEF

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define ROTARY_DIAL_DEBUG_GUI                   DEF_ENABLED

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

#if (ROTARY_DIAL_DEBUG_GUI == DEF_ENABLED)
bool RD_DebugDrawOnce;
#endif

//-------------------------------------------------------------------------------------------------
//
//  Constructor:    WidgetRotaryDial
//
//  Parameter(s):   RotaryDial_t         pRotaryDial         Pointer to RotaryDial_t structure
//
//  Description:    Initialize widget's service and build widget.
//
//-------------------------------------------------------------------------------------------------
WidgetRotaryDial::WidgetRotaryDial(RotaryDial_t* pRotaryDial)
{
    m_pRotaryDial = pRotaryDial;
    m_Value       = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Create
//
//  Parameter(s):   PageWidget_t* pPageWidget
//  Return:         Link_e        link
//
//  Description:    This function call service and create the widget
//
//-------------------------------------------------------------------------------------------------
Link_e WidgetRotaryDial::Create(PageWidget_t* pPageWidget)
{
    ServiceReturn_t* pService;

    m_pPageWidget  = pPageWidget;
    m_ServiceState = SERVICE_START;
    if((pService = ServiceCall(&m_pRotaryDial->Service, &m_ServiceState)) != nullptr)
    {
        if(pService->ServiceType == SERVICE_RETURN_TYPE1)
        {
            m_Value = ((ServiceType1_t*)pService)->Data;
        }

      #if (ROTARY_DIAL_DEBUG_GUI == DEF_ENABLED)
        RD_DebugDrawOnce = true;
      #endif

        Draw(pService);
        FreeServiceStruct(&pService);
    }

    return INVALID_LINK;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Refresh
//
//  Parameter(s):   MsgRefresh_t*        pMsg
//  Return:         Link_e               Link            Link to switch page if it's needed
//
//  Description:    This function call service to refresh widget
//
//-------------------------------------------------------------------------------------------------
Link_e WidgetRotaryDial::Refresh(MsgRefresh_t* pMsg)
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_REFRESH;

    // Invoke the application service for this meter (It might change by itself the m_ServiceState)
    if((pService = ServiceCall(&m_pRotaryDial->Service, &m_ServiceState)) != nullptr)
    {
        if(pService->ServiceType == SERVICE_RETURN_TYPE1)
        {
            m_Value = ((ServiceType1_t*)pService)->Data;
        }

        if(pService->ServiceType == SERVICE_RETURN_TYPE2)
        {
            m_Value = ((ServiceType2_t*)pService)->Data[0];
        }

        Draw(pService);
        FreeServiceStruct(&pService);
    }

    return INVALID_LINK;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Finalize
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    This function call service associated with widget to finalize it properly.
//
//-------------------------------------------------------------------------------------------------
void WidgetRotaryDial::Finalize()
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_FINALIZE;

    if((pService = ServiceCall(&m_pRotaryDial->Service, &m_ServiceState)) != nullptr)
    {
        FreeServiceStruct(&pService);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Draw
//
//  Parameter(s):   ServiceReturn_t* pService   Pointer to the service structure providing the
//                                              current dial state.  pService->IndexState contains
//                                              the dynamic rotation angle of the dial, expressed
//                                              in degrees, where a positive value represents a
//                                              clockwise visual rotation. Since the internal
//                                              rotation engine uses mathematical angles
//                                              (counter‑clockwise positive), this value is negated
//                                              before use.
//
//  Return:         None
//
//  Description:    Render the rotary dial widget onto the active drawing layer.
//
//                  The rotary dial consists of a sequence of elements (numbers, ticks, labels)
//                  distributed along a circular arc defined by StartAngle and EndAngle. The
//                  angular spacing between two consecutive elements is StepAngle. For example, a
//                  StepAngle of 20° places elements at 0°, 20°, 40°, 60°, etc.
//
//                  For each element in the range [StartValue, EndValue):
//                      - Compute its base angular position:
//                        BaseAngle = StartAngle + (ElementIndex - StartValue) * StepAngle
//
//                      - Apply the dynamic rotation offset:
//                        FinalAngle = BaseAngle - pService->IndexState
//                        (The subtraction ensures that a positive IndexState rotates the dial
//                        clockwise on screen, matching visual expectations.)
//
//                      - Convert FinalAngle to Cartesian coordinates on the dial radius.
//
//                      - Generate the bitmap for the element text
//                        (may contain multiple characters).
//
//                      - Rotate the bitmap using the Q8.8 rotation engine.
//
//                      - Compute the top‑left placement of the rotated bitmap using
//                        ComputeCircularPlacement(), ensuring proper alignment on the circular
//                        path.
//
//                      - Blend the rotated bitmap onto the construction or foreground layer,
//                        depending on the active rendering mode.
//
//      When construction layers are enabled, the function restores the background region
//      behind each element before redrawing it, ensuring clean incremental updates
//      without visual artifacts.
//
//      All temporary bitmaps are allocated from the graphics memory pool and released
//      immediately after use.
//
//      Upon completion, the drawing state is restored to its previous configuration.
//
//-------------------------------------------------------------------------------------------------

/*
struct RotaryDial_t
{
    Service_t      Service;
    Box_t          Box;                         // This is the box position and dimension for this widget construction
    int            Radius;                      // Radius offset where to draw element
    uint16_t       StartAngle;                  // Static position of the display start angle (number outside angle range cover by StartAngle and EndAngle are not drawed)
    uint16_t       EndAngle;                    // Static position of the display end angle
    uint16_t       StepAngle;                   // Static value of the step angle    Ex. 20 Degree tell this widget to draw a number at every 20 degrees
    int16_t        StartValue;                  // Static value to tell this widget to draw from 0 Degree with this value. Signed value.
    int16_t        EndValue;                    // Static value to tell this widget to draw up to X Degree with this end value. Signed value.
    uint16_t       MovingStepAngle;             // This is the angle of the moving dial versus 0 degree at top of the circle
    Font_e         FontID;                      // Font ID to use on this widget
    uint16_t       Options;                     // Drawing option.
};
*/
#define TEST_ANGLE 64

void WidgetRotaryDial::Draw(ServiceReturn_t* pService)
{
    BlendMode_e BlendMode;
   // OffsetAngle

    //int16_t     StartAngle;
    //int16_t     EndAngle;
    //uint16_t    DisplayAngle;
    //uint16_t    Range;
    //uint16_t    Radius;

    BlendMode = ((m_pRotaryDial->Options & GRAFX_OPTION_BLEND_CLEAR) != 0) ? CLEAR_BLEND : ALPHA_BLEND;
    DisplayLayer::PushDrawing();

  #if (ROTARY_DIAL_DEBUG_GUI == DEF_ENABLED)
    DisplayLayer::SetDrawing(FOREGROUND_DISPLAY_LAYER_0);
    DisplayLayer::SetColor(RED);

    if(RD_DebugDrawOnce == true)
    {
        RD_DebugDrawOnce = false;

        DrawDebugBox(&m_pRotaryDial->Box);
        DrawBox(m_pRotaryDial->Arc.Circle.Pos.X, m_pRotaryDial->Arc.Circle.Pos.Y, 3, 3, 1);

    }

    myGrafx->DrawCircle(&m_pRotaryDial->Box,
                        m_pRotaryDial->Arc.Circle.Pos.X,
                        m_pRotaryDial->Arc.Circle.Pos.Y,
                        m_pRotaryDial->Arc.Circle.Radius - 10,
                        POLY_SHAPE);

	uint16_t RotaryOffset = ((ServiceType1_t*)pService)->Data;

	Text_t Text = {};
	Text.Box.Size.Width = 100;
	Text.Box.Size.Height = 20;
	Text.Font = m_pRotaryDial->FontID;
	Text.Blend = BlendMode;
	Text.Label = LBL_INT;
	Text.Color[0] = 0x00FF0000;

	for(i = 0; i < 60; i++)
	{
		((ServiceType1_t*)pService)->Data = ((i % 6) != 0) ? ' ' : (i / 6) + '0';
		
		RotaryOffset += i;
		RotaryOffset % 60;
		ComputeCircularPlacement(m_pRotaryDial->Arc.Circle.Pos.X, m_pRotaryDial->Arc.Circle.Pos.Y, m_pRotaryDial->Arc.Circle.Radius, RotaryOffset, &Text.Box.Pos.X,  &Text.Box.Pos.Y, m_pRotaryDial->RotationTable);

		if((Text.Box.Pos.X < 280) && (Text.Box.Pos.Y < 220))
		{
			DisplayLayer::SetDrawing(CONSTRUCTION_FOREGROUND_LAYER);
		   #if (GRAFX_USE_CONSTRUCTION_ON_SINGLE_LAYER == DEF_ENABLED)
			myGrafx->CopyBackgroundToConstruction(Text.Box.Pos);              		// if the display has no multilayer capability.
		   #endif


			WidgetPrint(&Text, pService, false);//, TEST_ANGLE);

		   #if (GRAFX_USE_FULL_FRAME_CONSTRUCTION_LAYER == DEF_DISABLED)
			myGrafx->CopyWidgetToDevice(Text.Box.Size, Text.Box.Pos);
		   #endif
		}
	}

  #else
   #if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
    DisplayLayer::SetDrawing(CONSTRUCTION_FOREGROUND_LAYER);
   #else
    DisplayLayer::SetDrawing(FOREGROUND_DISPLAY_LAYER_0);
   #endif
  #endif


    DisplayLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------

#endif // ROTARY_DIAL_DEF
#endif // DIGINI_USE_GRAFX




/*
class ImageRotatorQ8
{
    public:

            void RotateAlphaIntoScreen(const uint8_t*  srcAlpha,     // 8-bit alpha mask (font/shape)
                                       BoxSize_t       srcSize,      // srcW, srcH
                                       uint32_t*       dstScreen,    // ARGB8888 construction layer
									   BoxSize_t       screenSize,   // screenW, screenH
									   uint32_t        fgColorARGB,  // solid color to apply with alpha
									   int16_t         angleQ8,      // Q8.8 angle
								       Cartesian_t     centerScreen, // rotation center in SCREEN coords
									   const Box_t&    clipBox       // widget box in SCREEN coords
    );
};

I will need to print the font into into a temporary box (buffer) using DMA2D..  then do the rotation and check clipping before writing pixel.

struct TempBox_t
{
    uint8_t*  AlphaBuf;   // or uint32_t* if you let DMA2D render ARGB
    BoxSize_t Size;       // width/height in pixels
};

class ImageRotatorQ8
{
public:
    void RotateAlphaBoxIntoScreen(
        const uint8_t*  srcAlpha,     // temp box buffer (local coords)
        BoxSize_t       srcSize,      // temp box size

        uint32_t*       dstScreen,    // ARGB8888 construction layer
        BoxSize_t       screenSize,   // full screen size

        uint32_t        fgColorARGB,  // color to apply with alpha

        int16_t         angleQ8,      // Q8.8 angle

        Cartesian_t     centerScreen, // rotation center in SCREEN coords

        const Box_t&    clipBox       // widget box in SCREEN coords
    );
};

void RotaryDial::Draw(uint32_t* screenBuf, BoxSize_t screenSize)
{
    // 1) Ask DMA2D to render font/shape into temp alpha box
    //    TempBox_t Temp;  // owned by RotaryDial or shared pool
    //    DMA2D_RenderGlyphToAlpha(Temp.AlphaBuf, Temp.Size, ...);

    int16_t angleQ8 = AngleDeg << 8;

    Cartesian_t centerScreen;
    centerScreen.X = WidgetBox.Pos.X + CenterInWidget.X;
    centerScreen.Y = WidgetBox.Pos.Y + CenterInWidget.Y;

    Rotator.RotateAlphaBoxIntoScreen(
        Temp.AlphaBuf,
        Temp.Size,
        screenBuf,
        screenSize,
        DialColorARGB,
        angleQ8,
        centerScreen,
        WidgetBox   // clip to widget box
    );
}


*/
