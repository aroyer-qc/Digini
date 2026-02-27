//-------------------------------------------------------------------------------------------------
//
//  File : ETH_PHY_driver_interface.h
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
// class definition(s)
//-------------------------------------------------------------------------------------------------

class ETH_PHY_DriverInterface
{
    public:

        virtual SystemState_e       Initialize          (ETH_MAC_DriverInterface* pETH_Driver, uint32_t PHY_Address)    = 0;
        virtual SystemState_e       Uninitialize        (void)                                                          = 0;
        virtual SystemState_e       PowerControl        (ETH_PowerState_e state)                                        = 0;
        virtual SystemState_e       SetInterface        (ETH_MediaInterface_e Interface)                                = 0;
        virtual SystemState_e       SetMode             (ETH_PHY_Mode_e Mode)                                           = 0;
        virtual ETH_LinkState_e     GetLinkState        (void)                                                          = 0;
        virtual ETH_LinkInfo_t      GetLinkInfo         (void)                                                          = 0;
        virtual uint8_t             GetPHY_Address      (void)                                                          = 0;
      #if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
        virtual SystemState_e       SetLinkUpInterrupt  (bool State)                                                    = 0;
      #endif
};

//-------------------------------------------------------------------------------------------------

