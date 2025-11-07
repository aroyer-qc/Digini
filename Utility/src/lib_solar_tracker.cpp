//-------------------------------------------------------------------------------------------------
//
//  File : lib_solar_tracker.cpp
//
//-------------------------------------------------------------------------------------------------
//
//  This c++ file is base on the following:
//
//      SolTrack: a simple, free, fast and accurate C routine to compute the position of the Sun
//
//      Copyright (c) 2014-2021  Marc van der Sluys, Paul van Kan and Jurgen Reintjes,
//          Sustainable Energy research group, HAN University of applied sciences, Arnhem,
//          The Netherlands
//
//      The original file was part of the SolTrack package, see: http://soltrack.sourceforge.net
//      SolTrack is derived from libTheSky (http://libthesky.sourceforge.net)
//      under the terms of the GPL v.3
//
//      This is free software: you can redistribute it and/or modify it under the terms of the
//      GNU Lesser General
//      Public License as published by the Free Software Foundation,
//      either version 3 of the License, or (at your option) any later version.
//
//      This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//      without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//      See the GNU Lesser General Public License for more details.
//
//      See <http://www.gnu.org/licenses/> for a copy of the license.
//
//
//      Modification to the code into C++ class is
//          Copyright(c) 2025 Alain Royer.
//          Email: aroyer.qc@gmail.com
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

#include "lib_solar_tracker.h"
#include "lib_define.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define APPROXIMATE_DAYS_IN_MONTH           30.60010
#define FUDGE_FACTOR                        50.5                            // Subtracts 50.5 as a fudge factor to align with Julian Day convention.
#define YEAR_2000                           2000.0
#define LEAP_YEAR_GRANULARITY               4.0
#define JULIAN_DAYS_2000                    2451545.0

#define ST_PI                               3.14159265358979323846          // Pi
#define ST_TWO_PI                           6.28318530717958647693          // 2 pi

#define RADIAN_TO_DEGREE                    57.2957795130823208768          // Radians to degrees conversion factor
#define RADIAN_TO_HOUR                      3.81971863420548805845          // Radians to hours conversion factor

// These constants are tuned to match observed refraction effects under standard
#define MAGNITUDE_OF_CORRECTION             2.967e-4                        // Scaling factor for the overall magnitude of correction
#define STEEPNESS_CORRECTION_CURVE          3.1376e-3                       // Adjust the steepnessof the correction curve
#define SINGULARITY_NEAT_ZERO_ALTITUDE      8.92e-2                         // Prevent singularity near zero altitude

// These constant are for Greenwich Mean Sidereal Time (GMST)
#define PRECESSION_EFFECTS                  6.77e-6                         // Small correction. Accounts for long-term precession effects (quadratic term)
#define RATE_EARTH_ROTATION_PER_JULIAN_DAY  6.300388098985                  // Rate of Earth’s rotation in radians per Julian day

