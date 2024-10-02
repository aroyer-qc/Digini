//-------------------------------------------------------------------------------------------------
//
//  File : lib_label.h
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

#if (DIGINI_USE_LABEL == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Expanding macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_LBL_CFG_AS_ENUM(ENUM_ID, LBL1, LBL2) ENUM_ID,
#define EXPAND_X_LBL_CFG_AS_DATA(ENUM_ID, LBL1, LBL2) {LBL1, LBL2},

//-------------------------------------------------------------------------------------------------
// X macro(s)
//-------------------------------------------------------------------------------------------------

// Common label
#define COMMON_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_NULL,                       nullptr,                          nullptr                                         ) \
X_LABEL( LBL_OK,                         "Ok",                             nullptr                                         ) \
X_LABEL( LBL_CLEAR_SCREEN,               "\033[2J\033[H",                  nullptr                                         ) \
X_LABEL( LBL_RESET_TERMINAL,             "\033c\n",                        nullptr                                         ) \
X_LABEL( LBL_STRING,                     "%s",                             nullptr                                         ) \
X_LABEL( LBL_STRING_LINEFEED,            "%s\n",                           nullptr                                         ) \
X_LABEL( LBL_CHAR,                       "%c",                             nullptr                                         ) \
X_LABEL( LBL_INT,                        "%d",                             nullptr                                         ) \
X_LABEL( LBL_LINEFEED,                   "\n",                             nullptr                                         ) \
X_LABEL( LBL_DOUBLE_LINEFEED,            "\n\n",                           nullptr                                         ) \
X_LABEL( LBL_TIME,                       "%2u:%02u:%02u ",                 nullptr                                         ) \
X_LABEL( LBL_DATE,                       "%s %u, %u ",                     nullptr                                         ) \
X_LABEL( LBL_FULL_DATE,                  "%s, %s %u, %u, %u:%02u:%02u ",   "\x1A$0%s $2%u $1%s $3%u, $4%u:$5%02u:$6%02u "  ) \
X_LABEL( LBL_TIME_DATE_STAMP,            "%04u-%02u-%02u %2u:%02u:%02u: ", nullptr                                         ) \
X_LABEL( LBL_INT_TO_DIVIDE_BY_10,        "%ld.%d ",                        nullptr                                         ) \
X_LABEL( LBL_INT_TO_DIVIDE_BY_100,       "%ld.%02d ",                      nullptr                                         ) \
X_LABEL( LBL_INT_TO_DIVIDE_BY_1000,      "%ld.%03d ",                      nullptr                                         ) \
X_LABEL( LBL_INT_NO_DIVIDE,              "%ld ",                           nullptr                                         ) \
X_LABEL( LBL_LONG_UNSIGNED,              "%lu ",                           nullptr                                         ) \
X_LABEL( LBL_LONG_UNSIGNED_SEMICOLON,    "%lu:",                           nullptr                                         ) \
X_LABEL( LBL_UNSIGNED_2_DIGIT_SEMICOLON, "%02u:",                          nullptr                                         ) \
X_LABEL( LBL_UNSIGNED_2_DIGIT,           "%02u ",                          nullptr                                         ) \
X_LABEL( LBL_PERCENT,                    "%2d",                            nullptr                                         ) \
X_LABEL( LBL_STRING_AND_ONE_SPACE,       "%s ",                            nullptr                                         ) \
X_LABEL( LBL_SIZE_GIGABYTES,             "%u.%02u GBytes",                 "%u.%02u GOctets"                               ) \
X_LABEL( LBL_SIZE_MEGABYTES,             "%u.%02u MBytes",                 "%u.%02u MOctets"                               ) \
X_LABEL( LBL_SIZE_KILOBYTES,             "%u KBytes",                      "%u KOctets"                                    ) \
X_LABEL( LBL_SIZE_BYTES,                 "%lu Bytes  ",                    "%lu Octets  "                                  ) \
X_LABEL( LBL_YES,                        "Yes",                            "Oui"                                           ) \
X_LABEL( LBL_NO,                         "No ",                            "Non"                                           ) \
X_LABEL( LBL_DEGREE_CELSIUS,             "\xF8" "C",                       nullptr                                         ) \
X_LABEL( LBL_DEGREE_FAHRENHEIT,          "\xF8" "F",                       nullptr                                         ) \
X_LABEL( LBL_ENABLED,                    "Enabled ",                       "Activ\x82 "                                    ) \
X_LABEL( LBL_DISABLED,                   "Disabled",                       "D\x82" "activ\x82"                             ) \
X_LABEL( LBL_SYSTEM_INFO,                "System Info",                    "Info Syst\x8ame"                               ) \
X_LABEL( LBL_MAIN_MENU,                  "Main Menu",                      "Menu Principal"                                ) \

