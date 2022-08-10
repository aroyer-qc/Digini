//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100_label.h
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

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Console Generic Label
#define VT100_LABEL_LANGUAGE_DEF(X_VT100_LBL_CFG) \
X_VT100_LBL_CFG( VT100_LBL_CLEAR_SCREEN,               "\033[2J\033[H",                                                                                            "\033[2J\033[H"                                                                                            ) \
X_VT100_LBL_CFG( VT100_LBL_RESET_TERMINAL,             "\033c\r\n",                                                                                                "\033c\r\n"                                                                                                ) \
X_VT100_LBL_CFG( VT100_LBL_HIDE_CURSOR,                "\033[?25l",                                                                                                "\033[?25l"                                                                                                ) \
X_VT100_LBL_CFG( VT100_LBL_SHOW_CURSOR,                "\033[?25h",                                                                                                "\033[?25h"                                                                                                ) \
X_VT100_LBL_CFG( VT100_LBL_ATTRIBUTE,                  "\033[%dm",                                                                                                 "\033[%dm"                                                                                                 ) \
X_VT100_LBL_CFG( VT100_LBL_SAVE_CURSOR,                "\033[s",                                                                                                   "\033[s"                                                                                                   ) \
X_VT100_LBL_CFG( VT100_LBL_RESTORE_CURSOR,             "\033[u",                                                                                                   "\033[u"                                                                                                   ) \
X_VT100_LBL_CFG( VT100_LBL_MOVE_LEFT_CURSOR,           "\033[D",                                                                                                   "\033[D"                                                                                                   ) \
X_VT100_LBL_CFG( VT100_LBL_SAVE_ATTRIBUTE,             "\0337",                                                                                                    "\0337"                                                                                                    ) \
X_VT100_LBL_CFG( VT100_LBL_RESTORE_ATTRIBUTE,          "\0338",                                                                                                    "\0338"                                                                                                    ) \
X_VT100_LBL_CFG( VT100_LBL_SET_CURSOR,                 "\033[%d;%df",                                                                                              "\033[%d;%df"                                                                                              ) \
X_VT100_LBL_CFG( VT100_LBL_CURSOR_TO_SELECT,           "\033[%d;28f    \033[4D",                                                                                   "\033[%d;28f    \033[4D"                                                                                   ) \
X_VT100_LBL_CFG( VT100_LBL_START_PRINTING,             "\033[5i",                                                                                                  "\033[5i"                                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_STOP_PRINTING,              "\033[4i",                                                                                                  "\033[4i"                                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_SCROLL_ZONE,                "\033[%d;%dr",                                                                                              "\033[%d;%dr"                                                                                              ) \
X_VT100_LBL_CFG( VT100_LBL_SCROLL_UP,                  "\033M",                                                                                                    "\033M"                                                                                                    ) \
X_VT100_LBL_CFG( VT100_LBL_EOL_ERASE,                  "\033[K",                                                                                                   "\033[K"                                                                                                   ) \
X_VT100_LBL_CFG( VT100_LBL_LINE_SEPARATOR,             "----------------------------------------------------------------------------------------------------\r\n", "----------------------------------------------------------------------------------------------------\r\n" ) \
X_VT100_LBL_CFG( VT100_LBL_SELECT,                     "\r\n Please Select:\r\n\r\n",                                                                              "\r\n Faire une Sélection:\r\n\r\n"                                                                        ) \
X_VT100_LBL_CFG( VT100_LBL_QUIT,                       "Quit\r\n",                                                                                                 "Quitter\r\n"                                                                                              ) \
X_VT100_LBL_CFG( VT100_LBL_ENTER_SELECTION,            "\r\n Enter Selection [<ESC>, 1 - %c] > ",                                                                  "\r\n Entrer Sélection [<ESC>, 1 - %c] > "                                                                 ) \
X_VT100_LBL_CFG( VT100_LBL_ESCAPE,                     "\r\n\r\n\r\n Press <ESC> to return to menu",                                                               "\r\n\r\n\r\n Presser <ESC> pour retourner au menu"                                                        ) \
X_VT100_LBL_CFG( VT100_LBL_INPUT_VALIDATION,           "Press <ENTER> to accept or <ESC> to cancel",                                                               "Presser <ENTER> pour accepter ou <ESC> pour annuler"                                                      ) \
X_VT100_LBL_CFG( VT100_LBL_SAVE_CONFIGURATION,         "Save the Configuration\r\n",                                                                               "Sauvegarder la Configuration\r\n"                                                                         ) \
X_VT100_LBL_CFG( VT100_LBL_MINIMUM,                    "Minimum",                                                                                                  "Minimum"                                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_MAXIMUM,                    "Maximum",                                                                                                  "Maximum"                                                                                                  ) \
X_VT100_LBL_CFG( VT100_LBL_TIME_DATE_STAMP,            "%04u-%02u-%02u %2u:%02u:%02u: ",                                                                           "%04u-%02u-%02u %2u:%02u:%02u: "                                                                           ) \
X_VT100_LBL_CFG( VT100_LBL_INT_TO_DIVIDE_BY_10,        "%ld.%d",                                                                                                   "%ld.%d"                                                                                                   ) \
X_VT100_LBL_CFG( VT100_LBL_INT_TO_DIVIDE_BY_100,       "%ld.%02d",                                                                                                 "%ld.%02d"                                                                                                 ) \
X_VT100_LBL_CFG( VT100_LBL_INT_TO_DIVIDE_BY_1000,      "%ld.%03d",                                                                                                 "%ld.%03d"                                                                                                 ) \
X_VT100_LBL_CFG( VT100_LBL_INT_NO_DIVIDE,              "%ld",                                                                                                      "%ld"                                                                                                      ) \
X_VT100_LBL_CFG( VT100_LBL_STRING_AND_ONE_SPACE,       "%s ",                                                                                                      "%s "                                                                                                      ) \

#define VT100_MONO_LABEL_LANGUAGE_DEF(X_VT100_LBL_CFG) \
X_VT100_LBL_CFG( VT100_LBL_BACK_WHITE_FORE_BLACK,      "\033[30m\033[47m",                                                                                         "\033[30m\033[47m"                                                                                         ) \
X_VT100_LBL_CFG( VT100_LBL_BACK_BLACK_FORE_WHITE,      "\033[37m\033[40m",                                                                                         "\033[37m\033[40m"                                                                                         ) \

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

