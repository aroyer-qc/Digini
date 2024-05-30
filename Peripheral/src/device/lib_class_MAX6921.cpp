//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_MAX6921.cpp
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

#define LIB_MAX6921_GLOBAL
#include "lib_class_MAX6921.h"
#undef  LIB_MAX6921_GLOBAL

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MAX6921_BIT_STREAM_SIZE             20

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void*       pArg
//                  uint8_t     NumberOfDeviceInChain
//
//  Return:         SystemState_e
//
//  Description:    Initialize the MAX6921 Serial to parallel interface
//
//  Note(s):        Call this init when the OS is started
//
//-------------------------------------------------------------------------------------------------
SystemState_e MAX6921_Driver::Initialize(void* pArg, uint8_t NumberOfDeviceInChain)
{
    uint8_t* pData;
    
    m_pPinStruct = (MAX6921_PinStruct_t*)pArg;

    IO_PinInit(m_pPinStruct->IO_DOut);
    IO_PinInit(m_pPinStruct->IO_Clk);
    IO_PinInit(m_pPinStruct->IO_Load);
    IO_PinInit(m_pPinStruct->IO_Blank);                     // Default value will blank the VFD
    // timer->RegisterCallBack(our callback);
    
    m_ChainSize = NumberOfDeviceInChain * MAX6921_BIT_STREAM_SIZE.

    pData = pMemory->AllocAndClear((m_ChainSize / 8) + 1)   // Reserved x byte from the alloc mem library.
    this->Send(pData);
    IO_SetPinLow(m_pPinStruct->IO_Blank);

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameter(s):   
//
//  Return:         None
//
//  Description:    Start the IRQ process to send a stream to the chain of MAX6921
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void MAX6921_Driver::Send(uint8_t* pArray)
{
    m_BitCounter  = 0;
    m_BitMask     = 0x80;
    m_pDataToSend = pArray;
    // Start the timer
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Callback
//
//  Parameter(s):   
//
//  Return:         None
//
//  Description:    Callback that is register for a specific timer. This handle the stream of bits.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void MAX6921_Driver::Callback(void)
{
    if((*m_pDataToSend & m_BitMask) != 0)   IO_SetPinHigh(m_pPinStruct->IO_DOut);
    else                                    IO_SetPinLow (m_pPinStruct->IO_DOut);
    
    // Process bitmask here to create delay for IO propagation
    m_BitMask >>= 1;
    
    if(m_BitMask == 0)
    {
        m_BitMask = 0x80;
        m_pDataToSend++;
    }

    IO_SetPinHigh(m_pPinStruct->IO_Clk);

    // Process m_BitCounter here to create delay for IO propagation and also according to datasheet
    // LOAD should be raised when CLOCK is high
    m_BitCounter++;
    
    if(m_BitCounter == m_ChainSize)
    {
        IO_SetPinHigh(m_pPinStruct->IO_Load);
        // Stop the Timer..
    }        

    IO_SetPinLow(m_pPinStruct->IO_Clk);

    if(m_BitCounter == m_ChainSize)
    {
        IO_SetPinLow(m_pPinStruct->IO_DOut);            // Not neccessary, but add delay for LOAD
        IO_SetPinLow(m_pPinStruct->IO_Load);
    }        
}

//-------------------------------------------------------------------------------------------------
