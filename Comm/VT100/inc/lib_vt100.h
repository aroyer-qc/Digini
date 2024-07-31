//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100.h
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

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define COMPARE_MenuMain(x) 		        x
#define COMPARE_MenuProductInformation(x)   x
#define COMPARE_MenuStackUsage(x)           x
#define COMPARE_MenuMemoryPool(x)           x
#define COMPARE_MenuDebug(x)    	        x
#define COMPARE_MenuNetwork(x)              x
#define COMPARE_MenuSD_Card(x)     	        x
#define COMPARE_MenuSystemSetting(x)        x

#define VT100_TOKEN(x)                  VT100_Terminal::x

#define VT100_MENU_DEF(ENTRY) \
                                                ENTRY(MenuMain               )  \
    IF_USE( LABEL_USE_PRODUCT_INFO,             ENTRY(MenuProductInformation ) )\
    IF_USE( DIGINI_USE_STACKTISTIC,             ENTRY(MenuStackUsage         ) )\
    IF_USE( DIGINI_USE_STATIC_MEMORY_ALLOC,     ENTRY(MenuMemoryPool         ) )\
    IF_USE( DIGINI_USE_DEBUG_IN_CONSOLE,        ENTRY(MenuDebug              ) )\
    IF_USE( DIGINI_USE_ETHERNET,                ENTRY(MenuNetwork            ) )\
    IF_USE( DIGINI_DEBUG_SDCARD_INFO_ON_VT100,  ENTRY(MenuSD_Card            ) )\
                                                ENTRY(MenuSystemSetting        )\


#define VT100_CALLBACK(ENTRY)\
                                               ENTRY(CALLBACK_MenuMain           )  \
    IF_USE( LABEL_USE_PRODUCT_INFO,            ENTRY(CALLBACK_ProductInformation ) )\
    IF_USE( DIGINI_USE_STACKTISTIC,            ENTRY(CALLBACK_StackUsage         ) )\
    IF_USE( DIGINI_USE_STATIC_MEMORY_ALLOC,    ENTRY(CALLBACK_MemoryPool         ) )\
    IF_USE( DIGINI_USE_DEBUG_IN_CONSOLE,       ENTRY(CALLBACK_DebugLevelSetting  ) )\
    IF_USE( DIGINI_USE_ETHERNET,               ENTRY(CALLBACK_NetworkInfo        ) )\
    IF_USE( DIGINI_DEBUG_SDCARD_INFO_ON_VT100, ENTRY(CALLBACK_SD_CardInformation ) )\
                                               ENTRY(CALLBACK_SystemSetting        )\

#define VT100_MENU_TREE_DEF(ENTRY, MENU) \
\
                                                ENTRY  (MENU,  MenuMain,                ID_MAIN_TITLE,                       CALLBACK_None,                           VT100_MENU_MAIN_FALLBACK,         VT100_LBL_MAIN_MENU                             )  \
    IF_USE( LABEL_USE_PRODUCT_INFO,             ENTRY  (MENU,  MenuMain,                ID_INFO_DISPLAY,                     CALLBACK_ProductInformation,             MenuProductInformation,           VT100_LBL_SYSTEM_INFO                           ) )\
    IF_USE( DIGINI_USE_STACKTISTIC,             ENTRY  (MENU,  MenuMain,                ID_STACK_DISPLAY,                    CALLBACK_None,                           MenuStackUsage,                   VT100_LBL_STACKTISTIC                           ) )\
    IF_USE( DIGINI_USE_DEBUG_IN_CONSOLE,        ENTRY  (MENU,  MenuMain,                ID_DEBUG_MENU,                       CALLBACK_None,                           MenuDebug,                        VT100_LBL_DEBUG                                 ) )\
    IF_USE( DIGINI_USE_STATIC_MEMORY_ALLOC,     ENTRY  (MENU,  MenuMain,                ID_MEMORY_POOL_MENU,                 CALLBACK_None,                           MenuMemoryPool,                   VT100_LBL_MEMORY_POOL_STAT                      ) )\
    IF_USE( DIGINI_USE_ETHERNET,                ENTRY  (MENU,  MenuMain,                ID_NETWORK_INFO,                     CALLBACK_None,                           MenuNetwork,                      LBL_NETWORK_INFO                                ) )\
                                                ENTRY  (MENU,  MenuMain,                ID_SYSTEM_SETTING_MENU,              CALLBACK_None,                           MenuSystemSetting,                VT100_LBL_SYSTEM_SETTING                        )  \
