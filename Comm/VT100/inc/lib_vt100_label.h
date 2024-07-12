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
// Define(s)
//-------------------------------------------------------------------------------------------------

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
    X_VT100_LBL( VT100_LBL_MAIN_MENU,                  "Main Menu",                                                                                                "Menu Principal"                                                                         ) \
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

#define VT100_MONO_LABEL_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_BACK_WHITE_FORE_BLACK,      "\033[30m\033[47m",                                                                                         nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_BACK_BLACK_FORE_WHITE,      "\033[37m\033[40m",                                                                                         nullptr                                                                                  ) \

#define VT100_DEBUG_IN_CONSOLE_LANGUAGE_DEF(X_VT100_LBL) \
    X_VT100_LBL( VT100_LBL_DEBUG,                      "Debug",                                                                                                    "D\x82" "bogage"                                                                         ) \
    X_VT100_LBL( VT100_LBL_DEBUG_MENU,                 "Debug Menu",                                                                                               "Menu de D\x82" "bogage"                                                                 ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_1,              "Dbg level 1  - System Status   ( )",                                                                       "Dbg level 1  - Status Syst\x8ame  ( )"                                                  ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_2,              "Dbg level 2  - System Action   ( )",                                                                       "Dbg level 2  - Action Syst\x8ame  ( )"                                                  ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_3,              "Dbg level 3  - System Health   ( )",                                                                       "Dbg level 3  - Sant\x82 Syst\x8ame   ( )"                                               ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_4,              "Dbg level 4  - Monitor/Logs    ( )",                                                                       "Dbg level 4  - Moniteur/Logs   ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_5,              "Dbg level 5  - Stack Warning   ( )",                                                                       "Dbg level 5  - Alerte Stack    ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_6,              "Dbg level 6  - Ethernet driver ( )",                                                                       "Dbg level 6  - Pilote Ethernet ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_7,              "Dbg level 7  - Nano IP         ( )",                                                                       nullptr                                                                                  ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_8,              "Dbg level 8  - Memory Pool     ( )",                                                                       "Dbg level 8  - R\x82serve m\x82moire ( )"                                               ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_9,              "Dbg level 9  - Free            ( )",                                                                       "Dbg level 9  - Libre           ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_10,             "Dbg level 10 - Free            ( )",                                                                       "Dbg level 10 - Libre           ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_11,             "Dbg level 11 - Free            ( )",                                                                       "Dbg level 11 - Libre           ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_12,             "Dbg level 12 - Free            ( )",                                                                       "Dbg level 12 - Libre           ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_13,             "Dbg level 13 - Free            ( )",                                                                       "Dbg level 13 - Libre           ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_14,             "Dbg level 14 - Free            ( )",                                                                       "Dbg level 14 - Libre           ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_15,             "Dbg level 15 - Free            ( )",                                                                       "Dbg level 15 - Libre           ( )"                                                     ) \
    X_VT100_LBL( VT100_LBL_DEBUG_LEVEL_16,             "Dbg level 16 - Free            ( )",                                                                       "Dbg level 16 - Libre           ( )"                                                     ) \

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

#define VT100_SDCARD_INFO_DEF(X_VT100_LBL) \
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

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

