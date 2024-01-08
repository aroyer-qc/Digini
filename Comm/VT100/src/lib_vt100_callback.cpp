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
//       VT100_CALLBACK_INIT     -> Initialize part of the display in the menu, or variables in use
//                                  while in menu.
//
//                                  It is called only once when then menu is being draw item by
//                                  item.
//
//       VT100_CALLBACK_ON_INPUT -> This call is made when a key that select an item in the menu is
//                                  pressed.
//
//                                  If it is used for a display page with static or dynamic
//                                  informations, you need to return with a VT100_INPUT_ESCAPE.
//
//                                  If it is used for a modification in the actual menu, you only
//                                  perform the action and then return with the VT100_INPUT_MENU_CHOICE
//                                  to wait for a new input.
//
//       VT100_CALLBACK_REFRESH  -> Called while waiting for any key to be pressed. Use this to
//                                  refresh displayed information.
//                                  NOTE: There is no VT100_CALLBACK_REFRESH called if
//                                        VT100_INPUT_MENU_CHOICE is returned. UNLESS: you put a
//                                        callback in Option 0 of the menu list
//
//       VT100_CALLBACK_FLUSH    -> If variables need to be reset or function need to be called to
//                                  reset some app state. each existing callback will be called
//                                  leaving a menu.
//                                  Do not use any malloc function in the process of
//                                  VT100_CALLBACK_INIT/VT100_CALLBACK_FLUSH unless you manage that it
//                                  might already exist.
//
// return VT100_InputType_e     Kind of input management for this page
//
//
//       VT100_INPUT_ESCAPE      -> It is not a menu, it his a page with information updated on
//                                  screen, this kind of page is not waiting any key other than
//                                  <ESCAPE> to exit
//
//       VT100_INPUT_SAVE_DATA   -> This value inform the myVT100 of the save has being processed.
//
//       VT100_INPUT_MENU_CHOICE -> It tell the console it is a menu and waiting for an input.
//
//       VT100_INPUT_DECIMAL     -> If the callback make call for an VT100_SetDecimalInput(), then
//                                  it must return with this enum value to tell the console what
//                                  to do. It will comeback to the callback only when the input
//                                  decimal mode will be escaped by <ESCAPE> or value validated by
//                                  <ENTER>.
//
//       VT100_INPUT_STRING      -> If the callback make call for an VT100_SetStringInput(), then it
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

#define VT100_STRING_SIZE                                     80

#define VT100_NEW_LANGUAGE                                    0
#define VT100_ACTUAL_LANGUAGE                                 1

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

static nOS_Time                 VT100_LastUpTime;
static uint8_t                  VT100_LastSecond;
static CON_DebugLevel_e         VT100_LastDebugLevel;
static uint8_t*                 pBuffer1 = nullptr;
static uint8_t*                 pBuffer2 = nullptr;

//-------------------------------------------------------------------------------------------------
// Private(s) prototype(s)
//-------------------------------------------------------------------------------------------------

Language_e VT100_DisplayLanguageSelection(Language_e Language, bool StateInit = false);

//-------------------------------------------------------------------------------------------------
// Private(s) function(s)
//-------------------------------------------------------------------------------------------------


