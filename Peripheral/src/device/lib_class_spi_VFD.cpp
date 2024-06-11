//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_spi_VFD.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define LIB_VFD_GLOBAL
#include "./Digini/lib_digini.h"
#undef  LIB_VFD_GLOBAL

//-------------------------------------------------------------------------------------------------
//
//  Name:           Constructor
//
//  Parameter(s):   const VFD_Config_t* pConfig
//
//  Description:    Initialize config
//
//-------------------------------------------------------------------------------------------------
VFD_Driver::VFD_Driver(const VFD_Config_t* pConfig)
{
    m_pConfig = pConfig;                                                        // Get config for the stream
    m_pSPI    = pConfig->pSPI;
    m_pPWM    = pConfig->pPWM;
    m_DimValue  = VFD_DEFAULT_DIM_VALUE;
    m_IsItBlank = true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Initialize the VFD Serial to parallel interface
//
//  Note(s):        Call this init when the OS is started
//
//-------------------------------------------------------------------------------------------------
SystemState_e VFD_Driver::Initialize(void)
{
    IO_PinInit(m_pConfig->IO_Load);

    // SPI need multiple of 8 bits to send on module.
    // There is padding bits to add in the beginning of the stream if not multiple of 8 bits
    m_Padding = 8 - (m_pConfig->NumberOfBits % 8);                              // Paddings
    m_NumberOfBytes  = m_pConfig->NumberOfBits / 8;                             // Number of bits
    m_NumberOfBytes += (((m_pConfig->NumberOfBits % 8) != 0) ? 1 : 0);          // Add the bits necessary to complete the stream.
    m_pBitsStream = (uint8_t*) pMemoryPool->AllocAndClear(m_NumberOfBytes);     // No bit set at init.
    m_pBitArray = new BIT_Array(m_pBitsStream, m_pConfig->NumberOfBits);        // Create BIT_Array object

    m_pPWM->Start();
    Dim(m_DimValue);
    Blank(false);
    Send();
    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Send the stream to the VFD.
//
//-------------------------------------------------------------------------------------------------
void VFD_Driver::Send(void)
{
    m_pSPI->Write(m_pBitsStream, m_NumberOfBytes, m_pConfig->IO_Load);   // We use the IO for load to lock the SPI
    IO_SetPinHigh(m_pConfig->IO_Load);
    // delay
    IO_SetPinLow(m_pConfig->IO_Load);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Dim
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:    Dim the VFD
//
//  Note(s):        Will not override the blanking setting.
//
//-------------------------------------------------------------------------------------------------
void VFD_Driver::Dim(uint8_t DimValue)
{
    m_DimValue = DimValue;

    if(m_IsItBlank == false)
    {
        m_pPWM->SetDuty(m_DimValue);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Blank
//
//  Parameter(s):
//
//  Return:         None
//
//  Description:    Blank the VFD
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VFD_Driver::Blank(bool IsItBlank)
{
    m_IsItBlank = IsItBlank;

    if(m_IsItBlank == false)
    {
        m_pPWM->SetDuty(m_DimValue);
    }
    else
    {
        m_pPWM->SetDuty(0);
    }
}

//-------------------------------------------------------------------------------------------------
