//-------------------------------------------------------------------------------------------------
//
//  File :  lib_rotation.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#ifndef USE_ROTATION_TABLE_3
  #define USE_ROTATION_TABLE_3			DEF_DISABLED
#endif
#ifndef USE_ROTATION_TABLE_5
  #define USE_ROTATION_TABLE_5			DEF_DISABLED
#endif
#ifndef USE_ROTATION_TABLE_9
  #define USE_ROTATION_TABLE_9			DEF_DISABLED
#endif

#if (USE_ROTATION_TABLE_3 == DEF_ENABLED) || \
    (USE_ROTATION_TABLE_5 == DEF_ENABLED) || \
	(USE_ROTATION_TABLE_9 == DEF_ENABLED)
  #define USE_ROTATION					DEF_ENABLED
#endif

#define ROTATION_TABLE_DEF(X_TABLE) \
	IF_USE(USE_ROTATION_TABLE_3, X_TABLE(ROTATION_TABLE_3, RotationTable3, 120)) \
	IF_USE(USE_ROTATION_TABLE_5, X_TABLE(ROTATION_TABLE_5, RotationTable5, 72))  \
	IF_USE(USE_ROTATION_TABLE_9, X_TABLE(ROTATION_TABLE_9, RotationTable9, 40))  \

#define EXPAND_X_TABLE_AS_ENUM(ENUM_ID, POINTER, SIZE)			ENUM_ID,

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

enum RotationID_e
{
	ROTATION_TABLE_DEF(EXPAND_X_TABLE_AS_ENUM)
	ROTATION_TABLE_COUNT
}

//-------------------------------------------------------------------------------------------------
// Function prototype(s) c++ only
//-------------------------------------------------------------------------------------------------

void 	ImageRotationA8_Q8			(const uint8_t* pSrc, uint8_t* pDst, int Width, int Height, int AngleStep, RotationID_e RotationID);
void 	ComputeCircularPlacement	(int CenterX, int CenterY, int Radius, int AngleStep, int BitmapWidth, int BitmapHeight, int* pOutX,  int* pOutY, RotationID_e RotationID);

//-------------------------------------------------------------------------------------------------

// usage
//int AngleStep = angleDeg / 9;   // 0..39
//RotateA8_Q8(srcA8, dstA8, w, h, AngleStep);