// Network label
#define NET_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_NETWORK_INFO,               "Network Information",            "Information R\x82seau"                         ) \
X_LABEL( LBL_IP_ADDR,                    "IPV4 Address . . . . . :",       "Addresse IPV4  . . . . :"                      ) \
X_LABEL( LBL_IP_MASK,                    "Subnet Masks . . . . . :",       "Masque Sous-R\x82seau . . :"                   ) \
X_LABEL( LBL_IP_GATEWAY,                 "Default Gateway  . . . :",       "Passerelle D\x82" "faut  . . :"                ) \
X_LABEL( LBL_IP_DNS,                     "DNS Servers  . . . . . :",       "Serveurs DNS . . . . . :"                      ) \
X_LABEL( LBL_IP_DHCP_STATE,              "DHCP State . . . . . . :",       "Status DHCP  . . . . . :"                      ) \
X_LABEL( LBL_IP_LINK_STATE,              "Link State . . . . . . :",       "Status Lien  . . . . . :"                      ) \
X_LABEL( LBL_IP_LINK_SPEED,              "Link Speed . . . . . . :",       "Vitesse du Lien  . . . :"                      ) \
X_LABEL( LBL_MAC_ADDRESS,                "MAC Address  . . . . . :",       "Addresse MAC . . . . . :"                      ) \
X_LABEL( LBL_MAC_ADDRESS_VALUE,          "%02X:%02X:%02X:%02X:%02X:%02X",  nullptr                                         ) \
X_LABEL( LBL_ETH_RX_COUNT,               "RX Packet Count  . . . :",       "Nb de Paquets Re\x87" "es  . :"                ) \
X_LABEL( LBL_ETH_TX_COUNT,               "TX Packet Count  . . . :",       "Nb de Paquets Transmis :"                      ) \
X_LABEL( LBL_ETH_DROP,                   "Dropped  . . :",                 "Non Trait\x82" "es :"                          ) \

#define DATE_LABEL_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_JANUARY,                    "January",                        "Janvier"                                       ) \
X_LABEL( LBL_FEBRUARY,                   "February",                       "F\x82vrier"                                    ) \
X_LABEL( LBL_MARCH,                      "March",                          "Mars"                                          ) \
X_LABEL( LBL_APRIL,                      "April",                          "Avril"                                         ) \
X_LABEL( LBL_MAY,                        "May",                            "Mai"                                           ) \
X_LABEL( LBL_JUNE,                       "June",                           "Juin"                                          ) \
X_LABEL( LBL_JULY,                       "July",                           "Juillet"                                       ) \
X_LABEL( LBL_AUGUST,                     "August",                         "Ao\x93t"                                       ) \
X_LABEL( LBL_SEPTEMBER,                  "September",                      "Septembre"                                     ) \
X_LABEL( LBL_OCTOBER,                    "October",                        "Octobre"                                       ) \
X_LABEL( LBL_NOVEMBER,                   "November",                       "Novembre"                                      ) \
X_LABEL( LBL_DECEMBER,                   "December",                       "D\x82" "cembre"                                ) \
X_LABEL( LBL_SUNDAY,                     "Sunday",                         "Dimanche"                                      ) \
X_LABEL( LBL_MONDAY,                     "Monday",                         "Lundi"                                         ) \
X_LABEL( LBL_TUESDAY,                    "Tuesday",                        "Mardi"                                         ) \
X_LABEL( LBL_WEDNESDAY,                  "Wednesday",                      "Mercredi"                                      ) \
X_LABEL( LBL_THURSDAY,                   "Thursday",                       "Jeudi"                                         ) \
X_LABEL( LBL_FRIDAY,                     "Friday",                         "Vendredi"                                      ) \
X_LABEL( LBL_SATURDAY,                   "Saturday",                       "Samedi"                                        ) \

#define TIME_LABEL_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_HOUR,                       "Hour",                           "Heure"                                         ) \
X_LABEL( LBL_MINUTE,                     "Minute",                         nullptr                                         ) \
X_LABEL( LBL_SECOND,                     "Second",                         "Seconde"                                       ) \
X_LABEL( LBL_YEAR,                       "Year",                           "Ann\x82" "e"                                   ) \
X_LABEL( LBL_MONTH,                      "Month",                          "Mois"                                          ) \
X_LABEL( LBL_DAY,                        "Day",                            "Jour"                                          ) \

