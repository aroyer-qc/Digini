//-------------------------------------------------------------------------------------------------
//
//  File : lib_LMT86.c
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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

#define LIB_LMT86_GLOBAL
#include "./lib_digini.h"
#undef  LIB_LMT86_GLOBAL

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

#ifdef USE_LMT86_LOOK_UP_TABLE
const uint16_t LMT86_LookUpTable[] = 
{    
  ///*  -50 */ 2616, 2607, 2598, 2589, 2580, 2571, 2562, 2553, 2543, 2533,
    /*  -40	*/ 2522, 2512, 2501, 2491, 2481, 2470, 2460, 2449, 2439, 2429,
    /*  -30	*/ 2418, 2408, 2397, 2387, 2376, 2366, 2355, 2345, 2334, 2324,
    /*  -20	*/ 2313, 2302, 2292, 2281, 2271, 2260, 2250, 2239, 2228, 2218,
    /*  -10	*/ 2207, 2197, 2186, 2175, 2164, 2154, 2143, 2132, 2122, 2111,
    /*    0	*/ 2100, 2089, 2079, 2068, 2057, 2047, 2036, 2025, 2014, 2004,
    /*   10	*/ 1993, 1982, 1971, 1961, 1950, 1939, 1928, 1918, 1907, 1896,
    /*   20	*/ 1885, 1874, 1864, 1853, 1842, 1831, 1820, 1810, 1799, 1788,
    /*   30	*/ 1777, 1766, 1756, 1745, 1734, 1723, 1712, 1701, 1690, 1679,
    /*   40	*/ 1668, 1657, 1646, 1635, 1624, 1613, 1602, 1591, 1580, 1569,
  ///*   50	*/ 1558, 1547, 1536, 1525, 1514, 1503, 1492, 1481, 1470, 1459,
  ///*   60	*/ 1448, 1436, 1425, 1414, 1403, 1391, 1380, 1369, 1358, 1346,
  ///*   70	*/ 1335, 1324, 1313, 1301, 1290, 1279, 1268, 1257, 1245, 1234,
  ///*   80	*/ 1223, 1212, 1201, 1189, 1178, 1167, 1155, 1144, 1133, 1122,
  ///*   90	*/ 1110, 1099, 1088, 1076, 1065, 1054, 1042, 1031, 1020, 1008,
  ///*  100	*/  997,  986,  974,  963,  951,  940,  929,  917,  906,  895,
  ///*  110	*/  883,  872,  860,  849,  837,  826,  814,  803,  791,  780,
  ///*  120	*/  769,  757,  745,  734,  722,  711,  699,  688,  676,  665,
  ///*  130	*/  653,  642,  630,  618,  607,  595,  584,  572,  560,  549,
  ///*  140	*/  537,  525,  514,  502,  490,  479,  467,  455,  443,  432,
  ///*  150	*/  420
};
#endif

//-------------------------------------------------------------------------------------------------
// Public Function
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Function:       LMT86_FloatCalculation
//
//  Parameter(s):   RawAdc          RAW ADC value from ADC class
//  Return:         float           Temperature in float format
//
//  Description:    Calculate the temperature using float formula.
//
//-------------------------------------------------------------------------------------------------
#ifdef USE_LMT86_FLOAT_CALCULATION
float LMT86_FloatCalculation(uint16_t RawAdc)
{
    float Volts;
    float Temp;

    // Get Voltage from ADC CPU specific function

    // Formula from Datasheet of TI
    //                  _______________________________________________
    //              _  /         2                         
    //     10.888 -  \/  (10.888)  + 4 X 0.00347 X (1777.3 - Vtemp(mV)) 
    // T = ____________________________________________________________  + 30
    //                           2 X (-0.00347)
    //
    Temp  = 1777.3f - Volts;            // (1777.3f - Vtemp(mV))
    Temp *= 0.01388f;                   // x 4 x 0.00347
    Temp += 118.548544f;                // Add -> (10.888 exp 2)
    Temp  = sqrt(Temp);                 // Square root the result
    Temp  = 10.888f - Temp;             // Substract the result from 10.888
    Temp  = Temp / -0.00694f;           // Divide stract with (2 x -0.00347)
    Temp += 30f;                        // Add 30 to the result

    return Temp;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       LMT86_LookUpCalculation
//
//  Parameter(s):   RawAdc          RAW ADC value from ADC class
//  Return:         float           Temperature in float format
//
//  Description:    Calculate the temperature using float formula.
//
//-------------------------------------------------------------------------------------------------
#ifdef USE_LMT86_LOOK_UP_TABLE 
uint16_t LMT86_LookUpCalculation(uint16_t RawAdc)
{
    int      Index  = -1;
    uint32_t Temp   = 0;
    
    // Get Voltage from ADC CPU specific function

    // Find to what range the temperature belong
    for(int i = 0; (i < (sizeof(LookUpTable) / sizeof(uint16_t))) && (Index >= 0); i++)
    {
        if(RawAdc >= LookUpTable[i])
        {
            Index = i;
        }
    }

    if(Index >= 0)
    {
        // Formula from 2 points of the lookup table
        //
        //       (Vt - V1)             
        // T = ------------- + T1
        //       (V2 - V1)
        //
        //
        Temp  = uint32_t(RawAdc - LookUpTable[Index - 1]) * 100;           // (Vt - V1) * 100 (to increase resolution)
        Temp /= uint32_t(LookUpTable[Index - 1] - LookUpTable[Index]);     // (V2 - V1)
        Temp += uint32_t(Index - 1) * 100;                                 // + T1 * 100
        Temp /= 10;                                                        // Bring back to 0.1 deg. resolution
    }
    
    return uint16_t(Temp);
}
#endif
