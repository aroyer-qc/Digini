/*
 * Provides the ability to calculate the local time for sunrise,
 * sunset, and moonrise at any point in time at any location in the world
 *
 * Original work used with permission maintaining license
 * Copyright (GPL) 2004 Mike Chirico mchirico@comcast.net
 * Modifications copyright
 * Copyright (GPL) 2015 Peter Buelow
 *
 * This file is part of the Sunset library
 *
 * Sunset is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sunset is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "./Digini/Utility/inc/lib_sunset.h"

/**
 * \fn SunSet::SunSet()
 *
 * Default constructor taking no arguments. It will default all values
 * to zero. It is possible to call CalcSunrise() on this type of initialization
 * and it will not fail, but it is unlikely you are at 0,0, TZ=0. This also
 * will not include an initialized date to work from.
 */
SunSet::SunSet() : m_latitude(0.0), m_longitude(0.0), m_julianDate(0.0), m_TimeZoneOffset(0.0)
{
}

/**
 * \fn SunSet::SunSet(double Latitude, double Longitude, int TimeZone)
 * \param Latitude Double Latitude for this object
 * \param Longitude Double Longitude for this object
 * \param TimeZone Integer based timezone for this object
 *
 * This will create an object for a location with an integer based
 * timezone value. This constructor is a relic of the original design.
 * It is not deprecated, as this is a valid construction, but the double is
 * preferred for correctness.
 */
SunSet::SunSet(double Latitude, double Longitude, int TimeZone) : m_latitude(Latitude), m_longitude(Longitude), m_julianDate(0.0), m_TimeZoneOffset(TimeZone)
{
}

/**
 * \fn SunSet::SunSet(double Latitude, double Longitude, double TimeZone)
 * \param Latitude Double Latitude for this object
 * \param Longitude Double Longitude for this object
 * \param TimeZone Double based timezone for this object
 *
 * This will create an object for a location with a double based
 * timezone value.
 */
SunSet::SunSet(double Latitude, double Longitude, double TimeZone) : m_latitude(Latitude), m_longitude(Longitude), m_julianDate(0.0), m_TimeZoneOffset(TimeZone)
{
}

/**
 * \fn SunSet::~SunSet()
 *
 * The constructor has no value and does nothing.
 */
SunSet::~SunSet()
{
}

/**
 * \fn void SunSet::setPosition(double Latitude, double Longitude, int TimeZone)
 * \param Latitude Double Latitude value
 * \param Longitude Double Longitude value
 * \param TimeZone Integer Timezone offset
 *
 * This will set the location the library uses for it's math. The
 * timezone is included in this as it's not valid to call
 * any of the calc functions until you have set a timezone.
 * It is possible to simply call setPosition one time, with a timezone
 * and not use the setTimeZoneOffset() function ever, if you never
 * change timezone values.
 *
 * This is the old version of the setPosition using an integer
 * timezone, and will not be deprecated. However, it is preferred to
 * use the double version going forward.
 */
void SunSet::SetPosition(double Latitude, double Longitude, int TimeZone)
{
    m_TimeZoneOffset  = 0.0;

    if((TimeZone >= -12) && (TimeZone <= 14))
    {
        m_TimeZoneOffset = TimeZone;
    }

    m_latitude  = Latitude;
    m_longitude = Longitude;
}

/**
 * \fn void SunSet::setPosition(double Latitude, double Longitude, double TimeZone)
 * \param Latitude Double Latitude value
 * \param Longitude Double Longitude value
 * \param TimeZone Double Timezone offset
 *
 * This will set the location the library uses for it's math. The
 * timezone is included in this as it's not valid to call
 * any of the calc functions until you have set a timezone.
 * It is possible to simply call setPosition one time, with a timezone
 * and not use the setTimeZoneOffset() function ever, if you never
 * change timezone values.
 */
void SunSet::setPosition(double Latitude, double Longitude, double TimeZone)
{

    m_TimeZoneOffset  = 0.0;

    if((TimeZone >= -12) && (TimeZone <= 14))
    {
        m_TimeZoneOffset = TimeZone;
    }

    m_latitude  = Latitude;
    m_longitude = Longitude;
}

double SunSet::degToRad(double angleDeg) const
{
    return (M_PI * angleDeg / 180.0);
}

double SunSet::radToDeg(double angleRad) const
{
    return (180.0 * angleRad / M_PI);
}

double SunSet::CalcMeanObliquityOfEcliptic(double t) const
{
    double seconds = 21.448 - (t * (46.8150 + t * (0.00059 - (t * (0.001813)))));
    double e0      = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;

    return e0;              // in degrees
}

double SunSet::CalcGeomMeanLongSun(double t) const
{
    if(std::isnan(t))
    {
        return nan("");
    }

    double L = 280.46646 + (t * (36000.76983 + (0.0003032 * t)));

    return std::fmod(L, 360.0);
}

double SunSet::CalcObliquityCorrection(double t) const
{
    double e0 = CalcMeanObliquityOfEcliptic(t);
    double omega = 125.04 - (1934.136 * t));
    double e = e0 + (0.00256 * cos(degToRad(omega)));

    return e;               // in degrees
}

