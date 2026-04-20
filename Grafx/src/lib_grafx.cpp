//-------------------------------------------------------------------------------------------------
//
//  File : lib_grafx.cpp
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

#define GFX_GLOBAL
#include "./lib_digini.h"
#undef  GFX_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_LAYER_AS_CALC(ENUM_ID, WORK_LAYER, PIXEL_FORMAT, SIZE_X, SIZE_Y) ((SIZE_X * SIZE_Y) * GFX_PixelSize[PIXEL_FORMAT]) +
#define EXPAND_X_STATIC_IMAGE_AS_POINTER(ENUM_ID, SII) SII,

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

// Calculate the offset for the Free memory after the layers used by GRAFX
#if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
const uint32_t GFX_LoadingAddress =
    LAYER_DEF(EXPAND_X_LAYER_AS_CALC)
    GFX_BASE_ADDRESS;
#endif

#ifdef STATIC_IMAGE_DEF
const StaticImageInfo_t* StaticImageInfo[NUMBER_OF_STATIC_IMAGE] =
{
    STATIC_IMAGE_DEF(EXPAND_X_STATIC_IMAGE_AS_POINTER)
};
#endif

//-------------------------------------------------------------------------------------------------
// External low level function from driver
//-------------------------------------------------------------------------------------------------

extern void    DRV_Config                     (void);
extern void    DRV_DisplayOn                  (void);
extern void    DRV_DisplayOff                 (void);

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GRAFX_Initialize
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Perform pre-OS initialization
//
//-------------------------------------------------------------------------------------------------
SystemState_e GRAFX_Initialize(void)
{
   // SystemState_e State;
   // nOS_Error     Error;
    uint32_t      Address;

    // Init display device
    myGrafx->Initialize(GRAFX_DRIVER_ARGUMENT);         // Call the right driver according to configuration

  #ifdef LAYER_DEF
    // Pre calculate address for each layer and also clear the layer
    Address = GFX_BASE_ADDRESS;

    for(int Layer = LAYER_FIRST_ITEM; Layer < LAYER_COUNT; Layer++)
    {
        if(LayerTable[Layer].GetTotalSize() != 0)
        {
            LayerTable[Layer].SetAddress(Address);
            Address += LayerTable[Layer].GetTotalSize();
            LayerTable[Layer].Clear();
        }
        else
        {
            LayerTable[Layer].SetAddress(0);
        }
    }
  #endif

  #ifdef GFX_USE_SOFT_ALPHA
    GFX_PrecomputeAlphaTable();
  #endif

    DisplayLayer::SetActiveLayer(LAYER_FOREGROUND, FOREGROUND_DISPLAY_LAYER_0);
    DisplayLayer::SetActiveLayer(LAYER_BACKGROUND, BACKGROUND_DISPLAY_LAYER_0);
    DisplayLayer::SetDrawing(BACKGROUND_DISPLAY_LAYER_0);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GRAFX_PostInitialize
//
//   Parameter(s):  None
//   Return Value:  SystemState_e
//
//   Description:   Perform Post-OS initialization
//-------------------------------------------------------------------------------------------------
SystemState_e GRAFX_PostInitialize(void)
{
  #if (GRAFX_USE_POINTING_DEVICE == DEF_ENABLED)
    SystemState_e State;
  #endif
    nOS_Error     Error;

    DisplayLayer::Initialize();

  #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
    DB_Central.Set(&GFX_LoadingAddress, GFX_FREE_RAM_POINTER, 0, 0);   // Record the free RAM pointer in database at reload ID
  #endif

  #if (GRAFX_USE_FONT_SIZE_8 == DEF_ENABLED) || (GRAFX_USE_FONT_SIZE_12 == DEF_ENABLED) || (GRAFX_USE_FONT_SIZE_16 == DEF_ENABLED)
    FONT_Initialize();
  #endif

    if((Error = GUI_pTask->Initialize()) != NOS_OK)
    {
        return SYS_FAIL;
    }

  #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
    //  SKIN_pTask need then at the same pointer
    uint32_t      FreePointer;
    DB_Central.Get(&FreePointer, GFX_FREE_RAM_POINTER,  0, 0);
    LIB_AlignPointer(FreePointer);
    DB_Central.Set(&FreePointer, GFX_FREE_RAM_POINTER,  0, 0);
    DB_Central.Set(&FreePointer, GFX_FREE_RELOAD_POINTER, 0, 0);

    if((Error = SKIN_pTask->Initialize()) != NOS_OK)
    {
        return SYS_FAIL;
    }
  #endif

  #if (GRAFX_USE_POINTING_DEVICE == DEF_ENABLED)
    if((State = PDI_pDriver->Initialize(GRAFX_PostInitSubDriverPtr.PDI_pHardInterface)) != SYS_READY)
    {
        return State;
    }

    if((State = PDI_pTask->Initialize(PDI_pDriver, PDI_SWAP_NONE)) != SYS_READY)
    {
        return State;
    }
  #endif

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GRAFX_SelectBackgroundDrawingLayer
//                  GRAFX_SelectForegroundDrawingLayer
//
//   Parameter(s):  None
//   Return Value:  Layer_e
//
//   Description:   Return the appropriate layer to draw on according to configuration
//
//-------------------------------------------------------------------------------------------------
Layer_e GRAFX_SelectBackgroundDrawingLayer(void)
{
   #if (GRAFX_USE_CONSTRUCTION_BACKGROUND_LAYER == DEF_ENABLED)
     return CONSTRUCTION_BACKGROUND_LAYER;
   #else
     return BACKGROUND_DISPLAY_LAYER_0;
   #endif
}

Layer_e GRAFX_SelectForegroundDrawingLayer(void)
{
   #if (GRAFX_USE_CONSTRUCTION_FOREGROUND_LAYER == DEF_ENABLED)
    return CONSTRUCTION_FOREGROUND_LAYER;
   #else
    return FOREGROUND_DISPLAY_LAYER_0;
   #endif
}

//-------------------------------------------------------------------------------------------------
#endif // DIGINI_USE_GRAFX
