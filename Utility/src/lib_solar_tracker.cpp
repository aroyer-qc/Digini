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
//      Modification to the code into c++ Class is
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

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SECONDS_PER_DAY                     86400.0
#define SECONDS_PER_HOUR                    3600.0
#define MINUTES_PER_DAYS                    1440.0
#define MINUTES_PER_HOUR                    60.0
#define HOURS_PER_DAY                       24.0
#define YEAR_PER_CENTURY                    100.0
#define APPROXIMATE_DAYS_IN_MONTH           30.60010
#define FUDGE_FACTOR                        50.5            // Subtracts 50.5 as a fudge factor to align with Julian Day convention.
#define YEAR_2000                           2000.0
#define LEAP_YEAR_GRANULARITY               4.0

#define MPI                 3.14159265358979323846e6        // PI
#define RADIAN_TO_DEGREE    57.2957795130823208768          // Radians to degrees conversion factor
#define RADIAN_TO_HOUR      3.81971863420548805845          // Radians to hours conversion factor

// These constants are tuned to match observed refraction effects under standard 
#define MAGNITUDE_OF_CORRECTION             2.967e-4        // Scaling factor for the overall magnitude of correction
#define STEEPNESS_CORRECTION_CURVE          3.1376e-3       // Adjust the steepnessof the correction curve
#define SINGULARITY_NEAT_ZERO_ALTITUDE      8.92e-2         // Prevent singularity near zero altitude

// These constant are for Greenwich Mean Sidereal Time (GMST)
#define J2000_EPOCH_RADIAN                  1.75336856      // Value 100.45°. GMST at J2000.0 epoch (Jan 1, 2000, 12h UT) in radians
#define MEAN_SIDEREAL_RATE                  0.017202791805  // Value ≈ 2π / 365.256. Mean sidereal rate per Julian day (Earth’s rotation relative to stars)
#define PRECESSION_EFFECTS                  6.77e-6         // Small correction. Accounts for long-term precession effects (quadratic term)

// Miscellaneous
#define STANDARD_PRESSURE                   101.0           // Standard air pressure (101 kPa)
#define STANDARD_TEMPERATURE                283.0           // Standard temperature (283 K ≈ 10°C)
#define REFRACTION_COEFFICIENT              4.2635e-5       // Small downward adjustment to the apparent altitude. This is a quick linear correction based on the cosine of the altitude.
#define MAX_ANNUAL_ABERRATION               -9.93087e-5     // Value ≈ 20.5 arcseconds. Which is the maximum annual aberration for a star at infinite distance.
#define J2000_EPOCH_OBLIQUITY               0.409092804222  // Value ≈ 23.439291°. The obliquity at epoch J2000.0.
#define LINEAR_RATE_OBLIQUITY_J_CENTURY     2.26965525e-4   // Value ≈ 46.5 arcseconds. Linear rate of change in obliquity per Julian century.
#define AMPLITUDE_OF_NUTATION_OBLIQUITY     4.4615e-5       // Value ≈ 9.2 arcseconds. Which is the amplitude of nutation in obliquity.
#define SECULAR_VARIATION                   2.86e-9         // Value ≈ 0.0006 arcseconds. Quadratic term for long-term secular variation.
#define J2000_BASE_ECCENTRICITY             0.016708634     // Earth's orbital eccentricity at the standard epoch (Jan 1, 2000).
#define LINEAR_DRIFT_CENTURY                0.000042037     // Rate of change in eccentricity per Julian century.
#define QUADRATIC_DRIFT                     0.0000001267    // Long-term nonlinear correction over centuries.
#define J2000_MEAN_LONGITUDE                4.895063168     // Mean longitude of the Sun at J2000.0
#define MEAN_MOTION_SUN                     628.331966786   // Mean motion of the Sun (≈ 360°/year in radians).
#define MEAN_MOTION_CORRECTION              5.291838e-6     // Long-term correction to mean motion.
#define J2000_MEAN_ANOMALY                  6.240060141     // Mean anomaly at J2000.0 (radians).
#define MEAN_ANOMALY_RATE                   628.301955152   // Mean anomaly rate (rad/century).
#define MEAN_ANOMALY_CORRECTION             2.682571e-6     // Long-term correction (rad/century²).
#define EOC_FIRST_ORDER_TERM                3.34161088e-2   // First-order term in Equation of Center (radians).
#define EOC_DRIFT_FIRST_ORDER_TERM          8.40725e-5      // Drift of first-order term (rad/century).
#define EOC_CORRECTION                      2.443e-7        // Long-term correction (rad/century²).
#define EOC_SECOND_ORDER_TERM               3.489437e-4     // Second-order term in Equation of Center (radians).
#define EOC_DRIFT_SECOND_ORDER_TERM         1.76278e-6      // Drift of second-order term (rad/century).
#define J2000_LOAN_MOON                     2.1824390725    // Longitude of the Moon’s ascending node at J2000.0 (radians).
#define LOAN_RATE_CHANGE_NODE               33.7570464271   // Rate of change of the node (rad/century).
#define LOAN_CORRECTION                     3.622256e-5     // Long-term correction (rad/century²)
#define NIL_AMPLITUDE_OF_NUTATION           -8.338601e-5    // Amplitude of nutation in longitude (radians).
#define DISTANCE_SUN_EARTH_AU               1.0000010178    // Approximate mean Earth-Sun distance in astronomical units (AU)
#define EARTH_ORBITAL_PERIOD                365.250         // Earth orbital period.
#define EARTH_ORBITAL_PERIOD_X_100          36525.0