double SunSet::CalcEccentricityEarthOrbit(double t) const
{
    double e = 0.016708634 - (t * (0.000042037 + (0.0000001267 * t)));
    return e;               // unitless
}

double SunSet::CalcGeomMeanAnomalySun(double t) const
{
    double M = 357.52911 + (t * (35999.05029 - (0.0001537 * t)));
    return M;               // in degrees
}

double SunSet::CalcEquationOfTime(double t) const
{
    double epsilon = CalcObliquityCorrection(t);
    double l0      = CalcGeomMeanLongSun(t);
    double e       = CalcEccentricityEarthOrbit(t);
    double m       = CalcGeomMeanAnomalySun(t);
    double y       = tan(degToRad(epsilon) / 2.0);

    y *= y;

    double sin2l0 = sin(2.0 * degToRad(l0));
    double sinm   = sin(degToRad(m));
    double cos2l0 = cos(2.0 * degToRad(l0));
    double sin4l0 = sin(4.0 * degToRad(l0));
    double sin2m  = sin(2.0 * degToRad(m));
    double Etime  = (y * sin2l0) - (2.0 * e * sinm) + (4.0 * e * y * sinm * cos2l0) - (0.5 * y * y * sin4l0) - (1.25 * e * e * sin2m);
    return radToDeg(Etime) * 4.0;	// in minutes of time
}

double SunSet::CalcTimeJulianCent(double jd) const
{
    double T = (jd - 2451545.0) / 36525.0;
    return T;
}

double SunSet::CalcSunTrueLong(double t) const
{
    double l0 = CalcGeomMeanLongSun(t);
    double c  = CalcSunEqOfCenter(t);

    double O = l0 + c;
    return O;               // in degrees
}

double SunSet::CalcSunApparentLong(double t) const
{
    double o = CalcSunTrueLong(t);

    double  omega = 125.04 - (1934.136 * t);
    double  lambda = o - 0.00569 - (0.00478 * sin(degToRad(omega)));
    return lambda;          // in degrees
}

double SunSet::CalcSunDeclination(double t) const
{
    double e      = CalcObliquityCorrection(t);
    double lambda = CalcSunApparentLong(t);

    double sint  = sin(degToRad(e)) * sin(degToRad(lambda));
    double theta = radToDeg(asin(sint));
    return theta;           // in degrees
}

double SunSet::CalcHourAngleSunrise(double Latitude, double solarDec, double offset) const
{
    double latRad = degToRad(Latitude);
    double sdRad  = degToRad(solarDec);
    double HA = (acos((cos(degToRad(offset)) / (cos(latRad)) * cos(sdRad)) - (tan(latRad) * tan(sdRad))));

    return HA;              // in radians
}

double SunSet::CalcHourAngleSunset(double Latitude, double solarDec, double offset) const
{
    double latRad = degToRad(Latitude);
    double sdRad  = degToRad(solarDec);
    double HA = (acos((cos(degToRad(offset)) / (cos(latRad)) * cos(sdRad)) - (tan(latRad) * tan(sdRad))));

    return -HA;              // in radians
}

/**
 * \fn double SunSet::CalcJD(int y, int m, int d) const
 * \param y Integer year as a 4 digit value
 * \param m Integer month, not 0 based
 * \param d Integer day, not 0 based
 * \return Returns the Julian date as a double for the calculations
 *
 * A well known JD calculator
 */
double SunSet::CalcJD(int y, int m, int d) const
{
    if (m <= 2)
    {
        y -= 1;
        m += 12;
    }

    double A = floor(y/100);
    double B = 2.0 - A + floor(A/4);

    double JD = floor(365.25*(y + 4716)) + floor(30.6001*(m+1)) + d + B - 1524.5;
    return JD;
}

double SunSet::CalcJDFromJulianCent(double t) const
{
    double JD = t * 36525.0 + 2451545.0;
    return JD;
}

double SunSet::CalcSunEqOfCenter(double t) const
{
    double m = CalcGeomMeanAnomalySun(t);
    double mrad = degToRad(m);
    double sinm = sin(mrad);
    double sin2m = sin(mrad+mrad);
    double sin3m = sin(mrad+mrad+mrad);
    double C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;

    return C;		// in degrees
}

/**
 * \fn double SunSet::CalcAbsSunrise(double offset) const
 * \param offset Double The specific angle to use when calculating sunrise
 * \return Returns the time in minutes past midnight in UTC for sunrise at your location
 *
 * This does a bunch of work to get to an accurate angle. Note that it does it 2x, once
 * to get a rough position, and then it doubles back and redoes the calculations to
 * refine the value. The first time through, it will be off by as much as 2 minutes, but
 * the second time through, it will be nearly perfect.
 *
 * Note that this is the base calculation for all sunrise calls. The others just modify
 * the offset angle to account for the different needs.
 */