\
    IF_USE( LABEL_USE_PRODUCT_INFO,             ENTRY  (MENU,  MenuProductInformation,  ID_INFO_SYSTEM,                      CALLBACK_ProductInformation,             MenuMain,                         VT100_LBL_SYSTEM_INFO                           ) )\
\
    IF_USE( DIGINI_USE_STACKTISTIC,             ENTRY  (MENU,  MenuStackUsage,          ID_INFO_DISPLAY,                     CALLBACK_StackUsage,                     MenuMain,                         VT100_LBL_STACKTISTIC                           ) )\
\
    IF_USE( DIGINI_USE_STATIC_MEMORY_ALLOC,     ENTRY  (MENU,  MenuMemoryPool,          ID_MEM_POOL_STATS,                   CALLBACK_MemoryPool,                     MenuMain,                         VT100_LBL_MEMORY_POOL_STAT                      ) )\
\
                                                ENTRY  (MENU,  MenuSystemSetting,       ID_SYSTEM_SETTING,                   CALLBACK_SystemSetting,                  MenuMain,                         VT100_LBL_SYSTEM_SETTING                        )  \
                                                ENTRY  (MENU,  MenuSystemSetting,       ID_SYSTEM_LANGUAGE,                  CALLBACK_SystemSetting,                  MenuSystemSetting,                VT100_LBL_LANGUAGE_SELECTION                    )  \
                                                ENTRY  (MENU,  MenuSystemSetting,       ID_MISC_SERIAL_NUMBER,               CALLBACK_SystemSetting,                  MenuSystemSetting,                VT100_LBL_SERIAL_NUMBER_SETTING                 )  \
                                                ENTRY  (MENU,  MenuSystemSetting,       ID_MISC_TEMPERATURE_UNIT,            CALLBACK_SystemSetting,                  MenuSystemSetting,                VT100_LBL_TEMPERATURE_UNIT_SELECTION            )  \
                                                ENTRY  (MENU,  MenuSystemSetting,       ID_MISC_DATE,                        CALLBACK_SystemSetting,                  MenuSystemSetting,                VT100_LBL_DATE                                  )  \
                                                ENTRY  (MENU,  MenuSystemSetting,       ID_MISC_TIME,                        CALLBACK_SystemSetting,                  MenuSystemSetting,                VT100_LBL_TIME                                  )  \
                                                ENTRY  (MENU,  MenuSystemSetting,       ID_MISC_SAVE,                        CALLBACK_SystemSetting,                  MenuSystemSetting,                VT100_LBL_SAVE_CONFIGURATION                    )  \
\
    IF_USE( DIGINI_USE_DEBUG_IN_CONSOLE, \
                                                ENTRY  (MENU,  MenuDebug,               ID_DEBUG_TITLE,                      CALLBACK_DebugLevelSetting,              MenuMain,                         VT100_LBL_DEBUG_MENU                            )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_0,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_1                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_1,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_2                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_2,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_3                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_3,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_4                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_4,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_5                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_5,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_6                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_6,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_7                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_7,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_8                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_8,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_9                               )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_9,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_10                              )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_A,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_11                              )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_B,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_12                              )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_C,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_13                              )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_D,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_14                              )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_E,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_15                              )  \
                                                ENTRY  (MENU,  MenuDebug,               ID_DBG_LVL_F,                        CALLBACK_DebugLevelSetting,              MenuDebug,                        LBL_DEBUG_LEVEL_16                              )  \
    ) \
