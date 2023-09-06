///-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100_callback.cpp
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
//
//  Note: User CLI Command Function: This is the user space for the user CLI command set.
//
//-------------------------------------------------------------------------------------------------
//
// For all callback function here:
//
// Parameter Type
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

static nOS_Time                 VT100_LastUpTime;
static uint8_t                  VT100_LastSecond;

static CON_DebugLevel_e         VT100_LastDebugLevel;

//-------------------------------------------------------------------------------------------------
// Public(s) function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_None
//
//  Description:    Dummy Callback
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
//  Name:           CALLBACK_StackUsage
//
//  Description:    Display page for stack usage
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_StackUsage(uint8_t Input, VT100_CallBackType_e Type)
{
    static int  NbOfStack = 0;
    int32_t     Percent;

    VAR_UNUSED(Input);

    switch(Type)
    {
        case VT100_CALLBACK_INIT:
        {
            NbOfStack = myStacktistic.GetNumberOfRegisterStack();
        }
        break;

        case VT100_CALLBACK_REFRESH_ONCE:
        {
            uint8_t OffsetMultiplierY;

            myVT100.SetForeColor(VT100_COLOR_WHITE);
            myVT100.InMenuPrintf(1, 5, LBL_VT100_STACKTISTIC);

            for(int i = 0; i < NbOfStack; i++)
            {
                uint8_t OffsetMultiplierX = uint8_t(((i % 4) * 26) + 2);
                OffsetMultiplierY         = uint8_t(((i / 4) * 6) + 7);

                myVT100.InMenuPrintf(OffsetMultiplierX--, OffsetMultiplierY++, LBL_STRING, myStacktistic.GetStackName(i));
                myVT100.InMenuPrintf(OffsetMultiplierX,   OffsetMultiplierY++, VT100_LBL_BAR_BOX_20_LINE_1);
                myVT100.InMenuPrintf(OffsetMultiplierX,   OffsetMultiplierY++, VT100_LBL_BAR_BOX_20_LINE_2);
                myVT100.InMenuPrintf(OffsetMultiplierX,   OffsetMultiplierY++, VT100_LBL_BAR_BOX_20_LINE_3);
            }

            myVT100.InMenuPrintf(0, OffsetMultiplierY + 1, VT100_LBL_ESCAPE);
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            for(int i = 0; i < NbOfStack; i++)
            {
                uint8_t OffsetMultiplierX =  uint8_t(((i % 4) * 26) + 2);
                uint8_t OffsetMultiplierY = uint8_t((i / 4) * 6);

                Percent = myStacktistic.GetPercent(i);
              #if (VT100_USE_COLOR == DEF_ENABLED)
                myVT100.Bargraph(OffsetMultiplierX, OffsetMultiplierY + 9, (Percent >= 90) ? VT100_COLOR_RED : VT100_COLOR_GREEN, Percent, 100, 20);
              #else
                myVT100.Bargraph(OffsetMultiplierX, OffsetMultiplierY + 9, Percent, 100, 20);
              #endif
                myVT100.SetForeColor(VT100_COLOR_WHITE);
                myVT100.InMenuPrintf(OffsetMultiplierX, OffsetMultiplierY + 11, LBL_VT100_LBL_PERCENT_VALUE, Percent);
            }
        }
        break;

        // case VT100_CALLBACK_ON_INPUT: Nothing to do
        // case VT100_CALLBACK_FLUSH:    Nothing to do
        default: break;
    }

    return VT100_INPUT_ESCAPE;
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
    nOS_Time        UpTime;
    DateAndTime_t   TimeDate;

    VAR_UNUSED(Input);

    switch(Type)
    {
        case VT100_CALLBACK_INIT:
        {
            VT100_LastSecond = 60;
            VT100_LastUpTime = 0;
        }
        break;

        case VT100_CALLBACK_REFRESH_ONCE:
        {
            myVT100.ClearScreenWindow(0, 4, 80, 30);

            myVT100.InMenuPrintf(1, 6,  LBL_VT100_SYSTEM_INFO);
            myVT100.InMenuPrintf(1, 8,  LBL_VENDOR_NAME_INFO);
            myVT100.InMenuPrintf(       LBL_VENDOR_NAME);
            myVT100.InMenuPrintf(1, 9,  LBL_HARDWARE_INFO);
            myVT100.InMenuPrintf(       LBL_MODEL_NAME);
            myVT100.InMenuPrintf(1, 10, LBL_FW_NAME_INFO);
            myVT100.InMenuPrintf(       LBL_FIRMWARE_NAME);
            myVT100.InMenuPrintf(1, 11, LBL_FW_VERSION_INFO);
            myVT100.InMenuPrintf(       LBL_FIRMWARE_VERSION);
            myVT100.InMenuPrintf(1, 12, LBL_GUI_NAME_INFO);
            myVT100.InMenuPrintf(       LBL_GUI_NAME);
            myVT100.InMenuPrintf(1, 13, LBL_GUI_VERSION_INFO);
            myVT100.InMenuPrintf(       LBL_GUI_VERSION);
            myVT100.InMenuPrintf(1, 14, LBL_SERIAL_INFO);
          #ifdef DEBUG
            myVT100.InMenuPrintf(       LBL_SERIAL_NUMBER);
          #else
            char* pBuffer = (char*)pMemoryPool->Alloc(SERIAL_NUMBER_SIZE);
             DB_Central.Get(pBuffer, SYS_SERIAL_NUMBER);
             InMenuPrintf(LBL_STRING, pBuffer);
          #endif
            myVT100.InMenuPrintf(1, 15, LBL_COMPILE_DATE_INFO);
            myVT100.InMenuPrintf(       LBL_BUILT_DATE);
            myVT100.InMenuPrintf(1, 16, VT100_LBL_NOW);
            myVT100.InMenuPrintf(1, 17, VT100_LBL_UPTIME);
            myVT100.InMenuPrintf(0, 19, VT100_LBL_ESCAPE);
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            UpTime = nOS_GetTickCount() / NOS_CONFIG_TICKS_PER_SECOND;

            LIB_GetDateAndTime(&TimeDate);

            if(TimeDate.Time.Second != VT100_LastSecond)
            {
                VT100_LastSecond = TimeDate.Time.Second;
                myVT100.InMenuPrintf(19, 16, VT100_LBL_FULL_DATE, myLabel.GetPointer(Label_e((LIB_GetDayOfWeek(&TimeDate.Date)) + (int(LBL_FIRST_WEEK_DAY)))),
                                                                  myLabel.GetPointer(Label_e((TimeDate.Date.Month - 1) + (int(LBL_FIRST_MONTH)))),
                                                                  TimeDate.Date.Day,
                                                                  TimeDate.Date.Year,
                                                                  TimeDate.Time.Hour,
                                                                  TimeDate.Time.Minute,
                                                                  TimeDate.Time.Second);
            }

            if(UpTime != VT100_LastUpTime)
            {
                VT100_LastUpTime = UpTime;
                myVT100.InMenuPrintf(19, 17, VT100_LBL_LONG_UNSIGNED_SEMICOLON,    (uint32_t)(UpTime / TIME_SECONDS_PER_DAY));
                UpTime %= TIME_SECONDS_PER_DAY;
                myVT100.InMenuPrintf(        VT100_LBL_UNSIGNED_2_DIGIT_SEMICOLON, (uint16_t)(UpTime / TIME_SECONDS_PER_HOUR));
                UpTime %= TIME_SECONDS_PER_HOUR;
                myVT100.InMenuPrintf(        VT100_LBL_UNSIGNED_2_DIGIT_SEMICOLON, (uint16_t)(UpTime / TIME_SECONDS_PER_MINUTE));
                UpTime %= TIME_SECONDS_PER_MINUTE;
                myVT100.InMenuPrintf(        VT100_LBL_UNSIGNED_2_DIGIT,           (uint16_t)UpTime);
            }
        }
        break;

        // case VT100_CALLBACK_ON_INPUT: Nothing to do
        // case VT100_CALLBACK_FLUSH:    Nothing to do
        default: break;
    }

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

    // Update VT100_DebugLevel only on item 0 if it is VT100_CALLBACK_REFRESH type

    if(Type == VT100_CALLBACK_REFRESH)
    {
        DB_Central.Get(&DebugLevel, SYS_DEBUG_LEVEL, 0, 0);
        VT100_LastDebugLevel = DebugLevel;

        myVT100.SaveAttribute();
        myVT100.SetForeColor(VT100_COLOR_MAGENTA);

        myVT100.SaveCursorPosition();

        for(uint16_t i = 0; i < CON_NUMBER_OF_DEBUG_LEVEL; i++)
        {
            myVT100.SetCursorPosition(41, 14 + i);

            if(((1 << i) & uint16_t(DebugLevel)) != 0)
            {
                myVT100.InMenuPrintf(LBL_STRING, " ");
            }
            else
            {
                myVT100.InMenuPrintf(LBL_STRING, "*");
            }
        }

        myVT100.RestoreAttribute();
        myVT100.RestoreCursorPosition();
    }

    if(Type == VT100_CALLBACK_ON_INPUT)
    {
        uint16_t DebugValue = (1 << uint16_t(Input - 1));

        if((DebugLevel & DebugValue) == 0)
        {
            DebugLevel = CON_DebugLevel_e(int(DebugLevel) | DebugValue);
        }
        else
        {
            DebugLevel = CON_DebugLevel_e(int(DebugLevel) & ~(DebugValue));
        }
    }

    if(VT100_LastDebugLevel != DebugLevel)
    {
        DB_Central.Set(&DebugLevel, SYS_DEBUG_LEVEL);           // Write a Record in backup RAM
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
    static nOS_TimeDate TimeDate;
    uint32_t            Refresh;
    uint32_t            EditedValue;        // if come back from decimal input
    uint8_t             InputID;            // contain the value from this input ID

    Refresh = VT100_CFG_NO_REFRESH;

    if(Type == VT100_CALLBACK_INIT)         // Menu Redraw
    {
        /// Print the box
        myVT100.DrawBox(8, 23, 47, 4, VT100_COLOR_GREEN);

        /// Print the static info in the box
        myVT100.SetForeColor(VT100_COLOR_YELLOW);
        myVT100.InMenuPrintf(13, 24, LBL_TIME);
        myVT100.InMenuPrintf(13, 25, LBL_DATE);
        Refresh   = VT100_CFG_REFRESH_ALL;
    }

    if(Type == VT100_CALLBACK_ON_INPUT)
    {
        switch(Input)
        {
            case 1: // Input Hour
            {
                myVT100.SetDecimalInput(32, 12, 0, 23, TimeDate.hour, 1, Input, LBL_HOUR);
                return VT100_INPUT_DECIMAL;
            }

            case 2: // Edit Minute
            {
                myVT100.SetDecimalInput(32, 12, 0, 59, TimeDate.minute, 1, Input, LBL_MINUTE);
                return VT100_INPUT_DECIMAL;
            }

            case 3: // Edit Second
            {
                myVT100.SetDecimalInput(32, 12, 0, 59, TimeDate.second, 1, Input, LBL_SECOND);
                return VT100_INPUT_DECIMAL;
            }

            case 4: // Edit Day
            {
                // TODO used Digini time date method
                myVT100.SetDecimalInput(32, 12, 1, nOS_TimeGetDaysPerMonth(TimeDate.month, TimeDate.year), TimeDate.day, 1, Input, LBL_DAY);
                return VT100_INPUT_DECIMAL;
            }

            case 5: // Edit Month
            {
                myVT100.SetDecimalInput(32, 12, 1, 12, TimeDate.month, 1, Input, LBL_MONTH);
                return VT100_INPUT_DECIMAL;
            }

            case 6: // Edit Year
            {
                myVT100.SetDecimalInput(32, 12, 2000, 2255, TimeDate.year, 1, Input, LBL_YEAR);
                return VT100_INPUT_DECIMAL;
            }

            case 7:
            {
                if(myVT100.GetConfigFlag(0) != 0)
                {
                    myVT100.SetConfigFlag(0, 0);
                   // RTC_DateAndTime(&TimeDate, STATE_SET);
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
        myVT100.GetDecimalInputValue(&EditedValue, &InputID);

        if     (InputID == 1)   TimeDate.hour   = (uint8_t)EditedValue;
        else if(InputID == 2)   TimeDate.minute = (uint8_t)EditedValue;
        else if(InputID == 3)   TimeDate.second = (uint8_t)EditedValue;
        else if(InputID == 4)   TimeDate.day    = (uint8_t)EditedValue;
        else if(InputID == 5)   TimeDate.month  = (uint8_t)EditedValue;
        else if(InputID == 6)   TimeDate.year   = (uint16_t)EditedValue;
        else if((InputID == 0) && (myVT100.GetConfigFlag(0) == 0))
        {
           //RTC_DateAndTime(&TimeDate, STATE_GET);
        }

        if((InputID >= 1) && (InputID <= 6))
        {
            myVT100.SetConfigFlag(0, 1);
        }
    }

    ///--------------------------------------------------------------------------------------------

    if(Refresh & VT100_CFG_REFRESH_INFO)
    {
        // ***********************************************
        // Refresh label on the menu for what is available

      #if (VT100_USE_COLOR == DEF_ENABLED)
        myVT100.PrintSaveLabel(9, 16, (myVT100.GetConfigFlag(0) == 1) ? VT100_COLOR_YELLOW : VT100_COLOR_BLUE);
      #else
        myVT100.PrintSaveLabel(9, 16);
      #endif

        // ********************************************
        // Refresh information display on configuration

        myVT100.SetForeColor(VT100_COLOR_CYAN);
        myVT100.InMenuPrintf(26, 24, VT100_LBL_TIME, TimeDate.hour, TimeDate.minute, TimeDate.second);
        myVT100.InMenuPrintf(26, 25, VT100_LBL_DATE, myLabel.GetPointer(Label_e((TimeDate.month - 1) + (int(LBL_FIRST_MONTH)))),
                                                                                       TimeDate.day,
                                                                                       TimeDate.year);



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
#if (DIGINI_DEBUG_SDCARD_INFO_ON_VT100 == DEF_ENABLED)
VT100_InputType_e VT100_Terminal::CALLBACK_SD_CardInformation(uint8_t Input, VT100_CallBackType_e Type)
{
    VAR_UNUSED(Input);

    switch(Type)
    {
        case VT100_CALLBACK_REFRESH_ONCE:
        {
            FRESULT   Result;
            char      str[80];
            FATFS*    FatFs;
            uint32_t  FreeCluster;
            uint32_t  FreeBytes;
            uint32_t  TotalBytes;
            uint8_t   MMC_Type;
            SD_CID_t* pCID;
            SD_CSD_t* pCSD;
            uint32_t  CardCapacity;

            FatFs = (FATFS*)pMemoryPool->AllocAndClear(sizeof(FATFS));      // Get work area for the volume

            do
            {
                Result = f_mount(FatFs, "", 0);
                nOS_Sleep(10);
            }
            while(Result != FR_OK);

            // Get information of SD Card
            f_getfree("", &FreeCluster, &FatFs);

            // Get total bytes and free  bytes
            TotalBytes = ((FatFs->n_fatent - 2) * FatFs->csize) /2;
            FreeBytes = (FreeCluster * FatFs->csize) / 2;

            disk_initialize(FatFs->pdrv);
            disk_ioctl(FatFs->pdrv, MMC_GET_TYPE, &MMC_Type);
            disk_ioctl(FatFs->pdrv, GET_CID_STRUCT, &pCID);
            disk_ioctl(FatFs->pdrv, GET_CSD_STRUCT, &pCSD);
            disk_ioctl(FatFs->pdrv, GET_CARD_CAPACITY, &CardCapacity);


            myVT100.ClearScreenWindow(0, 4, 80, 30);

            myVT100.InMenuPrintf(1,  6,  LBL_VT100_SD_CARD_INFORMATION);
            myVT100.InMenuPrintf(1,  8,  LBL_STRING, "Card Type:");
            myVT100.InMenuPrintf(50, 8,  LBL_STRING, "Sd Spec Version:");
            myVT100.InMenuPrintf(1,  9,  LBL_STRING, "High Speed Type??:");
            myVT100.InMenuPrintf(50, 9,  LBL_STRING, "Max Speed:");
            myVT100.InMenuPrintf(1,  10, LBL_STRING, "Speed Class:");
            myVT100.InMenuPrintf(1,  11, LBL_STRING, "Manufacturer ID:");
            myVT100.InMenuPrintf(50, 11, LBL_STRING, "OEM ID:");
            myVT100.InMenuPrintf(1,  12, LBL_STRING, "Product:");
            myVT100.InMenuPrintf(50, 12, LBL_STRING, "Revision:");
            myVT100.InMenuPrintf(1,  13, LBL_STRING, "Serial Number:");
            myVT100.InMenuPrintf(50, 13, LBL_STRING, "Manufacturing date:");
            myVT100.InMenuPrintf(1,  15, LBL_STRING, "Card Size:");
            myVT100.InMenuPrintf(50, 15, LBL_STRING, "Flash Erase Size:");
            myVT100.InMenuPrintf(1,  16, LBL_STRING, "Erase Single Block:");
            myVT100.InMenuPrintf(50, 16, LBL_STRING, "Data After Erase:");

            myVT100.InMenuPrintf(4,  19, LBL_STRING, "Fat Information");

            myVT100.InMenuPrintf(1,  21, LBL_STRING, "Volume:");
            myVT100.InMenuPrintf(50, 21, LBL_STRING, "Volume Name:");
            myVT100.InMenuPrintf(1,  22, LBL_STRING, "Capacity:");
            myVT100.InMenuPrintf(1,  23, LBL_STRING, "Used Space:");
            myVT100.InMenuPrintf(50, 23, LBL_STRING, "Free Space:");
            myVT100.InMenuPrintf(50, 24, LBL_STRING, "Number Of Files:");

            myVT100.InMenuPrintf(1,  26, LBL_STRING, "Sector Per Cluster:");
            myVT100.InMenuPrintf(50, 26, LBL_STRING, "Cluster Count:");
            myVT100.InMenuPrintf(1,  27, LBL_STRING, "Free Cluster Count:");
            myVT100.InMenuPrintf(1,  28, LBL_STRING, "Fat Start Sector:");
            myVT100.InMenuPrintf(50, 28, LBL_STRING, "Data Start Sector:");

            myVT100.InMenuPrintf(0,  30, VT100_LBL_ESCAPE);

            myVT100.SetForeColor(VT100_COLOR_WHITE);

            switch(MMC_Type)
            {
                case SD_STD_CAPACITY_V1_1:  memcpy(str, "Standard Capacity V1.1", 23);   break;
                case SD_STD_CAPACITY_V2_0:  memcpy(str, "Standard Capacity V2.0", 23);   break;
                case SD_HIGH_CAPACITY:      memcpy(str, "High Capacity", 14);            break;
                default:                    snprintf(str, 80, "Undefined %d", MMC_Type); break;
            }
            myVT100.InMenuPrintf(25, 8,  LBL_STRING, str);

            snprintf(str, 80, "%d", int(pCSD->SysSpecVersion));
            myVT100.InMenuPrintf(75, 8,  LBL_STRING, str);


            switch(pCSD->MaxBusClkFrec)
            {
                case 0x32:  memcpy(str, "25 MHz", 7);         break;
                case 0x5A:  memcpy(str, "50 MHz", 7);         break;
                case 0x0B:  memcpy(str, "100 Mbits/sec", 14); break;
                case 0x2B:  memcpy(str, "200 Mbits/sec", 14); break;
                default:    snprintf(str, 80, "Undefined %2X", pCSD->MaxBusClkFrec); break;
            }
            myVT100.InMenuPrintf(75, 9,  LBL_STRING, str);                                                  // Max Speed

            snprintf(str, 80, "%d", pCSD->WrSpeedFact);
            myVT100.InMenuPrintf(1,  10, LBL_STRING, str);                                                  // Speed Class

            snprintf(str, 80, "0x%02X", pCID->ManufacturerID);
            myVT100.InMenuPrintf(25,  11, LBL_STRING, str);                                                 // Manufacturer ID

            snprintf(str, 80, "0x%04X", pCID->OEM_AppliID);
            myVT100.InMenuPrintf(75, 11, LBL_STRING, str);                                                  // OEM ID

            snprintf(str, 80, "%c%c%c%c%c", pCID->OEM_AppliID);
            myVT100.InMenuPrintf(25,  12, LBL_STRING, pCID->ProductName);                                   // Product

            snprintf(str, 80, "%d.%d", pCID->ProductRev >> 4, pCID->ProductRev & 0x0F);
            myVT100.InMenuPrintf(75, 12, LBL_STRING, str);                                                  // Revision

            snprintf(str, 80, "0x%06lX", pCID->ProductSN);
            myVT100.InMenuPrintf(25, 13, LBL_STRING, str);

            snprintf(str, 80, "%u/20%2u", pCID->ManufacturingDate & 0x000F, pCID->ManufacturingDate >> 4);  // Manufacturing Date
            myVT100.InMenuPrintf(75, 13, LBL_STRING, str);

            if(CardCapacity >= 1000000)
            {
                snprintf(str, 80, "%u.%02u GBytes", uint16_t(CardCapacity / 1000000), uint16_t((CardCapacity % 1000000) / 1000));
            }
            else
            {
                snprintf(str, 80, "%u.%02 MBytes", uint16_t(CardCapacity / 1000), uint16_t(CardCapacity % 1000));
            }
            myVT100.InMenuPrintf(25, 15, LBL_STRING, str);                                                  // Card Capacity

            // Fat information

            switch(FatFs->fs_type)
            {
                case FS_FAT12:  memcpy(str, "FAT12", 6); break;
                case FS_FAT16:  memcpy(str, "FAT16", 6); break;
                case FS_FAT32:  memcpy(str, "FAT32", 6); break;
                case FS_EXFAT:  memcpy(str, "exFAT", 6); break;
            }
            myVT100.InMenuPrintf(25,  21, LBL_STRING, str);                                                 // Fat type

            f_getlabel("", str, 0);                                                                         // Drive name
            myVT100.InMenuPrintf(75, 21, LBL_STRING, str);

            snprintf(str, 80, "%lu KB Total", TotalBytes);                                                  // FAT Capacity
            myVT100.InMenuPrintf(25, 22, LBL_STRING, str);

            snprintf(str, 80, "%lu KB Used", TotalBytes - FreeBytes);                                       // Used Sector
            myVT100.InMenuPrintf(25, 23, LBL_STRING, str);

            snprintf(str, 80, "%lu KB Available", FreeBytes);                                               // Free Sector
            myVT100.InMenuPrintf(75, 23, LBL_STRING, str);



            f_mount(nullptr, "", 0);

        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
        }
        break;

        // case VT100_CALLBACK_INIT:     Nothing to do
        // case VT100_CALLBACK_ON_INPUT: Nothing to do
        // case VT100_CALLBACK_FLUSH:    Nothing to do
        default: break;
    }

    return VT100_INPUT_ESCAPE;
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)


#if 0


    do
    {
        m_FResult = f_mount(m_pFS, m_pDrive, 1);
        nOS_Sleep(10);
    }
    while(m_FResult != FR_OK);

    // Open source file on the internal drive (SPI Flash)
    m_FResult = f_open(m_pFile, m_Path, FA_OPEN_EXISTING | FA_READ);
    if((m_FResult = f_stat(m_Path, m_pFileInfo)) != FR_OK) return SYS_FAIL;
    m_TotalToLoad = f_size(m_pFile);

    do
    {
        if((m_FResult = f_lseek(m_pFile, NextBlock)) != FR_OK) return SYS_FAIL; // Jump to next block (first is zero)

        if((State = Get_uint32_t(&NextBlock)) != SYS_READY) return State;       // Get now the next block
        if((State = Get_uint16_t(&BlockTp))   != SYS_READY) return State;       // Get the Block type
        BlockType = SkinBlockType_e(BlockTp);

        switch(BlockType)
        {
            case SKIN_IMAGE:
            {
                m_pCompressionMethod = (uint8_t*)GRAFX_DECOMPRESS_METHOD_ADDRESS;
                memset(m_pCompressionMethod, 0x00, sizeof(uint8_t) * DBASE_MAX_SKIN_IMAGE_QTY);
                m_pDataSize          = (uint32_t*)GRAFX_DATA_SIZE_ADDRESS;
                memset(m_pDataSize, 0x00, sizeof(uint32_t) * DBASE_MAX_SKIN_IMAGE_QTY);

                if((State = Get_uint16_t(&m_ItemCount)) == SYS_READY)
                {
                    if((State = GetImageInfo()) == SYS_READY)
                    {
                        State = DeCompressAllImage();
                    }
                }

                break;
            }

            case SKIN_FONT:
            {
                m_pCompressionMethod = (uint8_t*)GRAFX_DECOMPRESS_METHOD_ADDRESS;
                memset(m_pCompressionMethod, 0x00, sizeof(uint8_t) * DBASE_MAX_SKIN_FONT_QTY * 256);

                if((State = Get_uint16_t(&m_ItemCount)) == SYS_READY)
                {
                    if((State = GetFontInfo()) == SYS_READY)
                    {
                        State = DeCompressAllFont();
                    }
                }

                break;
            }
        }
        //nOS_Yield();
        nOS_Sleep(1);
    }
    while((NextBlock != 0) && (State == SYS_READY));

    f_close(m_pFile);

    // Unregister work area prior to discard it
    f_mount(nullptr, m_pDrive, 0);


FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);				/* Open or create a file */
FRESULT f_close (FIL* fp);											/* Close an open file object */
FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);			/* Read data from the file */
FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to the file */
FRESULT f_lseek (FIL* fp, FSIZE_t ofs);								/* Move file pointer of the file object */
FRESULT f_truncate (FIL* fp);										/* Truncate the file */
FRESULT f_sync (FIL* fp);											/* Flush cached data of the writing file */
FRESULT f_opendir (DIR* dp, const TCHAR* path);						/* Open a directory */
FRESULT f_closedir (DIR* dp);										/* Close an open directory */
FRESULT f_readdir (DIR* dp, FILINFO* fno);							/* Read a directory item */
FRESULT f_findfirst (DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern);	/* Find first file */
FRESULT f_findnext (DIR* dp, FILINFO* fno);							/* Find next file */
FRESULT f_mkdir (const TCHAR* path);								/* Create a sub directory */
FRESULT f_unlink (const TCHAR* path);								/* Delete an existing file or directory */
FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	/* Rename/Move a file or directory */
FRESULT f_stat (const TCHAR* path, FILINFO* fno);					/* Get file status */
FRESULT f_chmod (const TCHAR* path, BYTE attr, BYTE mask);			/* Change attribute of a file/dir */
FRESULT f_utime (const TCHAR* path, const FILINFO* fno);			/* Change timestamp of a file/dir */
FRESULT f_chdir (const TCHAR* path);								/* Change current directory */
FRESULT f_chdrive (const TCHAR* path);								/* Change current drive */
FRESULT f_getcwd (TCHAR* buff, UINT len);							/* Get current directory */
FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);	/* Get number of free clusters on the drive */
FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn);	/* Get volume label */
FRESULT f_setlabel (const TCHAR* label);							/* Set volume label */
FRESULT f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
FRESULT f_expand (FIL* fp, FSIZE_t fsz, BYTE opt);					/* Allocate a contiguous block to the file */
FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt);			/* Mount/Unmount a logical drive */
FRESULT f_mkfs (const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len);	/* Create a FAT volume */
FRESULT f_fdisk (BYTE pdrv, const LBA_t ptbl[], void* work);		/* Divide a physical drive into some partitions */
FRESULT f_setcp (WORD cp);											/* Set current code page */
int f_putc (TCHAR c, FIL* fp);										/* Put a character to the file */
int f_puts (const TCHAR* str, FIL* cp);								/* Put a string to the file */
int f_printf (FIL* fp, const TCHAR* str, ...);						/* Put a formatted string to the file */
TCHAR* f_gets (TCHAR* buff, int len, FIL* fp);						/* Get a string from the file */


#endif
