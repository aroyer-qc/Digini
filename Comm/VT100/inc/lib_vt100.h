//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2022 Alain Royer.
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

#include "vt100_cfg.h"
#include "lib_vt100_expanding_macro.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define VT100_OFFSET_COLOR_FOREGROUND       30
#define VT100_OFFSET_COLOR_BACKGROUND       40
#define CON_FIFO_PARSER_RX_SZ               64
#define VT100_SZ_NONE                       0

                                                                      #define CONFIG_FLAG_SIZE                  8  //???

#define VT100_STRING_QTS                    8
#define VT100_ITEMS_QTS                     8
#define VT100_STRING_SZ                     32

// if VT100 is a task only
#define TASK_VT100_STACK_SIZE               256
#define TASK_VT100_PRIO                     4
#define TASK_VT100_SLEEP_TIME               16   // Run the task 60 times per seconds,
                                                 // so callback can refresh console

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------


enum VT100_Menu_e
{
    VT100_MENU_DEF(EXPAND_VT100_MENU_AS_ENUMS)
    NUMBER_OF_MENU,
    VT100_MENU_NONE,
};

enum VT100_Color_e
{
	VT100_COLOR_BLACK      = 0,
	VT100_COLOR_RED,
	VT100_COLOR_GREEN,
	VT100_COLOR_YELLOW,
	VT100_COLOR_BLUE,
	VT100_COLOR_MAGENTA,
	VT100_COLOR_CYAN,
	VT100_COLOR_WHITE,
};

enum VT100_Attribute_e
{
    VT100_RESET_ALL        = 0,
	VT100_BRIGHT,
	VT100_DIM,
	VT100_UNDERSCORE,
	VT100_BLINK,
	VT100_REVERSE,
	VT100_HIDDEN,
	VT100_FORE_BLACK       = 30,
	VT100_FORE_RED,
	VT100_FORE_GREEN,
	VT100_FORE_YELLOW,
	VT100_FORE_BLUE,
	VT100_FORE_MAGENTA,
	VT100_FORE_CYAN,
	VT100_FORE_WHITE,
	VT100_BACK_BLACK       = 40,
	VT100_BACK_RED,
	VT100_BACK_GREEN,
	VT100_BACK_YELLOW,
	VT100_BACK_BLUE,
	VT100_BACK_MAGENTA,
	VT100_BACK_CYAN,
	VT100_BACK_WHITE,
};

enum VT100_InputType_e
{
    VT100_INPUT_MENU_CHOICE,
    VT100_INPUT_DECIMAL,
    VT100_INPUT_STRING,
    VT100_INPUT_ESCAPE,
    VT100_INPUT_ESCAPE_TO_CONTINUE,
    VT100_INPUT_CLI,
};

enum VT100_CallBackType_e
{
    VT100_CALLBACK_INIT,
    VT100_CALLBACK_ON_INPUT,
    VT100_CALLBACK_REFRESH,
    VT100_CALLBACK_FLUSH,
};

struct VT100_MenuDef_t
{
    Label_e           Label;
    VT100_InputType_e (*Callback)(uint8_t, VT100_CallBackType_e);
    VT100_Menu_e      NextMenu;
};

struct VT100_MenuObject_t
{
    const VT100_MenuDef_t     Menu;
    size_t                    pMenuSize;
};


VT100_MENU_DEF(EXPAND_VT100_AS_MENU_ENUMS)

//-------------------------------------------------------------------------------------------------
// Function(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

class VT100_Terminal
{
    public:

                            VT100_Terminal              () {};


        void                Process                     (void);
        nOS_Error           Initialize                  (Console* pConsole, const char* pHeader);
        void                DrawBox                     (uint8_t PosX, uint8_t PosY, uint8_t H_Size, uint8_t V_Size, VT100_Color_e ForeColor);
        void                DrawVline                   (uint8_t PosX, uint8_t PosY, uint8_t V_Size, VT100_Color_e ForeColor);
        void                GoToMenu                    (VT100_Menu_e MenuID);
        size_t              InMenuPrintf                (int nSize, Label_e Label, ...);
        void                RestoreAttribute            (void);
        void                RestoreCursorPosition       (void);
        void                SaveAttribute               (void);
        void                SaveCursorPosition          (void);
        void                SetAttribute                (VT100_Attribute_e Attribute);

