//-------------------------------------------------------------------------------------------------
//
//  File : lib_widget_background.cpp
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

#include "lib_digini.h"
#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#ifdef BACK_DEF

//-------------------------------------------------------------------------------------------------
//
//  Constructor:    CBackground
//
//  Parameter(s):   Background_t         pBackground         Pointer to Background_t structure
//
//  Description:    Initialize widget's service and build widget.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
CBackground::CBackground(Background_t* pBackground)
{
    m_pBackground = pBackground;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Create
//
//  Parameter(s):   PageWidget_t* pPageWidget
//  Return:         Link_e        Link
//
//  Description:    This function call service and create the widget
//
//  Note(s)         Return a link or INVALID_LINK
//
//-------------------------------------------------------------------------------------------------
Link_e CBackground::Create(PageWidget_t* pPageWidget)
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_START;
    m_pPageWidget  = pPageWidget;
    if((pService = ServiceCall(&m_pBackground->Service, &m_ServiceState)) != nullptr)
    {
        this->Draw(pService);
        FreeServiceStruct(&pService);
    }

    if((m_pBackground->Options & GRAFX_OPTION_DRAW_ONCE_AND_REMOVE) != 0)
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
Link_e CBackground::Refresh(MsgRefresh_t* pMsg)
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_REFRESH;

    // Invoke the application service for this background (It might change by itself the m_ServiceState)
    if((pService = ServiceCall(&m_pBackground->Service, &m_ServiceState)) != nullptr)
    {
        this->Draw(pService);
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
void CBackground::Finalize()
{
    ServiceReturn_t* pService;

    m_ServiceState = SERVICE_FINALIZE;

    if((pService = ServiceCall(&m_pBackground->Service, &m_ServiceState)) != nullptr)
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
//  Description:    Draw the Background on display according to state.
//
//-------------------------------------------------------------------------------------------------
void CBackground::Draw(ServiceReturn_t* pService)
{
    ImageInfo_t ImageInfo;
    CLayer*     pLayer;


    CLayer::PushDrawing();

  #if (GRAFX_DEBUG_GUI == DEF_ENABLED)
        CLayer::SetDrawing(BACKGROUND_DISPLAY_LAYER_0);
        pLayer = &LayerTable[BACKGROUND_DISPLAY_LAYER_0];
  #else
    if(SKIN_pTask->IsSkinLoaded() == true)
    {
        CLayer::SetDrawing(CONSTRUCTION_BACKGROUND_LAYER);
        pLayer = &LayerTable[CONSTRUCTION_BACKGROUND_LAYER];
    }
    else
    {
        CLayer::SetDrawing(BACKGROUND_DISPLAY_LAYER_0);
        pLayer = &LayerTable[BACKGROUND_DISPLAY_LAYER_0];
    }
  #endif

    DB_Central.Get(&ImageInfo, GFX_IMAGE_INFO, uint16_t(m_pBackground->Image.ID_List[pService->IndexState]), 0);
    myGrafx->BlockCopy(ImageInfo.pPointer,
                       m_pBackground->Pos.X,
                       m_pBackground->Pos.Y,
                       ImageInfo.Size.Width,
                       ImageInfo.Size.Height,
                       m_pBackground->Pos.X,
                       m_pBackground->Pos.Y,
                       pLayer->GetPixelFormat(),
                       CLEAR_BLEND);


  #if (GRAFX_DEBUG_GUI == DEF_DISABLED)
    if(SKIN_pTask->IsSkinLoaded() == true)
    {
      #if (GRAFX_DRIVER_USE_V_SYNC == DEF_DEFINED)
        myGrafx->WaitFor_V_Sync();
      #endif
        myGrafx->CopyLayerToLayer(CONSTRUCTION_BACKGROUND_LAYER, BACKGROUND_DISPLAY_LAYER_0, 0, 0, GRAFX_DRIVER_SIZE_X, GRAFX_DRIVER_SIZE_Y);
    }
  #endif

    CLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------

#endif // BACK_DEF
#endif // DIGINI_USE_GRAFX