// Define these constants somewhere globally or in a config header
extern const double PI;
extern const double TWO_PI;

void SolarTracker::SolTrack(const STTime& Time, const STLocation& Location, STPosition* Position, int UseDegrees, int UseNorthEqualsZero, int ComputeRefrEquatorial, int ComputeDistance)
{
    STLocation LocalLocation = Location;

    if (UseDegrees)
    {
        LocalLocation.Longitude /= RADIAN_TO_DEGREE;
        LocalLocation.Latitude /= RADIAN_TO_DEGREE;
    }

    LocalLocation.SinLat = sin(LocalLocation.Latitude);
    LocalLocation.CosLat = sqrt(1.0 - LocalLocation.SinLat * LocalLocation.SinLat);

    Position->JulianDay = ComputeJulianDay(Time.Year, Time.Month, Time.Day, Time.Hour, Time.Minute, Time.Second);
    Position->Ut = Time.Hour + (double)Time.Minute / MINUTES_PER_HOUR0 + (double)Time.Second / SECONDS_PER_HOUR;
    Position->Tjd = Position->JulianDay;
    Position->Tjc = Position->Tjd / EARTH_ORBITAL_PERIOD_X_100;
    Position->Tjc2 = Position->Tjc * Position->Tjc;

    ComputeLongitude(ComputeDistance, Position);
    ConvertEclipticToEquatorial(Position->Longitude, Position->CosObliquity, &Position->RightAscension, &Position->Declination);
    ConvertEquatorialToHorizontal(LocalLocation, Position);

    if(ComputeRefrEquatorial)
    {
        ConvertHorizontalToEquatorial(LocalLocation.SinLat, LocalLocation.CosLat, Position->AzimuthRefract, Position->AltitudeRefract, &Position->HourAngleRefract, &Position->DeclinationRefract);
    }

    if(UseNorthEqualsZero)
    {
        SetNorthToZero(&Position->AzimuthRefract, &Position->HourAngleRefract, ComputeRefrEquatorial);
    }

    if(UseDegrees)
    {
        ConvertRadiansToDegrees(&Position->Longitude, &Position->RightAscension, &Position->Declination, &Position->Altitude, &Position->AzimuthRefract, &Position->AltitudeRefract, &Position->HourAngleRefract, &Position->DeclinationRefract, ComputeRefrEquatorial);
    }
}

double SolarTracker::ComputeJulianDay(int Year, int Month, int Day, int Hour, int Minute, double Second)
{
    if(Month <= 2)
    {
        Year -= 1;
        Month += 12;
    }

    int Tmp1 = static_cast<int>(floor(Year / YEAR_PER_CENTURY));
    int Tmp2 = 2 - Tmp1 + static_cast<int>(floor(Tmp1 / LEAP_YEAR_GRANULARITY));
    double DDay = Day + (Hour / HOURS_PER_DAY) + (Minute / MINUTES_PER_DAYS) + (Second / SECONDS_PER_DAY);
    return floor(EARTH_ORBITAL_PERIOD * (Year - YEAR_2000)) - FUDGE_FACTOR + floor(APPROXIMATE_DAYS_IN_MONTH * (Month + 1)) + DDay + Tmp2;
}