#define PRODUCT_LABEL_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_VENDOR_NAME_INFO,           "Vendor:           ",             "Vendeur:          "                            ) \
X_LABEL( LBL_FW_NAME_INFO,               "Name:             ",             "Nom:              "                            ) \
X_LABEL( LBL_FW_VERSION_INFO,            "Version:          ",             nullptr                                         ) \
X_LABEL( LBL_GUI_NAME_INFO,              "GUI Name:         ",             "Nom GUI:          "                            ) \
X_LABEL( LBL_GUI_VERSION_INFO,           "GUI Version:      ",             "Version GUI:      "                            ) \
X_LABEL( LBL_HARDWARE_INFO,              "Hardware:         ",             "Mat\x82riel:         "                         ) \
X_LABEL( LBL_SERIAL_INFO,                "Serial No.:       ",             "No. S\x82rie:        "                         ) \
X_LABEL( LBL_COMPILE_DATE_INFO,          "Built Date:       ",             "Date logiciel:    "                            ) \
X_LABEL( LBL_VENDOR_NAME,                OEM_VENDOR_NAME,                  nullptr                                         ) \
X_LABEL( LBL_MODEL_NAME,                 OEM_MODEL_NAME,                   nullptr                                         ) \
X_LABEL( LBL_FIRMWARE_NAME,              OUR_FIRMWARE_NAME,                nullptr                                         ) \
X_LABEL( LBL_FIRMWARE_VERSION,           OUR_FIRMWARE_VERSION,             nullptr                                         ) \
X_LABEL( LBL_DIGINI_NAME,                OUR_FIRMWARE_DIGINI_NAME,         nullptr                                         ) \
X_LABEL( LBL_DIGINI_VERSION,             OUR_FIRMWARE_DIGINI_VERSION,      nullptr                                         ) \
X_LABEL( LBL_SERIAL_NUMBER,              OEM_SERIAL_NUMBER,                nullptr                                         ) \
X_LABEL( LBL_BUILT_DATE,                 OUR_FIRMWARE_BUILD_DATE,          nullptr                                         ) \
X_LABEL( LBL_CPU_VOLTAGE,                "Voltages:",                      nullptr                                         ) \
X_LABEL( LBL_CPU_TEMPERATURE,            "CPU Temp:         ",             nullptr                                         ) \

#define DECIMAL_KEY_LABEL_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_1,                          "1",                              nullptr                                         ) \
X_LABEL( LBL_2,                          "2",                              nullptr                                         ) \
X_LABEL( LBL_3,                          "3",                              nullptr                                         ) \
X_LABEL( LBL_4,                          "4",                              nullptr                                         ) \
X_LABEL( LBL_5,                          "5",                              nullptr                                         ) \
X_LABEL( LBL_6,                          "6",                              nullptr                                         ) \
X_LABEL( LBL_7,                          "7",                              nullptr                                         ) \
X_LABEL( LBL_8,                          "8",                              nullptr                                         ) \
X_LABEL( LBL_9,                          "9",                              nullptr                                         ) \
X_LABEL( LBL_0,                          "0",                              nullptr                                         ) \
X_LABEL( LBL_DEL,                        "Del",                            nullptr                                         ) \
X_LABEL( LBL_DOT,                        ".",                              nullptr                                         ) \
X_LABEL( LBL_SIGN,                       "-",                              nullptr                                         ) \
X_LABEL( LBL_TOGGLE_SIGN,                "+/-",                            nullptr                                         ) \

#define HEXADECIMAL_KEY_LABEL_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_A,                          "A",                              nullptr                                         ) \
X_LABEL( LBL_B,                          "B",                              nullptr                                         ) \
X_LABEL( LBL_C,                          "C",                              nullptr                                         ) \
X_LABEL( LBL_D,                          "D",                              nullptr                                         ) \
X_LABEL( LBL_E,                          "E",                              nullptr                                         ) \
X_LABEL( LBL_F,                          "F",                              nullptr                                         ) \

#define STANDARD_TEST_STRING_LANGUAGE_DEF(X_LABEL) \
X_LABEL( LBL_TEST,  "the quick brown fox jumps over the lazy dog",         "portez ce vieux whisky au juge blond qui fume" ) \

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CMD_LINE == DEF_ENABLED)

#if (CLI_USE_AT_PREFIX_ON_COMMAND == DEF_ENABLED)
    #define CLI_RESPONSE_FOR_LABEL              "AT%s = %s%s\n>"
#else
    #define CLI_RESPONSE_FOR_LABEL              "%s = %s%s\n>"
#endif

// Console Generic Label
#define CLI_LABEL_LANGUAGE_DEF(X_CLI_LBL_CFG) \
    X_CLI_LBL_CFG( LBL_CLI_NEW_LINE,                    "\n\n>",                                                                                                   nullptr                                                                                  ) \
    X_CLI_LBL_CFG( LBL_NACK,                            "NACK, ",                                                                                                  nullptr                                                                                  ) \
    X_CLI_LBL_CFG( LBL_ERROR,                           "ERROR, %s",                                                                                               "ERREUR, %s"                                                                             ) \
    X_CLI_LBL_CFG( LBL_CMD_LINE_PROC_STARTED,           "Command Line Process Started\n>",                                                                         "Proc\x82" "d\x82 Ligne de Commande, D\x82marr\x82\n>"                                   ) \
    X_CLI_LBL_CFG( LBL_COMMAND_IS_INVALID,              "\nCommand is Invalid\n>",                                                                                 "\nCommande Invalide\n>"                                                                 ) \
    X_CLI_LBL_CFG( LBL_MALFORMED_PACKED,                "\nMalformed Command\n>",                                                                                  "\nCommande Mal Form\x82\n>"                                                             ) \
    X_CLI_LBL_CFG( LBL_BUFFER_OVERFLOW,                 "\nBuffer Overflow\n>",                                                                                    "\nD\x82" "bordement du Tampon\n"                                                        ) \
    X_CLI_LBL_CFG( LBL_CMD_TIME_OUT,                    " ... Command Timeout\n>",                                                                                 " ... Expiration du D\x82lai de Commande\n>"                                             ) \
    X_CLI_LBL_CFG( LBL_CLI_RESPONSE,                    CLI_RESPONSE_FOR_LABEL,                                                                                    nullptr                                                                                  ) \

