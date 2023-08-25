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
    LBL_STRING_LINEFEED,
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
    LBL_FIRST_WEEK_DAY,
    LBL_SUNDAY         = LBL_FIRST_WEEK_DAY,
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
