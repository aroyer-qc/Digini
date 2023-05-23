//-------------------------------------------------------------------------------------------------
//
//  File : lib_database_wrapper.cpp
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
//   Function:      AccessTime
//
//   Description:   Request time
//
//   Note(s):       AccessTime() does not use Number and SubNumber
//
//-------------------------------------------------------------------------------------------------
void AccessTime(AccessRequest_e AccessRequest, const void* pData, uint16_t Number, uint16_t SubNumber)
{
    VAR_UNUSED(Number);
    VAR_UNUSED(SubNumber);

  #if (USE_RTC_DRIVER == DEF_ENABLED)
    if(AccessRequest == ACCESS_READ) BSP_pRTC->GetTime((Time_t*)pData);
    else                             BSP_pRTC->SetTime((Time_t*)pData);
  #else // User function
    if(AccessRequest == ACCESS_READ) LIB_GetTime((Time_t*)pData);
    else                             LIB_SetTime((Time_t*)pData);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      AccessDate
//
//   Description:   Request date
//
//-------------------------------------------------------------------------------------------------
void AccessDate(AccessRequest_e AccessRequest, const void* pData, uint16_t Number, uint16_t SubNumber)
{
    VAR_UNUSED(Number);
    VAR_UNUSED(SubNumber);

  #if (USE_RTC_DRIVER == DEF_ENABLED)
    if(AccessRequest == ACCESS_READ) BSP_pRTC->GetDate((Date_t*)pData);
    else                             BSP_pRTC->SetDate((Date_t*)pData);
  #else // User function
    if(AccessRequest == ACCESS_READ) LIB_GetDate((Date_t*)pData);
    else                             LIB_SetDate((Date_t*)pData);
  #endif
}


//-------------------------------------------------------------------------------------------------
//
//   Function:      AccessTimeFormat
//
//   Description:   Request date
//
//-------------------------------------------------------------------------------------------------
void AccessTimeFormat(AccessRequest_e AccessRequest, const void* pData, uint16_t Number, uint16_t SubNumber)
{
    VAR_UNUSED(Number);
    VAR_UNUSED(SubNumber);

  #if (USE_RTC_DRIVER == DEF_ENABLED)
    if(AccessRequest == ACCESS_READ) BSP_pRTC->GetTimeFormat((TimeFormat_e*)pData);
    else                             BSP_pRTC->SetTimeFormat((TimeFormat_e*)pData);
  #else // User function
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      GetLabel
//
//   Description:   Request time
//
//   Note(s):       GetLabel() does not use Number and SubNumber
//
//-------------------------------------------------------------------------------------------------
void GetLabel(AccessRequest_e AccessRequest, const void* pData, uint16_t Number, uint16_t SubNumber)
{
    VAR_UNUSED(SubNumber);

    if(AccessRequest == ACCESS_READ)
    {
        if(Number < NB_LABEL_CONST)
        {
            pData = myLabel.GetPointer(Label_e(Number));
        }
    }
}



//-------------------------------------------------------------------------------------------------
