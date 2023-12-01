//-------------------------------------------------------------------------------------------------
//
//  File : ethernetif.h
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

#include "lwip/err.h"

//-------------------------------------------------------------------------------------------------

#if (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
 extern "C" {
#endif
err_t    ethernetif_init    (struct netif *netif);
void     ethernetif_input   (void *param);
#ifdef __cplusplus
 }
#endif

//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
 extern "C" {
#endif
  #ifdef ETHERNET_DRIVER_GLOBAL
  ETH_Driver                  ETH_Mac;
  PHY_DRIVER_INTERFACE        ETH_Phy(0);
  PHY_DriverInterface*        myETH_PHY = &ETH_Phy;
  ETH_LinkState_e             ETH_Link;                // Ethernet Link State

uint32_t DBG_RX_Count;
uint32_t DBG_TX_Count;
uint32_t DBG_RX_Drop;
uint32_t DBG_TX_Drop;


 #else
  extern ETH_Driver           ETH_Mac;
  extern PHY_DRIVER_INTERFACE ETH_Phy;
  extern PHY_DriverInterface* myETH_PHY;
  extern ETH_LinkState_e      ETH_Link;                // Ethernet Link State

extern uint32_t DBG_RX_Count;
extern uint32_t DBG_TX_Count;
extern uint32_t DBG_RX_Drop;
extern uint32_t DBG_TX_Drop;

 #endif
#ifdef __cplusplus
 }
#endif

//-------------------------------------------------------------------------------------------------

#endif
