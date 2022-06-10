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

#define VT100_CFG_NO_REFRESH                                  0
#define VT100_CFG_REFRESH_INFO                                32768
#define VT100_CFG_REFRESH_ALL                                 65535
#define VT100_CFG_REFRESH_COMMON_MASK                         (32768 + 16384)

#define VT100_MISC_CFG_REFRESH_SET_POINT_LOW                  1
#define VT100_MISC_CFG_REFRESH_SET_POINT_HIGH                 2
#define VT100_MISC_CFG_REFRESH_SET_POINT_BACKUP_LOW_VOLTAGE   4
#define VT100_MISC_CFG_REFRESH_SET_POINT_POWER_LOW_VOLTAGE    8
#define VT100_MISC_CFG_REFRESH_SERIAL                         16
#define VT100_MISC_CFG_REFRESH_LOCATION                       32
#define VT100_MISC_CFG_REFRESH_SITE_ID                        64

#define VT100_TIME_CFG_REFRESH_HOUR                           1
#define VT100_TIME_CFG_REFRESH_MINUTE                         2
#define VT100_TIME_CFG_REFRESH_SECOND                         4
#define VT100_TIME_CFG_REFRESH_DAY                            8
#define VT100_TIME_CFG_REFRESH_MONTH                          16
#define VT100_TIME_CFG_REFRESH_YEAR                           32

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

// Auto adjusting menu item according to configuration
typedef enum
{
    MISC_SETTING_MENU_TITLE         = 0,
    MISC_SETTING_TEMP_LOW_SET_POINT,
    MISC_SETTING_TEMP_HIGH_SET_POINT,
    MISC_SETTING_BATTERY_BACKUP_RTC_LOW_VOLTAGE,
    MISC_SETTING_BATTERY_POWER_LOW_VOLTAGE,
    MISC_SETTING_SERIAL_NUMBER,
    MISC_SETTING_LOCATION,
    MISC_SETTING_SITE_ID,
    MISC_SETTING_SAVE_CONFIGURATION,
} VT100_MiscSetting_e;

//-------------------------------------------------------------------------------------------------
// Prototype(s)
//-------------------------------------------------------------------------------------------------

//static void    VT100_PrintVoltage    (uint8_t xPos, uint8_t yPos, uint32_t Voltage);

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

//static uint8_t                  VT100_LastDebugLevel;
//static nOS_Time                 VT100_LastUpTime;
//static uint8_t                  VT100_LastSecond;
//static bool                     VT100_DrawOnlyOnce;
//static bool                     VT100_BluetoothAdvertisement;

// scratch pad variable..
// static valid for the life of one menu iteration.. should not contain data that need to live longer.
// Uncomment if needed
//static bool                     m_GenericStringUpdated;
//static bool                     VT100_Generic_uint64_t_Updated;
//static nOS_TickCounter          VT100_GenericTimeOut1;
//static nOS_TickCounter          VT100_GenericTimeOut2;
//static uint64_t                 VT100_Generic_uint64;                                                 // uint64_t that can be used by any callback

// TO DO change for the Bluetooth variable for the TX power
//static uint8_t  VT100_TX_Power;

//-------------------------------------------------------------------------------------------------
// Private(s) function(s)
//-------------------------------------------------------------------------------------------------
/*
static void VT100_PrintVoltage(uint8_t xPos, uint8_t yPos, uint32_t Voltage)
{
    myVT100_Terminal.SetCursorPosition(xPos, yPos);
    myVT100_Terminal.InMenuPrintf(VT100_SZ_NONE, "%u.%03u", Voltage / 1000, Voltage % 1000);
}
*/

//-------------------------------------------------------------------------------------------------
// Public(s) function(s)
//-------------------------------------------------------------------------------------------------

/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Callback Initialize
  *
  *         put in this function what you want to be initialize only once at boot up
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
void VT100_CallbackInitialize(void)
{
}


/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Menu Selection
  *
  *         This menu is not displayed, it only redirect a new menu
  *
  *--------------------------------------------------------------------------------------------------------------------
  */


