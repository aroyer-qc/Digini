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

#include "lib_digini.h"

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
        for(j = 0; j <= LIB_TIMING_CFG_DELAY_LOOP_VALUE_FOR_1_USEC; j++);
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
    nOS_TimeDate TimeDate = nOS_TimeDateGet();

    pData->Hour   = TimeDate.hour;
    pData->Minute = TimeDate.minute;
    pData->Second = TimeDate.second;
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
    nOS_TimeDate TimeDate;

    TimeDate = nOS_TimeDateGet();
    TimeDate.hour   = pData->Hour;
    TimeDate.minute = pData->Minute;
    TimeDate.second = pData->Second;
    nOS_TimeDateSet(TimeDate);
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
    nOS_TimeDate TimeDate = nOS_TimeDateGet();

    pData->Day   = TimeDate.day;
    pData->Month = TimeDate.month;
    pData->Year  = TimeDate.year - 2000;
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
    nOS_TimeDate TimeDate;

    TimeDate = nOS_TimeDateGet();
    TimeDate.day   = pData->Day;
    TimeDate.month = pData->Month;
    TimeDate.year  = (uint16_t)pData->Year + 2000;
    nOS_TimeDateSet(TimeDate);
}

//-------------------------------------------------------------------------------------------------

