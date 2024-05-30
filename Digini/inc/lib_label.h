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

#if (DIGINI_USE_CMD_LINE == DEF_ENABLED)
#include "./Comm/CmdLine/inc/lib_cli_label.h"
#endif

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
#include "./Comm/VT100/inc/lib_vt100_label.h"
#endif

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_LBL_CFG_AS_ENUM(ENUM_ID, LBL1, LBL2) ENUM_ID,
#define EXPAND_X_LBL_CFG_AS_DATA(ENUM_ID, LBL1, LBL2) {LBL1, LBL2},

//-------------------------------------------------------------------------------------------------
// X macro(s)
//-------------------------------------------------------------------------------------------------

#define COMMON_LANGUAGE_DEF(X_LABEL)  /* Common label */ \
X_LABEL( LBL_NULL,                       "",                               nullptr                                         ) \
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


#define NET_LANGUAGE_DEF(X_LABEL)  /* Network label */ \
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

#define TIMDAT_LABEL_LANGUAGE_DEF(X_LABEL) \
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
X_LABEL( LBL_CPU_TEMPERATURE,            "CPU Temp:   \xF8" "C",           nullptr                                         ) \

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

  #if (LABEL_USE_PRODUCT_INFO == DEF_ENABLED)
    PRODUCT_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

  #if (LABEL_USE_TIME_AND_DATE == DEF_ENABLED)
    TIMDAT_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
  #endif

    LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)

  #if (DIGINI_USE_CMD_LINE == DEF_ENABLED)
    CLI_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #if (CLI_USE_EXTENDED_ERROR == DEF_ENABLED)
    CLI_LABEL_EXT_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #endif
  #endif

    // Include VT100 label if VT100 is defined
  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    VT100_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
    VT100_USER_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #if (DIGINI_DEBUG_SDCARD_INFO_ON_VT100 == DEF_ENABLED)
      VT100_SDCARD_INFO_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #endif
   #if (VT100_USE_COLOR != DEF_ENABLED)
    VT100_MONO_LABEL_LANGUAGE_DEF(EXPAND_X_LBL_CFG_AS_ENUM)
   #endif
  #endif

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
