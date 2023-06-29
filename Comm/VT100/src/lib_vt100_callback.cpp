///-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100_callback.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2022 Alain Royer.
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
//
//  Note: User CLI Command Function: This is the user space for the user CLI command set.
//
//-------------------------------------------------------------------------------------------------
//
// For all callback function here:
//
// Parameter Type
//
//
//         VT100_CALLBACK_INIT     -> Initialize part of the display in the menu, or variables in use
//                                  while in menu.
//
//                                  It is called only once when then menu is being draw item by
//                                  item.
//
//         VT100_CALLBACK_ON_INPUT -> This call is made when a key that select an item in the menu is
//                                  pressed.
//
//                                  If it is used for a display page with static or dynamic
//                                  informations, you need to return with a VT100_INPUT_ESCAPE.
//
//                                  If it is used for a modification in the actual menu, you only
//                                  perform the action and then return with the VT100_INPUT_MENU_CHOICE
//                                  to wait for a new input.
//
//         VT100_CALLBACK_REFRESH  -> Called while waiting for any key to be pressed. Use this to
//                                  refresh displayed information.
//                                  NOTE: There is no VT100_CALLBACK_REFRESH called if
//                                        VT100_INPUT_MENU_CHOICE is returned. UNLESS: you put a
//                                        callback in Option 0 of the menu list
//
//         VT100_CALLBACK_FLUSH    -> If variables need to be reset or function need to be called to
//                                  reset some app state. each existing callback will be called
//                                  leaving a menu.
//                                  Do not use any malloc function in the process of
//                                  VT100_CALLBACK_INIT/VT100_CALLBACK_FLUSH unless you manage that it
//                                  might already exist.
//
// return VT100_InputType_e     Kind of input management for this page
//
//
//         VT100_INPUT_ESCAPE      -> It is not a menu is this a page with information updated on
//                                  screen, this kind of is not waiting any key other than
//                                  <ESCAPE> to exit
//
//         VT100_INPUT_MENU_CHOICE -> It tell the console it is a menu and waiting for an input.
//
//         VT100_INPUT_DECIMAL     -> If the callback make call for an VT100_SetDecimalInput(), then
//                                  it must return with this enum value to tell the console what
//                                  to do. It will comeback to the callback only when the input
//                                  decimal mode will be escaped by <ESCAPE> or value validated by
//                                  <ENTER>.
//
//         VT100_INPUT_STRING      -> If the callback make call for an VT100_SetStringInput(), then it
//                                  must return with this enum value to tell the console what to
//                                  do. We will comeback to the callback only when the input
//                                  decimal mode will be escaped by <ESCAPE> or value validated
//                                  by <ENTER>.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// this is not the place for those Settings only what is generic should stay here


#define VT100_CFG_NO_REFRESH                                  0
#define VT100_CFG_REFRESH_INFO                                32768
#define VT100_CFG_REFRESH_ALL                                 65535
#define VT100_CFG_REFRESH_COMMON_MASK                         (32768 + 16384)

#define VT100_TIME_CFG_REFRESH_HOUR                           1
#define VT100_TIME_CFG_REFRESH_MINUTE                         2
#define VT100_TIME_CFG_REFRESH_SECOND                         4
#define VT100_TIME_CFG_REFRESH_DAY                            8
#define VT100_TIME_CFG_REFRESH_MONTH                          16
#define VT100_TIME_CFG_REFRESH_YEAR                           32

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// Prototype(s)
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

static CON_DebugLevel_e         VT100_LastDebugLevel;
//static nOS_Time                 VT100_LastUpTime;
//static uint8_t                  VT100_LastSecond;
//static bool                     VT100_DrawOnlyOnce;

// scratch pad variable..
// static valid for the life of one menu iteration.. should not contain data that need to live longer.
// Uncomment if needed
//static bool                     m_GenericStringUpdated;
//static bool                     VT100_Generic_uint64_t_Updated;
//static nOS_TickCounter          VT100_GenericTimeOut1;
//static nOS_TickCounter          VT100_GenericTimeOut2;
//static uint64_t                 VT100_Generic_uint64;                                                 // uint64_t that can be used by any callback

