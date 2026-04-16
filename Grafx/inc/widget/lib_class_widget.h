//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_widget.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"
#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class CWidgetInterface
{
    public:

        virtual                ~CWidgetInterface            (){};

        virtual Link_e          Create                      (PageWidget_t* pPageWidget)     = 0;
        virtual Link_e          Refresh                     (MsgRefresh_t* pMsg)            = 0;
        virtual void            Finalize                    (void)                          = 0;
};

#ifdef BACK_DEF
class WidgetBackground : public CWidgetInterface
{
    public:
                                WidgetBackground            (Background_t* pBackground);
                               ~WidgetBackground            (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Background_t*           m_pBackground;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef BASIC_BOX_DEF
class WidgetBasicBox : public CWidgetInterface
{
    public:
                                WidgetBasicBox              (BasicBox_t* pBasicBox);
                               ~WidgetBasicBox              (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        BasicBox_t*             m_pBasicBox;
        Cartesian_t             m_TopLeft;
        Cartesian_t             m_TopRight;
        Cartesian_t             m_BottomLeft;
        Cartesian_t             m_BottomRight;

        // Horizontal bar info for draw
        int16_t                 m_X_Hor;
        int16_t                 m_Y_HorTop;
        int16_t                 m_Y_HorBot;
        int16_t                 m_HorUpTo;

        // Vertical bar info for draw
        int16_t                 m_Y_Ver;
        int16_t                 m_X_VerLeft;
        int16_t                 m_X_VerRight;
        int16_t                 m_VerUpTo;

        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef BASIC_BTN_DEF
class WidgetBasicButton : public CWidgetInterface
{
    public:
                                WidgetBasicButton           (BasiWidgetButton_t* pBasiWidgetButton);
                               ~WidgetBasicButton           (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        BasicWidgetButton_t*    m_pBasicWidgetButton;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef BASIC_RECT_DEF
class WidgetBasicRectangle : public CWidgetInterface
{
    public:
                                WidgetBasicRectangle        (BasicRect_t* pBasicRect);
                               ~WidgetBasicRectangle        (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        BasicRect_t*            m_pBasicRect;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef BTN_DEF
class WidgetButton : public CWidgetInterface
{
    public:
                                WidgetButton                (Button_t* pButton);
                               ~WidgetButton                (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Button_t*               m_pButton;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef GIF_DEF
class WidgetGif : public CWidgetInterface
{
    public:
                                WidgetGif                   (Gif_t* pGif);
                               ~WidgetGif                   (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Gif_t*                  m_pGif;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef GRAPH_DEF
class WidgetGraph : public CWidgetInterface
{
    public:
                                WidgetGraph                 (Graph_t* pGraph);
                               ~WidgetGraph                 (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService, bool IsItDrawingGrid);

        Graph_t*                m_pGraph;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;

        uint16_t                m_DrawX;
        uint16_t                m_MaxDrawX;
};
#endif

#ifdef ICON_DEF
class WidgetIcon : public CWidgetInterface
{
    public:
                                WidgetIcon                  (Icon_t* pIcon);
                               ~WidgetIcon                  (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Icon_t*                 m_pIcon;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef LABEL_DEF
class WidgetLabel : public CWidgetInterface
{
    public:
                                WidgetLabel                 (Label_t* pLabel);
                                ~WidgetLabel                 (){};


        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Label_t*                m_pLabel;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef LABEL_LIST_DEF
class WidgetLabelList : public CWidgetInterface
{
    public:
                                WidgetLabelList             (LabelList_t* pLabelList);
                               ~WidgetLabelList             (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        LabelList_t*            m_pLabelList;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef METER_DEF
class WidgetMeter : public CWidgetInterface
{
    public:
                                WidgetMeter                 (Meter_t* pMeter);
                               ~WidgetMeter                 (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Meter_t*                m_pMeter;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;

        uint16_t                m_Value;
};
#endif

#ifdef PAGE_SLIDE_DEF
class WidgetPageSlide : public CWidgetInterface
{
    public:
                                WidgetPageSlide             (PageSlide_t* pPageSlide);
                               ~WidgetPageSlide             (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        PageSlide_t*            m_pPageSlide;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
        ServiceEvent_e          m_LastServiceState;
};
#endif

#ifdef PANEL_DEF
class WidgetPanel : public CWidgetInterface
{
    public:
                                WidgetPanel                 (Panel_t* pPanel);
                               ~WidgetPanel                 (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Panel_t*                m_pPanel;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
        ServiceEvent_e          m_LastServiceState;
};
#endif

#ifdef PROGRESS_DEF
class WidgetProgress : public CWidgetInterface
{
    public:
                                WidgetProgress              (Progress_t* pProgress);
                               ~WidgetProgress              (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);
        void                    DrawOnce                    (ServiceReturn_t* pService);

        Progress_t*             m_pProgress;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;

        uint16_t                m_Range;
        uint16_t                m_Value;
        bool                    m_NegativeMovement;
        Cartesian_t             m_CursorPos;
};
#endif

#ifdef ROTARY_DIAL_DEF  // TODO
class WidgetRotaryDial : public CWidgetInterface
{
    public:
                                WidgetRotaryDial            (RotaryDial_t* pRotaryDial);
                               ~WidgetRotaryDial            (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Pie_t*                  m_pPie;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef ROUND_METER_DEF
class WidgetRoundMeter : public CWidgetInterface
{
    public:
                                WidgetRoundMeter            (RoundMeter_t* pPie);
                               ~WidgetRoundMeter            (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Pie_t*                  m_pPie;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;

        uint16_t                m_Value;
};
#endif

#ifdef SPECTRUM_DEF
class WidgetSpectrum : public CWidgetInterface
{
    public:
                                WidgetSpectrum              (Spectrum_t* pSpectrum);
                               ~WidgetSpectrum              (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Spectrum_t*             m_pSpectrum;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;

        uint16_t                m_DotSize;
        BoxSize_t               m_BarSize;
        void*                   m_BarAddress;
        PixelFormat_e           m_PixelFormat;
};
#endif

#ifdef SPRITE_DEF
class WidgetSprite : public CWidgetInterface
{
    public:
                                WidgetSprite                (Sprite_t* pSprite);
                               ~WidgetSprite                (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (void);

        Sprite_t*               m_pSprite;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef TERMINAL_DEF
class WidgetTerminal : public CWidgetInterface
{
    public:
                                WidgetTerminal              (Terminal_t* pTerminal);
                               ~WidgetTerminal              (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (ServiceReturn_t* pService);

        Terminal_t*             m_pTerminal;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;

        uint8_t                 m_NumberOfLine;
        uint8_t                 m_NbOfCharPerLine;
        uint8_t                 m_OffsetLine;
        uint8_t                 m_CurrentLine;
        uint8_t                 m_LineSize;
        uint8_t*                m_pScreen;
};
#endif

#ifdef VIRTUAL_HUB_DEF
class WidgetVirtualHub : public CWidgetInterface
{
    public:
                                WidgetVirtualHub            (VirtualHub_t* pVirtualHub);
                               ~WidgetVirtualHub            (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        VirtualHub_t*           m_pVirtualHub;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;
};
#endif

#ifdef VIRTUAL_WINDOW_DEF
class WidgetVirtualWindow : public CWidgetInterface
{
    public:
                                WidgetVirtualWindow         (VirtualWindow_t* pVirtualWindow);
                               ~WidgetVirtualWindow         (){};

        Link_e                  Create                      (PageWidget_t* pPageWidget);
        Link_e                  Refresh                     (MsgRefresh_t* pMsg);
        void                    Finalize                    (void);

    private:

        void                    Draw                        (void);

        VirtualWindow_t*        m_pVirtualWindow;
        ServiceEvent_e          m_ServiceState;
        PageWidget_t*           m_pPageWidget;

        PixelFormat_e           m_PixelFormat;
        uint8_t*                m_pAddress;
};
#endif

//-------------------------------------------------------------------------------------------------

#endif //DIGINI_USE_GRAFX

