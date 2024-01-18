//-------------------------------------------------------------------------------------------------
//
//  File :  arp.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2010-2024 Alain Royer.
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

#ifdef ARP_GLOBAL
    #define ARP_EXTERN
    #define ARP_PRIVATE
#else   
    #define ARP_EXTERN      extern
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define ARP_REQUEST                     htons(1)        // 1
#define ARP_REPLY                       htons(2)        // 2

#define ARP_HARDWARE_TYPE_ETHERNET      htons(1)        // 1

//-------------------------------------------------------------------------------------------------
// macro(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------
    
//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef struct
{
    uint32_t                IP_Addr;
    IP_EthernetAddress_t    Ethernet;
    uint8_t                 Time;
} ARP_TableEntry_t;

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Private variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

// ...

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------
 
class NetARP
{
    public:
    
        void            Inititialize            (void);
        void            ProcessIP               (IP_PacketMsg_t* pRX);
        void            ProcessARP              (IP_PacketMsg_t* pRX);
        void            ProcessOut              (IP_PacketMsg_t* pTX);
        void            Resolve                 (void);
        
    private:
    
};

//-------------------------------------------------------------------------------------------------
