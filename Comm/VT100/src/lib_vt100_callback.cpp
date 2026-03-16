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

#include "./lib_digini.h"

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

#define VT100_NEW_TEMPERATURE_SELECTION                       0
#define VT100_ACTUAL_TEMPERATURE_SELECTION                    1

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

static nOS_Time                 VT100_LastUpTime;
static uint8_t                  VT100_LastSecond;
static uint8_t*                 pBuffer1 = nullptr;
static uint8_t*                 pBuffer2 = nullptr;

#if (CON_USE_DEBUG_LOG == DEF_ENABLED)
static SystemDebugLevel_e       VT100_LastDebugLevel;
#endif

//-------------------------------------------------------------------------------------------------
// Private(s) prototype(s)
//-------------------------------------------------------------------------------------------------

Language_e VT100_DisplayLanguageSelection(Language_e Language, bool StateInit = false);
TempUnit_e VT100_DisplayTemperatureSelection(TempUnit_e Unit, bool StateInit = false);

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

TempUnit_e VT100_DisplayTemperatureSelection(TempUnit_e Unit, bool StateInit)
{
    myVT100.SetForeColor(VT100_COLOR_CYAN);

    if(StateInit == false)
    {
        Unit = (Unit == TEMP_CELSIUS) ? TEMP_FAHRENHEIT : TEMP_CELSIUS;
    }

    if(Unit == TEMP_CELSIUS)
    {
        myVT100.InMenuPrintf(36, 12, LBL_STRING, "*");
        myVT100.InMenuPrintf(50, 12, LBL_STRING, " ");
    }
    else
    {
        myVT100.InMenuPrintf(36, 12, LBL_STRING, " ");
        myVT100.InMenuPrintf(50, 12, LBL_STRING, "*");
    }

    return Unit;
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
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
VT100_InputType_e VT100_Terminal::CALLBACK_StackUsage(uint8_t Input, VT100_CallBackType_e Type)
{
    static int  NbOfStack = 0;
    int32_t     Percent;
    size_t      StackSize;
    size_t      StackUsage;
    char        String[10];
    int         PrintSize;

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

            SetForeColor(VT100_COLOR_WHITE);
            InMenuPrintf(1, 5, VT100_LBL_STACKTISTIC);

            for(int i = 0; i < NbOfStack; i++)
            {
                uint8_t OffsetMultiplierX = uint8_t(((i % 5) * 27) + 2);
                OffsetMultiplierY         = uint8_t(((i / 5) * 6) + 7);

                InMenuPrintf(OffsetMultiplierX--, OffsetMultiplierY++, LBL_STRING, myStacktistic.GetStackName(i));
                DrawBox(OffsetMultiplierX, OffsetMultiplierY, 25, 3, VT100_COLOR_WHITE);
            }

            InMenuPrintf(VT100_LBL_ESCAPE);
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            for(int i = 0; i < NbOfStack; i++)
            {
                uint8_t OffsetMultiplierX =  uint8_t(((i % 5) * 27) + 2);
                uint8_t OffsetMultiplierY = uint8_t((i / 5) * 6);

                myStacktistic.GetStat(i, &StackSize, &StackUsage, &Percent);
                Bargraph(OffsetMultiplierX, OffsetMultiplierY + 9, (Percent >= 90) ? VT100_COLOR_RED : VT100_COLOR_GREEN, Percent, VT100_COLOR_BLUE, 0, 100, 23);
                SetForeColor(VT100_COLOR_WHITE);
                InMenuPrintf(OffsetMultiplierX, OffsetMultiplierY + 11, VT100_LBL_PERCENT_VALUE, Percent);

                PrintSize = sprintf(String, "%d/%d", StackUsage, StackSize);
                InMenuPrintf((OffsetMultiplierX + 23) - PrintSize, OffsetMultiplierY + 11, LBL_STRING, String);
            }
        }
        break;

        default: break;
    }

    return VT100_INPUT_ESCAPE;
}
#endif // (DIGINI_USE_STACKTISTIC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_ProductInformation
//
//  Description:    Product Information
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_LABEL_PRODUCT_INFO == DEF_ENABLED)
VT100_InputType_e VT100_Terminal::CALLBACK_ProductInformation(uint8_t Input, VT100_CallBackType_e Type)
{
    nOS_Time        UpTime;
    DateAndTime_t   DateTime;
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
            ClearScreenWindow(0, 4, VT100_SCREEN_WIDTH, 30);

            InMenuPrintf(1, 6,  LBL_SYSTEM_INFO);

            InMenuPrintf(1, 8,  LBL_VENDOR_NAME_INFO);
            InMenuPrintf(       LBL_VENDOR_NAME);

            InMenuPrintf(1, 9,  LBL_HARDWARE_INFO);
            InMenuPrintf(       LBL_MODEL_NAME);

            InMenuPrintf(1, 10, LBL_FW_NAME_INFO);
            InMenuPrintf(       LBL_FIRMWARE_NAME);

            InMenuPrintf(1, 11, LBL_FW_VERSION_INFO);
            InMenuPrintf(       LBL_FIRMWARE_VERSION);

            InMenuPrintf(1, 12, LBL_GUI_VERSION_INFO);
            InMenuPrintf(       LBL_DIGINI_VERSION);

            InMenuPrintf(1, 13, LBL_SERIAL_INFO);
          #if defined(DEBUG) || (DIGINI_USE_DATABASE == DEF_DISABLED)
            InMenuPrintf(       LBL_SERIAL_NUMBER);
          #else
             char* pBuffer;

             if((pBuffer = (char*)pMemoryPool->Alloc(SERIAL_NUMBER_SIZE, MEM_DBG_VTCB1)) != nullptr)
             {
                 DB_Central.Get(pBuffer, SERIAL_NUMBER_TEXT);
                 InMenuPrintf(LBL_STRING, pBuffer);
                 pMemoryPool->Free((void**)&pBuffer);
             }
          #endif

            InMenuPrintf(1, 14, LBL_COMPILE_DATE_INFO);
            InMenuPrintf(       LBL_BUILT_DATE);

            InMenuPrintf(1, 15, VT100_LBL_NOW);
            InMenuPrintf(1, 16, VT100_LBL_UPTIME);

            InMenuPrintf(1, 17, LBL_CPU_VOLTAGE);

            InMenuPrintf(1, 18, LBL_CPU_TEMPERATURE);
          #if (DIGINI_USE_DATABASE == DEF_DISABLED)
            InMenuPrintf(24, 18, LBL_DEGREE_CELSIUS);
          #else
            DB_Central.Get(&Unit, SYSTEM_TEMPERATURE_UNIT);
            InMenuPrintf(24, 18, (Unit == TEMP_CELSIUS) ? LBL_DEGREE_CELSIUS : LBL_DEGREE_FAHRENHEIT);
          #endif

            InMenuPrintf(1, 20, VT100_LBL_REPEAT_CHARACTER, '-', VT100_SCREEN_WIDTH);
            InMenuPrintf(1, 22, VT100_LBL_FONT_TERMINAL);
            uint8_t y = 24;

            for(uint32_t i = 128; i < 256; i++)
            {
                if((i % 32) == 0)
                {
                    InMenuPrintf(1, y, LBL_CHAR, ASCII_LINE_FEED);
                    SetCursorPosition(4, y++);
                }

                InMenuPrintf(LBL_CHAR, i);
            }

            InMenuPrintf(1, 29, VT100_LBL_ESCAPE);
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            UpTime = nOS_GetTickCount() / NOS_CONFIG_TICKS_PER_SECOND;

            LIB_GetDateAndTime(&DateTime);

            if(DateTime.Time.Second != VT100_LastSecond)
            {
                VT100_LastSecond = DateTime.Time.Second;
                DisplayTimeDateStamp(19, 15, &DateTime);
            }

            // Get the temperature of the CPU form the ADC class
          #if (DIGINI_USE_DATABASE != DEF_DISABLED)
            DB_Central.Get(&Unit, SYSTEM_TEMPERATURE_UNIT);

            if(Unit == TEMP_CELSIUS) // Temperature = Read ADC in Celsius
            {
            }
            else
          #endif
            {

            }                     // Temperature = Read ADC in Fahrenheit

            InMenuPrintf(19, 17, LBL_STRING, "3.28V");       // TODO replace by the right method
            InMenuPrintf(19, 18, LBL_STRING, "25.5");       // TODO replace by the right method


            if(UpTime != VT100_LastUpTime)
            {
                VT100_LastUpTime = UpTime;
                InMenuPrintf(19, 16, LBL_LONG_UNSIGNED_SEMICOLON,    (uint32_t)(UpTime / TIME_SECONDS_PER_DAY));
                UpTime %= TIME_SECONDS_PER_DAY;
                InMenuPrintf(        LBL_UNSIGNED_2_DIGIT_SEMICOLON, (uint16_t)(UpTime / TIME_SECONDS_PER_HOUR));
                UpTime %= TIME_SECONDS_PER_HOUR;
                InMenuPrintf(        LBL_UNSIGNED_2_DIGIT_SEMICOLON, (uint16_t)(UpTime / TIME_SECONDS_PER_MINUTE));
                UpTime %= TIME_SECONDS_PER_MINUTE;
                InMenuPrintf(        LBL_UNSIGNED_2_DIGIT,           (uint16_t)UpTime);
            }
        }
        break;

        default: break;
    }

    return VT100_INPUT_ESCAPE;
}
#endif //  (DIGINI_USE_LABEL_PRODUCT_INFO == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_DebugLevelSetting
//
//  Description:    Configure product debug level for serial logging
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#if (CON_USE_DEBUG_LOG == DEF_ENABLED)
VT100_InputType_e VT100_Terminal::CALLBACK_DebugLevelSetting(uint8_t Input, VT100_CallBackType_e Type)
{
  #if (DIGINI_USE_DATABASE == DISABLED)
    extern SystemDebugLevel_e BSP_GlobalDebugLevel; // TODO get a better method ( automatically generated into debug file
  #endif

    static SystemDebugLevel_e DebugLevel = SYS_DEBUG_NONE;

    // Update VT100_DebugLevel only on item 0 if it is VT100_CALLBACK_REFRESH type

    if(Type == VT100_CALLBACK_REFRESH)
    {
      #if (DIGINI_USE_DATABASE == DISABLED)
        DebugLevel = BSP_GlobalDebugLevel;
      #else
        DB_Central.Get(&DebugLevel, SYSTEM_DEBUG_LEVEL, 0, 0);
      #endif
        VT100_LastDebugLevel = DebugLevel;

        SaveAttribute();
        SetForeColor(VT100_COLOR_MAGENTA);

        SaveCursorPosition();

        for(uint16_t i = 0; i < CON_NUMBER_OF_DEBUG_LEVEL; i++)
        {
            SetCursorPosition(41, 10 + i);

            if(((1 << i) & uint16_t(DebugLevel)) != 0)
            {
                InMenuPrintf(LBL_STRING, " ");
            }
            else
            {
                InMenuPrintf(LBL_STRING, "*");
            }
        }

        RestoreAttribute();
        RestoreCursorPosition();
    }

    if(Type == VT100_CALLBACK_ON_INPUT)
    {
        uint16_t DebugValue = (1 << uint16_t(Input - 1));

        if((DebugLevel & DebugValue) == 0)
        {
            DebugLevel = SystemDebugLevel_e(int(DebugLevel) | DebugValue);
        }
        else
        {
            DebugLevel = SystemDebugLevel_e(int(DebugLevel) & ~(DebugValue));
        }
    }

    if(VT100_LastDebugLevel != DebugLevel)
    {
      #if (DIGINI_USE_DATABASE == DISABLED)
        BSP_GlobalDebugLevel = DebugLevel;
      #else
        DB_Central.Set(&DebugLevel, SYSTEM_DEBUG_LEVEL);           // Write a Record in backup RAM
      #endif
    }

    return VT100_INPUT_MENU_CHOICE;
}
#endif

