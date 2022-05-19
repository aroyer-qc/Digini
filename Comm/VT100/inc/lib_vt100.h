//-------------------------------------------------------------------------------------------------
//
//  File : vt100.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "label.h"
#include "console_cfg.h"
#include "lib_macro.h"
#include "fifo.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//#ifdef CONSOLE_GLOBAL
//    #define CONSOLE_EXTERN
//#else
//    #define CONSOLE_EXTERN extern
//#endif

//-------------------------------------------------------------------------------------------------

#define CON_OFFSET_COLOR_FOREGROUND     30
#define CON_OFFSET_COLOR_BACKGROUND     40
#define CON_FIFO_PARSER_RX_SZ           64
#define CON_SZ_NONE                     0
#define CONFIG_FLAG_SIZE                8
#define CON_STRING_QTS                  8
#define CON_ITEMS_QTS                   8
#define CON_STRING_SZ                   32


#ifndef VT100_USE_COLOR
#define VT100_SetColor(f,b)
#define VT100_SetForeColor(c)
#define VT100_SetBackColor(c)
#endif

// Console Generic Label
#define VT100_LBL_CLEAR_SCREEN            "\e[2J\e[H"
#define VT100_LBL_RESET_TERMINAL          "\ec\r\n"
#define VT100_LBL_HIDE_CURSOR             "\e[?25l"
#define VT100_LBL_SHOW_CURSOR             "\e[?25h"
#define VT100_LBL_LINE_SEPARATOR          "----------------------------------------------------------------------------------------------------\r\n"
#define VT100_LBL_SELECT                  "\r\n Please Select:\r\n\r\n"
#define VT100_LBL_QUIT                    "Quit\r\n"
#define VT100_LBL_ENTER_SELECTION         "\r\n Enter Selection [<ESC>, 1 - %c] > "
#define VT100_LBL_ESCAPE                  "\r\n\r\n\r\n Press <ESC> to return to menu"
#define VT100_LBL_INPUT_VALIDATION        "Press <ENTER> to accept or <ESC> to cancel"
#define VT100_LBL_SAVE_CONFIGURATION      "Save the Configuration\r\n"
#define VT100_LBL_MINIMUM                 "Minimum"
#define VT100_LBL_MAXIMUM                 "Maximum"
#define VT100_LBL_ATTRIBUTE               "\e[%dm"
#define VT100_LBL_SAVE_CURSOR             "\e[s"
#define VT100_LBL_RESTORE_CURSOR          "\e[u"
#define VT100_LBL_SAVE_ATTRIBUTE          "\e7"
#define VT100_LBL_RESTORE_ATTRIBUTE       "\e8"
#define VT100_LBL_SET_CURSOR              "\e[%d;%df"
#define VT100_LBL_CURSOR_TO_SELECT        "\e[%d;28f    \e[4D"
#define VT100_LBL_START_PRINTING          "\e[5i"
#define VT100_LBL_STOP_PRINTING           "\e[4i"
#define VT100_LBL_TIME_DATE_STAMP         "%04u-%02u-%02u %2u:%02u:%02u: "
#define VT100_LBL_SCROLL_ZONE             "\e[%d;%dr"
#define VT100_LBL_SCROLL_UP               "\eM"
#define VT100_LBL_EOL_ERASE               "\e[K"

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

struct VT100_MenuDef_t
{
    LABEL_Name_e      Label;
    VT100_InputType_e (*Callback)(uint8_t, VT100_CallBackType_e);
    VT100_Menu_e      NextMenu;
};

//-------------------------------------------------------------------------------------------------
// Function(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

class VT100_Terminal
{
    public:

        void            DrawBox                     (uint8_t PosX, uint8_t PosY, uint8_t H_Size, uint8_t V_Size, VT100_Color_e ForeColor);
        void            DrawVline                   (uint8_t PosX, uint8_t PosY, uint8_t V_Size, VT100_Color_e ForeColor);
        void            GoToMenu                    (VT100_Menu_e MenuID);
        size_t          InMenuPrintf                (int nSize, const char* pFormat, ...);
        void            RestoreAttribute            (void);
        void            RestoreCursorPosition       (void);
        void            SaveAttribute               (void);
        void            SaveCursorPosition          (void);
        void            SetAttribute                (CON_VT100_Attribute_e Attribute);

      #ifdef CONSOLE_USE_COLOR
        void            SetColor                    (VT100_Color_e ForeColor, VT100_Color_e BackColor);
        void            PrintSaveLabel              (uint8_t PosX, uint8_t PosY, VT100_Color_e Color);
        void            Bargraph                    (uint8_t PosX, uint8_t PosY, VT100_Color_e Color, uint8_t Value, uint8_t Max, uint8_t Size);
      #else
        void            InvertMono                  (bool Invert);
        void            PrintSaveLabel              (uint8_t PosX, uint8_t PosY);
        void            Bargraph                    (uint8_t PosX, uint8_t PosY, uint8_t Value, uint8_t Max, uint8_t Size);
      #endif

        void            SetCursorPosition           (uint8_t PosX, uint8_t PosY);
        void            SetScrollZone               (uint8_t FirstLine, uint8_t LastLine);

        // Use by user callback to request a decimal input
        void            GetDecimalInputValue        (uint32_t* pValue, uint8_t* pID);
        void            GetStringInput              (char* pString, uint8_t* pID);
        void            SetDecimalInput             (uint8_t PosX, uint8_t PosY, int32_t Minimum, int32_t Maximum, int32_t Value, uint16_t Divider, uint8_t ID, const char* pMsg);
        void            SetStringInput              (uint8_t PosX, uint8_t PosY, int32_t Maximum, uint8_t ID, const char* pMsg, const char* pString);

//        void            CON_SetConsoleMuteLogs      (bool Mute);
        void            ForceMenuRefresh            (void);

      #ifdef VT100_USE_COLOR
        inline void     SetForeColor                (VT100_Color_e Color)		{ SetAttribute((CON_VT100_Attribute_e)Color + CON_OFFSET_COLOR_FOREGROUND); }
        inline void     SetBackColor                (VT100_Color_e Color)       { SetAttribute((CON_VT100_Attribute_e)Color + CON_OFFSET_COLOR_BACKGROUND); }
      #endif

    private:

        VT100_CALLBACK_DEF(EXPAND_VT100_CMD_AS_FUNCTION)   // Generation of all prototype

        bool            m_IsItInStartup;
        bool            m_BackFromEdition;
        //char                      m_BufferParserRX[CLI_FIFO_PARSER_RX_SZ];
        //Fifo_t                    m_FIFO_ParserRX;
        //CLI_ParserInfo_t            m_AT_ParserInfo;
        //uint32_t         CLI_NewConfigFlag[CONFIG_FLAG_SIZE];
        //char                  CLI_GenericString[CLI_STRING_QTS][CLI_ITEMS_QTS][CLI_STRING_SZ];


}

//-------------------------------------------------------------------------------------------------


