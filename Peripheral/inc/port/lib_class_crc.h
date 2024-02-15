//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_crc.h
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

#include "crc_cfg.h"

#if defined STM32F1xx
 #include "lib_class_STM32F1_crc.h"
#elif defined STM32F4xx
 #include "lib_class_STM32F4_crc.h"
#elif defined STM32F7xx
 #include "lib_class_STM32F7_crc.h"
#endif

//-------------------------------------------------------------------------------------------------

struct
{
  uint8_t   Polynomial; 
  uint8_t   InitialValue;
//  uint32_t  GeneratingPolynomial;     // Set CRC generating polynomial. 7, 8, 16 or 32-bit long value for a polynomial degree respectively equal to 7, 8, 16 or 32. This field is written in normal representation, e.g., for a polynomial of degree 7, X^7 + X^6 + X^5 + X^2 + 1 is written 0x65. No need to specify it if DefaultPolynomialUse is set to DEFAULT_POLYNOMIAL_ENABLE
  uint32_t  CRC_Length;                 // Indicates CRC length.    CRC_LENGTH_32 (32-bit CRC)
                                        //                          CRC_LENGTH_16 (16-bit CRC)
                                        //                          CRC_LENGTH_8  (8-bit CRC)
                                        //                          CRC_LENGTH_7  (7-bit CRC)
                                              
  uint32_t InputDataInversionMode;    /*!< This parameter is a value of @ref CRCEx_Input_Data_Inversion and specifies input data inversion mode. 
                                           Can be either one of the following values 
                                           CRC_INPUTDATA_INVERSION_NONE      no input data inversion
                                           CRC_INPUTDATA_INVERSION_BYTE      byte-wise inversion, 0x1A2B3C4D becomes 0x58D43CB2
                                           CRC_INPUTDATA_INVERSION_HALFWORD  halfword-wise inversion, 0x1A2B3C4D becomes 0xD458B23C
                                           CRC_INPUTDATA_INVERSION_WORD      word-wise inversion, 0x1A2B3C4D becomes 0xB23CD458 */  
                                              
  uint32_t OutputDataInversionMode;   /*!< This parameter is a value of @ref CRCEx_Output_Data_Inversion and specifies output data (i.e. CRC) inversion mode.
                                            Can be either 
                                            CRC_OUTPUTDATA_INVERSION_DISABLE   no CRC inversion, or
                                            CRC_OUTPUTDATA_INVERSION_ENABLE    CRC 0x11223344 is converted into 0x22CC4488 */
} CRC_Init_t;