// Extended message support CLI_USE_EXTENDED_ERROR
#define CLI_LABEL_EXT_LANGUAGE_DEF(X_CLI_LBL_CFG) \
    X_CLI_LBL_CFG( LBL_DENIED,                          "Denied",                                                                                                  "Refuser"                                                                                ) \
    X_CLI_LBL_CFG( LBL_NO_READ_SUPPORT,                 "No Read Support",                                                                                         "Aucun Support de Lecture"                                                               ) \
    X_CLI_LBL_CFG( LBL_NO_WRITE_SUPPORT,                "No Write Support",                                                                                        "Aucun Support d'\x82" "criture"                                                         ) \
    X_CLI_LBL_CFG( LBL_INVALID_PARAMETER,               "Invalid Parameter",                                                                                       "Param\x88tre Invalide"                                                                  ) \
    X_CLI_LBL_CFG( LBL_PASSWORD_INVALID,                "Password Invalid",                                                                                        "Mot de Passe Invalide"                                                                  ) \
    X_CLI_LBL_CFG( LBL_MEM_ALLOC_ERROR,                 "Memory allocation Error",                                                                                 "Erreur d'Allocation M\x82moire"                                                         ) \
    X_CLI_LBL_CFG( LBL_PLAIN_CMD_ONLY,                  "Plain Command Only",                                                                                      "Commande Simple Uniquement"                                                             ) \

#define CLI_LABEL_HELP_DEF(X_CLI_LBL_CFG) \
    X_CLI_LBL_CFG( LBL_MENU_HEADER,                     "\nFor more information on a specific command, type HELP <cmd-name>\n\n",                                  "\nPour plus d'information sur une commande specifique, tapez HELP <Nom-de-CMD>\n\n"     ) \
  IF_USE(DIGINI_USE_VT100_MENU,                                                                                                                                                                                                                               \
    X_CLI_LBL_CFG( LBL_CMD_HELP_MENU,                   "Access the VT100 Menu",                                                                                   "AccD\x8a" "der au menu VT100"                                                           ) \
  )                                                                                                                                                                                                                                                           \
    X_CLI_LBL_CFG( LBL_CMD_HELP_MUTE_DEBUG,             "Mute the debugging output",                                                                               "D\x82" "activer l'affichage du d\x82" "bogage"                                          ) \
    X_CLI_LBL_CFG( LBL_CMD_HELP_UNMUTE_DEBUG,           "Unmute the debugging output",                                                                             "R\x82" "activer l'affichage du d\x82" "bogage"                                          ) \
  IF_USE(DIGINI_USE_DEBUG_IN_CONSOLE,                                                                                                                                                                                                                         \
    X_CLI_LBL_CFG( LBL_CMD_HELP_DEBUG,                  "Display debugging information",                                                                           "Afficher les informations de d\x82" "bogage"                                            ) \
  )                                                                                                                                                                                                                                                           \
    X_CLI_LBL_CFG( LBL_CMD_HELP_HOLD,                   "Hold on the startup sequence",                                                                            "Suspendre la s\x82quence de démarrage"                                                  ) \
    X_CLI_LBL_CFG( LBL_CMD_HELP_RELEASE,                "Release the startup sequence",                                                                            "R\x82sumer la s\x82quence de démarrage"                                                 ) \
    X_CLI_LBL_CFG( LBL_CMD_HELP_VERSION,                "Display version information",                                                                             "Afficher les informations de version"                                                   ) \
    X_CLI_LBL_CFG( LBL_CMD_HELP_THIS_HELP,              "This help",                                                                                               "Aide de cette commande"                                                                 ) \
    X_CLI_LBL_CFG( LBL_CMD_HELP_APP_INFO,               "Display application information",                                                                         "Afficher les informations de cette application"                                         ) \
    X_CLI_LBL_CFG( LBL_CMD_HELP_RESET,                  "Reset the processor",                                                                                     "R\x82initializer le processeur"                                                         ) \
    X_CLI_LBL_CFG( LBL_CMD_HELP_STATUS,                 "Display general status",                                                                                  "Afficher les status courrant"                                                           ) \

#endif // (DIGINI_USE_CMD_LINE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

#define VT100_LBL_BLANK_FOR_SAVE                        LBL_NULL