// Miscellaneous
#define STANDARD_PRESSURE                   101.0                           // Standard air pressure (101 kPa)
#define STANDARD_TEMPERATURE                283.0                           // Standard temperature (283 K ≈ 10°C)
#define REFRACTION_COEFFICIENT              4.2635e-5                       // Small downward adjustment to the apparent altitude. This is a quick linear correction based on the cosine of the altitude.
#define MAX_ANNUAL_ABERRATION               -9.93087e-5                     // Value ≈ 20.5 arcseconds. Which is the maximum annual aberration for a star at infinite distance.
#define J2000_EPOCH_OBLIQUITY               0.409092804222                  // Value ≈ 23.439291°. The obliquity at epoch J2000.0.
#define LINEAR_RATE_OBLIQUITY_J_CENTURY     2.26965525e-4                   // Value ≈ 46.5 arcseconds. Linear rate of change in obliquity per Julian century.
#define AMPLITUDE_OF_NUTATION_OBLIQUITY     4.4615e-5                       // Value ≈ 9.2 arcseconds. Which is the amplitude of nutation in obliquity.
#define SECULAR_VARIATION                   2.86e-9                         // Value ≈ 0.0006 arcseconds. Quadratic term for long-term secular variation.
#define J2000_BASE_ECCENTRICITY             0.016708634                     // Earth's orbital eccentricity at the standard epoch (Jan 1, 2000).
#define LINEAR_DRIFT_CENTURY                0.000042037                     // Rate of change in eccentricity per Julian century.
#define QUADRATIC_DRIFT                     0.0000001267                    // Long-term nonlinear correction over centuries.
#define J2000_MEAN_LONGITUDE                4.895063168                     // Mean longitude of the Sun at J2000.0
#define MEAN_MOTION_SUN                     628.331966786                   // Mean motion of the Sun (≈ 360°/year in radians).
#define MEAN_MOTION_CORRECTION              5.291838e-6                     // Long-term correction to mean motion.
#define J2000_MEAN_ANOMALY                  6.240060141                     // Mean anomaly at J2000.0 (radians).
#define MEAN_ANOMALY_RATE                   628.301955152                   // Mean anomaly rate (rad/century).
#define MEAN_ANOMALY_CORRECTION             2.682571e-6                     // Long-term correction (rad/century²).
#define EOC_FIRST_ORDER_TERM                3.34161088e-2                   // First-order term in Equation of Center (radians).
#define EOC_DRIFT_FIRST_ORDER_TERM          8.40725e-5                      // Drift of first-order term (rad/century).
#define EOC_CORRECTION                      2.443e-7                        // Long-term correction (rad/century²).
#define EOC_SECOND_ORDER_TERM               3.489437e-4                     // Second-order term in Equation of Center (radians).
#define EOC_DRIFT_SECOND_ORDER_TERM         1.76278e-6                      // Drift of second-order term (rad/century).
#define J2000_LOAN_MOON                     2.1824390725                    // Longitude of the Moon’s ascending node at J2000.0 (radians).
#define LOAN_RATE_CHANGE_NODE               33.7570464271                   // Rate of change of the node (rad/century).
#define LOAN_CORRECTION                     3.622256e-5                     // Long-term correction (rad/century²)
#define NIL_AMPLITUDE_OF_NUTATION           -8.338601e-5                    // Amplitude of nutation in longitude (radians).
#define DISTANCE_SUN_EARTH_AU               1.0000010178                    // Approximate mean Earth-Sun distance in astronomical units (AU)
#define EARTH_ORBITAL_PERIOD                365.250                         // Earth orbital period.
#define EARTH_ORBITAL_PERIOD_X_100          36525.0


FLOAT  CustomAtan2                     (FLOAT Y, FLOAT X);