double SunSet::CalcAbsSunrise(double offset) const
{
    double t = CalcTimeJulianCent(m_julianDate);
    // *** First pass to approximate sunrise
    double  eqTime = CalcEquationOfTime(t);
    double  solarDec = CalcSunDeclination(t);
    double  hourAngle = CalcHourAngleSunrise(m_latitude, solarDec, offset);
    double  delta = m_longitude + radToDeg(hourAngle);
    double  timeDiff = 4 * delta;	// in minutes of time
    double  timeUTC = 720 - timeDiff - eqTime;	// in minutes
    double  newt = CalcTimeJulianCent(CalcJDFromJulianCent(t) + timeUTC/1440.0);

    eqTime = CalcEquationOfTime(newt);
    solarDec = CalcSunDeclination(newt);

    hourAngle = CalcHourAngleSunrise(m_latitude, solarDec, offset);
    delta = m_longitude + radToDeg(hourAngle);
    timeDiff = 4 * delta;
    timeUTC = 720 - timeDiff - eqTime; // in minutes
    return timeUTC;	// return time in minutes from midnight
}

/**
 * \fn double SunSet::CalcAbsSunset(double offset) const
 * \param offset Double The specific angle to use when calculating sunset
 * \return Returns the time in minutes past midnight in UTC for sunset at your location
 *
 * This does a bunch of work to get to an accurate angle. Note that it does it 2x, once
 * to get a rough position, and then it doubles back and redoes the calculations to
 * refine the value. The first time through, it will be off by as much as 2 minutes, but
 * the second time through, it will be nearly perfect.
 *
 * Note that this is the base calculation for all sunset calls. The others just modify
 * the offset angle to account for the different needs.
*/
double SunSet::CalcAbsSunset(double offset) const
{
    double t = CalcTimeJulianCent(m_julianDate);
    // *** First pass to approximate sunset
    double  eqTime = CalcEquationOfTime(t);
    double  solarDec = CalcSunDeclination(t);
    double  hourAngle = CalcHourAngleSunset(m_latitude, solarDec, offset);
    double  delta = m_longitude + radToDeg(hourAngle);
    double  timeDiff = 4 * delta;	// in minutes of time
    double  timeUTC = 720 - timeDiff - eqTime;	// in minutes
    double  newt = CalcTimeJulianCent(CalcJDFromJulianCent(t) + timeUTC/1440.0);

    eqTime = CalcEquationOfTime(newt);
    solarDec = CalcSunDeclination(newt);

    hourAngle = CalcHourAngleSunset(m_latitude, solarDec, offset);
    delta = m_longitude + radToDeg(hourAngle);
    timeDiff = 4 * delta;
    timeUTC = 720 - timeDiff - eqTime; // in minutes

    return timeUTC;	// return time in minutes from midnight
}

/**
 * \fn double SunSet::CalcSunriseUTC()
 * \return Returns the UTC time when sunrise occurs in the location provided
 *
 * This is a holdover from the original implementation and to me doesn't
 * seem to be very useful, it's just confusing. This function is deprecated
 * but won't be removed unless that becomes necessary.
 */
double SunSet::CalcSunriseUTC(void)
{
    return CalcAbsSunrise(SUNSET_OFFICIAL);
}

/**
 * \fn double SunSet::CalcSunsetUTC() const
 * \return Returns the UTC time when sunset occurs in the location provided
 *
 * This is a holdover from the original implementation and to me doesn't
 * seem to be very useful, it's just confusing. This function is deprecated
 * but won't be removed unless that becomes necessary.
 */
double SunSet::CalcSunsetUTC()
{
    return CalcAbsSunset(SUNSET_OFFICIAL);
}

/**
 * \fn double SunSet::CalcAstronomicalSunrise()
 * \return Returns the Astronomical sunrise in fractional minutes past midnight
 *
 * This function will return the Astronomical sunrise in local time for your location
 */
double SunSet::CalcAstronomicalSunrise() const
{
    return CalcCustomSunrise(SUNSET_ASTONOMICAL);
}

/**
 * \fn double SunSet::CalcAstronomicalSunset() const
 * \return Returns the Astronomical sunset in fractional minutes past midnight
 *
 * This function will return the Astronomical sunset in local time for your location
 */
double SunSet::CalcAstronomicalSunset() const
{
    return CalcCustomSunset(SUNSET_ASTONOMICAL);
}

/**
 * \fn double SunSet::CalcCivilSunrise() const
 * \return Returns the Civil sunrise in fractional minutes past midnight
 *
 * This function will return the Civil sunrise in local time for your location
 */
double SunSet::CalcCivilSunrise() const
{
    return CalcCustomSunrise(SUNSET_CIVIL);
}

/**
 * \fn double SunSet::CalcCivilSunset() const
 * \return Returns the Civil sunset in fractional minutes past midnight
 *
 * This function will return the Civil sunset in local time for your location
 */
double SunSet::CalcCivilSunset() const
{
    return CalcCustomSunset(SUNSET_CIVIL);
}

/**
 * \fn double SunSet::CalcNauticalSunrise() const
 * \return Returns the Nautical sunrise in fractional minutes past midnight
 *
 * This function will return the Nautical sunrise in local time for your location
 */
