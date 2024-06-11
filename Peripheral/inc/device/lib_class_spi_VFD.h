//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_VFD.h
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

// TODO this should replace most of the IV-11 class... IV-11 must be on top of ths class

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define VFD_DEFAULT_DIM_VALUE               50              // Percent

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct VFD_Config_t
{
    SPI_Driver* pSPI;
    PWM_Driver* pPWM;
    uint32_t    NumberOfBits;
    IO_ID_e     IO_Load;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class VFD_Driver
{
    public:

                        VFD_Driver                      (const VFD_Config_t* pConfig);

        SystemState_e   Initialize                      (void);
        void            Send                            (void);
        void            Dim                             (uint8_t DimValue);
        void            Blank                           (bool IsItBlank);

        bool            Get                             (uint32_t Index)                                   { return m_pBitArray->Get(Index);        }
        void            Set                             (uint32_t Index, bool Value)                       { m_pBitArray->Set(Index, Value);        }
        void            Set                             (uint32_t Index, uint8_t* pData, size_t Count)     { m_pBitArray->Set(Index, pData, Count); }

    private:

        SPI_Driver*                     m_pSPI;
        PWM_Driver*                     m_pPWM;
        const VFD_Config_t*             m_pConfig;
        uint8_t*                        m_pBitsStream;
        BIT_Array*                      m_pBitArray;
        bool                            m_IsItBlank;
        uint8_t                         m_DimValue;
        uint8_t                         m_Padding;          // Number of bit to add in front of the stream.
        size_t                          m_NumberOfBytes;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_VFD__
#include "device_var.h"
#undef  __CLASS_VFD__

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)