VT100_InputType_e VT100_Terminal::CALL_MenuSelection(uint8_t Input, VT100_CallBackType_e Type)
{
    VAR_UNUSED(Input);

    if(Type == VT100_CALLBACK_INIT)
    {
        if(m_IsItInStartup == true)
        {
            GoToMenu(VT100_MENU_MAIN_BOOT_TIME);
        }
        else
        {
            GoToMenu(VT100_MENU_MAIN_APP_RUNNING);
        }
    }

    return VT100_INPUT_MENU_CHOICE;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Product Information
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_ProductInformation(uint8_t Input, VT100_CallBackType_e Type)
{
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

            RTC_DateAndTime(&TimeDate, STATE_GET);

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

    return VT100_INPUT_ESCAPE;
}


/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Configure product debug level for serial logging
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_DebugLevelSetting(uint8_t Input, VT100_CallBackType_e Type)
{
    static uint8_t DebugLevel = 0;

    if(Type != VT100_CALLBACK_FLUSH)
    {
        // We update VT100_DebugLevel only on item 1 if it is VT100_CALLBACK_INIT type
        if(((Type == VT100_CALLBACK_INIT) && (Input == 1)) ||
           // all the time if it is a VT100_CALLBACK_REFRESH
           (Type == VT100_CALLBACK_REFRESH))
        {
       //     // SYS_Read(SYS_DEBUG_LEVEL, MAIN_ACU, 0, &DebugLevel, NULL);
            VT100_LastDebugLevel = DebugLevel;
        }

        SetCursorPosition(40, 9 + Input);
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_MAGENTA);
      #endif

        if((((uint8_t)1 << (Input - 1)) & DebugLevel) != 0)
        {
            if(Type != VT100_CALLBACK_INIT)
            {
                DebugLevel &= ~((uint8_t)1 << (Input - 1));
                InMenuPrintf(VT100_SZ_NONE, " ");
            }
            else
            {
                InMenuPrintf(VT100_SZ_NONE, "*");
            }
        }
        else
        {
            if(Type != VT100_CALLBACK_INIT)
            {
                DebugLevel |= ((uint8_t)1 << (Input - 1));
                InMenuPrintf(VT100_SZ_NONE, "*");
            }
            else
            {
                InMenuPrintf(VT100_SZ_NONE, " ");
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


/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Allowed the LED to be controlled in override mode
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_LedControl(uint8_t Input, VT100_CallBackType_e Type)
{
    uint8_t Led;

    Led = Input - 1;

    if(Type == VT100_CALLBACK_INIT)
    {
        if(Input == DOOR_CTRL_MENU_TITLE)
        {
            VT100_Generic_uint64 = 0;
        }
        else
        {
            IO_Ctrl(Led, STATE_CLEAR);
        }
    }

    if(Input != DOOR_CTRL_MENU_TITLE)
    {
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_GREEN);
      #endif

        if((((uint64_t)1 << Input) & VT100_Generic_uint64) != 0)
        {
            if(Type == VT100_CALLBACK_ON_INPUT)
            {
                VT100_Generic_uint64 &= ~((uint64_t)1 << Input);
                IO_Ctrl(Led, STATE_CLEAR);
            }
            else
            {
                IO_Ctrl(Led, STATE_SET);
            }
        }
        else
        {
            if(Type == VT100_CALLBACK_ON_INPUT)
            {
                VT100_Generic_uint64 |= ((uint64_t)1 << Input);
                IO_Ctrl(Led, STATE_SET);
            }
            else
            {
                IO_Ctrl(Led, STATE_CLEAR);
            }
        }
    }

    if((Type == VT100_CALLBACK_REFRESH) && (Input == DOOR_CTRL_MENU_TITLE))
    {
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_GREEN);
      #endif

        for(uint8_t i = 0; i <= 3; i++)
        {
            SetCursorPosition(37, 10 + i);
            if((VT100_Generic_uint64 & ((uint64_t)1 << (i + 1))) == 0)
            {
                InMenuPrintf(VT100_SZ_NONE, " ");
            }
            else
            {
                InMenuPrintf(VT100_SZ_NONE, "*");
            }
        }
    }

    return VT100_INPUT_MENU_CHOICE;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Display the Input of the Module
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_InputReading(uint8_t Input, VT100_CallBackType_e Type)
{
    int32_t Temperature;
    uint8_t VerticalOffset;

    VAR_UNUSED(Input);

    switch(Type)
    {
        case VT100_CALLBACK_ON_INPUT:
        {
            VT100_Generic_uint64 = 10;    // Use for refresh ADC

            VT100_DisplayMfg();
          #ifdef CONSOLE_USE_COLOR
            SetForeColor(VT100_COLOR_CYAN);
          #endif
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_TEMP_SENSOR]);
            InMenuPrintf(VT100_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
          #ifdef CONSOLE_USE_COLOR
            SetForeColor(VT100_COLOR_YELLOW);
          #endif
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_CPU_TEMP_SENSOR]);
          #ifdef CONSOLE_USE_COLOR
            SetForeColor(VT100_COLOR_CYAN);
          #endif
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_INPUT_ANALOG]);
            InMenuPrintf(VT100_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
          #ifdef CONSOLE_USE_COLOR
            SetForeColor(VT100_COLOR_YELLOW);
          #endif
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_12_VOLT]);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_CPU_VDD]);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BATTERY_LEVEL]);
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BACKUP_BATTERY_LEVEL]);
          #ifdef CONSOLE_USE_COLOR
            SetForeColor(VT100_COLOR_CYAN);
          #endif
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_INPUT_DIGITAL]);
            InMenuPrintf(VT100_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
          #ifdef CONSOLE_USE_COLOR
            SetForeColor(VT100_COLOR_YELLOW);
          #endif
            //InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_EXTERNAL_SWITCH]);

            InMenuPrintf(VT100_SZ_NONE, VT100_LBL_ESCAPE);
            break;
        }

        case VT100_CALLBACK_REFRESH:
        {
            VT100_Generic_uint64--;
            if(VT100_Generic_uint64 == 0)
            {
                VT100_Generic_uint64 = 10;
                //for(ADC_Channel_e i = ADC_FIRST_ITEM; i < NUMBER_OF_ADC_CHANNEL; i++)
                {
//                    ADC_StartConversion(i);
                }
            }

            // Temperature Sensor
          #ifdef CONSOLE_USE_COLOR
            SetForeColor(VT100_COLOR_WHITE);
          #endif

            // Print in Celsius For CPU Sensor
           // Temperature = ADC_GetTemperature();
            SetCursorPosition(28, 6);
            InMenuPrintf(VT100_SZ_NONE, "%3d.%02d ", Temperature / 1000, (abs(Temperature % 1000) / 10));

            // Print in Fahrenheit CPU
            Temperature = 0;//TEMP_SENSOR_ConvertTemperatureToFahrenheit(Temperature);
            SetCursorPosition(41, 6);
            InMenuPrintf(VT100_SZ_NONE, "%3d.%02d ", Temperature / 1000, (abs(Temperature % 1000) / 10));

            VerticalOffset = 10;

            VT100_PrintVoltage(28, VerticalOffset++, 1234);
            VT100_PrintVoltage(28, VerticalOffset++, 501);
            VT100_PrintVoltage(28, VerticalOffset++, 4200);
            VT100_PrintVoltage(28, VerticalOffset++, 301);

            VerticalOffset += 3;
            SetCursorPosition(28, VerticalOffset++);
            if(IO_Ctrl(IO_EXT_SWITCH, STATE_READ) == STATE_SET)
            {
              #ifdef CONSOLE_USE_COLOR
                SetColor(VT100_COLOR_BLACK, VT100_COLOR_GREEN);
              #endif
                InMenuPrintf(VT100_SZ_NONE, " High ");
            }
            else
            {
              #ifdef CONSOLE_USE_COLOR
                SetColor(VT100_COLOR_BLACK, VT100_COLOR_RED);
              #endif
                InMenuPrintf(VT100_SZ_NONE, " Low  ");
            }
        }

        default:
            break;
    }

    return VT100_INPUT_ESCAPE;
}


