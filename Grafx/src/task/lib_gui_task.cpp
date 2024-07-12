//-------------------------------------------------------------------------------------------------
//
//  File :  lib_gui_task.cpp
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

#define LIB_GUI_TASK_GLOBAL
#include "./lib_digini.h"
#undef  LIB_GUI_TASK_GLOBAL
#include <new>

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Static Variables
//
//-------------------------------------------------------------------------------------------------

nOS_Thread GUI_myClassTask::m_Handle;
nOS_Stack  GUI_myClassTask::m_Stack[GUI_TASK_STACK_SIZE];

//-------------------------------------------------------------------------------------------------
//
//  Name:           GUI_TaskWrapper
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    C++ Wrapper for the task
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" void GUI_TaskWrapper(void* pvParameters)
{
    (static_cast<GUI_myClassTask*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void
//  Return:         nOS_Error
//
//  Description:    Initialize the task
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
nOS_Error GUI_myClassTask::Initialize(void)
{
    nOS_Error Error;

    if((Error = nOS_QueueCreate(&this->m_Q_Msg, &this->m_ArrayMsg[0], sizeof(MsgRefresh_t), GUI_NUMBER_OF_MSG)) == NOS_OK)
    {
        Error = nOS_ThreadCreate(&m_Handle,
                                 GUI_TaskWrapper,
                                 this,
                                 &m_Stack[0],
                                 GUI_TASK_STACK_SIZE,
                                 GUI_TASK_PRIO
                               #if(NOS_CONFIG_THREAD_MPU_REGION_ENABLE > 0)
                                 , nullptr
                               #endif
                                );
    }

      #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
        myStacktistic.Register(&m_Stack[0], GUI_TASK_STACK_SIZE, "Digini GUI");
      #endif

  #if (GRAFX_USE_SLIDING_PAGE == DEF_ENABLED)
    m_SlideRange.StartPos = -1;
    m_SlideRange.EndPos   = -1;
  #endif

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           run
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//-------------------------------------------------------------------------------------------------
void GUI_myClassTask::Run()
{
    nOS_Error       Error;
    MsgRefresh_t    Msg;
    Link_e          NewLink;
    Link_e          PreviousLink;

  #if (GRAFX_USE_SLIDING_PAGE == DEF_ENABLED)
    bool IsPageWasSliding;
  #endif

    // Start at home page
    m_Link    = INVALID_LINK;
    NewLink   = LINK_MAIN_LOADING;

    // There is no force refresh. (Use for example to refresh page on language change)
    m_ForceRefresh = false;

  #if defined(STATIC_SKIN_DEF)
    // Static skin must be loaded
    while(SKIN_pTask->IsStaticSkinLoaded() != true)
    {
        nOS_Sleep(10);
    };

    nOS_Sleep(10); // Allow other task to run   TODO: (Alain#1#) maybe sleep is not the best option
  #endif

    for(;;)
    {
        if(m_ForceRefresh == false)
        {
            if((Error = nOS_QueueRead(&this->m_Q_Msg, &Msg, GRAFX_TICK_WAIT_BETWEEN_REFRESH_LOOP)) == NOS_E_TIMEOUT)
            {
                Msg.Type     = MESSAGE_TYPE_PDI_EVENT_INFO;
                Msg.Touch    = SERVICE_IDLE;
                Msg.WidgetID = INVALID_WIDGET;
                Error        = NOS_OK;
            }
        }

        if(Error == NOS_OK)
        {
            if((NewLink != INVALID_LINK) || (m_ForceRefresh == true))               // If we have a new link (switch to new page)
            {                                                                       // Finalize old page then create new page
                if((m_Link != INVALID_LINK)  || (m_ForceRefresh == true))           // finalize the actual page if it exist
                {
                    FinalizeAllWidget();
                }

              #if (GRAFX_USE_POINTING_DEVICE == DEF_ENABLED)
                PDI_pTask->ClearAllZone();
              #endif
                GUI_ClearWidgetLayer();

                // TODO (Alain#2#) maybe do a stack of previous link... now only one level is active
                if(m_ForceRefresh == false)
                {
                    PreviousLink = m_Link;
                    m_Link       = NewLink;
                }
                NewLink = CreateAllWidget();

              #if (GRAFX_USE_SLIDING_PAGE == DEF_ENABLED)
               #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
                if(SKIN_pTask->IsSkinLoaded() == true)
               #endif
                {
                    if(NewLink == INVALID_LINK)
                    {
                        IsPageWasSliding = this->SlidingPage();
                    }
                }
              #endif

                m_ForceRefresh = false;
            }
            else
            {
                NewLink = RefreshAllWidget(&Msg);
            }

            if(NewLink == PREVIOUS_LINK)
            {
                NewLink = PreviousLink;
            }

         #if (GRAFX_DEBUG_GUI == DEF_DISABLED)
          #if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
           #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
            if(SKIN_pTask->IsSkinLoaded() == true)
           #endif
            {
                if(NewLink == INVALID_LINK)         // Only make the copy if it's not an immediate redirection
                {
                  #if (GRAFX_DRIVER_USE_V_SYNC == DEF_ENABLED)
                    myGrafx->WaitFor_V_Sync();
                  #endif
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_DISPLAY_LAYER_0, 0, 0, GRAFX_DRIVER_SIZE_X, GRAFX_DRIVER_SIZE_Y);

                  #if (GRAFX_USE_SLIDING_PAGE == DEF_ENABLED)
                    if(IsPageWasSliding == true)
                    {
                        CLayer::SetActiveLayer(LAYER_FOREGROUND, FOREGROUND_DISPLAY_LAYER_0);
                    }
                  #endif
                }
            }
          #endif
         #endif
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CreateAllWidget
//
//  Parameter(s):   None
//  Return:         Link_e NewLink
//
//  Description:    Allocated all widget in the memory block
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
Link_e GUI_myClassTask::CreateAllWidget()
{
    CWidgetInterface*   pWidget;
    CWidgetInterface**  pWidgetListPointer;
    PageWidget_t*       pPage;
    Widget_e            Widget;
    Link_e              NewLink = INVALID_LINK;
    int                 Count;                                                          // Widget count in page
    int                 CountCopy;

    m_pPage = (PageWidget_t*)PageWidget[m_Link];                                        // Get the page pointer
    pPage   = m_pPage;
    m_WidgetCount = 0;
    while((pPage + m_WidgetCount)->ID != INVALID_WIDGET)                                // Get the widget count in the page
    {
        m_WidgetCount++;
    }

    // Reserve memory for CWidgetInterface pointer for the number of widget in the page
    m_pWidgetList = (CWidgetInterface**)pMemoryPool->Alloc(m_WidgetCount * sizeof(CWidgetInterface*));

    Count               = 0;
    pWidgetListPointer  = m_pWidgetList;

    while(Count < m_WidgetCount)
    {
        Widget = pPage->ID;

        if(Widget == INVALID_WIDGET)
        {
            // Do Nothing
        }

       #ifdef BACK_DEF
        else if((Widget > APP_START_BACK_CONST) && (Widget < APP_END_BACK_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CBackground));                       // Get the memory block for the widget class object
            *pWidgetListPointer = new(pWidget) CBackground(&Background[Widget - (APP_START_BACK_CONST + 1)]);       // Get a class object of the widget
        }
       #endif

       #ifdef BASIC_BOX_DEF
        else if((Widget > APP_START_BASIC_BOX_CONST) && (Widget < APP_END_BASIC_BOX_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CBasicBox));
            *pWidgetListPointer = new(pWidget) CBasicBox(&BasicBox[Widget - (APP_START_BASIC_BOX_CONST + 1)]);
        }
       #endif

       #ifdef BASIC_BTN_DEF
        else if((Widget > APP_START_BASIC_BTN_CONST) && (Widget < APP_END_BASIC_BTN_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CBasicButton));
            *pWidgetListPointer = new(pWidget) CBasicButton(&BasicButton[Widget - (APP_START_BASIC_BTN_CONST + 1)]);
        }
       #endif

       #ifdef BASIC_RECT_DEF
        else if((Widget > APP_START_BASIC_RECT_CONST) && (Widget < APP_END_BASIC_RECT_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CBasicRect));
            *pWidgetListPointer = new(pWidget) CBasicRect(&BasicRect[Widget - (APP_START_BASIC_RECT_CONST + 1)]);
        }
       #endif

       #ifdef BTN_DEF
        else if((Widget > APP_START_BTN_CONST) && (Widget < APP_END_BTN_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CButton));
            *pWidgetListPointer = new(pWidget) CButton(&Button[Widget - (APP_START_BTN_CONST + 1)]);
        }
       #endif

       #ifdef GIF_DEF
        else if((Widget > APP_START_GIF_CONST) && (Widget < APP_END_GIF_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CGif));
            *pWidgetListPointer = new(pWidget) CGif(&Gif[Widget - (APP_START_GIF_CONST + 1)]);
        }
       #endif

       #ifdef GRAPH_DEF
        else if((Widget > APP_START_GRAPH_CONST) && (Widget < APP_END_GRAPH_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CGraph));
            *pWidgetListPointer = new(pWidget) CGraph(&Graph[Widget - (APP_START_GRAPH_CONST + 1)]);
        }
       #endif

       #ifdef ICON_DEF
        else if((Widget > APP_START_ICON_CONST) && (Widget < APP_END_ICON_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CIcon));
            *pWidgetListPointer = new(pWidget) CIcon(&Icon[Widget - (APP_START_ICON_CONST + 1)]);
        }
       #endif

       #ifdef LABEL_DEF
        else if((Widget > APP_START_LABEL_CONST) && (Widget < APP_END_LABEL_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CLabel));
            *pWidgetListPointer = new(pWidget) CLabel(&Label[Widget - (APP_START_LABEL_CONST + 1)]);
        }
       #endif

       #ifdef LABEL_LIST_DEF
        else if((Widget > APP_START_LABEL_LIST_CONST)  && (Widget < APP_END_LABEL_LIST_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CLabelList));
            *pWidgetListPointer = new(pWidget) CLabelList(&LabelList[Widget - (APP_START_LABEL_LIST_CONST + 1)]);
        }
       #endif

       #ifdef METER_DEF
        else if((Widget > APP_START_METER_CONST) && (Widget < APP_END_METER_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CMeter));
            *pWidgetListPointer = new(pWidget) CMeter(&Meter[Widget - (APP_START_METER_CONST + 1)]);
        }
       #endif

       #ifdef PAGE_SLIDE_DEF
        else if((Widget > APP_START_PAGE_SLIDE_CONST) && (Widget < APP_END_PAGE_SLIDE_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CPageSlide));
            *pWidgetListPointer = new(pWidget) CPageSlide(&PageSlide[Widget - (APP_START_PAGE_SLIDE_CONST + 1)]);
       }
       #endif

       #ifdef PANEL_DEF
        else if((Widget > APP_START_PANEL_CONST) && (Widget < APP_END_PANEL_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CPanel));
            *pWidgetListPointer = new(pWidget) CPanel(&Panel[Widget - (APP_START_PANEL_CONST + 1)]);
        }
       #endif

       #ifdef PROGRESS_DEF
        else if((Widget > APP_START_PROGRESS_CONST) && (Widget < APP_END_PROGRESS_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CProgress));
            *pWidgetListPointer = new(pWidget) CProgress(&Progress[Widget - (APP_START_PROGRESS_CONST + 1)]);
        }
       #endif

       #ifdef ROUND_METER_DEF
        else if((Widget > APP_START_ROUND_METER_CONST) && (Widget < APP_END_ROUND_METER_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CRoundMeter));
            *pWidgetListPointer = new(pWidget) CRoundMeter(&RoundMeter[Widget - (APP_START_ROUND_METER_CONST + 1)]);
        }
       #endif

       #ifdef SPECTRUM_DEF
        else if((Widget > APP_START_SPECTRUM_CONST) && (Widget < APP_END_SPECTRUM_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CSpectrum));
            *pWidgetListPointer = new(pWidget) CSpectrum(&Spectrum[Widget - (APP_START_SPECTRUM_CONST + 1)]);
        }
       #endif

       #ifdef SPRITE_DEF
        else if((Widget > APP_START_SPRITE_CONST) && (Widget < APP_END_SPRITE_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CSprite));
            *pWidgetListPointer = new(pWidget) CSprite(&Sprite[Widget - (APP_START_SPRITE_CONST + 1)]);
        }
       #endif

        #ifdef TERMINAL_DEF
        else if((Widget > APP_START_TERMINAL_CONST) && (Widget < APP_END_TERMINAL_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CTerminal));
            *pWidgetListPointer = new(pWidget) CTerminal(&Terminal[Widget - (APP_START_TERMINAL_CONST + 1)]);
        }
       #endif

       #ifdef VIRTUAL_HUB_DEF
        else if((Widget > APP_START_VIRTUAL_HUB_CONST) && (Widget < APP_END_VIRTUAL_HUB_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CVirtualHub));
            *pWidgetListPointer = new(pWidget) CVirtualHub(&VirtualHub[Widget - (APP_START_VIRTUAL_HUB_CONST + 1)]);
        }
       #endif

       #ifdef VIRTUAL_WINDOW_DEF
        else if((Widget > APP_START_VIRTUAL_WINDOW_CONST) && (Widget < APP_END_VIRTUAL_WINDOW_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CVirtualWindow));
            *pWidgetListPointer = new(pWidget) CVirtualWindow(&VirtualWindow[Widget - (APP_START_VIRTUAL_WINDOW_CONST + 1)]);
        }
       #endif