//-------------------------------------------------------------------------------------------------
//
//  Name:           SolTrack
//
//  Parameter(s):   pDateTime   Struct date and time to compute the position of the sun, in UT.
//                  pLocation   Struct for geographic location to compute the position of the sun.
//                  pPosition   Struct for position of the Sun in horizontal coordinates.
//                                  - and equatorial if desired
//                  UseDegrees             Use degrees for in/out angular variables,
//                                         rather than radians
//                  UseNorthEqualsZero     Use the definition where azimuth = 0 denotes north,
//                                         rather than south
//                  ComputeRefrEquatorial  Compute refraction correction for equatorial coordinates
//                                         false -> no, true -> yes
//                  ComputeDistance        Compute distance to the Sun (in AU)
//                                         false -> no, true -> yes
//  Return:         None
//
//  Description:    Main function to compute the position of the Sun
//
//-------------------------------------------------------------------------------------------------
void SolTrack(DateAndTime_t* pDateTime, OriginLocation_t* pLocation, SunPosition_t* pPosition, bool UseDegrees, bool UseNorthEqualsZero, bool ComputeRefrEquatorial, bool ComputeDistance)
{
    if(UseDegrees == true)
    {
        pLocation->Longitude /= RADIAN_TO_DEGREE;
        pLocation->Latitude  /= RADIAN_TO_DEGREE;
    }

    // Compute these once and reuse:
    pLocation->SinLat = sin(pLocation->Latitude);
    pLocation->CosLat = sqrt(1.0 - pLocation->SinLat * pLocation->SinLat);

    ///-------------------------------------------------------------------------------------------------
    /// Compute the Julian day from the date and time

    int Year  = pDateTime->Date.Year;
    int Month = pDateTime->Date.Month;

    if(Month <= 2)
    {
        Year  -= 1;
        Month += 12;
    }

    int Tmp1 = static_cast<int>(floor(Year / TIME_YEAR_PER_CENTURY));
    int Tmp2 = 2 - Tmp1 + static_cast<int>(floor(Tmp1 / LEAP_YEAR_GRANULARITY));
    FLOAT DDay = FLOAT(pDateTime->Date.Day) + (FLOAT(pDateTime->Time.Hour) / FLOAT(TIME_HOURS_PER_DAY)) + (FLOAT(pDateTime->Time.Minute) / FLOAT(TIME_MINUTES_PER_DAYS)) + (FLOAT(pDateTime->Time.Second) / FLOAT(TIME_SECONDS_PER_DAY));
    pPosition->JulianDay = floor(EARTH_ORBITAL_PERIOD * (Year - YEAR_2000)) - FUDGE_FACTOR + floor(APPROXIMATE_DAYS_IN_MONTH * (Month + 1)) + DDay + Tmp2;

    ///------------------------------------------------------------------------------------------------
    /// Derived expressions of time:
    pPosition->tJD  = pPosition->JulianDay - JULIAN_DAYS_2000;      // Time in Julian days since 2000.0
    pPosition->tJC  = pPosition->tJD / EARTH_ORBITAL_PERIOD_X_100;
    pPosition->tJC2 = pPosition->tJC * pPosition->tJC;

    ///-------------------------------------------------------------------------------------------------
    /// Compute the ecliptic longitude of the Sun and the obliquity of the ecliptic:
    FLOAT L0 = J2000_MEAN_LONGITUDE + MEAN_MOTION_SUN * pPosition->tJC + MEAN_MOTION_CORRECTION * pPosition->tJC2;                  // Solar Mean Longitude
    FLOAT M =  J2000_MEAN_ANOMALY + MEAN_ANOMALY_RATE * pPosition->tJC - MEAN_ANOMALY_CORRECTION * pPosition->tJC2;                 // Mean Anomaly
    FLOAT C =  (EOC_FIRST_ORDER_TERM - EOC_DRIFT_FIRST_ORDER_TERM  * pPosition->tJC - EOC_CORRECTION * pPosition->tJC2) * sin(M) +
               (EOC_SECOND_ORDER_TERM - EOC_DRIFT_SECOND_ORDER_TERM * pPosition->tJC) * sin(2 * M);                                 // Sun's equation of the centre
    FLOAT ODot = L0 + C;

    // Nutation, aberration:
    FLOAT Omg = J2000_LOAN_MOON - LOAN_RATE_CHANGE_NODE * pPosition->tJC + LOAN_CORRECTION * pPosition->tJC2;                       // Longitude of Moon's ascending node
    FLOAT DPsi = NIL_AMPLITUDE_OF_NUTATION * sin(Omg);                                                                              // Nutation in Longitude
    FLOAT Dist = DISTANCE_SUN_EARTH_AU;                                                                                             // Mean distance to the Sun in AU

    if(ComputeDistance == true)
    {
        FLOAT Ecc = J2000_BASE_ECCENTRICITY - LINEAR_DRIFT_CENTURY * pPosition->tJC - QUADRATIC_DRIFT * pPosition->tJC2;            // Eccentricity of the Earth's orbit
        FLOAT Nu = M + C;                                                                                                           // True anomaly
        Dist = Dist * (1.0 - Ecc * Ecc) / (1.0 + Ecc * cos(Nu));                                                                    // Geocentric distance of the Sun in AU
    }

    FLOAT Aber = MAX_ANNUAL_ABERRATION / Dist;                                                                                      // Aberration

    // Obliquity of the ecliptic and nutation - do this here, since we've already computed many of the ingredients:
    FLOAT Eps0 = J2000_EPOCH_OBLIQUITY - (LINEAR_RATE_OBLIQUITY_J_CENTURY * pPosition->tJC + SECULAR_VARIATION * pPosition->tJC2);  // Mean obliquity of the ecliptic
    FLOAT Deps = AMPLITUDE_OF_NUTATION_OBLIQUITY * cos(Omg);                                                                        // Nutation in obliquity

    // Save position parameters:
    pPosition->Longitude = ODot + Aber + DPsi;

    while(pPosition->Longitude > ST_TWO_PI)
    {
        pPosition->Longitude -= ST_TWO_PI;
    }

    while(pPosition->Longitude < 0)
    {
        pPosition->Longitude += ST_TWO_PI;
    }

    // Apparent geocentric longitude, referred to the true equinox of date
    pPosition->Distance     = Dist;                                         // Distance (AU)
    pPosition->Obliquity    = Eps0 + Deps;                                  // True obliquity of the ecliptic
    pPosition->CosObliquity = cos(pPosition->Obliquity);                    // Need the cosine later on
    pPosition->NutationLon  = DPsi;                                         // Nutation in longitude

    ///-------------------------------------------------------------------------------------------------
    /// Convert ecliptic coordinates to geocentric equatorial coordinates:

    FLOAT SinLon = sin(pPosition->Longitude);
    FLOAT SinObl = sqrt(1.0 - pPosition->CosObliquity * pPosition->CosObliquity); // Sine of the obliquity of the ecliptic will be positive in the forseeable future

    pPosition->RightAscension = CustomAtan2(pPosition->CosObliquity * SinLon, cos(pPosition->Longitude));      // 0 <= azimuth < (2 * PI)
    pPosition->Declination = asin(SinObl * SinLon);

    ///-------------------------------------------------------------------------------------------------
    /// Convert equatorial coordinates to horizontal coordinates, correcting for parallax and refraction:
    //FLOAT Gmst = J2000_MEAN_LONGITUDE + RATE_EARTH_ROTATION_PER_JULIAN_DAY * pPosition->tJD + PRECESSION_EFFECTS * pPosition->tJC2; // Greenwich mean sidereal time
    /* according to copilot */ FLOAT Gmst = 280.46061837 + 360.98564736629 * (pPosition->tJD - JULIAN_DAYS_2000);

    pPosition->Agst = Gmst + pPosition->NutationLon * pPosition->CosObliquity;                                                            // Correction for equation of the equinoxes -> apparent Greenwich sidereal time

    FLOAT SinAlt = 0.0;
    // Azimuth does not need to be corrected for parallax or refraction, hence store the result in the 'azimuthRefract' variable directly:
    {
        FLOAT Ha  = pPosition->Agst + pLocation->Longitude - pPosition->RightAscension;                           // Local Hour Angle

        // Some preparation, saves ~29%:
        FLOAT SinHa  = sin(Ha);
        FLOAT CosHa  = cos(Ha);

        FLOAT SinDec = sin(pPosition->Declination);
        FLOAT CosDec = sqrt(1.0 - SinDec * SinDec);                                                               // Cosine of a declination is always positive or zero
        FLOAT TanDec = SinDec / CosDec;

        pPosition->AzimuthRefract = CustomAtan2(SinHa,  CosHa  * pLocation->SinLat - TanDec * pLocation->CosLat); // 0 <= azimuth < (2 * PI)
        SinAlt = pLocation->SinLat * SinDec + pLocation->CosLat * CosDec * CosHa;                                 // Sine of the altitude above the horizon
    }

    FLOAT Alt = asin(SinAlt);                                                                                     // Altitude of the Sun above the horizon (rad)
    FLOAT CosAlt = sqrt(1.0 - SinAlt * SinAlt);                                                                   // Cosine of the altitude is always positive or zero

    // Correct for parallax:
    Alt -= REFRACTION_COEFFICIENT * CosAlt;                                                                       // Horizontal parallax = 8.794" = 4.2635e-5 rad
    pPosition->Altitude = Alt;

    // Correct for atmospheric refraction:
    FLOAT DAlt = MAGNITUDE_OF_CORRECTION / tan(Alt + STEEPNESS_CORRECTION_CURVE / (Alt + SINGULARITY_NEAT_ZERO_ALTITUDE)); // Refraction correction in altitude
    DAlt *= pLocation->Pressure / STANDARD_PRESSURE * STANDARD_TEMPERATURE / pLocation->Temperature;
    Alt += DAlt;
    pPosition->AltitudeRefract = Alt;

    ///-------------------------------------------------------------------------------------------------
    /// Convert the corrected horizontal coordinates back to equatorial coordinates:
    if(ComputeRefrEquatorial == true)
    {
        FLOAT CosAz  = cos(pPosition->AzimuthRefract);
        FLOAT SinAz  = sin(pPosition->AzimuthRefract);                                                              // For symmetry
        FLOAT SinAlt = sin(pPosition->AltitudeRefract);
        FLOAT CosAlt = sqrt(1.0 - SinAlt * SinAlt);                                                                 // Cosine of an altitude is always positive or zero
        FLOAT TanAlt = SinAlt / CosAlt;

        pPosition->HourAngle          = CustomAtan2(SinAz, CosAz * pLocation->SinLat + TanAlt * pLocation->CosLat); // Local Hour Angle:  0 <= hourAngle < 2pi
        pPosition->DeclinationRefract = asin(pLocation->SinLat * SinAlt - pLocation->CosLat * CosAlt * CosAz);      // Declination
    }

    ///-------------------------------------------------------------------------------------------------
    // Use the North=0 convention for azimuth and hour angle (default: South = 0) if desired:
    if(UseNorthEqualsZero == true)
    {
        pPosition->AzimuthRefract += ST_PI;                                         // Add PI to set North = 0

        if(pPosition->AzimuthRefract > ST_TWO_PI)
        {
            pPosition->AzimuthRefract -= ST_TWO_PI;                                 // Ensure 0 <= azimuth < (2 * PI)
        }

        if(ComputeRefrEquatorial == true)
        {
            pPosition->HourAngleRefract = pPosition->HourAngleRefract + ST_PI;      // Add PI to set North=0

            if(pPosition->HourAngleRefract > ST_TWO_PI)
            {
                pPosition->HourAngleRefract -= ST_TWO_PI;                           // Ensure 0 <= hour angle < (2 * PI)
            }
        }
    }

    // If the user wants degrees, convert final results from radians to degrees:
    if(UseDegrees == true)
    {
        pPosition->Longitude       *= RADIAN_TO_DEGREE;
        pPosition->RightAscension  *= RADIAN_TO_DEGREE;
        pPosition->Declination     *= RADIAN_TO_DEGREE;
        pPosition->Altitude        *= RADIAN_TO_DEGREE;
        pPosition->AzimuthRefract  *= RADIAN_TO_DEGREE;
        pPosition->AltitudeRefract *= RADIAN_TO_DEGREE;

        if(ComputeRefrEquatorial == true)
        {
            pPosition->HourAngleRefract   *= RADIAN_TO_DEGREE;
            pPosition->DeclinationRefract *= RADIAN_TO_DEGREE;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CustomAtan2
//
//  Parameter(s):   y       Numerator of the fraction to compute the arctangent for
//                  x       Denominator of the fraction to compute the arctangent for
//  Return:         None
//
//  Description:    Alternate version of the atan2() function.
//                  ~39% faster than built-in (in terms of number of instructions)
//
//  Note(s)          https://en.wikipedia.org/wiki/Atan2#Definition_and_computation
//
//-------------------------------------------------------------------------------------------------
FLOAT CustomAtan2(FLOAT y, FLOAT x)
{
    if(x > 0.0)
    {
        return atan(y / x);

    }
    else if(x < 0.0)
    {
        if(y >= 0.0)
        {
            return atan(y / x) + ST_PI;
        }
        else // y < 0
        {
            return atan(y / x) - ST_PI;
        }
    }
    else // x == 0
    {
        if(y > 0.0)
        {
            return ST_PI / 2.0;
        }
        else if(y < 0.0)
        {
            return -ST_PI / 2.0;
        }
        else // y == 0
        {
            return 0.0;
        }
    }
}