//-------------------------------------------------------------------------------------------------
// Public(s) function(s)
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_StackUsage
//
//  Description:    Display page for stack usage
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_None(uint8_t Input, VT100_CallBackType_e Type)
{
    VAR_UNUSED(Input);
    VAR_UNUSED(Type);

    return VT100_INPUT_MENU_CHOICE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_MenuRedirection
//
//  Description:    This menu is not displayed, it only redirect a new menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_MenuRedirection(uint8_t Input, VT100_CallBackType_e Type)
{
    VAR_UNUSED(Input);

    if(Type == VT100_CALLBACK_INIT)
    {
        if(myVT100.m_IsItInStartup == true)
        {
            myVT100.GoToMenu(MenuBoot_ID);
        }
        else
        {
            myVT100.GoToMenu(MenuMain_ID);
        }
    }

    return VT100_INPUT_MENU_CHOICE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_ProductInformation
//
//  Description:    Product Information
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_ProductInformation(uint8_t Input, VT100_CallBackType_e Type)
{
    /*
    nOS_Time        UpTime;
    nOS_TimeDate    TimeDate;

    VAR_UNUSED(Input);

    switch(Type)
    {
        case VT100_CALLBACK_INIT:
        case VT100_CALLBACK_FLUSH:
        {
            // Nothing to do
            break;
        }

        case VT100_CALLBACK_ON_INPUT:
        {
            VT100_DisplayMfg();
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_PRODUCT_INFO]);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_PRODUCT_NAME],     PRODUCT_NAME);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_APP_VERSION],      SOFTWARE_REVISION);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_FW_RELEASE_DATE],  SOFTWARE_RELEASE_DATE, SOFTWARE_RELEASE_TIME);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BOOT_VERSION],     "none");
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_HARDWARE_VERSION], HW_REVISION);

            //// SYS_Read(SYS_SERIAL_NUMBER, MAIN_ACU, 0, &m_GenericString[0][0][0], NULL);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_PRODUCT_SERIAL]);
         // InMenuPrintf(SYS_GetSingleEntryTypeSize(SYS_SERIAL_NUMBER), &m_GenericString[0][0][0]);

            InMenuPrintf(VT100_SZ_NONE, " REMOVE Ble");

            //// SYS_Read(SYS_LOCATION, MAIN_ACU, 0, &m_GenericString[1][0][0], NULL);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_PRODUCT_LOCATION]);
          // InMenuPrintf(SYS_GetSingleEntryTypeSize(SYS_LOCATION), &m_GenericString[1][0][0]);

            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_UPTIME]);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_GMT_TIME]);
            InMenuPrintf(VT100_SZ_NONE, VT100_LBL_ESCAPE);

            VT100_LastSecond = 60;
            VT100_LastUpTime = 0;
            break;
        }

        case VT100_CALLBACK_REFRESH:
        {
            UpTime = nOS_GetTickCount() / NOS_CONFIG_TICKS_PER_SECOND;

            if(UpTime != VT100_LastUpTime)
            {
                VT100_LastUpTime = UpTime;
                SetCursorPosition(26, 14);
                InMenuPrintf(VT100_SZ_NONE, "%lu:",  (uint32_t)(UpTime / TIME_SECONDS_PER_DAY));
                UpTime %= TIME_SECONDS_PER_DAY;
                InMenuPrintf(VT100_SZ_NONE, "%02u:", (uint16_t)(UpTime / TIME_SECONDS_PER_HOUR));
                UpTime %= TIME_SECONDS_PER_HOUR;
                InMenuPrintf(VT100_SZ_NONE, "%02u:", (uint16_t)(UpTime / TIME_SECONDS_PER_MINUTE));
                UpTime %= TIME_SECONDS_PER_MINUTE;
                InMenuPrintf(VT100_SZ_NONE, "%02u",  (uint16_t)UpTime);
                //BLE112_getMAC();
            }

           // RTC_DateAndTime(&TimeDate, STATE_GET);

            if(TimeDate.second != VT100_LastSecond)
            {
                VT100_LastSecond = TimeDate.second;
                SetCursorPosition(26, 15);
                InMenuPrintf(VT100_SZ_NONE, "%s %u, %u,   %u:%02u:%02u ", RTC_MonthName[TimeDate.month - 1],
                                                                          TimeDate.day,
                                                                          TimeDate.year,
                                                                          TimeDate.hour,
                                                                          TimeDate.minute,
                                                                          TimeDate.second);
            }

            break;
        }
    }
*/
    return VT100_INPUT_ESCAPE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_DebugLevelSetting
//
//  Description:    Configure product debug level for serial logging
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_DebugLevelSetting(uint8_t Input, VT100_CallBackType_e Type)
{
    static CON_DebugLevel_e DebugLevel = CON_DEBUG_NONE;

    if(Type != VT100_CALLBACK_FLUSH)
    {
        // We update VT100_DebugLevel only on item 1 if it is VT100_CALLBACK_INIT type
        if(((Type == VT100_CALLBACK_INIT) && (Input == 1)) ||
           // all the time if it is a VT100_CALLBACK_REFRESH
           (Type == VT100_CALLBACK_REFRESH))
        {
            //SYS_Read(SYS_DEBUG_LEVEL, MAIN_ACU, 0, &DebugLevel, NULL);
            VT100_LastDebugLevel = DebugLevel;
        }

        myVT100.SetCursorPosition(41, 10 + Input);
      #if (VT100_USE_COLOR == DEF_ENABLED)
        myVT100.SetForeColor(VT100_COLOR_MAGENTA);
      #endif

        if(((uint8_t(1) << (Input - 1)) & DebugLevel) != 0)
        {
            if(Type != VT100_CALLBACK_INIT)
            {
                DebugLevel &= ~(1 << (Input - 1));
                myVT100.InMenuPrintf(VT100_SZ_NONE, LBL_STRING, " ");
            }
            else
            {
                myVT100.InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "*");
            }
        }
        else
        {
            if(Type != VT100_CALLBACK_INIT)
            {
                DebugLevel = CON_DebugLevel_e(uint8_t(DebugLevel) | (uint8_t(1) << (Input - 1)));
                myVT100.InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "*");
            }
            else
            {
                myVT100.InMenuPrintf(VT100_SZ_NONE, LBL_STRING, " ");
            }
        }
    }
    else
    {
        if(VT100_LastDebugLevel != DebugLevel)
        {
            // SYS_Write(SYS_DEBUG_LEVEL, MAIN_ACU, 0, &DebugLevel, 0, SYS_NO_UUID_CALL);
        }
    }

    return VT100_INPUT_MENU_CHOICE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_TimeDateCfg
//
//  Description:    Menu Time and Date Configuration.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_TimeDateCfg(uint8_t Input, VT100_CallBackType_e Type)
{
    /*
    static nOS_TimeDate TimeDate;
    uint32_t            Refresh;
    uint32_t            EditedValue;        // if we come back from decimal input
    uint8_t             InputID;            // contain the value from this input ID

    Refresh = VT100_CFG_NO_REFRESH;

    if((Type == VT100_CALLBACK_INIT) && (Input == DOOR_CTRL_MENU_TITLE)) // Menu Redraw
    {
        /// Print the box
        DrawBox(8, 23, 47, 4, VT100_COLOR_GREEN);

        /// Print the static info in the box
      #if (VT100_USE_COLOR == DEF_ENABLED)
        SetForeColor(VT100_COLOR_YELLOW);
      #endif
        SetCursorPosition(13, 24);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_TIME]);
        SetCursorPosition(13, 25);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_DATE]);
        Refresh   = VT100_CFG_REFRESH_ALL;
    }

    if(Type == VT100_CALLBACK_ON_INPUT)
    {
        switch(Input)
        {
            case 1: // Input Hour
            {
                SetDecimalInput(32, 12, 0, 23, TimeDate.hour, 1, Input, "Hour");
                return VT100_INPUT_DECIMAL;
            }

            case 2: // Edit Minute
            {
                SetDecimalInput(32, 12, 0, 59, TimeDate.minute, 1, Input, "Minute");
                return VT100_INPUT_DECIMAL;
            }

            case 3: // Edit Second
            {
                SetDecimalInput(32, 12, 0, 59, TimeDate.second, 1, Input, "Second");
                return VT100_INPUT_DECIMAL;
            }

            case 4: // Edit Day
            {
                // TODO used Digini time date method
                SetDecimalInput(32, 12, 1, nOS_TimeGetDaysPerMonth(TimeDate.month, TimeDate.year), TimeDate.day, 1, Input, "Day");
                return VT100_INPUT_DECIMAL;
            }

            case 5: // Edit Month
            {
                SetDecimalInput(32, 12, 1, 12, TimeDate.month, 1, Input, "Month");
                return VT100_INPUT_DECIMAL;
            }

            case 6: // Edit Year
            {
                SetDecimalInput(32, 12, 2000, 2255, TimeDate.year, 1, Input, "Year");
                return VT100_INPUT_DECIMAL;
            }

            case 7:  // Save the new configuration for the selected H-Bridge
            {
                if(m_NewConfigFlag[0] != 0)
                {
                    m_NewConfigFlag[0] = 0;
                    RTC_DateAndTime(&TimeDate, STATE_SET);
                }

                Refresh = VT100_CFG_REFRESH_INFO;
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------
    // Refresh all information on the page according to flag
    //--------------------------------------------------------------------------------------------

    if(Type == VT100_CALLBACK_INIT)
    {
        GetDecimalInputValue(&EditedValue, &InputID);

        if(InputID == 1)
        {
            TimeDate.hour      = (uint8_t)EditedValue;
            m_NewConfigFlag[0] = 1;
        }
        else if(InputID == 2)
        {
            TimeDate.minute    = (uint8_t)EditedValue;
            m_NewConfigFlag[0] = 1;
        }
        else if(InputID == 3)
        {
            TimeDate.second    = (uint8_t)EditedValue;
            m_NewConfigFlag[0] = 1;
        }
        else if(InputID == 4)
        {
            TimeDate.day       = (uint8_t)EditedValue;
            m_NewConfigFlag[0] = 1;
        }
        else if(InputID == 5)
        {
            TimeDate.month     = (uint8_t)EditedValue;
            m_NewConfigFlag[0] = 1;
        }
        else if(InputID == 6)
        {
            TimeDate.year      = (uint16_t)EditedValue;
            m_NewConfigFlag[0] = 1;
        }
        else if((InputID == 0) && (Input == DOOR_CTRL_MENU_TITLE) &&(m_NewConfigFlag[0] == 0))
        {
            RTC_DateAndTime(&TimeDate, STATE_GET);
        }
    }

    ///--------------------------------------------------------------------------------------------

    if(Refresh & VT100_CFG_REFRESH_INFO)
    {
        // ***********************************************
        // Refresh label on the menu for what is available

      #if (VT100_USE_COLOR == DEF_ENABLED)
        PrintSaveLabel(9, 16, (m_NewConfigFlag[0] == 1) ? VT100_COLOR_YELLOW : VT100_COLOR_BLUE);
      #else
        PrintSaveLabel(9, 16);
      #endif

        // ********************************************
        // Refresh information display on configuration

      #if (VT100_USE_COLOR == DEF_ENABLED)
        SetForeColor(VT100_COLOR_CYAN);
      #endif
        SetCursorPosition(26, 24);
        InMenuPrintf(VT100_SZ_NONE, "%u:%02u:%02u ", TimeDate.hour, TimeDate.minute, TimeDate.second);
        SetCursorPosition(26, 25);
        InMenuPrintf(VT100_SZ_NONE, "%s %u, %u ", RTC_MonthName[TimeDate.month - 1], TimeDate.day, TimeDate.year);
     }
*/
    return VT100_INPUT_MENU_CHOICE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_StackUsage
//
//  Description:    Display page for stack usage
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_StackUsage(uint8_t Input, VT100_CallBackType_e Type)
{
    return VT100_INPUT_MENU_CHOICE;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)