\
    IF_USE( DIGINI_USE_ETHERNET,                ENTRY  (MENU,  MenuNetwork,             ID_NETWORK_INFO,                     CALLBACK_NetworkInfo,                    MenuMain,                         LBL_NETWORK_INFO                                ) )\
\
    IF_USE( DIGINI_DEBUG_SDCARD_INFO_ON_VT100,  ENTRY  (MENU,  MenuSD_Card,             ID_SD_CARD_INFO,                     CALLBACK_SD_CardInformation,             MenuMain,                         LBL_SD_CARD_INFORMATION                         ) )\

//-------------------------------------------------------------------------------------------------
// Expanding macro(s)
//-------------------------------------------------------------------------------------------------

/// This enum is a list of all menu page that exist in the VT100_MENU_DEF and VT100_USER_MENU_DEF.
#define EXPAND_VT100_MENU_AS_ENUM(MENU_ID)                                                                  MENU_ID ## _ID,

/// This create list of pointer on each menu (see Note 1)
#define EXPAND_VT100_MENU_AS_DATA(NAME)                                                                     {VT100_Terminal::m_ ## NAME, NAME ## _NB_OF_ITEMS},


/// This will create a enum list of all sub menu items contains in each menu page.
#define EXPAND_VT100_MENU_ITEM_AS_ENUM(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)              WHEN(EQUAL(MENU, MEMBER_OF))(MEMBER_OF ## _ ## ITEM_ID,)
#define EXPAND_AS_MENU_ENUMS_ITEM(NAME)			   			       \
enum NAME ## _ItemID_e                                             \
{                                                                  \
	VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_ITEM_AS_ENUM, NAME)      \
    VT100_USER_MENU_TREE_DEF(EXPAND_VT100_MENU_ITEM_AS_ENUM, NAME) \
    NAME ## _NB_OF_ITEMS                                           \
};

/// This create the class member structure declaration containing sub item information for each menu.
#define EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER(NAME)                                                   static const VT100_MenuDef_t CAT(m_, NAME)[NAME ## _NB_OF_ITEMS];

/// (Note 1) This create the class member structure containing actual sub item information for each menu.
#define EXPAND_VT100_MENU_AS_MEMBER_VARIABLE_DATA(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)   WHEN(EQUAL(MENU, MEMBER_OF))({LABEL, (VT100_TOKEN(CALLBACK)), PRIMITIVE_CAT(NAVIGATE_TO, _ID)},)
#define EXPAND_VT100_AS_MENU_MEMBER_VARIABLE_DATA(NAME)                                                     const VT100_MenuDef_t VT100_Terminal::m_ ## NAME[NAME ## _NB_OF_ITEMS] =       \
                                                                                                            {                                                                              \
                                                                                                                VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_AS_MEMBER_VARIABLE_DATA, NAME)       \
                                                                                                            };

/// this automatically create all the method declaration in the class for each callback
#define EXPAND_VT100_MENU_CALLBACK(NAME)                                                                    static VT100_InputType_e NAME(uint8_t Input, VT100_CallBackType_e Type);

//-------------------------------------------------------------------------------------------------
// Configuration check
//-------------------------------------------------------------------------------------------------

#if (VT100_USE_STANDARD_MENU_STATIC_INFO == DEF_ENABLED) && (VT100_USE_USER_MENU_STATIC_INFO == DEF_ENABLED)
  #pragma message "Cannot have both VT100_USE_STANDARD_MENU_STATIC_INFO and VT100_USE_USER_MENU_STATIC_INFO use at the same time"
  #error DEF_ENABLED Error
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define VT100_X_SIZE                        100
#define VT100_Y_SIZE                        60

