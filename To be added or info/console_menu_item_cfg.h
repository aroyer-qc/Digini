//-------------------------------------------------------------------------------------------------
//
//  File : console_menu_item_cfg.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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
#define ABS(X)                          (((X) < 0) ? (-X) : (X))
#define STRINGIFY(X)                    STRING(X)
#define STRING(X)                       #X

// Defines used for macro unrolling
// These defines are also used to insert "logic" to emulate if-else statements
#define PRIMITIVE_CAT(a, ...)           a ## __VA_ARGS__
#define CAT(a, ...)                     PRIMITIVE_CAT(a, __VA_ARGS__)
#define DEC(x)                          PRIMITIVE_CAT(DEC_, x)

#define IIF(c)                          PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...)                   __VA_ARGS__
#define IIF_1(t, ...)                   t
#define COMPL(b)                        PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0                         1
#define COMPL_1                         0

#define BITAND(x)                       PRIMITIVE_CAT(BITAND_, x)
#define BITAND_0(y)                     0
#define BITAND_1(y)                     y


#define CHECK_N(x, n, ...)              n
#define CHECK(...)                      CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x)                        x, 1,
#define IS_PAREN(x)                     CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...)             PROBE(~)
#define NOT(x)                          CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0                           PROBE(~)
#define BOOL(x)                         COMPL(NOT(x))
#define IF(c)                           IIF(BOOL(c))
#define EAT(...)
#define EXPAND(...)                     __VA_ARGS__
#define WHEN(c)                         IF(c)(EXPAND, EAT)
#define IF_THEN(c, x, y)                IF(c)(x, y)

#define PRIMITIVE_COMPARE(x, y)         IS_PAREN                                \
                                        (                                       \
                                            COMPARE_ ## x ( COMPARE_ ## y) (()) \
                                        )
                                        
#define IS_COMPARABLE(...)              IS_PAREN(PRIMITIVE_CAT(COMPARE_, __VA_ARGS__)(()))

#define NOT_EQUAL(x, y)                 IIF(BITAND(IS_COMPARABLE(x))(IS_COMPARABLE(y)) ) \
                                        (                                                \
                                            PRIMITIVE_COMPARE,                           \
                                            1 EAT                                          \
                                        )(x, y)

#define EQUAL(x, y)                     COMPL(NOT_EQUAL(x, y))

#define COMPARE_nullptr(x) 		    x


// Define the compare name (I did not find a way to get rid of those definition by the use of advanced macro
#define COMPARE_MenuRedirection(x)  x
#define COMPARE_MenuMain(x) 		x
#define COMPARE_MenuBoot(x)    		x
#define COMPARE_MenuDebug(x)    	x
#define COMPARE_MenuTest(x)     	x
#define COMPARE_MenuSetting(x)		x
#define COMPARE_MenuSettingTime(x)  x
#define COMPARE_MenuSetMisc(x) 		x

// Define here all the menu and sub menu you will use in the VT100 terminal
#define VT100_MENU_DEF(ENTRY)  \
    ENTRY(MenuRedirection    ) \
    ENTRY(MenuMain           ) \
    ENTRY(MenuBoot           ) \
    ENTRY(MenuDebug          ) \
    ENTRY(MenuTest           ) \
    ENTRY(MenuSetting        ) \
    ENTRY(MenuSettingTime    ) \
    ENTRY(MenuSetMisc        ) \

// Define here all the callback you will use in the VT100 terminal
#define VT100_CALLBACK(ENTRY)           \
    ENTRY(CALLBACK_MenuRedirection    ) \
    ENTRY(CALLBACK_ProductInformation ) \
    ENTRY(CALLBACK_StackUsage         ) \
    ENTRY(CALLBACK_DebugLevelSetting  ) \
    ENTRY(CALLBACK_TimeDateCfg        ) \
    ENTRY(CALLBACK_MiscCfg            ) \



// Here you define how you will navigate in the menu tree and action associated with them                               1st item of a menu
//              Menu   Member Of                Item ID                   CALLBACK for refresh                     Navigate to this menu on <ESC>          Label ID
//                                                                        on first if nullptr there is no
//                                                                        refresh all other. Navigate tothis menu
//                                                                        All other items are items processing
//                                                                        services
#define VT100_MENU_TREE_DEF(ENTRY, MENU) \
\
        ENTRY  (MENU,  MenuRedirection,   ID_REDIRECTION,          CALLBACK_MenuRedirection,                MenuEscape,                        INVALID_LABEL                                   ) \