/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Bluetooth Test
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_BlueTooth(uint8_t Input, VT100_CallBackType_e Type)
{
    nOS_TimeDate TimeDate;

    VAR_UNUSED(Input);

    if(Type == VT100_CALLBACK_INIT)
    {
        //BLE112_Initialize();
        //BLE112_setSecurityParams(true,24, sm_io_capability_noinputnooutput);
        //BLE112_setBondable(false);
        //BLE112_setDisconnectCallback(BLUETOOTH_userDisconnected);

        SetCursorPosition(30, 18);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BLUETOOTH_STATUS]);
        SetCursorPosition(24, 23);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BLUETOOTH_RSSI]);
        SetCursorPosition(22, 20);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BLUETOOTH_BOX1]);
        SetCursorPosition(22, 21);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BLUETOOTH_BOX2]);
        SetCursorPosition(22, 22);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BLUETOOTH_BOX3]);
        SetCursorPosition(1, 25);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_STATUS]);
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_CYAN);
      #endif
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
        SetCursorPosition(1, 34);
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
        SetScrollZone(27, 33);
    }

    if(Type == VT100_CALLBACK_REFRESH)
    {
        // fake log
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_GREEN);
      #endif
        SetCursorPosition(1, 33);
        RTC_DateAndTime(&TimeDate, STATE_GET);
        VT100_DisplayTimeDateStamp(&TimeDate);
        SetCursorPosition(43, 18);

        //if(BLUETOOTH_isUserConnected() == true)
        {
          #ifdef CONSOLE_USE_COLOR
            SetColor(VT100_COLOR_BLACK, VT100_COLOR_GREEN);
          #endif
            InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BLUETOOTH_CONNECT]);
        }
        //else
        //{
            //#ifdef CONSOLE_USE_COLOR
        //    SetColor(VT100_COLOR_BLACK, VT100_COLOR_RED);
            // #endif
        //    InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_BLUETOOTH_DISCONNECT]);
        //}
    }

    return VT100_INPUT_MENU_CHOICE;
}

