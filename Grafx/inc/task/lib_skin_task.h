//-------------------------------------------------------------------------------------------------
//
//  File :  lib_skin_task.h
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
#include "ff.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SKIN_TASK_STACK_SIZE                    320
#define SKIN_TASK_PRIO                          4

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef void (* SKIN_PostLoadCallBack_t)           (void);

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class SKIN_myClassTask
{
    public:
      #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
                        SKIN_myClassTask            (const char* pDrive, const char* pFileName);
      #else
                        SKIN_myClassTask            ();
      #endif

        void            Run                         (void);
        nOS_Error       Initialize                  (void);
      #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
        bool            IsSkinLoaded                (void);
        uint16_t        PercentLoader               (void);
      #endif
      #ifdef STATIC_SKIN_DEF
        bool            IsStaticSkinLoaded          (void);
      #endif

        void            RegisterPostLoadingCallback (SKIN_PostLoadCallBack_t pCallBack);

    private:

      #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
        SystemState_e   Load                        (void);
        SystemState_e   GetImageInfo                (void);
        SystemState_e   DeCompressAllImage          (void);
        SystemState_e   GetFontInfo                 (void);
        SystemState_e   DeCompressAllFont           (void);
        SystemState_e   Get_uint8_t                 (uint8_t* pValue);
        SystemState_e   Get_uint16_t                (uint16_t* pValue);
        SystemState_e   Get_uint32_t                (uint32_t* pValue);
      #endif
      #ifdef STATIC_SKIN_DEF
        void            StaticLoad                  (void);           // Initialize all static image
      #endif

        static nOS_Thread                           m_Handle;
        static nOS_Stack                            m_Stack[SKIN_TASK_STACK_SIZE];

        SKIN_PostLoadCallBack_t                     m_pCallBack;
        nOS_Flag                                    m_SkinFlags;

      #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
        bool                                        m_IsSkinLoaded;
        uint32_t                                    m_TotalToLoad;
        uint32_t                                    m_ReadCount;
        FATFS*                                      m_pFS;
        FIL*                                        m_pFile;                // File objects
        FILINFO*                                    m_pFileInfo;
        FRESULT                                     m_FResult;
        const char*                                 m_pDrive;
        char                                        m_Path[15];
        uint8_t*                                    m_pRawInputBuffer;
      #endif
      #ifdef STATIC_SKIN_DEF
        volatile bool                               m_IsStaticLoaded;
      #endif
        uint16_t                                    m_ItemCount;
        uint32_t*                                   m_pDataSize;
        uint8_t*                                    m_pCompressionMethod;
        CompxWorkMem_t                              m_CompxWorkMem;
        DeCompression*                              m_pDecompress;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef LIB_SKIN_TASK_GLOBAL
  #if (GRAFX_USE_LOAD_SKIN == DEF_ENABLED)
                     class   SKIN_myClassTask       SKIN_Task("0:", GRAFX_SKIN_FILENAME);
  #else
                     class   SKIN_myClassTask       SKIN_Task;
  #endif

                     class   SKIN_myClassTask*      SKIN_pTask = &SKIN_Task;
#else
    extern           class   SKIN_myClassTask       SKIN_Task;
    extern           class   SKIN_myClassTask*      SKIN_pTask;
#endif

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

extern "C" void SKIN_TaskWrapper    (void* pvParameters);

//-------------------------------------------------------------------------------------------------

#endif // DIGINI_USE_GRAFX