/*
//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_DateTimeCfg
//
//  Description:    Menu Date and Time Configuration.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CALLBACK_DateTimeCfg(uint8_t Input, VT100_CallBackType_e Type)
{
    static DateAndTime_t TimeDate;
   // uint32_t             Refresh;
    uint32_t             EditedValue;        // if come back from decimal input
    uint8_t              InputID;            // contain the value from this input ID

   // Refresh = VT100_CFG_NO_REFRESH;

    if(Type == VT100_CALLBACK_INIT)         // Menu Redraw
    {
        /// Print the box
        DrawBox(8, 23, 47, 4, VT100_COLOR_GREEN);

        /// Print the static info in the box
        SetForeColor(VT100_COLOR_YELLOW);
        InMenuPrintf(13, 40, LBL_TIME);
        InMenuPrintf(13, 41, LBL_DATE);
    //    Refresh   = VT100_CFG_REFRESH_ALL;
    }

    if(Type == VT100_CALLBACK_ON_INPUT)
    {
        switch(Input)
        {
            case 1: // Input Hour
            {
                SetDecimalInput(32, 12, 0, 23, TimeDate.Time.Hour, 1, Input, LBL_HOUR);
                return VT100_INPUT_DECIMAL;
            }

            case 2: // Edit Minute
            {
                SetDecimalInput(32, 12, 0, 59, TimeDate.Time.Minute, 1, Input, LBL_MINUTE);
                return VT100_INPUT_DECIMAL;
            }

            case 3: // Edit Second
            {
                SetDecimalInput(32, 12, 0, 59, TimeDate.Time.Second, 1, Input, LBL_SECOND);
                return VT100_INPUT_DECIMAL;
            }

            case 4: // Edit Day
            {
                // TODO used Digini time date method
                SetDecimalInput(32, 12, 1, nOS_TimeGetDaysPerMonth(TimeDate.Date.Month, TimeDate.Date.Year), TimeDate.Date.Day, 1, Input, LBL_DAY);
                return VT100_INPUT_DECIMAL;
            }

            case 5: // Edit Month
            {
                SetDecimalInput(32, 12, 1, 12, TimeDate.Date.Month, 1, Input, LBL_MONTH);
                return VT100_INPUT_DECIMAL;
            }

            case 6: // Edit Year
            {
                SetDecimalInput(32, 12, 2000, 2255, TimeDate.Date.Year, 1, Input, LBL_YEAR);
                return VT100_INPUT_DECIMAL;
            }

            case 7:
            {
                if(GetConfigFlag(0) != 0)
                {
                    SetConfigFlag(0, 0);
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
        GetDecimalInputValue(&EditedValue, &InputID);

        if     (InputID == 1)   TimeDate.Time.Hour   = (uint8_t)EditedValue;
        else if(InputID == 2)   TimeDate.Time.Minute = (uint8_t)EditedValue;
        else if(InputID == 3)   TimeDate.Time.Second = (uint8_t)EditedValue;
        else if(InputID == 4)   TimeDate.Date.Day    = (uint8_t)EditedValue;
        else if(InputID == 5)   TimeDate.Date.Month  = (uint8_t)EditedValue;
        else if(InputID == 6)   TimeDate.Date.Year   = (uint16_t)EditedValue;
        else if((InputID == 0) && (GetConfigFlag(0) == 0))
        {
            LIB_GetDateAndTime(&TimeDate);
        }

        if((InputID >= 1) && (InputID <= 6))
        {
            SetConfigFlag(0, 1);
        }
    }

    ///--------------------------------------------------------------------------------------------

   // if(Refresh & VT100_CFG_REFRESH_INFO)
    {
        // ***********************************************
        // Refresh label on the menu for what is available

      #if (VT100_USE_COLOR == DEF_ENABLED)
   //     if(GetConfigFlag(0) == 1)
        {
            UpdateSaveLabel(VT100_COLOR_YELLOW);
        }
      #endif

        // ********************************************
        // Refresh information display on configuration

        SetForeColor(VT100_COLOR_CYAN);
        InMenuPrintf(26, 40, LBL_TIME, TimeDate.Time.Hour, TimeDate.Time.Minute, TimeDate.Time.Second);
        InMenuPrintf(26, 41, LBL_DATE, myLabel.GetPointer(Label_e((TimeDate.Date.Month - 1) + (int(LBL_JANUARY)))), TimeDate.Date.Day, TimeDate.Date.Year);
    }

    return VT100_INPUT_MENU_CHOICE;
}
*/

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

            pBuffer = (char*)pMemoryPool->Alloc(VT100_STRING_SIZE, MEM_DBG_VTCB2);
            FatFs   = (FATFS*)pMemoryPool->AllocAndClear(sizeof(FATFS), MEM_DBG_VTCB3);      // Get work area for the volume

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
            ClearScreenWindow(0, 4, VT100_SCREEN_WIDTH, 30);
            InMenuPrintf(1,  8,  LBL_SD_CARD_TYPE);
            InMenuPrintf(50, 8,  LBL_SD_SPEC_VER);
            //InMenuPrintf(1,  9,  LBL_STRING, "High Speed Type??:");
            InMenuPrintf(50, 9,  LBL_SD_MAX_SPEED);
            InMenuPrintf(1,  10, LBL_SD_MANUFACTURER_ID);
            InMenuPrintf(50, 10, LBL_SD_OEM_ID);
            InMenuPrintf(1,  11, LBL_SD_PRODUCT_NAME);
            InMenuPrintf(50, 11, LBL_SD_PRODUCT_REVISION);
            InMenuPrintf(1,  12, LBL_SD_SERIAL_NUMBER);
            InMenuPrintf(50, 12, LBL_SD_MANUFACTURING_DATE);
            InMenuPrintf(1,  13, LBL_SD_CARD_SIZE);
            InMenuPrintf(50, 13, LBL_SD_FLASH_ERASE_SIZE);
            InMenuPrintf(1,  14, LBL_SD_ERASE_SINGLE_BLOCK);
            InMenuPrintf(50, 14, LBL_SD_DATA_AFTER_ERASE);

            InMenuPrintf(1,  19, LBL_SD_VOLUME_NAME);
            InMenuPrintf(50, 19, LBL_SD_VOLUME_SN);
            InMenuPrintf(1,  20, LBL_SD_VOLUME_TYPE);
            InMenuPrintf(50, 20, LBL_SD_CAPACITY);
            InMenuPrintf(1,  21, LBL_SD_USED_SPACE);
            InMenuPrintf(50, 21, LBL_SD_FREE_SPACE);
            InMenuPrintf(1,  22, LBL_SD_CLUSTER_SIZE);
            InMenuPrintf(50, 22, LBL_SD_SECTOR_PER_CLUSTER);
            InMenuPrintf(1,  23, LBL_SD_CLUSTER_COUNT);
            InMenuPrintf(50, 23, LBL_SD_FREE_CLUSTER_COUNT);
            InMenuPrintf(1,  24, LBL_SD_FAT_START_SECTOR);
            InMenuPrintf(50, 24, LBL_SD_DATA_START_SECTOR);
            InMenuPrintf(        VT100_LBL_ESCAPE);

