//-------------------------------------------------------------------------------------------------
//
//  File : lib_rotation.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_ROTATION == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// define(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_TABLE_AS_SIZE(ENUM_ID, POINTER, SIZE)			SIZE,
#define EXPAND_X_TABLE_AS_PTR(ENUM_ID, POINTER, SIZE)			POINTER,

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct RotationLookup_t
{
    int16_t CosQ;
    int16_t SinQ;
};

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

// every 3°
#if (USE_ROTATION_TABLE_3 == DEF_ENABLED)
const RotationLookup_t RotationTable3[120] =
{
    {  256,    0 }, {  256,   13 }, {  255,   27 }, {  253,   40 }, {  250,   53 }, {  246,   66 }, {  241,   79 }, {  235,   92 },
    {  228,  104 }, {  220,  116 }, {  212,  128 }, {  202,  140 }, {  192,  151 }, {  181,  162 }, {  169,  172 }, {  156,  181 },
    {  143,  190 }, {  129,  199 }, {  114,  207 }, {   99,  214 }, {   83,  220 }, {   67,  226 }, {   50,  231 }, {   33,  235 },
    {   16,  239 }, {   -1,  241 }, {  -18,  243 }, {  -35,  244 }, {  -52,  244 }, {  -69,  244 }, {  -85,  243 }, { -101,  241 },
    { -117,  238 }, { -132,  234 }, { -147,  230 }, { -161,  224 }, { -175,  218 }, { -188,  211 }, { -200,  203 }, { -212,  194 },
    { -223,  185 }, { -233,  174 }, { -243,  163 }, { -251,  151 }, { -259,  138 }, { -266,  125 }, { -272,  111 }, { -277,   96 },
    { -281,   81 }, { -284,   66 }, { -286,   50 }, { -287,   34 }, { -287,   17 }, { -286,    0 }, { -284,  -17 }, { -281,  -34 },
    { -277,  -50 }, { -272,  -66 }, { -266,  -81 }, { -259,  -96 }, { -251, -111 }, { -243, -125 }, { -233, -138 }, { -223, -151 },
    { -212, -163 }, { -200, -174 }, { -188, -185 }, { -175, -194 }, { -161, -203 }, { -147, -211 }, { -132, -218 }, { -117, -224 },
    { -101, -230 }, {  -85, -234 }, {  -69, -238 }, {  -52, -241 }, {  -35, -243 }, {  -18, -244 }, {   -1, -244 }, {   16, -243 },
    {   33, -241 }, {   50, -239 }, {   67, -235 }, {   83, -231 }, {   99, -226 }, {  114, -220 }, {  129, -214 }, {  143, -207 },
    {  156, -199 }, {  169, -190 }, {  181, -181 }, {  192, -172 }, {  202, -162 }, {  212, -151 }, {  220, -140 }, {  228, -128 },
    {  235, -116 }, {  241, -104 }, {  246,  -92 }, {  250,  -79 }, {  253,  -66 }, {  255,  -53 }, {  256,  -40 }, {  256,  -27 },
    {  255,  -13 }, {  256,    0 }, {  256,   13 }, {  255,   27 }, {  253,   40 }, {  250,   53 }, {  246,   66 }, {  241,   79 },
    {  235,   92 }, {  228,  104 }, {  220,  116 }, {  212,  128 }, {  202,  140 }, {  192,  151 }, {  181,  162 }, {  169,  172 }
};
#endif

//every 5°
#if (USE_ROTATION_TABLE_5 == DEF_ENABLED)
const RotationLookup_t RotationTable5[72] =
{   {  256,    0 }, {  255,   22 }, {  252,   44 }, {  247,   66 }, {  239,   88 }, {  228,  109 }, {  216,  128 }, {  200,  147 },
    {  182,  165 }, {  162,  181 }, {  140,  195 }, {  116,  208 }, {   90,  218 }, {   63,  226 }, {   34,  232 }, {    4,  235 },
    {  -26,  236 }, {  -56,  234 }, {  -85,  230 }, { -113,  223 }, { -140,  214 }, { -165,  202 }, { -188,  188 }, { -209,  171 },
    { -228,  152 }, { -244,  131 }, { -258,  108 }, { -269,   84 }, { -277,   58 }, { -282,   31 }, { -284,    4 }, { -283,  -23 },
    { -279,  -50 }, { -272,  -76 }, { -262, -101 }, { -249, -125 }, { -233, -147 }, { -214, -168 }, { -193, -187 }, { -169, -204 },
    { -143, -219 }, { -115, -231 }, {  -85, -241 }, {  -54, -248 }, {  -22, -253 }, {   11, -255 }, {   44, -255 }, {   76, -252 },
    {  108, -247 }, {  138, -239 }, {  167, -228 }, {  194, -215 }, {  219, -200 }, {  242, -182 }, {  263, -162 }, {  281, -140 },
    {  297, -116 }, {  309,  -90 }, {  319,  -63 }, {  326,  -34 }, {  330,   -4 }, {  331,   26 }, {  329,   56 }, {  324,   85 },
    {  316,  113 }, {  305,  140 }, {  291,  165 }, {  274,  188 }, {  255,  209 }, {  233,  228 }, {  209,  244 }, {  183,  258 }
};
#endif