\
        ENTRY  (MENU,  MenuMain,          ID_MAIN_TITLE,           nullptr,                                 MenuRedirection,                  LBL_VT100_MAIN_MENU                             ) \
        ENTRY  (MENU,  MenuMain,          ID_INFO_DISPLAY,         CALLBACK_ProductInformation,             MenuMain,                         LBL_VT100_SYSTEM_INFO                           ) \
        ENTRY  (MENU,  MenuMain,          ID_DEBUG_MENU,           nullptr,                                 MenuDebug,                        LBL_VT100_DEBUG                                 ) \
        ENTRY  (MENU,  MenuMain,          ID_SETTING_MENU,         nullptr,                                 MenuSetting,                      LBL_VT100_SYSTEM_SETTING                        ) \
        ENTRY  (MENU,  MenuMain,          ID_TEST_MENU,            nullptr,                                 MenuTest1,                        LBL_VT100_TEST                                  ) \
        ENTRY  (MENU,  MenuMain,          ID_STACK_DISPLAY,        CALLBACK_StackUsage,                     MenuRedirection,                  LBL_VT100_STACK_USAGE                           ) \
\
		ENTRY  (MENU,  MenuBoot,          ID_MAIN_TITLE,           nullptr,                                 MenuRedirection,                  LBL_VT100_MAIN_MENU                             ) \
        ENTRY  (MENU,  MenuBoot,          ID_INFO_DISPLAY,         CALLBACK_ProductInformation,             MenuMain,                         LBL_VT100_SYSTEM_INFO                           ) \
        ENTRY  (MENU,  MenuBoot,          ID_DEBUG_MENU,           nullptr,                                 MenuDebug,                        LBL_VT100_DEBUG                                 ) \
        ENTRY  (MENU,  MenuBoot,          ID_SETTING_MENU,         nullptr,                                 MenuSetting,                      LBL_VT100_SYSTEM_SETTING                        ) \
        ENTRY  (MENU,  MenuBoot,          ID_TEST_MENU,            nullptr,                                 MenuTest1,                        LBL_VT100_TEST                                  ) \
        ENTRY  (MENU,  MenuBoot,          ID_STACK_DISPLAY,        CALLBACK_StackUsage,                     MenuRedirection,                  LBL_VT100_STACK_USAGE                           ) \
\
        ENTRY  (MENU,  MenuDebug,         ID_DEBUG_TITLE,          nullptr,                                 MenuRedirection,                  LBL_VT100_DEBUG_MENU                            ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_1,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_1                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_2,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_2                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_3,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_3                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_4,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_4                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_5,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_5                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_6,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_6                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_7,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_7                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_8,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_8                         ) \
        ENTRY  (MENU,  MenuDebug,         ID_DBG_LVL_9,            CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_VT100_DEBUG_LEVEL_9                         ) \
\
        ENTRY  (MENU,  MenuSetting,       ID_SETTING_TITLE,        nullptr,                                 MenuRedirection,                  LBL_VT100_SYSTEM_SETTING_MENU                   ) \
        ENTRY  (MENU,  MenuSetting,       ID_MISC_MENU,            nullptr,                                 MenuSetMisc,                      LBL_VT100_MISCELLEANEOUS                        ) \
        ENTRY  (MENU,  MenuSetting,       ID_TIME_MENU,            nullptr,                                 MenuSettingTime,                  LBL_VT100_TIME_AND_DATE                         ) \
\
        ENTRY  (MENU,  MenuTest,          ID_DBG_STACK,            CALLBACK_DebugStack,                     MenuTest,                         LBL_VT100_STACKTISTIC                           ) \
        ENTRY  (MENU,  MenuTest,          ID_DBG_LVL_10,           nullptr,                                 MenuRedirection,                  LBL_VT100_TEST_1                                ) \
        ENTRY  (MENU,  MenuTest,          ID_DBG_LVL_10,           nullptr,                                 MenuTest,                         LBL_VT100_POUTINE_1                             ) \
        ENTRY  (MENU,  MenuTest,          ID_DBG_LVL_11,           nullptr,                                 MenuTest,                         LBL_VT100_POUTINE_2                             ) \
\
		ENTRY  (MENU,  MenuSettingTime,   ID_TIME_DATE_TITLE,      nullptr,                                 MenuSetting,                      LBL_VT100_TIME_AND_DATE_CONFIGURATION           ) \
        ENTRY  (MENU,  MenuSettingTime,   ID_HOUR,                 CALLBACK_TimeDateCfg,                    MenuSettingTime,                  LBL_VT100_HOUR                                  ) \
        ENTRY  (MENU,  MenuSettingTime,   ID_MINUTE,               CALLBACK_TimeDateCfg,                    MenuSettingTime,                  LBL_VT100_MINUTE                                ) \
        ENTRY  (MENU,  MenuSettingTime,   ID_SECOND,               CALLBACK_TimeDateCfg,                    MenuSettingTime,                  LBL_VT100_SECOND                                ) \
        ENTRY  (MENU,  MenuSettingTime,   ID_DAY,                  CALLBACK_TimeDateCfg,                    MenuSettingTime,                  LBL_VT100_DAY                                   ) \
        ENTRY  (MENU,  MenuSettingTime,   ID_MONTH,                CALLBACK_TimeDateCfg,                    MenuSettingTime,                  LBL_VT100_MONTH                                 ) \
        ENTRY  (MENU,  MenuSettingTime,   ID_YEAR,                 CALLBACK_TimeDateCfg,                    MenuSettingTime,                  LBL_VT100_YEAR                                  ) \
        ENTRY  (MENU,  MenuSettingTime,   ID_TIME_DATE_SAVE,       CALLBACK_TimeDateCfg,                    MenuSettingTime,                  LBL_VT100_BLANK                                 ) \
