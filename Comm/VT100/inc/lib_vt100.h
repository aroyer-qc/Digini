//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "vt100_cfg.h"
#include "lib_vt100_expanding_macro.h"

//-------------------------------------------------------------------------------------------------
// Configuration check
//-------------------------------------------------------------------------------------------------

#if (VT100_USE_STANDARD_MENU_STATIC_INFO == DEF_ENABLED) && (VT100_USE_USER_MENU_STATIC_INFO == DEF_ENABLED)
  #pragma message "Cannot have both VT100_USE_STANDARD_MENU_STATIC_INFO and VT100_USE_USER_MENU_STATIC_INFO use at the same time"
  #error DEF_ENABLED Error
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define VT100_X_SIZE                        100
#define VT100_Y_SIZE                        60

#define VT100_OFFSET_COLOR_FOREGROUND       30
#define VT100_OFFSET_COLOR_BACKGROUND       40
#define CON_FIFO_PARSER_RX_SZ               64

#define CONFIG_FLAG_SIZE                    8  //???

#define VT100_STRING_QTS                    8
#define VT100_ITEMS_QTS                     8
#define VT100_STRING_SZ                     64

#if (VT100_USE_COLOR != DEF_ENABLED)
#define myVT100.SetColor(...)
#define myVT100.SetForeColor(...)
#define myVT100.SetBackColor(...)
#endif

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum VT100_Menu_e
{
    VT100_MENU_DEF(EXPAND_VT100_MENU_AS_ENUM)
    NUMBER_OF_MENU,
    VT100_MENU_NONE,
    VT100_MENU_NONE_ID = VT100_MENU_NONE,
};

enum VT100_Callback_e
{
    VT100_CALLBACK(EXPAND_VT100_MENU_AS_ENUM)               // Strange... this should be identical to the previous expand
    NUMBER_OF_CALLBACK,
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
  //  VT100_INPUT_ESCAPE_TO_CONTINUE,
};

enum VT100_CallBackType_e
{
    // For page that contain no menu selection and has only dynamic widget
    VT100_CALLBACK_INIT,
    VT100_CALLBACK_REFRESH_ONCE,
    VT100_CALLBACK_REFRESH,
    VT100_CALLBACK_FLUSH,

    VT100_CALLBACK_ON_INPUT,           // Selection input of items in thjs menu.

    // Dynamic item in selection menu
  // VT100_CALLBACK_ON_INIT,            // For Initializing items when they are listed in the menu (dynamic info on the menu page)
//    VT100_CALLBACK_ON_MENU_REFRESH,         // For Refreshing items when they are listed in the menu (dynamic info on the menu page)
    VT100_CALLBACK_ON_FLUSH,           // When escaping or going through another menu (EX. release memory)
};

typedef VT100_InputType_e (*CallbackMethod_t)(uint8_t, VT100_CallBackType_e);

struct VT100_MenuDef_t
{
    Label_e           Label;
    VT100_InputType_e (*pCallback)(uint8_t, VT100_CallBackType_e);
    //CallbackMethod_t  pCallback;
    VT100_Menu_e      NextMenu;
};

struct VT100_MenuObject_t
{
    const VT100_MenuDef_t*    pDefinition;
    size_t                    Size;
};

VT100_MENU_DEF(EXPAND_AS_MENU_ENUMS_ITEM)

//-------------------------------------------------------------------------------------------------
// class
//-------------------------------------------------------------------------------------------------

class VT100_Terminal : public ChildProcessInterface
{
    public:
                            VT100_Terminal              () {};

        void                IF_Process                  (void);
        nOS_Error           Initialize                  (Console* pConsole);
      #if (VT100_USER_CALLBACK_INITIALIZE == DEF_ENABLED)
        void                CallbackInitialize          (void);
      #endif
        void                DrawBox                     (uint8_t PosX, uint8_t PosY, uint8_t H_Size, uint8_t V_Size, VT100_Color_e ForeColor);
        void                DrawVline                   (uint8_t PosX, uint8_t PosY, uint8_t V_Size, VT100_Color_e ForeColor);
        void                GoToMenu                    (VT100_Menu_e MenuID);
        size_t              InMenuPrintf                (Label_e Label, ...);
        size_t              InMenuPrintf                (uint8_t PosX, uint8_t PosY, Label_e Label, ...);       // TODO this should use part of the code from the prior function
        void                RestoreAttribute            (void);
        void                RestoreCursorPosition       (void);
        void                SaveAttribute               (void);
        void                SaveCursorPosition          (void);
        void                SetAttribute                (VT100_Attribute_e Attribute);

