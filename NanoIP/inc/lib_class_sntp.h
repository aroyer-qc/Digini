//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_sntp.h
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

#if (IP_USE_SNTP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//#define SNTP_UNIX_START                     2208988800UL                // January 1, 1970
//#define SNTP_TIME_START                     3471292800UL                // January 1, 2010

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum SNTP_State_e
{
    SNTP_STATE_INITIAL,
    SNTP_STATE_WAIT_RESPONSE,
    SNTP_STATE_DONE,
    SNTP_STATE_ERROR
};

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class SNTP_Client
{
    public:

        bool            Initialize                  (NetworkContext& Context);
        bool            Start                       (const IP_Address_t ServerIP);
        void            Process                     (void);
        uint32_t        GetUnixTime                 (void) const                            { return m_UnixTime; }

    private:

        bool            ReceiveResponse             (void);
        bool            ParseResponse               (uint8_t* pPacket, size_t Length);
        uint32_t        GetSystemTime_Seconds_1900  (void);
        uint32_t        Convert1900ToUnix           (uint32_t Seconds1900);

        NetworkContext* m_pContext;
        Socket*         m_pSocket;
        uint32_t        m_UnixTime;
        SNTP_State_e    m_State;
        TickCount_t     m_WaitStart;
};

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_SNTP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