// Console Generic Label
#define VT100_LABEL_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_HIDE_CURSOR,                "\033[?25l",                                                                                                nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_SHOW_CURSOR,                "\033[?25h",                                                                                                nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_ATTRIBUTE,                  "\033[%dm",                                                                                                 nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_SAVE_CURSOR,                "\033[s",                                                                                                   nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_RESTORE_CURSOR,             "\033[u",                                                                                                   nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_MOVE_LEFT_CURSOR,           "\033[D",                                                                                                   nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_WHITE_MOVE_CURSOR_2_TO_LEFT,"\033[40m \033[2D",                                                                                         nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_SAVE_ATTRIBUTE,             "\0337",                                                                                                    nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_RESTORE_ATTRIBUTE,          "\0338",                                                                                                    nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_SET_CURSOR,                 "\033[%d;%df",                                                                                              nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_CURSOR_TO_SELECT,           "\033[%d;28f    \033[4D",                                                                                   nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_START_PRINTING,             "\033[5i",                                                                                                  nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_STOP_PRINTING,              "\033[4i",                                                                                                  nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_SCROLL_ZONE,                "\033[%d;%dr",                                                                                              nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_SCROLL_UP,                  "\033M",                                                                                                    nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_EOL_ERASE,                  "\033[K",                                                                                                   nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_ERASE_FROM_CURSOR_N_CHAR,   "\033[%dX",                                                                                                 nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_LINE_SEPARATOR,             "----------------------------------------------------------------------------------------------------\n",   nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_SELECT,                     "Please Select:\n",                                                                                         "Faire une S\x82lection:\n"                                                              ) \
    X_VT100_LBL( VT100_LBL_QUIT,                       "Quit\n",                                                                                                   "Quitter\n"                                                                              ) \
    X_VT100_LBL( VT100_LBL_ENTER_SELECTION,            "\r Enter Selection [<ESC>, 1 - %c] > ",                                                                    "\r Entrer S\x82lection [<ESC>, 1 - %c] > "                                              ) \
    X_VT100_LBL( VT100_LBL_ESCAPE,                     "\r\r\r Press <ESC> to return to menu",                                                                     "\r\r\r Presser <ESC> pour retourner au menu"                                            ) \
    X_VT100_LBL( VT100_LBL_INPUT_VALIDATION,           "Press <ENTER> to accept or <ESC> to cancel",                                                               "Presser <ENTER> pour accepter ou <ESC> pour annuler"                                    ) \
    X_VT100_LBL( VT100_LBL_SAVE_CONFIGURATION,         "Save the Configuration",                                                                                   "Sauvegarder la Configuration"                                                           ) \
    X_VT100_LBL( VT100_LBL_MINIMUM,                    "Minimum",                                                                                                  nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_MAXIMUM,                    "Maximum",                                                                                                  nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_STATUS,                     "( )",                                                                                                      nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_NOW,                        "Now:",                                                                                                     "Maintenant:"                                                                            ) \
    X_VT100_LBL( VT100_LBL_UPTIME,                     "Up Time:",                                                                                                 "Temps " "\x82" "coul\x82:"                                                              ) \
    X_VT100_LBL( VT100_LBL_PERCENT_VALUE,              "%ld %%",                                                                                                   nullptr                                                                                  ) \
\
    X_VT100_LBL( VT100_LBL_DATE,                       "Date",                                                                                                     "Date"                                                                                   ) \
    X_VT100_LBL( VT100_LBL_TIME,                       "Time",                                                                                                     "Heure"                                                                                  ) \

#define VT100_MONO_LABEL_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_BACK_WHITE_FORE_BLACK,      "\033[30m\033[47m",                                                                                         nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_BACK_BLACK_FORE_WHITE,      "\033[37m\033[40m",                                                                                         nullptr                                                                                  ) \