Language_e VT100_DisplayLanguageSelection(Language_e Language, bool StateInit)
{
    myVT100.SetForeColor(VT100_COLOR_CYAN);

    if(StateInit == false)
    {
        Language = (Language == LANG_ENGLISH) ? LANG_FRENCH : LANG_ENGLISH;
    }

    if(Language == LANG_ENGLISH)
    {
        myVT100.InMenuPrintf(36, 10, LBL_STRING, "*");
        myVT100.InMenuPrintf(50, 10, LBL_STRING, " ");
    }
    else
    {
        myVT100.InMenuPrintf(36, 10, LBL_STRING, " ");
        myVT100.InMenuPrintf(50, 10, LBL_STRING, "*");
    }

    return Language;
}

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
            myVT100.InMenuPrintf(1, 5, VT100_LBL_STACKTISTIC);

            for(int i = 0; i < NbOfStack; i++)
            {
                uint8_t OffsetMultiplierX = uint8_t(((i % 4) * 26) + 2);
                OffsetMultiplierY         = uint8_t(((i / 4) * 6) + 7);

                myVT100.InMenuPrintf(OffsetMultiplierX--, OffsetMultiplierY++, LBL_STRING, myStacktistic.GetStackName(i));
                myVT100.DrawBox(OffsetMultiplierX, OffsetMultiplierY, 22, 3, VT100_COLOR_WHITE);
            }

            myVT100.InMenuPrintf(VT100_LBL_ESCAPE);
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            for(int i = 0; i < NbOfStack; i++)
            {
                uint8_t OffsetMultiplierX =  uint8_t(((i % 4) * 26) + 2);
                uint8_t OffsetMultiplierY = uint8_t((i / 4) * 6);

                Percent = myStacktistic.GetPercent(i);
                myVT100.Bargraph(OffsetMultiplierX, OffsetMultiplierY + 9, (Percent >= 90) ? VT100_COLOR_RED : VT100_COLOR_GREEN, Percent, 100, 20);
                myVT100.SetForeColor(VT100_COLOR_WHITE);
                myVT100.InMenuPrintf(OffsetMultiplierX, OffsetMultiplierY + 11, VT100_LBL_PERCENT_VALUE, Percent);
            }
        }
        break;

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
    TempUnit_e      Unit;
    // type         Temperature;

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

            myVT100.InMenuPrintf(1, 6,  LBL_SYSTEM_INFO);
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
             DB_Central.Get(pBuffer, SYSTEM_SERIAL_NUMBER);
             myVT100.InMenuPrintf(LBL_STRING, pBuffer);
             pMemoryPool->Free((void**)&pBuffer);

          #endif


            myVT100.InMenuPrintf(1, 15, LBL_COMPILE_DATE_INFO);
            myVT100.InMenuPrintf(       LBL_BUILT_DATE);
            myVT100.InMenuPrintf(1, 16, VT100_LBL_NOW);
            myVT100.InMenuPrintf(1, 17, VT100_LBL_UPTIME);
            myVT100.InMenuPrintf(1, 19, VT100_LBL_ESCAPE);
            myVT100.InMenuPrintf(1, 20, LBL_CPU_VOLTAGE);
            myVT100.InMenuPrintf(1, 21, LBL_CPU_TEMPERATURE);
            DB_Central.Get(&Unit, SYSTEM_TEMPERATURE_UNIT);
            myVT100.InMenuPrintf(20, 21, (Unit == TEMP_CELSIUS) ? LBL_DEGREE_CELSIUS : LBL_DEGREE_FAHRENHEIT);
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            UpTime = nOS_GetTickCount() / NOS_CONFIG_TICKS_PER_SECOND;

            if(TimeDate.Time.Second != VT100_LastSecond)
            {
                VT100_LastSecond = TimeDate.Time.Second;
                myVT100.DisplayTimeDateStamp(19, 16, &TimeDate);
            }

            // Get the temperature of the CPU form the ADC class
            DB_Central.Get(&Unit, SYSTEM_TEMPERATURE_UNIT);

            if(Unit == TEMP_CELSIUS) ; // Temperature = Read ADC in Celsius
            else ;                     // Temperature = Read ADC in Fahrenheit

            myVT100.InMenuPrintf(21, 14, LBL_STRING, "25.5");       // TODO replace by the right method


            if(UpTime != VT100_LastUpTime)
            {
                VT100_LastUpTime = UpTime;
                myVT100.InMenuPrintf(19, 17, LBL_LONG_UNSIGNED_SEMICOLON,    (uint32_t)(UpTime / TIME_SECONDS_PER_DAY));
                UpTime %= TIME_SECONDS_PER_DAY;
                myVT100.InMenuPrintf(        LBL_UNSIGNED_2_DIGIT_SEMICOLON, (uint16_t)(UpTime / TIME_SECONDS_PER_HOUR));
                UpTime %= TIME_SECONDS_PER_HOUR;
                myVT100.InMenuPrintf(        LBL_UNSIGNED_2_DIGIT_SEMICOLON, (uint16_t)(UpTime / TIME_SECONDS_PER_MINUTE));
                UpTime %= TIME_SECONDS_PER_MINUTE;
                myVT100.InMenuPrintf(        LBL_UNSIGNED_2_DIGIT,           (uint16_t)UpTime);
            }
        }
        break;

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
            myVT100.SetCursorPosition(41, 10 + i);

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
    static DateAndTime_t TimeDate;
   // uint32_t             Refresh;
    uint32_t             EditedValue;        // if come back from decimal input
    uint8_t              InputID;            // contain the value from this input ID

   // Refresh = VT100_CFG_NO_REFRESH;

    if(Type == VT100_CALLBACK_INIT)         // Menu Redraw
    {
        /// Print the box
        myVT100.DrawBox(8, 23, 47, 4, VT100_COLOR_GREEN);

        /// Print the static info in the box
        myVT100.SetForeColor(VT100_COLOR_YELLOW);
        myVT100.InMenuPrintf(13, 40, LBL_TIME);
        myVT100.InMenuPrintf(13, 41, LBL_DATE);
    //    Refresh   = VT100_CFG_REFRESH_ALL;
    }

    if(Type == VT100_CALLBACK_ON_INPUT)
    {
        switch(Input)
        {
            case 1: // Input Hour
            {
                myVT100.SetDecimalInput(32, 12, 0, 23, TimeDate.Time.Hour, 1, Input, LBL_HOUR);
                return VT100_INPUT_DECIMAL;
            }

            case 2: // Edit Minute
            {
                myVT100.SetDecimalInput(32, 12, 0, 59, TimeDate.Time.Minute, 1, Input, LBL_MINUTE);
                return VT100_INPUT_DECIMAL;
            }

            case 3: // Edit Second
            {
                myVT100.SetDecimalInput(32, 12, 0, 59, TimeDate.Time.Second, 1, Input, LBL_SECOND);
                return VT100_INPUT_DECIMAL;
            }

            case 4: // Edit Day
            {
                // TODO used Digini time date method
                myVT100.SetDecimalInput(32, 12, 1, nOS_TimeGetDaysPerMonth(TimeDate.Date.Month, TimeDate.Date.Year), TimeDate.Date.Day, 1, Input, LBL_DAY);
                return VT100_INPUT_DECIMAL;
            }

            case 5: // Edit Month
            {
                myVT100.SetDecimalInput(32, 12, 1, 12, TimeDate.Date.Month, 1, Input, LBL_MONTH);
                return VT100_INPUT_DECIMAL;
            }

            case 6: // Edit Year
            {
                myVT100.SetDecimalInput(32, 12, 2000, 2255, TimeDate.Date.Year, 1, Input, LBL_YEAR);
                return VT100_INPUT_DECIMAL;
            }

            case 7:
            {
                if(myVT100.GetConfigFlag(0) != 0)
                {
                    myVT100.SetConfigFlag(0, 0);
                    LIB_SetDateAndTime(&TimeDate);
                }

               // Refresh = VT100_CFG_REFRESH_INFO;
            }
            break;
        }
    }

    //--------------------------------------------------------------------------------------------
    // Refresh all information on the page according to flag
    //--------------------------------------------------------------------------------------------

    if(Type == VT100_CALLBACK_INIT)
    {
        myVT100.GetDecimalInputValue(&EditedValue, &InputID);

        if     (InputID == 1)   TimeDate.Time.Hour   = (uint8_t)EditedValue;
        else if(InputID == 2)   TimeDate.Time.Minute = (uint8_t)EditedValue;
        else if(InputID == 3)   TimeDate.Time.Second = (uint8_t)EditedValue;
        else if(InputID == 4)   TimeDate.Date.Day    = (uint8_t)EditedValue;
        else if(InputID == 5)   TimeDate.Date.Month  = (uint8_t)EditedValue;
        else if(InputID == 6)   TimeDate.Date.Year   = (uint16_t)EditedValue;
        else if((InputID == 0) && (myVT100.GetConfigFlag(0) == 0))
        {
            LIB_GetDateAndTime(&TimeDate);
        }

        if((InputID >= 1) && (InputID <= 6))
        {
            myVT100.SetConfigFlag(0, 1);
        }
    }

    ///--------------------------------------------------------------------------------------------

   // if(Refresh & VT100_CFG_REFRESH_INFO)
    {
        // ***********************************************
        // Refresh label on the menu for what is available

      #if (VT100_USE_COLOR == DEF_ENABLED)
   //     if(myVT100.GetConfigFlag(0) == 1)
        {
            myVT100.UpdateSaveLabel(VT100_COLOR_YELLOW);
        }
      #endif

        // ********************************************
        // Refresh information display on configuration

        myVT100.SetForeColor(VT100_COLOR_CYAN);
        myVT100.InMenuPrintf(26, 40, LBL_TIME, TimeDate.Time.Hour, TimeDate.Time.Minute, TimeDate.Time.Second);
        myVT100.InMenuPrintf(26, 41, LBL_DATE, myLabel.GetPointer(Label_e((TimeDate.Date.Month - 1) + (int(LBL_JANUARY)))), TimeDate.Date.Day, TimeDate.Date.Year);
    }

    return VT100_INPUT_MENU_CHOICE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_ProductInformation
