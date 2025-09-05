//-------------------------------------------------------------------------------------------------
//
//  File : lib_solar_tracker.h
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

#pragma

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <cmath>
#include "lib_typedef.h"

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

// Location to compute the Sun's position for
struct OriginLocation_t
{
    double  Longitude;
    double  Latitude;
    double  SinLat;
    double  CosLat;
    double  Pressure;
    double  Temperature;
};

// Position of the Sun
struct SunPosition_t
{
    double  JulianDay;
    double  tJD;
    double  tJC;
    double  tJC2;
    double  Longitude;
    double  Distance;
    double  Obliquity;
    double  CosObliquity;
    double  NutationLon;
    double  RightAscension;
    double  Declination;
    double  HourAngle;
    double  Agst;
    double  Altitude;
    double  AltitudeRefract;            // Elevation
    double  AzimuthRefract;             // Azimuth
    double  HourAngleRefract;
    double  DeclinationRefract;
};

//-------------------------------------------------------------------------------------------------

class SolarTracker
{
    public:

        void    SolTrack                        (DateAndTime_t* pDateTime, OriginLocation_t* pLocation, SunPosition_t* pPosition, bool UseDegrees, bool UseNorthEqualsZero, bool ComputeRefrEquatorial, bool ComputeDistance);

    private:

        double  ComputeJulianDay                (void);
        void    ComputeLongitude                (bool ComputeDistance);
        void    ConvertEclipticToEquatorial     (void);
        void    ConvertEquatorialToHorizontal   (void);
        void    ConvertEquatorialToHorizontal   (double* SinAlt);
        void    ConvertHorizontalToEquatorial   (void);
        void    SetNorthToZero                  (bool ComputeRefrEquatorial);
        void    ConvertRadiansToDegrees         (bool ComputeRefrEquatorial);

        DateAndTime_t*      m_pDateTime;
        SunPosition_t*      m_pPosition;
        OriginLocation_t*   m_pLocation;
};

//-------------------------------------------------------------------------------------------------