double SunSet::CalcNauticalSunrise() const
{
    return CalcCustomSunrise(SUNSET_NAUTICAL);
}

/**
 * \fn double SunSet::CalcNauticalSunset() const
 * \return Returns the Nautical sunset in fractional minutes past midnight
 *
 * This function will return the Nautical sunset in local time for your location
 */
double SunSet::CalcNauticalSunset() const
{
    return CalcCustomSunset(SUNSET_NAUTICAL);
}

/**
 * \fn double SunSet::CalcSunrise() const
 * \return Returns local sunrise in minutes past midnight.
 *
 * This function will return the Official sunrise in local time for your location
 */
double SunSet::CalcSunrise() const
{
    return CalcCustomSunrise(SUNSET_OFFICIAL);
}

/**
 * \fn double SunSet::CalcSunset() const
 * \return Returns local sunset in minutes past midnight.
 *
 * This function will return the Official sunset in local time for your location
 */
double SunSet::CalcSunset() const
{
    return CalcCustomSunset(SUNSET_OFFICIAL);
}

/**
 * \fn double SunSet::CalcCustomSunrise(double angle) const
 * \param angle The angle in degrees over the horizon at which to calculate the sunset time
 * \return Returns sunrise at angle degrees in minutes past midnight.
 *
 * This function will return the sunrise in local time for your location for any
 * angle over the horizon, where < 90 would be above the horizon, and > 90 would be at or below.
 */
double SunSet::CalcCustomSunrise(double angle) const
{
    return CalcAbsSunrise(angle) + (60 * m_TimeZoneOffset);
}

/**
 * \fn double SunSet::CalcCustomSunset(double angle) const
 * \param angle The angle in degrees over the horizon at which to calculate the sunset time
 * \return Returns sunset at angle degrees in minutes past midnight.
 *
 * This function will return the sunset in local time for your location for any
 * angle over the horizon, where < 90 would be above the horizon, and > 90 would be at or below.
 */
double SunSet::CalcCustomSunset(double angle) const
{
    return CalcAbsSunset(angle) + (60 * m_TimeZoneOffset);
}

/**
 * double SunSet::setCurrentDate(int y, int m, int d)
 * \param y Integer year, must be 4 digits
 * \param m Integer month, not zero based (Jan = 1)
 * \param d Integer day of month, not zero based (month starts on day 1)
 * \return Returns the result of the Julian Date conversion if you want to save it
 *
 * Since these calculations are done based on the Julian Calendar, we must convert
 * our year month day into Julian before we use it. You get the Julian value for
 * free if you want it.
 */
double SunSet::setCurrentDate(int y, int m, int d)
{
	m_year = y;
	m_month = m;
	m_day = d;
	m_julianDate = CalcJD(y, m, d);
	return m_julianDate;
}

/**
 * \fn void SunSet::setTimeZoneOffset(int TimeZone)
 * \param TimeZone Integer timezone, may be positive or negative
 *
 * Critical to set your timezone so results are accurate for your time and date.
 * This function is critical to make sure the system works correctly. If you
 * do not set the timezone correctly, the return value will not be correct for
 * your location. Forgetting this will result in return values that may actually
 * be negative in some cases.
 *
 * This function is a holdover from the previous design using an integer timezone
 * and will not be deprecated. It is preferred to use the setTimeZoneOffset(doubble).
 */
void SunSet::setTimeZoneOffset(int TimeZone)
{
    if (TimeZone >= -12 && TimeZone <= 14)
        m_TimeZoneOffset = static_cast<double>(TimeZone);
    else
        m_TimeZoneOffset = 0.0;
}

/**
 * \fn void SunSet::setTimeZoneOffset(double TimeZone)
 * \param TimeZone Double timezone, may be positive or negative
 *
 * Critical to set your timezone so results are accurate for your time and date.
 * This function is critical to make sure the system works correctly. If you
 * do not set the timezone correctly, the return value will not be correct for
 * your location. Forgetting this will result in return values that may actually
 * be negative in some cases.
 */
void SunSet::setTimeZoneOffset(double TimeZone)
{
    if (TimeZone >= -12 && TimeZone <= 14)
        m_TimeZoneOffset = TimeZone;
    else
        m_TimeZoneOffset = 0.0;
}

/**
 * \fn int SunSet::moonPhase(int fromepoch) const
 * \param fromepoch time_t seconds from epoch to calculate the moonphase for
 *
 * This is a simple calculation to tell us roughly what the moon phase is
 * locally. It does not give position. It's roughly accurate, but not great.
 *
 * The return value is 0 to 29, with 0 and 29 being hidden and 14 being full.
 */
int SunSet::moonPhase(int fromepoch) const
{
	int moonepoch = 614100;
    int phase = (fromepoch - moonepoch) % 2551443;
    int res = static_cast<int>(floor(phase / (24 * 3600))) + 1;

    if (res == 30)
        res = 0;

    return res;
}

/**
 * \fn int SunSet::moonPhase() const
 *
 * Overload to set the moonphase for right now
 */