#define VT100_DEBUG_IN_CONSOLE_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_DEBUG,                      "Debug",                                                                                                    "D\x82" "bogage"                                                                         ) \
    X_VT100_LBL( VT100_LBL_DEBUG_MENU,                 "Debug Menu",                                                                                               "Menu de D\x82" "bogage"                                                                 ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_1,                    "Dbg level 1  - System Status   ( )",                                                                       "Dbg level 1  - Status Syst\x8ame  ( )"                                                  ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_2,                    "Dbg level 2  - System Action   ( )",                                                                       "Dbg level 2  - Action Syst\x8ame  ( )"                                                  ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_3,                    "Dbg level 3  - System Health   ( )",                                                                       "Dbg level 3  - Sant\x82 Syst\x8ame   ( )"                                               ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_4,                    "Dbg level 4  - Monitor/Logs    ( )",                                                                       "Dbg level 4  - Moniteur/Logs   ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_5,                    "Dbg level 5  - Stack Warning   ( )",                                                                       "Dbg level 5  - Alerte Stack    ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_6,                    "Dbg level 6  - Ethernet driver ( )",                                                                       "Dbg level 6  - Pilote Ethernet ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_7,                    "Dbg level 7  - Nano IP         ( )",                                                                       nullptr                                                                                  ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_8,                    "Dbg level 8  - Memory Pool     ( )",                                                                       "Dbg level 8  - R\x82serve m\x82moire ( )"                                               ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_9,                    "Dbg level 9  - Free            ( )",                                                                       "Dbg level 9  - Libre           ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_10,                   "Dbg level 10 - Free            ( )",                                                                       "Dbg level 10 - Libre           ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_11,                   "Dbg level 11 - Free            ( )",                                                                       "Dbg level 11 - Libre           ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_12,                   "Dbg level 12 - Free            ( )",                                                                       "Dbg level 12 - Libre           ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_13,                   "Dbg level 13 - Free            ( )",                                                                       "Dbg level 13 - Libre           ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_14,                   "Dbg level 14 - Free            ( )",                                                                       "Dbg level 14 - Libre           ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_15,                   "Dbg level 15 - Free            ( )",                                                                       "Dbg level 15 - Libre           ( )"                                                     ) \
    X_VT100_LBL( LBL_DEBUG_LEVEL_16,                   "Dbg level 16 - Free            ( )",                                                                       "Dbg level 16 - Libre           ( )"                                                     ) \

#define VT100_MEMORY_POOL_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_MEMORY_POOL_STAT,           "Memory Pool Statistic",                                                                                    "Statistiques Bassin de m\x82moire"                                                      ) \
    X_VT100_LBL( VT100_LBL_MEMORY_POOL,                "Memory Pool",                                                                                              "Bassin de m\x82moire"                                                                   ) \
    X_VT100_LBL( VT100_LBL_MEMORY_POOL_TOTAL,          "Total size     :",                                                                                         "Taille totale     :"                                                                    ) \
    X_VT100_LBL( VT100_LBL_MEMORY_POOL_USED,           "Used size :",                                                                                              "Taille Utilis\x82" "e :"                                                                ) \
    X_VT100_LBL( VT100_LBL_MEMORY_POOL_NB_OF_POOL,     "Number of Pool :    %u",                                                                                   "Nombre de bassins : %u"                                                                 ) \
    X_VT100_LBL( VT100_LBL_MEM_POOL_GROUP,             "Group %d: %u X %u Bytes",                                                                                  "Groupe %d: %u X %u Octets"                                                              ) \
    X_VT100_LBL( VT100_LBL_MEM_BLOCK_USED,             "Used Blocks : %u  Highest : %u ",                                                                          "Blocks Utilis\x82s: %u  Haut : %u "                                                     ) \

#define VT100_MEMORY_STACKTISTIC_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_STACKTISTIC,                "Stack Usage",                                                                                              "Usage des Stacks"                                                                       ) \

#define VT100_PRODUCT_INFO_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_SYSTEM_INFO,                "Display System Information",                                                                               "Information Syst\x8ame"                                                                 ) \
    X_VT100_LBL( VT100_LBL_FONT_TERMINAL,              "Extended ASCII Font From This Terminal:",                                                                  "Police de Caract\x8are ASCII \x90tendue Provenant de ce Terminal:"                      ) \

#define VT100_SELECT_THE_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_SELECT_LANGUAGE,            "Toggle Language:",                                                                                         "Basculer de Langage:"                                                                   ) \
    X_VT100_LBL( VT100_LBL_LANGUAGE_SELECTION,         "Choose Language:  English ( )  French   ( )",                                                              "Choisir Langage:  Anglais ( )  Fran\x87" "ais ( )"                                      ) \

#define VT100_SELECT_THE_TEMPERATURE_UNIT_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_TEMPERATURE_UNIT_SELECTION, "Choose Temp Unit:      \xF8" "C ( )        \xF8" "F ( )",                                                  "Choisir Unit\x82 Temp:    \xF8" "C ( )        \xF8" "F ( )"                             ) \

#define VT100_SERIAL_NUMBER_SETTING_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_SERIAL_NUMBER_SETTING,      "Setting Serial Number",                                                                                    "Configurer Num\x82ro de S\x82rie"                                                       ) \

#define VT100_TIME_AND_DATE_SETTING_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_TIME_AND_DATE,              "Time and Date",                                                                                            "Heure et Date"                                                                          ) \
    X_VT100_LBL( VT100_LBL_TIME_AND_DATE_CONFIG,       "Time and Date Configuration",                                                                              "Configuration Heure et Date"                                                            ) \

#define VT100_SDCARD_INFO_DEF(X_VT100_LBL) \
    X_VT100_LBL( LBL_SD_CARD_INFORMATION,              "SD-Card Information",                                                                                      "Information Carte SD"                                                                   ) \
    X_VT100_LBL( LBL_SD_CARD_TYPE,                     "Card Type:",                                                                                               "Type de Carte:"                                                                         ) \
    X_VT100_LBL( LBL_SD_SPEC_VER,                      "SD Spec Version:",                                                                                         "Spec Version SD:"                                                                       ) \
    X_VT100_LBL( LBL_SD_MAX_SPEED,                     "Max Speed:",                                                                                               "Vitesse Max:"                                                                           ) \
    X_VT100_LBL( LBL_SD_MANUFACTURER_ID,               "Manufacturer ID:",                                                                                         "ID du Manufacturier:"                                                                   ) \
    X_VT100_LBL( LBL_SD_OEM_ID,                        "OEM ID:",                                                                                                  "ID OEM:"                                                                                ) \
    X_VT100_LBL( LBL_SD_PRODUCT_NAME,                  "Product:",                                                                                                 "Produit:"                                                                               ) \
    X_VT100_LBL( LBL_SD_PRODUCT_REVISION,              "Revision:",                                                                                                "Revision:"                                                                              ) \
    X_VT100_LBL( LBL_SD_SERIAL_NUMBER,                 "Serial Number:",                                                                                           "Num\x82ro S\x82rie:"                                                                    ) \
    X_VT100_LBL( LBL_SD_MANUFACTURING_DATE,            "Manufacturing date:",                                                                                      "Date de Fabrication:"                                                                   ) \
    X_VT100_LBL( LBL_SD_CARD_SIZE,                     "Card Size:",                                                                                               "Taille de la Carte:"                                                                    ) \
    X_VT100_LBL( LBL_SD_FLASH_ERASE_SIZE,              "Flash Erase Size:",                                                                                        "Taille d'effacement du flash:"                                                          ) \
    X_VT100_LBL( LBL_SD_ERASE_SINGLE_BLOCK,            "Erase Single Block:",                                                                                      "effacer un seul bloc:"                                                                  ) \
    X_VT100_LBL( LBL_SD_DATA_AFTER_ERASE,              "Data After Erase:",                                                                                        "Donn\x82" "e Apr\x8as Effacement:"                                                      ) \
    X_VT100_LBL( LBL_SD_VOLUME_NAME,                   "Volume Name:",                                                                                             "Nom du Volume:"                                                                         ) \
    X_VT100_LBL( LBL_SD_VOLUME_SN,                     "Volume Serial Number:",                                                                                    "Num\x82ro de S\x82rie du Volume:"                                                       ) \
    X_VT100_LBL( LBL_SD_VOLUME_TYPE,                   "Volume Type:",                                                                                             "Type du Volume:"                                                                        ) \
    X_VT100_LBL( LBL_SD_CAPACITY,                      "Capacity:",                                                                                                "Capacit\x82:"                                                                           ) \
    X_VT100_LBL( LBL_SD_USED_SPACE,                    "Used Space:",                                                                                              "Espace Utilis\x82:"                                                                     ) \
    X_VT100_LBL( LBL_SD_FREE_SPACE,                    "Free Space:",                                                                                              "Espace Disponible:"                                                                     ) \
    X_VT100_LBL( LBL_SD_CLUSTER_SIZE,                  "Cluster Size:",                                                                                            "Taille d'un Cluster:"                                                                   ) \
    X_VT100_LBL( LBL_SD_SECTOR_PER_CLUSTER,            "Sector Per Cluster:",                                                                                      "Secteurs Par Cluster:"                                                                  ) \
    X_VT100_LBL( LBL_SD_CLUSTER_COUNT,                 "Cluster Count:",                                                                                           "Nombre de Clusters:"                                                                    ) \
    X_VT100_LBL( LBL_SD_FREE_CLUSTER_COUNT,            "Free Cluster Count:",                                                                                      "Nombre de Clusters Disponibles:"                                                        ) \
    X_VT100_LBL( LBL_SD_FAT_START_SECTOR,              "FAT Start Sector:",                                                                                        "Secteur d\x82" "but de la FAT:"                                                         ) \
    X_VT100_LBL( LBL_SD_DATA_START_SECTOR,             "Data Start Sector:",                                                                                       "Secteur d\x82" "but des donn\x82" "es:"                                                 ) \
    X_VT100_LBL( LBL_FAT_INFORMATION,                  "FAT Information",                                                                                          "Information sur la FAT"                                                                 ) \
    X_VT100_LBL( LBL_SD_STD_CAPACITY_V1_1,             "Standard Capacity V1.1",                                                                                   "Capacit\x82 Standard V1.1"                                                              ) \
    X_VT100_LBL( LBL_SD_STD_CAPACITY_V2_0,             "Standard Capacity V2.0",                                                                                   "Capacit\x82 Standard V2.0"                                                              ) \
    X_VT100_LBL( LBL_SD_HIGH_CAPACITY,                 "High Capacity",                                                                                            "Haute Capacit\x82"                                                                      ) \
    X_VT100_LBL( LBL_SD_UNDEFINED_D,                   "Undefined %d",                                                                                             "Non d\x82" "fini %d"                                                                    ) \
    X_VT100_LBL( LBL_SD_UNDEFINED_2X,                  "Undefined %2x",                                                                                            "Non d\x82" "fini %2x"                                                                   ) \
    X_VT100_LBL( LBL_SD_25MHZ,                         "25 MHz",                                                                                                   nullptr                                                                                  ) \
    X_VT100_LBL( LBL_SD_50MHZ,                         "50 MHz",                                                                                                   nullptr                                                                                  ) \
    X_VT100_LBL( LBL_SD_100MBITS,                      "100 Mbits/sec",                                                                                            nullptr                                                                                  ) \
    X_VT100_LBL( LBL_SD_200MBITS,                      "200 Mbits/sec",                                                                                            nullptr                                                                                  ) \
    X_VT100_LBL( LBL_SD_EMPTY,                         "<EMPTY>",                                                                                                  "<VIDE>"                                                                                 ) \
    X_VT100_LBL( LBL_SD_KB_TOTAL,                      "%lu KB Total",                                                                                             "%lu KB Totale"                                                                          ) \
    X_VT100_LBL( LBL_SD_KB_USED,                       "%lu KB Used",                                                                                              "%lu KB Utilis\x82"                                                                      ) \
    X_VT100_LBL( LBL_SD_KB_AVAILABLE,                  "%lu KB Available",                                                                                         "%lu KB Disponible"                                                                      ) \
    X_VT100_LBL( LBL_SD_SECTORS,                       "%lu Sectors",                                                                                              "%lu Secteurs"                                                                           ) \

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//  Typedef(s)
//-------------------------------------------------------------------------------------------------

enum Language_e
{
    // TODO: include here a .cfg file to include any language a user want

    LANG_ENGLISH = 0,
    LANG_FRENCH,
    NB_LANGUAGE_CONST,
    LANG_DEFAULT = LANG_ENGLISH,           // If no support for multi-language this will be the default value
};

enum Label_e
{
    COMMON_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
    LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)

 #if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)                       /// this doesn't exist actually need to do
    PRODUCT_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
 #endif

 #if (LABEL_USE_DATE == DEF_ENABLED)
    DATE_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
 #endif

 #if (LABEL_USE_TIME == DEF_ENABLED)
    TIME_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
 #endif

 #if (LABEL_USE_DECIMAL_KEY == DEF_ENABLED)
    DECIMAL_KEY_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
 #endif

 #if (LABEL_USE_HEXADECIMAL_KEY == DEF_ENABLED)
    HEXADECIMAL_KEY_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
 #endif

 #if (LABEL_USE_STANDARD_TEST_STRING == DEF_ENABLED)
    STANDARD_TEST_STRING_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
 #endif

  ///-----------------------------------------------------------------------------------------------
  /// Command line interface label

 #if (DIGINI_USE_CMD_LINE == DEF_ENABLED)

    CLI_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)

  #if (CLI_USE_EXTENDED_ERROR == DEF_ENABLED)
    CLI_LABEL_EXT_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (DIGINI_USE_HELP_IN_CONSOLE == DEF_ENABLED)
    CLI_LABEL_HELP_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #ifdef CLI_USER_LABEL_HELP_DEF
    CLI_USER_LABEL_HELP_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #endif
  #endif
 #endif

  ///-----------------------------------------------------------------------------------------------
  /// Include VT100 label if VT100 is defined

 #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

    VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
    VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)

  #if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)
    VT100_PRODUCT_INFO_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
    VT100_MEMORY_STACKTISTIC_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (DIGINI_USE_STATIC_MEMORY_ALLOC == DEF_ENABLED)
    VT100_MEMORY_POOL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (DIGINI_USE_DEBUG_IN_CONSOLE == DEF_ENABLED)
    VT100_DEBUG_IN_CONSOLE_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (DIGINI_DEBUG_SDCARD_INFO_ON_VT100 == DEF_ENABLED)
      VT100_SDCARD_INFO_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (VT100_USE_COLOR != DEF_ENABLED)
    VT100_MONO_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (VT100_USE_LANGUAGE_SELECTION == DEF_ENABLED)
    VT100_SELECT_THE_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (VT100_USE_TEMPERATURE_SELECTION == DEF_ENABLED)
    VT100_SELECT_THE_TEMPERATURE_UNIT_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (VT100_USE_SERIAL_NUMBER_SETTING == DEF_ENABLED)
    VT100_SERIAL_NUMBER_SETTING_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (VT100_USE_TIME_AND_DATE_SETTING == DEF_ENABLED)
    VT100_TIME_AND_DATE_SETTING_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

 #endif

  ///-----------------------------------------------------------------------------------------------
  /// Ethernet Label

 #if (DIGINI_USE_ETHERNET == DEF_ENABLED)
    NET_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
 #endif

    NB_LABEL_CONST,
    INVALID_LABEL,
};

//-------------------------------------------------------------------------------------------------
// class
//-------------------------------------------------------------------------------------------------

class Label
{
    public:

                            Label                       ();
        Language_e          GetLanguage                 (void);
        void                SetLanguage                 (Language_e Language);
        const char*         GetPointer                  (Label_e Label_ID);

    private:

        static const char*  LabelArray[NB_LABEL_CONST][NB_LANGUAGE_CONST];
        Language_e          m_ActualLanguage;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_GLOBAL

  Label myLabel;

#else

  extern Label myLabel;

#endif // DIGINI_GLOBAL

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_LABEL == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
