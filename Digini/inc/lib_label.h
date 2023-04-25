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
    LANG_DEFAULT = 0,           // If no support for multi-language this will be the default value
};

enum Label_e
{
    LBL_STRING,
    LBL_CHAR,
    LBL_DOUBLE_LINEFEED,

    LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)

    // Include VT100 label if VT100 is defined
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
    VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #if (DIGINI_VT100_USE_COLOR != DEF_ENABLED)
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
        const char*         GetLabelPointer             (Label_e Label_ID)              { return LabelArray[Label_ID][ActualLanguage]; }

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
      { "%s",                           nullptr     },  // LBL_STRING
      { "%c",                           nullptr     },  // LBL_CHAR
      { "\r\n\r\n",                     nullptr     },  // LBL_DOUBLE_LINEFEED

      LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)

      // Include VT100 label if VT100 is defined
    #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
      VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
      VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
     #if (DIGINI_VT100_USE_COLOR != DEF_ENABLED)
      VT100_MONO_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
     #endif
  #endif
  };

  Label myLabel;

#else

  extern Label myLabel;

#endif // DIGINI_GLOBAL


//-------------------------------------------------------------------------------------------------
