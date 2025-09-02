//-------------------------------------------------------------------------------------------------
//
//  File : EnergyMeter.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2025 Alain Royer.
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define TIME_SECONDS_PER_HOUR               3600                // use lib_define.h 

#define EM_SAMPLES_PER_SECOND               10
#define EM_BUFFER_SIZE                      (SAMPLES_PER_SECOND * NUMBER_OF_SECOND_PER_HOUR)

//-------------------------------------------------------------------------------------------------

class EnergyMeter
{

    public:
    
        void        AddSample           (float Voltage, float Current);
        float       GetWhLastSecond     (void);
        float       GetWhLastHour       (void);

    private:
    
        float       VoltageBuffer[EM_BUFFER_SIZE];
        float       CurrentBuffer[EM_BUFFER_SIZE];
        uint32_t    Index;
        bool        Filled;
};

//-------------------------------------------------------------------------------------------------
