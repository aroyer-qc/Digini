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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
#include "lib_vt100_label.h"
#endif

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_LBL_CFG_AS_ENUM(ENUM_ID, LBL1, LBL2) ENUM_ID,
#define EXPAND_X_LBL_CFG_AS_DATA(ENUM_ID, LBL1, LBL2) {LBL1, LBL2},

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
    LBL_STRING,
    LBL_CHAR,
    LBL_LINEFEED,
    LBL_DOUBLE_LINEFEED,

  #if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)
    LBL_VENDOR_NAME_INFO,
    LBL_FW_NAME_INFO,
    LBL_FW_VERSION_INFO,
    LBL_GUI_NAME_INFO,
    LBL_GUI_VERSION_INFO,
    LBL_HARDWARE_INFO,
    LBL_SERIAL_INFO,
    LBL_COMPILE_DATE_INFO,

    LBL_VENDOR_NAME,
    LBL_MODEL_NAME,
    LBL_FIRMWARE_NAME,
    LBL_FIRMWARE_VERSION,
    LBL_GUI_NAME,
    LBL_GUI_VERSION,
    LBL_SERIAL_NUMBER,
    LBL_BUILT_DATE,
  #endif

  #if (LABEL_USE_TIME_AND_DATE == DEF_ENABLED)
    LBL_FIRST_MONTH,
    LBL_JANUARY         = LBL_FIRST_MONTH,
    LBL_FEBRUARY,
    LBL_MARCH,
    LBL_APRIL,
    LBL_MAY,
    LBL_JUNE,
    LBL_JULY,
    LBL_AUGUST,
    LBL_SEPTEMBER,
    LBL_OCTOBER,
    LBL_NOVEMBER,
    LBL_DECEMBER,
    LBL_SUNDAY,
    LBL_MONDAY,
    LBL_TUESDAY,
    LBL_WEDNESDAY,
    LBL_THURSDAY,
    LBL_FRIDAY,
    LBL_SATURDAY,
    LBL_HOUR,
    LBL_MINUTE,
    LBL_SECOND,
    LBL_YEAR,
    LBL_MONTH,
    LBL_DAY,
  #endif

    LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)

    // Include VT100 label if VT100 is defined
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
    VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #if (VT100_USE_COLOR != DEF_ENABLED)
    VT100_MONO_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #endif
  #endif

    NB_LABEL_CONST,
    INVALID_LABEL,
};

//-------------------------------------------------------------------------------------------------
// Function(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

class Label
{
    public:

                            Label                       ()                              { ActualLanguage = LANG_ENGLISH;               }
        Language_e          GetLanguage                 (void)                          { return ActualLanguage;                       }
        void                SetLanguage                 (Language_e Language)           { ActualLanguage = Language;                   }
        const char*         GetPointer                  (Label_e Label_ID)              { return LabelArray[Label_ID][ActualLanguage]; }

    private:

        static const char*  LabelArray[NB_LABEL_CONST][NB_LANGUAGE_CONST];
        Language_e          ActualLanguage;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef DIGINI_GLOBAL

  const char* Label::LabelArray[NB_LABEL_CONST][NB_LANGUAGE_CONST] =
  {
      { "%s",                           nullptr            },  // LBL_STRING
      { "%c",                           nullptr            },  // LBL_CHAR
      { "\r",                           nullptr            },  // LBL_LINEFEED
      { "\r\r",                         nullptr            },  // LBL_DOUBLE_LINEFEED

    #if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)
      { "Vendor:       ",               "Vendeur:        " },  // LBL_FW_NAME_INFO
      { "Name:         ",               "Nom:            " },  // LBL_FW_NAME_INFO
      { "Version:      ",               nullptr            },  // LBL_FW_VERSION_INFO
      { "GUI Name:     ",               "Nom GUI:        " },  // LBL_GUI_NAME_INFO
      { "GUI Version:  ",               "Version GUI:    " },  // LBL_GUI_VERSION_INFO
      { "Hardware:     ",               "Matériel:       " },  // LBL_HARDWARE_INFO
      { "Serial No.:   ",               "No. Série:      " },  // BL_SERIAL_INFO
      { "Built Date:   ",               "Date logiciel:  " },  // LBL_COMPILE_DATE_INFO

      { OUR_VENDOR_NAME,                nullptr            },  // LBL_VENDOR_NAME
      { OUR_MODEL_NAME,                 nullptr            },  // LBL_MODEL_NAME
      { OUR_FIRMWARE_NAME,              nullptr            },  // LBL_FIRMWARE_NAME
      { OUR_FIRMWARE_VERSION,           nullptr            },  // LBL_FIRMWARE_VERSION
      { OUR_FIRMWARE_GUI_NAME,          nullptr            },  // LBL_GUI_NAME
      { OUR_FIRMWARE_GUI_VERSION,       nullptr            },  // LBL_GUI_VERSION
      { OUR_SERIAL_NUMBER,              nullptr            },  // LBL_SERIAL_NUMBER
      { OUR_BUILD_DATE/*[]*/,           nullptr            },  // LBL_BUILT_DATE
    #endif

    #if (LABEL_USE_TIME_AND_DATE == DEF_ENABLED)
      { "January",                      "Janvier"          },
      { "February",                     "Février"          },
      { "March",                        "Mars"             },
      { "April",                        "Avril"            },
      { "May",                          "Mai"              },
      { "June",                         "Juin"             },
      { "July",                         "Juillet"          },
      { "August",                       "Août"             },
      { "September",                    "Septembre  "      },
      { "October",                      "Octobre"          },
      { "November",                     "Novembre"         },
      { "December",                     "Décembre"         },
      { "Sunday",                       "Dimanche"         },
      { "Monday",                       "Lundi"            },
      { "Tuesday",                      "Mardi"            },
      { "Wednesday",                    "Mercredi"         },
      { "Thursday",                     "Jeudi"            },
      { "Friday",                       "Vendredi"         },
      { "Saturday",                     "Samedi"           },
      { "Hour",                         "Heure"            },
      { "Minute",                       nullptr            },
      { "Second",                       "Seconde"          },
      { "Year",                         "Année"            },
      { "Month",                        "Mois"             },
      { "Day",                          "Jour"             },
    #endif

      LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)

      // Include VT100 label if VT100 is defined
    #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
      VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
      VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
     #if (VT100_USE_COLOR != DEF_ENABLED)
      VT100_MONO_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
     #endif
  #endif
  };

  Label myLabel;

#else

  extern Label myLabel;

#endif // DIGINI_GLOBAL


//-------------------------------------------------------------------------------------------------