/*  // TODO (Alain#1#)  remove if the new method of exchange data work!!
       #ifdef XCHANGE_DEF
        else if((Widget > APP_START_XCHANGE_CONST) && (Widget < APP_END_XCHANGE_CONST))
        {
            pWidget             = (CWidgetInterface*)pMemoryPool->Alloc(sizeof(CExchange));
            *pWidgetListPointer = new(pWidget) CExchange(&Exchange[Widget - (APP_START_XCHANGE_CONST + 1)]);
        }
       #endif
*/

        Count++;
        pPage++;
        pWidgetListPointer++;
    }

    // Reset all pointer and count to beginning of the list
    pPage               = m_pPage;
    Count               = 0;
    pWidgetListPointer  = m_pWidgetList;

    while((Count < m_WidgetCount) && (NewLink == INVALID_LINK))
    {
        NewLink = (*pWidgetListPointer)->Create(pPage);

        // If a widget is a draw only once
        if(NewLink == REMOVE_WIDGET)
        {
            // Free this widget
            pMemoryPool->Free ((void**)&(*pWidgetListPointer));

            // Move all widget in the list up one position
            CountCopy = Count;
            while(CountCopy < m_WidgetCount)
            {
               //*(pWidgetListPointer + CountCopy) = *(pWidgetListPointer + CountCopy + 1);
                *(m_pWidgetList + CountCopy) = *(m_pWidgetList + CountCopy + 1);            // FIX  ?????  for crashing remove widget
                CountCopy++;
            }

            NewLink = INVALID_LINK;
            m_WidgetCount--;
        }
        else
        {
            pWidgetListPointer++;
            Count++;
        }
        pPage++;
    }

    return NewLink;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RefreshAllWidget
