//-------------------------------------------------------------------------------------------------
//
//  File : lib_widget_round_meter.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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

/*

Development notes:

This widget create pie slice according to angle from start angle to end angle. the beauty of this 
widget will be that it can draw the pie slice on a specific color from the actual layer.
the icon on what the pie must be blend should be already draw in the sequence on refresh widget. 


*/


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_USE_GRAFX
#ifdef ROUND_METER_DEF

//-------------------------------------------------------------------------------------------------
//
//  Constructor:    CRoundMeter
//
//  Parameter(s):   RoundMeter_t         pRoundMeter         Pointer to RoundMeter_t structure
//
//  Description:    Initialize widget's service and build widget.
//
//-------------------------------------------------------------------------------------------------
CRoundMeter::CRoundMeter(RoundMeter_t* pRoundMeter)
{
    m_pRoundMeter = pRoundMeter;
    m_Value       = 0;
    m_pRoundMeter->Text.Blend = ALPHA_BLEND;
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
Link_e CRoundMeter::Create(PageWidget_t* pPageWidget)
{
    ServiceReturn_t* pService;

    m_pPageWidget  = pPageWidget;
    m_ServiceState = SERVICE_START;
    if((pService = ServiceCall(&m_pRoundMeter->Service, &m_ServiceState)) != nullptr)
    {
        if(pService->ServiceType == SERVICE_RETURN_TYPE1)
        {
            m_Value = ((ServiceType1_t*)pService)->Data;
        }

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
Link_e CRoundMeter::Refresh(MsgRefresh_t* pMsg)
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_REFRESH;

    // Invoke the application service for this meter (It might change by itself the m_ServiceState)
    if((pService = ServiceCall(&m_pRoundMeter->Service, &m_ServiceState)) != nullptr)
    {
        if(pService->ServiceType == SERVICE_RETURN_TYPE1)
        {
            m_Value = ((ServiceType1_t*)pService)->Data;
        }

        if(pService->ServiceType == SERVICE_RETURN_TYPE2)
        {
            m_Value = ((ServiceType2_t*)pService)->Data[0].u_16;
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
void CRoundMeter::Finalize()
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_FINALIZE;

    if((pService = ServiceCall(&m_pRoundMeter->Service, &m_ServiceState)) != nullptr)
    {
        FreeServiceStruct(&pService);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Draw
//
//  Parameter(s):   ServiceReturn_t* pService
//  Return:         None
//
//
//  Description:    Draw the meter on display.
//
//-------------------------------------------------------------------------------------------------
void CRoundMeter::Draw(ServiceReturn_t* pService)
{
    uint16_t    Angle;
    uint16_t    EndAngle;
    BlendMode_e  BlendMode;

    BlendMode = ((m_pRoundMeter->Options & OPTION_BLEND_CLEAR) != 0) ? CLEAR_BLEND : ALPHA_BLEND;

    EndAngle = m_pRoundMeter->EndAngle;
    if(EndAngle < m_pRoundMeter->StartAngle) EndAngle += 360;
    Angle = (m_Value * (EndAngle - m_pRoundMeter->StartAngle)) / m_pRoundMeter->Range;

    CLayer::PushDrawing();

  #if (GRAFX_DEBUG_GUI == DEF_ENABLED)
    CLayer::SetDrawing(FOREGROUND_DISPLAY_LAYER_0);
  #else
   #if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
    CLayer::SetDrawing(CONSTRUCTION_FOREGROUND_LAYER);
   #else
    CLayer::SetDrawing(FOREGROUND_DISPLAY_LAYER_0);
   #endif
  #endif

     myGrafx->CopyLinear(m_pRoundMeter->Minimum, m_pRoundMeter->Box.Pos, BlendMode);
  //  CursorOnCircle(m_pRoundMeter->Cursor, &m_pRoundMeter->Box.Pos, m_pRoundMeter->Radius, 0);         // Put a cursor at the start

    if((m_pRoundMeter->Options & OPTION_CURSOR_PRIORITY) != 0)
    {
   VAR_UNUSED(Angle);
  //      CursorOnCircle(m_pRoundMeter->Cursor, &m_pRoundMeter->Box.Pos, m_pRoundMeter->Radius, Angle); // Put cursor at angle if priority is enable
    }

 //   RoundMeter(m_pRoundMeter->Maximum, m_pRoundMeter->Box.Pos, m_pRoundMeter->Radius, m_pRoundMeter->StartAngle, Angle, ALPHA_BLEND);  // Merge meter according to value

    if((m_pRoundMeter->Options & OPTION_CURSOR_PRIORITY) == 0)
    {
//        CursorOnCircle(m_pRoundMeter->Cursor, &m_pRoundMeter->Box.Pos, m_pRoundMeter->Radius, Angle);                      // Put cursor at angle if priority is disable
    }
    WidgetPrint(&m_pRoundMeter->Text, pService);
    CLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawOnce
//
//  Parameter(s):   ServiceReturn_t* pService
//  Return:         None
//
//
//  Description:    Draw only once what does not need to be refreshed
//
//-------------------------------------------------------------------------------------------------
void CRoundMeter::DrawOnce(ServiceReturn_t* pService)
{

    CLayer::PushDrawing();

    // Copy merge stuff

    CLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------

#endif // ROUND_METER_DEF
#endif // DIGINI_USE_GRAFX
