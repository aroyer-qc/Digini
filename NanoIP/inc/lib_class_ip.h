//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_ip.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2011-2024 Alain Royer.
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
// Include(s)
//-------------------------------------------------------------------------------------------------

#include 	<ip_debug.h>
#include 	<ip_cfg.h>
#include 	<task_IP.h>

#if NIC_IN_USE == NIC_W5100
#include	<w5100.h>
#define 	IP_ARP 					DEF_DISABLED
#define 	IP_UDP 					DEF_DISABLED
#define 	IP_TCP 					DEF_DISABLED
#define 	IP_ICMP					DEF_DISABLED
#define 	IP_HARDWARE_SOCKET		DEF_ENABLED
#endif

#if NIC_IN_USE == NIC_CS8900A
#include	<cs8900a.h>
#define 	IP_ARP 					DEF_ENABLED
#define 	IP_UDP 					DEF_ENABLED
#define 	IP_TCP 					DEF_ENABLED
#define 	IP_ICMP					DEF_ENABLED
#define 	IP_HARDWARE_SOCKET		DEF_DISABLED
#endif

#if (IP_UDP == DEF_ENABLED) & (IP_APP_USE_UDP == DEF_ENABLED)
#include <udp.h>
#endif

#if (IP_TCP == DEF_ENABLED) & (IP_APP_USE_TCP == DEF_ENABLED)
#include <tcp.h>
#endif

#if (IP_ICMP == DEF_ENABLED) & (IP_APP_USE_TCP == DEF_ENABLED)
#include <icmp.h>
#endif

#if (IP_APP_USE_ARP == DEF_ENABLED)
#include <arp.h>
#endif

#if (IP_APP_USE_DHCP == DEF_ENABLED)
#include <dhcp.h>
#endif

#if (IP_APP_USE_DNS == DEF_ENABLED)
#include <dns.h>
#endif

#if (IP_APP_USE_SNTP == DEF_ENABLED)
#include <sntp.h>
#endif

//-------------------------------------------------------------------------------------------------
