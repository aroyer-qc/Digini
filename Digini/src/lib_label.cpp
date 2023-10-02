//-------------------------------------------------------------------------------------------------
//
//  File : lib_label.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
// class
//-------------------------------------------------------------------------------------------------

const char* Label::LabelArray[NB_LABEL_CONST][NB_LANGUAGE_CONST] =
{
    { "%s",                             nullptr                                         },  // LBL_STRING
    { "%s\n",                           nullptr                                         },  // LBL_STRING_LINEFEED
    { "%c",                             nullptr                                         },  // LBL_CHAR
    { "\n",                             nullptr                                         },  // LBL_LINEFEED
    { "\n\n",                           nullptr                                         },  // LBL_DOUBLE_LINEFEED
    { "%2u:%02u:%02u ",                 nullptr                                         },  // LBL_TIME
    { "%s %u, %u ",                     nullptr                                         },  // LBL_DATE
    { "%04u-%02u-%02u %2u:%02u:%02u: ", nullptr                                         },  // LBL_TIME_DATE_STAMP
    { "%ld.%d ",                        nullptr                                         },  // LBL_INT_TO_DIVIDE_BY_10
    { "%ld.%02d ",                      nullptr                                         },  // LBL_INT_TO_DIVIDE_BY_100
    { "%ld.%03d ",                      nullptr                                         },  // LBL_INT_TO_DIVIDE_BY_1000
    { "%ld ",                           nullptr                                         },  // LBL_INT_NO_DIVIDE
    { "%lu ",                           nullptr                                         },  // LBL_LONG_UNSIGNED
    { "%lu: ",                          nullptr                                         },  // LBL_LONG_UNSIGNED_SEMICOLON
    { "%02u:",                          nullptr                                         },  // LBL_UNSIGNED_2_DIGIT_SEMICOLON
    { "%02u",                           nullptr                                         },  // LBL_UNSIGNED_2_DIGIT
    { "%s ",                            nullptr                                         },  // LBL_STRING_AND_ONE_SPACE
    { "%lu Bytes  ",                    "%lu Octets  "                                  },  // LBL_SIZE_BYTES
    { "%s, %s %u, %u, %u:%02u:%02u ",   "\x1A$0%s $2%u $1%s $3%u, $4%u:$5%02u:$6%02u "  },  // LBL_FULL_DATE



#if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)
    { "Vendor:           ",           "Vendeur:          "   },  // LBL_FW_NAME_INFO
    { "Name:             ",           "Nom:              "   },  // LBL_FW_NAME_INFO
    { "Version:          ",           nullptr                },  // LBL_FW_VERSION_INFO
    { "GUI Name:         ",           "Nom GUI:          "   },  // LBL_GUI_NAME_INFO
    { "GUI Version:      ",           "Version GUI:      "   },  // LBL_GUI_VERSION_INFO
    { "Hardware:         ",           "Mat\x82riel:         "},  // LBL_HARDWARE_INFO
    { "Serial No.:       ",           "No. S\x82rie:        "},  // BL_SERIAL_INFO
    { "Built Date:       ",           "Date logiciel:    "   },  // LBL_COMPILE_DATE_INFO

    { OEM_VENDOR_NAME,                nullptr                },  // LBL_VENDOR_NAME
    { OEM_MODEL_NAME,                 nullptr                },  // LBL_MODEL_NAME
    { OUR_FIRMWARE_NAME,              nullptr                },  // LBL_FIRMWARE_NAME
    { OUR_FIRMWARE_VERSION,           nullptr                },  // LBL_FIRMWARE_VERSION
    { OUR_FIRMWARE_GUI_NAME,          nullptr                },  // LBL_GUI_NAME
    { OUR_FIRMWARE_GUI_VERSION,       nullptr                },  // LBL_GUI_VERSION
    { OEM_SERIAL_NUMBER,              nullptr                },  // LBL_SERIAL_NUMBER
    { OUR_BUILD_DATE/*[]*/,           nullptr                },  // LBL_BUILT_DATE
#endif

#if (LABEL_USE_TIME_AND_DATE == DEF_ENABLED)
    { "January",                      "Janvier"              },
    { "February",                     "F\x82vrier"           },
    { "March",                        "Mars"                 },
    { "April",                        "Avril"                },
    { "May",                          "Mai"                  },
    { "June",                         "Juin"                 },
    { "July",                         "Juillet"              },
    { "August",                       "Ao\x93t"              },
    { "September",                    "Septembre"            },
    { "October",                      "Octobre"              },
    { "November",                     "Novembre"             },
    { "December",                     "D\x82" "cembre"       },
    { "Sunday",                       "Dimanche"             },
    { "Monday",                       "Lundi"                },
    { "Tuesday",                      "Mardi"                },
    { "Wednesday",                    "Mercredi"             },
    { "Thursday",                     "Jeudi"                },
    { "Friday",                       "Vendredi"             },
    { "Saturday",                     "Samedi"               },
    { "Hour",                         "Heure"                },
    { "Minute",                       nullptr                },
    { "Second",                       "Seconde"              },
    { "Year",                         "Ann\x82" "e"          },
    { "Month",                        "Mois"                 },
    { "Day",                          "Jour"                 },
#endif

    LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)

    // Include VT100 label if VT100 is defined
#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
    VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #if (DIGINI_DEBUG_SDCARD_INFO_ON_VT100 == DEF_ENABLED)
      VT100_SDCARD_INFO_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif
   #if (VT100_USE_COLOR != DEF_ENABLED)
    VT100_MONO_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif
#endif
};

Label::Label()
{
    m_ActualLanguage = LANG_ENGLISH;
}

Language_e Label::GetLanguage(void)
{
    return m_ActualLanguage;
}

void Label::SetLanguage(Language_e Language)
{
    m_ActualLanguage = Language;
}

const char* Label::GetPointer(Label_e Label_ID)
{
    Language_e Language = LANG_ENGLISH;

    if(LabelArray[Label_ID][m_ActualLanguage] != nullptr)
    {
        Language = m_ActualLanguage;
    }

    return LabelArray[Label_ID][Language];
}

//-------------------------------------------------------------------------------------------------