\
        ENTRY  (MENU,  MenuSetMisc,       ID_MISC_TITLE,           nullptr,                                 MenuSetting,                      LBL_VT100_MISC_CONFIG_MENU                      ) \
        ENTRY  (MENU,  MenuSetMisc,       ID_MISC_OPT1,            CALLBACK_MiscCfg,                        MenuSetMisc,                      LBL_VT100_BLANK                                 ) \
        ENTRY  (MENU,  MenuSetMisc,       ID_MISC_OPT2,            CALLBACK_MiscCfg,                        MenuSetMisc,                      LBL_VT100_BLANK                                 ) \
        ENTRY  (MENU,  MenuSetMisc,       ID_MISC_SERIAL_NUMBER,   CALLBACK_MiscCfg,                        MenuSetMisc,                      LBL_VT100_SERIAL_NUMBER_SETTING                 ) \
        ENTRY  (MENU,  MenuSetMisc,       ID_MISC_SAVE,            CALLBACK_MiscCfg,                        MenuSetMisc,                      LBL_VT100_BLANK                                 ) \



//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_VT100_MENU_AS_ENUM(MENU_ID)  CAT(MENU_ID, _ID),
#define EXPAND_VT100_MENU_AS_DATA(NAME)     CAT(VT100_Terminal::m_, NAME),

#define EXPAND_VT100_MENU_ITEM_AS_ENUM(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)            WHEN(EQUAL(MENU, MEMBER_OF))(MEMBER_OF ## _ ## ITEM_ID,)
#define EXPAND_AS_MENU_ENUMS_ITEM(NAME)			   			  \
enum CAT(NAME, _ItemID_e)                                     \
{                                                             \
	VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_ITEM_AS_ENUM, NAME) \
    CAT(NAME, _NB_OF_ITEMS)                                   \
};


#define EXPAND_VT100_MENU_ITEM_AS_FLAG_ENUM(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)     WHEN(EQUAL(MENU, MEMBER_OF))(ITEM_ID##_FLAG = (1 << ABS(CAT(MENU, ItemID_e)::ITEM_ID - 1)),)
#define EXPAND_AS_MENU_ENUMS_FLAG(NAME)                            \
enum CAT(NAME, _ItemFlag_e)                                        \
{                                                                  \
    VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_ITEM_AS_FLAG_ENUM, NAME) \
};


#define EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER(NAME)                                                  static const VT100_MenuDef_t CAT(m_, NAME)[CAT(NAME, _NB_OF_ITEMS)];
#define EXPAND_VT100_MENU_AS_MEMBER_VARIABLE_DATA(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)  WHEN(EQUAL(MENU, MEMBER_OF))({LABEL, IF_THEN(EQUAL(nullptr, CALLBACK), nullptr, VT100_Terminal::CALLBACK), CAT(NAVIGATE_TO, _ID)},)


#define EXPAND_VT100_AS_MENU_MEMBER_VARIABLE_DATA(NAME)                        \
const VT100_MenuDef_t VT100_Terminal::CAT(m_, NAME)[CAT(NAME, _NB_OF_ITEMS)] = \
{                                                                              \
    VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_AS_MEMBER_VARIABLE_DATA, NAME)       \
};

#define EXPAND_VT100_MENU_CALLBACK_AS_ENUM(NAME)             CAT(NAME, _ID),
#define EXPAND_VT100_MENU_CALLBACK(NAME)                    VT100_InputType_e NAME(uint8_t Input, VT100_CallBackType_e Type);

//-------------------------------------------------------------------------------------------------


 VT100_MENU_DEF(EXPAND_VT100_MENU_AS_ENUM)
 VT100_MENU_DEF(EXPAND_AS_MENU_ENUMS_ITEM)
 VT100_MENU_DEF(EXPAND_AS_MENU_ENUMS_FLAG)
 VT100_MENU_DEF(EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER)
 VT100_MENU_DEF(EXPAND_VT100_AS_MENU_MEMBER_VARIABLE_DATA)
 VT100_CALLBACK(EXPAND_VT100_MENU_CALLBACK_AS_ENUM)
 VT100_CALLBACK(EXPAND_VT100_MENU_CALLBACK)

 const VT100_MenuObject_t* VT100_Terminal::m_Menu[NUMBER_OF_MENU] =
 {
     VT100_MENU_DEF(EXPAND_VT100_MENU_AS_DATA)
 };


/* ------------------------------------------------------------------------------------------------------------------*/