#define VT100_OFFSET_COLOR_FOREGROUND       30
#define VT100_OFFSET_COLOR_BACKGROUND       40
#define CON_FIFO_PARSER_RX_SZ               64

#define CONFIG_FLAG_SIZE                    8  //???

#define VT100_STRING_QTS                    8
#define VT100_ITEMS_QTS                     8
#define VT100_STRING_SZ                     64

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum VT100_Menu_e
{
    VT100_MENU_DEF(EXPAND_VT100_MENU_AS_ENUM)
    VT100_USER_MENU_DEF(EXPAND_VT100_MENU_AS_ENUM)
    NUMBER_OF_MENU,
  #if (VT100_MENU_MAIN_FALLBACK_CFG != VT100_MENU_NONE)
    VT100_MENU_NONE,                                    // It is needed to create the enum, if it is not the fall-back for the main menu
  #endif
    VT100_MENU_MAIN_FALLBACK_CFG,
    VT100_MENU_NONE_ID = VT100_MENU_NONE,
    VT100_MENU_MAIN_FALLBACK_ID = VT100_MENU_MAIN_FALLBACK_CFG,
};

enum VT100_Color_e
{
	VT100_COLOR_BLACK      = 0,
	VT100_COLOR_RED,
	VT100_COLOR_GREEN,
	VT100_COLOR_YELLOW,
	VT100_COLOR_BLUE,
	VT100_COLOR_MAGENTA,
	VT100_COLOR_CYAN,
	VT100_COLOR_WHITE,
};

enum VT100_Attribute_e
{
    VT100_RESET_ALL        = 0,
	VT100_BRIGHT,
	VT100_DIM,
	VT100_UNDERSCORE,
	VT100_BLINK,
	VT100_REVERSE,
	VT100_HIDDEN,
	VT100_FORE_BLACK       = 30,
	VT100_FORE_RED,
	VT100_FORE_GREEN,
	VT100_FORE_YELLOW,
	VT100_FORE_BLUE,
	VT100_FORE_MAGENTA,
	VT100_FORE_CYAN,
	VT100_FORE_WHITE,
	VT100_BACK_BLACK       = 40,
	VT100_BACK_RED,
	VT100_BACK_GREEN,
	VT100_BACK_YELLOW,
	VT100_BACK_BLUE,
	VT100_BACK_MAGENTA,
	VT100_BACK_CYAN,
	VT100_BACK_WHITE,
};

enum VT100_InputType_e
{
    VT100_INPUT_MENU_CHOICE,
    VT100_INPUT_SAVE_DATA,
    VT100_INPUT_DECIMAL,
    VT100_INPUT_STRING,
    VT100_INPUT_ESCAPE,
  //  VT100_INPUT_ESCAPE_TO_CONTINUE,
};

enum VT100_CallBackType_e
{
    // For page that contain no menu selection and has only dynamic widget
    VT100_CALLBACK_INIT,
    VT100_CALLBACK_REFRESH_ONCE,
    VT100_CALLBACK_REFRESH,
    VT100_CALLBACK_FLUSH,

    VT100_CALLBACK_ON_INPUT,           // Selection input of items in thjs menu.

    // Dynamic item in selection menu
  // VT100_CALLBACK_ON_INIT,            // For Initializing items when they are listed in the menu (dynamic info on the menu page)
//    VT100_CALLBACK_ON_MENU_REFRESH,         // For Refreshing items when they are listed in the menu (dynamic info on the menu page)
    VT100_CALLBACK_ON_FLUSH,           // When escaping or going through another menu (EX. release memory)
};

typedef VT100_InputType_e (*CallbackMethod_t)(uint8_t, VT100_CallBackType_e);

struct VT100_MenuDef_t
{
    Label_e           Label;
    VT100_InputType_e (*pCallback)(uint8_t, VT100_CallBackType_e);
    //CallbackMethod_t  pCallback;
    VT100_Menu_e      NextMenu;
};