//
//  Parameter(s):   MsgRefresh_t*    pMsg
//  Return:         Link_e           Link
//
//  Description:    Refresh all widget
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
Link_e GUI_myClassTask::RefreshAllWidget(MsgRefresh_t* pMsg)
{
    CWidgetInterface**  pWidgetListPointer;
    Link_e              NewLink;
    uint8_t             Count;

    NewLink = INVALID_LINK;
    pWidgetListPointer = m_pWidgetList;
    Count = 0;

    while((Count < m_WidgetCount) && (NewLink == INVALID_LINK))
    {
        NewLink = (*pWidgetListPointer)->Refresh(pMsg);
        Count++;
        pWidgetListPointer++;
        nOS_Yield();
    }

    return NewLink;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FinalizeAllWidget
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Finalize all widget
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GUI_myClassTask::FinalizeAllWidget()
{
    CWidgetInterface**  pWidgetListPointer;
    uint8_t             Count;

    pWidgetListPointer = m_pWidgetList;
    Count = 0;

    while(Count < m_WidgetCount)
    {
        (*pWidgetListPointer)->Finalize();
        pMemoryPool->Free((void**)&(*pWidgetListPointer));
        pWidgetListPointer++;
        Count++;
        nOS_Yield();
    }

    pMemoryPool->Free((void**)&m_pWidgetList);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SlidingPage
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    With process sliding animation and return to GUI loop after it's completed
//
//  Note(s):        TODO (Alain#2#)   add a structure to overrides full screen slide, so a part of
//                                    screen can be fixed
//
//-------------------------------------------------------------------------------------------------
#if (GRAFX_USE_SLIDING_PAGE == DEF_ENABLED)
bool GUI_myClassTask::SlidingPage(void)
{
    int16_t         SlidePosActualPage;
    int16_t         SlidePosNewPage;
    Box_t           Box;
    bool            IsPageWasSliding;
    bool            IsItOverlapped;
    bool            IsItDeOverlapped;
    TickCount_t     Wait;
    uint16_t        Start;
    uint16_t        Size;


    Box.Pos.X = 0;
    Box.Pos.Y = 0;
    Box.Size.Width  = GRAFX_DRIVER_SIZE_X;
    Box.Size.Height = GRAFX_DRIVER_SIZE_Y;
    IsPageWasSliding = true;

    IsItOverlapped   = ((m_SlidingDir & SLIDING_OVERLAP_MASK)    != 0) ? true : false;
    IsItDeOverlapped = ((m_SlidingDir & SLIDING_DE_OVERLAP_MASK) != 0) ? true : false;

    Start = (m_SlideRange.StartPos == -1) ? 0 : m_SlideRange.StartPos;

    switch(m_SlidingDir & SLIDING_MASK)
    {
        //-------------------------------------------------------------------------------------------------------------------------
        case SLIDING_LEFT:
        {
            SlidePosNewPage    = GRAFX_DRIVER_SIZE_X;
            SlidePosActualPage = 0;
            myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, &Box);      // copy actual foreground to sliding layer
            CLayer::SetActiveLayer(LAYER_FOREGROUND, FOREGROUND_SLIDING_LAYER);
            Size = (m_SlideRange.EndPos == -1) ? GRAFX_DRIVER_SIZE_Y : m_SlideRange.EndPos - Start;

            do
            {
                Wait = GetTick();
                SlidePosActualPage += GRAFX_SLIDING_PAGE_GRANULARITY;
                SlidePosNewPage    -= GRAFX_SLIDING_PAGE_GRANULARITY;

                if(IsItOverlapped == false)
                {
                    myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, SlidePosActualPage, Start, 0, Start, GRAFX_DRIVER_SIZE_X - SlidePosActualPage, Size);
                }

                if(IsItDeOverlapped == true)
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, SlidePosNewPage, Start, SlidePosNewPage, Start, GRAFX_DRIVER_SIZE_X - SlidePosNewPage, Size);
                }
                else
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, 0, Start, SlidePosNewPage, Start, GRAFX_DRIVER_SIZE_X - SlidePosNewPage, Size);
                }

                do
                {
                    nOS_Sleep(1);
                }
                while(TickHasTimeOut(Wait, GRAFX_TICK_WAIT_BETWEEN_SLIDE_IN_LOOP) != true);
