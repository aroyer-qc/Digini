//-------------------------------------------------------------------------------------------------
//
//  File : lib_widget_icon.cpp
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
#ifdef ICON_DEF

//-------------------------------------------------------------------------------------------------
//
//  Constructor:    WidgetIcon
//
//  Parameter(s):   Icon_t         pIcon         Pointer to Icon_t structure
//
//  Description:    Initialize widget's service and build widget.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
WidgetIcon::WidgetIcon(Icon_t* pIcon)
{
    m_pIcon = pIcon;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Create
//
//  Parameter(s):   PageWidget_t* pPageWidget
//  Return:         Link_e        INVALID_LINK
//
//  Description:    This function call service and create the widget
//
//  Note(s)         No link, Always return INVALID_LINK
//
//-------------------------------------------------------------------------------------------------
Link_e WidgetIcon::Create(PageWidget_t* pPageWidget)
{
    ServiceReturn_t* pService;

    m_pPageWidget  = pPageWidget;
    m_ServiceState = SERVICE_START;
    if((pService = ServiceCall(&m_pIcon->Service, &m_ServiceState)) != nullptr)
    {
        this->Draw(pService);
        FreeServiceStruct(&pService);
    }

    if((m_pIcon->Options & GRAFX_OPTION_DRAW_ONCE_AND_REMOVE) != 0)
    {
        return REMOVE_WIDGET;
    }

    return INVALID_LINK;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Refresh
//
//  Parameter(s):   MsgRefresh_t*    pMsg
//  Return:         Link_e           INVALID_LINK
//
//  Description:    This function call service to refresh widget
//
//  Note(s)         No link, Always return INVALID_LINK
//
//-------------------------------------------------------------------------------------------------
Link_e WidgetIcon::Refresh(MsgRefresh_t* pMsg)
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_REFRESH;

    if((pService = ServiceCall(&m_pIcon->Service, &m_ServiceState)) != nullptr)        // Invoke the application service for this icon (It might change by itself the m_ServiceState)
    {
        if((m_pIcon->Options & GRAFX_OPTION_DRAW_ONCE) != GRAFX_OPTION_DRAW_ONCE)
        {
            this->Draw(pService);
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
void WidgetIcon::Finalize()
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_FINALIZE;
    if((pService = ServiceCall(&m_pIcon->Service, &m_ServiceState)) != nullptr)
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
//  Description:    Draw the Icon on display according to state.
//
//-------------------------------------------------------------------------------------------------
void WidgetIcon::Draw(ServiceReturn_t* pService)
{
    DisplayLayer::PushDrawing();

/*
  #if (GRAFX_DEBUG_GUI == DEF_ENABLED)
    DisplayLayer::SetDrawing(FOREGROUND_DISPLAY_LAYER_0);
  #else
   #if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
    DisplayLayer::SetDrawing(CONSTRUCTION_FOREGROUND_LAYER);
   #else
    DisplayLayer::SetDrawing(FOREGROUND_DISPLAY_LAYER_0);
   #endif
  #endif
*/

  #if (GRAFX_DEBUG_GUI == DEF_ENABLED)
    DisplayLayer::SetDrawing(((m_pIcon->Options & GRAFX_OPTION_DRAW_ON_BACK) != 0) ? BACKGROUND_DISPLAY_LAYER_0 : FOREGROUND_DISPLAY_LAYER_0);
  #else
    DisplayLayer::SetDrawing(((m_pIcon->Options & GRAFX_OPTION_DRAW_ON_BACK) != 0) ? GRAFX_SelectBackgroundDrawingLayer() :
                                                                                     GRAFX_SelectForegroundDrawingLayer());
   #endif

    ImageID_e ImageID = m_pIcon->Image.ID_List[pService->IndexState];

  #if (GRAFX_USE_CONSTRUCTION_ON_SINGLE_LAYER == DEF_ENABLED)
    myGrafx->CopyBackgroundToConstruction(ImageID, m_pIcon->Box.Pos);               // if the display has no multilayer capability.
  #else
    if((m_pIcon->Options & GRAFX_OPTION_CLEAR) != 0)
    {
        DisplayLayer::SetColor(BLACK);
        myGrafx->DrawRectangle(&m_pIcon->Box);
    }
  #endif

    if(pService->ServiceType == SERVICE_RETURN_TYPE5)
    {
        myGrafx->CopyLinear(((ServiceType5_t*)pService)->pAddress,
                            m_pIcon->Box.Pos.X,
                            m_pIcon->Box.Pos.Y,
                            ((ServiceType5_t*)pService)->Size.Width,
                            ((ServiceType5_t*)pService)->Size.Height,
                            ((ServiceType5_t*)pService)->PixelFormat,
                            CLEAR_BLEND);
    }
    else
    {
        myGrafx->CopyLinear(ImageID, m_pIcon->Box.Pos, ((m_pIcon->Options & GRAFX_OPTION_BLEND_CLEAR) != 0) ? CLEAR_BLEND : ALPHA_BLEND);
    }

  #if (GRAFX_USE_FULL_FRAME_CONSTRUCTION_LAYER == DEF_DISABLED)
    {
        myGrafx->CopyWidgetToDevice(ImageID, m_pIcon->Box.Pos);
    }
  #endif

    DisplayLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------

#endif // ICON_DEF
#endif // DIGINI_USE_GRAFX