//            InMenuPrintf(52, 22, LBL_STRING, "Number Of Files:");


            SetForeColor(VT100_COLOR_GREEN);
            InMenuPrintf(1,  6,  LBL_SD_CARD_INFORMATION);
            InMenuPrintf(1,  17, LBL_FAT_INFORMATION);

            SetForeColor(VT100_COLOR_WHITE);

            switch(MMC_Type)
            {
                case SD_STD_CAPACITY_V1_1:  Label = LBL_SD_STD_CAPACITY_V1_1;   break;
                case SD_STD_CAPACITY_V2_0:  Label = LBL_SD_STD_CAPACITY_V2_0;   break;
                case SD_HIGH_CAPACITY:      Label = LBL_SD_HIGH_CAPACITY;       break;
                default:                    Label = LBL_SD_UNDEFINED_D;         break;
            }
            InMenuPrintf(26, 8, Label, MMC_Type);

            InMenuPrintf(80, 8, LBL_INT, int(pCSD->SysSpecVersion));

            switch(pCSD->MaxBusClkFrec)
            {
                case 0x32:  Label = LBL_SD_25MHZ;        break;
                case 0x5A:  Label = LBL_SD_50MHZ;        break;
                case 0x0B:  Label = LBL_SD_100MBITS;     break;
                case 0x2B:  Label = LBL_SD_100MBITS;     break;
                default:    Label = LBL_SD_UNDEFINED_2X; break;
            }
            InMenuPrintf(80, 9,  Label, pCSD->MaxBusClkFrec);                                                           // Max Speed

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%02X", pCID->ManufacturerID);
            InMenuPrintf(26, 10, LBL_STRING, pBuffer);                                                                  // Manufacturer ID

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%04X", pCID->OEM_AppliID);
            InMenuPrintf(80, 10, LBL_STRING, pBuffer);                                                                  // OEM ID

            InMenuPrintf(26, 11, LBL_STRING, pCID->ProductName);                                                        // Product

            snprintf(pBuffer, VT100_STRING_SIZE, "%d.%d", pCID->ProductRev >> 4, pCID->ProductRev & 0x0F);
            InMenuPrintf(80, 11, LBL_STRING, pBuffer);                                                                  // Revision

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%08lX", pCID->ProductSN);
            InMenuPrintf(26, 12, LBL_STRING, pBuffer);

            snprintf(pBuffer, VT100_STRING_SIZE, "%u/20%2u", pCID->ManufacturingDate & 0x000F, pCID->ManufacturingDate >> 4);   // Manufacturing Date
            InMenuPrintf(80, 12, LBL_STRING, pBuffer);

            // Card Capacity
            if(CardCapacity >= 1000000)
            {
                InMenuPrintf(26, 13, LBL_SIZE_GIGABYTES, uint16_t(CardCapacity / 1000000), uint16_t((CardCapacity % 1000000) / 1000));  // Card Capacity
            }
            else
            {
                InMenuPrintf(26, 13, LBL_SIZE_MEGABYTES, uint16_t(CardCapacity / 1000), uint16_t(CardCapacity % 1000));
            }

            InMenuPrintf(80, 13, LBL_SIZE_KILOBYTES, ((pCSD->EraseGrMul + 1) * BLOCK_SIZE) / 1024);                       // Flash Erase Size
            Label = pCSD->EraseGrSize ? LBL_YES : LBL_NO;
            InMenuPrintf(26, 14, Label);                                                                                // Erase Single Block

            snprintf(pBuffer, VT100_STRING_SIZE, "0x%02X", ((pSCR->Array[0] & SD_VALUE_AFTER_ERASE) ? 0xFF : 0x00));
            InMenuPrintf(80, 14, LBL_STRING, pBuffer);                                                                  // Data After Erase

            // Fat information

            uint32_t VolumeSN;
            f_getlabel("", pBuffer, &VolumeSN);                                                                                 // Volume Name
            if(strlen(pBuffer) == 0)
            {
                SetForeColor(VT100_COLOR_RED);
                InMenuPrintf(26, 19, LBL_SD_EMPTY);
                SetForeColor(VT100_COLOR_WHITE);
            }
            else
            {
                InMenuPrintf(26, 19, LBL_STRING, pBuffer);
            }

            snprintf(pBuffer, VT100_STRING_SIZE, "%04X-%04X", uint16_t(VolumeSN >> 16), uint16_t(VolumeSN));                    // Volume Serial Number
            InMenuPrintf(80, 19, LBL_STRING, pBuffer);

            switch(FatFs->fs_type)
            {
                case FS_FAT12:  memcpy(pBuffer, "FAT12", 6); break;
                case FS_FAT16:  memcpy(pBuffer, "FAT16", 6); break;
                case FS_FAT32:  memcpy(pBuffer, "FAT32", 6); break;
                case FS_EXFAT:  memcpy(pBuffer, "exFAT", 6); break;
            }
            InMenuPrintf(26, 20, LBL_STRING, pBuffer);                                                                  // Volume Type
            InMenuPrintf(80, 20, LBL_SD_KB_TOTAL, TotalBytes);                                                          // FAT Capacity
            InMenuPrintf(26, 21, LBL_SD_KB_USED, TotalBytes - FreeBytes);                                               // Used Sector
            InMenuPrintf(80, 21, LBL_SD_KB_AVAILABLE, FreeBytes);                                                       // Free Sector
            InMenuPrintf(26, 22, LBL_SIZE_BYTES, uint32_t(FatFs->csize * BLOCK_SIZE));                                  // Cluster Size
            InMenuPrintf(80, 22, LBL_SD_SECTORS, FatFs->csize);                                                         // Sector Per Cluster
            InMenuPrintf(80, 23, LBL_SD_SECTORS, (TotalBytes * 1024) / (FatFs->csize * BLOCK_SIZE));                    // Cluster Count
            InMenuPrintf(26, 23, LBL_SD_SECTORS, (FreeBytes * 1024) / (FatFs->csize * BLOCK_SIZE));                     // Free Cluster Count
            InMenuPrintf(1, 23, VT100_LBL_SCROLL_ZONE, 30, 40);

