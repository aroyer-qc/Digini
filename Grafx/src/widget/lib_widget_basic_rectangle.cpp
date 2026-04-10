//-------------------------------------------------------------------------------------------------
//
//  File : lib_widget_basic_rectangle.cpp
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
#ifdef BASIC_RECT_DEF

//-------------------------------------------------------------------------------------------------
//
//  Constructor:    WidgetBasicRectangle
//
//  Parameter(s):   BasicRect_t     pBasicRect        Pointer to BasicRectangle_t structure
//
//  Description:    Initialize widget's service and build widget.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
WidgetBasicRectangle::WidgetBasicRectangle(BasicRect_t* pBasicRect)
{
    m_pBasicRect = pBasicRect;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Create
//
//  Parameter(s):   sPageWidget* pPageWidget
//  Return:         Link_e        link
//
//  Description:    This function call service and create the widget
//
//  Note(s)         No link on creation of button, always invalid
//
//-------------------------------------------------------------------------------------------------
Link_e WidgetBasicRectangle::Create(PageWidget_t* pPageWidget)
{
    ServiceReturn_t* pService;
    EventArea_t      EventArea;

    m_pPageWidget  = pPageWidget;
    m_ServiceState = SERVICE_START;

    if((pService = ServiceCall(&m_pBasicRect->Service, &m_ServiceState)) != nullptr)
    {
        EventArea.Rectangle.Box = m_pBasicRect->Box;
      #if (GRAFX_USE_POINTING_DEVICE == DEF_ENABLED)
        PDI_pTask->CreateZone(&EventArea, m_pBasicRect->Options, pPageWidget->ID);      // Create the zone on the touch sense virtual screen
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
Link_e WidgetBasicRectangle::Refresh(MsgRefresh_t* pMsg)
{
     ServiceReturn_t* pService;

    m_ServiceState = SERVICE_REFRESH;

   // If it is our touch zone that is pressed then change service state to process the touch
    GUI_FilterServiceState(&m_ServiceState, pMsg, m_pPageWidget->ID, m_pBasicRect->ServiceFilter, SERVICE_IDLE);

    // Invoke the application service for this basic box (It might change by itself the m_ServiceState)
    if((pService = ServiceCall(&m_pBasicRect->Service, &m_ServiceState)) != nullptr)
    {
        if((pService->ServiceType == SERVICE_RETURN) && (m_ServiceState == SERVICE_RELEASED))
        {
            if(m_pPageWidget->Link != INVALID_LINK)
            {
                FreeServiceStruct(&pService);
                return m_pPageWidget->Link;                             // Stop here, useless to continue because we will switch screen
            }
        }

        if(pService->ServiceType == SERVICE_RETURN_TYPE3)
        {
            Link_e Link = ((ServiceType3_t*)pService)->Link;
            if(Link != INVALID_LINK)
            {
                FreeServiceStruct(&pService);
                return Link;                                            // Stop here, useless to continue because we will switch screen
            }
        }

        if(m_ServiceState != SERVICE_IDLE)
        {
            Draw(pService);
        }
        FreeServiceStruct(&pService);
    }

    return INVALID_LINK;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Finalize
//
//  Parameter(s):   none
//  Return:         void
//
//  Description:    This function call service associated with widget to finalize it properly.
//
//-------------------------------------------------------------------------------------------------
void WidgetBasicRectangle::Finalize()
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_FINALIZE;

    if((pService = ServiceCall(&m_pBasicRect->Service, &m_ServiceState)) != nullptr)
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
//  Description:    Draw the button on display according to state.
//
//-------------------------------------------------------------------------------------------------
void WidgetBasicRectangle::Draw(ServiceReturn_t* pService)
{
    DisplayLayer::PushDrawing();

  #if (GRAFX_DEBUG_GUI == DEF_ENABLED)
    DisplayLayer::SetDrawing(((m_pBasicRect->Options & GRAFX_OPTION_DRAW_ON_BACK) != 0) ? BACKGROUND_DISPLAY_LAYER_0 : FOREGROUND_DISPLAY_LAYER_0);
  #else

   #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)     // TODO confirm this
    if(SKIN_pTask->IsSkinLoaded() == true)
   #endif
    {
        DisplayLayer::SetDrawing(((m_pBasicRect->Options & GRAFX_OPTION_DRAW_ON_BACK) != 0) ? GRAFX_SelectBackgroundDrawingLayer() :
                                                                                              GRAFX_SelectForegroundDrawingLayer());
    }
   #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
    else
    {
        DisplayLayer::SetDrawing(FOREGROUND_DISPLAY_LAYER_0);   // On loading with do print directly on foreground layer
    }
  #endif
 #endif

    DisplayLayer::SetColor(m_pBasicRect->RectColor);
    myGrafx->DrawRectangle(&m_pBasicRect->Box);

    DisplayLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------

#endif // BASIC_RECT_DEF
#endif // DIGINI_USE_GRAFX
