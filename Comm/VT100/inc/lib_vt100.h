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

#define VT100_OFFSET_COLOR_FOREGROUND     30
#define VT100_OFFSET_COLOR_BACKGROUND     40
#define CON_FIFO_PARSER_RX_SZ             64
#define VT100_SZ_NONE                     0
#define CONFIG_FLAG_SIZE                  8



#define VT100_STRING_QTS                  8
#define VT100_ITEMS_QTS                   8
#define VT100_STRING_SZ                   32

// Console Generic Label
#define VT100_LBL_CLEAR_SCREEN            "\033[2J\033[H"
#define VT100_LBL_RESET_TERMINAL          "\033c\r\n"
#define VT100_LBL_HIDE_CURSOR             "\033[?25l"
#define VT100_LBL_SHOW_CURSOR             "\033[?25h"
#define VT100_LBL_LINE_SEPARATOR          "----------------------------------------------------------------------------------------------------\r\n"
#define VT100_LBL_SELECT                  "\r\n Please Select:\r\n\r\n"
#define VT100_LBL_QUIT                    "Quit\r\n"
#define VT100_LBL_ENTER_SELECTION         "\r\n Enter Selection [<ESC>, 1 - %c] > "
#define VT100_LBL_ESCAPE                  "\r\n\r\n\r\n Press <ESC> to return to menu"
#define VT100_LBL_INPUT_VALIDATION        "Press <ENTER> to accept or <ESC> to cancel"
#define VT100_LBL_SAVE_CONFIGURATION      "Save the Configuration\r\n"
#define VT100_LBL_MINIMUM                 "Minimum"
#define VT100_LBL_MAXIMUM                 "Maximum"
#define VT100_LBL_ATTRIBUTE               "\033[%dm"
#define VT100_LBL_SAVE_CURSOR             "\033[s"
#define VT100_LBL_RESTORE_CURSOR          "\033[u"
#define VT100_LBL_SAVE_ATTRIBUTE          "\0337"
#define VT100_LBL_RESTORE_ATTRIBUTE       "\0338"
#define VT100_LBL_SET_CURSOR              "\033[%d;%df"
#define VT100_LBL_CURSOR_TO_SELECT        "\033[%d;28f    \e[4D"
#define VT100_LBL_START_PRINTING          "\033[5i"
#define VT100_LBL_STOP_PRINTING           "\033[4i"
#define VT100_LBL_TIME_DATE_STAMP         "%04u-%02u-%02u %2u:%02u:%02u: "
#define VT100_LBL_SCROLL_ZONE             "\033[%d;%dr"
#define VT100_LBL_SCROLL_UP               "\033M"
#define VT100_LBL_EOL_ERASE               "\033[K"

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum VT100_Menu_e
{
    VT100_MENU_DEF(EXPAND_VT100_MENU_AS_ENUM)
    NUMBER_OF_MENU,
    VT100_NO_MENU,
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

//-------------------------------------------------------------------------------------------------
// Function(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

class VT100_Terminal
{
    public:

                            VT100_Terminal              ();


        void                Process                     (void);
        void                Initialize                  (Console* pConsole, const char* pHeadLabel, const char* pDescription);
        void                DrawBox                     (uint8_t PosX, uint8_t PosY, uint8_t H_Size, uint8_t V_Size, VT100_Color_e ForeColor);
        void                DrawVline                   (uint8_t PosX, uint8_t PosY, uint8_t V_Size, VT100_Color_e ForeColor);
        void                GoToMenu                    (VT100_Menu_e MenuID);
        size_t              InMenuPrintf                (int nSize, const char* pFormat, ...);
        void                RestoreAttribute            (void);
        void                RestoreCursorPosition       (void);
        void                SaveAttribute               (void);
        void                SaveCursorPosition          (void);
        void                SetAttribute                (VT100_Attribute_e Attribute);

      #ifdef CONSOLE_USE_COLOR
        void                SetColor                    (VT100_Color_e ForeColor, VT100_Color_e BackColor);
        inline void         SetForeColor                (VT100_Color_e Color)		{ SetAttribute((CON_VT100_Attribute_e)Color + VT100__OFFSET_COLOR_FOREGROUND); }
        inline void         SetBackColor                (VT100_Color_e Color)       { SetAttribute((CON_VT100_Attribute_e)Color + VT100__OFFSET_COLOR_BACKGROUND); }
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

        void                DisplayDescription          (void);


void RX_Callback(uint8_t Data);

//      #if (VT100_IS_RUNNING_STAND_ALONE == DEF_ENABLED) // false it will alawys work with console
        //size_t              LoggingPrintf               (CLI_DebugLevel_e Level, const char* pFormat, ...);

// to check if needed in VT100
void                SeConsoleMuteLogs           (bool);
void                LockDisplay                 (bool);
void                DisplayTimeDateStamp        (nOS_TimeDate* pTimeDate);
bool                GetString                   (char* pBuffer, size_t Size);

    private:

        uint8_t             DisplayMenu                 (VT100_Menu_e MenuID);
        void                MenuSelectItems             (char ItemsChar);
        VT100_InputType_e   CallBack                    (VT100_InputType_e (*Callback)(uint8_t, VT100_CallBackType_e), VT100_CallBackType_e Type, uint8_t Item);
        void                EscapeCallback              (nOS_Timer* pTimer, void* pArg);
        void                InputString                 (void);
        void                InputDecimal                (void);
        void                ClearConfigFLag             (void);
        void                ClearGenericString          (void);

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
        char*                               m_pHeadLabel;
        char*                               m_pDescription;
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

        //static const VT100_MenuObject_t     m_Menu[NUMBER_OF_MENU];

        #define VT100_HEADER_CLASS_CONSTANT_MEMBER
        #include "vt100_var.h"            // Project variable
        #undef  VT100_HEADER_CLASS_CONSTANT_MEMBER

// not needed I think
//       EXPAND_VT100_MENU_AS_CALLBACK(VT100_CALLBACK_DEF)       // Generation of all prototype









/*


struct VT100_MenuDef_t
{
    Label_e           Label; //  ??? how  todo merge with label from digini
    VT100_InputType_e (*Callback)(uint8_t, VT100_CallBackType_e);
    VT100_Menu_e      NextMenu;
};

struct VT100_MenuObject_t
{
    const VT100_MenuDef_t*    pMenu;
    //size_t                    pMenuSize;
};


*/








};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#include "vt100_var.h"        // Project variable

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

