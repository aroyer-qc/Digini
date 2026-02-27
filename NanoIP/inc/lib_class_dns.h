//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_dns.h
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

#if (IP_USE_DNS == DEF_ENABLED)


//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef void (*DNS_Callback_t)(void* pContext, bool Success, IP_Address_t ResolveIP);

struct DNS_PendingRequest_t
{
    uint16_t        XID;
    DNS_Callback_t  pCallback;
    void*           pContext;
    TickCount_t     TimeStamp;
    bool            Pending;                        // App requested a DNS lookup
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class DNS_Manager
{
    public:

        void                    Initialize          (NetworkContext& Context);
        bool                    SendQuery           (const char* pDomainName, DNS_Callback_t pCallback, void* pContext);
        bool                    Process             (void);

private:

        inline int              FindFreeSlot        (void);
        int                     FindSlotByXID       (uint16_t XID);
        bool                    ParseResponse       (DNS_Header_t* pMessage, size_t PacketLength, IP_Address_t& OutIP);
        size_t                  BuildDNS_Query      (DNS_Header_t* pMessage, const char* pDomainName);

        NetworkContext*         m_pContext;
        Socket*                 m_pSocket;
        uint16_t                m_XID_Counter;
        DNS_PendingRequest_t    m_Pending[DNS_MAX_PENDING_COUNT];
};

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_DNS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
