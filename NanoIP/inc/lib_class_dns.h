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
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class DNS_Client
{
    public:

        void                Initialize          (NetworkContext* pContext);
        bool                SendQuery           (const char* pDomainName);
        bool                Resolve             (const char* pDomainName);
        bool                Process             (void);

        void                SetCallback         (DNS_Callback_t pCallback,IP_Manager* pIP_Manager)  { m_pCallback = pCallback; m_pCallbackContext = pIP_Manager; }
        bool                IsBusy              (void)                                              { return (m_State == DNS_STATE_WAIT_RESPONSE);               }

private:

        bool                ParseResponse       (DNS_Header_t* pMsg, size_t PacketLength);
        size_t              BuildDNS_Query      (DNS_Header_t* pMessage, const char* pDomainName);

        NetworkContext*     m_pContext;
        Socket*             m_pSocket;
        DNS_State_e         m_State;
        uint16_t            m_LastID;
        nOS_Timer           m_TimerQuery;
        DNS_Callback_t      m_pCallback;
        IP_Manager*         m_pCallbackContext;
        IP_Address_t        m_ResolvedIP;
};

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_DNS == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
