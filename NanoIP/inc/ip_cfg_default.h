//-------------------------------------------------------------------------------------------------
//
//  File : ip_cfg_default.h
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

//-------------------------------------------------------------------------------------------------
// Debug
#define IP_DBG_DHCP                             DEF_DISABLED
#define IP_DBG_DNS								DEF_DISABLED
#define IP_DBG_ARP								DEF_DISABLED
#define IP_DBG_TCP 								DEF_DISABLED
#define IP_DBG_MQTT								DEF_DISABLED
#define IP_DBG_ARP_RETRY_MSG					DEF_DISABLED

//-------------------------------------------------------------------------------------------------
// Deactivate all protocol (for lib_memory MEM_DBG_xxx list)

#define IP_USE_DHCP							    DEF_DISABLED
#define IP_USE_DNS							    DEF_DISABLED
#define IP_USE_HTTP                             DEF_DISABLED
#define IP_USE_ICMP							    DEF_DISABLED
#define IP_USE_MQTT 						    DEF_DISABLED
#define IP_USE_NTP							    DEF_DISABLED
#define IP_USE_SNTP							    DEF_DISABLED
#define IP_USE_SOAP 						    DEF_DISABLED
#define IP_USE_TCP_CLIENT                       DEF_DISABLED
#define IP_USE_TCP_SERVER                       DEF_DISABLED
#define IP_USE_UDP 							    DEF_DISABLED
#define IP_USE_RAW                              DEF_DISABLED

//-------------------------------------------------------------------------------------------------