      #if (DIGINI_VT100_USE_COLOR == DEF_ENABLED)
        void                SetColor                    (VT100_Color_e ForeColor, VT100_Color_e BackColor);
        inline void         SetForeColor                (VT100_Color_e Color)		{ SetAttribute(VT100_Attribute_e(int(Color) + VT100_OFFSET_COLOR_FOREGROUND)); }
        inline void         SetBackColor                (VT100_Color_e Color)       { SetAttribute(VT100_Attribute_e(int(Color) + VT100_OFFSET_COLOR_BACKGROUND)); }
        void                PrintSaveLabel              (uint8_t PosX, uint8_t PosY, VT100_Color_e Color);
        void                Bargraph                    (uint8_t PosX, uint8_t PosY, VT100_Color_e Color, uint8_t Value, uint8_t Max, uint8_t Size);
      #else
        void                InvertMono                  (bool Invert);
        void                PrintSaveLabel              (uint8_t PosX, uint8_t PosY);
        void                Bargraph                    (uint8_t PosX, uint8_t PosY, uint8_t Value, uint8_t Max, uint8_t Size);
      #endif

        void                SetCursorPosition           (uint8_t PosX, uint8_t PosY);
        void                SetScrollZone               (uint8_t FirstLine, uint8_t LastLine);

        // Use by user callback to request a decimal input
        void                GetDecimalInputValue        (uint32_t* pValue, uint8_t* pID);
        void                GetStringInput              (char* pString, uint8_t* pID);
        void                SetDecimalInput             (uint8_t PosX, uint8_t PosY, int32_t Minimum, int32_t Maximum, int32_t Value, uint16_t Divider, uint8_t ID, const char* pMsg);
        void                SetStringInput              (uint8_t PosX, uint8_t PosY, int32_t Maximum, uint8_t ID, const char* pMsg, const char* pString);

//        void            CON_SetConsoleMuteLogs      (bool Mute);
        void                ForceMenuRefresh            (void);

        void                DisplayHeader               (void);


void RX_Callback(uint8_t Data);

// to check if needed in VT100
//void                SeConsoleMuteLogs           (bool);  this should be in console... beacause when in VT100 we don't want any debug message going thru
void                LockDisplay                 (bool);
void                DisplayTimeDateStamp        (nOS_TimeDate* pTimeDate);
bool                GetString                   (char* pBuffer, size_t Size);

    private:

        uint8_t             DisplayMenu                 (VT100_Menu_e MenuID);
        void                MenuSelectItems             (char ItemsChar);
        void                CallbackInitialize          (void);
        VT100_InputType_e   CallBack                    (VT100_InputType_e (*Callback)(uint8_t, VT100_CallBackType_e), VT100_CallBackType_e Type, uint8_t Item);
        void                EscapeCallback              (nOS_Timer* pTimer, void* pArg);
        void                InputString                 (void);
        void                InputDecimal                (void);
        void                ClearConfigFLag             (void);
        void                ClearGenericString          (void);
        VT100_CALLBACK(EXPAND_VT100_MENU_CALLBACK)                  // Generation of all user callback prototype

      #if (DIGINI_VT100_IS_A_TASK == DEF_ENABLED)
        static nOS_Thread                   m_Handle;
        static nOS_Stack                    m_Stack[TASK_VT100_STACK_SIZE];
        nOS_Sem                             m_SemTaskRun;
      #endif

        Console*                            m_pConsole;
        bool                                m_IsItInStartup;
        bool                                m_BackFromEdition;
        bool                                m_RefreshMenu;
        VT100_Menu_e                        m_MenuID;
        VT100_Menu_e                        m_FlushMenuID;
        VT100_InputType_e                   m_InputType;
        uint64_t                            m_Input;
        uint64_t                            m_InputCount;
        bool                                m_ValidateInput;
        uint8_t                             m_ItemsQts;
        bool                                m_BypassPrintf;
        bool                                m_LogsAreMuted;
        nOS_Timer                           m_EscapeTimer;
        bool                                m_IsDisplayLock;
        bool                                m_FlushNextEntry;
        char*                               m_pHeader;
        bool                                m_ForceRefresh;

        // Input string or decimal service
        int32_t                             m_Minimum;
        int32_t                             m_Maximum;
        uint8_t                             m_PosX;
        uint8_t                             m_PosY;
        int16_t                             m_ID;
        int32_t                             m_Value;
        int32_t                             m_RefreshValue;
        int32_t                             m_OldValue;
        uint16_t                            m_Divider;
        bool                                m_InputDecimalMode;
        volatile bool                       m_InEscapeSequence;
        int32_t                             m_InputPtr;
        int32_t                             m_RefreshInputPtr;
        char                                m_String[VT100_STRING_SZ + 1];
        bool                                m_InputStringMode;
        bool                                m_IsItString;
        uint32_t                            m_NewConfigFlag[CONFIG_FLAG_SIZE];
        char                                m_GenericString[VT100_STRING_QTS][VT100_ITEMS_QTS][VT100_STRING_SZ];
        static const VT100_MenuObject_t     m_Menu[NUMBER_OF_MENU];

        VT100_MENU_DEF(EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER)
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "vt100_var.h"        // Project variable

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