//
//  Description:    Product Information
//
//  Note(s):        TODO Alain Royer use language !!!!!
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_DEBUG_SDCARD_INFO_ON_VT100 == DEF_ENABLED)
VT100_InputType_e VT100_Terminal::CALLBACK_SD_CardInformation(uint8_t Input, VT100_CallBackType_e Type)
{
    // TODO in future we should use input as menu choice.. so if we have more than one SD.. we can select which one to displayed.
    VAR_UNUSED(Input);

    switch(Type)
    {
        case VT100_CALLBACK_REFRESH_ONCE:
        {
            FRESULT   Result;
            char*     pBuffer;
            Label_e   Label;
            FATFS*    FatFs;
            uint32_t  FreeCluster;
            uint32_t  FreeBytes;
            uint32_t  TotalBytes;
            uint8_t   MMC_Type;
            SD_CID_t* pCID;
            SD_CSD_t* pCSD;
            SD_SCR_t* pSCR;
            uint32_t  CardCapacity;

            pBuffer = (char*)pMemoryPool->Alloc(VT100_STRING_SIZE);
            FatFs   = (FATFS*)pMemoryPool->AllocAndClear(sizeof(FATFS));      // Get work area for the volume

            do
            {
                Result = f_mount(FatFs, "", 0);
                nOS_Sleep(10);
            }
            while(Result != FR_OK);

            // Get information of SD Card
            f_getfree("", &FreeCluster, &FatFs);

            // Get total bytes and free  bytes
            TotalBytes = ((FatFs->n_fatent - 2) * FatFs->csize) / 2;
            FreeBytes = (FreeCluster * FatFs->csize) / 2;

            disk_initialize(FatFs->pdrv);
            disk_ioctl(FatFs->pdrv, MMC_GET_TYPE, &MMC_Type);
            disk_ioctl(FatFs->pdrv, GET_CID_STRUCT, &pCID);
            disk_ioctl(FatFs->pdrv, GET_CSD_STRUCT, &pCSD);
            disk_ioctl(FatFs->pdrv, GET_SCR_STRUCT, &pSCR);
            disk_ioctl(FatFs->pdrv, GET_CARD_CAPACITY, &CardCapacity);


            myVT100.ClearScreenWindow(0, 4, 80, 30);

            myVT100.InMenuPrintf(1,  8,  LBL_SD_CARD_TYPE);
            myVT100.InMenuPrintf(50, 8,  LBL_SD_SPEC_VER);
            //myVT100.InMenuPrintf(1,  9,  LBL_STRING, "High Speed Type??:");
            myVT100.InMenuPrintf(50, 9,  LBL_SD_MAX_SPEED);
            myVT100.InMenuPrintf(1,  10, LBL_SD_MANUFACTURER_ID);
            myVT100.InMenuPrintf(50, 10, LBL_SD_OEM_ID);
            myVT100.InMenuPrintf(1,  11, LBL_SD_PRODUCT_NAME);
            myVT100.InMenuPrintf(50, 11, LBL_SD_PRODUCT_REVISION);
            myVT100.InMenuPrintf(1,  12, LBL_SD_SERIAL_NUMBER);
            myVT100.InMenuPrintf(50, 12, LBL_SD_MANUFACTURING_DATE);
            myVT100.InMenuPrintf(1,  13, LBL_SD_CARD_SIZE);
            myVT100.InMenuPrintf(50, 13, LBL_SD_FLASH_ERASE_SIZE);
            myVT100.InMenuPrintf(1,  14, LBL_SD_ERASE_SINGLE_BLOCK);
            myVT100.InMenuPrintf(50, 14, LBL_SD_DATA_AFTER_ERASE);

            myVT100.InMenuPrintf(1,  19, LBL_SD_VOLUME_NAME);
            myVT100.InMenuPrintf(50, 19, LBL_SD_VOLUME_SN);
            myVT100.InMenuPrintf(1,  20, LBL_SD_VOLUME_TYPE);
            myVT100.InMenuPrintf(50, 20, LBL_SD_CAPACITY);
            myVT100.InMenuPrintf(1,  21, LBL_SD_USED_SPACE);
            myVT100.InMenuPrintf(50, 21, LBL_SD_FREE_SPACE);
            myVT100.InMenuPrintf(1,  22, LBL_SD_CLUSTER_SIZE);
            myVT100.InMenuPrintf(50, 22, LBL_SD_SECTOR_PER_CLUSTER);
            myVT100.InMenuPrintf(1,  23, LBL_SD_CLUSTER_COUNT);
            myVT100.InMenuPrintf(50, 23, LBL_SD_FREE_CLUSTER_COUNT);
            myVT100.InMenuPrintf(1,  24, LBL_SD_FAT_START_SECTOR);
            myVT100.InMenuPrintf(50, 24, LBL_SD_DATA_START_SECTOR);
            myVT100.InMenuPrintf(        VT100_LBL_ESCAPE);

//            myVT100.InMenuPrintf(52, 22, LBL_STRING, "Number Of Files:");


            myVT100.SetForeColor(VT100_COLOR_GREEN);
            myVT100.InMenuPrintf(1,  6,  VT100_LBL_SD_CARD_INFORMATION);
            myVT100.InMenuPrintf(1,  17, LBL_FAT_INFORMATION);

            myVT100.SetForeColor(VT100_COLOR_WHITE);

            switch(MMC_Type)
            {
                case SD_STD_CAPACITY_V1_1:  Label = LBL_SD_STD_CAPACITY_V1_1;   break;
                case SD_STD_CAPACITY_V2_0:  Label = LBL_SD_STD_CAPACITY_V2_0;   break;
                case SD_HIGH_CAPACITY:      Label = LBL_SD_HIGH_CAPACITY;       break;
                default:                    Label = LBL_SD_UNDEFINED_D;         break;
            }
            myVT100.InMenuPrintf(26, 8, Label, MMC_Type);

            myVT100.InMenuPrintf(80, 8, LBL_INT, int(pCSD->SysSpecVersion));

            switch(pCSD->MaxBusClkFrec)
            {
                case 0x32:  Label = LBL_SD_25MHZ;        break;
                case 0x5A:  Label = LBL_SD_50MHZ;        break;
                case 0x0B:  Label = LBL_SD_100MBITS;     break;
                case 0x2B:  Label = LBL_SD_100MBITS;     break;
                default:    Label = LBL_SD_UNDEFINED_2X; break;
            }
            myVT100.InMenuPrintf(80, 9,  Label, pCSD->MaxBusClkFrec);                                                           // Max Speed

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%02X", pCID->ManufacturerID);
            myVT100.InMenuPrintf(26, 10, LBL_STRING, pBuffer);                                                                  // Manufacturer ID

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%04X", pCID->OEM_AppliID);
            myVT100.InMenuPrintf(80, 10, LBL_STRING, pBuffer);                                                                  // OEM ID

            myVT100.InMenuPrintf(26, 11, LBL_STRING, pCID->ProductName);                                                        // Product

            snprintf(pBuffer, VT100_STRING_SIZE, "%d.%d", pCID->ProductRev >> 4, pCID->ProductRev & 0x0F);
            myVT100.InMenuPrintf(80, 11, LBL_STRING, pBuffer);                                                                  // Revision

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%08lX", pCID->ProductSN);
            myVT100.InMenuPrintf(26, 12, LBL_STRING, pBuffer);

            snprintf(pBuffer, VT100_STRING_SIZE, "%u/20%2u", pCID->ManufacturingDate & 0x000F, pCID->ManufacturingDate >> 4);   // Manufacturing Date
            myVT100.InMenuPrintf(80, 12, LBL_STRING, pBuffer);

            // Card Capacity
            if(CardCapacity >= 1000000)
            {
                myVT100.InMenuPrintf(26, 13, LBL_SIZE_GIGABYTES, uint16_t(CardCapacity / 1000000), uint16_t((CardCapacity % 1000000) / 1000));                                                                  // Card Capacity
            }
            else
            {
                myVT100.InMenuPrintf(26, 13, LBL_SIZE_MEGABYTES, uint16_t(CardCapacity / 1000), uint16_t(CardCapacity % 1000));
            }

            myVT100.InMenuPrintf(80, 13, LBL_SIZE_KILOBYTES, ((pCSD->EraseGrMul + 1) * BLOCK_SIZE) / 1024);                       // Flash Erase Size
            Label = pCSD->EraseGrSize ? LBL_YES : LBL_NO;
            myVT100.InMenuPrintf(26, 14, Label);                                                                                // Erase Single Block

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%02X", ((pSCR->Array[0] & SD_VALUE_AFTER_ERASE) ? 0xFF : 0x00));
            myVT100.InMenuPrintf(80, 14, LBL_STRING, pBuffer);                                                                  // Data After Erase

            // Fat information

            uint32_t VolumeSN;
            f_getlabel("", pBuffer, &VolumeSN);                                                                                 // Volume Name
            if(strlen(pBuffer) == 0)
            {
                myVT100.SetForeColor(VT100_COLOR_RED);
                myVT100.InMenuPrintf(26, 19, LBL_SD_EMPTY);
                myVT100.SetForeColor(VT100_COLOR_WHITE);
            }
            else
            {
                myVT100.InMenuPrintf(26, 19, LBL_STRING, pBuffer);
            }

            snprintf(pBuffer, VT100_STRING_SIZE, "%04X-%04X", uint16_t(VolumeSN >> 16), uint16_t(VolumeSN));                    // Volume Serial Number
            myVT100.InMenuPrintf(80, 19, LBL_STRING, pBuffer);

            switch(FatFs->fs_type)
            {
                case FS_FAT12:  memcpy(pBuffer, "FAT12", 6); break;
                case FS_FAT16:  memcpy(pBuffer, "FAT16", 6); break;
                case FS_FAT32:  memcpy(pBuffer, "FAT32", 6); break;
                case FS_EXFAT:  memcpy(pBuffer, "exFAT", 6); break;
            }
            myVT100.InMenuPrintf(26, 20, LBL_STRING, pBuffer);                                                                  // Volume Type
            myVT100.InMenuPrintf(80, 20, LBL_SD_KB_TOTAL, TotalBytes);                                                          // FAT Capacity
            myVT100.InMenuPrintf(26, 21, LBL_SD_KB_USED, TotalBytes - FreeBytes);                                               // Used Sector
            myVT100.InMenuPrintf(80, 21, LBL_SD_KB_AVAILABLE, FreeBytes);                                                       // Free Sector
            myVT100.InMenuPrintf(26, 22, LBL_SIZE_BYTES, uint32_t(FatFs->csize * BLOCK_SIZE));                                  // Cluster Size
            myVT100.InMenuPrintf(80, 22, LBL_SD_SECTORS, FatFs->csize);                                                                  // Sector Per Cluster
            myVT100.InMenuPrintf(80, 23, LBL_SD_SECTORS, (TotalBytes * 1024) / (FatFs->csize * BLOCK_SIZE));                                                                  // Cluster Count
            myVT100.InMenuPrintf(26, 23, LBL_SD_SECTORS, (FreeBytes * 1024) / (FatFs->csize * BLOCK_SIZE));                                                                  // Free Cluster Count

//test
myVT100.InMenuPrintf(1, 23, VT100_LBL_SCROLL_ZONE, 30, 40);

{
    FRESULT res;
    DIR dir;
    char string[300];

    myVT100.SetCursorPosition(1, 28);

    res = f_opendir(&dir, "");

    if (res == FR_OK)
    {
        while(1)
        {
            FILINFO fno;

            res = f_readdir(&dir, &fno);

            if ((res != FR_OK) || (fno.fname[0] == 0))  break;

            sprintf(string, "%c%c%c%c %10d %s/%s\n", ((fno.fattrib & AM_DIR) ? 'D' : '-'),
                                                     ((fno.fattrib & AM_RDO) ? 'R' : '-'),
                                                     ((fno.fattrib & AM_SYS) ? 'S' : '-'),
                                                     ((fno.fattrib & AM_HID) ? 'H' : '-'),
                                                     (int)fno.fsize, "", fno.fname);

            myVT100.InMenuPrintf(LBL_STRING, string);
        }
    }
}
            pMemoryPool->Free((void**)&pBuffer);
            pMemoryPool->Free((void**)&FatFs);
            f_mount(nullptr, "", 0);
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
        }
        break;

        case VT100_CALLBACK_FLUSH:
        {
            myVT100.InMenuPrintf(1, 23, VT100_LBL_SCROLL_ZONE, 1, 100);
        }

        // case VT100_CALLBACK_INIT:     Nothing to do
        // case VT100_CALLBACK_ON_INPUT: Nothing to do
        // case VT100_CALLBACK_FLUSH:    Nothing to do
        default: break;
    }

    return VT100_INPUT_ESCAPE;
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_MiscStatistic
//
//  Description:    Miscelleaneous statistic
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_MiscStatistic(uint8_t Input, VT100_CallBackType_e Type)
{
    uint32_t Max;
    uint8_t OffsetMultiplierX;
    uint8_t OffsetMultiplierY;

    VAR_UNUSED(Input);
    Max = pMemoryPool->GetNumberOfPool();

    switch(Type)
    {
        case VT100_CALLBACK_REFRESH_ONCE:
        {
            myVT100.SetForeColor(VT100_COLOR_WHITE);
            myVT100.InMenuPrintf(1, 5, VT100_LBL_MISC_STAT);

            myVT100.InMenuPrintf(1,  8,  VT100_LBL_MEMORY_POOL);
            myVT100.InMenuPrintf(4,  10, VT100_LBL_MEMORY_POOL_TOTAL);
            myVT100.InMenuPrintf(24, 10, LBL_SIZE_BYTES, pMemoryPool->GetTotalSizeReserved());
            myVT100.InMenuPrintf(50, 10, VT100_LBL_MEMORY_POOL_USED);
            myVT100.InMenuPrintf(4,  11, VT100_LBL_MEMORY_POOL_NB_OF_POOL, Max);

            for(uint32_t i = 0; i < Max; i++)
            {
                OffsetMultiplierX = uint8_t(((i % 3) * 33) + 2);
                OffsetMultiplierY = uint8_t(((i / 3) * 6) + 14);

                myVT100.DrawBox(OffsetMultiplierX, OffsetMultiplierY, 32, 3, VT100_COLOR_WHITE);
                myVT100.InMenuPrintf(OffsetMultiplierX--, OffsetMultiplierY - 1, VT100_LBL_MEM_POOL_GROUP, i, pMemoryPool->GetPoolNumberOfBlock(i), pMemoryPool->GetPoolBlockSize(i));
            }

            myVT100.InMenuPrintf(1, 25, VT100_LBL_LINE_SEPARATOR);
            myVT100.InMenuPrintf(1, 27, VT100_LBL_FONT_TERMINAL);
            uint8_t y = 29;

            for(uint32_t i = 128; i < 256; i++)
            {
                if((i % 32) == 0)
                {
                    myVT100.InMenuPrintf(1, y, LBL_CHAR, ASCII_LINE_FEED);
                    myVT100.SetCursorPosition(4, y++);
                }

                myVT100.InMenuPrintf(LBL_CHAR, i);
            }
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            myVT100.InMenuPrintf(68, 10, LBL_SIZE_BYTES, pMemoryPool->GetUsedMemory());

            for(uint32_t i = 0; i < Max; i++)
            {
                uint8_t NumberOfBlock = pMemoryPool->GetPoolNumberOfBlock(i);
                uint8_t UsedBlock     = pMemoryPool->GetPoolBlockUsed(i);

                OffsetMultiplierX = uint8_t(((i % 3) * 33) + 3);
                OffsetMultiplierY = uint8_t(((i / 3) * 6) + 15);

                myVT100.Bargraph(OffsetMultiplierX, OffsetMultiplierY, ((UsedBlock >= NumberOfBlock * 8) / 10) ? VT100_COLOR_RED : VT100_COLOR_GREEN, UsedBlock, NumberOfBlock, 30);
                myVT100.SetForeColor(VT100_COLOR_WHITE);
                myVT100.InMenuPrintf(OffsetMultiplierX - 1,  OffsetMultiplierY + 2, VT100_LBL_MEM_BLOCK_USED,  UsedBlock, pMemoryPool->GetPoolBlockHighPoint(i));
            }
        }
        break;

        default: break;
    }

    return VT100_INPUT_ESCAPE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_SystemSetting
//
//  Description:    System Setting
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_SystemSetting(uint8_t Input, VT100_CallBackType_e Type)
{
    static Language_e* pLanguage = nullptr;

    switch(Type)
    {
        case VT100_CALLBACK_INIT:
        {
            pLanguage = (Language_e*)pMemoryPool->Alloc(sizeof(Language_e) * 2);
            if(pLanguage != nullptr)
            {
                DB_Central.Get(&pLanguage[VT100_ACTUAL_LANGUAGE], SYSTEM_LANGUAGE);
                pLanguage[VT100_NEW_LANGUAGE] = pLanguage[VT100_ACTUAL_LANGUAGE];
                VT100_DisplayLanguageSelection(pLanguage[VT100_ACTUAL_LANGUAGE], true);
            }

            pBuffer1 = (uint8_t*)pMemoryPool->Alloc(sizeof(OEM_SERIAL_NUMBER));         // To get a new serial number
            pBuffer2 = (uint8_t*)pMemoryPool->Alloc(sizeof(OEM_SERIAL_NUMBER));         // use to compare Serial number
            if((pBuffer1 != nullptr) && (pBuffer2 != nullptr))
            {
                DB_Central.Get(&pBuffer1, SYSTEM_SERIAL_NUMBER);
                DB_Central.Get(&pBuffer2, SYSTEM_SERIAL_NUMBER);
            }

        }
        break;

        case VT100_CALLBACK_FLUSH:
        {
            pMemoryPool->Free((void**)&pBuffer1);
            pMemoryPool->Free((void**)&pBuffer2);
            pMemoryPool->Free((void**)&pLanguage);
        }
        break;

        case VT100_CALLBACK_ON_INPUT:
        {
            switch(int(MenuSystemSetting_ItemID_e(Input)))
            {
                case int(MenuSystemSetting_ID_SYSTEM_LANGUAGE):          // Language selection
                {
                    if(pLanguage != nullptr)
                    {
                        // Do toggle according to language
                        pLanguage[VT100_NEW_LANGUAGE] = VT100_DisplayLanguageSelection(pLanguage[VT100_NEW_LANGUAGE]);
                        myVT100.UpdateSaveLabel(VT100_COLOR_YELLOW);
                    }
                }
                break;

                case int(MenuSystemSetting_ID_MISC_SERIAL_NUMBER):       // Entering NEW Serial number (will be disable if a valid serial numer exist)
                {
                    if(pBuffer1 != nullptr)
                    {
                        myVT100.SetStringInput(32, 16, sizeof(OEM_SERIAL_NUMBER), Input, LBL_SERIAL_NUMBER, (const char*)pBuffer1);
                        return VT100_INPUT_STRING;
                    }
                }
                break;

                case int(MenuSystemSetting_ID_MISC_SAVE):
                {
                    if(pLanguage != nullptr)
                    {
                        if(pLanguage[VT100_NEW_LANGUAGE] != pLanguage[VT100_ACTUAL_LANGUAGE])
                        {
                            DB_Central.Set(&pLanguage[VT100_NEW_LANGUAGE], SYSTEM_LANGUAGE);
                            myLabel.SetLanguage(pLanguage[VT100_NEW_LANGUAGE]);
                            myVT100.SetRefreshFullPage();
                          #if (DIGINI_USE_GRAFX == DEF_ENABLED)
                            GUI_pTask->SetForceRefresh();                                                       // Force the graphic page to also be refresh to new language
                          #endif
                        }
                    }

                    if((pBuffer1 != nullptr) && (pBuffer2 != nullptr))
                    {
                        if(memcmp(pBuffer1, pBuffer2, sizeof(OEM_SERIAL_NUMBER)) != 0)
                        {
                            DB_Central.Set(&pBuffer1, SYSTEM_SERIAL_NUMBER);
                        }
                    }

                    return VT100_INPUT_SAVE_DATA;
                }
                break;
            }
        }
        break;

        default: break;
    }

    return VT100_INPUT_ESCAPE;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_NetworkInfo
//
//  Description:    Menu information about network
//
//  Note(s):        Should move this into as part of Digini
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_ETHERNET == DEF_ENABLED)
VT100_InputType_e VT100_Terminal::CALLBACK_NetworkInfo(uint8_t Input, VT100_CallBackType_e Type)
{
    VAR_UNUSED(Input);

    switch(Type)
    {
        case VT100_CALLBACK_INIT:
        {
            myVT100.SetForeColor(VT100_COLOR_WHITE);
            myVT100.InMenuPrintf(1, 5, LBL_NETWORK_INFO);

            myVT100.SetForeColor(VT100_COLOR_WHITE);
            myVT100.InMenuPrintf(2,  8,  LBL_IP_ADDR);
            myVT100.InMenuPrintf(2,  9,  LBL_IP_MASK);
            myVT100.InMenuPrintf(2,  10, LBL_IP_GATEWAY);
            myVT100.InMenuPrintf(2,  11, LBL_IP_DNS);
            myVT100.InMenuPrintf(2,  13, LBL_IP_DHCP_STATE);
            myVT100.InMenuPrintf(2,  14, LBL_IP_LINK_STATE);
            myVT100.InMenuPrintf(2,  15, LBL_IP_LINK_SPEED);
            myVT100.InMenuPrintf(2,  16, LBL_MAC_ADDRESS);
          #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
            myVT100.InMenuPrintf(2,  18, LBL_ETH_RX_COUNT);
            myVT100.InMenuPrintf(40, 18, LBL_ETH_DROP);
            myVT100.InMenuPrintf(2,  19, LBL_ETH_TX_COUNT);
            myVT100.InMenuPrintf(40, 19, LBL_ETH_DROP);
          #endif
            myVT100.InMenuPrintf(        VT100_LBL_ESCAPE);

            // Add Lease obtain and expire???
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            struct netif*    pNetif = netif_find(IF_NAME);
            ETH_LinkInfo_t   LinkInfo;
            ETH_LinkState_e  LinkState;
            const char*      pSpeed;

            myVT100.SetForeColor(VT100_COLOR_WHITE);
            myVT100.InMenuPrintf(28, 8,  LBL_STRING, ip_ntoa(&pNetif->ip_addr));
            myVT100.InMenuPrintf(28, 9,  LBL_STRING, ip_ntoa(&pNetif->netmask));
            myVT100.InMenuPrintf(28, 10, LBL_STRING, ip_ntoa(&pNetif->gw));
            myVT100.InMenuPrintf(28, 11, LBL_STRING, ip_ntoa(dns_getserver(0)));
            myVT100.InMenuPrintf(28, 12, LBL_STRING, ip_ntoa(dns_getserver(1)));
            myVT100.InMenuPrintf(28, 13, (ETH_DHCP_IsUsed == true) != 0 ? LBL_ENABLED : LBL_DISABLED);
            LinkState = myETH_PHY->GetLinkState();
            myVT100.InMenuPrintf(28, 14, LBL_STRING, (LinkState == ETH_LINK_UP) != 0 ? "Up  " : "Down");

            LinkInfo = myETH_PHY->GetLinkInfo();

            switch(LinkInfo.Speed)
            {
                case ETH_PHY_SPEED_10M:     pSpeed = "10  Mb/Sec";  break;
                case ETH_PHY_SPEED_100M:    pSpeed = "100 Mb/Sec";  break;
                case ETH_PHY_SPEED_1G:      pSpeed = "1   Gb/Sec";  break;
            }

            myVT100.InMenuPrintf(28, 15, LBL_STRING, pSpeed);
            myVT100.InMenuPrintf(28, 16, LBL_MAC_ADDRESS_VALUE, pNetif->hwaddr[0], pNetif->hwaddr[1], pNetif->hwaddr[2],
                                                                pNetif->hwaddr[3], pNetif->hwaddr[4], pNetif->hwaddr[5]);

          #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
            myVT100.InMenuPrintf(28, 18, LBL_LONG_UNSIGNED, DBG_RX_Count);
            myVT100.InMenuPrintf(56, 18, LBL_LONG_UNSIGNED, DBG_RX_Drop);
            myVT100.InMenuPrintf(28, 19, LBL_LONG_UNSIGNED, DBG_TX_Count);
            myVT100.InMenuPrintf(56, 19, LBL_LONG_UNSIGNED, DBG_TX_Drop);
          #endif
        }
        break;

        default:
            break;
    }

    return VT100_INPUT_ESCAPE;
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)
