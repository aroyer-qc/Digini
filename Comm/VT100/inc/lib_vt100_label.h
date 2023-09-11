//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100_label.h
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "vt100_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Console Generic Label
#define VT100_LABEL_LANGUAGE_DEF(X_VT100_LBL_CFG) \
X_VT100_LBL_CFG( VT100_LBL_CLEAR_SCREEN,               "\033[2J\033[H",                                                                                            nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_RESET_TERMINAL,             "\033c\r",                                                                                                  nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_HIDE_CURSOR,                "\033[?25l",                                                                                                nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SHOW_CURSOR,                "\033[?25h",                                                                                                nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_ATTRIBUTE,                  "\033[%dm",                                                                                                 nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SAVE_CURSOR,                "\033[s",                                                                                                   nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_RESTORE_CURSOR,             "\033[u",                                                                                                   nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_MOVE_LEFT_CURSOR,           "\033[D",                                                                                                   nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_WHITE_MOVE_CURSOR_2_TO_LEFT,"\033[40m \033[2D",                                                                                         nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SAVE_ATTRIBUTE,             "\0337",                                                                                                    nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_RESTORE_ATTRIBUTE,          "\0338",                                                                                                    nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SET_CURSOR,                 "\033[%d;%df",                                                                                              nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_CURSOR_TO_SELECT,           "\033[%d;28f    \033[4D",                                                                                   nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_START_PRINTING,             "\033[5i",                                                                                                  nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_STOP_PRINTING,              "\033[4i",                                                                                                  nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SCROLL_ZONE,                "\033[%d;%dr",                                                                                              nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SCROLL_UP,                  "\033M",                                                                                                    nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_EOL_ERASE,                  "\033[K",                                                                                                   nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_ERASE_FROM_CURSOR_N_CHAR,   "\033[%dX",                                                                                                 nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_LINE_SEPARATOR,             "----------------------------------------------------------------------------------------------------\r",   nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SELECT,                     "Please Select:\r",                                                                                         "Faire une Sélection:\r"                                                                 ) \
X_VT100_LBL_CFG( VT100_LBL_QUIT,                       "Quit\r",                                                                                                   "Quitter\r"                                                                              ) \
X_VT100_LBL_CFG( VT100_LBL_ENTER_SELECTION,            "\r Enter Selection [<ESC>, 1 - %c] > ",                                                                    "\r Entrer Sélection [<ESC>, 1 - %c] > "                                                 ) \
X_VT100_LBL_CFG( VT100_LBL_ESCAPE,                     "\r\r\r Press <ESC> to return to menu",                                                                     "\r\r\r Presser <ESC> pour retourner au menu"                                            ) \
X_VT100_LBL_CFG( VT100_LBL_INPUT_VALIDATION,           "Press <ENTER> to accept or <ESC> to cancel",                                                               "Presser <ENTER> pour accepter ou <ESC> pour annuler"                                    ) \
X_VT100_LBL_CFG( VT100_LBL_SAVE_CONFIGURATION,         "Save the Configuration",                                                                                   "Sauvegarder la Configuration"                                                           ) \
X_VT100_LBL_CFG( VT100_LBL_MINIMUM,                    "Minimum",                                                                                                  nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_MAXIMUM,                    "Maximum",                                                                                                  nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_TIME,                       "%u:%02u:%02u ",                                                                                            nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_DATE,                       "%s %u, %u ",                                                                                               nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_TIME_DATE_STAMP,            "%04u-%02u-%02u %2u:%02u:%02u: ",                                                                           nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_INT_TO_DIVIDE_BY_10,        "%ld.%d",                                                                                                   nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_INT_TO_DIVIDE_BY_100,       "%ld.%02d",                                                                                                 nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_INT_TO_DIVIDE_BY_1000,      "%ld.%03d",                                                                                                 nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_INT_NO_DIVIDE,              "%ld",                                                                                                      nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_LONG_UNSIGNED_SEMICOLON,    "%lu:",                                                                                                     nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_UNSIGNED_2_DIGIT_SEMICOLON, "%02u:",                                                                                                    nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_UNSIGNED_2_DIGIT,           "%02u",                                                                                                     nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_STRING_AND_ONE_SPACE,       "%s ",                                                                                                      nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_STATUS,                     "( )",                                                                                                      nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_FULL_DATE,                  "%s, %s %u, %u, %u:%02u:%02u ",                                                                             "\x1A$0%s $2%u $1%s $3%u, $4%u:$5%02u:$6%02u "                                          ) \
X_VT100_LBL_CFG( VT100_LBL_NOW,                        "Now:",                                                                                                     "Maintenant:"                                                                            ) \
X_VT100_LBL_CFG( VT100_LBL_UPTIME,                     "Up Time:",                                                                                                 "Temps Écoulé:"                                                                          ) \
X_VT100_LBL_CFG( VT100_LBL_PERCENT_VALUE,              "%ld %%",                                                                                                   nullptr                                                                                  ) \



#define VT100_MONO_LABEL_LANGUAGE_DEF(X_VT100_LBL_CFG) \
X_VT100_LBL_CFG( VT100_LBL_BACK_WHITE_FORE_BLACK,      "\033[30m\033[47m",                                                                                         nullptr                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_BACK_BLACK_FORE_WHITE,      "\033[37m\033[40m",                                                                                         nullptr                                                                                  ) \

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