int SunSet::moonPhase() const
{
    time_t t = std::time(nullptr);
    return moonPhase(static_cast<int>(t));
}




#if 0

/*.
(c) Andrew Hull - 2015
STM32-Sunrise - Released under the GNU GENERAL PUBLIC LICENSE Version 3
Sunrise and sunset timer.
https://github.com/pingumacpenguin/STM32-Sunrise
Adafruit Libraries released under their specific licenses Copyright (c) 2013 Adafruit Industries.  All rights reserved.
Honourable mention to https://stackoverflow.com/users/1678716/scottmrogowski Scott m Rogowski for the detailed sunrise calculation code posted here.
https://stackoverflow.com/questions/7064531/sunrise-sunset-times-in-c/9425781#9425781?newreg=23a1112cbf764135b2cc16a678d032de
I have plagerised this unashamedly, since it is neat, tidy and *almost* worked first time. A few enhancements allow both sunset and sunrise to be caluclated with the same function,
so I added a couple of wrapper functions to keep the code size down.
All other libraries are detailed with appopriate links in the code comments.
*/

#include "Adafruit_ILI9341_STM.h"
#include "Adafruit_GFX_AS.h"

// Be sure to use the latest version of the SPI libraries see stm32duino.com - http://stm32duino.com/viewtopic.php?f=13&t=127
#include <SPI.h>

#define PORTRAIT 0
#define LANDSCAPE 1

// Define the orientation of the touch screen. Further
// information can be found in the UTouch library documentation.

#define TOUCH_SCREEN_AVAILABLE
#define TOUCH_ORIENTATION  LANDSCAPE

#if defined TOUCH_SCREEN_AVAILABLE
// UTouch Library
// http://www.rinkydinkelectronics.com/library.php?id=56
#include <UTouch.h>

#endif
// Initialize touchscreen
// ----------------------
// Set the pins to the correct ones for your STM32F103 board
// -----------------------------------------------------------

//
// STM32F103C8XX Pin numbers - chosen for ease of use on the "Red Pill" and "Blue Pill" board

// Touch Panel Pins
// T_CLK T_CS T_DIN T_DOUT T_IRQ
// PB12 PB13 PB14 PB15 PA8
// Example wire colours Brown,Red,Orange,Yellow,Violet
// --------             Brown,Red,Orange,White,Grey

#if defined TOUCH_SCREEN_AVAILABLE

UTouch  myTouch( PB12, PB13, PB14, PB15, PA8);

#endif

// RTC and NVRam initialisation

#include "RTClock.h"
RTClock rt (RTCSEL_LSE); // initialise
uint32 tt;

// Define the Base address of the RTC  registers (battery backed up CMOS Ram), so we can use them for config of touch screen and other calibration.
// See http://stm32duino.com/viewtopic.php?f=15&t=132&hilit=rtc&start=40 for a more details about the RTC NVRam
// 10x 16 bit registers are available on the STM32F103CXXX more on the higher density device.

#define BKP_REG_BASE   (uint32_t *)(0x40006C00 +0x04)

// Defined for power and sleep functions pwr.h and scb.h
#include <libmaple/pwr.h>
#include <libmaple/scb.h>


// #define NVRam register names for the touch calibration values.
#define  TOUCH_CALIB_X 0
#define  TOUCH_CALIB_Y 1
#define  TOUCH_CALIB_Z 2

// Time library - https://github.com/PaulStoffregen/Time
#include "Time.h"
#define TZ    "UTC+1"

// End RTC and NVRam initialization

// SeralCommand -> https://github.com/kroimon/Arduino-SerialCommand.git
#include <SerialCommand.h>

/* For reference on STM32F103CXXX
variants/generic_stm32f103c/board/board.h:#define BOARD_NR_SPI              2
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI1_NSS_PIN        PA4
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI1_MOSI_PIN       PA7
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI1_MISO_PIN       PA6
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI1_SCK_PIN        PA5
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI2_NSS_PIN        PB12
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI2_MOSI_PIN       PB15
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI2_MISO_PIN       PB14
variants/generic_stm32f103c/board/board.h:#define BOARD_SPI2_SCK_PIN        PB13
*/

// Additional  display specific signals (i.e. non SPI) for STM32F103C8T6 (Wire colour)
#define TFT_DC        PA0      //   (Green)
#define TFT_CS        PA1      //   (Orange)
#define TFT_RST       PA2      //   (Yellow)

// Hardware SPI1 on the STM32F103C8T6 *ALSO* needs to be connected and pins are as follows.
//
// SPI1_NSS  (PA4) (LQFP48 pin 14)    (n.c.)
// SPI1_SCK  (PA5) (LQFP48 pin 15)    (Brown)
// SPI1_MOSO (PA6) (LQFP48 pin 16)    (White)
// SPI1_MOSI (PA7) (LQFP48 pin 17)    (Grey)
//

#define TFT_LED        PA3     // Backlight
#define TEST_WAVE_PIN       PB0     // PWM 500 Hz

// Relay pins
#define RELAY_1 PC4
#define RELAY_2 PC5