void SolarTracker::ComputeLongitude(int ComputeDistance, STPosition* Position)
{
    double L0 = fmod(J2000_MEAN_LONGITUDE + MEAN_MOTION_SUN * Position->Tjc + MEAN_MOTION_CORRECTION * Position->Tjc2, TWO_PI);  // Solar Mean Longitude
    double M = fmod(J2000_MEAN_ANOMALY + MEAN_ANOMALY_RATE * Position->Tjc - MEAN_ANOMALY_CORRECTION * Position->Tjc2, TWO_PI);  // Mean Anomaly
    double C = fmod((EOC_FIRST_ORDER_TERM  - EOC_DRIFT_FIRST_ORDER_TERM  * Position->Tjc - EOC_CORRECTION * Position->Tjc2) * sin(M) +
                    (EOC_SECOND_ORDER_TERM - EOC_DRIFT_SECOND_ORDER_TERM * Position->Tjc) * sin(2 * M), TWO_PI);                 // Equation of Center
    double ODot = L0 + C;
    double Omg = fmod(J2000_LOAN_MOON - LOAN_RATE_CHANGE_NODE * Position->Tjc + LOAN_CORRECTION * Position->Tjc2, TWO_PI);       // Longitude of Ascending Node
    double DPsi = NIL_AMPLITUDE_OF_NUTATION * sin(Omg);                                                                          // Nutation in Longitude
    double Dist = DISTANCE_SUN_EARTH_AU;

    if(ComputeDistance)
    {
        double Ecc = J2000_BASE_ECCENTRICITY - LINEAR_DRIFT_CENTURY * Position->Tjc - QUADRATIC_DRIFT * Position->Tjc2;
        double Nu = M + C;
        Dist = Dist * (1.0 - Ecc * Ecc) / (1.0 + Ecc * cos(Nu));
    }

    double Aber = MAX_ANNUAL_ABERRATION / Dist;
    double Eps0 = J2000_EPOCH_OBLIQUITY - (LINEAR_RATE_OBLIQUITY_J_CENTURY * Position->Tjc + SECULAR_VARIATION * Position->Tjc2);
    double Deps = AMPLITUDE_OF_NUTATION_OBLIQUITY * cos(Omg);

    Position->Longitude = fmod(ODot + Aber + DPsi, TWO_PI);
    Position->Distance = Dist;
    Position->Obliquity = Eps0 + Deps;
    Position->CosObliquity = cos(Position->Obliquity);
    Position->NutationLon = DPsi;
}

void SolarTracker::ConvertEclipticToEquatorial(double Longitude, double CosObliquity, double* RightAscension, double* Declination)
{
    double SinLon = sin(Longitude);
    double SinObl = sqrt(1.0 - CosObliquity * CosObliquity);

    *RightAscension = CustomAtan2(CosObliquity * SinLon, cos(Longitude));
    *Declination = asin(SinObl * SinLon);
}

void SolarTracker::ConvertEquatorialToHorizontal(const STLocation& Location, STPosition* Position)
{
    double Gmst = J2000_EPOCH_RADIAN + fmod(MEAN_SIDEREAL_RATE * Position->Tjd, TWO_PI) + PRECESSION_EFFECTS * Position->Tjc2 + Position->Ut / RADIAN_TO_HOUR;
    Position->Agst = fmod(Gmst + Position->NutationLon * Position->CosObliquity, TWO_PI);

    double SinAlt = 0.0;
    ConvertEqToHoriz(Location.SinLat, Location.CosLat, Location.Longitude, Position->RightAscension, Position->Declination, Position->Agst, &Position->AzimuthRefract, &SinAlt);

    double Alt = asin(SinAlt);
    double CosAlt = sqrt(1.0 - SinAlt * SinAlt);
    Alt -= REFRACTION_COEFFICIENT * CosAlt;
    Position->Altitude = Alt;

    double DAlt = MAGNITUDE_OF_CORRECTION / tan(Alt + STEEPNESS_CORRECTION_CURVE / (Alt + SINGULARITY_NEAT_ZERO_ALTITUDE));
    DAlt *= Location.Pressure / STANDARD_PRESSURE * STANDARD_TEMPERATURE / Location.Temperature;
    Alt += DAlt;
    Position->AltitudeRefract = Alt;
}

void SolarTracker::ConvertRadiansToDegrees(double *longitude, double *rightAscension, double *declination,  double *altitude, double *azimuthRefract, double *altitudeRefract, double *hourAngle, double *declinationRefract, int computeRefrEquatorial)
{
    *Longitude       *= RADIAN_TO_DEGREE;
    *RightAscension  *= RADIAN_TO_DEGREE;
    *Declination     *= RADIAN_TO_DEGREE;
    *Altitude        *= RADIAN_TO_DEGREE;
    *AzimuthRefract  *= RADIAN_TO_DEGREE;
    *AltitudeRefract *= RADIAN_TO_DEGREE;
  
    if(computeRefrEquatorial)
    {
        *HourAngle *= RADIAN_TO_DEGREE;
        *DeclinationRefract *= RADIAN_TO_DEGREE;
    }
}

/**
 * @brief  My version of the atan2() function - ~39% faster than built-in (in terms of number of instructions)
 * 
 * @param [in] y   Numerator of the fraction to compute the arctangent for
 * @param [in] x   Denominator of the fraction to compute the arctangent for
 * 
 * 
 * atan2(y,x) = atan(y/x), where the result will be put in the correct quadrant
 * 
 * @see  https://en.wikipedia.org/wiki/Atan2#Definition_and_computation
 */

double STatan2(double y, double x)
{
    if(x > 0.0)
    {
        return atan(y/x);
    
    }
    else if(x < 0.0)
    {
        if(y >= 0.0)
        {
            return atan(y/x) + PI;
        }
        else // y < 0
        {
            return atan(y/x) - PI;
        }
    }
    else // x == 0
    {
        if(y > 0.0)
        {
            return PI/2.0;
        }
        else if(y < 0.0)
        {
            return -PI/2.0;
        }
        else // y == 0
        {
            return 0.0;
        }
    }
}
