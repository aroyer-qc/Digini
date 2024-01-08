//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_WS281x.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct WS281x_Color_t
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------

class WS281x
{
    public:

        void    Initialize          (void* pArg, uint16_t NumberOfLED, uint16_t ResetTime);
        void    SetLed              (uint16_t Offset, WS281x_Color_t Color);
        void    Process             (void);   
        void    FillUp_24_Bits      (uint8_t* pBuffer);

    private:

        uint16_t                    m_LedChainSize;
        volatile uint16_t           m_LedPointer;
        WS281x_Color_t*             m_pLedChain;
        uint8_t*                    m_pDMA_Buffer;
        bool                        m_NeedRefresh;
        uint8_t                     m_ResetCount;
};

//-------------------------------------------------------------------------------------------------