/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Bluetooth get RSSI
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_GetRSSI(uint8_t Input, VT100_CallBackType_e Type)
{
    int8_t RSSI;
    uint8_t graphRSSI;
    //RSSI = BLUETOOTH_getRSSI();
    graphRSSI = (uint8_t)(RSSI + 103);

    VAR_UNUSED(Input);

    if((Type == VT100_CALLBACK_INIT) || (Type == VT100_CALLBACK_ON_INPUT))
    {
    #ifdef CONSOLE_USE_COLOR
        Bargraph(23, 21, VT100_COLOR_BLUE, graphRSSI, 65, 15);
    #else
        Bargraph(23, 21, graphRSSI, 65, 15);
    #endif
        SetCursorPosition(31, 23);
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_CYAN);
      #endif
        InMenuPrintf(VT100_SZ_NONE, "%4d", (int16_t)RSSI);
    }

    return VT100_INPUT_MENU_CHOICE;
}


/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Menu setting configuration for orphan item
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_MiscCfg(uint8_t Input, VT100_CallBackType_e Type)
{
    static int16_t  TemperatureAlarmLow;
    static int16_t  TemperatureAlarmHigh;
    static uint16_t BackupBatteryLowVoltage;
    static uint16_t PowerBatteryLowVoltage;
    uint32_t        Refresh;
    uint32_t        EditedValue;        // if we come back from decimal input
    uint8_t         InputID;            // contain the value from this input ID
    uint8_t         PosY;
    uint16_t        Size;
    char*           pStr;

    Refresh = VT100_CFG_NO_REFRESH;

    if(Type == VT100_CALLBACK_INIT) // Menu Redraw
    {
        /// Print the box
        DrawBox(8, 23, 77, 9, VT100_COLOR_GREEN);

        /// Print the static info in the
        PosY = 24;
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_YELLOW);
      #endif
        SetCursorPosition(13, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_MISC_TEMPERATURE_LOW]);
        SetCursorPosition(13, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_MISC_TEMPERATURE_HIGH]);
        SetCursorPosition(13, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_MISC_BACKUP_LOW_BATT]);
        SetCursorPosition(13, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_MISC_POWER_LOW_BATT]);
        SetCursorPosition(13, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_MISC_SERIAL_NUMBER]);
        SetCursorPosition(13, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_MISC_LOCATION]);
        SetCursorPosition(13, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LABEL_pStr[LBL_MISC_SITE_ID]);
        Refresh = VT100_CFG_REFRESH_ALL;
    }

    if(Type == VT100_CALLBACK_ON_INPUT)
    {
        switch(Input)
        {
            case MISC_SETTING_TEMP_LOW_SET_POINT: // Edit Temperature Alarm set LOW point
            {
                int16_t Min;
                int16_t Max;

                // // SYS_ReadMinMax(SYS_TEMP_ALARM_LEVEL_LOW, 0, &Min, &Max);
                SetDecimalInput(32, 16, Min, Max, TemperatureAlarmLow, 10, Input, LABEL_pStr[LBL_MISC_TEMPERATURE_LOW]);
                return VT100_INPUT_DECIMAL;
            }

            case MISC_SETTING_TEMP_HIGH_SET_POINT: // Edit Temperature Alarm set HIGH point
            {
                int16_t Min;
                int16_t Max;

                // // SYS_ReadMinMax(SYS_TEMP_ALARM_LEVEL_HIGH, 0, &Min, &Max);
                SetDecimalInput(32, 16, Min, Max, TemperatureAlarmHigh, 10, Input, LABEL_pStr[LBL_MISC_TEMPERATURE_HIGH]);
                return VT100_INPUT_DECIMAL;
            }

            case MISC_SETTING_BATTERY_BACKUP_RTC_LOW_VOLTAGE:
            {
                uint16_t Min;
                uint16_t Max;

                // // SYS_ReadMinMax(SYS_BACKUP_BATT_LOW_VOLT_THOLD, 0, &Min, &Max);
                SetDecimalInput(32, 16, Min, Max, BackupBatteryLowVoltage, 1000, Input, LABEL_pStr[LBL_MISC_BACKUP_LOW_BATT]);
                return VT100_INPUT_DECIMAL;
            }

            case MISC_SETTING_BATTERY_POWER_LOW_VOLTAGE:
            {
                uint16_t Min;
                uint16_t Max;

                // // SYS_ReadMinMax(SYS_BATT_LOW_VOLTAGE_THRESHOLD, 0, &Min, &Max);
                SetDecimalInput(32, 16, Min, Max, PowerBatteryLowVoltage, 1000, Input, LABEL_pStr[LBL_MISC_POWER_LOW_BATT]);
                return VT100_INPUT_DECIMAL;
            }

            case MISC_SETTING_SERIAL_NUMBER:
            {
                //Size = SYS_GetSingleEntryTypeSize(SYS_SERIAL_NUMBER);
                SetStringInput(32, 16, Size, Input, LABEL_pStr[LBL_MISC_SERIAL_NUMBER], &m_GenericString[1][0][0]);
                return VT100_INPUT_STRING;
            }

            case MISC_SETTING_LOCATION:
            {
               // Size = SYS_GetSingleEntryTypeSize(SYS_LOCATION);
                SetStringInput(32, 16, Size, Input, LABEL_pStr[LBL_MISC_LOCATION], &m_GenericString[2][0][0]);
                return VT100_INPUT_STRING;
            }

            case MISC_SETTING_SITE_ID:
            {
              //  Size = SYS_GetSingleEntryTypeSize(SYS_SITE_ID);
                SetStringInput(32, 16, Size, Input, LABEL_pStr[LBL_MISC_SITE_ID], &m_GenericString[3][0][0]);
                return VT100_INPUT_STRING;
            }

            case MISC_SETTING_SAVE_CONFIGURATION:
            {
                if(m_NewConfigFlag[0] != 0)
                {
                    if((m_NewConfigFlag[0] & VT100_MISC_CFG_REFRESH_SET_POINT_LOW) != 0)
                    {
                        // SYS_Write(SYS_TEMP_ALARM_LEVEL_LOW, MAIN_ACU, 0, &TemperatureAlarmLow, 0, SYS_NO_UUID_CALL);
                    }

                    if((m_NewConfigFlag[0] & VT100_MISC_CFG_REFRESH_SET_POINT_HIGH) != 0)
                    {
                        // SYS_Write(SYS_TEMP_ALARM_LEVEL_HIGH, MAIN_ACU, 0, &TemperatureAlarmHigh, 0, SYS_NO_UUID_CALL);
                    }

                    if((m_NewConfigFlag[0] & VT100_MISC_CFG_REFRESH_SET_POINT_BACKUP_LOW_VOLTAGE) != 0)
                    {
                        // SYS_Write(SYS_BACKUP_BATT_LOW_VOLT_THOLD, MAIN_ACU, 0, &BackupBatteryLowVoltage, 0, SYS_NO_UUID_CALL);
                    }

                    if((m_NewConfigFlag[0] & VT100_MISC_CFG_REFRESH_SET_POINT_POWER_LOW_VOLTAGE) != 0)
                    {
                        // SYS_Write(SYS_BATT_LOW_VOLTAGE_THRESHOLD, MAIN_ACU, 0, &PowerBatteryLowVoltage, 0, SYS_NO_UUID_CALL);
                    }

                    if((m_NewConfigFlag[0] & VT100_MISC_CFG_REFRESH_SERIAL) != 0)
                    {
                        // SYS_Write(SYS_SERIAL_NUMBER, MAIN_ACU, 0, &m_GenericString[1][0][0], 0, SYS_NO_UUID_CALL);
                    }

                    if((m_NewConfigFlag[0] & VT100_MISC_CFG_REFRESH_LOCATION) != 0)
                    {
                        // SYS_Write(SYS_LOCATION, MAIN_ACU, 0, &m_GenericString[2][0][0], 0, SYS_NO_UUID_CALL);
                    }

                    if((m_NewConfigFlag[0] & VT100_MISC_CFG_REFRESH_SITE_ID) != 0)
                    {
                        // SYS_Write(SYS_SITE_ID, MAIN_ACU, 0, &m_GenericString[3][0][0], 0, SYS_NO_UUID_CALL);
                    }
                }

                m_NewConfigFlag[0] = 0;
                Refresh   = VT100_CFG_REFRESH_INFO;
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------
    // Refresh all information on the page according to flag
    //--------------------------------------------------------------------------------------------

    if(Type == VT100_CALLBACK_INIT)
    {
        /// Get string
        InputID = 0;
        pStr = &m_GenericString[0][0][0];
        GetStringInput(pStr, &InputID);             // Get the newly edit string

        STR_strnstrip(pStr, strlen(pStr));          // Strip all trailing space

        if(InputID == MISC_SETTING_SERIAL_NUMBER)
        {
            memcpy(&m_GenericString[1][0][0], pStr, VT100_STRING_SZ);
            Refresh            |= VT100_MISC_CFG_REFRESH_SERIAL;
            m_NewConfigFlag[0] |= VT100_MISC_CFG_REFRESH_SERIAL;
        }
        else if(InputID == MISC_SETTING_LOCATION)
        {
            memcpy(&m_GenericString[2][0][0], pStr, VT100_STRING_SZ);
            Refresh            |= VT100_MISC_CFG_REFRESH_LOCATION;
            m_NewConfigFlag[0] |= VT100_MISC_CFG_REFRESH_LOCATION;
        }
        else if(InputID == MISC_SETTING_SITE_ID)
        {
            memcpy(&m_GenericString[3][0][0], pStr, VT100_STRING_SZ);
            Refresh            |= VT100_MISC_CFG_REFRESH_SITE_ID;
            m_NewConfigFlag[0] |= VT100_MISC_CFG_REFRESH_SITE_ID;
        }
        else if((InputID == 0) && (Input == DOOR_CTRL_MENU_TITLE) && (m_NewConfigFlag[0] == 0))
        {
            // SYS_Read(SYS_SERIAL_NUMBER, MAIN_ACU, 0, &m_GenericString[1][0][0], &Size);
            m_GenericString[1][0][Size] = '\0';
            // SYS_Read(SYS_LOCATION,      MAIN_ACU, 0, &m_GenericString[2][0][0], &Size);
            m_GenericString[2][0][Size] = '\0';
            // SYS_Read(SYS_SITE_ID,       MAIN_ACU, 0, &m_GenericString[3][0][0], &Size);
            m_GenericString[3][0][Size] = '\0';
        }

        /// Get decimal
        InputID = 0;
        GetDecimalInputValue(&EditedValue, &InputID);

        if(InputID == MISC_SETTING_TEMP_LOW_SET_POINT)
        {
            TemperatureAlarmLow  = (int16_t)EditedValue;
            Refresh             |= VT100_MISC_CFG_REFRESH_SET_POINT_LOW;
            m_NewConfigFlag[0]  |= VT100_MISC_CFG_REFRESH_SET_POINT_LOW;
        }
        else if(InputID == MISC_SETTING_TEMP_HIGH_SET_POINT)
        {
            TemperatureAlarmHigh  = (int16_t)EditedValue;
            Refresh              |= VT100_MISC_CFG_REFRESH_SET_POINT_HIGH;
            m_NewConfigFlag[0]   |= VT100_MISC_CFG_REFRESH_SET_POINT_HIGH;
        }
        else if(InputID == MISC_SETTING_BATTERY_BACKUP_RTC_LOW_VOLTAGE)
        {
            BackupBatteryLowVoltage = (uint16_t)EditedValue;
            Refresh                |= VT100_MISC_CFG_REFRESH_SET_POINT_BACKUP_LOW_VOLTAGE;
            m_NewConfigFlag[0]     |= VT100_MISC_CFG_REFRESH_SET_POINT_BACKUP_LOW_VOLTAGE;
        }
        else if(InputID == MISC_SETTING_BATTERY_POWER_LOW_VOLTAGE)
        {
            PowerBatteryLowVoltage  = (uint16_t)EditedValue;
            Refresh                |= VT100_MISC_CFG_REFRESH_SET_POINT_POWER_LOW_VOLTAGE;
            m_NewConfigFlag[0]     |= VT100_MISC_CFG_REFRESH_SET_POINT_POWER_LOW_VOLTAGE;
        }
        else if((InputID == 0) && (Input == MISC_SETTING_MENU_TITLE) && (m_NewConfigFlag[0] == 0))
        {
            // SYS_Read(SYS_TEMP_ALARM_LEVEL_LOW,       MAIN_ACU, 0, &TemperatureAlarmLow,     NULL);
            // SYS_Read(SYS_TEMP_ALARM_LEVEL_HIGH,      MAIN_ACU, 0, &TemperatureAlarmHigh,    NULL);
            // SYS_Read(SYS_BACKUP_BATT_LOW_VOLT_THOLD, MAIN_ACU, 0, &BackupBatteryLowVoltage, NULL);
            // SYS_Read(SYS_BATT_LOW_VOLTAGE_THRESHOLD, MAIN_ACU, 0, &PowerBatteryLowVoltage,  NULL);
        }
        VT100_BackFromEdition = false;
    }

    //--------------------------------------------------------------------------------------------
    // Refresh part of display


    if(Refresh != 0)
    {
        PosY = 17;

      #ifdef CONSOLE_USE_COLOR
        PrintSaveLabel(9, PosY, (m_NewConfigFlag[0] != 0) ? VT100_COLOR_YELLOW : VT100_COLOR_BLUE);
      #else
        PrintSaveLabel(9, PosY);
      #endif
    }

    if(((Refresh & VT100_MISC_CFG_REFRESH_SET_POINT_LOW) != 0) || ((Refresh & VT100_MISC_CFG_REFRESH_SET_POINT_HIGH) != 0))
    {
      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_CYAN);
      #endif

        if((Refresh & VT100_MISC_CFG_REFRESH_SET_POINT_LOW) != 0)
        {
            SetCursorPosition(46, 24);
            InMenuPrintf(VT100_SZ_NONE, "%ld.%d", TemperatureAlarmLow / 10,  abs(TemperatureAlarmLow % 10));
        }

        if((Refresh & VT100_MISC_CFG_REFRESH_SET_POINT_HIGH) != 0)
        {
            SetCursorPosition(46, 25);
            InMenuPrintf(VT100_SZ_NONE, "%ld.%d", TemperatureAlarmHigh / 10, abs(TemperatureAlarmHigh % 10));
        }

        if((Refresh & VT100_MISC_CFG_REFRESH_SET_POINT_BACKUP_LOW_VOLTAGE) != 0)
        {
            SetCursorPosition(46, 26);
            InMenuPrintf(VT100_SZ_NONE, "%ld.%03d", BackupBatteryLowVoltage / 1000,  abs(BackupBatteryLowVoltage % 1000));
        }

        if((Refresh & VT100_MISC_CFG_REFRESH_SET_POINT_POWER_LOW_VOLTAGE) != 0)
        {
            SetCursorPosition(46, 27);
            InMenuPrintf(VT100_SZ_NONE, "%ld.%03d", PowerBatteryLowVoltage / 1000, abs(PowerBatteryLowVoltage % 1000));
        }

        PosY = 28;

        if((Refresh & VT100_MISC_CFG_REFRESH_SERIAL) != 0)
        {
            SetCursorPosition(46, PosY);
            InMenuPrintf(VT100_STRING_SZ, &m_GenericString[1][0][0]);
        }

        if((Refresh & VT100_MISC_CFG_REFRESH_LOCATION) != 0)
        {
            SetCursorPosition(46, PosY + 1);
            InMenuPrintf(VT100_STRING_SZ, &m_GenericString[2][0][0]);
        }

        if((Refresh & VT100_MISC_CFG_REFRESH_SITE_ID) != 0)
        {
            SetCursorPosition(46, PosY + 2);
            InMenuPrintf(VT100_STRING_SZ, &m_GenericString[3][0][0]);
        }
    }

    return VT100_INPUT_MENU_CHOICE;
}


/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Menu Time and Date Configuration
  *
  *--------------------------------------------------------------------------------------------------------------------
  */
VT100_InputType_e VT100_Terminal::CALL_TimeDateCfg(uint8_t Input, VT100_CallBackType_e Type)
{
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
      #ifdef CONSOLE_USE_COLOR
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

      #ifdef CONSOLE_USE_COLOR
        PrintSaveLabel(9, 16, (m_NewConfigFlag[0] == 1) ? VT100_COLOR_YELLOW : VT100_COLOR_BLUE);
      #else
        PrintSaveLabel(9, 16);
      #endif

        // ********************************************
        // Refresh information display on configuration

      #ifdef CONSOLE_USE_COLOR
        SetForeColor(VT100_COLOR_CYAN);
      #endif
        SetCursorPosition(26, 24);
        InMenuPrintf(VT100_SZ_NONE, "%u:%02u:%02u ", TimeDate.hour, TimeDate.minute, TimeDate.second);
        SetCursorPosition(26, 25);
        InMenuPrintf(VT100_SZ_NONE, "%s %u, %u ", RTC_MonthName[TimeDate.month - 1], TimeDate.day, TimeDate.year);
     }

    return VT100_INPUT_MENU_CHOICE;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)

