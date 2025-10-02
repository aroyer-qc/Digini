//-------------------------------------------------------------------------------------------------
//
//  File : lib_energy_meter.cpp
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

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//#define TIME_SECONDS_PER_HOUR               3600                // use lib_define.h 
#define EM_SAMPLES_PER_SECOND               10
#define EM_SAMPLE_INTERVAL_HOURS            1.0 / EM_SAMPLES_PER_SECOND / TIME_SECONDS_PER_HOUR
#define EM_WINDOW_SIZE                      3.0

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   EnergyMeter
//
//   Parameter(s):  None
//
//   Description:   Initializes the Energy Meter
//
//-------------------------------------------------------------------------------------------------

EnergyMeter::EnergyMeter()
{
    Index = 0;
    Filled = false;
    
    for(uint32_t i = 0; i < EM_BUFFER_SIZE; ++i)
    {
        VoltageBuffer[i] = 0.0f;
        CurrentBuffer[i] = 0.0f;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: AddSample
//
//   Parameter(s):  float       Voltage
//                  float       Current
//   Return Value:  None
//
//   Description:   Add sample to the Energy Meter
//
//-------------------------------------------------------------------------------------------------

void EnergyMeter::AddSample(float Voltage, float Current)
{
    // Store raw values temporarily
    VoltageBuffer[Index] = Voltage;
    CurrentBuffer[Index] = Current;

    // Apply moving average if enough samples are available
    float SmoothedVoltage = Voltage;
    float SmoothedCurrent = Current;

    if((Index >= (EM_WINDOW_SIZE - 1)) || (Filled == true))
    {
        SmoothedVoltage = 0.0f;
        SmoothedCurrent = 0.0f;

        for(uint32_t j = 0; j < EM_WINDOW_SIZE; ++j)
        {
            uint32_t Idx = (Index + BufferSize - j) % BufferSize;
            SmoothedVoltage += VoltageBuffer[Idx];
            SmoothedCurrent += CurrentBuffer[Idx];
        }

        SmoothedVoltage /= EM_WINDOW_SIZE;
        SmoothedCurrent /= EM_WINDOW_SIZE;
    }

    // Overwrite with smoothed values
    VoltageBuffer[Index] = SmoothedVoltage;
    CurrentBuffer[Index] = SmoothedCurrent;

    // Advance circular buffer index
    Index = (Index + 1) % BufferSize;

    if(Index == 0)
    {
        Filled = true;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetWhLastSecond
//
//   Parameter(s):  None
//   Return Value:  Watt per hour estimate
//
//   Description:   Get the estimate Watts/hour for the last second
//
//-------------------------------------------------------------------------------------------------

float EnergyMeter::GetWhLastSecond(void)
{
    float    Energy = 0.0f;
    uint32_t Idx;
    uint32_t Available = Filled ? BufferSize : Index;                   // Only return 0 if we don't have enough samples yet
    
    if(Available < EM_SAMPLES_PER_SECOND)
    {
        return Energy;
    }

    for(uint32_t i = 0; i < EM_SAMPLES_PER_SECOND; i++)
    {
        Idx = (Index + BufferSize - EM_SAMPLES_PER_SECOND + i) % BufferSize;
    
        if(Idx > 0)
        {
            // Trapezoidal integration
            float vAvg = (VoltageBuffer[Idx] + VoltageBuffer[Idx - 1]) * 0.5f;
            float cAvg = (CurrentBuffer[Idx] + CurrentBuffer[Idx - 1]) * 0.5f;
            Energy += vAvg * cAvg * EM_SAMPLE_INTERVAL_HOURS;
        }
    }

    return Energy;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: GetWhLastHour
//
//   Parameter(s):  None
//   Return Value:  Watt per hour estimate
//
//   Description:   Get the estimate Watts/hour for the last second
//
//-------------------------------------------------------------------------------------------------

float EnergyMeter::GetWhLastHour(void)
{
    uint32_t Count  = Filled ? BufferSize : Index;
    float    Energy = 0.0f;

    if(Count == 0)
    {
        return 0.0f;                                                    // No samples yet
    }

    for(uint32_t i = 0; i < Count; ++i)
    {
        Energy += VoltageBuffer[i] * CurrentBuffer[i] * EM_SAMPLE_INTERVAL_HOURS;
    }

    return Energy;
}

//-------------------------------------------------------------------------------------------------



#if 0

#include <iostream>
#include "EnergyMeter.h"

int main()
{
    EnergyMeter Meter;

    // Simuler 10 échantillons (1 seconde)
    for(int i = 0; i < 10; ++i)
    {
        Meter.AddSample(12.0f, 1.5f); // 18W
    }

    std::cout << "Wh sur la dernière seconde : " << Meter.GetWhLastSecond() << std::endl;

    // Simuler 36000 échantillons (1 heure)
    for(int i = 0; i < 36000 - 10; ++i)
    {
        Meter.AddSample(12.0f, 1.5f);
    }

    std::cout << "Wh sur la dernière heure : " << Meter.GetWhLastHour() << std::endl;
    return 0;
}
#endif 