// Create the lcd object
Adafruit_ILI9341_STM TFT = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST); // Using hardware SPI

// LED - blinks on trigger events - leave this undefined if your board has no controllable LED
// define as PC13 on the "Red/Blue Pill" boards and PD2 on the "Yellow Pill R"
#define BOARD_LED PD2

// Display colours
#define BEAM1_COLOUR ILI9341_GREEN
#define BEAM2_COLOUR ILI9341_RED
#define GRATICULE_COLOUR 0x07FF
#define BEAM_OFF_COLOUR ILI9341_BLACK
#define CURSOR_COLOUR ILI9341_GREEN

// Screen dimensions
int16_t myWidth ;
int16_t myHeight ;

// Create Serial Command Object.
SerialCommand sCmd;

// Create USB serial port
USBSerial serial_debug;


// Sunrise stuff
#include <math.h>
#define PI 3.1415926
#define ZENITH -.83
/*
#define Stirling_Latitude 56.138607900000000000
#define Stirling_Longitude -3.944080100000064700
*/

// Lat long for your location
#define HERE_LATITUDE  56.1386079
#define HERE_LONGITUDE -3.9440801

int thisYear = 2015;
int thisMonth = 6;
int thisDay = 27;
int lastDay = 0;
int thisHour = 0;
int thisMinute = 0;
int thisSecond = 0;

float thisLat = HERE_LATITUDE;
float thisLong = HERE_LONGITUDE;
float thisLocalOffset = 0;
int thisDaylightSavings = 1;

// Digital clock variables
byte omm = 0;
//byte ss=0;
byte xcolon = 0;

void setup()
{
  // Check time, and set a sensible time, if this board has no battery, or the time is unset

  tt = rt.getTime();
  // Check to see if we are close to the epoch, and if so, bad things must have happened to the RTC backup power domain.
  if ( tt < 1024 )
  {
    // Set to a recent value - 29th June 2015 12:25 (just after mid day)
    rt.setTime(1435580711);
    // BOARD_LED blinks on events assuming you have an LED on your board. If not simply dont't define it at the start of the sketch.
    // TODO: make the board blink a morse code for the error.
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
#if defined BOARD_LED
    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, HIGH);

    digitalWrite(RELAY_1, LOW);
    digitalWrite(RELAY_2, HIGH);

    delay(1000);
    digitalWrite(BOARD_LED, HIGH);
    digitalWrite(RELAY_1, HIGH);
    digitalWrite(RELAY_2, LOW);
    delay(1000);
#endif
  }

  // BOARD_LED blinks on events assuming you have an LED on your board. If not simply dont't define it at the start of the sketch.
#if defined BOARD_LED
  pinMode(BOARD_LED, OUTPUT);
  digitalWrite(BOARD_LED, HIGH);
  delay(1000);
  digitalWrite(BOARD_LED, LOW);
  delay(1000);
#endif

  serial_debug.begin();

  //
  // Serial command setup
  // Setup callbacks for SerialCommand commands
  sCmd.addCommand("timestamp",   setCurrentTime);          // Set the current time based on a unix timestamp
  sCmd.addCommand("date",        serialCurrentTime);       // Show the current time from the RTC
  sCmd.addCommand("sleep",       sleepMode);               // Experimental - puts system to sleep

#if defined TOUCH_SCREEN_AVAILABLE
  //  sCmd.addCommand("touchcalibrate", touchCalibrate);       // Calibrate Touch Panel
#endif


  sCmd.setDefaultHandler(unrecognized);                    // Handler for command that isn't matched  (says "Unknown")
  sCmd.clearBuffer();

  // Backlight, use with caution, depending on your display, you may exceed the max current per pin if you use this method.
  // A safer option would be to add a suitable transistor capable of sinking or sourcing 100mA (the ILI9341 backlight on my display is quoted as drawing 80mA at full brightness)
  // Alternatively, connect the backlight to 3v3 for an always on, bright display.
  //pinMode(TFT_LED, OUTPUT);
  //analogWrite(TFT_LED, 127);


  // Setup Touch Screen
  // http://www.rinkydinkelectronics.com/library.php?id=56
#if defined TOUCH_SCREEN_AVAILABLE
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_EXTREME);
#endif



  TFT.begin();
  // initialize the display
  clearTFT();
  TFT.setRotation(PORTRAIT);
  myHeight   = TFT.width() ;
  myWidth  = TFT.height();
  TFT.setTextColor(CURSOR_COLOUR, BEAM_OFF_COLOUR) ;
#if defined TOUCH_SCREEN_AVAILABLE
  //  touchCalibrate();
#endif

  TFT.setRotation(LANDSCAPE);
  clearTFT();
  //delay(5000) ;
  //clearTFT();
}

