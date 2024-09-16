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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------
// class
//-------------------------------------------------------------------------------------------------

const char* Label::LabelArray[NB_LABEL_CONST][NB_LANGUAGE_CONST] =
{
    COMMON_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
    LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)

  #if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)
    PRODUCT_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
  #endif

  #if (LABEL_USE_DATE == DEF_ENABLED)
    DATE_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
  #endif

  #if (LABEL_USE_TIME == DEF_ENABLED)
    TIME_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
  #endif

  #if (LABEL_USE_DECIMAL_KEY == DEF_ENABLED)
    DECIMAL_KEY_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
  #endif
      
  #if (LABEL_USE_HEXADECIMAL_KEY == DEF_ENABLED)
    HEXADECIMAL_KEY_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
  #endif

  #if (LABEL_USE_STANDARD_TEST_STRING == DEF_ENABLED)
    STANDARD_TEST_STRING_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
  #endif

  ///-----------------------------------------------------------------------------------------------
  /// Command line interface label

  #if (DIGINI_USE_CMD_LINE == DEF_ENABLED)
    CLI_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)

   #if (CLI_USE_EXTENDED_ERROR == DEF_ENABLED)
    CLI_LABEL_EXT_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

  #endif

  ///-----------------------------------------------------------------------------------------------
  /// Include VT100 label if VT100 is defined

  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

    VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
    VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)

   #if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)                  // todo change LABEL.. for something better...
    VT100_PRODUCT_INFO_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
    VT100_MEMORY_STACKTISTIC_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (DIGINI_USE_STATIC_MEMORY_ALLOC == DEF_ENABLED)
    VT100_MEMORY_POOL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (DIGINI_USE_DEBUG_IN_CONSOLE == DEF_ENABLED)
    VT100_DEBUG_IN_CONSOLE_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (DIGINI_DEBUG_SDCARD_INFO_ON_VT100 == DEF_ENABLED)
    VT100_SDCARD_INFO_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (VT100_USE_COLOR != DEF_ENABLED)
    VT100_MONO_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif
   
   #if (VT100_USE_LANGUAGE_SELECTION == DEF_ENABLED)
    VT100_SELECT_THE_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (VT100_USE_TEMPERATURE_SELECTION == DEF_ENABLED)
    VT100_SELECT_THE_TEMPERATURE_UNIT_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (VT100_USE_SERIAL_NUMBER_SETTING == DEF_ENABLED)
    VT100_SERIAL_NUMBER_SETTING_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif

   #if (VT100_USE_TIME_AND_DATE_SETTING == DEF_ENABLED)
    VT100_TIME_AND_DATE_SETTING_DEF(EXPAND_X_LBL_CFG_AS_DATA)
   #endif


  #endif

  ///-----------------------------------------------------------------------------------------------
  /// Ethernet Label

  #if (DIGINI_USE_ETHERNET == DEF_ENABLED)
    NET_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_DATA)
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