//                nOS_Sleep(GRAFX_TICK_WAIT_BETWEEN_SLIDE_IN_LOOP + AddingToWait);
            }
            while(SlidePosNewPage > 0);

            m_SlidingDir = SLIDING_NONE;
        }
        break;

        //-------------------------------------------------------------------------------------------------------------------------
        case SLIDING_RIGHT:
        {
            SlidePosNewPage    = GRAFX_DRIVER_SIZE_X;
            SlidePosActualPage = 0;
            myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, &Box);      // copy actual foreground to sliding layer
            CLayer::SetActiveLayer(LAYER_FOREGROUND, FOREGROUND_SLIDING_LAYER);
            Size = (m_SlideRange.EndPos == -1) ? GRAFX_DRIVER_SIZE_Y : m_SlideRange.EndPos - Start;

            do
            {
                Wait = GetTick();
                SlidePosActualPage += GRAFX_SLIDING_PAGE_GRANULARITY;
                SlidePosNewPage    -= GRAFX_SLIDING_PAGE_GRANULARITY;

                if(IsItOverlapped == false)
                {
                    myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, Start, 0, SlidePosActualPage, Start, GRAFX_DRIVER_SIZE_X - SlidePosActualPage, Size);
                }

                if(IsItDeOverlapped == true)
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, 0, Start, 0, Start, GRAFX_DRIVER_SIZE_X - SlidePosNewPage, Size);
                }
                else
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, SlidePosNewPage, Start, 0, Start, GRAFX_DRIVER_SIZE_X - SlidePosNewPage, Size);
                }

                do
                {
                    nOS_Sleep(1);
                }
                while(TickHasTimeOut(Wait, GRAFX_TICK_WAIT_BETWEEN_SLIDE_IN_LOOP) != true);
            }
            while(SlidePosActualPage < GRAFX_DRIVER_SIZE_X);

            m_SlidingDir = SLIDING_NONE;
        }
        break;

        //-------------------------------------------------------------------------------------------------------------------------
        case SLIDING_UP:
        {
            SlidePosNewPage    = GRAFX_DRIVER_SIZE_Y;
            SlidePosActualPage = 0;
            myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, &Box);      // copy actual foreground to sliding layer
            CLayer::SetActiveLayer(LAYER_FOREGROUND, FOREGROUND_SLIDING_LAYER);
            Size = (m_SlideRange.EndPos == -1) ? GRAFX_DRIVER_SIZE_X : m_SlideRange.EndPos - Start;

            do
            {
                Wait = GetTick();
                SlidePosActualPage += GRAFX_SLIDING_PAGE_GRANULARITY;
                SlidePosNewPage    -= GRAFX_SLIDING_PAGE_GRANULARITY;

                if(IsItOverlapped == false)
                {
                    myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, Start, SlidePosActualPage, Start, 0, Size, GRAFX_DRIVER_SIZE_Y - SlidePosActualPage);
                }

                if(IsItDeOverlapped == true)
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, Start, SlidePosNewPage, Start, SlidePosNewPage, Size, GRAFX_DRIVER_SIZE_Y - SlidePosNewPage);
                }
                else
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, Start, 0, Start, SlidePosNewPage, Size, GRAFX_DRIVER_SIZE_Y - SlidePosNewPage);
                }

                do
                {
                    nOS_Sleep(1);
                }
                while(TickHasTimeOut(Wait, GRAFX_TICK_WAIT_BETWEEN_SLIDE_IN_LOOP) != true);
            }
            while(SlidePosNewPage > 0);

            m_SlidingDir = SLIDING_NONE;
        }
        break;

        //-------------------------------------------------------------------------------------------------------------------------
        case SLIDING_DOWN:
        {
            SlidePosNewPage    = GRAFX_DRIVER_SIZE_Y;
            SlidePosActualPage = 0;
            myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, &Box);      // copy actual foreground to sliding layer
            CLayer::SetActiveLayer(LAYER_FOREGROUND, FOREGROUND_SLIDING_LAYER);
            Size = (m_SlideRange.EndPos == -1) ? GRAFX_DRIVER_SIZE_X : m_SlideRange.EndPos - Start;

            do
            {
                Wait = GetTick();
                SlidePosActualPage += GRAFX_SLIDING_PAGE_GRANULARITY;
                SlidePosNewPage    -= GRAFX_SLIDING_PAGE_GRANULARITY;

                if(IsItOverlapped == false)
                {
                    myGrafx->CopyLayerToLayer(FOREGROUND_DISPLAY_LAYER_0, FOREGROUND_SLIDING_LAYER, Start, 0, Start, SlidePosActualPage, Size, GRAFX_DRIVER_SIZE_Y - SlidePosActualPage);
                }

                if(IsItDeOverlapped == true)
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, Start, 0, Start, 0, Size, GRAFX_DRIVER_SIZE_Y - SlidePosNewPage);
                }
                else
                {
                    myGrafx->CopyLayerToLayer(CONSTRUCTION_FOREGROUND_LAYER, FOREGROUND_SLIDING_LAYER, Start, SlidePosNewPage, Start, 0, Size, GRAFX_DRIVER_SIZE_Y - SlidePosNewPage);
                }

                do
                {
                    nOS_Sleep(1);
                }
                while(TickHasTimeOut(Wait, GRAFX_TICK_WAIT_BETWEEN_SLIDE_IN_LOOP) != true);
            }
            while(SlidePosActualPage < GRAFX_DRIVER_SIZE_Y);

            m_SlidingDir = SLIDING_NONE;
        }
        break;

        //-------------------------------------------------------------------------------------------------------------------------
        default:
        {
            IsPageWasSliding = false;
        }
        break;
    }

    // Reset override
    m_SlideRange.StartPos = -1;
    m_SlideRange.EndPos   = -1;

    return IsPageWasSliding;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           GUI_FilterServiceState