struct VT100_MenuObject_t
{
    const VT100_MenuDef_t*    pDefinition;
    size_t                    Size;
};

// Create all the menu enum ID for each existing menu definition
// Example MenuSystemSetting_ItemID_e it will have all ID for the Menu item 1 is ID_SYSTEM_LANGUAGE (to be used in callback)
VT100_MENU_DEF(EXPAND_AS_MENU_ENUMS_ITEM)
VT100_USER_MENU_DEF(EXPAND_AS_MENU_ENUMS_ITEM)

//-------------------------------------------------------------------------------------------------
// class
//-------------------------------------------------------------------------------------------------

class VT100_Terminal : public ChildProcessInterface
{
    public:

        void                IF_Process                  (void);
        nOS_Error           Initialize                  (Console* pConsole);
      #if (VT100_USER_CALLBACK_INITIALIZE == DEF_ENABLED)
        void                CallbackInitialize          (void);
      #endif
        void                GoToMenu                    (VT100_Menu_e MenuID);
        size_t              InMenuPrintf                (Label_e Label, ...);                                   // TODO they should also be a generic to CLI and other serial printing methos that need language
        size_t              InMenuPrintf                (uint8_t PosX, uint8_t PosY, Label_e Label, ...);       // TODO this should use part of the code from the prior function
        void                RestoreAttribute            (void);
        void                RestoreCursorPosition       (void);
        void                SaveAttribute               (void);
        void                SaveCursorPosition          (void);
        void                SetAttribute                (VT100_Attribute_e Attribute);

        void                Bargraph                    (uint8_t PosX, uint8_t PosY, VT100_Color_e Color, uint8_t Value, uint8_t Max, uint8_t Size);
        void                DrawBox                     (uint8_t PosX, uint8_t PosY, uint8_t H_Size, uint8_t V_Size, VT100_Color_e ForeColor);
        void                DrawVline                   (uint8_t PosX, uint8_t PosY, uint8_t V_Size, VT100_Color_e ForeColor);
        void                DrawHline                   (uint8_t PosX, uint8_t PosY, uint8_t H_Size, VT100_Color_e ForeColor);

      #if (VT100_USE_COLOR == DEF_ENABLED)
        void                SetColor                    (VT100_Color_e ForeColor, VT100_Color_e BackColor);
        inline void         SetForeColor                (VT100_Color_e Color)		{ SetAttribute(VT100_Attribute_e(int(Color) + VT100_OFFSET_COLOR_FOREGROUND)); }
        inline void         SetBackColor                (VT100_Color_e Color)       { SetAttribute(VT100_Attribute_e(int(Color) + VT100_OFFSET_COLOR_BACKGROUND)); }
      #else
        void                SetColor                    (...) {}
        inline void         SetForeColor                (...) {}
        inline void         SetBackColor                (...) {}
        void                InvertMono                  (bool Invert);
      #endif
        void                UpdateSaveLabel             (VT100_Color_e Color = VT100_COLOR_WHITE, bool State = true);
        void                SetRefreshFullPage          (void)                          { m_RefreshFullPage = true; }
        void                SetCursorPosition           (uint8_t PosX, uint8_t PosY);
        void                SetScrollZone               (uint8_t FirstLine, uint8_t LastLine);

        // Use by user callback to request a decimal input
        void                GetDecimalInputValue        (uint32_t* pValue, uint8_t* pID);
        void                GetStringInput              (char* pString, uint8_t* pID);
        void                SetDecimalInput             (uint8_t PosX, uint8_t PosY, int32_t Minimum, int32_t Maximum, int32_t Value, uint16_t Divider, uint8_t ID, Label_e Label);
        void                SetStringInput              (uint8_t PosX, uint8_t PosY, int32_t Maximum, uint8_t ID, Label_e Label, const char* pString);


        uint32_t            GetConfigFlag               (int Flag)                      { return m_ConfigFlag[Flag];  }      // todo check range or change the method
        void                SetConfigFlag               (int Flag, uint32_t Value)      { m_ConfigFlag[Flag] = Value; }      // todo check range or change the method

