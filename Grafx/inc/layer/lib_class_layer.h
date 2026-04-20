//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_layer.h
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
// Include(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

#define CLAYER_STACK_LEVEL  8

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class DisplayLayer
{
    public:

                                DisplayLayer            (Layer_e          VirtualLayer,
                                                         uint32_t         u32LayerAddress,
                                                         LayerType_e      ActiveOnLayer,
                                                         uint16_t         SizeX,
                                                         uint16_t         SizeY,
                                                         PixelFormat_e    PixelFormat);

        // Function

        static void             Initialize              (void);

        void                    Clear                   (void);
        void                    SetAddress              (uint32_t Address);
        uint32_t                GetAddress              (void);



        uint32_t                GetTotalSize            (void);
        Cartesian_t             GetSize                 (void);
        PixelFormat_e           GetPixelFormat          (void);
        uint8_t                 GetPixelSize            (void);
        void                    SetActive               (LayerType_e LayerType);
        LayerType_e             GetActive               (void);
        Layer_e                 GetVirtual              (void);
        void                    SetAlpha                (uint8_t);
        uint8_t                 GetAlpha                (void);

        // Static function

        static void             SetActiveLayer          (LayerType_e LayerType, DisplayLayer* pLayer);
        static void             SetActiveLayer          (LayerType_e LayerType, Layer_e Layer);

        static Layer_e          GetDrawing              (void);
        static void             SetDrawing              (Layer_e Layer);
        static void             PopDrawing              (void);
        static void             PushDrawing             (void);

        // Static function they all work on the drawing layer
        static void             SetColor                (ColorTable_e Index);
        static void             SetColor                (uint32_t Color);
        static uint32_t         GetColor                (void);
        static void             SetTextColor            (ColorTable_e Index);
        static void             SetTextColor            (uint32_t Color);
        static uint32_t         GetTextColor            (void);

        static uint8_t          GetPixelSize            (PixelFormat_e PixelFormat);

    private:

        static void             TakeMutex               (void);
        static void             GiveMutex               (void);

        //  Variables
        Layer_e                 m_VirtualLayer;                  // This is the memory layer
        uint32_t                m_LayerAddress;
        LayerType_e             m_ActiveOnLayer;                 // This is the physical if any ( to enable the physical part )
        PixelFormat_e           m_PixelFormat;
        uint8_t                 m_PixelSize;
        uint8_t                 m_Alpha;
        Cartesian_t             m_Size;
        uint32_t                m_TotalSize;
        uint32_t                m_Color;
        uint32_t                m_TextColor;

        // Static variables
        static bool             m_IsItInitialize;
        static nOS_Mutex        m_Mutex;
        static Layer_e          m_ActiveDrawingLayer;
        static DisplayLayer*    m_pActiveBG_Layer;
      #if (GRAFX_USE_CONSTRUCTION_BACKGROUND_LAYER == DEF_ENABLED)
        static DisplayLayer*    m_pConstructBG_Layer;
      #endif
        static DisplayLayer*    m_pActiveFG_Layer;
      #if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
        static DisplayLayer*    m_pConstructFG_Layer;
      #endif
        static uint8_t          m_LayerStackCounter;
        static Layer_e          m_LayerStack[CLAYER_STACK_LEVEL];
        static const uint8_t    m_LayerPixelSize[PIXEL_FORMAT_COUNT];
};

//-------------------------------------------------------------------------------------------------
// Global variable(s)
//-------------------------------------------------------------------------------------------------

extern DisplayLayer LayerTable[LAYER_COUNT];

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void                ClearLayer                  ();
void                SetConstructLayer           (LayerType_e LayerType, Layer_e Layer);
void                SetDrawingOnConstructLayer  (LayerType_e LayerType);
void                SetAlphaLayer               (uint8_t Alpha);

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