// every 9°
#if (USE_ROTATION_TABLE_9 == DEF_ENABLED)
const RotationLookup_t RotationTable9[40] =
{
    {  256,    0 }, {  253,   40 }, {  244,   79 }, {  228,  117 }, {  207,  154 }, {  181,  181 }, {  154,  207 }, {  117,  228 },
    {   79,  244 }, {   40,  253 }, {    0,  256 }, {  -40,  253 }, {  -79,  244 }, { -117,  228 }, { -154,  207 }, { -181,  181 },
    { -207,  154 }, { -228,  117 }, { -244,   79 }, { -253,   40 }, { -256,    0 }, { -253,  -40 }, { -244,  -79 }, { -228, -117 },
    { -207, -154 }, { -181, -181 }, { -154, -207 }, { -117, -228 }, {  -79, -244 }, {  -40, -253 }, {    0, -256 }, {   40, -253 },
    {   79, -244 }, {  117, -228 }, {  154, -207 }, {  181, -181 }, {  207, -154 }, {  228, -117 }, {  244,  -79 }, {  253,  -40 }
};
#endif

static const RotationLookup_t* pRotationTable[] =
{
	ROTATION_TABLE_DEF(EXPAND_X_TABLE_AS_PTR)
};

static const int RotationTableSize[] =
{
	ROTATION_TABLE_DEF(EXPAND_X_TABLE_AS_SIZE)
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           RotateA8_Q8
//
//  Parameter(s):   const uint8_t*  pSrc
//                  uint8_t*        pDst
//                  int             Width
//                  int             Height
//                  int             AngleStep
//                  RotationID_e 	RotationID
//
//  Return:         none
//
//  Description:    Rotate an 8-bit alpha (A8) bitmap using a Q8.8 fixed-point lookup table.
//                  Rotation is performed around the center of the box. The inner loop is fully
//                  incremental (no multiplications), making it extremely fast on Cortex-M.
//
//  Note(s):        - AngleStep = angleDeg / step   (step = 3°, 5°, 9°, etc.)
//                  - RotationTable[] must contain Q8.8 cos/sin values
//                  - Out-of-bounds samples are written as 0
//                  - Designed for MCU: no float, no FPU required
//                  - IMPORTANT: The destination buffer SHOULD BE SQUARE (Width == Height) to
//                    avoid clipping of rotated corners.
//
//					This is mostly for bitmap for font.
//
//-------------------------------------------------------------------------------------------------
void ImageRotationA8_Q8(const uint8_t* pSrc, uint8_t* pDst, int Width, int Height, int AngleStep, RotationID_e RotationID)
{
    if(RotationTableSize[RotationID] <= AngleStep)
	{
		int CenterX = Width  / 2;
		int CenterY = Height / 2;

		int16_t CosQ = pRotationTable[RotationID][AngleIndex].CosQ;
		int16_t SinQ = pRotationTable[RotationID][AngleIndex].SinQ;

		for(int y = 0; y < Height; y++)
		{
			int DeltaY = y - CenterY;

			// Starting point (x = 0) in Q8.8
			int RotatedX_Q = (((-CenterX) * CosQ) + (DeltaY * SinQ)) + (CenterX << 8);
			int RotatedY_Q = (  (CenterX  * SinQ) + (DeltaY * CosQ)) + (CenterY << 8);

			uint8_t* pDstPtr = pDst + y * Width;

			for(int x = 0; x < Width; x++)
			{
				int RotatedX = RotatedX_Q >> 8;
				int RotatedY = RotatedY_Q >> 8;

				if((unsigned)RotatedX < (unsigned)Width && (unsigned)RotatedY < (unsigned)Height)
				{
					pDstPtr[x] = pSrc[RotatedY * Width + RotatedX];
				}
				else
				{
					pDstPtr[x] = 0;
				}

				RotatedX_Q += CosQ;
				RotatedY_Q -= SinQ;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ComputeCircularPlacement
//
//  Parameter(s):   int             CenterX
//                  int             CenterY
//                  int             Radius
//                  int             AngleStep
//                  int             BitmapWidth
//                  int             BitmapHeight
//                  int*            pOutX
//                  int*            pOutY
//                  RotationID_e 	RotationID
//
//  Return:         none
//
//  Description:    Compute the top-left drawing position for a rotated bitmap placed on the
//                  circumference of a virtual circle. The angle uses the same Q8.8 LUT as the
//                  rotation function.
//
//  Note(s):        - AngleStep = angleDeg / step   (step = 3°, 5°, 9°, etc.)
//                  - RotationTable[] must contain Q8.8 cos/sin values
//                  - (pOutX, pOutY) receive the top-left corner where the bitmap must be drawn
//                  - The bitmap is centered on the circle point
//
//-------------------------------------------------------------------------------------------------
void ComputeCircularPlacement(int CenterX, int CenterY, int Radius, int AngleStep, int BitmapWidth, int BitmapHeight, int* pOutX,  int* pOutY, RotationID_e RotationID)
{
    if(RotationTableSize[RotationID] <= AngleStep)
	{
		int16_t CosQ = pRotationTable[RotationID][AngleIndex].CosQ;
		int16_t SinQ = pRotationTable[RotationID][AngleIndex].SinQ;

		// Bitmap position of the bitmap on the circle
		int PosX = CenterX + ((Radius * CosQ) >> 8);
		int PosY = CenterY + ((Radius * SinQ) >> 8);

		// Top-Left corner adjustment
		*pOutX = PosX - (BitmapWidth  / 2);
		*pOutY = PosY - (BitmapHeight / 2);
	}
}

//-------------------------------------------------------------------------------------------------

#endif // USE_ROTATION

//-------------------------------------------------------------------------------------------------