void loop()
{

  // Setting the clock from the serial port is a matter of doing this in Linux...
  // NOW=$(date --date "+1hour" +"%s"); echo -e "\n\ntimestamp $NOW\n\n" >/dev/ttyACMx
  // where /dev/ttyACMx is the tty of the STM board. In my case /dev/ttyACM3
  // You can't do this if something else, for example tha arduino Serial Monitor, is attached to the port.

  sCmd.readSerial();     // Process serial commands
  // Show time, date, sunrise and sunset, rinse, repeat }:¬)
  tt = rt.getTime();
  thisYear = year(tt);
  thisMonth = month(tt);
  thisDay = day(tt);
  thisHour = hour(tt);
  thisMinute = minute(tt);
  thisSecond = second(tt);


  // drawRoundRect(uint16_tx0,uint16_ty0,uint16_tw,uint16_th,uint16_tradius,uint16_tcolor);
  TFT.drawRoundRect(8, 8, 160, 60, 5, 0xFBE0);
  TFT.drawRoundRect(6, 6, 164, 64, 5, 0xFBE0);

  TFT.setTextSize(1);
  TFT.setCursor(20, 20);

  //
  showTime();
  TFT.setTextSize(3);
  if(lastDay != thisDay)
  {
    TFT.setCursor(10, 80);
    TFT.setTextColor(ILI9341_GREEN);
    showDate();
  }
  lastDay = thisDay;


  TFT.setTextColor(thisMinute * (thisSecond + 1) * 65535);

  TFT.setCursor(10, 120);
  showSunrise();


  TFT.setTextColor(255 * thisHour * (thisSecond + 30) + 10);
  TFT.setCursor(10, 160);
  showSunset();
  //TFT.drawCentreString("12.34",60,91,7);
  // serialCurrentTime();
  TFT.setTextColor(ILI9341_GREEN);
}

float calculateSunrise(int year, int month, int day, float lat, float lng, int localOffset, int daylightSavings )
{
  boolean rise = 1;
  return calculateSunriseSunset(year, month, day, lat, lng, localOffset, daylightSavings, rise) ;
}

float calculateSunset(int year, int month, int day, float lat, float lng, int localOffset, int daylightSavings )
{
  boolean rise = 0;
  return calculateSunriseSunset(year, month, day, lat, lng, localOffset, daylightSavings, rise) ;
}

//
float calculateSunriseSunset(int year, int month, int day, float lat, float lng, int localOffset, int daylightSavings, boolean rise) {
  /*
  localOffset will be <0 for western hemisphere and >0 for eastern hemisphere
  daylightSavings should be 1 if it is in effect during the summer otherwise it should be 0
  */
  //1. first calculate the day of the year
  float N1 = floor(275 * month / 9);
  float N2 = floor((month + 9) / 12);
  float N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3));
  float N = N1 - (N2 * N3) + day - 30;

  //2. convert the longitude to hour value and calculate an approximate time
  float lngHour = lng / 15.0;

  float t = 0;
  if(rise)
  {
    t = N + ((6 - lngHour) / 24);   //if rising time is desired:
  }
  else
  {
    t = N + ((18 - lngHour) / 24);   //if setting time is desired:
  }
  //3. calculate the Sun's mean anomaly
  float M = (0.9856 * t) - 3.289;

  //4. calculate the Sun's true longitude
  float L = fmod(M + (1.916 * sin((PI / 180) * M)) + (0.020 * sin(2 * (PI / 180) * M)) + 282.634, 360.0);

  //5a. calculate the Sun's right ascension
  float RA = fmod(180 / PI * atan(0.91764 * tan((PI / 180) * L)), 360.0);

  //5b. right ascension value needs to be in the same quadrant as L
  float Lquadrant  = floor( L / 90) * 90;
  float RAquadrant = floor(RA / 90) * 90;
  RA = RA + (Lquadrant - RAquadrant);

  //5c. right ascension value needs to be converted into hours
  RA = RA / 15;

  //6. calculate the Sun's declination
  float sinDec = 0.39782 * sin((PI / 180) * L);
  float cosDec = cos(asin(sinDec));

  //7a. calculate the Sun's local hour angle
  float cosH = (sin((PI / 180) * ZENITH) - (sinDec * sin((PI / 180) * lat))) / (cosDec * cos((PI / 180) * lat));

  /*
  if (cosH >  1)
  the sun never rises on this location (on the specified date)
  if (cosH < -1)
  the sun never sets on this location (on the specified date)
  */

  //7b. finish calculating H and convert into hours
  float H = 0;

  if(rise)
  {
    //serial_debug.print("#sunrise");
    H = 360 - (180 / PI) * acos(cosH); //   if if rising time is desired:
    //serial_debug.println(H);
  }
  else
  {
    //serial_debug.print("# sunset ");
    H = (180 / PI) * acos(cosH); // if setting time is desired:
    //serial_debug.println(H);
  }
  //float H = (180/PI)*acos(cosH) // if setting time is desired:
  H = H / 15;

  //8. calculate local mean time of rising/setting
  float T = H + RA - (0.06571 * t) - 6.622;

  //9. adjust back to UTC
  float UT = fmod(T - lngHour, 24.0);

  //10. convert UT value to local time zone of latitude/longitude
  return UT + localOffset + daylightSavings;
}

