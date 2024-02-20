//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F1_rtc.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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
// TO DO Add support for IRQ
//
//  Create a pool of event, with callback
//  use Alarm A for recurring alarm
//  use Alarm B for one shot alarm
//  need function to register an (event, callback, single or recurring)
//  need function to unregister an event
//  need function to sort the pool
//  need to write IRQ to handle event callback. and update for next event
//       Need to update recurring event with new stamp ans remap it...
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define RTC_DRIVER_GLOBAL
#include "lib_digini.h"
#undef  RTC_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_RTC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Class: RTC_Driver
//
//   Description:   Class RTC_Driver
//
//-------------------------------------------------------------------------------------------------

const uint8_t  RTC_Driver::m_MonthSize[12]     = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const uint8_t  RTC_Driver::m_WeekDayTable[12]  = {4, 7, 7, 3, 5, 8, 3, 6, 9, 4, 7, 9};
const uint16_t RTC_Driver::m_DaysSoFar[12]     = {0, 31, 59, 90, 120, 151, 181, 212, 243, 274, 303, 334};

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   RTC_Driver
//
//   Parameter(s):  uint32_t            Mode
//
//
//   Description:   Initializes the RTC peripheral according to the specified Parameters
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::Initialize(uint32_t Mode)
{
    nOS_Error Error;

    if(m_IsItInitialize == false)
    {
        m_IsItInitialize = true;
        Error = nOS_MutexCreate(&m_Mutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT);
        VAR_UNUSED(Error);
    }

    // Init power and clock for RTC
    RCC->APB1ENR |=  ( RCC_APB1ENR_PWREN);     // Backup interface & Power interface clock enable
    PWR->CR   |= PWR_CR_DBP;                                        // RTC register access allowed
    RCC->BDCR &= uint32_t(~RCC_BDCR_RTCSEL);                        // Clear clock selection
    RCC->BDCR |= Mode;                                              // Set it with argument
    RCC->BDCR |= RCC_BDCR_RTCEN;                                    // RTC clock enable
    if(Mode == RTC_CLOCK_MODE_LSE) RCC->BDCR |= RCC_BDCR_LSEON;     // External 32.768 kHz oscillator ON
    if(Mode == RTC_CLOCK_MODE_LSI) RCC->CSR  |= RCC_CSR_LSION;      // Internal 32 kHz oscillator ON
    PWR->CR   &= uint32_t(~PWR_CR_DBP);                             // RTC register access blocked
    UnlockRegister();
    EnterInitMode();
    switch(Mode)
    {
        case RTC_CLOCK_MODE_LSE:
        {
            RTC->PRER = 0x000000FF;                                 // As per data sheet to separate write are necessary (value from data sheet)
            RTC->PRER = 0x007F00FF;                                 // First is the synchronous prescaler then the asynchronous prescaler factor
            break;
        }

        case RTC_CLOCK_MODE_LSI:
        {
            RTC->PRER = 0x000000FA;
            RTC->PRER = 0x007F00FA;
            break;
        }
       #ifdef USE_RTC_HSE_CLOCK
        case RTC_CLOCK_MODE_HSE:
        {
            RTC->PRER = RTC_CLOCK_SYNC_PRESCALER;
            RTC->PRER = RTC_CLOCK_SYNC_PRESCALER | RTC_CLOCK_ASYNC_PRESCALER;
        }
       #endif
    }

    RTC->CR &= uint32_t(~RTC_CR_FMT);                           // 12/24 Time format mode set default to 24 hours
    ExitInitMode();
    LockRegister();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetBackupRegister
//
//   Parameter(s):  uint8_t     Register
//   Return value:  uint32_t    Value
//
//   Description:   Return value at requested backup register
//
//-------------------------------------------------------------------------------------------------
uint32_t RTC_Driver::GetBackupRegister(uint8_t Register)
{
    return *(&RTC->BKP0R + Register);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SetBackupRegister
//
//   Parameter(s):  uint8_t     Register
//                  uint32_t    Value
//   Return value:  None
//
//   Description:   Set value at a requested backup register
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::SetBackupRegister(uint8_t Register, uint32_t Value)
{
    PWR->CR  |= PWR_CR_DBP;
    *(&RTC->BKP0R + Register) = Value;
    PWR->CR  &= uint32_t(~PWR_CR_DBP);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetDate
//
//   Parameter(s):  Date_t*      pDate
//   Return value:  None
//
//   Description:   Return real date
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::GetDate(Date_t* pDate)
{
    Lock();
    UpdateTimeFeature();
    memcpy(pDate, &m_Clock.Date, sizeof(Date_t));
    Unlock();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: GetTime
//
//   Parameter(s):  Time_t*      pTime
//   Return value:  None
//
//   Description:   Return real time
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::GetTime(Time_t* pTime)
{
    Lock();
    UpdateTimeFeature();
    memcpy(pTime, &m_Clock.DateTime.Time, sizeof(Time_t));
    Unlock();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SetDate
//
//   Parameter(s):  Date_t Pointer on Date_t structure
//   Return value:  None
//
//   Description:   Set date in the RTC register
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::SetDate(Date_t* pDate)
{
    uint32_t Date;
    uint8_t  DayOfWeek;

    DayOfWeek = LIB_GetDayOfWeek(pDate);
    if(DayOfWeek == 0) DayOfWeek = 7;
    Date  = (uint32_t(DayOfWeek) + 1) << 13;
    Date |= uint32_t(LIB_4DecBcd(pDate->Year)) << 16;
    Date |= uint32_t(LIB_2DecBcd(pDate->Month)) << 8;
    Date |= uint32_t(LIB_2DecBcd(pDate->Day));

    Enable();
    RTC->DR = Date;
    memcpy(&m_Clock.DateTime.Date, pDate, sizeof(Date_t));
    m_Clock.DayOfWeek = DayOfWeek;
    UpdateTimeFeature();
    Disable();
}

void RTC_Driver::SetDate(uint8_t Day, uint8_t Month, uint16_t Year)
{
    Date_t Date;

    Date.Day   = Day;
    Date.Month = Month;
    Date.Year  = Year;
    SetDate(&Date);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: SetTime
//
//   Parameter(s):
//   Return value:
//
//   Description:
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::SetTime(Time_t* pTime)
{
    uint32_t Time;

    Time  = uint32_t(LIB_2DecBcd(pTime->Hour)) << 16;
    Time |= uint32_t(LIB_2DecBcd(pTime->Minute)) << 8;                      // Minute
    Time |= uint32_t(LIB_2DecBcd(pTime->Second));                           // Second

    Enable();
    RTC->TR = Time;
    memcpy(&m_Clock.DateTime.Time, pTime, sizeof(Time_t));
    UpdateTimeFeature();
    Disable();
}


void RTC_Driver::SetTime(uint8_t Hour, uint8_t Minute, uint8_t Second)
{
    Time_t Time;

    m_Clock.DateTime.Time.Hour   = Hour;
    m_Clock.DateTime.Time.Minute = Minute;
    m_Clock.DateTime.Time.Second = Second;
    SetTime(&Time);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: TickHook
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Call by the high level SD Card class
//
//   Note(s):       Do not put in OS tick hook
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::TickHook(void)
{
    if(m_TimeOut > 0)
    {
        m_TimeOut--;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Enable
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   Allow time, date, etc.. to be set
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::Enable(void)
{
    Lock();
    UnlockRegister();
    EnterInitMode();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: Disable
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   Disable all access to RTC
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::Disable(void)
{
    ExitInitMode();
    LockRegister();
    Unlock();
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: EnterInitMode
//
//   Parameter(s):  None
//   Return value:  SystemState_e    State
//
//   Description:   Stop calendar and enter initialization mode
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
SystemState_e RTC_Driver::EnterInitMode(void)
{
    SystemState_e    State;

    // Enter initialization mode, and stop calendar counter (INIT)
    RTC->ISR |= RTC_ISR_INIT;

    // Wait until RTC is ready or time out has expired
    m_TimeOut = 10;
    do
    {
        nOS_Yield();
    }
    while(((RTC->ISR & RTC_ISR_INITF) == 0) && (m_TimeOut != 0));

    if(m_TimeOut == 0) State = SYS_TIME_OUT;
    else               State = SYS_READY;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: ExitInitMode
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   Stop calendar and enter initialization mode
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::ExitInitMode(void)
{
    // Exit initialization mode, and restart calendar counter
    RTC->ISR &= uint32_t(~RTC_ISR_INIT);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Lock
//
//   Parameter(s):
//   Return Value:
//
//   Description:   Lock the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::Lock(void)
{
    while(nOS_MutexLock(&m_Mutex, NOS_WAIT_INFINITE) != NOS_OK);
}

//-------------------------------------------------------------------------------------------------
//
//   Function:      Unlock
//
//   Parameter(s):
//   Return Value:
//
//   Description:   Unlock the driver
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::Unlock(void)
{
    while(nOS_MutexUnlock(&m_Mutex) != NOS_OK);
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: UnlockRegister
//                  lockRegister
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   Lock or unlock access to the RTC register
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::LockRegister(void)
{
    // Write wrong key to relock "Write Protection Register"
    RTC->WPR  = 0x00;
    PWR->CR  &= uint32_t(~PWR_CR_DBP);
}


void RTC_Driver::UnlockRegister(void)
{
    // Write unlock key to "Write Protection Register"
    PWR->CR  |= PWR_CR_DBP;
    RTC->WPR  = 0xCA;
    RTC->WPR  = 0x53;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: UpdateTimeFeature
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   Update all feature unhandle by RTC
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::UpdateTimeFeature(void)
{
    uint32_t Time;
    uint32_t Date;

    Time = RTC->TR;
    Date = RTC->DR;

    m_Clock.DateTime.Time.Hour   = LIB_2BcdDec(uint8_t(Time >> 16) & 0x3F);
    if((Time & RTC_TR_PM) != 0) m_Clock.DateTime.Time.Hour += 12;
    m_Clock.DateTime.Time.Minute = LIB_2BcdDec(uint8_t(Time >> 8) & 0x7F);
    m_Clock.DateTime.Time.Second = LIB_2BcdDec(uint8_t(Time) & 0x7F);
    m_Clock.DateTime.Date.Day    = LIB_2BcdDec(uint8_t(Date) & 0x3F);
    m_Clock.DateTime.Date.Month  = LIB_2BcdDec(uint8_t(Date >> 8) & 0x1F);
    m_Clock.DateTime.Date.Year   = LIB_4BcdDec(uint16_t(Date >> 16)) + 2000;
    m_Clock.DayOfWeek   = uint8_t(Date >> 13);
    if(m_Clock.DayOfWeek == 7)  m_Clock.DayOfWeek = 0;

    // ---------- Minute of the day ------------------------------------------

    m_Clock.MinuteOfDay = (uint16_t(m_Clock.DateTime.Time.Hour) * 60) + uint16_t(m_Clock.DateTime.Time.Minute);

    // ---------- Day of the year --------------------------------------------

    // Set the day number in the year(1 - 365/366)
    m_Clock.DayOfYear = m_DaysSoFar[m_Clock.DateTime.Date.Month - 1] + m_Clock.DateTime.Date.Day;

    // ---------- Leap year --------------------------------------------------

    // Add one day for any month after february on a leap year
    if(((m_Clock.DateTime.Date.Year & 0x03) == 0) && (m_Clock.DateTime.Date.Month > 2))
    {
        // Add a day
        m_Clock.DayOfYear++;
    }
    // ---------- Week of the year -------------------------------------------

    // Set the week in the year (1 - 52) using day of the week
    m_Clock.WeekOfYear = 7 - m_Clock.DayOfWeek;
    m_Clock.WeekOfYear = (m_Clock.DayOfYear + m_Clock.WeekOfYear) / 7;
    m_Clock.WeekOfYear++;
}

//-------------------------------------------------------------------------------------------------
//
//   Function name: WaitForSynchro
//
//   Parameter(s):  None
//   Return value:  SystemState_e    State
//
//   Description:   Waits until the RTC Time and Date registers are synchronized
//
//-------------------------------------------------------------------------------------------------
SystemState_e RTC_Driver::WaitForSynchro(void)
{
    SystemState_e State;

    RTC->ISR &= uint32_t(~RTC_ISR_RSF);                         // Clear RSF flag

    // Wait the registers to be synchronised
    m_TimeOut = 10;
    do
    {
        nOS_Yield();
    }
    while(((RTC->ISR & RTC_ISR_RSF) == 0) && (m_TimeOut != 0));

    if(m_TimeOut == 0) State = SYS_TIME_OUT;
    else               State = SYS_READY;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//   IRQ Handler:   WakeUp_IRQ_Handler
//                  Stamp_IRQ_Handler
//                  Alarm_IRQ_Handler
//
//   Parameter(s):  None
//   Return value:  None
//
//   Description:   IRQ Handler of the RTC module
//
//   Note(s):
//
//-------------------------------------------------------------------------------------------------
void RTC_Driver::WakeUp_IRQ_Handler(void)
{

}

void RTC_Driver::Stamp_IRQ_Handler(void)
{

}

void RTC_Driver::Alarm_IRQ_Handler(void)
{

}

//-------------------------------------------------------------------------------------------------

#endif // (USE_RTC_DRIVER == DEF_ENABLED)
