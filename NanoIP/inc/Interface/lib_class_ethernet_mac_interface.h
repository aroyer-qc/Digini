//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ethernet_mac_interface.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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

#include "./Digini/Digini/inc/lib_typedef.h"

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class MAC_DriverInterface
{
    public:

        virtual SystemState_e           Initialize              (ETH_MAC_SignalEvent_t CallbackEvent)                               = 0;
        virtual void                    Start                   (void)                                                              = 0;
        virtual SystemState_e           GetMacAddress           (      IP_MAC_Address_t* pMAC_Address)                              = 0;
        virtual SystemState_e           SetMacAddress           (const IP_MAC_Address_t* pMAC_Address)                              = 0;
        virtual SystemState_e           SetAddressFilter        (const IP_MAC_Address_t* pMAC_Address, uint32_t NbAddress)          = 0;
        virtual SystemState_e           SendFrame               (const uint8_t* frame, size_t Length, uint32_t flags)               = 0;
        virtual SystemState_e           ReadFrame               (MemoryNode* pPacket, size_t Length)                                = 0;
        virtual uint32_t                GetRX_FrameSize         (void)                                                              = 0;
      #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
        virtual SystemState_e           GetRX_FrameTime         (ETH_MacTime_t* pTime)                                              = 0;
        virtual SystemState_e           GetTX_FrameTime         (ETH_MacTime_t* pTime)                                              = 0;
        virtual SystemState_e           ControlTimer            (ETH_ControlTimer_e Control, ETH_MacTime_t* pTime)                  = 0;
      #endif
        virtual SystemState_e           PHY_Read                (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* pData)     = 0;
        virtual SystemState_e           PHY_Write               (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t   Data)     = 0;
};

//-------------------------------------------------------------------------------------------------