{
    FRESULT res;
    DIR dir;
    char string[300];

    SetCursorPosition(1, 28);

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

            InMenuPrintf(LBL_STRING, string);
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
            InMenuPrintf(1, 23, VT100_LBL_SCROLL_ZONE, 1, 100);
        }

        // case VT100_CALLBACK_INIT:     Nothing to do
        // case VT100_CALLBACK_ON_INPUT: Nothing to do
        default: break;
    }

    return VT100_INPUT_ESCAPE;
}
#endif


//-------------------------------------------------------------------------------------------------
//
//  Name:           CALLBACK_MemoryPool
//
//  Description:    Memory Pool statistic
//
//-------------------------------------------------------------------------------------------------
#if (MEMORY_POOL_USE_DEBUG_STAT == DEF_ENABLED)
VT100_InputType_e VT100_Terminal::CALLBACK_MemoryPool(uint8_t Input, VT100_CallBackType_e Type)
{
    uint32_t Max;
    uint8_t OffsetMultiplierX;
    uint8_t OffsetMultiplierY;
    uint8_t PercentUsed;
    uint8_t PercentMax;

    VAR_UNUSED(Input);
    Max = pMemoryPool->GetNumberOfPool();

    switch(Type)
    {
        case VT100_CALLBACK_REFRESH_ONCE:
        {
            this->SetForeColor(VT100_COLOR_WHITE);
            this->InMenuPrintf(1, 5, VT100_LBL_MEMORY_POOL_STAT);

            InMenuPrintf(1,  8,  VT100_LBL_MEMORY_POOL);
            InMenuPrintf(4,  10, VT100_LBL_MEMORY_POOL_TOTAL);
            InMenuPrintf(24, 10, LBL_SIZE_BYTES, pMemoryPool->GetTotalSizeReserved());
            InMenuPrintf(50, 10, VT100_LBL_MEMORY_POOL_USED);
            InMenuPrintf(4,  11, VT100_LBL_MEMORY_POOL_NB_OF_POOL, Max);

            for(uint32_t i = 0; i < Max; i++)
            {
                OffsetMultiplierX = uint8_t(((i % 4) * 33) + 2);
                OffsetMultiplierY = uint8_t(((i / 4) * 6) + 14);
                DrawBox(OffsetMultiplierX, OffsetMultiplierY, 32, 3, VT100_COLOR_WHITE);
                InMenuPrintf(OffsetMultiplierX--, OffsetMultiplierY - 1, VT100_LBL_MEM_POOL_GROUP, i, pMemoryPool->GetPoolNumberOfBlock(i), pMemoryPool->GetPoolBlockSize(i));
            }

            uint32_t j = pMemoryPool->GetMaxDebugID();
            OffsetMultiplierY += 6;
            InMenuPrintf(0, OffsetMultiplierY++, VT100_LBL_ALLOC_COUNTER);

            for(uint32_t i = 1; i < j; i++)
            {
                OffsetMultiplierX = uint8_t(((i % 4) * 33) + 3);
                InMenuPrintf(OffsetMultiplierX - 1, (OffsetMultiplierY + uint8_t(i / 4)), Label_e((uint32_t(LBL_MEM_DBG_NONE) - 1) + i));
            }
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            InMenuPrintf(68, 10, LBL_SIZE_BYTES, pMemoryPool->GetUsedMemory());

            for(uint32_t i = 0; i < Max; i++)
            {
                uint32_t NumberOfBlock = pMemoryPool->GetPoolNumberOfBlock(i);
                uint32_t UsedBlock     = pMemoryPool->GetPoolBlockUsed(i);
                uint32_t MaxBlock      = pMemoryPool->GetPoolBlockHighPoint(i);

                OffsetMultiplierX = uint8_t(((i % 4) * 33) + 3);
                OffsetMultiplierY = uint8_t(((i / 4) * 6) + 15);
                PercentUsed = (UsedBlock * 100) / NumberOfBlock;
                PercentMax  = (MaxBlock  * 100) / NumberOfBlock;

                Bargraph(OffsetMultiplierX, OffsetMultiplierY, (PercentUsed >= 80) ? VT100_COLOR_RED : VT100_COLOR_GREEN, PercentUsed, VT100_COLOR_YELLOW, PercentMax, NumberOfBlock, 30);
                SetForeColor(VT100_COLOR_WHITE);
                InMenuPrintf(OffsetMultiplierX - 1,  OffsetMultiplierY + 2, VT100_LBL_MEM_BLOCK_USED,  UsedBlock, MaxBlock);
            }

            uint32_t j = pMemoryPool->GetMaxDebugID();
            OffsetMultiplierY += 6;

            for(uint32_t i = 1; i < j; i++)
            {
                OffsetMultiplierX = uint8_t(((i % 4) * 34) + 25);
                InMenuPrintf(OffsetMultiplierX - 1,  (OffsetMultiplierY + uint8_t(i / 4)), VT100_LBL_ALLOC_DEBUG_COUNTER, pMemoryPool->GetAllocCount(MEM_DebugListOfID_e(i)));
            }
        }
        break;

        default: break;
    }

    return VT100_INPUT_ESCAPE;
}
#endif

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
    static TempUnit_e* pTempUnit = nullptr;
    static int         ResetCount = 0;

    switch(Type)
    {
        case VT100_CALLBACK_INIT:
        {
            ResetCount       = 0;

            pLanguage = (Language_e*)pMemoryPool->Alloc(sizeof(Language_e) * 2, MEM_DBG_VTCB4);

            if(pLanguage != nullptr)
            {
              #if (DIGINI_USE_DATABASE != DEF_DISABLED)
                DB_Central.Get(&pLanguage[VT100_ACTUAL_LANGUAGE], SYSTEM_LANGUAGE);
              #else
                pLanguage[VT100_ACTUAL_LANGUAGE] = LANG_ENGLISH;
              #endif
                pLanguage[VT100_NEW_LANGUAGE] = pLanguage[VT100_ACTUAL_LANGUAGE];
                VT100_DisplayLanguageSelection(pLanguage[VT100_ACTUAL_LANGUAGE], true);
            }

            pTempUnit = (TempUnit_e*)pMemoryPool->Alloc(sizeof(TempUnit_e) * 2, MEM_DBG_VTCB5);

            if(pTempUnit != nullptr)
            {
              #if (DIGINI_USE_DATABASE != DEF_DISABLED)
                DB_Central.Get(&pTempUnit[VT100_ACTUAL_TEMPERATURE_SELECTION], SYSTEM_TEMPERATURE_UNIT);
              #else
                pTempUnit[VT100_ACTUAL_TEMPERATURE_SELECTION] = TEMP_CELSIUS;
              #endif
                pTempUnit[VT100_NEW_TEMPERATURE_SELECTION] = pTempUnit[VT100_ACTUAL_TEMPERATURE_SELECTION];
                VT100_DisplayTemperatureSelection(pTempUnit[VT100_ACTUAL_TEMPERATURE_SELECTION], true);
            }

            pBuffer1 = (uint8_t*)pMemoryPool->Alloc(sizeof(OEM_SERIAL_NUMBER), MEM_DBG_VTCB6);         // To get a new serial number
            pBuffer2 = (uint8_t*)pMemoryPool->Alloc(sizeof(OEM_SERIAL_NUMBER), MEM_DBG_VTCB7);         // use to compare Serial number

            if((pBuffer1 != nullptr) && (pBuffer2 != nullptr))
            {
              #if (DIGINI_USE_DATABASE != DEF_DISABLED)
                DB_Central.Get(&pBuffer1, SERIAL_NUMBER_TEXT);
                DB_Central.Get(&pBuffer2, SERIAL_NUMBER_TEXT);
              #else
                // TODO
              #endif
            }
        }
        break;

        case VT100_CALLBACK_FLUSH:
        {
            pMemoryPool->Free((void**)&pBuffer1);
            pMemoryPool->Free((void**)&pBuffer2);
            pMemoryPool->Free((void**)&pLanguage);
            pMemoryPool->Free((void**)&pTempUnit);
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
                        UpdateSaveLabel(VT100_COLOR_YELLOW);
                    }
                }
                break;

                case int(MenuSystemSetting_ID_MISC_SERIAL_NUMBER):       // Entering NEW Serial number (will be disable if a valid serial numer exist)
                {
                    if(pBuffer1 != nullptr)
                    {
                        SetStringInput(32, 16, sizeof(OEM_SERIAL_NUMBER), Input, LBL_SERIAL_NUMBER, (const char*)pBuffer1);
                        return VT100_INPUT_STRING;
                    }
                }
                break;

                case int(MenuSystemSetting_ID_SYSTEM_TEMPERATURE):       // Temperature Unit selection
                {
                    if(pTempUnit != nullptr)
                    {
                        // Do toggle according to language
                        pTempUnit[VT100_NEW_TEMPERATURE_SELECTION] = VT100_DisplayTemperatureSelection(pTempUnit[VT100_NEW_TEMPERATURE_SELECTION]);
                        UpdateSaveLabel(VT100_COLOR_YELLOW);
                    }
                }
                break;

                case int(MenuSystemSetting_ID_MISC_DATE):
                {
                    if(pBuffer1 != nullptr)
                    {
                        #define DATE_SIZE  11 // 0000-00-00             // move on header when working
                        SetStringInput(32, 16, DATE_SIZE, Input, VT100_LBL_DATE, (const char*)pBuffer1);
                        return VT100_INPUT_STRING;
                    }
                }
                break;

                case int(MenuSystemSetting_ID_MISC_TIME):
                {
                    if(pBuffer1 != nullptr)
                    {
                        #define TIME_SIZE  9 // 00:00:00             // move on header when working
                        SetStringInput(32, 16, TIME_SIZE, Input, VT100_LBL_TIME, (const char*)pBuffer1);
                        return VT100_INPUT_STRING;
                    }
                }
                break;

                case int(MenuSystemSetting_ID_MISC_SYSTEM_RESET):
                {
                    ResetCount++;

                    if(ResetCount >= 3)
                    {
                        CPU_SpecificSystemReset();
                    }
                    else
                    {
                        // Print number of press using consecutive *
                        for(int i = 0; i < ResetCount; i++)
                        {
                            InMenuPrintf(24 + i, 12, LBL_STRING, "*");
                        }
                    }
                }
                break;

                case int(MenuSystemSetting_ID_MISC_SAVE):
                {
                    if(pLanguage != nullptr)
                    {
                        if(pLanguage[VT100_NEW_LANGUAGE] != pLanguage[VT100_ACTUAL_LANGUAGE])
                        {
                          #if (DIGINI_USE_DATABASE != DEF_DISABLED)
                            DB_Central.Set(&pLanguage[VT100_NEW_LANGUAGE], SYSTEM_LANGUAGE);
                          #endif
                            myLabel.SetLanguage(pLanguage[VT100_NEW_LANGUAGE]);
                            SetRefreshFullPage();
                          #if (DIGINI_USE_GRAFX == DEF_ENABLED)
                            GUI_pTask->SetForceRefresh();                                                       // Force the graphic page to also be refresh to new language
                          #endif
                        }
                    }

                    if(pTempUnit != nullptr)
                    {
                        if(pTempUnit[VT100_NEW_TEMPERATURE_SELECTION] != pTempUnit[VT100_ACTUAL_TEMPERATURE_SELECTION])
                        {
                          #if (DIGINI_USE_DATABASE != DEF_DISABLED)
                            DB_Central.Set(&pTempUnit[VT100_NEW_TEMPERATURE_SELECTION], SYSTEM_TEMPERATURE_UNIT);
                          #endif
                            SetRefreshFullPage();
                          #if (DIGINI_USE_GRAFX == DEF_ENABLED)
                            GUI_pTask->SetForceRefresh();                                                       // Force the graphic page to also be refresh to new language
                          #endif
                        }
                    }

                  #if (DIGINI_USE_DATABASE != DEF_DISABLED)
                    if((pBuffer1 != nullptr) && (pBuffer2 != nullptr))
                    {
                        if(memcmp(pBuffer1, pBuffer2, sizeof(OEM_SERIAL_NUMBER)) != 0)
                        {
                            DB_Central.Set(&pBuffer1, SERIAL_NUMBER_TEXT);
                        }
                    }
                  #endif

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
            SetForeColor(VT100_COLOR_WHITE);
            InMenuPrintf(1, 5, LBL_NETWORK_INFO);

            SetForeColor(VT100_COLOR_WHITE);
            InMenuPrintf(2,  8,  LBL_IP_ADDR);
            InMenuPrintf(2,  9,  LBL_IP_MASK);
            InMenuPrintf(2,  10, LBL_IP_GATEWAY);
            InMenuPrintf(2,  11, LBL_IP_DNS);
            InMenuPrintf(2,  12, LBL_IP_DHCP_STATE);
            InMenuPrintf(2,  13, LBL_IP_LINK_STATE);
            InMenuPrintf(2,  14, LBL_IP_LINK_SPEED);
            InMenuPrintf(33, 14, LBL_IP_BYTE_PER_SECOND);

            InMenuPrintf(2,  15, LBL_MAC_ADDRESS);
          #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
            InMenuPrintf(2,  18, LBL_ETH_RX_COUNT);
            InMenuPrintf(36, 18, LBL_ETH_DROP);
            InMenuPrintf(2,  19, LBL_ETH_TX_COUNT);
            InMenuPrintf(36, 19, LBL_ETH_DROP);
          #endif

            InMenuPrintf(2, 21, LBL_ETH_ARP_TABLE);

            for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
            {
                InMenuPrintf(2, 23 + i, LBL_ETH_ARP_TABLE_ENTRY, i);
            }

            InMenuPrintf(        VT100_LBL_ESCAPE);
            m_pConsole->SetOverrideDebugLevel(SYS_DEBUG_LEVEL_ETHERNET);
            LogInitialize(56, 6, 78, 40);
            // Add Lease obtain and expire???
        }
        break;

        case VT100_CALLBACK_REFRESH:
        {
            Label_e           SpeedLabel;
            NetworkContext*   pContext = pTaskNetwork->GetContext();

            if(pContext != nullptr)
            {
                char              Buffer[20];
                IP_MAC_Address_t  MAC;

                SetForeColor(VT100_COLOR_WHITE);

                IP_Manager::IP_ToAscii(Buffer, pContext->GetActiveIP());
                InMenuPrintf(28, 8,  LBL_STRING, Buffer);
                IP_Manager::IP_ToAscii(Buffer, pContext->GetActiveSubnetMask());
                InMenuPrintf(28, 9,   LBL_STRING, Buffer);
                IP_Manager::IP_ToAscii(Buffer, pContext->GetActiveGatewayIP());
                InMenuPrintf(28, 10, LBL_STRING, Buffer);
                IP_Manager::IP_ToAscii(Buffer, pContext->GetActiveDNS_IP());
                InMenuPrintf(28, 11, LBL_STRING, Buffer);
              #if (IP_USE_DHCP == DEF_ENABLED)
                InMenuPrintf(28, 12, (pContext->IsDHCP_Enable() == true) != 0 ? LBL_ENABLED : LBL_DISABLED);
              #else
                InMenuPrintf(28, 12, LBL_DISABLED);
              #endif
                InMenuPrintf(28, 13, (pContext->GetLinkState() == ETH_LINK_UP) ? LBL_IP_UP: LBL_IP_DOWN);

                // tempo remove warning
                //LinkInfo.Duplex = ETH_PHY_FULL_DUPLEX;

                switch(pContext->GetLinkSpeed())
                {
                    case ETH_PHY_SPEED_NONE:    SpeedLabel = LBL_IP_SPEED_NONE; break;
                    case ETH_PHY_SPEED_10M:     SpeedLabel = LBL_IP_SPEED_10M;  break;
                    case ETH_PHY_SPEED_100M:    SpeedLabel = LBL_IP_SPEED_100M; break;
                    case ETH_PHY_SPEED_1G:      SpeedLabel = LBL_IP_SPEED_1G;   break;
                }

                InMenuPrintf(28, 14, SpeedLabel);
                pContext->GetMAC_Address(&MAC);
                InMenuPrintf(28, 15, LBL_MAC_ADDRESS_VALUE, MAC.Byte[0], MAC.Byte[1], MAC.Byte[2], MAC.Byte[3], MAC.Byte[4], MAC.Byte[5]);

              #if (ETH_DEBUG_PACKET_COUNT == DEF_ENABLED)
                InMenuPrintf(28, 18, LBL_LONG_UNSIGNED, DBG_RX_Count);
                InMenuPrintf(50, 18, LBL_LONG_UNSIGNED, DBG_RX_Drop);
                InMenuPrintf(28, 19, LBL_LONG_UNSIGNED, DBG_TX_Count);
                InMenuPrintf(50, 19, LBL_LONG_UNSIGNED, DBG_TX_Drop);
              #endif

                for(int i = 0; i < IP_ARP_TABLE_SIZE; i++)
                {
                    ARP_TableEntry_t* pARP_Entry = pContext->GetARP().GetTableEntryPointer(i);
                    const uint8_t* Byte = pARP_Entry->MAC_Address.Byte;

                    IP_Manager::IP_ToAscii(Buffer, pARP_Entry->IP_Address);
                    InMenuPrintf(14, 23 + i, LBL_STRING, Buffer);
                    InMenuPrintf(34, 23 + i, LBL_MAC_ADDRESS_VALUE, Byte[0], Byte[1], Byte[2], Byte[3], Byte[4], Byte[5]);
                }

                LogDisplay();
            }
        }
        break;

        case VT100_CALLBACK_FLUSH:
        {
            m_pConsole->SetOverrideDebugLevel(SYS_DEBUG_NONE);
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
