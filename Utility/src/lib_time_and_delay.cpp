//-------------------------------------------------------------------------------------------------
//
//  File : lib_timming.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const uint8_t WeekDayTable[12]  = {4, 7, 7, 3, 5, 8, 3, 6, 9, 4, 7, 9};

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_Delay_uSec
//
//  Parameter(s):   uint32_t       Delay
//  Return:         none
//
//  Description:    Approximation of 1 uSec delay
//
//  Note(s):        Adjust manually by changing value in clock_cfg.h in your config directory
//
//-------------------------------------------------------------------------------------------------
void LIB_Delay_uSec(uint32_t Delay)
{
    uint32_t i;
    uint32_t j;

    for(i = 0; i < Delay; i++)
    {
        for(j = 0; j <= CFG_DELAY_TIMING_LOOP_VALUE_FOR_1_USEC; j++);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_Delay_mSec
//
//  Parameter(s):   uint32_t       Delay in mSec
//  Return:         none
//
//  Description:    mSec delay
//
//
//-------------------------------------------------------------------------------------------------
void LIB_Delay_mSec(uint32_t Delay)
{
    LIB_Delay_uSec(Delay * 1000);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_GetTime
//
//  Parameter(s):   Time_t*       Pointer on time_t
//  Return:         none
//
//  Description:    Return time data in structure from system
//
//
//-------------------------------------------------------------------------------------------------
void LIB_GetTime(Time_t* pData)
{
  #if (USE_RTC_DRIVER == DEF_ENABLED)
    myRTC.GetTime((Time_t*)pData);
  #else
    nOS_TimeDate TimeDate = nOS_TimeDateGet();

    pData->Hour   = TimeDate.hour;
    pData->Minute = TimeDate.minute;
    pData->Second = TimeDate.second;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_SetTime
//
//  Parameter(s):   Time_t*       Pointer on Time_t
//  Return:         none
//
//  Description:    Update time data in system
//
//
//-------------------------------------------------------------------------------------------------
void LIB_SetTime(Time_t* pData)
{
  #if (USE_RTC_DRIVER == DEF_ENABLED)
    myRTC.SetTime((Time_t*)pData);
  #else
    nOS_TimeDate TimeDate;

    TimeDate = nOS_TimeDateGet();
    TimeDate.hour   = pData->Hour;
    TimeDate.minute = pData->Minute;
    TimeDate.second = pData->Second;
    nOS_TimeDateSet(TimeDate);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_GetDate
//
//  Parameter(s):   Date_t*       Pointer on Date_t
//  Return:         none
//
//  Description:    Return date data in structure from system
//
//
//-------------------------------------------------------------------------------------------------
void LIB_GetDate(Date_t* pData)
{
  #if (USE_RTC_DRIVER == DEF_ENABLED)
    myRTC.GetDate((Date_t*)pData);
  #else
    nOS_TimeDate TimeDate = nOS_TimeDateGet();

    pData->Day   = TimeDate.day;
    pData->Month = TimeDate.month;
    pData->Year  = TimeDate.year;
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_SetDate
//
//  Parameter(s):   Date_t*       Pointer on Date_t
//  Return:         none
//
//  Description:    Update date data in system
//
//
//-------------------------------------------------------------------------------------------------
void LIB_SetDate(Date_t* pData)
{
  #if (USE_RTC_DRIVER == DEF_ENABLED)
    myRTC.SetDate(pData);
  #else
    nOS_TimeDate TimeDate;

    TimeDate = nOS_TimeDateGet();
    TimeDate.day   = pData->Day;
    TimeDate.month = pData->Month;
    TimeDate.year  = pData->Year;
    nOS_TimeDateSet(TimeDate);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_GetDateAndTime
//
//  Parameter(s):   DateAndTime_t*       Pointer on Date And Time data
//  Return:         none
//
//  Description:    Get date and time into data
//
//
//-------------------------------------------------------------------------------------------------
void LIB_GetDateAndTime(DateAndTime_t* pTimeDate)
{
    Time_t SyncTime;

 //   do
    {
      #if (USE_RTC_DRIVER == DEF_ENABLED)
        myRTC.GetTime(&pTimeDate->Time);
        myRTC.GetDate(&pTimeDate->Date);
        myRTC.GetTime(&SyncTime);
      #else
        LIB_GetTime(&pTimeDate->Time);
        LIB_GetDate(&pTimeDate->Date);
        LIB_GetTime(&SyncTime);
      #endif
    }
 //   while(memcmp((uint8_t*)&pTimeDate->Time, (uint8_t*)&SyncTime, sizeof(Time_t)) == 0);   TODO seem to be a bug here !!!!!
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           LIB_SetDateAndTime
//
//  Parameter(s):   DateAndTime_t*       Pointer on Date And Time data
//  Return:         none
//
//  Description:    Set date and time from data
//
//
//-------------------------------------------------------------------------------------------------
void LIB_SetDateAndTime(DateAndTime_t* pTimeDate)
{
    Time_t SyncTime;

//    do
    {
      #if (USE_RTC_DRIVER == DEF_ENABLED)
        myRTC.SetTime(&pTimeDate->Time);
        myRTC.SetDate(&pTimeDate->Date);
        myRTC.GetTime(&SyncTime);
      #else
        LIB_SetTime(&pTimeDate->Time);
        LIB_SetDate(&pTimeDate->Date);
        LIB_GetTime(&SyncTime);
      #endif
    }
//    while(memcmp((uint8_t*)&pTimeDate->Time, (uint8_t*)&SyncTime, sizeof(Time_t)) == 0);   TODO seem to be a bug here !!!!!
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: LIB_GetDayOfWeek
//
//   Parameter(s):  Date_t*  pDate
//   Return value:  uint8_t DayOfWeek
//
//   Description:   Get the day of the week from the date
//
//   Note(s):       (0-6, Sunday-Saturday) to stay generic, even if ST RTC are 1-Monday 7-Sunday
//
//-------------------------------------------------------------------------------------------------
uint8_t  LIB_GetDayOfWeek(Date_t* pDate)
{
    uint16_t Day;

    uint16_t Year = pDate->Year - 2000;

    // Pre calculate the day of the week (0-6, SUNDAY-SATURDAY)
    Day  = (Year >> 2) + 2;
    Day += (Year + pDate->Day + WeekDayTable[pDate->Month - 1]);

    if((Year % 4) == 0)
    {
        if(pDate->Month < 3)
        {
            if(Day == 0) Day = 6;
            else         Day--;
        }
    }

    Day %= 7;

    return uint8_t(Day);
}

//-------------------------------------------------------------------------------------------------