//
//  Parameter(s):   ServiceEvent_e* pServiceState
//                  MsgRefresh_t*   pMsg
//                  Widget_e        Widget
//                  uint16_t        Filter              Filter option
//                  ServiceEvent_e  DefaultState
//  Return:         None
//
//  Description:    Will return only value in filter option otherwise it return default
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (GRAFX_USE_SLIDING_PAGE == DEF_ENABLED)
void GUI_myClassTask::SetSlidingRange(PageSlideRange_t* pPageSlideRange)
{
    memcpy(&this->m_SlideRange, pPageSlideRange, sizeof(PageSlideRange_t));
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           GUI_FilterServiceState
//
//  Parameter(s):   ServiceEvent_e* pServiceState
//                  MsgRefresh_t*   pMsg
//                  Widget_e        Widget
//                  uint16_t        Filter              Filter option
//                  ServiceEvent_e  DefaultState
//  Return:         None
//
//  Description:    Will return only value in filter option otherwise it return default
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void GUI_FilterServiceState(ServiceEvent_e* pServiceState, MsgRefresh_t* pMsg, Widget_e Widget, uint16_t Filter, ServiceEvent_e DefaultState)
{
    uint16_t State;
    *pServiceState = DefaultState;

    if(pMsg->WidgetID == Widget)
    {
        State = Filter & pMsg->Touch;
        if(State != 0)
        {
            *pServiceState = ServiceEvent_e(State);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GUI_ClearWidgetLayer
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Will clear the widget layer for a new layer to be draw
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
// TODO (Alain#1#): it might already be handle by the class
void GUI_ClearWidgetLayer()
{
    Box_t Box = {{0, 0},{GRAFX_DRIVER_SIZE_X, GRAFX_DRIVER_SIZE_Y}};
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

    CLayer::SetColor(TRANSPARENT);
    myGrafx->DrawRectangle(&Box);
    CLayer::PopDrawing();
}

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