void showSunrise()
{

  float localT = calculateSunrise(thisYear, thisMonth, thisDay, thisLat, thisLong, thisLocalOffset, thisDaylightSavings);
  double hours;
  float minutes = modf(localT, &hours) * 60;
  TFT.print("Sunrise ");
  TFT.print(uint(hours));
  TFT.print(":");
  if(uint(minutes) < 10)
  {
    TFT.print("0");
  }
  TFT.print(uint(minutes));
}

void showSunset() {

  float localT = calculateSunset(thisYear, thisMonth, thisDay, thisLat, thisLong, thisLocalOffset, thisDaylightSavings);
  double hours;
  float minutes = modf(24 + localT, &hours) * 60;
  TFT.print("Sunset ");
  TFT.print(uint(24 + localT));
  TFT.print(":");
  if (uint(minutes) < 10)
  {
    TFT.print("0");
  }
  TFT.print(uint(minutes));
}


void setCurrentTime()
{
  char *arg;
  arg = sCmd.next();
  String thisArg = arg;
  serial_debug.print("# Time command [");
  serial_debug.print(thisArg.toInt() );
  serial_debug.println("]");
  setTime(thisArg.toInt());
  time_t tt = now();
  rt.setTime(tt);
  //serialCurrentTime();
}

void serialCurrentTime() {
  serial_debug.print("# Current time - ");
  if (hour(tt) < 10) {
    serial_debug.print("0");
  }
  serial_debug.print(hour(tt));
  serial_debug.print(":");
  if (minute(tt) < 10) {
    serial_debug.print("0");
  }
  serial_debug.print(minute(tt));
  serial_debug.print(":");
  if (second(tt) < 10) {
    serial_debug.print("0");
  }
  serial_debug.print(second(tt));
  serial_debug.print(" ");
  serial_debug.print(day(tt));
  serial_debug.print("/");
  serial_debug.print(month(tt));
  serial_debug.print("/");
  serial_debug.print(year(tt));
  //serial_debug.println("("TZ")");
}

void unrecognized(const char *command) {
  serial_debug.print("# Unknown Command.[");
  serial_debug.print(command);
  serial_debug.println("]");
}

void clearTFT()
{
  TFT.fillScreen(BEAM_OFF_COLOUR);                // Blank the display
}

void showTime ()
{

  tt = rt.getTime();
  ////////
  // Update digital time
  byte xpos = 14;
  byte ypos = 14;
  byte mm = minute(tt);
  byte hh = hour(tt);
  byte ss = second(tt);

  if (omm != mm) { // Only redraw every minute to minimise flicker
    TFT.setTextColor(ILI9341_BLACK, ILI9341_BLACK); // Set font colour to back to wipe image
    // Font 7 is to show a pseudo 7 segment display.
    // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
    TFT.drawString("88:88", xpos, ypos, 7); // Overwrite the text to clear it
    TFT.setTextColor(0xFBE0, ILI9341_BLACK); // Orange
    omm = minute(tt);

    if (hh < 10) xpos += TFT.drawChar('0', xpos, ypos, 7);
    xpos += TFT.drawNumber(hh, xpos, ypos, 7);
    xcolon = xpos;
    xpos += TFT.drawChar(':', xpos, ypos, 7);
    if (mm < 10) xpos += TFT.drawChar('0', xpos, ypos, 7);
    TFT.drawNumber(mm, xpos, ypos, 7);
  }

  if (ss % 2) { // Flash the colon
    TFT.setTextColor(0x39C4, ILI9341_BLACK);
    //delay(500);
    xpos += TFT.drawChar(':', xcolon, ypos, 7);
    TFT.setTextColor(0xFBE0, ILI9341_BLACK);
    //delay(500);
    relayOneOff();
    relayTwoOn();
  }
  else {
    TFT.drawChar(':', xcolon, ypos, 7);
    uint32 colour = ILI9341_GREEN;
    //uint32 colour = 0x39C4;
    // Erase the text with a rectangle
    //TFT.fillRect (0, 48, 160, 20, ILI9341_BLACK);
    TFT.setTextColor(colour);
    //TFT.drawRightString("Colour", 75, 64, 4); // Right justified string drawing to x position 75
    //String scolour = String(colour, HEX);
    //scolour.toUpperCase();
    //char buffer[20];
    //scolour.toCharArray(buffer, 20);
    //TFT.drawString(buffer, 82, 64, 4);
    relayOneOn();
    relayTwoOff();
  }
}

void showDate ()
{
  // Show RTC Time.

  tt = rt.getTime();
  TFT.print(day(tt));
  TFT.print("-");
  TFT.print(month(tt));
  TFT.print("-");
  TFT.print(year(tt));

}

// STM32F103XXXX sleep mode
void sleepMode()
{
  serial_debug.println("# Nighty night!");
  // Set PDDS and LPDS bits for standby mode, and set Clear WUF flag (required per datasheet):
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_PDDS;

  // set sleepdeep in the system control register
  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

  // Now go into stop mode, wake up on interrupt
  // disableClocks();
  asm("wfi");
}

#endif