        void                DisplayTimeDateStamp        (uint8_t PosX, uint8_t PosY, DateAndTime_t* pTimeDate);

// to check if needed in VT100
void                LockDisplay                 (bool);
bool                GetString                   (char* pBuffer, size_t Size);

    private:

        void                        ProcessRX                   (void);
        void                        DisplayMenu                 (void);
        void                        FinalizeAllItems            (void);
      #if (VT100_USE_STANDARD_MENU_STATIC_INFO == DEF_ENABLED) || (VT100_USER_MENU_STATIC_INFO == DEF_ENABLED)
        void                        PrintMenuStaticInfo         (void);
      #endif
        void                        ClearScreenWindow           (uint8_t PosX, uint8_t PosY, uint8_t SizeX, uint8_t SizeY);
        void                        ClearInputMenuSelection     (void);
        void                        RepeatChar                  (uint8_t Char, size_t Count);
        void                        MenuSelectItems             (char ItemsChar);

        VT100_InputType_e           CallBack                    (VT100_InputType_e (*pCallback)(uint8_t, VT100_CallBackType_e), VT100_CallBackType_e Type, uint8_t Item);
        static void                 EscapeCallback              (nOS_Timer* pTimer, void* pArg);
        void                        InputString                 (void);
        void                        InputDecimal                (void);
        void                        ClearConfigFLag             (void);
        static VT100_InputType_e    CALLBACK_None               (uint8_t Input, VT100_CallBackType_e Type);
        VT100_CALLBACK(EXPAND_VT100_MENU_CALLBACK)                  // Generation of all user callback prototype
        VT100_USER_CALLBACK(EXPAND_VT100_MENU_CALLBACK)
        Console*                            m_pConsole;
        bool                                m_IsItInitialized;
        bool                                m_IsItInStartup;
        VT100_Menu_e                        m_MenuID;
        const VT100_MenuDef_t*              m_pMenu;
        uint8_t                             m_PosY_SaveLabel;
        uint8_t                             m_LastSetCursorPosX;                // Set position to input selection location on screen
        uint8_t                             m_LastSetCursorPosY;
        VT100_InputType_e                   m_InputType;
        uint8_t                             m_Input;
        uint8_t                             m_InputCount;
        bool                                m_ValidateInput;
        uint8_t                             m_ItemsQts;
        bool                                m_BypassPrintf;
        bool                                m_LogsAreMuted;
        nOS_Timer                           m_EscapeTimer;
        bool                                m_IsDisplayLock;
        bool                                m_FlushNextEntry;
        bool                                m_ForceRefresh;
        bool                                m_RefreshOnce;
        bool                                m_NeedToSave;
        bool                                m_RefreshFullPage;

        // Input string or decimal service
        int32_t                             m_Minimum;
        int32_t                             m_Maximum;
        uint8_t                             m_PosX;
        uint8_t                             m_PosY;
        int16_t                             m_ID;
        int32_t                             m_Value;
        int32_t                             m_RefreshValue;
        int32_t                             m_OldValue;
        uint16_t                            m_Divider;
        bool                                m_InputDecimalMode;
        volatile bool                       m_InEscapeSequence;
        int32_t                             m_InputPtr;
        int32_t                             m_RefreshInputPtr;
        char*                               m_pString;
        bool                                m_InputStringMode;
        bool                                m_IsItString;
        uint32_t                            m_ConfigFlag[CONFIG_FLAG_SIZE];
        static const VT100_MenuObject_t     m_Menu[NUMBER_OF_MENU];

        VT100_MENU_DEF(EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER)
        VT100_USER_MENU_DEF(EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER)

};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef VT100_GLOBAL

class VT100_Terminal            myVT100;

#else

extern class VT100_Terminal     myVT100;

#endif // VT100_GLOBAL

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