      #if (VT100_USE_COLOR == DEF_ENABLED)
        void                SetColor                    (VT100_Color_e ForeColor, VT100_Color_e BackColor);
        inline void         SetForeColor                (VT100_Color_e Color)		{ SetAttribute(VT100_Attribute_e(int(Color) + VT100_OFFSET_COLOR_FOREGROUND)); }
        inline void         SetBackColor                (VT100_Color_e Color)       { SetAttribute(VT100_Attribute_e(int(Color) + VT100_OFFSET_COLOR_BACKGROUND)); }
        void                UpdateSaveLabel             (VT100_Color_e Color);
        void                Bargraph                    (uint8_t PosX, uint8_t PosY, VT100_Color_e Color, uint8_t Value, uint8_t Max, uint8_t Size);
      #else
        void                InvertMono                  (bool Invert);
        void                Bargraph                    (uint8_t PosX, uint8_t PosY, uint8_t Value, uint8_t Max, uint8_t Size);
      #endif

        void                SetCursorPosition           (uint8_t PosX, uint8_t PosY);
        void                SetScrollZone               (uint8_t FirstLine, uint8_t LastLine);

        // Use by user callback to request a decimal input
        void                GetDecimalInputValue        (uint32_t* pValue, uint8_t* pID);
        void                GetStringInput              (char* pString, uint8_t* pID);
        void                SetDecimalInput             (uint8_t PosX, uint8_t PosY, int32_t Minimum, int32_t Maximum, int32_t Value, uint16_t Divider, uint8_t ID, Label_e Label);
        void                SetStringInput              (uint8_t PosX, uint8_t PosY, int32_t Maximum, uint8_t ID, Label_e Label, const char* pString);


        uint32_t            GetConfigFlag               (int Flag)                      { return m_ConfigFlag[Flag];  }      // todo check range or change the method
        void                SetConfigFlag               (int Flag, uint32_t Value)      { m_ConfigFlag[Flag] = Value; }      // todo check range or change the method

        void                DisplayTimeDateStamp        (uint8_t PosX, uint8_t PosY, DateAndTime_t* pTimeDate);

// to check if needed in VT100
void                LockDisplay                 (bool);
bool                GetString                   (char* pBuffer, size_t Size);

    private:

        void                        ProcessRX                   (void);
        void                        DisplayMenu                 (void);
        void                        FinalizeAllItems            (void);
      #if (VT100_USE_STANDARD_MENU_STATIC_INFO == DEF_ENABLED) || (VT100_USER_MENU_STATIC_INFO == DEF_ENABLED)
        void                        PrintMenuStaticInfo         (void);
      #endif
        void                        ClearScreenWindow           (uint8_t PosX, uint8_t PosY, uint8_t SizeX, uint8_t SizeY);
        void                        ClearInputMenuSelection     (void);
        void                        RepeatChar                  (uint8_t Char, size_t Count);
        void                        MenuSelectItems             (char ItemsChar);

        VT100_InputType_e           CallBack                    (VT100_InputType_e (*pCallback)(uint8_t, VT100_CallBackType_e), VT100_CallBackType_e Type, uint8_t Item);
        static void                 EscapeCallback              (nOS_Timer* pTimer, void* pArg);
        void                        InputString                 (void);
        void                        InputDecimal                (void);
        void                        ClearConfigFLag             (void);
        size_t                      MenuPrintfCommon            (Label_e Label, va_list* p_vaArg);
        static VT100_InputType_e    CALLBACK_None               (uint8_t Input, VT100_CallBackType_e Type);
        VT100_CALLBACK(EXPAND_VT100_MENU_CALLBACK)                  // Generation of all user callback prototype

        Console*                            m_pConsole;
        bool                                m_IsItInitialized;
        bool                                m_IsItInStartup;
        VT100_Menu_e                        m_MenuID;
        uint8_t                             m_PosY_SaveLabel;
        uint8_t                             m_LastSetCursorPosX;                // Set position to input selection location on screen
        uint8_t                             m_LastSetCursorPosY;
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
        bool                                m_ForceRefresh;
        bool                                m_RefreshOnce;

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
        char*                               m_pString;
        bool                                m_InputStringMode;
        bool                                m_IsItString;
        uint32_t                            m_ConfigFlag[CONFIG_FLAG_SIZE];
        static const VT100_MenuObject_t     m_Menu[NUMBER_OF_MENU];

        VT100_MENU_DEF(EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER)
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef VT100_GLOBAL

class VT100_Terminal            myVT100;

#else

extern class VT100_Terminal     myVT100;

#endif // VT100_GLOBAL

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

